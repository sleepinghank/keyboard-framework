cmake_minimum_required(VERSION 3.29)
project(keyboard-framework C)

# 设置 C 标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

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
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/power
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/storage
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/system/platforms/test
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/system
        ${CMAKE_CURRENT_SOURCE_DIR}/middleware
        ${CMAKE_CURRENT_SOURCE_DIR}/middleware/communication
        ${CMAKE_CURRENT_SOURCE_DIR}/middleware/keyboard
        ${CMAKE_CURRENT_SOURCE_DIR}/middleware/keyboard/keymap_extras
        ${CMAKE_CURRENT_SOURCE_DIR}/application
        ${CMAKE_CURRENT_SOURCE_DIR}/application/system
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/logging
        # CH584 标准外设库头文件路径
        ${CMAKE_CURRENT_SOURCE_DIR}/project/ch584m/StdPeriphDriver/inc
)

# 添加所有源文件
set(SOURCES
    ./application/system/system_init.c
    ./drivers/communication/bluetooth/bt_driver.c
    ./drivers/communication/p2p4g/p24g_driver.c
    ./drivers/input/keyboard/debounce.c
    ./drivers/input/keyboard/matrix.c
    ./drivers/output/backlight/backlight.c
    ./drivers/output/backlight/backlight_pwm.c
    ./drivers/output/indicators/indicator.c
    ./drivers/power/battery.c
    ./drivers/storage/storage.c
    ./drivers/storage/test/eeprom.c
    ./drivers/system/platforms/test/event_manager.c
    ./drivers/system/platforms/test/wait.c
    ./drivers/system/timer.c
    ./drivers/system/timer_manager.c
    ./hal/platforms/test/_adc.c
    ./hal/platforms/test/_i2c_master.c
    ./hal/platforms/test/_i2c_slave.c
    ./hal/platforms/test/_pwm.c
    ./hal/platforms/test/_spi_master.c
    ./hal/platforms/test/_uart.c
    ./hal/platforms/test/pin_mapper.c
    # CH584平台HAL实现
    ./hal/platforms/ch584/_gpio.c
    ./hal/platforms/ch584/_adc.c
    ./hal/platforms/ch584/_i2c_master.c
    ./hal/platforms/ch584/_i2c_slave.c
    ./hal/platforms/ch584/_pwm.c
    ./hal/platforms/ch584/_spi_master.c
    ./hal/platforms/ch584/_uart.c
    ./hal/platforms/ch584/pin_mapper.c
    ./middleware/communication/host.c
    ./middleware/communication/lpm.c
    ./middleware/communication/report_buffer.c
    ./middleware/communication/transport.c
    ./middleware/communication/wireless.c
    ./middleware/keyboard/action.c
    ./middleware/keyboard/action_layer.c
    ./middleware/keyboard/action_util.c
    ./middleware/keyboard/combo.c
    ./middleware/keyboard/custom_function.c
    ./middleware/keyboard/keyboard.c
    ./middleware/keyboard/keycode_config.c
    ./middleware/keyboard/keymap_common.c
    ./middleware/keyboard/keymap_introspection.c
    ./middleware/keyboard/report.c
    ./utils/bitwise.c
    ./utils/logging/debug.c
    ./utils/logging/print.c
    ./utils/logging/sendchar.c
)

# 将源文件添加到可执行目标
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})


# 可选：添加编译定义
target_compile_definitions(${PROJECT_NAME} PRIVATE SIMULATION_MODE=1)

# 可选：设置编译选项
if(CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -g)
endif()
