# 系统类型层切换实现计划

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 根据主机系统类型自动切换键盘基础层

**Architecture:** wireless 层连接成功时调用 keyboard 层新增的 `keyboard_update_base_layer_by_system()` 函数，该函数读取 `host_system_type` 并映射到对应层

**Tech Stack:** C, 现有 layer API (keymap.h)

---

## Task 1: 添加函数声明到 keyboard.h

**Files:**
- Modify: `middleware/keyboard/keyboard.h:21`

**Step 1: 在 keyboard.h 末尾添加函数声明**

在 `#endif` 之前添加：

```c
// 根据主机系统类型更新基础层
void keyboard_update_base_layer_by_system(void);
```

**Step 2: 编译验证**

Run: `cmd.exe /c "cd /d D:\\Code\\C_Project\\keyboard-framework\\project\\ch584m\\obj && set PATH=D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\components\\WCH\\Toolchain\\RISC-V Embedded GCC12\\bin;D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\others\\Build_Tools\\Make\\bin;%PATH% && make.exe main-build 2>&1"`

Expected: 编译失败，提示 `keyboard_update_base_layer_by_system` 未定义

**Step 3: Commit**

```bash
git add middleware/keyboard/keyboard.h
git commit -m "feat(keyboard): add keyboard_update_base_layer_by_system declaration"
```

---

## Task 2: 实现函数到 keyboard.c

**Files:**
- Modify: `middleware/keyboard/keyboard.c`

**Step 1: 在 keyboard.c 添加必要的 include**

在文件顶部的 include 区域添加：

```c
#include "keymap.h"
```

**Step 2: 在 keyboard.c 末尾添加函数实现**

```c
/**
 * @brief 根据主机系统类型更新基础层
 * @note 供 wireless 层在连接状态变化时调用
 */
void keyboard_update_base_layer_by_system(void) {
    extern uint8_t host_system_type;
    layer_id_t target_layer;

    switch (host_system_type) {
        case MAC:
        case IOS:
            target_layer = LAYER_MACOS;
            break;
        case WIN:
        case ANDROID:
        default:
            target_layer = LAYER_WINDOWS;
            break;
    }

    dprintf("Layer switch: system_type=%d, target_layer=%d\r\n", host_system_type, target_layer);
    layer_set(target_layer);
}
```

**Step 3: 编译验证**

Run: `cmd.exe /c "cd /d D:\\Code\\C_Project\\keyboard-framework\\project\\ch584m\\obj && set PATH=D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\components\\WCH\\Toolchain\\RISC-V Embedded GCC12\\bin;D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\others\\Build_Tools\\Make\\bin;%PATH% && make.exe main-build 2>&1"`

Expected: 编译成功

**Step 4: Commit**

```bash
git add middleware/keyboard/keyboard.c
git commit -m "feat(keyboard): implement keyboard_update_base_layer_by_system"
```

---

## Task 3: 在 wireless 层调用层切换函数

**Files:**
- Modify: `middleware/communication/wireless.c:306-326`

**Step 1: 在 wireless.c 添加 keyboard.h include**

在文件顶部的 include 区域添加：

```c
#include "keyboard.h"
```

**Step 2: 在 wireless_enter_connected 函数中添加调用**

在 `wireless_enter_connected()` 函数的 `clear_keyboard()` 之后添加调用：

```c
void wireless_enter_connected(uint8_t host_idx) {
    kc_printf("wireless_connected %d\n\r", host_idx);

    wireless_state = WT_CONNECTED;
    wireless_indicator_update(wireless_state, host_idx);
    host_index = host_idx;

    clear_keyboard();

    // 根据系统类型切换层
    keyboard_update_base_layer_by_system();

    /* Enable NKRO since it may be disabled in pin code entry */
#if defined(NKRO_ENABLE) && !defined(WIRELESS_NKRO_ENABLE)
    keymap_config.nkro = false;
#endif

    // wireless_enter_connected_kb(host_idx);
    if (battery_is_empty()) {
        indicator_battery_low_enable(true);
    }
    if (wireless_transport.update_bat_level) wireless_transport.update_bat_level(battery_get_percentage());
    lpm_timer_reset();
}
```

**Step 3: 编译验证**

Run: `cmd.exe /c "cd /d D:\\Code\\C_Project\\keyboard-framework\\project\\ch584m\\obj && set PATH=D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\components\\WCH\\Toolchain\\RISC-V Embedded GCC12\\bin;D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\others\\Build_Tools\\Make\\bin;%PATH% && make.exe main-build 2>&1"`

Expected: 编译成功

**Step 4: Commit**

```bash
git add middleware/communication/wireless.c
git commit -m "feat(wireless): call keyboard_update_base_layer_by_system on connect"
```

---

## Task 4: 清理注释掉的 process_layer_switch_key 代码

**Files:**
- Modify: `middleware/keyboard/keyboard.c`

**Step 1: 删除 process_layer_switch_key 相关代码**

删除以下内容：
- 第 28 行的静态函数声明
- 第 122-148 行的函数实现
- 第 68-72 行的注释掉函数调用

**Step 2: 编译验证**

Run: `cmd.exe /c "cd /d D:\\Code\\C_Project\\keyboard-framework\\project\\ch584m\\obj && set PATH=D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\components\\WCH\\Toolchain\\RISC-V Embedded GCC12\\bin;D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\others\\Build_Tools\\Make\\bin;%PATH% && make.exe main-build 2>&1"`

Expected: 编译成功

**Step 3: Commit**

```bash
git add middleware/keyboard/keyboard.c
git commit -m "refactor(keyboard): remove commented process_layer_switch_key code"
```

---

## 依赖说明

- `host_system_type` - wireless.h 中定义的全局变量，值为 MAC/IOS/WIN/ANDROID
- `layer_set()` - keymap.h 中定义的层切换 API
- `layer_id_t` - keymap.h 中定义的层枚举 (LAYER_MACOS=2, LAYER_WINDOWS=3)

## 测试方法

1. 烧录固件到设备
2. 连接到 Mac 主机，验证层切换到 LAYER_MACOS
3. 连接到 Windows 主机，验证层切换到 LAYER_WINDOWS
4. 查看调试日志确认 `Layer switch: system_type=X, target_layer=Y`
