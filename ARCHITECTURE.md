# 键盘框架架构设计文档

## 目录
1. [架构概览](#1-架构概览)
2. [分层架构设计](#2-分层架构设计)
3. [目录结构](#3-目录结构)
4. [核心模块详解](#4-核心模块详解)
5. [数据流程图](#5-数据流程图)
6. [模块调用关系](#6-模块调用关系)
7. [初始化流程](#7-初始化流程)
8. [按键处理流程](#8-按键处理流程)
9. [通信流程](#9-通信流程)
10. [配置系统](#10-配置系统)
11. [平台适配](#11-平台适配)

---

## 1. 架构概览

### 1.1 设计理念
本键盘框架采用**四层分层架构**设计，灵感来源于QMK固件，但更加模块化和平台无关。设计原则：
- **分层解耦**: 严格遵循HAL → Driver → Middleware → Application 的分层
- **平台无关**: 通过HAL层抽象硬件差异
- **事件驱动**: 采用事件队列和任务调度机制
- **模块化**: 每个模块职责单一，易于测试和维护

### 1.2 整体架构图

```mermaid
graph TB
    subgraph "应用层 Application Layer"
        AS[应用服务]
        SM[系统管理]
        MAIN[主程序]
    end

    subgraph "中间件层 Middleware Layer"
        KB[键盘处理]
        COM[通信管理]
        LPM[低功耗]
        RPT[报告缓冲]
    end

    subgraph "驱动层 Driver Layer"
        subgraph "输入驱动"
            MTX[矩阵扫描]
            DBN[防抖处理]
            TCH[触摸板]
        end

        subgraph "输出驱动"
            LED[LED控制]
            BKL[背光]
            IND[指示灯]
        end

        subgraph "通信驱动"
            BT[蓝牙]
            P24G[2.4G]
            USB[USB]
        end

        subgraph "系统驱动"
            TMR[定时器]
            EVT[事件管理]
            BAT[电池]
            STG[存储]
        end
    end

    subgraph "硬件抽象层 HAL"
        GPIO[GPIO抽象]
        I2C[I2C总线]
        UART[串口]
        PWM[PWM]
        ADC[ADC]
        SPI[SPI]
        PIN[引脚定义]
    end

    subgraph "硬件平台"
        CH584[CH584M]
        NRF52[nRF52]
        P2860[PixArt2860]
        TEST[Test平台]
    end

    MAIN --> AS
    AS --> KB
    KB --> COM
    COM --> MTX
    MTX --> GPIO
    GPIO --> CH584
```

---

## 2. 分层架构设计

### 2.1 硬件抽象层 (HAL - Hardware Abstraction Layer)

**职责**: 抽象硬件平台差异，提供统一的硬件操作接口

**设计原则**:
- 定义统一的接口规范
- 平台-specific实现位于 `hal/platforms/` 目录
- 上层代码不直接操作硬件寄存器

**主要模块**:

| 模块 | 头文件 | 平台实现 | 说明 |
|------|--------|----------|------|
| GPIO | `gpio.h` | `platforms/*/_gpio.c` | GPIO引脚控制 |
| I2C | `i2c_master.h` | `platforms/*/_i2c_master.c` | I2C主设备 |
| UART | `uart.h` | `platforms/*/_uart.c` | 串口通信 |
| PWM | `pwm.h` | `platforms/*/_pwm.c` | PWM控制 |
| ADC | `adc.h` | `platforms/*/_adc.c` | ADC采样 |
| SPI | `spi_master.h` | `platforms/*/_spi_master.c` | SPI主设备 |
| 引脚定义 | `pin_defs.h` | `platforms/*/_pin_defs.h` | 引脚映射 |

**接口示例**:
```c
// GPIO接口 (hal/gpio.h)
void gpio_set_pin_input(pin_t pin);
void gpio_set_pin_input_high(pin_t pin);
void gpio_set_pin_output_push_pull(pin_t pin);
void gpio_write_pin_high(pin_t pin);
void gpio_write_pin_low(pin_t pin);
uint8_t gpio_read_pin(pin_t pin);
void gpio_toggle_pin(pin_t pin);
```

### 2.2 驱动层 (Driver Layer)

**职责**: 设备驱动实现，将硬件能力封装为功能模块

**设计原则**:
- 每个驱动对应一类硬件设备
- 驱动可以相互依赖，但避免循环依赖
- 提供标准化的驱动接口

**驱动分类**:

#### 2.2.1 输入驱动
- **矩阵扫描** (`drivers/input/keyboard/matrix.c`)
  - 扫描键盘矩阵，读取按键状态
  - 支持COL2ROW和ROW2COL
  - 可配置扫描顺序

- **防抖处理** (`drivers/input/keyboard/debounce.c`)
  - 硬件信号滤波
  - 多种防抖算法
  - 时间戳跟踪

- **触摸板** (`drivers/input/touchpad/`)
  - 触摸手势识别
  - 多点触控支持

#### 2.2.2 输出驱动
- **LED控制** (`drivers/output/leds/`)
  - RGB矩阵控制
  - 背光控制

- **背光** (`drivers/output/backlight/`)
  - PWM背光调节
  - 亮度控制

- **指示灯** (`drivers/output/indicators/`)
  - 状态指示
  - 蓝牙/无线状态

#### 2.2.3 通信驱动
- **蓝牙** (`drivers/communication/bluetooth/`)
  - BLE协议栈
  - 配对管理
  - HID报告

- **2.4G无线** (`drivers/communication/p2p4g/`)
  - 2.4G协议
  - 主机配对
  - 报告发送

- **USB** (`drivers/communication/usb/`)
  - USB HID
  - 端点管理
  - 设备枚举

#### 2.2.4 系统驱动
- **定时器** (`drivers/system/timer.c`)
  - 系统时间基准
  - 延时管理
  - 溢出处理

- **事件管理** (`drivers/system/event_manager.c`)
  - 事件队列
  - 任务调度
  - 消息传递

- **电池管理** (`drivers/power/battery.c`)
  - 电量检测
  - 低电告警
  - 功耗优化

- **存储** (`drivers/storage/storage.c`)
  - EEPROM操作
  - 配置保存
  - CRC校验

### 2.3 中间件层 (Middleware Layer)

**职责**: 提供高级功能服务，连接驱动和应用

**设计原则**:
- 组合多个驱动能力形成完整功能
- 状态管理和逻辑处理
- 为应用层提供简化接口

**主要模块**:

#### 2.3.1 键盘处理中间件
- **键盘主逻辑** (`middleware/keyboard/keyboard.c`)
  - 按键事件处理
  - 状态机管理
  - 任务调度

- **动作执行** (`middleware/keyboard/action.c`)
  - 动作解析
  - 层切换
  - 修饰键处理

- **层管理** (`middleware/keyboard/action_layer.c`)
  - 多层键位映射
  - 层切换逻辑
  - 临时层

- **组合键** (`middleware/keyboard/combo.c`)
  - 组合键检测
  - 时序判断
  - 触发动作

- **键码处理** (`middleware/keyboard/keymap_common.c`)
  - 键位映射
  - 键码转换
  - 特殊键处理

#### 2.3.2 通信中间件
- **无线管理** (`middleware/communication/wireless.c`)
  - 无线状态机
  - 连接管理
  - 重连机制

- **传输层** (`middleware/communication/transport.c`)
  - 协议切换
  - 驱动管理
  - 负载均衡

- **报告缓冲** (`middleware/communication/report_buffer.c`)
  - HID报告队列
  - 发送节流
  - 丢包重传

- **低功耗** (`middleware/communication/lpm.c`)
  - 功耗模式
  - 休眠唤醒
  - 空闲管理

### 2.4 应用层 (Application Layer)

**职责**: 业务逻辑和用户交互

**设计原则**:
- 调用中间件服务
- 实现产品特定功能
- 系统状态监控

**主要模块**:

#### 2.4.1 应用服务
- **输入服务** (`application/service/input_service.c`)
  - 输入任务管理
  - 矩阵扫描调度

- **输出服务** (`application/service/output_service.c`)
  - 输出任务管理
  - LED控制协调

- **通信服务** (`application/service/communication_service.c`)
  - 通信任务管理
  - 连接状态监控

- **系统服务** (`application/service/system_service.c`)
  - 系统监控
  - 故障恢复

#### 2.4.2 系统管理
- **系统初始化** (`application/system/system_init.c`)
  - 分层初始化协调
  - 状态跟踪
  - 依赖管理

---

## 3. 目录结构

```
keyboard-framework/
│
├── hal/                          # 硬件抽象层
│   ├── interface/                # HAL接口定义 (旧版)
│   ├── platforms/                # 平台特定实现
│   │   ├── ch584/                # WCH CH584平台
│   │   │   ├── _gpio.c           # GPIO实现
│   │   │   ├── _i2c_master.c     # I2C实现
│   │   │   ├── _uart.c           # UART实现
│   │   │   ├── _pwm.c            # PWM实现
│   │   │   ├── _adc.c            # ADC实现
│   │   │   ├── _spi_master.c     # SPI实现
│   │   │   ├── _i2c_slave.c      # I2C从设备实现
│   │   │   ├── pin_mapper.c      # 引脚映射表
│   │   │   └── _pin_defs.h       # 引脚定义
│   │   ├── nrf52/                # Nordic nRF52平台
│   │   │   ├── gpio_hal.h        # GPIO HAL
│   │   │   ├── i2c_hal.h         # I2C HAL
│   │   │   ├── uart_hal.h        # UART HAL
│   │   │   ├── pwm_hal.h         # PWM HAL
│   │   │   ├── adc_hal.h         # ADC HAL
│   │   │   ├── timer_hal.h       # 定时器 HAL
│   │   │   └── power_hal.h       # 电源 HAL
│   │   ├── pixart2860/           # PixArt 2860平台
│   │   │   ├── gpio_hal.h        # GPIO HAL
│   │   │   ├── i2c_hal.h         # I2C HAL
│   │   │   ├── uart_hal.h        # UART HAL
│   │   │   ├── pwm_hal.h         # PWM HAL
│   │   │   ├── adc_hal.h         # ADC HAL
│   │   │   ├── timer_hal.h       # 定时器 HAL
│   │   │   └── power_hal.h       # 电源 HAL
│   │   └── test/                 # 测试平台
│   │       ├── _gpio.c           # GPIO模拟实现
│   │       ├── _i2c_master.c     # I2C模拟实现
│   │       ├── _uart.c           # UART模拟实现
│   │       ├── _pwm.c            # PWM模拟实现
│   │       ├── _adc.c            # ADC模拟实现
│   │       ├── _spi_master.c     # SPI模拟实现
│   │       ├── _i2c_slave.c      # I2C从设备模拟实现
│   │       ├── pin_mapper.c      # 引脚映射表
│   │       └── _pin_defs.h       # 引脚定义
│   │
│   ├── gpio.h                    # GPIO接口定义
│   ├── i2c_master.h              # I2C主设备接口
│   ├── i2c_slave.h               # I2C从设备接口
│   ├── uart.h                    # UART接口
│   ├── pwm.h                     # PWM接口
│   ├── adc.h                     # ADC接口
│   ├── spi_master.h              # SPI主设备接口
│   ├── pin_defs.h                # 引脚类型定义
│   ├── pin_mapper.h              # 引脚映射接口
│   └── power.h                   # 电源管理接口
│
├── drivers/                      # 设备驱动层
│   ├── communication/            # 通信驱动
│   │   ├── bluetooth/            # 蓝牙驱动
│   │   │   └── bt_driver.c       # 蓝牙驱动实现
│   │   └── p2p4g/                # 2.4G无线驱动
│   │       └── p24g_driver.c     # 2.4G驱动实现
│   │
│   ├── input/                    # 输入设备驱动
│   │   ├── keyboard/             # 键盘相关
│   │   │   ├── matrix.c          # 矩阵扫描
│   │   │   ├── matrix.h          # 矩阵扫描接口
│   │   │   ├── debounce.c        # 防抖处理
│   │   │   └── debounce.h        # 防抖接口
│   │   └── touchpad/             # 触摸板驱动
│   │       └── touchpad.c        # 触摸板实现
│   │
│   ├── output/                   # 输出设备驱动
│   │   ├── backlight/            # 背光控制
│   │   │   ├── backlight.c       # 背光主逻辑
│   │   │   ├── backlight_pwm.c   # PWM背光
│   │   │   └── backlight.h       # 背光接口
│   │   ├── indicators/           # 指示灯
│   │   │   └── indicator.c       # 指示灯控制
│   │   └── leds/                 # LED控制
│   │       └── led_matrix.c      # LED矩阵
│   │
│   ├── power/                    # 电源管理驱动
│   │   └── battery.c             # 电池管理
│   │
│   ├── storage/                  # 存储驱动
│   │   ├── storage.c             # 存储主模块
│   │   ├── storage.h             # 存储接口
│   │   └── test/                 # 测试相关
│   │       └── eeprom.c          # EEPROM模拟
│   │
│   └── system/                   # 系统驱动
│       ├── timer.c               # 定时器驱动
│       ├── timer.h               # 定时器接口
│       ├── timer_manager.c       # 定时器管理
│       ├── timer_manager.h       # 定时器管理接口
│       ├── event_manager.c       # 事件管理实现
│       ├── event_manager.h       # 事件管理接口
│       ├── wait.h                # 等待接口
│       ├── atomic_util.h         # 原子操作工具
│       └── platforms/            # 平台特定驱动
│           ├── test/
│           │   ├── event_manager.c   # 事件管理测试实现
│           │   └── wait.c            # 等待测试实现
│           └── ch584/             # CH584平台驱动
│               └── event_manager.c   # 事件管理CH584实现
│
├── middleware/                   # 中间件层
│   ├── keyboard/                 # 键盘处理中间件
│   │   ├── action.c              # 动作执行
│   │   ├── action.h              # 动作接口
│   │   ├── action_layer.c        # 层管理
│   │   ├── action_layer.h        # 层管理接口
│   │   ├── action_util.c         # 动作工具
│   │   ├── action_util.h         # 动作工具接口
│   │   ├── action_code.h         # 动作码定义
│   │   ├── combo.c               # 组合键
│   │   ├── combo.h               # 组合键接口
│   │   ├── keyboard.c            # 键盘主逻辑
│   │   ├── keyboard.h            # 键盘接口
│   │   ├── keycode_config.c      # 键码配置
│   │   ├── keycode_config.h      # 键码配置接口
│   │   ├── keymap_common.c       # 键位映射
│   │   ├── keymap_common.h       # 键位映射接口
│   │   ├── keymap_introspection.c # 键位映射内省
│   │   ├── keymap_introspection.h # 键位映射内省接口
│   │   ├── keymap_extras/        # 键位映射扩展
│   │   │   ├── keymap_cn.h       # 中文键位
│   │   │   └── keymap_jp.h       # 日文键位
│   │   ├── custom_function.c     # 自定义功能
│   │   ├── custom_function.h     # 自定义功能接口
│   │   └── report.c              # 报告处理
│   │
│   └── communication/            # 通信中间件
│       ├── host.c                # 主机抽象
│       ├── host.h                # 主机接口
│       ├── host_driver.h         # 主机驱动接口
│       ├── lpm.c                 # 低功耗管理
│       ├── lpm.h                 # 低功耗接口
│       ├── report_buffer.c       # 报告缓冲
│       ├── report_buffer.h       # 报告缓冲接口
│       ├── transport.c           # 传输层
│       ├── transport.h           # 传输层接口
│       ├── wireless.c            # 无线管理
│       ├── wireless.h            # 无线管理接口
│       └── wireless_event_type.h # 无线事件类型
│
├── application/                  # 应用层
│   ├── service/                  # 应用服务
│   │   ├── input_service.c       # 输入服务
│   │   ├── input_service.h       # 输入服务接口
│   │   ├── output_service.c      # 输出服务
│   │   ├── output_service.h      # 输出服务接口
│   │   ├── communication_service.c # 通信服务
│   │   ├── communication_service.h # 通信服务接口
│   │   ├── system_service.c      # 系统服务
│   │   └── system_service.h      # 系统服务接口
│   │
│   ├── system/                   # 系统管理
│   │   ├── system_init.c         # 系统初始化协调器
│   │   └── system_init.h         # 系统初始化接口
│   │
│   ├── main.c                    # 主程序入口
│   ├── sys_config.h              # 系统配置
│   └── sys_error.h               # 系统错误定义
│
├── keyboards/                    # 产品配置
│   ├── product_config.h          # 产品特定配置
│   └── keymaps/                  # 键位映射表
│       ├── keymap_ansi.c         # ANSI布局
│       ├── keymap_iso.c          # ISO布局
│       └── keymap_jis.c          # JIS布局
│
├── utils/                        # 工具库
│   ├── bitwise.c                 # 位操作工具
│   ├── bitwise.h                 # 位操作接口
│   └── logging/                  # 日志系统
│       ├── debug.c               # 调试输出
│       ├── debug.h               # 调试接口
│       ├── print.c               # 打印实现
│       ├── print.h               # 打印接口
│       ├── sendchar.c            # 字符发送
│       └── sendchar.h            # 字符发送接口
│
├── test/                         # 测试相关
│   ├── test_main.c               # 测试主程序
│   ├── test_matrix.c             # 矩阵测试
│   └── unity/                    # Unity测试框架
│
├── DOCS/                         # 文档目录
│   ├── api/                      # API文档
│   ├── guides/                   # 使用指南
│   └── architecture/             # 架构文档
│
├── examples/                     # 示例代码
│   ├── basic_keyboard/           # 基础键盘示例
│   ├── wireless_keyboard/        # 无线键盘示例
│   └── gaming_keyboard/          # 游戏键盘示例
│
├── project/                      # 项目文件
│   └── ch584m/                   # CH584M项目
│       ├── .mrs/                 # MRS项目配置
│       └── StdPeriphDriver/      # CH584标准外设库
│           ├── inc/              # 头文件
│           └── src/              # 源文件
│
├── code_example/                 # 代码参考
│   └── qmk_firmware/             # QMK固件参考
│       ├── docs/                 # QMK文档
│       └── drivers/              # QMK驱动参考
│
├── CMakeLists.txt                # CMake构建配置
├── CLAUDE.md                     # Claude Code指南
└── ARCHITECTURE.md               # 本文档
```

---

## 4. 核心模块详解

### 4.1 矩阵扫描模块 (matrix.c)

**位置**: `drivers/input/keyboard/matrix.c`

**功能**:
- 扫描键盘矩阵，检测按键状态
- 支持多种二极管方向配置
- 与防抖系统集成

**核心函数**:
```c
// 初始化矩阵
void matrix_init(void);

// 扫描矩阵，返回是否有变化
uint8_t matrix_scan(void);

// 矩阵状态查询
bool matrix_is_on(uint8_t row, uint8_t col);
matrix_row_t matrix_get_row(uint8_t row);

// 调试输出
void matrix_print(void);
```

**工作流程**:

```mermaid
flowchart TD
    A[开始扫描] --> B[创建当前矩阵缓冲区]
    B --> C[初始化行移位器]
    C --> D[选择当前列]
    D --> E[读取所有行]
    E --> F[检测按键状态]
    F --> G[更新当前矩阵]
    G --> H[取消选择列]
    H --> I[下一列?]
    I -->|是| D
    I -->|否| J[比较原始矩阵]
    J --> K[有变化?]
    K -->|是| L[更新原始矩阵]
    L --> M[应用防抖]
    M --> N[返回变化标志]
    K -->|否| N
```

**配置参数**:
```c
// 矩阵尺寸
#define MATRIX_ROWS 8
#define MATRIX_COLS 16

// 引脚配置
#define MATRIX_ROW_PINS { A4,A5,A6,A0,A1,A8,A9,B9 }
#define MATRIX_COL_PINS { B5,B8,B17,B16,B15,B14,B13,B12,B3,B4,B2,A7,B7,B18,B1,B6 }

// 二极管方向
#define DIODE_DIRECTION ROW2COL  // 或 COL2ROW

// 输入状态
#define MATRIX_INPUT_PRESSED_STATE 0  // 0表示按下，1表示释放
```

### 4.2 防抖模块 (debounce.c)

**位置**: `drivers/input/keyboard/debounce.c`

**功能**:
- 过滤按键抖动信号
- 提供多种防抖算法
- 按键级独立跟踪

**防抖算法**:

#### 1. 对称延迟 (DEBOUNCE_SYM_DEFER_PK)
- 按下和释放都延迟
- 最稳定的防抖效果
- QMK默认算法

```mermaid
sequenceDiagram
    participant R as Raw Matrix
    participant D as Debounce
    participant C as Cooked Matrix

    R->>D: 按下事件
    D->>D: 记录时间戳
    D->>D: 等待DEBOUNCE_DELAY
    D->>C: 更新状态
    D->>D: 记录时间戳
```

#### 2. 对称急切 (DEBOUNCE_SYM_EAGER_PK)
- 按下立即响应
- 释放延迟处理
- 适合游戏场景

#### 3. 非对称 (DEBOUNCE_ASYM_EAGER_DEFER_PK)
- 按下无延迟
- 释放延迟
- 极致响应速度

**核心函数**:
```c
// 初始化防抖
void debounce_init(uint8_t num_rows);

// 主防抖处理
bool debounce(matrix_row_t raw[], matrix_row_t cooked[],
              uint8_t num_rows, bool changed);

// 查询防抖状态
const matrix_row_t* debounce_get_matrix(void);
bool debounce_changed(void);

// 重置防抖状态
void debounce_reset(void);
```

### 4.3 事件管理模块 (event_manager.c)

**位置**: `drivers/system/event_manager.c`

**功能**:
- 事件队列管理
- 任务调度
- 消息传递

**核心概念**:

```mermaid
classDiagram
    class Task {
        +task_id: uint8_t
        +event_handler: osal_task_event_handler_t
        +events: uint16_t
        +next: Task*
        +register_event()
        +process_events()
    }

    class EventQueue {
        +head: uint8_t
        +tail: uint8_t
        +queue: Event[16]
        +enqueue(event) bool
        +dequeue() Event
        +is_empty() bool
    }

    class Timer {
        +task_id: uint8_t
        +event: uint16_t
        +interval: uint32_t
        +reload: bool
        +remaining: uint32_t
        +start()
        +stop()
        +update()
    }

    class Event {
        +type: uint16_t
        +task_id: uint8_t
        +timestamp: uint32_t
        +data: void*
    }

    Task o--> Timer : manages
    EventQueue o--> Event : contains
    Task --> EventQueue : uses
```

**核心API**:
```c
// 任务管理
uint8_t OSAL_ProcessEventRegister(osal_task_event_handler_t eventCb);
error_code_t OSAL_MsgSend(uint8_t taskID, uint16_t event);
error_code_t OSAL_StartReloadTask(uint8_t taskID, uint16_t event, uint32_t time);
error_code_t OSAL_StopTask(uint8_t taskID, uint16_t event);
void OSAL_SystemProcess(void);
```

**使用示例**:
```c
// 1. 定义事件处理器
uint16_t keyboard_process_event(uint8_t task_id, uint16_t events) {
    if (events & MATRIX_SCAN_EVENT) {
        matrix_scan();
        return events ^ MATRIX_SCAN_EVENT;
    }
    return 0;
}

// 2. 注册任务
void keyboard_init(void) {
    keyboard_taskID = OSAL_ProcessEventRegister(keyboard_process_event);
    OSAL_StartReloadTask(keyboard_taskID, MATRIX_SCAN_EVENT, 5);
}

// 3. 触发事件
OSAL_MsgSend(keyboard_taskID, LED_BLINK_EVENT);
```

### 4.4 键盘处理模块 (keyboard.c)

**位置**: `middleware/keyboard/keyboard.c`

**功能**:
- 键盘主任务循环
- 事件处理协调
- 状态机管理

**工作流程**:

```mermaid
flowchart TD
    A[keyboard_task] --> B[处理队列事件]
    B --> C[matrix_scan触发?]
    C -->|是| D[扫描矩阵]
    D --> E[有变化?]
    E -->|是| F[遍历所有按键]
    F --> G[创建按键事件]
    G --> H[action_exec]
    H --> I[下一按键?]
    I -->|是| G
    I -->|否| J[发送报告]
    E -->|否| K[清理事件]
    C -->|否| K
    J --> K
    K --> L[处理其他事件]
    L --> B
```

### 4.5 动作执行模块 (action.c)

**位置**: `middleware/keyboard/action.c`

**功能**:
- 解析按键动作
- 执行具体操作
- 层切换处理
- 修饰键管理

**动作类型**:
```c
// 动作类型枚举
typedef enum {
    ACT_NONE,              // 无动作
    ACT_LAYER_TAP,         // 层+点击
    ACT_LAYER,             // 层切换
    ACT_MODS,              // 修饰键
    ACT_MODS_TAP,          // 修饰键+点击
    ACT_USAGE,             // 使用者按键(媒体键)
    ACT_MACRO,             // 宏
    ACT_COMMAND,           // 命令
    ACT_FUNCTION,          // 功能键
} action_kind_t;
```

**处理流程**:

```mermaid
flowchart TD
    A[按键事件] --> B[action_exec]
    B --> C[创建记录]
    C --> D[获取键位动作]
    D --> E[组合键处理?]
    E -->|是| F[process_combo]
    E -->|否| G[直接处理]
    F --> H[触发?]
    H -->|是| I[执行组合动作]
    H -->|否| J[处理点击/按住]
    G --> K[process_action_tapping]
    J --> K
    I --> L[发送键码]
    K --> M[判断动作类型]
    M --> N[ACT_LAYER_TAP?]
    M --> O[ACT_MODS_TAP?]
    M --> P[ACT_USAGE?]
    N -->|是| Q[处理层点击]
    O -->|是| R[处理修饰键点击]
    P -->|是| S[处理媒体键]
    N -->|否| T[处理普通键]
    O -->|否| T
    P -->|否| T
    Q --> T
    R --> T
    S --> T
    L --> U[结束]
    T --> U
```

### 4.6 无线管理模块 (wireless.c)

**位置**: `middleware/communication/wireless.c`

**功能**:
- 无线状态机管理
- 连接/断开处理
- 配对管理
- 报告发送

**状态机**:

```mermaid
stateDiagram-v2
    [*] --> WT_RESET: 系统复位
    WT_RESET --> WT_INITIALIZED: 初始化完成

    WT_INITIALIZED --> WT_DISCONNECTED: 准备就绪
    WT_DISCONNECTED --> WT_PARING: 开始配对
    WT_DISCONNECTED --> WT_RECONNECTING: 发起连接

    WT_PARING --> WT_CONNECTED: 配对成功
    WT_PARING --> WT_DISCONNECTED: 配对失败/超时
    WT_PARING --> WT_SUSPEND: 进入休眠

    WT_RECONNECTING --> WT_CONNECTED: 连接成功
    WT_RECONNECTING --> WT_DISCONNECTED: 连接失败

    WT_CONNECTED --> WT_DISCONNECTED: 连接断开
    WT_CONNECTED --> WT_SUSPEND: 休眠

    WT_SUSPEND --> WT_RECONNECTING: 唤醒
    WT_SUSPEND --> WT_DISCONNECTED: 唤醒后断连
```

**事件流程**:

```mermaid
sequenceDiagram
    participant App as 应用层
    participant WM as 无线管理
    participant BT as 蓝牙驱动
    participant IND as 指示灯

    App->>WM: wireless_connect()
    WM->>BT: bt_connect()
    BT->>WM: EVT_CONNECTED
    WM->>IND: 设置连接指示
    WM->>App: 连接成功回调

    Note over BT: 连接断开
    BT->>WM: EVT_DISCONNECTED
    WM->>IND: 设置断开指示
    WM->>App: 断开回调
```

---

## 5. 数据流程图

### 5.1 整体数据流

```mermaid
flowchart LR
    subgraph "硬件层"
        HW[键盘矩阵]
    end

    subgraph "HAL层"
        GPIO[GPIO驱动]
    end

    subgraph "驱动层"
        MTX[矩阵扫描]
        DBN[防抖]
    end

    subgraph "中间件层"
        KB[键盘处理]
        ACT[动作执行]
        RPT[报告生成]
    end

    subgraph "通信层"
        USB[USB驱动]
        BT[蓝牙驱动]
        P24G[2.4G驱动]
    end

    subgraph "主机"
        HOST[PC/手机]
    end

    HW --> GPIO
    GPIO --> MTX
    MTX --> DBN
    DBN --> KB
    KB --> ACT
    ACT --> RPT
    RPT --> USB
    RPT --> BT
    RPT --> P24G
    USB --> HOST
    BT --> HOST
    P24G --> HOST
```

### 5.2 按键处理数据流

```mermaid
flowchart TD
    A[硬件按键] --> B[GPIO中断]
    B --> C[标记扫描]
    C --> D[矩阵扫描任务]
    D --> E[读取硬件状态]
    E --> F[原始矩阵]
    F --> G[防抖处理]
    G --> H[去抖矩阵]
    H --> I[状态变化?]
    I -->|是| J[遍历按键]
    J --> K[生成按键事件]
    K --> L[动作执行]
    L --> M[层查询]
    M --> N[键码映射]
    N --> O[动作类型判断]
    O --> P[发送HID报告]
    O --> Q[层切换]
    O --> R[宏执行]
    P --> S[报告缓冲]
    Q --> T[更新层状态]
    R --> U[宏队列]
    S --> V[传输层]
    T --> J
    U --> P
    I -->|否| W[任务结束]
```

### 5.3 报告发送数据流

```mermaid
flowchart TD
    A[键盘动作] --> B[生成HID报告]
    B --> C[启用报告缓冲?]
    C -->|是| D[加入队列]
    C -->|否| E[直接发送]
    D --> F[队列处理任务]
    F --> G[队列空?]
    G -->|否| H[检查发送间隔]
    H --> I[间隔到?]
    I -->|是| J[出队]
    I -->|否| K[等待]
    J --> L[选择传输方式]
    L --> M[USB?]
    L --> N[蓝牙?]
    L --> O[2.4G?]
    M -->|是| P[USB发送]
    N -->|是| Q[蓝牙发送]
    O -->|是| R[2.4G发送]
    P --> S[发送成功?]
    Q --> S
    R --> S
    S -->|是| T[清除重试计数]
    S -->|否| U[重试计数+1]
    U --> V[达到最大重试?]
    V -->|否| W[重新入队]
    V -->|是| X[丢弃报告]
    T --> Y[发送下一条]
    W --> F
    X --> Y
    E --> Z[立即发送]
    Z --> Y
    G -->|是| AA[任务结束]
    K --> F
```

### 5.4 报告缓冲详细流程

```mermaid
flowchart TD
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
    L -->|MOUSE| P[wireless_transport.send_mouse]
    M --> Q[发送到蓝牙模块]
    N --> Q
    O --> Q
    P --> Q
    Q --> R[发送成功?]
    R -->|是| S[更新重试计数]
    R -->|否| T[重试计数+1]
    S --> U[继续发送下一条]
    T --> V{达到最大重试?}
    V -->|否| W[重新入队]
    V -->|是| X[丢弃报告]
    W --> E
    X --> U
    J --> E
    C --> Y[强制入队]
    Y --> E
    G --> Z[任务休眠]
    U --> AA{队列还有?}
    AA -->|是| H
    AA -->|否| G
```

---

## 6. 模块调用关系

### 6.1 初始化调用链

```mermaid
sequenceDiagram
    participant M as main()
    participant SI as system_init_coordinator()
    participant SH as system_setup_hal()
    participant SD as system_setup_drivers()
    participant SM as system_setup_middleware()
    participant SA as system_setup_application()
    participant IH as system_init_hal()
    participant ID as system_init_drivers()
    participant IM as system_init_middleware()
    participant IA as system_init_application()

    M->>SI: 系统初始化
    SI->>SH: HAL setup
    SH-->>SI: 完成
    SI->>SD: Driver setup
    SD-->>SI: 完成
    SI->>SM: Middleware setup
    SM-->>SI: 完成
    SI->>SA: Application setup
    SA-->>SI: 完成

    SI->>IH: HAL init
    IH-->>SI: 完成
    SI->>ID: Driver init
    ID-->>SI: 完成
    SI->>IM: Middleware init
    IM-->>SI: 完成
    SI->>IA: Application init
    IA-->>SI: 完成
```

### 6.2 按键处理调用链

```mermaid
sequenceDiagram
    participant App as 应用服务
    participant IS as 输入服务
    participant KB as 键盘任务
    participant MTX as 矩阵扫描
    participant DBN as 防抖
    participant ACT as 动作执行
    participant LYR as 层管理
    participant RPT as 报告生成
    participant WLS as 无线服务

    App->>IS: 输入服务初始化
    IS->>KB: 注册任务
    IS->>KB: 启动扫描任务

    loop 矩阵扫描 (5ms)
        KB->>MTX: matrix_scan()
        MTX->>DBN: debounce()
        DBN-->>MTX: 返回变化
        MTX-->>KB: 返回变化
        alt 有变化
            KB->>KB: 遍历按键
            KB->>ACT: action_exec()
            ACT->>LYR: layer_switch_get_action()
            LYR-->>ACT: 返回动作
            ACT->>ACT: process_combo()
            ACT->>ACT: process_action_tapping()
            ACT->>RPT: 发送报告
            RPT->>WLS: wireless_send_keyboard()
        end
    end
```

### 6.3 事件处理调用链

```mermaid
sequenceDiagram
    participant OS as OSAL系统
    participant T1 as 任务1
    participant T2 as 任务2
    participant T3 as 任务3

    Note over OS: 事件队列
    OS->>OS: 事件入队

    OS->>OS: 系统轮询
    OS->>OS: 获取下一个事件

    alt 事件属于任务1
        OS->>T1: process_event()
        T1->>T1: 处理事件
        T1-->>OS: 返回未处理事件
    else 事件属于任务2
        OS->>T2: process_event()
        T2->>T2: 处理事件
        T2-->>OS: 返回未处理事件
    else 事件属于任务3
        OS->>T3: process_event()
        T3->>T3: 处理事件
        T3-->>OS: 返回未处理事件
    end

    OS->>OS: 检查定时器
    OS->>OS: 触发超时事件
```

---

## 7. 初始化流程

### 7.1 系统初始化流程图

```mermaid
flowchart TD
    A[系统上电] --> B[芯片初始化]
    B --> C[执行system_setup阶段]
    C --> D[HAL setup]
    D --> E[绑定I2C引脚]
    E --> F[绑定UART引脚]
    F --> G[绑定ADC引脚]
    G --> H[初始化PWM]
    H --> I[Driver setup]
    I --> J[矩阵setup]
    J --> K[Middleware setup]
    K --> L[Application setup]
    L --> M[执行system_init阶段]
    M --> N[HAL init]
    N --> O[初始化I2C]
    O --> P[初始化UART]
    P --> Q[初始化PWM]
    Q --> R[Driver init]
    R --> S[初始化定时器]
    S --> T[初始化存储]
    T --> U[初始化电池]
    U --> V[初始化指示灯]
    V --> W[Middleware init]
    W --> X[初始化报告缓冲]
    X --> Y[初始化低功耗]
    Y --> Z[初始化无线]
    Z --> AA[初始化键盘]
    AA --> BB[Application init]
    BB --> CC[启动任务调度]
    CC --> DD[进入主循环]
```

### 7.2 分层初始化顺序

```mermaid
graph TD
    subgraph "阶段1: Setup (早期启动)"
        A1[HAL Setup<br/>- 绑定硬件引脚<br/>- 配置时钟] --> A2[Driver Setup<br/>- 初始化硬件无关设置<br/>- 准备数据结构]
        A2 --> A3[Middleware Setup<br/>- 分配内存<br/>- 初始化静态变量]
        A3 --> A4[Application Setup<br/>- 初始化应用服务<br/>- 注册任务]
    end

    subgraph "阶段2: Init (主机协议初始化后)"
        B1[HAL Init<br/>- 初始化外设<br/>- 配置通信接口] --> B2[Driver Init<br/>- 初始化设备<br/>- 启动定时器]
        B2 --> B3[Middleware Init<br/>- 初始化协议栈<br/>- 启动状态机]
        B3 --> B4[Application Init<br/>- 启动应用任务<br/>- 注册回调]
    end

    A4 --> B1
```

---

## 8. 按键处理流程

### 8.1 完整按键流程

```mermaid
flowchart TD
    subgraph "1. 硬件层"
        A1[按键按下]
        A2[硬件电路导通]
        A3[GPIO电平变化]
    end

    subgraph "2. 扫描层"
        B1[矩阵扫描任务触发]
        B2[选择列]
        B3[读取行数据]
        B4[更新原始矩阵]
    end

    subgraph "3. 防抖层"
        C1[防抖处理]
        C2[时间戳检查]
        C3[状态稳定?]
        C3 -->|否| C4[等待]
        C4 --> C2
        C3 -->|是| C5[更新去抖矩阵]
    end

    subgraph "4. 键盘处理层"
        D1[检测状态变化]
        D2[生成按键事件]
        D3[遍历变化的按键]
    end

    subgraph "5. 动作执行层"
        E1[获取键位动作]
        E2[组合键检查]
        E3[触发组合键?]
        E3 -->|是| E4[执行组合动作]
        E3 -->|否| E5[层切换检查]
        E5 --> E6[Tap/Hold判断]
        E6 --> E7[执行具体动作]
    end

    subgraph "6. 报告层"
        F1[生成HID报告]
        F2[加入报告队列]
        F3[传输层处理]
        F4[发送到主机]
    end

    A1 --> A2 --> A3
    A3 --> B1 --> B2 --> B3 --> B4
    B4 --> C1 --> C2 --> C3
    C5 --> D1 --> D2 --> D3
    D3 --> E1 --> E2 --> E3
    E4 --> F1
    E5 --> E6 --> E7 --> F1
    F1 --> F2 --> F3 --> F4
```

### 8.2 组合键处理流程

```mermaid
flowchart TD
    A[按键事件] --> B[获取键码]
    B --> C[查找组合键表]
    C --> D[匹配组合?]
    D -->|否| E[普通按键处理]
    D -->|是| F[记录按键]
    F --> G[检查时间窗口]
    G --> H[超时?]
    H -->|是| I[清除组合键缓冲]
    I --> E
    H -->|否| J[检查所有按键按下]
    J --> K[组合键完整?]
    K -->|否| L[等待更多按键]
    K -->|是| M[检查修饰键]
    M --> N[修饰键匹配?]
    N -->|否| O[触发普通按键]
    N -->|是| P[执行组合动作]
    P --> Q[清除缓冲]
    Q --> R[结束]
    L --> S[继续监控]
    S --> G
    E --> R
    O --> R
```

---

## 9. 通信流程

### 9.1 三模式切换流程

```mermaid
flowchart TD
    A[set_transport] --> B{目标模式}
    B -->|USB| C[usb_transport_enable]
    B -->|蓝牙| D[bt_transport_enable]
    B -->|2.4G| E[p24g_transport_enable]

    C --> F[切换到USB驱动]
    F --> G[断开无线连接]
    G --> H[停止低功耗定时器]
    H --> I[USB模式激活]
    I --> J[结束]

    D --> K[切换到无线驱动]
    K --> L[断开当前连接]
    L --> M[延时50ms]
    M --> N[连接蓝牙]
    N --> O[重置低功耗定时器]
    O --> P[蓝牙模式激活]
    P --> J

    E --> Q[切换到无线驱动]
    Q --> R[断开蓝牙连接]
    R --> S[延时50ms]
    S --> T[连接2.4G]
    T --> U[重置低功耗定时器]
    U --> V[2.4G模式激活]
    V --> J
```

### 9.2 蓝牙配对流程

```mermaid
sequenceDiagram
    participant U as 用户
    participant K as 键盘
    participant BT as 蓝牙模块
    participant H as 主机

    U->>K: 按下配对键
    K->>BT: 发起配对请求
    BT->>K: 返回配对模式
    K->>K: 设置配对指示灯

    BT->>H: 广播可发现
    H->>BT: 扫描设备
    H->>BT: 选择键盘
    H->>BT: 发起配对

    BT->>K: 发送配对请求
    K->>K: 生成PIN码
    K->>H: 显示PIN码
    H->>BT: 输入PIN码
    BT->>K: 验证PIN码
    K->>BT: 确认配对

    BT->>K: 配对成功
    K->>K: 清除配对指示
    K->>K: 保存配对信息
    K->>K: 切换到已连接状态

    BT->>H: 建立连接
    H->>K: HID连接建立
    K->>K: 启用NKRO
```

### 9.3 报告发送流程

```mermaid
flowchart TD
    A[按键事件] --> B[生成报告]
    B --> C{无线状态}
    C -->|未连接| D[尝试重连]
    C -->|配对中| E{处于PIN输入?}
    C -->|已连接| F[允许发送]

    D --> G[启动重连定时器]
    G --> H[延时后重试]
    H --> A

    E -->|是| F
    E -->|否| I[丢弃报告]
    I --> J[结束]

    F --> K{启用报告缓冲?}
    K -->|是| L[入队]
    K -->|否| M[立即发送]

    L --> N[队列管理任务]
    N --> O[队列空?]
    O -->|是| P[等待]
    P --> N
    O -->|否| Q[检查发送间隔]
    Q --> R[间隔到?]
    R -->|否| S[等待]
    S --> N
    R -->|是| T[出队]
    T --> U[选择传输方式]
    U --> V[USB/蓝牙/2.4G]
    V --> W[发送报告]
    W --> X[发送成功?]
    X -->|是| Y[清除重试计数]
    X -->|否| Z[重试计数+1]
    Z --> AA[达到最大重试?]
    AA -->|否| BB[重新入队]
    AA -->|是| CC[丢弃报告]
    BB --> N
    CC --> Y
    Y --> DD[继续发送]
    DD --> N

    M --> EE[调用传输回调]
    EE --> FF[驱动发送]
    FF --> DD
```

---

## 10. 配置系统

### 10.1 配置层次结构

```mermaid
graph TD
    subgraph "产品配置 keyboards/product_config.h"
        A1[矩阵配置<br/>- 行数/列数<br/>- 引脚定义<br/>- 二极管方向]
        A2[USB配置<br/>- 设备描述符<br/>- 端点配置<br/>- 功耗设置]
        A3[功能配置<br/>- 组合键<br/>- Tap/Dance<br/>- 层配置]
    end

    subgraph "系统配置 application/sys_config.h"
        B1[芯片配置<br/>- 芯片型号<br/>- 时钟配置<br/>- 调试设置]
        B2[通信配置<br/>- USB开关<br/>- 蓝牙开关<br/>- 无线配置]
        B3[调试配置<br/>- 日志级别<br/>- 串口配置<br/>- 打印开关]
    end

    subgraph "驱动配置 drivers/*/*.h"
        C1[矩阵扫描<br/>- 扫描间隔<br/>- 输出延迟]
        C2[防抖<br/>- 防抖算法<br/>- 防抖延迟]
        C3[定时器<br/>- 系统时钟<br/>- 计数器宽度]
    end
```

### 10.2 配置继承关系

```mermaid
flowchart TD
    A[默认配置] --> B[驱动层配置]
    B --> C[产品层配置]
    C --> D[系统配置]
    D --> E[运行时配置]

    F[预处理器宏] --> B
    G[编译时定义] --> C
    H[EEPROM配置] --> E
    I[运行时API] --> E
```

### 10.3 关键配置项

#### 矩阵配置
```c
// 矩阵尺寸
#define MATRIX_ROWS 8
#define MATRIX_COLS 16

// 引脚定义 (使用宏定义引脚)
#define MATRIX_ROW_PINS { A4,A5,A6,A0,A1,A8,A9,B9 }
#define MATRIX_COL_PINS { B5,B8,B17,B16,B15,B14,B13,B12,B3,B4,B2,A7,B7,B18,B1,B6 }

// 二极管方向
#define DIODE_DIRECTION ROW2COL  // 或 COL2ROW

// 输入配置
#define MATRIX_INPUT_PRESSED_STATE 0  // 0=按下为低电平, 1=按下为高电平

// 扫描配置
#define MATRIX_SCAN_TIMER 5  // 扫描间隔 (ms)
#define MATRIX_IO_DELAY 30   // IO延迟 (us)
```

#### 防抖配置
```c
// 防抖算法选择
#define DEBOUNCE_ALGORITHM DEBOUNCE_SYM_DEFER_PK
// 可选值:
// - DEBOUNCE_NONE: 无防抖
// - DEBOUNCE_SYM_DEFER_PK: 对称延迟
// - DEBOUNCE_SYM_EAGER_PK: 对称急切
// - DEBOUNCE_ASYM_EAGER_DEFER_PK: 非对称

// 防抖延迟
#define DEBOUNCE_DELAY 5  // ms

// 最大行数
#define MAX_MATRIX_ROWS 16
```

#### 通信配置
```c
// USB配置
#define USB_ENABLE TRUE
#define USB_MAX_POWER_CONSUMPTION 500  // mA
#define USB_POLLING_INTERVAL_MS 10     // 轮询间隔

// 蓝牙配置
#define BLE_ENABLE TRUE
#define BLE_NAME "CH584M_BLE"
#define BLE_ADVERTISE_INTERVAL 1000    // 广播间隔 (ms)

// 2.4G配置
#define P2P4G_ENABLE TRUE
#define P2P4G_HOST_DEVICES_COUNT 1     // 支持的主机数量

// 传输模式开关
#define USB_ENABLE_FLAG FALSE
#define BLUETOOTH_ENABLE_FLAG TRUE
#define P2P4G_ENABLE_FLAG FALSE
```

#### 高级功能配置
```c
// Tap配置
#define TAPPING_TERM 200      // 点击判定时间 (ms)
#define TAPPING_TOGGLE 2      // 点击次数切换
#define QUICK_TAP_TERM 100    // 快速点击时间 (ms)

// Oneshot配置
#define ONESHOT_TIMEOUT 300   // 单次触发超时 (ms)
#define ONESHOT_TAP_TOGGLE 2  // 点击次数切换

// Combo配置
#define COMBO_TERM 200        // 组合键判定时间 (ms)
#define COMBO_MOD_TERM 200    // 组合修饰键时间 (ms)

// Leader配置
#define LEADER_TIMEOUT 300    // Leader键超时 (ms)
```

---

## 11. 平台适配

### 11.1 支持的平台

| 平台 | 芯片 | 特点 | 状态 |
|------|------|------|------|
| CH584 | WCH CH584M | 蓝牙5.0, USB, 低成本 | ✅ 已实现 |
| nRF52 | Nordic nRF52832/40 | 多协议, 强大BLE | 🔄 部分实现 |
| PixArt2860 | PixArt PAN2860 | 2.4G专用 | 🔄 部分实现 |
| Test | 仿真平台 | 调试测试 | ✅ 已实现 |

### 11.2 HAL层平台适配

```mermaid
graph TD
    subgraph "HAL接口层"
        H1[GPIO接口<br/>gpio.h]
        H2[I2C接口<br/>i2c_master.h]
        H3[UART接口<br/>uart.h]
        H4[PWM接口<br/>pwm.h]
        H5[ADC接口<br/>adc.h]
    end

    subgraph "平台实现层"
        P1[CH584平台]
        P2[nRF52平台]
        P3[PixArt2860平台]
        P4[Test平台]
    end

    P1 --> P1A[_gpio.c]
    P1 --> P1B[_i2c_master.c]
    P1 --> P1C[_uart.c]
    P1 --> P1D[_pwm.c]
    P1 --> P1E[_adc.c]

    P2 --> P2A[gpio_hal.h]
    P2 --> P2B[i2c_hal.h]
    P2 --> P2C[uart_hal.h]
    P2 --> P2D[pwm_hal.h]
    P2 --> P2E[adc_hal.h]

    P3 --> P3A[gpio_hal.h]
    P3 --> P3B[i2c_hal.h]
    P3 --> P3C[uart_hal.h]
    P3 --> P3D[pwm_hal.h]
    P3 --> P3E[adc_hal.h]

    P4 --> P4A[_gpio.c]
    P4 --> P4B[_i2c_master.c]
    P4 --> P4C[_uart.c]
    P4 --> P4D[_pwm.c]
    P4 --> P4E[_adc.c]

    H1 --> P1A
    H1 --> P2A
    H1 --> P3A
    H1 --> P4A

    H2 --> P1B
    H2 --> P2B
    H2 --> P3B
    H2 --> P4B
```

### 11.3 平台选择机制

```c
// 在sys_config.h中选择芯片
#define CHIP_TYPE CHIP_CH584M
// 或
// #define CHIP_TYPE CHIP_PAR2860

// 在CMakeLists.txt中根据芯片选择平台
if(CHIP_TYPE STREQUAL "CH584M")
    set(PLATFORM_DIR "ch584")
elseif(CHIP_TYPE STREQUAL "PAR2860")
    set(PLATFORM_DIR "pixart2860")
endif()

// 包含平台特定目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/hal/platforms/${PLATFORM_DIR}
)
```

### 11.4 引脚映射机制

```mermaid
flowchart TD
    A[产品配置<br/>MATRIX_ROW_PINS] --> B[标准引脚编号<br/>A4, A5, B9...]
    B --> C[引脚映射函数<br/>GET_GPIO_PORT<br/>GET_GPIO_PIN]
    C --> D[HAL层实现<br/>platform_specific_gpio.c]
    D --> E[芯片寄存器操作<br/>GPIOA_ModeCfg<br/>GPIOB_SetBits]
```

---

## 总结

本键盘框架采用分层架构设计，具有以下特点：

1. **清晰的分层**: HAL → Driver → Middleware → Application，职责明确
2. **平台无关**: 通过HAL层抽象硬件差异，支持多平台
3. **事件驱动**: 采用事件队列和任务调度，响应及时
4. **模块化**: 每个模块职责单一，易于测试和维护
5. **可配置**: 通过配置文件适配不同产品和需求
6. **功能完整**: 涵盖输入、处理、通信、输出全流程

框架设计参考了QMK固件的架构思想，但在模块化、平台抽象、事件管理等方面进行了优化和增强，是一个现代化的嵌入式键盘框架解决方案。

---

*本文档版本: v1.0*
*最后更新: 2025-12-20*
