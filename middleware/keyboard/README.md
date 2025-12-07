# 跨芯片键盘中间件框架

## 概述

这是一个高内聚、低耦合的跨芯片平台键盘中间件框架，支持多模无线键盘和触控板产品开发。框架参考QMK固件的处理流程，专为快速产品迭代与高效开发而设计。

### 支持的平台

- **原相 PAN32860** - 高性能低功耗无线键盘芯片
- **沁恒 CH584** - 国产RISC-V无线键盘芯片
- **Nordic nRF52/nRF53** - BLE/2.4G无线键盘芯片
- **STM32系列** - 通用ARM Cortex-M键盘控制

### 主要特性

✅ **模块化设计** - 高度解耦的模块化架构
✅ **跨平台支持** - 统一的API，适配多种芯片
✅ **低延迟处理** - 优化的按键处理流程
✅ **功能丰富** - 消抖、层切换、组合键、自定义功能
✅ **易于扩展** - 清晰的接口和回调钩子
✅ **开发友好** - 详细的调试信息和示例

## 架构设计

### 模块结构

```
middleware/keyboard/
├── keyboard_framework.h       # 主框架接口
├── keyboard_framework.c       # 框架实现
├── keyboard_core.h/.c         # 核心键盘处理
├── keycode.h                  # 键码定义
├── layer.h/.c                 # 层管理
├── debounce.h/.c              # 消抖处理
├── action.h/.c                # 动作处理
├── combo.h/.c                 # 组合键
├── custom_function.h/.c       # 自定义功能
├── platform_*.h               # 平台特定头文件
├── example_usage.c            # 使用示例
└── README.md                  # 本文档
```

### 数据流程

```
矩阵扫描 → 原始矩阵 → 消抖 → 键事件 → 动作查找 → 执行动作
    ↓           ↓         ↓       ↓         ↓
  GPIO读取   硬件滤波   状态过滤  事件触发   按键/层/宏
```

### 核心模块

#### 1. 消抖模块 (debounce.h/c)
提供多种消抖算法，滤除按键抖动：
- **对称延迟消抖** (DEBOUNCE_SYM_DEFER_PK) - 最通用，平衡响应和稳定性
- **对称急动消抖** (DEBOUNCE_SYM_EAGER_PK) - 游戏优化，按键立即响应
- **非对称消抖** (DEBOUNCE_ASYM_EAGER_DEFER_PK) - 按键快释放慢

#### 2. 层管理模块 (layer.h/c)
基于层的键码映射系统：
- 支持最多8/16/32层（可配置）
- 透明键传递
- 三层互斥层支持
- 层状态调试

#### 3. 组合键模块 (combo.h/c)
组合键检测和处理：
- 最多4键组合
- 可配置超时时间
- 修饰键扩展支持
- 防误触优化

#### 4. 动作模块 (action.h/c)
统一的动作执行系统：
- 常规按键动作
- 修饰键管理
- 层切换动作
- Mod-Tap/Layer-Tap支持

#### 5. 自定义功能模块 (custom_function.h/c)
高级键盘功能：
- **Tap Dance** - 快速连击触发不同动作
- **Leader Key** - 序列按键触发功能
- **宏录制** - 按键序列录制和回放

## 快速开始

### 1. 基础使用

```c
#include "keyboard_framework.h"

int main(void) {
    // 初始化框架
    KEYBOARD_FRAMEWORK_INIT();

    // 设置默认层
    default_layer_set(LAYER_BIT(0));

    while (1) {
        // 主循环调用
        KEYBOARD_FRAMEWORK_TASK();

        wait_ms(1);
    }
}
```

### 2. 定义键位映射

```c
const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // Layer 0 - 基础层
    [0] = {
        {KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5},
        {KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T},
        {KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G},
        {KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B},
        {KC_LCTL, KC_LALT, KC_LGUI, KC_SPACE,KC_RGUI, KC_RCTL}
    },
    // Layer 1 - 功能层
    [1] = {
        {KC_GRV,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5},
        {KC_TRNS, KC_TRNS, KC_UP,   KC_TRNS, KC_TRNS, KC_F6},
        {KC_TRNS, KC_LEFT, KC_DOWN, KC_RIGHT,KC_TRNS, KC_F7},
        {KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_F8},
        {KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS}
    }
};

const uint8_t num_keymaps = 2;
```

### 3. 配置层切换

```c
// 在键位映射中直接使用
[0] = {
    {KC_ESC,  KC_1,   KC_2,   KC_3},
    {KC_TAB,  KC_Q,   KC_W,   MO(1)},   // 按住时激活层1
    ...
}

// 或在代码中控制
layer_on(1);      // 激活层1
layer_off(1);     // 关闭层1
layer_invert(1);  // 切换层1
```

### 4. 配置组合键

```c
#ifdef COMBO_ENABLE
const combo_t COMBO[] = {
    {
        .key = {{0, 0}, {1, 0}},  // Ctrl位置和C位置
        .keycode = KC_C,
        .count = 2
    },
    {
        .key = {{0, 0}, {1, 2}},  // Ctrl位置和V位置
        .keycode = KC_V,
        .count = 2
    }
};

const uint16_t COMBO_COUNT = sizeof(COMBO) / sizeof(COMBO[0]);
#endif
```

### 5. 自定义键码处理

```c
bool process_record_user(action_record_t* record, uint16_t keycode) {
    switch (keycode) {
        case KC_CAPS:
            if (record->event.pressed) {
                // 自定义Caps Lock处理
                led_toggle_caps_lock();
            }
            break;

        case KC_LGUI:
            // 自定义GUI键处理
            break;

        default:
            break;
    }
    return true;
}
```

### 6. 平台特定实现

```c
#ifdef PLATFORM_CHIPSTM32
void matrix_init(void) {
    // STM32 GPIO配置
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct = {
        .GPIO_Pin = GPIO_Pin_All,
        .GPIO_Mode = GPIO_Mode_IPU,
        .GPIO_Speed = GPIO_Speed_50MHz
    };
    GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void matrix_scan(void) {
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        // 读取GPIO并更新matrix[row]
    }
}
#endif
```

## 配置选项

在 `keyboards/product_config.h` 中配置：

```c
// 消抖配置
#define DEBOUNCE_ALGORITHM DEBOUNCE_SYM_DEFER_PK
#define DEBOUNCE_DELAY 5

// 层配置
#define LAYER_STATE_16BIT
#define MAX_LAYER_COUNT 16

// 组合键配置
#define COMBO_ENABLE 1
#define COMBO_TERM 200
#define COMBO_MOD_TERM 200

// 自定义功能
#define TAP_DANCE_ENABLE 1
#define LEADER_KEY_ENABLE 0
#define MACRO_ENABLE 1

// 调试选项
#define KEYBOARD_DEBUG 1
```

## API参考

### 核心API

```c
// 初始化
int keyboard_framework_init(void);

// 主循环
void keyboard_framework_task(void);

// 层操作
void layer_on(uint8_t layer);
void layer_off(uint8_t layer);
void layer_invert(uint8_t layer);
layer_state_t layer_state_get(void);

// 修饰键
void register_mods(uint8_t mods);
void unregister_mods(uint8_t mods);
uint8_t get_mods(void);

// 按键操作
void register_code(uint8_t code);
void unregister_code(uint8_t code);
void tap_code(uint8_t code);

// 调试
void matrix_print(void);
void layer_debug(void);
```

### 高级API

```c
// 组合键
bool combo_event(keyevent_t event);
void combo_task(void);

// Tap Dance
#if TAP_DANCE_ENABLE
void tap_dance_init(void);
void tap_dance_task(void);
#endif

// Leader Key
#if LEADER_KEY_ENABLE
void leader_init(void);
void leader_task(void);
#endif

// 宏
#if MACRO_ENABLE
void macro_init(void);
void macro_play(uint8_t macro_id);
void macro_task(void);
#endif
```

## 性能优化

### 1. 消抖算法选择

- **游戏场景**: 使用 `DEBOUNCE_SYM_EAGER_PK` 获得最快响应
- **办公场景**: 使用 `DEBOUNCE_SYM_DEFER_PK` 保证稳定性
- **特殊需求**: 使用 `DEBOUNCE_ASYM_EAGER_DEFER_PK`

### 2. 矩阵扫描优化

- 合理设置扫描频率（一般5-10ms）
- 使用硬件中断检测按键变化
- 优化GPIO读取速度

### 3. 内存优化

```c
// 根据需要调整层数
#define MAX_LAYER_COUNT 4  // 默认16，可减少节省内存

// 调整组合键数量
#define MAX_COMBOS 16  // 默认32，可减少节省内存

// 关闭不需要的功能
#define COMBO_ENABLE 0
#define TAP_DANCE_ENABLE 0
```

## 调试指南

### 1. 启用调试

```c
#define KEYBOARD_DEBUG 1
```

### 2. 调试输出

```c
// 在代码中添加调试信息
dprintf("Keyboard: Key pressed at (%d, %d)\n", row, col);

// 查看层状态
layer_debug();

// 查看矩阵状态
matrix_print();
```

### 3. 常见问题

**Q: 按键不响应**
A: 检查矩阵定义、GPIO配置、消抖设置

**Q: 组合键不生效**
A: 检查COMBO_ENABLE是否启用、组合键定义是否正确

**Q: 层切换无效**
A: 确认键位映射中使用了正确的层切换键码

**Q: 响应延迟高**
A: 调整消抖算法、增加扫描频率、优化中断处理

## 扩展开发

### 添加新的消抖算法

```c
// 在debounce.c中添加新的算法函数
static bool debounce_custom_algorithm(matrix_row_t raw[], matrix_row_t cooked[], uint8_t num_rows, bool changed) {
    // 实现自定义消抖逻辑
    return true;
}

// 在debounce.h中定义常量
#define DEBOUNCE_CUSTOM_ALGO 4

// 在keyboard_framework.h中选择算法
#define DEBOUNCE_ALGORITHM DEBOUNCE_CUSTOM_ALGO
```

### 添加新的自定义功能

```c
// 在custom_function.h中定义
typedef void (*custom_fn_t)(void);
extern custom_fn_t custom_functions[];

// 在custom_function.c中实现
void custom_function_0(void) {
    dprintf("Executing custom function 0\n");
    // 执行自定义功能
}

custom_fn_t custom_functions[] = {
    custom_function_0,
    // 更多自定义功能
};
```

### 适配新平台

1. 在 `platform_<platform>.h` 中定义平台特定函数
2. 在 `keyboard_framework.c` 中检测平台
3. 实现对应的GPIO、计时器、矩阵扫描函数

## 许可证

本项目采用 GPL-3.0 许可证发布。

## 贡献

欢迎提交Issue和Pull Request来改进这个框架。

## 联系方式

如有问题或建议，请提交到项目Issues页面。

---

**版本**: 1.0.0
**更新日期**: 2024-12-06
