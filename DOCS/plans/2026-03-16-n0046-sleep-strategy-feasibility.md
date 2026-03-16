# N0046 睡眠方案可行性分析报告

> 分析日期：2026-03-16
> 参考文档：`DOCS/N0046_sleep_strategy.md`
> 参考代码：`DOCS/code_demo/KB04122-13A-WCH_code.xml`、`project/ch584m/platforms_HAL/SLEEP.c`、`project/ch584m/platforms_HAL/RTC.c`、`hal/platforms/ch584/_system_hal.c`
> 用户确认决策：Option A（HAL_SLEEP=1）、N0046 矩阵方向 COL2ROW

---

## 一、总体结论（金字塔顶层）

**设计方案整体可行，采用 Option A（HAL_SLEEP=1）后原有 4 个高风险问题全部化解，最终遗留 2 个需要在实现中落地的中风险项，以及 1 个需要修正的硬件配置项。**

CH584 的 GPIO + RTC + `LowPower_Sleep()` 睡眠唤醒链路经过 04122-13A 实机验证，是可靠的基础。设计文档的 OSAL 事件驱动 + service 自治 + prepare 位图汇聚架构合理，与 WCH TMOS 的调度模型能够协调配合。

---

## 二、CH584 休眠/唤醒完整流程（HAL_SLEEP=1）

### 2.1 底层链路全景

启用 `HAL_SLEEP=1` 后，WCH TMOS 在 BLE 空闲窗口自动调用 `CH58x_LowPower()`，完整链路如下：

```
【初始化阶段】
HAL_Init()
  └── HAL_SleepInit()
        ├── sys_safe_access: R8_SLP_WAKE_CTRL |= RB_SLP_RTC_WAKE   // 允许 RTC 唤醒
        ├── sys_safe_access: R8_RTC_MODE_CTRL |= RB_RTC_TRIG_EN    // 触发模式
        └── PFIC_EnableIRQ(RTC_IRQn)                               // 使能 RTC 中断

【运行阶段 - BLE 空闲窗口触发】
TMOS 调度器 → cfg.idleCB → CH58x_LowPower(next_event_time)
  ├── 计算提前唤醒时间 time_tign = time - WAKE_UP_RTC_MAX_TIME
  ├── 检查 time_sleep: 若 < SLEEP_RTC_MIN_TIME 或 > SLEEP_RTC_MAX_TIME 则跳过
  ├── RTC_SetTignTime(time_tign)          // 设置 RTC 触发时间
  ├── 检查 !RTCTigFlag                    // 防止已超时
  ├── LowPower_Sleep(RB_PWR_RAM32K | RB_PWR_RAM96K | RB_PWR_EXTEND | RB_XT_PRE_EN)
  │     // MCU 进入低功耗，TMOS 挂起
  └── HSECFG_Current(HSE_RCur_100)       // 唤醒后恢复 HSE 驱动电流（关键！）

【唤醒阶段】
GPIO 中断（矩阵行/PA2）或 RTC 中断 → LowPower_Sleep() 返回
  ├── RTC_IRQHandler: 清 RTC 标志，RTCTigFlag = 1
  ├── GPIO ISR: 锁存唤醒原因，投递 SYSTEM_LPM_WAKE_EVT
  └── TMOS 调度器恢复，处理 OSAL 事件队列
```

### 2.2 Application LPM 状态机与 TMOS 的协作关系

| 层级 | 职责 | 实际执行者 |
|---|---|---|
| LPM 状态机（Application） | 管理 Idle/Deep 状态，调度 prepare/resume 事件 | `lpm.c` + `system_service.c` |
| 外设 prepare（Service） | 停扫描定时器、配置矩阵 GPIO 唤醒、BLE 低功耗/断连、关灯 | `input/communication/output_service` |
| 实际 MCU 睡眠（TMOS HAL） | 由 `CH58x_LowPower()` 在下一个 BLE 空闲窗口自动执行 | WCH HAL（`SLEEP.c`） |
| 唤醒恢复（ISR + Service） | GPIO ISR 投递事件，各 service 恢复本地资源 | ISR + service |

**核心理解：** LPM 状态机不直接调用 `LowPower_Sleep()`。`SYSTEM_LPM_ENTER_IDLE_EVT` / `SYSTEM_LPM_ENTER_DEEP_EVT` 的作用是"标记所有外设 prepare 已完成，MCU 可以安全睡眠"，真正的睡眠指令由 TMOS 在下一个空闲窗口触发。

---

## 三、原设计文档风险分析（更新版）

### 3.1 高风险项（Option A 后全部化解）

| 风险 | 描述 | Option A 后状态 | 解法 |
|---|---|---|---|
| P1 - HSE 恢复缺失 | `_system_hal.c` 唤醒后未调用 `HSECFG_Current(HSE_RCur_100)` | **已化解** | `CH58x_LowPower()` 已内置此恢复 |
| P2 - 缺少 `RB_XT_PRE_EN` | Idle 睡眠参数不含晶振预热位 | **已化解** | `CH58x_LowPower()` 使用完整参数组 |
| P3 - RTC 唤醒链路不完整 | `HAL_SLEEP=0` 下 `HAL_SleepInit()` 不执行，RTC 未初始化 | **已化解** | `HAL_SLEEP=1` 后 `HAL_Init()` 自动调用 `HAL_SleepInit()` |
| P4 - TMOS 调度冲突 | 在 TMOS 事件中直接调 `LowPower_Sleep()` 绕过调度器 | **已化解** | TMOS 通过 `idleCB` 自主管理睡眠时机 |

### 3.2 中风险项（仍需落地实现）

| 风险 | 描述 | 影响程度 | 解决方案 |
|---|---|---|---|
| P5 - 矩阵方向 | 设计文档误写 N0046 为 ROW2COL | **已消除** | 用户确认为 COL2ROW，与 04122-13A 一致，`product_config.h` 需修正 |
| P6 - 跨层调用 | `system_service.c` 当前在 Deep Sleep 事件中直接调 `indicator_off_all()`、`wireless_disconnect()` 等 | 中 | 按设计文档改为 prepare 事件扇出，替换现有事件处理逻辑 |
| P7 - PA2 恢复顺序 | 唤醒后必须按正确顺序恢复 PA2 中断，易遗漏或顺序错误 | 中 | 在 `input_service` 的 resume 流程中强制执行 `power_key_resume()` 三步序列 |

### 3.3 配置项修正

| 项目 | 当前值 | 修正为 | 文件 |
|---|---|---|---|
| `HAL_SLEEP` | `0` (FALSE) | `1` (TRUE) | `project/ch584m/platforms_HAL/include/CONFIG.h` |
| `DIODE_DIRECTION` | `ROW2COL`（设计文档描述有误） | `COL2ROW` | `keyboards/n0046/config.h` 或 `keyboards/product_config.h` |

---

## 四、CH584 睡眠/唤醒关键硬件约束（实现参考）

### 4.1 GPIO 唤醒准备顺序（必须严格遵守）

```
// 正确顺序（参考 04122-13A peripheral_enter_sleep()）
1. 关闭耗电外设（ADC、LED PWM）
2. 未使用 GPIO 设为上拉/下拉输入（防悬空漏电）
3. 矩阵 COL 引脚：先置高，再配置为推挽输出，再逐列拉低
4. 清除矩阵 ROW 引脚的 GPIO 中断标志位
5. 清除 PA2 中断标志位
6. 配置 PA2 为输入上拉 + 下降沿中断
7. 配置矩阵 ROW 引脚为下降沿中断
8. PFIC_ClearPendingIRQ(GPIO_A_IRQn / GPIO_B_IRQn)
9. PFIC_EnableIRQ(GPIO_A_IRQn / GPIO_B_IRQn)
10. PWR_PeriphWakeUpCfg(ENABLE, RB_SLP_GPIO_WAKE, Long_Delay)
// 之后由 TMOS 在空闲窗口触发 LowPower_Sleep()
```

**关键：步骤 4-5 清标志必须在步骤 6-7 配置中断之后，步骤 9 使能 IRQ 之前。**

### 4.2 PA2 唤醒后恢复三步序列（power_key_resume 等价）

```c
// 唤醒后必须执行，顺序不可颠倒
GPIOA_ModeCfg(GPIO_Pin_2, GPIO_ModeIN_PU);           // 1. 重置为输入上拉
GPIOA_ITModeCfg(GPIO_Pin_2, GPIO_ITMode_FallEdge);   // 2. 配置下降沿中断
GPIOA_ClearITFlagBit(GPIO_Pin_2);                    // 3. 清除中断标志
```

### 4.3 RTC 安全访问规范

```c
// 写 RTC 触发寄存器的正确方式（RTC_SetTignTime）
sys_safe_access_enable();
R32_RTC_TRIG = time;
sys_safe_access_disable();
RTCTigFlag = 0;  // 清除标志
```

### 4.4 睡眠参数基线（HAL_SLEEP=1 路径，勿改动）

```c
// CH58x_LowPower() 中使用的已验证参数组
LowPower_Sleep(RB_PWR_RAM32K | RB_PWR_RAM96K | RB_PWR_EXTEND | RB_XT_PRE_EN);
HSECFG_Current(HSE_RCur_100);  // 退出后必须恢复
```

**首版禁止改动此参数组。** `RB_XT_PRE_EN` 保证 BLE 时钟恢复，`RB_PWR_EXTEND` 保留 BLE 协议栈 RAM。

---

## 五、修正后的实现清单

在原设计文档（`N0046_sleep_strategy.md`第10节）基础上，新增/修正以下内容：

| 文件 | 操作类型 | 具体内容 |
|---|---|---|
| `project/ch584m/platforms_HAL/include/CONFIG.h` | **修改** | `HAL_SLEEP` 从 `0` 改为 `1` |
| `keyboards/n0046/config.h` | **修改** | `DIODE_DIRECTION` 从 `ROW2COL` 改为 `COL2ROW` |
| `hal/platforms/ch584/_system_hal.c` | **修改** | 移除 `system_hal_enter_sleep()` 中直接调用 `LowPower_Sleep()` 的路径（该路径由 TMOS HAL 接管）；保留唤醒源配置接口供 `input_service` 调用 |
| `middleware/communication/lpm.c` | **新增** | Idle/Deep 状态枚举、5s/10min 超时逻辑、`lpm_note_activity()`、`prepare_pending_mask`/`prepare_done_mask` 位图 |
| `application/service/system_service.c` | **重构** | 删除 `SYSTEM_DEEP_SLEEP_EVT` 中的跨层调用；实现 LPM 事件调度（IDLE_REQ/DEEP_REQ/STEP_DONE/ENTER_IDLE/ENTER_DEEP/WAKE）|
| `application/service/input_service.c` | **新增** | `INPUT_LPM_PREPARE_IDLE_EVT`、`INPUT_LPM_PREPARE_DEEP_EVT`、`INPUT_LPM_RESUME_IDLE_EVT`、`INPUT_LPM_RESUME_DEEP_EVT` 处理；`power_key_resume()` 等价逻辑 |
| `application/service/communication_service.c` | **新增** | BLE Idle 低功耗 prepare、Deep 断连 prepare、唤醒后本地上下文恢复 |
| `application/service/output_service.c` | **新增** | `OUTPUT_LPM_PREPARE_DEEP_EVT`（关背光/指示灯）、`OUTPUT_LPM_RESUME_DEEP_EVT` |
| `drivers/input/keyboard/matrix.c` | **新增** | `matrix_prepare_wakeup()`（COL 拉低，ROW 配中断）、`matrix_resume_from_sleep()`、`matrix_scan_once_after_wakeup()` |

**不再需要修改的文件（Option A 后）：**
- `hal/platforms/ch584/_system_hal.c` 无需补 HSE 恢复（`CH58x_LowPower()` 已处理）
- `hal/system_hal.h` 的 `SYSTEM_WAKEUP_TIMER` 抽象仍可保留，但 HAL 层无需特殊实现（RTC 由 TMOS 自动管理）

---

## 六、验证清单（首版上机必验）

| 验证项 | 验证方式 | 通过标准 |
|---|---|---|
| HAL_SLEEP=1 基础睡眠 | 编译烧录，连接 BLE，空闲等待 | 电流表显示 MCU 周期性进入低功耗（BLE 连接间隔期间） |
| 矩阵 GPIO 唤醒 | 进入 Idle，按任意键 | 系统唤醒，按键事件正常上报 |
| PA2 电源键唤醒 | 进入 Idle/Deep，按电源键 | 系统唤醒，电源键逻辑正常 |
| RTC 定时推进 | 进入 Idle，等待 10min | 系统自动推进到 Deep，日志确认 `SYSTEM_LPM_DEEP_REQ_EVT` 触发 |
| BLE 连接 Idle 不断连 | 进入 Idle，BLE 主机观察 | 连接保持，无断连日志 |
| BLE 深睡断连恢复 | 进入 Deep，唤醒后按键 | 设备重新广播，主机可重新连接 |
| 背光/指示灯行为 | 进入 Idle，观察灯效 | 背光和指示灯保持不变（不熄灭） |
| Deep 关灯 | 进入 Deep | 背光和指示灯全灭 |
| PA2 唤醒后中断恢复 | 唤醒后多次按电源键 | 每次按下均正确响应，无失响 |
| 矩阵首键不丢失 | 唤醒后立刻快速输入 | 唤醒触发的那次按键上报正常，不丢首键 |

---

## 七、设计文档 N0046_sleep_strategy.md 需要修订的内容

以下是原设计文档中需要更正的描述，供文档维护参考：

| 位置 | 原描述 | 修正为 |
|---|---|---|
| 第 3.2 节 | "N0046 是 `ROW2COL`，不能照抄 04122-13A 矩阵方向" | N0046 采用 COL2ROW，矩阵 prepare 方向与 04122-13A 一致 |
| 第 5.4 节步骤 12 | `system_hal_enter_sleep(SYSTEM_POWER_MODE_SLEEP, wake_mask)` | Option A 下此步骤改为"确认 prepare 完成，等待 TMOS idleCB 触发睡眠" |
| 第 6.4 节步骤 13 | `system_hal_enter_sleep(SYSTEM_POWER_MODE_DEEP_SLEEP, wake_mask)` | 同上，睡眠由 TMOS 自动触发 |
| 第 8.3 节 | "Idle 参数基线：以 04122/WCH 已验证参数组为基线" | 明确为 `CH58x_LowPower()` 中的固定参数组，不通过 `system_hal_enter_sleep()` 传入 |
| 全文 HAL_SLEEP | 未提及需开启 | 明确要求：必须在 `CONFIG.h` 中将 `HAL_SLEEP` 设为 `TRUE` |
