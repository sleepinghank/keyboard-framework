/* Copyright 2022~2023 @ lokher (https://www.keychron.com)
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
#include "lpm.h"
#include "transport.h"
#include "host_driver.h"
#include "keycode_config.h"
#include "host.h"
#include "wait.h"
#include "storage.h"

#ifndef REINIT_LED_DRIVER
#    define REINIT_LED_DRIVER 0
#endif

extern host_driver_t usb_driver;
extern host_driver_t   wireless_driver;
extern keymap_config_t keymap_config;
extern wt_func_t       wireless_transport;

static transport_t transport = TRANSPORT_NONE;

#ifdef NKRO_ENABLE
nkro_t nkro = {false, false};
#endif

static void transport_changed(transport_t new_transport);

#ifdef BLUETOOTH_ENABLE_FLAG
void bt_transport_enable(bool enable) {
    if (enable) {
        // if (host_get_driver() != &wireless_driver) {
        host_set_driver(&wireless_driver);
        dprintf("Transport: Switching to Bluetooth\n");
        wireless_disconnect();

        uint32_t t = timer_read32();
        // while (timer_elapsed32(t) < 50) {
        //     wireless_transport.task();
        // }
        wireless_connect();

        // TODO: Clear USB report
        //}
    } else {
        // indicator_stop();

        if (wireless_get_state() == WT_CONNECTED && transport == TRANSPORT_BLUETOOTH) {
            report_keyboard_t empty_report = {0};
            wireless_driver.send_keyboard(&empty_report);
        }
    }
}
#endif

#ifdef P2P4G_ENABLE_FLAG
void p24g_transport_enable(bool enable) {
    if (enable) {
        // if (host_get_driver() != &wireless_driver) {
        host_set_driver(&wireless_driver);

        /* Disconnect and reconnect to sync the wireless state
         * TODO: query bluetooth state to sync
         */
        wireless_disconnect();

        uint32_t t = timer_read32();
        // while (timer_elapsed32(t) < 50) {
        //     wireless_transport.task();
        // }
        wireless_connect_ex(P24G_INDEX, 0);
        // wireless_connect();
        //  TODO: Clear USB report
        //}
    } else {
        // indicator_stop();

        if (wireless_get_state() == WT_CONNECTED && transport == TRANSPORT_P2P4) {
            report_keyboard_t empty_report = {0};
            wireless_driver.send_keyboard(&empty_report);
        }
    }
}
#endif

#ifdef USB_ENABLE_FLAG
void usb_power_connect(void) {}
void usb_power_disconnect(void) {}
#endif

#ifdef USB_ENABLE_FLAG
void usb_transport_enable(bool enable) {
    if (enable) {
        if (host_get_driver() != &usb_driver) {
#if !defined(KEEP_USB_CONNECTION_IN_WIRELESS_MODE)
            usb_power_connect();
//            usb_start(&USBD1);
#endif
            host_set_driver(&usb_driver);
        }
    } else {
//        if (USB_DRIVER.state == USB_ACTIVE) {
//            report_keyboard_t empty_report = {0};
//            usb_driver.send_keyboard(&empty_report);
//        }

#if !defined(KEEP_USB_CONNECTION_IN_WIRELESS_MODE)
//        usbStop(&USBD1);
//        usbDisconnectBus(&USBD1);
        usb_power_disconnect();
#endif
    }
}
#endif

void set_transport(transport_t new_transport) {
    if (transport != new_transport) {
#ifdef USB_ENABLE_FLAG
        if (transport == TRANSPORT_USB || ((transport != TRANSPORT_USB) && wireless_get_state() == WT_CONNECTED)) clear_keyboard();
#endif

        transport = new_transport;
        dprintf("Transport: Switching to %d\n", transport);

        switch (transport) {
#ifdef USB_ENABLE_FLAG
            case TRANSPORT_USB:
                usb_transport_enable(true);
#ifdef BLUETOOTH_ENABLE_FLAG
                bt_transport_enable(false);
#endif
                wait_ms(5);
#ifdef P2P4G_ENABLE_FLAG
                p24g_transport_enable(false);
#endif
                wireless_disconnect();
                lpm_timer_stop();
                // 通知无线层切换到USB模式
                wireless_switch_to_usb_mode();
                break;
#endif

#ifdef BLUETOOTH_ENABLE_FLAG
            case TRANSPORT_BLUETOOTH:
#ifdef P2P4G_ENABLE_FLAG
                p24g_transport_enable(false);
                wait_ms(1);
#endif
                dprintf("Transport: Switching to Bluetooth\n");
                // 先切换无线层驱动绑定，确保 wireless_transport 指向蓝牙驱动
                wireless_switch_to_bt_driver();
                bt_transport_enable(true);
#ifdef USB_ENABLE_FLAG
                usb_transport_enable(false);
#endif
                lpm_timer_reset();
                break;
#endif

#ifdef P2P4G_ENABLE_FLAG
            case TRANSPORT_P2P4:
#ifdef BLUETOOTH_ENABLE_FLAG
                bt_transport_enable(false);
                wait_ms(1);
#endif
                // 先切换无线层驱动绑定，确保 wireless_transport 指向2.4G驱动
                wireless_switch_to_p24g_driver();
                p24g_transport_enable(true);
#ifdef USB_ENABLE_FLAG
                usb_transport_enable(false);
#endif
                lpm_timer_reset();
                break;
#endif

            default:
                break;
        }

        transport_changed(transport);
    }
}

// 驱动切换通知函数实现
void transport_notify_driver_switch(transport_t new_transport) {
    kc_printf("Transport: Notifying driver switch to %d\n", new_transport);

    switch (new_transport) {
#ifdef USB_ENABLE_FLAG
        case TRANSPORT_USB:
            wireless_switch_to_usb_mode();
            break;
#endif
#ifdef BLUETOOTH_ENABLE_FLAG
        case TRANSPORT_BLUETOOTH:
            wireless_switch_to_bt_driver();
            break;
#endif
#ifdef P2P4G_ENABLE_FLAG
        case TRANSPORT_P2P4:
            wireless_switch_to_p24g_driver();
            break;
#endif
        default:
            break;
    }
}

transport_t get_transport(void) {
    return transport;
}

#if (REINIT_LED_DRIVER)
/* Changing transport may cause bronw-out reset of led driver
 * withoug MCU reset, which lead backlight to not work,
 * reinit the led driver workgound this issue */
static void reinit_led_drvier(void) {
    /* Wait circuit to discharge for a while */
    systime_t start = chVTGetSystemTime();
    while (chTimeI2MS(chVTTimeElapsedSinceX(start)) < 100) {
    };

#    ifdef LED_MATRIX_ENABLE
    led_matrix_init();
#    endif
#    ifdef RGB_MATRIX_ENABLE
    rgb_matrix_init();
#    endif
}
#endif

void transport_changed(transport_t new_transport) {
    kc_printf("transport_changed %d\n\r", new_transport);
    // indicator_init();

#if (REINIT_LED_DRIVER)
    reinit_led_drvier();
#endif

#if defined(RGB_MATRIX_ENABLE) && defined(RGB_MATRIX_TIMEOUT)
#    if (RGB_MATRIX_TIMEOUT > 0)
    rgb_matrix_disable_timeout_set(RGB_MATRIX_TIMEOUT_INFINITE);
    rgb_matrix_disable_time_reset();
#    endif
#endif
#if defined(LED_MATRIX_ENABLE) && defined(LED_MATRIX_TIMEOUT)
#    if (LED_MATRIX_TIMEOUT > 0)
    led_matrix_disable_timeout_set(LED_MATRIX_TIMEOUT_INFINITE);
    led_matrix_disable_time_reset();
#    endif
#endif
}

void usb_remote_wakeup(void) {
//    if (USB_DRIVER.state == USB_SUSPENDED) {
//        while (USB_DRIVER.state == USB_SUSPENDED) {
//            wireless_pre_task();
//            if (get_transport() != TRANSPORT_USB) {
//                suspend_wakeup_init_quantum();
//                return;
//            }
//            /* Do this in the suspended state */
//            suspend_power_down(); // on AVR this deep sleeps for 15ms
//            /* Remote wakeup */
//            if (suspend_wakeup_condition()
//#ifdef ENCODER_ENABLE
//                || encoder_read()
//#endif
//                ) {
//                usbWakeupHost(&USB_DRIVER);
//                wait_ms(300);
//#ifdef MOUSEKEY_ENABLE
//                // Wiggle to wakeup
//                mousekey_on(KC_MS_LEFT);
//                mousekey_send();
//                wait_ms(10);
//                mousekey_on(KC_MS_RIGHT);
//                mousekey_send();
//                wait_ms(10);
//                mousekey_off((KC_MS_RIGHT));
//                mousekey_send();
//#else
//                set_mods(0x02);
//                send_keyboard_report();
//                wait_ms(10);
//                del_mods(0x02);
//                send_keyboard_report();
//#endif
//            }
//        }
//        /* Woken up */
//        // variables has been already cleared by the wakeup hook
//        send_keyboard_report();
//#ifdef MOUSEKEY_ENABLE
//        mousekey_send();
//#endif /* MOUSEKEY_ENABLE */
//        usb_event_queue_task();
//    }
}
