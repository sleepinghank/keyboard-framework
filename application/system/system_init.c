/**
 * @file system_init.c
 * @brief 系统初始化协调器实现
 * @version 2.0.0
 * @date 2026-03-19
 *
 * 设计说明:
 * - 按HAL → Driver → Middleware → Application的层级顺序初始化
 * - 简化的初始化流程: 删除空的_setup阶段
 * - 使用状态机管理初始化进度
 */

#include "kb904/config.h"
//  application
#include "system_init.h"
#include "sys_config.h"
#include "indicator_test.h"

// drivers
#include "timer.h"
#include "matrix.h"
#include "storage.h"
#include "eeprom.h"
#include "battery.h"
#include "indicator.h"
#include "event_manager.h"
#include "bt_driver.h"
#include "backlight.h"

// middleware
#include "report_buffer.h"
#include "lpm.h"
#include "wireless.h"
#include "keyboard.h"
#include "transport.h"

// application services
#include "system_service.h"
#include "input_service.h"
#include "output_service.h"
#include "communication_service.h"

// hal
#include "i2c_master.h"
#include "gpio.h"
#include "uart.h"
#include "pin_defs.h"
#include "pwm.h"
#include "adc.h"
#include "system_hal.h"
#include "hw_timer.h"

// ch584
#include "CONFIG.h"
#include "hal.h"

static volatile system_init_status_t g_system_init_status = SYSTEM_INIT_STATUS_NOT_STARTED;
static volatile bool g_system_initialized = false;

/*==========================================
 * 初始化阶段
 * =========================================*/

void system_init_hal(void) {
    // HAL层初始化阶段
    // Timer已在setup阶段初始化，此处可进行HAL层其他初始化
    // i2c_init();
    // platform_uart_init(PLATFORM_UART_1, 115200, 0);
    // pwm_init();
    uint8_t status = 3;
    // platform_uart_bind_pins(NO_PIN, B21, PLATFORM_UART_3);
    // platform_uart_init(PLATFORM_UART_3, 921600, 0);
    platform_uart_bind_pins(NO_PIN, A9, PLATFORM_UART_1);
    platform_uart_init(PLATFORM_UART_1, 921600, 0);

    i2c_init();
    status = i2c_init_channel_with_pins(I2C_CHANNEL_0, TOUCHPAD_SDA, TOUCHPAD_SCL, 400000);
    PRINT("I2C init status: %d\r\n", status);
    PCT1336_Communication_Test();
    PRINT("B19 HAL initialized\r\n");

    // 硬件定时器初始化
    hw_timer_init();
    PRINT("Hardware timer initialized\r\n");

    // 标记HAL init完成
    g_system_init_status = SYSTEM_INIT_STATUS_HAL;
}

void system_init_drivers(void) {
    // 驱动层初始化阶段
    // 按依赖关系顺序初始化各驱动

    // 时钟初始化
    timer_init();

    // 1. 存储系统初始化 (最优先)
    storage_init();

#ifdef BLUETOOTH_ENABLE_FLAG
    // 2. 蓝牙驱动初始化 - BLE 协议栈
    bt_driver_init(false);
#endif

    // 3. 电池管理初始化 (从 input_service 移入)
    // battery_init();

    // 4. 背光初始化 (从 output_service 移入)
    // backlight_init(NULL);

    // 5. 指示灯初始化
    indicator_init();

    // 标记Driver init完成
    g_system_init_status = SYSTEM_INIT_STATUS_DRIVER;
}

void system_init_middleware(void) {
    // 中间件初始化阶段
    // 按依赖关系顺序初始化各中间件

    // 3. 报告缓冲区初始化
    // report_buffer_init();

    // 4. 低功耗管理初始化
    lpm_init();

    // 5. 无线管理层初始化
    wireless_init();

    // 6. 键盘处理初始化
    keyboard_init();

    // 标记Middleware init完成
    g_system_init_status = SYSTEM_INIT_STATUS_MIDDLEWARE;
}


void system_init_application(void) {
    // 应用层初始化阶段
    // 初始化各应用服务
    system_service_init();
    input_service_init();   // 启动矩阵扫描定时器
    output_service_init();
    commu_service_init();
    indicator_test();

    // 标记Application init完成
    g_system_init_status = SYSTEM_INIT_STATUS_APPLICATION;

    // 最后标记完全初始化完成
    g_system_init_status = SYSTEM_INIT_STATUS_COMPLETED;
    g_system_initialized = true;
}


/*==========================================
 * 系统初始化协调器主函数
 * =========================================*/

uint32_t system_init_coordinator(void) {
    // 1. 系统硬件初始化
    system_hal_init();

    // init 阶段
    system_init_hal();
    system_init_drivers();
    system_init_middleware();
    system_init_application();

    return 0;
}

/*==========================================
 * 状态查询函数
 * =========================================*/

bool system_is_initialized(void) {
    return g_system_initialized;
}

uint8_t system_get_init_status(void) {
    return (uint8_t)g_system_init_status;
}
