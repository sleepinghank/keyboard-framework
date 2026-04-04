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
 * @file power_config.h
 * @brief 电源管理模块配置
 *
 * 配置加载顺序：
 * 1. 产品配置 (keyboards/kb904/config_product.h) - 定义覆盖值
 * 2. 默认配置 (defaults/power_defaults.h) - 使用 #ifndef 保护
 * 3. 本文件 - 电源驱动配置
 */

#pragma once

#include "kb904/config_product.h"

/* ============ 默认配置加载 ============ */
/* 如果使用新的配置系统，defaults 在产品 config_product.h 中已包含 */
/* 否则使用以下默认值 */

#ifndef POWER_DEEP_SLEEP_MS
#define POWER_DEEP_SLEEP_MS  (10 * 60 * 1000)  // 10分钟
#endif

#ifndef POWER_IDLE_SLEEP_MS
#define POWER_IDLE_SLEEP_MS (5 * 60 * 1000)   // 5分钟
#endif

#ifndef POWER_LOW_BATTERY_PERCENT
#define POWER_LOW_BATTERY_PERCENT  20
#endif

#ifndef POWER_CRITICAL_BATTERY_PERCENT
#define POWER_CRITICAL_BATTERY_PERCENT  5
#endif

#ifndef BATTERY_CAPACITY_MAH
#define BATTERY_CAPACITY_MAH  500
#endif

#ifndef BATTERY_ADC_SAMPLES
#define BATTERY_ADC_SAMPLES   8
#endif

#ifndef BATTERY_CHECK_INTERVAL_MS
#define BATTERY_CHECK_INTERVAL_MS  60000
#endif

/* ============ 兼容 LPM 模块命名 ============ */

#ifndef LPM_IDLE_TIMEOUT_MS
#define LPM_IDLE_TIMEOUT_MS  POWER_IDLE_SLEEP_MS
#endif

#ifndef LPM_DEEP_TIMEOUT_MS
#define LPM_DEEP_TIMEOUT_MS  POWER_DEEP_SLEEP_MS
#endif

/* ============ 兼容旧命名 ============ */

#define CONNECTED_IDLE_TIME  (LPM_DEEP_TIMEOUT_MS / 1000)

#ifndef CONNECTED_BACKLIGHT_DISABLE_TIMEOUT
#define CONNECTED_BACKLIGHT_DISABLE_TIMEOUT  60  // 连接状态下背光关闭超时（秒）
#endif

#ifndef DISCONNECTED_BACKLIGHT_DISABLE_TIMEOUT
#define DISCONNECTED_BACKLIGHT_DISABLE_TIMEOUT  30  // 断开状态下背光关闭超时（秒）
#endif
