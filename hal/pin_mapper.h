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

#include <stdint.h>
#include <stdbool.h>
#include "gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 无效通道标记 */
#define PIN_MAP_INVALID_CHANNEL 0xFF

/* 协议类型定义 */
typedef enum {
    PIN_PROTOCOL_NONE = 0,
    PIN_PROTOCOL_I2C  = (1 << 0),
    PIN_PROTOCOL_UART = (1 << 1),
    PIN_PROTOCOL_SPI  = (1 << 2),
    PIN_PROTOCOL_ADC  = (1 << 3),
    PIN_PROTOCOL_PWM  = (1 << 4)
} pin_protocol_t;

/*==========================================
 * GPIO引脚到协议信道号映射查询
 *=========================================*/

/**
 * @brief   根据GPIO引脚查询I2C信道号
 * @param   pin GPIO引脚号
 * @return  I2C信道号，0xFF表示该引脚不支持I2C
 */
uint8_t pin_to_i2c_channel(pin_t pin);

/**
 * @brief   根据GPIO引脚查询UART RX信道号
 * @param   pin GPIO引脚号
 * @return  UART RX信道号，0xFF表示该引脚不支持UART RX
 */
uint8_t pin_to_uart_rx_channel(pin_t pin);

/**
 * @brief   根据GPIO引脚查询UART TX信道号
 * @param   pin GPIO引脚号
 * @return  UART TX信道号，0xFF表示该引脚不支持UART TX
 */
uint8_t pin_to_uart_tx_channel(pin_t pin);

/**
 * @brief   根据GPIO引脚查询SPI信道号
 * @param   pin GPIO引脚号
 * @return  SPI信道号，0xFF表示该引脚不支持SPI
 */
uint8_t pin_to_spi_channel(pin_t pin);

/**
 * @brief   根据GPIO引脚查询ADC信道号
 * @param   pin GPIO引脚号
 * @return  ADC信道号，0xFF表示该引脚不支持ADC
 */
uint8_t pin_to_adc_channel(pin_t pin);

/**
 * @brief   根据GPIO引脚查询PWM信道号
 * @param   pin GPIO引脚号
 * @return  PWM信道号，0xFF表示该引脚不支持PWM
 */
uint8_t pin_to_pwm_channel(pin_t pin);

/*==========================================
 * GPIO引脚协议支持检查
 *=========================================*/

/**
 * @brief   检查GPIO引脚是否支持I2C
 * @param   pin GPIO引脚号
 * @return  true表示支持，false表示不支持
 */
static inline bool pin_supports_i2c(pin_t pin) {
    return (pin_to_i2c_channel(pin) != PIN_MAP_INVALID_CHANNEL);
}

/**
 * @brief   检查GPIO引脚是否支持UART
 * @param   pin GPIO引脚号
 * @return  true表示支持，false表示不支持
 */
static inline bool pin_supports_uart(pin_t pin) {
    return (pin_to_uart_rx_channel(pin) != PIN_MAP_INVALID_CHANNEL) ||
           (pin_to_uart_tx_channel(pin) != PIN_MAP_INVALID_CHANNEL);
}

/**
 * @brief   检查GPIO引脚是否支持UART RX
 * @param   pin GPIO引脚号
 * @return  true表示支持，false表示不支持
 */
static inline bool pin_supports_uart_rx(pin_t pin) {
    return (pin_to_uart_rx_channel(pin) != PIN_MAP_INVALID_CHANNEL);
}

/**
 * @brief   检查GPIO引脚是否支持UART TX
 * @param   pin GPIO引脚号
 * @return  true表示支持，false表示不支持
 */
static inline bool pin_supports_uart_tx(pin_t pin) {
    return (pin_to_uart_tx_channel(pin) != PIN_MAP_INVALID_CHANNEL);
}

/**
 * @brief   检查GPIO引脚是否支持SPI
 * @param   pin GPIO引脚号
 * @return  true表示支持，false表示不支持
 */
static inline bool pin_supports_spi(pin_t pin) {
    return (pin_to_spi_channel(pin) != PIN_MAP_INVALID_CHANNEL);
}

/**
 * @brief   检查GPIO引脚是否支持ADC
 * @param   pin GPIO引脚号
 * @return  true表示支持，false表示不支持
 */
static inline bool pin_supports_adc(pin_t pin) {
    return (pin_to_adc_channel(pin) != PIN_MAP_INVALID_CHANNEL);
}

/**
 * @brief   检查GPIO引脚是否支持PWM
 * @param   pin GPIO引脚号
 * @return  true表示支持，false表示不支持
 */
static inline bool pin_supports_pwm(pin_t pin) {
    return (pin_to_pwm_channel(pin) != PIN_MAP_INVALID_CHANNEL);
}

/**
 * @brief   获取GPIO引脚支持的协议类型
 * @param   pin GPIO引脚号
 * @return  协议类型掩码（可组合使用）
 */
uint32_t pin_get_supported_protocols(pin_t pin);

/*==========================================
 * 便捷宏定义
 *=========================================*/

/* I2C便捷宏 */
#define PIN_HAS_I2C(pin)              pin_supports_i2c(pin)
#define PIN_GET_I2C_CHANNEL(pin)      pin_to_i2c_channel(pin)

/* UART便捷宏 */
#define PIN_HAS_UART(pin)             pin_supports_uart(pin)
#define PIN_HAS_UART_RX(pin)          pin_supports_uart_rx(pin)
#define PIN_HAS_UART_TX(pin)          pin_supports_uart_tx(pin)
#define PIN_GET_UART_RX_CHANNEL(pin)  pin_to_uart_rx_channel(pin)
#define PIN_GET_UART_TX_CHANNEL(pin)  pin_to_uart_tx_channel(pin)

/* SPI便捷宏 */
#define PIN_HAS_SPI(pin)              pin_supports_spi(pin)
#define PIN_GET_SPI_CHANNEL(pin)      pin_to_spi_channel(pin)

/* ADC便捷宏 */
#define PIN_HAS_ADC(pin)              pin_supports_adc(pin)
#define PIN_GET_ADC_CHANNEL(pin)      pin_to_adc_channel(pin)

/* PWM便捷宏 */
#define PIN_HAS_PWM(pin)              pin_supports_pwm(pin)
#define PIN_GET_PWM_CHANNEL(pin)      pin_to_pwm_channel(pin)

#ifdef __cplusplus
}
#endif
