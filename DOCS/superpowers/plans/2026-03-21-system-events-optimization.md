# System Events Optimization Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 删除 3 个冗余系统事件（SYSTEM_IDLE_EVT, SYSTEM_DEEP_SLEEP_EVT, SYSTEM_WAKEUP_EVT），统一使用 LPM 事件族管理睡眠/唤醒。

**Architecture:** 将睡眠/唤醒操作统一到 LPM 状态机模型。SYSTEM_DEEP_SLEEP_EVT 的所有调用点迁移到 SYSTEM_LPM_DEEP_REQ_EVT；SYSTEM_WAKEUP_EVT 的逻辑已由 SYSTEM_LPM_WAKE_EVT 完全覆盖。

**Tech Stack:** C (嵌入式), OSAL 事件框架, CH584M 平台

---

## File Structure

| 文件 | 修改类型 | 职责 |
|------|----------|------|
| `application/service/system_service.h` | 删除定义 | 移除 3 个事件枚举值 |
| `application/service/system_service.c` | 删除处理 | 移除 3 个事件处理分支 |
| `middleware/communication/wireless_callbacks.c` | 替换调用 | 更新延迟事件函数 |
| `drivers/communication/bluetooth/ch584/hidkbd.c` | 替换调用 | 更新超时常量和日志 |
| `drivers/communication/bluetooth/ch584/hidkbd.h` | 删除定义 | 移除超时宏定义 |

---

## Task 1: 更新 wireless_callbacks.c 中的延迟事件函数

**Files:**
- Modify: `middleware/communication/wireless_callbacks.c:196-215`

- [ ] **Step 1: 更新 access_ble_schedule_deep_sleep_evt 函数**

将 `SYSTEM_DEEP_SLEEP_EVT` 替换为 `SYSTEM_LPM_DEEP_REQ_EVT`:

```c
void access_ble_schedule_deep_sleep_evt(uint32_t delay_ticks) {
    if (system_taskID == 0xFF) {
        dprintf("[WT_SLEEP] system task not ready, skip schedule delay=%lu\r\n", (unsigned long)delay_ticks);
        return;
    }

    OSAL_SetDelayedEvent(system_taskID, SYSTEM_LPM_DEEP_REQ_EVT, delay_ticks);
    dprintf("[WT_SLEEP] schedule SYSTEM_LPM_DEEP_REQ_EVT delay=%lu\r\n", (unsigned long)delay_ticks);
}
```

- [ ] **Step 2: 更新 access_ble_cancel_deep_sleep_evt 函数**

将 `SYSTEM_DEEP_SLEEP_EVT` 替换为 `SYSTEM_LPM_DEEP_REQ_EVT`:

```c
void access_ble_cancel_deep_sleep_evt(void) {
    if (system_taskID == 0xFF) {
        return;
    }

    if (OSAL_GetTaskTimer(system_taskID, SYSTEM_LPM_DEEP_REQ_EVT)) {
        OSAL_StopTask(system_taskID, SYSTEM_LPM_DEEP_REQ_EVT);
        dprintf("[WT_SLEEP] cancel SYSTEM_LPM_DEEP_REQ_EVT\r\n");
    }
}
```

---

## Task 2: 更新 hidkbd.c/h 中的超时调用

**Files:**
- Modify: `drivers/communication/bluetooth/ch584/hidkbd.h:71`
- Modify: `drivers/communication/bluetooth/ch584/hidkbd.c:1038-1040`

- [ ] **Step 1: 在 hidkbd.h 中替换超时常量名**

将 `SYSTEM_DEEP_SLEEP_EVT_TIMEOUT` 重命名为 `LPM_DEEP_REQ_DELAY_TICKS`:

```c
// 旧定义（删除）
// #define SYSTEM_DEEP_SLEEP_EVT_TIMEOUT          (1600 * 60)

// 新定义
#define LPM_DEEP_REQ_DELAY_TICKS              (1600 * 60)
```

- [ ] **Step 2: 在 hidkbd.c 中更新函数调用**

文件: `hidkbd.c:1038-1040`

```c
// 旧代码
access_ble_schedule_deep_sleep_evt(SYSTEM_DEEP_SLEEP_EVT_TIMEOUT);
dprint("[TRACE_ADV] set SYSTEM_DEEP_SLEEP_EVT timeout=%d req=%x idx=%x\n",
       SYSTEM_DEEP_SLEEP_EVT_TIMEOUT, initial_advertising_enable, access_state.ble_idx);

// 新代码
access_ble_schedule_deep_sleep_evt(LPM_DEEP_REQ_DELAY_TICKS);
dprint("[TRACE_ADV] set LPM_DEEP_REQ delay=%d req=%x idx=%x\n",
       LPM_DEEP_REQ_DELAY_TICKS, initial_advertising_enable, access_state.ble_idx);
```

---

## Task 3: 删除 system_service.h 中的冗余事件定义

**Files:**
- Modify: `application/service/system_service.h:20-39`

- [ ] **Step 1: 删除三个事件枚举值**

删除以下三行:
- Line 23: `SYSTEM_IDLE_EVT = 1 << 1,`
- Line 25: `SYSTEM_DEEP_SLEEP_EVT = 1 << 3,`
- Line 27: `SYSTEM_WAKEUP_EVT = 1 << 5,`

修改后的枚举:

```c
typedef enum {
    /* 基础系统事件 (bits 0-7) */
    SYSTEM_LOW_BATTERY_SHUTDOWN_EVT = 1 << 0,  /**< 低电关机事件 */
    /* bit 1 预留 */
    SYSTEM_SHUTDOWN_EVT              = 1 << 2,  /**< 系统关机事件 */
    /* bit 3 预留，请使用 SYSTEM_LPM_DEEP_REQ_EVT */
    SYSTEM_STORAGE_EVT               = 1 << 4,  /**< 系统存储事件 */
    /* bit 5 预留，请使用 SYSTEM_LPM_WAKE_EVT */
    SYSTEM_FACTORY_RESET_EVT         = 1 << 6,  /**< 恢复出厂设置事件 */
    SYSTEM_OTA_EVT                   = 1 << 7,  /**< OTA升级事件 */

    /* LPM 调度事件（bits 8-14） */
    SYSTEM_LPM_IDLE_REQ_EVT          = 1 << 8,  /**< 请求进入 Idle */
    SYSTEM_LPM_DEEP_REQ_EVT          = 1 << 9,  /**< 请求进入 Deep */
    SYSTEM_LPM_STEP_DONE_EVT         = 1 << 10, /**< prepare 完成汇聚 */
    SYSTEM_LPM_ENTER_IDLE_EVT        = 1 << 11, /**< 最终进入 Idle */
    SYSTEM_LPM_ENTER_DEEP_EVT        = 1 << 12, /**< 最终进入 Deep */
    SYSTEM_LPM_WAKE_EVT              = 1 << 13, /**< 唤醒恢复入口 */
    SYSTEM_LPM_CHECK_EVT             = 1 << 14, /**< LPM 周期检查 */
} system_task_event_t;
```

- [ ] **Step 2: 更新文件头注释**

更新描述以反映新架构:

```c
/**
 * @file system_service.h
 * @brief 系统服务事件定义
 *
 * 所有系统相关服务事件：
 * - 低电关机
 * - 关机
 * - 存储
 * - 恢复出厂设置
 * - OTA
 * - LPM 调度事件（Idle/Deep 睡眠、唤醒）
 */
```

---

## Task 4: 删除 system_service.c 中的冗余事件处理

**Files:**
- Modify: `application/service/system_service.c`

- [ ] **Step 1: 删除 SYSTEM_DEEP_SLEEP_EVT 处理分支**

删除 lines 72-78:

```c
// 删除以下代码块
// 处理深度睡眠事件（保留兼容，转发到 LPM 调度）
if (events & SYSTEM_DEEP_SLEEP_EVT) {
    /* 转发为 LPM Deep 请求，由 LPM 状态机处理 */
    dprintf("System: Legacy deep sleep event, forwarding to LPM\r\n");
    OSAL_SetEvent(system_taskID, SYSTEM_LPM_DEEP_REQ_EVT);
    return (events ^ SYSTEM_DEEP_SLEEP_EVT);
}
```

- [ ] **Step 2: 删除 SYSTEM_WAKEUP_EVT 处理分支**

删除 lines 88-101:

```c
// 删除以下代码块
// 处理系统唤醒事件
if (events & SYSTEM_WAKEUP_EVT) {
    println("System: System wakeup");
    // 唤醒恢复流程:
    // 1. 从存储读取配置
    storage_init();
    // 2. 重置低功耗定时器
    lpm_timer_reset();
    // 3. 根据传输模式恢复连接
    if (get_transport() == TRANSPORT_BLUETOOTH) {
        wireless_connect();
    }
    return (events ^ SYSTEM_WAKEUP_EVT);
}
```

---

## Task 5: 验证编译

**Files:**
- Test: 编译验证

- [ ] **Step 1: 执行标准构建**

```bash
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build -j4
```

Expected: 编译成功，无 undefined symbol 错误

- [ ] **Step 2: 全局搜索确认无遗漏**

```bash
# 确认旧事件名称完全消失
grep -r "SYSTEM_DEEP_SLEEP_EVT" --include="*.c" --include="*.h" .
grep -r "SYSTEM_WAKEUP_EVT" --include="*.c" --include="*.h" .
grep -r "SYSTEM_IDLE_EVT" --include="*.c" --include="*.h" .
```

Expected: 无匹配结果（除了文档文件）

---

## Task 6: 提交变更

**Files:**
- Commit all modified files

- [ ] **Step 1: 查看变更**

```bash
git status
git diff --stat
```

- [ ] **Step 2: 提交**

```bash
git add application/service/system_service.h \
        application/service/system_service.c \
        middleware/communication/wireless_callbacks.c \
        drivers/communication/bluetooth/ch584/hidkbd.c \
        drivers/communication/bluetooth/ch584/hidkbd.h

git commit -m "refactor(system): remove redundant system events

- Remove SYSTEM_IDLE_EVT (unused legacy event)
- Remove SYSTEM_DEEP_SLEEP_EVT (replaced by SYSTEM_LPM_DEEP_REQ_EVT)
- Remove SYSTEM_WAKEUP_EVT (replaced by SYSTEM_LPM_WAKE_EVT)
- Update wireless_callbacks.c to use new LPM events
- Rename SYSTEM_DEEP_SLEEP_EVT_TIMEOUT to LPM_DEEP_REQ_DELAY_TICKS

This simplifies the event table from 15 to 12 events and unifies
all sleep/wake operations through the LPM state machine."
```

---

## Summary

| Task | 文件 | 修改行数 |
|------|------|----------|
| Task 1 | wireless_callbacks.c | ~6 行 |
| Task 2 | hidkbd.c, hidkbd.h | ~4 行 |
| Task 3 | system_service.h | -3 行 |
| Task 4 | system_service.c | -24 行 |
| Task 5 | - | 编译验证 |
| Task 6 | - | Git 提交 |

**总修改量**: ~34 行代码删除/修改，低风险
