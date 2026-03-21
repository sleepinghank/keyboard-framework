# GPIO 中断管理重构 — 需求文档

> 日期：2026-03-21
> 状态：需求确认

## 1. 问题陈述

当前项目的 GPIO 中断管理存在三个核心问题：

1. **HAL 实现缺失**：`hal/gpio.h` 已声明 `gpio_enable_interrupt()` / `gpio_disable_interrupt()`，但 CH584 平台 `_gpio.c` 未实现
2. **分层违规**：`matrix.c:matrix_prepare_wakeup()` 绕过 HAL 直接调用芯片库函数（`GPIOA_ITModeCfg` 等）
3. **ISR 位置不合理**：`GPIOA_IRQHandler` / `GPIOB_IRQHandler` 定义在 `application/service/input_service.c`，应属于 HAL 平台层

## 2. 需求目标

将 GPIO 中断管理下沉到 HAL 层，提供简洁统一的回调接口，屏蔽芯片内部中断处理细节。

## 3. 需求决策

| # | 决策项 | 结论 |
|---|--------|------|
| 1 | ISR 放置位置 | `hal/platforms/ch584/_gpio.c` |
| 2 | 回调函数签名 | `void (*gpio_int_callback_t)(pin_t pin)` — 带参，支持多 pin 共享回调 |
| 3 | 批量接口 | HAL 层提供 `gpio_enable_interrupt_batch` + `gpio_disable_interrupt_batch` |
| 4 | 唤醒源识别 | 不需要，唤醒后统一全矩阵扫描 |
| 5 | 回调执行上下文 | ISR 中直接调用回调 |

## 4. 功能需求

### 4.1 单 pin 中断接口（已有声明，需实现）

- `gpio_enable_interrupt(pin_t pin, gpio_int_trigger_t trigger, gpio_int_callback_t callback)` — 注册单个 pin 的中断回调
- `gpio_disable_interrupt(pin_t pin)` — 注销单个 pin 的中断

### 4.2 批量中断接口（新增）

- `gpio_enable_interrupt_batch(pin_t *pins, uint8_t count, gpio_int_trigger_t trigger, gpio_int_callback_t callback)` — 批量注册同一回调
- `gpio_disable_interrupt_batch(pin_t *pins, uint8_t count)` — 批量注销

### 4.3 回调类型变更（破坏性变更）

- 现有：`void (*gpio_int_callback_t)(void)`
- 改为：`void (*gpio_int_callback_t)(pin_t pin)`
- 影响范围：`touchpad_service.c` 等所有现有调用方需同步修改签名

### 4.4 ISR 迁移

- `GPIOA_IRQHandler` / `GPIOB_IRQHandler` 从 `input_service.c` 迁移到 `hal/platforms/ch584/_gpio.c`
- ISR 内部通过中断标志位识别触发 pin，查回调表调用对应回调
- 迁移后删除 `input_service.c` 中的旧 ISR 定义，避免链接重复符号

## 5. 约束与边界

### 5.1 范围内

- CH584 平台 `_gpio.c` 中断实现
- `hal/gpio.h` 回调类型修改 + 批量接口声明
- `matrix.c` 唤醒中断代码改用 HAL 接口
- `input_service.c` ISR 迁移 + 旧代码清理
- `touchpad_service.c` 回调签名适配
- test 平台 `_gpio.c` 空实现适配（保证编译通过）

### 5.2 范围外

- test 平台的 `gpio_simulate_interrupt()` 仿真（后续可加）
- 其他平台（nRF52、PixArt 2860）的中断实现
- LPM 状态机重构

## 6. 质量要求

- 批量注册需原子语义：全部成功或全部回滚
- 逐模块替换，每步编译验证
- 回调函数约定：ISR 上下文中执行，必须短小（设标志位、投递事件等）

## 7. 现状分析

### 7.1 相关文件

| 文件 | 职责 | 需改动 |
|------|------|--------|
| `hal/gpio.h` | GPIO 接口声明 | 修改回调类型 + 新增批量接口 |
| `hal/platforms/ch584/_gpio.c` | CH584 GPIO 实现 | 新增中断实现 + ISR |
| `hal/platforms/test/_gpio.c` | test 平台 GPIO 实现 | 适配新签名 |
| `drivers/input/keyboard/matrix.c` | 矩阵扫描 | 改用 HAL 接口 |
| `application/service/input_service.c` | 输入服务 | 删除 ISR，调整事件投递 |
| `component/touch_component/touchpad_service.c` | 触控板服务 | 适配回调签名 |

### 7.2 当前调用链

```
睡眠准备:
  input_service.c (INPUT_LPM_PREPARE_EVT)
    → matrix_prepare_wakeup()
      → GPIOA_ITModeCfg() / GPIOB_ITModeCfg()  [直接调芯片库]
      → PFIC_EnableIRQ()

中断触发:
  input_service.c: GPIOA_IRQHandler()
    → 读 R16_PA_INT_IF
    → OSAL_SetEvent(SYSTEM_LPM_WAKE_EVT)
```

### 7.3 目标调用链

```
睡眠准备:
  input_service.c (INPUT_LPM_PREPARE_EVT)
    → matrix_prepare_wakeup()
      → gpio_enable_interrupt_batch(row_pins, count, FALLING, wakeup_callback)

中断触发:
  _gpio.c: GPIOA_IRQHandler()
    → 查回调表 → wakeup_callback(pin)

唤醒恢复:
  input_service.c (INPUT_LPM_RESUME_EVT)
    → matrix_resume_from_wakeup()
      → gpio_disable_interrupt_batch(row_pins, count)
```
