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

#pragma once

#include "product_config.h"
#include "pwm.h"

/* ============ 背光类型配置 ============ */

// 在 product_config.h 中定义以下宏之一:
// #define BACKLIGHT_TYPE_SINGLE   // 单色灯模式
#define BACKLIGHT_TYPE_RGB      // RGB三色灯模式

#if !defined(BACKLIGHT_TYPE_SINGLE) && !defined(BACKLIGHT_TYPE_RGB)
#define BACKLIGHT_TYPE_SINGLE
#endif

/* ============ 引脚配置 ============ */

#ifndef BACKLIGHT_PIN
#define BACKLIGHT_PIN NO_PIN
#endif

#ifndef BACKLIGHT_PIN_R
#define BACKLIGHT_PIN_R NO_PIN
#endif

#ifndef BACKLIGHT_PIN_G
#define BACKLIGHT_PIN_G NO_PIN
#endif

#ifndef BACKLIGHT_PIN_B
#define BACKLIGHT_PIN_B NO_PIN
#endif

/* ============ PWM通道配置 ============ */

#ifndef BACKLIGHT_PWM_CHANNEL
#define BACKLIGHT_PWM_CHANNEL PWM_CHANNEL_0
#endif

#ifndef BACKLIGHT_PWM_CHANNEL_R
#define BACKLIGHT_PWM_CHANNEL_R PWM_CHANNEL_0
#endif

#ifndef BACKLIGHT_PWM_CHANNEL_G
#define BACKLIGHT_PWM_CHANNEL_G PWM_CHANNEL_1
#endif

#ifndef BACKLIGHT_PWM_CHANNEL_B
#define BACKLIGHT_PWM_CHANNEL_B PWM_CHANNEL_2
#endif

/* ============ 默认值配置 ============ */

#ifndef BACKLIGHT_DEFAULT_BRIGHTNESS
#define BACKLIGHT_DEFAULT_BRIGHTNESS 100
#endif

#ifndef BACKLIGHT_DEFAULT_ON
#define BACKLIGHT_DEFAULT_ON true
#endif

#ifndef BACKLIGHT_STEP
#define BACKLIGHT_STEP 10
#endif

/* ============ 预设亮度值 ============ */

#ifndef BACKLIGHT_LEVEL_LOW
#define BACKLIGHT_LEVEL_LOW 30
#endif

#ifndef BACKLIGHT_LEVEL_MEDIUM
#define BACKLIGHT_LEVEL_MEDIUM 60
#endif

#ifndef BACKLIGHT_LEVEL_HIGH
#define BACKLIGHT_LEVEL_HIGH 100
#endif

/* ============ LED极性配置 ============ */

#ifndef BACKLIGHT_ON_STATE
#define BACKLIGHT_ON_STATE 1
#endif
