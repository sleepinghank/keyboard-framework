cmake_minimum_required(VERSION 3.29)
project(keyboard_framework C)

set(CMAKE_C_STANDARD 11)


# 包含目录
include_directories(
    .
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
    middleware/protocol
    keyboards
    utils
    utils/debug
)

# 源文件列表（排除application/main.c）
file(GLOB_RECURSE SOURCE_FILES
    "drivers/*.c"
    "hal/platforms/${PLATFORM}/*.c"
    "middleware/*.c"
    "utils/*.c"
)


add_executable(keyboard_framework main.c)
