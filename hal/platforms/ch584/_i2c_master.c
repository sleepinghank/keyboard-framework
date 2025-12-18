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

#include "i2c_master.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

// 调试输出宏
#define I2C_DEBUG_PRINT(fmt, ...) printf("[I2C] " fmt "\n", ##__VA_ARGS__)

/* I2C channel state structure */
typedef struct {
    pin_t sda_pin;        // SDA pin bound to this channel
    pin_t scl_pin;        // SCL pin bound to this channel
    bool initialized;     // Channel initialization flag
    bool bound;           // Pin binding flag
} i2c_channel_state_t;

/* I2C channel state array */
static i2c_channel_state_t i2c_channels[I2C_CHANNEL_MAX] = {0};

/*==========================================
 * 基于通道号的I2C函数
 *=========================================*/

/**
 * @brief   初始化I2C通道
 * @param   channel I2C通道号
 * @return  none
 */
void i2c_init_channel(i2c_channel_t channel) {
    if (channel >= I2C_CHANNEL_MAX) {
        I2C_DEBUG_PRINT("i2c_init_channel: Invalid channel %d", channel);
        return;
    }

    i2c_channels[channel].initialized = true;
    I2C_DEBUG_PRINT("i2c_init_channel: Initialized channel %d", channel);

    // TODO: In a real implementation, you would:
    // 1. Configure the GPIO pins as open-drain outputs
    // 2. Initialize the I2C peripheral
    // 3. Set appropriate pull-up resistors
}

/**
 * @brief   启动I2C传输
 * @param   channel I2C通道号
 * @param   address 从设备地址
 * @param   timeout 超时时间（毫秒）
 * @return  I2C_STATUS_SUCCESS 成功, I2C_STATUS_ERROR 失败, I2C_STATUS_TIMEOUT 超时
 */
i2c_status_t i2c_start_channel(i2c_channel_t channel, uint8_t address, uint16_t timeout) {
    if (channel >= I2C_CHANNEL_MAX) {
        I2C_DEBUG_PRINT("i2c_start_channel: Invalid channel %d", channel);
        return I2C_STATUS_ERROR;
    }

    I2C_DEBUG_PRINT("i2c_start_channel: Channel %d, address 0x%02X, timeout %d",
                    channel, address, timeout);

    // TODO: In a real implementation, you would:
    // 1. Generate START condition
    // 2. Send device address with read/write bit
    // 3. Wait for ACK
    // 4. Return appropriate status

    return I2C_STATUS_SUCCESS;
}

/**
 * @brief   向I2C总线写入一个字节
 * @param   channel I2C通道号
 * @param   data 要写入的数据
 * @param   timeout 超时时间（毫秒）
 * @return  I2C_STATUS_SUCCESS 成功, I2C_STATUS_ERROR 失败, I2C_STATUS_TIMEOUT 超时
 */
i2c_status_t i2c_write_channel(i2c_channel_t channel, uint8_t data, uint16_t timeout) {
    if (channel >= I2C_CHANNEL_MAX) {
        I2C_DEBUG_PRINT("i2c_write_channel: Invalid channel %d", channel);
        return I2C_STATUS_ERROR;
    }

    I2C_DEBUG_PRINT("i2c_write_channel: Channel %d, data 0x%02X, timeout %d",
                    channel, data, timeout);

    // TODO: In a real implementation, you would:
    // 1. Put data on I2C data line
    // 2. Generate clock pulse
    // 3. Wait for ACK
    // 4. Return appropriate status

    return I2C_STATUS_SUCCESS;
}

/**
 * @brief   从I2C总线读取一个字节并发送ACK
 * @param   channel I2C通道号
 * @param   timeout 超时时间（毫秒）
 * @return  读取的数据（0-255），负值表示错误
 */
int16_t i2c_read_ack_channel(i2c_channel_t channel, uint16_t timeout) {
    if (channel >= I2C_CHANNEL_MAX) {
        I2C_DEBUG_PRINT("i2c_read_ack_channel: Invalid channel %d", channel);
        return I2C_STATUS_ERROR;
    }

    I2C_DEBUG_PRINT("i2c_read_ack_channel: Channel %d, timeout %d", channel, timeout);

    // TODO: In a real implementation, you would:
    // 1. Generate clock pulse
    // 2. Read data from I2C data line
    // 3. Send ACK to slave
    // 4. Return the read data

    // Test implementation: return 0
    return 0;
}

/**
 * @brief   从I2C总线读取一个字节并发送NACK
 * @param   channel I2C通道号
 * @param   timeout 超时时间（毫秒）
 * @return  读取的数据（0-255），负值表示错误
 */
int16_t i2c_read_nack_channel(i2c_channel_t channel, uint16_t timeout) {
    if (channel >= I2C_CHANNEL_MAX) {
        I2C_DEBUG_PRINT("i2c_read_nack_channel: Invalid channel %d", channel);
        return I2C_STATUS_ERROR;
    }

    I2C_DEBUG_PRINT("i2c_read_nack_channel: Channel %d, timeout %d", channel, timeout);

    // TODO: In a real implementation, you would:
    // 1. Generate clock pulse
    // 2. Read data from I2C data line
    // 3. Send NACK to slave (indicating last byte)
    // 4. Return the read data

    // Test implementation: return 0
    return 0;
}

/**
 * @brief   通过I2C传输多个字节
 * @param   channel I2C通道号
 * @param   address 从设备地址
 * @param   data 要发送的数据缓冲区
 * @param   length 数据长度
 * @param   timeout 超时时间（毫秒）
 * @return  I2C_STATUS_SUCCESS 成功, I2C_STATUS_ERROR 失败, I2C_STATUS_TIMEOUT 超时
 */
i2c_status_t i2c_transmit_channel(i2c_channel_t channel, uint8_t address, const uint8_t* data, uint16_t length, uint16_t timeout) {
    if (channel >= I2C_CHANNEL_MAX) {
        I2C_DEBUG_PRINT("i2c_transmit_channel: Invalid channel %d", channel);
        return I2C_STATUS_ERROR;
    }

    I2C_DEBUG_PRINT("i2c_transmit_channel: Channel %d, address 0x%02X, length %d, timeout %d",
                    channel, address, length, timeout);

    // TODO: In a real implementation, you would:
    // 1. Call i2c_start_channel
    // 2. Send each byte using i2c_write_channel
    // 3. Call i2c_stop_channel
    // 4. Return appropriate status

    return I2C_STATUS_SUCCESS;
}

/**
 * @brief   通过I2C接收多个字节
 * @param   channel I2C通道号
 * @param   address 从设备地址
 * @param   data 接收数据缓冲区
 * @param   length 要接收的数据长度
 * @param   timeout 超时时间（毫秒）
 * @return  I2C_STATUS_SUCCESS 成功, I2C_STATUS_ERROR 失败, I2C_STATUS_TIMEOUT 超时
 */
i2c_status_t i2c_receive_channel(i2c_channel_t channel, uint8_t address, uint8_t* data, uint16_t length, uint16_t timeout) {
    if (channel >= I2C_CHANNEL_MAX) {
        I2C_DEBUG_PRINT("i2c_receive_channel: Invalid channel %d", channel);
        return I2C_STATUS_ERROR;
    }

    I2C_DEBUG_PRINT("i2c_receive_channel: Channel %d, address 0x%02X, length %d, timeout %d",
                    channel, address, length, timeout);

    // TODO: In a real implementation, you would:
    // 1. Call i2c_start_channel with write bit
    // 2. Send register address to read from
    // 3. Call i2c_start_channel with read bit
    // 4. Read bytes using i2c_read_ack_channel and i2c_read_nack_channel
    // 5. Call i2c_stop_channel
    // 6. Return appropriate status

    // Test implementation: fill buffer with zeros
    uint16_t i;
    for (i = 0; i < length; i++) {
        data[i] = 0;
    }

    return I2C_STATUS_SUCCESS;
}

/**
 * @brief   向I2C设备的寄存器写入数据（8位寄存器地址）
 * @param   channel I2C通道号
 * @param   devaddr 从设备地址
 * @param   regaddr 寄存器地址
 * @param   data 要写入的数据
 * @param   length 数据长度
 * @param   timeout 超时时间（毫秒）
 * @return  I2C_STATUS_SUCCESS 成功, I2C_STATUS_ERROR 失败, I2C_STATUS_TIMEOUT 超时
 */
i2c_status_t i2c_writeReg_channel(i2c_channel_t channel, uint8_t devaddr, uint8_t regaddr, const uint8_t* data, uint16_t length, uint16_t timeout) {
    if (channel >= I2C_CHANNEL_MAX) {
        I2C_DEBUG_PRINT("i2c_writeReg_channel: Invalid channel %d", channel);
        return I2C_STATUS_ERROR;
    }

    I2C_DEBUG_PRINT("i2c_writeReg_channel: Channel %d, devaddr 0x%02X, regaddr 0x%02X, length %d, timeout %d",
                    channel, devaddr, regaddr, length, timeout);

    // TODO: In a real implementation, you would:
    // 1. Start I2C transmission
    // 2. Send device address (write)
    // 3. Send register address
    // 4. Send data bytes
    // 5. Stop I2C transmission
    // 6. Return appropriate status

    return I2C_STATUS_SUCCESS;
}

/**
 * @brief   向I2C设备的寄存器写入数据（16位寄存器地址）
 * @param   channel I2C通道号
 * @param   devaddr 从设备地址
 * @param   regaddr 寄存器地址（16位）
 * @param   data 要写入的数据
 * @param   length 数据长度
 * @param   timeout 超时时间（毫秒）
 * @return  I2C_STATUS_SUCCESS 成功, I2C_STATUS_ERROR 失败, I2C_STATUS_TIMEOUT 超时
 */
i2c_status_t i2c_writeReg16_channel(i2c_channel_t channel, uint8_t devaddr, uint16_t regaddr, const uint8_t* data, uint16_t length, uint16_t timeout) {
    if (channel >= I2C_CHANNEL_MAX) {
        I2C_DEBUG_PRINT("i2c_writeReg16_channel: Invalid channel %d", channel);
        return I2C_STATUS_ERROR;
    }

    I2C_DEBUG_PRINT("i2c_writeReg16_channel: Channel %d, devaddr 0x%02X, regaddr 0x%04X, length %d, timeout %d",
                    channel, devaddr, regaddr, length, timeout);

    // TODO: In a real implementation, you would:
    // 1. Start I2C transmission
    // 2. Send device address (write)
    // 3. Send high byte of register address
    // 4. Send low byte of register address
    // 5. Send data bytes
    // 6. Stop I2C transmission
    // 7. Return appropriate status

    return I2C_STATUS_SUCCESS;
}

/**
 * @brief   从I2C设备的寄存器读取数据（8位寄存器地址）
 * @param   channel I2C通道号
 * @param   devaddr 从设备地址
 * @param   regaddr 寄存器地址
 * @param   data 接收数据缓冲区
 * @param   length 要读取的数据长度
 * @param   timeout 超时时间（毫秒）
 * @return  I2C_STATUS_SUCCESS 成功, I2C_STATUS_ERROR 失败, I2C_STATUS_TIMEOUT 超时
 */
i2c_status_t i2c_readReg_channel(i2c_channel_t channel, uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length, uint16_t timeout) {
    if (channel >= I2C_CHANNEL_MAX) {
        I2C_DEBUG_PRINT("i2c_readReg_channel: Invalid channel %d", channel);
        return I2C_STATUS_ERROR;
    }

    I2C_DEBUG_PRINT("i2c_readReg_channel: Channel %d, devaddr 0x%02X, regaddr 0x%02X, length %d, timeout %d",
                    channel, devaddr, regaddr, length, timeout);

    // TODO: In a real implementation, you would:
    // 1. Start I2C transmission
    // 2. Send device address (write)
    // 3. Send register address
    // 4. Start I2C transmission again
    // 5. Send device address (read)
    // 6. Read data bytes
    // 7. Stop I2C transmission
    // 8. Return appropriate status

    // Test implementation: fill buffer with zeros
    uint16_t i;
    for (i = 0; i < length; i++) {
        data[i] = 0;
    }

    return I2C_STATUS_SUCCESS;
}

/**
 * @brief   从I2C设备的寄存器读取数据（16位寄存器地址）
 * @param   channel I2C通道号
 * @param   devaddr 从设备地址
 * @param   regaddr 寄存器地址（16位）
 * @param   data 接收数据缓冲区
 * @param   length 要读取的数据长度
 * @param   timeout 超时时间（毫秒）
 * @return  I2C_STATUS_SUCCESS 成功, I2C_STATUS_ERROR 失败, I2C_STATUS_TIMEOUT 超时
 */
i2c_status_t i2c_readReg16_channel(i2c_channel_t channel, uint8_t devaddr, uint16_t regaddr, uint8_t* data, uint16_t length, uint16_t timeout) {
    if (channel >= I2C_CHANNEL_MAX) {
        I2C_DEBUG_PRINT("i2c_readReg16_channel: Invalid channel %d", channel);
        return I2C_STATUS_ERROR;
    }

    I2C_DEBUG_PRINT("i2c_readReg16_channel: Channel %d, devaddr 0x%02X, regaddr 0x%04X, length %d, timeout %d",
                    channel, devaddr, regaddr, length, timeout);

    // TODO: In a real implementation, you would:
    // 1. Start I2C transmission
    // 2. Send device address (write)
    // 3. Send high byte of register address
    // 4. Send low byte of register address
    // 5. Start I2C transmission again
    // 6. Send device address (read)
    // 7. Read data bytes
    // 8. Stop I2C transmission
    // 9. Return appropriate status

    // Test implementation: fill buffer with zeros
    uint16_t i;
    for (i = 0; i < length; i++) {
        data[i] = 0;
    }

    return I2C_STATUS_SUCCESS;
}

/**
 * @brief   停止I2C传输
 * @param   channel I2C通道号
 * @return  none
 */
void i2c_stop_channel(i2c_channel_t channel) {
    if (channel >= I2C_CHANNEL_MAX) {
        I2C_DEBUG_PRINT("i2c_stop_channel: Invalid channel %d", channel);
        return;
    }

    I2C_DEBUG_PRINT("i2c_stop_channel: Channel %d", channel);

    // TODO: In a real implementation, you would:
    // 1. Generate STOP condition on I2C bus
    // 2. Release the bus
}

/*==========================================
 * GPIO引脚绑定到I2C信道
 *=========================================*/

/**
 * @brief   绑定GPIO引脚到I2C信道
 * @param   sda_pin SDA引脚号
 * @param   scl_pin SCL引脚号
 * @param   channel I2C通道号
 * @return  i2c_status_t 状态码
 */
i2c_status_t i2c_bind_pins(pin_t sda_pin, pin_t scl_pin, i2c_channel_t channel) {
    if (channel >= I2C_CHANNEL_MAX) {
        I2C_DEBUG_PRINT("i2c_bind_pins: Invalid channel %d", channel);
        return I2C_STATUS_ERROR;
    }

    // Check if pins are already bound to another channel
    uint8_t i;
    for (i = 0; i < I2C_CHANNEL_MAX; i++) {
        if (i != channel) {
            if (i2c_channels[i].sda_pin == sda_pin || i2c_channels[i].scl_pin == scl_pin) {
                I2C_DEBUG_PRINT("i2c_bind_pins: Pin already bound to channel %d", i);
                return I2C_STATUS_ERROR;
            }
        }
    }

    // Bind the pins to this channel
    i2c_channels[channel].sda_pin = sda_pin;
    i2c_channels[channel].scl_pin = scl_pin;
    i2c_channels[channel].bound = true;
    i2c_channels[channel].initialized = true;

    I2C_DEBUG_PRINT("i2c_bind_pins: Bound SDA pin 0x%02X, SCL pin 0x%02X to channel %d",
                    sda_pin, scl_pin, channel);

    // TODO: In a real implementation, you would:
    // 1. Configure the GPIO pins as open-drain outputs
    // 2. Enable pull-up resistors
    // 3. Initialize the I2C peripheral

    return I2C_STATUS_SUCCESS;
}

/**
 * @brief   获取I2C信道绑定的SDA引脚
 * @param   channel I2C通道号
 * @return  SDA引脚号
 */
pin_t i2c_get_sda_pin(i2c_channel_t channel) {
    if (channel >= I2C_CHANNEL_MAX) {
        I2C_DEBUG_PRINT("i2c_get_sda_pin: Invalid channel %d", channel);
        return 0xFF;  // Return invalid pin
    }

    I2C_DEBUG_PRINT("i2c_get_sda_pin: Channel %d, pin 0x%02X", channel, i2c_channels[channel].sda_pin);

    return i2c_channels[channel].sda_pin;
}

/**
 * @brief   获取I2C信道绑定的SCL引脚
 * @param   channel I2C通道号
 * @return  SCL引脚号
 */
pin_t i2c_get_scl_pin(i2c_channel_t channel) {
    if (channel >= I2C_CHANNEL_MAX) {
        I2C_DEBUG_PRINT("i2c_get_scl_pin: Invalid channel %d", channel);
        return 0xFF;  // Return invalid pin
    }

    I2C_DEBUG_PRINT("i2c_get_scl_pin: Channel %d, pin 0x%02X", channel, i2c_channels[channel].scl_pin);

    return i2c_channels[channel].scl_pin;
}

/**
 * @brief   检查I2C信道是否已绑定GPIO引脚
 * @param   channel I2C通道号
 * @return  true表示已绑定，false表示未绑定
 */
bool i2c_is_bound(i2c_channel_t channel) {
    if (channel >= I2C_CHANNEL_MAX) {
        I2C_DEBUG_PRINT("i2c_is_bound: Invalid channel %d", channel);
        return false;
    }

    bool bound = i2c_channels[channel].bound;
    I2C_DEBUG_PRINT("i2c_is_bound: Channel %d, bound = %s", channel, bound ? "true" : "false");

    return bound;
}