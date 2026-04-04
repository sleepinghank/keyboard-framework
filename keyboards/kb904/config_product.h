#pragma once

/**
 * @file config_product.h
 * @brief KB904 产品配置（合并 sys_config.h）
 * @note 配置入口统一在此文件
 */

#include "system_enums.h"
#include "ISP585.h"
#include "CONFIG.h"
/* ============================================================
 * 第一部分：芯片与系统配置（原 sys_config.h）
 * ============================================================ */

/* ---------- 芯片型号 ---------- */
#define CHIP_TYPE          CHIP_CH584M

/* ---------- 日志配置 ---------- */
// 在编译环境中启用
#ifdef DEBUG
#define PRINTF_ENABLE TRUE
#else
#define PRINTF_ENABLE FALSE
#endif

#define INDICATOR_TEST_ENABLE

#if (PRINTF_ENABLE == TRUE)
    /* --- 业务日志级别（控制 LOG_E/LOG_W/LOG_I/LOG_D 输出）--- */
    #define LOG_LEVEL               LOG_LEVEL_DEBUG

    #define PRINTF_UART               DEBUG
    #define PRINTF_BAUDRATE           115200
    #define LOG_NEWLINE_SIGN          "\r\n"

#endif

#if (CHIP_TYPE == CHIP_CH584M)
#include "CH58x_common.h"
    #ifndef SYSTEM_CLOCK_SOURCE
    #define SYSTEM_CLOCK_SOURCE  CLK_SOURCE_HSE_PLL_62_4MHz
    #endif
    #ifndef __HIGH_CODE
    #define __HIGH_CODE  __attribute__((section(".highcode")))
    #endif
#else
    #define __HIGH_CODE
#endif

/* ---------- 功能总开关 ---------- */
#define USB_ENABLE         FALSE
#define BLE_ENABLE         TRUE
#define TOUCHPAD_ENABLE     TRUE
#define	TP_MAX_CONTACT_COUNT							4 // 最大总触摸点数

/* ============================================================
 * 第二部分：产品标识
 * ============================================================ */

#define PRODUCT_ID_VALUE           0x0904
#define ProductModel               "inateck NeoMagic"
#define ChipModel                  "PAR2860"
#define ProductModelCode           (_PRODUCT_inateck_NeoMagic_SE_US)
#define ChipModelCode              (_CHIP_CH584M)
#define HardwareVersion            (0x010000)
#define SoftwareVersion            (0x000001)
#define ProtocolVersion            (0x010500)
#define EncryptFlag                (0x00)

#define BLE_PAIRING_NAME           "inateck NeoMagic Pro"
#define BLE_HOST_COUNT             1
/* ============================================================
 * 第三部分：功能开关与参数覆盖
 * ============================================================ */

#define BLUETOOTH_ENABLE_FLAG      TRUE
#define COMBO_ENABLE               TRUE

#define BACKLIGHT_TYPE_RGB
#define BACKLIGHT_COLOR_COUNT      13
#define BACKLIGHT_DEFAULT_COLOR    BL_COLOR_WHITE
#define BACKLIGHT_DEFAULT_LEVEL    2
#define BACKLIGHT_IDLE_TIMEOUT_MS  5000
#define POWER_DEEP_SLEEP_MS        (10 * 60 * 1000)
#define IND_LED_COUNT              3

#define EEPROM_CUSTOM
#define EEPROM_SIZE                512
#define EEPROM_BASE_ADDR           (0x76000 - FLASH_ROM_MAX_SIZE)

/* ============================================================
 * 第四部分：默认值包含（#ifndef 保护）
 * ============================================================ */

#include "defaults/backlight_defaults.h"
#include "defaults/ble_defaults.h"
#include "defaults/indicator_defaults.h"
#include "defaults/keyboard_defaults.h"
#include "defaults/power_defaults.h"

/* 启用鬼键检测（KB904 无二极管矩阵，需要防止幽灵键） */
#define MATRIX_HAS_GHOST

#include "defaults/matrix_defaults.h"

/* ============================================================
 * 第五部分：硬件配置
 * ============================================================ */

#include "config_hw.h"

/* ============================================================
 * 第六部分：国家/地区配置
 * ============================================================ */

#define KEYBOARD_COUNTRY  COUNTRY_US

#define FirmwareCode       "NeoMagic"
#define FirmwareType       "beta"
#define CountryCode        COUNTRY_CODE_STRING
#define ExternalCode       "N0046"
#define MANUFACTURER_NAME_STRING  "Inateck Inc."

/* ============================================================
 * 第七部分：产品 ID 与配置验证
 * ============================================================ */

#ifndef PRODUCT_ID
#define PRODUCT_ID         PRODUCT_ID_VALUE
#endif

#include "config_check.h"
