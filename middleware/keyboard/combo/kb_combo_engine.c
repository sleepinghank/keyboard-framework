// middleware/keyboard/combo/process_combo.c
#include "kb_combo_engine.h"
#include "kb_fn_action.h"
#include "linkedlist.h"
#include <string.h>
#include "timer.h"
#include "wireless.h"
#include "debug.h"

#define COMBO_DISABLED(combo) (combo->disabled)
#define COMBO_STATE(combo) (combo->state)

#define DISABLE_COMBO(combo) do { combo->disabled = true; } while (0)
#define RESET_COMBO_STATE(combo) do { combo->disabled = false; combo->event = 0; } while (0)

#define NO_COMBO_KEYS_ARE_DOWN (0 == COMBO_STATE(combo))
#define ALL_COMBO_KEYS_ARE_DOWN(state, key_count) (((1 << key_count) - 1) == state)
#define ONLY_ONE_KEY_IS_DOWN(state) !(state & (state - 1))
#define KEY_NOT_YET_RELEASED(state, key_index) ((1 << key_index) & state)
#define KEY_STATE_DOWN(state, key_index) do { state |= (1 << key_index); } while (0)
#define KEY_STATE_UP(state, key_index) do { state &= ~(1 << key_index); } while (0)
#define PRESS_REPEAT_MAX_NUM  15

// 外部引用（在 keyboard.c 中定义）
extern list_t* _key_code_list;
extern list_t* _key_code_list_extend;
extern combo_t key_combos[];
extern uint8_t number_of_combos;
extern uint8_t combinations_flag;

// 是否有键盘组合触发了事件
uint8_t active_event = 0;
// 是否有 Fn 组合键被激活
uint8_t active_fn_combo = 0;
// Fn 键状态
uint8_t FN_st = 0;
// Fn 功能键是否被激活（如 F1, F2 等，而非 S_FN_KEY 本身）
static uint8_t fn_function_fired = 0;

// 内部函数声明
static void button_ticks(combo_t *combo);
static void del_combo_keys(const uint16_t *keys);
static void add_combo_result(const combo_t *combo, uint16_t* arr);
static uint8_t apply_combo(uint16_t combo_index, combo_t *combo);

static void _find_key_index_and_count(const uint16_t *keys, uint16_t keycode,
                                       uint16_t *key_index, uint8_t *key_count) {
    while (true) {
        uint16_t key = pgm_read_word(&keys[*key_count]);
        if (keycode == key) *key_index = *key_count;
        if (COMBO_END == key) break;
        (*key_count)++;
    }
}

static uint8_t apply_combo(uint16_t combo_index, combo_t *combo) {
    node_t* current = _key_code_list->head;
    uint8_t state = 0;

    if (COMBO_DISABLED(combo)) {
        return 0;
    }

    combo->active_status = 0;

    while (current != NULL) {
        uint16_t keycode = current->data.key_code;
        uint8_t  key_count = 0;
        uint16_t key_index = 65535;
        _find_key_index_and_count(combo->keys, keycode, &key_index, &key_count);

        if (65535 == key_index) {
            current = current->next;
            continue;
        }

        KEY_STATE_DOWN(state, key_index);
        if (ALL_COMBO_KEYS_ARE_DOWN(state, key_count)) {
            combo->active_status = 1;
            return 255;
        }
        current = current->next;
    }

    return state;
}

static void button_ticks(combo_t *combo) {
    uint32_t elapsed = (combo->state > 0) ? timer_elapsed32(combo->press_time) : 0;

    // 电平变化检测
    if (combo->active_status != combo->button_level) {
        combo->button_level = combo->active_status;
    }

    // 状态机
    switch (combo->state) {
        case 0:
            if (combo->button_level == 1) {
                combo->event = (uint8_t)PRESS_DOWN;
                combo->press_time = timer_read32();
                combo->repeat = 1;
                combo->state = 1;
            } else {
                combo->event = (uint8_t)NONE_PRESS;
            }
            break;

        case 1:
            if (combo->button_level != 1) {
                combo->event = (uint8_t)PRESS_UP;
                combo->press_time = timer_read32();
                combo->state = 2;
            } else if (elapsed > combo->long_press_ms) {
                combo->event = (uint8_t)LONG_PRESS_START;
                combo->state = 5;
            }
            break;

        case 2:
            if (combo->button_level == 1) {
                combo->event = (uint8_t)PRESS_DOWN;
                if (combo->repeat != PRESS_REPEAT_MAX_NUM) {
                    combo->repeat++;
                }
                combo->press_time = timer_read32();
                combo->state = 3;
            } else if (elapsed > SHORT_MS) {
                if (combo->repeat == 1) {
                    combo->event = (uint8_t)SINGLE_CLICK;
                } else if (combo->repeat == 2) {
                    combo->event = (uint8_t)DOUBLE_CLICK;
                }
                combo->state = 0;
            }
            break;

        case 3:
            if (combo->button_level != 1) {
                combo->event = (uint8_t)PRESS_UP;
                if (elapsed < SHORT_MS) {
                    combo->press_time = timer_read32();
                    combo->state = 2;
                } else {
                    combo->state = 0;
                }
            } else if (elapsed > SHORT_MS) {
                combo->state = 1;
            }
            break;

        case 5:
            if (combo->button_level == 1) {
                combo->event = (uint8_t)LONG_PRESS_HOLD;
            } else {
                combo->event = (uint8_t)PRESS_UP;
                combo->state = 0;
            }
            break;

        default:
            combo->state = 0;
            break;
    }
}

static void add_combo_result(const combo_t *combo, uint16_t* buf) {
    memset(buf, 0, 10 * sizeof(uint16_t));
    if (combo->cb[combo->event] != NULL) {
        uint8_t idx = combo->cb[combo->event](buf);
        for (uint8_t i = 0; i < idx; ++i) {
            add(buf[i], _key_code_list_extend);
        }
    }
}

static void del_combo_keys(const uint16_t *keys) {
    uint8_t key_count = 0;
    while (true) {
        uint16_t key = pgm_read_word(&keys[key_count]);
        if (COMBO_END == key) break;
        deactivate(key, _key_code_list);
        key_count++;
    }
}

void combo_init(void) {
    for (uint8_t i = 0; i < number_of_combos; ++i) {
        combo_t *combo = &key_combos[i];
        uint8_t  key_count = 0;
        uint16_t key_index = 65535;
        _find_key_index_and_count(combo->keys, S_FN_KEY, &key_index, &key_count);
        // 存在 Fn 键并且按键数量大于1，即为 Fn 组合键
        if ((65535 != key_index) && key_count > 1) {
            combo->fn_combo = 1;
        }
    }
}

void combo_task(key_update_st_t _keyUpdateSt) {
    uint16_t buf[10] = {0};

    if (_keyUpdateSt == GHOST_KEY) {
        return;
    }

    // 清空所有扩展键
    del_all_child(_key_code_list_extend);
    active_event = 0;
    active_fn_combo = 0;
    fn_function_fired = 0;

    // 循环所有事件，逐个进行处理
    for (uint8_t i = 0; i < number_of_combos; ++i) {
        combo_t *combo = &key_combos[i];
        if (combo -> system_type != ALL_SYSTEM){
            // 判断当前系统不触发，直接跳过
            //DEBUG_log("ccombo -> system_type = %d",combo -> system_type);
            if ((combo -> system_type  & host_system_type)== 0) continue;
        }
        // 判断是否触发组合
        apply_combo(i, combo);
        if (combo->state > 0) {
            // 如果组合键处于按下状态，进行状态机处理
             dprintf("Combo %d state machine processing, event: %d\n", i, combo->event);
        }
        // 按键状态机处理
        button_ticks(combo);
        // 判断是否执行事件
        if (combo->event < number_of_event && combo->cb[combo->event]) {
            active_event = 1;
            del_combo_keys(combo->keys);
            add_combo_result(combo, buf);
            // 如果 FN_st == 1 且不是单纯的 S_FN_KEY 按键事件，则标记 Fn 功能键被触发
            if (FN_st == 1 && combo->fn_combo == 1) {
                fn_function_fired = 1;
            }
            combo->event = (uint8_t)NONE_PRESS;
        } else if (combo->active_status == 1) {
            del_combo_keys(combo->keys);
        }

        if (combo->active_status == 1 && combo->fn_combo == 1) {
            active_fn_combo = 1;
        }
    }

    // Fn 组合键触发后，设置 combinations_flag
    if (active_fn_combo == 1) {
        combinations_flag = 0;
    }

    // Earth 状态机后处理决策
    earth_post_loop_decision(fn_function_fired, _key_code_list,_key_code_list_extend);
}
