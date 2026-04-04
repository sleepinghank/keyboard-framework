# indicator OSAL 事件驱动改造 — 需求文档

**日期：** 2026-04-04
**状态：** 需求确认中

---

## 1. 背景与问题

项目整体基于 OSAL 事件驱动架构（TMOS/BLE 栈）。当前 `indicator_task()` 在 `while` 循环的 `peripheral_process()` 中轮询执行，占用 CPU 时间过长导致 BLE 事件无法及时处理，出现蓝牙发送阻塞问题。

**根因：** `peripheral_process()` 内包含同步轮询代码，与 TMOS 的事件驱动调度模型冲突。

---

## 2. 需求目标

将 `indicator_task` 从 `while` 循环轮询改造为 **OSAL 纯延迟事件驱动**，消除对 BLE 调度的干扰。

---

## 3. 需求范围

### 3.1 改造边界
- **仅改造** `drivers/output/indicators/indicator.c`
- 调用方（指示灯控制接口 `indicator_start`、`indicator_stop` 等）**不需要修改**
- 从 `peripheral_process()` 中移除 `indicator_task()` 调用

### 3.2 方案选型：纯延迟事件驱动（方案 A）
- 每个 LED 分配一个独立 OSAL 事件位（如 `LED_0_EVT=0x0001`，`LED_1_EVT=0x0002`...）
- 效果启动时通过 `OSAL_SetDelayedEvent` 设置首次定时
- 到期回调中推进 LED 状态机（DELAY → ON ⇄ OFF），并设置下一个延迟事件
- 无周期 tick，精确按各状态实际时长触发

### 3.3 参数
- LED 数量：≤4 个（事件位充足，独立 task_id 拥有 16 位空间）
- 定时精度：5ms（TMOS tick 对齐精度）

---

## 4. 需求约束（来自评审）

### 4.1 重复触发语义（已确认）
当 LED 效果未结束时再次调用 `indicator_start()`：
- **行为：重置计时** — 取消当前挂起的 OSAL 延迟事件，重新从头开始新效果
- 实现：先调用 `OSAL_StopTask(task_id, LED_N_EVT)`，再重新设置新效果并调用 `OSAL_SetDelayedEvent`

### 4.2 睡眠/唤醒处理
进入 Deep Sleep 前，必须取消所有已挂起的 OSAL 延迟事件（`OSAL_StopTask`），防止唤醒后 LED 状态错乱。建议在 `indicator_prepare_deep()` 回调中处理。

### 4.3 OSAL task 注册时机
`OSAL_ProcessEventRegister` 必须在 BLE 栈完全初始化之后调用（晚于 `GAPRole_StartDevice`），应在 `indicator_init()`（`_init` 阶段）而非 `_setup` 阶段执行。

### 4.4 定时精度说明
OSAL 延迟事件在高 BLE 负载下可能有 ±1 tick 误差，5ms 精度在正常场景下可接受。禁止设置 0ms 延迟事件（会在当前 tick 重入导致问题）。

### 4.5 LED 状态上下文存储
事件位仅作触发信号，LED 效果的完整上下文（当前阶段、剩余次数、各阶段时长）保存在状态结构体 `ind_states[]` 中，不依赖事件位传递状态。

---

## 5. 不在范围内

- 调用方接口修改
- 其他 while 循环模块的改造（如 keyboard_task、communication_task）
- 呼吸灯/渐变等新效果（当前架构兼容，但本次不实现）

---

## 6. 现状分析（来自代码探索）

### 6.1 当前调用链
```
TMOS_SystemProcess()
  → peripheral_process()       # application/system/system_init.c
    → indicator_task()          # drivers/output/indicators/indicator.c
      → 遍历 ind_states[IND_LED_COUNT]
        → timer_elapsed32() 超时检查
          → process_led_timeout()  # 四段状态机
```

### 6.2 process_led_timeout 状态机
```
IDLE: 不处理
DELAY: 超时 → 进入 ON（直接亮）或 BLINK（闪烁）
ON: 超时 → 熄灭，判断是否结束（BLINK → OFF）
OFF: 超时 → 点亮，count++，判断 repeat → ON
每次状态切换重置 state->timer 和 state->next_period
```

### 6.3 indicator 当前 OSAL 集成状态
- **无 task_id**，未调用 `OSAL_ProcessEventRegister`
- 头文件注释声称 OSAL 驱动，但实现为轮询
- `indicator_test.c` 已有完整的链式延迟事件调度示例，可参考

### 6.4 关键数据

- `IND_LED_COUNT = 3`（`keyboards/kb904/config_product.h` 第 86 行）
- `indicator_task()` 调用位置：`system_init.c` 第 184 行（`peripheral_process()` 内）
- `indicator_init()` 调用位置：`system_init.c` 第 95 行

### 6.5 ind_led_state_t 结构体字段

| 字段 | 类型 | 改造后 |
|------|------|--------|
| effect | ind_effect_t | 保留 |
| phase | ind_phase_t | 保留 |
| count | uint8_t | 保留 |
| is_on | bool | 保留 |
| timer | uint32_t | **可删除**（OSAL 接管计时） |
| next_period | uint16_t | **可删除**（OSAL 接管计时） |

### 6.6 start_led_effect 特殊情况
- 有 delay 时：进入 `PHASE_DELAY`，设 `next_period=delay_ms`
- **无 delay 时**：`next_period=0`，立即同步调用 `process_led_timeout()`（改造后改为 `OSAL_SetDelayedEvent(..., 1)` 最小值触发）

### 6.7 OSAL API 函数签名
```c
uint8_t      OSAL_ProcessEventRegister(osal_task_event_handler_t eventCb);
error_code_t OSAL_SetDelayedEvent(uint8_t taskID, uint16_t event, uint32_t time);
error_code_t OSAL_StopTask(uint8_t taskID, uint16_t event);
error_code_t OSAL_SetEvent(uint8_t taskID, uint16_t event);
```

### 6.8 相关文件路径
| 文件 | 说明 |
|------|------|
| `drivers/output/indicators/indicator.c` | 核心改造文件（约 365 行） |
| `drivers/output/indicators/indicator.h` | 对外接口（不变） |
| `application/system/system_init.c` | 需移除第 184 行 indicator_task() 调用 |
| `application/indicator_test.c` | OSAL 链式调度完整参考示例 |
| `drivers/system/event_manager.h` | OSAL 抽象 API |

---

## 7. 改造差距列表

| 序号 | 类型 | 内容 |
|------|------|------|
| 1 | 新增 | `static uint8_t ind_task_id` + `indicator_init()` 中注册 OSAL task |
| 2 | 新增 | 3 个 LED 事件位宏：`IND_EVT_LED_0/1/2` |
| 3 | 新增 | `indicator_process_event()` 事件处理回调函数 |
| 4 | 修改 | `start_led_effect()` 改为调用 `OSAL_SetDelayedEvent` / `OSAL_StopTask` |
| 5 | 修改 | `process_led_timeout()` 末尾状态切换改为 `OSAL_SetDelayedEvent` |
| 6 | 删除 | `indicator_task()` 轮询逻辑（或保留空壳） |
| 7 | 删除 | `system_init.c` 第 184 行的 `indicator_task()` 调用 |
| 8 | 可选 | 删除 `ind_led_state_t` 中 `timer` 和 `next_period` 字段 |
