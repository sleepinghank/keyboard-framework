# CH584 蓝牙驱动实现计划

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 完善 CH584 蓝牙驱动，实现 HID 报告发送、状态通知、多主机切换、配对管理和电池通知功能

**Architecture:** 在现有 `_bt_driver.c` 适配层上添加状态回调机制，修复 HID 报告发送逻辑，完善主机切换和配对流程，集成 Battery Service

**Tech Stack:** C 语言、CH584 BLE 协议栈、TMOS 事件系统、HID Profile

---

## Context

### 设计文档
- `docs/plans/2026-03-10-ch584-bluetooth-driver-design.md`

### 现有模块
- `drivers/communication/bluetooth/bt_driver.h` - 驱动接口定义
- `drivers/communication/bluetooth/ch584/_bt_driver.c` - 驱动适配层
- `drivers/communication/bluetooth/ch584/hidkbd.c` - BLE 应用层

### 官方资源
- `project/ch584m/Profile/include/hidkbdservice.h` - HID Service 定义
- `project/ch584m/Profile/include/battservice.h` - Battery Service 定义
- `docs/marker-output/沁恒低功耗蓝牙软件开发参考手册-rdqj/` - 开发手册

### 编译命令
```bash
cmd.exe /c "cd /d D:\\Code\\C_Project\\keyboard-framework\\project\\ch584m\\obj && set PATH=D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\components\\WCH\\Toolchain\\RISC-V Embedded GCC12\\bin;D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\others\\Build_Tools\\Make\\bin;%PATH% && make.exe clean && make.exe main-build"
```

---

## Tasks

### Task 1: 修复 HID 报告发送

**Files:**
- 修改: `drivers/communication/bluetooth/ch584/_bt_driver.c`
- 参考: `project/ch584m/Profile/include/hidkbdservice.h`

**Step 1: 查看 HID Report ID 定义**

先读取 `hidkbdservice.h` 确认正确的 Report ID：

```bash
# 查看文件中 Report ID 相关定义
```

**Step 2: 修复 bt_driver_send_keyboard 函数**

将错误的 `HID_RPT_ID_MOUSE_IN` 改为正确的 `HID_RPT_ID_KEY_IN`：

```c
uint8_t bt_driver_send_keyboard(uint8_t *report) {
    return HidDev_Report(HID_RPT_ID_KEY_IN, HID_REPORT_TYPE_INPUT,
                        HID_KEYBOARD_IN_RPT_LEN, report);
}
```

**Step 3: 修复 bt_driver_send_nkro 函数**

```c
uint8_t bt_driver_send_nkro(uint8_t *report) {
    return HidDev_Report(HID_RPT_ID_KEY_IN, HID_REPORT_TYPE_INPUT,
                        HID_KEYBOARD_IN_RPT_LEN, report);
}
```

**Step 4: 修复 bt_driver_send_consumer 函数**

```c
uint8_t bt_driver_send_consumer(uint16_t report) {
    uint8_t buffer[2];
    buffer[0] = LO_UINT16(report);
    buffer[1] = HI_UINT16(report);
    return HidDev_Report(HID_RPT_ID_CONSUMER_IN, HID_REPORT_TYPE_INPUT, 2, buffer);
}
```

**Step 5: 修复 bt_driver_send_system 函数**

```c
uint8_t bt_driver_send_system(uint16_t report) {
    uint8_t buffer[2];
    buffer[0] = LO_UINT16(report);
    buffer[1] = HI_UINT16(report);
    return HidDev_Report(HID_RPT_ID_SYSTEM_IN, HID_REPORT_TYPE_INPUT, 2, buffer);
}
```

**Step 6: 修复 bt_driver_send_mouse 函数**

```c
uint8_t bt_driver_send_mouse(uint8_t *report) {
    return HidDev_Report(HID_RPT_ID_MOUSE_IN, HID_REPORT_TYPE_INPUT,
                        HID_MOUSE_IN_RPT_LEN, report);
}
```

**Step 7: 编译验证**

```bash
cmd.exe /c "cd /d D:\\Code\\C_Project\\keyboard-framework\\project\\ch584m\\obj && set PATH=D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\components\\WCH\\Toolchain\\RISC-V Embedded GCC12\\bin;D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\others\\Build_Tools\\Make\\bin;%PATH% && make.exe main-build"
```

**Expected:** 编译成功，无新增错误

---

### Task 2: 添加状态通知机制

**Files:**
- 修改: `drivers/communication/bluetooth/ch584/_bt_driver.h`
- 修改: `drivers/communication/bluetooth/ch584/_bt_driver.c`

**Step 1: 在 _bt_driver.h 添加类型定义**

在 `#pragma once` 之后添加：

```c
/**
 * @brief 蓝牙驱动状态枚举
 */
typedef enum {
    BT_DRIVER_STATE_DISCONNECTED,   /**< 已断开连接 */
    BT_DRIVER_STATE_CONNECTED,      /**< 已连接 */
    BT_DRIVER_STATE_PAIRING,        /**< 配对中 */
    BT_DRIVER_STATE_RECONNECTING    /**< 重连中 */
} bt_driver_state_t;

/**
 * @brief 状态变化回调函数类型
 * @param state 新状态
 * @param host_idx 主机索引
 */
typedef void (*bt_driver_state_callback_t)(bt_driver_state_t state, uint8_t host_idx);

/**
 * @brief 蓝牙驱动回调函数集合
 */
typedef struct {
    bt_driver_state_callback_t on_state_change;      /**< 状态变化回调 */
    bt_driver_state_callback_t on_pairing_complete;  /**< 配对完成回调 */
    bt_driver_state_callback_t on_disconnect;        /**< 断开连接回调 */
} bt_driver_callbacks_t;

/**
 * @brief 注册回调函数
 * @param callbacks 回调函数结构体指针
 */
void bt_driver_register_callbacks(bt_driver_callbacks_t *callbacks);
```

**Step 2: 在 _bt_driver.c 添加静态变量**

在文件顶部的静态变量区域添加：

```c
/* 状态回调管理 */
static bt_driver_callbacks_t *g_bt_callbacks = NULL;
```

**Step 3: 实现 bt_driver_register_callbacks 函数**

添加函数实现：

```c
/**
 * @brief 注册回调函数
 */
void bt_driver_register_callbacks(bt_driver_callbacks_t *callbacks) {
    g_bt_callbacks = callbacks;
}

/**
 * @brief 内部函数：通知状态变化
 */
static void bt_driver_notify_state(bt_driver_state_t state, uint8_t host_idx) {
    if (g_bt_callbacks && g_bt_callbacks->on_state_change) {
        g_bt_callbacks->on_state_change(state, host_idx);
    }
}

/**
 * @brief 内部函数：通知配对完成
 */
static void bt_driver_notify_pairing_complete(uint8_t host_idx) {
    if (g_bt_callbacks && g_bt_callbacks->on_pairing_complete) {
        g_bt_callbacks->on_pairing_complete(BT_DRIVER_STATE_CONNECTED, host_idx);
    }
}

/**
 * @brief 内部函数：通知断开连接
 */
static void bt_driver_notify_disconnect(uint8_t host_idx) {
    if (g_bt_callbacks && g_bt_callbacks->on_disconnect) {
        g_bt_callbacks->on_disconnect(BT_DRIVER_STATE_DISCONNECTED, host_idx);
    }
}
```

**Step 4: 在 bt_driver.h 中添加声明**

在 `bt_driver.h` 文件末尾添加：

```c
/* 回调机制（CH584 特有） */
#ifdef CHIP_CH584
#include "ch584/_bt_driver.h"
#endif
```

**Step 5: 编译验证**

```bash
cmd.exe /c "cd /d D:\\Code\\C_Project\\keyboard-framework\\project\\ch584m\\obj && set PATH=D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\components\\WCH\\Toolchain\\RISC-V Embedded GCC12\\bin;D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\others\\Build_Tools\\Make\\bin;%PATH% && make.exe main-build"
```

**Expected:** 编译成功

---

### Task 3: 在 hidkbd.c 中集成状态通知

**Files:**
- 修改: `drivers/communication/bluetooth/ch584/hidkbd.c`

**Step 1: 添加头文件包含**

在文件顶部的 includes 中添加：

```c
#include "_bt_driver.h"
```

**Step 2: 找到 GAP 状态回调函数**

查找 `peripheralStateNotificationCB` 或类似的 GAP 状态回调函数。

**Step 3: 在连接成功处添加通知**

在 `GAPROLE_CONNECTED` 分支中添加：

```c
case GAPROLE_CONNECTED:
    // ... 现有代码 ...
    bt_driver_notify_state(BT_DRIVER_STATE_CONNECTED, access_state.ble_idx);
    break;
```

**Step 4: 在断开连接处添加通知**

在 `GAPROLE_DISCONNECTED` 分支中添加：

```c
case GAPROLE_DISCONNECTED:
    // ... 现有代码 ...
    bt_driver_notify_disconnect(access_state.ble_idx);
    break;
```

**Step 5: 在开始广播处添加通知**

在进入广播（配对或重连）时添加：

```c
case GAPROLE_ADVERTISING:
    if (access_state.pairing_state) {
        bt_driver_notify_state(BT_DRIVER_STATE_PAIRING, access_state.ble_idx);
    } else {
        bt_driver_notify_state(BT_DRIVER_STATE_RECONNECTING, access_state.ble_idx);
    }
    break;
```

**Step 6: 编译验证**

```bash
cmd.exe /c "cd /d D:\\Code\\C_Project\\keyboard-framework\\project\\ch584m\\obj && set PATH=D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\components\\WCH\\Toolchain\\RISC-V Embedded GCC12\\bin;D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\others\\Build_Tools\\Make\\bin;%PATH% && make.exe main-build"
```

**Expected:** 编译成功

---

### Task 4: 完善多主机切换逻辑

**Files:**
- 修改: `drivers/communication/bluetooth/ch584/_bt_driver.c`

**Step 1: 优化 bt_driver_connect_ex 函数**

替换现有实现：

```c
void bt_driver_connect_ex(uint8_t host_idx, uint16_t timeout) {
    uint8_t ble_state = 0;
    (void)timeout;  // 暂不使用超时参数

    dprintf("bt_driver_connect_ex: host_idx=%d\n", host_idx);

    /* 验证主机索引有效性 */
    if (host_idx < BLE_INDEX_1 || host_idx > BLE_INDEX_3) {
        /* 从存储获取当前索引 */
        storage_config_t *cfg = storage_get_config_ptr();
        if (cfg && cfg->ble_idx >= BLE_INDEX_1 && cfg->ble_idx <= BLE_INDEX_3) {
            host_idx = cfg->ble_idx;
        } else {
            host_idx = BLE_INDEX_1;
        }
    }

    /* 获取当前蓝牙状态 */
    GAPRole_GetParameter(GAPROLE_STATE, &ble_state);

    /* 如果已经是目标主机且已连接，无需操作 */
    if (con_work_mode == host_idx && ble_state == GAPROLE_CONNECTED) {
        dprintf("Already connected to host %d\n", host_idx);
        return;
    }

    /* 设置目标主机 */
    access_state.ble_idx = host_idx;
    access_state.pairing_state = FALSE;

    /* 检查是否已绑定 */
    if (!hidEmu_is_ble_bonded(host_idx)) {
        dprintf("Host %d not bonded, entering pairing mode\n", host_idx);
        bt_driver_pairing_ex(host_idx, NULL);
        return;
    }

    /* 根据当前状态处理 */
    switch (ble_state) {
        case GAPROLE_CONNECTED:
            /* 已连接其他主机，先断开 */
            dprintf("Disconnecting current host to switch to %d\n", host_idx);
            hidEmu_disconnect();
            /* 断开后会自动触发重连流程 */
            break;

        case GAPROLE_ADVERTISING:
            /* 正在广播，停止后重新开始 */
            hidEmu_adv_enable(DISABLE);
            /* 等待停止后再开始广播 */
            break;

        case GAPROLE_WAITING:
        case GAPROLE_ERROR:
            /* 等待状态或错误状态，直接开始广播 */
            con_work_mode = host_idx;
            hidEmu_adv_enable(ENABLE);
            break;

        default:
            /* 其他状态，设置工作模式 */
            con_work_mode = host_idx;
            break;
    }
}
```

**Step 2: 编译验证**

```bash
cmd.exe /c "cd /d D:\\Code\\C_Project\\keyboard-framework\\project\\ch584m\\obj && set PATH=D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\components\\WCH\\Toolchain\\RISC-V Embedded GCC12\\bin;D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\others\\Build_Tools\\Make\\bin;%PATH% && make.exe main-build"
```

**Expected:** 编译成功

---

### Task 5: 完善配对管理

**Files:**
- 修改: `drivers/communication/bluetooth/ch584/_bt_driver.c`

**Step 1: 优化 bt_driver_pairing_ex 函数**

```c
void bt_driver_pairing_ex(uint8_t host_idx, void *param) {
    uint8_t ble_state;
    pairing_param_t default_param = {0, 60, 0, 0, 0};  // 默认 60 秒超时

    dprintf("bt_driver_pairing_ex: host_idx=%d\n", host_idx);

    if (param == NULL) {
        param = &default_param;
    }

    /* 验证主机索引 */
    if (host_idx < BLE_INDEX_1 || host_idx > BLE_INDEX_3) {
        host_idx = BLE_INDEX_1;
    }

    /* 设置目标主机 */
    access_state.ble_idx = host_idx;
    access_state.pairing_state = TRUE;

    /* 获取当前状态 */
    GAPRole_GetParameter(GAPROLE_STATE, &ble_state);

    /* 如果已连接，先断开 */
    if (ble_state == GAPROLE_CONNECTED) {
        dprintf("Disconnecting before pairing\n");
        hidEmu_disconnect();
        return;  /* 断开后会自动进入配对广播 */
    }

    /* 如果正在广播，先停止 */
    if (ble_state == GAPROLE_ADVERTISING) {
        hidEmu_adv_enable(DISABLE);
    }

    /* 更新工作模式 */
    con_work_mode = host_idx;

    /* 开始配对广播 */
    hidEmu_adv_enable(ENABLE);

    /* 通知进入配对状态 */
    bt_driver_notify_state(BT_DRIVER_STATE_PAIRING, host_idx);
}
```

**Step 2: 编译验证**

```bash
cmd.exe /c "cd /d D:\\Code\\C_Project\\keyboard-framework\\project\\ch584m\\obj && set PATH=D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\components\\WCH\\Toolchain\\RISC-V Embedded GCC12\\bin;D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\others\\Build_Tools\\Make\\bin;%PATH% && make.exe main-build"
```

**Expected:** 编译成功

---

### Task 6: 实现电池电量通知

**Files:**
- 修改: `drivers/communication/bluetooth/ch584/_bt_driver.c`
- 修改: `drivers/communication/bluetooth/ch584/hidkbd.c`

**Step 1: 在 _bt_driver.c 中实现 bt_driver_update_bat_level**

```c
void bt_driver_update_bat_level(uint8_t bat_lvl) {
    dprintf("bt_driver_update_bat_level: %d%%\n", bat_lvl);
    Batt_SetParameter(BATT_PARAM_LEVEL, sizeof(uint8_t), &bat_lvl);
}
```

**Step 2: 确认 hidkbd.c 中已初始化 Battery Service**

检查 `HidEmu_Init()` 函数，确认包含以下调用：

```c
/* 添加 Battery Service */
Batt_AddService();

/* 设置初始电量 */
uint8_t initial_level = 100;
Batt_SetParameter(BATT_PARAM_LEVEL, sizeof(uint8_t), &initial_level);
```

如果没有，需要添加。

**Step 3: 确认头文件包含**

确保 `_bt_driver.c` 包含了 Battery Service 头文件：

```c
#include "battservice.h"
```

**Step 4: 编译验证**

```bash
cmd.exe /c "cd /d D:\\Code\\C_Project\\keyboard-framework\\project\\ch584m\\obj && set PATH=D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\components\\WCH\\Toolchain\\RISC-V Embedded GCC12\\bin;D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\others\\Build_Tools\\Make\\bin;%PATH% && make.exe main-build"
```

**Expected:** 编译成功

---

### Task 7: 提交代码

**Step 1: 查看修改**

```bash
git status
git diff
```

**Step 2: 提交**

```bash
git add drivers/communication/bluetooth/ch584/_bt_driver.h
git add drivers/communication/bluetooth/ch584/_bt_driver.c
git add drivers/communication/bluetooth/ch584/hidkbd.c
git add drivers/communication/bluetooth/bt_driver.h
git add docs/plans/2026-03-10-ch584-bluetooth-driver-design.md
git add docs/plans/2026-03-10-ch584-bluetooth-driver-implementation.md

git commit -m "$(cat <<'EOF'
feat(ch584): implement complete bluetooth driver

- Fix HID report sending with correct Report IDs
- Add state notification callback mechanism
- Improve multi-host switching logic
- Complete pairing management flow
- Implement battery level notification via Battery Service

Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>
EOF
)"
```

---

## Verification Checklist

- [ ] 编译无错误
- [ ] HID 键盘报告发送正确
- [ ] HID 鼠标报告发送正确
- [ ] HID 消费者键报告发送正确
- [ ] 状态通知回调正常触发
- [ ] 多主机切换逻辑正确
- [ ] 配对流程正常
- [ ] 电池电量通知正常

---

## Critical Files

| 文件 | 用途 |
|------|------|
| `drivers/communication/bluetooth/ch584/_bt_driver.h` | 驱动内部定义 |
| `drivers/communication/bluetooth/ch584/_bt_driver.c` | 驱动实现 |
| `drivers/communication/bluetooth/ch584/hidkbd.c` | BLE 应用层 |
| `project/ch584m/Profile/include/hidkbdservice.h` | HID Service 定义 |
| `project/ch584m/Profile/include/battservice.h` | Battery Service 定义 |

---

*计划创建时间: 2026-03-10*
