# 精简键盘按键处理流程设计

## 设计目标

基于现有框架，精简 `middleware/keyboard` 模块，实现最小可用的按键处理流程：

- 保持分层架构，复用驱动层 matrix/debounce
- 只保留层切换功能（MO + TG），不实现复杂动作
- **组合键直接复用 kb-04122 的实现**（最小更改原则）
- 保留所有键码定义，方便日后扩展
- 发送报文通过 `report_buffer_enqueue()`

## 目标流程

```
矩阵扫描 → 防抖 → 层级映射 → 获取键码 → 组合键处理 → 键码转换 → HID报告生成
   │         │         │          │           │            │           │
   ▼         ▼         ▼          ▼           ▼            ▼           ▼
matrix   debounce   keymap     keycode     combo      classify    report_buffer
_scan()             _lookup()   _list       _task()    _keycode()  _enqueue()
```

## 架构设计

### 模块划分

```
middleware/keyboard/
├── keyboard.c/h           # [精简重写] 主协调模块
├── keymap.c/h             # [新增] 层级映射 + 层切换
├── combo/                 # [直接复用] kb-04122 组合键实现
│   ├── process_combo.c/h  # 核心组合键处理逻辑
│   ├── FN_Combo.c/h       # Fn 组合键回调
│   ├── Special_Combo.c/h  # 特殊组合键回调
│   └── media_combo.c      # 组合键定义 (key_combos[])
├── linkedlist.c/h         # [直接复用] 链表数据结构
├── report.c/h             # [精简] HID 报告生成
├── keycode.h              # [保留] 键码定义
├── keycodes.h             # [保留] 扩展键码
├── quantum_keycodes.h     # [保留] QMK 量子键码
└── modifiers.h            # [保留] 修饰键定义

# 删除的文件（日后扩展时再恢复）
# action.c/h, action_layer.c/h, action_util.c/h
# action_code.h, custom_function.c/h
```

### 复用文件清单

#### 直接复用（从 kb-04122 移植）

| 源文件 | 目标位置 | 说明 |
|--------|----------|------|
| `kb-04122/PAR2860/Keyboard/process_combo.c` | `middleware/keyboard/combo/` | 组合键核心逻辑 |
| `kb-04122/PAR2860/Keyboard/include/process_combo.h` | `middleware/keyboard/combo/` | 组合键头文件 |
| `kb-04122/PAR2860/Keyboard/FN_Combo.c` | `middleware/keyboard/combo/` | Fn 组合键回调 |
| `kb-04122/PAR2860/Keyboard/include/FN_Combo.h` | `middleware/keyboard/combo/` | Fn 组合键头文件 |
| `kb-04122/PAR2860/Keyboard/Special_Combo.c` | `middleware/keyboard/combo/` | 特殊组合键回调 |
| `kb-04122/PAR2860/Keyboard/include/Special_Combo.h` | `middleware/keyboard/combo/` | 特殊组合键头文件 |
| `kb-04122/PAR2860/Keyboard/media_combo.c` | `middleware/keyboard/combo/` | 组合键定义表 |
| `kb-04122/PAR2860/linkedlist.c` | `utils/` | 链表实现 |
| `kb-04122/PAR2860/Include/linkedlist.h` | `utils/` | 链表头文件 |

#### 需要适配的接口

复用的组合键代码依赖以下外部接口，需要在适配层提供：

```c
// 1. 按键列表（链表）- 在 keyboard.c 中定义
extern list_t* _key_code_list;          // 当前按下的键码
extern list_t* _key_code_list_extend;   // 组合键扩展键码

// 2. 按键更新状态枚举 - 在 keyboard.h 中定义
typedef enum {
    NO_KEY_UPDATE = 0,
    KEY_UPDATE,
    GHOST_KEY
} key_update_st_t;

// 3. 组合键数组 - 在 media_combo.c 中定义
extern combo_t key_combos[];
extern uint8_t number_of_combos;

// 4. 平台相关函数 - 需要提供空实现或适配
// FN_Combo.c / Special_Combo.c 中的回调可能调用：
void ble_tx_data(uint8_t idx, uint8_t len, uint8_t* data);  // BLE 发送
void _set_evt(uint8_t type, uint8_t evt);                    // 事件设置
void LED_SetStateN(...);                                      // LED 控制
// 这些需要适配到当前框架的对应接口
```

### 数据结构

#### 1. 链表结构（直接复用 linkedlist.h）

```c
// linkedlist.h - 直接复用，无需修改

typedef struct {
    uint16_t key_code;      // 键值
    uint8_t cycle;          // 按下时长（防抖用）
    uint8_t is_report;      // 是否上报
    report_t report_type;   // 报告类型
} bouncing_data_t;

typedef struct node {
    bouncing_data_t data;
    struct node* next;
} node_t;

typedef struct list {
    node_t* head;
} list_t;

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
void destroy(list_t* list);
```

#### 2. 组合键结构（直接复用 process_combo.h）

```c
// process_combo.h - 直接复用，无需修改

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

// 回调函数类型
typedef uint8_t (*BtnCallback)(uint16_t* add_keys);

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
#define COMBO(ck, event_idx, callback) ...
#define COMBO_LONG_TICKS(ck, long_tick, event_idx, callback) ...
#define COMBO2_LONG_TICKS(ck, long_tick, event_idx1, cb1, event_idx2, cb2) ...
```

#### 3. 层级管理（新增）

```c
// keymap.h

// 层类型
typedef enum {
    LAYER_BASE = 0,                 // 基础层
    LAYER_FN,                       // Fn 层
    LAYER_MACOS,                    // MacOS 层
    LAYER_WINDOWS,                  // Windows 层
    LAYER_CUSTOM_1,                 // 自定义层
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

extern layer_state_t g_layer_state;
extern const uint16_t keymap_layers[LAYER_MAX][MATRIX_ROWS][MATRIX_COLS];
```

### 核心函数设计

#### 1. keyboard.c - 主协调模块

```c
// keyboard.h
#include "linkedlist.h"

// 按键更新状态（兼容 process_combo.c）
typedef enum {
    NO_KEY_UPDATE = 0,
    KEY_UPDATE,
    GHOST_KEY
} key_update_st_t;

void keyboard_init(void);
void keyboard_task(void);

// keyboard.c
#include "keyboard.h"
#include "matrix.h"
#include "debounce.h"
#include "keymap.h"
#include "process_combo.h"
#include "report.h"
#include "report_buffer.h"

// 按键列表（供 process_combo.c 使用）
list_t* _key_code_list;
list_t* _key_code_list_extend;

// 防抖相关
static list_t* _make_list;
static list_t* _break_list;

// 上一次矩阵状态
static matrix_row_t matrix_previous[MATRIX_ROWS];

void keyboard_init(void) {
    // 初始化矩阵
    matrix_init();

    // 初始化层级
    keymap_init();

    // 初始化链表
    _make_list = make_list_proc();
    _break_list = make_list_proc();
    _key_code_list = make_list_proc();
    _key_code_list_extend = make_list_proc();

    // 初始化组合键
    combo_init();

    // 清空矩阵状态
    memset(matrix_previous, 0, sizeof(matrix_previous));
}

void keyboard_task(void) {
    // 1. 矩阵扫描（调用驱动层）
    key_update_st_t key_st = key_matrix_proc();

    // 2. 防抖处理
    if (key_st != GHOST_KEY) {
        debounce_proc(key_st);

        // 3. 组合键处理（直接调用复用的函数）
        combo_task(key_st);

        // 4. 生成并发送 HID 报告
        report_update_proc(key_st);
    }

    // 5. 清空扩展键列表
    del_all_child(_key_code_list_extend);
}

// 矩阵扫描处理（参考 kb-04122 的 key_matrix_proc）
static key_update_st_t key_matrix_proc(void) {
    bool changed = matrix_scan();  // 调用驱动层

    if (!changed) {
        return NO_KEY_UPDATE;
    }

    // 检测幽灵键（可选）
    // ...

    return KEY_UPDATE;
}

// 防抖处理（参考 kb-04122 的 debounce_proc）
static void debounce_proc(key_update_st_t key_st) {
    // 增加 make/break 链表计时
    increase_cycle(_make_list);
    increase_cycle(_break_list);

    if (key_st == KEY_UPDATE) {
        // 遍历矩阵，检测按键变化
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            matrix_row_t current = matrix_get_row(row);
            matrix_row_t changes = current ^ matrix_previous[row];

            for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                if (!(changes & ((matrix_row_t)1 << col))) continue;

                uint16_t keycode = keymap_get_keycode(row, col);
                if (keycode == KC_NO) continue;

                bool pressed = current & ((matrix_row_t)1 << col);

                if (pressed) {
                    // 新按下
                    if (find_key(_break_list, keycode)) {
                        del(keycode, _break_list);
                    } else {
                        add(keycode, _make_list);
                    }
                } else {
                    // 释放
                    if (find_key(_make_list, keycode)) {
                        del(keycode, _make_list);
                    } else {
                        add(keycode, _break_list);
                    }
                }
            }
            matrix_previous[row] = current;
        }
    }

    // 弹出已防抖完成的按键
    static uint16_t make_keys[20], break_keys[20];
    static uint8_t make_sz, break_sz;

    pop_key(_make_list, make_keys, &make_sz, MAKE_DEBOUNCE_CYCLE);
    pop_key(_break_list, break_keys, &break_sz, BREAK_DEBOUNCE_CYCLE);

    // 更新 _key_code_list
    for (uint8_t i = 0; i < break_sz; i++) {
        del(break_keys[i], _key_code_list);
    }
    for (uint8_t i = 0; i < make_sz; i++) {
        add(make_keys[i], _key_code_list);
    }
}
```

#### 2. keymap.c - 层级映射

```c
// keymap.c
#include "keymap.h"
#include "keyboard.h"

layer_state_t g_layer_state;

void keymap_init(void) {
    g_layer_state.current_layer = LAYER_BASE;
    g_layer_state.base_layer = LAYER_BASE;
    g_layer_state.mo_stack_top = 0;
}

uint16_t keymap_get_keycode(uint8_t row, uint8_t col) {
    layer_id_t layer = g_layer_state.current_layer;
    uint16_t keycode = keymap_layers[layer][row][col];

    // 透明键回退到基础层
    if (keycode == KC_TRANSPARENT && layer != g_layer_state.base_layer) {
        keycode = keymap_layers[g_layer_state.base_layer][row][col];
    }

    return keycode;
}

// 层切换 API
void layer_set(layer_id_t layer) {
    g_layer_state.base_layer = layer;
    if (g_layer_state.mo_stack_top == 0) {
        g_layer_state.current_layer = layer;
    }
}

void layer_on(layer_id_t layer) {
    if (g_layer_state.mo_stack_top < 4) {
        g_layer_state.mo_stack[g_layer_state.mo_stack_top++] = layer;
        g_layer_state.current_layer = layer;
    }
}

void layer_off(layer_id_t layer) {
    // 从栈中移除
    for (int i = g_layer_state.mo_stack_top - 1; i >= 0; i--) {
        if (g_layer_state.mo_stack[i] == layer) {
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

#### 3. report.c - HID 报告生成

```c
// report.c
#include "report.h"
#include "keyboard.h"
#include "report_buffer.h"

#define KEYBOARD_REPORT_KEYS 6

static report_keyboard_t last_kb_report;
static uint16_t last_consumer_report;

void report_init(void) {
    memset(&last_kb_report, 0, sizeof(last_kb_report));
    last_consumer_report = 0;
}

// 报告生成（参考 kb-04122 的 report_update_proc）
void report_update_proc(key_update_st_t key_st) {
    if (key_st == GHOST_KEY) {
        // 幽灵键处理
        return;
    }

    report_keyboard_t kb_report = {0};
    uint16_t consumer_report = 0;
    uint8_t key_idx = 0;

    // 遍历 _key_code_list
    node_t* current = _key_code_list->head;
    while (current != NULL) {
        if (current->data.is_report == 0) {
            current = current->next;
            continue;
        }

        uint16_t keycode = current->data.key_code;
        classify_and_add_keycode(keycode, &kb_report, &consumer_report, &key_idx);
        current = current->next;
    }

    // 遍历 _key_code_list_extend（组合键扩展）
    current = _key_code_list_extend->head;
    while (current != NULL) {
        if (current->data.is_report == 0) {
            current = current->next;
            continue;
        }

        // 跳过已在主列表中的键
        if (find_activate_key(_key_code_list, current->data.key_code)) {
            current = current->next;
            continue;
        }

        uint16_t keycode = current->data.key_code;
        classify_and_add_keycode(keycode, &kb_report, &consumer_report, &key_idx);
        current = current->next;
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

// 键码分类
static void classify_and_add_keycode(uint16_t keycode,
                                      report_keyboard_t* kb_report,
                                      uint16_t* consumer_report,
                                      uint8_t* key_idx) {
    // 跳过层切换键
    if (IS_QK_MOMENTARY(keycode) || IS_QK_TOGGLE_LAYER(keycode)) {
        return;
    }

    // 修饰键
    if (keycode >= KC_LCTRL && keycode <= KC_RGUI) {
        kb_report->mods |= (1 << (keycode - KC_LCTRL));
        return;
    }

    // 普通键
    if (keycode >= KC_A && keycode < KC_LCTRL) {
        if (*key_idx < KEYBOARD_REPORT_KEYS) {
            kb_report->keys[(*key_idx)++] = keycode;
        }
        return;
    }

    // 媒体键（M_KEY_TYPE）
    if ((keycode & M_KEY_TYPE) == M_KEY_TYPE) {
        *consumer_report = keycode ^ M_KEY_TYPE;
        return;
    }
}
```

### 适配层设计

#### 平台相关函数适配

复用的组合键回调中使用了平台相关函数，需要提供适配：

```c
// platform_adapter.c

#include "platform_adapter.h"

// BLE 发送适配
void ble_tx_data(uint8_t idx, uint8_t len, uint8_t* data) {
    // 适配到当前框架的 BLE 发送接口
    // 例如：report_buffer_enqueue() 或直接调用 BLE 驱动
}

// 事件设置适配
void _set_evt(uint8_t type, uint8_t evt) {
    // 适配到当前框架的事件系统
    // 例如：调用 OSAL 事件或状态机
}

// LED 控制适配
void LED_SetStateN(uint8_t led, uint8_t state, uint8_t times) {
    // 适配到当前框架的 LED 驱动
}

// 背光控制适配
void Backlight_SwitchColor(void) {
    // 适配到当前框架的背光驱动
}

void Backlight_SwitchBrightness(void) {
    // 适配到当前框架的背光驱动
}
```

### 文件修改清单

#### 需要复制的文件

| 操作 | 源文件 | 目标位置 |
|------|--------|----------|
| 复制 | `kb-04122/.../process_combo.c` | `middleware/keyboard/combo/` |
| 复制 | `kb-04122/.../process_combo.h` | `middleware/keyboard/combo/` |
| 复制 | `kb-04122/.../FN_Combo.c` | `middleware/keyboard/combo/` |
| 复制 | `kb-04122/.../Special_Combo.c` | `middleware/keyboard/combo/` |
| 复制 | `kb-04122/.../media_combo.c` | `middleware/keyboard/combo/` |
| 复制 | `kb-04122/linkedlist.c` | `utils/` |
| 复制 | `kb-04122/linkedlist.h` | `utils/` |

#### 需要修改的复用文件

| 文件 | 修改内容 |
|------|----------|
| `process_combo.h` | 移除 `#include "keyboard.h"` 中的平台依赖，改为包含适配头文件 |
| `FN_Combo.c` | 平台相关函数调用改为适配层接口 |
| `Special_Combo.c` | 平台相关函数调用改为适配层接口 |
| `linkedlist.h` | 移除 `#include "armcm0.h"`，改为标准头文件 |

#### 需要新建的文件

| 文件 | 说明 |
|------|------|
| `middleware/keyboard/keyboard.c` | 主协调模块（精简实现） |
| `middleware/keyboard/keyboard.h` | 主协调模块头文件 |
| `middleware/keyboard/keymap.c` | 层级映射模块 |
| `middleware/keyboard/keymap.h` | 层级映射头文件 |
| `middleware/keyboard/report.c` | 报告生成模块（精简实现） |
| `middleware/keyboard/platform_adapter.c` | 平台适配层 |
| `middleware/keyboard/platform_adapter.h` | 平台适配头文件 |

#### 需要删除/备份的文件

| 文件 | 说明 |
|------|------|
| `action.c/h` | 备份到 `_backup/` |
| `action_layer.c/h` | 备份到 `_backup/` |
| `action_util.c/h` | 备份到 `_backup/` |
| `action_code.h` | 备份到 `_backup/` |
| `custom_function.c/h` | 备份到 `_backup/` |
| `keymap_common.c/h` | 备份到 `_backup/` |
| `keymap_introspection.c/h` | 备份到 `_backup/` |

### 调用流程图

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           keyboard_task() 主循环                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌──────────────┐                                                          │
│  │ matrix_scan()│  drivers/input/keyboard/matrix.c （已有）                 │
│  └──────┬───────┘                                                          │
│         │                                                                  │
│         ▼                                                                  │
│  ┌──────────────────┐                                                      │
│  │ debounce_proc()  │  参考 kb-04122 逻辑                                   │
│  │ + keymap_get_    │  通过层级获取键码                                     │
│  │   keycode()      │  更新 _key_code_list（链表）                          │
│  └──────┬───────────┘                                                      │
│         │ _key_code_list (链表)                                            │
│         ▼                                                                  │
│  ┌──────────────────┐                                                      │
│  │ combo_task()     │  【直接复用 process_combo.c】                         │
│  │                  │  检查组合键，触发回调                                  │
│  │                  │  修改 _key_code_list（删除组合键成员）                 │
│  │                  │  填充 _key_code_list_extend（回调返回的键）            │
│  └──────┬───────────┘                                                      │
│         │                                                                  │
│         ▼                                                                  │
│  ┌──────────────────┐                                                      │
│  │report_update_    │  遍历两个链表，分类键码                                │
│  │proc()            │  生成 HID 报告                                        │
│  └──────┬───────────┘                                                      │
│         │ report_buffer_t                                                  │
│         ▼                                                                  │
│  ┌──────────────────┐                                                      │
│  │report_buffer_    │  middleware/communication/report_buffer.c （已有）    │
│  │enqueue()         │  发送到 USB/BLE/2.4G                                  │
│  └──────────────────┘                                                      │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 组合键定义示例

复用 media_combo.c 的定义方式：

```c
// combo/media_combo.c（复用并根据产品定制）

#include "process_combo.h"
#include "FN_Combo.h"
#include "Special_Combo.h"

// 组合键触发键定义
const uint16_t PROGMEM FN_DOWN_combo[] = { S_FN_KEY, COMBO_END };
const uint16_t PROGMEM FN_UP_combo[]   = { S_FN_KEY, COMBO_END };
const uint16_t PROGMEM FN_F1_combo[]   = { KB_F1,    COMBO_END };
const uint16_t PROGMEM FN_DEL_combo[]  = { S_FN_KEY, KB_DELETE_FORWARD, COMBO_END };
const uint16_t PROGMEM Reset_combo[]   = { KB_ESCAPE, KB_EQUAL_N_PLUS, COMBO_END };

// 组合键数组
combo_t key_combos[] = {
    // Fn 键按下/释放
    [0] = COMBO(FN_DOWN_combo, PRESS_DOWN, FN_DOWN_KEY),
    [1] = COMBO(FN_UP_combo,   PRESS_UP,   FN_UP_KEY),

    // F1~F12 媒体键
    [2] = COMBO_LONG_TICKS(FN_F1_combo, 5, LONG_PRESS_HOLD, Bringhness_Down),
    // ...

    // 特殊组合
    [10] = COMBO_LONG_TICKS(FN_DEL_combo, 3000, LONG_PRESS_START, Pair_button),
    [11] = COMBO_LONG_TICKS(Reset_combo,  3000, LONG_PRESS_START, Factory_Reset),
};

uint8_t number_of_combos = sizeof(key_combos) / sizeof(combo_t);
```

### 后续扩展

当需要更复杂功能时，可逐步恢复备份的文件：

1. **Tap/Hold**：恢复 `action.c` 中的 `process_action_tapping()`
2. **OneShot**：恢复 `action_util.c` 中的 `oneshot_*` 函数
3. **多层同时激活**：恢复 `action_layer.c` 中的位图层管理

---

*设计日期: 2026-01-23*
*基于 keyboard-framework 精简设计*
*组合键实现复用 kb-04122*
