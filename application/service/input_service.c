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
#include "kb904/config.h"
#include "gpio.h"
#include "system_hal.h"
#include "lpm.h"
#include "matrix.h"
#include "CH58x_common.h"

#ifdef TOUCH_EN
#include "touchpad_service.h"
#endif

/* 低电量阈值 */
#ifndef LOW_BATTERY_THRESHOLD
#define LOW_BATTERY_THRESHOLD  20  /* 20% 触发低电量警告 */
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

/* 唤醒原因（由 GPIO ISR 锁存） */
static volatile lpm_wakeup_source_t g_last_wakeup_source = LPM_WAKEUP_NONE;
static uint8_t g_last_battery_power_state = 0xFF;

lpm_wakeup_source_t input_get_last_wakeup_source(void) {
    return g_last_wakeup_source;
}

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
        battery_task();

        // 读取电池电量
        uint8_t battery_level = battery_get_percentage();
        xprintf("Input: Battery level = %d%%\r\n", battery_level);

        // 更新蓝牙电池服务上报
        bt_driver_update_bat_level(battery_level);

        // 基于 power state 做迁移，避免重复触发
        uint8_t current_power_state = battery_get_power_state();

        if (g_last_battery_power_state == 0xFF) {
            g_last_battery_power_state = current_power_state;

            if (current_power_state == BAT_POWER_LOW) {
                println("Input: Low battery warning");
                output_service_request_indicator(IND_REQ_LOW_BATTERY, 0);
            } else if ((current_power_state == BAT_POWER_CRITICAL_LOW) ||
                       (current_power_state == BAT_POWER_SHUTDOWN)) {
                println("Input: Critical battery, triggering shutdown");
                OSAL_SetEvent(system_taskID, SYSTEM_LOW_BATTERY_SHUTDOWN_EVT);
            }
        } else if (current_power_state != g_last_battery_power_state) {
            g_last_battery_power_state = current_power_state;

            if (current_power_state == BAT_POWER_LOW) {
                println("Input: Low battery warning");
                output_service_request_indicator(IND_REQ_LOW_BATTERY, 0);
            } else if (current_power_state == BAT_POWER_NORMAL) {
                println("Input: Battery normal");
                output_service_request_indicator(IND_REQ_BATTERY_NORMAL, 0);
            } else if ((current_power_state == BAT_POWER_CRITICAL_LOW) ||
                       (current_power_state == BAT_POWER_SHUTDOWN)) {
                println("Input: Critical battery, triggering shutdown");
                OSAL_SetEvent(system_taskID, SYSTEM_LOW_BATTERY_SHUTDOWN_EVT);
            }
        } else if ((current_power_state == BAT_POWER_CRITICAL_LOW) ||
                   (current_power_state == BAT_POWER_SHUTDOWN)) {
            // 保持原有 critical shutdown 语义
            println("Input: Critical battery, triggering shutdown");
            OSAL_SetEvent(system_taskID, SYSTEM_LOW_BATTERY_SHUTDOWN_EVT);
        }

        return (events ^ INPUT_BATTERY_DETE_EVT);
    }

    /*========================================
     * LPM prepare/resume 事件处理
     *========================================*/

    // 处理 LPM prepare 事件（Idle/Deep 通用，mode 通过 lpm_get_mode() 查询）
    if (events & INPUT_LPM_PREPARE_EVT) {
        lpm_mode_t mode = lpm_get_mode();
        dprintf("Input: LPM prepare start (mode=%d)\r\n", mode);

        /* 1. 停止矩阵扫描定时器 */
        matrix_scan_timer_stop();

        /* 2. 配置矩阵 GPIO 为唤醒中断模式（COL 拉低，ROW 配下降沿中断） */
        matrix_prepare_wakeup();

        /* 3. PA2 电源键：保留独立唤醒中断（三步序列，顺序不可颠倒） */
        GPIOA_ModeCfg(GPIO_Pin_2, GPIO_ModeIN_PU);
        GPIOA_ITModeCfg(GPIO_Pin_2, GPIO_ITMode_FallEdge);
        GPIOA_ClearITFlagBit(GPIO_Pin_2);   /* 最后清标志，防止配置过程产生假中断 */

#ifdef TOUCH_EN
        if (mode == LPM_MODE_IDLE) {
            /* 触控低功耗，保留 INT 唤醒 */
            // touch_prepare_idle_sleep();
        } else {
            /* 触控更深低功耗，保留 INT 唤醒 */
            // touch_prepare_deep_sleep();
        }
#endif

        /* 4. 暂停 ADC 电量采样 */
        OSAL_StopTask(input_taskID, INPUT_BATTERY_DETE_EVT);

        /* 5. 标记 input prepare 完成，通知 system_service 汇聚 */
        lpm_mark_prepare_done(LPM_PREPARE_INPUT);
        OSAL_SetEvent(system_taskID, SYSTEM_LPM_STEP_DONE_EVT);

        dprintf("Input: LPM prepare done\r\n");
        return (events ^ INPUT_LPM_PREPARE_EVT);
    }

    // 处理 LPM resume 事件
    if (events & INPUT_LPM_RESUME_EVT) {
        dprintf("Input: LPM resume start\r\n");

        /* 1. 恢复矩阵 GPIO 为正常扫描模式 */
        matrix_resume_from_sleep();

        /* 2. PA2 恢复（三步序列，顺序不可颠倒） */
        GPIOA_ModeCfg(GPIO_Pin_2, GPIO_ModeIN_PU);
        GPIOA_ITModeCfg(GPIO_Pin_2, GPIO_ITMode_FallEdge);
        GPIOA_ClearITFlagBit(GPIO_Pin_2);

#ifdef TOUCH_EN
        // touch_resume_from_sleep();
#endif

        /* 3. 补一次矩阵扫描（防止首键丢失） */
        matrix_scan_once_after_wakeup();

        /* 4. 重启矩阵扫描定时器 */
        matrix_scan_timer_start();

        /* 5. 恢复 ADC 电量采样 */
        OSAL_StartReloadTask(input_taskID, INPUT_BATTERY_DETE_EVT, BATTERY_DETECT_INTERVAL);

        /* 6. 重置唤醒原因 */
        g_last_wakeup_source = LPM_WAKEUP_NONE;

        dprintf("Input: LPM resume done\r\n");
        return (events ^ INPUT_LPM_RESUME_EVT);
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
    g_last_battery_power_state = 0xFF;
    /* 启动硬件定时器驱动的矩阵扫描 */
    matrix_scan_timer_start();
    dprintf("Input: Matrix scan timer started\r\n");
    /* 启动电量检测定时任务 */
    OSAL_StartReloadTask(input_taskID, INPUT_BATTERY_DETE_EVT, BATTERY_DETECT_INTERVAL);

#ifdef TOUCH_EN
    /* 初始化触控板 */
    touch_power_on();
    dprintf("Input: Touchpad initialized\r\n");
    /* 启动触控板看门狗定时检查 */
    OSAL_StartReloadTask(input_taskID, INPUT_TOUCH_WATCHDOG_EVT, TOUCH_WATCHDOG_INTERVAL);
#endif
}

/*==========================================
 * GPIO 唤醒中断服务程序
 * 在睡眠期间由 GPIO 下降沿触发，锁存唤醒原因并投递 SYSTEM_LPM_WAKE_EVT
 *=========================================*/

__INTERRUPT
__HIGH_CODE
void GPIOA_IRQHandler(void) {
    uint32_t flags = R16_PA_INT_IF;
    R16_PA_INT_IF = flags;  /* 清除中断标志，防止重入 */

    if (lpm_is_in_sleep()) {
        /* PA2（GPIO_Pin_2 = bit2）为电源键，其余为矩阵 ROW 引脚 */
        g_last_wakeup_source = (flags & GPIO_Pin_2) ? LPM_WAKEUP_PA2 : LPM_WAKEUP_MATRIX;
        OSAL_SetEvent(system_taskID, SYSTEM_LPM_WAKE_EVT);
    }
}

__INTERRUPT
__HIGH_CODE
void GPIOB_IRQHandler(void) {
    uint32_t flags = R16_PB_INT_IF;
    R16_PB_INT_IF = flags;  /* 清除中断标志，防止重入 */

    if (lpm_is_in_sleep()) {
        g_last_wakeup_source = LPM_WAKEUP_MATRIX;
        OSAL_SetEvent(system_taskID, SYSTEM_LPM_WAKE_EVT);
    }
}

#ifdef __cplusplus
}
#endif
