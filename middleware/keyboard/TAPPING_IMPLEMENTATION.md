# 简化 Tap 处理实现

本文档描述了在 `action.c` 中实现的简化 tap 处理逻辑。

## 概述

简化了 QMK 复杂的 tap 状态跟踪，专注于核心功能：
- 层切换（Layer Tap）
- 键值转换（Key Tap）
- 修饰键组合（Modifier Tap）

## 实现位置

### 文件修改

1. **action.c**
   - 修改了 `action_exec()` 函数
   - 新增了 `process_action_tapping()` 函数

2. **action.h**
   - 新增了 `process_action_tapping()` 函数声明

### 函数说明

#### `action_exec(keyevent_t event)`

主要的按键事件处理函数，处理流程：

1. 清除弱修饰键（如果按键按下）
2. 创建 `keyrecord_t` 记录
3. 获取当前层状态
4. 获取该键位置的动作
5. 如果启用 Combo，先处理 Combo
6. 如果 Combo 未触发，调用 `process_action_tapping()` 处理 tap

#### `process_action_tapping(keyrecord_t *record, action_t action)`

简化的 tap 处理函数，支持：

**1. 层 Tap (ACT_LAYER_TAP, ACT_LAYER_TAP_EXT)**

- `OP_TAP_TOGGLE` (TG): 点击切换层状态
- `OP_ON_OFF` (MO): 按下激活层，释放关闭层
- `OP_OFF_ON`: 释放时激活层
- `OP_SET_CLEAR` (TO): 点击切换到指定层
- 默认行为: 短按发送键码，长按保持层激活

**2. 修饰键 Tap (ACT_LMODS_TAP, ACT_RMODS_TAP)**

- 短按: 发送键码 + 修饰键
- 长按: 仅保持修饰键

**3. 常规键 (ACT_LMODS, ACT_RMODS)**

- 按下时注册键码
- 释放时注销键码

**4. 层操作 (ACT_LAYER)**

- 支持位操作: AND, OR, XOR, SET

**5. 层修饰键组合 (ACT_LAYER_MODS)**

- 按下时激活层并注册修饰键
- 释放时注销修饰键并关闭层

## Tap 检测机制

使用静态数组跟踪按键按下时间：

```c
static uint32_t key_press_time[MATRIX_ROWS][MATRIX_COLS];
static bool key_was_pressed[MATRIX_ROWS][MATRIX_COLS];
```

判断逻辑：
- 按下时记录时间
- 释放时计算持续时间
- 如果持续时间 < `TAPPING_TERM`，则为 tap
- 否则为 hold

## 配置

### 必要头文件

```c
#include "timer.h"          // 用于 timer_read32()
#include "layer.h"          // 用于层操作函数
#include "action_util.h"    // 用于修饰键操作
```

### 宏定义

需要在 `product_config.h` 中定义：

```c
#ifndef TAPPING_TERM
#define TAPPING_TERM 200  // tap/hold 判定时间（毫秒）
#endif
```

## 使用示例

### 1. 基础层切换

```c
// MO(1) - 按住切换到层1，释放回到层0
#define LAYER_TAP_KEY  MO(1)

// TG(2) - 点击切换层2状态
#define LAYER_TOGGLE   TG(2)

// TO(3) - 点击切换到层3
#define LAYER_SWITCH   TO(3)
```

### 2. 修饰键组合

```c
// MT(MOD_LSFT, KC_A) - 短按发送 'a'，长按保持 Shift
#define SHIFT_A        MT(MOD_LSFT, KC_A)

// LT(1, KC_SPACE) - 短按发送空格，长按切换到层1
#define SPACE_LAYER    LT(1, KC_SPACE)
```

### 3. 键位映射

```c
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_jis_113(
        // ...
        KC_LCTL, KC_LGUI, KC_LALT,    KC_SPC,    KC_RALT, KC_RGUI, KC_APP,  KC_RCTL,
        // ...
        LAYER_TAP_KEY, KC_F1, KC_F2,  // 基础层使用 MO(1)
    ),
    [1] = LAYOUT_jis_113(
        // 层1定义
        // ...
    )
};
```

## 注意事项

1. **性能**: 使用静态数组跟踪按键状态，无额外内存开销
2. **线程安全**: 单线程环境，无需考虑并发
3. **状态持久**: 状态在函数调用间保持，直到下次按键
4. **简化逻辑**: 不处理复杂的 tap 中断、多层嵌套等

## 后续扩展

如果需要更复杂的功能，可以扩展：

1. **按键级配置**: 不同键使用不同 `TAPPING_TERM`
2. **多层嵌套**: 支持 LT(layer, key) 嵌套
3. **Tap Dance**: 多次点击触发不同动作
4. **One-shot 修饰键**: 临时修饰键功能

## 调试

启用 `ACTION_DEBUG` 宏来查看调试信息：

```c
#define ACTION_DEBUG
```

这将输出详细的动作处理日志。
