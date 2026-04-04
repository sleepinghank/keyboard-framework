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

/**
 * @file _gpio.c
 * @brief GPIO HAL 层测试平台实现
 */

#include "gpio.h"
#include "pin_defs.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "debug.h"

/* 调试输出宏 */
#define DEBUG_PRINT(fmt, ...) printf("[GPIO_TEST] " fmt "\n", ##__VA_ARGS__)

/* 模拟的 GPIO 状态 */
static uint8_t gpio_porta_state = 0;
static uint8_t gpio_portb_state = 0;
static uint8_t gpio_porta_dir = 0;   /* 0=input, 1=output */
static uint8_t gpio_portb_dir = 0;

/**
 * @brief 设置引脚为输入模式（浮空输入）
 */
void gpio_set_pin_input(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint8_t pin_num = GET_GPIO_PIN(pin);

    DEBUG_PRINT("gpio_set_pin_input(pin: 0x%02X, port: %d, num: %d)", pin, port, pin_num);

    if (port == PORTA) {
        gpio_porta_dir &= ~(1 << pin_num);
    } else if (port == PORTB) {
        gpio_portb_dir &= ~(1 << pin_num);
    }
}

/**
 * @brief 设置引脚为高电平输入模式（上拉输入）
 */
void gpio_set_pin_input_high(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint8_t pin_num = GET_GPIO_PIN(pin);

    DEBUG_PRINT("gpio_set_pin_input_high(pin: 0x%02X, port: %d, num: %d)", pin, port, pin_num);

    if (port == PORTA) {
        gpio_porta_dir &= ~(1 << pin_num);
        gpio_porta_state |= (1 << pin_num);  /* 上拉 */
    } else if (port == PORTB) {
        gpio_portb_dir &= ~(1 << pin_num);
        gpio_portb_state |= (1 << pin_num);
    }
}

/**
 * @brief 设置引脚为低电平输入模式（下拉输入）
 */
void gpio_set_pin_input_low(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint8_t pin_num = GET_GPIO_PIN(pin);

    DEBUG_PRINT("gpio_set_pin_input_low(pin: 0x%02X, port: %d, num: %d)", pin, port, pin_num);

    if (port == PORTA) {
        gpio_porta_dir &= ~(1 << pin_num);
        gpio_porta_state &= ~(1 << pin_num);  /* 下拉 */
    } else if (port == PORTB) {
        gpio_portb_dir &= ~(1 << pin_num);
        gpio_portb_state &= ~(1 << pin_num);
    }
}

/**
 * @brief 设置引脚为推挽输出模式
 */
void gpio_set_pin_output_push_pull(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint8_t pin_num = GET_GPIO_PIN(pin);

    DEBUG_PRINT("gpio_set_pin_output_push_pull(pin: 0x%02X, port: %d, num: %d)", pin, port, pin_num);

    if (port == PORTA) {
        gpio_porta_dir |= (1 << pin_num);
    } else if (port == PORTB) {
        gpio_portb_dir |= (1 << pin_num);
    }
}

/**
 * @brief 设置引脚为开漏输出模式
 */
void gpio_set_pin_output_open_drain(pin_t pin) {
    gpio_set_pin_output_push_pull(pin);  /* 测试平台简化处理 */
}

/**
 * @brief 设置引脚为输出模式并输出低电平
 */
void gpio_set_pin_output_low(pin_t pin) {
    gpio_set_pin_output_push_pull(pin);
    gpio_write_pin_low(pin);
}

/**
 * @brief 设置引脚为输出模式并输出高电平
 */
void gpio_set_pin_output_high(pin_t pin) {
    gpio_set_pin_output_push_pull(pin);
    gpio_write_pin_high(pin);
}

/**
 * @brief 读取引脚电平
 */
uint8_t gpio_read_pin(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint8_t pin_num = GET_GPIO_PIN(pin);

    if (port == PORTA) {
        return (gpio_porta_state >> pin_num) & 1;
    } else if (port == PORTB) {
        return (gpio_portb_state >> pin_num) & 1;
    }
    return 0;
}

/**
 * @brief 写入引脚高电平
 */
void gpio_write_pin_high(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint8_t pin_num = GET_GPIO_PIN(pin);

    DEBUG_PRINT("gpio_write_pin_high(pin: 0x%02X, port: %d, num: %d)", pin, port, pin_num);

    if (port == PORTA) {
        gpio_porta_state |= (1 << pin_num);
    } else if (port == PORTB) {
        gpio_portb_state |= (1 << pin_num);
    }
}

/**
 * @brief 写入引脚低电平
 */
void gpio_write_pin_low(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint8_t pin_num = GET_GPIO_PIN(pin);

    if (port == PORTA) {
        gpio_porta_state &= ~(1 << pin_num);
    } else if (port == PORTB) {
        gpio_portb_state &= ~(1 << pin_num);
    }
}

/**
 * @brief 翻转引脚电平
 */
void gpio_toggle_pin(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint8_t pin_num = GET_GPIO_PIN(pin);

    DEBUG_PRINT("gpio_toggle_pin(pin: 0x%02X, port: %d, num: %d)", pin, port, pin_num);

    if (port == PORTA) {
        gpio_porta_state ^= (1 << pin_num);
    } else if (port == PORTB) {
        gpio_portb_state ^= (1 << pin_num);
    }
}

/**
 * @brief 批量设置引脚为输入模式
 */
void gpio_set_pin_batch_input(pin_t *pins, uint8_t count) {
    for (uint8_t i = 0; i < count; i++) {
        if (pins[i] != NO_PIN) {
            gpio_set_pin_input(pins[i]);
        }
    }
}

/**
 * @brief 批量设置引脚为输入上拉模式
 */
void gpio_set_pin_batch_input_high(pin_t *pins, uint8_t count) {
    for (uint8_t i = 0; i < count; i++) {
        if (pins[i] != NO_PIN) {
            gpio_set_pin_input_high(pins[i]);
        }
    }
}

/**
 * @brief 批量设置引脚为输出模式
 */
void gpio_set_pin_batch_output(pin_t *pins, uint8_t count) {
    for (uint8_t i = 0; i < count; i++) {
        if (pins[i] != NO_PIN) {
            gpio_set_pin_output_push_pull(pins[i]);
        }
    }
}

/**
 * @brief 批量设置引脚输出低电平
 */
void gpio_set_pin_batch_output_low(pin_t *pins, uint8_t count) {
    for (uint8_t i = 0; i < count; i++) {
        if (pins[i] != NO_PIN) {
            gpio_set_pin_output_low(pins[i]);
        }
    }
}

/**
 * @brief 批量使能引脚中断（测试平台空实现）
 */
bool gpio_enable_interrupt_batch(const pin_t *pins, uint8_t count,
                                 gpio_int_trigger_t trigger,
                                 gpio_int_callback_t callback) {
    (void)pins;
    (void)count;
    (void)trigger;
    (void)callback;
    DEBUG_PRINT("gpio_enable_interrupt_batch: called (stub)");
    return true;
}

/**
 * @brief 批量禁用引脚中断（测试平台空实现）
 */
void gpio_disable_interrupt_batch(const pin_t *pins, uint8_t count) {
    (void)pins;
    (void)count;
    DEBUG_PRINT("gpio_disable_interrupt_batch: called (stub)");
}
