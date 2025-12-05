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

#include "gpio.h"
#include "pin_defs.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * @fn      adc_set_pin
 * 
 * @brief   设置引脚为ADC输入模式
 * 
 * @param   pin 引脚编号
 * 
 * @return  none
 * 
 * @note    此函数将引脚配置为模拟输入模式，用于ADC采样
 */
void adc_set_pin(pin_t pin);

/*********************************************************************
 * @fn      adc_read
 * 
 * @brief   读取ADC值
 * 
 * @param   pin 引脚编号
 * 
 * @return  ADC采样值（通常为12位，范围0-4095）
 * 
 * @note    读取指定引脚的ADC值，需要先调用adc_set_pin配置引脚
 */
uint16_t adc_read(pin_t pin);

/*********************************************************************
 * @fn      adc_read_pin
 * 
 * @brief   读取ADC值（别名函数，与adc_read功能相同）
 * 
 * @param   pin 引脚编号
 * 
 * @return  ADC采样值（通常为12位，范围0-4095）
 */
static inline uint16_t adc_read_pin(pin_t pin) {
    return adc_read(pin);
}

#ifdef __cplusplus
}
#endif
