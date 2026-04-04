# 键盘与触控板统一发送流控 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在不主动删帧的前提下，为键盘和触控板补齐统一的 BLE 发送窗口控制、真实长度保持和续调度闭环，消除连续滑动触控板时的 `blePending` 堆积失败。

**Architecture:** 保持现有 `wireless -> report_buffer -> REPORT_SEND_EVT -> bt_driver` 的统一发送路径，不新增 PTP 专用通道。`report_buffer` 增加“当前报文 + inflight 窗口”状态，成功提交后等待链路窗口释放再处理下一帧；键盘和触控板继续全部入队，同时在 `wireless.c` 中保留各自最新帧镜像，但当前阶段不用于覆盖旧帧。

**Tech Stack:** C11, CH584 BLE HID, OSAL 事件系统, 现有 `report_buffer` 队列, WCH BLE SDK `LL_GetNumberOfUnAckPacket`

**Spec:** `docs/plans/2026-04-02-touchpad-ptp-flow-control-simplification-design.md`

---

## 文件变更清单

| 文件 | 操作 | 职责 |
|------|------|------|
| `test/report_buffer_retry_test.c` | 修改 | 增加 PTP 长度保持、inflight 窗口阻塞、窗口释放后继续发送的单测 |
| `test/stubs/report_buffer/report.h` | 修改 | 为 report_buffer 单测补齐最小 `report_mouse_t` / `report_ptp_t` 结构定义 |
| `middleware/communication/wireless.h` | 修改 | 为 `wt_func_t` 增加链路未确认包查询回调，保持 middleware 对驱动的抽象依赖 |
| `middleware/communication/wireless.c` | 修改 | 入队时保存真实长度、检查入队失败、维护键盘/PTP 最新帧镜像、注册 BLE 窗口查询回调 |
| `middleware/communication/report_buffer.h` | 修改 | 为队列项增加 `payload_len`，新增“是否仍有待处理工作”查询接口 |
| `middleware/communication/report_buffer.c` | 修改 | 引入 `current_report/current_inflight` 状态，发送成功后等待窗口释放再出队下一帧 |
| `application/service/communication_service.c` | 修改 | `REPORT_SEND_EVT` 改为根据 `report_buffer` 待处理状态持续续调度 |
| `drivers/communication/bluetooth/ch584/_bt_driver.h` | 修改 | 声明 BLE 未确认包查询接口 |
| `drivers/communication/bluetooth/ch584/_bt_driver.c` | 修改 | 封装 `LL_GetNumberOfUnAckPacket()` 并对 busy 状态降噪日志 |

---

### Task 0: 扩单测锁定真实长度与窗口行为

**Files:**
- Modify: `test/report_buffer_retry_test.c`
- Modify: `test/stubs/report_buffer/report.h`

- [ ] **Step 1: 扩展夹具，增加 PTP 发送和窗口查询 mock**

先在 [test/stubs/report_buffer/report.h](D:/Code/VScode/keyboard-framework/test/stubs/report_buffer/report.h) 增加最小结构，保证单测可以独立编译：

```c
typedef struct __attribute__((packed)) {
    uint8_t button;
    uint8_t x_l8;
    uint8_t x_m4:4;
    uint8_t y_l4:4;
    uint8_t y_m8;
    uint8_t wheel;
    uint8_t twheel;
} report_mouse_t;

typedef struct __attribute__((packed)) {
    uint8_t raw[19];
} report_ptp_t;
```

然后在现有键盘 mock 基础上增加：

```c
static int      ptp_send_count = 0;
static uint8_t  last_ptp_len = 0;
static uint8_t  ptp_send_status[8];
static size_t   ptp_status_len = 0;
static size_t   ptp_status_index = 0;
static uint32_t unack_sequence[8];
static size_t   unack_sequence_len = 0;
static size_t   unack_sequence_index = 0;

static uint8_t mock_send_ptp(uint8_t *report, uint8_t len) {
    uint8_t status = 0;
    (void)report;
    ptp_send_count++;
    last_ptp_len = len;
    if (ptp_status_index < ptp_status_len) {
        status = ptp_send_status[ptp_status_index++];
    }
    return status;
}

static uint32_t mock_get_unack_packets(void) {
    if (unack_sequence_index < unack_sequence_len) {
        return unack_sequence[unack_sequence_index++];
    }
    return 0;
}

static void set_unack_sequence(const uint32_t *sequence, size_t length) {
    memset(unack_sequence, 0, sizeof(unack_sequence));
    memcpy(unack_sequence, sequence, length * sizeof(uint32_t));
    unack_sequence_len = length;
    unack_sequence_index = 0;
}
```

并在 `reset_fixture()` 里同步清理：

```c
ptp_send_count = 0;
last_ptp_len = 0;
ptp_status_len = 0;
ptp_status_index = 0;
unack_sequence_len = 0;
unack_sequence_index = 0;
wireless_transport.send_ptp = NULL;
wireless_transport.get_unack_packets = NULL;
```

- [ ] **Step 2: 新增“PTP 真实长度必须原样透传”的失败测试**

在 `test/report_buffer_retry_test.c` 追加：

```c
static void test_ptp_report_preserves_payload_len(void) {
    report_buffer_t report;

    reset_fixture();
    memset(&report, 0, sizeof(report));
    report.type = REPORT_TYPE_PTP;
    report.payload_len = 17;
    wireless_transport.send_ptp = mock_send_ptp;

    if (!report_buffer_enqueue(&report)) {
        fail("PTP 报告入队失败");
    }

    advance_time(5);
    report_buffer_task();

    expect_int(1, ptp_send_count, "PTP 报告应被发送一次");
    expect_u8(17, last_ptp_len, "PTP 发送长度必须保持为入队长度");
}
```

- [ ] **Step 3: 新增“上一帧仍占窗口时不得发送下一帧”的失败测试**

继续追加：

```c
static void test_inflight_blocks_next_report_until_window_released(void) {
    uint32_t unack_seq[] = {1, 1, 0};
    report_buffer_t first = make_keyboard_report(13);
    report_buffer_t second = make_keyboard_report(29);

    reset_fixture();
    wireless_transport.send_keyboard = mock_send_keyboard;
    wireless_transport.get_unack_packets = mock_get_unack_packets;
    set_unack_sequence(unack_seq, sizeof(unack_seq) / sizeof(unack_seq[0]));

    report_buffer_enqueue(&first);
    report_buffer_enqueue(&second);

    advance_time(5);
    report_buffer_task();
    expect_int(1, keyboard_send_count, "首帧应先发送");

    advance_time(5);
    report_buffer_task();
    expect_int(1, keyboard_send_count, "窗口未释放前不得发送第二帧");

    advance_time(5);
    report_buffer_task();
    expect_int(2, keyboard_send_count, "窗口释放后才允许继续发送第二帧");
}
```

- [ ] **Step 4: 运行单测，确认当前实现先失败**

Run:

```bash
gcc -o test/report_buffer_retry_test.exe test/report_buffer_retry_test.c middleware/communication/report_buffer.c -Itest/stubs/report_buffer -Itest/stubs -Imiddleware/communication -Imiddleware/keyboard -Icomponent/touch_component/include -Idrivers/system -Ikeyboards -Ihal -Iutils
./test/report_buffer_retry_test.exe
```

Expected:

- 现状至少有一条测试失败
- 失败点应落在“PTP 长度被写死”或“首帧成功后第二帧过早发送”

- [ ] **Step 5: Commit**

```bash
git add test/report_buffer_retry_test.c test/stubs/report_buffer/report.h
git commit -m "test(report-buffer): lock payload length and inflight window behavior"
```

---

### Task 1: 扩 transport 抽象，给 report_buffer 提供窗口观测能力

**Files:**
- Modify: `middleware/communication/wireless.h`
- Modify: `middleware/communication/wireless.c`
- Modify: `drivers/communication/bluetooth/ch584/_bt_driver.h`
- Modify: `drivers/communication/bluetooth/ch584/_bt_driver.c`

- [ ] **Step 1: 在 `wt_func_t` 中增加未确认包查询回调**

在 [wireless.h](D:/Code/VScode/keyboard-framework/middleware/communication/wireless.h) 的 `wt_func_t` 中追加：

```c
typedef struct {
    void (*init)(bool wakeup_from_low_power);
    void (*connect_ex)(uint8_t host_idx, uint16_t timeout);
    void (*pairing_ex)(uint8_t host_idx, void *param);
    void (*disconnect)(void);
    uint8_t (*send_keyboard)(uint8_t *report);
    uint8_t (*send_consumer)(uint16_t usage);
    uint8_t (*send_system)(uint16_t usage);
    uint8_t (*send_mouse)(uint8_t *report);
    uint8_t (*send_ptp)(uint8_t *report, uint8_t len);
    uint32_t (*get_unack_packets)(void);
    void (*update_bat_level)(uint8_t level);
} wt_func_t;
```

- [ ] **Step 2: 在蓝牙驱动头文件声明查询接口**

在 [\_bt_driver.h](D:/Code/VScode/keyboard-framework/drivers/communication/bluetooth/ch584/_bt_driver.h) 追加声明：

```c
uint32_t bt_driver_get_unack_packet_count(void);
```

- [ ] **Step 3: 在蓝牙驱动中封装 `LL_GetNumberOfUnAckPacket()`**

在 [\_bt_driver.c](D:/Code/VScode/keyboard-framework/drivers/communication/bluetooth/ch584/_bt_driver.c) 中新增：

```c
extern uint16_t hidEmuConnHandle;

uint32_t bt_driver_get_unack_packet_count(void) {
    if (hidEmuConnHandle == GAP_CONNHANDLE_INIT) {
        return 0;
    }
    return LL_GetNumberOfUnAckPacket(hidEmuConnHandle);
}
```

- [ ] **Step 4: 在 BLE 函数表注册该回调**

修改 [wireless.c](D:/Code/VScode/keyboard-framework/middleware/communication/wireless.c) 中 `bt_driver_func_table`：

```c
static const wt_func_t bt_driver_func_table = {
    .init = bt_driver_init,
    .connect_ex = bt_driver_connect_ex,
    .pairing_ex = bt_driver_pairing_ex,
    .disconnect = bt_driver_disconnect,
    .send_keyboard = bt_driver_send_keyboard,
    .send_consumer = bt_driver_send_consumer,
    .send_system = bt_driver_send_system,
    .send_mouse = bt_driver_send_mouse,
    .send_ptp = bt_driver_send_ptp,
    .get_unack_packets = bt_driver_get_unack_packet_count,
    .update_bat_level = bt_driver_update_bat_level,
};
```

保持非 BLE 路径默认 `NULL`，本阶段不扩展 2.4G 专用窗口逻辑。

- [ ] **Step 5: Commit**

```bash
git add middleware/communication/wireless.h middleware/communication/wireless.c drivers/communication/bluetooth/ch584/_bt_driver.h drivers/communication/bluetooth/ch584/_bt_driver.c
git commit -m "feat(wireless): expose BLE unack packet count through transport abstraction"
```

---

### Task 2: 扩 `report_buffer` 数据模型，保存真实长度与当前报文状态

**Files:**
- Modify: `middleware/communication/report_buffer.h`
- Modify: `middleware/communication/report_buffer.c`

- [ ] **Step 1: 为队列项增加真实发送长度字段**

修改 [report_buffer.h](D:/Code/VScode/keyboard-framework/middleware/communication/report_buffer.h)：

```c
typedef struct {
    uint8_t type;
    uint8_t payload_len;
    union {
        report_keyboard_t keyboard;
        uint16_t          consumer;
        report_ptp_t      ptp;
        report_mouse_t    mouse;
    };
} report_buffer_t;
```

- [ ] **Step 2: 增加“是否还有待处理工作”的查询接口**

在 [report_buffer.h](D:/Code/VScode/keyboard-framework/middleware/communication/report_buffer.h) 声明：

```c
bool report_buffer_has_pending_work(void);
```

- [ ] **Step 3: 在 `report_buffer.c` 中增加当前报文和 inflight 状态**

将当前全局 `kb_rpt` 扩展为更明确的状态：

```c
static report_buffer_t current_report;
static bool            current_report_valid = false;
static bool            current_inflight = false;
static uint32_t        current_send_time = 0;
```

初始化逻辑同步调整：

```c
memset(&current_report, 0, sizeof(current_report));
current_report_valid = false;
current_inflight = false;
current_send_time = 0;
retry = 0;
```

- [ ] **Step 4: 实现 `report_buffer_has_pending_work()`**

在 [report_buffer.c](D:/Code/VScode/keyboard-framework/middleware/communication/report_buffer.c) 增加：

```c
bool report_buffer_has_pending_work(void) {
    return !report_buffer_is_empty() || retry > 0 || current_report_valid || current_inflight;
}
```

- [ ] **Step 5: Commit**

```bash
git add middleware/communication/report_buffer.h middleware/communication/report_buffer.c
git commit -m "refactor(report-buffer): add payload length and current report state"
```

---

### Task 3: 调整无线入队路径，统一保存长度并保留最新帧镜像

**Files:**
- Modify: `middleware/communication/wireless.c`

- [ ] **Step 1: 在 `wireless.c` 内部维护键盘和 PTP 最新帧镜像**

在文件静态区增加：

```c
static report_keyboard_t latest_keyboard_report;
static bool              latest_keyboard_valid = false;
static report_ptp_t      latest_ptp_report;
static uint8_t           latest_ptp_len = 0;
static bool              latest_ptp_valid = false;
```

这些镜像本阶段只做状态保留，不参与覆盖队列旧帧。

- [ ] **Step 2: 所有入队路径都显式填写 `payload_len`**

按类型补齐：

```c
report_buffer.type = REPORT_TYPE_KB;
report_buffer.payload_len = sizeof(report_keyboard_t);
```

```c
report_buffer.type = REPORT_TYPE_CONSUMER;
report_buffer.payload_len = sizeof(report_buffer.consumer);
```

```c
report_buffer.type = REPORT_TYPE_MOUSE;
report_buffer.payload_len = sizeof(report_mouse_t);
```

```c
report_buffer.type = REPORT_TYPE_PTP;
report_buffer.payload_len = len;
```

- [ ] **Step 3: 键盘和 PTP 入队前同步刷新最新帧镜像**

修改 [wireless.c](D:/Code/VScode/keyboard-framework/middleware/communication/wireless.c) 中的发送入口：

```c
memcpy(&latest_keyboard_report, report, sizeof(report_keyboard_t));
latest_keyboard_valid = true;
```

```c
memcpy(&latest_ptp_report, report, sizeof(report_ptp_t));
latest_ptp_len = len;
latest_ptp_valid = true;
```

- [ ] **Step 4: 处理入队失败，不再静默吞掉**

将当前直接入队改为：

```c
if (!report_buffer_enqueue(&report_buffer)) {
    dprintf("[WT_QUEUE] enqueue fail type=%d len=%d\r\n", report_buffer.type, report_buffer.payload_len);
    return;
}

OSAL_SetEvent(commu_taskID, REPORT_SEND_EVT);
```

键盘、鼠标、消费者键、PTP 统一采用这个分支。

- [ ] **Step 5: Commit**

```bash
git add middleware/communication/wireless.c
git commit -m "refactor(wireless): persist payload length and latest frame shadows for queued reports"
```

---

### Task 4: 重写 `report_buffer_task()`，发送成功后等待窗口释放再取下一帧

**Files:**
- Modify: `middleware/communication/report_buffer.c`

- [ ] **Step 1: 为当前报文补一个统一发送分发函数**

在 [report_buffer.c](D:/Code/VScode/keyboard-framework/middleware/communication/report_buffer.c) 内新增：

```c
static uint8_t report_buffer_send_current(void) {
    if (current_report.type == REPORT_TYPE_CONSUMER && wireless_transport.send_consumer) {
        return wireless_transport.send_consumer(current_report.consumer);
    }
    if (current_report.type == REPORT_TYPE_KB && wireless_transport.send_keyboard) {
        return wireless_transport.send_keyboard(&current_report.keyboard.mods);
    }
    if (current_report.type == REPORT_TYPE_MOUSE && wireless_transport.send_mouse) {
        return wireless_transport.send_mouse((uint8_t *)&current_report.mouse);
    }
    if (current_report.type == REPORT_TYPE_PTP && wireless_transport.send_ptp) {
        return wireless_transport.send_ptp((uint8_t *)&current_report.ptp, current_report.payload_len);
    }
    return 0xFF;
}
```

- [ ] **Step 2: 在 `report_buffer_task()` 最前面先处理 inflight**

把窗口释放判断放到出队前：

```c
if (current_inflight) {
    uint32_t unack = 0;

    if (wireless_transport.get_unack_packets) {
        unack = wireless_transport.get_unack_packets();
    } else if (timer_elapsed32(current_send_time) <= report_interval) {
        return;
    }

    if (unack > 0) {
        return;
    }

    memset(&current_report, 0, sizeof(current_report));
    current_report_valid = false;
    current_inflight = false;
}
```

- [ ] **Step 3: 只有当前报文为空时才允许从队列取下一帧**

继续调整 `report_buffer_task()`：

```c
if (!current_report_valid) {
    if (!report_buffer_dequeue(&current_report)) {
        return;
    }
    current_report_valid = (current_report.type != REPORT_TYPE_NONE);
}

if (!current_report_valid) {
    return;
}
```

- [ ] **Step 4: 保持现有 retry 语义，但成功后进入 inflight 而不是立即清空**

发送结果处理改为：

```c
send_status = report_buffer_send_current();

if (send_status == 0) {
    retry = 0;
    current_inflight = true;
    current_send_time = timer_read32();
} else if (retry == 0) {
    retry = RETPORT_RETRY_COUNT;
    retry_time_buffer = timer_read32();
} else {
    --retry;
    retry_time_buffer = timer_read32();
    if (retry == 0) {
        memset(&current_report, 0, sizeof(current_report));
        current_report_valid = false;
        current_inflight = false;
    }
}
```

当前阶段保留“非 0 返回走重试”的现有策略，不在 middleware 直接引入 BLE SDK 状态判断。

- [ ] **Step 5: 保持原有节拍门控，但确保 retry 和 inflight 都能继续轮询**

保留：

```c
if (!report_buffer_next_inverval() && !retry && !current_inflight) {
    return;
}
```

目标是避免无意义空转，同时保证窗口释放和 retry 都不会丢轮询机会。

- [ ] **Step 6: 重新运行 `report_buffer_retry_test.exe`，确认绿灯**

Run:

```bash
gcc -o test/report_buffer_retry_test.exe test/report_buffer_retry_test.c middleware/communication/report_buffer.c -Itest/stubs/report_buffer -Itest/stubs -Imiddleware/communication -Imiddleware/keyboard -Icomponent/touch_component/include -Idrivers/system -Ikeyboards -Ihal -Iutils
./test/report_buffer_retry_test.exe
```

Expected:

- 输出 `PASS`
- 新增的长度保持和 inflight 阻塞测试全部通过

- [ ] **Step 7: Commit**

```bash
git add middleware/communication/report_buffer.c test/report_buffer_retry_test.c
git commit -m "feat(report-buffer): wait for BLE window release before sending next report"
```

---

### Task 5: 补齐 `REPORT_SEND_EVT` 的续调度闭环

**Files:**
- Modify: `application/service/communication_service.c`

- [ ] **Step 1: 让 `REPORT_SEND_EVT` 依据 `report_buffer_has_pending_work()` 续调度**

修改 [communication_service.c](D:/Code/VScode/keyboard-framework/application/service/communication_service.c) 中 `REPORT_SEND_EVT` 处理块：

```c
if (events & REPORT_SEND_EVT) {
    report_buffer_task();

    if (report_buffer_has_pending_work()) {
        OSAL_ClearEvent(commu_taskID, REPORT_SEND_EVT);
        OSAL_SetDelayedEvent(commu_taskID, REPORT_SEND_EVT, 16 * 15);
    }
    return (events ^ REPORT_SEND_EVT);
}
```

这里继续沿用现有 `16 * 15` 延迟值，本任务只修续调度条件，不额外引入新的调度常量。

- [ ] **Step 2: 检查断链后是否会残留待处理状态**

重点回读：

- `wireless_enter_disconnected()` 中的 `report_buffer_init()`
- `wireless_low_battery_shutdown()` 中的 `report_buffer_init()`

确认不需要额外新增清理逻辑；若发现 `current_report` 未被 `report_buffer_init()` 一并清理，回到 Task 2 修正。

- [ ] **Step 3: Commit**

```bash
git add application/service/communication_service.c
git commit -m "fix(communication): keep REPORT_SEND_EVT alive while reports remain pending"
```

---

### Task 6: 降低 busy 日志噪音，保留真正异常

**Files:**
- Modify: `drivers/communication/bluetooth/ch584/_bt_driver.c`

- [ ] **Step 1: 增加 busy 状态判断辅助函数**

在 [\_bt_driver.c](D:/Code/VScode/keyboard-framework/drivers/communication/bluetooth/ch584/_bt_driver.c) 内新增：

```c
static bool bt_driver_is_busy_status(uint8_t ret) {
    return ret == blePending || ret == bleNoResources || ret == bleTimeout;
}
```

- [ ] **Step 2: 键盘、鼠标、PTP 的失败日志改成“busy 降级、异常保留 error”**

例如：

```c
if (ret != 0) {
    if (bt_driver_is_busy_status(ret)) {
        dprintf("[BT] send_ptp busy, ret=%d,len=%d\r\n", ret, len);
    } else {
        LOG_E("[BT] send_ptp fail, ret=%d,len:%d", ret, len);
    }
}
```

同步处理：

- `bt_driver_send_keyboard()`
- `bt_driver_send_mouse()`
- `bt_driver_send_ptp()`
- `bt_driver_send_consumer()`
- `bt_driver_send_system()`

- [ ] **Step 3: Commit**

```bash
git add drivers/communication/bluetooth/ch584/_bt_driver.c
git commit -m "fix(bt-driver): downgrade flow-control busy logs for HID report sends"
```

---

### Task 7: 编译与人工验证

**Files:** 无新增修改

- [ ] **Step 1: 运行 report_buffer 单测**

Run:

```bash
gcc -o test/report_buffer_retry_test.exe test/report_buffer_retry_test.c middleware/communication/report_buffer.c -Itest/stubs/report_buffer -Itest/stubs -Imiddleware/communication -Imiddleware/keyboard -Icomponent/touch_component/include -Idrivers/system -Ikeyboards -Ihal -Iutils
./test/report_buffer_retry_test.exe
```

Expected:

- 输出 `PASS`

- [ ] **Step 2: 按仓库要求执行整仓编译验证**

Run:

```bash
/wch-riscv-build
```

Expected:

- 编译通过
- 无 `wt_func_t` 结构扩展引起的初始化缺项报错
- 无 `payload_len`、`report_buffer_has_pending_work()` 相关未声明符号

- [ ] **Step 3: 实机验证 BLE 连续滑动**

验证场景：

- 单指连续滑动 30 秒，观察是否仍持续刷 `[BT] send_ptp fail, ret=22`
- 滑动同时快速敲击键盘，确认键盘和触控板都仍能持续上报
- 长时间空闲后再触摸，确认首帧不会长时间堵住
- 断连重连后再次滑动，确认 `report_buffer_init()` 能正确清空旧状态

- [ ] **Step 4: 如验证通过，汇总行为变化**

记录至少三项观察：

- `ret=22` 是否从“每帧刷屏”下降到“偶发 busy”
- 滑动手感是否仍连续
- 键盘输入是否未被触控连续流量拖慢

---

## 实施顺序依赖

```text
Task 0 (单测补强)
   ↓
Task 1 (transport 增加窗口查询)
   ↓
Task 2 (report_buffer 数据模型扩展)
   ↓
Task 3 (wireless 入队长度与最新帧镜像)
   ↓
Task 4 (report_buffer inflight/window 行为)
   ↓
Task 5 (REPORT_SEND_EVT 续调度)
   ↓
Task 6 (busy 日志降噪)
   ↓
Task 7 (编译 + 实机验证)
```

## 风险提示

- `wt_func_t` 扩展后，若后续启用 2.4G 路径，需要检查对应函数表初始化是否需要补默认值。
- 本阶段“不删除旧帧”，极端高频输入下仍可能观察到队列增长；这次先验证窗口控制是否足以解决主问题。
- 最新帧镜像当前不参与覆盖逻辑，若后续实测仍不理想，再单独评审是否切到 latest-only。
