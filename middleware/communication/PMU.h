#pragma once
#include <stdbool.h>
#include <stdint.h>


/* ---- PMU TMOS 事件定义 ---- */
#define IDLE_SLEEP_EVENT    (1 << 8)   /* Idle 睡眠超时事件 */
#define DEEP_SLEEP_EVENT    (1 << 9)   /* 深度睡眠超时事件 */

/*===========================================
 * 函数声明
 *==========================================*/

/* TMOS 任务接口 */
void     pmu_init(void);
uint16_t pmu_process_event(uint8_t task_id, uint16_t events);

/* 睡眠计时器控制（供 transport.c 在切换到 USB 时调用） */
void PMU_Update(void);
void PMU_stop(void);


/* 外设级睡眠/唤醒硬件操作（供 system_service.c 调用） */
void peripheral_enter_sleep(void);
void peripheral_exit_sleep(void);

extern uint8_t pmu_taskID;
