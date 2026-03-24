# Keyboard 中间件模块

按键处理中间件，负责从矩阵扫描到 HID 报告生成的完整按键处理流程。

## 目录结构

```
middleware/keyboard/
├── keyboard.h/c          # 主协调模块（入口）
├── keymap.h/c            # 层级管理
├── report.h/c            # HID 报告生成
├── keycode.h             # 键码定义（含媒体键）
├── keycodes.h            # QMK 标准键码
├── modifiers.h           # 修饰键定义
├── keycode_config.h/c    # 键码配置
├── combo/                # 组合键子模块
│   ├── process_combo.h/c # 组合键状态机
│   ├── FN_Combo.h/c      # Fn 组合键定义
│   ├── Special_Combo.h/c # 特殊组合键
│   └── media_combo.c     # 媒体组合键
└── keymap_extras/        # 多语言键位映射
```

## 按键处理流程

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         keyboard_task() 主循环                          │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  1. scan_and_debounce()                                                 │
│     ├── matrix_scan()          驱动层矩阵扫描                            │
│     └── debounce()             驱动层防抖处理                            │
│                                                                         │
│  2. process_layer_switch_key() 优先处理层切换键 ★                        │
│     ├── 检查 FN_KEY_ROW/COL    固定位置，零遍历开销                      │
│     ├── IS_QK_MOMENTARY()      判断是否为 MO 键                          │
│     └── layer_on/off()         更新层状态                                │
│                                                                         │
│  3. update_key_code_list()     更新按键链表                              │
│     ├── 检测矩阵变化           current ^ previous                        │
│     ├── keymap_get_keycode()   获取当前层键码（层状态已正确）             │
│     └── 维护 _key_code_list    按下添加 / 释放移除                       │
│                                                                         │
│  4. combo_task()               组合键处理                                │
│     ├── apply_combo()          检测组合键触发                            │
│     ├── button_ticks()         状态机更新                                │
│     └── 回调执行               触发事件回调                              │
│                                                                         │
│  5. report_update_proc()       生成 HID 报告                             │
│     ├── 遍历 _key_code_list    收集活跃键                                │
│     ├── classify_and_add()     键码分类                                  │
│     └── report_buffer_enqueue  入队发送                                  │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

> **设计说明**：步骤 2 通过宏定义 `FN_KEY_ROW/FN_KEY_COL` 指定层切换键位置，
> 在遍历矩阵前优先处理，解决"Fn + 功能键同时按下"时的层状态时序问题。

## 核心模块详解

### 1. keyboard.c - 主协调模块

**职责**：协调整个按键处理流程

**核心数据结构**：
```c
// 按键列表（链表实现）
list_t* _key_code_list;         // 当前按下的键
list_t* _key_code_list_extend;  // 组合键扩展输出

// 矩阵状态
static matrix_row_t matrix_previous[MATRIX_ROWS];   // 上一次状态
static matrix_row_t matrix_debounced[MATRIX_ROWS];  // 防抖后状态
```

**主要函数**：
```c
void keyboard_init(void);                      // 初始化所有子模块
void keyboard_task(void);                      // 主循环任务
key_update_st_t keyboard_get_last_update_state(void);  // 获取更新状态
```

**按键更新状态**：
```c
typedef enum {
    NO_KEY_UPDATE = 0,  // 无变化
    KEY_UPDATE,         // 有按键变化
    GHOST_KEY           // 检测到幽灵键
} key_update_st_t;
```

### 2. keymap.c - 层级管理

**层级定义**：
```c
typedef enum {
    LAYER_BASE = 0,     // 基础层
    LAYER_FN,           // 功能层
    LAYER_MACOS,        // macOS 层
    LAYER_WINDOWS,      // Windows 层
    LAYER_CUSTOM_1,     // 自定义层 1
    LAYER_CUSTOM_2,     // 自定义层 2
    LAYER_MAX
} layer_id_t;
```

**层状态管理**：
```c
typedef struct {
    layer_id_t current_layer;   // 当前激活层
    layer_id_t base_layer;      // 基础层（TG 切换目标）
    layer_id_t mo_stack[4];     // MO 临时层栈
    uint8_t    mo_stack_top;    // 栈顶索引
} layer_state_t;
```

**层切换 API**：
```c
void layer_set(layer_id_t layer);     // TG: 切换基础层
void layer_on(layer_id_t layer);      // MO: 临时激活层（压栈）
void layer_off(layer_id_t layer);     // MO: 取消临时层（出栈）
layer_id_t layer_get_current(void);   // 获取当前层

// 键码获取（支持透明键回退）
uint16_t keymap_get_keycode(uint8_t row, uint8_t col);
```

**透明键处理**：
- 当前层键码为 `KC_TRANSPARENT` 时，自动回退到基础层

### 3. combo/process_combo.c - 组合键状态机

**组合键对象**：
```c
typedef struct combo_t {
    const uint16_t *keys;           // 组合键列表（以 COMBO_END 结尾）
    bool  disabled;                 // 是否禁用
    uint32_t press_time;            // 按下时刻时间戳（ms）
    uint8_t  repeat : 4;            // 重复次数
    uint8_t  event : 4;             // 当前事件
    uint8_t  state : 5;             // 状态机状态
    uint8_t  fn_combo : 1;          // 是否为 Fn 组合键
    uint8_t  active_status : 1;     // 是否激活
    uint8_t  button_level : 1;      // 当前电平
    uint16_t long_press_ms;         // 长按阈值（ms）
    BtnCallback cb[number_of_event]; // 事件回调数组
} combo_t;
```

**支持的事件类型**：
```c
typedef enum {
    PRESS_DOWN = 0,      // 按下
    PRESS_UP,            // 释放
    PRESS_REPEAT,        // 重复
    SINGLE_CLICK,        // 单击
    DOUBLE_CLICK,        // 双击
    LONG_PRESS_START,    // 长按开始
    LONG_PRESS_HOLD,     // 长按保持
    NONE_PRESS           // 无事件
} PressEvent;
```

**状态机流程**：
```
        ┌───────────────────────────────────────────────────┐
        │                                                   │
        ▼                                                   │
    ┌───────┐   按下    ┌───────┐   释放    ┌───────┐      │
    │ STATE │ ────────► │ STATE │ ────────► │ STATE │      │
    │   0   │           │   1   │           │   2   │      │
    │ 空闲  │           │等待释放│           │等待重按│      │
    └───────┘           └───────┘           └───────┘      │
                             │                   │          │
                             │ 超时              │ 超时      │
                             ▼                   ▼          │
                        ┌───────┐          单击/双击        │
                        │ STATE │ ─────────────────────────┘
                        │   5   │
                        │ 长按  │
                        └───────┘
```

**组合键定义宏**：
```c
// 单事件组合键
COMBO(keys, event_idx, callback)

// 双事件组合键
COMBO2(keys, event1, cb1, event2, cb2)

// 带自定义长按阈值
COMBO_LONG_MS(keys, long_press_ms, event_idx, callback)
```

### 4. report.c - HID 报告生成

**键盘报告结构**：
```c
typedef struct {
    uint8_t mods;                       // 修饰键位图
    uint8_t reserved;                   // 保留字节
    uint8_t keys[KEYBOARD_REPORT_KEYS]; // 普通键（6KRO）
} report_keyboard_t;
```

**键码分类逻辑**：
```c
static void classify_and_add_keycode(uint16_t keycode, ...) {
    // 1. 跳过层切换键
    if (IS_QK_MOMENTARY(keycode) || IS_QK_TOGGLE_LAYER(keycode)) return;

    // 2. 修饰键 (0xE0-0xE7)
    if (keycode >= KC_LCTRL && keycode <= KC_RGUI) {
        kb_report->mods |= (1 << (keycode - KC_LCTRL));
        return;
    }

    // 3. 普通键 (0x04-0xDF)
    if (keycode >= KC_A && keycode < KC_LCTRL) {
        kb_report->keys[key_idx++] = (uint8_t)keycode;
        return;
    }

    // 4. 媒体键（M_KEY_TYPE 标记）
    if ((keycode & M_KEY_TYPE) == M_KEY_TYPE) {
        *consumer_report = keycode ^ M_KEY_TYPE;
        return;
    }
}
```

**报告发送流程**：
1. 遍历 `_key_code_list` 和 `_key_code_list_extend`
2. 仅处理 `is_report == 1` 的节点
3. 与上一次报告比较，有变化才发送
4. 通过 `report_buffer_enqueue()` 入队

### 5. linkedlist - 按键链表

**节点数据结构**：
```c
typedef struct {
    uint16_t key_code;      // 键值
    uint8_t cycle;          // 按下时长
    uint8_t is_report;      // 是否上报（0=禁用, 1=启用）
    report_t report_type;   // 报告类型
} bouncing_data_t;
```

**核心操作**：
```c
add(keycode, list)          // 添加按键（按下时）
del(keycode, list)          // 删除按键（释放时）
deactivate(keycode, list)   // 禁用上报（组合键触发时）
find_key(list, keycode)     // 查找按键
find_activate_key(list, keycode)  // 查找活跃按键
```

## 键码范围

| 范围 | 类型 | 说明 |
|------|------|------|
| 0x00-0x00 | KC_NO | 无键 |
| 0x01 | KC_TRANSPARENT | 透明键 |
| 0x04-0xDF | 普通键 | 标准 HID 键码 |
| 0xE0-0xE7 | 修饰键 | LCtrl~RGui |
| 0x1000+ | 媒体键 | M_KEY_TYPE 标记 |
| 0x5220-0x523F | MO(n) | 临时层切换 |
| 0x5260-0x527F | TG(n) | 切换层 |

## 配置选项

在 `keyboards/product_config.h` 中配置：

```c
// 矩阵配置
#define MATRIX_ROWS 6
#define MATRIX_COLS 13
#define MATRIX_ROW_PINS { 0, 1, 2, 3, 4, 5 }
#define MATRIX_COL_PINS { 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 }
#define DIODE_DIRECTION COL2ROW

// 防抖配置
#define DEBOUNCE 5  // ms

// 层切换键物理位置（Fn 键）
// 解决 Fn + 功能键同时按下时的层状态时序问题
#define FN_KEY_ROW  6   // Fn 键所在行
#define FN_KEY_COL  10  // Fn 键所在列
```

组合键时间参数（`process_combo.h`）：
```c
#define SHORT_MS  300    // 短按阈值（ms）
#define LONG_MS   2500   // 默认长按阈值（ms）
```

## 使用示例

### 初始化

```c
void main(void) {
    keyboard_init();

    while (1) {
        keyboard_task();
        // 其他任务...
        wait_ms(5);  // combo 判定已改为基于 timer_elapsed32() 的绝对时间
    }
}
```

### 定义键位映射

```c
// keyboards/product_keymap.c
const uint16_t keymap_layers[LAYER_MAX][MATRIX_ROWS][MATRIX_COLS] = {
    [LAYER_BASE] = {
        { KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5 },
        { KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T },
        { KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G },
        { KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B },
        { KC_LCTL, KC_LALT, KC_LGUI, KC_SPC,  MO(1),   KC_RCTL }
    },
    [LAYER_FN] = {
        { KC_GRV,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5 },
        { KC_TRNS, KC_TRNS, KC_UP,   KC_TRNS, KC_TRNS, KC_F6 },
        { KC_TRNS, KC_LEFT, KC_DOWN, KC_RGHT, KC_TRNS, KC_F7 },
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_F8 },
        { KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS }
    },
};
```

### 定义组合键

```c
// 组合键按键列表
const uint16_t combo_esc_reset[] = { KC_ESC, KC_LCTL, COMBO_END };
const uint16_t combo_fn_f1[] = { S_FN_KEY, KC_F1, COMBO_END };

// 回调函数
uint8_t on_esc_reset(uint16_t* add_keys) {
    // 执行复位操作
    return 0;
}

uint8_t on_fn_f1(uint16_t* add_keys) {
    add_keys[0] = M_BACKLIGHT_UP;
    return 1;
}

// 组合键数组
combo_t key_combos[] = {
    COMBO(combo_esc_reset, LONG_PRESS_START, on_esc_reset),
    COMBO(combo_fn_f1, PRESS_DOWN, on_fn_f1),
};
uint8_t number_of_combos = sizeof(key_combos) / sizeof(combo_t);
```

## 依赖关系

```
keyboard.c
├── drivers/input/keyboard/matrix.h    矩阵扫描
├── drivers/input/keyboard/debounce.h  防抖处理
├── utils/linkedlist.h                 链表工具
├── middleware/communication/report_buffer.h  报告队列
└── combo/process_combo.h              组合键处理
```

## 调试

启用调试输出（`application/sys_config.h`）：
```c
#define PRINTF_LEVEL PRINTF_LEVEL_DEBUG
```

调试函数：
```c
// 打印矩阵状态
matrix_print();

// 打印链表内容
display(_key_code_list);
```

## 注意事项

1. **6KRO 限制**：标准 HID 报告仅支持同时 6 个普通键，修饰键不计入
2. **组合键优先级**：组合键触发时会调用 `deactivate()` 禁用成员键的上报
3. **层切换键**：MO/TG 键不会出现在 HID 报告中
4. **媒体键**：使用 Consumer 报告发送，与键盘报告分开
