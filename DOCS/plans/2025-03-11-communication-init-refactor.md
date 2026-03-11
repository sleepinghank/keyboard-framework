# 1通信模块初始化流程重构实现计划

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 重构通信模块初始化流程，明确 transport 和 wireless 职责边界，确保 BLE 协议栈正确初始化。

**Architecture:** Driver 层负责具体硬件/协议栈初始化（bt_driver_init），Middleware 层负责状态机（wireless_init），Application 层负责通道选择（set_transport）。

**Tech Stack:** C, CH584 BLE SDK, 层级架构

---

## Task 1: 修改 system_init_drivers() 添加蓝牙驱动初始化

**Files:**
- Modify: `application/system/system_init.c:142-163`

**Step 1: 添加 bt_driver_init() 调用**

在 `system_init_drivers()` 函数中，`storage_init()` 之后添加蓝牙驱动初始化：

```c
void system_init_drivers(void) {
    // 驱动层初始化阶段
    // 按依赖关系顺序初始化各驱动

    // 1. 存储系统初始化 (最优先)
    storage_init();

#ifdef BLUETOOTH_ENABLE_FLAG
    // 2. 蓝牙驱动初始化 - BLE 协议栈
    bt_driver_init(false);
#endif

#ifdef P2P4G_ENABLE_FLAG
    // 3. 2.4G 驱动初始化
    p24g_driver_init();
#endif

    // 标记Driver init完成
    g_system_init_status = SYSTEM_INIT_STATUS_DRIVER_INIT;
}
```

**Step 2: 确认头文件包含**

确认文件顶部已包含 `bt_driver.h`（当前已有）。如需 2.4G，添加 `p24g_driver.h`。

**Step 3: 验证编译**

Run: `cmd.exe /c "cd /d D:\\Code\\C_Project\\keyboard-framework\\project\\ch584m\\obj && set PATH=D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\components\\WCH\\Toolchain\\RISC-V Embedded GCC12\\bin;D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\others\\Build_Tools\\Make\\bin;%PATH% && make.exe clean && make.exe main-build"`

Expected: 编译成功，无错误

**Step 4: Commit**

```bash
git add application/system/system_init.c
git commit -m "feat(init): add bt_driver_init call in driver layer"
```

---

## Task 2: 修改 system_init_middleware() 移除无线驱动切换

**Files:**
- Modify: `application/system/system_init.c:165-186`

**Step 1: 移除 wireless_switch_to_bt_driver() 调用**

修改 `system_init_middleware()` 函数：

```c
void system_init_middleware(void) {
    // 中间件初始化阶段
    // 按依赖关系顺序初始化各中间件

    // 4. 低功耗管理初始化
    lpm_init();

    // 5. 无线管理层初始化（只初始化状态机，不切换驱动）
    wireless_init();

    // 6. 键盘处理初始化
    // keyboard_init();

    // 标记Middleware init完成
    g_system_init_status = SYSTEM_INIT_STATUS_MIDDLEWARE_INIT;
}
```

**Step 2: 验证编译**

Run: `cmd.exe /c "cd /d D:\\Code\\C_Project\\keyboard-framework\\project\\ch584m\\obj && set PATH=D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\components\\WCH\\Toolchain\\RISC-V Embedded GCC12\\bin;D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\others\\Build_Tools\\Make\\bin;%PATH% && make.exe main-build"`

Expected: 编译成功，无错误

**Step 3: Commit**

```bash
git add application/system/system_init.c
git commit -m "refactor(init): remove wireless_switch_to_bt_driver from middleware init"
```

---

## Task 3: 添加默认通道选择函数

**Files:**
- Modify: `application/system/system_init.c`
- Modify: `application/system/system_init.h`（如需声明）

**Step 1: 添加 system_select_default_transport() 函数**

在 `system_init_application()` 末尾添加通道选择逻辑：

```c
void system_select_default_transport(void) {
    transport_t default_transport = TRANSPORT_NONE;

#ifdef USB_ENABLE_FLAG
    // TODO: 实现 USB 连接检测
    // if (usb_is_connected()) {
    //     default_transport = TRANSPORT_USB;
    // }
#endif

    // 默认使用蓝牙通道
    if (default_transport == TRANSPORT_NONE) {
#ifdef BLUETOOTH_ENABLE_FLAG
        default_transport = TRANSPORT_BLUETOOTH;
#elif defined(P2P4G_ENABLE_FLAG)
        default_transport = TRANSPORT_P2P4;
#endif
    }

    if (default_transport != TRANSPORT_NONE) {
        set_transport(default_transport);
    }
}

void system_init_application(void) {
    // 应用层初始化阶段
    input_service_init();
    output_service_init();

    // 选择默认传输通道
    system_select_default_transport();

    // 标记Application init完成
    g_system_init_status = SYSTEM_INIT_STATUS_APPLICATION_INIT;
    g_system_init_status = SYSTEM_INIT_STATUS_COMPLETED;
    g_system_initialized = true;
}
```

**Step 2: 添加必要的头文件包含**

在文件顶部确认包含 `transport.h`：

```c
// middleware
#include "report_buffer.h"
#include "lpm.h"
#include "wireless.h"
#include "keyboard.h"
#include "transport.h"  // 添加此行
```

**Step 3: 验证编译**

Run: `cmd.exe /c "cd /d D:\\Code\\C_Project\\keyboard-framework\\project\\ch584m\\obj && set PATH=D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\components\\WCH\\Toolchain\\RISC-V Embedded GCC12\\bin;D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\others\\Build_Tools\\Make\\bin;%PATH% && make.exe main-build"`

Expected: 编译成功，无错误

**Step 4: Commit**

```bash
git add application/system/system_init.c
git commit -m "feat(init): add system_select_default_transport for channel selection"
```

---

## Task 4: 验证初始化流程

**Files:**
- None (测试验证)

**Step 1: 代码审查 - 确认初始化顺序**

检查 `system_init_coordinator()` 的调用顺序应为：

1. `system_hal_init()` - HAL 初始化
2. `system_setup_*()` - setup 阶段
3. `system_init_hal()` - HAL init
4. `system_init_drivers()` - **bt_driver_init()** 在此调用
5. `system_init_middleware()` - **wireless_init()** 在此调用
6. `system_init_application()` - **set_transport()** 在此调用

**Step 2: 添加调试日志确认流程**

在 `bt_driver_init()` 中确认有调试输出：
```c
void bt_driver_init(bool wakeup_from_low_power) {
    if (g_bt_driver_initialized) return;

    dprintf("bt_driver_init: starting BLE stack init\n");
    CH58x_BLEInit();
    // ...
    dprintf("bt_driver_init: BLE stack initialized\n");
    g_bt_driver_initialized = true;
}
```

**Step 3: 烧录测试**

1. 编译固件
2. 使用 WCH-Link 烧录
3. 观察串口输出确认初始化流程

**Step 4: 功能验证**

- [ ] 设备上电后串口输出 `bt_driver_init: starting BLE stack init`
- [ ] 设备可被手机蓝牙扫描发现
- [ ] 设备可配对并连接
- [ ] 按键可通过蓝牙发送

**Step 5: Final Commit**

```bash
git add docs/plans/2025-03-11-communication-init-refactor.md
git commit -m "docs: add communication init refactor plan"
```

---

## 验证清单

| 检查项 | 预期结果 |
|--------|----------|
| 编译通过 | 无错误无警告 |
| bt_driver_init() 被调用 | 串口输出初始化日志 |
| wireless_init() 状态正确 | wireless_get_state() == WT_INITIALIZED |
| set_transport() 触发连接 | 设备开始广播 |
| 蓝牙可发现 | 手机可扫描到设备 |

## 回滚方案

如需回滚，恢复以下修改：
1. 从 `system_init_drivers()` 移除 `bt_driver_init()` 调用
2. 在 `system_init_middleware()` 恢复 `wireless_switch_to_bt_driver()` 调用
3. 移除 `system_select_default_transport()` 函数





