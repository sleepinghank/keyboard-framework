# CH584 蓝牙驱动实现设计（修订版）

> **设计日期:** 2026-03-10
> **修订日期:** 2026-03-10
> **基于需求:** 用户需求 - 实现 CH584 平台蓝牙驱动完整功能

---

## 1. 设计目标

基于 CH584 平台实现完整的蓝牙驱动功能，包括：
- HID 报告发送（标准键盘、鼠标、消费者键、系统控制）
- 多主机切换（支持 3 个蓝牙主机）
- 配对和连接管理
- 电池电量通知
- wireless_callbacks.c 迁移至合理层级

---

## 2. 架构概览

```
middleware/communication/wireless_callbacks.c
(BLE 状态同步 → 无线状态机)
    ↑ extern 调用
drivers/communication/bluetooth/ch584/hidkbd.c
(底层 BLE 实现 - GAP 状态回调)
    ↑
drivers/communication/bluetooth/ch584/_bt_driver.c
(驱动适配层 - 实现 bt_driver_xxx 接口)
    ↑
drivers/communication/bluetooth/bt_driver.h
(驱动接口定义 - 供框架调用)
```

**职责划分：**
- `_bt_driver.c`：适配层，连接框架和底层 BLE
- `hidkbd.c`：BLE 应用层，处理协议栈事件，调用 `access_ble_notify_xxx`
- `wireless_callbacks.c`：BLE 状态同步，驱动无线状态机（属于 middleware 层）

---

## 3. 状态通知机制（已存在，无需改动）

`hidkbd.c` 已经通过以下 extern 函数通知上层：

```c
// hidkbd.c 中已有的 extern 声明
extern void access_ble_notify_advertising(uint8_t pairing_state, uint8_t host_idx);
extern void access_ble_notify_connected(uint8_t host_idx);
extern void access_ble_notify_disconnected(uint8_t host_idx, uint8_t reason);
```

**调用点：**
- 连接成功：`hidEmuStateCB` → `GAPROLE_CONNECTED` → `access_ble_notify_connected()`
- 断开连接：`hidEmuStateCB` → `GAPROLE_WAITING/GAP_LINK_TERMINATED_EVENT` → `access_ble_notify_disconnected()`
- 开始广播：`hidEmu_adv_enable()` → `access_ble_notify_advertising()`

**结论：** 原设计中的「添加回调注册机制」任务冗余，不实现。

---

## 4. wireless_callbacks.c 迁移

### 4.1 原因

`application/service/wireless_callbacks.c` 实现的是 BLE 事件 → 无线状态机的同步，
本质上属于通信中间件层，不应放在 application 层。

### 4.2 操作

```
application/service/wireless_callbacks.c
    → middleware/communication/wireless_callbacks.c
```

同步更新 `CMakeLists.txt` 中的源文件路径。

---

## 5. HID 描述符扩展

### 5.1 问题

当前 `hidkbdservice.c` 的 HID 报告描述符只有标准键盘（8字节），无 Report ID：
- 无鼠标 Collection
- 无消费者键 Collection
- 无系统控制 Collection

添加多个 Collection 后，**必须引入 Report ID**（HID 标准要求）。

### 5.2 Report ID 分配

| Report ID | 类型 | 方向 | 长度 |
|-----------|------|------|------|
| 1 | 标准键盘 | Input | 8 字节（modifier + reserved + 6 keys）|
| 1 | LED 输出 | Output | 1 字节 |
| 2 | 鼠标 | Input | 5 字节（buttons + X + Y + wheel + reserved）|
| 3 | 消费者键 | Input | 2 字节（16-bit usage code）|
| 4 | 系统控制 | Input | 2 字节（16-bit usage code）|

### 5.3 修改文件

**hidkbdservice.h：**
```c
#define HID_NUM_REPORTS        9    // 更新报告数量

// 更新后的 Report ID（引入 Report ID 机制）
#define HID_RPT_ID_KEY_IN      1    // 键盘输入
#define HID_RPT_ID_LED_OUT     1    // LED 输出（同 Collection）
#define HID_RPT_ID_MOUSE_IN    2    // 鼠标输入
#define HID_RPT_ID_CONSUMER_IN 3    // 消费者键输入
#define HID_RPT_ID_SYSTEM_IN   4    // 系统控制输入
#define HID_RPT_ID_FEATURE     0    // Feature（保留）
```

**hidkbdservice.c：**
- 扩展 `hidReportMap[]`，添加鼠标、消费者键、系统控制 Collection
- 添加鼠标 Input Report 的 GATT 特性（Characteristic + CCCD + Report Reference）
- 添加消费者键 Input Report 的 GATT 特性
- 添加系统控制 Input Report 的 GATT 特性
- 更新 `hidRptMap[]` 映射表

---

## 6. HID 报告发送修复

### 6.1 问题

当前 `_bt_driver.c` 中所有发送函数都使用了错误的 Report ID（`HID_RPT_ID_MOUSE_IN`），
且 buffer 有无意义的前缀字节（`buffer[0]` 未使用）。

### 6.2 修复方案

```c
// 标准键盘：8字节，Report ID = 1
uint8_t bt_driver_send_keyboard(uint8_t *report) {
    return HidDev_Report(HID_RPT_ID_KEY_IN, HID_REPORT_TYPE_INPUT,
                         HID_KEYBOARD_IN_RPT_LEN, report);
}

// NKRO：stub，转发给标准键盘（明确标记待优化）
uint8_t bt_driver_send_nkro(uint8_t *report) {
    // BLE 不支持标准 NKRO，转发给标准键盘报告
    return bt_driver_send_keyboard(report);
}

// 鼠标：5字节，Report ID = 2
uint8_t bt_driver_send_mouse(uint8_t *report) {
    return HidDev_Report(HID_RPT_ID_MOUSE_IN, HID_REPORT_TYPE_INPUT,
                         HID_MOUSE_IN_RPT_LEN, report);
}

// 消费者键：2字节，Report ID = 3
uint8_t bt_driver_send_consumer(uint16_t report) {
    uint8_t buffer[2];
    buffer[0] = LO_UINT16(report);
    buffer[1] = HI_UINT16(report);
    return HidDev_Report(HID_RPT_ID_CONSUMER_IN, HID_REPORT_TYPE_INPUT,
                         sizeof(buffer), buffer);
}

// 系统控制：2字节，Report ID = 4
uint8_t bt_driver_send_system(uint16_t report) {
    uint8_t buffer[2];
    buffer[0] = LO_UINT16(report);
    buffer[1] = HI_UINT16(report);
    return HidDev_Report(HID_RPT_ID_SYSTEM_IN, HID_REPORT_TYPE_INPUT,
                         sizeof(buffer), buffer);
}

// PTP：stub，当前阶段不实现
uint8_t bt_driver_send_ptp(uint8_t *report, uint8_t len) {
    (void)report;
    (void)len;
    return 0;
}
```

**新增常量：**
```c
// 在 _bt_driver.c 或 hidkbdservice.h 中定义
#define HID_KEYBOARD_IN_RPT_LEN    8
#define HID_MOUSE_IN_RPT_LEN       5
```

---

## 7. 配对管理修复

### 7.1 问题

`bt_driver_pairing_ex()` 在 `GAPROLE_CONNECTED` 分支中调用 `hidEmu_disconnect()` 之前，
未更新 `access_state.ble_idx`，导致断开后自动发起广播时使用旧的主机索引。

### 7.2 修复方案

```c
void bt_driver_pairing_ex(uint8_t host_idx, void *param) {
    uint8_t ble_state;

    // 验证主机索引
    if ((host_idx <= BLE_INDEX_IDEL) || (host_idx >= BLE_INDEX_MAX)) {
        host_idx = BLE_INDEX_1;
    }

    // 先更新目标主机和配对状态
    access_state.ble_idx = host_idx;
    access_state.pairing_state = TRUE;

    GAPRole_GetParameter(GAPROLE_STATE, &ble_state);

    if (ble_state == GAPROLE_CONNECTED) {
        // 断开后，hidEmuStateCB 会根据 access_state 发起新广播
        hidEmu_disconnect();
        return;
    }

    if (ble_state == GAPROLE_ADVERTISING) {
        hidEmu_adv_enable(DISABLE);
    } else {
        hidEmu_adv_enable(ENABLE);
    }
}
```

---

## 8. 多主机切换完善

### 8.1 当前问题

`bt_driver_connect_ex()` 在主机索引相同时的分支逻辑冗余，且未绑定时处理不清晰。

### 8.2 改进方案

```c
void bt_driver_connect_ex(uint8_t host_idx, uint16_t timeout) {
    uint8_t ble_state = 0;
    (void)timeout;

    // 验证并修正主机索引
    if ((host_idx <= BLE_INDEX_IDEL) || (host_idx >= BLE_INDEX_MAX)) {
        storage_config_t *cfg = storage_get_config_ptr();
        host_idx = (cfg && cfg->ble_idx > BLE_INDEX_IDEL && cfg->ble_idx < BLE_INDEX_MAX)
                   ? cfg->ble_idx : BLE_INDEX_1;
    }

    access_state.ble_idx = host_idx;
    access_state.pairing_state = FALSE;

    GAPRole_GetParameter(GAPROLE_STATE, &ble_state);

    if (con_work_mode == host_idx) {
        // 目标主机与当前一致
        if ((ble_state == GAPROLE_CONNECTED) ||
            OSAL_GetTaskTimer(hidEmuTaskId, SEND_DISCONNECT_EVT)) {
            return;  // 已连接或正在断开，不操作
        }
        if (ble_state != GAPROLE_ADVERTISING && hidEmu_is_ble_bonded(host_idx)) {
            hidEmu_adv_enable(ENABLE);
        }
    } else {
        // 切换到不同主机
        if (OSAL_GetTaskTimer(hidEmuTaskId, SEND_DISCONNECT_EVT)) {
            OSAL_StopTask(hidEmuTaskId, SEND_DISCONNECT_EVT);
        }
        if (ble_state == GAPROLE_CONNECTED) {
            hidEmu_disconnect();
        } else if (ble_state == GAPROLE_ADVERTISING) {
            hidEmu_adv_enable(DISABLE);
        } else if (hidEmu_is_ble_bonded(host_idx)) {
            hidEmu_adv_enable(ENABLE);
        } else {
            con_work_mode = host_idx;
        }
    }
}
```

---

## 9. 电池电量通知

### 9.1 问题

`bt_driver_update_bat_level()` 函数体被注释掉。

### 9.2 解决方案

```c
void bt_driver_update_bat_level(uint8_t bat_lvl) {
    Batt_SetParameter(BATT_PARAM_LEVEL, sizeof(uint8_t), &bat_lvl);
}
```

**说明：** `hidkbd.c` 已包含 `battservice.h`，Battery Service 已在 `HidEmu_Init()` 中通过
`Batt_AddService()` 初始化。`_bt_driver.c` 需要添加 `#include "battservice.h"`。

---

## 10. 文件变更清单（修订后）

| 文件 | 操作 | 内容 |
|------|------|------|
| `project/ch584m/Profile/hidkbdservice.c` | 修改 | 扩展 HID 报告描述符，添加鼠标/消费者/系统 Collection 及 GATT 属性 |
| `project/ch584m/Profile/include/hidkbdservice.h` | 修改 | 更新 Report ID 常量，添加 CONSUMER/SYSTEM ID 和报告长度宏 |
| `drivers/communication/bluetooth/ch584/_bt_driver.c` | 修改 | 修复所有报告发送函数，修复 pairing_ex，改善 connect_ex，实现电池电量 |
| `application/service/wireless_callbacks.c` | 移动 | → `middleware/communication/wireless_callbacks.c` |
| `CMakeLists.txt` | 修改 | 更新 wireless_callbacks.c 源文件路径 |

**不修改的文件：**
- `hidkbd.c`（状态通知已完整，无需改动）
- `_bt_driver.h`（无新增接口需要暴露）
- `bt_driver.h`（接口定义不变）

---

## 11. 实现顺序

按照逐个功能迭代，每步完成后可单独验证：

1. **迁移 wireless_callbacks.c** — 纯文件移动，功能不变，验证编译通过
2. **扩展 HID 描述符** — 修改 hidkbdservice.c/h，为后续报告发送打基础
3. **修复 HID 报告发送** — 修复 _bt_driver.c 中所有发送函数
4. **修复配对管理** — 修复 bt_driver_pairing_ex 的 host_idx 更新
5. **完善多主机切换** — 改善 bt_driver_connect_ex 逻辑
6. **电池电量通知** — 解注释 Batt_SetParameter 调用

---

## 12. 参考资料

- `docs/marker-output/沁恒低功耗蓝牙软件开发参考手册-rdqj/沁恒低功耗蓝牙软件开发参考手册-rdqj.md`
- `project/ch584m/Profile/include/hidkbdservice.h` - HID Service 定义
- `project/ch584m/Profile/include/battservice.h` - Battery Service 定义

---

*设计修订完成时间: 2026-03-10*
