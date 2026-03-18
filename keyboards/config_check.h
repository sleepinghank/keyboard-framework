#pragma once

/**
 * @file config_check.h
 * @brief 编译时配置验证
 * @note 在产品 config.h 末尾包含此文件进行配置完整性检查
 */

/* ========== 必需配置项验证 ========== */
#ifndef PRODUCT_ID
    #error "PRODUCT_ID must be defined in product config.h"
#endif

#ifndef PRODUCT_NAME
    #error "PRODUCT_NAME must be defined in product config.h"
#endif

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

#if BACKLIGHT_LEVEL_COUNT < 1 || BACKLIGHT_LEVEL_COUNT > 10
    #error "BACKLIGHT_LEVEL_COUNT must be 1-10"
#endif

#if BACKLIGHT_COLOR_COUNT < 1 || BACKLIGHT_COLOR_COUNT > 20
    #error "BACKLIGHT_COLOR_COUNT must be 1-20"
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
#endif

/* ========== GPIO 冲突检测 ========== */
/* 注意：C 预处理器无法直接比较宏值，以下检测需要在实际使用时实现 */
#if defined(LED_CAPS_PIN) && defined(LED_BT_PIN)
    #if LED_CAPS_PIN == LED_BT_PIN
        #error "GPIO conflict: LED_CAPS_PIN and LED_BT_PIN use the same pin"
    #endif
#endif

#if defined(LED_CAPS_PIN) && defined(LED_POWER_PIN)
    #if LED_CAPS_PIN == LED_POWER_PIN
        #error "GPIO conflict: LED_CAPS_PIN and LED_POWER_PIN use the same pin"
    #endif
#endif

#if defined(LED_BT_PIN) && defined(LED_POWER_PIN)
    #if LED_BT_PIN == LED_POWER_PIN
        #error "GPIO conflict: LED_BT_PIN and LED_POWER_PIN use the same pin"
    #endif
#endif

/* ========== 配置一致性检查 ========== */
#if (BACKLIGHT_IDLE_TIMEOUT_MS / 1000) > (POWER_DEEP_SLEEP_MS / 1000 / 60)
    #warning "BACKLIGHT_IDLE_TIMEOUT_MS should be less than POWER_DEEP_SLEEP_MS"
#endif
