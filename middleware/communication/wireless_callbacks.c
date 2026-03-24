/* Copyright 2024
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

#include "wireless.h"
#include "output_service.h"
#include "battery.h"
#include "print.h"
#include "debug.h"
#include "storage.h"
#include "system_hal.h"
#include "bt_driver.h"
#include "event_manager.h"
#include "system_service.h"
#include "../../drivers/communication/bluetooth/ch584/hidkbd.h"
#include "communication_service.h"

#ifdef RGB_MATRIX_ENABLE
#    include "rgb_matrix.h"
#endif

#ifdef LED_MATRIX_ENABLE
#    include "led_matrix.h"
#endif

void access_ble_schedule_deep_sleep_evt(uint32_t delay_ticks);
void access_ble_cancel_deep_sleep_evt(void);

extern uint8_t commu_taskID;
extern wireless_event_t event;

static bool access_ble_is_discoverable_session(uint8_t host_idx) {
    if (access_state.intent == BLE_INTENT_PAIRING) {
        return true;
    }

    return (hidEmu_is_ble_bonded((access_ble_idx_t)host_idx) == 0u);
}

/*********************************************************************
 * @fn      wireless_enter_reset_kb
 *
 * @brief   无线模块重置回调函数
 * @param   reason 重置原因
 * @return  none
 *
 * @note    应用层可以重写此函数以实现自定义的初始化逻辑
 *********************************************************************/
void wireless_enter_reset_kb(uint8_t reason) {
    dprintf("Wireless: System reset");
    // 测试平台实现：无需实际硬件初始化
}

/*********************************************************************
 * @fn      wireless_enter_discoverable_kb
 *
 * @brief   进入可发现模式（配对模式）回调函数
 * @param   host_idx 主机索引
 * @return  none
 *
 * @note    当无线模块进入配对模式时调用
 *********************************************************************/
void wireless_enter_discoverable_kb(uint8_t host_idx) {
    dprintf("Wireless: Entering discoverable mode\r\n");
    output_service_request_indicator(IND_REQ_BT_PAIRING, host_idx);
}

/*********************************************************************
 * @fn      wireless_enter_reconnecting_kb
 *
 * @brief   进入重连模式回调函数
 * @param   host_idx 主机索引
 * @return  none
 *
 * @note    当无线模块尝试重连时调用
 *********************************************************************/
void wireless_enter_reconnecting_kb(uint8_t host_idx) {
    dprintf("Wireless: Entering reconnecting mode\r\n");
    output_service_request_indicator(IND_REQ_BT_RECONNECTING, host_idx);
}

/*********************************************************************
 * @fn      wireless_enter_connected_kb
 *
 * @brief   连接成功回调函数
 * @param   host_idx 主机索引
 * @return  none
 *
 * @note    当无线模块成功连接主机时调用
 *********************************************************************/
void wireless_enter_connected_kb(uint8_t host_idx) {
    dprintf("Wireless: Connected to host");

    // 清除键盘状态
    // clear_keyboard();

#ifdef NKRO_ENABLE
    // 启用NKRO
    // keymap_config.nkro = true;
#endif

    // 更新电池电量到模块
    // wireless_update_battery_level(battery_get_percentage());

    output_service_request_indicator(IND_REQ_BT_CONNECTED, host_idx);
}

/*********************************************************************
 * @fn      wireless_enter_disconnected_kb
 *
 * @brief   断开连接回调函数
 * @param   host_idx 主机索引
 * @param   reason 断开原因
 * @return  none
 *
 * @note    当无线模块断开连接时调用
 *********************************************************************/
void wireless_enter_disconnected_kb(uint8_t host_idx, uint8_t reason) {
    dprintf("Wireless: Disconnected from host");
    (void)reason;
    output_service_request_indicator(IND_REQ_BT_DISCONNECTED, host_idx);
}

/*********************************************************************
 * @fn      wireless_enter_sleep_kb
 *
 * @brief   进入睡眠模式回调函数
 * @return  none
 *
 * @note    当无线模块进入低功耗模式时调用
 *********************************************************************/
void wireless_enter_sleep_kb(void) {
    dprintf("Wireless: Entering sleep mode");

#if defined(LED_MATRIX_ENABLE) || defined(RGB_MATRIX_ENABLE)
    // 关闭LED背光以节省电量
#    ifdef LED_MATRIX_ENABLE
    // led_matrix_disable_noeeprom();
#    endif

#    ifdef RGB_MATRIX_ENABLE
    // rgb_matrix_disable_noeeprom();
#    endif
#endif
}

void access_ble_notify_advertising(uint8_t pairing_state, uint8_t host_idx) {
    bool discoverable = access_ble_is_discoverable_session(host_idx);

    dprintf("[WT_SYNC] adv pairing=%d host=%d discoverable=%d\r\n",
            pairing_state, host_idx, discoverable);
    event.evt_type = discoverable ? EVT_DISCOVERABLE : EVT_RECONNECTING;
    event.params.hostIndex = host_idx;
    if (discoverable) {
        OSAL_SetEvent(commu_taskID, WL_DISCOVERABLE_EVT);
    } else {
        OSAL_SetEvent(commu_taskID, WL_RECONNECTING_EVT);
    }
}

void access_ble_notify_connected(uint8_t host_idx) {
    dprintf("[WT_SYNC] connected host=%d\r\n", host_idx);
    event.evt_type = EVT_CONNECTED;
    event.params.hostIndex = host_idx;
    OSAL_SetEvent(commu_taskID, WL_CONNECTED_EVT);
    // access_ble_cancel_deep_sleep_evt();
    wireless_state_set_connected(host_idx);
}

void access_ble_notify_disconnected(uint8_t host_idx, uint8_t reason) {
    dprintf("[WT_SYNC] disconnected host=%d reason=%d\r\n", host_idx, reason);
    /* 不在驱动层预判后续行为，由中间件状态机根据 wireless_state 决定 */
    event.evt_type = EVT_DISCONNECTED;
    event.params.hostIndex = host_idx;
    event.data = reason;
    OSAL_SetEvent(commu_taskID, WL_DISCONNECTED_EVT);
}

void access_ble_enter_idel_sleep(void) {
    if (wireless_get_state() != WT_CONNECTED) {
        dprintf("[WT_SLEEP] ignore state=%d deep=%d\r\n", wireless_get_state(), access_state.deep_sleep_flag);
        return;
    }

    if (!access_state.deep_sleep_flag) {
        dprintf("[WT_SLEEP] skip deep_sleep_flag=0");
        return;
    }

    dprintf("[WT_SLEEP] schedule deep sleep via OSAL event\r\n");

    /* 不直接调用 system_hal_enter_sleep()，改为投递 LPM Deep 请求 */
    /* 由 lpm 状态机和 system_service 协调执行 */
    OSAL_SetEvent(system_taskID, SYSTEM_LPM_DEEP_REQ_EVT);
    /* 唤醒后的回连由 COMMU_LPM_RESUME_EVT 或业务层决定，此处不直接调用 bt_driver_connect_ex */
}

void access_ble_schedule_deep_sleep_evt(uint32_t delay_ticks) {
    if (system_taskID == 0xFF) {
        dprintf("[WT_SLEEP] system task not ready, skip schedule delay=%lu\r\n", (unsigned long)delay_ticks);
        return;
    }

    OSAL_SetDelayedEvent(system_taskID, SYSTEM_LPM_DEEP_REQ_EVT, delay_ticks);
    dprintf("[WT_SLEEP] schedule SYSTEM_LPM_DEEP_REQ_EVT delay=%lu\r\n", (unsigned long)delay_ticks);
}

void access_ble_cancel_deep_sleep_evt(void) {
    if (system_taskID == 0xFF) {
        return;
    }

    if (OSAL_GetTaskTimer(system_taskID, SYSTEM_LPM_DEEP_REQ_EVT)) {
        OSAL_StopTask(system_taskID, SYSTEM_LPM_DEEP_REQ_EVT);
        dprintf("[WT_SLEEP] cancel SYSTEM_LPM_DEEP_REQ_EVT\r\n");
    }
}

// BLE 模块初始化完成
void access_ble_init_done(void) {
    dprintf("Wireless: BLE initialization completed");
        // 启动无线模块初始化事件
    OSAL_SetEvent(commu_taskID, WL_INIT_EVT);
}
