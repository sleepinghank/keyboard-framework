/* Copyright 2022 @ lokher (https://www.keychron.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include <stdbool.h>
#include <stdint.h>

/* ---- 保留：非睡眠类电源模式（关机/低电路径仍使用） ---- */
typedef enum {
    PM_RUN,
    PM_LOW_POWER_RUN,
    PM_SLEEP,
    PM_LOW_POWER_SLEEP,
    PM_STOP0,
    PM_STOP1,
    PM_STOP2,
    PM_STANDBY_WITH_RAM,
    PM_STANDBY,
    PM_SHUTDOWN
} pm_t;

/* ---- 新增：LPM 状态 ---- */
typedef enum {
    LPM_STATE_ACTIVE        = 0,
    LPM_STATE_IDLE_PENDING  = 1,
    LPM_STATE_IDLE_SLEEP    = 2,
    LPM_STATE_DEEP_PENDING  = 3,
    LPM_STATE_DEEP_SLEEP    = 4,
    LPM_STATE_WAKE_RESUME   = 5,
} lpm_state_t;

/* ---- 新增：睡眠深度（供 service 查询） ---- */
typedef enum {
    LPM_MODE_NONE  = 0,
    LPM_MODE_IDLE  = 1,
    LPM_MODE_DEEP  = 2,
} lpm_mode_t;

/* ---- 新增：prepare 位图掩码 ---- */
#define LPM_PREPARE_INPUT    (1 << 0)
#define LPM_PREPARE_COMMU    (1 << 1)
#define LPM_PREPARE_OUTPUT   (1 << 2)
#define LPM_IDLE_PREPARE_MASK  (LPM_PREPARE_INPUT | LPM_PREPARE_COMMU)
#define LPM_DEEP_PREPARE_MASK  (LPM_PREPARE_INPUT | LPM_PREPARE_COMMU | LPM_PREPARE_OUTPUT)

/* ---- 超时配置（可在 product_config.h 中覆盖） ---- */
#ifndef LPM_IDLE_TIMEOUT_MS
#define LPM_IDLE_TIMEOUT_MS    5000UL       /* 5 秒进入 Idle */
#endif
#ifndef LPM_DEEP_TIMEOUT_MS
#define LPM_DEEP_TIMEOUT_MS    600000UL     /* 10 分钟进入 Deep */
#endif

/*===========================================
 * 函数声明
 *==========================================*/

/* 初始化（在 system_init_middleware() 中调用） */
void lpm_init(void);

/* 主循环任务（在 main loop 或 OSAL 周期任务中调用） */
void lpm_task(void);

/* 活动记录：键盘扫描/触控/报文发送时调用，重置超时计时器 */
void lpm_note_activity(void);

/* 超时计时器接口（兼容旧代码） */
void lpm_timer_reset(void);
void lpm_timer_stop(void);

/* 状态查询 */
lpm_state_t lpm_get_state(void);
lpm_mode_t  lpm_get_mode(void);
bool        lpm_is_in_sleep(void);

/* 状态切换 */
void lpm_set_state(lpm_state_t state);

/* prepare 位图操作（由各 service 在 prepare 完成后调用） */
void lpm_mark_prepare_done(uint8_t prepare_bit);
bool lpm_all_prepare_done(void);

/* 禁止进入低功耗（USB 活动/OTA 等场景） */
void lpm_inhibit(bool inhibit);
bool lpm_is_inhibited(void);

/* 兼容旧接口 */
__attribute__((weak)) void enter_power_mode(pm_t mode);
__attribute__((weak)) bool usb_power_connected(void);
