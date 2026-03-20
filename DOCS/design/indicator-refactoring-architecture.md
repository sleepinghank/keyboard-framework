# 指示灯驱动架构优化设计方案（简化版）

## 文档信息

- **版本**: v2.0
- **日期**: 2025-01-10
- **作者**: Claude Code
- **状态**: 架构设计阶段

---

## 一、设计目标

### 1.1 核心原则

1. **分层简化**：仅三层架构（硬件层、驱动层、应用层）
2. **完全重构**：不考虑旧代码兼容
3. **参数驱动**：通过参数配置实现各种灯效，不预设业务场景
4. **GPIO控制**：仅支持GPIO控制LED，暂不支持RGB/Matrix

### 1.2 文件结构

```
drivers/output/indicators/
├── indicator_hal.h      # 硬件抽象层接口
├── indicator_hal.c      # GPIO硬件操作实现
├── indicator_driver.h   # 驱动层接口（状态机+定时器）
├── indicator_driver.c   # 驱动层实现
├── indicator.h          # 应用层API接口
└── indicator.c          # 应用层API实现
```

---

## 二、架构设计

### 2.1 分层架构

```
┌─────────────────────────────────────────────────────────────┐
│                   应用层 (indicator.c/h)                     │
│  - 对外API接口                                               │
│  - LED实例管理                                               │
│  - 参数配置接口                                              │
└─────────────────────────────────────────────────────────────┘
                              ▲
                              │
┌─────────────────────────────────────────────────────────────┐
│                  驱动层 (indicator_driver.c/h)                │
│  - 状态机引擎                                                │
│  - 定时器管理                                                │
│  - 灯效处理逻辑                                              │
└─────────────────────────────────────────────────────────────┘
                              ▲
                              │
┌─────────────────────────────────────────────────────────────┐
│               硬件层 (indicator_hal.c/h)                      │
│  - GPIO操作封装                                              │
│  - 平台相关代码                                              │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 数据流向

```
应用调用
    │
    ▼
indicator_start() ──→ 设置配置参数
    │
    ▼
驱动层状态机 ──→ 定时器调度 ──→ 灯效计算
    │
    ▼
硬件层 ──→ GPIO控制 ──→ LED亮/灭
```

---

## 三、数据结构设计

### 3.1 核心类型定义（indicator.h）

```c
/**
 * @file indicator.h
 * @brief 指示灯应用层接口
 */

#pragma once
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief LED模式
 */
typedef enum {
    IND_MODE_OFF     = 0,    // 熄灭
    IND_MODE_ON,             // 常亮
    IND_MODE_BLINK,          // 闪烁
    IND_MODE_HEARTBEAT,      // 心跳（短亮短灭，周期性）
} ind_mode_t;

/**
 * @brief LED状态（内部使用）
 */
typedef enum {
    IND_STATE_IDLE     = 0,  // 空闲
    IND_STATE_RUNNING,       // 运行中
} ind_state_t;

/**
 * @brief LED配置参数
 */
typedef struct {
    ind_mode_t mode;         // 模式
    uint32_t on_time;        // 亮持续时间 (ms)
    uint32_t off_time;       // 灭持续时间 (ms)
    uint32_t duration;       // 总持续时间 (ms), 0=无限
    uint16_t repeat;         // 重复次数, 0=无限
} ind_config_t;

/**
 * @brief LED实例描述符
 */
typedef struct {
    // 硬件配置
    struct {
        pin_t  pin;          // GPIO引脚
        bool   active_high;  // true=高电平亮, false=低电平亮
    } hw;

    // 运行时状态（驱动层使用）
    struct {
        ind_state_t state;    // 当前状态
        uint32_t   timer;     // 定时器
        uint32_t   elapsed;   // 已运行时间
        uint16_t   count;     // 重复计数
        bool       is_on;     // 当前是否点亮
    } runtime;

    // 当前配置
    ind_config_t config;
} indicator_t;
```

### 3.2 硬件层接口（indicator_hal.h）

```c
/**
 * @file indicator_hal.h
 * @brief 指示灯硬件抽象层
 */

#pragma once
#include "gpio.h"
#include <stdbool.h>

/**
 * @brief LED硬件配置
 */
typedef struct {
    pin_t pin;            // GPIO引脚
    bool  active_high;    // true=高电平亮, false=低电平亮
} ind_hal_config_t;

/**
 * @brief 初始化LED硬件
 * @param cfg 硬件配置
 */
void ind_hal_init(const ind_hal_config_t* cfg);

/**
 * @brief 设置LED状态
 * @param pin GPIO引脚
 * @param is_on true=点亮, false=熄灭
 */
void ind_hal_set(pin_t pin, bool is_on);

/**
 * @brief 反初始化LED硬件
 * @param pin GPIO引脚
 */
void ind_hal_deinit(pin_t pin);
```

### 3.3 驱动层接口（indicator_driver.h）

```c
/**
 * @file indicator_driver.h
 * @brief 指示灯驱动层
 */

#pragma once
#include "indicator.h"

/**
 * @brief 驱动层初始化
 */
void ind_driver_init(void);

/**
 * @brief 启动LED指示
 * @param ind LED实例
 * @param config 配置参数
 */
void ind_driver_start(indicator_t* ind, const ind_config_t* config);

/**
 * @brief 停止LED指示
 * @param ind LED实例
 */
void ind_driver_stop(indicator_t* ind);

/**
 * @brief LED任务函数（主循环调用）
 * @param ind LED实例
 * @return true=LED仍在运行, false=LED已完成
 */
bool ind_driver_task(indicator_t* ind);
```

---

## 四、应用层API设计

### 4.1 初始化和控制API

```c
/**
 * @file indicator.h (续)
 * @brief 应用层API接口
 */

/* ========== 初始化 ========== */

/**
 * @brief 初始化LED实例
 * @param ind LED实例
 * @param pin GPIO引脚
 * @param active_high true=高电平亮, false=低电平亮
 * @return 0=成功, <0=失败
 */
int indicator_init(indicator_t* ind, pin_t pin, bool active_high);

/**
 * @brief 反初始化LED实例
 * @param ind LED实例
 */
void indicator_deinit(indicator_t* ind);

/* ========== 控制 ========== */

/**
 * @brief 启动LED指示
 * @param ind LED实例
 * @param config 配置参数
 * @return 0=成功, <0=失败
 */
int indicator_start(indicator_t* ind, const ind_config_t* config);

/**
 * @brief 停止LED指示
 * @param ind LED实例
 */
void indicator_stop(indicator_t* ind);

/**
 * @brief 更新配置
 * @param ind LED实例
 * @param config 新配置
 * @return 0=成功, <0=失败
 */
int indicator_update(indicator_t* ind, const ind_config_t* config);

/* ========== 状态查询 ========== */

/**
 * @brief 检查LED是否正在运行
 * @param ind LED实例
 * @return true=运行中, false=空闲
 */
bool indicator_is_running(const indicator_t* ind);

/* ========== 任务 ========== */

/**
 * @brief LED任务函数（在主循环中调用）
 * @param ind LED实例
 * @return true=LED仍在运行, false=LED已完成
 */
bool indicator_task(indicator_t* ind);
```

---

## 五、驱动层实现详解

### 5.1 状态机设计

```
         ┌──────────────┐
         │    IDLE      │
         └───────┬──────┘
                 │ start()
                 ▼
         ┌──────────────┐
         │   RUNNING    │�────────┐
         └───────┬──────┘         │
                 │                │
                 │ timeout/       │ repeat
                 │ duration       │
                 ▼                │
         ┌──────────────┐         │
         │    IDLE      │◀────────┘
         └──────────────┘
```

### 5.2 模式处理逻辑

```c
/**
 * @file indicator_driver.c
 * @brief 驱动层实现
 */

#include "indicator_driver.h"
#include "indicator_hal.h"
#include "timer.h"

// 模式处理函数类型
typedef void (*mode_handler_t)(indicator_t* ind, uint32_t delta);

/* ========== OFF模式：熄灭 ========== */
static void mode_off_handler(indicator_t* ind, uint32_t delta) {
    ind_hal_set(ind->hw.pin, false);
    ind->runtime.is_on = false;
}

/* ========== ON模式：常亮 ========== */
static void mode_on_handler(indicator_t* ind, uint32_t delta) {
    ind_hal_set(ind->hw.pin, true);
    ind->runtime.is_on = true;

    // 检查持续时间
    if (ind->config.duration > 0) {
        if (ind->runtime.elapsed >= ind->config.duration) {
            ind_driver_stop(ind);  // 时间到，停止
        }
    }
}

/* ========== BLINK模式：闪烁 ========== */
static void mode_blink_handler(indicator_t* ind, uint32_t delta) {
    uint32_t period = ind->runtime.is_on ? ind->config.off_time : ind->config.on_time;

    if (ind->runtime.timer >= period) {
        // 切换状态
        ind->runtime.is_on = !ind->runtime.is_on;
        ind_hal_set(ind->hw.pin, ind->runtime.is_on);
        ind->runtime.timer = 0;

        // 如果从亮切换到灭，计数一次
        if (!ind->runtime.is_on) {
            ind->runtime.count++;
            if (ind->config.repeat > 0 && ind->runtime.count >= ind->config.repeat) {
                ind_driver_stop(ind);
            }
        }
    }
}

/* ========== HEARTBEAT模式：心跳 ========== */
static void mode_heartbeat_handler(indicator_t* ind, uint32_t delta) {
    // 心跳：短亮 - 短灭 - 长灭
    // 使用 on_time 作为亮的时间
    // off_time 作为短灭时间
    // 短灭后自动亮起，周期由 duration 控制

    static const uint32_t HEARTBEAT_LONG_OFF = 1000;  // 长灭时间

    if (ind->runtime.is_on) {
        // 亮状态
        if (ind->runtime.timer >= ind->config.on_time) {
            ind->runtime.is_on = false;
            ind_hal_set(ind->hw.pin, false);
            ind->runtime.timer = 0;
            ind->runtime.count++;  // 记录心跳次数
        }
    } else {
        // 灭状态
        if (ind->runtime.timer >= ind->config.off_time) {
            // 完成一次短灭，检查是否需要长灭
            if (ind->config.repeat > 0 && ind->runtime.count >= ind->config.repeat) {
                // 完成所有心跳
                ind_driver_stop(ind);
            } else {
                // 下一次心跳开始
                ind->runtime.is_on = true;
                ind_hal_set(ind->hw.pin, true);
                ind->runtime.timer = 0;
            }
        }
    }
}

/* ========== 模式处理表 ========== */
static const mode_handler_t mode_handlers[] = {
    [IND_MODE_OFF]      = mode_off_handler,
    [IND_MODE_ON]       = mode_on_handler,
    [IND_MODE_BLINK]    = mode_blink_handler,
    [IND_MODE_HEARTBEAT]= mode_heartbeat_handler,
};

/* ========== 驱动层实现 ========== */

void ind_driver_init(void) {
    // 初始化硬件层（如需要）
}

void ind_driver_start(indicator_t* ind, const ind_config_t* config) {
    if (ind == NULL || config == NULL) return;

    // 保存配置
    ind->config = *config;

    // 重置状态
    ind->runtime.state   = IND_STATE_RUNNING;
    ind->runtime.timer   = 0;
    ind->runtime.elapsed = 0;
    ind->runtime.count   = 0;
    ind->runtime.is_on   = false;

    // 立即执行一次模式处理
    if (mode_handlers[config->mode]) {
        mode_handlers[config->mode](ind, 0);
    }

    // 记录起始时间
    ind->runtime.timer = timer_read32();
}

void ind_driver_stop(indicator_t* ind) {
    if (ind == NULL) return;

    ind->runtime.state = IND_STATE_IDLE;
    ind_hal_set(ind->hw.pin, false);  // 确保LED熄灭
    ind->runtime.is_on = false;
}

bool ind_driver_task(indicator_t* ind) {
    if (ind == NULL || ind->runtime.state != IND_STATE_RUNNING) {
        return false;
    }

    // 计算时间差
    uint32_t now = timer_read32();
    uint32_t delta = timer_elapsed32(ind->runtime.timer);
    ind->runtime.timer = now;
    ind->runtime.elapsed += delta;

    // 调用模式处理函数
    if (mode_handlers[ind->config.mode]) {
        mode_handlers[ind->config.mode](ind, delta);
    }

    // 检查总持续时间
    if (ind->config.duration > 0 && ind->runtime.elapsed >= ind->config.duration) {
        ind_driver_stop(ind);
        return false;
    }

    return (ind->runtime.state == IND_STATE_RUNNING);
}
```

---

## 六、应用层实现

### 6.1 应用层代码（indicator.c）

```c
/**
 * @file indicator.c
 * @brief 应用层实现
 */

#include "indicator.h"
#include "indicator_hal.h"
#include "indicator_driver.h"
#include <string.h>

/* ========== 初始化 ========== */

int indicator_init(indicator_t* ind, pin_t pin, bool active_high) {
    if (ind == NULL) return -1;

    // 清零结构
    memset(ind, 0, sizeof(indicator_t));

    // 硬件配置
    ind->hw.pin         = pin;
    ind->hw.active_high = active_high;

    // 初始化硬件
    ind_hal_config_t hal_cfg = { .pin = pin, .active_high = active_high };
    ind_hal_init(&hal_cfg);

    // 默认配置
    ind->config.mode     = IND_MODE_OFF;
    ind->config.on_time  = 0;
    ind->config.off_time = 0;
    ind->config.duration = 0;
    ind->config.repeat   = 0;

    return 0;
}

void indicator_deinit(indicator_t* ind) {
    if (ind == NULL) return;

    // 停止指示
    indicator_stop(ind);

    // 反初始化硬件
    ind_hal_deinit(ind->hw.pin);
}

/* ========== 控制 ========== */

int indicator_start(indicator_t* ind, const ind_config_t* config) {
    if (ind == NULL || config == NULL) return -1;

    ind_driver_start(ind, config);
    return 0;
}

void indicator_stop(indicator_t* ind) {
    if (ind == NULL) return;

    ind_driver_stop(ind);
}

int indicator_update(indicator_t* ind, const ind_config_t* config) {
    if (ind == NULL || config == NULL) return -1;

    // 停止当前指示
    ind_driver_stop(ind);

    // 启动新配置
    ind_driver_start(ind, config);

    return 0;
}

/* ========== 状态查询 ========== */

bool indicator_is_running(const indicator_t* ind) {
    if (ind == NULL) return false;
    return (ind->runtime.state == IND_STATE_RUNNING);
}

/* ========== 任务 ========== */

bool indicator_task(indicator_t* ind) {
    if (ind == NULL) return false;
    return ind_driver_task(ind);
}
```

---

## 七、硬件层实现

### 7.1 硬件层代码（indicator_hal.c）

```c
/**
 * @file indicator_hal.c
 * @brief 硬件抽象层实现
 */

#include "indicator_hal.h"
#include "gpio.h"
#include "timer.h"

void ind_hal_init(const ind_hal_config_t* cfg) {
    if (cfg == NULL) return;

    // 设置GPIO为输出
    gpio_set_pin_output(cfg->pin);

    // 初始状态：熄灭
    gpio_write_pin(cfg->pin, cfg->active_high ? 0 : 1);
}

void ind_hal_set(pin_t pin, bool is_on) {
    // 需要根据 active_high 决定电平
    // 这里简化处理，实际需要保存active_high状态
    // 可以通过在indicator_t中保存
    gpio_write_pin(pin, is_on ? 1 : 0);
}

void ind_hal_deinit(pin_t pin) {
    // 设置为输入（低功耗）
    gpio_set_pin_input(pin);
}
```

---

## 八、使用示例

### 8.1 基础使用

```c
/* 示例：定义LED实例 */
indicator_t bt_led;
indicator_t p24g_led;
indicator_t battery_led;

void setup(void) {
    // 初始化LED
    // BT LED: PIN_A1, 高电平亮
    indicator_init(&bt_led, PIN_A1, true);

    // 2.4G LED: PIN_A2, 高电平亮
    indicator_init(&p24g_led, PIN_A2, true);

    // 电池LED: PIN_A3, 低电平亮
    indicator_init(&battery_led, PIN_A3, false);
}

void loop(void) {
    // 处理LED任务
    indicator_task(&bt_led);
    indicator_task(&p24g_led);
    indicator_task(&battery_led);

    // ... 其他任务
}
```

### 8.2 各种灯效配置

```c
/* ========== 已连接：常亮2秒 ========== */
void set_connected(indicator_t* ind) {
    ind_config_t config = {
        .mode     = IND_MODE_ON,
        .on_time  = 0,
        .off_time = 0,
        .duration = 2000,  // 亮2秒
        .repeat   = 0,
    };
    indicator_start(ind, &config);
}

/* ========== 配对中：闪烁 ========== */
void set_pairing(indicator_t* ind) {
    ind_config_t config = {
        .mode     = IND_MODE_BLINK,
        .on_time  = 500,   // 亮500ms
        .off_time = 500,   // 灭500ms
        .duration = 0,     // 无限
        .repeat   = 0,
    };
    indicator_start(ind, &config);
}

/* ========== 重连中：快闪 ========== */
void set_reconnecting(indicator_t* ind) {
    ind_config_t config = {
        .mode     = IND_MODE_BLINK,
        .on_time  = 100,   // 亮100ms
        .off_time = 100,   // 灭100ms
        .duration = 0,     // 无限
        .repeat   = 0,
    };
    indicator_start(ind, &config);
}

/* ========== 已断开：灭 ========== */
void set_disconnected(indicator_t* ind) {
    ind_config_t config = {
        .mode     = IND_MODE_OFF,
        .on_time  = 0,
        .off_time = 0,
        .duration = 0,
        .repeat   = 0,
    };
    indicator_start(ind, &config);
}

/* ========== 低电量：心跳 ========== */
void set_battery_low(indicator_t* ind) {
    ind_config_t config = {
        .mode     = IND_MODE_HEARTBEAT,
        .on_time  = 200,   // 亮200ms
        .off_time = 200,   // 灭200ms（短灭）
        .duration = 0,     // 无限
        .repeat   = 0,
    };
    indicator_start(ind, &config);
}

/* ========== 充电中：闪烁3次 ========== */
void set_charging(indicator_t* ind) {
    ind_config_t config = {
        .mode     = IND_MODE_BLINK,
        .on_time  = 500,
        .off_time = 500,
        .duration = 0,
        .repeat   = 3,     // 闪烁3次后停止
    };
    indicator_start(ind, &config);
}
```

### 8.3 无线状态指示示例

```c
/* 无线状态指示 */
void wireless_state_indicator(wt_state_t state, uint8_t host_idx) {
    indicator_t* led = get_led_by_transport();  // 根据传输模式获取LED

    switch (state) {
        case WT_CONNECTED:
            set_connected(led);
            break;
        case WT_DISCONNECTED:
            set_disconnected(led);
            break;
        case WT_PARING:
            set_pairing(led);
            break;
        case WT_RECONNECTING:
            set_reconnecting(led);
            break;
        default:
            break;
    }
}
```

---

## 九、配置参数规范

### 9.1 参数说明

| 参数 | 类型 | 说明 |
|------|------|------|
| `mode` | ind_mode_t | LED模式：OFF/ON/BLINK/HEARTBEAT |
| `on_time` | uint32_t | 亮持续时间（ms），仅在BLINK/HEARTBEAT模式有效 |
| `off_time` | uint32_t | 灭持续时间（ms），仅在BLINK/HEARTBEAT模式有效 |
| `duration` | uint32_t | 总持续时间（ms），0=无限 |
| `repeat` | uint16_t | 重复次数，0=无限 |

### 9.2 模式详解

| 模式 | on_time | off_time | duration | repeat | 效果 |
|------|---------|----------|----------|--------|------|
| OFF | - | - | - | - | 熄灭 |
| ON | - | - | 持续时间 | - | 常亮指定时间 |
| BLINK | 亮时间 | 灭时间 | 总时间 | 闪烁次数 | 闪烁 |
| HEARTBEAT | 亮时间 | 短灭时间 | - | 心跳次数 | 心跳效果 |

### 9.3 常用配置参考

```c
// 常亮
#define IND_ON_FOREVER     {IND_MODE_ON, 0, 0, 0, 0}
#define IND_ON_2S          {IND_MODE_ON, 0, 0, 2000, 0}

// 闪烁
#define IND_BLINK_SLOW     {IND_MODE_BLINK, 500, 500, 0, 0}
#define IND_BLINK_FAST     {IND_MODE_BLINK, 100, 100, 0, 0}
#define IND_BLINK_3_TIMES  {IND_MODE_BLINK, 500, 500, 0, 3}

// 心跳
#define IND_HEARTBEAT      {IND_MODE_HEARTBEAT, 200, 200, 0, 0}
```

---

## 十、与业务层解耦

### 10.1 业务层调用方式

```c
/* 业务层代码示例（application/service/xxx_service.c） */

#include "indicator.h"

// 定义LED实例（业务层管理）
static indicator_t bt_led;
static indicator_t p24g_led;

// 业务层初始化
void my_service_init(void) {
    indicator_init(&bt_led, PIN_A1, true);
    indicator_init(&p24g_led, PIN_A2, true);
}

// 业务层任务
void my_service_task(void) {
    indicator_task(&bt_led);
    indicator_task(&p24g_led);
}

// 业务事件处理
void on_wireless_connected(void) {
    ind_config_t cfg = {IND_MODE_ON, 0, 0, 2000, 0};
    indicator_start(&bt_led, &cfg);
}

void on_wireless_pairing(void) {
    ind_config_t cfg = {IND_MODE_BLINK, 500, 500, 0, 0};
    indicator_start(&bt_led, &cfg);
}
```

### 10.2 解耦优势

1. **指示灯驱动不依赖任何业务模块**
2. **业务层通过配置参数控制LED行为**
3. **新增业务场景只需配置新参数**
4. **指示灯驱动可独立测试**

---

## 十一、实现检查清单

### 11.1 硬件层 (indicator_hal.c/h)

- [ ] `ind_hal_init()` - GPIO初始化
- [ ] `ind_hal_set()` - 设置LED状态
- [ ] `ind_hal_deinit()` - GPIO反初始化

### 11.2 驱动层 (indicator_driver.c/h)

- [ ] `ind_driver_init()` - 驱动初始化
- [ ] `ind_driver_start()` - 启动指示
- [ ] `ind_driver_stop()` - 停止指示
- [ ] `ind_driver_task()` - 任务处理
- [ ] 模式处理函数（OFF/ON/BLINK/HEARTBEAT）

### 11.3 应用层 (indicator.c/h)

- [ ] `indicator_init()` - 初始化LED实例
- [ ] `indicator_deinit()` - 反初始化
- [ ] `indicator_start()` - 启动指示
- [ ] `indicator_stop()` - 停止指示
- [ ] `indicator_update()` - 更新配置
- [ ] `indicator_is_running()` - 状态查询
- [ ] `indicator_task()` - 任务函数

---

## 十二、总结

### 12.1 设计特点

| 特点 | 说明 |
|------|------|
| **简化分层** | 仅3层：硬件层、驱动层、应用层 |
| **文件精简** | 3个.c文件 + 3个.h文件 |
| **完全重构** | 不考虑旧代码兼容 |
| **参数驱动** | 通过配置参数实现各种灯效 |
| **GPIO控制** | 仅支持GPIO，暂不支持RGB/Matrix |
| **业务解耦** | 不预设业务场景，完全由参数控制 |

### 12.2 代码量估算

| 文件 | 预估代码行数 |
|------|-------------|
| indicator_hal.c | ~50行 |
| indicator_hal.h | ~20行 |
| indicator_driver.c | ~150行 |
| indicator_driver.h | ~30行 |
| indicator.c | ~80行 |
| indicator.h | ~80行 |
| **总计** | **~410行** |

### 12.3 后续扩展

如需支持RGB/Matrix，可按以下方式扩展：

1. 在 `indicator_config_t` 中添加 `color` 字段
2. 在 `indicator_hal.h` 中添加 `ind_hal_set_color()` 接口
3. 在 `indicator_hal.c` 中实现RGB/Matrix的硬件操作

**文档结束**
