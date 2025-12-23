/* Copyright 2023 Keychron
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY of any kind, either express or implied.
 * See the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "sys_config.h"
#include "sys_error.h"

/*********************************************************************
 * 蓝牙驱动接口方法 - CH584 蓝牙驱动实现
 *********************************************************************/

// 生命周期管理
void bt_driver_setup(void);
void bt_driver_init(bool wakeup_from_low_power);

// 连接管理
void bt_driver_connect_ex(uint8_t host_idx, uint16_t timeout);
void bt_driver_pairing_ex(uint8_t host_idx, void *param);
void bt_driver_disconnect(void);

// 数据传输
void bt_driver_send_keyboard(uint8_t *report);
void bt_driver_send_nkro(uint8_t *report);
void bt_driver_send_consumer(uint16_t report);
void bt_driver_send_system(uint16_t report);
void bt_driver_send_mouse(uint8_t *report);

// 电池管理
void bt_driver_update_bat_level(uint8_t bat_lvl);

