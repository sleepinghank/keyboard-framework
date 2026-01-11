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
 * 提供GPIO LED的硬件操作接口，封装底层GPIO操作
 */

#pragma once

#include "gpio.h"
#include <stdbool.h>

/**
 * @brief LED硬件配置
 */
typedef struct {
    pin_t pin;            /**< GPIO引脚 */
    bool  active_high;    /**< true=高电平亮, false=低电平亮 */
} ind_hal_config_t;

/**
 * @brief 初始化LED硬件
 * @param cfg 硬件配置
 *
 * 配置GPIO为输出模式，并设置初始状态为熄灭
 */
void ind_hal_init(const ind_hal_config_t* cfg);

/**
 * @brief 设置LED状态
 * @param pin GPIO引脚
 * @param is_on true=点亮, false=熄灭
 *
 * 根据active_high配置自动转换电平
 */
void ind_hal_set(pin_t pin, bool is_on);

/**
 * @brief 反初始化LED硬件
 * @param pin GPIO引脚
 *
 * 设置为输入模式以节省功耗
 */
void ind_hal_deinit(pin_t pin);
