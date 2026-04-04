# 日志规范化实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 统一业务代码日志为 LOG_E/LOG_W/LOG_I/LOG_D 宏，删除冗余调试打印，从约 250 条精简到约 45 条。

**Architecture:** 在现有 debug.h 中新增轻量级带级别的日志宏，复用底层 xprintf 输出；同步调整 config_product.h 日志配置；逐模块清理所有业务代码日志。

**Tech Stack:** C, 预处理器宏, CH584 RISC-V 平台

---

## Task 1: 新增日志宏定义

**Files:**
- Modify: `drivers/output/logging/debug.h`

- [ ] **Step 1: 在 debug.h 末尾（#endif /* NO_DEBUG */ 之前）添加 LOG_LEVEL 常量和 LOG_E/LOG_W/LOG_I/LOG_D 宏**

在 `#endif /* NO_DEBUG */` 行（第170行）之前，添加以下内容：

```c
/* === 统一日志宏 (LOG_E / LOG_W / LOG_I / LOG_D) === */
/* 日志级别定义，由 config_product.h 中的 LOG_LEVEL 选择 */
#ifndef LOG_LEVEL_NONE
#define LOG_LEVEL_NONE    0
#endif
#ifndef LOG_LEVEL_ERROR
#define LOG_LEVEL_ERROR   1
#endif
#ifndef LOG_LEVEL_WARN
#define LOG_LEVEL_WARN    2
#endif
#ifndef LOG_LEVEL_INFO
#define LOG_LEVEL_INFO    3
#endif
#ifndef LOG_LEVEL_DEBUG
#define LOG_LEVEL_DEBUG   4
#endif

/* 默认级别：未配置时仅输出 WARN 和 ERROR */
#ifndef LOG_LEVEL
#define LOG_LEVEL  LOG_LEVEL_WARN
#endif

#if (LOG_LEVEL >= LOG_LEVEL_ERROR) && !defined(NO_PRINT)
#define LOG_E(fmt, ...) xprintf("[E] " fmt "\r\n", ##__VA_ARGS__)
#else
#define LOG_E(fmt, ...)
#endif

#if (LOG_LEVEL >= LOG_LEVEL_WARN) && !defined(NO_PRINT)
#define LOG_W(fmt, ...) xprintf("[W] " fmt "\r\n", ##__VA_ARGS__)
#else
#define LOG_W(fmt, ...)
#endif

#if (LOG_LEVEL >= LOG_LEVEL_INFO) && !defined(NO_PRINT)
#define LOG_I(fmt, ...) xprintf("[I] " fmt "\r\n", ##__VA_ARGS__)
#else
#define LOG_I(fmt, ...)
#endif

#if (LOG_LEVEL >= LOG_LEVEL_DEBUG) && !defined(NO_PRINT)
#define LOG_D(fmt, ...) xprintf("[D] " fmt "\r\n", ##__VA_ARGS__)
#else
#define LOG_D(fmt, ...)
#endif
```

- [ ] **Step 2: 验证宏定义不与现有代码冲突**

搜索确认项目中没有已存在的 `LOG_E`、`LOG_W`、`LOG_I`、`LOG_D` 定义：

```bash
grep -r "define LOG_E\|define LOG_W\|define LOG_I\|define LOG_D" --include="*.h" --include="*.c" .
```

预期：无冲突。

- [ ] **Step 3: Commit**

```bash
git add drivers/output/logging/debug.h
git commit -m "feat(logging): add LOG_E/LOG_W/LOG_I/LOG_D macros"
```

---

## Task 2: 调整 config_product.h 日志配置

**Files:**
- Modify: `keyboards/kb904/config_product.h`

- [ ] **Step 1: 替换日志配置区域**

将 config_product.h 中第18-43行的日志配置替换为：

```c
/* ---------- 日志配置 ---------- */
#define PRINTF_ENABLE      TRUE
#define INDICATOR_TEST_ENABLE

#if (PRINTF_ENABLE == TRUE)
    /* --- 业务日志级别（控制 LOG_E/LOG_W/LOG_I/LOG_D 输出）--- */
    #define LOG_LEVEL               LOG_LEVEL_WARN

    /* --- 平台日志输出配置（CH584 LOG 系统底层）--- */
    #define PRINTF_LEVEL              PRINTF_LEVEL_WARN
    #define PRINTF_UART               PLATFORM_UART_3
    #define PRINTF_BAUDRATE           115200
    #define LOG_COLOR_ENABLE          FALSE
    #define LOG_FMT_USING_DIR_ENABLE  FALSE
    #define LOG_FMT_USING_LINE_ENABLE FALSE
    #define LOG_FMT_USING_FUNC_ENABLE FALSE
    #define LOG_LINE_BUF_SIZE         256
    #define LOG_NEWLINE_SIGN          "\r\n"
    #define LOG_LINE_NUM_MAX_LEN      4
    #define LOG_FILTER_TAG_MAX_LEN    20

    #if (CHIP_TYPE == CHIP_CH584M)
        #ifndef __HIGH_CODE
        #define __HIGH_CODE  __attribute__((section(".highcode")))
        #endif
    #else
        #define __HIGH_CODE
    #endif
#endif
```

关键变更：
- 删除 `DEBUG_DEBOUNCE TRUE`（由 LOG_LEVEL 统一控制）
- 新增 `LOG_LEVEL LOG_LEVEL_WARN`
- `PRINTF_LEVEL` 从 `PRINTF_LEVEL_DEBUG` 改为 `PRINTF_LEVEL_WARN`
- `LOG_COLOR_ENABLE` 从 `TRUE` 改为 `FALSE`
- `LOG_FMT_USING_DIR_ENABLE` 从 `TRUE` 改为 `FALSE`
- `LOG_FMT_USING_LINE_ENABLE` 从 `TRUE` 改为 `FALSE`
- `LOG_LINE_BUF_SIZE` 从 `1024` 改为 `256`

- [ ] **Step 2: 验证 debounce.c 中 DEBUG_DEBOUNCE 的处理**

检查 `drivers/input/keyboard/debounce.c` 第277行 `#ifdef DEBUG_DEBOUNCE` 块内的日志。由于 DEBUG_DEBOUNCE 不再定义，该代码块自动不编译，无需修改 debounce.c。

- [ ] **Step 3: Commit**

```bash
git add keyboards/kb904/config_product.h
git commit -m "refactor(logging): unify log config in config_product.h"
```

---

## Task 3: 清理 application/main.c 和 system_init.c

**Files:**
- Modify: `application/main.c`
- Modify: `application/system/system_init.c`

- [ ] **Step 1: 修改 application/main.c**

找到第32行附近的：
```c
dprintf("ERROR: System initialization failed! Error code: %d\r\n", error_code);
```
替换为：
```c
LOG_E("[SYS] init failed, err=%d", error_code);
```

找到第36行附近的：
```c
dprint("System initialized successfully\r\n");
```
替换为：
```c
LOG_I("[SYS] init done");
```

确保文件顶部 `#include "debug.h"` 已存在。

- [ ] **Step 2: 修改 application/system/system_init.c**

找到第73行附近的：
```c
dprintf("Hardware timer initialized\r\n");
```
删除此行。

找到第158行附近的：
```c
dprintf("[SYS_INIT] bt_driver_init before commu_service_init\r\n");
```
替换为：
```c
LOG_I("[SYS] bt_driver_init done");
```

- [ ] **Step 3: Commit**

```bash
git add application/main.c application/system/system_init.c
git commit -m "refactor(logging): normalize application/main and system_init logs"
```

---

## Task 4: 清理 application/service/communication_service.c

**Files:**
- Modify: `application/service/communication_service.c`

这是日志最多的文件（~20条），按设计文档精简为约7条。

- [ ] **Step 1: 在文件顶部确认已包含 debug.h**

确保有 `#include "debug.h"`。

- [ ] **Step 2: 清理 WL_STS_INIT 分支**

找到 WL_STS_INIT 处理中的日志，将状态转换日志替换为 LOG_I，删除冗余的纯文本描述日志：

将类似：
```c
dprintf("[COMMU_STS] WL_STS_INIT state_before=%d evt_type=%d task=%d\r\n", ...)
dprintf("Communication: Wireless module initialization\r\n");
```
替换为仅保留：
```c
LOG_I("[COMMU] WL_STS_INIT state=%d evt=%d", state_before, evt_type);
```
删除 `Communication: Wireless module initialization` 行。

- [ ] **Step 3: 清理 WL_STS_DISCOVERABLE 分支**

将：
```c
dprintf("[COMMU_STS] WL_STS_DISCOVERABLE host=%d state_before=%d evt_type=%d\r\n", ...)
dprintf("Communication: Wireless discoverable entered\r\n");
```
替换为：
```c
LOG_I("[COMMU] WL_STS_DISCOVERABLE host=%d", host_idx);
```

- [ ] **Step 4: 清理 WL_STS_RECONNECTING 分支**

将：
```c
dprintf("[COMMU_STS] WL_STS_RECONNECTING host=%d state_before=%d evt_type=%d\r\n", ...)
dprintf("Communication: Wireless reconnecting\r\n");
```
替换为：
```c
LOG_I("[COMMU] WL_STS_RECONNECTING host=%d", host_idx);
```

- [ ] **Step 5: 清理 WL_STS_CONNECTED 分支**

将：
```c
dprintf("[COMMU_STS] WL_STS_CONNECTED host=%d state_before=%d evt_type=%d\r\n", ...)
dprintf("Communication: Wireless connected\r\n");
```
替换为：
```c
LOG_I("[COMMU] WL_STS_CONNECTED host=%d", host_idx);
```

- [ ] **Step 6: 清理 WL_STS_DISCONNECTED 分支**

将：
```c
dprintf("[COMMU_STS] WL_STS_DISCONNECTED host=%d reason=%d state_before=%d evt_type=%d\r\n", ...)
dprintf("Communication: Wireless disconnected\r\n");
```
替换为：
```c
LOG_W("[COMMU] WL_STS_DISCONNECTED host=%d reason=%d", host_idx, reason);
```
（断连用 WARN 级别）

- [ ] **Step 7: 清理 WL_STS_SUSPEND 分支**

删除：
```c
dprintf("Communication: Wireless suspended\r\n");
```

- [ ] **Step 8: 清理请求函数日志**

在 `WL_REQ_PAIR` 处理中，将：
```c
dprintf("[COMMU_REQ] WL_REQ_PAIR host=%d\r\n", ...)
```
替换为：
```c
LOG_I("[COMMU] WL_REQ_PAIR host=%d", host_idx);
```

在 `WL_REQ_RECONNECT` 处理中，将：
```c
dprintf("[COMMU_REQ] WL_REQ_RECONNECT host=%d state_before=%d\r\n", ...)
dprintf("Communication: Wireless reconnect\r\n");
```
替换为：
```c
LOG_I("[COMMU] WL_REQ_RECONNECT host=%d", host_idx);
```

删除 `Communication: Wireless disconnect` 行。
删除 `Communication: USB connected` 行。
删除 `Communication: USB disconnected` 行。
删除 `Communication: Service initialized with task ID %d` 行。

- [ ] **Step 9: Commit**

```bash
git add application/service/communication_service.c
git commit -m "refactor(logging): normalize communication_service logs"
```

---

## Task 5: 清理 application/service/input_service.c, output_service.c, system_service.c

**Files:**
- Modify: `application/service/input_service.c`
- Modify: `application/service/output_service.c`
- Modify: `application/service/system_service.c`

- [ ] **Step 1: 清理 input_service.c**

删除以下日志：
- `dprintf("Input: Touch toggle event received (not implemented)\r\n")` — 未实现功能标记
- `dprintf("Input: Matrix scan timer started\r\n")` — 非关键
- `dprintf("Input: Touchpad setup\r\n")` — 非关键

将以下迁移到 LOG 宏：
- `xprintf("Input: Battery level = %d%%\r\n", ...)` → `LOG_I("[INPUT] battery=%d%%", ...);`
- `println("Input: Low battery warning")` → `LOG_W("[INPUT] low battery");`
- `println("Input: Critical battery, triggering shutdown")` → `LOG_W("[INPUT] critical battery, shutdown");`
- `println("Input: Battery normal")` → 删除（正常状态无需日志）
- `dprintf("Input: Service initialized with task ID %d\r\n", ...)` → 删除

注意：检查文件中是否有多个重复的电池日志调用，合并为统一的处理。

- [ ] **Step 2: 清理 output_service.c**

删除以下日志：
- `dprintf("backlight_5s_sleep_timer_start\r\n")` — 内部定时器
- `dprintf("Output service init start\r\n")` — 非关键
- `dprintf("Output service init done\r\n")` — 非关键

将以下迁移：
- `dprintf("Output service task register failed\r\n")` → `LOG_E("[OUTPUT] task register failed");`
- `dprintf("Task registered, ID=%d\r\n", ...)` → 删除

- [ ] **Step 3: 清理 system_service.c**

将所有 `println` 迁移为 LOG 宏：
- `println("System: Low battery shutdown")` → `LOG_W("[SYS] low battery shutdown");`
- `println("System: System shutdown")` → `LOG_I("[SYS] system shutdown");`
- `println("System: Storage operation")` → `LOG_I("[SYS] storage operation");`
- `println("System: Factory reset requested")` → `LOG_W("[SYS] factory reset requested");`
- `dprintf("System: Factory reset delay scheduling failed, resetting now\r\n")` → `LOG_E("[SYS] factory reset delay failed");`
- `dprintf("System: Factory reset already pending\r\n")` → 删除
- `println("System: Factory reset reboot")` → `LOG_I("[SYS] factory reset reboot");`
- `println("System: OTA update")` → `LOG_I("[SYS] OTA update");`
- `println("System: Enter Idle sleep")` → `LOG_I("[SYS] enter idle sleep");`
- `println("System: Enter Deep sleep")` → `LOG_I("[SYS] enter deep sleep");`
- `dprintf("System: Enter Deep sleep: wt=%d\n", ...)` → 删除（与上一条重复）
- `println("System: Wake resume")` → `LOG_I("[SYS] wake resume");`
- `dprintf("System: Service initialized with task ID %d\r\n", ...)` → 删除

- [ ] **Step 4: Commit**

```bash
git add application/service/input_service.c application/service/output_service.c application/service/system_service.c
git commit -m "refactor(logging): normalize input/output/system service logs"
```

---

## Task 6: 清理 middleware/communication/ 日志

**Files:**
- Modify: `middleware/communication/wireless.c`
- Modify: `middleware/communication/wireless_callbacks.c`
- Modify: `middleware/communication/host.c`
- Modify: `middleware/communication/transport.c`
- Modify: `middleware/communication/PMU.c`

- [ ] **Step 1: 清理 wireless.c**

删除所有 `kc_printf(...)` 调用，只保留状态机转换日志（迁移为 LOG_I）：

将 `[WT_STATE]` 开头的日志替换：
- `kc_printf("[WT_STATE] -> WT_PARING host=%d\n", host_idx)` → `LOG_I("[WL] state->WT_PAIRING host=%d", host_idx);`
- `kc_printf("[WT_STATE] -> WT_RECONNECTING host=%d\n", host_idx)` → `LOG_I("[WL] state->WT_RECONNECTING host=%d", host_idx);`
- `kc_printf("[WT_STATE] -> WT_CONNECTED host=%d\n", host_idx)` → `LOG_I("[WL] state->WT_CONNECTED host=%d", host_idx);`
- `kc_printf("[WT_STATE] -> WT_DISCONNECTED host=%d reason=%d\n", host_idx, reason)` → `LOG_W("[WL] state->WT_DISCONNECTED host=%d reason=%d", host_idx, reason);`
- `kc_printf("[WT_STATE] -> WT_SUSPEND\n")` → `LOG_I("[WL] state->WT_SUSPEND");`

删除所有其他 kc_printf 调用（约15条），包括：
- `kc_printf("Wireless: Switched to BT driver\n")` 等
- `kc_printf("wireless_pairing_ex %d\n\r", ...)` 等
- `kc_printf("WT_SUSPEND\n\r")` 等

将 LED 状态日志：
- `dprintf("[WT_LED] state=%02X\n", state)` → 删除

- [ ] **Step 2: 清理 wireless_callbacks.c**

精简回调日志，每类事件只保留一条：

将成对的 `[WT_SYNC]` 日志合并：
```c
// 原始：两条日志
dprintf("[WT_SYNC] adv pairing=%d host=%d\r\n", pairing_state, host_idx);
dprintf("[WT_SYNC] queue adv task=%d state=%d\r\n", commu_taskID, wireless_get_state());
```
替换为：
```c
LOG_I("[WL_CB] adv pairing=%d host=%d", pairing_state, host_idx);
```

对 connected/disconnected/adv_end 回调做同样处理：
- connected → `LOG_I("[WL_CB] connected host=%d", host_idx);`
- disconnected → `LOG_W("[WL_CB] disconnected host=%d reason=%d", host_idx, reason);`
- adv_end → 删除（非关键）

将 BLE 初始化完成日志：
```c
dprintf("Wireless: BLE initialization completed:%d\r\n", commu_taskID);
dprintf("[WT_SYNC] init_done task=%d will_post=%d\r\n", commu_taskID, ...);
```
替换为：
```c
LOG_I("[WL_CB] BLE init done");
```

- [ ] **Step 3: 清理 host.c**

键盘报告调试日志（第89-95行，仅在 debug_keyboard 为真时输出）：
全部删除。这些是高频调试输出，不应保留在产品代码中。

- [ ] **Step 4: 清理 transport.c**

- `dprintf("Transport: Switching to", ...)` → 删除（输出不完整）
- `kc_printf("Transport: Notifying driver switch to %d\n", new_transport)` → `LOG_I("[TRANS] driver switch to %d", new_transport);`
- `kc_printf("transport_changed %d\n\r", new_transport)` → 删除（与上一条重复）

- [ ] **Step 5: 清理 PMU.c**

- `PRINT("enter_sleep\n")` → 删除
- `PRINT("exit\n")` → 删除
- `dprintf("PMU_Update: state=%d\n", state)` → 删除
- `PRINT("@S\n")` → 删除
- `PRINT("@Deep_sleep\n")` → 删除
- `PRINT("PMU_WAKE\n")` → 删除
- `dprintf("PMU: initialized, taskID=%d\r\n", pmu_taskID)` → `LOG_I("[PMU] initialized taskID=%d", pmu_taskID);`

- [ ] **Step 6: Commit**

```bash
git add middleware/communication/wireless.c middleware/communication/wireless_callbacks.c middleware/communication/host.c middleware/communication/transport.c middleware/communication/PMU.c
git commit -m "refactor(logging): normalize middleware/communication logs"
```

---

## Task 7: 清理 middleware/keyboard/ 和 middleware/touchpad/ 日志

**Files:**
- Modify: `middleware/keyboard/report.c`
- Modify: `middleware/keyboard/keyboard.c`
- Modify: `middleware/keyboard/keymap.c`
- Modify: `middleware/keyboard/combo/kb_combo_engine.c`
- Modify: `middleware/keyboard/combo/kb_fn_action.c`
- Modify: `middleware/keyboard/combo/kb_sys_action.c`
- Modify: `middleware/touchpad/touchpad.c`

- [ ] **Step 1: 清理 report.c**

保留错误日志，迁移为 LOG_E：
- `dprintf("add_key_bit: can't add: %02X\n", code)` → `LOG_E("[RPT] add_key_bit failed, code=%02X", code);`
- `dprintf("del_key_bit: can't del: %02X\n", code)` → `LOG_E("[RPT] del_key_bit failed, code=%02X", code);`

删除所有其他日志：
- `dprintf("Holding consumer key: 0x%04X for %d ms\r\n", ...)` — 删除
- `dprintf("  Consumer keycode: 0x%04X, adding to consumer report\n", ...)` — 删除
- `dprintf("Processing keycode from _key_code_list: 0x%04X\r\n", ...)` — 删除
- `dprintf("Processing keycode from _key_code_list_extend: 0x%04X\r\n", ...)` — 删除
- `dprintf("Consumer hold expired for key: 0x%04X\r\n", ...)` — 删除
- `dprintf("Maintaining hold on consumer key: 0x%04X\r\n", ...)` — 删除
- `dprintf("Report: Consumer report changed, sending update:%04X\r\n", ...)` — 删除
- `dprintf("sending keycode:%d,%d,...\r\n", ...)` — 删除
- 所有被注释的 `// dprintf(...)` — 删除

- [ ] **Step 2: 清理 keyboard.c**

- `dprintf("Matrix scan detected changes\r\n")` → 删除
- `dprintf("Key pressed\r\n")` → 删除
- `dprintf("Key released\r\n")` → 删除
- `dprintf("Layer switch: system_type=%d, target_layer=%d\r\n", ...)` → `LOG_I("[KB] layer switch sys=%d layer=%d", host_system_type, target_layer);`

- [ ] **Step 3: 清理 keymap.c**

删除被注释的日志：
- `// dprintf("Get keycode for row %d, col %d on layer %d: 0x%04X\r\n", ...)`

- [ ] **Step 4: 清理 kb_combo_engine.c**

删除被注释的日志：
- `// dprintf("Combo %d state machine processing, event: %d\n", i, combo->event)`

- [ ] **Step 5: 清理 kb_fn_action.c**

- `dprintf("FN_UP_KEY called, current Earth state: %d\r\n", earth_state)` → 删除
- `dprintf("Bringhness_Down: FN_st == 1\r\n")` → 删除
- `dprintf("Bringhness_Down: FN_st == 0\r\n")` → 删除

- [ ] **Step 6: 清理 kb_sys_action.c**

全部删除（7条功能触发日志，均为高频无调试价值）：
- `dprintf("Set_FN_Lock triggered\n")`
- `dprintf("Backlight_Color triggered\n")`
- `dprintf("Backlight_Light triggered\n")`
- `dprintf("Touch_Locked triggered\n")`
- `dprintf("Factory_Reset triggered\n")`
- `dprintf("Bat_Check triggered\n")`
- `dprintf("Pair_button triggered\n")`

- [ ] **Step 7: 清理 touchpad.c**

将 `TOUCHPAD_MW_LOG` 调用迁移：
- `TOUCHPAD_MW_LOG("Touchpad: register failed after max retries\r\n")` → `LOG_E("[TP] register failed after max retries");`
- `TOUCHPAD_MW_LOG("Touchpad: task registration failed\r\n")` → `LOG_E("[TP] task registration failed");`
- `TOUCHPAD_MW_LOG("Touchpad: power on failed\r\n")` → `LOG_E("[TP] power on failed");`
- `TOUCHPAD_MW_LOG("Touchpad: watchdog abnormal, reinitializing\r\n")` → `LOG_W("[TP] watchdog abnormal, reinit");`
- `TOUCHPAD_MW_LOG("Touchpad: register success\r\n")` → 删除

删除正常流程日志：
- `TOUCHPAD_MW_LOG("Touchpad: INIT\r\n")`
- `TOUCHPAD_MW_LOG("Touchpad: data event\r\n")`
- `TOUCHPAD_MW_LOG("Touchpad: powering on\r\n")`

- [ ] **Step 8: Commit**

```bash
git add middleware/keyboard/report.c middleware/keyboard/keyboard.c middleware/keyboard/keymap.c middleware/keyboard/combo/kb_combo_engine.c middleware/keyboard/combo/kb_fn_action.c middleware/keyboard/combo/kb_sys_action.c middleware/touchpad/touchpad.c
git commit -m "refactor(logging): normalize middleware/keyboard and touchpad logs"
```

---

## Task 8: 清理 drivers/ 业务日志

**Files:**
- Modify: `drivers/input/keyboard/matrix.c`
- Modify: `drivers/input/keyboard/debounce.c`
- Modify: `drivers/communication/bluetooth/ch584/_bt_driver.c`
- Modify: `drivers/communication/bluetooth/ch584/hidkbd.c`
- Modify: `drivers/power/battery.c`
- Modify: `drivers/storage/storage.c`
- Modify: `drivers/output/backlight/backlight_hal.c`

- [ ] **Step 1: 清理 matrix.c**

- `dprintf("Matrix scan detected changes\r\n")` → 删除

- [ ] **Step 2: 清理 debounce.c**

保留错误日志，迁移为 LOG_E：
- `dprintf("Debounce: ERROR - num_rows (%d) exceeds MAX_MATRIX_ROWS (%d)\n", ...)` → `LOG_E("[DEBOUNCE] num_rows %d exceeds MAX %d", num_rows, MAX_MATRIX_ROWS);`
- `dprintf("Debounce: ERROR - Invalid parameters\n")` → `LOG_E("[DEBOUNCE] invalid parameters");`
- `dprintf("Debounce: ERROR - Unknown algorithm %d, using DEBOUNCE_SYM_DEFER_PK\n", ...)` → `LOG_E("[DEBOUNCE] unknown algo %d, fallback", DEBOUNCE_ALGORITHM);`

删除其他日志：
- `dprintf("Debounce: Initialized for %d rows\n", ...)` — 删除
- `dprintf("Debounce: Freed\n")` — 删除
- `dprintf("Debounce: Matrix changed\n")` — 删除（含后续的 row dump）
- `dprintf("  Row %d: 0x%04X\n", ...)` — 删除
- `dprintf("Debounce: Reset\n")` — 删除

- [ ] **Step 3: 清理 _bt_driver.c**

保留发送失败日志，迁移为 LOG_E：
- `dprintf("[BT] send_keyboard fail ret=%d\n", ret)` → `LOG_E("[BT] send_keyboard fail, ret=%d", ret);`
- `dprintf("[BT] send_mouse fail ret=%d\n", ret)` → `LOG_E("[BT] send_mouse fail, ret=%d", ret);`
- `dprintf("[BT] send_ptp fail ret=%d\n", ret)` → `LOG_E("[BT] send_ptp fail, ret=%d", ret);`

保留关键操作，迁移为 LOG_I：
- `dprintf("[BT] init done\n")` → `LOG_I("[BT] init done");`
- `dprintf("[BT] disconnect\n")` → `LOG_I("[BT] disconnect");`
- `dprintf("[BT] clear_bonding\n")` → `LOG_I("[BT] clear_bonding");`

删除所有其他 dprintf 调用（约20条），包括：
- 所有 `connect_ex` 内部的状态判断日志
- 所有 `pairing_ex` 内部的状态判断日志
- `send_consumer` 和 `send_system` 的正常发送日志
- `update_bat_level` 的日志
- `setup`、`init` 的中间步骤日志
- `init skip, already initialized` 日志

- [ ] **Step 4: 清理 hidkbd.c**

保留错误日志，迁移为 LOG_E（需确认 PRINT 宏是否可替换）：
- `PRINT("[BT_ADDR] update skip: cfg=NULL\n")` → `LOG_E("[HID] addr update skip: cfg=NULL");`
- `PRINT("[BT_ADDR] update skip: invalid base=...")` → `LOG_E("[HID] addr update skip: invalid base");`
- `PRINT("[BT_ADDR] update skip: invalid slot=...")` → `LOG_E("[HID] addr update skip: invalid slot=%x idx=%x", slot, ble_idx);`
- `PRINT("[BOND] save failed: cfg=NULL\n")` → `LOG_E("[HID] bond save failed: cfg=NULL");`
- `PRINT("[BOND] save failed: ble_idx=%x invalid\n", ...)` → `LOG_E("[HID] bond save failed: idx=%x invalid", ble_idx);`
- `PRINT("work mode err %x\n", ...)` → `LOG_E("[HID] work mode err %x", ble_idx);`
- `PRINT("[BOND] delete_by_idx: invalid idx=%x\n", ...)` → `LOG_E("[HID] bond delete: invalid idx=%x", idx);`

删除所有其他 PRINT 调用（约15条），包括：
- GAP 广告相关日志
- 连接状态变更日志
- 地址版本追踪日志
- 绑定保存成功日志
- 各种状态输出

- [ ] **Step 5: 清理 battery.c**

全部删除：
- `dprintf("ADC值(4次平均): %d\n", g_adc_value)`
- `dprintf("电压: %d\n", g_battery_voltage)`
- `dprintf("电量百分比: %d\n", g_battery_percentage)`

- [ ] **Step 6: 清理 storage.c**

- `dprintf("storage_init: ble_bond_flag=%d,ble_addr_ver[...]")` → `LOG_I("[STOR] init bond=%d ver=[%d,%d,%d]", ...);`
- `dprintf("storage_save: success=%d\n", success)` → 删除

- [ ] **Step 7: 清理 backlight_hal.c**

- `dprintf("pwm_stop\r\n")` → 删除

- [ ] **Step 8: Commit**

```bash
git add drivers/input/keyboard/matrix.c drivers/input/keyboard/debounce.c drivers/communication/bluetooth/ch584/_bt_driver.c drivers/communication/bluetooth/ch584/hidkbd.c drivers/power/battery.c drivers/storage/storage.c drivers/output/backlight/backlight_hal.c
git commit -m "refactor(logging): normalize drivers logs"
```

---

## Task 9: 编译验证

- [ ] **Step 1: 使用 /wch-riscv-build 编译**

使用编译技能验证所有改动编译通过。重点关注：
- LOG_E/LOG_W/LOG_I 宏展开是否正确
- 删除日志后是否有未使用的变量警告
- PRINT 宏替换为 LOG 宏后是否缺少头文件包含

- [ ] **Step 2: 修复编译错误（如有）**

常见问题：
- 某些文件缺少 `#include "debug.h"` → 添加
- 删除日志后参数变量未使用 → 添加 `(void)var;` 抑制警告
- PRINT 和 LOG 宏的格式化参数不匹配 → 调整

- [ ] **Step 3: 最终 Commit**

```bash
git add -A
git commit -m "fix(logging): fix compilation errors after log normalization"
```
