cmake_minimum_required(VERSION 3.29)

project(keyboard_framework C)

set(CMAKE_C_STANDARD 11)

# 包含目录
include_directories(
    application
    drivers
    drivers/communication
    drivers/communication/bluetooth
    drivers/communication/p2p4g
    drivers/communication/usb
    drivers/input
    drivers/input/keyboard
    drivers/input/touchpad
    drivers/output
    drivers/output/indicators
    drivers/storage
    drivers/system
    drivers/power
    hal
    hal/platforms
    middleware
    middleware/config
    middleware/keyboard
    middleware/keyboard/keymap_extras
    middleware/communication
    keyboards
    utils
    utils/logging
)

# 添加源文件 - 仅核心模块
set(SOURCES
    # 应用程序入口
    main.c

    # 驱动层 - 输入设备（核心）
    drivers/input/keyboard/matrix.c
    drivers/input/keyboard/debounce.c
#    drivers/communication/bluetoothd
#    drivers/communication/p2p4g
#    drivers/communication/usb
    drivers/output/indicators/indicator.c
    drivers/power/battery.c
    drivers/system/timer.c
    drivers/system/timer_manager.c
    drivers/system/platforms/test/wait.c

    # 中间件 - 键盘处理
    middleware/keyboard/action.c
#    middleware/keyboard/combo.c
    middleware/keyboard/custom_function.c
    middleware/keyboard/keyboard.c
    middleware/keyboard/action_util.c
    middleware/keyboard/keymap_common.c
    middleware/keyboard/action_layer.c
    middleware/keyboard/keymap_introspection.c
    middleware/keyboard/report.c
    middleware/keyboard/keycode_config.c


    # 中间件 - 通信处理
    middleware/communication/host.c
    middleware/communication/report_buffer.c
    middleware/communication/transport.c
    middleware/communication/wireless.c
    middleware/communication/lpm.c

    # 硬件抽象层 - 测试平台实现
    hal/platforms/test/_gpio.c

    utils/logging/debug.c
    utils/logging/print.c
    utils/logging/sendchar.c
    utils/bitwise.c
)

add_executable(keyboard_framework ${SOURCES})
