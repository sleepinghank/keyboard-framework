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

#include "../../pin_mapper.h"
#include "_pin_defs.h"

/* 映射表结构体 */
typedef struct {
    pin_t pin;           /* GPIO引脚号 */
    uint8_t channel;     /* 对应的协议信道号 */
} pin_channel_map_t;

/* I2C映射表 - 假设PA0/PA1为I2C0，PB0/PB1为I2C1 */
static const pin_channel_map_t i2c_map[] = {
    { A0,  0 },   /* PA0 -> I2C_CHANNEL_0 (SCL) */
    { A1,  0 },   /* PA1 -> I2C_CHANNEL_0 (SDA) */
    { B0,  1 },   /* PB0 -> I2C_CHANNEL_1 (SCL) */
    { B1,  1 },   /* PB1 -> I2C_CHANNEL_1 (SDA) */
    { A2,  2 },   /* PA2 -> I2C_CHANNEL_2 (SCL) */
    { A3,  2 },   /* PA3 -> I2C_CHANNEL_2 (SDA) */
    { 0xFF, 0xFF }  /* 结束标记 */
};

/* UART RX映射表 - 假设PA4为UART0_RX，PA5为UART1_RX */
static const pin_channel_map_t uart_rx_map[] = {
    { A4,  0 },   /* PA4 -> UART_CHANNEL_0 (RX) */
    { A5,  1 },   /* PA5 -> UART_CHANNEL_1 (RX) */
    { B2,  2 },   /* PB2 -> UART_CHANNEL_2 (RX) */
    { B3,  3 },   /* PB3 -> UART_CHANNEL_3 (RX) */
    { 0xFF, 0xFF }  /* 结束标记 */
};

/* UART TX映射表 - 假设PA6为UART0_TX，PA7为UART1_TX */
static const pin_channel_map_t uart_tx_map[] = {
    { A6,  0 },   /* PA6 -> UART_CHANNEL_0 (TX) */
    { A7,  1 },   /* PA7 -> UART_CHANNEL_1 (TX) */
    { B4,  2 },   /* PB4 -> UART_CHANNEL_2 (TX) */
    { B5,  3 },   /* PB5 -> UART_CHANNEL_3 (TX) */
    { 0xFF, 0xFF }  /* 结束标记 */
};

/* SPI映射表 - 假设PA8/PA9/PA10为SPI0，PB6/PB7/PB8为SPI1 */
static const pin_channel_map_t spi_map[] = {
    { A8,  0 },   /* PA8 -> SPI_CHANNEL_0 (SCK) */
    { A9,  0 },   /* PA9 -> SPI_CHANNEL_0 (MISO) */
    { A10, 0 },   /* PA10 -> SPI_CHANNEL_0 (MOSI) */
    { B6,  1 },   /* PB6 -> SPI_CHANNEL_1 (SCK) */
    { B7,  1 },   /* PB7 -> SPI_CHANNEL_1 (MISO) */
    { B8,  1 },   /* PB8 -> SPI_CHANNEL_1 (MOSI) */
    { 0xFF, 0xFF }  /* 结束标记 */
};

/* ADC映射表 - 假设PA11-PA15为ADC0-ADC4，PB9-PB13为ADC5-ADC9 */
static const pin_channel_map_t adc_map[] = {
    { A11, 0 },   /* PA11 -> ADC_CHANNEL_0 */
    { A12, 1 },   /* PA12 -> ADC_CHANNEL_1 */
    { A13, 2 },   /* PA13 -> ADC_CHANNEL_2 */
    { A14, 3 },   /* PA14 -> ADC_CHANNEL_3 */
    { A15, 4 },   /* PA15 -> ADC_CHANNEL_4 */
    { B9,  5 },   /* PB9 -> ADC_CHANNEL_5 */
    { B10, 6 },   /* PB10 -> ADC_CHANNEL_6 */
    { B11, 7 },   /* PB11 -> ADC_CHANNEL_7 */
    { B12, 8 },   /* PB12 -> ADC_CHANNEL_8 */
    { B13, 9 },   /* PB13 -> ADC_CHANNEL_9 */
    { 0xFF, 0xFF }  /* 结束标记 */
};

/* PWM映射表 - 假设PA16-PA19为PWM0-PWM3，PB14-PB17为PWM4-PWM7 */
static const pin_channel_map_t pwm_map[] = {
    { A16, 0 },   /* PA16 -> PWM_CHANNEL_0 */
    { A17, 1 },   /* PA17 -> PWM_CHANNEL_1 */
    { A18, 2 },   /* PA18 -> PWM_CHANNEL_2 */
    { A19, 3 },   /* PA19 -> PWM_CHANNEL_3 */
    { B14, 4 },   /* PB14 -> PWM_CHANNEL_4 */
    { B15, 5 },   /* PB15 -> PWM_CHANNEL_5 */
    { B16, 6 },   /* PB16 -> PWM_CHANNEL_6 */
    { B17, 7 },   /* PB17 -> PWM_CHANNEL_7 */
    { 0xFF, 0xFF }  /* 结束标记 */
};

/* 查找映射表的辅助函数 */
static uint8_t find_channel(const pin_channel_map_t *map, pin_t pin) {
    uint8_t i;
    for (i = 0; map[i].pin != 0xFF; i++) {
        if (map[i].pin == pin) {
            return map[i].channel;
        }
    }
    return PIN_MAP_INVALID_CHANNEL;
}

/*==========================================
 * 实现接口函数
 *=========================================*/

uint8_t pin_to_i2c_channel(pin_t pin) {
    return find_channel(i2c_map, pin);
}

uint8_t pin_to_uart_rx_channel(pin_t pin) {
    return find_channel(uart_rx_map, pin);
}

uint8_t pin_to_uart_tx_channel(pin_t pin) {
    return find_channel(uart_tx_map, pin);
}

uint8_t pin_to_spi_channel(pin_t pin) {
    return find_channel(spi_map, pin);
}

uint8_t pin_to_adc_channel(pin_t pin) {
    return find_channel(adc_map, pin);
}

uint8_t pin_to_pwm_channel(pin_t pin) {
    return find_channel(pwm_map, pin);
}

uint32_t pin_get_supported_protocols(pin_t pin) {
    uint32_t protocols = PIN_PROTOCOL_NONE;

    if (pin_supports_i2c(pin)) {
        protocols |= PIN_PROTOCOL_I2C;
    }
    if (pin_supports_uart(pin)) {
        protocols |= PIN_PROTOCOL_UART;
    }
    if (pin_supports_spi(pin)) {
        protocols |= PIN_PROTOCOL_SPI;
    }
    if (pin_supports_adc(pin)) {
        protocols |= PIN_PROTOCOL_ADC;
    }
    if (pin_supports_pwm(pin)) {
        protocols |= PIN_PROTOCOL_PWM;
    }

    return protocols;
}
