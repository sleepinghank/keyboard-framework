//
// Created by 18494 on 2025/12/18.
//
#include "system_service.h"
#include <stdint.h>
#include "kb904/config_product.h"
#include "CH58x_common.h"
#include "event_manager.h"
#include "debug.h"
#include "print.h"
#include "storage.h"
#include "battery.h"
#include "wireless.h"
#include "transport.h"
#include "system_hal.h"
#include "bt_driver.h"
#include "input_service.h"
#include "communication_service.h"
#include "output_service.h"
#include "indicator.h"
#include "kb904/config_hw.h"
#include "backlight.h"
#include "PMU.h"
#include "hidkbd.h"
#include "_bt_driver.h"

/* 等待 BLE 断连完成的重试延迟（约 30ms，与参考工程 DISCONNECT_WAIT_REPORT_END_TIMEOUT 对应） */
#define DISCONNECT_WAIT_TIMEOUT  48

#ifdef __cplusplus
extern "C" {
#endif

uint8_t system_taskID = 0xFF;
static bool g_factory_reset_pending = false;

#ifndef FACTORY_RESET_REBOOT_DELAY_MS
#define FACTORY_RESET_REBOOT_DELAY_MS  16*2000
#endif

/**
 * @brief 系统服务事件处理器
 * @param task_id 任务ID
 * @param events 事件标志
 * @return 未处理的事件标志
 */
uint16_t system_process_event(uint8_t task_id, uint16_t events) {

    // 处理低电关机事件
    if (events & SYSTEM_LOW_BATTERY_SHUTDOWN_EVT) {
        println("System: Low battery shutdown");
        // 低电关机流程:
        // 1. 保存当前配置到存储
        storage_save();
        // 2. 关闭所有指示灯
        indicator_off_all();
        // 3. 断开无线连接
        wireless_disconnect();
        // 4. 进入关机模式
        //enter_power_mode(PM_SHUTDOWN);
        return (events ^ SYSTEM_LOW_BATTERY_SHUTDOWN_EVT);
    }

    // 处理系统关机事件
    if (events & SYSTEM_SHUTDOWN_EVT) {
        println("System: System shutdown");
        // 安全关机流程:
        // 1. 保存所有配置
        storage_save();
        // 2. 关闭指示灯
        indicator_off_all();
        // 3. 断开无线连接
        wireless_disconnect();
        // 4. 执行关机
        //enter_power_mode(PM_SHUTDOWN);
        return (events ^ SYSTEM_SHUTDOWN_EVT);
    }

    // 处理系统存储事件
    if (events & SYSTEM_STORAGE_EVT) {
        println("System: Storage operation");
        // 执行存储保存
        storage_save();
        return (events ^ SYSTEM_STORAGE_EVT);
    }

    // 处理恢复出厂设置事件
    if (events & SYSTEM_FACTORY_RESET_EVT) {
        if (!g_factory_reset_pending) {
            println("System: Factory reset requested");
            g_factory_reset_pending = true;

            // 1. 断开无线连接
            wireless_disconnect();

            // 2. 清除蓝牙配对记录
            bt_driver_clear_bonding();

            // 3. 重置存储配置
            storage_factory_reset();

            // // 4. 背光重置为白光中档
            backlight_set_preset_color(BL_COLOR_WHITE);
            backlight_set_preset_level(BL_LEVEL_MEDIUM);

            // 5. 指示灯闪烁提示用户已进入工厂重置流程
            indicator_set(LED_CAPS, &IND_BLINK_3);
            indicator_set(LED_POWER_RED, &IND_BLINK_3);
            indicator_set(LED_BT, &IND_BLINK_3);

            // 6. 二阶段复位：等待灯效跑完后再执行系统复位
            if (OSAL_SetDelayedEvent(system_taskID, SYSTEM_FACTORY_RESET_EVT,
                                     FACTORY_RESET_REBOOT_DELAY_MS) != NO_ERROR) {
                dprintf("System: Factory reset delay scheduling failed, resetting now\r\n");
                system_hal_reset();
            }
        } else {
            if (OSAL_GetTaskTimer(system_taskID, SYSTEM_FACTORY_RESET_EVT) > 0) {
                dprintf("System: Factory reset already pending\r\n");
                return (events ^ SYSTEM_FACTORY_RESET_EVT);
            }
            println("System: Factory reset reboot");
            g_factory_reset_pending = false;
            system_hal_reset();
        }
        return (events ^ SYSTEM_FACTORY_RESET_EVT);
    }

    // 处理OTA升级事件
    if (events & SYSTEM_OTA_EVT) {
        println("System: OTA update");
        // OTA升级流程:
        // 1. 断开当前连接
        wireless_disconnect();
        // 2. 进入OTA模式（具体实现依赖平台）
        // 注意：OTA模式通常需要跳转到bootloader
        return (events ^ SYSTEM_OTA_EVT);
    }

    // 处理最终进入 Idle 睡眠事件
    if (events & SYSTEM_ENTER_IDLE_EVT) {
        println("System: Enter Idle sleep");

        access_state.sleep_en = TRUE;
        access_state.idel_sleep_flag = TRUE;

        // /* 关闭指示灯 */
         //indicator_off_all();

        // /* 进入硬件睡眠（配置 GPIO 唤醒，使能 LowPower_Sleep） */
         peripheral_enter_sleep();

        return (events ^ SYSTEM_ENTER_IDLE_EVT);
    }

    // 处理最终进入 Deep 睡眠事件
    if (events & SYSTEM_ENTER_DEEP_EVT) {
        println("System: Enter Deep sleep");

        access_state.sleep_en =TRUE;
        access_state.deep_sleep_flag = TRUE;

        /* 停止 Idle 睡眠事件（防止 Idle/Deep 双重触发） */
        OSAL_StopTask(pmu_taskID, IDLE_SLEEP_EVENT);//可以尝试注释看看
        OSAL_StopTask(system_taskID, SYSTEM_ENTER_IDLE_EVT);
        OSAL_StopTask(system_taskID, SYSTEM_WAKE_EVT);
        // TODO: 应该调用communication_service 事件，或者封装方法
        /* 检查是否处于有效 BLE 通道 */
        //if ((access_state.ble_idx > BLE_INDEX_IDEL) && (access_state.ble_idx < BLE_INDEX_MAX))
        {
            wt_state_t wt = wireless_get_state();
            dprintf("System: Enter Deep sleep: wt=%d\n", wt);
            if (wt == WT_CONNECTED)
            {
                /* BLE 仍连接中：先断连，延迟重试等待断连完成 */
                OSAL_SetEvent(commu_taskID, WL_REQ_DISCONNECT);
                OSAL_SetDelayedEvent(system_taskID, SYSTEM_ENTER_DEEP_EVT, DISCONNECT_WAIT_TIMEOUT);
                return (events ^ SYSTEM_ENTER_DEEP_EVT);
            }
            else if (wt == WT_PARING || wt == WT_RECONNECTING)
            {
                /* 广播/回连广播中：先停止，延迟重试等待停止完成 */
                OSAL_SetEvent(commu_taskID, WL_REQ_DISCONNECT);
                OSAL_SetDelayedEvent(system_taskID, SYSTEM_ENTER_DEEP_EVT, DISCONNECT_WAIT_TIMEOUT);
                return (events ^ SYSTEM_ENTER_DEEP_EVT);
            }
        }

        /* 关闭指示灯 */
        indicator_off_all();

        // /* 进入硬件深度睡眠 */
        peripheral_enter_sleep();

        return (events ^ SYSTEM_ENTER_DEEP_EVT);
    }

    // 处理唤醒恢复事件
    if (events & SYSTEM_WAKE_EVT) {
        println("System: Wake resume");

        /* 深度睡眠唤醒：触发无线回连
         * wireless 处于 WT_SUSPEND 时需主动发起 WL_REQ_RECONNECT，
         * 否则 BLE 永远不会重新广播，按键报告无法发出 */
        wt_state_t wt = wireless_get_state();
        dprintf("System: Wake resume wt=%d\n", wt);
        if (wt == WT_SUSPEND || wt == WT_DISCONNECTED) {
            OSAL_SetEvent(commu_taskID, WL_REQ_RECONNECT);
        }

        access_state.sleep_en      = FALSE;
        access_state.idel_sleep_flag = FALSE;
        access_state.deep_sleep_flag = FALSE;
        //OSAL_SetDelayedEvent(input_taskID, INPUT_BATTERY_DETE_EVT, 100);

        return (events ^ SYSTEM_WAKE_EVT);
    }

    return 0;
}

/*********************************************************************
 * @fn      System_Enter_Idle_Sleep
 *
 * @brief
 *
 * @return  none
 */
void System_Enter_Idle_Sleep( void )
{
    OSAL_SetEvent(system_taskID, SYSTEM_ENTER_IDLE_EVT);
}

/*********************************************************************
 * @fn      System_Enter_Deep_Sleep
 *
 * @brief
 *
 * @return  none
 */
void System_Enter_Deep_Sleep( void )
{
    OSAL_SetEvent(system_taskID, SYSTEM_ENTER_DEEP_EVT);
}

/*********************************************************************
 * @fn      System_Wake_Up
 *
 * @brief
 *
 * @return  none
 */
void System_Wake_Up( void )
{
    OSAL_SetEvent(system_taskID, SYSTEM_WAKE_EVT);
}

/**
 * @brief 系统服务初始化
 */
void system_service_init(void) {
    // 注册任务并获取任务ID
    system_taskID = OSAL_ProcessEventRegister(system_process_event);
    dprintf("System: Service initialized with task ID %d\r\n", system_taskID);

}

#ifdef __cplusplus
}
#endif
