#pragma once

#include <stdint.h>

/**
 * @file system_service.h
 * @brief 系统服务事件定义
 *
 * 所有系统相关服务事件：
 * - 低电关机
 * - idle
 * - 关机
 * - 深度睡眠
 * - 存储
 * - 唤醒
 * - 恢复出厂设置
 * - ota
 * - LPM 调度事件
 */
typedef enum {
    /* 基础系统事件 (bits 0-7) */
    SYSTEM_LOW_BATTERY_SHUTDOWN_EVT = 1 << 0,  /**< 低电关机事件 */
    SYSTEM_IDLE_EVT                  = 1 << 1,  /**< 系统空闲事件（兼容旧接口） */
    SYSTEM_SHUTDOWN_EVT              = 1 << 2,  /**< 系统关机事件 */
    SYSTEM_DEEP_SLEEP_EVT            = 1 << 3,  /**< 深度睡眠事件 */
    SYSTEM_STORAGE_EVT               = 1 << 4,  /**< 系统存储事件 */
    SYSTEM_WAKEUP_EVT                = 1 << 5,  /**< 系统唤醒事件 */
    SYSTEM_FACTORY_RESET_EVT         = 1 << 6,  /**< 恢复出厂设置事件 */
    SYSTEM_OTA_EVT                   = 1 << 7,  /**< OTA升级事件 */

    /* LPM 调度事件（bits 8-15，不与现有事件冲突） */
    SYSTEM_LPM_IDLE_REQ_EVT          = 1 << 8,  /**< 请求进入 Idle（由 lpm_task 投递） */
    SYSTEM_LPM_DEEP_REQ_EVT          = 1 << 9,  /**< 请求进入 Deep（由 lpm_task 投递） */
    SYSTEM_LPM_STEP_DONE_EVT         = 1 << 10, /**< prepare 完成汇聚（由各 service 投递） */
    SYSTEM_LPM_ENTER_IDLE_EVT        = 1 << 11, /**< 最终进入 Idle（自投递，做二次确认） */
    SYSTEM_LPM_ENTER_DEEP_EVT        = 1 << 12, /**< 最终进入 Deep（自投递，做二次确认） */
    SYSTEM_LPM_WAKE_EVT              = 1 << 13, /**< 唤醒恢复入口（由 GPIO ISR 投递） */
} system_task_event_t;

extern uint8_t system_taskID;

uint16_t system_process_event(uint8_t task_id, uint16_t events);
void system_service_init(void);
