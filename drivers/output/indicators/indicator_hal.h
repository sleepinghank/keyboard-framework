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
 * @file indicator_hal.h
 * @brief 指示灯硬件抽象层
 *
 * 提供 LED 硬件操作接口，封装底层 GPIO 操作
 * HAL 层内部处理极性转换，上层只需传入逻辑状态
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 初始化所有 LED 硬件
 *
 * 根据配置表初始化所有 LED 引脚为输出模式，并设置初始状态为熄灭
 */
void ind_hal_init(void);

/**
 * @brief 设置 LED 状态
 * @param index LED 索引
 * @param on true=点亮, false=熄灭
 *
 * HAL 层内部根据配置表自动处理 active_high/active_low 极性转换
 */
void ind_hal_set(uint8_t index, bool on);

/**
 * @brief 反初始化所有 LED 硬件
 *
 * 将所有 LED 引脚设置为输入模式以节省功耗
 */
void ind_hal_deinit(void);
