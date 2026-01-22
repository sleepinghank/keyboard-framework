# Project Context

## Purpose

模块化键盘框架（Modular Keyboard Framework），专为多模无线键盘和触摸板设计。提供跨平台、分层架构，支持多种 MCU 平台（PixArt 2860、WCH CH584、Nordic nRF52 系列）。设计灵感来源于 QMK 固件的按键处理流水线，通过 HAL、驱动、中间件和应用层实现清晰的关注点分离。

## Tech Stack

- **语言**: C (GNU11 标准)
- **构建工具**: CMake 3.29+ / MinGW Makefiles
- **编译器**: GCC (MinGW-w64) 15.2.0+
- **测试框架**: Unity
- **代码格式化**: clang-format
- **Shell 环境**: MSYS2 MINGW64
- **目标平台**:
  - WCH CH584M（蓝牙 + 2.4G）
  - Nordic nRF52 系列
  - PixArt PAR2860

## Project Conventions

### Code Style

- **缩进**: 4 空格
- **行宽限制**: 120 字符
- **命名规范**: snake_case（变量、函数、文件名）
- **头文件保护**: `#pragma once`
- **文件末尾**: 保留空行
- **注释语言**: 中文
- **代码规范参考**: QMK coding conventions

### Architecture Patterns

分层架构设计：

```
Application Layer (应用层)
    ├── mode/          - 操作/电源/连接模式
    └── service/       - 输入/输出/通信服务

Middleware Layer (中间件层)
    ├── config/        - 配置管理
    ├── protocol/      - HID 协议处理
    ├── keyboard/      - 按键映射、组合键
    └── communication/ - 传输层、无线管理

Driver Layer (驱动层)
    ├── input/         - 矩阵扫描、触摸板、按钮
    ├── output/        - LED、背光、指示灯
    ├── communication/ - BLE、USB、2.4G
    ├── storage/       - EEPROM、Flash
    ├── power/         - 电池监控、电源管理
    └── system/        - 定时器、看门狗、事件管理

HAL Layer (硬件抽象层)
    ├── interface/     - GPIO、ADC、PWM、I2C、UART、Timer、Power
    └── platforms/     - 平台特定实现
```

**模块生命周期规范**:
```c
_setup(void);      // 早期启动阶段
_init(void);       // MCU 外设初始化后
_pre_task(void);   // task 之前
_task(void);       // 主循环
_post_task(void);  // task 之后
```

### Testing Strategy

- **框架**: Unity（集成在 main.c）
- **测试宏**: `RUN_TEST()`, `TEST_ASSERT_TRUE/FALSE/EQUAL/EQUAL_HEX16`
- **钩子函数**: `setUp()` / `tearDown()`
- **测试覆盖**: 矩阵扫描、防抖系统、边界条件
- **运行方式**: 编译后直接执行可执行文件

### Git Workflow

- **Commit 格式**: `type(scope): description`
  - type: `feat`, `fix`, `refactor`, `docs`, `test`, `chore`
- **分支命名**: `feature/xxx`, `fix/xxx`, `refactor/xxx`
- **Co-Author**: `Co-Authored-By: Claude Opus 4.5 <noreply@anthropic.com>`
- **注意**: 不自动 push/commit，等待用户确认

## Domain Context

### 按键处理流程

```
矩阵扫描 → 硬件防抖 → 状态变化检测 → 层级映射 → 动作处理 → HID 报告生成 → 发送
```

### 三模无线系统

- **传输层 (Transport)**: USB / Bluetooth / 2.4G 模式切换
- **无线管理层 (Wireless)**: 状态机、事件队列、HID 报告处理
- **服务层 (Service)**: 指示灯、电池、报告缓冲、定时器
- **驱动层 (Driver)**: 蓝牙芯片驱动、低功耗管理

### 无线状态机

```
WT_RESET → WT_INITIALIZED → WT_DISCONNECTED ⇄ WT_PARING
                                ↓↑              ↓
                          WT_RECONNECTING → WT_CONNECTED ⇄ WT_SUSPEND
```

### 核心模块

| 模块 | 文件路径 | 功能 |
|------|----------|------|
| 矩阵扫描 | `drivers/input/keyboard/matrix.c` | COL2ROW/ROW2COL 矩阵扫描 |
| 防抖系统 | `drivers/input/keyboard/debounce.c` | 多算法支持 |
| 传输控制 | `middleware/communication/transport.c` | 三模式切换 |
| 状态机 | `middleware/communication/wireless.c` | 无线状态管理 |
| 指示灯 | `drivers/output/indicators/indicator.c` | LED 状态指示 |
| 存储管理 | `drivers/storage/storage.c` | CRC 验证存储 |

## Important Constraints

- **内存限制**: 嵌入式环境，需注意 RAM/Flash 占用
- **实时性**: 按键响应延迟敏感（目标 < 1ms 扫描周期）
- **功耗**: 支持低功耗模式，电池供电设备
- **兼容性**: 需要适配多个 MCU 平台的差异
- **安全**: 不提交 `.env`、credentials、API keys 等敏感文件

## External Dependencies

- **BLE 协议栈**: 平台厂商提供（CH584 使用 WCH BLE 库）
- **USB 协议栈**: 平台厂商提供
- **OSAL**: 事件驱动操作系统抽象层
- **HID 规范**: USB HID 1.11 规范
