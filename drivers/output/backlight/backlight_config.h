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
 * @file backlight_config.h
 * @brief 背光模块配置
 *
 * 配置加载顺序：
 * 1. 产品配置 (keyboards/kb904/config.h) - 定义覆盖值
 * 2. 默认配置 (defaults/backlight_defaults.h) - 使用 #ifndef 保护
 * 3. 本文件 - 硬件引脚和 PWM 通道配置
 */

#pragma once

#include "kb904/config.h"
#include "pwm.h"

/* ============ 默认配置加载 ============ */
/* 如果使用新的配置系统，defaults 在产品 config.h 中已包含 */
/* 否则从这里加载默认值 */

#ifndef BACKLIGHT_DEFAULT_ON
/* 旧配置系统兼容：在此定义默认值 */
#define BACKLIGHT_DEFAULT_ON     true
#define BACKLIGHT_DEFAULT_BRIGHTNESS 100
#define BACKLIGHT_STEP           10
#define BACKLIGHT_LEVEL_LOW      30
#define BACKLIGHT_LEVEL_MEDIUM   60
#define BACKLIGHT_LEVEL_HIGH     100
#define BACKLIGHT_ON_STATE       1
#endif

/* ============ 背光类型配置 ============ */

// 在产品 config.h 中定义以下宏之一:
// #define BACKLIGHT_TYPE_SINGLE   // 单色灯模式
// #define BACKLIGHT_TYPE_RGB      // RGB三色灯模式

#if !defined(BACKLIGHT_TYPE_SINGLE) && !defined(BACKLIGHT_TYPE_RGB)
#define BACKLIGHT_TYPE_SINGLE
#endif

/* ============ 引脚配置 ============ */
/* 这些配置与硬件相关，由 config_hw.h 或 product_config.h 定义 */

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
/* PWM 通道映射，与具体 MCU 相关 */

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

/* ============ 亮度预设宏 ============ */
/* 供 backlight.c 预设亮度表使用 */

#ifndef BACKLIGHT_LEVEL_LOW_VAL
#define BACKLIGHT_LEVEL_LOW_VAL    BACKLIGHT_LEVEL_LOW
#endif

#ifndef BACKLIGHT_LEVEL_MEDIUM_VAL
#define BACKLIGHT_LEVEL_MEDIUM_VAL BACKLIGHT_LEVEL_MEDIUM
#endif

#ifndef BACKLIGHT_LEVEL_HIGH_VAL
#define BACKLIGHT_LEVEL_HIGH_VAL   BACKLIGHT_LEVEL_HIGH
#endif
