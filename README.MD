# 键盘项目模块化

> 跨平台模块化实现

----

你现在是一名嵌入式软件开发工程师，负责为公司多模无线键盘及触控板产品设计并实现一个跨芯片平台（至少兼容原相2860,沁恒CH584,Nordic系列）的高内聚、低耦合软件框架。该框架参考qmk按键处理流程。同时需支持快速产品迭代与高效开发，具备良好的可移植性、模块解耦性和配置灵活性。

请基于以下具体功能需求，设计清晰的分层架构（如硬件抽象层HAL、驱动层、应用逻辑层等），并在第一阶段输出详细的模块划分与层级接口定义文档，明确各模块职责、依赖关系及跨层交互机制：

1. **通信协议栈**：同时支持BLE（GATT HID）、USB-HID和专有2.4GHz无线协议，三者可动态切换或并行运行，需提供统一的上层数据发送接口。
2. **按键处理模块**：参考qmk按键处理核心流程，使用同样的按键处理链
3. **状态指示灯**：控制单颗RGB LED，实现预定义灯效（如：开机常亮2秒后熄灭、关机快闪3次、BLE配对慢闪1Hz、低电急促双闪），灯效可通过配置参数调整时序。
4. **背光控制**：驱动三色独立PWM背光（红、绿、蓝），支持0–100%亮度调节（至少16级）及任意颜色混合，提供API供上层设置静态颜色或动态灯效（如呼吸、渐变）。
5. **触控板驱动**：通过I²C接口读取原相1336触控芯片数据，在中断触发后≤2ms内完成触点坐标/手势解析，并封装为标准HID触摸板报告格式上报。
6. **电池管理**：基于ADC采样，每30秒更新一次电量百分比（0–100%），在电量≤10%时触发低电提醒事件，并在≤5%时执行自动关机。
7. **非易失存储**：使用Flash或EEPROM存储用户配置（如按键映射、背光模式）和系统参数（如蓝牙绑定信息、设备ID），要求写入寿命≥10,000次，掉电后数据保持≥10年。
8. **低功耗管理**：
   - 无操作30秒进入Idle模式（CPU停，外设可唤醒）；
   - 无操作5分钟进入深度睡眠（仅RTC和GPIO唤醒源有效）；
   - 任意按键或BLE连接请求可唤醒系统，唤醒至正常工作状态时间≤100ms。
9. **自定义通信通道**：通过BLE或USB实现与PC端上位机的私有协议通信（如配置同步、固件校验），定义包结构（含头、命令、负载、CRC），支持双向可靠传输。
10. **事件调度系统**：实现基于队列或状态机的事件处理机制，统一处理关键系统事件（如BLE连接/断连、广播启停、低电关机、OTA触发、触控中断等），确保事件不丢失、可优先级排序。

框架设计必须满足：

- 模块间通过明确定义的接口通信，禁止跨层直接调用；
- 硬件相关代码全部封装在HAL层，更换主控芯片时仅需重写HAL；
- 所有功能模块默认可裁剪，通过编译宏控制是否启用；
- 提供标准化的初始化流程与配置结构体，新项目可在1人日内完成基础工程搭建。

不需要每个模块具体的实现，现在是框架搭建阶段，需要明确模块划分和代码分层。请根据上述思路，按照层级和模块。输出所有的文件层级和相应文件名。并且标注每个文件中必须实现的几个方法。做成表格整理处理出来。方便开发人员搭建基础工程。

## 文件目录

hal/                              # 硬件抽象层
├── interface/                    # HAL接口定义
│   ├── gpio_interface.h         # GPIO接口
│   ├── adc_interface.h          # ADC接口
│   ├── pwm_interface.h          # PWM接口
│   ├── i2c_interface.h          # I2C接口
│   ├── uart_interface.h         # UART接口
│   ├── timer_interface.h        # 定时器接口
│   └── power_interface.h        # 电源管理接口
└── platforms/                    # 各平台HAL实现
    ├── pixart2860/              # 原相2860实现
    │   ├── gpio_hal.c/.h
    │   ├── adc_hal.c/.h
    │   ├── pwm_hal.c/.h
    │   ├── i2c_hal.c/.h
    │   ├── uart_hal.c/.h
    │   ├── timer_hal.c/.h
    │   └── power_hal.c/.h
    ├── ch584/                   # 沁恒CH584实现
    └── nrf52/                   # Nordic实现

drivers/                          # 驱动层
├── interface/                    # 驱动接口定义
│   ├── communication_interface.h
│   ├── input_interface.h
│   ├── output_interface.h
│   ├── storage_interface.h
│   └── system_interface.h
├── communication/
│   ├── interface/                # 通信接口定义
│   │   ├── ble_interface.h       # BLE通用接口
│   │   ├── usb_interface.h       # USB通用接口
│   │   └── wireless24_interface.h # 2.4G通用接口
│   ├── ble/                      # BLE通信实现
│   │   ├── core/                 # BLE协议核心逻辑(平台无关)
│   │   │   ├── ble_manager.c/.h
│   │   │   ├── ble_hid.c/.h
│   │   │   └── ble_state.c/.h
│   │   └── platforms/            # 各平台BLE实现
│   │       ├── pixart2860/       # 原相2860 BLE实现
│   │       │   ├── ble_hal.c/.h
│   │       │   └── ble_config.h
│   │       ├── ch584/           # 沁恒CH584 BLE实现
│   │       │   ├── ble_hal.c/.h
│   │       │   └── ble_config.h
│   │       └── nrf52/           # Nordic BLE实现
│   │           ├── ble_hal.c/.h
│   │           └── ble_config.h
│   ├── usb/                      # USB通信实现
│   │   ├── core/                 # USB协议核心逻辑
│   │   │   ├── usb_manager.c/.h
│   │   │   ├── usb_hid.c/.h
│   │   │   └── usb_descriptors.c/.h
│   │   └── platforms/            # 各平台USB实现
│   │       ├── pixart2860/
│   │       ├── ch584/
│   │       └── nrf52/
│   └── wireless24/               # 2.4G通信实现
│       ├── core/                 # 2.4G协议核心逻辑
│       └── platforms/            # 各平台2.4G实现
├── input/
│   ├── interface/                # 输入接口定义
│   │   ├── keyboard_interface.h  # 键盘输入接口
│   │   ├── touchpad_interface.h  # 触控板输入接口
│   │   └── gesture_interface.h   # 手势识别接口
│   ├── keyboard/                 # 键盘输入驱动
│   │   ├── scanner/              # 矩阵扫描
│   │   │   ├── matrix_scanner.c/.h
│   │   │   ├── debounce.c/.h
│   │   │   └── rollover.c/.h
│   ├── touchpad/                 # 触控板驱动
│   │   ├── drivers/              # 各芯片触控板驱动
│   │   │   ├── pixart_driver.c/.h
│   │   │   ├── generic_i2c_driver.c/.h
│   │   │   └── spi_driver.c/.h
│   │   ├── processor/            # 触控数据处理
│   │   │   ├── touch_processor.c/.h
│   │   │   ├── filter.c/.h       # 滤波算法
│   │   │   └── calibration.c/.h
│   │   └── gesture/              # 手势识别
│   │       ├── gesture_detector.c/.h
│   │       ├── tap_gesture.c/.h
│   │       ├── scroll_gesture.c/.h
│   │       └── swipe_gesture.c/.h
│   └── buttons/                  # 额外按键(如侧键)
│       ├── button_scanner.c/.h
│       └── button_processor.c/.h
├── output/                       # 输出驱动
│   ├── leds/                     # LED驱动
│   │   ├── led_driver.c/.h       # LED基础驱动
│   │   ├── led_pwm.c/.h          # PWM控制LED
│   │   ├── led_timer.c/.h        # 定时器控制LED
│   │   └── led_effects.c/.h      # LED特效
│   ├── backlight/                # 背光驱动
│   │   ├── backlight_driver.c/.h
│   │   ├── backlight_pwm.c/.h
│   │   └── backlight_rgb.c/.h
│   └── indicators/               # 指示灯驱动
│       ├── indicator_driver.c/.h
│       ├── battery_indicator.c/.h
│       └── connection_indicator.c/.h
├── storage/                      # 存储驱动
│   ├── eeprom_driver.c/.h        # EEPROM驱动
│   ├── storage_manager.c/.h      # 存储管理器
│   └── config_storage.c/.h       # 配置存储
├── power/                        # 电源管理驱动
│   ├── battery_monitor.c/.h      # 电池监控
│   ├── power_manager.c/.h        # 电源管理器
└── system/                       # 系统驱动
    ├── timer_manager.c/.h        # 定时器管理
    ├── watchdog.c/.h             # 看门狗
    ├── timer.c/.h         # 系统时钟
    ├── atomic.c/.h         # 禁用中断
	├── event_manager.c/.h         # 与平台相关的事件处理
    └── wait.c/.h         # 等待
middleware/                       # 中间层
├── config/                       # 配置管理
│   ├── config_manager.c/.h        # 配置管理器
│   ├── config_loader.c/.h         # 配置加载器
│   ├── config_validator.c/.h      # 配置验证器
│   └── default_config.h           # 默认配置
├── protocol/                     # 协议处理
│   ├── hid_processor.c/.h        # HID协议处理
│   ├── report_builder.c/.h       # 报告构建器
│   ├── protocol_dispatcher.c/.h   # 协议分发器
└── keyboard/                       # 热键处理模块
    ├── mapper/               # 按键映射
    │   ├── key_mapper.c/.h
    │   ├── layer_manager.c/.h
    │   └── keymap_loader.c/.h
    ├── combo/               # 组合键处理
    │   ├── combo.c/.h
    └── processor/            # 按键处理
		├── key_processor.c/.h
		├── modifier_handler.c/.h
		└── auto_repeat.c/.h
keyboards/                        # 产品特定配置目录
├── product_name/                 # 具体产品目录
│   ├── keymaps/                 # 按键映射配置
│   │   ├── default_keymap.h      # 默认按键映射
│   │   ├── gaming_keymap.h       # 游戏模式映射
│   │   └── office_keymap.h       # 办公模式映射
│   ├── combos/                  # 固定组合键配置
│   │   ├── fixed_combos.h        # 预定义组合键
│   │   ├── combo_config.h        # 组合键配置
│   │   └── combo_tables.c         # 组合键查找表
│   └── product_config.h          # 产品特定配置
application/                      # 应用层
├── main.c                        # 主程序
├── sys_config.h                  # 应用配置
├── app_state.c/.h                # 应用状态
├── product/                      # 产品管理模块
│   ├── product_init.c/.h        # 产品初始化
│   ├── product_config.c/.h       # 产品配置管理
│   └── product_profile.c/.h      # 产品特性配置
├── mode/                         # 模式管理模块
│   ├── mode_manager.c/.h        # 模式管理器(替换feature_manager)
│   ├── operation_mode.c/.h       # 工作模式(办公/游戏/编程)
│   ├── power_mode.c/.h           # 电源模式(正常/节能/高性能)
│   └── connection_mode.c/.h      # 连接模式(BLE/USB/2.4G)
├── service/                      # 服务管理模块(替换device_manager概念)
│   ├── input_service.c/.h        # 输入服务(键盘+触控板)
│   ├── output_service.c/.h       # 输出服务(LED+背光+指示灯)
│   ├── communication_service.c/.h # 通信服务(BLE/USB/2.4G)
│   ├── power_service.c/.h        # 电源服务
│   └── storage_service.c/.h      # 存储服务
utils/                           # 工具类
├── common/                      # 通用工具
│   ├── utils.c/.h               # 基础工具函数
│   ├── ring_buffer.c/.h         # 环形缓冲区
│   ├── crc.c/.h                 # CRC校验
│   └── string_utils.c/.h        # 字符串工具
├── debug/                       # 调试工具
│   ├── logger.c/.h              # 日志系统
│   ├── profiler.c/.h            # 性能分析
│   └── debug_uart.c/.h          # 调试串口
└── memory/                      # 内存工具
    ├── memory_pool.c/.h         # 内存池
    ├── object_allocator.c/.h    # 对象分配器
    └── stack_monitor.c/.h       # 栈监控