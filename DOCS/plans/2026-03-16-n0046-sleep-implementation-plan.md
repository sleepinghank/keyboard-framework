# N0046 Sleep Strategy 实施计划

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 N0046/CH584M 实现完整的 Idle Sleep（5s）和 Deep Sleep（10min）低功耗方案，基于 TMOS `HAL_SLEEP=1` 机制，采用 OSAL 事件驱动 + service 自治架构。

**Architecture:** 启用 `HAL_SLEEP=TRUE` 后 TMOS 通过 `CH58x_LowPower()` idleCB 自动管理 MCU 睡眠时机；本框架的 LPM 状态机负责 prepare/resume 事件调度，各 service 独立处理自己的外设；GPIO ISR 仅锁存唤醒原因并投递 `SYSTEM_LPM_WAKE_EVT`，不在 ISR 中做复杂操作。

**Tech Stack:** CH584M / WCH TMOS / OSAL / C99

**参考文档：**
- 设计：`DOCS/N0046_sleep_strategy.md`
- 可行性分析：`DOCS/plans/2026-03-16-n0046-sleep-strategy-feasibility.md`
- 参考代码：`DOCS/code_demo/KB04122-13A-WCH_code.xml`

---

## 文件修改总表

| 文件 | 操作 | 说明 |
|---|---|---|
| `project/ch584m/platforms_HAL/include/CONFIG.h` | 修改 | `HAL_SLEEP FALSE → TRUE` |
| `keyboards/product_config.h` | 修改 | `DIODE_DIRECTION ROW2COL → COL2ROW` |
| `middleware/communication/lpm.h` | 重写 | 新增 LPM 状态机类型、prepare 位图、接口声明 |
| `middleware/communication/lpm.c` | 重写 | 完整 LPM 状态机实现，5s/10min 超时逻辑 |
| `application/service/system_service.h` | 修改 | 新增 6 个 LPM 事件到 `system_task_event_t` |
| `application/service/system_service.c` | 重构 | 删除跨层调用；实现 LPM 事件调度与 prepare 位图汇聚 |
| `drivers/input/keyboard/matrix.h` | 修改 | 新增 `matrix_prepare_wakeup()`、`matrix_resume_from_sleep()`、`matrix_scan_once_after_wakeup()` 声明 |
| `drivers/input/keyboard/matrix.c` | 修改 | 实现上述三个函数 |
| `application/service/input_service.h` | 修改 | 新增 2 个 LPM 事件；新增 `lpm_wakeup_source_t` 枚举 |
| `application/service/input_service.c` | 修改 | 实现 PREPARE/RESUME 事件处理；添加 GPIO ISR wakeup 事件投递 |
| `application/service/communication_service.h` | 修改 | 新增 2 个 LPM 事件（使用最后 2 个可用 bit） |
| `application/service/communication_service.c` | 修改 | 实现 BLE Idle 低功耗/Deep 断连的 prepare 和 resume 处理 |
| `application/service/output_service.h` | 修改 | 新增 2 个 LPM 事件 |
| `application/service/output_service.c` | 修改 | 实现 Deep 关灯/唤醒恢复灯效的事件处理 |
| `middleware/communication/wireless_callbacks.c` | 修改 | 删除 `access_ble_enter_idel_sleep()` 中的直接睡眠调用，改为投递 OSAL 事件 |
| `hal/platforms/ch584/_system_hal.c` | 修改 | 移除 `system_hal_enter_sleep()` 中直接调用 `LowPower_Sleep()` 的分支；仅保留唤醒源配置 |

---

## Chunk 1：前置配置修正（编译前提）

这两处是后续所有工作的硬性前提，必须最先修改并确认编译通过。

### Task 1：启用 HAL_SLEEP

**文件：**
- 修改：`project/ch584m/platforms_HAL/include/CONFIG.h:83-84`

**背景：** `HAL_SLEEP` 控制 WCH TMOS 的 BLE 低功耗执行链路。`FALSE` 时 `HAL_SleepInit()`、`cfg.idleCB = CH58x_LowPower` 注册、`CH58x_LowPower()` 核心逻辑均不生效。整套睡眠方案依赖此开关为 `TRUE`。

- [ ] **Step 1.1：确认当前值**

  打开 `project/ch584m/platforms_HAL/include/CONFIG.h`，确认第 83-84 行：
  ```c
  #ifndef HAL_SLEEP
  #define HAL_SLEEP    FALSE
  #endif
  ```

- [ ] **Step 1.2：修改为 TRUE**

  将第 84 行改为：
  ```c
  #define HAL_SLEEP    TRUE
  ```

- [ ] **Step 1.3：验证副作用——确认 `HAL_Init()` 调用链**

  打开 `project/ch584m/platforms_HAL/MCU.c`，确认存在以下路径：
  ```c
  // 期望看到类似：
  #if(defined(HAL_SLEEP)) && (HAL_SLEEP == TRUE)
      cfg.idleCB = CH58x_LowPower;    // BLE 空闲回调
  #endif
  // 以及 HAL_Init() 中：
  #if(defined(HAL_SLEEP)) && (HAL_SLEEP == TRUE)
      HAL_SleepInit();
  #endif
  ```
  目的是确认修改后这两处路径会被编译进去。

- [ ] **Step 1.4：编译验证**

  使用 WCH 工具链编译（参考 CLAUDE.md 的 WCH 编译命令），确认无编译错误，输出 `keyboard-framework.hex`。
  ```
  预期结果：编译成功，无 HAL_SLEEP 相关 warning
  ```

- [ ] **Step 1.5：提交**
  ```
  git add project/ch584m/platforms_HAL/include/CONFIG.h
  git commit -m "config(ch584): enable HAL_SLEEP for TMOS BLE low-power callback"
  ```

---

### Task 2：修正矩阵扫描方向

**文件：**
- 修改：`keyboards/product_config.h:20`

**背景：** 硬件确认 N0046 为 COL2ROW（COL 引脚为输出驱动，ROW 引脚为输入读取）。`matrix.c` 的扫描逻辑（`select_col()` → `read row pins`）也是 COL2ROW 风格。当前 `product_config.h` 的 `ROW2COL` 是错误配置，会影响 HID 键码映射和睡眠时矩阵 GPIO 方向的理解。

- [ ] **Step 2.1：确认当前值**

  打开 `keyboards/product_config.h`，确认第 20 行：
  ```c
  #define DIODE_DIRECTION ROW2COL
  ```

- [ ] **Step 2.2：修改为 COL2ROW**

  ```c
  #define DIODE_DIRECTION COL2ROW
  ```

- [ ] **Step 2.3：确认 matrix.c 不使用 DIODE_DIRECTION 宏做条件编译**

  执行：
  ```bash
  grep -n "DIODE_DIRECTION\|COL2ROW\|ROW2COL" drivers/input/keyboard/matrix.c
  ```
  预期：无输出（matrix.c 中已实现 COL2ROW 扫描，不通过宏切换）。

- [ ] **Step 2.4：编译验证**

  编译确认无错误。若有条件编译报错，检查其他引用 `DIODE_DIRECTION` 的文件并修正。

- [ ] **Step 2.5：提交**
  ```
  git add keyboards/product_config.h
  git commit -m "config(n0046): fix DIODE_DIRECTION to COL2ROW matching hardware"
  ```

---

## Chunk 2：LPM 状态机重构

### Task 3：重构 lpm.h / lpm.c

**文件：**
- 重写：`middleware/communication/lpm.h`
- 重写：`middleware/communication/lpm.c`

**背景：** 当前 `lpm.h` 只有简单的 `pm_t` 枚举和 4 个函数。`lpm.c` 的核心逻辑被注释。需要完全重写，引入：
- `lpm_state_t` 六状态机（ACTIVE / IDLE_PENDING / IDLE_SLEEP / DEEP_PENDING / DEEP_SLEEP / WAKE_RESUME）
- `lpm_mode_t`（区分 Idle 和 Deep 两种深度）
- `prepare_pending_mask` / `prepare_done_mask`（16-bit 位图，按 service 分配）
- 5s Idle 超时 / 10min Deep 超时逻辑
- `lpm_note_activity()` 重置超时
- `lpm_get_state()` / `lpm_get_mode()` 供 service 查询

**注意：** `enter_power_mode()` 和 `pm_t` 仍需保留，供 `system_service.c` 中的关机/低电等非睡眠路径使用，但 `PM_SLEEP` / `PM_STANDBY_WITH_RAM` 对应的 LPM 路径由新状态机接管，不再通过 `enter_power_mode()` 触发。

- [ ] **Step 3.1：明确 prepare 位图分配**

  ```c
  // 准备好位图常量（写在 lpm.h 中）
  #define LPM_PREPARE_INPUT    (1 << 0)   // input_service
  #define LPM_PREPARE_COMMU    (1 << 1)   // communication_service
  #define LPM_PREPARE_OUTPUT   (1 << 2)   // output_service

  // Idle prepare 需要等待：input + commu（output 无操作）
  #define LPM_IDLE_PREPARE_MASK  (LPM_PREPARE_INPUT | LPM_PREPARE_COMMU)
  // Deep prepare 需要等待：input + commu + output
  #define LPM_DEEP_PREPARE_MASK  (LPM_PREPARE_INPUT | LPM_PREPARE_COMMU | LPM_PREPARE_OUTPUT)
  ```

- [ ] **Step 3.2：编写新的 lpm.h**

  完整接口声明如下（替换现有 `lpm.h` 全部内容）：

  ```c
  #pragma once
  #include <stdbool.h>
  #include <stdint.h>

  /* ---- 保留：非睡眠类电源模式（关机/低电路径仍使用） ---- */
  typedef enum {
      PM_RUN, PM_LOW_POWER_RUN, PM_SLEEP, PM_LOW_POWER_SLEEP,
      PM_STOP0, PM_STOP1, PM_STOP2,
      PM_STANDBY_WITH_RAM, PM_STANDBY, PM_SHUTDOWN
  } pm_t;

  /* ---- 新增：LPM 状态 ---- */
  typedef enum {
      LPM_STATE_ACTIVE        = 0,
      LPM_STATE_IDLE_PENDING  = 1,
      LPM_STATE_IDLE_SLEEP    = 2,
      LPM_STATE_DEEP_PENDING  = 3,
      LPM_STATE_DEEP_SLEEP    = 4,
      LPM_STATE_WAKE_RESUME   = 5,
  } lpm_state_t;

  /* ---- 新增：睡眠深度（供 service 查询） ---- */
  typedef enum {
      LPM_MODE_NONE  = 0,
      LPM_MODE_IDLE  = 1,
      LPM_MODE_DEEP  = 2,
  } lpm_mode_t;

  /* ---- 新增：prepare 位图掩码 ---- */
  #define LPM_PREPARE_INPUT    (1 << 0)
  #define LPM_PREPARE_COMMU    (1 << 1)
  #define LPM_PREPARE_OUTPUT   (1 << 2)
  #define LPM_IDLE_PREPARE_MASK  (LPM_PREPARE_INPUT | LPM_PREPARE_COMMU)
  #define LPM_DEEP_PREPARE_MASK  (LPM_PREPARE_INPUT | LPM_PREPARE_COMMU | LPM_PREPARE_OUTPUT)

  /* ---- 超时配置（可在 product_config.h 中覆盖） ---- */
  #ifndef LPM_IDLE_TIMEOUT_MS
  #define LPM_IDLE_TIMEOUT_MS    5000UL       /* 5 秒进入 Idle */
  #endif
  #ifndef LPM_DEEP_TIMEOUT_MS
  #define LPM_DEEP_TIMEOUT_MS    600000UL     /* 10 分钟进入 Deep */
  #endif

  /* ---- 函数声明 ---- */

  /* 初始化（在 system_init_middleware() 中调用） */
  void lpm_init(void);

  /* 主循环任务（在 main loop 或 OSAL 周期任务中调用） */
  void lpm_task(void);

  /* 活动记录：键盘扫描/触控/报文发送时调用，重置超时计时器 */
  void lpm_note_activity(void);

  /* 超时计时器接口（兼容旧代码） */
  void lpm_timer_reset(void);
  void lpm_timer_stop(void);

  /* 状态查询 */
  lpm_state_t lpm_get_state(void);
  lpm_mode_t  lpm_get_mode(void);
  bool        lpm_is_in_sleep(void);

  /* prepare 位图操作（由各 service 在 prepare 完成后调用） */
  void lpm_mark_prepare_done(uint8_t prepare_bit);
  bool lpm_all_prepare_done(void);

  /* 状态强制切换（由 system_service 调用） */
  void lpm_set_state(lpm_state_t state);

  /* 禁止进入低功耗（USB 活动/OTA 等场景） */
  void lpm_inhibit(bool inhibit);
  bool lpm_is_inhibited(void);

  /* 兼容旧接口 */
  __attribute__((weak)) void enter_power_mode(pm_t mode);
  __attribute__((weak)) bool usb_power_connected(void);
  ```

- [ ] **Step 3.3：编写新的 lpm.c**

  关键实现逻辑如下（替换现有 `lpm.c` 全部内容）：

  ```c
  #include "lpm.h"
  #include "timer.h"
  #include "system_service.h"    // SYSTEM_LPM_IDLE_REQ_EVT 等
  #include "event_manager.h"
  #include <string.h>

  /* ---- 内部状态 ---- */
  static lpm_state_t  g_lpm_state          = LPM_STATE_ACTIVE;
  static lpm_mode_t   g_lpm_mode           = LPM_MODE_NONE;
  static uint8_t      g_prepare_pending    = 0;
  static uint8_t      g_prepare_done       = 0;
  static uint32_t     g_last_activity_ms   = 0;
  static bool         g_inhibited          = false;

  void lpm_init(void) {
      g_lpm_state        = LPM_STATE_ACTIVE;
      g_lpm_mode         = LPM_MODE_NONE;
      g_prepare_pending  = 0;
      g_prepare_done     = 0;
      g_inhibited        = false;
      lpm_timer_reset();
  }

  void lpm_note_activity(void) {
      g_last_activity_ms = timer_read32();
      /* 若处于 PENDING 状态收到活动，取消本轮睡眠 */
      if (g_lpm_state == LPM_STATE_IDLE_PENDING ||
          g_lpm_state == LPM_STATE_DEEP_PENDING) {
          g_lpm_state = LPM_STATE_ACTIVE;
          g_prepare_pending = 0;
          g_prepare_done = 0;
      }
  }

  void lpm_timer_reset(void) {
      g_last_activity_ms = timer_read32();
  }

  void lpm_timer_stop(void) {
      g_last_activity_ms = 0;
  }

  lpm_state_t lpm_get_state(void) { return g_lpm_state; }
  lpm_mode_t  lpm_get_mode(void)  { return g_lpm_mode; }
  bool        lpm_is_in_sleep(void) {
      return g_lpm_state == LPM_STATE_IDLE_SLEEP ||
             g_lpm_state == LPM_STATE_DEEP_SLEEP;
  }

  void lpm_set_state(lpm_state_t state) {
      g_lpm_state = state;
  }

  void lpm_mark_prepare_done(uint8_t prepare_bit) {
      g_prepare_done |= prepare_bit;
  }

  bool lpm_all_prepare_done(void) {
      return (g_prepare_done & g_prepare_pending) == g_prepare_pending;
  }

  void lpm_inhibit(bool inhibit) {
      g_inhibited = inhibit;
  }

  bool lpm_is_inhibited(void) {
      return g_inhibited;
  }

  void lpm_task(void) {
      extern uint8_t system_taskID;

      if (g_lpm_state != LPM_STATE_ACTIVE) return;
      if (g_inhibited) return;
      if (g_last_activity_ms == 0) return;

      uint32_t elapsed = timer_elapsed32(g_last_activity_ms);

      if (elapsed >= LPM_DEEP_TIMEOUT_MS) {
          /* 直接推进到 Deep（跳过 Idle pending） */
          g_lpm_state = LPM_STATE_ACTIVE;   /* 由 system_service 接管 */
          g_lpm_mode  = LPM_MODE_DEEP;
          g_prepare_pending = LPM_DEEP_PREPARE_MASK;
          g_prepare_done    = 0;
          OSAL_SetEvent(system_taskID, SYSTEM_LPM_DEEP_REQ_EVT);
      } else if (elapsed >= LPM_IDLE_TIMEOUT_MS) {
          g_lpm_state = LPM_STATE_ACTIVE;
          g_lpm_mode  = LPM_MODE_IDLE;
          g_prepare_pending = LPM_IDLE_PREPARE_MASK;
          g_prepare_done    = 0;
          OSAL_SetEvent(system_taskID, SYSTEM_LPM_IDLE_REQ_EVT);
      }
  }

  __attribute__((weak)) void enter_power_mode(pm_t mode) { (void)mode; }
  __attribute__((weak)) bool usb_power_connected(void)   { return true; }
  ```

  > **注意：** `lpm_task()` 需要在主循环中被周期性调用。检查 `test_main.c` 或 OSAL 主任务中是否已调用 `lpm_task()`，若无则需要添加。

- [ ] **Step 3.4：检查 lpm_task() 调用点**

  ```bash
  grep -rn "lpm_task\|lpm_init" application/ test_main.c
  ```
  确认 `lpm_init()` 在 `system_init_middleware()` 中被调用，`lpm_task()` 在主循环中被调用。若缺失，记录需要补充的调用位置（不在此 Task 修改代码，仅记录）。

- [ ] **Step 3.5：编译验证**

  编译确认 lpm.h / lpm.c 无错误，新符号可被其他模块引用。

- [ ] **Step 3.6：提交**
  ```
  git add middleware/communication/lpm.h middleware/communication/lpm.c
  git commit -m "feat(lpm): rewrite LPM state machine with Idle/Deep states and prepare bitmap"
  ```

---

## Chunk 3：system_service LPM 事件调度

### Task 4：扩展 system_service.h 事件定义

**文件：**
- 修改：`application/service/system_service.h`

**背景：** 当前 `system_task_event_t` 用 bits 0-7（共 8 个事件）。OSAL 每个任务支持 uint16_t，还有 bits 8-15 可用。需要新增 6 个 LPM 调度事件。

- [ ] **Step 4.1：在 `system_task_event_t` 中追加 LPM 事件**

  在枚举末尾追加（`SYSTEM_OTA_EVT = 1 << 7` 之后）：

  ```c
  /* LPM 调度事件（bits 8-13，不与现有事件冲突） */
  SYSTEM_LPM_IDLE_REQ_EVT   = 1 << 8,   /**< 请求进入 Idle（由 lpm_task 投递） */
  SYSTEM_LPM_DEEP_REQ_EVT   = 1 << 9,   /**< 请求进入 Deep（由 lpm_task 投递） */
  SYSTEM_LPM_STEP_DONE_EVT  = 1 << 10,  /**< prepare 完成汇聚（由各 service 投递） */
  SYSTEM_LPM_ENTER_IDLE_EVT = 1 << 11,  /**< 最终进入 Idle（自投递，做二次确认） */
  SYSTEM_LPM_ENTER_DEEP_EVT = 1 << 12,  /**< 最终进入 Deep（自投递，做二次确认） */
  SYSTEM_LPM_WAKE_EVT       = 1 << 13,  /**< 唤醒恢复入口（由 GPIO ISR 投递） */
  ```

- [ ] **Step 4.2：编译验证**

  编译确认 `system_service.h` 无错误。

- [ ] **Step 4.3：提交**
  ```
  git add application/service/system_service.h
  git commit -m "feat(system_service): add LPM event definitions (bits 8-13)"
  ```

---

### Task 5：重构 system_service.c LPM 调度逻辑

**文件：**
- 修改：`application/service/system_service.c`

**背景：** 当前 `system_service.c` 在 `SYSTEM_DEEP_SLEEP_EVT` 和 `SYSTEM_IDLE_EVT` 中直接调用 `indicator_off_all()`、`wireless_disconnect()`、`bt_driver_set_advertising()` 等上层接口，违反层级隔离原则。需要重构为纯调度模式（只投递事件，不直接操作外设）。

- [ ] **Step 5.1：删除 SYSTEM_IDLE_EVT 中的跨层操作**

  当前代码（第 47-54 行）：
  ```c
  if (events & SYSTEM_IDLE_EVT) {
      indicator_off_all();          // 跨层：直接操作输出
      enter_power_mode(PM_SLEEP);   // 旧路径，由 LPM 状态机接管
      return (events ^ SYSTEM_IDLE_EVT);
  }
  ```

  修改为（移除内部实现，改为记录日志，实际触发由 lpm_task() 负责）：
  ```c
  if (events & SYSTEM_IDLE_EVT) {
      /* 此事件已由 SYSTEM_LPM_IDLE_REQ_EVT 替代，保留为空以兼容旧调用 */
      return (events ^ SYSTEM_IDLE_EVT);
  }
  ```

- [ ] **Step 5.2：删除 SYSTEM_DEEP_SLEEP_EVT 中的跨层操作**

  当前代码（第 72-87 行）直接调用 `bt_driver_set_advertising(false)`、`wireless_disconnect()`、`indicator_off_all()`，替换为纯调度：
  ```c
  if (events & SYSTEM_DEEP_SLEEP_EVT) {
      /* 转发为 LPM Deep 请求，由 LPM 状态机处理 */
      println("System: Deep sleep requested via legacy event");
      lpm_note_activity();          /* 先重置，让 lpm_task 重新计时 */
      /* 直接触发 Deep req（忽略超时，立即推进） */
      lpm_set_state(LPM_STATE_ACTIVE);
      OSAL_SetEvent(system_taskID, SYSTEM_LPM_DEEP_REQ_EVT);
      return (events ^ SYSTEM_DEEP_SLEEP_EVT);
  }
  ```

- [ ] **Step 5.3：新增 SYSTEM_LPM_IDLE_REQ_EVT 处理**

  ```c
  if (events & SYSTEM_LPM_IDLE_REQ_EVT) {
      println("System: Idle sleep requested");
      lpm_set_state(LPM_STATE_IDLE_PENDING);
      /* 扇出 prepare 事件到各 service */
      OSAL_SetEvent(input_taskID,  INPUT_LPM_PREPARE_EVT);
      OSAL_SetEvent(commu_taskID,  COMMU_LPM_PREPARE_EVT);
      /* output_service Idle 无操作，直接标记完成 */
      lpm_mark_prepare_done(LPM_PREPARE_OUTPUT);
      return (events ^ SYSTEM_LPM_IDLE_REQ_EVT);
  }
  ```

- [ ] **Step 5.4：新增 SYSTEM_LPM_DEEP_REQ_EVT 处理**

  ```c
  if (events & SYSTEM_LPM_DEEP_REQ_EVT) {
      println("System: Deep sleep requested");
      lpm_set_state(LPM_STATE_DEEP_PENDING);
      OSAL_SetEvent(input_taskID,  INPUT_LPM_PREPARE_EVT);
      OSAL_SetEvent(commu_taskID,  COMMU_LPM_PREPARE_EVT);
      OSAL_SetEvent(output_taskID, OUTPUT_LPM_PREPARE_EVT);
      return (events ^ SYSTEM_LPM_DEEP_REQ_EVT);
  }
  ```

- [ ] **Step 5.5：新增 SYSTEM_LPM_STEP_DONE_EVT 处理（prepare 汇聚）**

  ```c
  if (events & SYSTEM_LPM_STEP_DONE_EVT) {
      lpm_state_t cur = lpm_get_state();
      if (!lpm_all_prepare_done()) {
          /* 还有 service 未完成，继续等待 */
          return (events ^ SYSTEM_LPM_STEP_DONE_EVT);
      }
      /* 全部完成，投递最终进入事件（拆开汇聚与执行） */
      if (cur == LPM_STATE_IDLE_PENDING) {
          OSAL_SetEvent(system_taskID, SYSTEM_LPM_ENTER_IDLE_EVT);
      } else if (cur == LPM_STATE_DEEP_PENDING) {
          OSAL_SetEvent(system_taskID, SYSTEM_LPM_ENTER_DEEP_EVT);
      }
      return (events ^ SYSTEM_LPM_STEP_DONE_EVT);
  }
  ```

- [ ] **Step 5.6：新增 SYSTEM_LPM_ENTER_IDLE_EVT 处理（二次确认）**

  ```c
  if (events & SYSTEM_LPM_ENTER_IDLE_EVT) {
      /* 二次确认：若有新活动则取消本次睡眠 */
      if (lpm_get_state() != LPM_STATE_IDLE_PENDING) {
          println("System: Idle enter cancelled (state changed)");
          return (events ^ SYSTEM_LPM_ENTER_IDLE_EVT);
      }
      println("System: Entering Idle sleep, waiting TMOS idleCB");
      lpm_set_state(LPM_STATE_IDLE_SLEEP);
      /* Option A：MCU 睡眠由 TMOS CH58x_LowPower() idleCB 自动触发，此处无需调用 LowPower_Sleep() */
      return (events ^ SYSTEM_LPM_ENTER_IDLE_EVT);
  }
  ```

- [ ] **Step 5.7：新增 SYSTEM_LPM_ENTER_DEEP_EVT 处理（二次确认）**

  ```c
  if (events & SYSTEM_LPM_ENTER_DEEP_EVT) {
      if (lpm_get_state() != LPM_STATE_DEEP_PENDING) {
          println("System: Deep enter cancelled (state changed)");
          return (events ^ SYSTEM_LPM_ENTER_DEEP_EVT);
      }
      println("System: Entering Deep sleep, waiting TMOS idleCB");
      lpm_set_state(LPM_STATE_DEEP_SLEEP);
      return (events ^ SYSTEM_LPM_ENTER_DEEP_EVT);
  }
  ```

- [ ] **Step 5.8：新增 SYSTEM_LPM_WAKE_EVT 处理（统一唤醒恢复入口）**

  ```c
  if (events & SYSTEM_LPM_WAKE_EVT) {
      println("System: Wake resume start");
      lpm_set_state(LPM_STATE_WAKE_RESUME);
      lpm_mode_t mode = lpm_get_mode();

      /* 先恢复输入侧（最关键，需要最快响应） */
      OSAL_SetEvent(input_taskID, INPUT_LPM_RESUME_EVT);

      if (mode == LPM_MODE_DEEP) {
          /* Deep 唤醒需要恢复输出侧和通信侧 */
          OSAL_SetEvent(output_taskID, OUTPUT_LPM_RESUME_EVT);
          OSAL_SetEvent(commu_taskID,  COMMU_LPM_RESUME_EVT);
      }
      /* Idle 唤醒：通信侧保持（BLE 未断连），output 无需恢复 */

      lpm_set_state(LPM_STATE_ACTIVE);
      lpm_timer_reset();
      return (events ^ SYSTEM_LPM_WAKE_EVT);
  }
  ```

- [ ] **Step 5.9：确认 include 依赖完整**

  `system_service.c` 需要 include：
  ```c
  #include "lpm.h"          // lpm_set_state, lpm_note_activity, lpm_get_mode
  #include "input_service.h"   // INPUT_LPM_PREPARE_EVT, INPUT_LPM_RESUME_EVT
  #include "communication_service.h"  // COMMU_LPM_PREPARE_EVT
  #include "output_service.h"  // OUTPUT_LPM_PREPARE_EVT, OUTPUT_LPM_RESUME_EVT
  ```

  检查现有 include 是否已覆盖，按需补充。

- [ ] **Step 5.10：编译验证**

  编译确认无错误，注意 `input_taskID`、`commu_taskID`、`output_taskID` 均为 `extern` 可见。

- [ ] **Step 5.11：提交**
  ```
  git add application/service/system_service.c
  git commit -m "refactor(system_service): replace cross-layer sleep calls with LPM event dispatch"
  ```

---

## Chunk 4：矩阵唤醒接口

### Task 6：matrix.c / matrix.h 新增唤醒接口

**文件：**
- 修改：`drivers/input/keyboard/matrix.h`（新增函数声明）
- 修改：`drivers/input/keyboard/matrix.c`（新增函数实现）

**背景：** 睡眠前需要将矩阵切换为唤醒检测模式（COL 引脚拉低输出，ROW 引脚配下降沿中断）；唤醒后需要恢复正常扫描模式；第一次扫描需要手动触发一次以检测触发唤醒的那个按键。

**N0046 矩阵方向（已确认 COL2ROW）：**
- 正常扫描：COL 引脚逐个拉低（输出），ROW 引脚读取（输入上拉）
- 睡眠准备：COL 引脚全部拉低输出（保持），ROW 引脚改为下降沿中断

**参考：** `DOCS/code_demo/KB04122-13A-WCH_code.xml` 的 `peripheral_enter_sleep()` 第 10583-10660 行。

- [ ] **Step 6.1：在 matrix.h 添加函数声明**

  在 `matrix.h` 末尾添加：
  ```c
  /**
   * @brief 配置矩阵 GPIO 为睡眠唤醒模式
   *        COL 引脚：全部拉低输出
   *        ROW 引脚：下降沿中断 + 使能 GPIO IRQ
   *        最后开启 RB_SLP_GPIO_WAKE
   * @note  必须在所有 GPIO 中断标志清除之后调用
   */
  void matrix_prepare_wakeup(void);

  /**
   * @brief 从睡眠恢复矩阵 GPIO 到正常扫描模式
   *        COL 引脚：恢复输入上拉（unselect 状态）
   *        ROW 引脚：恢复输入上拉
   *        关闭行中断（中断模式 → 轮询模式）
   */
  void matrix_resume_from_sleep(void);

  /**
   * @brief 唤醒后立即执行一次矩阵扫描
   * @note  用于检测触发唤醒的那个按键，防止首键丢失
   */
  void matrix_scan_once_after_wakeup(void);
  ```

- [ ] **Step 6.2：在 matrix.c 实现 matrix_prepare_wakeup()**

  实现需要包含（基于 CH584 GPIO API，参考 04122-13A 逻辑）：

  ```c
  void matrix_prepare_wakeup(void) {
      /* 1. unselect 所有 COL（先置高再切输出低，确保状态一致） */
      for (uint8_t c = 0; c < MATRIX_COLS; c++) {
          if (col_pins[c] != NO_PIN) {
              setPinOutput(col_pins[c]);
              writePinLow(col_pins[c]);   /* COL 拉低：与按键下拉形成检测回路 */
          }
      }

      /* 2. ROW 引脚：切换为下降沿中断模式 */
      for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
          if (row_pins[r] != NO_PIN) {
              /* 先清中断标志，再配置中断模式，再使能 IRQ */
              /* 具体调用：GPIOA_ClearITFlagBit / GPIOB_ClearITFlagBit 按引脚所在 port */
              /* 再调用 GPIOA_ITModeCfg / GPIOB_ITModeCfg 配置下降沿 */
          }
      }

      /* 3. 清 PFIC 挂起位，使能 GPIO IRQ */
      PFIC_ClearPendingIRQ(GPIO_A_IRQn);
      PFIC_ClearPendingIRQ(GPIO_B_IRQn);
      PFIC_EnableIRQ(GPIO_A_IRQn);
      PFIC_EnableIRQ(GPIO_B_IRQn);

      /* 4. 开启 GPIO 唤醒源 */
      PWR_PeriphWakeUpCfg(ENABLE, RB_SLP_GPIO_WAKE, Long_Delay);
  }
  ```

  > **实现说明：**
  > - `row_pins[]` 中存储的是抽象 `pin_t`，需要通过 HAL GPIO API 区分 PORTA/PORTB 并调用对应的中断配置函数
  > - 具体引脚到 GPIO_Pin_x 的映射参考 `hal/platforms/ch584/_pin_defs.h`
  > - 中断清标志必须在配置中断模式之前完成

- [ ] **Step 6.3：在 matrix.c 实现 matrix_resume_from_sleep()**

  ```c
  void matrix_resume_from_sleep(void) {
      /* 恢复 COL 为输入上拉（unselect 状态） */
      for (uint8_t c = 0; c < MATRIX_COLS; c++) {
          if (col_pins[c] != NO_PIN) {
              setPinInputHigh(col_pins[c]);
          }
      }

      /* 恢复 ROW 为输入上拉（关闭中断模式） */
      for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
          if (row_pins[r] != NO_PIN) {
              setPinInputHigh(row_pins[r]);
              /* 关闭行中断：配置为普通输入（不使用中断模式） */
              /* GPIOA_ITModeCfg(pin, GPIO_ITMode_Disable) 或等价操作 */
          }
      }
  }
  ```

- [ ] **Step 6.4：在 matrix.c 实现 matrix_scan_once_after_wakeup()**

  ```c
  void matrix_scan_once_after_wakeup(void) {
      /* 直接调用一次 matrix_scan()，触发防抖和按键事件 */
      matrix_scan();
  }
  ```

- [ ] **Step 6.5：编译验证**

  编译确认三个函数无错误，GPIO API 引用正确。

- [ ] **Step 6.6：提交**
  ```
  git add drivers/input/keyboard/matrix.h drivers/input/keyboard/matrix.c
  git commit -m "feat(matrix): add wakeup prepare/resume/scan interfaces for sleep support"
  ```

---

## Chunk 5：input_service prepare / resume

### Task 7：扩展 input_service.h / input_service.c

**文件：**
- 修改：`application/service/input_service.h`
- 修改：`application/service/input_service.c`

**背景：** `input_service` 是唤醒链路的主要拥有者：停矩阵扫描定时器、配置 GPIO 唤醒、恢复 PA2 中断。同时 GPIO ISR 需要在睡眠状态下投递 `SYSTEM_LPM_WAKE_EVT`，这也属于输入侧职责。

**PA2 电源键唤醒后恢复三步序列（不可颠倒）：**
```
1. GPIOA_ModeCfg(GPIO_Pin_2, GPIO_ModeIN_PU)       // 重置为输入上拉
2. GPIOA_ITModeCfg(GPIO_Pin_2, GPIO_ITMode_FallEdge) // 配置下降沿中断
3. GPIOA_ClearITFlagBit(GPIO_Pin_2)                  // 清除中断标志
```

- [ ] **Step 7.1：在 input_service.h 新增 LPM 事件和唤醒源枚举**

  在 `input_task_event_t` 末尾追加（当前已用 bits 1-4，使用 bits 5-6）：
  ```c
  INPUT_LPM_PREPARE_EVT  = 1 << 5,  /**< LPM prepare（Idle/Deep，mode 通过 lpm_get_mode() 查询） */
  INPUT_LPM_RESUME_EVT   = 1 << 6,  /**< LPM resume（Idle/Deep 唤醒后恢复） */
  ```

  同时新增唤醒原因枚举（供 ISR 锁存）：
  ```c
  typedef enum {
      LPM_WAKEUP_NONE    = 0,
      LPM_WAKEUP_MATRIX  = 1,    /* 矩阵按键触发 */
      LPM_WAKEUP_PA2     = 2,    /* 电源键 PA2 触发 */
      LPM_WAKEUP_TOUCH   = 3,    /* 触控板 INT 触发 */
      LPM_WAKEUP_RTC     = 4,    /* RTC 定时触发 */
  } lpm_wakeup_source_t;

  /* 供外部（system_service）查询最近一次唤醒原因 */
  lpm_wakeup_source_t input_get_last_wakeup_source(void);
  ```

- [ ] **Step 7.2：在 input_service.c 添加静态唤醒原因变量**

  ```c
  static volatile lpm_wakeup_source_t g_last_wakeup_source = LPM_WAKEUP_NONE;

  lpm_wakeup_source_t input_get_last_wakeup_source(void) {
      return g_last_wakeup_source;
  }
  ```

- [ ] **Step 7.3：实现 INPUT_LPM_PREPARE_EVT 处理**

  在 `input_process_event()` 中添加：
  ```c
  if (events & INPUT_LPM_PREPARE_EVT) {
      lpm_mode_t mode = lpm_get_mode();
      println("Input: LPM prepare start");

      /* 1. 停止矩阵扫描定时器 */
      matrix_scan_timer_stop();

      /* 2. 配置矩阵 GPIO 为唤醒中断模式（COL 拉低，ROW 配下降沿中断） */
      matrix_prepare_wakeup();

      /* 3. PA2 电源键：保留独立唤醒中断（已由 matrix_prepare_wakeup 之外单独处理） */
      /*    PA2 配置：GPIO_ModeIN_PU + ITMode_FallEdge，由 power_key 模块或此处显式配置 */
      GPIOA_ClearITFlagBit(GPIO_Pin_2);
      GPIOA_ModeCfg(GPIO_Pin_2, GPIO_ModeIN_PU);
      GPIOA_ITModeCfg(GPIO_Pin_2, GPIO_ITMode_FallEdge);

  #ifdef TOUCH_EN
      if (mode == LPM_MODE_IDLE) {
          touch_prepare_idle_sleep();   /* 触控低功耗，保留 INT 唤醒 */
      } else {
          touch_prepare_deep_sleep();   /* 触控更深低功耗，保留 INT 唤醒 */
      }
  #endif

      /* 4. 暂停 ADC 电量采样 */
      OSAL_StopReloadTask(input_taskID, INPUT_BATTERY_DETE_EVT);

      /* 5. 标记 input prepare 完成，通知 system_service 汇聚 */
      lpm_mark_prepare_done(LPM_PREPARE_INPUT);
      OSAL_SetEvent(system_taskID, SYSTEM_LPM_STEP_DONE_EVT);
      println("Input: LPM prepare done");
      return (events ^ INPUT_LPM_PREPARE_EVT);
  }
  ```

- [ ] **Step 7.4：实现 INPUT_LPM_RESUME_EVT 处理**

  ```c
  if (events & INPUT_LPM_RESUME_EVT) {
      lpm_mode_t mode = lpm_get_mode();
      println("Input: LPM resume start");

      /* 1. 恢复矩阵 GPIO 为正常扫描模式 */
      matrix_resume_from_sleep();

      /* 2. PA2 恢复（三步序列，顺序不可颠倒） */
      GPIOA_ModeCfg(GPIO_Pin_2, GPIO_ModeIN_PU);
      GPIOA_ITModeCfg(GPIO_Pin_2, GPIO_ITMode_FallEdge);
      GPIOA_ClearITFlagBit(GPIO_Pin_2);

  #ifdef TOUCH_EN
      touch_resume_from_sleep();
  #endif

      /* 3. 补一次矩阵扫描（防止首键丢失） */
      matrix_scan_once_after_wakeup();

      /* 4. 重启矩阵扫描定时器 */
      matrix_scan_timer_start();

      /* 5. 恢复 ADC 电量采样 */
      OSAL_StartReloadTask(input_taskID, INPUT_BATTERY_DETE_EVT, BATTERY_DETECT_INTERVAL);

      /* 6. 重置唤醒原因 */
      g_last_wakeup_source = LPM_WAKEUP_NONE;

      println("Input: LPM resume done");
      return (events ^ INPUT_LPM_RESUME_EVT);
  }
  ```

- [ ] **Step 7.5：实现 GPIO ISR 的唤醒事件投递**

  在 CH584 的 GPIOA_IRQHandler / GPIOB_IRQHandler 中（文件位置：`hal/platforms/ch584/` 或 `project/ch584m/` 下的中断处理文件），添加 LPM 唤醒处理：

  ```c
  __INTERRUPT
  __HIGH_CODE
  void GPIOA_IRQHandler(void) {
      /* 检测 PA2 电源键 */
      if (GPIOA_ReadITFlagBit(GPIO_Pin_2)) {
          GPIOA_ClearITFlagBit(GPIO_Pin_2);
          if (lpm_is_in_sleep()) {
              g_last_wakeup_source = LPM_WAKEUP_PA2;
              OSAL_SetEvent(system_taskID, SYSTEM_LPM_WAKE_EVT);
          }
          /* 正常按键逻辑在 lpm_is_in_sleep() == false 时执行 */
      }

      /* 检测矩阵 ROW 引脚中断（仅睡眠状态下有效） */
      if (lpm_is_in_sleep()) {
          /* 检测所有 ROW 引脚的中断标志 */
          /* 若任意行有下降沿，清标志并投递唤醒事件 */
          uint32_t it_flag = R16_PA_INT_IF;  /* 读 GPIOA 中断标志 */
          if (it_flag) {
              GPIOA_ClearITFlagBit(it_flag);
              g_last_wakeup_source = LPM_WAKEUP_MATRIX;
              OSAL_SetEvent(system_taskID, SYSTEM_LPM_WAKE_EVT);
          }
      }
  }
  ```

  > **注意：** 实际实现需要确认：
  > 1. GPIO ISR 文件的实际位置（可能在 `project/ch584m/` 或由 WCH HAL 提供框架）
  > 2. ROW 引脚是否跨 GPIOA/GPIOB，GPIOB_IRQHandler 需要同样处理
  > 3. ISR 中调用 `OSAL_SetEvent` 的线程安全性（WCH TMOS 支持在中断中调用）

- [ ] **Step 7.6：编译验证**

  编译确认所有新事件和函数引用无错误。

- [ ] **Step 7.7：提交**
  ```
  git add application/service/input_service.h application/service/input_service.c
  git commit -m "feat(input_service): add LPM prepare/resume handlers and GPIO wakeup ISR posting"
  ```

---

## Chunk 6：communication_service prepare / resume

### Task 8：扩展 communication_service.h / communication_service.c

**文件：**
- 修改：`application/service/communication_service.h`
- 修改：`application/service/communication_service.c`

**背景：** `communication_service` 的 `commu_task_EVT_t` 已使用 bits 0-13（14个事件），剩余 bits 14-15 可用，恰好容纳 2 个 LPM 事件。通过查询 `lpm_get_mode()` 区分 Idle/Deep 模式，无需 4 个独立事件。

- [ ] **Step 8.1：在 commu_task_EVT_t 末尾追加 LPM 事件**

  ```c
  COMMU_LPM_PREPARE_EVT  = 1 << 14,  /**< LPM prepare（mode 查 lpm_get_mode()） */
  COMMU_LPM_RESUME_EVT   = 1 << 15,  /**< LPM resume */
  ```

  > **注意：** `commu_task_EVT_t` 使用 uint16_t 枚举，16 位已满，不可再增加事件。

- [ ] **Step 8.2：实现 COMMU_LPM_PREPARE_EVT 处理**

  在 `commu_process_event()` 中添加：
  ```c
  if (events & COMMU_LPM_PREPARE_EVT) {
      lpm_mode_t mode = lpm_get_mode();
      println("Commu: LPM prepare start");

      if (mode == LPM_MODE_IDLE) {
          /* Idle：请求 BLE 进入低功耗，不断连 */
          /* WCH BLE 低功耗依赖 HAL_SLEEP=1 的 idleCB，此处主要是减少业务活动 */
          /* 停止主动发送报文（report_buffer 停发） */
          /* 注意：不调用 wireless_disconnect()，保持连接 */
          println("Commu: BLE idle low-power prepare done");
      } else {
          /* Deep：断开 BLE 连接，停止广播 */
          wireless_disconnect();
          dprintf("Commu: BLE disconnected for deep sleep\r\n");
      }

      lpm_mark_prepare_done(LPM_PREPARE_COMMU);
      OSAL_SetEvent(system_taskID, SYSTEM_LPM_STEP_DONE_EVT);
      println("Commu: LPM prepare done");
      return (events ^ COMMU_LPM_PREPARE_EVT);
  }
  ```

- [ ] **Step 8.3：实现 COMMU_LPM_RESUME_EVT 处理**

  ```c
  if (events & COMMU_LPM_RESUME_EVT) {
      println("Commu: LPM resume start");
      /* Deep 唤醒后：仅恢复本地无线上下文，不自动回连 */
      /* 业务层回连由上层（如 SYSTEM_WAKEUP_EVT 或用户按键触发）决定 */
      /* 此处清理睡眠标志，恢复 report_buffer 状态 */
      println("Commu: LPM resume done (reconnect decision deferred to app)");
      return (events ^ COMMU_LPM_RESUME_EVT);
  }
  ```

- [ ] **Step 8.4：编译验证**

  编译确认无 bit 溢出（枚举最大值 `1 << 15` 在 uint16_t 范围内）。

- [ ] **Step 8.5：提交**
  ```
  git add application/service/communication_service.h application/service/communication_service.c
  git commit -m "feat(commu_service): add LPM prepare/resume handlers for BLE sleep coordination"
  ```

---

## Chunk 7：output_service prepare / resume

### Task 9：扩展 output_service.h / output_service.c

**文件：**
- 修改：`application/service/output_service.h`
- 修改：`application/service/output_service.c`

**背景：** Idle 模式不关灯（output_service 无需参与 Idle prepare）；Deep 模式需要关闭背光和指示灯。`output_task_event_t` 当前用 bits 0-5，新增 bits 6-7 够用。

- [ ] **Step 9.1：在 output_task_event_t 末尾追加 LPM 事件**

  ```c
  OUTPUT_LPM_PREPARE_EVT  = 1 << 6,  /**< LPM prepare（仅 Deep 时关灯，Idle 为 no-op） */
  OUTPUT_LPM_RESUME_EVT   = 1 << 7,  /**< LPM resume（Deep 唤醒后按系统状态恢复灯效） */
  ```

- [ ] **Step 9.2：实现 OUTPUT_LPM_PREPARE_EVT 处理**

  ```c
  if (events & OUTPUT_LPM_PREPARE_EVT) {
      lpm_mode_t mode = lpm_get_mode();
      if (mode == LPM_MODE_DEEP) {
          /* Deep：关闭背光和指示灯 */
          indicator_off_all();
          /* 若有背光驱动，此处调用 backlight_disable() 或等价接口 */
          println("Output: LEDs off for deep sleep");
      }
      /* Idle：不关灯，直接标记完成 */
      lpm_mark_prepare_done(LPM_PREPARE_OUTPUT);
      OSAL_SetEvent(system_taskID, SYSTEM_LPM_STEP_DONE_EVT);
      return (events ^ OUTPUT_LPM_PREPARE_EVT);
  }
  ```

- [ ] **Step 9.3：实现 OUTPUT_LPM_RESUME_EVT 处理**

  ```c
  if (events & OUTPUT_LPM_RESUME_EVT) {
      /* Deep 唤醒后：根据当前无线状态恢复指示灯显示 */
      /* 例如：当前未连接则显示"未连接"指示，已连接则显示连接指示 */
      /* 具体指示灯状态由 wireless_state 决定，此处触发一次状态同步 */
      println("Output: Resume LEDs after deep wake");
      /* 调用 indicator 模块的状态刷新接口（如有） */
      return (events ^ OUTPUT_LPM_RESUME_EVT);
  }
  ```

- [ ] **Step 9.4：编译验证**

  编译确认 `indicator_off_all()`、`lpm_get_mode()` 等引用可见。

- [ ] **Step 9.5：提交**
  ```
  git add application/service/output_service.h application/service/output_service.c
  git commit -m "feat(output_service): add LPM prepare (deep LED off) and resume handlers"
  ```

---

## Chunk 8：HAL 层与回调层清理

### Task 10：清理 wireless_callbacks.c 直接睡眠调用

**文件：**
- 修改：`middleware/communication/wireless_callbacks.c`

**背景：** `access_ble_enter_idel_sleep()`（第 188-212 行）直接调用 `system_hal_enter_sleep()` 并在返回后立即调用 `bt_driver_connect_ex()`，这是典型的跨层违规。需要替换为投递 OSAL 事件。

- [ ] **Step 10.1：确认问题代码范围**

  打开 `middleware/communication/wireless_callbacks.c`，确认第 188-212 行是目标范围：
  ```c
  void access_ble_enter_idel_sleep(void) {
      // ...
      system_hal_enter_sleep(SYSTEM_POWER_MODE_DEEP_SLEEP, ...);  // 直接睡眠
      bt_driver_connect_ex(0, 0);   // 唤醒后直接重连
  }
  ```

- [ ] **Step 10.2：重构 access_ble_enter_idel_sleep()**

  将直接睡眠调用替换为投递 LPM 事件：
  ```c
  void access_ble_enter_idel_sleep(void) {
      if (wireless_get_state() != WT_CONNECTED) {
          dprintf("[WT_SLEEP] ignore state=%d\r\n", wireless_get_state());
          return;
      }
      if (!access_state.deep_sleep_flag) {
          dprintf("[WT_SLEEP] skip deep_sleep_flag=0");
          return;
      }

      dprintf("[WT_SLEEP] schedule deep sleep via OSAL event\r\n");
      /* 不直接调用 system_hal_enter_sleep()，改为投递 LPM Deep 请求 */
      /* 由 lpm 状态机和 system_service 协调执行 */
      OSAL_SetEvent(system_taskID, SYSTEM_LPM_DEEP_REQ_EVT);
      /* 唤醒后的回连由 COMMU_LPM_RESUME_EVT 或业务层决定，此处不直接调用 bt_driver_connect_ex */
  }
  ```

- [ ] **Step 10.3：编译验证**

  确认移除 `system_hal_enter_sleep()` 调用后无链接错误。

- [ ] **Step 10.4：提交**
  ```
  git add middleware/communication/wireless_callbacks.c
  git commit -m "refactor(wireless_callbacks): replace direct sleep call with LPM OSAL event"
  ```

---

### Task 11：调整 _system_hal.c 的 sleep 接口职责

**文件：**
- 修改：`hal/platforms/ch584/_system_hal.c`

**背景：** Option A（HAL_SLEEP=1）后，`LowPower_Sleep()` 由 TMOS `CH58x_LowPower()` 统一执行，`system_hal_enter_sleep()` 不应再直接调用 `LowPower_Sleep()`。该接口只保留唤醒源配置职责（`PWR_PeriphWakeUpCfg`）。

- [ ] **Step 11.1：修改 system_hal_enter_sleep() 函数体**

  当前代码（第 137-182 行）包含 `LowPower_Sleep()` 的 switch-case 调用，修改为：

  ```c
  system_result_t system_hal_enter_sleep(system_power_mode_t mode, uint32_t wakeup_sources)
  {
      /* 配置唤醒源（GPIO / RTC / USB） */
      uint8_t wake_ctrl = 0;
      if (wakeup_sources & SYSTEM_WAKEUP_GPIO) {
          wake_ctrl |= RB_SLP_GPIO_WAKE;
      }
      if (wakeup_sources & SYSTEM_WAKEUP_RTC) {
          wake_ctrl |= RB_SLP_RTC_WAKE;
      }
      if (wakeup_sources & SYSTEM_WAKEUP_USB) {
          wake_ctrl |= RB_SLP_USB_WAKE;
      }
      if (wake_ctrl) {
          PWR_PeriphWakeUpCfg(ENABLE, wake_ctrl, Short_Delay);
      }

      /* Option A: LowPower_Sleep() 由 TMOS CH58x_LowPower() idleCB 自动触发
       * 此函数只负责唤醒源配置，不直接执行睡眠指令
       * 保留 mode 参数供将来扩展（如 Shutdown 路径仍直接执行） */
      if (mode == SYSTEM_POWER_MODE_SHUTDOWN) {
          LowPower_Shutdown(0);
          /* 不会返回 */
      }

      return SYSTEM_OK;
  }
  ```

  > **注意：** `SYSTEM_POWER_MODE_SHUTDOWN` 对应关机操作，仍保留直接调用 `LowPower_Shutdown()`，这是合理的（关机不通过 TMOS 调度）。

- [ ] **Step 11.2：确认 system_hal_shutdown() 不受影响**

  第 120-130 行的 `system_hal_shutdown()` 直接调用 `LowPower_Shutdown(0)`，用于关机路径，保持不变。

- [ ] **Step 11.3：编译验证**

  编译确认无错误。注意 `LowPower_Sleep` 的符号引用不再出现在此文件中（Shutdown 除外）。

- [ ] **Step 11.4：提交**
  ```
  git add hal/platforms/ch584/_system_hal.c
  git commit -m "refactor(_system_hal): remove LowPower_Sleep from sleep entry, keep wakeup source config only"
  ```

---

## Chunk 9：集成验证

### Task 12：编译完整性与日志验证

**目标：** 确认所有改动可以无错误编译，并通过日志验证状态机流转路径。

- [ ] **Step 12.1：完整清理编译**

  执行完整 clean + build，确认无 warning/error：
  ```bash
  # WCH 工具链命令（参考 CLAUDE.md）
  cmd.exe /c "cd /d <project_path>\obj && ... make.exe clean && make.exe main-build"
  ```
  预期：`keyboard-framework.elf` 和 `keyboard-framework.hex` 生成成功，无编译错误。

- [ ] **Step 12.2：检查 lpm_task() 调用点是否补全**

  确认以下调用存在：
  ```bash
  grep -rn "lpm_task\(\)" test_main.c application/
  ```
  若 `lpm_task()` 未被调用，找到主循环（`OSAL_SystemProcess()` 之前或之后）并添加调用。这是 LPM 超时推进的前提。

- [ ] **Step 12.3：检查 lpm_note_activity() 调用点**

  以下位置应调用 `lpm_note_activity()`：
  ```bash
  grep -rn "lpm_note_activity\|lpm_timer_reset" middleware/ application/
  ```
  确认以下路径已调用（现有 `lpm_timer_reset` 调用等价）：
  - `report_buffer.c`：发送报文时
  - `wireless.c`：连接成功时

- [ ] **Step 12.4：日志路径验证（上机前检查）**

  通过代码走读，确认以下日志序列可达：
  ```
  [5s 无操作]
  System: Idle sleep requested
  Input: LPM prepare start
  Input: LPM prepare done
  Commu: LPM prepare start
  Commu: LPM prepare done
  System: Entering Idle sleep, waiting TMOS idleCB

  [10min 无操作]
  System: Deep sleep requested
  Input: LPM prepare start / done
  Commu: BLE disconnected for deep sleep
  Commu: LPM prepare done
  Output: LEDs off for deep sleep
  Output: LPM prepare done
  System: Entering Deep sleep, waiting TMOS idleCB

  [按键唤醒]
  System: Wake resume start
  Input: LPM resume start
  Input: LPM resume done
  ```

- [ ] **Step 12.5：确认事件 bit 无冲突**

  ```bash
  grep -n "= 1 <<" application/service/system_service.h \
       application/service/input_service.h \
       application/service/communication_service.h \
       application/service/output_service.h
  ```
  检查同一个 `.h` 文件内没有重复的 bit 值。各文件内 bit 分配：
  - `system_service.h`：bits 0-7（原有）+ bits 8-13（新增 LPM）
  - `input_service.h`：bits 1-6
  - `communication_service.h`：bits 0-15（满）
  - `output_service.h`：bits 0-7

- [ ] **Step 12.6：最终整体提交标记**
  ```
  git add .
  git commit -m "feat(lpm): complete N0046 sleep strategy implementation - Idle/Deep/Wake state machine"
  ```

---

## 验证清单（上机必验）

完成编译后，按以下顺序上机验证，参考 `DOCS/plans/2026-03-16-n0046-sleep-strategy-feasibility.md` 第六节：

| 序号 | 验证项 | 验证方法 | 通过标准 |
|---|---|---|---|
| V1 | HAL_SLEEP=1 基础功耗 | 连接 BLE，空闲，电流表 | 周期性低功耗下降（BLE 连接间隔） |
| V2 | Idle 5s 触发 | 等待 5s，串口日志 | 出现 "Entering Idle sleep" 日志 |
| V3 | 矩阵按键唤醒 | Idle 中按任意键 | 出现 "Wake resume" 日志，按键上报 |
| V4 | PA2 电源键唤醒 | Idle 中按电源键 | 唤醒成功，PA2 逻辑正常 |
| V5 | PA2 多次响应 | 唤醒后连按电源键 3 次 | 每次均响应，无失响 |
| V6 | Idle 中 BLE 不断连 | 主机侧观察连接状态 | 连接保持，无断连 |
| V7 | Idle 灯效保持 | 进 Idle，观察指示灯 | 灯效不变 |
| V8 | Deep 10min 触发 | 等待 10min 或修改超时为 30s 测试 | 出现 "Entering Deep sleep" 日志 |
| V9 | Deep 中灯效关闭 | 进 Deep，观察指示灯 | 所有灯关闭 |
| V10 | Deep 唤醒后广播 | Deep 唤醒后，主机侧 BLE 扫描 | 设备重新广播可见 |
| V11 | 首键不丢失 | 唤醒后立刻快速打字 | 唤醒触发键有效上报 |

---

## 依赖关系图

```
Task 1 (HAL_SLEEP)
  └── Task 2 (DIODE_DIR)
        └── Task 3 (lpm.h/c)
              └── Task 4 (system_service events)
                    └── Task 5 (system_service refactor)
                          ├── Task 6 (matrix wakeup)
                          │     └── Task 7 (input_service)
                          ├── Task 8 (commu_service)
                          └── Task 9 (output_service)
Task 10 (wireless_callbacks) ─────┘ (并行)
Task 11 (_system_hal) ────────────┘ (并行)
Task 12 (集成验证) ← 所有 Task 完成后
```

Tasks 6-11 在 Task 5 完成后可并行执行。
