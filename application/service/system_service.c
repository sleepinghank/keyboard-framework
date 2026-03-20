//
// Created by 18494 on 2025/12/18.
//
#include "system_service.h"
#include <stdint.h>
#include "event_manager.h"
#include "debug.h"
#include "print.h"
#include "storage.h"
#include "battery.h"
#include "lpm.h"
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

#ifdef __cplusplus
extern "C" {
#endif

uint8_t system_taskID = 0xFF;
static bool g_factory_reset_pending = false;

#ifndef FACTORY_RESET_REBOOT_DELAY_MS
#define FACTORY_RESET_REBOOT_DELAY_MS  2000
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
        enter_power_mode(PM_SHUTDOWN);
        return (events ^ SYSTEM_LOW_BATTERY_SHUTDOWN_EVT);
    }

    // 处理系统空闲事件（保留兼容旧调用，实际由 LPM 调度替代）
    if (events & SYSTEM_IDLE_EVT) {
        /* 此事件已由 SYSTEM_LPM_IDLE_REQ_EVT 替代，保留为空以兼容旧调用 */
        dprintf("System: Legacy idle event received, ignored\r\n");
        return (events ^ SYSTEM_IDLE_EVT);
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
        enter_power_mode(PM_SHUTDOWN);
        return (events ^ SYSTEM_SHUTDOWN_EVT);
    }

    // 处理深度睡眠事件（保留兼容，转发到 LPM 调度）
    if (events & SYSTEM_DEEP_SLEEP_EVT) {
        /* 转发为 LPM Deep 请求，由 LPM 状态机处理 */
        dprintf("System: Legacy deep sleep event, forwarding to LPM\r\n");
        OSAL_SetEvent(system_taskID, SYSTEM_LPM_DEEP_REQ_EVT);
        return (events ^ SYSTEM_DEEP_SLEEP_EVT);
    }

    // 处理系统存储事件
    if (events & SYSTEM_STORAGE_EVT) {
        println("System: Storage operation");
        // 执行存储保存
        storage_save();
        return (events ^ SYSTEM_STORAGE_EVT);
    }

    // 处理系统唤醒事件
    if (events & SYSTEM_WAKEUP_EVT) {
        println("System: System wakeup");
        // 唤醒恢复流程:
        // 1. 从存储读取配置
        storage_init();
        // 2. 重置低功耗定时器
        lpm_timer_reset();
        // 3. 根据传输模式恢复连接
        if (get_transport() == TRANSPORT_BLUETOOTH) {
            wireless_connect();
        }
        return (events ^ SYSTEM_WAKEUP_EVT);
    }

    // 处理恢复出厂设置事件
    if (events & SYSTEM_FACTORY_RESET_EVT) {
        if (!g_factory_reset_pending) {
            println("System: Factory reset requested");
            g_factory_reset_pending = true;

            // 1. 断开无线连接
            wireless_disconnect();

            // 1. 指示灯反馈（至少 CAPS 闪 3 次；若有 BT 则一起闪）
            indicator_set(LED_CAPS, &IND_BLINK_3);
#ifdef LED_BT
            indicator_set(LED_BT, &IND_BLINK_3);
#endif

            // 2. 清除蓝牙配对记录
            bt_driver_clear_bonding();

            // 3. 重置存储配置
            storage_factory_reset();

            // 4. 背光重置为白光中档
            backlight_set_preset_color(BL_COLOR_WHITE);
            backlight_set_preset_level(BL_LEVEL_MEDIUM);

            // 5. 二阶段复位：等待灯效跑完后再执行系统复位
            if (OSAL_SetDelayedEvent(system_taskID, SYSTEM_FACTORY_RESET_EVT,
                                     FACTORY_RESET_REBOOT_DELAY_MS) != NO_ERROR) {
                dprintf("System: Factory reset delay scheduling failed, resetting now\r\n");
                g_factory_reset_pending = false;
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

    /*========================================
     * LPM 调度事件处理
     *========================================*/

    // 处理 Idle 睡眠请求
    if (events & SYSTEM_LPM_IDLE_REQ_EVT) {
        dprintf("System: Idle sleep requested\r\n");
        lpm_set_state(LPM_STATE_IDLE_PENDING);

        /* 扇出 prepare 事件到各 service */
        OSAL_SetEvent(input_taskID,  INPUT_LPM_PREPARE_EVT);
        OSAL_SetEvent(commu_taskID,  COMMU_LPM_PREPARE_EVT);
        /* output_service Idle 无操作，直接标记完成 */
        lpm_mark_prepare_done(LPM_PREPARE_OUTPUT);

        return (events ^ SYSTEM_LPM_IDLE_REQ_EVT);
    }

    // 处理 Deep 睡眠请求
    if (events & SYSTEM_LPM_DEEP_REQ_EVT) {
        dprintf("System: Deep sleep requested\r\n");
        lpm_set_state(LPM_STATE_DEEP_PENDING);

        OSAL_SetEvent(input_taskID,  INPUT_LPM_PREPARE_EVT);
        OSAL_SetEvent(commu_taskID,  COMMU_LPM_PREPARE_EVT);
        OSAL_SetEvent(output_taskID, OUTPUT_LPM_PREPARE_EVT);

        return (events ^ SYSTEM_LPM_DEEP_REQ_EVT);
    }

    // 处理 prepare 完成汇聚事件
    if (events & SYSTEM_LPM_STEP_DONE_EVT) {
        lpm_state_t cur = lpm_get_state();

        if (!lpm_all_prepare_done()) {
            /* 还有 service 未完成，继续等待 */
            return (events ^ SYSTEM_LPM_STEP_DONE_EVT);
        }

        /* 全部完成，投递最终进入事件（拆开汇聚与执行） */
        if (cur == LPM_STATE_IDLE_PENDING) {
            OSAL_SetEvent(system_taskID, SYSTEM_LPM_ENTER_IDLE_EVT);
        } else if (cur == LPM_STATE_DEEP_PENDING) {
            OSAL_SetEvent(system_taskID, SYSTEM_LPM_ENTER_DEEP_EVT);
        }

        return (events ^ SYSTEM_LPM_STEP_DONE_EVT);
    }

    // 处理最终进入 Idle 睡眠事件（二次确认）
    if (events & SYSTEM_LPM_ENTER_IDLE_EVT) {
        /* 二次确认：若有新活动则取消本次睡眠 */
        if (lpm_get_state() != LPM_STATE_IDLE_PENDING) {
            dprintf("System: Idle enter cancelled (state changed)\r\n");
            return (events ^ SYSTEM_LPM_ENTER_IDLE_EVT);
        }

        dprintf("System: Entering Idle sleep, waiting TMOS idleCB\r\n");
        lpm_set_state(LPM_STATE_IDLE_SLEEP);
        /* MCU 睡眠由 TMOS CH58x_LowPower() idleCB 自动触发，此处无需调用 LowPower_Sleep() */

        return (events ^ SYSTEM_LPM_ENTER_IDLE_EVT);
    }

    // 处理最终进入 Deep 睡眠事件（二次确认）
    if (events & SYSTEM_LPM_ENTER_DEEP_EVT) {
        if (lpm_get_state() != LPM_STATE_DEEP_PENDING) {
            dprintf("System: Deep enter cancelled (state changed)\r\n");
            return (events ^ SYSTEM_LPM_ENTER_DEEP_EVT);
        }

        dprintf("System: Entering Deep sleep, waiting TMOS idleCB\r\n");
        lpm_set_state(LPM_STATE_DEEP_SLEEP);

        return (events ^ SYSTEM_LPM_ENTER_DEEP_EVT);
    }

    // LPM 周期检查（1s 定时驱动，替代主循环轮询）
    if (events & SYSTEM_LPM_CHECK_EVT) {
        lpm_task();
        return (events ^ SYSTEM_LPM_CHECK_EVT);
    }

    // 处理唤醒恢复事件
    if (events & SYSTEM_LPM_WAKE_EVT) {
        lpm_mode_t mode = lpm_get_mode();
        dprintf("System: WAKE event, mode=%s\r\n", mode == LPM_MODE_DEEP ? "DEEP" : "IDLE");
        lpm_set_state(LPM_STATE_WAKE_RESUME);

        /* 先恢复输入侧（最关键，需要最快响应） */
        OSAL_SetEvent(input_taskID, INPUT_LPM_RESUME_EVT);

        if (mode == LPM_MODE_DEEP) {
            /* Deep 唤醒需要恢复输出侧和通信侧 */
            OSAL_SetEvent(output_taskID, OUTPUT_LPM_RESUME_EVT);
            OSAL_SetEvent(commu_taskID,  COMMU_LPM_RESUME_EVT);
        }
        /* Idle 唤醒：通信侧保持（BLE 未断连），output 无需恢复 */

        lpm_set_state(LPM_STATE_ACTIVE);
        lpm_timer_reset();
        dprintf("System: Wake complete, now ACTIVE\r\n");

        return (events ^ SYSTEM_LPM_WAKE_EVT);
    }

    return 0;
}

/**
 * @brief 系统服务初始化
 */
void system_service_init(void) {
    // 注册任务并获取任务ID
    system_taskID = OSAL_ProcessEventRegister(system_process_event);

    // lpm_init() 已在 system_init_middleware() 中调用，此处不重复初始化

    // 启动 LPM 周期检查任务（每 1000ms 调用一次 lpm_task）
    OSAL_StartReloadTask(system_taskID, SYSTEM_LPM_CHECK_EVT, 1000);
}

#ifdef __cplusplus
}
#endif
