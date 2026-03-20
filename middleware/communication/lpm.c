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

/******************************************************************************
 *
 *  Filename:      lpm.c
 *
 *  Description:   LPM 状态机实现 - Idle/Deep 睡眠管理
 *                 基于 OSAL 事件驱动 + service prepare 位图汇聚架构
 *
 ******************************************************************************/

#include "lpm.h"
#include "timer.h"
#include "event_manager.h"
#include "system_service.h"   /* SYSTEM_LPM_*_EVT 定义，system_taskID extern 声明 */
#include "debug.h"
#include <string.h>

/* ---- 内部状态 ---- */
static lpm_state_t  g_lpm_state          = LPM_STATE_ACTIVE;
static lpm_mode_t   g_lpm_mode           = LPM_MODE_NONE;
static uint8_t      g_prepare_pending    = 0;
static uint8_t      g_prepare_done       = 0;
static uint32_t     g_last_activity_ms   = 0;
static bool         g_inhibited          = false;

/*===========================================
 * 初始化
 *==========================================*/

void lpm_init(void) {
    g_lpm_state        = LPM_STATE_ACTIVE;
    g_lpm_mode         = LPM_MODE_NONE;
    g_prepare_pending  = 0;
    g_prepare_done     = 0;
    g_inhibited        = false;
    g_last_activity_ms = timer_read32();

#ifdef USB_POWER_SENSE_PIN
#    if (USB_POWER_CONNECTED_LEVEL == 0)
    setPinInputHigh(USB_POWER_SENSE_PIN);
#    else
    setPinInputLow(USB_POWER_SENSE_PIN);
#    endif
#endif

    dprintf("[LPM] Initialized, idle=%lums, deep=%lums\r\n",
            (unsigned long)LPM_IDLE_TIMEOUT_MS,
            (unsigned long)LPM_DEEP_TIMEOUT_MS);
}

/*===========================================
 * 活动记录
 *==========================================*/

void lpm_note_activity(void) {
    g_last_activity_ms = timer_read32();

    /* 若处于 PENDING 状态收到活动，取消本轮睡眠 */
    if (g_lpm_state == LPM_STATE_IDLE_PENDING ||
        g_lpm_state == LPM_STATE_DEEP_PENDING) {
        dprintf("[LPM] Activity detected, cancel pending sleep\r\n");
        g_lpm_state = LPM_STATE_ACTIVE;
        g_prepare_pending = 0;
        g_prepare_done = 0;
    }
}

void lpm_timer_reset(void) {
    g_last_activity_ms = timer_read32();
}

void lpm_timer_stop(void) {
    g_last_activity_ms = 0;
}

/*===========================================
 * 状态查询
 *==========================================*/

lpm_state_t lpm_get_state(void) {
    return g_lpm_state;
}

lpm_mode_t lpm_get_mode(void) {
    return g_lpm_mode;
}

bool lpm_is_in_sleep(void) {
    return g_lpm_state == LPM_STATE_IDLE_SLEEP ||
           g_lpm_state == LPM_STATE_DEEP_SLEEP;
}

/*===========================================
 * 状态切换
 *==========================================*/

void lpm_set_state(lpm_state_t state) {
    lpm_state_t old = g_lpm_state;
    g_lpm_state = state;

    /* 只在状态实际变化时输出日志 */
    if (old != state) {
        static const char* const state_names[] = {
            "ACTIVE", "IDLE_PENDING", "IDLE_SLEEP", "DEEP_PENDING", "DEEP_SLEEP", "WAKE_RESUME"
        };
        dprintf("[LPM] State: %s -> %s\r\n",
                state_names[old], state_names[state]);
    }
}

/*===========================================
 * prepare 位图操作
 *==========================================*/

void lpm_mark_prepare_done(uint8_t prepare_bit) {
    g_prepare_done |= prepare_bit;
}

bool lpm_all_prepare_done(void) {
    return (g_prepare_done & g_prepare_pending) == g_prepare_pending;
}

/*===========================================
 * 禁止进入低功耗
 *==========================================*/

void lpm_inhibit(bool inhibit) {
    g_inhibited = inhibit;
    if (inhibit) {
        dprintf("[LPM] Inhibit enabled\r\n");
    } else {
        dprintf("[LPM] Inhibit disabled\r\n");
    }
}

bool lpm_is_inhibited(void) {
    return g_inhibited;
}

/*===========================================
 * 主循环任务
 *==========================================*/

void lpm_task(void) {
    if (g_lpm_state != LPM_STATE_ACTIVE) {
        return;
    }
    if (g_inhibited) {
        return;
    }
    if (g_last_activity_ms == 0) {
        return;
    }

    uint32_t elapsed = timer_elapsed32(g_last_activity_ms);

    // if (elapsed >= LPM_DEEP_TIMEOUT_MS) {
    //     /* 直接推进到 Deep（跳过 Idle pending） */
    //     dprintf("[LPM] Deep timeout reached (%lu ms), requesting deep sleep\r\n", elapsed);
    //     g_lpm_mode        = LPM_MODE_DEEP;
    //     g_prepare_pending = LPM_DEEP_PREPARE_MASK;
    //     g_prepare_done    = 0;
    //     g_lpm_state       = LPM_STATE_DEEP_PENDING;  /* 先推进状态，防止主循环重复投递 */
    //     OSAL_SetEvent(system_taskID, SYSTEM_LPM_DEEP_REQ_EVT);
    // } else if (elapsed >= LPM_IDLE_TIMEOUT_MS) {
    //     dprintf("[LPM] Idle timeout reached (%lu ms), requesting idle sleep\r\n", elapsed);
    //     g_lpm_mode        = LPM_MODE_IDLE;
    //     g_prepare_pending = LPM_IDLE_PREPARE_MASK;
    //     g_prepare_done    = 0;
    //     g_lpm_state       = LPM_STATE_IDLE_PENDING;  /* 先推进状态，防止主循环重复投递 */
    //     OSAL_SetEvent(system_taskID, SYSTEM_LPM_IDLE_REQ_EVT);
    // }
}

/*===========================================
 * 兼容旧接口
 *==========================================*/

__attribute__((weak)) void enter_power_mode(pm_t mode) {
    (void)mode;
    /* 由 HAL 层实现 */
}

__attribute__((weak)) bool usb_power_connected(void) {
#ifdef USB_POWER_SENSE_PIN
    return readPin(USB_POWER_SENSE_PIN) == USB_POWER_CONNECTED_LEVEL;
#endif
    return true;
}
