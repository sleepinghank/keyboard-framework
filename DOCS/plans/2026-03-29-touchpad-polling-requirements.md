# 需求文档：Touchpad 轮询模式改造

## 1. 需求概述

将 touchpad 模块从 OSAL 事件调度改为在主 while 循环（peripheral_process）中直接轮询处理，减少触控延迟。

**当前流程**：GPIO 中断 → 设置标志位 → OSAL 事件调度 → touchpad 事件处理函数
**目标流程**：GPIO 中断 → 设置标志位 → 主循环轮询检测 → touchpad_task 直接处理

## 2. 需求 Q&A

| 问题 | 决定 |
|------|------|
| 中断保留方式 | 保留 GPIO 中断设置标志位，主循环中检查标志位并处理 |
| 处理频率 | 每次 peripheral_process 循环都调用，与 keyboard_task 同频 |
| OSAL 清理 | 完全移除 touchpad 相关的 OSAL 事件注册和处理代码 |

## 3. 评审意见

### 产品视角（通过）
- 功能完整，保留中断机制仅改变调度方式
- 触控延迟将显著减少
- 需确认无其他模块依赖 touchpad OSAL 事件

### 架构视角（有疑虑 → 已回应）
- 疑虑：破坏事件驱动模式一致性
- 回应：keyboard_task 已使用轮询模式，touchpad 跟随同样模式提高了一致性
- 疑虑：中断与轮询时序竞争
- 回应：标志位为标准生产者-消费者模式（ISR 写、主循环读），时序安全

### 质量视角（有疑虑 → 已回应）
- 疑虑：需量化延迟改善
- 回应：成功标准为触控响应主观无延迟感
- 疑虑：LPM/组合键回归风险
- 回应：回归范围集中在 touchpad 通路，不影响 keyboard/LPM

## 4. 范围边界

### 包含
- 移除 touchpad OSAL 事件注册和处理代码
- 在 peripheral_process 中添加 touchpad_task 调用
- 保留 GPIO 中断和标志位机制

### 不包含
- 不修改 touchpad 数据处理逻辑本身
- 不修改 keyboard_task 或其他模块
- 不修改触控板驱动层（driver/input/touchpad）

## 5. 代码现状分析

### 数据流变化

```
当前: GPIO中断 → g_touchpad_int_flag → input_service → OSAL_SetEvent → touchpad_process_event → touch_evt_task
目标: GPIO中断 → g_touchpad_int_flag → peripheral_process → touchpad_task → touch_evt_task
```

### 差距清单

| 文件 | 位置 | 操作 | 说明 |
|------|------|------|------|
| touchpad.c | 约L106 | 移除 | `OSAL_ProcessEventRegister` 事件注册 |
| touchpad.c | 约L53-97 | 移除 | `touchpad_process_event` 整个事件处理函数 |
| touchpad.c | 约L84,90,169 | 移除 | `OSAL_SetDelayedEvent`/`OSAL_StopTask` 调用 |
| touchpad.c | 全文件 | 移除 | `touchpad_taskID` 变量定义和使用 |
| touchpad.c | 需新增 | 添加 | `touchpad_task()` 轮询处理函数 |
| input_service.c | 约L194-195 | 移除 | `touchpad_setup()` 调用和 `OSAL_SetEvent(touchpad_taskID, ...)` |
| system_init.c | peripheral_process 内 | 添加 | `touchpad_task()` 调用 |
| touchpad.h | 约L9-12 | 移除 | 事件宏定义（TOUCHPAD_INIT_EVT 等） |
| touchpad.h | touchpad_notify_int | 修改 | 改为直接处理标志位而非投递 OSAL 事件 |

### 不受影响的部分

- GPIO 中断驱动层（drivers/input/touchpad/）：中断设置和标志位机制保持不变
- touch_evt_task 核心处理逻辑：仅改变调度入口，处理逻辑不变
- keyboard_task 和其他 peripheral_process 任务：不受影响
