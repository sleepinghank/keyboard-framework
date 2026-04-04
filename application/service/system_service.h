#pragma once

#include <stdint.h>

/**
 * @file system_service.h
 * @brief 系统服务事件定义
 *
 * 所有系统相关服务事件：
 * - 低电关机
 * - 关机
 * - 存储
 * - 恢复出厂设置
 * - OTA
 * - LPM 调度事件（Idle/Deep 睡眠、唤醒）
 */
/* 基础系统事件 (bits 0-7) */
#define SYSTEM_LOW_BATTERY_SHUTDOWN_EVT (1 << 0)  /* 低电关机事件 */
/* bit 1 预留 */
#define SYSTEM_SHUTDOWN_EVT             (1 << 2)  /* 系统关机事件 */
/* bit 3 预留 */
#define SYSTEM_STORAGE_EVT              (1 << 4)  /* 系统存储事件 */
/* bit 5 预留，请使用 SYSTEM_LPM_WAKE_EVT */
#define SYSTEM_FACTORY_RESET_EVT        (1 << 6)  /* 恢复出厂设置事件 */
#define SYSTEM_OTA_EVT                  (1 << 7)  /* OTA升级事件 */


#define SYSTEM_ENTER_IDLE_EVT       (1 << 11)  /* 进入 Idle */
#define SYSTEM_ENTER_DEEP_EVT       (1 << 12)  /* 进入 Deep */
#define SYSTEM_WAKE_EVT             (1 << 13)  /* 唤醒恢复入口 */

extern uint8_t system_taskID;

uint16_t system_process_event(uint8_t task_id, uint16_t events);
void system_service_init(void);

void System_Enter_Idle_Sleep( void );
void System_Enter_Deep_Sleep( void );
void System_Wake_Up( void );
