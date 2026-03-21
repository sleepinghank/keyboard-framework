# 触控板中断标志重构 — 需求文档

> 文档类型：requirements
> 日期：2026-03-21
> 状态：已确认

## 1. 需求概述

废弃 `touchpad_service.c` 中的 `_touch_int_flag` 全局变量，将 ISR 中断通知机制从「标志位轮询」改为「OSAL 事件直接投递」，使触控板数据流与项目整体事件驱动架构对齐。

## 2. 背景与动机

### 2.1 当前问题

当前存在两套并行的中断通知机制，且未正确衔接：

- **旧机制（实际运行）：** ISR `_touch_cb()` → `_touch_int_flag++` → `touch_timer_task()` 轮询检测 → `touch_evt_task()` 原子消费
- **新机制（已实现但未对接）：** `touchpad_notify_int()` → `OSAL_SetEvent(TOUCHPAD_DATA_EVT)` → `touchpad_process_event()` → `touch_evt_task()`

ISR 只操作旧机制的 flag，新机制的入口 `touchpad_notify_int()` 从未被 ISR 调用，导致 OSAL 事件路径形同虚设。

### 2.2 目标

- 统一为纯 OSAL 事件驱动模式
- 消除 `_touch_int_flag` 全局变量及其所有引用
- 废弃 `touch_timer_task()` 函数
- 简化 `touch_evt_task()` 实现

## 3. 需求决策记录

| # | 决策项 | 结论 | 理由 |
|---|--------|------|------|
| Q1 | INT 引脚异常恢复逻辑 | 废弃 | 已有 `touchpad_watchdog_check()` 每 2 秒兜底 |
| Q2 | 按钮重复发送逻辑 | 废弃 | PCT1336 按住时持续产生中断，无需软件补发 |
| Q3 | `kb_break_cnt` 递减方式 | 改为 OSAL 一次性定时事件 | touch 模块自行管理，避免跨模块耦合 |
| Q4 | ISR 中 `OSAL_SetEvent` 安全性 | ISR-safe | CH584 TMOS 的 `tmos_set_event` 是位或操作，天然幂等且中断安全 |
| Q5 | `kb_break_cnt` 归属 | 保持在 touch 模块内部 | 架构评审建议避免移入矩阵定时器引入耦合 |
| Q6 | PCT1336 按钮持续中断 | 已确认 | 按住按钮时芯片持续触发 INT |

## 4. 变更范围

### 4.1 涉及文件

| 文件 | 改动类型 | 说明 |
|------|----------|------|
| `component/touch_component/touchpad_service.c` | 修改 | 主要改动文件，移除 `_touch_int_flag` 所有引用 |
| `component/touch_component/include/touchpad_service.h` | 修改 | 移除 `extern uint8_t _touch_int_flag` 声明 |
| `middleware/touchpad/touchpad.c` | 修改 | 新增 `kb_break_cnt` 定时事件处理 |
| `middleware/touchpad/touchpad.h` | 修改 | 新增事件定义 |

### 4.2 不涉及文件

| 文件 | 原因 |
|------|------|
| `application/service/input_service.c` | 不存在对 `_touch_int_flag` 或 `touch_timer_task` 的引用 |
| `middleware/touchpad/touchpad.c:touchpad_notify_int()` | 实现已完备，无需修改 |

## 5. `_touch_int_flag` 引用清单

以下为需要逐一处理的全部引用点：

| 文件:行号 | 当前代码 | 处理方式 |
|-----------|----------|----------|
| `touchpad_service.c:72` | `uint8_t _touch_int_flag = 0` 定义 | 删除 |
| `touchpad_service.c:780` | ISR `_touch_int_flag++` | 改为 `touchpad_notify_int()` |
| `touchpad_service.c:869-871` | `touch_power_off()` 清零 flag | 改为 `OSAL_StopTask` 停止事件 |
| `touchpad_service.c:937-941` | `end_touchpad_input()` 清零 flag | 改为事件清理，保留 `pct1336_clear_int()` |
| `touchpad_service.c:953-957` | `touch_timer_task()` 检测 flag | 随函数整体废弃 |
| `touchpad_service.c:963-967` | INT 异常恢复强制置 flag | 随函数整体废弃 |
| `touchpad_service.c:1008-1014` | `touch_evt_task()` 原子消费 flag | 简化为无条件执行 `touch_task()` |
| `touchpad_service.h:99` | `extern uint8_t _touch_int_flag` | 删除 |

## 6. 数据流变更

### 6.1 变更前

```
GPIO 中断触发
  |
  v
_touch_cb() [ISR, __HIGH_CODE]
  |  _touch_int_flag++
  v
touch_timer_task() [系统定时器轮询, ~10ms]
  |  检测 _touch_int_flag > 0
  |  kb_break_cnt--
  |  INT 异常恢复
  |  按钮重复发送
  v
touch_evt_task() [主循环]
  |  ATOMIC_BLOCK { _touch_int_flag-- }
  |  touch_task()
  v
I2C 读取 + HID 发送
```

### 6.2 变更后

```
GPIO 中断触发
  |
  v
_touch_cb() [ISR, __HIGH_CODE]
  |  touchpad_notify_int()
  |    -> OSAL_SetEvent(touchpad_taskID, TOUCHPAD_DATA_EVT)
  v
OSAL 事件调度
  |
  v
touchpad_process_event() [middleware]
  |  TOUCHPAD_DATA_EVT -> touch_evt_task()
  |  TOUCHPAD_KB_BREAK_EVT -> kb_break_cnt = 0
  v
touch_evt_task() [简化]
  |  touch_task()
  v
I2C 读取 + HID 发送
```

## 7. 废弃函数与变量

### 7.1 废弃的全局变量

| 变量 | 位置 | 替代 |
|------|------|------|
| `_touch_int_flag` | `touchpad_service.c:72` | OSAL 事件 `TOUCHPAD_DATA_EVT` |
| `int_low_cnt` | `touchpad_service.c:120` | 由 `touchpad_watchdog_check()` 覆盖 |

### 7.2 废弃的函数

| 函数 | 位置 | 替代 |
|------|------|------|
| `touch_timer_task()` | `touchpad_service.c:945` | 功能拆解至 OSAL 事件 |

### 7.3 废弃的逻辑段

| 逻辑 | 位置 | 原因 |
|------|------|------|
| INT 引脚异常恢复 | `touchpad_service.c:960-968` | `touchpad_watchdog_check()` 兜底 |
| 按钮重复发送 | `touchpad_service.c:970-985` | PCT1336 持续中断，无需软件补发 |

## 8. `kb_break_cnt` 生命周期变更

### 8.1 当前

- **设置：** `set_kb_break_cnt(cnt)` 由外部调用（当前无调用者）
- **递减：** `touch_timer_task()` 每次调用时 `kb_break_cnt--`
- **读取：** `ProcessPTPData():706` 中 `if (kb_break_cnt > 0)` 抑制触摸 confidence

### 8.2 变更后

- **设置：** `set_kb_break_cnt(cnt)` 被调用时，同时启动 OSAL 一次性定时事件 `TOUCHPAD_KB_BREAK_EVT`
- **清零：** 定时事件到期后，`touchpad_process_event()` 处理事件并 `kb_break_cnt = 0`
- **读取：** 不变

## 9. 评审意见摘要

### 9.1 用户/产品视角

- 按钮重复废弃需基于 PCT1336 持续中断的确认（已确认）
- INT 异常恢复废弃风险低（watchdog 兜底）
- 延迟反而降低（省去轮询间隔）

### 9.2 架构/编码视角

- ISR 调 middleware 是合理的「向上通知」模式
- OSAL 事件合并语义足够（`touch_task` 一次读完所有数据）
- `kb_break_cnt` 应保持在 touch 模块内部（避免跨模块耦合）
- 移除 `ATOMIC_BLOCK_FORCEON` 无竞态风险

### 9.3 测试/质量视角

- `_touch_int_flag` 无外部调用者，改动封闭
- `set_kb_break_cnt` 当前无调用点，接口保留但实现改为事件驱动
- 建议实机验证：单指/多指触摸、长按按钮 3 秒+、键盘触控交替
- 建议 30 分钟压力测试

## 10. 成功标准

1. 编译通过，无新增 warning
2. 项目中不存在 `_touch_int_flag` 的任何引用
3. `touch_timer_task()` 函数被移除，无调用者
4. 触控板单指/多指触摸正常上报（PTP 和 Mouse 模式）
5. 触控板关机后无 I2C 误访问
6. 键盘打字后触控板 confidence 抑制正常工作
