#pragma once

/**
 * @file config.h
 * @brief N0046 产品配置
 */

/* ========== 产品信息 ========== */
#define PRODUCT_ID           0x0046
#define VENDOR_ID            0x3554
#define MANUFACTURER         "inateck"
#define PRODUCT              "inateck AceTouch"
#define DEVICE_VER           0x0100

#include "config_gpio.h"
#include "config_matrix.h"

/* ========== 功能开关 ========== */
#define USB_ENABLE_FLAG      FALSE
#define BLUETOOTH_ENABLE     TRUE
#define BLUETOOTH_HOST_COUNT 1

/* ========== 蓝牙配置 ========== */
#define BLE_DEVICE_NAME      "inateck AceTouch"
#define BLE_PAIRING_TIMEOUT  60000
#define BLE_RECONN_TIMEOUT   60000
#define BLE_IDLE_TIMEOUT     600000

/* ========== 电池配置 ========== */
#define RVD_R1               560
#define RVD_R2               2000
#define BATTERY_LOW_THRESHOLD 20

/* ========== 按键配置 ========== */
#define TAPPING_TERM         200
#define COMBO_TERM           200
#define COMBO_ENABLE
#define EXTRAKEY_ENABLE
