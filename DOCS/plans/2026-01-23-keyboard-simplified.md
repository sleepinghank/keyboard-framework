# 精简键盘按键处理流程 Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 精简 middleware/keyboard 模块，复用 kb-04122 的组合键实现，创建最小可用的按键处理流程

**Architecture:** 保持分层架构，复用驱动层 matrix/debounce。只保留层切换功能（MO + TG），组合键直接复用 kb-04122 实现。通过链表管理按键状态，通过 report_buffer_enqueue() 发送 HID 报告。平台相关功能直接设置 OSAL 事件调用。

**Tech Stack:** C11, CMake, MinGW, Unity test framework, Git Worktrees

---

## Task 1: 创建隔离工作区

使用 git worktrees 创建隔离工作区进行开发，保持主分支干净。

**Step 1: 创建功能分支**

```bash
git checkout -b feature/keyboard-simplified
```

**Step 2: 验证分支创建成功**

```bash
git branch --show-current
```
Expected: `feature/keyboard-simplified`

**Step 3: Commit 空提交作为起点（可选）**

```bash
git commit --allow-empty -m "chore: start keyboard simplified refactor"
```

---

## Task 2: 复制并适配 linkedlist 模块

**Files:**
- Create: `utils/linkedlist.c`
- Create: `utils/linkedlist.h`
- Reference: `code_example/kb-04122/PAR2860/linkedlist.c`
- Reference: `code_example/kb-04122/PAR2860/Include/linkedlist.h`

**Step 1: 创建 linkedlist.h**

```c
// utils/linkedlist.h
#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct node node_t;
typedef struct list list_t;

typedef enum _report_t {
    NONE_RPT = 0,
    NORMAL_RPT = 1,
    FUNCTION_RPT = 2,
} report_t;

typedef struct {
    uint16_t key_code;      // 键值
    uint8_t cycle;          // 按下时长（防抖用）
    uint8_t is_report;      // 是否上报
    report_t report_type;   // 报告类型
} bouncing_data_t;

struct node {
    bouncing_data_t data;
    node_t* next;
};

struct list {
    node_t* head;
};

// 链表操作函数
list_t* make_list_proc(void);
void add(uint16_t data, list_t* list);
void del(uint16_t data, list_t* list);
void deactivate(uint16_t data, list_t* list);
void del_all_child(list_t* list);
uint8_t find_key(list_t* list, uint16_t key);
uint8_t find_activate_key(list_t* list, uint16_t key);
void increase_cycle(list_t* list);
void pop_key(list_t* list, uint16_t* key, uint8_t* sz, uint8_t cycle_limit);
void display(list_t* list);
void reverse(list_t* list);
void destroy(list_t* list);
uint8_t insert_key(list_t* list, uint16_t _pre, uint16_t _new);

#endif
```

**Step 2: 创建 linkedlist.c**

```c
// utils/linkedlist.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linkedlist.h"

static node_t* create_node(bouncing_data_t data) {
    node_t* new_node = malloc(sizeof(node_t));
    if (!new_node) return NULL;
    new_node->data = data;
    new_node->next = NULL;
    return new_node;
}

list_t* make_list_proc(void) {
    list_t* list = malloc(sizeof(list_t));
    if (!list) return NULL;
    list->head = NULL;
    return list;
}

void display(list_t* list) {
    node_t* current = list->head;
    if (list->head == NULL) return;
    for (; current != NULL; current = current->next) {
        // 调试输出
    }
}

void add(uint16_t data, list_t* list) {
    node_t* current = NULL;
    bouncing_data_t bouncing_data;
    memset((uint8_t*)&bouncing_data, 0x00, sizeof(bouncing_data));
    bouncing_data.key_code = data;
    bouncing_data.is_report = 1;

    if (list->head == NULL) {
        list->head = create_node(bouncing_data);
    } else {
        current = list->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = create_node(bouncing_data);
    }
}

void del(uint16_t data, list_t* list) {
    node_t* current = list->head;
    node_t* previous = current;

    while (current != NULL) {
        if (current->data.key_code == data) {
            previous->next = current->next;
            if (current == list->head) {
                list->head = current->next;
            }
            free(current);
            return;
        }
        previous = current;
        current = current->next;
    }
}

static void delete_node(node_t* node) {
    if (node == NULL) return;
    delete_node(node->next);
    free(node);
}

void del_all_child(list_t* list) {
    node_t* head = list->head;
    delete_node(head);
    list->head = NULL;
}

void deactivate(uint16_t data, list_t* list) {
    node_t* current = list->head;
    while (current != NULL) {
        if (current->data.key_code == data) {
            current->data.is_report = 0;
            return;
        }
        current = current->next;
    }
}

void reverse(list_t* list) {
    node_t* reversed = NULL;
    node_t* current = list->head;
    node_t* temp = NULL;

    while (current != NULL) {
        temp = current;
        current = current->next;
        temp->next = reversed;
        reversed = temp;
    }
    list->head = reversed;
}

void destroy(list_t* list) {
    node_t* current = list->head;
    node_t* next = current;

    if (list == NULL) return;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    free(list);
}

void increase_cycle(list_t* list) {
    node_t* current = list->head;
    while (current != NULL) {
        current->data.cycle++;
        current = current->next;
    }
}

void pop_key(list_t* list, uint16_t* key, uint8_t* sz, uint8_t cycle_limit) {
    node_t* current = list->head;
    node_t* previous = current;
    uint8_t idx = 0;
    *sz = 0;

    while (current != NULL) {
        if (current->data.cycle >= cycle_limit) {
            previous->next = current->next;
            key[idx++] = current->data.key_code;
            *sz = *sz + 1;

            if (*sz >= 20) return;

            if (current == list->head) {
                list->head = current->next;
            }
            free(current);
            current = previous->next;
            continue;
        }
        previous = current;
        current = current->next;
    }
}

uint8_t find_key(list_t* list, uint16_t key) {
    node_t* current = list->head;
    while (current != NULL) {
        if (current->data.key_code == key) return 1;
        current = current->next;
    }
    return 0;
}

uint8_t find_activate_key(list_t* list, uint16_t key) {
    node_t* current = list->head;
    while (current != NULL) {
        if (current->data.key_code == key && current->data.is_report == 1) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

uint8_t insert_key(list_t* list, uint16_t pre, uint16_t n) {
    node_t* current = list->head;
    node_t* next = current;

    while (current != NULL) {
        next = current->next;
        if (current->data.key_code == pre) {
            bouncing_data_t bouncing_data = {0};
            bouncing_data.key_code = n;
            current->next = create_node(bouncing_data);
            current->next->next = next;
            return 1;
        }
        current = next;
    }
    return 0;
}
```

**Step 3: 验证文件创建成功**

```bash
ls utils/linkedlist.*
```
Expected: `linkedlist.c  linkedlist.h`

**Step 4: Commit**

```bash
git add utils/linkedlist.c utils/linkedlist.h
git commit -m "feat(utils): add linkedlist module from kb-04122"
```

---

## Task 3: 创建组合键目录并复制 combo 文件

**Files:**
- Create: `middleware/keyboard/combo/` (directory)
- Create: `middleware/keyboard/combo/process_combo.h`
- Create: `middleware/keyboard/combo/process_combo.c`
- Create: `middleware/keyboard/combo/FN_Combo.h`
- Create: `middleware/keyboard/combo/FN_Combo.c`
- Create: `middleware/keyboard/combo/Special_Combo.h`
- Create: `middleware/keyboard/combo/Special_Combo.c`
- Create: `middleware/keyboard/combo/media_combo.c`

**Step 1: 创建 combo 目录**

```bash
mkdir -p middleware/keyboard/combo
```

**Step 2: 创建 process_combo.h**

```c
// middleware/keyboard/combo/process_combo.h
#ifndef C_PROCESS_COMBO_H
#define C_PROCESS_COMBO_H

#include <stdint.h>
#include <stdbool.h>
#include "keycode.h"

// 平台无关宏定义
#define PROGMEM
#define pgm_read_word(address_short) *((uint16_t*)(address_short))

// 时间配置（单位：ms）
#define TICKS_INTERVAL    5
#define DEBOUNCE_TICKS    3
#define SHORT_TICKS       (300 / TICKS_INTERVAL)
#define LONG_TICKS        (2500 / TICKS_INTERVAL)

// 组合键结束标志
#define COMBO_END 0

// 按键更新状态
typedef enum {
    NO_KEY_UPDATE = 0,
    KEY_UPDATE,
    GHOST_KEY
} key_update_st_t;

// 回调函数类型
typedef uint8_t (*BtnCallback)(uint16_t* add_keys);

// 按键事件种类
typedef enum {
    PRESS_DOWN = 0,         // 按下
    PRESS_UP,               // 释放
    PRESS_REPEAT,           // 重复
    SINGLE_CLICK,           // 单击
    DOUBLE_CLICK,           // 双击
    LONG_PRESS_START,       // 长按开始
    LONG_PRESS_HOLD,        // 长按保持
    number_of_event,        // 事件数量
    NONE_PRESS              // 无事件
} PressEvent;

// 组合键对象
typedef struct combo_t {
    const uint16_t *keys;           // 组合键按键列表
    bool  disabled;                 // 是否禁用
    uint16_t ticks;                 // 计时器
    uint8_t  repeat : 4;            // 重复次数
    uint8_t  event : 4;             // 当前事件
    uint8_t  state : 5;             // 状态机状态
    uint8_t  fn_combo : 1;          // 是否为 Fn 组合键
    uint8_t  active_status : 1;     // 是否激活
    uint8_t  button_level : 1;      // 当前电平
    uint16_t long_press_ticks;      // 长按阈值
    BtnCallback cb[number_of_event]; // 事件回调数组
} combo_t;

// 组合键定义宏
#define COMBO(ck, event_idx, callback) \
    { .keys = &(ck)[0], .cb[event_idx] = (callback), \
      .event = (uint8_t)NONE_PRESS, .long_press_ticks = LONG_TICKS, .state = 0 }

#define COMBO2(ck, event_idx1, callback1, event_idx2, callback2) \
    { .keys = &(ck)[0], .cb[event_idx1] = (callback1), .cb[event_idx2] = (callback2), \
      .event = (uint8_t)NONE_PRESS, .long_press_ticks = LONG_TICKS, .state = 0 }

#define COMBO3(ck, event_idx1, callback1, event_idx2, callback2, event_idx3, callback3) \
    { .keys = &(ck)[0], .cb[event_idx1] = (callback1), .cb[event_idx2] = (callback2), \
      .cb[event_idx3] = (callback3), .event = (uint8_t)NONE_PRESS, \
      .long_press_ticks = LONG_TICKS, .state = 0 }

#define COMBO_LONG_TICKS(ck, long_tick, event_idx, callback) \
    { .keys = &(ck)[0], .cb[event_idx] = (callback), \
      .event = (uint8_t)NONE_PRESS, \
      .long_press_ticks = (uint16_t)(long_tick / TICKS_INTERVAL), .state = 0 }

#define COMBO2_LONG_TICKS(ck, long_tick, event_idx1, callback1, event_idx2, callback2) \
    { .keys = &(ck)[0], .cb[event_idx1] = (callback1), .cb[event_idx2] = (callback2), \
      .event = (uint8_t)NONE_PRESS, \
      .long_press_ticks = (uint16_t)(long_tick / TICKS_INTERVAL), .state = 0 }

#define COMBO3_LONG_TICKS(ck, long_tick, event_idx1, callback1, event_idx2, callback2, event_idx3, callback3) \
    { .keys = &(ck)[0], .cb[event_idx1] = (callback1), .cb[event_idx2] = (callback2), \
      .cb[event_idx3] = (callback3), .event = (uint8_t)NONE_PRESS, \
      .long_press_ticks = (uint16_t)(long_tick / TICKS_INTERVAL), .state = 0 }

// 组合键处理函数
void combo_init(void);
void combo_task(key_update_st_t _keyUpdateSt);

#endif // C_PROCESS_COMBO_H
```

**Step 3: 创建 process_combo.c**

```c
// middleware/keyboard/combo/process_combo.c
#include "process_combo.h"
#include "linkedlist.h"
#include <string.h>

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
    if ((combo->state) > 0) combo->ticks++;

    // 电平变化检测
    if (combo->active_status != combo->button_level) {
        combo->button_level = combo->active_status;
    }

    // 状态机
    switch (combo->state) {
        case 0:
            if (combo->button_level == 1) {
                combo->event = (uint8_t)PRESS_DOWN;
                combo->ticks = 0;
                combo->repeat = 1;
                combo->state = 1;
            } else {
                combo->event = (uint8_t)NONE_PRESS;
            }
            break;

        case 1:
            if (combo->button_level != 1) {
                combo->event = (uint8_t)PRESS_UP;
                combo->ticks = 0;
                combo->state = 2;
            } else if (combo->ticks > combo->long_press_ticks) {
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
                combo->ticks = 0;
                combo->state = 3;
            } else if (combo->ticks > SHORT_TICKS) {
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
                if (combo->ticks < SHORT_TICKS) {
                    combo->ticks = 0;
                    combo->state = 2;
                } else {
                    combo->state = 0;
                }
            } else if (combo->ticks > SHORT_TICKS) {
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
    memset(buf, 0, 20 * sizeof(uint16_t));
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

    // 循环所有事件，逐个进行处理
    for (uint8_t i = 0; i < number_of_combos; ++i) {
        combo_t *combo = &key_combos[i];
        // 判断是否触发组合
        apply_combo(i, combo);
        // 按键状态机处理
        button_ticks(combo);
        // 判断是否执行事件
        if (combo->event < number_of_event && combo->cb[combo->event]) {
            active_event = 1;
            del_combo_keys(combo->keys);
            add_combo_result(combo, buf);
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
}
```

**Step 4: 创建 FN_Combo.h**

```c
// middleware/keyboard/combo/FN_Combo.h
#ifndef __FN_COMBO_H__
#define __FN_COMBO_H__

#include <stdint.h>

// 亮度控制
uint8_t Bringhness_Down(uint16_t* add_keys);
uint8_t Bringhness_Up(uint16_t* add_keys);

// 音量控制
uint8_t Volume_Down(uint16_t* add_keys);
uint8_t Volume_Up(uint16_t* add_keys);
uint8_t Volume_Mute(uint16_t* add_keys);

// 媒体控制
uint8_t Media_Play_Pause(uint16_t* add_keys);
uint8_t Media_Next(uint16_t* add_keys);
uint8_t Media_Previous(uint16_t* add_keys);

// 系统功能
uint8_t Multi_Task(uint16_t* add_keys);
uint8_t Search(uint16_t* add_keys);
uint8_t Print_Screen(uint16_t* add_keys);
uint8_t Lock_Screen(uint16_t* add_keys);
uint8_t Earth(uint16_t* add_keys);

// Fn 键处理
uint8_t FN_DOWN_KEY(uint16_t* add_keys);
uint8_t FN_UP_KEY(uint16_t* add_keys);
uint8_t FN_ESC_button(uint16_t* add_keys);

#endif
```

**Step 5: 创建 FN_Combo.c（精简版）**

```c
// middleware/keyboard/combo/FN_Combo.c
#include "FN_Combo.h"
#include "keycode.h"
#include "process_combo.h"

// 外部变量
extern uint8_t FN_st;
extern uint8_t combinations_flag;

// Fn 键按下
uint8_t FN_DOWN_KEY(uint16_t* add_keys) {
    FN_st = 1;
    combinations_flag = 1;
    return 0;
}

// Fn 键释放
uint8_t FN_UP_KEY(uint16_t* add_keys) {
    FN_st = 0;
    combinations_flag = 0;
    return 0;
}

// 亮度降低
uint8_t Bringhness_Down(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KB_F1;
    } else {
        add_keys[idx++] = M_BACKLIGHT_DOWN;
    }
    return idx;
}

// 亮度增加
uint8_t Bringhness_Up(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KB_F2;
    } else {
        add_keys[idx++] = M_BACKLIGHT_UP;
    }
    return idx;
}

// 音量降低
uint8_t Volume_Down(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KB_F10;
    } else {
        add_keys[idx++] = M_VOLUME_DOWN;
    }
    return idx;
}

// 音量增加
uint8_t Volume_Up(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KB_F11;
    } else {
        add_keys[idx++] = M_VOLUME_UP;
    }
    return idx;
}

// 静音
uint8_t Volume_Mute(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KB_F9;
    } else {
        add_keys[idx++] = M_MUTE;
    }
    return idx;
}

// 上一首
uint8_t Media_Previous(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KB_F6;
    } else {
        add_keys[idx++] = M_SCAN_PREVIOUS_TRACK;
    }
    return idx;
}

// 播放/暂停
uint8_t Media_Play_Pause(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KB_F7;
    } else {
        add_keys[idx++] = M_PLAY_N_PAUSE;
    }
    return idx;
}

// 下一首
uint8_t Media_Next(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KB_F8;
    } else {
        add_keys[idx++] = M_SCAN_NEXT_TRACK;
    }
    return idx;
}

// 截图
uint8_t Print_Screen(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KB_F4;
    } else {
        // Windows: Win+Shift+S
        add_keys[idx++] = KB_L_GUI;
        add_keys[idx++] = KB_L_SHIFT;
        add_keys[idx++] = KB_S;
    }
    return idx;
}

// 锁屏
uint8_t Lock_Screen(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KB_F12;
    } else {
        // Windows: Win+L
        add_keys[idx++] = KB_L_GUI;
        add_keys[idx++] = KB_L;
    }
    return idx;
}

// 搜索
uint8_t Search(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KB_F3;
    } else {
        add_keys[idx++] = M_WWW_SEARCH;
    }
    return idx;
}

// 多任务
uint8_t Multi_Task(uint16_t* add_keys) {
    uint8_t idx = 0;
    // Windows: Win+Tab
    add_keys[idx++] = KB_L_GUI;
    add_keys[idx++] = KB_TAB;
    return idx;
}

// 小地球
uint8_t Earth(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KB_F5;
    } else {
        add_keys[idx++] = M_EARTH;
    }
    return idx;
}

// Fn+Esc
uint8_t FN_ESC_button(uint16_t* add_keys) {
    uint8_t idx = 0;
    add_keys[idx++] = KB_ESCAPE;
    return idx;
}
```

**Step 6: 创建 Special_Combo.h**

```c
// middleware/keyboard/combo/Special_Combo.h
#ifndef __SPECIAL_COMBO_H__
#define __SPECIAL_COMBO_H__

#include <stdint.h>

// 特殊功能（通过 OSAL 事件触发）
uint8_t Set_FN_Lock(uint16_t* add_keys);
uint8_t Backlight_Color(uint16_t* add_keys);
uint8_t Backlight_Light(uint16_t* add_keys);
uint8_t Touch_Locked(uint16_t* add_keys);
uint8_t Factory_Reset(uint16_t* add_keys);
uint8_t Bat_Check(uint16_t* add_keys);

// 配对功能
uint8_t Pair_button(uint16_t* add_keys);

#endif
```

**Step 7: 创建 Special_Combo.c（使用 OSAL 事件）**

```c
// middleware/keyboard/combo/Special_Combo.c
#include "Special_Combo.h"
#include "osal.h"

// 组合键触发标志位
uint8_t combinations_flag = 1;

// OSAL 事件定义（需要在 sys_config.h 或相关头文件中定义）
// 示例事件 ID，根据实际项目调整
#ifndef SYS_EVT_STORAGE_WRITE
#define SYS_EVT_STORAGE_WRITE    0x0001
#endif
#ifndef SYS_EVT_FACTORY_RESET
#define SYS_EVT_FACTORY_RESET    0x0002
#endif
#ifndef SYS_EVT_BLE_PAIRING
#define SYS_EVT_BLE_PAIRING      0x0004
#endif
#ifndef SYS_EVT_BACKLIGHT_COLOR
#define SYS_EVT_BACKLIGHT_COLOR  0x0008
#endif
#ifndef SYS_EVT_BACKLIGHT_BRIGHT
#define SYS_EVT_BACKLIGHT_BRIGHT 0x0010
#endif
#ifndef SYS_EVT_TOUCH_TOGGLE
#define SYS_EVT_TOUCH_TOGGLE     0x0020
#endif
#ifndef SYS_EVT_BATTERY_CHECK
#define SYS_EVT_BATTERY_CHECK    0x0040
#endif

// 系统服务任务 ID（需要根据实际项目定义）
#ifndef SYSTEM_SERVICE_TASK_ID
#define SYSTEM_SERVICE_TASK_ID   0
#endif

// Fn 锁定
uint8_t Set_FN_Lock(uint16_t* add_keys) {
    // TODO: 实现 Fn 锁定逻辑
    osal_set_event(SYSTEM_SERVICE_TASK_ID, SYS_EVT_STORAGE_WRITE);
    return 0;
}

// 切换背光颜色
uint8_t Backlight_Color(uint16_t* add_keys) {
    osal_set_event(SYSTEM_SERVICE_TASK_ID, SYS_EVT_BACKLIGHT_COLOR);
    osal_set_event(SYSTEM_SERVICE_TASK_ID, SYS_EVT_STORAGE_WRITE);
    return 0;
}

// 切换背光亮度
uint8_t Backlight_Light(uint16_t* add_keys) {
    osal_set_event(SYSTEM_SERVICE_TASK_ID, SYS_EVT_BACKLIGHT_BRIGHT);
    osal_set_event(SYSTEM_SERVICE_TASK_ID, SYS_EVT_STORAGE_WRITE);
    return 0;
}

// 触摸板锁定
uint8_t Touch_Locked(uint16_t* add_keys) {
    osal_set_event(SYSTEM_SERVICE_TASK_ID, SYS_EVT_TOUCH_TOGGLE);
    osal_set_event(SYSTEM_SERVICE_TASK_ID, SYS_EVT_STORAGE_WRITE);
    return 0;
}

// 恢复出厂设置
uint8_t Factory_Reset(uint16_t* add_keys) {
    osal_set_event(SYSTEM_SERVICE_TASK_ID, SYS_EVT_FACTORY_RESET);
    return 0;
}

// 电池检测
uint8_t Bat_Check(uint16_t* add_keys) {
    osal_set_event(SYSTEM_SERVICE_TASK_ID, SYS_EVT_BATTERY_CHECK);
    return 0;
}

// 配对按钮
uint8_t Pair_button(uint16_t* add_keys) {
    osal_set_event(SYSTEM_SERVICE_TASK_ID, SYS_EVT_BLE_PAIRING);
    return 0;
}
```

**Step 8: 创建 media_combo.c（组合键定义）**

```c
// middleware/keyboard/combo/media_combo.c
#include "process_combo.h"
#include "FN_Combo.h"
#include "Special_Combo.h"
#include "keycode.h"

// 组合键枚举
enum combos {
    FN_DOWN_ID = 0,
    FN_UP_ID,
    FN_F1_ID,
    FN_F2_ID,
    FN_F3_ID,
    FN_F4_ID,
    FN_F5_ID,
    FN_F6_ID,
    FN_F7_ID,
    FN_F8_ID,
    FN_F9_ID,
    FN_F10_ID,
    FN_F11_ID,
    FN_F12_ID,
    FN_DEL_ID,
    Reset_factory_ID,
    COMBOS_END,
};

uint8_t number_of_combos = COMBOS_END;

// 组合键触发键定义
const uint16_t PROGMEM FN_DOWN_combo[] = { S_FN_KEY, COMBO_END };
const uint16_t PROGMEM FN_UP_combo[]   = { S_FN_KEY, COMBO_END };

const uint16_t PROGMEM FN_F1_combo[]  = { KB_F1,  COMBO_END };
const uint16_t PROGMEM FN_F2_combo[]  = { KB_F2,  COMBO_END };
const uint16_t PROGMEM FN_F3_combo[]  = { KB_F3,  COMBO_END };
const uint16_t PROGMEM FN_F4_combo[]  = { KB_F4,  COMBO_END };
const uint16_t PROGMEM FN_F5_combo[]  = { KB_F5,  COMBO_END };
const uint16_t PROGMEM FN_F6_combo[]  = { KB_F6,  COMBO_END };
const uint16_t PROGMEM FN_F7_combo[]  = { KB_F7,  COMBO_END };
const uint16_t PROGMEM FN_F8_combo[]  = { KB_F8,  COMBO_END };
const uint16_t PROGMEM FN_F9_combo[]  = { KB_F9,  COMBO_END };
const uint16_t PROGMEM FN_F10_combo[] = { KB_F10, COMBO_END };
const uint16_t PROGMEM FN_F11_combo[] = { KB_F11, COMBO_END };
const uint16_t PROGMEM FN_F12_combo[] = { KB_F12, COMBO_END };

const uint16_t PROGMEM FN_DEL_combo[]       = { S_FN_KEY, KB_DELETE_FORWARD, COMBO_END };
const uint16_t PROGMEM Reset_factory_combo[] = { KB_ESCAPE, KB_EQUAL_N_PLUS, COMBO_END };

// 组合键数组
combo_t key_combos[] = {
    // Fn 键按下/释放
    [FN_DOWN_ID] = COMBO(FN_DOWN_combo, PRESS_DOWN, FN_DOWN_KEY),
    [FN_UP_ID]   = COMBO(FN_UP_combo,   PRESS_UP,   FN_UP_KEY),

    // F1~F12 媒体键（长按保持触发）
    [FN_F1_ID]  = COMBO_LONG_TICKS(FN_F1_combo,  5, LONG_PRESS_HOLD, Bringhness_Down),
    [FN_F2_ID]  = COMBO_LONG_TICKS(FN_F2_combo,  5, LONG_PRESS_HOLD, Bringhness_Up),
    [FN_F3_ID]  = COMBO_LONG_TICKS(FN_F3_combo,  5, LONG_PRESS_HOLD, Search),
    [FN_F4_ID]  = COMBO_LONG_TICKS(FN_F4_combo,  5, PRESS_DOWN,      Print_Screen),
    [FN_F5_ID]  = COMBO_LONG_TICKS(FN_F5_combo,  5, LONG_PRESS_HOLD, Earth),
    [FN_F6_ID]  = COMBO_LONG_TICKS(FN_F6_combo,  5, LONG_PRESS_HOLD, Media_Previous),
    [FN_F7_ID]  = COMBO_LONG_TICKS(FN_F7_combo,  5, LONG_PRESS_HOLD, Media_Play_Pause),
    [FN_F8_ID]  = COMBO_LONG_TICKS(FN_F8_combo,  5, LONG_PRESS_HOLD, Media_Next),
    [FN_F9_ID]  = COMBO_LONG_TICKS(FN_F9_combo,  5, LONG_PRESS_HOLD, Volume_Mute),
    [FN_F10_ID] = COMBO_LONG_TICKS(FN_F10_combo, 5, LONG_PRESS_HOLD, Volume_Down),
    [FN_F11_ID] = COMBO_LONG_TICKS(FN_F11_combo, 5, LONG_PRESS_HOLD, Volume_Up),
    [FN_F12_ID] = COMBO_LONG_TICKS(FN_F12_combo, 5, PRESS_DOWN,      Lock_Screen),

    // 特殊组合键
    [FN_DEL_ID]        = COMBO_LONG_TICKS(FN_DEL_combo,        3000, LONG_PRESS_START, Pair_button),
    [Reset_factory_ID] = COMBO_LONG_TICKS(Reset_factory_combo, 3000, LONG_PRESS_START, Factory_Reset),
};
```

**Step 9: 验证文件创建成功**

```bash
ls middleware/keyboard/combo/
```
Expected: 显示所有 combo 相关文件

**Step 10: Commit**

```bash
git add middleware/keyboard/combo/
git commit -m "feat(keyboard): add combo module from kb-04122 with OSAL events"
```

---

## Task 4: 创建 keymap 模块

**Files:**
- Create: `middleware/keyboard/keymap.h`
- Create: `middleware/keyboard/keymap.c`

**Step 1: 创建 keymap.h**

```c
// middleware/keyboard/keymap.h
#ifndef __KEYMAP_H__
#define __KEYMAP_H__

#include <stdint.h>
#include "product_config.h"

// 层类型
typedef enum {
    LAYER_BASE = 0,
    LAYER_FN,
    LAYER_MACOS,
    LAYER_WINDOWS,
    LAYER_CUSTOM_1,
    LAYER_CUSTOM_2,
    LAYER_MAX
} layer_id_t;

// 层状态
typedef struct {
    layer_id_t current_layer;       // 当前激活层（唯一）
    layer_id_t base_layer;          // 基础层（TG 切换的目标）
    layer_id_t mo_stack[4];         // MO 层栈
    uint8_t    mo_stack_top;        // 栈顶索引
} layer_state_t;

// 层状态全局变量
extern layer_state_t g_layer_state;

// 层级映射表（需要在产品配置中定义）
extern const uint16_t keymap_layers[LAYER_MAX][MATRIX_ROWS][MATRIX_COLS];

// 初始化
void keymap_init(void);

// 获取当前层的键码
uint16_t keymap_get_keycode(uint8_t row, uint8_t col);

// 层切换 API
void layer_set(layer_id_t layer);       // TG: 切换基础层
void layer_on(layer_id_t layer);        // MO: 临时激活层
void layer_off(layer_id_t layer);       // MO: 取消临时层
layer_id_t layer_get_current(void);     // 获取当前层

#endif
```

**Step 2: 创建 keymap.c**

```c
// middleware/keyboard/keymap.c
#include "keymap.h"
#include "keycode.h"

layer_state_t g_layer_state;

void keymap_init(void) {
    g_layer_state.current_layer = LAYER_BASE;
    g_layer_state.base_layer = LAYER_BASE;
    g_layer_state.mo_stack_top = 0;
}

uint16_t keymap_get_keycode(uint8_t row, uint8_t col) {
    if (row >= MATRIX_ROWS || col >= MATRIX_COLS) {
        return KC_NO;
    }

    layer_id_t layer = g_layer_state.current_layer;
    uint16_t keycode = keymap_layers[layer][row][col];

    // 透明键回退到基础层
    if (keycode == KC_TRANSPARENT && layer != g_layer_state.base_layer) {
        keycode = keymap_layers[g_layer_state.base_layer][row][col];
    }

    return keycode;
}

// TG: 切换基础层
void layer_set(layer_id_t layer) {
    if (layer >= LAYER_MAX) return;

    g_layer_state.base_layer = layer;
    if (g_layer_state.mo_stack_top == 0) {
        g_layer_state.current_layer = layer;
    }
}

// MO: 临时激活层（压栈）
void layer_on(layer_id_t layer) {
    if (layer >= LAYER_MAX) return;

    if (g_layer_state.mo_stack_top < 4) {
        g_layer_state.mo_stack[g_layer_state.mo_stack_top++] = layer;
        g_layer_state.current_layer = layer;
    }
}

// MO: 取消临时层（出栈）
void layer_off(layer_id_t layer) {
    // 从栈中移除指定层
    for (int i = g_layer_state.mo_stack_top - 1; i >= 0; i--) {
        if (g_layer_state.mo_stack[i] == layer) {
            // 移除该元素
            for (int j = i; j < g_layer_state.mo_stack_top - 1; j++) {
                g_layer_state.mo_stack[j] = g_layer_state.mo_stack[j + 1];
            }
            g_layer_state.mo_stack_top--;
            break;
        }
    }

    // 更新当前层
    if (g_layer_state.mo_stack_top > 0) {
        g_layer_state.current_layer = g_layer_state.mo_stack[g_layer_state.mo_stack_top - 1];
    } else {
        g_layer_state.current_layer = g_layer_state.base_layer;
    }
}

layer_id_t layer_get_current(void) {
    return g_layer_state.current_layer;
}
```

**Step 3: 验证文件创建成功**

```bash
ls middleware/keyboard/keymap.*
```
Expected: `keymap.c  keymap.h`

**Step 4: Commit**

```bash
git add middleware/keyboard/keymap.c middleware/keyboard/keymap.h
git commit -m "feat(keyboard): add keymap module with MO/TG layer switching"
```

---

## Task 5: 重写 keyboard.c 主协调模块

**Files:**
- Modify: `middleware/keyboard/keyboard.c`
- Modify: `middleware/keyboard/keyboard.h`

使用驱动层的 `matrix.h` 和 `debounce.h` 进行矩阵扫描和防抖处理。

**Step 1: 重写 keyboard.h**

```c
// middleware/keyboard/keyboard.h
#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <stdint.h>
#include <stdbool.h>
#include "linkedlist.h"
#include "combo/process_combo.h"

// 按键列表（供 combo 模块使用）
extern list_t* _key_code_list;
extern list_t* _key_code_list_extend;

// 主函数
void keyboard_init(void);
void keyboard_task(void);

// 获取上一次的更新状态
key_update_st_t keyboard_get_last_update_state(void);

#endif
```

**Step 2: 重写 keyboard.c（使用驱动层防抖）**

```c
// middleware/keyboard/keyboard.c
#include "keyboard.h"
#include "keymap.h"
#include "matrix.h"
#include "debounce.h"
#include "report.h"
#include "report_buffer.h"
#include "keycode.h"
#include <string.h>

// 按键列表（供 process_combo.c 使用）
list_t* _key_code_list = NULL;
list_t* _key_code_list_extend = NULL;

// 上一次矩阵状态（防抖后）
static matrix_row_t matrix_previous[MATRIX_ROWS];

// 当前防抖后的矩阵状态
static matrix_row_t matrix_debounced[MATRIX_ROWS];

// 上一次更新状态
static key_update_st_t last_update_state = NO_KEY_UPDATE;

// 内部函数声明
static key_update_st_t scan_and_debounce(void);
static void update_key_code_list(void);

void keyboard_init(void) {
    // 初始化矩阵
    matrix_init();

    // 初始化防抖
    debounce_init(MATRIX_ROWS);

    // 初始化层级
    keymap_init();

    // 初始化链表
    _key_code_list = make_list_proc();
    _key_code_list_extend = make_list_proc();

    // 初始化组合键
    combo_init();

    // 初始化报告
    report_init();

    // 清空矩阵状态
    memset(matrix_previous, 0, sizeof(matrix_previous));
    memset(matrix_debounced, 0, sizeof(matrix_debounced));
}

void keyboard_task(void) {
    // 1. 矩阵扫描 + 驱动层防抖
    key_update_st_t key_st = scan_and_debounce();
    last_update_state = key_st;

    // 2. 幽灵键直接返回
    if (key_st == GHOST_KEY) {
        return;
    }

    // 3. 更新按键列表（基于防抖后的矩阵变化）
    if (key_st == KEY_UPDATE) {
        update_key_code_list();
    }

    // 4. 组合键处理
    combo_task(key_st);

    // 5. 生成并发送 HID 报告
    report_update_proc(key_st);

    // 6. 清空扩展键列表
    del_all_child(_key_code_list_extend);
}

key_update_st_t keyboard_get_last_update_state(void) {
    return last_update_state;
}

// 矩阵扫描 + 驱动层防抖处理
static key_update_st_t scan_and_debounce(void) {
    // 调用驱动层矩阵扫描
    bool raw_changed = matrix_scan();

    // 获取原始矩阵数据
    static matrix_row_t raw_matrix[MATRIX_ROWS];
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        raw_matrix[row] = matrix_get_row(row);
    }

    // 调用驱动层防抖处理
    bool debounced_changed = debounce(raw_matrix, matrix_debounced, MATRIX_ROWS, raw_changed);

    if (!debounced_changed) {
        return NO_KEY_UPDATE;
    }

    // TODO: 可选的幽灵键检测
    // if (detect_ghost_key(matrix_debounced)) {
    //     return GHOST_KEY;
    // }

    return KEY_UPDATE;
}

// 更新按键列表（基于防抖后的矩阵变化）
static void update_key_code_list(void) {
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        matrix_row_t current = matrix_debounced[row];
        matrix_row_t changes = current ^ matrix_previous[row];

        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            if (!(changes & ((matrix_row_t)1 << col))) continue;

            uint16_t keycode = keymap_get_keycode(row, col);
            if (keycode == KC_NO) continue;

            bool pressed = current & ((matrix_row_t)1 << col);

            if (pressed) {
                // 按键按下：添加到列表
                if (!find_key(_key_code_list, keycode)) {
                    add(keycode, _key_code_list);
                }
            } else {
                // 按键释放：从列表移除
                del(keycode, _key_code_list);
            }
        }
        matrix_previous[row] = current;
    }
}
```

**Step 3: 验证文件修改成功**

```bash
ls middleware/keyboard/keyboard.*
```
Expected: `keyboard.c  keyboard.h`

**Step 4: Commit**

```bash
git add middleware/keyboard/keyboard.c middleware/keyboard/keyboard.h
git commit -m "refactor(keyboard): rewrite keyboard.c using driver layer debounce"
```

---

## Task 6: 重写 report.c 报告生成模块

**Files:**
- Modify: `middleware/keyboard/report.c`
- Modify: `middleware/keyboard/report.h`

**Step 1: 更新 report.h（添加新函数声明）**

在现有 report.h 中添加：

```c
// 在 report.h 中添加
#include "combo/process_combo.h"

// 初始化报告模块
void report_init(void);

// 更新并发送报告
void report_update_proc(key_update_st_t key_st);
```

**Step 2: 更新 report.c（添加新实现）**

在现有 report.c 末尾添加：

```c
// middleware/keyboard/report.c 新增部分

#include "linkedlist.h"
#include "report_buffer.h"
#include "keymap.h"

// 外部链表
extern list_t* _key_code_list;
extern list_t* _key_code_list_extend;

// 上一次报告缓存
static report_keyboard_t last_kb_report;
static uint16_t last_consumer_report;

void report_init(void) {
    memset(&last_kb_report, 0, sizeof(last_kb_report));
    last_consumer_report = 0;
}

// 键码分类并添加到报告
static void classify_and_add_keycode(uint16_t keycode,
                                      report_keyboard_t* kb_report,
                                      uint16_t* consumer_report,
                                      uint8_t* key_idx) {
    // 跳过层切换键
    if (IS_QK_MOMENTARY(keycode) || IS_QK_TOGGLE_LAYER(keycode)) {
        return;
    }

    // 修饰键 (0xE0-0xE7)
    if (keycode >= KC_LCTRL && keycode <= KC_RGUI) {
        kb_report->mods |= (1 << (keycode - KC_LCTRL));
        return;
    }

    // 普通键 (0x04-0xDF)
    if (keycode >= KC_A && keycode < KC_LCTRL) {
        if (*key_idx < KEYBOARD_REPORT_KEYS) {
            kb_report->keys[(*key_idx)++] = (uint8_t)keycode;
        }
        return;
    }

    // 媒体键（M_KEY_TYPE 标记）
    if ((keycode & M_KEY_TYPE) == M_KEY_TYPE) {
        *consumer_report = keycode ^ M_KEY_TYPE;
        return;
    }
}

void report_update_proc(key_update_st_t key_st) {
    if (key_st == GHOST_KEY) {
        return;
    }

    report_keyboard_t kb_report = {0};
    uint16_t consumer_report = 0;
    uint8_t key_idx = 0;

    // 遍历 _key_code_list
    if (_key_code_list != NULL) {
        node_t* current = _key_code_list->head;
        while (current != NULL) {
            if (current->data.is_report != 0) {
                uint16_t keycode = current->data.key_code;
                classify_and_add_keycode(keycode, &kb_report, &consumer_report, &key_idx);
            }
            current = current->next;
        }
    }

    // 遍历 _key_code_list_extend（组合键扩展）
    if (_key_code_list_extend != NULL) {
        node_t* current = _key_code_list_extend->head;
        while (current != NULL) {
            if (current->data.is_report != 0) {
                // 跳过已在主列表中的键
                if (!find_activate_key(_key_code_list, current->data.key_code)) {
                    uint16_t keycode = current->data.key_code;
                    classify_and_add_keycode(keycode, &kb_report, &consumer_report, &key_idx);
                }
            }
            current = current->next;
        }
    }

    // 检查并发送键盘报告
    if (memcmp(&kb_report, &last_kb_report, sizeof(kb_report)) != 0) {
        memcpy(&last_kb_report, &kb_report, sizeof(kb_report));

        report_buffer_t report;
        report.type = REPORT_TYPE_KB;
        memcpy(&report.keyboard, &kb_report, sizeof(kb_report));
        report_buffer_enqueue(&report);
    }

    // 检查并发送消费者报告
    if (consumer_report != last_consumer_report) {
        last_consumer_report = consumer_report;

        report_buffer_t report;
        report.type = REPORT_TYPE_CONSUMER;
        report.consumer = consumer_report;
        report_buffer_enqueue(&report);
    }
}
```

**Step 3: Commit**

```bash
git add middleware/keyboard/report.c middleware/keyboard/report.h
git commit -m "feat(keyboard): add report generation with combo support"
```

---

## Task 7: 更新 keycode.h 添加缺失定义

**Files:**
- Modify: `middleware/keyboard/keycode.h`

**Step 1: 检查并添加缺失的键码定义**

在 keycode.h 中确保存在以下定义：

```c
// 特殊功能键（Fn 键）
#ifndef S_FN_KEY
#define S_FN_KEY 0xF0
#endif

// 媒体键类型标记
#ifndef M_KEY_TYPE
#define M_KEY_TYPE 0x1000
#endif

// 媒体键定义
#ifndef M_BACKLIGHT_DOWN
#define M_BACKLIGHT_DOWN    (M_KEY_TYPE | 0x0070)
#endif

#ifndef M_BACKLIGHT_UP
#define M_BACKLIGHT_UP      (M_KEY_TYPE | 0x006F)
#endif

#ifndef M_VOLUME_DOWN
#define M_VOLUME_DOWN       (M_KEY_TYPE | 0x00EA)
#endif

#ifndef M_VOLUME_UP
#define M_VOLUME_UP         (M_KEY_TYPE | 0x00E9)
#endif

#ifndef M_MUTE
#define M_MUTE              (M_KEY_TYPE | 0x00E2)
#endif

#ifndef M_PLAY_N_PAUSE
#define M_PLAY_N_PAUSE      (M_KEY_TYPE | 0x00CD)
#endif

#ifndef M_SCAN_NEXT_TRACK
#define M_SCAN_NEXT_TRACK   (M_KEY_TYPE | 0x00B5)
#endif

#ifndef M_SCAN_PREVIOUS_TRACK
#define M_SCAN_PREVIOUS_TRACK (M_KEY_TYPE | 0x00B6)
#endif

#ifndef M_WWW_SEARCH
#define M_WWW_SEARCH        (M_KEY_TYPE | 0x0221)
#endif

#ifndef M_EARTH
#define M_EARTH             (M_KEY_TYPE | 0x029D)
#endif

// 透明键
#ifndef KC_TRANSPARENT
#define KC_TRANSPARENT 0x01
#endif

// 层切换键检测宏
#ifndef IS_QK_MOMENTARY
#define IS_QK_MOMENTARY(kc) (((kc) & 0xFF00) == 0x5100)
#endif

#ifndef IS_QK_TOGGLE_LAYER
#define IS_QK_TOGGLE_LAYER(kc) (((kc) & 0xFF00) == 0x5300)
#endif
```

**Step 2: Commit**

```bash
git add middleware/keyboard/keycode.h
git commit -m "feat(keyboard): add media keycode definitions"
```

---

## Task 8: 创建示例 keymap 配置

**Files:**
- Create: `keyboards/default_keymap.c`

**Step 1: 创建默认键盘映射**

```c
// keyboards/default_keymap.c
#include "keymap.h"
#include "keycode.h"

// 默认层级映射（6行 x 13列示例）
// 实际项目中根据产品配置修改
const uint16_t keymap_layers[LAYER_MAX][MATRIX_ROWS][MATRIX_COLS] = {
    // LAYER_BASE - 基础层
    [LAYER_BASE] = {
        { KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL  },
        { KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC },
        { KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_ENT  },
        { KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT, KC_NO   },
        { KC_LCTL, KC_LGUI, KC_LALT, KC_NO,   KC_NO,   KC_SPC,  KC_NO,   KC_NO,   KC_RALT, S_FN_KEY,KC_APP,  KC_RCTL, KC_NO   },
        { KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO   },
    },

    // LAYER_FN - Fn层
    [LAYER_FN] = {
        { KC_GRV,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12  },
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS },
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS },
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_NO   },
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_NO,   KC_NO,   KC_TRNS, KC_NO,   KC_NO,   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_NO   },
        { KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO   },
    },

    // LAYER_MACOS - MacOS层（示例）
    [LAYER_MACOS] = {
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS },
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS },
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS },
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_NO   },
        { KC_TRNS, KC_LALT, KC_LGUI, KC_NO,   KC_NO,   KC_TRNS, KC_NO,   KC_NO,   KC_RGUI, KC_TRNS, KC_RALT, KC_TRNS, KC_NO   },
        { KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO   },
    },

    // LAYER_WINDOWS - Windows层（与基础层相同）
    [LAYER_WINDOWS] = {
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS },
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS },
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS },
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_NO   },
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_NO,   KC_NO,   KC_TRNS, KC_NO,   KC_NO,   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_NO   },
        { KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO   },
    },

    // LAYER_CUSTOM_1 - 自定义层1（全透明）
    [LAYER_CUSTOM_1] = {
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS },
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS },
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS },
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_NO   },
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_NO,   KC_NO,   KC_TRNS, KC_NO,   KC_NO,   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_NO   },
        { KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO   },
    },

    // LAYER_CUSTOM_2 - 自定义层2（全透明）
    [LAYER_CUSTOM_2] = {
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS },
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS },
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS },
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_NO   },
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_NO,   KC_NO,   KC_TRNS, KC_NO,   KC_NO,   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_NO   },
        { KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO   },
    },
};
```

**Step 2: Commit**

```bash
git add keyboards/default_keymap.c
git commit -m "feat(keyboards): add default keymap configuration"
```

---

## Task 9: 更新 CMakeLists.txt

**Files:**
- Modify: `CMakeLists.txt`

**Step 1: 添加新源文件到构建系统**

在 CMakeLists.txt 中添加：

```cmake
# utils 目录
set(UTILS_SOURCES
    utils/linkedlist.c
    # ... 其他 utils 文件
)

# keyboard middleware
set(KEYBOARD_SOURCES
    middleware/keyboard/keyboard.c
    middleware/keyboard/keymap.c
    middleware/keyboard/report.c
    middleware/keyboard/combo/process_combo.c
    middleware/keyboard/combo/FN_Combo.c
    middleware/keyboard/combo/Special_Combo.c
    middleware/keyboard/combo/media_combo.c
    # ... 保留的其他文件
    middleware/keyboard/keycode_config.c
)

# keyboards 配置
set(KEYBOARDS_SOURCES
    keyboards/default_keymap.c
)
```

**Step 2: 添加 include 路径**

```cmake
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/utils
    ${CMAKE_SOURCE_DIR}/middleware/keyboard
    ${CMAKE_SOURCE_DIR}/middleware/keyboard/combo
    ${CMAKE_SOURCE_DIR}/drivers/input/keyboard
    # ... 其他目录
)
```

**Step 3: Commit**

```bash
git add CMakeLists.txt
git commit -m "build: update CMakeLists.txt for simplified keyboard module"
```

---

## Task 10: 编译验证

**Files:**
- None (build verification)

**Step 1: 清理并重新构建**

```bash
rm -rf build
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
```

**Step 2: 编译**

```bash
mingw32-make
```
Expected: 编译成功，无错误

**Step 3: 修复编译错误（如有）**

根据编译错误提示修复问题。

**Step 4: Commit（如有修复）**

```bash
git add -A
git commit -m "fix: resolve compilation errors in keyboard module"
```

---

## Task 11: 最终提交

**Step 1: 查看所有更改**

```bash
git status
git log --oneline -10
```

**Step 2: 确认分支状态**

```bash
git branch --show-current
```
Expected: `feature/keyboard-simplified`

**Step 3: 推送分支（用户确认后）**

```bash
git push -u origin feature/keyboard-simplified
```

---

## 文件清单汇总

### 新建文件

| 文件路径 | 说明 |
|----------|------|
| `utils/linkedlist.c` | 链表实现 |
| `utils/linkedlist.h` | 链表头文件 |
| `middleware/keyboard/combo/process_combo.h` | 组合键处理头文件 |
| `middleware/keyboard/combo/process_combo.c` | 组合键处理实现 |
| `middleware/keyboard/combo/FN_Combo.h` | Fn 组合键头文件 |
| `middleware/keyboard/combo/FN_Combo.c` | Fn 组合键实现 |
| `middleware/keyboard/combo/Special_Combo.h` | 特殊组合键头文件 |
| `middleware/keyboard/combo/Special_Combo.c` | 特殊组合键实现（OSAL 事件） |
| `middleware/keyboard/combo/media_combo.c` | 组合键定义表 |
| `middleware/keyboard/keymap.h` | 层级映射头文件 |
| `middleware/keyboard/keymap.c` | 层级映射实现 |
| `keyboards/default_keymap.c` | 默认键盘映射 |

### 修改文件

| 文件路径 | 说明 |
|----------|------|
| `middleware/keyboard/keyboard.h` | 重写主协调头文件 |
| `middleware/keyboard/keyboard.c` | 重写主协调实现（使用驱动层防抖） |
| `middleware/keyboard/report.h` | 添加新函数声明 |
| `middleware/keyboard/report.c` | 添加报告生成实现 |
| `middleware/keyboard/keycode.h` | 添加媒体键定义 |
| `CMakeLists.txt` | 更新构建配置 |

### 驱动层依赖

| 文件路径 | 说明 |
|----------|------|
| `drivers/input/keyboard/matrix.h` | 矩阵扫描接口 |
| `drivers/input/keyboard/matrix.c` | 矩阵扫描实现 |
| `drivers/input/keyboard/debounce.h` | 防抖接口 |
| `drivers/input/keyboard/debounce.c` | 防抖实现 |

### 架构流程

```
keyboard_task()
    │
    ├─► matrix_scan()           [drivers/input/keyboard/matrix.c]
    │       │
    │       ▼
    ├─► debounce()              [drivers/input/keyboard/debounce.c]
    │       │
    │       ▼
    ├─► update_key_code_list()  [middleware/keyboard/keyboard.c]
    │   + keymap_get_keycode()  [middleware/keyboard/keymap.c]
    │       │
    │       ▼ _key_code_list
    │
    ├─► combo_task()            [middleware/keyboard/combo/process_combo.c]
    │       │
    │       ▼ _key_code_list_extend
    │
    └─► report_update_proc()    [middleware/keyboard/report.c]
            │
            ▼
        report_buffer_enqueue() [middleware/communication/report_buffer.c]
```
