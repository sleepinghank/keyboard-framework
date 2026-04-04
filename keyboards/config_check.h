#pragma once

/**
 * @file config_check.h
 * @brief 编译时配置验证
 * @note 在产品 config_product.h 末尾包含此文件进行配置完整性检查
 */

/* ========== TEST_PLATFORM 特殊定义 ========== */
#ifdef TEST_PLATFORM
    /* 为测试平台提供 PRINT 宏定义 */
    #include <stdio.h>
    #define PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#endif

/* ========== 必需配置项验证 ========== */
#ifndef PRODUCT_ID
    #error "PRODUCT_ID must be defined in product config_product.h"
#endif

/* ========== 系统配置验证 ========== */
#ifndef CHIP_TYPE
    #error "CHIP_TYPE must be defined in config_product.h"
#endif

#ifndef PRINTF_ENABLE
    #error "PRINTF_ENABLE must be defined in config_product.h"
#endif

#ifndef USB_ENABLE
    #error "USB_ENABLE must be defined in config_product.h"
#endif

#ifndef BLE_ENABLE
    #error "BLE_ENABLE must be defined in config_product.h"
#endif

/* ========== BLE 广播模式编译期校验 ========== */
#include "ble_adv_policy.h"

/* ========== 矩阵配置验证 ========== */
#ifndef MATRIX_ROWS
    #error "MATRIX_ROWS must be defined in config_hw.h"
#endif

#ifndef MATRIX_COLS
    #error "MATRIX_COLS must be defined in config_hw.h"
#endif

#ifndef MATRIX_ROW_PINS
    #error "MATRIX_ROW_PINS must be defined in config_hw.h"
#endif

#ifndef MATRIX_COL_PINS
    #error "MATRIX_COL_PINS must be defined in config_hw.h"
#endif

/* ========== 范围验证 ========== */
#if MATRIX_ROWS < 1 || MATRIX_ROWS > 16
    #error "MATRIX_ROWS must be 1-16"
#endif

#if MATRIX_COLS < 1 || MATRIX_COLS > 32
    #error "MATRIX_COLS must be 1-32"
#endif



#if BLE_HOST_COUNT < 1 || BLE_HOST_COUNT > 5
    #error "BLE_HOST_COUNT must be 1-5"
#endif

/* ========== 依赖验证 ========== */
#ifdef TOUCHPAD_ENABLE
    #ifndef TOUCHPAD_SDA
        #error "TOUCHPAD_ENABLE requires TOUCHPAD_SDA definition"
    #endif
    #ifndef TOUCHPAD_SCL
        #error "TOUCHPAD_ENABLE requires TOUCHPAD_SCL definition"
    #endif
    #ifndef TOUCHPAD_INT
        #error "TOUCHPAD_ENABLE requires TOUCHPAD_INT definition"
    #endif
#endif

#ifdef BACKLIGHT_TYPE_RGB
    #ifndef BACKLIGHT_PIN_R
        #error "BACKLIGHT_TYPE_RGB requires BACKLIGHT_PIN_R definition"
    #endif
    #ifndef BACKLIGHT_PIN_G
        #error "BACKLIGHT_TYPE_RGB requires BACKLIGHT_PIN_G definition"
    #endif
    #ifndef BACKLIGHT_PIN_B
        #error "BACKLIGHT_TYPE_RGB requires BACKLIGHT_PIN_B definition"
    #endif

    #if BACKLIGHT_LEVEL_COUNT < 1 || BACKLIGHT_LEVEL_COUNT > 10
    #error "BACKLIGHT_LEVEL_COUNT must be 1-10"
    #endif

    #if BACKLIGHT_COLOR_COUNT < 1 || BACKLIGHT_COLOR_COUNT > 20
        #error "BACKLIGHT_COLOR_COUNT must be 1-20"
    #endif
#endif

/* ========== 配置一致性检查 ========== */
#if (BACKLIGHT_IDLE_TIMEOUT_MS / 1000) > (POWER_DEEP_SLEEP_MS / 1000 / 60)
    #warning "BACKLIGHT_IDLE_TIMEOUT_MS should be less than POWER_DEEP_SLEEP_MS"
#endif

// #if (CHIP_TYPE == CHIP_CH584M)
//     #ifndef FREQ_SYS
//         #error "CH584 工程必须在编译参数中定义 FREQ_SYS"
//     #endif

//     #if (SYSTEM_CLOCK_SOURCE == CLK_SOURCE_HSE_PLL_62_4MHz)
//         #if (FREQ_SYS != 62400000UL)
//             #error "SYSTEM_CLOCK_SOURCE 与 FREQ_SYS 不匹配"
//         #endif
//     #elif (SYSTEM_CLOCK_SOURCE == CLK_SOURCE_HSE_PLL_78MHz)
//         #if (FREQ_SYS != 78000000UL)
//             #error "SYSTEM_CLOCK_SOURCE 与 FREQ_SYS 不匹配"
//         #endif
//     #else
//         #error "当前 SYSTEM_CLOCK_SOURCE 尚未建立 FREQ_SYS 映射"
//     #endif
// #endif

