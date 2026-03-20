# 系统初始化流程重构 - 实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 简化系统初始化生命周期，从 10 状态简化为 4 状态，移除跨层调用，提升代码可维护性。

**Architecture:** 删除空的 setup 阶段，将 battery_init/backlight_init 从 service 层移至 driver 层，为 wireless_init 添加幂等保护。

**Tech Stack:** C (嵌入式), CH584M MCU, OSAL 事件系统

---

## File Structure

| 文件 | 操作 | 职责 |
|------|------|------|
| `middleware/communication/wireless.c` | 修改 | 添加 wireless_init 幂等保护 |
| `application/service/input_service.c` | 修改 | 删除 battery_init() 调用 |
| `application/service/output_service.c` | 修改 | 删除 backlight_init() 调用 |
| `application/service/communication_service.c` | 修改 | 删除 WL_INIT_EVT 中的 wireless_init() |
| `application/system/system_init.c` | 修改 | 删除 setup 阶段，新增 driver init 调用 |
| `application/system/system_init.h` | 修改 | 删除 setup 函数声明，更新状态枚举 |
| `docs/system_init_startup_flow.md` | 修改 | 更新初始化流程文档 |

---

## Task 1: 添加 wireless_init 幂等保护

**Files:**
- Modify: `middleware/communication/wireless.c:116-126`

- [ ] **Step 1: 修改 wireless_init 函数**

在 `wireless_init()` 函数开头添加幂等保护标志：

```c
void wireless_init(void) {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    wireless_state = WT_INITIALIZED;

#if defined(EECONFIG_BASE_WIRELESS_CONFIG)
    wireless_config_load();
#endif

#ifndef DISABLE_REPORT_BUFFER
    report_buffer_init();
#endif
}
```

---

## Task 2: 移除 input_service 中的跨层调用

**Files:**
- Modify: `application/service/input_service.c:268`

- [ ] **Step 1: 删除 battery_init() 调用**

修改 `input_service_init()` 函数，删除 `battery_init()` 调用：

```c
void input_service_init(void) {
    /* 注册任务并获取任务ID */
    input_taskID = OSAL_ProcessEventRegister(input_process_event);
    dprintf("Input: Service initialized with task ID %d\r\n", input_taskID);
    // battery_init();  // 已移至 system_init_drivers()
    g_last_battery_power_state = 0xFF;
    /* 启动硬件定时器驱动的矩阵扫描 */
    matrix_scan_timer_start();
    dprintf("Input: Matrix scan timer started\r\n");
    /* 启动电量检测定时任务 */
    OSAL_StartReloadTask(input_taskID, INPUT_BATTERY_DETE_EVT, BATTERY_DETECT_INTERVAL);

#ifdef TOUCH_EN
    /* 初始化触控板 */
    touch_power_on();
    dprintf("Input: Touchpad initialized\r\n");
    /* 启动触控板看门狗定时检查 */
    OSAL_StartReloadTask(input_taskID, INPUT_TOUCH_WATCHDOG_EVT, TOUCH_WATCHDOG_INTERVAL);
#endif
}
```


---

## Task 3: 移除 output_service 中的跨层调用

**Files:**
- Modify: `application/service/output_service.c:302`

- [ ] **Step 1: 删除 backlight_init() 调用**

修改 `output_service_init()` 函数，删除 `backlight_init(NULL)` 调用：

```c
void output_service_init(void) {
    PRINT("Output service init start\r\n");

    output_taskID = OSAL_ProcessEventRegister(output_process_event);
    // backlight_init(NULL);  // 已移至 system_init_drivers()
    backlight_set_preset_color(BL_COLOR_WHITE);
    backlight_set_preset_level(BL_LEVEL_MEDIUM);
    output_service_ready = (output_taskID != 0xFF);
    backlight_sleeping = false;
    backlight_low_battery = false;
    backlight_lpm_disabled = false;
    backlight_enabled_before_low_battery = backlight_is_enabled();
    if (output_service_ready) {
        output_service_note_backlight_activity();
    } else {
        PRINT("Output service task register failed\r\n");
    }

    PRINT("Task registered, ID=%d\r\n", output_taskID);
    PRINT("Output service init done\r\n");
}
```

---

## Task 4: 移除 communication_service 中的重复初始化

**Files:**
- Modify: `application/service/communication_service.c:53-58`

- [ ] **Step 1: 删除 WL_INIT_EVT 中的 wireless_init() 调用**

修改 `commu_process_event()` 函数：

```c
    // 处理无线模块初始化事件
    if (events & WL_INIT_EVT) {
        // wireless_init();  // 已在 system_init_middleware() 调用
        // 选择默认传输通道
        system_select_default_transport();
        dprintf("Communication: Wireless module initialization\r\n");
        return (events ^ WL_INIT_EVT);
    }
```


---

## Task 5: 添加 GPIO 安全状态初始化到 system_hal_init

**Files:**
- Modify: `hal/platforms/ch584/_system_hal.c:23-46`

- [ ] **Step 1: 在 system_hal_init() 开头添加 GPIO 安全状态初始化**

在 `system_hal_init()` 函数开头调用 `system_hal_gpio_init_all()`：

```c
system_result_t system_hal_init(void)
{
    if (g_system_initialized) {
        return SYSTEM_ERROR_NOT_SUPPORTED;
    }

    // 首先初始化所有GPIO为安全状态，防止悬空漏电
    system_hal_gpio_init_all();

    //电源相关
#if(defined(DCDC_ENABLE)) && (DCDC_ENABLE == TRUE)
    // ... 后续代码不变
```


---

## Task 6: 简化 system_init 状态机和删除 setup 阶段

**Files:**
- Modify: `application/system/system_init.c`
- Modify: `application/system/system_init.h`

- [ ] **Step 1: 更新状态枚举 (system_init.h)**

将状态枚举从 10 状态简化为 7 状态（保留 STATUS_ 前缀以保持兼容）：

```c
// 系统初始化状态
typedef enum {
    SYSTEM_INIT_STATUS_NOT_STARTED = 0,    // 未开始
    SYSTEM_INIT_STATUS_HAL,                // HAL init 完成
    SYSTEM_INIT_STATUS_DRIVER,             // Driver init 完成
    SYSTEM_INIT_STATUS_MIDDLEWARE,         // Middleware init 完成
    SYSTEM_INIT_STATUS_APPLICATION,        // Application init 完成
    SYSTEM_INIT_STATUS_TASK,               // 进入主循环
    SYSTEM_INIT_STATUS_COMPLETED           // 完全初始化完成
} system_init_status_t;
```

- [ ] **Step 2: 删除 setup 函数声明 (system_init.h)**

删除以下声明：
```c
// 删除这些声明
void system_setup_hal(void);
void system_setup_drivers(void);
void system_setup_middleware(void);
void system_setup_application(void);
```

- [ ] **Step 3: 删除 setup 函数实现 (system_init.c)**

删除以下 4 个函数：
- `system_setup_hal()` (约 76-92 行)
- `system_setup_drivers()` (约 94-101 行)
- `system_setup_middleware()` (约 103-109 行)
- `system_setup_application()` (约 111-117 行)

- [ ] **Step 4: 更新 system_init_drivers() 添加 battery_init 和 backlight_init**

```c
void system_init_drivers(void) {
    // 驱动层初始化阶段
    // 按依赖关系顺序初始化各驱动

    // 时钟初始化
    timer_init();

    // 1. 存储系统初始化 (最优先)
    storage_init();

#ifdef BLUETOOTH_ENABLE_FLAG
    // 2. 蓝牙驱动初始化 - BLE 协议栈
    bt_driver_init(false);
#endif

    // 3. 电池管理初始化 (从 input_service 移入)
    battery_init();

    // 4. 背光初始化 (从 output_service 移入)
    backlight_init(NULL);

    // 5. 指示灯初始化
    indicator_init();

    // 标记Driver init完成
    g_system_init_status = SYSTEM_INIT_STATUS_DRIVER;
}
```

- [ ] **Step 5: 更新 system_init_coordinator()**

删除 setup 阶段调用：

```c
uint32_t system_init_coordinator(void) {
    // 1. 系统硬件初始化（时钟、GPIO等）
    system_hal_init();

    // 阶段: _init 阶段 (主机协议初始化后)
    system_init_hal();
    system_init_drivers();
    system_init_middleware();
    system_init_application();

    return 0;  // 成功
}
```

- [ ] **Step 6: 更新状态变量引用**

确保所有状态变量引用使用新的枚举值（保留 STATUS_ 前缀）：
- `SYSTEM_INIT_STATUS_HAL_SETUP` → 删除
- `SYSTEM_INIT_STATUS_DRIVER_SETUP` → 删除
- `SYSTEM_INIT_STATUS_MIDDLEWARE_SETUP` → 删除
- `SYSTEM_INIT_STATUS_APPLICATION_SETUP` → 删除
- `SYSTEM_INIT_STATUS_HAL_INIT` → `SYSTEM_INIT_STATUS_HAL`
- `SYSTEM_INIT_STATUS_DRIVER_INIT` → `SYSTEM_INIT_STATUS_DRIVER`
- `SYSTEM_INIT_STATUS_MIDDLEWARE_INIT` → `SYSTEM_INIT_STATUS_MIDDLEWARE`
- `SYSTEM_INIT_STATUS_APPLICATION_INIT` → `SYSTEM_INIT_STATUS_APPLICATION`
- `SYSTEM_INIT_STATUS_COMPLETED` → 保持不变


---

## Task 7: 更新初始化流程文档

**Files:**
- Modify: `docs/system_init_startup_flow.md`

- [ ] **Step 1: 更新文档**

更新初始化流程图，反映新的 4 阶段架构：

```markdown
# 系统初始化启动流程

## 生命周期

简化后的生命周期：**init → task**

## 初始化阶段

### 1. HAL 层初始化 (system_init_hal)
- UART 初始化
- GPIO 配置
- 硬件定时器初始化

### 2. Driver 层初始化 (system_init_drivers)
- timer_init() - 时钟
- storage_init() - EEPROM
- bt_driver_init() - BLE 协议栈
- battery_init() - 电池管理
- backlight_init() - 背光
- indicator_init() - 指示灯

### 3. Middleware 层初始化 (system_init_middleware)
- lpm_init() - 低功耗管理
- wireless_init() - 无线状态机
- keyboard_init() - 键盘处理

### 4. Application 层初始化 (system_init_application)
- system_service_init() - 系统服务
- input_service_init() - 输入服务
- output_service_init() - 输出服务
- commu_service_init() - 通信服务

## 状态机

```
SYSTEM_INIT_IDLE → SYSTEM_INIT_HAL → SYSTEM_INIT_DRIVER →
SYSTEM_INIT_MIDDLEWARE → SYSTEM_INIT_APPLICATION → SYSTEM_INIT_TASK →
SYSTEM_INIT_COMPLETE
```

## 添加新服务模块

1. 创建 `xxx_service.c/h` 文件
2. 在 `system_init_application()` 添加一行 `xxx_service_init()` 调用
3. （可选）在 main.c 的 task 循环添加 task 调用
```

- [ ] **Step 2: Commit**

```bash
git add docs/system_init_startup_flow.md
git commit -m "docs: update system init flow documentation"
```

---

## Task 8: 最终编译验证

- [ ] **Step 2: CH584M 固件编译**

Run: 使用 `/wch-riscv-build` skill 或执行以下命令：
```bash
cmd.exe /c "cd /d D:\\Code\\C_Project\\keyboard-framework\\project\\ch584m\\obj && \
  set PATH=D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\components\\WCH\\Toolchain\\RISC-V Embedded GCC12\\bin;D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\others\\Build_Tools\\Make\\bin;%PATH% && \
  make.exe clean && make.exe main-build"
```
Expected: 无编译错误，生成 `keyboard-framework.hex`

---

## Task 9: 功能回归测试

测试前请确保烧录到 CH584M 硬件。

- [ ] **Step 1: 冷启动测试**
- 上电后指示灯正常亮起
- 无线可连接

- [ ] **Step 2: 按键输入测试**
- 按键响应正常，无漏键

- [ ] **Step 3: 三模切换测试**
- USB/BLE/2.4G 切换正常

- [ ] **Step 4: 低功耗测试**
- 5 秒无操作进入 Idle
- 观察电流或 LED 状态

- [ ] **Step 5: 唤醒测试**
- Idle 后按键唤醒正常
- Deep 后配置恢复正常（背光/无线状态）

---

## Task 10: 提交最终代码

- [ ] **Step 1: 检查所有改动**

Run: `git status`
Expected: 所有改动已提交

- [ ] **Step 2: 推送到远程（可选）**

```bash
git push origin <branch-name>
```

---

## Summary

| Task | 内容 | 文件 |
|------|------|------|
| 1 | wireless_init 幂等保护 | wireless.c |
| 2 | 删除 input_service 跨层调用 | input_service.c |
| 3 | 删除 output_service 跨层调用 | output_service.c |
| 4 | 删除 commu_service 重复初始化 | communication_service.c |
| 5 | GPIO 安全状态移至 system_hal_init | _system_hal.c |
| 6 | 简化 system_init 状态机 | system_init.c/h |
| 7 | 更新文档 | docs/ |
| 8 | 编译验证 | - |
| 9 | 功能回归测试 | - |
| 10 | 最终提交 | - |
