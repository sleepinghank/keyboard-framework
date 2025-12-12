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

### Configuration

- **CMake**: Minimum version 3.29
- **C Standard**: GNU C11
- **Build Generators**: Visual Studio 17 2022 or Ninja
- **Platform**: Windows (cross-platform support planned for HAL layer)

### Build Commands

```bash
# Create build directory
mkdir cmake-build-debug && cd cmake-build-debug

# Configure with Visual Studio (recommended on Windows)
cmake -G "Visual Studio 17 2022" ..

# Build the project
cmake --build .

# Or build with specific configuration
cmake --build . --config Debug

# Alternative: Use Ninja generator (requires Ninja)
cmake -G Ninja ..
ninja

# Clean rebuild
rm -rf cmake-build-debug
mkdir cmake-build-debug && cd cmake-build-debug
cmake -G "Visual Studio 17 2022" ..
cmake --build .
```

### CMake Configuration Key Points

- Source files are automatically discovered via `file(GLOB_RECURSE)` from drivers/, middleware/, and utils/
- The main executable is built from `main.c` which contains Unity-based unit tests
- Include paths configured for all major directories (drivers, hal, middleware, keyboards, utils)

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

## Current Status

### Implemented

- Matrix scanning core logic
- Debounce system with multiple algorithms
- GPIO abstraction layer interface
- Unity-based test framework
- Basic build system with CMake

### In Progress / Planned

- Platform-specific HAL implementations
- Communication stack (BLE, USB, 2.4G)
- Touchpad driver integration
- Power management system
- Event management framework

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

## Process

### 1.按键流程

```mermaid
graph TD
    A[矩阵扫描] --> B[硬件防抖处理]
    B --> C[状态变化检测]
    
    C --> D{有按键变化?}
    D -->|是| E[逐按键处理]
    D -->|否| Z[结束本次扫描]
    
    E --> F[获取物理位置 row/col]
    F --> G[层级映射查询]
    
    G --> H[获取实际键码]
    H --> I[动作类型判断]
    
    I -->|普通按键| J[注册/注销键码]
    I -->|层切换键| K[更新层状态]
    I -->|宏键| L[执行宏序列]
    I -->|组合键| M[组合键处理]
    
    M --> N[状态跟踪和缓冲]
    N --> O[时间窗口检查]
    O --> P{条件满足?}
    P -->|是| Q[触发组合动作]
    P -->|否| R[继续监控]
    
    J --> S[USB HID报告生成]
    K --> T[刷新键码映射]
    L --> S
    Q --> S
    
    S --> U[发送到主机]
    U --> V[完成处理]
    
    T --> G
```



```mermaid
  graph TD
      A[矩阵扫描 Matrix Scan] --> B[硬件防抖 Debounce]
      B --> C[状态变化检测]
      C --> D{有按键变化?}
      D -->|是| E[action_exec]
      D -->|否| Z[结束]

      E --> F[创建 keyrecord_t]
      F --> G[get_record_keycode 获取键码]
      G --> H{是否启用 Combo?}
      H -->|是| I[process_combo 处理组合键]
      H -->|否| J[直接处理 Action]

      I --> K{Combo 触发?}
      K -->|是| L[执行 Combo Action]
      K -->|否| M[处理 Tap/Dance]

      J --> N[process_action 处理动作]
      M --> N

      N --> O[判断 Action 类型]
      O --> P{ACT_LMODS_TAP?}
      O --> Q{ACT_LAYER_TAP?}
      O --> R{其他 Action?}

      P -->|是| S[处理修饰键 Tap]
      Q -->|是| T[处理层 Tap]
      R -->|是| U[处理普通键/修饰键]

      S --> V[发送键码或保持修饰键]
      T --> W[短按发送键码/长按切换层]
      U --> X[注册/注销键码]

      L --> Y[完成]
      V --> Y
      W --> Y
      X --> Y
      Y --> Z
```



媒体键发送流程

```mermaid
  graph TD
      A[按键按下/释放] --> B[action_exec]
      B --> C[process_record]
      C --> D[process_record_handler]
      D --> E[process_action]

      E --> F{动作类型判断}
      F -->|ACT_USAGE| G[检查页面类型]
      G -->|PAGE_CONSUMER| H[调用 host_consumer_send]
      G -->|PAGE_SYSTEM| I[调用 host_system_send]

      H --> J[构建 report_extra_t]
      I --> J

      J --> K[检查重复发送]
      K -->|新 usage| L[调用 driver->send_extra]
      K -->|重复 usage| M[丢弃]

      L --> N{传输方式}
      N -->|USB| O[调用 USB 驱动]
      N -->|BLE| P[调用蓝牙驱动]

      O --> Q[USB 端点发送]
      P --> R[蓝牙协议发送]

      Q --> S[完成]
      R --> S
```

 三模无线键盘管理系统架构分析

  系统层次结构

  ┌─────────────────────────────────────────┐
  │         传输层 (Transport Layer)         │  USB / Bluetooth / 2.4G 模式切换
  ├─────────────────────────────────────────┤
  │       无线管理层 (Wireless Layer)        │  状态机、事件队列、HID报告处理
  ├─────────────────────────────────────────┤
  │        服务层 (Service Layer)           │  指示灯、电池、报告缓冲、定时器
  ├─────────────────────────────────────────┤
  │        驱动层 (Driver Layer)            │  蓝牙芯片驱动、低功耗管理
  └─────────────────────────────────────────┘

  核心文件结构

  1. 传输层管理: transport.c/h - 三模式切换控制
  2. 无线核心: wireless.c/h - 状态机与事件处理
  3. 指示灯: indicator.c/h - LED状态指示
  4. 电池管理: battery.c/h - 电量监测
  5. 报告缓冲: report_buffer.c/h - HID报告队列
  6. 低功耗: lpm.c/h - 功耗控制
  7. 蓝牙驱动: lkbt51.c/h - 蓝牙芯片通信

  完整流程图

  1. 系统初始化流程
  2. 
```mermaid
  graph TD
      A[系统上电] --> B[wireless_init]
      B --> C[初始化事件队列]
      C --> D[初始化指示灯]
      D --> E[初始化电池管理]
      E --> F[初始化低功耗管理]
      F --> G[初始化RTC定时器]
      G --> H[加载EEPROM配置]
      H --> I[wireless_set_transport 注册回调]
      I --> J[设置初始状态 WT_INITIALIZED]
```

  2. 三模式切换流程

```mermaid
  graph TD
      A[set_transport] --> B{目标模式}
      B -->|USB| C[usb_transport_enable]
      B -->|蓝牙| D[bt_transport_enable]
      B -->|2.4G| E[p24g_transport_enable]

      C --> F[切换到chibios_driver]
      C --> G[断开无线连接]
      C --> H[停止低功耗定时器]
    
      D --> I[切换到wireless_driver]
      D --> J[断开当前连接]
      D --> K[延时50ms]
      K --> L[wireless_connect_ex]
      L --> M[重置低功耗定时器]
    
      E --> N[切换到wireless_driver]
      E --> O[断开蓝牙连接]
      E --> P[延时50ms]
      P --> Q[连接2.4G]
      Q --> R[重置低功耗定时器]
    
      H --> S[transport_changed 回调]
      M --> S
      R --> S
      S --> T[重新初始化LED驱动]
      T --> U[设置RGB/LED矩阵超时]
```

  3. 蓝牙配对流程

```mermaid
  graph TD
      A[wireless_pairing] --> B[检查电池电量]
      B -->|低电| C[返回]
      B -->|正常| D[wireless_pairing_ex]
      D --> E[调用lkbt51_become_discoverable]
      E --> F[设置状态 WT_PARING]
      F --> G[设置配对指示]
      G --> H[发送EVT_DISCOVERABLE事件]
      H --> I[等待主机配对]
      I --> J{配对成功?}
      J -->|是| K[收到EVT_CONNECTED事件]
      J -->|否| L{超时?}
      L -->|是| M[进入WT_DISCONNECTED]
      L -->|否| I

      K --> N[wireless_enter_connected]
      N --> O[清除键盘状态]
      O --> P[启用NKRO]
      P --> Q[更新电池电量到模块]
      Q --> R[设置连接指示]
```

  4. 蓝牙连接流程

```mermaid
  graph TD
      A[wireless_connect] --> B[检查电池]
      B -->|低电| C[返回]
      B --> D[wireless_transport.connect_ex]
      D --> E[调用lkbt51_connect]
      E --> F[设置状态 WT_RECONNECTING]
      F --> G[设置重连指示]
      G --> H[蓝牙模块发起连接]
      H --> I{连接成功?}
      I -->|是| J[发送EVT_CONNECTED]
      I -->|否| K{重连超时?}
      K -->|是| L[发送EVT_DISCONNECTED]
      K -->|否| H

      J --> M[wireless_enter_connected]
      L --> N[wireless_enter_disconnected]
```

  5. HID报告发送流程

```mermaid
  graph TD
      A[按键事件] --> B[wireless_send_keyboard]
      B --> C[检查电池电量]
      C -->|低电| D[返回]
      C -->|正常| E{无线状态}

      E -->|WT_PARING| F{处于PIN码输入?}
      F -->|是| G[允许发送]
      F -->|否| H[返回]
    
      E -->|WT_CONNECTED| G
      E -->|其他| I[wireless_connect 自动重连]
    
      G --> J{启用报告缓冲?}
      J -->|是| K[report_buffer_enqueue]
      J -->|否| L[直接调用transport回调]
    
      K --> M[队列是否为空?]
      M -->|是| N[report_buffer_task 立即发送]
      M -->|否| O[等待发送]
    
      L --> P[wireless_transport.send_keyboard]
      P --> Q[调用lkbt51_send_keyboard]
      Q --> R[SPI发送到蓝牙芯片]
    
      N --> S[完成]
      O --> T[定时器触发发送]
      T --> R
      I --> U[延时后重试]
      U --> A
```

  6. 无线状态机

```mermaid
  stateDiagram-v2
      [*] --> WT_RESET : 系统复位
      WT_RESET --> WT_INITIALIZED : 初始化完成

      WT_INITIALIZED --> WT_DISCONNECTED : 准备就绪
      WT_DISCONNECTED --> WT_PARING : 开始配对
      WT_DISCONNECTED --> WT_RECONNECTING : 发起连接
    
      WT_PARING --> WT_CONNECTED : 配对成功
      WT_PARING --> WT_DISCONNECTED : 配对失败/超时
      WT_PARING --> WT_SUSPEND : 进入休眠
    
      WT_RECONNECTING --> WT_CONNECTED : 连接成功
      WT_RECONNECTING --> WT_DISCONNECTED : 连接失败
    
      WT_CONNECTED --> WT_DISCONNECTED : 连接断开
      WT_CONNECTED --> WT_SUSPEND : 休眠
    
      WT_SUSPEND --> WT_RECONNECTING : 唤醒
      WT_SUSPEND --> WT_DISCONNECTED : 唤醒后断连
```

  7. 事件处理流程

```mermaid
  graph TD
      A[硬件中断/定时器] --> B[创建wireless_event_t]
      B --> C[wireless_event_enqueue]
      C --> D[添加到事件队列]
      D --> E[wireless_task 轮询]
      E --> F[wireless_event_dequeue]
      F --> G{事件类型}

      G -->|EVT_RESET| H[wireless_enter_reset]
      G -->|EVT_DISCOVERABLE| I[wireless_enter_discoverable]
      G -->|EVT_CONNECTED| J[wireless_enter_connected]
      G -->|EVT_DISCONNECTED| K[wireless_enter_disconnected]
      G -->|EVT_RECONNECTING| L[wireless_enter_reconnecting]
      G -->|EVT_SLEEP| M[wireless_enter_sleep]
      G -->|EVT_BT_PINCODE_ENTRY| N[进入PIN码输入]
      G -->|EVT_HID_INDICATOR| O[更新LED状态]
    
      H --> P[indicator_set 设置指示]
      I --> P
      J --> P
      K --> P
      L --> P
      M --> P
      N --> Q[禁用NKRO]
      O --> R[led_update_ports]
```


  8. 指示灯控制流程

```mermaid
  graph TD
      A[indicator_set] --> B{传输模式}
      B -->|USB| C[返回]
      B -->|无线| D[检查状态变化]

      D --> E{当前状态}
      E -->|WT_DISCONNECTED| F[设置断开指示]
      E -->|WT_CONNECTED| G[设置连接指示]
      E -->|WT_PARING| H[设置配对指示]
      E -->|WT_RECONNECTING| I[设置重连指示]
      E -->|WT_SUSPEND| J[设置休眠指示]
    
      F --> K[更新LED矩阵]
      G --> K
      H --> K
      I --> K
      J --> K
    
      K --> L[启动定时器回调]
      L --> M[indicator_timer_cb]
      M --> N{指示类型}
    
      N -->|INDICATOR_ON| O[保持LED亮]
      N -->|INDICATOR_BLINK| P[闪烁LED]
      N -->|INDICATOR_ON_OFF| Q[定时开关]
      N -->|INDICATOR_OFF| R[关闭LED]
    
      O --> S[设置背光超时]
      P --> S
      Q --> S
      R --> S
```

  9. 电池管理流程
```mermaid
  graph TD
      A[battery_task 定时任务] --> B[读取ADC电压]
      B --> C[battery_calculate_voltage]
      C --> D[计算电量百分比]
      D --> E{电量状态}

      E -->|电量空| F[wireless_low_battery_shutdown]
      E -->|低电量| G[indicator_battery_low_enable]
      E -->|正常| H[更新显示]
    
      F --> I[清空键盘状态]
      I --> J[发送空报告]
      J --> K[等待300ms]
      K --> L[断开连接]
      L --> M[进入深度休眠]
    
      G --> N[启动低电指示]
      N --> O[定时闪烁LED]
      O --> P[检查是否需要关机]
      P -->|是| F
      P -->|否| Q[继续监控]
    
      H --> R[更新电池图标]
      R --> S[通知蓝牙模块电量]
```
  10. 低功耗管理流程

```mermaid
  graph TD
      A[lpm_task] --> B[检查USB连接]
      B -->|USB连接| C[返回_RUN模式]
      B -->|无USB| D[检查空闲时间]

      D --> E{空闲超时?}
      E -->|否| F[返回_RUN模式]
      E -->|是| G{无线状态}
    
      G -->|WT_CONNECTED| H[进入_STOP模式]
      G -->|WT_PARING| I[进入_STANDBY模式]
      G -->|其他| J[进入_SLEEP模式]
    
      H --> K[保持蓝牙连接]
      K --> L[等待唤醒事件]
      I --> M[等待配对]
      J --> N[完全休眠]
    
      L --> O[按键唤醒]
      M --> O
      N --> P[外部事件唤醒]
    
      O --> Q[lpm_timer_reset]
      P --> Q
      Q --> R[退出低功耗]
```

  11. 报告缓冲机制

```mermaid
  graph TD
      A[report_buffer_enqueue] --> B[检查队列是否满]
      B -->|满| C[覆盖最旧报告]
      B -->|否| D[添加到队列尾部]

      D --> E[report_buffer_task]
      E --> F{队列为空?}
      F -->|是| G[返回]
      F -->|否| H[检查发送间隔]
    
      H --> I{间隔时间到?}
      I -->|否| J[等待]
      I -->|是| K[report_buffer_dequeue]
    
      K --> L{报告类型}
      L -->|KB| M[wireless_transport.send_keyboard]
      L -->|NKRO| N[wireless_transport.send_nkro]
      L -->|CONSUMER| O[wireless_transport.send_consumer]
    
      M --> P[发送到蓝牙模块]
      N --> P
      O --> P
    
      P --> Q[发送成功?]
      Q -->|是| R[更新重试计数]
      Q -->|否| S[重试计数+1]
      S --> T{达到最大重试?}
      T -->|否| U[重新入队]
      T -->|是| V[丢弃报告]
    
      R --> W[继续发送下一条]
      U --> E
      V --> W
      J --> E
```

  12. 2.4G模式流程

```mermaid
  graph TD
      A[set_transport TRANSPORT_P2P4] --> B[禁用蓝牙]
      B --> C[延时1ms]
      C --> D[启用2.4G模块]
      D --> E[禁用USB]
      E --> F[重置低功耗定时器]

      F --> G[wireless_connect_ex P24G_INDEX]
      G --> H[调用p24g_transport_enable]
      H --> I[切换到wireless_driver]
      I --> J[断开当前连接]
      J --> K[延时50ms]
      K --> L[连接2.4G主机]
    
      L --> M[进入WT_RECONNECTING]
      M --> N[设置2.4G指示]
      N --> O[等待连接完成]
      O --> P{连接成功?}
      P -->|是| Q[EVT_CONNECTED]
      P -->|否| R{超时?}
      R -->|否| O
      R -->|是| S[EVT_DISCONNECTED]
    
      Q --> T[切换LED指示为绿色]
      S --> U[切换LED指示为熄灭]
```

### 指示灯实现流程

  A. 初始化流程 (indicator_init)

```mermaid
  graph TD
      A[indicator_init] --> B[清零配置]
      B --> C[初始化蓝牙LED引脚]
      C --> D[初始化2.4G LED引脚]
      D --> E[初始化通用LED引脚]
      E --> F[初始化低电量LED]
      F --> G[设置默认状态为熄灭]
```

  B. 状态设置流程 (indicator_set)

```mermaid
  graph TD
      A[indicator_set] --> B{检查是否为USB模式?}
      B -->|是| C[直接返回无指示]
      B -->|否| D[检查状态是否变化]
      D --> E{状态变化?}
      E -->|否| F[返回]
      E -->|是| G[设置新状态]
      G --> H{无线状态判断}
      H -->|WT_DISCONNECTED| I[应用断开配置]
      H -->|WT_CONNECTED| J[应用连接配置]
      H -->|WT_PARING| K[应用配对配置]
      H -->|WT_RECONNECTING| L[应用重连配置]
      H -->|WT_SUSPEND| M[应用休眠配置]
      I --> N[设置LED颜色/引脚]
      J --> N
      K --> N
      L --> N
      M --> N
      N --> O[启动定时器]
      O --> P[设置背光超时]
```

  C. 定时器回调处理 (indicator_timer_cb)

```mermaid
  graph TD
      A[定时器触发] --> B[获取指示灯类型]
      B --> C{类型判断}
      C -->|INDICATOR_ON| D[常亮处理]
      C -->|INDICATOR_BLINK| E[闪烁处理]
      C -->|INDICATOR_ON_OFF| F[开关处理]
      C -->|INDICATOR_OFF| G[关闭处理]
      D --> H[检查持续时间]
      E --> H
      F --> H
      G --> H
      H --> I{时间到?}
      I -->|否| J[更新LED状态]
      I -->|是| K[清除LED状态]
      J --> L[设置下一周期]
      K --> M[重置低功耗定时器]
      L --> N[完成]
      M --> N
```