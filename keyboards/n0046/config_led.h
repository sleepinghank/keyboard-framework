#pragma once

/**
 * @file config_led.h
 * @brief N0046 LED 配置
 */

/* ========== 指示灯配置 ========== */
#define IND_LED_COUNT        2
#define IND_LED_CAPS         0
#define IND_LED_POWER        1

/* ========== 背光配置 ========== */
#define BACKLIGHT_TYPE_RGB
#define BACKLIGHT_DEFAULT_ON          TRUE
#define BACKLIGHT_DEFAULT_BRIGHTNESS  66
#define BACKLIGHT_SLEEP_TIMEOUT       5000

typedef enum {
    BL_COLOR_RED, BL_COLOR_YELLOW, BL_COLOR_CYAN,
    BL_COLOR_BLUE, BL_COLOR_GREEN, BL_COLOR_WHITE, BL_COLOR_MAGENTA,
    BL_COLOR_COUNT
} bl_preset_color_t;

typedef enum {
    BL_LEVEL_OFF, BL_LEVEL_LOW, BL_LEVEL_MEDIUM, BL_LEVEL_HIGH, BL_LEVEL_COUNT
} bl_preset_level_t;
