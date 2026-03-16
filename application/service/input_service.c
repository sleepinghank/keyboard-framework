#include "input_service.h"
#include "keyboard.h"
#include <stdint.h>
#include "event_manager.h"
#include "debug.h"
#include "battery.h"
#include "indicator.h"
#include "system_service.h"
#include "bt_driver.h"
#include "hw_timer.h"
#include "product_config.h"
#include "gpio.h"
#include "system_hal.h"

#ifdef TOUCH_EN
#include "touchpad_service.h"
#endif

/* 低电量阈值 */
#ifndef LOW_BATTERY_THRESHOLD
#define LOW_BATTERY_THRESHOLD  10  /* 10% 触发低电量警告 */
#endif

#ifndef CRITICAL_BATTERY_THRESHOLD
#define CRITICAL_BATTERY_THRESHOLD  5   /* 5% 触发关机 */
#endif

/* 电量检测周期 (ms) */
#ifndef BATTERY_DETECT_INTERVAL
#define BATTERY_DETECT_INTERVAL  30000  /* 30秒检测一次 */
#endif

/* 触控板看门狗检查周期 (ms) */
#ifndef TOUCH_WATCHDOG_INTERVAL
#define TOUCH_WATCHDOG_INTERVAL  2000   /* 2秒检测一次 */
#endif

#ifdef __cplusplus
extern "C" {
#endif

uint8_t input_taskID = 0;

/* 矩阵扫描标志位 - volatile 保证中断可见性 */
static volatile bool g_matrix_scan_flag = false;

/*==========================================
 * 矩阵扫描定时器回调
 *=========================================*/

/**
 * @brief 硬件定时器回调函数
 *        在定时器中断中被调用，设置矩阵扫描标志位
 */
__HIGH_CODE
static void matrix_scan_timer_callback(void)
{
    g_matrix_scan_flag = true;
}

/*==========================================
 * 矩阵扫描定时器接口实现
 *=========================================*/

/**
 * @brief 启动矩阵扫描定时器
 * @return error_code_t 错误码
 */

error_code_t matrix_scan_timer_start(void)
{
    return hw_timer_start(MATRIX_SCAN_TIMER_ID,
                          MATRIX_SCAN_INTERVAL_MS,
                          matrix_scan_timer_callback);
}

/**
 * @brief 停止矩阵扫描定时器
 * @return error_code_t 错误码
 */
error_code_t matrix_scan_timer_stop(void)
{
    return hw_timer_stop(MATRIX_SCAN_TIMER_ID);
}

/*==========================================
 * 矩阵扫描标志位接口实现
 *=========================================*/


__HIGH_CODE
bool input_get_matrix_scan_flag(void) {
    return g_matrix_scan_flag;
}

__HIGH_CODE
void input_clear_matrix_scan_flag(void) {
    g_matrix_scan_flag = false;
}

/**
 * @brief 输入服务事件处理器
 * @param task_id 任务ID
 * @param events 事件标志
 * @return 未处理的事件标志
 */
uint16_t input_process_event(uint8_t task_id, uint16_t events) {

    // 处理触控中断事件
    if (events & INPUT_TOUCH_INT_EVT) {
#ifdef TOUCH_EN
        touch_evt_task();
#endif
        return (events ^ INPUT_TOUCH_INT_EVT);
    }

    // 处理触控板看门狗检查事件
#ifdef TOUCH_EN
    if (events & INPUT_TOUCH_WATCHDOG_EVT) {
        touch_watchdog_check();
        return (events ^ INPUT_TOUCH_WATCHDOG_EVT);
    }
#endif

    // 处理电量检测事件
    if (events & INPUT_BATTERY_DETE_EVT) {
        // 读取电池电量
        uint8_t battery_level = battery_get_percentage();
        xprintf("Input: Battery level = %d%%\r\n", battery_level);

        // 更新蓝牙电池服务上报
        bt_driver_update_bat_level(battery_level);

        // 检查低电量状态
        if (battery_level <= CRITICAL_BATTERY_THRESHOLD) {
            // 电量极低，触发关机事件
            println("Input: Critical battery, triggering shutdown");
            extern uint8_t system_taskID;
            OSAL_SetEvent(system_taskID, SYSTEM_LOW_BATTERY_SHUTDOWN_EVT);
        } else if (battery_level <= LOW_BATTERY_THRESHOLD) {
            // 低电量警告，闪烁指示灯
            println("Input: Low battery warning");
            indicator_set(0, &IND_BLINK_FAST);
        }

        return (events ^ INPUT_BATTERY_DETE_EVT);
    }

    return 0;
}

/**
 * @brief 输入服务初始化
 */
void input_service_init(void) {
    /* 注册任务并获取任务ID */
    input_taskID = OSAL_ProcessEventRegister(input_process_event);
    dprintf("Input: Service initialized with task ID %d\r\n", input_taskID);
    /* 启动硬件定时器驱动的矩阵扫描 */
    matrix_scan_timer_start();
    dprintf("Input: Matrix scan timer started\r\n");
    /* 启动电量检测定时任务 */
    // OSAL_StartReloadTask(input_taskID, INPUT_BATTERY_DETE_EVT, BATTERY_DETECT_INTERVAL);

#ifdef TOUCH_EN
    /* 初始化触控板 */
    touch_power_on();
    dprintf("Input: Touchpad initialized\r\n");
    /* 启动触控板看门狗定时检查 */
    OSAL_StartReloadTask(input_taskID, INPUT_TOUCH_WATCHDOG_EVT, TOUCH_WATCHDOG_INTERVAL);
#endif
}

#ifdef __cplusplus
}
#endif

