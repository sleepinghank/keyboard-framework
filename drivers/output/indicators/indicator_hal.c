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
 * @brief 指示灯硬件抽象层实现 (GPIO 版本)
 */

#include "indicator_hal.h"
#include "indicator_config.h"
#include "gpio.h"

void ind_hal_init(void) {
    for (uint8_t i = 0; i < IND_LED_COUNT; i++) {
        const ind_led_def_t* led = &ind_led_table[i];

        // 配置为推挽输出
        gpio_set_pin_output(led->pin);

        // 初始状态：熄灭
        ind_hal_set(i, false);
    }
}

void ind_hal_set(uint8_t index, bool on) {
    if (index >= IND_LED_COUNT) {
        return;
    }

    const ind_led_def_t* led = &ind_led_table[index];

    // 根据极性转换电平
    // active_high: on=高电平, off=低电平
    // active_low:  on=低电平, off=高电平
    bool level = led->active_high ? on : !on;

    gpio_write_pin(led->pin, level ? 1 : 0);
}

void ind_hal_deinit(void) {
    for (uint8_t i = 0; i < IND_LED_COUNT; i++) {
        const ind_led_def_t* led = &ind_led_table[i];

        // 先熄灭
        ind_hal_set(i, false);

        // 设置为输入模式（低功耗）
        gpio_set_pin_input(led->pin);
    }
}
