/* Copyright 2021 QMK
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#include "gpio.h"
#include "pin_defs.h"
#include <stdint.h>

/* 引入CH584标准外设库头文件 */
#include "CH58x_gpio.h"

/**
 * @brief 将标准引脚编号转换为CH584的GPIO位掩码
 *
 * @param pin 标准引脚编号
 * @return CH584 GPIO位掩码
 */
static uint32_t get_gpio_bit_mask(pin_t pin) {
    uint8_t pin_num = GET_GPIO_PIN(pin);
    return (1U << pin_num);
}

/**
 * @brief 设置引脚为输入模式（浮空输入）
 *
 * @param pin 引脚编号
 */
void gpio_set_pin_input(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint32_t pin_bit = get_gpio_bit_mask(pin);

    if (port == PORTA) {
        GPIOA_ModeCfg(pin_bit, GPIO_ModeIN_Floating);
    } else if (port == PORTB) {
        GPIOB_ModeCfg(pin_bit, GPIO_ModeIN_Floating);
    }
}

/**
 * @brief 设置引脚为高电平输入模式（上拉输入）
 *
 * @param pin 引脚编号
 */
void gpio_set_pin_input_high(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint32_t pin_bit = get_gpio_bit_mask(pin);

    if (port == PORTA) {
        GPIOA_ModeCfg(pin_bit, GPIO_ModeIN_PU);
    } else if (port == PORTB) {
        GPIOB_ModeCfg(pin_bit, GPIO_ModeIN_PU);
    }
}

/**
 * @brief 设置引脚为低电平输入模式（下拉输入）
 *
 * @param pin 引脚编号
 */
void gpio_set_pin_input_low(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint32_t pin_bit = get_gpio_bit_mask(pin);

    if (port == PORTA) {
        GPIOA_ModeCfg(pin_bit, GPIO_ModeIN_PD);
    } else if (port == PORTB) {
        GPIOB_ModeCfg(pin_bit, GPIO_ModeIN_PD);
    }
}

/**
 * @brief 设置引脚为推挽输出模式（默认5mA驱动能力）
 *
 * @param pin 引脚编号
 */
void gpio_set_pin_output_push_pull(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint32_t pin_bit = get_gpio_bit_mask(pin);

    if (port == PORTA) {
        /* 使用5mA推挽输出作为默认输出模式 */
        GPIOA_ModeCfg(pin_bit, GPIO_ModeOut_PP_5mA);
    } else if (port == PORTB) {
        /* 使用5mA推挽输出作为默认输出模式 */
        GPIOB_ModeCfg(pin_bit, GPIO_ModeOut_PP_5mA);
    }
}

/**
 * @brief 设置引脚为开漏输出模式
 *
 * @param pin 引脚编号
 * @note CH584芯片GPIO不支持真正的开漏输出模式，这里使用浮空输入模拟
 *       实际使用中需要在外部接上拉电阻
 */
void gpio_set_pin_output_open_drain(pin_t pin) {
    /* CH584不支持开漏输出，使用浮空输入作为替代 */
    /* 实际使用需要在外部连接上拉电阻 */
    gpio_set_pin_input(pin);
}

/**
 * @brief 设置引脚为输出模式（默认推挽输出，5mA）
 *
 * @param pin 引脚编号
 */
void gpio_set_pin_output(pin_t pin) {
    gpio_set_pin_output_push_pull(pin);
}

/**
 * @brief 设置引脚为高电平
 *
 * @param pin 引脚编号
 */
void gpio_write_pin_high(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint32_t pin_bit = get_gpio_bit_mask(pin);

    if (port == PORTA) {
        GPIOA_SetBits(pin_bit);
    } else if (port == PORTB) {
        GPIOB_SetBits(pin_bit);
    }
}

/**
 * @brief 设置引脚为低电平
 *
 * @param pin 引脚编号
 */
void gpio_write_pin_low(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint32_t pin_bit = get_gpio_bit_mask(pin);

    if (port == PORTA) {
        GPIOA_ResetBits(pin_bit);
    } else if (port == PORTB) {
        GPIOB_ResetBits(pin_bit);
    }
}

/**
 * @brief 根据电平值设置引脚状态
 *
 * @param pin 引脚编号
 * @param level 电平值，1为高电平，0为低电平
 */
void gpio_write_pin(pin_t pin, uint8_t level) {
    if (level) {
        gpio_write_pin_high(pin);
    } else {
        gpio_write_pin_low(pin);
    }
}

/**
 * @brief 读取引脚电平状态
 *
 * @param pin 引脚编号
 * @return 引脚当前电平状态（0或1）
 */
uint8_t gpio_read_pin(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint32_t pin_bit = get_gpio_bit_mask(pin);

    if (port == PORTA) {
        /* GPIOA_ReadPortPin 返回非零值表示高电平，0表示低电平 */
        return (GPIOA_ReadPortPin(pin_bit) ? 1 : 0);
    } else if (port == PORTB) {
        /* GPIOB_ReadPortPin 返回非零值表示高电平，0表示低电平 */
        return (GPIOB_ReadPortPin(pin_bit) ? 1 : 0);
    }
    return 0;
}

/**
 * @brief 切换引脚电平状态
 *
 * @param pin 引脚编号
 */
void gpio_toggle_pin(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint32_t pin_bit = get_gpio_bit_mask(pin);

    if (port == PORTA) {
        GPIOA_InverseBits(pin_bit);
    } else if (port == PORTB) {
        GPIOB_InverseBits(pin_bit);
    }
}