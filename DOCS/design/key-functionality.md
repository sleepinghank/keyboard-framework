# 键盘按键功能技术文档

> **文档版本:** 1.0
> **更新日期:** 2026-03-06
> **适用分支:** master

---

## 1. 概述

### 1.1 架构总览

本项目是一个模块化键盘固件框架，采用分层架构设计，参考 QMK 固件的按键处理流程。

```
┌─────────────────────────────────────────────────────────────────┐
│                    Application Layer                             │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────┐  │
│  │ test_main.c │  │ Services    │  │ system_init.c           │  │
│  └─────────────┘  └─────────────┘  └─────────────────────────┘  │
├─────────────────────────────────────────────────────────────────┤
│                    Middleware Layer                              │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │ keyboard/                                               │    │
│  │ ├── keyboard.c    (主协调器)                            │    │
│  │ ├── action.c      (动作执行)                            │    │
│  │ ├── action_layer.c (层级管理)                           │    │
│  │ ├── action_util.c (工具函数)                            │    │
│  │ ├── combo.c       (组合键)                              │    │
│  │ ├── report.c      (HID报告)                             │    │
│  │ └── custom_function.c (自定义功能)                      │    │
│  └─────────────────────────────────────────────────────────┘    │
├─────────────────────────────────────────────────────────────────┤
│                    Driver Layer                                  │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │ input/keyboard/                                         │    │
│  │ ├── matrix.c      (矩阵扫描)                            │    │
│  │ └── debounce.c    (防抖处理)                            │    │
│  └─────────────────────────────────────────────────────────┘    │
├─────────────────────────────────────────────────────────────────┤
│                    HAL Layer                                     │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │ platforms/test/  platforms/ch584/                       │    │
│  │ GPIO, ADC, PWM, I2C, SPI, UART 抽象接口                 │    │
│  └─────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────┘
```

### 1.2 数据流向

```
┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│  matrix_scan │───▶│   debounce   │───▶│ action_exec  │
│  (矩阵扫描)   │    │   (防抖)      │    │  (动作执行)   │
└──────────────┘    └──────────────┘    └──────┬───────┘
                                                │
                    ┌───────────────────────────┼───────────────────────────┐
                    ▼                           ▼                           ▼
           ┌──────────────┐           ┌──────────────┐           ┌──────────────┐
           │ layer_switch │           │ combo_task   │           │ report_gen   │
           │  (层级切换)   │           │  (组合键)     │           │  (报告生成)   │
           └──────────────┘           └──────────────┘           └──────┬───────┘
                                                                               │
                                                                               ▼
                                                                      ┌──────────────┐
                                                                      │ host_send    │
                                                                      │ (HID发送)    │
                                                                      └──────────────┘
```

### 1.3 模块依赖关系

```
keyboard.c
    ├── matrix.c (驱动层)
    ├── debounce.c (驱动层)
    ├── action.c
    │       ├── action_layer.c
    │       ├── action_util.c
    │       └── combo.c
    ├── report.c
    └── custom_function.c
```

---

## 2. 驱动层实现

### 2.1 矩阵扫描 (matrix.c)

**文件位置:** `drivers/input/keyboard/matrix.c`

#### 2.1.1 扫描模式

支持两种二极管方向：

| 模式 | 宏定义 | 描述 |
|------|--------|------|
| COL2ROW | `DIODE_DIRECTION COL2ROW` | 列选通，行读取（默认） |
| ROW2COL | `DIODE_DIRECTION ROW2COL` | 行选通，列读取 |

#### 2.1.2 核心数据结构

```c
// 矩阵行数据类型
typedef uint16_t matrix_row_t;

// 原始矩阵状态（防抖前）
extern matrix_row_t raw_matrix[MATRIX_ROWS];

// 防抖后矩阵状态
extern matrix_row_t matrix[MATRIX_ROWS];

// 行列引脚配置
pin_t row_pins[MATRIX_ROWS] = MATRIX_ROW_PINS;
pin_t col_pins[MATRIX_COLS] = MATRIX_COL_PINS;
```

#### 2.1.3 API 参考

| 函数 | 描述 | 返回值 |
|------|------|--------|
| `void matrix_init(void)` | 初始化矩阵 GPIO | - |
| `uint8_t matrix_scan(void)` | 执行一次矩阵扫描 | 矩阵是否变化 |
| `matrix_row_t matrix_get_row(uint8_t row)` | 获取指定行的状态 | 行数据位图 |
| `bool matrix_is_on(uint8_t row, uint8_t col)` | 检查指定键是否按下 | true/false |
| `void matrix_print(void)` | 打印矩阵状态（调试用） | - |

#### 2.1.4 扫描流程 (COL2ROW 模式)

```
for each col:
    1. select_col(col)     // 拉低列线
    2. matrix_output_select_delay()  // 等待信号稳定
    3. for each row:
        - read row pin
        - if LOW: set bit in matrix[row]
    4. unselect_col(col)   // 拉高列线
    5. matrix_output_unselect_delay()  // 等待释放
```

#### 2.1.5 配置项

```c
// keyboards/product_config.h
#define MATRIX_ROWS 8
#define MATRIX_COLS 16
#define MATRIX_IO_DELAY 30
#define DIODE_DIRECTION ROW2COL
#define MATRIX_INPUT_PRESSED_STATE 0  // 按下时的电平
```

#### 2.1.6 待完善项

| 项目 | 状态 | 说明 |
|------|------|------|
| `matrix_can_read()` | 未实现 | 硬件就绪检测 |
| `matrix_scan_kb/user()` | 注释掉 | 键盘/用户回调 |
| SPLIT_KEYBOARD | 条件编译 | 分体键盘支持 |

---

### 2.2 防抖处理 (debounce.c)

**文件位置:** `drivers/input/keyboard/debounce.c`

#### 2.2.1 算法选择

```c
#define DEBOUNCE_ALGORITHM DEBOUNCE_SYM_DEFER_PK  // 默认算法
```

| 算法 | 宏定义 | 特点 | 适用场景 |
|------|--------|------|----------|
| 无防抖 | `DEBOUNCE_NONE` | 直接透传 | 调试/硬件防抖 |
| 对称延迟 | `DEBOUNCE_SYM_DEFER_PK` | 按下/释放都等待 | **默认推荐** |
| 对称急切 | `DEBOUNCE_SYM_EAGER_PK` | 按下立即响应 | 游戏/快速输入 |
| 非对称 | `DEBOUNCE_ASYM_EAGER_DEFER_PK` | 按下快/释放慢 | 混合需求 |

#### 2.2.2 算法详解

**对称延迟 (sym_defer_pk):**
```
按下检测: 等待 DEBOUNCE_DELAY ms 稳定后确认
释放检测: 等待 DEBOUNCE_DELAY ms 稳定后确认
优点: 最稳定，无误触发
缺点: 响应延迟
```

**对称急切 (sym_eager_pk):**
```
按下检测: 立即确认
释放检测: 等待 DEBOUNCE_DELAY ms 稳定后确认
优点: 按下响应快
缺点: 可能误触发按下
```

**非对称 (asym_eager_defer_pk):**
```
按下检测: 等待 DEBOUNCE_PRESS_DELAY ms (可设为0)
释放检测: 等待 DEBOUNCE_RELEASE_DELAY ms
优点: 灵活配置
```

#### 2.2.3 API 参考

| 函数 | 描述 |
|------|------|
| `void debounce_init(uint8_t num_rows)` | 初始化防抖模块 |
| `bool debounce(matrix_row_t raw[], matrix_row_t cooked[], uint8_t num_rows, bool changed)` | 执行防抖处理 |
| `void debounce_free(void)` | 释放资源 |
| `void debounce_reset(void)` | 重置状态 |
| `const matrix_row_t* debounce_get_matrix(void)` | 获取防抖后矩阵 |
| `bool debounce_changed(void)` | 检查是否有变化 |

#### 2.2.4 配置项

```c
#define DEBOUNCE 5                    // 防抖时间 ms
#define DEBOUNCE_DELAY 5              // 延迟时间
#define MAX_MATRIX_ROWS 16            // 最大行数
#define DEBOUNCE_PRESS_DELAY 0        // 非对称模式按下延迟
#define DEBOUNCE_RELEASE_DELAY 5      // 非对称模式释放延迟
```

#### 2.2.5 待完善项

| 项目 | 状态 | 说明 |
|------|------|------|
| Per-key debounce | 未实现 | 每键独立防抖参数 |
| Debounce stats | 未实现 | 统计信息收集 |

---

## 3. 中间件层实现

### 3.1 键盘核心 (keyboard.c)

**文件位置:** `middleware/keyboard/keyboard.c`

#### 3.1.1 事件类型

```c
typedef enum keyevent_type_t {
    TICK_EVENT = 0,        // 定时事件
    KEY_EVENT = 1,         // 按键事件
    ENCODER_CW_EVENT = 2,  // 编码器顺时针
    ENCODER_CCW_EVENT = 3, // 编码器逆时针
    COMBO_EVENT = 4        // 组合键事件
} keyevent_type_t;
```

#### 3.1.2 事件结构

```c
typedef struct {
    keypos_t key;          // 按键位置 {row, col}
    uint16_t time;         // 时间戳
    keyevent_type_t type;  // 事件类型
    bool pressed;          // 按下/释放
} keyevent_t;

typedef struct {
    keyevent_t event;      // 事件
    uint8_t tap_count;     // 敲击计数
    uint16_t keycode;      // 键码
} keyrecord_t;
```

#### 3.1.3 主任务流程

```c
void keyboard_task(void) {
    // 1. 矩阵扫描 + 防抖
    if (matrix_task()) {
        activity_has_occurred = true;
    }

    // 2. Combo 处理
    #ifdef COMBO_ENABLE
    combo_task();
    #endif

    // 3. 自定义功能
    #if TAP_DANCE_ENABLE
    tap_dance_task();
    #endif
    #if LEADER_KEY_ENABLE
    leader_task();
    #endif
    #if MACRO_ENABLE
    macro_task();
    #endif

    // 4. 后台任务
    housekeeping_task();
}
```

#### 3.1.4 幽灵键检测

```c
// 检测条件：当一行有多个键按下，且与其他行的列有交叉时
static inline bool has_ghost_in_row(uint8_t row, matrix_row_t rowdata) {
    // 少于2个键按下，无幽灵
    if (popcount_more_than_one(rowdata) == 0) {
        return false;
    }
    // 检查与其他行的列交叉
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        if (i != row && popcount_more_than_one(get_real_keys(i, matrix_get_row(i)) & rowdata)) {
            return true;  // 检测到幽灵键
        }
    }
    return false;
}
```

#### 3.1.5 API 参考

| 函数 | 描述 |
|------|------|
| `void keyboard_init(void)` | 初始化键盘系统 |
| `void keyboard_task(void)` | 主任务循环 |
| `void keyboard_process_key(uint8_t row, uint8_t col, bool pressed)` | 处理单个按键事件 |
| `void housekeeping_task(void)` | 后台任务（可扩展） |

---

### 3.2 Action 处理系统 (action.c)

**文件位置:** `middleware/keyboard/action.c`

#### 3.2.1 Action 类型定义

```c
// Action 编码 (16位)
typedef union {
    uint16_t code;
    struct {
        uint8_t id;      // Action kind ID
        uint8_t param;   // 参数
    } kind;
    struct {
        uint8_t mods;    // 修饰键
        uint8_t code;    // 键码
    } key;
    // ... 其他联合成员
} action_t;
```

#### 3.2.2 Action Kind 分类

| Kind ID | 宏定义 | 描述 |
|---------|--------|------|
| 0x00 | `ACT_LMODS` | 左修饰键 + 键码 |
| 0x01 | `ACT_RMODS` | 右修饰键 + 键码 |
| 0x02 | `ACT_LMODS_TAP` | 左修饰键 Tap |
| 0x03 | `ACT_RMODS_TAP` | 右修饰键 Tap |
| 0x04 | `ACT_USAGE` | 系统/消费键 |
| 0x05 | `ACT_MOUSEKEY` | 鼠标键 |
| 0x06 | `ACT_LAYER` | 层位操作 |
| 0x07 | `ACT_LAYER_MODS` | 层 + 修饰键 |
| 0x08 | `ACT_LAYER_TAP` | 层 Tap |
| 0x09 | `ACT_LAYER_TAP_EXT` | 扩展层 Tap |
| 0x0A | `ACT_SWAP_HANDS` | 交换手 |

#### 3.2.3 层操作码

```c
#define OP_ON_OFF    0x00  // 按下开，释放关 (MO)
#define OP_OFF_ON    0x01  // 按下关，释放开
#define OP_SET_CLEAR 0x02  // 按下切换到层 (TO)
#define OP_ONESHOT   0x03  // 单次触发 (OSL)
#define OP_TAP_TOGGLE 0x04 // 敲击切换 (TG)
```

#### 3.2.4 Tapping 检测机制

```c
// Tapping 配置
#define TAPPING_TERM 200        // Tap 判定时间窗口 ms
#define TAPPING_TOGGLE 2        // TG 需要的敲击次数
#define QUICK_TAP_TERM 100      // 快速敲击判定

// Tapping 状态跟踪
static uint32_t key_press_time[MATRIX_ROWS][MATRIX_COLS];
static bool key_was_pressed[MATRIX_ROWS][MATRIX_COLS];

// Tap 判定逻辑
bool is_tap = (timer_read32() - key_press_time[row][col]) < TAPPING_TERM;
```

#### 3.2.5 修饰键管理

```c
uint8_t current_mods = 0;  // 当前修饰键状态

// 修饰键操作
void add_mods(uint8_t mods);
void del_mods(uint8_t mods);
void set_mods(uint8_t mods);
void clear_mods(void);

// 弱修饰键（临时）
void add_weak_mods(uint8_t mods);
void del_weak_mods(uint8_t mods);
void clear_weak_mods(void);

// 注册/注销
void register_mods(uint8_t mods);
void unregister_mods(uint8_t mods);
void register_code(uint8_t code);
void unregister_code(uint8_t code);
void tap_code(uint8_t code);
void tap_code_delay(uint8_t code, uint16_t delay);
```

#### 3.2.6 待完善项

| 项目 | 状态 | 说明 |
|------|------|------|
| Retro Tapping | 部分实现 | 长按后释放自动发送 Tap |
| Auto Shift | 未实现 | 长按自动 Shift |
| Hold on Other Key Press | 部分实现 | 被其他键打断时判定为 Hold |

---

### 3.3 层级管理 (action_layer.c)

**文件位置:** `middleware/keyboard/action_layer.c`

#### 3.3.1 层状态管理

```c
// 默认层（持久）
layer_state_t default_layer_state = 0;

// 活动层（临时）
layer_state_t layer_state = 0;

// 层状态类型（32位，最多32层）
typedef uint32_t layer_state_t;
```

#### 3.3.2 层操作 API

| 函数 | 描述 |
|------|------|
| `void layer_on(uint8_t layer)` | 激活指定层 |
| `void layer_off(uint8_t layer)` | 关闭指定层 |
| `void layer_move(uint8_t layer)` | 切换到指定层（关闭其他） |
| `void layer_clear(void)` | 关闭所有层 |
| `void layer_invert(uint8_t layer)` | 切换指定层状态 |
| `bool layer_state_is(uint8_t layer)` | 检查层是否激活 |
| `uint8_t layer_switch_get_layer(keypos_t key)` | 获取按键所在层 |

#### 3.3.3 层切换算法

```c
// 从最高层向下查找第一个非透明键码
uint8_t layer_switch_get_layer(keypos_t key) {
    layer_state_t layers = layer_state | default_layer_state;

    // 从最高层开始检查
    for (int8_t i = MAX_LAYER - 1; i >= 0; i--) {
        if (layers & ((layer_state_t)1 << i)) {
            action_t action = action_for_key(i, key);
            if (action.code != ACTION_TRANSPARENT) {
                return i;  // 找到非透明层
            }
        }
    }
    return 0;  // 默认层
}
```

#### 3.3.4 源层缓存

```c
// 用于解决层切换时的"按键卡住"问题
uint8_t source_layers_cache[MATRIX_ROWS * MATRIX_COLS / 8][MAX_LAYER_BITS];

void update_source_layers_cache(keypos_t key, uint8_t layer);
uint8_t read_source_layers_cache(keypos_t key);
```

#### 3.3.5 Tri-Layer 支持

```c
// 当 layer1 和 layer2 同时激活时，自动激活 layer3
layer_state_t update_tri_layer_state(layer_state_t state,
                                      uint8_t layer1,
                                      uint8_t layer2,
                                      uint8_t layer3);
```

---

### 3.4 Combo 系统 (combo.c)

**文件位置:** `middleware/keyboard/combo.c`

#### 3.4.1 Combo 配置

```c
// Combo 定义
const uint16_t ab_combo[] PROGMEM = {KC_A, KC_B, COMBO_END};
const uint16_t fghi_combo[] PROGMEM = {KC_F, KC_G, KC_H, KC_I, COMBO_END};

// Combo 表
combo_t key_combos[] = {
    [AB_COMBO] = COMBO(ab_combo, KC_SPC),     // A+B = Space
    [FGHI_COMBO] = COMBO(fghi_combo, KC_W),   // F+G+H+I = W
};
```

#### 3.4.2 Combo 参数

```c
#define COMBO_TERM 200           // Combo 判定时间窗口
#define COMBO_MOD_TERM 200       // 修饰键 Combo 时间
#define COMBO_KEY_BUFFER_LENGTH 8  // 键缓冲区大小
#define COMBO_BUFFER_LENGTH 16   // Combo 缓冲区大小
```

#### 3.4.3 Combo 状态

```c
typedef struct {
    const uint16_t *keys;    // 组合键列表
    uint16_t keycode;        // 触发的键码
    uint8_t state;           // 状态位图
    bool active;             // 是否激活
    bool disabled;           // 是否禁用
} combo_t;
```

#### 3.4.4 检测流程

```
1. 按键按下 → 检查是否属于某个 Combo
2. 如果是，标记 Combo 状态位
3. 所有 Combo 键按下 → 缓存 Combo
4. 等待 COMBO_TERM 超时
5. 超时后执行 Combo 或释放原按键
```

#### 3.4.5 API 参考

| 函数 | 描述 |
|------|------|
| `void combo_init(void)` | 初始化 Combo 系统 |
| `void combo_task(void)` | Combo 定时任务 |
| `bool process_combo(uint16_t keycode, keyrecord_t *record)` | 处理 Combo 事件 |
| `void combo_enable(void)` | 启用 Combo |
| `void combo_disable(void)` | 禁用 Combo |
| `void combo_toggle(void)` | 切换 Combo 状态 |
| `bool is_combo_enabled(void)` | 检查是否启用 |

#### 3.4.6 扩展功能 (条件编译)

| 功能 | 宏定义 | 说明 |
|------|--------|------|
| 每Combo必须Hold | `COMBO_MUST_HOLD_PER_COMBO` | 指定Combo必须长按 |
| 每Combo必须Tap | `COMBO_MUST_TAP_PER_COMBO` | 指定Combo必须点击 |
| 每Combo独立Term | `COMBO_TERM_PER_COMBO` | 每个Combo独立超时 |
| 必须按顺序 | `COMBO_MUST_PRESS_IN_ORDER` | 按键顺序要求 |

#### 3.4.7 待完善项

| 项目 | 状态 | 说明 |
|------|------|------|
| `action_tapping_process()` | 注释掉 | 与 Tapping 系统集成 |
| `process_record()` | 未完全集成 | 事件记录系统 |

---

### 3.5 HID 报告生成 (report.c)

**文件位置:** `middleware/keyboard/report.c`

#### 3.5.1 报告结构

```c
// 标准 6KRO 报告
typedef struct {
    uint8_t mods;           // 修饰键位图
    uint8_t reserved;       // 保留
    uint8_t keys[6];        // 最多6个普通键
} report_keyboard_t;

// NKRO 报告 (可选)
typedef struct {
    uint8_t report_id;
    uint8_t bits[NKRO_REPORT_BITS];  // 位图表示所有键
} report_nkro_t;
```

#### 3.5.2 修饰键位图

```
Bit 0: LCtrl    Bit 4: LGui
Bit 1: LShift   Bit 5: RCtrl
Bit 2: LAlt     Bit 6: RShift
Bit 3: LGui     Bit 7: RAlt
```

#### 3.5.3 报告操作 API

| 函数 | 描述 |
|------|------|
| `void add_key(uint8_t key)` | 添加键到报告 |
| `void del_key(uint8_t key)` | 从报告删除键 |
| `void clear_keys(void)` | 清空所有键 |
| `void add_key_to_report(uint8_t key)` | 添加键（自动选择6KRO/NKRO） |
| `void del_key_from_report(uint8_t key)` | 删除键（自动选择6KRO/NKRO） |
| `bool is_key_pressed(uint8_t key)` | 检查键是否在报告中 |
| `uint8_t has_anykey(void)` | 获取按下键数量 |

#### 3.5.4 6KRO 环形缓冲 (可选)

```c
#ifdef RING_BUFFERED_6KRO_REPORT_ENABLE
#define KEYBOARD_REPORT_KEYS 6
static int8_t cb_head = 0;
static int8_t cb_tail = 0;
static int8_t cb_count = 0;
#endif
```

#### 3.5.5 待完善项

| 项目 | 状态 | 说明 |
|------|------|------|
| `send_keyboard_report()` | 外部依赖 | 需要传输层实现 |
| Mouse 报告 | 部分实现 | `has_mouse_report_changed()` |
| NKRO 报告 | 条件编译 | `NKRO_ENABLE` |

---

### 3.6 自定义功能 (custom_function.c)

**文件位置:** `middleware/keyboard/custom_function.c`

#### 3.6.1 Tap Dance

```c
#if TAP_DANCE_ENABLE

typedef struct {
    uint8_t count;      // 敲击次数
    uint16_t time;      // 首次敲击时间
} tap_dance_state_t;

void tap_dance_init(void);
void tap_dance_task(void);
void tap_dance_action(keyevent_t event, uint8_t tap_count, uint8_t action);
uint8_t tap_dance_get_taps(keypos_t key);

#endif
```

**使用示例：**
```c
// 单击=A, 双击=B, 三击=C
void dance_a(keyrecord_t *record, uint8_t count) {
    switch (count) {
        case 1: tap_code(KC_A); break;
        case 2: tap_code(KC_B); break;
        default: tap_code(KC_C); break;
    }
}
```

#### 3.6.2 Leader Key

```c
#if LEADER_KEY_ENABLE

typedef struct {
    bool active;          // 是否激活
    uint8_t length;       // 序列长度
    uint16_t start_time;  // 开始时间
    uint16_t sequence[LEADER_MAX_LENGTH];  // 按键序列
} leader_state_t;

void leader_init(void);
void leader_event(keyevent_t event);
void leader_task(void);
bool leader_is_active(void);

#endif
```

**配置：**
```c
#define LEADER_TIMEOUT 300  // 超时时间 ms
```

#### 3.6.3 宏 (Macro)

```c
#if MACRO_ENABLE

void macro_init(void);
void macro_task(void);
void macro_play(const macro_t *macro);
void macro_record_start(void);
void macro_record_stop(void);

#endif
```

#### 3.6.4 待完善项

| 项目 | 状态 | 说明 |
|------|------|------|
| Tap Dance 回调 | 部分实现 | 需要 action_tapping 集成 |
| Leader 序列匹配 | 未实现 | 序列到动作的映射 |
| 宏录制/回放 | 未实现 | 动态宏功能 |

---

### 3.7 Action 工具函数 (action_util.c)

**文件位置:** `middleware/keyboard/action_util.c`

#### 3.7.1 修饰键状态

```c
static uint8_t real_mods;    // 物理修饰键
static uint8_t weak_mods;    // 弱修饰键（临时）

// One-shot 修饰键
static uint8_t oneshot_mods;
static uint8_t oneshot_locked_mods;
```

#### 3.7.2 One-shot 修饰键 API

| 函数 | 描述 |
|------|------|
| `void set_oneshot_mods(uint8_t mods)` | 设置 one-shot 修饰键 |
| `void clear_oneshot_mods(void)` | 清除 one-shot 修饰键 |
| `void add_oneshot_mods(uint8_t mods)` | 添加 one-shot 修饰键 |
| `uint8_t get_oneshot_mods(void)` | 获取 one-shot 修饰键 |
| `bool has_oneshot_mods_timed_out(void)` | 检查是否超时 |

#### 3.7.3 One-shot 层

```c
static uint8_t oneshot_layer_data;  // LLLL LSSS (层号+状态)

uint8_t get_oneshot_layer(void);
uint8_t get_oneshot_layer_state(void);
void set_oneshot_layer(uint8_t layer, uint8_t state);
void clear_oneshot_layer_state(uint8_t state);
void reset_oneshot_layer(void);
```

#### 3.7.4 配置

```c
#define ONESHOT_TIMEOUT 300       // 超时时间
#define ONESHOT_TAP_TOGGLE 2       // 切换需要的敲击次数
```

---

## 4. 配置系统

### 4.1 产品配置 (product_config.h)

**文件位置:** `keyboards/product_config.h`

#### 4.1.1 矩阵配置

```c
#define MATRIX_ROWS 8
#define MATRIX_COLS 16
#define MATRIX_IO_DELAY 30
#define DIODE_DIRECTION ROW2COL
#define MATRIX_ROW_PINS { A4, A5, A6, A0, A1, A8, A9, B9 }
#define MATRIX_COL_PINS { B5, B8, B17, B16, ... }
```

#### 4.1.2 时序配置

```c
#define TAPPING_TERM 200        // Tap 判定窗口
#define TAPPING_TOGGLE 2        // TG 敲击次数
#define QUICK_TAP_TERM 100      // 快速敲击
#define TAP_CODE_DELAY 100      // Tap 延迟
#define TAP_HOLD_CAPS_DELAY 80  // Caps Lock 延迟
```

#### 4.1.3 功能开关

```c
#define COMBO_ENABLE
#define EXTRAKEY_ENABLE
// #define NKRO_ENABLE
// #define MOUSE_ENABLE
```

### 4.2 键位映射 (default_keymap.h)

**文件位置:** `keyboards/keymaps/default_keymap.h`

#### 4.2.1 层定义

```c
enum layers {
    MAC_BASE,   // Mac 基础层
    MAC_FN,     // Mac 功能层
    WIN_BASE,   // Windows 基础层
    WIN_FN,     // Windows 功能层
};
```

#### 4.2.2 自定义键码

```c
enum {
    KC_LOPTN = QK_KB_0,     // 左 Option
    KC_ROPTN,               // 右 Option
    KC_LCMMD,               // 左 Command
    KC_RCMMD,               // 右 Command
    BT_HST1,                // 蓝牙主机1
    BT_HST2,                // 蓝牙主机2
    TOUCH_LOCK,             // 触摸板锁定
    // ...
};
```

#### 4.2.3 层切换宏

```c
#define FN_MAC MO(MAC_FN)   // Mac Fn 键
#define FN_WIN MO(WIN_FN)   // Windows Fn 键
```

---

## 5. 待完善功能清单

### 5.1 未实现功能

| 功能 | 优先级 | 复杂度 | 说明 |
|------|--------|--------|------|
| 完整 NKRO 支持 | 高 | 中 | 需要传输层配合 |
| Mouse 键支持 | 中 | 低 | `ACT_MOUSEKEY` 未实现 |
| Encoder 支持 | 中 | 低 | 编码器事件定义已有 |
| Swap Hands | 低 | 中 | `ACT_SWAP_HANDS` 部分实现 |
| Caps Word | 低 | 中 | 自动大小写 |
| Auto Shift | 低 | 高 | 长按自动Shift |
| Key Override | 低 | 高 | 按键重映射 |
| 动态宏 | 低 | 高 | 运行时录制 |

### 5.2 部分实现功能

| 功能 | 完成度 | 缺失部分 |
|------|--------|----------|
| Combo | 80% | 与 action_tapping 集成 |
| Tap Dance | 60% | 回调系统不完整 |
| Leader Key | 50% | 序列匹配未实现 |
| Layer 缓存 | 90% | 边界情况处理 |
| One-shot | 85% | Swap Hands 集成 |

### 5.3 建议改进

1. **代码解耦**: `action.c` 文件过大（976行），建议拆分为:
   - `action_process.c` - 主处理逻辑
   - `action_mods.c` - 修饰键处理
   - `action_layer.c` - 层操作（已部分分离）
   - `action_tapping.c` - Tapping 状态机

2. **统一事件系统**: 当前有多个事件分发点，建议统一到 `event_manager.c`

3. **测试覆盖**: 添加单元测试，特别是:
   - Tapping 检测边界条件
   - 层切换逻辑
   - Combo 状态机

4. **文档完善**: 为每个公开 API 添加 Doxygen 注释

---

## 附录 A: 文件清单

### 驱动层

| 文件 | 行数 | 描述 |
|------|------|------|
| `drivers/input/keyboard/matrix.c` | 258 | 矩阵扫描 |
| `drivers/input/keyboard/matrix.h` | - | 矩阵接口 |
| `drivers/input/keyboard/debounce.c` | 322 | 防抖处理 |
| `drivers/input/keyboard/debounce.h` | - | 防抖接口 |

### 中间件层

| 文件 | 行数 | 描述 |
|------|------|------|
| `middleware/keyboard/keyboard.c` | 230 | 键盘核心 |
| `middleware/keyboard/keyboard.h` | 123 | 键盘接口 |
| `middleware/keyboard/action.c` | 976 | Action 处理 |
| `middleware/keyboard/action.h` | - | Action 接口 |
| `middleware/keyboard/action_layer.c` | 364 | 层级管理 |
| `middleware/keyboard/action_util.c` | 622 | Action 工具 |
| `middleware/keyboard/combo.c` | 673 | Combo 系统 |
| `middleware/keyboard/report.c` | 362 | HID 报告 |
| `middleware/keyboard/custom_function.c` | 182 | 自定义功能 |

### 配置文件

| 文件 | 描述 |
|------|------|
| `keyboards/product_config.h` | 产品配置 |
| `keyboards/keymaps/default_keymap.h` | 默认键位 |
| `keyboards/keymaps/layout_8x16.h` | 布局宏 |

---

## 附录 B: 编译宏参考

### 功能开关

```c
// 禁用功能
#define NO_ACTION_LAYER      // 禁用层功能
#define NO_ACTION_TAPPING    // 禁用 Tapping
#define NO_ACTION_ONESHOT    // 禁用 One-shot

// 启用功能
#define COMBO_ENABLE         // 组合键
#define NKRO_ENABLE          // N键无冲
#define EXTRAKEY_ENABLE      // 扩展键
#define MOUSE_ENABLE         // 鼠标键
#define TAP_DANCE_ENABLE     // Tap Dance
#define LEADER_KEY_ENABLE    // Leader Key
#define MACRO_ENABLE         // 宏
```

### 调试开关

```c
#define ACTION_DEBUG         // Action 调试输出
#define DEBUG_DEBOUNCE       // 防抖调试输出
```

---

*文档结束*
