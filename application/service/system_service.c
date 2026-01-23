//
// Created by 18494 on 2025/12/18.
//
#include "system_service.h"
#include <stdint.h>
#include "event_manager.h"
#include "print.h"
#include "storage.h"
#include "battery.h"
#include "indicator.h"
#include "lpm.h"
#include "wireless.h"
#include "transport.h"
#include "system_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t system_taskID = 0;

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

    // 处理系统空闲事件
    if (events & SYSTEM_IDLE_EVT) {
        println("System: System idle detected");
        // 系统空闲处理:
        // 1. 关闭背光灯节省功耗
        indicator_off_all();
        // 2. 进入轻度睡眠模式
        enter_power_mode(PM_SLEEP);
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

    // 处理深度睡眠事件
    if (events & SYSTEM_DEEP_SLEEP_EVT) {
        println("System: Enter deep sleep");
        // 深度睡眠流程:
        // 1. 保存关键状态
        storage_save();
        // 2. 关闭指示灯
        indicator_off_all();
        // 3. 进入深度睡眠（保留RAM）
        enter_power_mode(PM_STANDBY_WITH_RAM);
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
        println("System: Factory reset");
        // 恢复出厂设置:
        // 1. 断开所有连接
        wireless_disconnect();
        // 2. 重置存储配置
        storage_factory_reset();
        // 3. 执行系统复位
        system_hal_reset();
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

    return 0;
}

/**
 * @brief 系统服务初始化
 */
void system_service_init(void) {
    // 注册任务并获取任务ID
    system_taskID = OSAL_ProcessEventRegister(system_process_event);

    // TODO: 根据配置启动相应的定时任务
    // 例如：空闲检测、电池检测等
    // OSAL_StartReloadTask(system_taskID, SYSTEM_IDLE_CHECK_EVT, 10000);
}

#ifdef __cplusplus
}
#endif

