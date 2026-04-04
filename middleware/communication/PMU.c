/******************************************************************************
 *
 *  Filename:      PMU.c
 *
 *  Description:   电源管理单元（Power Management Unit）
 *
 *                 职责：
 *                  - 维护 LPM（低功耗模式）状态，供 SLEEP.c HAL 层使用
 *                  - 每次按键扫描调用 PMU_Update() 刷新睡眠计时
 *                  - 超时后分两档触发睡眠（Idle / Deep）
 *                  - 实现外设进入/退出睡眠的硬件操作
 *                  - GPIO 中断唤醒后通过 PMU_WAKE_EVENT 恢复系统
 *
 ******************************************************************************/

#include "PMU.h"
#include "event_manager.h"
#include "system_service.h"
#include "communication_service.h"
#include "input_service.h"
#include "wireless.h"
#include "matrix.h"
#include "debug.h"
#include "kb904/config_product.h"
#include "CH58x_common.h"
#include "input_service.h"
#include "hidkbd.h"
#include "_bt_driver.h"

uint8_t pmu_taskID = 0;

/* ---- 睡眠超时（单位：TMOS ticks，1600 ticks ≈ 1 秒） ---- */
#define IDLE_SLEEP_TIMEOUT      (1600 * 10)        /* 5 秒无操作 → Idle 睡眠（调试用，正式建议 3 分钟） */
#define DEEP_SLEEP_TIMEOUT      (1600 * 60 * 10)  /* 10 分钟无操作 → 深度睡眠（已连接） */
#define ADV_DEEP_SLEEP_TIMEOUT  (1600 * 60)       /* 1 分钟无操作 → 深度睡眠（广播/未连接） */


/*********************************************************************
 * @fn      PMU_stop
 *
 * @brief   停止所有睡眠定时事件（切换到 USB 时调用）
 *          由 transport.c 在切换到 USB 模式时调用
 */
void PMU_stop(void)
{
    OSAL_StopTask(pmu_taskID, IDLE_SLEEP_EVENT);
    OSAL_StopTask(pmu_taskID, DEEP_SLEEP_EVENT);
}

/*********************************************************************
 * @fn      peripheral_enter_sleep
 *
 * @brief   外设进入睡眠前的硬件操作
 *          1. 关闭 ADC 电源
 *          2. 配置矩阵 GPIO 为唤醒中断模式（列拉低，行配置下降沿中断）
 *          3. 标记 LPM 状态为 DEEP_SLEEP，使 SLEEP.c 允许执行 LowPower_Sleep()
 *
 * @note    本函数返回后，下一次 TMOS 空闲时 CH58x_LowPower() 将调用 LowPower_Sleep()
 */
void peripheral_enter_sleep(void)
{
    // if (g_lpm_state == LPM_STATE_DEEP_SLEEP)
    // {
    //     return; /* 已处于睡眠准备态，防止 SYSTEM_ENTER_DEEP_EVT 与 CH58x_LowPower 双重调用 */
    // }

    PRINT("enter_sleep\n");

    /* 必须最先停止硬件定时器，防止 TMR0 中断在 matrix_prepare_wakeup() 之后
     * 触发 keyboard_task() → unselect_col()，将 COL 引脚从输出低变回输入上拉，
     * 破坏唤醒 GPIO 配置，导致按键无法产生下降沿中断唤醒 CPU */
    matrix_scan_timer_stop();
    input_clear_matrix_scan_flag();
    //关闭背光
    backlight_disable();
    if(access_state.idel_sleep_flag)
    {
        OSAL_StopTask(input_taskID, INPUT_BATTERY_DETE_EVT);
        OSAL_StartReloadTask(input_taskID, INPUT_BATTERY_DETE_EVT, BATTERY_DETECT_INTERVAL_IDLE);
    }
    else if(access_state.deep_sleep_flag)
    {
        OSAL_StopTask(input_taskID, INPUT_BATTERY_DETE_EVT);
        touchpad_power_off();
        setPinInputLow(B10);  // PB10 USB引脚
        setPinInputLow(B11);  // PB11 USB引脚
    }

    /* 配置矩阵 GPIO 为唤醒模式（列输出低，行下降沿中断） */
    matrix_prepare_wakeup();

}

/*********************************************************************
 * @fn      peripheral_exit_sleep
 *
 * @brief   从睡眠退出时的硬件恢复操作
 *          1. 立即将 LPM 状态恢复为 ACTIVE，阻止 SLEEP.c 再次进入 LowPower_Sleep
 *          2. 清除睡眠标志
 *          3. 恢复矩阵 GPIO 扫描模式
 *          4. 投递 SYSTEM_WAKE_EVT 触发上层（BLE 回连等）恢复逻辑
 *
 * @note    本函数具有幂等性：若已退出睡眠（g_lpm_state == ACTIVE）则直接返回
 */
void peripheral_exit_sleep(void)
{
    PRINT("exit\n");

    OSAL_SetEvent(system_taskID, SYSTEM_WAKE_EVT);
    /* 清除应用层睡眠标志 */
    access_state.sleep_en      = FALSE;
    access_state.idel_sleep_flag = FALSE;
    access_state.deep_sleep_flag = FALSE;

#if (PRINTF_ENABLE == TRUE)
    // 2. UART 初始化 - 用于调试输出，必须在 bt_driver_init 前初始化以确保蓝牙日志可用
    platform_uart_bind_pins(DEBUG_UART_RX, DEBUG_UART_TX, PRINTF_UART);
    platform_uart_init(PRINTF_UART, PRINTF_BAUDRATE, 0);
#endif 

    OSAL_StartReloadTask(input_taskID, INPUT_BATTERY_DETE_EVT, BATTERY_DETECT_INTERVAL);
    backlight_enable();
    indicator_init();
    /* 关闭矩阵行引脚的中断，恢复正常扫描 GPIO 状态 */
    matrix_resume_from_sleep();

    /* 重启矩阵扫描硬件定时器（与 peripheral_enter_sleep 中的停止对称） */
    matrix_scan_timer_start();


}

/*********************************************************************
 * @fn      PMU_Update
 *
 * @brief   每次按键扫描调用，刷新睡眠计时器
 *          - 若当前处于睡眠状态，立即执行退出睡眠（唤醒）
 *          - 否则根据无线连接状态重新调度睡眠超时事件
 *
 * @return  None.
 */
__HIGH_CODE
void PMU_Update(void)
{
    /* 睡眠中被唤醒 */
    // PRINT("PMU_Update: idel_sleep_flag=%d, deep_sleep_flag=%d\n", access_state.idel_sleep_flag, access_state.deep_sleep_flag);
    if (access_state.idel_sleep_flag || access_state.deep_sleep_flag)
    {
        peripheral_exit_sleep();
        /* 退出睡眠后继续向下执行，重新启动睡眠倒计时 */
    }

    /* 根据无线状态刷新睡眠定时事件 */
    wt_state_t state = wireless_get_state();
    // dprintf("PMU_Update: state=%d\n", state);
    // if (state == WT_CONNECTED)
    // {
    //     OSAL_StopTask(pmu_taskID, IDLE_SLEEP_EVENT);
    //     OSAL_StopTask(pmu_taskID, DEEP_SLEEP_EVENT);
    //     /* 已连接：调度两档睡眠 */
    //     OSAL_SetDelayedEvent(pmu_taskID, IDLE_SLEEP_EVENT, IDLE_SLEEP_TIMEOUT);
    //     OSAL_SetDelayedEvent(pmu_taskID, DEEP_SLEEP_EVENT, DEEP_SLEEP_TIMEOUT);
    // }
    // else if (state == WT_PARING || state == WT_RECONNECTING)
    // {
    //     /* 广播/回连中：只调度深度睡眠，超时更短 */
    //     OSAL_StopTask(pmu_taskID, IDLE_SLEEP_EVENT);
    //     OSAL_StopTask(pmu_taskID, DEEP_SLEEP_EVENT);
    //     OSAL_SetDelayedEvent(pmu_taskID, DEEP_SLEEP_EVENT, ADV_DEEP_SLEEP_TIMEOUT);
    // }
    // else
    // {
    //     /* 未连接/其他状态 */
    //     OSAL_StopTask(pmu_taskID, IDLE_SLEEP_EVENT);
    //     OSAL_StopTask(pmu_taskID, DEEP_SLEEP_EVENT);
    //     OSAL_SetDelayedEvent(pmu_taskID, DEEP_SLEEP_EVENT, ADV_DEEP_SLEEP_TIMEOUT);
    // }
}

/*********************************************************************
 * @fn      pmu_process_event
 *
 * @brief   PMU TMOS 任务事件处理
 *
 *          IDLE_SLEEP_EVENT  → 设置 idel_sleep_flag，触发 Idle 睡眠流程
 *          DEEP_SLEEP_EVENT  → 设置 deep_sleep_flag，触发 Deep 睡眠流程
 *
 * @param   task_id - 任务 ID
 * @param   events  - 事件标志位
 * @return  未处理的事件标志
 */
uint16_t pmu_process_event(uint8_t task_id, uint16_t events)
{
    if (events & IDLE_SLEEP_EVENT)
    {
        PRINT("@Idle_sleep\n"); 
        System_Enter_Idle_Sleep(); /* → OSAL_SetEvent(system_taskID, SYSTEM_ENTER_IDLE_EVT) */
        return events ^ IDLE_SLEEP_EVENT;
    }

    if (events & DEEP_SLEEP_EVENT)
    {
        PRINT("@Deep_sleep\n");
        System_Enter_Deep_Sleep(); /* → OSAL_SetEvent(system_taskID, SYSTEM_ENTER_DEEP_EVT) */
        return events ^ DEEP_SLEEP_EVENT;
    }

    return 0;
}

/*********************************************************************
 * @fn      pmu_init
 *
 * @brief   PMU 模块初始化，注册 TMOS 任务
 */
void pmu_init(void)
{
    access_state.sleep_en =FALSE;
    access_state.deep_sleep_flag = FALSE;
    access_state.idel_sleep_flag = FALSE;
    pmu_taskID  = OSAL_ProcessEventRegister(pmu_process_event);
    dprintf("PMU: initialized, taskID=%d\r\n", pmu_taskID);
}
