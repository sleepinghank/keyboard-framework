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
#pragma once


#define PAL_LINE(port, pad) ((port) << 6) | (pad)

#define PORTA 0
#define PORTB 1

/* GPIO端口类型判断宏 */
#define GPIO_PORT_MASK 0xFC0  /* 端口号掩码: 高6位 (0xFC0 = 111111000000b) */
#define GPIO_PIN_MASK  0x3F   /* 引脚号掩码: 低6位 (0x3F = 000000111111b) */

/* 判断GPIO引脚属于哪个端口 */
#define IS_PORTA(gpio) (((gpio) & GPIO_PORT_MASK) == (PORTA << 6))
#define IS_PORTB(gpio) (((gpio) & GPIO_PORT_MASK) == (PORTB << 6))

/* 获取GPIO的端口号 */
#define GET_GPIO_PORT(gpio) (((gpio) & GPIO_PORT_MASK) >> 6)

/* 获取GPIO的引脚号 */
#define GET_GPIO_PIN(gpio) ((gpio) & GPIO_PIN_MASK)

/* 测试宏示例 */
/*
 * 示例用法:
 * if (IS_PORTA(A5)) { // 判断A5是否属于PORTA
 *     // PORTA端口的处理逻辑
 * }
 * if (IS_PORTB(B3)) { // 判断B3是否属于PORTB  
 *     // PORTB端口的处理逻辑
 * }
 * 
 * uint8_t port = GET_GPIO_PORT(gpio_pin); // 获取端口号
 * uint8_t pin = GET_GPIO_PIN(gpio_pin);   // 获取引脚号
 */

#ifdef PORTA
#    define A0 PAL_LINE(PORTA, 0)
#    define A1 PAL_LINE(PORTA, 1)
#    define A2 PAL_LINE(PORTA, 2)
#    define A3 PAL_LINE(PORTA, 3)
#    define A4 PAL_LINE(PORTA, 4)
#    define A5 PAL_LINE(PORTA, 5)
#    define A6 PAL_LINE(PORTA, 6)
#    define A7 PAL_LINE(PORTA, 7)
#    define A8 PAL_LINE(PORTA, 8)
#    define A9 PAL_LINE(PORTA, 9)
#    define A10 PAL_LINE(PORTA, 10)
#    define A11 PAL_LINE(PORTA, 11)
#    define A12 PAL_LINE(PORTA, 12)
#    define A13 PAL_LINE(PORTA, 13)
#    define A14 PAL_LINE(PORTA, 14)
#    define A15 PAL_LINE(PORTA, 15)
#    define A16 PAL_LINE(PORTA, 16)
#    define A17 PAL_LINE(PORTA, 17)
#    define A18 PAL_LINE(PORTA, 18)
#    define A19 PAL_LINE(PORTA, 19)
#    define A20 PAL_LINE(PORTA, 20)
#    define A21 PAL_LINE(PORTA, 21)
#    define A22 PAL_LINE(PORTA, 22)
#    define A23 PAL_LINE(PORTA, 23)
#    define A24 PAL_LINE(PORTA, 24)
#    define A25 PAL_LINE(PORTA, 25)
#    define A26 PAL_LINE(PORTA, 26)
#    define A27 PAL_LINE(PORTA, 27)
#    define A28 PAL_LINE(PORTA, 28)
#    define A29 PAL_LINE(PORTA, 29)
#    define A30 PAL_LINE(PORTA, 30)
#    define A31 PAL_LINE(PORTA, 31)
#endif
#ifdef PORTB
#    define B0 PAL_LINE(PORTB, 0)
#    define B1 PAL_LINE(PORTB, 1)
#    define B2 PAL_LINE(PORTB, 2)
#    define B3 PAL_LINE(PORTB, 3)
#    define B4 PAL_LINE(PORTB, 4)
#    define B5 PAL_LINE(PORTB, 5)
#    define B6 PAL_LINE(PORTB, 6)
#    define B7 PAL_LINE(PORTB, 7)
#    define B8 PAL_LINE(PORTB, 8)
#    define B9 PAL_LINE(PORTB, 9)
#    define B10 PAL_LINE(PORTB, 10)
#    define B11 PAL_LINE(PORTB, 11)
#    define B12 PAL_LINE(PORTB, 12)
#    define B13 PAL_LINE(PORTB, 13)
#    define B14 PAL_LINE(PORTB, 14)
#    define B15 PAL_LINE(PORTB, 15)
#    define B16 PAL_LINE(PORTB, 16)
#    define B17 PAL_LINE(PORTB, 17)
#    define B18 PAL_LINE(PORTB, 18)
#    define B19 PAL_LINE(PORTB, 19)
#    define B20 PAL_LINE(PORTB, 20)
#    define B21 PAL_LINE(PORTB, 21)
#    define B22 PAL_LINE(PORTB, 22)
#    define B23 PAL_LINE(PORTB, 23)
#    define B24 PAL_LINE(PORTB, 24)
#    define B25 PAL_LINE(PORTB, 25)
#    define B26 PAL_LINE(PORTB, 26)
#    define B27 PAL_LINE(PORTB, 27)
#    define B28 PAL_LINE(PORTB, 28)
#    define B29 PAL_LINE(PORTB, 29)
#    define B30 PAL_LINE(PORTB, 30)
#    define B31 PAL_LINE(PORTB, 31)
#endif



