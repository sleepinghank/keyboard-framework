# CH584 蓝牙驱动实现设计

> **设计日期:** 2026-03-10
> **基于需求:** 用户需求 - 实现 CH584 平台蓝牙驱动完整功能

---

## 1. 设计目标

基于 CH584 平台实现完整的蓝牙驱动功能，包括：
- HID 报告发送（键盘、鼠标、消费者键、系统控制）
- 多主机切换（支持 3 个蓝牙主机）
- 配对和连接管理
- 电池电量通知
- 状态变化通知机制

---

## 2. 架构概览

```
┌─────────────────────────────────────────────────────────────┐
│  middleware/communication/wireless.c                        │
│  (框架无线模块 - 调用驱动接口)                               │
└─────────────────────┬───────────────────────────────────────┘
                      │ wt_func_t 函数表调用
                      ▼
┌─────────────────────────────────────────────────────────────┐
│  drivers/communication/bluetooth/bt_driver.h                │
│  (驱动接口定义)                                              │
└─────────────────────┬───────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────┐
│  drivers/communication/bluetooth/ch584/_bt_driver.c         │
│  (驱动适配层 - 实现接口 + 状态回调管理)                       │
│  - 实现 bt_driver_xxx() 函数                                │
│  - 注册/触发状态回调通知                                     │
└─────────────────────┬───────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────┐
│  drivers/communication/bluetooth/ch584/hidkbd.c             │
│  (底层 BLE 实现 - 来自官方 demo)                             │
│  - HidEmu_ProcessEvent() TMOS 事件处理                      │
│  - hidEmu_xxx() 功能函数                                    │
│  - GAP 状态机回调                                            │
└─────────────────────┬───────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────┐
│  project/ch584m/Profile/ & LIB/                             │
│  (官方 BLE 协议栈 + HID Profile)                             │
└─────────────────────────────────────────────────────────────┘
```

**职责划分：**
- `_bt_driver.c`：适配层，连接框架和底层 BLE
- `hidkbd.c`：BLE 应用层，处理协议栈事件
- `bt_driver.h`：接口定义，供框架调用

---

## 3. 状态通知机制

### 3.1 问题

`hidkbd.c` 中的 GAP 状态变化（连接/断开/配对）需要通知到框架层。

### 3.2 解决方案

添加状态回调注册机制

### 3.3 数据结构

```c
/* 在 _bt_driver.h 中添加 */

typedef enum {
    BT_STATE_DISCONNECTED,
    BT_STATE_CONNECTED,
    BT_STATE_PAIRING,
    BT_STATE_RECONNECTING
} bt_driver_state_t;

typedef void (*bt_state_callback_t)(bt_driver_state_t state, uint8_t host_idx);

typedef struct {
    bt_state_callback_t on_state_change;
    bt_state_callback_t on_pairing_complete;
    bt_state_callback_t on_disconnect;
} bt_driver_callbacks_t;

/* 注册回调函数 */
void bt_driver_register_callbacks(bt_driver_callbacks_t *callbacks);
```

### 3.4 实现

```c
/* 在 _bt_driver.c 中实现 */

static bt_driver_callbacks_t *g_callbacks = NULL;

void bt_driver_register_callbacks(bt_driver_callbacks_t *callbacks) {
    g_callbacks = callbacks;
}

/* 状态通知辅助函数 */
static void notify_state_change(bt_driver_state_t state, uint8_t host_idx) {
    if (g_callbacks && g_callbacks->on_state_change) {
        g_callbacks->on_state_change(state, host_idx);
    }
}
```

### 3.5 调用点

在 `hidkbd.c` 的 GAP 回调中调用：

```c
// 连接成功时
static void gap_role_state_change_callback(gapRole_States_t newState, gapRoleEvent_t *pGapParam) {
    switch (newState) {
        case GAPROLE_CONNECTED:
            notify_state_change(BT_STATE_CONNECTED, access_state.ble_idx);
            break;
        case GAPROLE_DISCONNECTED:
            notify_state_change(BT_STATE_DISCONNECTED, access_state.ble_idx);
            break;
        // ...
    }
}
```

---

## 4. HID 报告发送

### 4.1 问题

现有代码中报告 ID 使用错误：

```c
/* 当前错误的代码 */
uint8_t bt_driver_send_keyboard(uint8_t *report) {
    return HidDev_Report(HID_RPT_ID_MOUSE_IN, ...);  // 错误：用了鼠标报告 ID
}
```

### 4.2 解决方案

使用正确的 HID Report ID

```c
/* hidkbdservice.h 中的 Report ID 定义 */
#define HID_RPT_ID_KEY_IN       1    // 标准键盘
#define HID_RPT_ID_MOUSE_IN     2    // 鼠标
#define HID_RPT_ID_LED_OUT      3    // LED 输出
#define HID_RPT_ID_FEATURE_IN   4    // 消费者键
#define HID_RPT_ID_SYSTEM_IN    5    // 系统控制键

/* 修正后的发送函数 */
uint8_t bt_driver_send_keyboard(uint8_t *report) {
    return HidDev_Report(HID_RPT_ID_KEY_IN, HID_REPORT_TYPE_INPUT,
                       HID_KEYBOARD_IN_RPT_LEN, report);
}

uint8_t bt_driver_send_consumer(uint16_t report) {
    return HidDev_Report(HID_RPT_ID_FEATURE_IN, HID_REPORT_TYPE_INPUT,
                       sizeof(uint16_t), &report);
}

uint8_t bt_driver_send_system(uint16_t report) {
    return HidDev_Report(HID_RPT_ID_SYSTEM_IN, HID_REPORT_TYPE_INPUT,
                       sizeof(uint16_t), &report);
}

uint8_t bt_driver_send_mouse(uint8_t *report) {
    return HidDev_Report(HID_RPT_ID_MOUSE_IN, HID_REPORT_TYPE_INPUT,
                       HID_MOUSE_IN_RPT_LEN, report);
}

uint8_t bt_driver_send_nkro(uint8_t *report) {
    // NKRO 使用与标准键盘相同的报告 ID，但数据格式不同
    return HidDev_Report(HID_RPT_ID_KEY_IN, HID_REPORT_TYPE_INPUT,
                       HID_KEYBOARD_IN_RPT_LEN, report);
}
```

---

## 5. 多主机切换

### 5.1 问题

支持 3 个蓝牙主机 (BLE_INDEX_1/2/3) 的切换

### 5.2 当前实现分析

- `access_state.ble_idx` - 当前蓝牙主机索引
- `con_work_mode` - 当前工作模式
- `hidEmu_is_ble_bonded()` - 检查是否已绑定

### 5.3 解决方案

完善主机切换逻辑

```c
void bt_driver_connect_ex(uint8_t host_idx, uint16_t timeout) {
    // 1. 验证主机索引有效性
    if (host_idx < BLE_INDEX_1 || host_idx > BLE_INDEX_3) {
        host_idx = BLE_INDEX_1;
    }

    // 2. 检查是否已绑定该主机
    if (!hidEmu_is_ble_bonded(host_idx)) {
        bt_driver_pairing_ex(host_idx, NULL);
        return;
    }

    // 3. 如果当前已连接其他主机，先断开
    uint8_t ble_state;
    GAPRole_GetParameter(GAPROLE_STATE, &ble_state);

    if (ble_state == GAPROLE_CONNECTED && con_work_mode != host_idx) {
        hidEmu_disconnect();
        access_state.ble_idx = host_idx;
        access_state.pairing_state = FALSE;
    } else {
        access_state.ble_idx = host_idx;
        access_state.pairing_state = FALSE;
        hidEmu_adv_enable(ENABLE);
    }
}
```

### 5.4 切换流程

```
用户切换主机 → bt_driver_connect_ex()
    ↓
检查是否已绑定 → 否 → bt_driver_pairing_ex()
    ↓ 是
断开当前连接 → 等待 GAP 断开事件
    ↓
自动开始广播 → 连接新主机
```

---

## 6. 配对和连接管理

### 6.1 配对流程

```
bt_driver_pairing_ex(host_idx, param)
    ↓
断开当前连接（如果已连接）
    ↓
删除该主机的旧绑定信息（如果有）
    ↓
设置 access_state.ble_idx = host_idx
设置 access_state.pairing_state = TRUE
    ↓
开始广播（可连接模式）
    ↓
[主机发起连接和配对]
    ↓
GAP 绑定完成回调 → 保存绑定信息
    ↓
通知框架：BT_STATE_CONNECTED
```

### 6.2 代码实现

```c
void bt_driver_pairing_ex(uint8_t host_idx, void *param) {
    uint8_t ble_state;
    GAPRole_GetParameter(GAPROLE_STATE, &ble_state);

    // 1. 设置目标主机
    access_state.ble_idx = host_idx;
    access_state.pairing_state = TRUE;

    // 2. 如果已连接，先断开
    if (ble_state == GAPROLE_CONNECTED) {
        hidEmu_disconnect();
        return;
    }

    // 3. 如果正在广播，先停止
    if (ble_state == GAPROLE_ADVERTISING) {
        hidEmu_adv_enable(DISABLE);
    }

    // 4. 删除该主机的旧绑定信息
    // hidEmu_delete_ble_bonded_for_idx(host_idx);

    // 5. 开始广播
    hidEmu_adv_enable(ENABLE);

    // 6. 通知框架进入配对状态
    notify_state_change(BT_STATE_PAIRING, host_idx);
}
```

### 6.3 绑定信息保存

- 在 `hidkbd.c` 的 `GAPBondMgr` 回调中调用 `hidEmu_save_ble_bonded()`
- 绑定信息存储在 SNV 区域（由 BLE_SNV_ADDR 配置）

---

## 7. 电池电量通知

### 7.1 问题

`bt_driver_update_bat_level()` 函数体被注释掉了

### 7.2 解决方案

使用 Battery Service 上报电量

```c
void bt_driver_update_bat_level(uint8_t bat_lvl) {
    Batt_SetParameter(BATT_PARAM_LEVEL, sizeof(uint8_t), &bat_lvl);
}
```

### 7.3 Battery Service 初始化

在 `hidkbd.c` 的 `HidEmu_Init()` 中添加：

```c
void HidEmu_Init(void) {
    // ... 现有初始化代码 ...

    // 添加 Battery Service
    Batt_AddService();

    // 设置初始电量
    uint8_t initial_level = 100;
    Batt_SetParameter(BATT_PARAM_LEVEL, sizeof(uint8_t), &initial_level);
}
```

---

## 8. 文件修改清单

| 文件 | 修改内容 |
|------|----------|
| `drivers/communication/bluetooth/ch584/_bt_driver.h` | 添加回调类型定义和注册函数声明 |
| `drivers/communication/bluetooth/ch584/_bt_driver.c` | 实现回调注册、修复 HID 报告发送、完善连接/配对逻辑 |
| `drivers/communication/bluetooth/ch584/hidkbd.c` | 添加 GAP 状态变化通知调用、初始化 Battery Service |
| `drivers/communication/bluetooth/ch584/hidkbd.h` | 可能需要添加新的导出函数声明 |

---

## 9. 实现顺序

按照逐个功能迭代的方式：

1. **HID 报告发送** - 修复报告 ID，确保基本通信正常
2. **状态通知机制** - 建立框架与驱动的通信桥梁
3. **多主机切换** - 实现主机索引管理
4. **配对和连接管理** - 完善配对流程
5. **电池电量通知** - 添加 Battery Service 支持

每个功能完成后编译烧录测试。

---

## 10. 参考资料

- `docs/marker-output/沁恒低功耗蓝牙软件开发参考手册-rdqj/沁恒低功耗蓝牙软件开发参考手册-rdqj.md`
- 官方 demo: `D:\Code\CLionProjects\kb04122-13a-wch\BLE\3mode\`
- `project/ch584m/Profile/include/hidkbdservice.h` - HID Service 定义
- `project/ch584m/Profile/include/battservice.h` - Battery Service 定义

---

*设计完成时间: 2026-03-10*
