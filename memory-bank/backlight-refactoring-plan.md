# Backlight 背光灯模块重构计划

## 1. 需求概述

### 1.1 功能需求

| 序号 | 需求 | 说明 |
|------|------|------|
| 1 | 支持单色或三色灯 | 通过配置宏切换，硬件为三个独立LED（R/G/B各一个引脚） |
| 2 | 三色单独调节 | RGB三通道可独立设置亮度，组合形成多色 |
| 3 | RGB色调节 | 支持任意RGB值设置，封装固定7色用于组合键切换 |
| 4 | 亮度无极调节 | 0-100%精度，封装3档亮度用于组合键切换 |
| 5 | 代码解耦 | PWM调用hal/pwm.h，不包含存储模块，提供接口给output_service.h |

### 1.2 预设颜色定义

| 颜色 | R | G | B | 说明 |
|------|---|---|---|------|
| 红 (Red) | 100 | 0 | 0 | 纯红 |
| 绿 (Green) | 0 | 100 | 0 | 纯绿 |
| 蓝 (Blue) | 0 | 0 | 100 | 纯蓝 |
| 黄 (Yellow) | 100 | 100 | 0 | R+G |
| 青 (Cyan) | 0 | 100 | 100 | G+B |
| 品红 (Magenta) | 100 | 0 | 100 | R+B |
| 白 (White) | 100 | 100 | 100 | R+G+B |

### 1.3 预设亮度档位

| 档位 | 亮度值 | 说明 |
|------|--------|------|
| 低 (LOW) | 30% | 省电模式 |
| 中 (MEDIUM) | 60% | 正常模式 |
| 高 (HIGH) | 100% | 最亮模式 |

---

## 2. 现有代码分析

### 2.1 代码位置

- `drivers/output/backlight/backlight.h`
- `drivers/output/backlight/backlight.c`

### 2.2 现有实现特点

#### 来源
代码源自 QMK 固件 (Copyright 2013 Mathias Andersson)，遵循 GPL v2 协议。

#### 数据结构

```c
typedef union {
    uint8_t raw;
    struct {
        bool    enable : 1;
        bool    breathing : 1;
        uint8_t reserved : 1;
        uint8_t level : 5;     // 仅支持0-31级
    };
} backlight_config_t;
```

#### 功能列表

| 函数 | 功能 | 问题 |
|------|------|------|
| `backlight_init()` | 初始化 | 直接调用eeprom读取 |
| `backlight_toggle()` | 开关切换 | OK |
| `backlight_enable/disable()` | 启用/禁用 | 会写eeprom |
| `backlight_increase/decrease()` | 亮度增减 | 会写eeprom |
| `backlight_level()` | 设置亮度 | 会写eeprom |
| `backlight_level_noeeprom()` | 设置亮度(不存储) | OK |
| `backlight_set()` | HAL实现(weak) | 未连接hal/pwm.h |

### 2.3 存在的问题

#### 问题1: 仅支持单色
- 只有一个 `level` 值控制亮度
- 没有 RGB 三通道概念
- 无法实现颜色混合

#### 问题2: 存储耦合严重
```c
// backlight.c:18-19
#include "eeprom.h"

// backlight.c:55, 71, 84, 113, 126, 149, 170, 178, 222, 235
eeconfig_read_backlight() / eeconfig_update_backlight()
```
几乎每个操作都直接调用eeprom，违反了解耦需求。

#### 问题3: 未使用HAL PWM接口
```c
// backlight.c:281
__attribute__((weak)) void backlight_set(uint8_t level) {}
```
`backlight_set()` 是空的weak函数，需要平台实现，但没有调用 `hal/pwm.h`。

#### 问题4: 亮度级数限制
- 当前最大31级 (`level : 5` 位域)
- 需求要求100级无极调节

#### 问题5: 呼吸灯实现不完整
- breathing相关函数也是weak函数
- 需要平台单独实现

### 2.4 依赖分析

```
当前依赖:
backlight.c
├── eeprom.h          (存储 - 需移除)
├── debug.h           (调试 - 可选保留)
└── (weak functions)  (无HAL调用)

目标依赖:
backlight.c
├── hal/pwm.h         (硬件抽象)
├── debug.h           (调试 - 可选)
└── backlight_hal.h   (内部HAL抽象)
```

---

## 3. HAL PWM 接口分析

### 3.1 可用接口 (`hal/pwm.h`)

```c
// 核心控制
void pwm_init(void);
void pwm_start(pwm_channel_t channel);
void pwm_stop(pwm_channel_t channel);
void pwm_set_duty_cycle(pwm_channel_t channel, uint8_t duty_cycle);  // 0-100

// 引脚绑定
bool pwm_bind_pin(pin_t pin, pwm_channel_t channel);
bool pwm_unbind_pin(pin_t pin);

// 便捷宏
#define pwm_set_brightness(channel, brightness) pwm_set_duty_cycle((channel), (brightness))
#define pwm_led_on(channel)  pwm_set_level((channel), 1)
#define pwm_led_off(channel) pwm_set_level((channel), 0)
```

### 3.2 通道分配方案

| LED | PWM通道 | 配置宏 |
|-----|---------|--------|
| Red | PWM_CHANNEL_0 | BACKLIGHT_PWM_CHANNEL_R |
| Green | PWM_CHANNEL_1 | BACKLIGHT_PWM_CHANNEL_G |
| Blue | PWM_CHANNEL_2 | BACKLIGHT_PWM_CHANNEL_B |

---

## 4. 参考设计: indicator模块

`drivers/output/indicators/` 模块采用了良好的分层设计：

```
indicator.h         - 公共接口
indicator.c         - 逻辑实现
indicator_hal.h     - HAL抽象层
indicator_config.h  - 配置定义
```

### 4.1 关键设计模式

1. **状态机驱动**: 使用 `ind_phase_t` 状态枚举驱动灯效
2. **HAL抽象**: `ind_hal_set(led_id, bool)` 封装底层操作
3. **预定义效果**: `IND_BLINK_SLOW` 等常量简化调用
4. **回调机制**: `ind_lpm_callback_t` 支持低功耗通知
5. **无存储依赖**: 配置由调用者管理

---

## 5. 重构架构设计

### 5.1 文件结构

```
drivers/output/backlight/
├── backlight.h          # 公共接口 (供output_service.h使用)
├── backlight.c          # 核心逻辑实现
├── backlight_hal.h      # HAL抽象层定义
├── backlight_hal.c      # HAL实现 (调用hal/pwm.h)
└── backlight_config.h   # 配置宏定义
```

### 5.2 数据结构设计

```c
// backlight.h

/* ============ 类型定义 ============ */

/**
 * @brief 背光灯类型
 */
typedef enum {
    BL_TYPE_SINGLE = 0,   /**< 单色灯 */
    BL_TYPE_RGB,          /**< 三色灯 (RGB) */
} bl_type_t;

/**
 * @brief RGB颜色结构
 */
typedef struct {
    uint8_t r;    /**< 红色分量 0-100 */
    uint8_t g;    /**< 绿色分量 0-100 */
    uint8_t b;    /**< 蓝色分量 0-100 */
} bl_rgb_t;

/**
 * @brief 预设颜色枚举
 */
typedef enum {
    BL_COLOR_RED = 0,     /**< 红 */
    BL_COLOR_GREEN,       /**< 绿 */
    BL_COLOR_BLUE,        /**< 蓝 */
    BL_COLOR_YELLOW,      /**< 黄 */
    BL_COLOR_CYAN,        /**< 青 */
    BL_COLOR_MAGENTA,     /**< 品红 */
    BL_COLOR_WHITE,       /**< 白 */
    BL_COLOR_COUNT        /**< 颜色数量 */
} bl_preset_color_t;

/**
 * @brief 预设亮度枚举
 */
typedef enum {
    BL_LEVEL_OFF = 0,     /**< 关闭 */
    BL_LEVEL_LOW,         /**< 低亮度 30% */
    BL_LEVEL_MEDIUM,      /**< 中亮度 60% */
    BL_LEVEL_HIGH,        /**< 高亮度 100% */
    BL_LEVEL_COUNT        /**< 档位数量 */
} bl_preset_level_t;

/**
 * @brief 背光灯状态结构 (用于外部存储)
 */
typedef struct {
    bool     enable;           /**< 是否启用 */
    uint8_t  brightness;       /**< 当前亮度 0-100 */
    bl_rgb_t color;            /**< 当前颜色 (仅RGB模式) */
} bl_state_t;
```

### 5.3 公共接口设计

```c
// backlight.h

/* ============ 初始化 ============ */

/**
 * @brief 初始化背光模块
 * @param state 初始状态指针，NULL则使用默认值
 */
void backlight_init(const bl_state_t* state);

/**
 * @brief 反初始化背光模块
 */
void backlight_deinit(void);

/* ============ 开关控制 ============ */

/**
 * @brief 启用背光
 */
void backlight_enable(void);

/**
 * @brief 禁用背光
 */
void backlight_disable(void);

/**
 * @brief 切换背光开关
 */
void backlight_toggle(void);

/**
 * @brief 获取背光启用状态
 */
bool backlight_is_enabled(void);

/* ============ 亮度控制 ============ */

/**
 * @brief 设置亮度 (无极调节)
 * @param brightness 亮度值 0-100
 */
void backlight_set_brightness(uint8_t brightness);

/**
 * @brief 获取当前亮度
 * @return 亮度值 0-100
 */
uint8_t backlight_get_brightness(void);

/**
 * @brief 亮度增加
 * @param step 增加步进值 (默认10)
 */
void backlight_brightness_increase(uint8_t step);

/**
 * @brief 亮度减少
 * @param step 减少步进值 (默认10)
 */
void backlight_brightness_decrease(uint8_t step);

/**
 * @brief 设置预设亮度档位
 * @param level 预设档位
 */
void backlight_set_preset_level(bl_preset_level_t level);

/**
 * @brief 切换到下一档亮度
 */
void backlight_level_step(void);

/* ============ 颜色控制 (仅RGB模式) ============ */

/**
 * @brief 设置RGB颜色
 * @param rgb 颜色结构指针
 */
void backlight_set_color(const bl_rgb_t* rgb);

/**
 * @brief 设置RGB颜色 (分量形式)
 * @param r 红色分量 0-100
 * @param g 绿色分量 0-100
 * @param b 蓝色分量 0-100
 */
void backlight_set_rgb(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief 获取当前颜色
 * @param rgb 输出颜色结构指针
 */
void backlight_get_color(bl_rgb_t* rgb);

/**
 * @brief 设置单通道亮度 (仅RGB模式)
 * @param channel 通道: 'r', 'g', 'b'
 * @param value 亮度值 0-100
 */
void backlight_set_channel(char channel, uint8_t value);

/**
 * @brief 设置预设颜色
 * @param color 预设颜色枚举
 */
void backlight_set_preset_color(bl_preset_color_t color);

/**
 * @brief 切换到下一个预设颜色
 */
void backlight_color_step(void);

/* ============ 状态管理 ============ */

/**
 * @brief 获取当前状态 (供外部存储使用)
 * @param state 输出状态结构指针
 */
void backlight_get_state(bl_state_t* state);

/**
 * @brief 恢复状态 (从外部存储加载)
 * @param state 状态结构指针
 */
void backlight_restore_state(const bl_state_t* state);

/* ============ 便捷宏 ============ */

#define backlight_on()   backlight_enable()
#define backlight_off()  backlight_disable()
```

### 5.4 HAL抽象层设计

```c
// backlight_hal.h

#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 初始化背光HAL
 */
void bl_hal_init(void);

/**
 * @brief 反初始化背光HAL
 */
void bl_hal_deinit(void);

/**
 * @brief 设置单色灯亮度 (单色模式)
 * @param brightness 亮度 0-100
 */
void bl_hal_set_single(uint8_t brightness);

/**
 * @brief 设置RGB灯亮度 (RGB模式)
 * @param r 红色亮度 0-100
 * @param g 绿色亮度 0-100
 * @param b 蓝色亮度 0-100
 */
void bl_hal_set_rgb(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief 设置单通道亮度
 * @param channel PWM通道
 * @param brightness 亮度 0-100
 */
void bl_hal_set_channel(uint8_t channel, uint8_t brightness);
```

### 5.5 配置宏设计

```c
// backlight_config.h

#pragma once

/* ============ 背光类型配置 ============ */

// 在 product_config.h 中定义以下宏之一:
// #define BACKLIGHT_TYPE_SINGLE   // 单色灯模式
// #define BACKLIGHT_TYPE_RGB      // RGB三色灯模式

#ifndef BACKLIGHT_TYPE_SINGLE
#ifndef BACKLIGHT_TYPE_RGB
#define BACKLIGHT_TYPE_SINGLE      // 默认单色模式
#endif
#endif

/* ============ 引脚配置 ============ */

// 单色灯引脚 (在product_config.h中定义)
#ifndef BACKLIGHT_PIN
#define BACKLIGHT_PIN NO_PIN
#endif

// RGB灯引脚 (在product_config.h中定义)
#ifndef BACKLIGHT_PIN_R
#define BACKLIGHT_PIN_R NO_PIN
#endif

#ifndef BACKLIGHT_PIN_G
#define BACKLIGHT_PIN_G NO_PIN
#endif

#ifndef BACKLIGHT_PIN_B
#define BACKLIGHT_PIN_B NO_PIN
#endif

/* ============ PWM通道配置 ============ */

#ifndef BACKLIGHT_PWM_CHANNEL
#define BACKLIGHT_PWM_CHANNEL PWM_CHANNEL_0
#endif

#ifndef BACKLIGHT_PWM_CHANNEL_R
#define BACKLIGHT_PWM_CHANNEL_R PWM_CHANNEL_0
#endif

#ifndef BACKLIGHT_PWM_CHANNEL_G
#define BACKLIGHT_PWM_CHANNEL_G PWM_CHANNEL_1
#endif

#ifndef BACKLIGHT_PWM_CHANNEL_B
#define BACKLIGHT_PWM_CHANNEL_B PWM_CHANNEL_2
#endif

/* ============ 默认值配置 ============ */

#ifndef BACKLIGHT_DEFAULT_BRIGHTNESS
#define BACKLIGHT_DEFAULT_BRIGHTNESS 100
#endif

#ifndef BACKLIGHT_DEFAULT_ON
#define BACKLIGHT_DEFAULT_ON true
#endif

#ifndef BACKLIGHT_STEP
#define BACKLIGHT_STEP 10          // 亮度步进值
#endif

/* ============ 预设亮度值 ============ */

#ifndef BACKLIGHT_LEVEL_LOW
#define BACKLIGHT_LEVEL_LOW 30
#endif

#ifndef BACKLIGHT_LEVEL_MEDIUM
#define BACKLIGHT_LEVEL_MEDIUM 60
#endif

#ifndef BACKLIGHT_LEVEL_HIGH
#define BACKLIGHT_LEVEL_HIGH 100
#endif

/* ============ LED极性配置 ============ */

#ifndef BACKLIGHT_ON_STATE
#define BACKLIGHT_ON_STATE 1       // 1=高电平点亮, 0=低电平点亮
#endif
```

---

## 6. output_service.h 集成

### 6.1 事件定义 (已存在)

```c
// output_service.h
typedef enum {
    OUTPUT_BACKLIGHT_BRIGHTNESS_EVT = 1 << 1,  /**< 背光灯亮度事件 */
    OUTPUT_BACKLIGHT_COLOR_EVT      = 1 << 2,  /**< 背光灯颜色事件 */
    // ...
} output_task_event_t;
```

### 6.2 事件处理建议

```c
// output_service.c 中的事件处理
if (events & OUTPUT_BACKLIGHT_BRIGHTNESS_EVT) {
    // 由上层决定具体操作，如:
    // backlight_level_step();
    // 或 backlight_set_brightness(new_value);
    events ^= OUTPUT_BACKLIGHT_BRIGHTNESS_EVT;
}

if (events & OUTPUT_BACKLIGHT_COLOR_EVT) {
    // 由上层决定具体操作，如:
    // backlight_color_step();
    // 或 backlight_set_preset_color(new_color);
    events ^= OUTPUT_BACKLIGHT_COLOR_EVT;
}
```

---

## 7. 实现计划

### 7.1 阶段一: 创建框架文件

| 任务 | 文件 | 说明 |
|------|------|------|
| 1.1 | `backlight_config.h` | 创建配置宏定义 |
| 1.2 | `backlight_hal.h` | 创建HAL接口定义 |
| 1.3 | `backlight.h` | 重写公共接口定义 |

### 7.2 阶段二: 实现HAL层

| 任务 | 文件 | 说明 |
|------|------|------|
| 2.1 | `backlight_hal.c` | 实现HAL层，调用hal/pwm.h |
| 2.2 | 测试 | 验证PWM控制功能 |

### 7.3 阶段三: 实现核心逻辑

| 任务 | 文件 | 说明 |
|------|------|------|
| 3.1 | `backlight.c` | 重写核心逻辑，移除eeprom依赖 |
| 3.2 | 预设颜色 | 实现7色切换 |
| 3.3 | 预设亮度 | 实现3档切换 |
| 3.4 | 单通道控制 | 实现R/G/B单独调节 |

### 7.4 阶段四: 集成测试

| 任务 | 说明 |
|------|------|
| 4.1 | 更新product_config.h添加背光配置 |
| 4.2 | 更新output_service.c集成背光控制 |
| 4.3 | 编写单元测试 |
| 4.4 | 硬件验证测试 |

---

## 8. API对照表

### 8.1 旧接口 → 新接口映射

| 旧接口 | 新接口 | 说明 |
|--------|--------|------|
| `backlight_init()` | `backlight_init(state)` | 新增初始状态参数 |
| `backlight_toggle()` | `backlight_toggle()` | 保持 |
| `backlight_enable()` | `backlight_enable()` | 移除eeprom写入 |
| `backlight_disable()` | `backlight_disable()` | 移除eeprom写入 |
| `backlight_increase()` | `backlight_brightness_increase(step)` | 新增步进参数 |
| `backlight_decrease()` | `backlight_brightness_decrease(step)` | 新增步进参数 |
| `backlight_level(level)` | `backlight_set_brightness(brightness)` | 范围从0-31改为0-100 |
| `backlight_level_noeeprom()` | 移除 | 新接口默认不写eeprom |
| `backlight_step()` | `backlight_level_step()` | 重命名 |
| `get_backlight_level()` | `backlight_get_brightness()` | 重命名 |
| `is_backlight_enabled()` | `backlight_is_enabled()` | 重命名 |
| N/A | `backlight_set_color(rgb)` | 新增 |
| N/A | `backlight_set_rgb(r,g,b)` | 新增 |
| N/A | `backlight_set_preset_color(color)` | 新增 |
| N/A | `backlight_color_step()` | 新增 |
| N/A | `backlight_get_state(state)` | 新增 |
| N/A | `backlight_restore_state(state)` | 新增 |

### 8.2 废弃接口

| 接口 | 原因 |
|------|------|
| `eeconfig_read_backlight()` | 存储解耦，由上层管理 |
| `eeconfig_update_backlight()` | 存储解耦，由上层管理 |
| `eeconfig_update_backlight_current()` | 存储解耦，由上层管理 |
| `eeconfig_update_backlight_default()` | 存储解耦，由上层管理 |
| 所有breathing相关接口 | 暂不实现，后续可扩展 |

---

## 9. 注意事项

### 9.1 向后兼容

- 提供 `BACKLIGHT_LEGACY_API` 宏，启用后可使用旧接口名称
- 旧接口通过宏映射到新接口

### 9.2 存储方案

存储由上层（如config_service）管理:

```c
// 示例: config_service.c
void config_save_backlight(void) {
    bl_state_t state;
    backlight_get_state(&state);
    storage_write(BACKLIGHT_CONFIG_ADDR, &state, sizeof(state));
}

void config_load_backlight(void) {
    bl_state_t state;
    storage_read(BACKLIGHT_CONFIG_ADDR, &state, sizeof(state));
    backlight_restore_state(&state);
}
```

### 9.3 低功耗考虑

- 背光关闭时应停止PWM输出以节省功耗
- 可添加空闲超时自动关闭功能（由上层实现）

### 9.4 PWM频率建议

- LED背光建议PWM频率: 1kHz - 10kHz
- 避免低频闪烁影响视觉

---

## 10. 时序图

### 10.1 初始化流程

```
┌──────────────────┐     ┌──────────────────┐     ┌──────────────────┐
│   Application    │     │    backlight.c   │     │  backlight_hal.c │
└────────┬─────────┘     └────────┬─────────┘     └────────┬─────────┘
         │                        │                        │
         │ backlight_init(state)  │                        │
         │───────────────────────>│                        │
         │                        │ bl_hal_init()          │
         │                        │───────────────────────>│
         │                        │                        │ pwm_init()
         │                        │                        │ pwm_bind_pin()
         │                        │       OK               │
         │                        │<───────────────────────│
         │                        │ restore state          │
         │                        │ apply brightness/color │
         │       OK               │                        │
         │<───────────────────────│                        │
```

### 10.2 颜色设置流程

```
┌──────────────────┐     ┌──────────────────┐     ┌──────────────────┐
│   Key Handler    │     │    backlight.c   │     │  backlight_hal.c │
└────────┬─────────┘     └────────┬─────────┘     └────────┬─────────┘
         │                        │                        │
         │ backlight_set_preset_  │                        │
         │ color(BL_COLOR_YELLOW) │                        │
         │───────────────────────>│                        │
         │                        │ lookup preset RGB      │
         │                        │ (100, 100, 0)          │
         │                        │                        │
         │                        │ bl_hal_set_rgb(100,100,0)
         │                        │───────────────────────>│
         │                        │                        │ pwm_set_duty_cycle(R, 100)
         │                        │                        │ pwm_set_duty_cycle(G, 100)
         │                        │                        │ pwm_set_duty_cycle(B, 0)
         │                        │       OK               │
         │                        │<───────────────────────│
         │       OK               │                        │
         │<───────────────────────│                        │
```

---

## 11. 总结

本重构计划将现有的QMK单色背光驱动改造为支持单色/RGB双模式的独立背光模块，主要改进：

1. **解耦存储**: 移除所有eeprom直接调用，状态管理由上层负责
2. **HAL抽象**: 通过backlight_hal.c调用hal/pwm.h，实现平台无关
3. **RGB支持**: 新增三色独立控制和颜色混合
4. **预设封装**: 提供7色和3档快捷接口，简化组合键实现
5. **接口清晰**: 为output_service.h提供简洁的控制接口

实现后，背光模块将符合框架的分层架构设计，与indicator模块风格一致，易于维护和扩展。
