# Keyboard Framework - 项目架构文档

## 项目概述

这是一个**模块化键盘框架**，专为多模无线键盘和触摸板设计。采用分层架构，支持多种 MCU 平台（PixArt 2860、WCH CH584、Nordic nRF52 系列）。框架设计灵感来源于 QMK 固件的按键处理流程，通过 HAL（硬件抽象层）、驱动层、中间件层和应用层实现清晰的关注点分离。

## 项目目录结构

```
keyboard-framework/
├── hal/                        # 硬件抽象层（Hardware Abstraction Layer）
├── drivers/                    # 设备驱动层
├── middleware/                 # 中间件层
├── application/                # 应用层
├── keyboards/                  # 产品配置
├── component/                  # 组件库
├── utils/                      # 通用工具
├── test/                       # 测试文件
├── unity/                      # Unity 测试框架
├── examples/                   # 示例代码
├── lib/                        # 第三方库
├── DOCS/                       # 文档
├── memory-bank/                # 知识库
├── code_example/               # 代码示例（不纳入框架说明）
├── project/                    # 芯片 SDK（不纳入框架说明）
├── CMakeLists.txt              # CMake 构建配置
├── CLAUDE.md                   # Claude Code 项目说明
├── ARCHITECTURE.md             # 架构文档
└── README.MD                   # 项目说明
```

---

## 一、HAL 层（Hardware Abstraction Layer）

HAL 层提供跨平台的硬件抽象接口，使上层代码无需关心具体硬件实现。

### 目录结构
```
hal/
├── interface/                  # HAL 抽象接口定义
│   ├── gpio.h                 # GPIO 接口
│   ├── adc.h                  # ADC 接口
│   ├── pwm.h                  # PWM 接口
│   ├── i2c_master.h           # I2C 主机接口
│   ├── i2c_slave.h            # I2C 从机接口
│   ├── spi_master.h           # SPI 主机接口
│   ├── uart.h                 # UART 接口
│   ├── power.h                # 电源管理接口
│   ├── pin_defs.h             # 引脚定义
│   └── pin_mapper.h           # 引脚映射
└── platforms/                  # 平台特定实现
    ├── ch584/                 # WCH CH584 平台
    │   ├── _gpio.c           # GPIO 实现
    │   ├── _adc.c            # ADC 实现
    │   ├── _pwm.c            # PWM 实现
    │   ├── _i2c_master.c     # I2C 主机实现
    │   ├── _i2c_slave.c      # I2C 从机实现
    │   ├── _spi_master.c     # SPI 主机实现
    │   ├── _uart.c           # UART 实现
    │   ├── _pin_defs.h       # CH584 引脚定义
    │   └── pin_mapper.c      # CH584 引脚映射
    ├── nrf52/                 # Nordic nRF52 平台
    │   ├── gpio_hal.h        # GPIO HAL
    │   ├── adc_hal.h         # ADC HAL
    │   ├── pwm_hal.h         # PWM HAL
    │   ├── i2c_hal.h         # I2C HAL
    │   ├── uart_hal.h        # UART HAL
    │   ├── timer_hal.h       # Timer HAL
    │   └── power_hal.h       # Power HAL
    ├── pixart2860/            # PixArt 2860 平台
    │   ├── gpio_hal.h        # GPIO HAL
    │   ├── adc_hal.h         # ADC HAL
    │   ├── pwm_hal.h         # PWM HAL
    │   ├── i2c_hal.h         # I2C HAL
    │   ├── uart_hal.h        # UART HAL
    │   ├── timer_hal.h       # Timer HAL
    │   └── power_hal.h       # Power HAL
    └── test/                  # 测试平台（用于单元测试）
        ├── _gpio.c           # GPIO Mock 实现
        ├── _adc.c            # ADC Mock 实现
        ├── _pwm.c            # PWM Mock 实现
        ├── _i2c_master.c     # I2C 主机 Mock
        ├── _i2c_slave.c      # I2C 从机 Mock
        ├── _spi_master.c     # SPI 主机 Mock
        ├── _uart.c           # UART Mock
        ├── _pin_defs.h       # 测试引脚定义
        └── pin_mapper.c      # 测试引脚映射
```

### 核心文件说明

#### 接口层

| 文件 | 作用 |
|------|------|
| `gpio.h` | GPIO 通用接口，提供引脚输入输出、电平读写、模式配置等功能 |
| `adc.h` | ADC 接口，提供模拟信号采集功能 |
| `pwm.h` | PWM 接口，提供 PWM 波形生成功能 |
| `i2c_master.h` | I2C 主机接口，提供 I2C 通信功能 |
| `i2c_slave.h` | I2C 从机接口 |
| `spi_master.h` | SPI 主机接口 |
| `uart.h` | UART 串口接口，提供串口通信功能 |
| `power.h` | 电源管理接口 |
| `pin_defs.h` | 引脚定义和宏 |
| `pin_mapper.h` | 引脚映射工具 |

#### 平台实现

每个平台目录包含该平台的具体硬件实现：

- **ch584/**: 沁恒 CH584 芯片的完整实现
- **nrf52/**: Nordic nRF52 系列芯片的 HAL 头文件
- **pixart2860/**: PixArt 2860 芯片的 HAL 头文件
- **test/**: 用于单元测试的 Mock 实现

---

## 二、驱动层（Drivers）

驱动层实现各类硬件设备的驱动程序，基于 HAL 层接口实现跨平台兼容。

### 目录结构
```
drivers/
├── communication/              # 通信驱动
│   ├── bluetooth/             # 蓝牙驱动
│   │   ├── bt_driver.h       # 蓝牙驱动接口
│   │   └── ch584/            # CH584 蓝牙实现
│   │       ├── _bt_driver.c  # CH584 蓝牙驱动实现
│   │       ├── _bt_driver.h  # CH584 蓝牙驱动头文件
│   │       ├── hidkbd.c      # HID 键盘实现
│   │       └── hidkbd.h      # HID 键盘头文件
│   └── p2p4g/                 # 2.4G 无线驱动
│       ├── p24g_driver.c     # 2.4G 驱动实现
│       └── p24g_driver.h     # 2.4G 驱动头文件
├── input/                      # 输入设备驱动
│   ├── keyboard/              # 键盘驱动
│   │   ├── matrix.c          # 矩阵扫描核心实现
│   │   ├── matrix.h          # 矩阵扫描接口
│   │   ├── debounce.c        # 防抖算法实现
│   │   └── debounce.h        # 防抖接口
│   └── touchpad/              # 触摸板驱动
│       └── pixart_driver.h   # PixArt 触摸板驱动
├── output/                     # 输出设备驱动
│   ├── backlight/             # 背光驱动
│   │   ├── backlight.c       # 背光核心逻辑
│   │   ├── backlight.h       # 背光接口
│   │   └── backlight_pwm.c   # PWM 背光实现
│   ├── indicators/            # 指示灯驱动
│   │   ├── indicator.c       # 指示灯实现
│   │   └── indicator.h       # 指示灯接口
│   ├── leds/                  # LED 驱动
│   │   ├── led_driver.h      # LED 驱动接口
│   │   ├── led_pwm.h         # PWM LED 驱动
│   │   └── led_timer.h       # Timer LED 驱动
│   └── logging/               # 日志输出
│       ├── debug.c           # 调试输出实现
│       ├── debug.h           # 调试输出接口
│       ├── print.c           # 打印函数实现
│       ├── print.h           # 打印函数接口
│       ├── sendchar.c        # 字符发送实现
│       └── sendchar.h        # 字符发送接口
├── power/                      # 电源管理驱动
│   ├── battery.c              # 电池管理实现
│   └── battery.h              # 电池管理接口
├── storage/                    # 存储驱动
│   ├── eeprom.h               # EEPROM 接口
│   ├── storage.c              # 存储管理实现
│   ├── storage.h              # 存储管理接口
│   └── test/                  # 测试实现
│       └── eeprom.c          # EEPROM Mock 实现
└── system/                     # 系统驱动
    ├── timer.c                # 定时器实现
    ├── timer.h                # 定时器接口
    ├── timer_manager.c        # 定时器管理器
    ├── timer_manager.h        # 定时器管理器接口
    ├── wait.h                 # 延时等待接口
    ├── event_manager.h        # 事件管理器接口
    ├── atomic_util.h          # 原子操作工具
    └── platforms/             # 平台特定实现
        ├── ch584/
        │   └── event_manager.c
        └── test/
            ├── event_manager.c
            └── wait.c
```

### 核心模块说明

#### 1. 通信驱动（communication/）

| 文件/目录 | 作用 |
|-----------|------|
| `bluetooth/bt_driver.h` | 蓝牙驱动统一接口 |
| `bluetooth/ch584/` | CH584 蓝牙驱动实现（包含 HID 键盘支持） |
| `p2p4g/p24g_driver.*` | 2.4G 无线通信驱动 |

#### 2. 输入驱动（input/）

| 文件 | 作用 |
|------|------|
| `keyboard/matrix.c` | 矩阵扫描核心，支持 COL2ROW 和 ROW2COL 两种扫描方向 |
| `keyboard/matrix.h` | 矩阵扫描接口定义 |
| `keyboard/debounce.c` | 防抖算法实现（支持多种算法：sym_defer_pk, sym_eager_pk, asym_eager_defer_pk） |
| `keyboard/debounce.h` | 防抖接口 |
| `touchpad/pixart_driver.h` | PixArt 触摸板驱动接口 |

#### 3. 输出驱动（output/）

| 文件/目录 | 作用 |
|-----------|------|
| `backlight/` | 背光控制驱动（支持 PWM 调光） |
| `indicators/` | 状态指示灯驱动（用于显示连接状态、电量等） |
| `leds/` | LED 驱动（支持 PWM 和 Timer 方式） |
| `logging/` | 日志输出系统（debug、print、sendchar） |

#### 4. 电源驱动（power/）

| 文件 | 作用 |
|------|------|
| `battery.c/h` | 电池电量监测和管理 |

#### 5. 存储驱动（storage/）

| 文件 | 作用 |
|------|------|
| `storage.c/h` | 存储管理模块，支持系统配置和用户配置的持久化 |
| `eeprom.h` | EEPROM 抽象接口 |
| `test/eeprom.c` | EEPROM Mock 实现（用于测试） |

#### 6. 系统驱动（system/）

| 文件 | 作用 |
|------|------|
| `timer.c/h` | 系统定时器（用于时间戳、延时等） |
| `timer_manager.c/h` | 定时器管理器（支持多个定时器任务） |
| `wait.h` | 延时等待工具 |
| `event_manager.h` | 事件管理器接口 |
| `atomic_util.h` | 原子操作工具（用于多线程安全） |

---

## 三、中间件层（Middleware）

中间件层实现业务逻辑和协议处理，连接驱动层和应用层。

### 目录结构
```
middleware/
├── communication/              # 通信中间件
│   ├── host.c                 # 主机通信核心
│   ├── host.h                 # 主机通信接口
│   ├── host_driver.h          # 主机驱动接口
│   ├── lpm.c                  # 低功耗管理实现
│   ├── lpm.h                  # 低功耗管理接口
│   ├── report_buffer.c        # HID 报告缓冲实现
│   ├── report_buffer.h        # HID 报告缓冲接口
│   ├── transport.c            # 传输层管理（USB/BLE/2.4G 切换）
│   ├── transport.h            # 传输层接口
│   ├── wireless.c             # 无线通信核心
│   ├── wireless.h             # 无线通信接口
│   └── wireless_event_type.h  # 无线事件类型定义
└── keyboard/                   # 键盘中间件
    ├── action.c               # 按键动作处理
    ├── action.h               # 按键动作接口
    ├── action_code.h          # 动作代码定义
    ├── action_layer.c         # 层级管理实现
    ├── action_layer.h         # 层级管理接口
    ├── action_util.c          # 动作工具函数
    ├── action_util.h          # 动作工具接口
    ├── combo.c                # 组合键实现
    ├── combo.h                # 组合键接口
    ├── custom_function.c      # 自定义功能实现
    ├── custom_function.h      # 自定义功能接口
    ├── keyboard.c             # 键盘核心逻辑
    ├── keyboard.h             # 键盘接口
    ├── keycode.h              # 键码定义
    ├── keycode_config.c       # 键码配置实现
    ├── keycode_config.h       # 键码配置接口
    ├── keycodes.h             # QMK 兼容键码
    ├── keymap_common.c        # 键映射通用函数
    ├── keymap_common.h        # 键映射通用接口
    ├── keymap_introspection.c # 键映射内省
    ├── keymap_introspection.h # 键映射内省接口
    ├── modifiers.h            # 修饰键定义
    ├── quantum_keycodes.h     # Quantum 键码
    ├── quantum_keycodes_legacy.h # 旧版 Quantum 键码
    ├── report.c               # HID 报告生成
    ├── report.h               # HID 报告接口
    └── keymap_extras/         # 多语言键盘布局（100+ 个布局文件）
        ├── keymap_us.h        # 美式布局
        ├── keymap_german.h    # 德语布局
        ├── keymap_french.h    # 法语布局
        ├── keymap_japanese.h  # 日语布局
        └── ... (更多布局)
```

### 核心模块说明

#### 1. 通信中间件（communication/）

| 文件 | 作用 |
|------|------|
| `host.c/h` | 主机通信核心，处理 HID 报告发送 |
| `host_driver.h` | 主机驱动接口（USB/BLE/2.4G 抽象） |
| `lpm.c/h` | 低功耗管理（Low Power Management），控制休眠和唤醒 |
| `report_buffer.c/h` | HID 报告缓冲队列，避免丢失报告 |
| `transport.c/h` | 传输模式管理（USB/蓝牙/2.4G 三模切换） |
| `wireless.c/h` | 无线通信状态机（连接、配对、断开等） |
| `wireless_event_type.h` | 无线事件类型定义 |

#### 2. 键盘中间件（keyboard/）

| 文件/目录 | 作用 |
|-----------|------|
| `keyboard.c/h` | 键盘核心逻辑，处理按键事件和任务循环 |
| `action.c/h` | 按键动作处理（普通键、修饰键、层切换等） |
| `action_code.h` | 动作代码定义 |
| `action_layer.c/h` | 层级管理（支持多层键映射） |
| `action_util.c/h` | 动作处理工具函数 |
| `combo.c/h` | 组合键功能（多键同时按下触发特定动作） |
| `custom_function.c/h` | 自定义功能（用户自定义按键行为） |
| `keycode.h` | 键码定义 |
| `keycode_config.c/h` | 键码配置 |
| `keycodes.h` | QMK 兼容键码 |
| `keymap_common.c/h` | 键映射通用函数 |
| `keymap_introspection.c/h` | 键映射内省（运行时查询键映射） |
| `modifiers.h` | 修饰键定义（Ctrl、Shift、Alt 等） |
| `quantum_keycodes.h` | Quantum 键码（QMK 高级功能） |
| `report.c/h` | HID 报告生成 |
| `keymap_extras/` | 100+ 种语言和布局的键盘映射 |

---

## 四、应用层（Application）

应用层实现系统初始化、服务协调和主程序入口。

### 目录结构
```
application/
├── main.c                      # 主程序入口（包含测试代码）
├── sys_config.h                # 系统配置（芯片类型、调试级别、功能开关）
├── sys_error.h                 # 系统错误定义
├── product/                    # 产品配置（预留）
├── service/                    # 应用服务层
│   ├── communication_service.c # 通信服务实现
│   ├── communication_service.h # 通信服务接口
│   ├── input_service.c        # 输入服务实现
│   ├── input_service.h        # 输入服务接口
│   ├── output_service.c       # 输出服务实现
│   ├── output_service.h       # 输出服务接口
│   ├── system_service.c       # 系统服务实现
│   └── system_service.h       # 系统服务接口
└── system/                     # 系统初始化
    ├── system_init.c          # 系统初始化实现
    └── system_init.h          # 系统初始化接口
```

### 核心文件说明

| 文件/目录 | 作用 |
|-----------|------|
| `main.c` | 主程序入口，包含 Unity 测试框架的测试用例 |
| `sys_config.h` | 系统配置文件（芯片类型、USB/BLE 开关、日志级别等） |
| `sys_error.h` | 系统错误代码定义 |
| `service/communication_service.*` | 通信服务（管理 USB/BLE/2.4G 通信） |
| `service/input_service.*` | 输入服务（管理键盘、触摸板输入） |
| `service/output_service.*` | 输出服务（管理背光、指示灯、日志输出） |
| `service/system_service.*` | 系统服务（管理定时器、事件、电源） |
| `system/system_init.*` | 系统初始化协调器（按 HAL → Driver → Middleware → Application 顺序初始化） |

---

## 五、产品配置（Keyboards）

产品配置目录定义具体产品的硬件参数和键盘布局。

### 目录结构
```
keyboards/
├── product_config.h            # 产品硬件配置
└── keymaps/                    # 键映射定义
    ├── default_keymap.h       # 默认键映射
    ├── layout_8x16.h          # 8x16 矩阵布局
    └── layout_jis_113.h       # JIS 113 键布局
```

### 文件说明

| 文件 | 作用 |
|------|------|
| `product_config.h` | 产品配置文件（矩阵尺寸、引脚定义、USB/BLE 参数、功能开关） |
| `keymaps/default_keymap.h` | 默认键映射 |
| `keymaps/layout_8x16.h` | 8x16 矩阵布局定义 |
| `keymaps/layout_jis_113.h` | JIS 113 键布局定义 |

---

## 六、组件库（Component）

组件库提供可复用的高级功能模块。

### 目录结构
```
component/
├── include/                    # 组件头文件
│   ├── bayes_filtering.h      # 贝叶斯滤波算法
│   ├── mcs_gesture.h          # 手势识别
│   ├── pct1336_driver.h       # PCT1336 触摸芯片驱动
│   └── touchpad_service.h     # 触摸板服务
├── bayes_filtering.c          # 贝叶斯滤波实现
├── mcs_gesture.c              # 手势识别实现
├── pct1336_driver.c           # PCT1336 驱动实现
└── touchpad_service.c         # 触摸板服务实现
```

### 文件说明

| 文件 | 作用 |
|------|------|
| `bayes_filtering.c/h` | 贝叶斯滤波算法（用于触摸板数据滤波） |
| `mcs_gesture.c/h` | 多点触控手势识别（滑动、缩放、旋转等） |
| `pct1336_driver.c/h` | PCT1336 触摸芯片驱动 |
| `touchpad_service.c/h` | 触摸板服务（整合驱动、手势、滤波） |

---

## 七、工具库（Utils）

工具库提供通用的辅助函数和算法。

### 目录结构
```
utils/
├── AES128.c                    # AES128 加密实现
├── AES128.h                    # AES128 加密接口
├── bitwise.c                   # 位操作工具实现
├── bitwise.h                   # 位操作工具接口
├── crc16.c                     # CRC16 校验实现
├── crc16.h                     # CRC16 校验接口
├── progmem.h                   # 程序存储器宏定义
└── util.h                      # 通用工具宏
```

### 文件说明

| 文件 | 作用 |
|------|------|
| `AES128.c/h` | AES128 加密算法（用于数据加密） |
| `bitwise.c/h` | 位操作工具（位反转、位计数等） |
| `crc16.c/h` | CRC16 校验算法（用于数据完整性校验） |
| `progmem.h` | 程序存储器访问宏（兼容 AVR） |
| `util.h` | 通用工具宏（MIN、MAX 等） |

---

## 八、测试框架（Test & Unity）

### 目录结构
```
test/
└── storage_test.c              # 存储模块测试

unity/
├── unity.c                     # Unity 测试框架实现
├── unity.h                     # Unity 测试框架接口
└── unity_internals.h           # Unity 内部定义
```

### 文件说明

| 文件 | 作用 |
|------|------|
| `test/storage_test.c` | 存储模块单元测试 |
| `unity/unity.c/h` | Unity 测试框架（C 语言单元测试框架） |
| `unity/unity_internals.h` | Unity 内部实现定义 |

---

## 九、其他目录

### examples/
示例代码目录（预留）。

### lib/
第三方库目录（预留）。

### DOCS/
文档目录（预留）。

### memory-bank/
知识库目录，存储项目相关文档和知识。

### code_example/
代码示例目录，包含参考代码（沁恒蓝牙示例、QMK 固件示例等），**不纳入框架核心说明**。

### project/
芯片 SDK 目录，包含芯片厂商 SDK（CH584 SDK 等），**不纳入框架核心说明**。

---

## 系统架构分层说明

```
┌─────────────────────────────────────────┐
│         应用层 (Application)             │  系统初始化、服务协调、主程序
├─────────────────────────────────────────┤
│        中间件层 (Middleware)             │  键盘逻辑、通信协议、HID 报告
├─────────────────────────────────────────┤
│         驱动层 (Drivers)                 │  设备驱动、电源管理、存储管理
├─────────────────────────────────────────┤
│    硬件抽象层 (HAL)                      │  跨平台硬件接口抽象
└─────────────────────────────────────────┘
```

### 分层职责

1. **HAL 层**：提供跨平台的硬件抽象，屏蔽硬件差异
2. **驱动层**：基于 HAL 实现具体设备驱动
3. **中间件层**：实现业务逻辑和协议处理
4. **应用层**：协调各模块，实现产品功能

---

## 核心数据流

### 按键处理流程

```
矩阵扫描 (matrix.c)
  → 防抖 (debounce.c)
  → 键盘任务 (keyboard.c)
  → 动作处理 (action.c)
  → HID 报告生成 (report.c)
  → 主机通信 (host.c)
  → 传输层 (transport.c)
  → USB/BLE/2.4G 发送
```

### 无线通信流程

```
传输模式选择 (transport.c)
  → 无线管理 (wireless.c)
  → 报告缓冲 (report_buffer.c)
  → 蓝牙/2.4G 驱动
  → 主机接收
```

### 系统初始化流程

```
system_init_coordinator()
  → HAL 初始化 (gpio, timer, uart)
  → 驱动初始化 (matrix, battery, storage)
  → 中间件初始化 (keyboard, transport)
  → 应用服务初始化
  → 进入主循环
```

---

## 支持的平台

| 平台 | 状态 | 说明 |
|------|------|------|
| CH584 | ✅ 完整支持 | 沁恒 CH584 蓝牙芯片，已实现完整 HAL 和驱动 |
| nRF52 | 🔨 开发中 | Nordic nRF52 系列，HAL 头文件已定义 |
| PixArt 2860 | 🔨 开发中 | PixArt 2860 触摸板控制器，HAL 头文件已定义 |
| Test | ✅ 完整支持 | 测试平台（Mock 实现，用于单元测试） |

---

## 支持的功能特性

### 输入功能
- ✅ 矩阵扫描（COL2ROW / ROW2COL）
- ✅ 防抖算法（多种算法可选）
- ✅ 组合键（Combo）
- ✅ 多层键映射
- 🔨 触摸板支持（开发中）

### 通信功能
- ✅ USB HID
- ✅ 蓝牙 BLE HID
- ✅ 2.4G 无线
- ✅ 三模切换

### 电源管理
- ✅ 电池电量监测
- ✅ 低功耗模式
- ✅ 自动休眠

### 存储功能
- ✅ EEPROM 存储
- ✅ 系统配置持久化
- ✅ 用户配置持久化
- ✅ CRC16 校验

### 输出功能
- ✅ 背光控制（PWM）
- ✅ 状态指示灯
- ✅ 日志输出

---

## 构建系统

项目使用 **CMake** 构建系统，支持以下构建配置：

- **编译器**：GCC (MinGW-w64) 15.2.0+
- **构建工具**：mingw32-make
- **CMake 版本**：3.29+
- **操作系统**：Windows (MSYS2 环境)

详细构建说明请参考 `CLAUDE.md` 文件。

---

## 测试框架

项目使用 **Unity** 测试框架进行单元测试。测试用例位于 `application/main.c`，包括：

1. 矩阵扫描基础功能测试
2. 多键同时按下测试
3. 按键序列测试
4. 单个按键状态测试
5. 边界条件测试

运行测试：
```bash
cd cmake-build-debug
cmake --build .
.\Debug\keyboard_framework.exe
```

---

## 代码规范

项目遵循以下代码规范：

1. **模块方法命名规范**：
   - `_setup()`: 早期启动阶段初始化
   - `_init()`: 主初始化阶段
   - `_pre_task()`: 任务前执行
   - `_task()`: 主任务循环
   - `_post_task()`: 任务后执行

2. **头文件保护**：使用 `#pragma once`

3. **平台特定代码**：使用平台前缀或 HAL 抽象

4. **注释风格**：Doxygen 风格注释

详细代码规范请参考 QMK 固件的编码规范文档。

---

## 未来计划

- [ ] 完善 nRF52 平台支持
- [ ] 完善 PixArt 2860 平台支持
- [ ] 触摸板手势识别优化
- [ ] RGB 背光支持
- [ ] OLED 显示支持
- [ ] 宏录制功能
- [ ] 配置工具（GUI）

---

## 项目文件统计

| 层级 | 文件数量 | 主要语言 |
|------|----------|----------|
| HAL | ~40 个文件 | C/H |
| Drivers | ~50 个文件 | C/H |
| Middleware | ~150 个文件 | C/H（含 100+ 个键盘布局） |
| Application | ~15 个文件 | C/H |
| Component | 8 个文件 | C/H |
| Utils | 8 个文件 | C/H |
| **总计** | **~270 个核心文件** | C/H |

---

## 许可证

本项目基于 **GNU General Public License v3.0**（部分代码继承自 QMK 固件）。

---

**文档版本**: v1.0
**最后更新**: 2025-01-07
**维护者**: Claude Code
