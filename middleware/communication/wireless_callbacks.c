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


extern uint8_t commu_taskID;
extern wireless_event_t event;


void access_ble_notify_advertising(ble_adv_purpose_t pairing_state, uint8_t host_idx) {
    LOG_I("[WL_CB] adv pairing=%d host=%d", pairing_state, host_idx);
    event.evt_type = (pairing_state == BLE_ADV_PURPOSE_PAIRING_NEW) ? EVT_DISCOVERABLE : EVT_RECONNECTING;
    event.params.hostIndex = host_idx;
    if (pairing_state == BLE_ADV_PURPOSE_PAIRING_NEW) {
        OSAL_SetEvent(commu_taskID, WL_STS_DISCOVERABLE);
    } else {
        OSAL_SetEvent(commu_taskID, WL_STS_RECONNECTING);
    }
}

void access_ble_notify_connected(uint8_t host_idx) {
    LOG_I("[WL_CB] connected host=%d", host_idx);
    event.evt_type = EVT_CONNECTED;
    event.params.hostIndex = host_idx;
    OSAL_SetEvent(commu_taskID, WL_STS_CONNECTED);
}

void access_ble_notify_disconnected(uint8_t host_idx, uint8_t reason) {
    LOG_W("[WL_CB] disconnected host=%d reason=%d", host_idx, reason);
    /* 不在驱动层预判后续行为，由中间件状态机根据 wireless_state 决定 */
    event.evt_type = EVT_DISCONNECTED;
    event.params.hostIndex = host_idx;
    event.data = reason;
    OSAL_SetEvent(commu_taskID, WL_STS_DISCONNECTED);
}

void access_ble_notify_advertis_end(ble_adv_purpose_t pairing_state, uint8_t host_idx) {
    (void)pairing_state;
    (void)host_idx;
    event.evt_type = EVT_SLEEP;
    event.params.hostIndex = host_idx;
    OSAL_SetEvent(commu_taskID, WL_STS_ADV_END);
}



// BLE 模块初始化完成
void access_ble_init_done(void) {
    LOG_I("[WL_CB] BLE init done");
    if (commu_taskID != 0xFF) {
        OSAL_SetEvent(commu_taskID, WL_STS_INIT);
    }
}
