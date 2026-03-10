cmake_minimum_required(VERSION 3.29)
project(keyboard-framework C)

# 设置 C 标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# 自动收集源文件（可根据需要手动指定）
file(GLOB_RECURSE APP_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/keyboards/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hal/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/middleware/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/application/*.c
)

# === 第二步：创建可执行目标（关键！必须在 target_include_directories 之前）===
add_executable(keyboard-framework test_main.c)

# 包含目录（保持与 Keil 项目一致）
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/keyboards
        ${CMAKE_CURRENT_SOURCE_DIR}/keyboards/keymaps
        ${CMAKE_CURRENT_SOURCE_DIR}/hal
        ${CMAKE_CURRENT_SOURCE_DIR}/hal/platforms/test
        ${CMAKE_CURRENT_SOURCE_DIR}/hal/platforms/ch584
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/communication
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/communication/bluetooth
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/communication/p2p4g
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/input/keyboard
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/input/touchpad
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/output/backlight
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/output/indicators
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/output/logging
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/power
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/storage
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/system/platforms/test
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/system
        ${CMAKE_CURRENT_SOURCE_DIR}/middleware
        ${CMAKE_CURRENT_SOURCE_DIR}/middleware/communication
        ${CMAKE_CURRENT_SOURCE_DIR}/middleware/keyboard
        ${CMAKE_CURRENT_SOURCE_DIR}/middleware/keyboard/combo
        ${CMAKE_CURRENT_SOURCE_DIR}/middleware/keyboard/keymap_extras
        ${CMAKE_CURRENT_SOURCE_DIR}/application
        ${CMAKE_CURRENT_SOURCE_DIR}/application/system
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/logging
        ${CMAKE_CURRENT_SOURCE_DIR}/component/touch_component/include
        ${CMAKE_CURRENT_SOURCE_DIR}/application/service
)

# 添加所有源文件 (Test Platform Only - 本机编译仅使用test平台实现)
set(SOURCES
    # Application Layer
    ./application/system/system_init.c
    ./application/service/communication_service.c
    ./application/service/input_service.c
    ./application/service/output_service.c
    ./application/service/system_service.c
    ./application/service/wireless_callbacks.c

    # Drivers - Communication (Test Platform Only)
    ./drivers/communication/bluetooth/test/_bt_driver.c
    ./drivers/communication/p2p4g/p24g_driver.c

    # Drivers - Input
    ./drivers/input/keyboard/debounce.c
    ./drivers/input/keyboard/matrix.c

    # Drivers - Output
    ./drivers/output/backlight/backlight.c
    ./drivers/output/backlight/backlight_hal.c
    ./drivers/output/indicators/indicator.c
    ./drivers/output/indicators/indicator_hal.c
    ./drivers/output/logging/debug.c
    ./drivers/output/logging/print.c
    ./drivers/output/logging/sendchar.c

    # Drivers - Power
    ./drivers/power/battery.c

    # Drivers - Storage (Test Platform Only)
    ./drivers/storage/storage.c
    ./drivers/storage/test/eeprom.c

    # Drivers - System (Test Platform Only)
    ./drivers/system/platforms/test/event_manager.c
    ./drivers/system/platforms/test/wait.c
    ./drivers/system/timer.c
    ./drivers/system/timer_manager.c

    # HAL - Test Platform Only
    ./hal/platforms/test/_gpio.c
    ./hal/platforms/test/_adc.c
    ./hal/platforms/test/_i2c_master.c
    ./hal/platforms/test/_i2c_slave.c
    ./hal/platforms/test/_pwm.c
    ./hal/platforms/test/_spi_master.c
    ./hal/platforms/test/_uart.c
    ./hal/platforms/test/pin_mapper.c

    # Middleware - Communication
    ./middleware/communication/host.c
    ./middleware/communication/lpm.c
    ./middleware/communication/report_buffer.c
    ./middleware/communication/transport.c
    ./middleware/communication/wireless.c

    # Middleware - Keyboard (精简版)
    ./middleware/keyboard/keyboard.c
    ./middleware/keyboard/keymap.c
    ./middleware/keyboard/keycode_config.c
    ./middleware/keyboard/report.c

    # Middleware - Keyboard - Combo (新增精简模块)
    ./middleware/keyboard/combo/process_combo.c
    ./middleware/keyboard/combo/FN_Combo.c
    ./middleware/keyboard/combo/Special_Combo.c
    ./middleware/keyboard/combo/media_combo.c

    # Keyboards - Keymap
    ./keyboards/default_keymap.c

    # Utils
    ./utils/AES128.c
    ./utils/bitwise.c
    ./utils/crc16.c
    ./utils/linkedlist.c

    # Component - Touchpad
    ./component/touch_component/pct1336_driver.c
    ./component/touch_component/touchpad_service.c
    # ./component/touch_component/bayes_filtering.c   # 可选：贝叶斯防误触过滤
    # ./component/touch_component/mcs_gesture.c       # 可选：手势识别
)

# 将源文件添加到可执行目标
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})


# 可选：添加编译定义
target_compile_definitions(${PROJECT_NAME} PRIVATE SIMULATION_MODE=1)

# 可选：设置编译选项
if(CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -g)
endif()
