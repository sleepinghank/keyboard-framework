# N0046 睡眠策略设计

> 更新日期：2026-03-13  
> 适用对象：N0046 / CH584M / 本地 `keyboard-framework`  
> 参考实现：
> - `docs/code_demo/KB04122-13A-WCH_code.xml`
> - `project/ch584m/platforms_HAL/SLEEP.c`
> - `project/ch584m/platforms_HAL/RTC.c`

## 1. 设计目标与范围

本文档用于定义 N0046 在本地框架中的 `Idle Sleep` 和 `Deep Sleep` 方案，目标如下：

| 目标 | 说明 |
|---|---|
| 保证层级隔离 | 执行层/HAL 只负责寄存器、唤醒源和睡眠指令，不直接操作按键、背光、指示灯、BLE 业务逻辑 |
| 保持 OSAL 事件化 | 所有关键动作都通过 OSAL 事件调度，不允许在 `system_service` 中直接跨 service 调方法完成整套睡眠动作 |
| 明确区分 Idle 与 Deep | Idle 追求快速恢复和不断连；Deep 追求更低功耗和完整恢复 |
| 严格参考 CH584/WCH 实现 | 睡眠入口、RTC 唤醒、GPIO 唤醒、PA2 恢复等底层细节必须参考 04122-13A 与 WCH 原始 `SLEEP.c/RTC.c` |
| 首版优先稳定 | 首版优先保证“能稳定睡、能稳定醒、不会睡死、不会假唤醒”，不追求最激进功耗 |

本次设计范围包括：

1. Idle / Deep 的触发条件与进入流程。
2. Idle / Deep 的唤醒条件与恢复流程。
3. 睡眠模式下的功耗优化措施。
4. 睡眠模式下的外设管理策略。
5. 所有需要改造的模块与接口清单。

本次设计明确不包含：

1. Deep Sleep 唤醒后是否自动回连 BLE。
2. 业务层回连策略、回连时机、广播策略。
3. 直接修改代码实现。

## 2. 总体设计原则

| 原则 | 设计要求 |
|---|---|
| 层级隔离 | `HAL -> Drivers -> Middleware -> Application` 依赖方向不反转 |
| 事件驱动 | 睡眠进入、睡眠准备、唤醒恢复全部通过 OSAL 事件执行 |
| Service 自治 | `input_service` 只处理输入相关准备/恢复；`communication_service` 只处理无线；`output_service` 只处理背光/指示灯 |
| HAL 不碰上层资源 | `system_hal_enter_sleep()` 不允许调用 `indicator_off_all()`、`backlight_disable()`、`wireless_disconnect()` 等上层接口 |
| Idle / Deep 独立状态机 | 不能把 Idle 做成“轻版 Deep”，两者准备和恢复动作不同 |
| 以 04122-13A 为平台基线 | 参考其 GPIO/RTC/PA2 的睡前准备和唤醒恢复，不直接复用旧 `HAL_SLEEP` 组织方式 |

## 3. CH584 / 04122-13A 复核结论

### 3.1 平台级关键结论

| 复核项 | 04122-13A / WCH 参考行为 | 对本项目的约束 |
|---|---|---|
| 定时唤醒 | WCH `SLEEP.c` 通过 `RTC_SetTignTime()`、`RTC_IRQHandler()`、`RB_SLP_RTC_WAKE` 实现真正 RTC 硬件唤醒 | 本项目 `SYSTEM_WAKEUP_TIMER` 必须落到 RTC 唤醒链路，不能只保留逻辑概念 |
| GPIO 唤醒 | 04122-13A 在 `peripheral_enter_sleep()` 中先配置矩阵、PA2、GPIO 中断，再使能 `RB_SLP_GPIO_WAKE` | 本项目必须先完成输入 prepare，再打开 GPIO 唤醒位 |
| PA2 恢复 | 04122-13A 的 `power_key_resume()` 明确在唤醒后重新配置 PA2 模式、中断和 IRQ | 本项目唤醒恢复必须显式恢复电源键中断，不能假定原配置仍有效 |
| 睡眠参数 | WCH / 04122 均直接给出 `LowPower_Sleep(...)` 位组合，而不是抽象成“任意低功耗模式” | 本项目首版必须以已验证参数组为基线，不允许拍脑袋改 RAM 保留位或删 `RB_XT_PRE_EN` |
| 唤醒源抽象 | 本地 `_system_hal.c` 当前只映射 `GPIO/RTC/USB` | 文档中的“键盘唤醒/触控唤醒/BLE 低功耗”需要拆开理解：键盘和触控最终都落到 GPIO，BLE 只是协议低功耗，不是独立硬件唤醒位 |
| USB 低功耗 | 04122-13A 睡前会把 USB 相关管脚切到低功耗输入态 | N0046 当前 `USB_ENABLE_FLAG == FALSE`，Deep 设计应把 USB 管脚纳入低功耗处理 |

### 3.2 当前本地实现与平台基线的差异

| 项目 | 当前状态 | 设计结论 |
|---|---|---|
| `hal/platforms/ch584/_system_hal.c` | 仅映射 `SYSTEM_WAKEUP_GPIO`、`SYSTEM_WAKEUP_RTC`、`SYSTEM_WAKEUP_USB` | 需要补齐 `SYSTEM_WAKEUP_TIMER -> RTC` 的平台映射 |
| `SYSTEM_WAKEUP_KEYBOARD/BLE` | 当前没有落到 CH584 硬件唤醒位 | 不能把它们当成真实硬件唤醒位使用 |
| `touch_power_off()` | 现有路径会在睡前继续 `touch_gpio_uninit()` | 不适合作为“睡眠中仍可触控唤醒”的实现 |
| 矩阵方向 | N0046 是 `ROW2COL` | 不能照抄 04122-13A 的矩阵 GPIO 方向配置 |
| `PM_STANDBY_WITH_RAM` | 当前本地路径不完整 | Deep 首版不走该路径，统一走 `system_hal_enter_sleep(SYSTEM_POWER_MODE_DEEP_SLEEP)` |

### 3.3 CH584 底层强约束

| 约束 | 要求 |
|---|---|
| RTC 寄存器写入 | 必须参考 WCH `RTC_SetTignTime()` 的安全访问方式，不能裸写触发寄存器 |
| RTC 中断处理 | 必须在中断里清 RTC 标志并锁存唤醒原因 |
| GPIO 唤醒顺序 | 必须先配置 GPIO，再清中断标志，再开 IRQ，最后使能 `RB_SLP_GPIO_WAKE` |
| 唤醒后恢复 | 必须恢复矩阵、PA2、触控等实际中断配置，不仅是恢复软件标志 |
| 睡眠参数验证 | `LowPower_Sleep(...)` 的位组合需要以 04122/WCH 为基线，并经过 N0046 实机验证 |

## 4. 总体状态机

| 状态 | 含义 | 进入条件 | 退出条件 |
|---|---|---|---|
| `LPM_STATE_ACTIVE` | 正常运行 | 上电完成或睡眠恢复完成 | 5s 无操作进入 `IDLE_PENDING` |
| `LPM_STATE_IDLE_PENDING` | Idle 准备中 | 收到 `SYSTEM_LPM_IDLE_REQ_EVT` | 所有 Idle prepare 完成后进入 `IDLE_SLEEP`；若有新活动则回 `ACTIVE` |
| `LPM_STATE_IDLE_SLEEP` | Idle 已进入 | `SYSTEM_LPM_ENTER_IDLE_EVT` 成功执行 | GPIO / 触控 / PA2 / RTC 唤醒后进入 `WAKE_RESUME` |
| `LPM_STATE_DEEP_PENDING` | Deep 准备中 | 收到 `SYSTEM_LPM_DEEP_REQ_EVT` | 所有 Deep prepare 完成后进入 `DEEP_SLEEP`；若有新活动则回 `ACTIVE` |
| `LPM_STATE_DEEP_SLEEP` | Deep 已进入 | `SYSTEM_LPM_ENTER_DEEP_EVT` 成功执行 | GPIO / 触控 / PA2 / RTC 唤醒后进入 `WAKE_RESUME` |
| `LPM_STATE_WAKE_RESUME` | 唤醒恢复中 | 收到 `SYSTEM_LPM_WAKE_EVT` | 恢复完成后回 `ACTIVE` |

## 5. Idle Sleep 方案

### 5.1 触发条件

| 条件 | 说明 |
|---|---|
| 5s 无操作 | 由 `lpm_note_activity()` 维护最后活动时间，连续 5s 无键盘/触控/业务活动后成立 |
| 当前允许进入低功耗 | USB 活动、OTA、关键事务、升级等场景下禁止进入 Idle |
| 未被更深状态抢占 | 若 10min 条件已经成立，则直接转入 Deep 流程 |

### 5.2 Idle 模式目标

| 项目 | Idle 目标 |
|---|---|
| 键盘扫描 | 停止矩阵扫描定时器，改为中断唤醒 |
| BLE | 请求 BLE 进入低功耗，但不断连 |
| 背光 | 保持当前状态，不关闭 |
| 指示灯 | 保持当前状态，不关闭 |
| 触控 | 进入低功耗，但保留唤醒能力 |
| 电源键 PA2 | 保留独立唤醒能力 |

### 5.3 Idle 进入机制

Idle 不采用“在 `IDLE_PENDING` 中直接跨 service 调方法后立刻睡眠”的做法，而采用最小事件汇聚模型：

| 项目 | 作用 |
|---|---|
| `prepare_pending_mask` | 标记本轮 Idle 需要等待哪些 service |
| `prepare_done_mask` | 标记哪些 service 已完成 prepare |
| `SYSTEM_LPM_STEP_DONE_EVT` | 统一的 prepare 完成通知 |
| `SYSTEM_LPM_ENTER_IDLE_EVT` | 真正执行 Idle 睡眠的最终事件 |

首版 Idle 仅等待两个 service：

| Service | 是否进入 Idle prepare 汇聚 | 原因 |
|---|---|---|
| `input_service` | 是 | 需要停扫描定时器、配置矩阵/触控/PA2/RTC 唤醒 |
| `communication_service` | 是 | 需要请求 BLE 低功耗 |
| `output_service` | 否 | Idle 不关背光和指示灯，首版无需参与 |

### 5.4 Idle 进入流程

| 步骤 | 执行者 | 动作 | 结果 |
|---|---|---|---|
| 1 | `lpm` | 5s 无活动后投递 `SYSTEM_LPM_IDLE_REQ_EVT` | 触发 Idle 流程 |
| 2 | `system_service` | 状态切到 `LPM_STATE_IDLE_PENDING` | 建立本轮 Idle 上下文 |
| 3 | `system_service` | 初始化 `prepare_pending_mask = PREPARE_INPUT | PREPARE_COMMU`，`prepare_done_mask = 0` | 开始等待 prepare 完成 |
| 4 | `system_service` | 投递 `INPUT_LPM_PREPARE_IDLE_EVT` | 输入侧开始准备 |
| 5 | `system_service` | 投递 `COMMU_LPM_PREPARE_IDLE_EVT` | 通信侧开始准备 |
| 6 | `input_service` | 停止矩阵扫描定时器；切矩阵 GPIO 为唤醒模式；保留触控 INT；配置 PA2；按需准备 RTC 唤醒 | 输入 prepare 完成 |
| 7 | `communication_service` | 请求 BLE 进入低功耗 | 通信 prepare 完成 |
| 8 | 各 service | 完成本地 prepare 后更新 `prepare_done_mask`，投递 `SYSTEM_LPM_STEP_DONE_EVT` | 上报完成 |
| 9 | `system_service` | 收到 `SYSTEM_LPM_STEP_DONE_EVT` 后检查 `prepare_done_mask == prepare_pending_mask` | 全部完成则可进入睡眠 |
| 10 | `system_service` | 投递 `SYSTEM_LPM_ENTER_IDLE_EVT` 给自己 | 将“完成判断”和“实际睡眠”拆开 |
| 11 | `system_service` | 二次确认当前仍为 `IDLE_PENDING` 且没有新活动 | 防止误睡 |
| 12 | `system_service` | 调用 `system_hal_enter_sleep(SYSTEM_POWER_MODE_SLEEP, wake_mask)` | MCU 进入 Idle Sleep |

### 5.5 为什么不直接在 `IDLE_PENDING` 中跨 service 调方法

| 维度 | 直接调用 | 事件汇聚方案 | 结论 |
|---|---|---|---|
| 代码量 | 更少 | 略多一个位图和一个完成事件 | 直接调用略少 |
| OSAL 一致性 | 差 | 好 | 事件汇聚更优 |
| 层级隔离 | 差 | 好 | 事件汇聚更优 |
| 扩展性 | 差 | 好 | 事件汇聚更优 |
| 防误睡 | 较差 | 可以在 `ENTER_IDLE_EVT` 做二次确认 | 事件汇聚更优 |
| 本项目适配性 | 不符合既定约束 | 完全符合 | 事件汇聚更优 |

结论：本项目 Idle 进入必须采用“准备事件扇出 + 位图汇聚 + 最终进入事件”的最小实现方案。

### 5.6 Idle 唤醒条件

| 唤醒源 | 硬件落点 | 说明 |
|---|---|---|
| 矩阵按键 | GPIO 唤醒 | 键盘唤醒在 CH584 上本质是 GPIO 唤醒 |
| 触控板 INT | GPIO 唤醒 | 触控不能在睡前 uninit 唤醒 GPIO |
| 电源键 PA2 | GPIO 唤醒 | 需保留独立的 PA2 下降沿中断 |
| RTC | RTC 唤醒 | 若 Idle 设计包含“定时唤醒”或“下一阶段推进”，则必须走 RTC |

### 5.7 Idle 唤醒恢复流程

| 步骤 | 执行者 | 动作 | 结果 |
|---|---|---|---|
| 1 | GPIO/RTC ISR | 仅锁存唤醒原因并投递 `SYSTEM_LPM_WAKE_EVT` | 不在 ISR 中做恢复 |
| 2 | `system_service` | 状态切到 `LPM_STATE_WAKE_RESUME` | 开始恢复 |
| 3 | `system_service` | 投递 `INPUT_LPM_RESUME_IDLE_EVT` | 输入侧恢复矩阵/PA2/触控中断 |
| 4 | `input_service` | 恢复矩阵 GPIO、补一次扫描、恢复扫描定时器 | 键盘恢复正常扫描 |
| 5 | `communication_service` | 如需要，仅恢复 BLE 低功耗前的本地上下文 | 不直接回连 |
| 6 | `system_service` | 恢复完成后切回 `LPM_STATE_ACTIVE` | Idle 唤醒完成 |

### 5.8 Idle 模式功耗优化措施

| 措施 | 说明 |
|---|---|
| 停止矩阵扫描定时器 | 这是 Idle 首要的功耗优化动作 |
| BLE 进入低功耗 | 保持不断连，但降低无线功耗 |
| 可暂停 ADC 周期采样 | 进入 Idle 后可暂停电量采样定时任务 |
| 保留背光/指示灯 | 这是产品体验要求，不纳入 Idle 关断动作 |

### 5.9 Idle 模式外设管理策略

| 外设 | Idle 处理策略 |
|---|---|
| 矩阵扫描 | 停止扫描定时器，改为 GPIO 唤醒 |
| 触控板 | 进入低功耗，保留 INT 唤醒 |
| 电源键 PA2 | 保留独立中断唤醒 |
| BLE | 请求低功耗，不断连 |
| 背光 | 不处理，保持当前状态 |
| 指示灯 | 不处理，保持当前状态 |
| ADC | 可暂停周期采样 |
| USB 管脚 | 若 USB 未启用，可进入更低功耗 GPIO 状态 |

## 6. Deep Sleep 方案

### 6.1 触发条件

| 条件 | 说明 |
|---|---|
| 10min 无操作 | 从最后一次用户活动开始持续 10 分钟无操作 |
| 当前允许进入 Deep | 无 USB 活动、无 OTA、无关键业务占用 |
| Idle 已经稳定或可直接推进 | 一般从 Idle 保持态推进到 Deep，也允许在状态机中直接切换到 `DEEP_PENDING` |

### 6.2 Deep 模式目标

| 项目 | Deep 目标 |
|---|---|
| BLE | 断连，停止保持当前连接状态 |
| 背光 | 关闭 |
| 指示灯 | 关闭 |
| 输入侧 | 停矩阵扫描，保留必要唤醒链路 |
| 触控 | 进入更深低功耗，但仍保留唤醒能力 |
| 电源键 PA2 | 保留独立唤醒能力 |
| 恢复方式 | 唤醒后重新初始化本地状态和外设，不在底层自动回连 BLE |

### 6.3 Deep 进入机制

Deep 与 Idle 一样采用 prepare 汇聚机制，但等待集合更完整：

| Service | 是否进入 Deep prepare 汇聚 | 原因 |
|---|---|---|
| `input_service` | 是 | 需要切更深的矩阵/触控/PA2 状态 |
| `communication_service` | 是 | 需要断连、停止广播或连接保持 |
| `output_service` | 是 | 需要关闭背光和指示灯 |

### 6.4 Deep 进入流程

| 步骤 | 执行者 | 动作 | 结果 |
|---|---|---|---|
| 1 | `lpm` | 10min 无活动后投递 `SYSTEM_LPM_DEEP_REQ_EVT` | 触发 Deep 流程 |
| 2 | `system_service` | 状态切到 `LPM_STATE_DEEP_PENDING` | 建立本轮 Deep 上下文 |
| 3 | `system_service` | 初始化 `prepare_pending_mask = PREPARE_INPUT | PREPARE_COMMU | PREPARE_OUTPUT` | 开始等待 prepare 完成 |
| 4 | `system_service` | 投递 `INPUT_LPM_PREPARE_DEEP_EVT` | 输入侧开始 Deep 准备 |
| 5 | `system_service` | 投递 `COMMU_LPM_PREPARE_DEEP_EVT` | 通信侧开始 Deep 准备 |
| 6 | `system_service` | 投递 `OUTPUT_LPM_PREPARE_DEEP_EVT` | 输出侧开始 Deep 准备 |
| 7 | `input_service` | 停扫描；切矩阵唤醒；触控更深低功耗；准备 PA2/RTC 唤醒 | 输入 prepare 完成 |
| 8 | `communication_service` | 断连并停止相关无线保持动作 | 通信 prepare 完成 |
| 9 | `output_service` | 关闭背光和指示灯 | 输出 prepare 完成 |
| 10 | 各 service | 更新 `prepare_done_mask` 并投递 `SYSTEM_LPM_STEP_DONE_EVT` | 上报完成 |
| 11 | `system_service` | 收敛完成后投递 `SYSTEM_LPM_ENTER_DEEP_EVT` | 准备进入 Deep |
| 12 | `system_service` | 二次确认条件仍成立 | 防止误睡 |
| 13 | `system_service` | 调用 `system_hal_enter_sleep(SYSTEM_POWER_MODE_DEEP_SLEEP, wake_mask)` | MCU 进入 Deep Sleep |

### 6.5 Deep 唤醒条件

| 唤醒源 | 硬件落点 | 说明 |
|---|---|---|
| 矩阵按键 | GPIO 唤醒 | 必须按 N0046 矩阵方向配置 |
| 触控板 INT | GPIO 唤醒 | 需要保留唤醒 GPIO |
| 电源键 PA2 | GPIO 唤醒 | 需要显式恢复 |
| RTC | RTC 唤醒 | 若设计保留 RTC 推进或定时恢复机制，可使用 |

### 6.6 Deep 唤醒恢复流程

| 步骤 | 执行者 | 动作 | 结果 |
|---|---|---|---|
| 1 | GPIO/RTC ISR | 锁存唤醒原因并投递 `SYSTEM_LPM_WAKE_EVT` | ISR 不做复杂动作 |
| 2 | `system_service` | 切到 `LPM_STATE_WAKE_RESUME` | 开始恢复 |
| 3 | `system_service` | 先投递 `INPUT_LPM_RESUME_DEEP_EVT` | 恢复矩阵、PA2、触控、扫描状态 |
| 4 | `system_service` | 再投递 `OUTPUT_LPM_RESUME_DEEP_EVT` | 恢复背光/指示灯基础状态 |
| 5 | `system_service` | 投递 `COMMU_LPM_RESUME_DEEP_EVT` | 仅恢复本地无线上下文，不直接回连 |
| 6 | 各 service | 清理睡眠标志、恢复本地资源 | 本地资源恢复完成 |
| 7 | `system_service` | 投递业务层唤醒事件 | 由业务决定是否回连 |
| 8 | `system_service` | 状态回到 `LPM_STATE_ACTIVE` | Deep 唤醒完成 |

### 6.7 Deep 模式功耗优化措施

| 措施 | 说明 |
|---|---|
| 停止矩阵扫描定时器 | 降低输入轮询功耗 |
| BLE 断连 | Deep 模式的主要节电措施之一 |
| 关闭背光与指示灯 | 符合用户要求 |
| 关闭 ADC 周期采样 | 避免无意义采样 |
| USB 管脚低功耗配置 | N0046 当前 USB 未启用，Deep 需纳入处理 |
| 采用保守的 `LowPower_Sleep(...)` 参数 | 首版优先稳定，不走未验证的 Shutdown 路径 |

### 6.8 Deep 模式外设管理策略

| 外设 | Deep 处理策略 |
|---|---|
| 矩阵扫描 | 停止扫描，保留 GPIO 唤醒 |
| 触控板 | 更深低功耗，保留唤醒链路 |
| 电源键 PA2 | 保留独立唤醒 |
| BLE | 断连 |
| 背光 | 关闭 |
| 指示灯 | 关闭 |
| ADC | 关闭周期采样 |
| USB 管脚 | 切到低功耗态 |

## 7. OSAL 事件调度方案

### 7.1 核心原则

| 原则 | 说明 |
|---|---|
| ISR 只置位 | GPIO/RTC ISR 只记录唤醒原因和投递系统事件 |
| `system_service` 只调度 | 不直接操作背光、指示灯、矩阵、触控、BLE |
| 各 service 只处理自己的资源 | 不跨 service 改配置 |
| `STEP_DONE_EVT` 只做汇聚 | 不在其中直接进入睡眠 |
| `ENTER_*_EVT` 只做最终睡眠 | 进入前做二次确认 |

### 7.2 建议事件集

#### `system_service`

| 事件 | 用途 |
|---|---|
| `SYSTEM_LPM_IDLE_REQ_EVT` | 请求进入 Idle |
| `SYSTEM_LPM_DEEP_REQ_EVT` | 请求进入 Deep |
| `SYSTEM_LPM_STEP_DONE_EVT` | prepare 完成汇聚事件 |
| `SYSTEM_LPM_ENTER_IDLE_EVT` | 最终进入 Idle |
| `SYSTEM_LPM_ENTER_DEEP_EVT` | 最终进入 Deep |
| `SYSTEM_LPM_WAKE_EVT` | 唤醒恢复入口 |

#### `input_service`

| 事件 | 用途 |
|---|---|
| `INPUT_LPM_PREPARE_IDLE_EVT` | Idle 输入 prepare |
| `INPUT_LPM_PREPARE_DEEP_EVT` | Deep 输入 prepare |
| `INPUT_LPM_RESUME_IDLE_EVT` | Idle 唤醒恢复 |
| `INPUT_LPM_RESUME_DEEP_EVT` | Deep 唤醒恢复 |

#### `communication_service`

| 事件 | 用途 |
|---|---|
| `COMMU_LPM_PREPARE_IDLE_EVT` | BLE 低功耗 prepare |
| `COMMU_LPM_PREPARE_DEEP_EVT` | BLE 断连 prepare |
| `COMMU_LPM_RESUME_IDLE_EVT` | Idle 唤醒恢复 |
| `COMMU_LPM_RESUME_DEEP_EVT` | Deep 唤醒恢复 |

#### `output_service`

| 事件 | 用途 |
|---|---|
| `OUTPUT_LPM_PREPARE_IDLE_EVT` | Idle 首版可为 no-op |
| `OUTPUT_LPM_PREPARE_DEEP_EVT` | Deep 关闭背光/指示灯 |
| `OUTPUT_LPM_RESUME_IDLE_EVT` | Idle 首版可为 no-op |
| `OUTPUT_LPM_RESUME_DEEP_EVT` | Deep 唤醒恢复输出侧状态 |

## 8. CH584 HAL 设计要求

### 8.1 唤醒源映射要求

| 抽象唤醒源 | CH584 实际落点 | 说明 |
|---|---|---|
| `SYSTEM_WAKEUP_GPIO` | `RB_SLP_GPIO_WAKE` | 用于矩阵、触控、PA2 等所有 GPIO 唤醒 |
| `SYSTEM_WAKEUP_TIMER` | `RB_SLP_RTC_WAKE` + RTC 触发配置 | 必须由 HAL 显式落到 RTC |
| `SYSTEM_WAKEUP_RTC` | `RB_SLP_RTC_WAKE` | 与 `SYSTEM_WAKEUP_TIMER` 的硬件落点一致 |
| `SYSTEM_WAKEUP_USB` | `RB_SLP_USB_WAKE` | 仅在 USB 模式下启用 |
| `SYSTEM_WAKEUP_KEYBOARD` | 不应直接映射为独立硬件位 | 键盘最终属于 GPIO 唤醒 |
| `SYSTEM_WAKEUP_BLE` | 不应直接映射为独立硬件位 | BLE 低功耗是协议状态，不是 CH584 独立唤醒位 |

### 8.2 RTC 设计要求

| 要点 | 要求 |
|---|---|
| 初始化 | 必须有等价于 `HAL_SleepInit()` 的初始化动作，确保 `RB_SLP_RTC_WAKE`、`RB_RTC_TRIG_EN`、`RTC_IRQn` 已就绪 |
| 触发时间配置 | 必须有等价于 `RTC_SetTignTime()` 的平台实现 |
| 安全访问 | 写 RTC 触发寄存器时必须使用 CH584 安全访问流程 |
| 中断清标志 | `RTC_IRQHandler()` 必须清 RTC 标志并锁存唤醒原因 |
| 无效窗口保护 | 睡眠窗口过短时不进入 RTC 睡眠，防止无意义睡眠/假唤醒 |

### 8.3 Sleep 参数要求

| 要点 | 要求 |
|---|---|
| Idle 参数基线 | 以 04122 / WCH 已验证参数组为基线，优先保证 BLE 和系统时钟恢复稳定 |
| Deep 参数基线 | 首版采用保守参数，不直接使用未验证的 `LowPower_Shutdown()` |
| 实机校验 | `LowPower_Sleep(...)` 的最终参数必须经过 N0046 实测确认 |

### 8.4 HAL 的边界

| 允许做的事 | 不允许做的事 |
|---|---|
| 设置唤醒源、写 RTC、执行 `LowPower_Sleep(...)`、锁存唤醒原因 | 调背光、关指示灯、断 BLE、停键盘扫描、触发业务重连 |

## 9. 外设管理策略汇总

| 外设 | Active | Idle | Deep | 唤醒恢复 |
|---|---|---|---|---|
| 矩阵扫描 | 周期扫描 | 停扫描，切 GPIO 唤醒 | 停扫描，切 GPIO 唤醒 | 恢复 GPIO，补一扫描，恢复定时器 |
| 触控板 | 正常工作 | 低功耗，保留 INT | 更深低功耗，保留 INT | 恢复 I2C/INT/工作模式 |
| PA2 电源键 | 正常按键逻辑 | 保留独立唤醒 | 保留独立唤醒 | 重新配置中断并清标志 |
| BLE | 正常工作 | 请求低功耗，不断连 | 断连 | 恢复本地上下文，是否回连由业务决定 |
| 背光 | 正常 | 保持当前状态 | 关闭 | Deep Wake 后按系统状态恢复 |
| 指示灯 | 正常 | 保持当前状态 | 关闭 | Deep Wake 后按系统/无线状态恢复 |
| ADC | 周期采样 | 可暂停 | 停止 | 恢复周期采样 |
| USB 管脚 | 视模式而定 | 若未启用可低功耗 | Deep 中切低功耗态 | 需要时恢复 |

## 10. 修改内容总清单

| 模块 / 文件 | 需要修改的内容 | 原因 |
|---|---|---|
| `middleware/communication/lpm.h/.c` | 增加 Idle / Deep 状态、5s/10min 超时、`prepare_pending_mask` / `prepare_done_mask`、活动记录接口 | 负责低功耗状态机和超时推进 |
| `application/service/system_service.h/.c` | 增加 LPM 事件调度、prepare 汇聚、最终进入事件、统一唤醒恢复入口 | 只负责调度，不直接操作外设 |
| `application/service/input_service.h/.c` | 增加 Idle / Deep prepare / resume 流程；停止扫描定时器；矩阵 GPIO 唤醒；PA2 恢复；触控唤醒恢复 | 输入是主要唤醒链路拥有者 |
| `application/service/communication_service.h/.c` | 增加 BLE Idle 低功耗 prepare、Deep 断连 prepare、唤醒后本地上下文恢复 | 需要把 BLE 睡眠动作从系统层剥离出来 |
| `application/service/output_service.h/.c` | 增加 Deep 关闭背光/指示灯和 Deep Wake 恢复流程 | Idle 不关灯，Deep 关灯 |
| `component/touch_component/touchpad_service.h/.c` | 增加 `touch_prepare_idle_sleep()`、`touch_prepare_deep_sleep()`、`touch_resume_from_sleep()` | 当前 `touch_power_off()` 路径不适合保留触控唤醒 |
| `drivers/input/keyboard/matrix.c/.h` | 增加 `matrix_prepare_wakeup()`、`matrix_resume_from_sleep()`、`matrix_scan_once_after_wakeup()` | N0046 必须按 `ROW2COL` 重做矩阵唤醒 |
| `hal/system_hal.h` | 明确 `SYSTEM_WAKEUP_TIMER` 的平台语义；增加唤醒原因查询接口 | 避免把软件定时器误当成硬件唤醒 |
| `hal/platforms/ch584/_system_hal.c` | 补齐 `SYSTEM_WAKEUP_TIMER -> RTC` 映射、RTC 初始化前置条件检查、唤醒原因锁存、Sleep 参数说明 | 当前平台映射不完整 |
| `application/service/wireless_callbacks.c` | 去掉直接进入睡眠的路径，统一改为投递 OSAL 事件 | 保持所有关键动作都走 OSAL |
| `keyboards/n0046/config.h` | 增加 Idle/Deep 超时和行为配置项 | 方便后续调参与产品化 |

### 10.1 参考但不直接修改的文件

| 文件 | 用途 |
|---|---|
| `project/ch584m/platforms_HAL/SLEEP.c` | 作为 CH584 RTC 休眠和 `LowPower_Sleep(...)` 参数基线参考 |
| `project/ch584m/platforms_HAL/RTC.c` | 作为 RTC 触发寄存器写入、RTC IRQ 清标志、唤醒锁存的参考 |
| `docs/code_demo/KB04122-13A-WCH_code.xml` | 作为 04122-13A 实机工程的睡前准备、GPIO 唤醒、PA2 恢复参考 |

## 11. 风险与验证清单

| 风险点 | 必须验证的内容 |
|---|---|
| `LowPower_Sleep(...)` 参数选择 | Idle / Deep 是否都能稳定唤醒，BLE/时钟/中断是否恢复正常 |
| RTC 唤醒链路 | `SYSTEM_WAKEUP_TIMER` 是否真正通过 RTC 唤醒 MCU |
| 矩阵方向适配 | `ROW2COL` 下是否不会出现睡死、假唤醒、首键丢失 |
| PA2 恢复 | 唤醒后 PA2 是否需要且已经重新配置中断 |
| 触控唤醒 | 睡眠后触控 INT 是否仍可唤醒，恢复后是否正常工作 |
| Idle 灯效保持 | Idle 下背光和指示灯是否保持稳定，不受扫描定时器停止影响 |
| Deep 唤醒后业务解耦 | 唤醒后是否只恢复本地资源，并通过事件通知业务，而不是直接自动回连 |

## 12. 结论

本项目的睡眠实现必须同时满足三条主线：

1. 业务和架构层面，严格遵守“OSAL 事件调度 + service 自治 + HAL 不碰上层资源”。
2. 产品行为层面，严格区分 `Idle Sleep` 和 `Deep Sleep`：Idle 只停扫描和降无线功耗，不关灯；Deep 才断连、关灯、关背光。
3. 平台实现层面，严格参考 CH584 / 04122-13A 的 RTC、GPIO、PA2、`LowPower_Sleep(...)` 底层链路，防止因为寄存器或时序处理错误导致睡眠异常。

因此，后续实现时应当把“事件流正确”和“CH584 底层序列正确”放在同等优先级上。前者保证架构可维护，后者保证设备能稳定进入和退出睡眠。
