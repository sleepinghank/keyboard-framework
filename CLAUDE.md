# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

<!-- OPENSPEC:START -->
## OpenSpec Instructions

Always open `@/openspec/AGENTS.md` when the request:
- Mentions planning or proposals (words like proposal, spec, change, plan)
- Introduces new capabilities, breaking changes, architecture shifts, or big performance/security work
- Sounds ambiguous and you need the authoritative spec before coding

<!-- OPENSPEC:END -->

## Project Overview

**Modular keyboard framework** for multi-mode wireless keyboards and touchpads. Cross-platform layered architecture supporting PixArt 2860, WCH CH584, Nordic nRF52 series MCUs. Inspired by QMK firmware's key processing pipeline.

## Build Commands

### 标准构建 (MSYS2 MINGW64)

```bash
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build -j4
./build/keyboard-framework.exe

# 使用 toolchain 文件构建
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=mingw-toolchain.cmake
cmake --build build -j4
```

**环境要求:** MSYS2 MINGW64, CMake 3.29+, GCC 15.2.0+

**注意:** `test_main.c` 使用无限循环 (`OSAL_SystemProcess()`)，不是传统的单元测试框架。

### WCH RISC-V BLE 项目编译 (CH584M)

使用 MounRiver Studio 工具链编译 CH58x/CH59x 系列 BLE 固件：

```bash
# 编译命令 (通过 cmd.exe 执行，因为 makefile 使用 Windows 路径格式)
cmd.exe /c "cd /d D:\\Code\\C_Project\\keyboard-framework\\project\\ch584m\\obj && \
  set PATH=D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\components\\WCH\\Toolchain\\RISC-V Embedded GCC12\\bin;D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\others\\Build_Tools\\Make\\bin;%PATH% && \
  make.exe main-build"

# 清理并重新编译
cmd.exe /c "cd /d D:\\Code\\C_Project\\keyboard-framework\\project\\ch584m\\obj && \
  set PATH=D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\components\\WCH\\Toolchain\\RISC-V Embedded GCC12\\bin;D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\others\\Build_Tools\\Make\\bin;%PATH% && \
  make.exe clean && make.exe main-build"
```

**工具链路径:**
- GCC 编译器: `MounRiver_Studio2/resources/app/resources/win32/components/WCH/Toolchain/RISC-V Embedded GCC12/bin/`
- Make 工具: `MounRiver_Studio2/resources/app/resources/win32/others/Build_Tools/Make/bin/`

**编译输出:** `project/ch584m/obj/`
- `keyboard-framework.elf` - ELF 可执行文件（调试用）
- `keyboard-framework.hex` - HEX 固件（烧录用）
- `keyboard-framework.map` - 内存映射文件

**烧录工具:**
- WCH-Link: `MounRiver_Studio2/.../WCH-LinkUtility/WCH-LinkUtility.exe`
- ISP 串口: `MounRiver_Studio2/.../WCHISPTool_CH57x-59x/WCHISPTool_CH57x-59x.exe`

**快捷方式:** 使用 `/wch-riscv-build` skill 自动执行编译流程。

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│  Application Layer (test_main.c, services)                  │
├─────────────────────────────────────────────────────────────┤
│  Middleware Layer                                           │
│  ├── keyboard/   按键处理、层级、组合键、HID报告            │
│  └── communication/  三模无线、传输、报告缓冲               │
├─────────────────────────────────────────────────────────────┤
│  Driver Layer                                               │
│  ├── input/      矩阵扫描、防抖、触摸板                     │
│  ├── output/     背光、指示灯、日志                         │
│  ├── communication/  BLE、2.4G 驱动                         │
│  └── system/     定时器、事件管理                           │
├─────────────────────────────────────────────────────────────┤
│  HAL Layer (hal/platforms/${PLATFORM}/)                     │
│  GPIO, ADC, PWM, I2C, SPI, UART 抽象接口                    │
└─────────────────────────────────────────────────────────────┘
```

## Key Processing Flow

```
matrix_scan() → debounce() → action_exec() → layer_switch/keycode
     ↓              ↓             ↓               ↓
drivers/input  drivers/input  middleware/    middleware/
  /keyboard      /keyboard     keyboard/       keyboard/
   matrix.c      debounce.c    action.c       action_layer.c
```

## Core Modules

### 按键处理 (middleware/keyboard/)
- `keyboard.c` - 主协调：矩阵扫描→防抖→action处理→combo
- `action.c/action_layer.c` - 按键动作与层级切换
- `combo.c` - 组合键状态机
- `report.c` - HID 报告生成
- `keycode.h/keycodes.h` - 键码定义

### 驱动层 (drivers/input/keyboard/)
- `matrix.c` - 矩阵扫描，支持 COL2ROW/ROW2COL
- `debounce.c` - 多算法防抖：sym_defer_pk, sym_eager_pk, asym_eager_defer_pk

### 三模无线 (middleware/communication/)
- `transport.c` - USB/BLE/2.4G 模式切换
- `wireless.c` - 无线状态机
- `report_buffer.c` - HID 报告队列
- `lpm.c` - 低功耗管理

### 无线初始化顺序

```
┌─────────────────────────────────────────────────────────────────┐
│  system_init_drivers()                                          │
│  ├── storage_init()                                             │
│  └── bt_driver_init(false)     // 具体驱动硬件/协议栈初始化    │
│      └── CH58x_BLEInit() → HAL_Init() → GAPRole_PeripheralInit()│
├─────────────────────────────────────────────────────────────────┤
│  system_init_middleware()                                       │
│  ├── lpm_init()                                                 │
│  └── wireless_init()            // 无线状态机初始化            │
│      └── 设置 WT_INITIALIZED, 加载配置, report_buffer_init()  │
├─────────────────────────────────────────────────────────────────┤
│  运行时 (用户按键/上电策略)                                     │
│  └── set_transport(TRANSPORT_BLUETOOTH)  // 选择通道           │
│      ├── bt_transport_enable(true)                              │
│      │   ├── host_set_driver(&wireless_driver)                 │
│      │   └── wireless_connect_ex(host_idx, 0)                  │
│      └── wireless_switch_to_bt_driver()  // 绑定驱动函数表     │
└─────────────────────────────────────────────────────────────────┘
```

### 职责边界

| 层级 | 函数 | 职责 |
|------|------|------|
| Driver | `bt_driver_init()` | BLE 协议栈初始化（CH58x_BLEInit, HAL_Init 等） |
| Middleware | `wireless_init()` | 状态机初始化、配置加载、事件队列初始化 |
| Middleware | `set_transport()` | 通道切换、host_driver 设置、发起连接 |

### 无线状态机
```
WT_RESET → WT_INITIALIZED → WT_DISCONNECTED ⇄ WT_PARING
                                ↓↑              ↓
                          WT_RECONNECTING → WT_CONNECTED ⇄ WT_SUSPEND
```

## Configuration

### 产品配置 (keyboards/product_config.h)
```c
#define MATRIX_ROWS 6
#define MATRIX_COLS 13
#define MATRIX_ROW_PINS { 0, 1, 2, 3, 4, 5 }
#define MATRIX_COL_PINS { 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 }
#define DIODE_DIRECTION COL2ROW
#define DEBOUNCE 5
```

### 系统配置 (application/sys_config.h)
- `CHIP_TYPE` - 目标 MCU (CH584M, PAR2860)
- `USB_ENABLE`, `BLE_ENABLE` - 功能开关
- `PRINTF_LEVEL` - 调试日志级别

## Testing

```bash
# 运行主程序
./build/keyboard-framework.exe

# 单文件测试编译示例 (不依赖 CMake)
gcc -DTEST_MODE -o storage_test.exe test/storage_test.c drivers/storage/storage.c drivers/storage/test/eeprom.c -Idrivers/storage -Ikeyboards -Ihal -Iutils
./storage_test.exe
```

测试宏: `RUN_TEST()`, `TEST_ASSERT_TRUE/FALSE/EQUAL` (Unity 框架)

## Code Style

- **命名:** 函数/变量使用 `snake_case`，宏/常量使用 `UPPER_SNAKE_CASE`，类型以 `_t` 结尾
- **类型:** 优先使用固定宽度整数 (`uint8_t`, `uint16_t`, `uint32_t`)，语义布尔值使用 `bool`
- **头文件:** 使用 `#pragma once`，显式包含直接依赖
- **错误处理:** 早期验证指针和范围输入，保持错误路径显式且无副作用
- **日志风格:** 与模块保持一致 (`dprintf`, `println`, `xprintf`)
- **预处理器:** 特性开关 (`#ifdef`) 是可移植性的核心，不要随意移除

## Event-Driven Patterns

- 事件处理器清除已消费的位并返回剩余位
- 通过 `OSAL_ProcessEventRegister` 注册，通过 `OSAL_SetEvent` 触发
- 保持状态机名称显式 (`WT_*`, init status enums)

## Module Lifecycle

```c
_setup();      // 早期启动，_init 之前
_init();       // 主机协议、MCU 外设初始化后
_pre_task();   // task 之前
_task();       // 主循环
_post_task();  // task 之后
```

## Platform Porting

1. 创建 `hal/platforms/${PLATFORM}/` 目录
2. 实现 GPIO 等 HAL 接口 (`_gpio.c`, `_adc.c`, etc.)
3. 更新 `CMakeLists.txt` 添加源文件
4. 配置 `keyboards/product_config.h`

## Feature Branches

- `feature/keyboard-simplified` - 精简键盘模块（使用 linkedlist + 新 combo 系统）

## Tools

- Context7 MCP - 用于获取库/API 文档
- cmake-build-tester agent - 构建验证
- `/feature-brainstorming` skill - 功能开发前的需求分析与方案设计（金字塔原理 + MECE），位于 `.claude/skills/feature-brainstorming/`

## Agent Working Rules

1. 修改前阅读 `CMakeLists.txt` 和相关模块头文件
2. 保持层级边界，避免跨层直接调用
3. 优先复用现有事件/状态机模式
4. 保持 diff 最小化和本地化
5. 如果构建失败，报告确切的缺失文件/符号和所属层级
6. 如果添加测试，在注释中包含清晰的单测试运行命令

## Critical Files

- 入口: `test_main.c`
- 启动流程: `application/system/system_init.c`
- 事件 API: `drivers/system/event_manager.h`
- 键盘核心: `middleware/keyboard/keyboard.c`
- 传输切换: `middleware/communication/transport.c`
- 无线抽象: `middleware/communication/wireless.h`

## Important Notes

**开发流程要求:** 实现任何需求时，严禁直接修改代码。必须先理清需求、确认方案、编写详细设计文档，经过评审后才能动手编码。

**Windows 注意:** 项目根目录存在名为 `nul` 的文件，在 Windows 上进行广度递归 grep 可能会失败。
