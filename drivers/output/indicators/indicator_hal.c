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
 * @file indicator_hal.c
 * @brief 指示灯硬件抽象层实现
 */

#include "indicator_hal.h"

void ind_hal_init(const ind_hal_config_t* cfg) {
    if (cfg == NULL) return;

    // 设置GPIO为输出
    gpio_set_pin_output(cfg->pin);

    // 初始状态：熄灭
    // active_high=true时，低电平熄灭
    // active_high=false时，高电平熄灭
    gpio_write_pin(cfg->pin, cfg->active_high ? 0 : 1);
}

void ind_hal_set(pin_t pin, bool is_on) {
    // 这里简化处理，电平直接由is_on决定
    // active_high的转换在调用层处理
    gpio_write_pin(pin, is_on ? 1 : 0);
}

void ind_hal_deinit(pin_t pin) {
    // 设置为输入（低功耗）
    gpio_set_pin_input(pin);
}
