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
 * @file backlight_hal.c
 * @brief 背光灯HAL层实现
 */

#include "backlight_hal.h"
#include "backlight_config.h"
#include "pwm.h"

/* ============ 内部变量 ============ */

static bool bl_hal_initialized = false;

/* ============ 公共接口实现 ============ */

void bl_hal_init(void) {
    if (bl_hal_initialized) {
        return;
    }

    pwm_init();

#ifdef BACKLIGHT_TYPE_RGB
    if (BACKLIGHT_PIN_R != NO_PIN) {
        pwm_bind_pin(BACKLIGHT_PIN_R, BACKLIGHT_PWM_CHANNEL_R);
    }
    if (BACKLIGHT_PIN_G != NO_PIN) {
        pwm_bind_pin(BACKLIGHT_PIN_G, BACKLIGHT_PWM_CHANNEL_G);
    }
    if (BACKLIGHT_PIN_B != NO_PIN) {
        pwm_bind_pin(BACKLIGHT_PIN_B, BACKLIGHT_PWM_CHANNEL_B);
    }
#else
    if (BACKLIGHT_PIN != NO_PIN) {
        pwm_bind_pin(BACKLIGHT_PIN, BACKLIGHT_PWM_CHANNEL);
    }
#endif

    bl_hal_initialized = true;
}

void bl_hal_deinit(void) {
    if (!bl_hal_initialized) {
        return;
    }

#ifdef BACKLIGHT_TYPE_RGB
    pwm_stop(BACKLIGHT_PWM_CHANNEL_R);
    pwm_stop(BACKLIGHT_PWM_CHANNEL_G);
    pwm_stop(BACKLIGHT_PWM_CHANNEL_B);

    if (BACKLIGHT_PIN_R != NO_PIN) {
        pwm_unbind_pin(BACKLIGHT_PIN_R);
    }
    if (BACKLIGHT_PIN_G != NO_PIN) {
        pwm_unbind_pin(BACKLIGHT_PIN_G);
    }
    if (BACKLIGHT_PIN_B != NO_PIN) {
        pwm_unbind_pin(BACKLIGHT_PIN_B);
    }
#else
    pwm_stop(BACKLIGHT_PWM_CHANNEL);

    if (BACKLIGHT_PIN != NO_PIN) {
        pwm_unbind_pin(BACKLIGHT_PIN);
    }
#endif

    bl_hal_initialized = false;
}

void bl_hal_set_single(uint8_t brightness) {
#ifdef BACKLIGHT_TYPE_SINGLE
    if (!bl_hal_initialized) {
        return;
    }

    uint8_t duty = brightness;
    if (duty > 100) {
        duty = 100;
    }

#if BACKLIGHT_ON_STATE == 0
    duty = 100 - duty;
#endif

    if (duty > 0) {
        pwm_set_duty_cycle(BACKLIGHT_PWM_CHANNEL, duty);
        pwm_start(BACKLIGHT_PWM_CHANNEL);
    } else {
        pwm_stop(BACKLIGHT_PWM_CHANNEL);
    }
#else
    (void)brightness;
#endif
}

void bl_hal_set_rgb(uint8_t r, uint8_t g, uint8_t b) {
#ifdef BACKLIGHT_TYPE_RGB
    if (!bl_hal_initialized) {
        return;
    }

    if (r > 100) r = 100;
    if (g > 100) g = 100;
    if (b > 100) b = 100;

#if BACKLIGHT_ON_STATE == 0
    r = 100 - r;
    g = 100 - g;
    b = 100 - b;
#endif

    if (r > 0 || g > 0 || b > 0) {
        pwm_set_duty_cycle(BACKLIGHT_PWM_CHANNEL_R, r);
        pwm_set_duty_cycle(BACKLIGHT_PWM_CHANNEL_G, g);
        pwm_set_duty_cycle(BACKLIGHT_PWM_CHANNEL_B, b);
        pwm_start(BACKLIGHT_PWM_CHANNEL_R | BACKLIGHT_PWM_CHANNEL_G | BACKLIGHT_PWM_CHANNEL_B);
    } else {
        pwm_stop(BACKLIGHT_PWM_CHANNEL_R | BACKLIGHT_PWM_CHANNEL_G | BACKLIGHT_PWM_CHANNEL_B);
    }
#else
    (void)r;
    (void)g;
    (void)b;
#endif
}
