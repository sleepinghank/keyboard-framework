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
 * @file backlight.c
 * @brief 背光灯驱动实现
 */

#include "backlight.h"
#include "backlight_hal.h"
#include "backlight_config.h"
#include "string.h"
/* ============ 预设颜色表 ============ */

static const bl_rgb_t preset_colors[BL_COLOR_COUNT] = {
    [BL_COLOR_RED]     = {100,   0,   0},
    [BL_COLOR_YELLOW]  = {100, 100,   0},
    [BL_COLOR_CYAN]    = {  0, 100, 100},
    [BL_COLOR_BLUE]    = {  0,   0, 100},
    [BL_COLOR_GREEN]   = {  0, 100,   0},
    [BL_COLOR_WHITE]   = {100, 100, 100},
    [BL_COLOR_MAGENTA] = {100,   0, 100},
};

/* ============ 预设亮度表 ============ */

static const uint8_t preset_levels[BL_LEVEL_COUNT] = {
    [BL_LEVEL_OFF]    = 0,
    [BL_LEVEL_LOW]    = BACKLIGHT_LEVEL_LOW,
    [BL_LEVEL_MEDIUM] = BACKLIGHT_LEVEL_MEDIUM,
    [BL_LEVEL_HIGH]   = BACKLIGHT_LEVEL_HIGH,
};

/* ============ 模块状态 ============ */

static bl_state_t bl_state;
static bl_preset_color_t bl_current_color_index = BL_COLOR_WHITE;
static bl_preset_level_t bl_current_level_index = BL_LEVEL_HIGH;
static bool bl_initialized = false;

/* ============ 内部函数 ============ */

static void apply_output(void) {
    if (!bl_state.enable) {
#ifdef BACKLIGHT_TYPE_RGB
        bl_hal_set_rgb(0, 0, 0);
#else
        bl_hal_set_single(0);
#endif
        return;
    }

#ifdef BACKLIGHT_TYPE_RGB
    uint8_t r = (uint16_t)bl_state.color.r * bl_state.brightness / 100;
    uint8_t g = (uint16_t)bl_state.color.g * bl_state.brightness / 100;
    uint8_t b = (uint16_t)bl_state.color.b * bl_state.brightness / 100;
    bl_hal_set_rgb(r, g, b);
#else
    bl_hal_set_single(bl_state.brightness);
#endif
}

/* ============ 初始化 ============ */

void backlight_init(const bl_state_t* state) {
    if (bl_initialized) {
        return;
    }

    bl_hal_init();

    if (state != NULL) {
        bl_state = *state;
    } else {
        bl_state.enable = BACKLIGHT_DEFAULT_ON;
        bl_state.brightness = BACKLIGHT_DEFAULT_BRIGHTNESS;
        bl_state.color = preset_colors[BL_COLOR_WHITE];
    }

    apply_output();
    bl_initialized = true;
}

void backlight_deinit(void) {
    if (!bl_initialized) {
        return;
    }

    bl_state.enable = false;
    apply_output();
    bl_hal_deinit();
    bl_initialized = false;
}

/* ============ 开关控制 ============ */

void backlight_enable(void) {
    if (!bl_initialized || bl_state.enable) {
        return;
    }

    bl_state.enable = true;
    apply_output();
}

void backlight_disable(void) {
    if (!bl_initialized || !bl_state.enable) {
        return;
    }

    bl_state.enable = false;
    apply_output();
}

void backlight_toggle(void) {
    if (!bl_initialized) {
        return;
    }

    bl_state.enable = !bl_state.enable;
    apply_output();
}

bool backlight_is_enabled(void) {
    return bl_state.enable;
}

/* ============ 亮度控制 ============ */

void backlight_set_brightness(uint8_t brightness) {
    if (!bl_initialized) {
        return;
    }

    if (brightness > 100) {
        brightness = 100;
    }

    bl_state.brightness = brightness;
    apply_output();
}

uint8_t backlight_get_brightness(void) {
    return bl_state.brightness;
}

void backlight_brightness_increase(uint8_t step) {
    if (!bl_initialized) {
        return;
    }

    uint8_t new_val = bl_state.brightness + step;
    if (new_val > 100) {
        new_val = 100;
    }

    bl_state.brightness = new_val;
    bl_state.enable = true;
    apply_output();
}

void backlight_brightness_decrease(uint8_t step) {
    if (!bl_initialized) {
        return;
    }

    if (bl_state.brightness <= step) {
        bl_state.brightness = 0;
    } else {
        bl_state.brightness -= step;
    }

    apply_output();
}

void backlight_set_preset_level(bl_preset_level_t level) {
    if (!bl_initialized || level >= BL_LEVEL_COUNT) {
        return;
    }

    bl_current_level_index = level;
    bl_state.brightness = preset_levels[level];

    if (level == BL_LEVEL_OFF) {
        bl_state.enable = false;
    } else {
        bl_state.enable = true;
    }

    apply_output();
}

void backlight_level_step(void) {
    if (!bl_initialized) {
        return;
    }

    bl_current_level_index++;
    if (bl_current_level_index >= BL_LEVEL_COUNT) {
        bl_current_level_index = BL_LEVEL_OFF;
    }

    backlight_set_preset_level(bl_current_level_index);
}

/* ============ 颜色控制 ============ */

void backlight_set_color(const bl_rgb_t* rgb) {
    if (!bl_initialized || rgb == NULL) {
        return;
    }

    bl_state.color = *rgb;
    apply_output();
}

void backlight_set_rgb(uint8_t r, uint8_t g, uint8_t b) {
    if (!bl_initialized) {
        return;
    }

    if (r > 100) r = 100;
    if (g > 100) g = 100;
    if (b > 100) b = 100;

    bl_state.color.r = r;
    bl_state.color.g = g;
    bl_state.color.b = b;
    apply_output();
}

void backlight_get_color(bl_rgb_t* rgb) {
    if (rgb == NULL) {
        return;
    }

    *rgb = bl_state.color;
}

void backlight_set_channel(char channel, uint8_t value) {
    if (!bl_initialized) {
        return;
    }

    if (value > 100) {
        value = 100;
    }

    switch (channel) {
        case 'r':
        case 'R':
            bl_state.color.r = value;
            break;
        case 'g':
        case 'G':
            bl_state.color.g = value;
            break;
        case 'b':
        case 'B':
            bl_state.color.b = value;
            break;
        default:
            return;
    }

    apply_output();
}

void backlight_set_preset_color(bl_preset_color_t color) {
    if (!bl_initialized || color >= BL_COLOR_COUNT) {
        return;
    }

    bl_current_color_index = color;
    bl_state.color = preset_colors[color];
    apply_output();
}

void backlight_color_step(void) {
    if (!bl_initialized) {
        return;
    }

    bl_current_color_index++;
    if (bl_current_color_index >= BL_COLOR_COUNT) {
        bl_current_color_index = BL_COLOR_RED;
    }

    backlight_set_preset_color(bl_current_color_index);
}

/* ============ 状态管理 ============ */

void backlight_get_state(bl_state_t* state) {
    if (state == NULL) {
        return;
    }

    *state = bl_state;
}

void backlight_restore_state(const bl_state_t* state) {
    if (!bl_initialized || state == NULL) {
        return;
    }

    bl_state = *state;
    apply_output();
}
