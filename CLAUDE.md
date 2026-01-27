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

```bash
# MSYS2 MINGW64 环境
cd /d/Code/C_Project/keyboard-framework
rm -rf build && mkdir build && cd build
cmake .. -G 'MinGW Makefiles'
mingw32-make
./keyboard-framework.exe
```

**环境要求:** MSYS2 MINGW64, CMake 3.29+, GCC 15.2.0+

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
# 运行测试
./build/keyboard-framework.exe
```

测试宏: `RUN_TEST()`, `TEST_ASSERT_TRUE/FALSE/EQUAL`

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
