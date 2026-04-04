/* Copyright 2023 @ lokher (https://www.keychron.com)
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
#include "report_buffer.h"
#include "battery.h"
#include "transport.h"
#include "host_driver.h"
#include "report.h"
#include "keyboard.h"
#include "kb904/config_product.h"
#include "wait.h"
#include "string.h"
#include "print.h"
#include "debug.h"
#include "storage.h"
#include "output_service.h"
#include "communication_service.h"
// #include "rtc_timer.h"
// #include "keychron_wireless_common.h"
// #include "keychron_task.h"
// #include "wireless_config.h"
// #include "keychron_raw_hid.h"

extern uint8_t         pairing_indication;
host_driver_t   usb_driver;
extern report_buffer_t kb_rpt;
extern uint32_t        retry_time_buffer;
extern uint8_t         retry;

void indicator_battery_low_enable(bool enable) {
    (void)enable;
}

static uint8_t host_index = 0;
static uint8_t led_state  = 0;
//系统状态变量
uint8_t host_system_type = IOS;

wt_func_t  wireless_transport;
#ifdef P2P4G_ENABLE_FLAG
extern wt_func_t p24g_driver;  // 2.4G驱动实例
#endif

static wt_state_t wireless_state           = WT_RESET;
static bool       pincodeEntry             = false;
uint8_t           wireless_report_protocol = true;

uint16_t backlit_disable_time = CONNECTED_BACKLIGHT_DISABLE_TIMEOUT;
uint16_t connected_idle_time = CONNECTED_IDLE_TIME;

/* declarations */
uint8_t wreless_keyboard_leds(void);
void    wireless_send_keyboard(report_keyboard_t *report);
// void    wireless_send_nkro(report_nkro_t *report);
void    wireless_send_mouse(report_mouse_t *report);
void    wireless_send_extra(report_extra_t *report);
bool    process_record_wireless(uint16_t keycode);

/* BT LED 回调：BLE 收到 HID Out Report 时由 bt_driver 触发 */
static void wireless_bt_led_handler(uint8_t state) {
    led_state = state;
    dprintf("[WT_LED] state=%02X\n", state);

    /* Caps Lock (bit1) → 通知 output_service 更新物理 LED */
    if (state & 0x02) {
        output_service_request_indicator(IND_REQ_CAPS_ON, 0);
    } else {
        output_service_request_indicator(IND_REQ_CAPS_OFF, 0);
    }
}

/* host struct */
host_driver_t wireless_driver = {wreless_keyboard_leds, wireless_send_keyboard, wireless_send_mouse, wireless_send_extra};

bool wireless_lpm_set(uint8_t *data);

#if defined(EECONFIG_BASE_WIRELESS_CONFIG)
void wireless_config_reset(void) {
    uint8_t data[4] = { 0 };

    uint16_t backlit_disable_time = CONNECTED_BACKLIGHT_DISABLE_TIMEOUT;
    uint16_t connected_idle_time = CONNECTED_IDLE_TIME;

    memcpy(&data[0], &backlit_disable_time, sizeof(backlit_disable_time));
    memcpy(&data[2], &connected_idle_time, sizeof(connected_idle_time));
    wireless_lpm_set(data);
}

void wireless_config_load(void) {
    uint8_t offset = 0;
    eeprom_read_block(&backlit_disable_time, (uint8_t *)(EECONFIG_BASE_WIRELESS_CONFIG+offset), sizeof(backlit_disable_time));
    offset += sizeof(backlit_disable_time);
    eeprom_read_block(&connected_idle_time, (uint8_t *)(EECONFIG_BASE_WIRELESS_CONFIG+offset), sizeof(connected_idle_time));

    if (backlit_disable_time == 0)
        backlit_disable_time = CONNECTED_BACKLIGHT_DISABLE_TIMEOUT;
    else if (backlit_disable_time < 5 ) backlit_disable_time = 5;

    if (connected_idle_time == 0)
        connected_idle_time = CONNECTED_IDLE_TIME;
    else if (connected_idle_time < 30 ) connected_idle_time = 30;
}

void wireless_config_save(void) {
    uint8_t offset = 0;
    eeprom_update_block(&backlit_disable_time, (uint8_t *)(EECONFIG_BASE_WIRELESS_CONFIG+offset), sizeof(backlit_disable_time));
    offset += sizeof(backlit_disable_time);
    eeprom_update_block(&connected_idle_time, (uint8_t *)(EECONFIG_BASE_WIRELESS_CONFIG+offset), sizeof(connected_idle_time));
}
#endif

/*
 * Wireless init.
 */
void wireless_init(void) {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    wireless_state = WT_INITIALIZED;

#ifndef DISABLE_REPORT_BUFFER
    report_buffer_init();
#endif
}

/*
 * Bluetooth trasponrt init. Bluetooth module driver shall use this function to register a callback
 * to its implementation.
 */
void wireless_set_transport(wt_func_t *transport) {
    if (transport) memcpy(&wireless_transport, transport, sizeof(wt_func_t));
}

// 蓝牙驱动函数表（静态初始化）
#ifdef BLUETOOTH_ENABLE_FLAG
static const wt_func_t bt_driver_func_table = {
    .init = bt_driver_init,
    .connect_ex = bt_driver_connect_ex,
    .pairing_ex = bt_driver_pairing_ex,
    .disconnect = bt_driver_disconnect,
    .send_keyboard = bt_driver_send_keyboard,
    // .send_nkro = bt_driver_send_nkro,
    .send_consumer = bt_driver_send_consumer,
    .send_system = bt_driver_send_system,
    .send_mouse = bt_driver_send_mouse,
    .send_ptp = bt_driver_send_ptp,
    .get_unack_packets = bt_driver_get_unack_packets,
    .update_bat_level = bt_driver_update_bat_level,
};

void wireless_switch_to_bt_driver(void) {
    wireless_set_transport((wt_func_t*)&bt_driver_func_table);
    bt_driver_register_led_cb(wireless_bt_led_handler);
    kc_printf("Wireless: Switched to BT driver\n");
}
#endif

// 切换到2.4G驱动
#ifdef P2P4G_ENABLE_FLAG
void wireless_switch_to_p24g_driver(void) {
    if (p24g_driver) {
        wireless_transport = p24g_driver;
        kc_printf("Wireless: Switched to P24G driver\n");
    } else {
        kc_printf("Wireless: P24G driver not registered\n");
    }
}
#endif

// 切换到USB模式
#ifdef USB_ENABLE_FLAG
void wireless_switch_to_usb_mode(void) {

    kc_printf("Wireless: Switched to USB mode\n");
}
#endif

/*
 * Enter pairing with current host index
 */
void wireless_pairing(void) {
    // if (battery_is_critical_low()) return;

    wireless_pairing_ex(0, NULL);
    wireless_state = WT_PARING;
}

/*
 * Enter pairing with specified host index and param
 */
void wireless_pairing_ex(uint8_t host_idx, void *param) {
    kc_printf("wireless_pairing_ex %d\n\r", host_idx);
    // if (battery_is_critical_low()) return;

    if (wireless_transport.pairing_ex) wireless_transport.pairing_ex(host_idx, param);
    wireless_state = WT_PARING;

    host_index = host_idx;
}

/*
 * Initiate connection request to paired host
 */
void wireless_connect(void) {
    kc_printf("wireless_connect\n\r");

    wireless_transport.connect_ex(BLE_INDEX_1, 0);
    wireless_state = WT_RECONNECTING;
}

/*
 * Initiate connection request to paired host with argument
 */
void wireless_connect_ex(uint8_t host_idx, uint16_t timeout) {
    kc_printf("wireless_connect_ex %d\n\r", host_idx);

    if (host_idx != 0) {

        /* Do nothing when trying to connect to current connected host*/
        if (host_index == host_idx && wireless_state == WT_CONNECTED) return;

        host_index = host_idx;
        led_state  = 0;
    }
    wireless_transport.connect_ex(host_idx, timeout);
    wireless_state = WT_RECONNECTING;
}

/* Initiate a disconnection */
void wireless_disconnect(void) {
    kc_printf("wireless_disconnect\n\r");
    if (wireless_transport.disconnect) wireless_transport.disconnect();
}

/* Called when the BT device is reset. */
void wireless_enter_reset(uint8_t reason) {
    kc_printf("wireless_enter_reset\n\r");
    wireless_state = WT_RESET;
}

/* Enters discoverable state. Upon entering this state we perform the following actions:
 *   - change state to WT_PARING
 *   - set pairing indication
 */
void wireless_enter_discoverable(uint8_t host_idx) {
    kc_printf("wireless_enter_discoverable: %d\n\r", host_idx);
    host_index = host_idx;

    wireless_state = WT_PARING;
}

/*
 * Enters reconnecting state. Upon entering this state we perform the following actions:
 *   - change state to RECONNECTING
 *   - set reconnect indication
 */
void wireless_enter_reconnecting(uint8_t host_idx) {
    host_index = host_idx;

    kc_printf("wireless_reconnecting %d\n\r", host_idx);
    wireless_state = WT_RECONNECTING;
}

/* Enters connected state. Upon entering this state we perform the following actions:
 *   - change state to CONNECTED
 *   - set connected indication
 *   - enable NKRO if it is support
 */
void wireless_enter_connected(uint8_t host_idx) {
    kc_printf("wireless_connected %d\n\r", host_idx);

    wireless_state = WT_CONNECTED;
    host_index = host_idx;

    clear_keyboard();

    /* Enable NKRO since it may be disabled in pin code entry */
#if defined(NKRO_ENABLE) && !defined(WIRELESS_NKRO_ENABLE)
    keymap_config.nkro = false;
#endif

    // if (wireless_transport.update_bat_level) wireless_transport.update_bat_level(battery_get_percentage());
    //lpm_timer_reset();
}

/* Enters disconnected state. Upon entering this state we perform the following actions:
 *   - change state to DISCONNECTED
 *   - set disconnected indication
 */
void wireless_enter_disconnected(uint8_t host_idx, uint8_t reason) {
    kc_printf("wireless_disconnected %d, %d\n\r", host_idx, reason);

    uint8_t previous_state = wireless_state;
    led_state              = 0;
    if (get_transport() & TRANSPORT_WIRELESS)

    wireless_state = WT_DISCONNECTED;

    if (previous_state == WT_CONNECTED) {
        //lpm_timer_reset();
    } else {
#if defined(RGB_MATRIX_ENABLE) || defined(LED_MATRIX_ENABLE)
        if (reason && (get_transport() & TRANSPORT_WIRELESS)) {
            indicator_set_backlit_timeout(DISCONNECTED_BACKLIGHT_DISABLE_TIMEOUT*1000);
        }
#endif
    }

#ifndef DISABLE_REPORT_BUFFER
    report_buffer_init();
#endif

    indicator_battery_low_enable(false);
}

/* Enter pin code entry state. */
void wireless_enter_bluetooth_pin_code_entry(void) {
#if defined(NKRO_ENABLE)
    keymap_config.nkro = FALSE;
#endif
    pincodeEntry = true;
}

/* Exit pin code entry state. */
void wireless_exit_bluetooth_pin_code_entry(void) {
#if defined(NKRO_ENABLE) || defined(WIRELESS_NKRO_ENABLE)
    keymap_config.raw = eeconfig_read_keymap();
#endif
    pincodeEntry = false;
}

/* Enters disconnected state. Upon entering this state we perform the following actions:
 *   - change state to DISCONNECTED
 *   - set disconnected indication
 */
void wireless_enter_sleep(void) {
    kc_printf("wireless_enter_sleep %d\n\r", wireless_state);

    led_state = 0;

    /* WT_RECONNECTING 也必须转为 WT_SUSPEND，否则深度睡眠流程中
     * SYSTEM_ENTER_DEEP_EVT 会因 wt==WT_RECONNECTING 持续发 WL_REQ_DISCONNECT
     * + 延迟重试，形成"断连→仍是RECONNECTING→再断连"的死循环 */
    if (wireless_state == WT_CONNECTED || wireless_state == WT_PARING || wireless_state == WT_RECONNECTING) {
        wireless_state = WT_SUSPEND;
        kc_printf("WT_SUSPEND\n\r");

        indicator_battery_low_enable(false);
    }
}



/*  */
static void wireless_hid_set_protocol(bool report_protocol) {
    wireless_report_protocol = false;
}

uint8_t wreless_keyboard_leds(void) {
    if (wireless_state == WT_CONNECTED) {
        return led_state;
    }

    return 0;
}

//extern keymap_config_t keymap_config;

void wireless_send_keyboard(report_keyboard_t *report) {
    if ((wireless_state == WT_PARING || wireless_state == WT_RECONNECTING) && !pincodeEntry) return;

    if (wireless_state == WT_CONNECTED || (wireless_state == WT_PARING && pincodeEntry)) {
        if (wireless_transport.send_keyboard) {
            report_buffer_t report_buffer;
            report_buffer.type = REPORT_TYPE_KB;
            report_buffer.payload_len = sizeof(report_keyboard_t);
            memcpy(&report_buffer.keyboard, report, sizeof(report_keyboard_t));
            report_buffer_enqueue(&report_buffer);

            // 触发 OSAL 事件，异步发送
            OSAL_SetEvent(commu_taskID, REPORT_SEND_EVT);
        }
    } else if (wireless_state != WT_RESET) {
        wireless_connect();
    }
}

// void wireless_send_nkro(report_nkro_t *report) {
//     if (battery_is_critical_low()) return;

//     if (wireless_state == WT_PARING && !pincodeEntry) return;

//     if (wireless_state == WT_CONNECTED || (wireless_state == WT_PARING && pincodeEntry)) {
//         if (wireless_transport.send_nkro) {
// #ifndef DISABLE_REPORT_BUFFER
//             bool empty = report_buffer_is_empty();

//             report_buffer_t report_buffer;
//             report_buffer.type = REPORT_TYPE_NKRO;
//             memcpy(&report_buffer.nkro, report, sizeof(report_nkro_t));
//             report_buffer_enqueue(&report_buffer);

//             if (empty)
//                 report_buffer_task();
// #else
//             wireless_transport.send_nkro(&report->mods);
// #endif
//         }
//     } else if (wireless_state != WT_RESET) {
//         wireless_connect();
//     }
// }

void wireless_send_mouse(report_mouse_t *report) {
    if (wireless_state == WT_PARING || wireless_state == WT_RECONNECTING) return;

    if (wireless_state == WT_CONNECTED || (wireless_state == WT_PARING)) {
        if (wireless_transport.send_mouse) {
            report_buffer_t report_buffer;
            report_buffer.type = REPORT_TYPE_MOUSE;
            report_buffer.payload_len = sizeof(report_mouse_t);
            memcpy(&report_buffer.mouse, report, sizeof(report_mouse_t));
            report_buffer_enqueue(&report_buffer);

            // 触发 OSAL 事件，异步发送
            OSAL_SetEvent(commu_taskID, REPORT_SEND_EVT);
        }
    } else if (wireless_state != WT_RESET) {
        wireless_connect();
    }
}


void wireless_send_ptp(report_ptp_t *report,uint8_t len) {
    if (wireless_state == WT_PARING || wireless_state == WT_RECONNECTING) return;

    if (wireless_state == WT_CONNECTED || (wireless_state == WT_PARING)) {
        if (wireless_transport.send_ptp) {
            report_buffer_t report_buffer;
            report_buffer.type = REPORT_TYPE_PTP;
            report_buffer.payload_len = len;
            memcpy(&report_buffer.ptp, report, sizeof(report_ptp_t));
            report_buffer_enqueue(&report_buffer);

            // 触发 OSAL 事件，异步发送
            OSAL_SetEvent(commu_taskID, REPORT_SEND_EVT);
        }
    } else if (wireless_state != WT_RESET) {
        wireless_connect();
    }
}

void wireless_send_system(uint16_t data) {
    if (wireless_state == WT_CONNECTED) {
        if (wireless_transport.send_system) wireless_transport.send_system(data);
    } else if (wireless_state != WT_RESET) {
        wireless_connect();
    }
}

void wireless_send_consumer(uint16_t data) {
    if (wireless_state == WT_CONNECTED) {
        if (wireless_transport.send_consumer) {
            report_buffer_t report_buffer;
            report_buffer.type     = REPORT_TYPE_CONSUMER;
            report_buffer.payload_len = sizeof(uint16_t);
            report_buffer.consumer = data;
            report_buffer_enqueue(&report_buffer);

            // 触发 OSAL 事件，异步发送
            OSAL_SetEvent(commu_taskID, REPORT_SEND_EVT);
        }
    } else if (wireless_state != WT_RESET) {
        wireless_connect();
    }
}

void wireless_send_extra(report_extra_t *report) {
    // if (battery_is_critical_low()) return;

    if (report->report_id == REPORT_ID_SYSTEM) {
        wireless_send_system(report->usage);
    } else if (report->report_id == REPORT_ID_CONSUMER) {
        wireless_send_consumer(report->usage);
    }
}

void wireless_low_battery_shutdown(void) {
    indicator_battery_low_enable(false);
    report_buffer_init();
    clear_keyboard(); //
    wait_ms(50);      // wait a while for bt module to free buffer by sending report

    // Release all keys by sending empty reports
//    if (keymap_config.nkro) {
//        report_nkro_t empty_nkro_report;
//        memset(&empty_nkro_report, 0, sizeof(empty_nkro_report));
//        wireless_transport.send_nkro(&empty_nkro_report.mods);
//    } else {
//        report_keyboard_t empty_report;
//        memset(&empty_report, 0, sizeof(empty_report));
//        wireless_transport.send_keyboard(&empty_report.mods);
//    }
    wait_ms(10);
    wireless_transport.send_consumer(0);
    wait_ms(10);
    report_mouse_t empty_mouse_report;
    memset(&empty_mouse_report, 0, sizeof(empty_mouse_report));
    wireless_transport.send_mouse((uint8_t *)&empty_mouse_report);
    wait_ms(300); // Wait for bt module to send all buffered report

    wireless_disconnect();
}


void wireless_task(void) {
    // report_buffer_task() 已迁移到 commu_process_event 的 REPORT_SEND_EVT 处理
}

void send_string_task(void) {
    if ((get_transport() & TRANSPORT_WIRELESS) && wireless_get_state() == WT_CONNECTED) {
        // report_buffer_task() 已迁移到 commu_process_event 的 REPORT_SEND_EVT 处理
    }
}

void wireless_state_set_pairing(uint8_t host_idx) {
    kc_printf("[WT_STATE] -> WT_PARING host=%d\n", host_idx);
    wireless_enter_discoverable(host_idx);
}

void wireless_state_set_reconnecting(uint8_t host_idx) {
    kc_printf("[WT_STATE] -> WT_RECONNECTING host=%d\n", host_idx);
    wireless_enter_reconnecting(host_idx);
}

void wireless_state_set_connected(uint8_t host_idx) {
    kc_printf("[WT_STATE] -> WT_CONNECTED host=%d\n", host_idx);
    wireless_enter_connected(host_idx);
}

void wireless_state_set_disconnected(uint8_t host_idx, uint8_t reason) {
    kc_printf("[WT_STATE] -> WT_DISCONNECTED host=%d reason=%d\n", host_idx, reason);
    wireless_enter_disconnected(host_idx, reason);
}

void wireless_state_set_sleep(void) {
    kc_printf("[WT_STATE] -> WT_SUSPEND\n");
    wireless_enter_sleep();
}

wt_state_t wireless_get_state(void) {
    return wireless_state;
};

bool process_record_wireless(uint16_t keycode) {
    if (get_transport() & TRANSPORT_WIRELESS) {
        //lpm_timer_reset();

        if (battery_is_empty() && wireless_get_state() == WT_CONNECTED ) {
            indicator_battery_low_enable(true);
        }
    }

//    if (!process_record_keychron_wireless(keycode, record)) return false;

    return true;
}
