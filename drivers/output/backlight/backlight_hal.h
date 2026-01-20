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
 * @file backlight_hal.h
 * @brief 背光灯HAL抽象层接口
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 初始化背光HAL
 */
void bl_hal_init(void);

/**
 * @brief 反初始化背光HAL
 */
void bl_hal_deinit(void);

/**
 * @brief 设置单色灯亮度
 * @param brightness 亮度 0-100
 */
void bl_hal_set_single(uint8_t brightness);

/**
 * @brief 设置RGB灯亮度
 * @param r 红色亮度 0-100
 * @param g 绿色亮度 0-100
 * @param b 蓝色亮度 0-100
 */
void bl_hal_set_rgb(uint8_t r, uint8_t g, uint8_t b);
