/* Copyright 2025 @ keyboard-framework
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

/**
 * @file ble_config.h
 * @brief 蓝牙模块配置
 *
 * 配置加载顺序：
 * 1. 产品配置 (keyboards/kb904/config.h) - 定义覆盖值
 * 2. 默认配置 (defaults/ble_defaults.h) - 使用 #ifndef 保护
 * 3. 本文件 - 蓝牙驱动配置
 */

#pragma once

#include "kb904/config.h"

/* ============ 默认配置加载 ============ */
/* 如果使用新的配置系统，defaults 在产品 config.h 中已包含 */
/* 否则使用以下默认值 */

#ifndef BLE_PAIRING_NAME
#define BLE_PAIRING_NAME     "Keyboard"
#endif

#ifndef BLE_PAIRING_TIMEOUT_SEC
#define BLE_PAIRING_TIMEOUT_SEC  60
#endif

#ifndef BLE_RECONNECT_TIMEOUT_SEC
#define BLE_RECONNECT_TIMEOUT_SEC 60
#endif

#ifndef BLE_ADVERTISE_INTERVAL_MS
#define BLE_ADVERTISE_INTERVAL_MS 1000
#endif

#ifndef BLE_HOST_COUNT
#define BLE_HOST_COUNT       3
#endif

/* ============ 功能开关 ============ */

#ifndef BLUETOOTH_ENABLE_FLAG
#define BLUETOOTH_ENABLE_FLAG      TRUE
#endif

/* ============ 连接参数 ============ */

#ifndef BLE_CONNECTION_INTERVAL_MIN_MS
#define BLE_CONNECTION_INTERVAL_MIN_MS  10
#endif

#ifndef BLE_CONNECTION_INTERVAL_MAX_MS
#define BLE_CONNECTION_INTERVAL_MAX_MS  30
#endif

#ifndef BLE_SLAVE_LATENCY
#define BLE_SLAVE_LATENCY    0
#endif

#ifndef BLE_SUPERVISION_TIMEOUT_MS
#define BLE_SUPERVISION_TIMEOUT_MS  1000
#endif

/* ============ 兼容旧命名 ============ */

#define BLE_NAME             BLE_PAIRING_NAME
#define BT_HOST_DEVICES_COUNT BLE_HOST_COUNT
