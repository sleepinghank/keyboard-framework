# CH584 蓝牙驱动完整实现计划

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完善 CH584 蓝牙驱动，修复 HID 报告发送逻辑、迁移 wireless_callbacks.c 到合理层级、修复配对和多主机切换逻辑、实现电池电量通知。

**Architecture:** 在现有适配层 `_bt_driver.c` 中修复报告发送逻辑；将 `wireless_callbacks.c` 从 `application/service/` 迁移至 `middleware/communication/`；Profile 目录（hidkbdservice.c/h）**不做任何修改**（由芯片工具生成）。

**Tech Stack:** C 语言、CH584 BLE 协议栈（TMOS）、GATT HID Profile

**约束：**
- `project/ch584m/Profile/` 目录下所有文件不修改（后续由芯片工具生成）
- 涉及 Profile 支持的功能（consumer/system 报告类型）添加 TODO 注释，保留正确结构

---

## Context

### 设计文档
- `DOCS/plans/2026-03-10-ch584-bluetooth-driver-design.md`

### 关键文件
| 文件 | 角色 |
|------|------|
| `drivers/communication/bluetooth/bt_driver.h` | 驱动接口定义（只读）|
| `drivers/communication/bluetooth/ch584/_bt_driver.c` | 驱动适配层（主要修改目标）|
| `drivers/communication/bluetooth/ch584/hidkbd.c` | BLE 应用层（不修改）|
| `application/service/wireless_callbacks.c` | BLE 状态同步（迁移源）|
| `CMakeLists.txt` | 构建文件（更新路径）|

### 不修改的文件
- `project/ch584m/Profile/**`（Profile 由芯片工具生成）
- `drivers/communication/bluetooth/ch584/hidkbd.c`
- `drivers/communication/bluetooth/ch584/_bt_driver.h`
- `drivers/communication/bluetooth/bt_driver.h`

### 当前 Profile 中的 Report ID（只读参考）
```c
// hidkbdservice.h（不修改）
#define HID_RPT_ID_KEY_IN      0    // 键盘输入
#define HID_RPT_ID_MOUSE_IN    1    // 鼠标输入
// Consumer/System Report ID 未定义（待 Profile 工具生成后支持）
```

---

## Chunk 1: 迁移 wireless_callbacks.c

### Task 1: 将 wireless_callbacks.c 迁移至 middleware/communication/

**Files:**
- 移动: `application/service/wireless_callbacks.c` → `middleware/communication/wireless_callbacks.c`
- 修改: `CMakeLists.txt`

**背景：** `wireless_callbacks.c` 实现 BLE 事件 → 无线状态机同步，属于通信中间件层。
文件中 `#include "../../drivers/communication/bluetooth/ch584/hidkbd.h"` 的相对路径
从 `middleware/communication/` 与从 `application/service/` 到目标文件的层级相同，**无需修改**。

- [ ] **Step 1: 复制文件到新位置**

在项目根目录执行：
```bash
cp application/service/wireless_callbacks.c middleware/communication/wireless_callbacks.c
```

- [ ] **Step 2: 验证文件内容**

读取 `middleware/communication/wireless_callbacks.c`，确认包含以下函数：
- `access_ble_notify_advertising()`
- `access_ble_notify_connected()`
- `access_ble_notify_disconnected()`
- `access_ble_enter_idel_sleep()`

- [ ] **Step 3: 更新 CMakeLists.txt 源文件路径**

将：
```cmake
./application/service/wireless_callbacks.c
```
替换为：
```cmake
./middleware/communication/wireless_callbacks.c
```

- [ ] **Step 4: 删除原文件**

```bash
rm application/service/wireless_callbacks.c
```

---

## Chunk 2: 修复 _bt_driver.c 报告发送

### Task 2: 修复所有 HID 报告发送函数

**Files:**
- 修改: `drivers/communication/bluetooth/ch584/_bt_driver.c`

**背景：**
- 当前所有发送函数都错误地使用 `HID_RPT_ID_MOUSE_IN`
- buffer 有无意义的前缀字节（`buffer[0]` 留空，数据从 `buffer[1]` 开始，多发了一个字节）
- `HID_RPT_ID_KEY_IN=0`，`HID_RPT_ID_MOUSE_IN=1` 已在 `hidkbdservice.h` 中定义
- Consumer/System/PTP 报告类型需等 Profile 工具生成后支持，本次保留正确结构并添加 TODO

- [ ] **Step 1: 在文件顶部添加 battservice.h 和本地长度宏**

在 `_bt_driver.c` 现有 includes 之后添加：
```c
#include "battservice.h"

/* 报告数据长度：与 HID 描述符保持一致 */
#define BT_DRV_KEYBOARD_RPT_LEN    8    /* modifier(1) + reserved(1) + keycodes(6) */
#define BT_DRV_MOUSE_RPT_LEN       5    /* buttons(1) + X(1) + Y(1) + wheel(1) + pan(1) */

/* TODO: 以下 Report ID 待 Profile 工具生成 hidkbdservice.h 后替换为对应常量 */
#define BT_DRV_RPT_ID_CONSUMER_IN  3    /* 消费者键输入 */
#define BT_DRV_RPT_ID_SYSTEM_IN    4    /* 系统控制输入 */
```

- [ ] **Step 2: 修复 bt_driver_send_keyboard**

将：
```c
uint8_t bt_driver_send_keyboard(uint8_t *report)
{
    uint8_t buffer[21];
    // buffer[0] = CMD_CLASS_KEYBOARD;
    memcpy(&buffer[1], report, 20);
    return HidDev_Report(HID_RPT_ID_MOUSE_IN, HID_REPORT_TYPE_INPUT, 21, buffer);
}
```

替换为：
```c
uint8_t bt_driver_send_keyboard(uint8_t *report)
{
    return HidDev_Report(HID_RPT_ID_KEY_IN, HID_REPORT_TYPE_INPUT,
                         BT_DRV_KEYBOARD_RPT_LEN, report);
}
```

- [ ] **Step 3: 修复 bt_driver_send_nkro（转为 stub）**

将：
```c
uint8_t bt_driver_send_nkro(uint8_t *report)
{
    uint8_t buffer[21];
    // buffer[0] = CMD_ALL_KEYBOARD;
    memcpy(&buffer[1], report, 20);
    return HidDev_Report(HID_RPT_ID_MOUSE_IN, HID_REPORT_TYPE_INPUT, 21, buffer);
}
```

替换为：
```c
uint8_t bt_driver_send_nkro(uint8_t *report)
{
    /* BLE 不支持标准 NKRO，转发给标准键盘报告 */
    return bt_driver_send_keyboard(report);
}
```

- [ ] **Step 4: 修复 bt_driver_send_consumer**

将：
```c
uint8_t bt_driver_send_consumer(uint16_t report)
{
    uint8_t buffer[3];
    // buffer[0] = CMD_CONSUMER;
    buffer[1] = LO_UINT16(report);
    buffer[2] = HI_UINT16(report);
    return HidDev_Report(HID_RPT_ID_MOUSE_IN, HID_REPORT_TYPE_INPUT, 3, buffer);
}
```

替换为：
```c
uint8_t bt_driver_send_consumer(uint16_t report)
{
    uint8_t buffer[2];
    buffer[0] = LO_UINT16(report);
    buffer[1] = HI_UINT16(report);
    /* TODO: Profile 工具生成后，将 BT_DRV_RPT_ID_CONSUMER_IN 替换为 hidkbdservice.h 中的常量 */
    return HidDev_Report(BT_DRV_RPT_ID_CONSUMER_IN, HID_REPORT_TYPE_INPUT,
                         sizeof(buffer), buffer);
}
```

- [ ] **Step 5: 修复 bt_driver_send_system**

将：
```c
uint8_t bt_driver_send_system(uint16_t report)
{
    uint8_t buffer[3];
    // buffer[0] = CMD_SYS_CTL;
    buffer[1] = LO_UINT16(report);
    buffer[2] = HI_UINT16(report);
    return HidDev_Report(HID_RPT_ID_MOUSE_IN, HID_REPORT_TYPE_INPUT, 3, buffer);
}
```

替换为：
```c
uint8_t bt_driver_send_system(uint16_t report)
{
    uint8_t buffer[2];
    buffer[0] = LO_UINT16(report);
    buffer[1] = HI_UINT16(report);
    /* TODO: Profile 工具生成后，将 BT_DRV_RPT_ID_SYSTEM_IN 替换为 hidkbdservice.h 中的常量 */
    return HidDev_Report(BT_DRV_RPT_ID_SYSTEM_IN, HID_REPORT_TYPE_INPUT,
                         sizeof(buffer), buffer);
}
```

- [ ] **Step 6: 修复 bt_driver_send_mouse**

将：
```c
uint8_t bt_driver_send_mouse(uint8_t *report)
{
    uint8_t buffer[21];
    // buffer[0] = CMD_MOUSE;
    memcpy(&buffer[1], report, 20);
    return HidDev_Report(HID_RPT_ID_MOUSE_IN, HID_REPORT_TYPE_INPUT, 21, buffer);
}
```

替换为：
```c
uint8_t bt_driver_send_mouse(uint8_t *report)
{
    return HidDev_Report(HID_RPT_ID_MOUSE_IN, HID_REPORT_TYPE_INPUT,
                         BT_DRV_MOUSE_RPT_LEN, report);
}
```

- [ ] **Step 7: 实现 bt_driver_update_bat_level**

将：
```c
void bt_driver_update_bat_level(uint8_t bat_lvl)
{
    // Batt_SetParameter(BATT_PARAM_LEVEL, sizeof(uint8_t), &bat_lvl);
}
```

替换为：
```c
void bt_driver_update_bat_level(uint8_t bat_lvl)
{
    Batt_SetParameter(BATT_PARAM_LEVEL, sizeof(uint8_t), &bat_lvl);
}
```

- [ ] **Step 8: 在文件末尾添加 bt_driver_send_ptp stub**

在 `bt_driver_update_bat_level` 函数之后追加：

```c
/*********************************************************************
 * @fn      bt_driver_send_ptp
 *
 * @brief   发送 PTP 报告（当前阶段不实现）
 *
 * @param   report - PTP 报告数据
 * @param   len - 报告数据长度
 *
 * @return  uint8_t - 0（成功）
 *
 * @note    TODO: 待 Profile 工具生成 PTP 描述符后实现
 */
uint8_t bt_driver_send_ptp(uint8_t *report, uint8_t len)
{
    (void)report;
    (void)len;
    return 0;
}
```

---

## Chunk 3: 修复连接和配对逻辑

### Task 3: 修复 bt_driver_pairing_ex 的 host_idx 更新逻辑

**Files:**
- 修改: `drivers/communication/bluetooth/ch584/_bt_driver.c`

**背景：** 当 `ble_state == GAPROLE_CONNECTED` 时，原代码先调用 `hidEmu_disconnect()`，
但在此之前**没有更新 `access_state.ble_idx`**。断开后 `hidEmuStateCB` 在
`GAP_LINK_TERMINATED_EVENT` 中会用旧的 `access_state.ble_idx` 发起广播，
导致配对到错误主机。正确做法：调用 disconnect **之前**先更新目标状态。

- [ ] **Step 1: 替换 bt_driver_pairing_ex 函数体**

将现有完整函数：
```c
void bt_driver_pairing_ex(uint8_t host_idx, void *param)
{
    uint8_t ble_state;
    pairing_param_t default_pairing_param = {0, 0, 0, 0, 0};
    dprintf("bt_driver_pairing_ex: host_idx=%d\n", host_idx);
    if (param == NULL) {
        param = &default_pairing_param;
    }
    pairing_param_t* p = (pairing_param_t*)param;

    GAPRole_GetParameter( GAPROLE_STATE, &ble_state );


    if( ble_state == GAPROLE_CONNECTED )
    {


        hidEmu_disconnect();

        access_state.pairing_state = TRUE;

        // access_update_idel_sleep_timeout(ADV_IDEL_SLEEP_EVT_TIMEOUT);

        return;
    }


    if( hidEmu_is_ble_bonded( access_state.ble_idx ) )
    {


        access_state.pairing_state = TRUE;
    }


    if( ble_state == GAPROLE_ADVERTISING )
    {

        if( con_work_mode == access_state.ble_idx )
        {

        }

        hidEmu_adv_enable( DISABLE );
    }
    else
    {


        hidEmu_adv_enable( ENABLE );
    }

}
```

替换为：
```c
void bt_driver_pairing_ex(uint8_t host_idx, void *param)
{
    uint8_t ble_state;
    (void)param;

    dprintf("bt_driver_pairing_ex: host_idx=%d\n", host_idx);

    /* 验证并修正主机索引 */
    if ((host_idx <= BLE_INDEX_IDEL) || (host_idx >= BLE_INDEX_MAX)) {
        host_idx = BLE_INDEX_1;
    }

    /* 先更新目标主机和配对状态，确保断开后广播使用正确索引 */
    access_state.ble_idx = host_idx;
    access_state.pairing_state = TRUE;

    GAPRole_GetParameter(GAPROLE_STATE, &ble_state);

    if (ble_state == GAPROLE_CONNECTED) {
        /* 先断开，hidEmuStateCB 在 GAP_LINK_TERMINATED_EVENT 中会根据
         * access_state.ble_idx 和 access_state.pairing_state 发起配对广播 */
        hidEmu_disconnect();
        return;
    }

    if (ble_state == GAPROLE_ADVERTISING) {
        /* 停止当前广播，GAPROLE_WAITING 事件中会根据 access_state 重新发起 */
        hidEmu_adv_enable(DISABLE);
    } else {
        hidEmu_adv_enable(ENABLE);
    }
}
```

---

### Task 4: 完善 bt_driver_connect_ex 多主机切换逻辑

**Files:**
- 修改: `drivers/communication/bluetooth/ch584/_bt_driver.c`

**背景：** 当前函数在 `con_work_mode == target_host_idx` 分支中存在冗余逻辑，
且切换不同主机时未取消可能挂起的 `SEND_DISCONNECT_EVT` 定时器。

- [ ] **Step 1: 替换 bt_driver_connect_ex 函数体**

将现有完整函数：
```c
void bt_driver_connect_ex(uint8_t host_idx, uint16_t timeout)
{
    uint8_t ble_state = 0;
    uint8_t target_host_idx = host_idx;
    dprintf("bt_driver_connect_ex: host_idx=%d, timeout=%d\n", host_idx, timeout);
    if ((target_host_idx <= BLE_INDEX_IDEL) || (target_host_idx >= BLE_INDEX_MAX)) {
        storage_config_t *cfg = storage_get_config_ptr();
        if ((cfg != NULL) && (cfg->ble_idx > BLE_INDEX_IDEL) && (cfg->ble_idx < BLE_INDEX_MAX)) {
            target_host_idx = cfg->ble_idx;
        } else {
            target_host_idx = BLE_INDEX_1;
        }
    }

    access_state.ble_idx = target_host_idx;



    GAPRole_GetParameter( GAPROLE_STATE, &ble_state );

    if (con_work_mode != target_host_idx){

        access_state.pairing_state = FALSE;

        if( ble_state == GAPROLE_CONNECTED )
        {

            hidEmu_disconnect();
        }
        else if( ble_state == GAPROLE_ADVERTISING )
        {

            hidEmu_adv_enable( DISABLE );
        }
        else
        {

            if( hidEmu_is_ble_bonded( access_state.ble_idx ) )
            {

                hidEmu_adv_enable( ENABLE );
            }
            else
            {

                con_work_mode = access_state.ble_idx;
            }
        }


        if( OSAL_GetTaskTimer( hidEmuTaskId, SEND_DISCONNECT_EVT ) /* || (tmos_get_event(hidEmuTaskId)&SEND_DISCONNECT_EVT)*/ )
        {

            OSAL_StopTask(hidEmuTaskId, SEND_DISCONNECT_EVT);
        }
    } else {


        access_state.pairing_state = FALSE;

        if( (ble_state == GAPROLE_CONNECTED) || OSAL_GetTaskTimer( hidEmuTaskId, SEND_DISCONNECT_EVT ))
        {
        } else if (ble_state == GAPROLE_ADVERTISING){


        } else {

            if( hidEmu_is_ble_bonded( access_state.ble_idx ) )
            {

                hidEmu_adv_enable( ENABLE );
            }
            else
            {

            }

        }
    }

}
```

替换为：
```c
void bt_driver_connect_ex(uint8_t host_idx, uint16_t timeout)
{
    uint8_t ble_state = 0;
    uint8_t target_host_idx = host_idx;
    (void)timeout;

    dprintf("bt_driver_connect_ex: host_idx=%d\n", host_idx);

    /* 验证并修正主机索引 */
    if ((target_host_idx <= BLE_INDEX_IDEL) || (target_host_idx >= BLE_INDEX_MAX)) {
        storage_config_t *cfg = storage_get_config_ptr();
        if ((cfg != NULL) && (cfg->ble_idx > BLE_INDEX_IDEL) && (cfg->ble_idx < BLE_INDEX_MAX)) {
            target_host_idx = cfg->ble_idx;
        } else {
            target_host_idx = BLE_INDEX_1;
        }
    }

    access_state.ble_idx = target_host_idx;
    access_state.pairing_state = FALSE;

    GAPRole_GetParameter(GAPROLE_STATE, &ble_state);

    if (con_work_mode == target_host_idx) {
        /* 目标主机与当前一致：已连接或正在处理断开流程则不操作 */
        if ((ble_state == GAPROLE_CONNECTED) ||
            OSAL_GetTaskTimer(hidEmuTaskId, SEND_DISCONNECT_EVT)) {
            return;
        }
        /* 未连接且未在广播，已绑定则发起广播 */
        if ((ble_state != GAPROLE_ADVERTISING) && hidEmu_is_ble_bonded(target_host_idx)) {
            hidEmu_adv_enable(ENABLE);
        }
    } else {
        /* 切换到不同主机：先取消可能挂起的断开定时器 */
        if (OSAL_GetTaskTimer(hidEmuTaskId, SEND_DISCONNECT_EVT)) {
            OSAL_StopTask(hidEmuTaskId, SEND_DISCONNECT_EVT);
        }

        if (ble_state == GAPROLE_CONNECTED) {
            /* 已连接，断开后 hidEmuStateCB 根据 access_state 发起新广播 */
            hidEmu_disconnect();
        } else if (ble_state == GAPROLE_ADVERTISING) {
            /* 正在广播，停止后 GAPROLE_WAITING 事件处理切换 */
            hidEmu_adv_enable(DISABLE);
        } else if (hidEmu_is_ble_bonded(target_host_idx)) {
            hidEmu_adv_enable(ENABLE);
        } else {
            /* 未绑定，仅更新工作模式，等待配对命令 */
            con_work_mode = target_host_idx;
        }
    }
}
```

---

## Verification Checklist

- [ ] `middleware/communication/wireless_callbacks.c` 存在，内容与原文件一致
- [ ] `application/service/wireless_callbacks.c` 已删除
- [ ] `CMakeLists.txt` 路径已更新为 `./middleware/communication/wireless_callbacks.c`
- [ ] `_bt_driver.c` 顶部包含 `battservice.h`
- [ ] `_bt_driver.c` 顶部定义了 `BT_DRV_KEYBOARD_RPT_LEN=8`、`BT_DRV_MOUSE_RPT_LEN=5`
- [ ] `_bt_driver.c` 顶部定义了 `BT_DRV_RPT_ID_CONSUMER_IN=3`、`BT_DRV_RPT_ID_SYSTEM_IN=4`（带 TODO）
- [ ] `bt_driver_send_keyboard` 使用 `HID_RPT_ID_KEY_IN` 和 `BT_DRV_KEYBOARD_RPT_LEN`，无前缀字节
- [ ] `bt_driver_send_nkro` 转发给 `bt_driver_send_keyboard`
- [ ] `bt_driver_send_mouse` 使用 `HID_RPT_ID_MOUSE_IN` 和 `BT_DRV_MOUSE_RPT_LEN`，无前缀字节
- [ ] `bt_driver_send_consumer` 使用 `BT_DRV_RPT_ID_CONSUMER_IN`，2字节正确拼包，带 TODO
- [ ] `bt_driver_send_system` 使用 `BT_DRV_RPT_ID_SYSTEM_IN`，2字节正确拼包，带 TODO
- [ ] `bt_driver_update_bat_level` 调用 `Batt_SetParameter`（已解注释）
- [ ] `bt_driver_send_ptp` stub 已添加（返回 0，带 TODO）
- [ ] `bt_driver_pairing_ex` 在 `hidEmu_disconnect()` 之前更新 `access_state.ble_idx` 和 `access_state.pairing_state`
- [ ] `bt_driver_connect_ex` 切换不同主机时先取消 `SEND_DISCONNECT_EVT`，相同主机分支已简化

---

## Critical Files

| 文件 | 用途 |
|------|------|
| `drivers/communication/bluetooth/ch584/_bt_driver.c` | 驱动适配层（主要修改）|
| `middleware/communication/wireless_callbacks.c` | BLE 状态同步（迁移后）|
| `CMakeLists.txt` | 构建文件（路径更新）|

---

*计划创建时间: 2026-03-10*
