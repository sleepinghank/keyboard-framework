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

#include "kb904/config_product.h"
//  application
#include "system_init.h"
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

#include "debug.h"
// middleware
#include "report_buffer.h"
#include "wireless.h"
#include "keyboard.h"
#include "transport.h"
#include "PMU.h"
#include "touchpad/touchpad.h"

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
#include "wait.h"

// ch584
#if (CHIP_TYPE == CHIP_CH584M)
#include "CONFIG.h"
#include "hal.h"
#endif

#include <string.h>

static volatile system_init_status_t g_system_init_status = SYSTEM_INIT_STATUS_NOT_STARTED;
static volatile bool g_system_initialized = false;

/*==========================================
 * 初始化阶段
 * =========================================*/

void system_init_hal(void) 
{
    // 硬件定时器初始化
    hw_timer_init();
    dprintf("Hardware timer initialized\r\n");

    // 标记HAL init完成
    g_system_init_status = SYSTEM_INIT_STATUS_HAL;
}

void system_init_drivers(void) 
{
    // 驱动层初始化阶段
    // 按依赖关系顺序初始化各驱动

    // 时钟初始化
    timer_init();

    // 3. 改成adc初始化 不立即进行电量采集会有数据异常，如果功耗有问题这里还能改一下，只计算校准值不初始化adc，把adc初始化反倒电量采集中
    battery_init();

    // 4. pwm初始化 (从 output_service 移入)，这部分应该只从flash中恢复背光状态，不进行实际点亮
    backlight_init(NULL);

    // 5. 指示灯初始化
    indicator_init();

    // 标记Driver init完成
    g_system_init_status = SYSTEM_INIT_STATUS_DRIVER;
}

void system_init_middleware(void) 
{
    // 中间件初始化阶段
    // 按依赖关系顺序初始化各中间件

    // 3. 报告缓冲区初始化
    report_buffer_init();


    // 5. 无线管理层初始化
    wireless_init();
    // 6. 键盘处理初始化
    keyboard_init();

    // 标记Middleware init完成
    g_system_init_status = SYSTEM_INIT_STATUS_MIDDLEWARE;
}


void system_init_application(void) 
{
    // 应用层初始化阶段
    // 初始化各应用服务
    pmu_init();             // PMU 电源管理初始化（在所有服务注册后，系统完全初始化前）
    system_service_init();
    input_service_init();   // 启动矩阵扫描定时器
    output_service_init();  // 背光实际点亮
    commu_service_init();
   
    //indicator_test();

    // 标记Application init完成
    g_system_init_status = SYSTEM_INIT_STATUS_APPLICATION;

    // 最后标记完全初始化完成
    g_system_init_status = SYSTEM_INIT_STATUS_COMPLETED;
    g_system_initialized = true;
}


/*==========================================
 * 系统初始化协调器主函数
 * =========================================*/

uint32_t system_init_coordinator(void) 
{
    // 1. 系统硬件初始化
    system_hal_init();

#if (PRINTF_ENABLE == TRUE)
    // 2. UART 初始化 - 用于调试输出，必须在 bt_driver_init 前初始化以确保蓝牙日志可用
    platform_uart_bind_pins(DEBUG_UART_RX, DEBUG_UART_TX, PRINTF_UART);
    platform_uart_init(PRINTF_UART, PRINTF_BAUDRATE, 0);
#endif  
    // 3. 存储系统初始化 
    storage_init();

    setPinOutput(A9);

    // 必须要先注册bt驱动
#ifdef BLUETOOTH_ENABLE_FLAG
    // 2. 蓝牙驱动初始化 - BLE 协议栈
    bt_driver_init(false);
#endif
    // init 阶段
    system_init_hal();
    system_init_drivers();
    system_init_middleware();
    system_init_application();

    return 0;
}

__HIGH_CODE
void peripheral_process()
{
    
    keyboard_task();
// writePinLow(A9);
#ifdef TOUCHPAD_ENABLE
    touchpad_task();
#endif
//    writePinHigh(A9);
    indicator_task();
    
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
