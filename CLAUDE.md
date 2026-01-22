<!-- OPENSPEC:START -->
# OpenSpec Instructions

These instructions are for AI assistants working in this project.

Always open `@/openspec/AGENTS.md` when the request:
- Mentions planning or proposals (words like proposal, spec, change, plan)
- Introduces new capabilities, breaking changes, architecture shifts, or big performance/security work
- Sounds ambiguous and you need the authoritative spec before coding

Use `@/openspec/AGENTS.md` to learn:
- How to create and apply change proposals
- Spec format and conventions
- Project structure and guidelines

Keep this managed block so 'openspec update' can refresh the instructions.

<!-- OPENSPEC:END -->

# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a **modular keyboard framework** designed for multi-mode wireless keyboards and touchpads. It provides a cross-platform, layered architecture supporting multiple MCU platforms (PixArt 2860, WCH CH584, Nordic nRF52 series). The framework is inspired by QMK firmware's key processing pipeline and implements a clean separation of concerns through HAL (Hardware Abstraction Layer), driver, middleware, and application layers.

## Repository Structure

```
keyboard-framework/
├── hal/                        # Hardware Abstraction Layer
│   ├── interface/              # HAL interfaces (GPIO, ADC, PWM, I2C, UART, Timer, Power)
│   └── platforms/              # Platform-specific HAL implementations
├── drivers/                    # Device Drivers
│   ├── communication/          # BLE, USB, 2.4G wireless protocols
│   ├── input/                  # Keyboard matrix, touchpad, buttons
│   ├── output/                 # LEDs, backlight, indicators
│   ├── storage/                # EEPROM, Flash storage
│   ├── power/                  # Battery monitoring, power management
│   └── system/                 # Timers, watchdog, event management
├── middleware/                 # Middleware Services
│   ├── config/                 # Configuration management
│   ├── protocol/               # HID protocol processing
│   └── keyboard/               # Key mapping, combo handling
├── keyboards/                  # Product-specific configurations
├── application/                # Application layer
│   ├── main.c                  # Test application with Unity framework
│   ├── mode/                   # Operation/power/connection modes
│   └── service/                # Input/output/communication services
├── utils/                      # Utilities (ring buffer, CRC, logger)
└── test/                       # Test utilities and Unity framework
```

## Build System

### 构建环境要求

- **操作系统**: Windows
- **构建工具**: MSYS2 (MINGW64)
- **CMake**: 版本 3.29+
- **编译器**: GCC (MinGW-w64) 15.2.0+
- **Make**: mingw32-make

### Build Commands

### 方法一：使用 MSYS2 Bash（推荐）

```bash
# 进入项目目录（使用 MSYS2 路径格式）
cd /d/Code/C_Project/keil_example

# 清理之前的构建
rm -rf build

# 创建构建目录
mkdir build
cd build

# 配置 CMake（生成 MinGW Makefiles）
cmake .. -G 'MinGW Makefiles'

# 编译项目
mingw32-make

# 运行程序
./keyboard-framework.exe
```
## Key Components

### 1. Matrix Scanning (`drivers/input/keyboard/matrix.c`)

- Core matrix scanning implementation supporting COL2ROW and ROW2COL diode directions
- Platform-agnostic implementation with GPIO abstraction
- Reads key states row-by-row or column-by-column depending on hardware configuration
- Integrates with debounce module for signal filtering

### 2. Debounce System (`drivers/input/keyboard/debounce.c`)

- Multiple debounce algorithms: sym_defer_pk, sym_eager_pk, asym_eager_defer_pk
- Per-key, per-row, and global tracking options
- Configurable debounce timing via `DEBOUNCE` macro
- Provides clean key state transitions

### 3. GPIO HAL (`hal/gpio.h`)

- Abstract GPIO interface with functions:
  - `gpio_set_pin_input/output_push_pull/output_open_drain`
  - `gpio_write_pin_high/low`
  - `gpio_read_pin`
  - `gpio_toggle_pin`
- Platform-specific implementations in `hal/platforms/${PLATFORM}/`

### 4. Product Configuration (`keyboards/product_config.h`)

- Matrix configuration (rows, cols, pins, diode direction)
- USB and BLE parameters
- Feature toggles (tap, combo, leader key configurations)
- Pin definitions and IO settings

### 5. System Configuration (`application/sys_config.h`)

- Chip type selection (CH584M, PAR2860)
- Debug/logging configuration with levels (ASSERT, DEBUG, INFO, WARN, ERROR, VERBOSE)
- Feature enables (USB, BLE)
- Platform UART configuration for logging

## Testing Framework

The project uses a **custom Unity-based testing framework** integrated in `main.c`:

### Test Structure

- `RUN_TEST(test_func)` - Macro to execute tests with setup/teardown
- `TEST_ASSERT_TRUE/FALSE/EQUAL/EQUAL_HEX16` - Assertion macros
- `setUp()` / `tearDown()` - Test initialization/cleanup hooks

### Available Tests (in main.c)

1. `test_matrix_scan_basic` - Basic matrix scanning functionality
2. `test_matrix_multiple_keys_scan` - Multiple simultaneous key presses
3. `test_matrix_scan_sequence` - Key press/release sequences
4. `test_matrix_is_on` - Individual key state checking
5. `test_matrix_row_boundaries` - Boundary condition testing

### Running Tests

```bash
# Build and run tests
cd cmake-build-debug
cmake --build .
.\Debug\keyboard_framework.exe

# Or from project root
.\cmake-build-debug\Debug\keyboard_framework.exe
```

Test output includes:

- Test execution counter
- Pass/fail counts
- Detailed failure messages with file and line numbers

## Configuration Options

### Matrix Configuration (keyboards/product_config.h)

- `MATRIX_ROWS`, `MATRIX_COLS` - Matrix dimensions
- `MATRIX_ROW_PINS`, `MATRIX_COL_PINS` - GPIO pin assignments
- `DIODE_DIRECTION` - COL2ROW or ROW2COL
- `MATRIX_INPUT_PRESSED_STATE` - Active low/high configuration

### Debounce Configuration

- `DEBOUNCE` - Debounce delay in milliseconds
- `DEBOUNCE_ALGORITHM` - Algorithm selection (DEBOUNCE_SYM_DEFER_PK, etc.)
- `MAX_MATRIX_ROWS` - Maximum supported rows

### Platform Features (application/sys_config.h)

- `CHIP_TYPE` - Target MCU (CH584M, PAR2860)
- `USB_ENABLE` - Enable USB HID support
- `BLE_ENABLE` - Enable Bluetooth support
- `PRINTF_LEVEL` - Debug logging verbosity

## Development Workflow

### Adding New Features

1. **Hardware abstraction**: Add platform-specific code to `hal/platforms/${PLATFORM}/`
2. **Drivers**: Implement device drivers in `drivers/` following the existing pattern
3. **Product config**: Define hardware-specific settings in `keyboards/product_config.h`
4. **Testing**: Add unit tests to `main.c` using the provided macros

### Platform Porting

1. Create platform directory: `hal/platforms/${PLATFORM_NAME}/`
2. Implement GPIO and other HAL interfaces
3. Update `CMakeLists.txt` to include platform-specific source files
4. Configure product settings in `keyboards/product_config.h`

### Code Style

- Follow QMK coding conventions (see `code_example/qmk_firmware/docs/coding_conventions_c.md`)
- Use `#pragma once` for header guards
- Prefix platform-specific functions with platform name or use HAL abstraction
- Document all public APIs with Doxygen-style comments

## Main Entry Point

- **主程序入口**: `test_main.c`
- **系统初始化**: `system_init_coordinator()` in `application/system/system_init.c`
- **主循环**: `OSAL_SystemProcess()` 事件驱动模式

## Current Status

### Implemented

- Matrix scanning core logic
- Debounce system with multiple algorithms
- GPIO/ADC/PWM/I2C/SPI/UART HAL interfaces
- Wireless communication framework (BLE, 2.4G)
- Transport layer with mode switching
- Indicator LED system
- Battery monitoring
- Report buffer mechanism
- Low power management (LPM)
- Storage system with CRC validation
- Event manager (OSAL-based)
- Test platform HAL implementations

### In Progress / Planned

- Platform-specific HAL for CH584/nRF52
- USB HID implementation
- Touchpad driver integration
- RGB backlight effects

## Common Tasks

### Modify Matrix Configuration

Edit `keyboards/product_config.h`:

```c
#define MATRIX_ROWS 6
#define MATRIX_COLS 13
#define MATRIX_ROW_PINS { 0, 1, 2, 3, 4, 5 }
#define MATRIX_COL_PINS { 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 }
```

### Add New Test Case

In `main.c`, add a new test function:

```c
void test_my_feature(void) {
    printf("Testing my feature...\n");
    // Test implementation
    TEST_ASSERT_TRUE(condition);
}

int main(void) {
    // ...
    RUN_TEST(test_my_feature);
    // ...
}
```

### Debug Logging

Enable debug output by setting appropriate level in `application/sys_config.h`:

```c
#define PRINTF_LEVEL PRINTF_LEVEL_DEBUG
```

Use logging macros (defined in logger.h when implemented):

```c
LOG_DEBUG("Matrix scan changed: %d", changed);
LOG_INFO("Battery level: %d%%", battery_level);
```

## Troubleshooting

### Build Errors

- **Character encoding**: Files contain UTF-8 Chinese comments - ensure editor preserves encoding
- **Missing platform files**: Platform-specific HAL files may be missing
- **Linker errors**: Verify all source files are included in CMakeLists.txt

### Runtime Issues

- **Matrix scan not working**: Check pin definitions in product_config.h
- **Debounce issues**: Verify DEBOUNCE algorithm and timing settings
- **Test failures**: Check TEST_ASSERT statements and expected vs actual values

### Platform-Specific Issues

- **GPIO not responding**: Verify HAL implementation for your platform
- **Compilation errors**: Check C standard (-std=gnu11) and include paths

## Process Overview

### 按键处理流程

矩阵扫描 → 硬件防抖 → 状态变化检测 → 层级映射 → 动作处理 → HID报告生成 → 发送

### 三模无线系统架构

```
传输层 (Transport)     →  USB / Bluetooth / 2.4G 模式切换
无线管理层 (Wireless)  →  状态机、事件队列、HID报告处理
服务层 (Service)       →  指示灯、电池、报告缓冲、定时器
驱动层 (Driver)        →  蓝牙芯片驱动、低功耗管理
```

### 无线状态机

```
WT_RESET → WT_INITIALIZED → WT_DISCONNECTED ⇄ WT_PARING
                                ↓↑              ↓
                          WT_RECONNECTING → WT_CONNECTED ⇄ WT_SUSPEND
```

### 核心文件

- `middleware/communication/transport.c` - 三模式切换控制
- `middleware/communication/wireless.c` - 状态机与事件处理
- `drivers/output/indicators/indicator.c` - LED状态指示
- `drivers/power/battery.c` - 电量监测
- `middleware/communication/report_buffer.c` - HID报告队列
- `middleware/communication/lpm.c` - 功耗控制
- `drivers/storage/storage.c` - 存储管理

### 模块生命周期规范

```c
/* 早期启动阶段，在 _init 之前执行 */
_setup(void);
/* 主机协议、调试和 MCU 外设初始化之后执行 */
_init(void);
/* task 之前执行 */
_pre_task();
/* 主循环中重复执行 */
_task(void);
/* task 之后执行 */
_post_task();
```
