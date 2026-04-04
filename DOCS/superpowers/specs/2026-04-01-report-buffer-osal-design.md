# HID 报告缓冲 + OSAL 调度发送设计

> 日期: 2026-04-01
> 状态: 待实施
> 范围: report.c, wireless.c, communication_service.c, 构建配置

## 背景

键盘扫描在 main while 循环的 `peripheral_process()` 中执行。经测试发现，键盘长按时单次扫描处理时间（1-5ms）加上同步 BT 报告发送，会阻塞 `OSAL_SystemProcessOnce()`（BLE 协议栈处理），导致 BLE supervision timeout（错误码 0x08 = LL_CONN_TIMEOUT）断连。

### 根因分析

当前 `report.c` 直接调用 `wireless_transport.send_keyboard/send_consumer`，完全绕过了 `wireless.c` 中的 `wireless_send_keyboard()` 缓冲路径：

```
keyboard_scan() → report_update_proc()
    → wireless_transport.send_keyboard()    // 直接调用 BT 驱动
    → bt_driver_send_keyboard()             // 同步 BLE 写入，阻塞主循环
    → OSAL_SystemProcessOnce() 被延迟       // BLE 协议栈饿死
    → supervision timeout → 0x08 断连
```

即使启用现有 report_buffer，`wireless_send_keyboard()` 在队列为空时仍同步调用 `report_buffer_task()`，且该函数没有被 OSAL 定期调度。

### 调试补充结论（2026-04-01）

在将发送链路迁移为 `REPORT_SEND_EVT` 后，又暴露出一个新的 BLE 发送调度问题：

1. `_bt_driver.c` 日志中的 `ret=22` 实际对应 `blePending (0x16)`，表示前一次 GATT Notification 仍在处理中，并非 ATT 长度错误。
2. `report_buffer_task()` 当前会在**每次成功出队后立即预置 `retry = RETPORT_RETRY_COUNT`**，而不是在发送失败后才进入重试。
3. 这套“发送后等待 ACK 再清零 retry”的旧语义来自历史无线链路；当前 BLE 路径没有对应 ACK 回调来调用 `report_buffer_set_retry(0)`。
4. 结果就是同一份键盘报告会被 `REPORT_SEND_EVT` 持续重复提交给 BLE 栈，BLE 栈返回 `blePending`，日志连续刷出 `send_keyboard fail, ret=22`，并造成按键重复发送。

因此，这次修复除了保留“入队 + OSAL 调度”的主方案外，还必须同步修正 `report_buffer_task()` 的重试语义。

## 设计决策

### 1. 报告缓冲范围

| 报告类型 | 是否缓冲 | 原因 |
|----------|----------|------|
| keyboard | 是 | 主要阻塞源，1-3ms 延时对按键无感知 |
| consumer | 是 | 与 keyboard 相同路径 |
| mouse | 否 | 触控板报告对延时敏感，且 BT 发送本身不耗时 |

### 2. 发送调度方式

使用 OSAL 事件驱动：报告入队时设置 `REPORT_SEND_EVT`，由 OSAL 任务处理发送。相比定时轮询，事件驱动响应更快且不浪费 CPU。

### 3. BLE 重试策略修正

`report_buffer_task()` 改为“按发送结果决定是否保留当前报告”：

- `SUCCESS`：当前报告发送完成，清空当前 in-flight 报告，不再自动重发。
- `blePending` / `bleNoResources` / `bleTimeout`：认为是临时忙态，保留当前报告并延后重试。
- 其他错误：认为是不可恢复错误，丢弃当前报告并清空重试状态，避免单份坏报告长期阻塞队列。

`commu_process_event()` 在缓冲区仍有数据或当前报告仍需重试时，不再立即自触发，而是优先使用短延迟事件继续推进，避免在 `report_interval` 未到时形成空转。

## 修改方案

### 修改 1: report.c — 路由到 wireless 层

将直接调用 transport 改为通过 wireless 层的缓冲路径：

```c
// report.c report_update_proc() 中

// 键盘报告（原: wireless_transport.send_keyboard）
wireless_send_keyboard(&kb_report);

// consumer 报告（原: wireless_transport.send_consumer）
wireless_send_consumer(consumer_report);
```

**涉及函数：**
- `report.c:report_update_proc()` — 键盘报告发送（约第 533 行）
- `report.c:report_update_proc()` — consumer 报告发送（约第 515 行）

### 修改 2: wireless.c — 纯入队，去掉同步发送

`wireless_send_keyboard()` 修改：

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

`wireless_send_consumer()` 同理：去掉直接发送逻辑，入队后触发 `OSAL_SetEvent(commu_taskID, REPORT_SEND_EVT)`。

`wireless_task()` 和 `send_string_task()` 中的 `report_buffer_task()` 调用移除，改为由 `commu_process_event` 统一处理。

### 修改 3: communication_service — 在现有 commu_taskID 上新增 REPORT_SEND_EVT

不新增 OSAL 任务，在现有 `commu_process_event` 中添加报告发送事件处理。`commu_taskID` 已在 `communication_service.h` 中 extern 声明，`wireless.c` 可直接使用。

```c
// communication_service.h — 新增事件定义
// bit 7 可用（bit 0-6 已用于 WL_STS_*, bit 10-12 已用于 WL_REQ_*）
#define REPORT_SEND_EVT  (1 << 7)

// communication_service.c commu_process_event() 中新增处理
if (events & REPORT_SEND_EVT) {
    report_buffer_task();
    // 缓冲区非空时继续触发，确保所有报告发送完毕
    if (!report_buffer_is_empty() || report_buffer_get_retry() > 0) {
        OSAL_SetEvent(commu_taskID, REPORT_SEND_EVT);
    }
    return (events ^ REPORT_SEND_EVT);
}
```

`wireless.c` 中直接使用 extern 的 `commu_taskID` 触发事件：
```c
OSAL_SetEvent(commu_taskID, REPORT_SEND_EVT);
```

### 修改 4: 构建配置 — 启用 report_buffer

取消 `DISABLE_REPORT_BUFFER` 宏定义（在产品配置或编译选项中移除 `-DDISABLE_REPORT_BUFFER`）。

### 修改 5: report_buffer.c — 调整 RAM 占用

当前队列大小 256 条报告占用 8704 字节 RAM。对于 CH584M（SRAM 较小），需要缩小队列：

- 典型宏序列最长约 15 个字符 = 30 条报告（按下+释放）
- 将 `REPORT_BUFFER_QUEUE_SIZE` 从 256 降为 32（约 1088 字节）

## 改动后的数据流

```
main while(1):
  ┌──────────────────────────────────────────────┐
  │ OSAL_SystemProcessOnce()                     │
  │   ├── BLE 协议栈处理                         │
  │   ├── commu_process_event()                  │
  │   │   ├── WL_STS_*/WL_REQ_* 现有事件        │
  │   │   └── REPORT_SEND_EVT [NEW]              │
  │   │       └── report_buffer_task()           │
  │   │           └── wireless_transport.send()  │
  │   └── 其他 OSAL 任务                         │
  │                                              │
  │ peripheral_process()                         │
  │   ├── keyboard_scan()                        │
  │   │   └── report_update_proc()               │
  │   │       └── wireless_send_keyboard()       │
  │   │           └── enqueue + SetEvent 快速返回│
  │   ├── touchpad_task()                        │
  │   └── indicator_task()                       │
  └──────────────────────────────────────────────┘
```

**关键改进：**
- `peripheral_process()` 中无 BT 发送阻塞，执行时间缩短
- BT 发送由 OSAL 调度，与 BLE 协议栈交错执行
- BLE 协议栈获得足够的处理时间

## 风险评估

| 风险 | 影响 | 缓解措施 |
|------|------|----------|
| 报告入队到发送的延迟增加 | 1-3ms 额外延时 | 对按键操作无感知，可接受 |
| 缓冲区满时丢报告 | 宏序列可能不完整 | 队列大小 32 足以覆盖典型宏 |
| OSAL 事件处理不及时 | 报告堆积 | 报告发送自我续触发（非空时继续 SetEvent） |
| report_buffer RAM 占用 | CH584M SRAM 有限 | 队列从 256 缩减到 32（约 1088 字节） |
| BLE 忙态导致重复发送 | 单键被连续上报，日志刷 `ret=22` | 仅在临时失败时保留当前报告，成功后立即清空 retry |

## 验证方法

1. GPIO 翻转测量：对比修改前后 `peripheral_process()` 执行时间
2. BLE 连接稳定性测试：长按 30 秒以上，确认不再 0x08 断连
3. 宏序列测试：验证缓冲后的宏序列完整发送
4. 触控板测试：确认 mouse 报告不受影响
