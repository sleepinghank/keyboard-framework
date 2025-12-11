cmake_minimum_required(VERSION 3.29)

project(keyboard_framework C)

set(CMAKE_C_STANDARD 11)

# 包含目录
include_directories(
    application
    drivers
    drivers/communication
    drivers/input
    drivers/output
    drivers/storage
    drivers/system
    hal
    hal/platforms
    middleware
    middleware/config
    middleware/keyboard
    middleware/keyboard/keymap_extras
    middleware/protocol
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
    drivers/system/timer.c

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

    # 硬件抽象层 - 测试平台实现
    hal/platforms/test/_gpio.c

    utils/logging/debug.c
    utils/logging/print.c
    utils/logging/sendchar.c
    utils/bitwise.c
)

add_executable(keyboard_framework ${SOURCES})
