// middleware/keyboard/combo/FN_Combo.c
#include "kb_fn_action.h"
#include "keyboard.h"
#include "keycode.h"
#include "kb_combo_engine.h"
#include "wireless.h"
#include "linkedlist.h"
#include "kb904/config_hw.h"
#include <stddef.h>
#include "debug.h"

// 外部变量
extern uint8_t FN_st;
extern uint8_t combinations_flag;

// Apple Globe/Earth 键 - 用于切换输入法


// ============================================================================
// Earth 状态机定义
// ============================================================================

/**
 * Earth 状态机状态枚举
 *
 * 状态转换流程：
 * EARTH_IDLE → [FN_DOWN] → EARTH_PENDING
 *                               │
 *               ┌───────────────┼───────────────┐
 *               ▼               ▼               ▼
 *         fn_function_fired   普通键触发       Apple 长按
 *               │               │               │
 *               ▼               ▼               ▼
 *        EARTH_FN_TAKEN    EARTH_DOWN       EARTH_HOLD
 *               │               │               │
 *               └───────────────┴───────┬───────┘
 *                       ▼
 *                  EARTH_IDLE
 */
typedef enum {
    EARTH_IDLE = 0,      // 空闲状态
    EARTH_PENDING,       // FN 键按下，等待决策
    EARTH_DOWN,          // Earth 键已进入按下保持态（普通键组合）
    EARTH_FN_TAKEN,      // Fn 功能键被激活，Earth 不发送
    EARTH_HOLD,          // Apple 平台长按保持 Earth
} earth_state_t;

static earth_state_t earth_state = EARTH_IDLE;

// ============================================================================
// 辅助函数
// ============================================================================

/**
 * @brief 判断当前是否为 Apple 平台
 */
static inline uint8_t is_apple_platform(void) {
    return (host_system_type == IOS || host_system_type == MAC);
}

/**
 * @brief 填充当前平台对应的 Earth 键序列
 */
static uint8_t fill_earth_keys(uint16_t* add_keys) {
    uint8_t idx = 0;

    switch (host_system_type) {
        case IOS:
        case MAC:
            add_keys[idx++] = KC_KEYBOARD_LAYOUT;
            break;
        case WIN:
            add_keys[idx++] = KC_LEFT_GUI;
            add_keys[idx++] = KC_SPACE;
            break;
        case ANDROID:
            add_keys[idx++] = KC_LEFT_SHIFT;
            add_keys[idx++] = KC_SPACE;
            break;
        default:
            add_keys[idx++] = KC_KEYBOARD_LAYOUT;
            break;
    }

    return idx;
}

/**
 * @brief 将 Earth 键序列加入扩展上报列表
 */
static void push_earth_keys(list_t* key_list_extend) {
    uint16_t earth_keys[2] = {0};
    uint8_t idx = fill_earth_keys(earth_keys);

    for (uint8_t i = 0; i < idx; ++i) {
        add(earth_keys[i], key_list_extend);
    }
}

/**
 * @brief 检测按键列表中是否有普通键（非 S_FN_KEY）
 */
static uint8_t has_normal_key_in_list(list_t* key_list) {
    if (key_list == NULL || key_list->head == NULL) {
        return 0;
    }

    node_t* current = key_list->head;
    while (current != NULL) {
        uint16_t keycode = current->data.key_code;
        // 跳过 S_FN_KEY 和媒体键
        if (keycode != S_FN_KEY && !(IS_CONSUMER_KEYCODE(keycode)) && current->data.is_report == 1) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

// ============================================================================
// Earth 状态机接口实现
// ============================================================================

void earth_post_loop_decision(uint8_t fn_fired, list_t* key_list, list_t* key_list_extend) {
    if (earth_state == EARTH_PENDING) {
        if (fn_fired) {
            // Fn 功能键被激活，Earth 不发送
            earth_state = EARTH_FN_TAKEN;
            return;
        }

        if (has_normal_key_in_list(key_list)) {
            combinations_flag = 0;
            earth_state = EARTH_DOWN;
        } else {
            return;
        }
    }

    if (earth_state == EARTH_DOWN || earth_state == EARTH_HOLD) {
        push_earth_keys(key_list_extend);
    }
}

uint8_t earth_is_down(void) {
    return (earth_state == EARTH_DOWN || earth_state == EARTH_HOLD) ? 1 : 0;
}

void earth_reset(void) {
    earth_state = EARTH_IDLE;
}

void earth_set_fn_taken(void) {
    earth_state = EARTH_FN_TAKEN;
}

// ============================================================================
// Fn 键回调实现
// ============================================================================

// Fn 键按下
uint8_t FN_DOWN_KEY(uint16_t* add_keys) {
    FN_st = 1;
    combinations_flag = 1;
    // 设置 Earth 状态为等待决策
    earth_state = EARTH_PENDING;
    return 0;
}

// Fn 键长按
uint8_t FN_LONG_PRESS_KEY(uint16_t* add_keys) {
    (void)add_keys;

    if (is_apple_platform() && earth_state == EARTH_PENDING) {
        combinations_flag = 0;
        earth_state = EARTH_HOLD;
    }

    return 0;
}

// Fn 键释放
uint8_t FN_UP_KEY(uint16_t* add_keys) {
    uint8_t idx = 0;
    FN_st = 0;
    combinations_flag = 0;
    dprintf("FN_UP_KEY called, current Earth state: %d\r\n", earth_state);
    // 根据 Earth 状态机决定发送动作
    switch (earth_state) {
        case EARTH_PENDING:
            // 没有触发任何 Fn 功能或普通键，发送 Earth tap
            idx = fill_earth_keys(add_keys);
            break;
        case EARTH_DOWN:
        case EARTH_HOLD:
            // Earth 保持由 earth_post_loop_decision 注入，本帧不再注入即可自然释放
            break;
        case EARTH_FN_TAKEN:
        case EARTH_IDLE:
        default:
            // 无需发送任何动作
            break;
    }

    // 重置状态
    earth_state = EARTH_IDLE;
    return idx;
}

// 亮度降低
uint8_t Bringhness_Down(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F1;
    } else {
        add_keys[idx++] = QK_BACKLIGHT_DOWN;
    }
    return idx;
}

// 亮度增加
uint8_t Bringhness_Up(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F2;
    } else {
        add_keys[idx++] = QK_BACKLIGHT_UP;
    }
    return idx;
}

// 音量降低
uint8_t Volume_Down(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F10;
    } else {
        add_keys[idx++] = KC_AUDIO_VOL_DOWN;
    }
    return idx;
}

// 音量增加
uint8_t Volume_Up(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F11;
    } else {
        add_keys[idx++] = KC_AUDIO_VOL_UP;
    }
    return idx;
}

// 静音
uint8_t Volume_Mute(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F9;
    } else {
        add_keys[idx++] = KC_AUDIO_MUTE;
    }
    return idx;
}

// 上一首
uint8_t Media_Previous(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F6;
    } else {
        add_keys[idx++] = KC_MEDIA_PREV_TRACK;
    }
    return idx;
}

// 播放/暂停
uint8_t Media_Play_Pause(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F7;
    } else {
        add_keys[idx++] = KC_MEDIA_PLAY_PAUSE;
    }
    return idx;
}

// 下一首
uint8_t Media_Next(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F8;
    } else {
        add_keys[idx++] = KC_MEDIA_NEXT_TRACK;
    }
    return idx;
}

// 截图
uint8_t Print_Screen(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F4;
    } else {
        // Windows: Win+Shift+S
        add_keys[idx++] = KC_LEFT_GUI;
        add_keys[idx++] = KC_LEFT_SHIFT;
        add_keys[idx++] = KC_S;
    }
    return idx;
}

// 锁屏
uint8_t Lock_Screen(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F12;
    } else {
        // Windows: Win+L
        add_keys[idx++] = KC_LEFT_GUI;
        add_keys[idx++] = KC_L;
    }
    return idx;
}

// 搜索
uint8_t Search(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F3;
    } else {
        add_keys[idx++] = KC_WWW_SEARCH;
    }
    return idx;
}

// 多任务
uint8_t Multi_Task(uint16_t* add_keys) {
    uint8_t idx = 0;
    // Windows: Win+Tab
    add_keys[idx++] = KC_LEFT_GUI;
    add_keys[idx++] = KC_TAB;
    return idx;
}

// 小地球 (Apple Globe 键)
uint8_t Earth(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F5;
    } else {
        add_keys[idx++] = KC_KEYBOARD_LAYOUT;
    }
    return idx;
}

// Fn+Esc
uint8_t FN_ESC_button(uint16_t* add_keys) {
    uint8_t idx = 0;
    add_keys[idx++] = KC_ESCAPE;
    return idx;
}

/**
 * @brief Siri 调用
 *
 * 触发设备的语音命令功能
 */
uint8_t Siri_Invoke(uint16_t* add_keys) {
    uint8_t idx = 0;
    add_keys[idx++] = KC_KEYBOARD_LAYOUT;
    add_keys[idx++] = KC_S;
    return idx;
}
