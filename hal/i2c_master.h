/*  Copyright (C) 2019 Elia Ritterbusch
 +
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
/* Library made by: g4lvanix
 * GitHub repository: https://github.com/g4lvanix/I2C-master-lib
 */

#pragma once

#include <stdint.h>
#include "pin_mapper.h"
#include "gpio.h"

#define I2C_READ 0x01
#define I2C_WRITE 0x00

typedef int16_t i2c_status_t;

#define I2C_STATUS_SUCCESS (0)
#define I2C_STATUS_ERROR (-1)
#define I2C_STATUS_TIMEOUT (-2)

#define I2C_TIMEOUT_IMMEDIATE (0)
#define I2C_TIMEOUT_INFINITE (0xFFFF)

/* I2C通道枚举 */
typedef enum {
    I2C_CHANNEL_0 = 0,
    I2C_CHANNEL_1 = 1,
    I2C_CHANNEL_2 = 2,
    I2C_CHANNEL_3 = 3,
    I2C_CHANNEL_MAX = 4
} i2c_channel_t;

/*==========================================
 * 基于通道号的I2C函数
 *=========================================*/

void         i2c_init_channel(i2c_channel_t channel);
i2c_status_t i2c_start_channel(i2c_channel_t channel, uint8_t address, uint16_t timeout);
i2c_status_t i2c_write_channel(i2c_channel_t channel, uint8_t data, uint16_t timeout);
int16_t      i2c_read_ack_channel(i2c_channel_t channel, uint16_t timeout);
int16_t      i2c_read_nack_channel(i2c_channel_t channel, uint16_t timeout);
i2c_status_t i2c_transmit_channel(i2c_channel_t channel, uint8_t address, const uint8_t* data, uint16_t length, uint16_t timeout);
i2c_status_t i2c_receive_channel(i2c_channel_t channel, uint8_t address, uint8_t* data, uint16_t length, uint16_t timeout);
i2c_status_t i2c_writeReg_channel(i2c_channel_t channel, uint8_t devaddr, uint8_t regaddr, const uint8_t* data, uint16_t length, uint16_t timeout);
i2c_status_t i2c_writeReg16_channel(i2c_channel_t channel, uint8_t devaddr, uint16_t regaddr, const uint8_t* data, uint16_t length, uint16_t timeout);
i2c_status_t i2c_readReg_channel(i2c_channel_t channel, uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length, uint16_t timeout);
i2c_status_t i2c_readReg16_channel(i2c_channel_t channel, uint8_t devaddr, uint16_t regaddr, uint8_t* data, uint16_t length, uint16_t timeout);
void         i2c_stop_channel(i2c_channel_t channel);

/*==========================================
 * GPIO引脚绑定到I2C信道
 *=========================================*/

/**
 * @brief   绑定GPIO引脚到I2C信道
 * @param   sda_pin SDA引脚号
 * @param   scl_pin SCL引脚号
 * @param   channel I2C通道号
 * @return  i2c_status_t 状态码
 * @note    此函数将GPIO引脚绑定到指定I2C信道，后续操作直接使用信道号
 */
i2c_status_t i2c_bind_pins(pin_t sda_pin, pin_t scl_pin, i2c_channel_t channel);

/**
 * @brief   获取I2C信道绑定的SDA引脚
 * @param   channel I2C通道号
 * @return  SDA引脚号
 */
pin_t i2c_get_sda_pin(i2c_channel_t channel);

/**
 * @brief   获取I2C信道绑定的SCL引脚
 * @param   channel I2C通道号
 * @return  SCL引脚号
 */
pin_t i2c_get_scl_pin(i2c_channel_t channel);

/**
 * @brief   检查I2C信道是否已绑定GPIO引脚
 * @param   channel I2C通道号
 * @return  true表示已绑定，false表示未绑定
 */
bool i2c_is_bound(i2c_channel_t channel);

/* 便捷宏 - 默认使用通道0 */

#define i2c_init()                  i2c_init_channel(I2C_CHANNEL_0)
#define i2c_start(address, timeout) i2c_start_channel(I2C_CHANNEL_0, (address), (timeout))
#define i2c_write(data, timeout)    i2c_write_channel(I2C_CHANNEL_0, (data), (timeout))
#define i2c_read_ack(timeout)       i2c_read_ack_channel(I2C_CHANNEL_0, (timeout))
#define i2c_read_nack(timeout)      i2c_read_nack_channel(I2C_CHANNEL_0, (timeout))
#define i2c_transmit(address, data, length, timeout) \
    i2c_transmit_channel(I2C_CHANNEL_0, (address), (data), (length), (timeout))
#define i2c_receive(address, data, length, timeout) \
    i2c_receive_channel(I2C_CHANNEL_0, (address), (data), (length), (timeout))
#define i2c_writeReg(devaddr, regaddr, data, length, timeout) \
    i2c_writeReg_channel(I2C_CHANNEL_0, (devaddr), (regaddr), (data), (length), (timeout))
#define i2c_writeReg16(devaddr, regaddr, data, length, timeout) \
    i2c_writeReg16_channel(I2C_CHANNEL_0, (devaddr), (regaddr), (data), (length), (timeout))
#define i2c_readReg(devaddr, regaddr, data, length, timeout) \
    i2c_readReg_channel(I2C_CHANNEL_0, (devaddr), (regaddr), (data), (length), (timeout))
#define i2c_readReg16(devaddr, regaddr, data, length, timeout) \
    i2c_readReg16_channel(I2C_CHANNEL_0, (devaddr), (regaddr), (data), (length), (timeout))
#define i2c_stop()                  i2c_stop_channel(I2C_CHANNEL_0)
