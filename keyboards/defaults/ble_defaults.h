#pragma once

/**
 * @file ble_defaults.h
 * @brief 蓝牙模块默认配置
 * @note 产品可通过在 config.h 中 #define 覆盖默认值
 */

/* ========== 蓝牙基础参数 ========== */
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

/* ========== 通道配置 ========== */
#ifndef BLE_HOST_COUNT
#define BLE_HOST_COUNT       3
#endif

/* ========== 连接参数 ========== */
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
