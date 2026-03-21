#pragma once

/**
 * @file config.h
 * @brief KB904 产品配置
 * @note 仅定义与默认值不同的配置项
 *       覆盖值必须先于 defaults/*.h 定义（#ifndef 保护机制）
 */

/* ========================================
 * KB904 产品配置
 * 仅定义与默认值不同的配置项
 * ======================================== */

/* ========== 产品标识 ========== */
#define PRODUCT_ID_VALUE     0x0904
#define PRODUCT_NAME         "inateck NeoMagic Pro"
#define VENDOR_ID_VALUE      0x0002
#define MANUFACTURER         "inateck"
#define DEVICE_VERSION       0x0100

/* ========== 功能开关 ========== */
#define BLUETOOTH_ENABLE_FLAG      TRUE
// #define USB_ENABLE_FLAG      FALSE
// #define P2P4G_ENABLE_FLAG    FALSE
#define TOUCHPAD_ENABLE      TRUE
#define COMBO_ENABLE         TRUE

/* ========== 覆盖背光默认值 ========== */
#define BACKLIGHT_TYPE_RGB   /* 使用 RGB 模式 */
#define BACKLIGHT_COLOR_COUNT 13
#define BACKLIGHT_DEFAULT_COLOR  BL_COLOR_WHITE
#define BACKLIGHT_DEFAULT_LEVEL  2
#define BACKLIGHT_IDLE_TIMEOUT_MS   5000

/* ========== 覆盖蓝牙默认值 ========== */
#define BLE_PAIRING_NAME     "inateck NeoMagic Pro"
#define BLE_HOST_COUNT       1  // 单通道

/* ========== 覆盖电源默认值 ========== */
#define POWER_DEEP_SLEEP_MS  (10 * 60 * 1000)  // 10分钟

/* ========== 覆盖指示灯默认值 ========== */
#define IND_LED_COUNT        3

/* ========== EEPROM 存储配置 ========== */
#define EEPROM_CUSTOM                          // 使用自定义 EEPROM 驱动
#define EEPROM_SIZE          512               // EEPROM 大小（字节）
#define EEPROM_BASE_ADDR     0x0000            // EEPROM 基地址

/* ========== 包含默认配置（使用 #ifndef 保护，上述覆盖值生效） ========== */
#include "defaults/backlight_defaults.h"
#include "defaults/ble_defaults.h"
#include "defaults/indicator_defaults.h"
#include "defaults/keyboard_defaults.h"
#include "defaults/power_defaults.h"
#include "defaults/matrix_defaults.h"

/* ========== 包含硬件配置 ========== */
#include "config_hw.h"

/* ========== 产品 ID 宏（供条件编译使用） ========== */
#ifndef PRODUCT_ID
#define PRODUCT_ID           PRODUCT_ID_VALUE
#endif

/* ========== 配置验证 ========== */
#include "config_check.h"
