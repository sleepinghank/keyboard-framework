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
#include "indicator.h"
#include "battery.h"
#include "print.h"
#include "debug.h"
#include "storage.h"
#include "system_hal.h"
#include "bt_driver.h"
#include "../../drivers/communication/bluetooth/ch584/hidkbd.h"
#include "../../drivers/communication/bluetooth/ch584/_bt_driver.h"

/* 无线状态名称（用于日志）*/
static const char *wt_state_name(wt_state_t s) {
    switch (s) {
        case WT_RESET:        return "WT_RESET";
        case WT_INITIALIZED:  return "WT_INITIALIZED";
        case WT_DISCONNECTED: return "WT_DISCONNECTED";
        case WT_CONNECTED:    return "WT_CONNECTED";
        case WT_PARING:       return "WT_PARING";
        case WT_RECONNECTING: return "WT_RECONNECTING";
        case WT_SUSPEND:      return "WT_SUSPEND";
        default:              return "WT_UNKNOWN";
    }
}

#ifdef RGB_MATRIX_ENABLE
#    include "rgb_matrix.h"
#endif

#ifdef LED_MATRIX_ENABLE
#    include "led_matrix.h"
#endif

static uint8_t wireless_cb_indicator_led(uint8_t host_idx) {
    if (host_idx >= 1 && host_idx <= 3) {
        return (uint8_t)(host_idx - 1);
    }
    return 0;
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
__attribute__((weak)) void wireless_enter_reset_kb(uint8_t reason) {
    println("Wireless: System reset");
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
__attribute__((weak)) void wireless_enter_discoverable_kb(uint8_t host_idx) {
    println("Wireless: Entering discoverable mode");
    // 设置指示灯为配对状态
    indicator_set(wireless_cb_indicator_led(host_idx), &IND_BLINK_SLOW);
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
__attribute__((weak)) void wireless_enter_reconnecting_kb(uint8_t host_idx) {
    println("Wireless: Entering reconnecting mode");
    // 设置指示灯为重连状态
    indicator_set(wireless_cb_indicator_led(host_idx), &IND_BLINK_FAST);
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
__attribute__((weak)) void wireless_enter_connected_kb(uint8_t host_idx) {
    println("Wireless: Connected to host");

    // 清除键盘状态
    // clear_keyboard();

#ifdef NKRO_ENABLE
    // 启用NKRO
    // keymap_config.nkro = true;
#endif

    // 更新电池电量到模块
    // wireless_update_battery_level(battery_get_percentage());

    // 设置指示灯为连接状态
    indicator_set(wireless_cb_indicator_led(host_idx), &IND_ON);
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
__attribute__((weak)) void wireless_enter_disconnected_kb(uint8_t host_idx, uint8_t reason) {
    println("Wireless: Disconnected from host");

    // 设置指示灯为断开状态
    indicator_set(wireless_cb_indicator_led(host_idx), &IND_OFF);
}

/*********************************************************************
 * @fn      wireless_enter_sleep_kb
 *
 * @brief   进入睡眠模式回调函数
 * @return  none
 *
 * @note    当无线模块进入低功耗模式时调用
 *********************************************************************/
__attribute__((weak)) void wireless_enter_sleep_kb(void) {
    println("Wireless: Entering sleep mode");

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
    wt_state_t cur = wireless_get_state();
    bt_driver_dump_state();
    if (pairing_state) {
        dprintf("[WT] %s -> WT_PARING  host=%d\r\n", wt_state_name(cur), host_idx);
        wireless_state_set_pairing(host_idx);
    } else {
        dprintf("[WT] %s -> WT_RECONNECTING  host=%d\r\n", wt_state_name(cur), host_idx);
        wireless_state_set_reconnecting(host_idx);
    }
}

void access_ble_notify_connected(uint8_t host_idx) {
    wt_state_t cur = wireless_get_state();
    bt_driver_dump_state();
    dprintf("[WT] %s -> WT_CONNECTED  host=%d\r\n", wt_state_name(cur), host_idx);
    wireless_state_set_connected(host_idx);
}

void access_ble_notify_disconnected(uint8_t host_idx, uint8_t reason) {
    wt_state_t cur = wireless_get_state();
    bt_driver_dump_state();
    dprintf("[WT] %s -> DISCONNECTED  host=%d reason=0x%02x\r\n", wt_state_name(cur), host_idx, reason);
    wireless_state_set_disconnected(host_idx, reason);
}

void access_ble_enter_idel_sleep(void) {
    wt_state_t cur = wireless_get_state();
    if (cur != WT_CONNECTED) {
        dprintf("[WT] %s -> sleep ignored (deep=%d)\r\n", wt_state_name(cur), access_state.deep_sleep_flag);
        return;
    }

    if (!access_state.deep_sleep_flag) {
        dprintf("[WT] %s -> sleep skipped (deep_sleep_flag=0)\r\n", wt_state_name(cur));
        return;
    }

    bt_driver_dump_state();
    dprintf("[WT] %s -> WT_SUSPEND  (idle timeout)\r\n", wt_state_name(cur));
    wireless_state_set_sleep();

    if (storage_is_initialized()) {
        storage_save();
    }
    indicator_off_all();

    system_hal_enter_sleep(SYSTEM_POWER_MODE_DEEP_SLEEP,
                           SYSTEM_WAKEUP_GPIO | SYSTEM_WAKEUP_KEYBOARD | SYSTEM_WAKEUP_BLE);

    println("Wireless: Wakeup from deep sleep, reconnect");
    bt_driver_connect_ex(0, 0);
}
