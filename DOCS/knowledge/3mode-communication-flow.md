# CH585 3模键盘通讯流程分析文档

> 基于现有代码库的完整流程分析，用于新项目参考设计

## 目录

1. [系统架构概览](#1-系统架构概览)
2. [3模切换与管理流程](#2-3模切换与管理流程)
3. [BLE模块详细分析](#3-ble模块详细分析)
4. [2.4G RF模块分析](#4-24g-rf模块分析)
5. [USB模块分析](#5-usb模块分析)
6. [数据传输抽象层](#6-数据传输抽象层)
7. [存储与配置管理](#7-存储与配置管理)
8. [电源管理](#8-电源管理)
9. [代码重构建议](#9-代码重构建议)

---

## 1. 系统架构概览

### 1.1 整体架构图

```
┌─────────────────────────────────────────────────────────────────┐
│                        应用层 (APP/)                             │
├─────────────┬─────────────┬─────────────┬─────────────────────┤
│   main.c    │   trans.c   │  access.c   │     hidkbd.c         │
│  模式选择    │  传输抽象    │ BLE设备管理  │   BLE HID实现        │
└─────────────┴─────────────┴─────────────┴─────────────────────┘
                              │
┌─────────────────────────────┼───────────────────────────────────┐
│                     通讯层 (传输协议)                             │
├─────────────┬───────────────┼───────────────┬─────────────────┤
│   USB HID   │    2.4G RF    │   BLE GATT    │    trans.c       │
│   usb.c     │     RF.c      │ hidkbdservice │  (统一接口)       │
└─────────────┴───────────────┴───────────────┴─────────────────┘
                              │
┌─────────────────────────────┼───────────────────────────────────┐
│                     硬件抽象层 (HAL)                              │
├─────────────┬───────────────┼───────────────┬─────────────────┤
│  USB外设    │   RF收发器    │   BLE控制器   │   GPIO/ADC/PWM  │
└─────────────┴───────────────┴───────────────┴─────────────────┘
```

### 1.2 核心文件职责

| 文件 | 职责 | 关键函数 |
|------|------|----------|
| `main.c` | 系统初始化、模式选择 | `main()`, `process_main()` |
| `trans.c` | 传输抽象层，统一发送接口 | `trans_send_data()`, `trans_Init()` |
| `access.c` | BLE多设备管理，配对控制 | `access_ctl_process()`, `access_switch_ble_mode()` |
| `hidkbd.c` | BLE HID键盘实现 | `HidEmu_Init()`, `hidEmu_adv_enable()` |
| `hiddev.c` | BLE HID设备框架 | `HidDev_Report()`, `hidDevGapStateCB()` |
| `RF.c` | 2.4G RF协议实现 | `RF_Init()`, `rf_send_data()`, `rfRoleBoundProcess()` |
| `usb.c` | USB HID实现 | `USB_Init()`, `USB_class_keyboard_report()` |
| `keyboard.c` | 键盘硬件控制，FN键处理 | `keyboard_fn_press()`, `peripheral_*_cb()` |

### 1.3 模式定义

```c
// KEYBOARD/include/peripheral.h
#define MODE_USB            0   // USB有线模式
#define MODE_2_4G           1   // 2.4GHz RF无线模式
#define MODE_BT             2   // 蓝牙BLE模式
```

---

## 2. 3模切换与管理流程

### 2.1 系统启动流程

```
                    系统上电
                        │
                        ▼
            ┌───────────────────────┐
            │   硬件初始化           │
            │   - 时钟配置           │
            │   - GPIO初始化        │
            │   - NVS Flash读取     │
            └───────────────────────┘
                        │
                        ▼
            ┌───────────────────────┐
            │   模式选择             │
            │   KEY_BT(PA5) 检测    │
            │   KEY_2_4G(PA6) 检测  │
            └───────────────────────┘
                        │
          ┌─────────────┼─────────────┐
          ▼             ▼             ▼
    ┌──────────┐  ┌──────────┐  ┌──────────┐
    │ MODE_BT  │  │MODE_2_4G │  │ MODE_USB │
    └──────────┘  └──────────┘  └──────────┘
          │             │             │
          ▼             ▼             ▼
    BLE初始化      RF初始化      USB初始化
    (5通道)        (配对/回连)   (枚举)
```

**关键代码路径**: `main.c:60-188`

```c
int main(void)
{
    // ... 硬件初始化 ...

    nvs_flash_init();  // 读取存储的配置

    // 模式选择 - 通过硬件引脚
    if(!KEY_BT_ST) {
        work_mode = MODE_BT;
    } else if(!KEY_2_4G_ST) {
        work_mode = MODE_2_4G;
    } else {
        work_mode = MODE_USB;
    }

    // 注意：代码中强制覆盖为2.4G模式
    work_mode = MODE_2_4G;  // main.c:104

    // 根据模式初始化对应通讯模块
    if(work_mode == MODE_BT) {
        // BLE模式：初始化5个BLE通道
        // 详细流程见第3节
    } else {
        CH58x_BLEInit(BLE_SNV_ADDR);
        HAL_Init();
        if(work_mode == MODE_USB) {
            USB_Init();
        } else {
            RF_Init();
        }
    }

    trans_Init();  // 初始化传输抽象层
    OTA_Init();    // 初始化OTA升级
    process_main(); // 进入主循环
}
```

### 2.2 主循环架构

```c
// main.c:44-51
void process_main(void)
{
    while(1) {
        TMOS_SystemProcess();   // TMOS任务调度
        peripheral_process();    // 外设处理(键盘扫描等)
    }
}
```

**TMOS任务系统**:
- 事件驱动架构
- 任务通过`TMOS_ProcessEventRegister()`注册
- 通过`tmos_set_event()`、`tmos_start_task()`触发事件

### 2.3 模式生命周期

```
┌─────────────────────────────────────────────────────────────────┐
│                        模式生命周期                              │
├─────────────┬───────────────────────────────────────────────────┤
│   状态      │                    描述                           │
├─────────────┼───────────────────────────────────────────────────┤
│   INIT      │ 模式初始化，资源配置                              │
│   IDLE      │ 空闲状态，等待连接/用户输入                        │
│   CONNECTING│ 正在建立连接（BLE广播/RF配对/USB枚举）            │
│   CONNECTED │ 已连接，可传输数据                                │
│   SLEEP     │ 低功耗睡眠（仅BLE/RF模式）                        │
│   DEEP_SLEEP│ 深度睡眠，等待唤醒                                │
└─────────────┴───────────────────────────────────────────────────┘
```

---

## 3. BLE模块详细分析

### 3.1 BLE多设备管理架构

键盘支持5个独立的BLE设备配对，每个设备使用独立的MAC地址和绑定信息。

```
┌─────────────────────────────────────────────────────────────┐
│                    BLE多设备管理                             │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌────────┐│
│  │BLE_INDEX│ │BLE_INDEX│ │BLE_INDEX│ │BLE_INDEX│ │BLE_INDEX││
│  │    1    │ │    2    │ │    3    │ │    4    │ │    5   ││
│  └────┬────┘ └────┬────┘ └────┬────┘ └────┬────┘ └───┬────┘│
│       │           │           │           │          │      │
│  SNV: 0x100   SNV: 0x200  SNV: 0x300  SNV: 0x400  SNV: 0x000│
│                                                             │
│  con_work_mode: 当前活跃的BLE通道索引                        │
│  access_state.ble_idx: 状态机中的当前索引                    │
└─────────────────────────────────────────────────────────────┘
```

**BLE索引定义**: `KEYBOARD/include/nvs_flash.h:59-68`

```c
typedef enum {
    BLE_INDEX_IDEL = 0,   // 空闲/未选择
    BLE_INDEX_1,          // 通道1
    BLE_INDEX_2,          // 通道2
    BLE_INDEX_3,          // 通道3
    BLE_INDEX_4,          // 通道4
    BLE_INDEX_5,          // 通道5
    BLE_INDEX_MAX,        // 最大值
} access_ble_idx_t;
```

### 3.2 BLE初始化流程

**代码路径**: `main.c:108-166`

```c
if(work_mode == MODE_BT) {
    // 初始化5个BLE通道
    // 每个通道使用不同的SNV地址存储绑定信息

    // 通道1-4初始化 (预加载绑定信息)
    for(int i = 1; i <= 4; i++) {
        start_device_over = FALSE;
        CH58x_BLEInit(BLE_SNV_ADDR + i * 0x100);
        HAL_Init();
        GAPRole_PeripheralInit();
        HidDev_Init();
        HidEmu_Init();
        while(!start_device_over) {
            TMOS_SystemProcess();
        }
    }

    // 通道5初始化 (活跃通道)
    CH58x_BLEInit(BLE_SNV_ADDR);
    HAL_Init();
    GAPRole_PeripheralInit();
    RF_RoleInit();
    HidDev_Init();
    HidEmu_Init();
    access_init();  // 初始化访问控制
}
```

**SNV地址分配**:

| 通道 | SNV地址偏移 | 用途 |
|------|-------------|------|
| BLE_INDEX_1 | +0x100 | 设备1绑定信息 |
| BLE_INDEX_2 | +0x200 | 设备2绑定信息 |
| BLE_INDEX_3 | +0x300 | 设备3绑定信息 |
| BLE_INDEX_4 | +0x400 | 设备4绑定信息 |
| BLE_INDEX_5 | +0x000 | 设备5绑定信息(默认) |

### 3.3 BLE广播流程

#### 3.3.1 广播控制

**核心函数**: `hidkbd.c` 中的 `hidEmu_adv_enable()`

```c
// 启用/禁用广播
void hidEmu_adv_enable(uint8_t enable)
{
    uint8_t param = enable;
    uint8_t ble_state;

    GAPRole_GetParameter(GAPROLE_STATE, &ble_state);

    if(enable) {
        // 设置广播参数
        uint16_t advInt = HID_INITIAL_ADV_INT_MIN;
        GAP_SetParamValue(TGAP_DISC_ADV_INT_MIN, advInt);
        GAP_SetParamValue(TGAP_DISC_ADV_INT_MAX, HID_INITIAL_ADV_INT_MAX);

        // 启动广播
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &param);
        adv_enable_process_flag = TRUE;

        // 状态上报
        if(access_state.pairing_state) {
            access_tran_report(REPORT_CMD_STATE, STATE_PAIRING);
        }
    } else {
        // 停止广播
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &param);
        adv_enable_process_flag = FALSE;
    }
}
```

#### 3.3.2 广播状态机

```
                    ┌───────────────┐
                    │  GAPROLE_INIT │
                    └───────┬───────┘
                            │ hidEmu_adv_enable(ENABLE)
                            ▼
                    ┌───────────────┐
                    │ GAPROLE_      │
                    │ ADVERTISING   │◄───────┐
                    └───────┬───────┘        │
                            │                │
              ┌─────────────┼─────────────┐  │
              │             │             │  │
              ▼             ▼             │  │
      ┌───────────────┐ ┌───────────────┐│  │
      │ 主机发起连接  │ │ 广播超时(60s)││  │
      └───────┬───────┘ └───────┬───────┘│  │
              │                 │        │  │
              ▼                 │        │  │
      ┌───────────────┐         │        │  │
      │GAPROLE_       │         │        │  │
      │CONNECTED      │         │        │  │
      └───────┬───────┘         │        │  │
              │                 │        │  │
              │ 断开连接        │ 重新启动│  │
              │                 │ 广播   │  │
              ▼                 └────────┘  │
      ┌───────────────┐                     │
      │ 主机断开/     │                     │
      │ 本地断开      │─────────────────────┘
      └───────────────┘   hidEmu_adv_enable(ENABLE)
```

#### 3.3.3 广播参数

```c
// Profile/hiddev.c:36-46
#define HID_INITIAL_ADV_INT_MIN    48    // 30ms (单位: 625us)
#define HID_INITIAL_ADV_INT_MAX    80    // 50ms
#define HID_HIGH_ADV_INT_MIN       32    // 20ms (高优先级)
#define HID_HIGH_ADV_INT_MAX       48    // 30ms
#define HID_LOW_ADV_INT_MIN        160   // 100ms (低优先级)
#define HID_LOW_ADV_INT_MAX        160   // 100ms

#define HID_INITIAL_ADV_TIMEOUT    60    // 60秒
#define HID_HIGH_ADV_TIMEOUT       5     // 5秒
#define HID_LOW_ADV_TIMEOUT        0     // 无限
```

### 3.4 BLE连接流程

#### 3.4.1 连接建立

```
┌─────────────────────────────────────────────────────────────────┐
│                      BLE连接建立流程                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  主机                    从机(键盘)                             │
│    │                         │                                  │
│    │──── CONNECT_REQ ───────►│ GAP建立连接请求                  │
│    │                         │                                  │
│    │◄── CONNECT_RSP ─────────│                                  │
│    │                         │                                  │
│    │                         ▼                                  │
│    │                  gapEstLinkReqEvent                        │
│    │                  hidDevGapStateCB()                        │
│    │                         │                                  │
│    │                         ▼                                  │
│    │                  hidDevConnSecure = FALSE                  │
│    │                  停止广播                                  │
│    │                         │                                  │
│    │◄─ Security Request ─────┤ 请求加密配对                     │
│    │                         │                                  │
│    │── Pairing Request ─────►│                                  │
│    │                         │                                  │
│    │◄─ Pairing Response ─────┤                                  │
│    │                         │                                  │
│    │        (配对过程)        │                                  │
│    │                         │                                  │
│    │                         ▼                                  │
│    │                  hidDevPairStateCB()                       │
│    │                  GAPBOND_PAIRING_STATE_BONDED              │
│    │                         │                                  │
│    │                         ▼                                  │
│    │                  hidDevConnSecure = TRUE                   │
│    │                  access_tran_report(STATE_CONNECTED)       │
│    │                         │                                  │
│    │◄─ HID Report ───────────┤ 可以发送HID数据                  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

#### 3.4.2 连接状态回调

**代码路径**: `Profile/hiddev.c:797-847`

```c
static void hidDevGapStateCB(gapRole_States_t newState, gapRoleEvent_t *pEvent)
{
    if(newState == GAPROLE_CONNECTED) {
        // 获取连接句柄
        gapConnHandle = event->connectionHandle;

        // 连接尚未加密
        hidDevConnSecure = FALSE;

        // 停止广播
        param = FALSE;
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &param);

        // 检查连接参数，必要时请求更新
        if(event->connInterval > DEFAULT_DESIRED_MAX_CONN_INTERVAL) {
            tmos_start_task(hidDevTaskId, START_PARAM_UPDATE_EVT, ...);
        }
    }
    else if(hidDevGapState == GAPROLE_CONNECTED && newState != GAPROLE_CONNECTED) {
        // 断开连接处理
        hidDevDisconnected();
    }

    // 更新状态
    hidDevGapState = newState;
}
```

### 3.5 BLE回连流程

#### 3.5.1 回连判断

**核心函数**: `access.c:270-357`

```c
void access_switch_ble_mode(void)
{
    uint8_t ble_state;

    // 检查是否需要切换通道
    if(con_work_mode != access_state.ble_idx) {
        access_state.pairing_state = FALSE;
        GAPRole_GetParameter(GAPROLE_STATE, &ble_state);

        if(ble_state == GAPROLE_CONNECTED) {
            // 当前已连接，先断开
            hidEmu_disconnect();
        }
        else if(ble_state == GAPROLE_ADVERTISING) {
            // 当前在广播，先停止
            hidEmu_adv_enable(DISABLE);
        }
        else {
            // 检查目标通道是否已绑定
            if(hidEmu_is_ble_bonded(access_state.ble_idx)) {
                // 已绑定，启动回连广播
                access_tran_report(REPORT_CMD_STATE, STATE_RE_CONNECTING);
                hidEmu_adv_enable(ENABLE);
            }
            else {
                // 未绑定，无法回连
                access_tran_report(REPORT_CMD_STATE, STATE_RE_CONNECT_FAIL);
            }
        }
    }
}
```

#### 3.5.2 绑定状态检查

```c
// hidkbd.c
uint8_t hidEmu_is_ble_bonded(access_ble_idx_t ble_idx)
{
    // 检查对应通道的绑定标志
    switch(ble_idx) {
        case BLE_INDEX_1:
            return (nvs_flash_info.ble_bond_flag & BLE_BOND_FLAG_1);
        case BLE_INDEX_2:
            return (nvs_flash_info.ble_bond_flag & BLE_BOND_FLAG_2);
        // ... 其他通道 ...
    }
    return FALSE;
}
```

### 3.6 BLE断联流程

#### 3.6.1 断开连接

```c
// hidkbd.c
void hidEmu_disconnect(void)
{
    uint8_t ble_state;
    GAPRole_GetParameter(GAPROLE_STATE, &ble_state);

    if(ble_state == GAPROLE_CONNECTED) {
        // 发送终止连接请求
        GAPRole_TerminateLink(hidEmuConnHandle);

        // 设置断开事件，等待断开完成后再处理
        tmos_start_task(hidEmuTaskId, SEND_DISCONNECT_EVT, TERMINATE_EVT_TIMEOUT);
    }
}
```

#### 3.6.2 断开后的处理

```c
// Profile/hiddev.c:768-786
static void hidDevDisconnected(void)
{
    // 重置客户端特征配置描述符
    Batt_HandleConnStatusCB(gapConnHandle, LINKDB_STATUS_UPDATE_REMOVED);
    ScanParam_HandleConnStatusCB(gapConnHandle, LINKDB_STATUS_UPDATE_REMOVED);
    hidDevHandleConnStatusCB(gapConnHandle, LINKDB_STATUS_UPDATE_REMOVED);

    // 重置协议模式
    hidProtocolMode = HID_PROTOCOL_MODE_REPORT;
}

// keyboard.c:218-234
void peripheral_disconnected_cb(void)
{
    connect_state = CON_IDEL;

    // 更新睡眠计时
    if(!deep_sleep_flag) {
        peripheral_sleep_update();
    }

    // LED闪烁指示断开状态
    if(nvs_flash_info.led_onoff) {
        led_blink_range = LED_BLINK_R1;
        tmos_start_task(keyboard_taskID, KEYBOARD_LED_TIMEOUT_EVENT,
                        DISCONNECT_LED_BLINK_TIMEOUT);
        tmos_start_reload_task(keyboard_taskID, KEYBOARD_LED_BLINK_EVENT,
                               DISCONNECT_LED_BLINK_INTERVAL);
    }
}
```

### 3.7 BLE地址管理

#### 3.7.1 MAC地址生成策略

每个BLE通道使用不同的MAC地址，通过IRK（Identity Resolving Key）实现：

```c
// hidkbd.c - 地址生成相关
// 每个通道使用不同的SNV地址存储独立的绑定信息
// MAC地址基于IRK生成，确保每次广播使用可解析的随机地址

// 切换通道时更新SNV地址
ble.SNVAddr = (access_state.ble_idx - BLE_INDEX_1) * 0x100 + BLE_SNV_ADDR;
```

#### 3.7.2 绑定信息存储

```c
// KEYBOARD/include/nvs_flash.h:83-105
typedef struct __PACKED {
    // ... 其他配置 ...
    uint8_t   ble_bond_flag;    // BLE绑定标志位图
    uint8_t   ble_mac_flag;     // BLE MAC地址标志
    uint8_t   ble_irk_flag[5];  // 每个通道的IRK标志
    // ... 其他配置 ...
} nvs_flash_info_t;

// 绑定标志位定义
#define BLE_BOND_FLAG_1    (1<<0)
#define BLE_BOND_FLAG_2    (1<<1)
#define BLE_BOND_FLAG_3    (1<<2)
#define BLE_BOND_FLAG_4    (1<<3)
#define BLE_BOND_FLAG_5    (1<<4)
```

### 3.8 BLE配对模式

#### 3.8.1 进入配对模式

**触发方式**: 长按FN+数字键1-3

```c
// keyboard.c:389-428
void keyboard_fn_press_timeout(uint8_t key)
{
    switch(key) {
        case 0x1E:  // 数字键1
            if(work_mode == MODE_BT) {
                access_pairing_process(CTL_MODE_BLE_1);
            }
            break;
        // ... 其他数字键 ...
    }
}

// access.c:420-456
void access_pairing_process(uint8_t ctl_type)
{
    // 检查当前是否在对应通道
    if(access_state.ble_idx == target_idx) {
        access_pairing_mode();
    }
}

void access_pairing_mode(void)
{
    uint8_t ble_state;
    GAPRole_GetParameter(GAPROLE_STATE, &ble_state);

    if(ble_state == GAPROLE_CONNECTED) {
        // 已连接，先断开
        hidEmu_disconnect();
        access_state.pairing_state = TRUE;
        return;
    }

    // 清除旧绑定
    if(hidEmu_is_ble_bonded(access_state.ble_idx)) {
        access_state.pairing_state = TRUE;
    }

    // 重新开始广播
    access_tran_report(REPORT_CMD_STATE, STATE_PAIRING);
    hidEmu_adv_enable(ENABLE);
}
```

#### 3.8.2 删除所有配对信息

```c
// access.c:504-528
case CTL_DELETE_PAIR_INFO:
{
    // 清除所有5个通道的绑定信息
    for(int offset = 0; offset <= 0x400; offset += 0x100) {
        ble.SNVAddr = BLE_SNV_ADDR + offset;
        GAPBondMgr_SetParameter(GAPBOND_ERASE_ALLBONDS, 0, 0);
    }

    // 清除NVS中的绑定标志
    nvs_flash_info.ble_bond_flag = 0;
    nvs_flash_info.ble_mac_flag = 0;
    nvs_flash_store();

    // 断开当前连接，停止广播
    con_work_mode = BLE_INDEX_IDEL;
    hidEmu_disconnect();
    hidEmu_adv_enable(DISABLE);
    break;
}
```

### 3.9 BLE HID数据发送

#### 3.9.1 发送接口

```c
// hidkbd.c
uint8_t hidEmu_class_keyboard_report(uint8_t *pData, uint8_t len)
{
    return HidDev_Report(HID_RPT_ID_CLASS_KEY_IN,
                         HID_REPORT_TYPE_INPUT,
                         len, pData);
}

// Profile/hiddev.c:306-340
uint8_t HidDev_Report(uint8_t id, uint8_t type, uint8_t len, uint8_t *pData)
{
    // 检查连接状态
    if(hidDevGapState == GAPROLE_CONNECTED) {
        // 检查加密状态
        if(hidDevConnSecure) {
            return hidDevSendReport(id, type, len, pData);
        }
    }
    return bleNotReady;
}
```

#### 3.9.2 Report ID定义

```c
// Profile/include/hidkbdservice.h
#define HID_RPT_ID_SYS_CTL_IN       1    // 系统控制
#define HID_RPT_ID_CLASS_KEY_IN     2    // 标准键盘
#define HID_RPT_ID_MOUSE_IN         3    // 鼠标
#define HID_RPT_ID_CONSUMER_IN      8    // 多媒体键
#define HID_RPT_ID_FN_IN            0x11 // FN键
```

---

## 4. 2.4G RF模块分析

### 4.1 RF初始化流程

**代码路径**: `APP/RF.c:676-720`

```c
void RF_Init(void)
{
    // 注册TMOS任务
    rfTaskID = TMOS_ProcessEventRegister(RF_ProcessEvent);

    // 获取本机MAC地址
    GetMACAddress(self_mac);

    // 初始化DMA描述符
    rfDMADescInit();

    // 配置RF参数
    conf.TxPower = LL_TX_POWEER_4_DBM;
    conf.pTx = DMATxDscrTab;
    conf.pRx = DMARxDscrTab;
    conf.rfProcessCB = RF_ProcessCallBack;
    conf.processMask = RF_STATE_RX | RF_STATE_RBU | RF_STATE_TX_IDLE |
                       RF_STATE_RX_RETRY | RF_STATE_MAP_UPDATE;

    RFRole_FastInit(&conf);

    // 设置RF通信参数
    gParm.accessAddress = 0x71437247;
    gParm.crcInit = 0x555555;
    gParm.frequency = 16;  // 频道
    gParm.properties = PHY_MODE;
    gParm.rxMaxLen = RF_RX_BUF_SZE + PKT_ACK_LEN;

    RFRole_SetParam(&gParm);

    // 启动绑定流程
    tmos_set_event(rfTaskID, RF_START_BOUND_EVENT);
    RF_set_con_status(RF_CON_IDEL);
}
```

### 4.2 RF配对流程

```
┌─────────────────────────────────────────────────────────────────┐
│                      RF配对流程                                  │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  键盘                          Dongle                           │
│    │                             │                              │
│    │──── RFBound_StartDevice ───►│ 启动设备绑定                 │
│    │                             │                              │
│    │◄───── 配对请求 ────────────  │                              │
│    │                             │                              │
│    │──── 配对响应 ──────────────►│                              │
│    │                             │                              │
│    │        (配对握手)           │                              │
│    │                             │                              │
│    │◄───── 分配设备ID ───────────│                              │
│    │                             │                              │
│    ▼                             ▼                              │
│  存储设备ID和Dongle MAC         存储键盘MAC                     │
│  RF_CON_CONNECTED               RF_CON_CONNECTED               │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

**配对回调**: `APP/RF.c:278-358`

```c
void rfRoleBoundProcess(staBound_t *pSta)
{
    if(!pSta->status) {
        // 配对成功
        RF_set_con_status(RF_CON_CONNECTED);
        gDeviceId = pSta->devId;

        // 存储配对信息
        if(nvs_flash_info.rf_device_id != pSta->devId ||
           !tmos_memcmp(nvs_flash_info.peer_mac, pSta->PeerInfo, 6)) {
            nvs_flash_info.rf_device_id = pSta->devId;
            tmos_memcpy(nvs_flash_info.peer_mac, pSta->PeerInfo, 6);
            nvs_flash_store();
        }

        peripheral_connected_cb();
    }
    else {
        // 配对失败
        RF_set_con_status(RF_CON_PAIRING);

        if(pSta->status == FAILURE) {
            // 自动配对失败，重新开始
            RF_set_con_status(RF_CON_IDEL);
            rfDMADescInit();
            RFRole_FastInit(&conf);
            tmos_set_event(rfTaskID, RF_START_BOUND_EVENT);
        }
        else if(pSta->status == bleTimeout) {
            // 超时
            tmos_start_task(rfTaskID, RF_REPORT_DISCONNECT_EVENT,
                           RF_REPORT_DISCONNECT_DELAY);
        }
    }
}
```

### 4.3 RF连接状态

```c
// APP/include/RF.h
typedef enum {
    RF_CON_IDEL,       // 空闲
    RF_CON_PAIRING,    // 配对中
    RF_CON_CONNECTED,  // 已连接
} RF_con_status_t;
```

### 4.4 RF数据发送

```c
// APP/RF.c:208-229
uint8_t rf_send_data(uint8_t *pData, uint8_t len)
{
    if(!(pDMATxGet->Status & STA_DMA_ENABLE)) {
        rfPackage_t *p = (rfPackage_t *)pDMATxGet->BufferAddr;

        p->type = gDeviceId;
        p->length = PKT_ACK_LEN + len;
        tmos_memcpy((uint8_t *)(pDMATxGet->BufferAddr + PKT_HEAD_LEN),
                    pData, len);
        pDMATxGet->BufferSize = len + PKT_HEAD_LEN;
        pDMATxGet->Status = STA_DMA_ENABLE;
        pDMATxGet = (RF_DMADESCTypeDef *)pDMATxGet->NextDescAddr;

        return 0;
    }
    return 0xFF;  // 缓冲区满
}
```

### 4.5 RF睡眠管理

```c
// APP/RF.c:606-629
void RF_enter_idel_sleep(void)
{
    idel_sleep_flag = 1;
    tmos_set_event(rfTaskID, RF_SLEEP_EVENT);
}

void RF_enter_deep_sleep(void)
{
    deep_sleep_flag = 1;
    tmos_set_event(rfTaskID, RF_SLEEP_EVENT);
}

void RF_weakup(void)
{
    deep_sleep_flag = 0;
    idel_sleep_flag = 0;
    tmos_stop_task(rfTaskID, RF_SLEEP_EVENT);

    if(!RF_check_con_status(RF_CON_CONNECTED)) {
        // 未连接时重新开始配对
        RFRole_Shut();
        RFRole_FastInit(&conf);
        tmos_set_event(rfTaskID, RF_START_BOUND_EVENT);
    }
}
```

---

## 5. USB模块分析

### 5.1 USB初始化

**代码路径**: `APP/usb.c`

```c
void USB_Init(void)
{
    // 配置USB时钟
    USB32M_init();

    // 初始化USB设备
    USB_DeviceInit();
    USB_SleepStatus = 0;

    // 注册TMOS任务
    usb_taskID = TMOS_ProcessEventRegister(USB_ProcessEvent);

    // 使能USB中断
    PFIC_EnableIRQ(USB_IRQn);
}
```

### 5.2 USB枚举流程

```
┌─────────────────────────────────────────────────────────────────┐
│                      USB枚举流程                                 │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  主机                          设备(键盘)                        │
│    │                             │                              │
│    │──── RESET ─────────────────►│                              │
│    │                             │                              │
│    │──── GET_DEVICE_DESC ───────►│ 返回设备描述符                │
│    │                             │                              │
│    │──── SET_ADDRESS ───────────►│ 设置设备地址                  │
│    │                             │                              │
│    │──── GET_CONFIG_DESC ───────►│ 返回配置描述符                │
│    │                             │                              │
│    │──── SET_CONFIGURATION ─────►│ 激活配置                      │
│    │                             │                              │
│    │──── GET_HID_REPORT_DESC ───►│ 返回HID报告描述符             │
│    │                             │                              │
│    │                             ▼                              │
│    │                      usb_enum_success_flag = TRUE          │
│    │                      可以发送HID数据                        │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 5.3 USB HID报告接口

```c
// APP/usb.c
uint8_t USB_class_keyboard_report(uint8_t *pData, uint8_t len)
{
    if(usb_enum_success_flag) {
        // 检查主机睡眠状态
        if((USB_SleepStatus & HOST_SET_FEATURE) &&
           (USB_SleepStatus & HOST_SET_SUSPEND)) {
            if(USB_SleepStatus & HOST_WAKEUP_ENABLE) {
                USB_Wake_up();  // 唤醒主机
            }
        }
        // 发送HID报告
        return USB_HID_SendReport(pData, len);
    }
    return 0xFF;
}
```

---

## 6. 数据传输抽象层

### 6.1 trans.c 统一接口

`trans.c` 提供了统一的发送接口，根据当前模式路由到对应的通讯模块。

**核心函数**: `APP/trans.c:64-127`

```c
uint8_t trans_send_data(uint8_t *pData, uint8_t len)
{
    if(work_mode == MODE_USB) {
        if(usb_enum_success_flag) {
            // USB模式：处理主机睡眠唤醒
            if((USB_SleepStatus & HOST_SET_FEATURE) &&
               (USB_SleepStatus & HOST_SET_SUSPEND)) {
                if(USB_SleepStatus & HOST_WAKEUP_ENABLE) {
                    USB_SleepStatus &= ~HOST_WAKEUP_ENABLE;
                    USB_Wake_up();
                }
            }
            // 添加到发送缓冲区
            input_idx = (trans_buf_out_idx + trans_buf_data_num) % TRANS_SEND_BUF_NUM;
            trans_send_buffer[input_idx].len = len;
            tmos_memcpy(trans_send_buffer[input_idx].pData, pData, len);
            trans_buf_data_num++;

            tmos_set_event(tran_taskID, SBP_TRANS_USB_EVT);
        }
    }
    else if(work_mode == MODE_2_4G) {
        if(RF_check_con_status(RF_CON_CONNECTED)) {
            // 2.4G模式：直接发送
            return rf_send_data(pData, len);
        }
    }
    // 注意：BLE模式不经过trans_send_data，直接使用trans_ble_send_data

    return 0;
}

// BLE专用发送接口
uint8_t trans_ble_send_data(uint8_t *pData, uint8_t len)
{
    return hidEmu_receive(pData, len);
}
```

### 6.2 数据命令格式

```c
// APP/include/access.h
#define CMD_CLASS_KEYBOARD    0x??  // 标准键盘报告
#define CMD_ALL_KEYBOARD      0x??  // 全键键盘报告
#define CMD_CONSUMER          0x??  // 多媒体键
#define CMD_SYS_CTL           0x??  // 系统控制
#define CMD_MOUSE             0x??  // 鼠标
#define CMD_MANUFACTURER      0x??  // 厂商命令
```

### 6.3 发送流程图

```
┌─────────────────────────────────────────────────────────────────┐
│                      数据发送流程                                │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  键盘扫描 ──► FN处理 ──► 模式判断                               │
│                               │                                 │
│               ┌───────────────┼───────────────┐                 │
│               ▼               ▼               ▼                 │
│          MODE_USB       MODE_2_4G        MODE_BT                │
│               │               │               │                 │
│               ▼               ▼               ▼                 │
│         trans_send_data  trans_send_data  trans_ble_send_data   │
│               │               │               │                 │
│               ▼               ▼               ▼                 │
│          USB报告队列     rf_send_data    hidEmu_*_report        │
│               │               │               │                 │
│               ▼               ▼               ▼                 │
│           USB HID        RF DMA          BLE GATT Notify        │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 7. 存储与配置管理

### 7.1 NVS Flash布局

```
Flash地址空间:
┌─────────────────────────────────────────────────────────────────┐
│ 0x76000 - FLASH_ROM_MAX_SIZE                                    │
│ ├── NVS_FLASH_INFO_ADDRESS (配置信息)                           │
│ │   ├── check_val_A/B (校验)                                    │
│ │   ├── led_onoff, led_mode, led_light                          │
│ │   ├── ble_idx (当前BLE通道)                                   │
│ │   ├── ble_bond_flag (绑定标志)                                │
│ │   ├── ble_mac_flag, ble_irk_flag[5]                          │
│ │   ├── rf_device_id, peer_mac[6]                              │
│ │   └── usb_vid_pid[4], usb_prod_info                          │
│                                                                 │
│ 0x77800 - FLASH_ROM_MAX_SIZE                                    │
│ ├── BLE_SNV_ADDR + 0x000 (BLE通道5绑定信息)                     │
│ ├── BLE_SNV_ADDR + 0x100 (BLE通道1绑定信息)                     │
│ ├── BLE_SNV_ADDR + 0x200 (BLE通道2绑定信息)                     │
│ ├── BLE_SNV_ADDR + 0x300 (BLE通道3绑定信息)                     │
│ └── BLE_SNV_ADDR + 0x400 (BLE通道4绑定信息)                     │
└─────────────────────────────────────────────────────────────────┘
```

### 7.2 配置结构体

```c
// KEYBOARD/include/nvs_flash.h:83-105
typedef struct __PACKED {
    uint8_t   check_val_A;       // 校验值A = 0x5A
    uint8_t   check_val_B;       // 校验值B = 0xA5
    uint8_t   led_onoff;         // LED开关
    uint8_t   led_mode;          // LED模式(常亮/1Hz/2Hz)
    uint8_t   ble_idx;           // 当前BLE通道索引
    uint8_t   ble_bond_flag;     // BLE绑定标志位图
    uint8_t   ble_mac_flag;      // BLE MAC地址标志
    uint8_t   ble_irk_flag[5];   // 每通道IRK标志
    uint8_t   ble_name_len;      // BLE设备名长度
    uint8_t   ble_name_data[22]; // BLE设备名
    uint8_t   led_light;         // LED亮度(0-4)
    uint8_t   work_mode;         // 系统模式(Win/Mac)
    uint8_t   rf_device_id;      // RF设备ID
    uint8_t   peer_mac[6];       // RF对端MAC
    uint8_t   usb_vid_pid[4];    // USB VID/PID
    uint8_t   usb_prod_info_len; // USB产品信息长度
    uint8_t   usb_prod_info[31]; // USB产品信息
    uint8_t   capacitance;       // 晶振负载电容
    uint8_t   check_sum;         // 校验和
} nvs_flash_info_t;
```

---

## 8. 电源管理

### 8.1 睡眠状态定义

```c
// 全局变量
volatile uint8_t idel_sleep_flag = 0;   // 空闲睡眠标志
volatile uint8_t deep_sleep_flag = 0;   // 深度睡眠标志
```

### 8.2 睡眠流程

```
┌─────────────────────────────────────────────────────────────────┐
│                      电源管理状态机                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│                     ┌──────────────┐                            │
│                     │    活跃      │                            │
│                     │   (Active)   │                            │
│                     └──────┬───────┘                            │
│                            │ 无操作超时                          │
│                            ▼                                     │
│                     ┌──────────────┐                            │
│                     │   空闲睡眠   │                            │
│                     │(Idle Sleep)  │◄───────┐                   │
│                     └──────┬───────┘        │                   │
│                            │ 长时间无操作   │ 按键唤醒           │
│                            ▼                 │                   │
│                     ┌──────────────┐        │                   │
│                     │   深度睡眠   │────────┘                   │
│                     │(Deep Sleep)  │                            │
│                     └──────────────┘                            │
│                                                                 │
│  空闲睡眠: 保持BLE/RF连接，降低扫描频率                          │
│  深度睡眠: 断开连接，保持RAM，等待GPIO唤醒                       │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 8.3 BLE睡眠控制

```c
// access.c:551-566
void access_ble_enter_idel_sleep(void)
{
    tmos_set_event(access_taskId, ACCESS_IDEL_SLEEP_EVT);
}

void access_ble_enter_deep_sleep(void)
{
    tmos_set_event(access_taskId, ACCESS_SLEEP_EVT);
}

void access_bleweakup(void)
{
    access_state.sleep_en = FALSE;
    tmos_set_event(access_taskId, ACCESS_WAKE_UP_EVT);
}
```

### 8.4 RF睡眠控制

```c
// RF.c:369-402
void RF_LowPower(uint32_t time)
{
    // 配置RTC唤醒时间
    RTC_SetTignTime(time_set);
    gSleepFlag = TRUE;

    // 进入睡眠模式
    LowPower_Sleep(RB_PWR_RAM96K | RB_PWR_EXTEND | RB_XT_PRE_EN);

    // 唤醒后恢复HSE配置
    HSECFG_Current(HSE_RCur_100);
}
```

---

## 9. 代码重构建议

### 9.1 当前架构问题

1. **模式判断散落各处**: `work_mode`检查分布在多个文件中
2. **BLE/RF/USB接口不统一**: 发送接口各异，难以维护
3. **状态管理分散**: 连接状态在多个模块独立管理
4. **配置耦合严重**: `nvs_flash_info`直接暴露

### 9.2 建议的架构改进

```
┌─────────────────────────────────────────────────────────────────┐
│                    建议的通讯层架构                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │              通讯管理器 (CommManager)                    │   │
│  │  - 统一的模式切换接口                                    │   │
│  │  - 统一的数据发送接口                                    │   │
│  │  - 统一的连接状态管理                                    │   │
│  └─────────────────────────────────────────────────────────┘   │
│                            │                                   │
│         ┌──────────────────┼──────────────────┐                │
│         ▼                  ▼                  ▼                │
│  ┌────────────┐     ┌────────────┐     ┌────────────┐          │
│  │ USB Driver │     │ RF Driver  │     │BLE Driver  │          │
│  │ Interface  │     │ Interface  │     │ Interface  │          │
│  └────────────┘     └────────────┘     └────────────┘          │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 9.3 统一接口定义建议

```c
// 建议的统一接口
typedef struct {
    uint8_t (*init)(void);
    uint8_t (*connect)(void);
    uint8_t (*disconnect)(void);
    uint8_t (*send)(uint8_t *data, uint8_t len);
    uint8_t (*get_state)(void);
    void    (*enter_sleep)(uint8_t deep);
    void    (*wakeup)(void);
} comm_driver_t;

// 驱动注册
void comm_register_driver(uint8_t mode, comm_driver_t *driver);

// 统一调用
uint8_t comm_send(uint8_t *data, uint8_t len);
uint8_t comm_switch_mode(uint8_t new_mode);
```

---

## 附录A: 关键函数索引

| 函数 | 文件 | 行号 | 用途 |
|------|------|------|------|
| `main()` | main.c | 60 | 系统入口 |
| `process_main()` | main.c | 44 | 主循环 |
| `trans_send_data()` | trans.c | 64 | 统一发送接口 |
| `access_ctl_process()` | access.c | 465 | 模式控制处理 |
| `access_switch_ble_mode()` | access.c | 270 | BLE通道切换 |
| `hidEmu_adv_enable()` | hidkbd.c | - | 广播控制 |
| `hidEmu_disconnect()` | hidkbd.c | - | BLE断开 |
| `RF_Init()` | RF.c | 676 | RF初始化 |
| `rfRoleBoundProcess()` | RF.c | 278 | RF配对回调 |
| `USB_Init()` | usb.c | - | USB初始化 |

---

## 附录B: 状态码定义

```c
// 连接状态
#define STATE_PAIRING           0x31  // 配对中
#define STATE_CONNECTED         0x32  // 已连接
#define STATE_CON_TERMINATE     0x33  // 断开
#define STATE_SWITCH_OVER       0x34  // 切换完成
#define STATE_RE_CONNECTING     0x35  // 回连中
#define STATE_RE_CONNECT_FAIL   0x36  // 回连失败

// 控制命令
#define CTL_MODE_USB            0x11
#define CTL_MODE_2_4G           0x30
#define CTL_MODE_BLE_1          0x31
#define CTL_MODE_BLE_2          0x32
#define CTL_MODE_BLE_3          0x33
#define CTL_MODE_BLE_4          0x34
#define CTL_MODE_BLE_5          0x35
#define CTL_PAIRING             0x51
#define CTL_DELETE_PAIR_INFO    0x52
```

---

*文档生成日期: 2026-03-08*
*基于CH585 3模键盘项目代码分析*
