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

#pragma once

typedef enum {
    TRANSPORT_NONE,
#ifdef USB_ENABLE_FLAG
    TRANSPORT_USB       = 0x01 << 0,
#endif
#ifdef BLUETOOTH_ENABLE_FLAG
    TRANSPORT_BLUETOOTH = 0x01 << 1,
#endif
#ifdef P2P4G_ENABLE_FLAG
    TRANSPORT_P2P4      = 0x01 << 2,
#endif
    TRANSPORT_MAX,
} transport_t;
#define P24G_INDEX 24
// 驱动切换通知函数声明
void transport_notify_driver_switch(transport_t new_transport);

#ifdef NKRO_ENABLE
typedef struct {
#ifdef USB_ENABLE_FLAG
    bool usb : 1;
#endif
#ifdef BLUETOOTH_ENABLE_FLAG
    bool bluetooth : 1;
#endif
} nkro_t;
#endif

#if defined(BLUETOOTH_ENABLE_FLAG) && defined(P2P4G_ENABLE_FLAG)
#    define TRANSPORT_WIRELESS (TRANSPORT_BLUETOOTH | TRANSPORT_P2P4)
#elif defined(BLUETOOTH_ENABLE_FLAG)
#    define TRANSPORT_WIRELESS TRANSPORT_BLUETOOTH
#elif defined(P2P4G_ENABLE_FLAG)
#    define TRANSPORT_WIRELESS TRANSPORT_P2P4
else 
#    define TRANSPORT_WIRELESS 0
#endif

void        set_transport(transport_t new_transport);
transport_t get_transport(void);

#ifdef USB_ENABLE_FLAG
void usb_power_connect(void);
void usb_power_disconnect(void);
void usb_transport_enable(bool enable);
void usb_remote_wakeup(void);
#endif

#ifdef BLUETOOTH_ENABLE_FLAG
void bt_transport_enable(bool enable);
#endif

#ifdef P2P4G_ENABLE_FLAG
void p24g_transport_enable(bool enable);
#endif
