# Touchpad 组件集成设计

## 概述

将 `component/touch_component` 触控板模块重构为使用框架 HAL 层，直接调用框架接口替代 PixArt 2860 平台特定 API。

## 架构

```
┌─────────────────────────────────────────────────┐
│  组件层 (Component - touch_component)           │
│  直接调用框架 HAL 和 Driver 接口                 │
└─────────────────────────────────────────────────┘
          ↓              ↓              ↓
┌──────────────┐  ┌──────────────┐  ┌──────────────┐
│  HAL 层      │  │  Driver 层   │  │  事件系统    │
│  gpio.h      │  │  bt_driver.h │  │  event_mgr.h │
│  i2c_master.h│  │              │  │              │
│  wait.h      │  │              │  │              │
└──────────────┘  └──────────────┘  └──────────────┘
```

## 需要扩展的接口

### 1. GPIO 中断接口 (hal/gpio.h)

```c
/* GPIO 中断触发类型 */
typedef enum {
    GPIO_INT_FALLING = 0,  // 下降沿触发
    GPIO_INT_RISING,       // 上升沿触发
    GPIO_INT_LOW_LEVEL,    // 低电平触发
    GPIO_INT_HIGH_LEVEL    // 高电平触发
} gpio_int_trigger_t;

/* GPIO 中断回调类型 */
typedef void (*gpio_int_callback_t)(void);

/* 启用 GPIO 中断 */
void gpio_enable_interrupt(pin_t pin, gpio_int_trigger_t trigger, gpio_int_callback_t callback);

/* 禁用 GPIO 中断 */
void gpio_disable_interrupt(pin_t pin);
```

### 2. PTP 报告接口 (bt_driver.h)

```c
/**
 * @brief 发送 PTP (Precision Touchpad) 报告
 * @param report - PTP 报告数据
 * @param len - 报告长度
 * @return uint8_t - 发送状态
 */
uint8_t bt_driver_send_ptp(uint8_t *report, uint8_t len);
```

## 代码改动清单

### pct1336_driver.c

| 原代码 | 替换为 |
|-------|--------|
| `#include "pxi_par2860_ble_lib.h"` | 移除 |
| `#include "timer.h"` | `#include "event_manager.h"` |
| - | `#include "wait.h"` |
| `pxi_delay_us(n)` | `wait_us(n)` |
| `pxi_delay_ms(n)` | `wait_ms(n)` |
| `u_timer_enable(U_TIMER0, cb, 10, 1)` | `OSAL_StartReloadTask(touch_taskID, TOUCH_REG_POLL_EVT, 10)` |
| `u_timer_disable(U_TIMER0)` | `OSAL_StopTask(touch_taskID, TOUCH_REG_POLL_EVT)` |

### touchpad_service.c

| 原代码 | 替换为 |
|-------|--------|
| `#include "config.h"` | `#include "product_config.h"` |
| - | `#include "gpio.h"` |
| - | `#include "event_manager.h"` |
| - | `#include "atomic_util.h"` |
| - | `#include "bt_driver.h"` |
| `DEBUG_log(...)` | `dprintf(...)` |
| `pad_mux_write()` / `gpi_config()` | `gpio_set_pin_input_high()` 等 |
| `gpi_get_val(pin)` | `gpio_read_pin(pin)` |
| `gpi_enable_int(...)` | `gpio_enable_interrupt(...)` |
| `gpi_disable_int(...)` | `gpio_disable_interrupt(...)` |
| `CS_IN()` / `CS_OUT()` | `ATOMIC_BLOCK_FORCEON { }` |
| `touch_tx_data(MOUSE_REPORT_IDX, ...)` | `bt_driver_send_mouse(...)` |
| `touch_tx_data(TOUCHPAD_REPORT_IDX, ...)` | `bt_driver_send_ptp(...)` |
| `tmos_*/TMOS_*` 系列 | `OSAL_*` 系列 |
| `_set_evt()` / `_clr_evt()` / `_check_evt()` | 移除或映射到模块内部状态 |

## 已有可复用接口

| 功能 | 框架文件 | 接口 |
|-----|---------|-----|
| GPIO 操作 | `hal/gpio.h` | `gpio_read_pin()` 等 |
| I2C 通信 | `hal/i2c_master.h` | `i2c_readReg()` / `i2c_writeReg()` |
| 延时 | `hal/wait.h` | `wait_us()` / `wait_ms()` |
| 临界区 | `atomic_util.h` | `ATOMIC_BLOCK_FORCEON` |
| 事件系统 | `event_manager.h` | `OSAL_SetEvent()` 等 |
| 鼠标报告 | `bt_driver.h` | `bt_driver_send_mouse()` |

## 与 input_service 集成

```c
// application/service/input_service.c

// 触控中断回调 - 由 GPIO 中断调用
static void touch_interrupt_callback(void) {
    OSAL_SetEvent(input_taskID, INPUT_TOUCH_INT_EVT);
}

// 在 input_service_init() 中初始化触控板
void input_service_init(void) {
    input_taskID = OSAL_ProcessEventRegister(input_process_event);

    // 初始化触控板
    touch_power_on();

    // 矩阵扫描定时器...
}

// 在事件处理中
if (events & INPUT_TOUCH_INT_EVT) {
    touch_evt_task();
    return (events ^ INPUT_TOUCH_INT_EVT);
}
```

## 实施步骤

1. **扩展 HAL 层**
   - 在 `hal/gpio.h` 添加 GPIO 中断接口声明
   - 在 `hal/platforms/pixart2860/` 实现 GPIO 中断

2. **扩展 Driver 层**
   - 在 `bt_driver.h` 添加 `bt_driver_send_ptp()` 声明
   - 在对应平台实现 PTP 报告发送

3. **重构 pct1336_driver.c**
   - 替换延时函数、定时器为框架接口
   - 添加 OSAL 任务注册和事件处理

4. **重构 touchpad_service.c**
   - 替换 GPIO、事件、临界区为框架接口
   - 替换 HID 发送为 bt_driver 接口
   - 集成 OSAL 事件系统

5. **集成到 input_service**
   - 添加触控板初始化调用
   - 添加触控中断事件处理

6. **更新 CMakeLists.txt**
   - 添加 touch_component 源文件到构建

## 文件结构

```
component/touch_component/
├── include/
│   ├── touchpad_service.h    # 更新：移除平台依赖的类型定义
│   ├── pct1336_driver.h      # 更新：修复语法错误，移除平台头文件
│   ├── mcs_gesture.h         # 不变
│   └── bayes_filtering.h     # 不变
├── touchpad_service.c        # 重构：使用框架 HAL/OSAL
├── pct1336_driver.c          # 重构：使用框架 HAL/OSAL
├── mcs_gesture.c             # 不变
├── bayes_filtering.c         # 不变
└── README.md                 # 不变
```
