# 三模键盘通讯模块完整流程分析

> **文档版本**: 1.0
> **生成日期**: 2026-03-08
> **适用项目**: keyboard-framework

---

## 目录

1. [架构概览](#1-架构概览)
2. [三模切换与管理流程](#2-三模切换与管理流程)
3. [BLE 模块深度分析](#3-ble-模块深度分析)
4. [2.4G 模块流程](#4-24g-模块流程)
5. [USB 模块流程](#5-usb-模块流程)
6. [HID 报告缓冲系统](#6-hid-报告缓冲系统)
7. [关键代码路径汇总](#7-关键代码路径汇总)
8. [新项目设计建议](#8-新项目设计建议)

---

## 1. 架构概览

### 1.1 分层架构图

```
┌─────────────────────────────────────────────────────────────────────┐
│                        Application Layer                             │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │  communication_service.c  │  事件处理、状态回调              │    │
│  └─────────────────────────────────────────────────────────────┘    │
├─────────────────────────────────────────────────────────────────────┤
│                        Middleware Layer                              │
│  ┌──────────────┬──────────────┬──────────────┬──────────────┐      │
│  │ transport.c  │ wireless.c   │ report_buffer│ lpm.c        │      │
│  │ 模式切换     │ 状态机管理   │ HID报告队列  │ 低功耗管理   │      │
│  └──────────────┴──────────────┴──────────────┴──────────────┘      │
├─────────────────────────────────────────────────────────────────────┤
│                         Driver Layer                                 │
│  ┌────────────────────────────┬────────────────────────────┐        │
│  │    Bluetooth Driver        │      2.4G Driver           │        │
│  │  _bt_driver.c  │ hidkbd.c  │  p24g_driver.c             │        │
│  │  驱动接口      │ BLE实现   │  SPI通信实现               │        │
│  └────────────────────────────┴────────────────────────────┘        │
├─────────────────────────────────────────────────────────────────────┤
│                           HAL Layer                                  │
│  ┌──────────────────────────────────────────────────────────────┐   │
│  │  GPIO │ SPI │ UART │ Timer │ Power │ EEPROM                  │   │
│  └──────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────┘
```

### 1.2 核心文件清单

| 层级 | 文件路径 | 职责 |
|------|----------|------|
| 中间件 | `middleware/communication/transport.c` | 三模切换控制 |
| 中间件 | `middleware/communication/wireless.c` | 无线状态机 |
| 中间件 | `middleware/communication/report_buffer.c` | HID报告缓冲 |
| 驱动 | `drivers/communication/bluetooth/ch584/_bt_driver.c` | BLE驱动接口 |
| 驱动 | `drivers/communication/bluetooth/ch584/hidkbd.c` | BLE协议实现 |
| 驱动 | `drivers/communication/p2p4g/p24g_driver.c` | 2.4G驱动 |
| 存储 | `drivers/storage/storage.h` | 配置存储(含BLE绑定信息) |
| 服务 | `application/service/communication_service.c` | 通信事件服务 |

---

## 2. 三模切换与管理流程

### 2.1 传输模式定义

**文件**: `middleware/communication/transport.h`

```c
typedef enum {
    TRANSPORT_NONE,
    TRANSPORT_USB       = 0x01 << 0,  // USB模式
    TRANSPORT_BLUETOOTH = 0x01 << 1,  // 蓝牙模式
    TRANSPORT_P2P4      = 0x01 << 2,  // 2.4G模式
    TRANSPORT_MAX,
} transport_t;
```

### 2.2 模式切换核心函数

**文件**: `middleware/communication/transport.c:133-197`

```c
void set_transport(transport_t new_transport) {
    if (transport != new_transport) {
        // 1. 清除键盘状态
        if (transport == TRANSPORT_USB || wireless_get_state() == WT_CONNECTED)
            clear_keyboard();

        transport = new_transport;

        switch (transport) {
            case TRANSPORT_USB:
                usb_transport_enable(true);
                bt_transport_enable(false);
                p24g_transport_enable(false);
                wireless_disconnect();
                lpm_timer_stop();
                wireless_switch_to_usb_mode();  // 通知无线层
                break;

            case TRANSPORT_BLUETOOTH:
                p24g_transport_enable(false);
                bt_transport_enable(true);
                usb_transport_enable(false);
                lpm_timer_reset();
                wireless_switch_to_bt_driver();  // 切换驱动
                break;

            case TRANSPORT_P2P4:
                bt_transport_enable(false);
                p24g_transport_enable(true);
                usb_transport_enable(false);
                lpm_timer_reset();
                wireless_switch_to_p24g_driver();  // 切换驱动
                break;
        }
        transport_changed(transport);
    }
}
```

### 2.3 模式切换生命周期

```
┌────────────────────────────────────────────────────────────────────────┐
│                        模式切换生命周期                                 │
├────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  USB → BLE:                                                            │
│  ┌─────────┐    ┌──────────┐    ┌──────────┐    ┌──────────────┐      │
│  │ USB断开 │ -> │ 清空报告 │ -> │ 切换驱动 │ -> │ BLE连接请求  │      │
│  │ power() │    │clear_kb()│    │set_driver│    │ connect_ex() │      │
│  └─────────┘    └──────────┘    └──────────┘    └──────────────┘      │
│                                                                         │
│  BLE → USB:                                                            │
│  ┌─────────────┐  ┌───────────────┐  ┌───────────┐  ┌─────────────┐   │
│  │ BLE断开连接 │->│ 切换USB驱动   │->│ USB使能   │->│ LPM计时停止 │   │
│  │ disconnect()│  │ set_driver()  │  │ enable()  │  │ timer_stop()│   │
│  └─────────────┘  └───────────────┘  └───────────┘  └─────────────┘   │
│                                                                         │
│  BLE ↔ 2.4G:                                                           │
│  ┌─────────────┐  ┌───────────────┐  ┌────────────┐  ┌──────────────┐ │
│  │ 当前模式断开│->│ 切换无线驱动  │->│ 新模式使能 │->│ 发起连接请求 │ │
│  │ disconnect()│  │switch_driver()│  │ enable()   │  │ connect_ex() │ │
│  └─────────────┘  └───────────────┘  └────────────┘  └──────────────┘ │
└────────────────────────────────────────────────────────────────────────┘
```

### 2.4 无线状态机

**文件**: `middleware/communication/wireless.h:45-53`

```c
typedef enum {
    WT_RESET,         // 0: 复位状态
    WT_INITIALIZED,   // 1: 已初始化
    WT_DISCONNECTED,  // 2: 已断开
    WT_CONNECTED,     // 3: 已连接
    WT_PARING,        // 4: 配对中
    WT_RECONNECTING,  // 5: 重连中
    WT_SUSPEND        // 6: 挂起/低功耗
} wt_state_t;
```

### 2.5 状态机转换图

```
                    ┌─────────────────────────────────────────┐
                    │                                         │
                    ▼                                         │
┌──────────┐    ┌───────────────┐    ┌──────────────┐       │
│ WT_RESET │───>│ WT_INITIALIZED│───>│WT_DISCONNECTED│◄──────┤
└──────────┘    └───────────────┘    └──────────────┘       │
                     │                      │                │
                     │                      │                │
                     ▼                      ▼                │
                ┌─────────────┐      ┌──────────────┐       │
                │ WT_PARING   │      │WT_RECONNECTING│      │
                └─────────────┘      └──────────────┘       │
                     │                      │                │
                     │                      ▼                │
                     │              ┌──────────────┐        │
                     └─────────────>│ WT_CONNECTED │────────┘
                                    └──────────────┘
                                           │
                                           ▼
                                    ┌──────────────┐
                                    │ WT_SUSPEND   │
                                    └──────────────┘
```

### 2.6 无线传输接口

**文件**: `middleware/communication/wireless.h:57-68`

```c
typedef struct {
    void (*init)(bool);                          // 初始化
    void (*connect_ex)(uint8_t, uint16_t);       // 连接请求
    void (*pairing_ex)(uint8_t, void *);         // 配对请求
    void (*disconnect)(void);                    // 断开连接
    uint8_t (*send_keyboard)(uint8_t *);         // 发送键盘报告
    uint8_t (*send_nkro)(uint8_t *);             // 发送NKRO报告
    uint8_t (*send_consumer)(uint16_t);          // 发送消费键报告
    uint8_t (*send_system)(uint16_t);            // 发送系统键报告
    uint8_t (*send_mouse)(uint8_t *);            // 发送鼠标报告
    void (*update_bat_level)(uint8_t);           // 更新电量
} wt_func_t;
```

---

## 3. BLE 模块深度分析

### 3.1 BLE 驱动初始化流程

**文件**: `drivers/communication/bluetooth/ch584/_bt_driver.c:38-53`

```c
void bt_driver_init(bool wakeup_from_low_power) {
    if (g_bt_driver_initialized) {
        return;
    }

    // 1. BLE协议栈初始化
    CH58x_BLEInit();

    // 2. HAL层初始化
    HAL_Init();

    // 3. GAP外设角色初始化
    GAPRole_PeripheralInit();

    // 4. HID设备初始化
    HidDev_Init();

    // 5. HID模拟器初始化
    HidEmu_Init();

    g_bt_driver_initialized = true;
}
```

### 3.2 BLE 连接流程

**文件**: `drivers/communication/bluetooth/ch584/_bt_driver.c:65-145`

```c
void bt_driver_connect_ex(uint8_t host_idx, uint16_t timeout) {
    uint8_t ble_state = 0;
    uint8_t target_host_idx = host_idx;

    // 1. 确定目标主机索引
    if ((target_host_idx <= BLE_INDEX_IDEL) || (target_host_idx >= BLE_INDEX_MAX)) {
        storage_config_t *cfg = storage_get_config_ptr();
        if ((cfg != NULL) && (cfg->ble_idx > BLE_INDEX_IDEL)) {
            target_host_idx = cfg->ble_idx;
        } else {
            target_host_idx = BLE_INDEX_1;
        }
    }

    access_state.ble_idx = target_host_idx;
    GAPRole_GetParameter(GAPROLE_STATE, &ble_state);

    // 2. 根据当前状态处理
    if (con_work_mode != target_host_idx) {
        access_state.pairing_state = FALSE;

        if (ble_state == GAPROLE_CONNECTED) {
            // 已连接其他设备，先断开
            hidEmu_disconnect();
        } else if (ble_state == GAPROLE_ADVERTISING) {
            // 正在广播，先停止
            hidEmu_adv_enable(DISABLE);
        } else {
            // 检查是否已绑定
            if (hidEmu_is_ble_bonded(access_state.ble_idx)) {
                hidEmu_adv_enable(ENABLE);  // 开启回连广播
            } else {
                con_work_mode = access_state.ble_idx;
            }
        }
    }
}
```

### 3.3 BLE 广播管理

**文件**: `drivers/communication/bluetooth/ch584/hidkbd.c:949-1048`

```c
void hidEmu_adv_enable(uint8_t enable) {
    uint8_t ownAddr[6];

    if (initial_advertising_enable) {
        // 1. 配置SNV地址（每个通道独立的存储区）
        ble_config.SNVAddr = (access_state.ble_idx - BLE_INDEX_1) * 0x100 + BLE_SNV_ADDR;

        // 2. 配置解析列表(Resolving List)
        GAPBondMgr_SetParameter(GAPBOND_AUTO_SYNC_RL, sizeof(uint8_t), &RL_enable);

        // 3. 获取并修改蓝牙地址
        GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddr);
        ownAddr[4] += access_state.ble_idx;  // 根据通道号偏移

        // 4. 配对模式需要特殊MAC处理
        if (access_state.pairing_state) {
            if (!hidEmu_is_ble_mac_change(access_state.ble_idx)) {
                ownAddr[3] += access_state.ble_idx;  // 配对时更换MAC
            }
        } else {
            if (hidEmu_is_ble_mac_change(access_state.ble_idx)) {
                ownAddr[3] += access_state.ble_idx;  // 回连时使用原MAC
            }
        }
        GAP_ConfigDeviceAddr(ADDRTYPE_STATIC, ownAddr);
    }

    // 5. 根据绑定状态选择广播过滤策略
    uint8_t bonded = hidEmu_is_ble_bonded(access_state.ble_idx);
    if (initial_advertising_enable && bonded && (!access_state.pairing_state)) {
        // 已绑定且非配对模式：使用白名单过滤
        uint8_t filter_policy = GAP_FILTER_POLICY_WHITE;
        GAPRole_SetParameter(GAPROLE_ADV_FILTER_POLICY, sizeof(uint8_t), &filter_policy);
    } else {
        // 未绑定或配对模式：通用广播
        uint8_t filter_policy = GAP_FILTER_POLICY_ALL;
        GAPRole_SetParameter(GAPROLE_ADV_FILTER_POLICY, sizeof(uint8_t), &filter_policy);
    }

    // 6. 启动广播超时计时器（非配对模式）
    if (initial_advertising_enable && (!access_state.pairing_state)) {
        OSAL_SetDelayedEvent(hidEmuTaskId, ADV_TIMEOUT_SLEEP_EVT, ADV_TIMEOUT_SLEEP_EVT_TIMEOUT);
    }

    GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &initial_advertising_enable);
}
```

### 3.4 BLE 广播数据结构

**文件**: `drivers/communication/bluetooth/ch584/hidkbd.c:158-224`

```c
// 扫描响应数据
static uint8_t scanRspData[] = {
    0x0D,                           // 数据长度
    GAP_ADTYPE_LOCAL_NAME_COMPLETE, // AD类型：完整本地名称
    'H', 'I', 'D', ' ', 'K', 'e', 'y', 'b', 'T', 'E', 'S', 'T',

    // 连接间隔范围
    0x05, GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
    LO_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),  // 100ms
    HI_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
    LO_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),  // 1s
    HI_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),

    // 服务UUID
    0x05, GAP_ADTYPE_16BIT_MORE,
    LO_UINT16(HID_SERV_UUID), HI_UINT16(HID_SERV_UUID),
    LO_UINT16(BATT_SERV_UUID), HI_UINT16(BATT_SERV_UUID),

    // 发射功率
    0x02, GAP_ADTYPE_POWER_LEVEL, 0  // 0dBm
};

// 广播数据
static uint8_t advertData[31] = {
    // Flags
    0x02, GAP_ADTYPE_FLAGS,
    GAP_ADTYPE_FLAGS_LIMITED | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

    // 外观：HID键盘
    0x03, GAP_ADTYPE_APPEARANCE,
    LO_UINT16(GAP_APPEARE_HID_KEYBOARD),
    HI_UINT16(GAP_APPEARE_HID_KEYBOARD)
};
```

### 3.5 BLE 状态回调函数

**文件**: `drivers/communication/bluetooth/ch584/hidkbd.c:1227-1511`

```c
static void hidEmuStateCB(gapRole_States_t newState, gapRoleEvent_t *pEvent) {
    switch (newState & GAPROLE_STATE_ADV_MASK) {
        case GAPROLE_STARTED:
            // GAP角色已启动
            start_device_over = TRUE;
            LL_SetDataRelatedAddressChanges(1, 1);
            break;

        case GAPROLE_ADVERTISING:
            // 正在广播
            if (pEvent->gap.opcode == GAP_MAKE_DISCOVERABLE_DONE_EVENT) {
                con_work_mode = access_state.ble_idx;
            }
            break;

        case GAPROLE_CONNECTED:
            // 连接建立
            if (pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT) {
                hidEmuConnHandle = event->connectionHandle;
                access_state.deep_sleep_flag = FALSE;

                // 停止广播超时计时器
                OSAL_StopTask(hidEmuTaskId, ADV_TIMEOUT_SLEEP_EVT);

                // 启动安全请求
                OSAL_SetDelayedEvent(hidEmuTaskId, PERI_SECURITY_REQ_EVT, 4800);
            }
            break;

        case GAPROLE_WAITING:
            // 等待状态（广播结束或连接断开）
            if (pEvent->gap.opcode == GAP_LINK_TERMINATED_EVENT) {
                // 连接断开处理
                // 1. 检查是否需要重新广播
                // 2. 恢复未发送的数据包
                hidEmu_resend_BUF();
                hidEmu_save_ble_bonded(FALSE);
                hidEmu_adv_enable(ENABLE);
            }
            break;
    }
}
```

### 3.6 BLE 配对流程

**文件**: `drivers/communication/bluetooth/ch584/_bt_driver.c:157-209`

```c
void bt_driver_pairing_ex(uint8_t host_idx, void *param) {
    uint8_t ble_state;
    GAPRole_GetParameter(GAPROLE_STATE, &ble_state);

    // 1. 如果已连接，先断开
    if (ble_state == GAPROLE_CONNECTED) {
        hidEmu_disconnect();
        access_state.pairing_state = TRUE;
        return;
    }

    // 2. 检查是否已绑定
    if (hidEmu_is_ble_bonded(access_state.ble_idx)) {
        access_state.pairing_state = TRUE;
    }

    // 3. 控制广播
    if (ble_state == GAPROLE_ADVERTISING) {
        hidEmu_adv_enable(DISABLE);  // 停止当前广播
    } else {
        hidEmu_adv_enable(ENABLE);   // 开启配对广播
    }
}
```

### 3.7 BLE 蓝牙地址管理

#### 3.7.1 蓝牙设备索引

**文件**: `drivers/storage/storage.h:62-69`

```c
typedef enum {
    BLE_INDEX_IDEL = 0,  // 空闲
    BLE_INDEX_1,         // 蓝牙通道1
    BLE_INDEX_2,         // 蓝牙通道2
    BLE_INDEX_3,         // 蓝牙通道3
    BLE_INDEX_MAX,
} access_ble_idx_t;
```

#### 3.7.2 绑定标志管理

**文件**: `drivers/storage/storage.h:78-80`

```c
#define BLE_BOND_FLAG_1    (1<<0)  // 通道1绑定标志
#define BLE_BOND_FLAG_2    (1<<1)  // 通道2绑定标志
#define BLE_BOND_FLAG_3    (1<<2)  // 通道3绑定标志
```

#### 3.7.3 绑定状态检查

**文件**: `drivers/communication/bluetooth/ch584/hidkbd.c:911-940`

```c
uint8_t hidEmu_is_ble_bonded(access_ble_idx_t ble_idx) {
    storage_config_t *cfg = storage_get_config_ptr();
    uint8_t bond_flag = 0;

    switch (ble_idx) {
        case BLE_INDEX_1: bond_flag = BLE_BOND_FLAG_1; break;
        case BLE_INDEX_2: bond_flag = BLE_BOND_FLAG_2; break;
        case BLE_INDEX_3: bond_flag = BLE_BOND_FLAG_3; break;
        default: return 0;
    }

    return ((cfg->ble_bond_flag & bond_flag) != 0) ? TRUE : FALSE;
}
```

#### 3.7.4 MAC地址更换判断

**文件**: `drivers/communication/bluetooth/ch584/hidkbd.c:875-902`

```c
uint8_t hidEmu_is_ble_mac_change(access_ble_idx_t ble_idx) {
    storage_config_t *cfg = storage_get_config_ptr();
    uint8_t bond_flag = 0;

    switch (ble_idx) {
        case BLE_INDEX_1: bond_flag = BLE_BOND_FLAG_1; break;
        case BLE_INDEX_2: bond_flag = BLE_BOND_FLAG_2; break;
        case BLE_INDEX_3: bond_flag = BLE_BOND_FLAG_3; break;
        default: return 0;
    }

    // MAC更换标志用于区分配对和回连
    return ((cfg->ble_mac_flag & bond_flag) != 0) ? TRUE : FALSE;
}
```

#### 3.7.5 绑定信息存储

**文件**: `drivers/communication/bluetooth/ch584/hidkbd.c:831-866`

```c
void hidEmu_save_ble_bonded(uint8_t is_pairing) {
    storage_config_t *cfg = storage_get_config_ptr();
    uint8_t bond_flag = 0;

    switch (con_work_mode) {
        case BLE_INDEX_1: bond_flag = BLE_BOND_FLAG_1; break;
        case BLE_INDEX_2: bond_flag = BLE_BOND_FLAG_2; break;
        case BLE_INDEX_3: bond_flag = BLE_BOND_FLAG_3; break;
        default: return;
    }

    // 设置绑定标志
    cfg->ble_bond_flag |= bond_flag;

    // 配对模式下翻转MAC标志
    if (is_pairing) {
        cfg->ble_mac_flag ^= bond_flag;
    }

    storage_save();
}
```

### 3.8 BLE 断开连接流程

**文件**: `drivers/communication/bluetooth/ch584/hidkbd.c:773-784`

```c
void hidEmu_disconnect() {
    // 1. 发送终止连接命令
    GAPRole_TerminateLink(hidEmuConnHandle);

    // 2. 设置等待终止事件
    if (!OSAL_GetTaskTimer(hidEmuTaskId, WAIT_TERMINATE_EVT)) {
        OSAL_SetDelayedEvent(hidEmuTaskId, WAIT_TERMINATE_EVT, TERMINATE_EVT_TIMEOUT);
    }
}
```

### 3.9 BLE 回连流程

```
┌────────────────────────────────────────────────────────────────────────┐
│                           BLE回连流程                                   │
├────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  用户触发回连                                                           │
│       │                                                                 │
│       ▼                                                                 │
│  ┌──────────────────┐                                                   │
│  │ bt_driver_       │                                                   │
│  │ connect_ex()     │                                                   │
│  └────────┬─────────┘                                                   │
│           │                                                             │
│           ▼                                                             │
│  ┌──────────────────┐     否    ┌──────────────────┐                   │
│  │ 检查是否已绑定？ │──────────>│ 等待配对命令     │                   │
│  └────────┬─────────┘           └──────────────────┘                   │
│           │ 是                                                          │
│           ▼                                                             │
│  ┌──────────────────┐                                                   │
│  │ 配置SNV地址      │  根据通道索引计算                                │
│  │ (独立绑定存储)   │                                                   │
│  └────────┬─────────┘                                                   │
│           │                                                             │
│           ▼                                                             │
│  ┌──────────────────┐                                                   │
│  │ 配置MAC地址      │  使用原MAC(非配对模式)                           │
│  │ (基于通道偏移)   │                                                   │
│  └────────┬─────────┘                                                   │
│           │                                                             │
│           ▼                                                             │
│  ┌──────────────────┐                                                   │
│  │ 设置白名单过滤   │  只允许已绑定主机连接                            │
│  │ GAP_FILTER_      │                                                   │
│  │ POLICY_WHITE     │                                                   │
│  └────────┬─────────┘                                                   │
│           │                                                             │
│           ▼                                                             │
│  ┌──────────────────┐                                                   │
│  │ 开启定向广播     │  等待主机连接                                    │
│  └────────┬─────────┘                                                   │
│           │                                                             │
│           ▼                                                             │
│  ┌──────────────────┐                                                   │
│  │ GAPROLE_         │  通过hidEmuStateCB回调                           │
│  │ CONNECTED        │                                                   │
│  └────────┬─────────┘                                                   │
│           │                                                             │
│           ▼                                                             │
│  ┌──────────────────┐                                                   │
│  │ 启动安全请求     │  触发加密连接                                    │
│  │ PERI_SECURITY_   │                                                   │
│  │ REQ_EVT          │                                                   │
│  └──────────────────┘                                                   │
└────────────────────────────────────────────────────────────────────────┘
```

---

## 4. 2.4G 模块流程

### 4.1 2.4G 驱动状态

**文件**: `drivers/communication/p2p4g/p24g_driver.c:52-60`

```c
typedef enum {
    P24G_STATE_IDLE = 0,
    P24G_STATE_INITIALIZED,
    P24G_STATE_CONNECTING,
    P24G_STATE_CONNECTED,
    P24G_STATE_PAIRING,
    P24G_STATE_ERROR
} p24g_state_t;
```

### 4.2 2.4G 命令定义

**文件**: `drivers/communication/p2p4g/p24g_driver.c:39-50`

```c
#define P24G_CMD_NOP            0x00  // 空操作
#define P24G_CMD_READ_REG       0x01  // 读寄存器
#define P24G_CMD_WRITE_REG      0x02  // 写寄存器
#define P24G_CMD_SEND_KEYBOARD  0x10  // 发送键盘报告
#define P24G_CMD_SEND_NKRO      0x11  // 发送NKRO报告
#define P24G_CMD_SEND_CONSUMER  0x12  // 发送消费键
#define P24G_CMD_SEND_SYSTEM    0x13  // 发送系统键
#define P24G_CMD_SEND_MOUSE     0x14  // 发送鼠标报告
#define P24G_CMD_CONNECT        0x20  // 连接命令
#define P24G_CMD_DISCONNECT     0x21  // 断开命令
#define P24G_CMD_PAIRING        0x22  // 配对命令
#define P24G_CMD_UPDATE_BAT     0x30  // 更新电量
```

### 4.3 2.4G 初始化流程

**文件**: `drivers/communication/p2p4g/p24g_driver.c:83-111`

```c
void p24g_driver_init_impl(bool wakeup_from_low_power) {
    if (g_p24g_initialized && !wakeup_from_low_power) {
        return;
    }

    // 1. 初始化SPI接口
    spi_init();

    // 2. 配置控制引脚
    if (P24G_RST_PIN != NO_PIN) {
        gpio_set_pin_output_push_pull(P24G_RST_PIN);
        // 硬件复位模块
        gpio_write_pin_low(P24G_RST_PIN);
        wait_ms(10);
        gpio_write_pin_high(P24G_RST_PIN);
        wait_ms(50);
    }

    // 3. 配置中断引脚
    if (P24G_INT_PIN != NO_PIN) {
        gpio_set_pin_input_high(P24G_INT_PIN);
    }

    // 4. 发送初始化命令
    uint8_t init_data[] = {0x01};
    p24g_send_cmd(P24G_CMD_NOP, init_data, sizeof(init_data));

    g_p24g_state = P24G_STATE_INITIALIZED;
    g_p24g_initialized = true;
}
```

### 4.4 2.4G SPI 通信

**文件**: `drivers/communication/p2p4g/p24g_driver.c:66-80`

```c
static uint8_t p24g_spi_transfer(uint8_t data) {
    spi_write(data);
    return (uint8_t)spi_read();
}

static void p24g_send_cmd(uint8_t cmd, const uint8_t* data, uint8_t len) {
    if (P24G_CS_PIN == NO_PIN) return;

    spi_start(P24G_CS_PIN, false, 0, 8);
    p24g_spi_transfer(cmd);
    for (uint8_t i = 0; i < len; i++) {
        p24g_spi_transfer(data[i]);
    }
    spi_stop();
}
```

### 4.5 2.4G 连接流程

```c
void p24g_driver_connect_ex_impl(uint8_t host_idx, uint16_t timeout) {
    if (!g_p24g_initialized) return;

    // 2.4G点对点连接
    uint8_t connect_data[3] = {
        host_idx,
        (uint8_t)(timeout & 0xFF),
        (uint8_t)(timeout >> 8)
    };
    p24g_send_cmd(P24G_CMD_CONNECT, connect_data, sizeof(connect_data));
    g_p24g_state = P24G_STATE_CONNECTING;
}
```

---

## 5. USB 模块流程

### 5.1 USB 传输使能

**文件**: `middleware/communication/transport.c:109-131`

```c
void usb_transport_enable(bool enable) {
    if (enable) {
        if (host_get_driver() != &usb_driver) {
            // USB电源连接
            usb_power_connect();
            // 切换到USB驱动
            host_set_driver(&usb_driver);
        }
    } else {
        // USB电源断开
        usb_power_disconnect();
    }
}
```

### 5.2 USB 模式特点

- 无需配对流程
- 无低功耗管理（由主机供电）
- 直接通过 USB HID 协议发送报告
- 支持远程唤醒功能

---

## 6. HID 报告缓冲系统

### 6.1 报告缓冲结构

**文件**: `middleware/communication/report_buffer.h`

```c
typedef struct {
    uint8_t type;
    union {
        report_keyboard_t keyboard;
        report_nkro_t     nkro;
        uint16_t          consumer;
    };
} report_buffer_t;

enum {
    REPORT_TYPE_NONE,
    REPORT_TYPE_KB,       // 标准键盘报告
    REPORT_TYPE_NKRO,     // NKRO报告
    REPORT_TYPE_CONSUMER, // 消费键报告
};
```

### 6.2 报告缓冲队列

**文件**: `middleware/communication/report_buffer.c:30-50`

```c
#ifndef REPORT_BUFFER_QUEUE_SIZE
#    define REPORT_BUFFER_QUEUE_SIZE 256
#endif

report_buffer_t report_buffer_queue[REPORT_BUFFER_QUEUE_SIZE];
uint16_t        report_buffer_queue_head;
uint16_t        report_buffer_queue_tail;

uint8_t report_interval = DEFAULT_2P4G_REPORT_INVERVAL_MS;  // 默认1ms
```

### 6.3 报告缓冲任务

**文件**: `middleware/communication/report_buffer.c:109-143`

```c
void report_buffer_task(void) {
    if (wireless_get_state() == WT_CONNECTED &&
        (!report_buffer_is_empty() || retry) &&
        report_buffer_next_inverval()) {

        bool pending_data = false;

        if (!retry) {
            // 从队列取出报告
            if (report_buffer_dequeue(&kb_rpt) && kb_rpt.type != REPORT_TYPE_NONE) {
                pending_data = true;
                retry = RETPORT_RETRY_COUNT;  // 设置重试次数
                retry_time_buffer = timer_read32();
            }
        } else {
            // 重试逻辑
            if (timer_elapsed32(retry_time_buffer) > 2) {
                pending_data = true;
                --retry;
                retry_time_buffer = timer_read32();
            }
        }

        if (pending_data) {
            // 根据报告类型调用对应发送函数
            if (kb_rpt.type == REPORT_TYPE_KB) {
                wireless_transport.send_keyboard(&kb_rpt.keyboard.mods);
            } else if (kb_rpt.type == REPORT_TYPE_NKRO) {
                wireless_transport.send_nkro(&kb_rpt.nkro.mods);
            } else if (kb_rpt.type == REPORT_TYPE_CONSUMER) {
                wireless_transport.send_consumer(kb_rpt.consumer);
            }
            report_timer_buffer = timer_read32();
            lpm_timer_reset();
        }
    }
}
```

### 6.4 报告发送流程

```
按键事件 → wireless_send_keyboard()
               │
               ▼
        ┌──────────────┐
        │ 检查连接状态  │
        └──────┬───────┘
               │
               ▼
        ┌──────────────┐
        │ 入队报告缓冲  │
        │ enqueue()    │
        └──────┬───────┘
               │
               ▼
        ┌──────────────┐
        │ report_buffer│
        │ _task()      │
        └──────┬───────┘
               │
               ▼
        ┌──────────────┐
        │ 检查发送间隔  │
        └──────┬───────┘
               │
               ▼
        ┌──────────────┐
        │ 调用驱动发送  │
        │ send_keyboard│
        └──────┬───────┘
               │
               ▼
        ┌──────────────┐
        │ 重试/确认    │
        └──────────────┘
```

---

## 7. 关键代码路径汇总

### 7.1 模式切换路径

```
用户按键 → set_transport(TRANSPORT_BLUETOOTH)
    │
    ├─> bt_transport_enable(true)
    │       └─> host_set_driver(&wireless_driver)
    │       └─> wireless_disconnect()
    │       └─> wireless_connect_ex(30, 0)
    │
    └─> wireless_switch_to_bt_driver()
            └─> wireless_transport = bt_driver函数表
```

### 7.2 BLE 连接路径

```
wireless_connect_ex(host_idx, timeout)
    │
    └─> bt_driver_connect_ex()
            │
            ├─> GAPRole_GetParameter(GAPROLE_STATE, &ble_state)
            │
            ├─> [已连接] → hidEmu_disconnect()
            │
            ├─> [正在广播] → hidEmu_adv_enable(DISABLE)
            │
            └─> [其他] → hidEmu_is_ble_bonded()
                    │
                    ├─> [已绑定] → hidEmu_adv_enable(ENABLE)
                    │                   │
                    │                   └─> 设置白名单过滤广播
                    │
                    └─> [未绑定] → 等待配对命令
```

### 7.3 按键报告路径

```
key_press → wireless_send_keyboard()
    │
    ├─> [检查状态] WT_CONNECTED 或 (WT_PARING && pincodeEntry)
    │
    ├─> [入队] report_buffer_enqueue()
    │
    └─> [发送] report_buffer_task()
            │
            └─> wireless_transport.send_keyboard()
                    │
                    ├─> [BLE] → bt_driver_send_keyboard()
                    │               └─> HidDev_Report()
                    │
                    └─> [2.4G] → p24g_driver_send_keyboard()
                                    └─> SPI传输
```

---

## 8. 新项目设计建议

### 8.1 当前架构问题

1. **2.4G驱动实现不完整**：大量TODO占位符
2. **事件处理分散**：wireless.c和communication_service.c职责重叠
3. **缺少统一的错误处理**：各模块独立处理错误
4. **配置存储耦合**：BLE绑定信息与通用配置混在一起

### 8.2 简化建议

#### 8.2.1 统一传输层接口

```c
// 建议的统一传输接口
typedef struct {
    // 生命周期
    int (*init)(void);
    int (*deinit)(void);

    // 连接管理
    int (*connect)(uint8_t host_id);
    int (*disconnect)(void);
    int (*pair)(uint8_t host_id);

    // 数据传输
    int (*send)(uint8_t report_type, const void* data, uint8_t len);

    // 状态查询
    transport_state_t (*get_state)(void);
} transport_driver_t;
```

#### 8.2.2 状态机统一管理

```c
// 建议的状态机设计
typedef struct {
    transport_type_t current_transport;
    transport_state_t state;

    // 统一的状态转换表
    const state_transition_t* transitions;
    uint8_t num_transitions;

    // 事件回调
    void (*on_state_change)(transport_state_t old, transport_state_t new);
} transport_fsm_t;
```

#### 8.2.3 配置存储分离

```c
// 建议的存储分离设计
typedef struct {
    // 通用配置
    uint8_t fn_lock_state;
    uint8_t backlight_brightness;
    // ...
} device_config_t;

typedef struct {
    uint8_t bond_flag;
    uint8_t mac[6];
    uint8_t irk[16];
    uint8_t peer_addr[6];
} ble_bond_info_t;

typedef struct {
    device_config_t device;
    ble_bond_info_t ble_hosts[3];  // 每个通道独立存储
    uint8_t p24g_pairing_info[32]; // 2.4G配对信息
} storage_layout_t;
```

### 8.3 推荐的模块化设计

```
┌─────────────────────────────────────────────────────────────┐
│                  Transport Manager (新增)                    │
│  ┌─────────────────────────────────────────────────────────┐│
│  │ - 统一的状态机管理                                      ││
│  │ - 模式切换协调                                         ││
│  │ - 错误处理中心                                         ││
│  └─────────────────────────────────────────────────────────┘│
├─────────────────────────────────────────────────────────────┤
│                     Driver Abstraction                       │
│  ┌─────────────┬─────────────┬─────────────┐                │
│  │ USB Driver  │ BLE Driver  │ 2.4G Driver │                │
│  │ Interface   │ Interface   │ Interface   │                │
│  └─────────────┴─────────────┴─────────────┘                │
├─────────────────────────────────────────────────────────────┤
│                     Platform HAL                             │
│  ┌─────────────────────────────────────────────────────────┐│
│  │ GPIO │ SPI │ UART │ BLE Stack │ RF Module               ││
│  └─────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────┘
```

### 8.4 关键改进点

1. **添加Transport Manager层**
   - 统一管理三种传输模式
   - 集中处理状态转换
   - 提供统一的上层API

2. **完善2.4G驱动**
   - 实现完整的配对流程
   - 添加连接状态管理
   - 实现可靠的数据传输

3. **优化BLE模块**
   - 简化MAC地址管理逻辑
   - 统一绑定信息存储
   - 添加连接参数协商

4. **增强报告缓冲**
   - 添加优先级队列支持
   - 实现拥塞控制
   - 支持报告合并

---

## 附录A: 事件类型定义

**文件**: `application/service/communication_service.h`

```c
typedef enum {
    WL_INIT_EVT             = 1 << 0,   // 无线初始化
    WL_RESET_EVT            = 1 << 1,   // 无线重置
    WL_DISCOVERABLE_EVT     = 1 << 2,   // 可发现模式
    WL_RECONNECTING_EVT     = 1 << 3,   // 重连中
    WL_CONNECTED_EVT        = 1 << 4,   // 已连接
    WL_DISCONNECTED_EVT     = 1 << 5,   // 已断开
    WL_ADVEND_EVT           = 1 << 6,   // 广播结束
    WL_HID_SET_PROTOCOL_EVT = 1 << 7,   // HID协议设置
    WL_HID_INDICATOR_EVT    = 1 << 8,   // HID指示器
    WL_CONECTION_INTERVAL_EVT = 1 << 9, // 连接间隔更新
    WL_PAIR_EVT             = 1 << 10,  // 配对事件
    WL_RECONNECT_EVT        = 1 << 11,  // 回连事件
    USB_CONNECT_EVT         = 1 << 12,  // USB连接
    USB_DISCONNECT_EVT      = 1 << 13,  // USB断开
} commu_task_EVT_t;
```

---

## 附录B: 存储配置结构

**文件**: `drivers/storage/storage.h:92-125`

```c
typedef struct {
    // 系统配置
    uint8_t fn_lock_state;              // Fn锁状态
    uint8_t device_type;                // 设备类型
    uint8_t backlight_brightness;       // 背光亮度

    // BLE配置
    uint8_t ble_idx;                    // 当前BLE通道索引
    uint8_t ble_bond_flag;              // BLE绑定标志(位掩码)
    uint8_t ble_mac_flag;               // MAC更换标志
    uint8_t ble_irk_flag[5];            // IRK标志
    uint8_t ble_name_len;               // 设备名称长度
    uint8_t ble_name_data[22];          // 设备名称

    // 2.4G配置
    uint8_t work_mode;                  // 工作模式
    uint8_t rf_device_id;               // RF设备ID
    uint8_t peer_mac[6];                // 对端MAC地址

    // USB配置
    uint8_t usb_vid_pid[4];             // VID/PID
    uint8_t usb_prod_info_len;          // 产品信息长度
    uint8_t usb_prod_info[31];          // 产品信息

    // 用户配置
    uint16_t gesture_map[32];           // 手势映射
    uint8_t macro_data[32];             // 宏数据
    uint16_t shortcuts[16];             // 快捷键
} storage_config_t;
```

---

*文档结束*
