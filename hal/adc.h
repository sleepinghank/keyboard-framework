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
#include "pin_mapper.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ADC通道枚举 */
typedef enum {
    ADC_CHANNEL_0 = 0,
    ADC_CHANNEL_1 = 1,
    ADC_CHANNEL_2 = 2,
    ADC_CHANNEL_3 = 3,
    ADC_CHANNEL_4 = 4,
    ADC_CHANNEL_5 = 5,
    ADC_CHANNEL_6 = 6,
    ADC_CHANNEL_7 = 7,
    ADC_CHANNEL_8 = 8,
    ADC_CHANNEL_9 = 9,
    ADC_CHANNEL_10 = 10,
    ADC_CHANNEL_11 = 11,
    ADC_CHANNEL_12 = 12,
    ADC_CHANNEL_13 = 13,
    ADC_CHANNEL_14 = 14,
    ADC_CHANNEL_15 = 15,
    ADC_CHANNEL_MAX = 16
} adc_channel_t;

/*==========================================
 * 基于通道号的ADC函数
 *=========================================*/

/*********************************************************************
 * @fn      adc_init
 *
 * @brief   初始化ADC模块
 *
 * @param   channel ADC通道号
 *
 * @return  none
 */
void adc_init(adc_channel_t channel);

/*********************************************************************
 * @fn      adc_init_all
 *
 * @brief   初始化所有ADC通道
 *
 * @param   none
 *
 * @return  none
 */
void adc_init_all(void);

/*********************************************************************
 * @fn      adc_read
 *
 * @brief   读取ADC值
 *
 * @param   channel ADC通道号
 *
 * @return  ADC采样值（通常为12位，范围0-4095）
 */
uint16_t adc_read(adc_channel_t channel);

/*********************************************************************
 * @fn      adc_read_average
 *
 * @brief   读取ADC平均值（多次采样取平均）
 *
 * @param   channel ADC通道号
 * @param   samples 采样次数
 *
 * @return  ADC采样平均值
 */
uint16_t adc_read_average(adc_channel_t channel, uint8_t samples);

/*********************************************************************
 * @fn      adc_read_voltage
 *
 * @brief   读取ADC电压值（毫伏）
 *
 * @param   channel ADC通道号
 *
 * @return  电压值（毫伏）
 */
uint32_t adc_read_voltage(adc_channel_t channel);

/*********************************************************************
 * @fn      adc_read_advanced
 *
 * @brief   读取ADC值（高级模式，支持配置）
 *
 * @param   channel ADC通道号
 * @param   samples 采样次数
 * @param   reference_mv 参考电压（毫伏）
 *
 * @return  ADC采样值
 */
uint16_t adc_read_advanced(adc_channel_t channel, uint8_t samples, uint32_t reference_mv);

/*==========================================
 * GPIO引脚绑定到ADC信道
 *=========================================*/

/*********************************************************************
 * @fn      adc_bind_pin
 *
 * @brief   绑定GPIO引脚到ADC信道
 * @param   pin GPIO引脚号
 * @param   channel ADC通道号
 * @return  bool 绑定是否成功
 * @note    此函数将GPIO引脚绑定到指定ADC信道，后续操作直接使用信道号
 */
bool adc_bind_pin(pin_t pin, adc_channel_t channel);

/*********************************************************************
 * @fn      adc_get_bound_pin
 *
 * @brief   获取ADC信道绑定的GPIO引脚
 * @param   channel ADC通道号
 * @return  GPIO引脚号（如果未绑定则返回NO_PIN）
 */
pin_t adc_get_bound_pin(adc_channel_t channel);

/*********************************************************************
 * @fn      adc_is_bound
 *
 * @brief   检查ADC信道是否已绑定GPIO引脚
 * @param   channel ADC通道号
 * @return  true表示已绑定，false表示未绑定
 */
bool adc_is_bound(adc_channel_t channel);

/* 便捷宏 - 默认使用通道0 */

#define adc_init_default()              adc_init(ADC_CHANNEL_0)
#define adc_read_default()              adc_read(ADC_CHANNEL_0)
#define adc_read_average_default(samples) adc_read_average(ADC_CHANNEL_0, (samples))
#define adc_read_voltage_default()      adc_read_voltage(ADC_CHANNEL_0)
#define adc_read_advanced_default(samples, ref_mv) adc_read_advanced(ADC_CHANNEL_0, (samples), (ref_mv))

/* 快速读取宏 */

#define ADC_READ(channel)               adc_read(channel)
#define ADC_READ_AVG(channel, samples)  adc_read_average(channel, samples)
#define ADC_READ_VOLT(channel)          adc_read_voltage(channel)

#ifdef __cplusplus
}
#endif