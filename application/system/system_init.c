/**
 * @file system_init.c
 * @brief 系统初始化协调器实现
 * @version 1.0.0
 * @date 2025-12-16
 *
 * 设计说明:
 * - 按HAL → Driver → Middleware → Application的层级顺序初始化
 * - 每个层级按_setup → _init → _pre_task → _task → _post_task的生命周期
 * - 使用状态机管理初始化进度
 */

#include "product_config.h"
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
// 系统初始化状态
typedef enum {
    SYSTEM_INIT_STATUS_NOT_STARTED = 0,    // 未开始
    SYSTEM_INIT_STATUS_HAL_SETUP,          // HAL setup完成
    SYSTEM_INIT_STATUS_DRIVER_SETUP,       // Driver setup完成
    SYSTEM_INIT_STATUS_MIDDLEWARE_SETUP,   // Middleware setup完成
    SYSTEM_INIT_STATUS_APPLICATION_SETUP,  // Application setup完成
    SYSTEM_INIT_STATUS_HAL_INIT,           // HAL init完成
    SYSTEM_INIT_STATUS_DRIVER_INIT,        // Driver init完成
    SYSTEM_INIT_STATUS_MIDDLEWARE_INIT,    // Middleware init完成
    SYSTEM_INIT_STATUS_APPLICATION_INIT,   // Application init完成
    SYSTEM_INIT_STATUS_COMPLETED           // 完全初始化完成
} system_init_status_t;

static volatile system_init_status_t g_system_init_status = SYSTEM_INIT_STATUS_NOT_STARTED;
static volatile bool g_system_initialized = false;

/*==========================================
 * 早期启动阶段 - 在_init之前运行
 * =========================================*/

void system_setup_hal(void) {
    // HAL层基础初始化

    // 首先初始化所有GPIO为安全状态，防止悬空漏电
    // system_hal_gpio_init_all();

    // i2c_bind_pins(SDA_PIN, SCL_PIN, I2C_CHANNEL_0);

    // platform_uart_bind_pins(UART_TX_PIN, UART_RX_PIN, PLATFORM_UART_1);

    // adc_bind_pin(ADC_PIN,ADC_CHANNEL);

    // pwm_init();

    // 标记HAL setup完成
    g_system_init_status = SYSTEM_INIT_STATUS_HAL_SETUP;
}

void system_setup_drivers(void) {
    // 驱动层setup阶段
    // - 矩阵扫描setup
    // matrix_setup();

    // 标记Driver setup完成
    g_system_init_status = SYSTEM_INIT_STATUS_DRIVER_SETUP;
}

void system_setup_middleware(void) {
    // 中间件setup阶段
    // 目前中间件层在setup阶段无需特殊处理

    // 标记Middleware setup完成
    g_system_init_status = SYSTEM_INIT_STATUS_MIDDLEWARE_SETUP;
}

void system_setup_application(void) {
    // 应用层setup阶段
    // 目前应用层在setup阶段无需特殊处理

    // 标记Application setup完成
    g_system_init_status = SYSTEM_INIT_STATUS_APPLICATION_SETUP;
}

/*==========================================
 * 初始化阶段 - 在主机协议、调试和MCU外设初始化后运行
 * =========================================*/

void system_init_hal(void) {
    // HAL层初始化阶段
    // Timer已在setup阶段初始化，此处可进行HAL层其他初始化
    // i2c_init();
    // platform_uart_init(PLATFORM_UART_1, 115200, 0);
    // pwm_init();
    platform_uart_bind_pins(NO_PIN, B13, PLATFORM_UART_1);
    platform_uart_init(PLATFORM_UART_1, 115200, 0);

    setPinOutput(B14);
    writePinHigh(B14);

    setPinOutput(B19);
    writePinHigh(B19);
    PRINT("B19 HAL initialized\r\n");

    // 硬件定时器初始化
    hw_timer_init();
    PRINT("Hardware timer initialized\r\n");

    // 标记HAL init完成
    g_system_init_status = SYSTEM_INIT_STATUS_HAL_INIT;
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

    // 3. 电池管理初始化
    // battery_init();

    // 3. 指示灯初始化
    indicator_init();


    // 标记Driver init完成
    g_system_init_status = SYSTEM_INIT_STATUS_DRIVER_INIT;
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
    g_system_init_status = SYSTEM_INIT_STATUS_MIDDLEWARE_INIT;
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
    g_system_init_status = SYSTEM_INIT_STATUS_APPLICATION_INIT;

    // 最后标记完全初始化完成
    g_system_init_status = SYSTEM_INIT_STATUS_COMPLETED;
    g_system_initialized = true;
}


/*==========================================
 * 系统初始化协调器主函数
 * =========================================*/

uint32_t system_init_coordinator(void) {
    // 1. 系统硬件初始化（时钟、GPIO等）
    system_hal_init();

    // // 阶段1: _setup 阶段 (早期启动)
    system_setup_hal();
    system_setup_drivers();
    system_setup_middleware();
    system_setup_application();

    // // 阶段2: _init 阶段 (主机协议初始化后)
    system_init_hal();
    system_init_drivers();
    system_init_middleware();
    system_init_application();

    return 0;  // 成功
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
