#pragma once

#include <stdint.h>
/**
 * 所有系统相关服务事件
 * 低电关机
 * idle
 * 关机
 * 深度睡眠
 * 存储
 * 唤醒
 * 恢复出厂设置
 * ota
 */
typedef enum {
    SYSTEM_LOW_BATTERY_SHUTDOWN_EVT = 1 << 0,  /**< 低电关机事件 */
    SYSTEM_IDLE_EVT                  = 1 << 1,  /**< 系统空闲事件 */
    SYSTEM_SHUTDOWN_EVT              = 1 << 2,  /**< 系统关机事件 */
    SYSTEM_DEEP_SLEEP_EVT            = 1 << 3,  /**< 深度睡眠事件 */
    SYSTEM_STORAGE_EVT               = 1 << 4,  /**< 系统存储事件 */
    SYSTEM_WAKEUP_EVT                = 1 << 5,  /**< 系统唤醒事件 */
    SYSTEM_FACTORY_RESET_EVT         = 1 << 6,  /**< 恢复出厂设置事件 */
    SYSTEM_OTA_EVT                   = 1 << 7,  /**< OTA升级事件 */
} system_task_event_t;

extern uint8_t system_taskID;

uint16_t system_process_event(uint8_t task_id, uint16_t events);
void system_service_init(void);
