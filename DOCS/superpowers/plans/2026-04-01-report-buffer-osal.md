# HID 报告缓冲 + OSAL 调度发送 实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将 HID keyboard/consumer 报告从同步直接发送改为缓冲 + OSAL 事件驱动异步发送，并修复 BLE 路径下单份报告被重复发送的问题。

**Architecture:** report.c 不再直接调用 wireless_transport.send_xxx，改走 wireless_send_keyboard/consumer() 入队路径。OSAL 事件 REPORT_SEND_EVT 在现有 commu_process_event 中处理，调用 report_buffer_task() 从缓冲区取报告并发送；`report_buffer_task()` 只在临时发送失败时保留当前报告重试，成功后立即清空重试态。

**Tech Stack:** C, CH584M BLE, OSAL 事件系统, 现有 report_buffer 队列

**Spec:** `docs/superpowers/specs/2026-04-01-report-buffer-osal-design.md`

---

## 文件变更清单

| 文件 | 操作 | 职责 |
|------|------|------|
| `keyboards/kb904/config_product.h` | 修改 | 定义 `REPORT_BUFFER_QUEUE_SIZE=32` |
| `middleware/communication/report_buffer.h` | 修改 | 队列大小改为 32 |
| `middleware/communication/report_buffer.c` | 修改 | 队列默认值改为 32 |
| `middleware/keyboard/report.c` | 修改 | 改用 wireless_send_keyboard/consumer() |
| `middleware/communication/wireless.c` | 修改 | 去掉同步发送，纯入队 + OSAL_SetEvent |
| `application/service/communication_service.h` | 修改 | 新增 REPORT_SEND_EVT 定义 |
| `application/service/communication_service.c` | 修改 | 新增 REPORT_SEND_EVT 处理 |
| `application/system/system_init.c` | 修改 | 取消注释 report_buffer_init() |
| `test/report_buffer_retry_test.c` | 新增 | 回归测试：单份报告不得被自动重复发送 |
| `test/stubs/report_buffer/*.h` | 新增 | report_buffer 单测桩头文件 |

---

### Task 0: 添加回归测试锁定重复发送问题

**Files:**
- Create: `test/report_buffer_retry_test.c`
- Create: `test/stubs/report_buffer/report.h`
- Create: `test/stubs/report_buffer/bt_driver.h`
- Create: `test/stubs/report_buffer/kb904/config_product.h`
- Create: `test/stubs/report_buffer/timer.h`
- Create: `test/stubs/report_buffer/PMU.h`

- [ ] **Step 1: 编写“单份键盘报告成功发送后不得自动重发”的失败测试**

覆盖场景：

- 键盘报告首次发送成功后，再次调用 `report_buffer_task()` 不得再次发送同一报告。
- 键盘报告首次返回 `blePending` 后允许重试；一旦重试成功，后续不得继续重复发送。

- [ ] **Step 2: 运行单测，确认红灯**

```bash
gcc -o test/report_buffer_retry_test.exe test/report_buffer_retry_test.c middleware/communication/report_buffer.c -Itest/stubs/report_buffer -Imiddleware/communication -Idrivers/system
./test/report_buffer_retry_test.exe
```

预期结果：当前实现失败，暴露“成功后仍自动重发”问题。

- [ ] **Step 3: Commit**

```bash
git add test/report_buffer_retry_test.c test/stubs/report_buffer
git commit -m "test(report-buffer): add regression coverage for duplicate BLE report sending"
```

---

### Task 1: 配置 report_buffer 队列大小

**Files:**
- Modify: `middleware/communication/report_buffer.c:32-34`

- [ ] **Step 1: 修改 report_buffer.c 队列默认值为 32**

将 `REPORT_BUFFER_QUEUE_SIZE` 默认值从 256 改为 32：

```c
// report_buffer.c 约第 32 行
#ifndef REPORT_BUFFER_QUEUE_SIZE
#    define REPORT_BUFFER_QUEUE_SIZE 32
#endif
```

- [ ] **Step 2: Commit**

```bash
git add middleware/communication/report_buffer.c
git commit -m "refactor(report-buffer): reduce queue size from 256 to 32 for CH584M RAM"
```

---

### Task 2: 启用 report_buffer_init 并移除构建配置中的 DISABLE_REPORT_BUFFER

**Files:**
- Modify: `application/system/system_init.c:105`

- [ ] **Step 1: 取消注释 system_init.c 中的 report_buffer_init()**

```c
// system_init.c system_init_middleware() 中，约第 104-105 行
// 将注释掉的初始化取消注释：
    // 3. 报告缓冲区初始化
    report_buffer_init();
```

- [ ] **Step 2: Commit**

```bash
git add application/system/system_init.c
git commit -m "feat(report-buffer): enable report_buffer_init in system startup"
```

---

### Task 3: communication_service 新增 REPORT_SEND_EVT

**Files:**
- Modify: `application/service/communication_service.h`
- Modify: `application/service/communication_service.c`

- [ ] **Step 1: 在 communication_service.h 中新增 REPORT_SEND_EVT 事件定义**

在事件定义区域（WL_STS_ADV_END 之后，WL_REQ_PAIR 之前）添加：

```c
// communication_service.h 约第 22 行，WL_STS_ADV_END 之后添加：

/* 报告缓冲发送事件 */
#define REPORT_SEND_EVT            (1 << 7)   // 报告缓冲区有数据待发送
```

- [ ] **Step 2: 在 communication_service.c 中新增 REPORT_SEND_EVT 处理**

在 `commu_process_event()` 函数中，`WL_STS_ADV_END` 处理之后、`WL_REQ_PAIR` 之前添加：

```c
    // communication_service.c commu_process_event() 中
    // 在 WL_STS_ADV_END 处理块之后添加：

    /* ---- 报告缓冲发送 ---- */
    if (events & REPORT_SEND_EVT) {
        report_buffer_task();
        // 缓冲区非空或还有重试，继续触发
        if (!report_buffer_is_empty() || report_buffer_get_retry() > 0) {
            OSAL_SetEvent(commu_taskID, REPORT_SEND_EVT);
        }
        return (events ^ REPORT_SEND_EVT);
    }
```

确保 `communication_service.c` 顶部已包含 `report_buffer.h`（当前第 8 行已有）。

- [ ] **Step 3: Commit**

```bash
git add application/service/communication_service.h application/service/communication_service.c
git commit -m "feat(communication): add REPORT_SEND_EVT to commu_process_event for async report sending"
```

---

### Task 4: wireless.c — 改为纯入队 + OSAL 事件触发

**Files:**
- Modify: `middleware/communication/wireless.c`

- [ ] **Step 1: 添加 communication_service.h 头文件引用**

在 wireless.c 顶部的 include 区域添加：

```c
#include "communication_service.h"
```

- [ ] **Step 2: 修改 wireless_send_keyboard() — 纯入队 + OSAL 事件**

将当前的 `wireless_send_keyboard()` 函数（约第 383-407 行）替换为：

```c
void wireless_send_keyboard(report_keyboard_t *report) {
    if (wireless_state == WT_PARING && !pincodeEntry) return;

    if (wireless_state == WT_CONNECTED || (wireless_state == WT_PARING && pincodeEntry)) {
        if (wireless_transport.send_keyboard) {
            report_buffer_t report_buffer;
            report_buffer.type = REPORT_TYPE_KB;
            memcpy(&report_buffer.keyboard, report, sizeof(report_keyboard_t));
            report_buffer_enqueue(&report_buffer);

            // 触发 OSAL 事件，异步发送
            OSAL_SetEvent(commu_taskID, REPORT_SEND_EVT);
        }
    } else if (wireless_state != WT_RESET) {
        wireless_connect();
    }
}
```

- [ ] **Step 3: 修改 wireless_send_consumer() — 纯入队 + OSAL 事件**

将当前的 `wireless_send_consumer()` 函数（约第 453-471 行）替换为：

```c
void wireless_send_consumer(uint16_t data) {
    if (wireless_state == WT_CONNECTED) {
        if (wireless_transport.send_consumer) {
            report_buffer_t report_buffer;
            report_buffer.type     = REPORT_TYPE_CONSUMER;
            report_buffer.consumer = data;
            report_buffer_enqueue(&report_buffer);

            // 触发 OSAL 事件，异步发送
            OSAL_SetEvent(commu_taskID, REPORT_SEND_EVT);
        }
    } else if (wireless_state != WT_RESET) {
        wireless_connect();
    }
}
```

- [ ] **Step 4: 清理 wireless_task() 和 send_string_task() 中的 report_buffer_task() 调用**

`wireless_task()`（约第 511-520 行）中移除 `report_buffer_task()` 调用：

```c
void wireless_task(void) {
    // report_buffer_task() 已迁移到 commu_process_event 的 REPORT_SEND_EVT 处理
}
```

`send_string_task()`（约第 522-529 行）同样移除：

```c
void send_string_task(void) {
    if ((get_transport() & TRANSPORT_WIRELESS) && wireless_get_state() == WT_CONNECTED) {
        // report_buffer_task() 已迁移到 commu_process_event 的 REPORT_SEND_EVT 处理
    }
}
```

- [ ] **Step 5: Commit**

```bash
git add middleware/communication/wireless.c
git commit -m "refactor(wireless): change send_keyboard/consumer to pure enqueue with OSAL event trigger"
```

---

### Task 5: report.c — 路由到 wireless 层

**Files:**
- Modify: `middleware/keyboard/report.c`

- [ ] **Step 1: 修改 consumer 报告发送**

在 `report_update_proc()` 中（约第 511-522 行），将直接调用 transport 改为调用 wireless 层函数：

```c
    // 检查并发送消费者报告（约第 511-522 行）
    if (consumer_report != last_consumer_report) {
        last_consumer_report = consumer_report;
        dprintf("Report: Consumer report changed, sending update:%04X\r\n", consumer_report);
        wireless_send_consumer(consumer_report);
    }
```

替换原来的：
```c
    if (consumer_report != last_consumer_report) {
        last_consumer_report = consumer_report;
        dprintf("Report: Consumer report changed, sending update:%04X\r\n", consumer_report);
        if (wireless_transport.send_consumer) {
            wireless_transport.send_consumer(consumer_report);
        }
    }
```

- [ ] **Step 2: 修改 keyboard 报告发送**

在 `report_update_proc()` 中（约第 524-536 行），将直接调用 transport 改为调用 wireless 层函数：

```c
    // 检查并发送键盘报告（约第 524-536 行）
    if (memcmp(&kb_report, &last_kb_report, sizeof(kb_report)) != 0) {
        memcpy(&last_kb_report, &kb_report, sizeof(kb_report));
        dprintf("sending keycode:%d,%d,%d,%d,%d,%d,%d,%d\r\n", kb_report.mods,kb_report.reserved,
                kb_report.keys[0], kb_report.keys[1], kb_report.keys[2], kb_report.keys[3], kb_report.keys[4], kb_report.keys[5]);
        wireless_send_keyboard(&kb_report);
    }
```

替换原来的：
```c
    if (memcmp(&kb_report, &last_kb_report, sizeof(kb_report)) != 0) {
        memcpy(&last_kb_report, &kb_report, sizeof(kb_report));
        dprintf("sending keycode:%d,%d,%d,%d,%d,%d,%d,%d\r\n", kb_report.mods,kb_report.reserved,
                kb_report.keys[0], kb_report.keys[1], kb_report.keys[2], kb_report.keys[3], kb_report.keys[4], kb_report.keys[5]);
        if (wireless_transport.send_keyboard) {
            wireless_transport.send_keyboard((uint8_t *)&kb_report);
        }
    }
```

- [ ] **Step 3: 移除不再需要的 wireless_transport extern 声明**

report.c 第 456 行的 `extern wt_func_t wireless_transport;` 可以移除（改用 wireless.h 中声明的函数接口），但需确认该文件其他位置是否还直接使用 wireless_transport。检查后如无其他引用则删除该行。

- [ ] **Step 4: Commit**

```bash
git add middleware/keyboard/report.c
git commit -m "refactor(report): route keyboard/consumer reports through wireless layer instead of direct transport"
```

---

### Task 6: 编译验证

**Files:** 无新增修改

- [ ] **Step 1: 使用 MounRiver Studio 工具链编译**

```bash
cmd.exe /c "cd /d D:\\Code\\C_Project\\keyboard-framework\\project\\ch584m\\obj && \
  set PATH=D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\components\\WCH\\Toolchain\\RISC-V Embedded GCC12\\bin;D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\others\\Build_Tools\\Make\\bin;%PATH% && \
  make.exe clean && make.exe main-build"
```

预期结果：编译通过，无链接错误。

- [ ] **Step 2: 检查编译输出中的 RAM 占用**

确认 `report_buffer_queue` 的 RAM 占用从 ~8704 字节降至 ~1088 字节（32 * sizeof(report_buffer_t)）。

- [ ] **Step 3: 如编译失败则修复**

常见问题：
- `commu_taskID` 未声明：确认 wireless.c 包含了 `communication_service.h`
- `wireless_send_keyboard` 未声明：确认 report.c 包含了 `wireless.h`（第 22 行已有）
- `REPORT_SEND_EVT` 未声明：确认 communication_service.c 包含了自己的头文件
- `report_buffer_retry_test.exe` 失败：优先检查 retry 清零条件是否只发生在成功路径

---

### Task 7: 最终 Commit（如 Task 6 需要额外修复）

- [ ] **Step 1: Commit 所有修复**

```bash
git add -A
git commit -m "fix(report-buffer): resolve build issues after buffer+OSAL migration"
```

---

## 实施顺序依赖

```
Task 1 (队列大小) ──┐
Task 2 (init 启用) ──┤
Task 3 (OSAL 事件) ──┼── Task 6 (编译验证) ── Task 7 (修复)
Task 4 (wireless.c) ─┤
Task 5 (report.c) ───┘
```

Task 1-5 相互独立，可以并行实施。Task 6 依赖所有前置任务完成。
