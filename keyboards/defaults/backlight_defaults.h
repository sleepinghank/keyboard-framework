#pragma once

/**
 * @file backlight_defaults.h
 * @brief 背光模块默认配置
 * @note 产品可通过在 config_product.h 中 #define 覆盖默认值
 */

/* ========== 背光类型 ========== */
#ifndef BACKLIGHT_TYPE_SINGLE
#ifndef BACKLIGHT_TYPE_RGB
#define BACKLIGHT_TYPE_SINGLE
#endif
#endif

/* ========== 背光颜色数量 ========== */
#ifndef BACKLIGHT_COLOR_COUNT
#define BACKLIGHT_COLOR_COUNT    7
#endif

/* ========== 亮度档位数量 ========== */
#ifndef BACKLIGHT_LEVEL_COUNT
#define BACKLIGHT_LEVEL_COUNT    4
#endif

/* ========== 默认值 ========== */
#ifndef BACKLIGHT_DEFAULT_ON
#define BACKLIGHT_DEFAULT_ON     true
#endif

#ifndef BACKLIGHT_DEFAULT_BRIGHTNESS
#define BACKLIGHT_DEFAULT_BRIGHTNESS 100
#endif

#ifndef BACKLIGHT_DEFAULT_COLOR
#define BACKLIGHT_DEFAULT_COLOR  BL_COLOR_WHITE
#endif

#ifndef BACKLIGHT_DEFAULT_LEVEL
#define BACKLIGHT_DEFAULT_LEVEL  2
#endif

/* ========== 亮度步进 ========== */
#ifndef BACKLIGHT_STEP
#define BACKLIGHT_STEP           10
#endif

/* ========== 预设亮度值 ========== */
#ifndef BACKLIGHT_LEVEL_LOW
#define BACKLIGHT_LEVEL_LOW      10
#endif

#ifndef BACKLIGHT_LEVEL_MEDIUM
#define BACKLIGHT_LEVEL_MEDIUM   15
#endif

#ifndef BACKLIGHT_LEVEL_HIGH
#define BACKLIGHT_LEVEL_HIGH     20
#endif

/* ========== LED 极性 ========== */
#ifndef BACKLIGHT_ON_STATE
#define BACKLIGHT_ON_STATE       1
#endif

/* ========== 休眠配置 ========== */
#ifndef BACKLIGHT_IDLE_TIMEOUT_MS
#define BACKLIGHT_IDLE_TIMEOUT_MS   5000
#endif

/* ========== 低电量阈值 ========== */
#ifndef BACKLIGHT_LOW_BATTERY_THRESHOLD
#define BACKLIGHT_LOW_BATTERY_THRESHOLD  20
#endif
