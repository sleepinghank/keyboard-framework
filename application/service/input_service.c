#include "input_service.h"
#include "keyboard.h"
#include <stdint.h>
#include "event_manager.h"
#include "debug.h"
#include "battery.h"
#include "output_service.h"
#include "system_service.h"
#include "bt_driver.h"
#include "hw_timer.h"
#include "kb904/config_product.h"
#include "gpio.h"
#include "system_hal.h"
#include "matrix.h"

#if (CHIP_TYPE == CHIP_CH584M)
#include "CH58x_common.h"
#endif

#ifdef TOUCHPAD_ENABLE
#include "touchpad/touchpad.h"
#endif

/* 低电量阈值 */
#ifndef LOW_BATTERY_THRESHOLD
#define LOW_BATTERY_THRESHOLD  20  /* 20% 触发低电量警告 */
#endif

#ifndef CRITICAL_BATTERY_THRESHOLD
#define CRITICAL_BATTERY_THRESHOLD  5   /* 5% 触发关机 */
#endif



#ifdef __cplusplus
extern "C" {
#endif

uint8_t input_taskID = 0;

/* 矩阵扫描标志位 - volatile 保证中断可见性 */
static volatile bool g_matrix_scan_flag = false;
/* 矩阵扫描标志位 - volatile 保证中断可见性 */
static volatile bool g_touchpad_int_flag = false;

/* 唤醒原因（由 GPIO ISR 锁存） */
static volatile lpm_wakeup_source_t g_last_wakeup_source = LPM_WAKEUP_NONE;
static uint8_t g_last_battery_power_state = 0xFF;

lpm_wakeup_source_t input_get_last_wakeup_source(void) {
    return g_last_wakeup_source;
}
void input_set_last_wakeup_source(lpm_wakeup_source_t source) {
    g_last_wakeup_source = source;
    return;
}

/*==========================================
 * 矩阵扫描定时器回调
 *=========================================*/

/**
 * @brief 硬件定时器回调函数
 *        在定时器中断中被调用，设置矩阵扫描标志位
 */
__HIGH_CODE
void matrix_scan_timer_callback(void)
{
    g_matrix_scan_flag = true;
}
__HIGH_CODE
bool input_get_matrix_scan_flag(void) {
    return g_matrix_scan_flag;
}

__HIGH_CODE
void input_clear_matrix_scan_flag(void) {
    g_matrix_scan_flag = false;
}

__HIGH_CODE
void keyboard_task(){
    if (input_get_matrix_scan_flag()) {
        input_clear_matrix_scan_flag(); 
        keyboard_scan();
    } else if (g_last_wakeup_source == LPM_WAKEUP_MATRIX) {
        g_last_wakeup_source = LPM_WAKEUP_NONE;  // 清除唤醒原因，防止重复处理
        PMU_Update();
    }
}

/*==========================================
 * 触控板中断标志位接口
 *=========================================*/

bool input_get_touchpad_int_flag(void) {
    return g_touchpad_int_flag;
}

void input_set_touchpad_int_flag(bool val) {
    g_touchpad_int_flag = val;
}

void input_clear_touchpad_int_flag(void) {
    g_touchpad_int_flag = false;
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

/**
 * @brief 输入服务事件处理器
 * @param task_id 任务ID
 * @param events 事件标志
 * @return 未处理的事件标志
 */
uint16_t input_process_event(uint8_t task_id, uint16_t events) {

    if (events & INPUT_TOUCH_TOGGLE_EVT) 
    {
            LOG_I("[INPUT] touch toggle event");
        // TODO: 触摸板锁定/解锁逻辑，待 touchpad 模块接口稳定后实现
        dprintf("Input: Touch toggle event received (not implemented)\r\n");
        return (events ^ INPUT_TOUCH_TOGGLE_EVT);
    }

    // 处理电量检测事件
    if (events & INPUT_BATTERY_DETE_EVT) 
    {
        LOG_I("[INPUT] battery detect event");
        battery_task();

        // 读取电池电量 太复杂待更新
        uint8_t battery_level = battery_get_percentage();
        xprintf("Input: Battery level = %d%%\r\n", battery_level);

        // 更新蓝牙电池服务上报
        bt_driver_update_bat_level(battery_level);

        // 基于 power state 做迁移，避免重复触发
        uint8_t current_power_state = battery_get_power_state();

        if (g_last_battery_power_state == 0xFF) {
            g_last_battery_power_state = current_power_state;

            if (current_power_state == BAT_POWER_LOW) {
                LOG_W("[INPUT] low battery");
                output_service_request_indicator(IND_REQ_LOW_BATTERY, 0);
            } else if ((current_power_state == BAT_POWER_CRITICAL_LOW) ||
                       (current_power_state == BAT_POWER_SHUTDOWN)) {
                LOG_W("[INPUT] critical battery, shutdown");
                OSAL_SetEvent(system_taskID, SYSTEM_LOW_BATTERY_SHUTDOWN_EVT);
            }
        } else if (current_power_state != g_last_battery_power_state) {
            g_last_battery_power_state = current_power_state;

            if (current_power_state == BAT_POWER_LOW) {
                LOG_W("[INPUT] low battery");
                output_service_request_indicator(IND_REQ_LOW_BATTERY, 0);
            } else if (current_power_state == BAT_POWER_NORMAL) {
                output_service_request_indicator(IND_REQ_BATTERY_NORMAL, 0);
            } else if ((current_power_state == BAT_POWER_CRITICAL_LOW) ||
                       (current_power_state == BAT_POWER_SHUTDOWN)) {
                LOG_W("[INPUT] critical battery, shutdown");
                OSAL_SetEvent(system_taskID, SYSTEM_LOW_BATTERY_SHUTDOWN_EVT);
            }
        } else if ((current_power_state == BAT_POWER_CRITICAL_LOW) ||
                   (current_power_state == BAT_POWER_SHUTDOWN)) {
            // 保持原有 critical shutdown 语义
            LOG_W("[INPUT] critical battery, shutdown");
            OSAL_SetEvent(system_taskID, SYSTEM_LOW_BATTERY_SHUTDOWN_EVT);
        }

        return (events ^ INPUT_BATTERY_DETE_EVT);
    }
    #ifdef TOUCHPAD_ENABLE
    if (events & INPUT_TOUCH_ON_EVT) 
    {
        LOG_I("[INPUT] touch on event");
        OSAL_SetEvent(touchpad_taskID,TOUCHPAD_INIT_EVT);
        return (events ^ INPUT_TOUCH_ON_EVT);
    }
    if (events & INPUT_TOUCH_OFF_EVT) 
    {
        LOG_I("[INPUT] touch off event");
        return (events ^ INPUT_TOUCH_OFF_EVT);
    }
    #endif
    return 0;
}

/**
 * @brief 输入服务初始化
 */
void input_service_init(void) {
    /* 注册任务并获取任务ID */
    input_taskID = OSAL_ProcessEventRegister(input_process_event);

    g_last_battery_power_state = 0xFF;
    /* 启动硬件定时器驱动的矩阵扫描 */
    matrix_scan_timer_start();
    /* 启动电量检测定时任务 */
    OSAL_StartReloadTask(input_taskID, INPUT_BATTERY_DETE_EVT, BATTERY_DETECT_INTERVAL);

#ifdef TOUCHPAD_ENABLE
    /* 初始化触控板 */
    touchpad_setup();
    OSAL_SetEvent(input_taskID, INPUT_TOUCH_ON_EVT);
#endif
}

#ifdef __cplusplus
}
#endif
