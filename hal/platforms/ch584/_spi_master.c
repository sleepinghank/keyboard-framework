/*  Copyright 2020
 *
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

#include "spi_master.h"
#include "kb904/config_product.h"
#include "CH58x_common.h"
#include "gpio.h"

/* SPI配置状态 */
static bool g_spi_initialized = false;
static pin_t g_current_slave_pin = NO_PIN;

/**
 * @brief 初始化SPI主机模式
 * @note CH584 SPI0默认引脚: SCK=PA12, MOSI=PA13, MISO=PA14
 */
void spi_init(void)
{
    if (g_spi_initialized) {
        return;
    }

    /* 使用SPI0默认初始化：模式0 + 3线全双工 + 8MHz */
    SPI0_MasterDefInit();

    /* 可选：配置时钟分频 (默认8MHz = 64MHz/8) */
    /* SPI0_CLKCfg(8); */

    /* 配置数据模式：模式0，高位在前（标准SPI） */
    SPI0_DataMode(Mode0_HighBitINFront);

    g_spi_initialized = true;
    g_current_slave_pin = NO_PIN;
}

/**
 * @brief 开始SPI传输，选择从设备
 * @param slavePin 从设备片选引脚
 * @param lsbFirst 是否低位在前
 * @param mode SPI模式（0-3）
 * @param divisor 时钟分频系数
 * @return true 成功，false 失败
 */
bool spi_start(pin_t slavePin, bool lsbFirst, uint8_t mode, uint16_t divisor)
{
    if (!g_spi_initialized) {
        spi_init();
    }

    if (g_current_slave_pin != NO_PIN) {
        /* 上一次传输未正确结束 */
        return false;
    }

    /* 配置时钟分频 */
    if (divisor > 0 && divisor <= 255) {
        SPI0_CLKCfg((uint8_t)divisor);
    }

    /* 配置数据模式 */
    ModeBitOrderTypeDef data_mode;
    switch (mode) {
        case 0:
            data_mode = lsbFirst ? Mode0_LowBitINFront : Mode0_HighBitINFront;
            break;
        case 3:
            data_mode = lsbFirst ? Mode3_LowBitINFront : Mode3_HighBitINFront;
            break;
        default:
            /* CH584仅支持模式0和3，其他模式默认使用模式0 */
            data_mode = lsbFirst ? Mode0_LowBitINFront : Mode0_HighBitINFront;
            break;
    }
    SPI0_DataMode(data_mode);

    /* 配置片选引脚为输出并拉低 */
    if (slavePin != NO_PIN) {
        gpio_set_pin_output_push_pull(slavePin);
        gpio_write_pin_low(slavePin);
    }

    g_current_slave_pin = slavePin;
    return true;
}

/**
 * @brief 写入单字节数据
 * @param data 要写入的数据
 * @return SPI_STATUS_SUCCESS 成功
 */
spi_status_t spi_write(uint8_t data)
{
    if (!g_spi_initialized) {
        return SPI_STATUS_ERROR;
    }

    SPI0_MasterSendByte(data);
    return SPI_STATUS_SUCCESS;
}

/**
 * @brief 读取单字节数据
 * @return 读取到的数据，失败返回 SPI_STATUS_ERROR
 */
spi_status_t spi_read(void)
{
    if (!g_spi_initialized) {
        return SPI_STATUS_ERROR;
    }

    return (spi_status_t)SPI0_MasterRecvByte();
}

/**
 * @brief 发送多字节数据
 * @param data 数据缓冲区
 * @param length 数据长度
 * @return SPI_STATUS_SUCCESS 成功
 */
spi_status_t spi_transmit(const uint8_t *data, uint16_t length)
{
    if (!g_spi_initialized || data == NULL) {
        return SPI_STATUS_ERROR;
    }

    /* 使用FIFO连续发送 */
    SPI0_MasterTrans((uint8_t *)data, length);
    return SPI_STATUS_SUCCESS;
}

/**
 * @brief 接收多字节数据
 * @param data 数据缓冲区
 * @param length 数据长度
 * @return SPI_STATUS_SUCCESS 成功
 */
spi_status_t spi_receive(uint8_t *data, uint16_t length)
{
    if (!g_spi_initialized || data == NULL) {
        return SPI_STATUS_ERROR;
    }

    /* 使用FIFO连续接收 */
    SPI0_MasterRecv(data, length);
    return SPI_STATUS_SUCCESS;
}

/**
 * @brief 结束SPI传输，释放从设备
 */
void spi_stop(void)
{
    /* 释放片选引脚（拉高） */
    if (g_current_slave_pin != NO_PIN) {
        gpio_write_pin_high(g_current_slave_pin);
    }

    g_current_slave_pin = NO_PIN;
}
