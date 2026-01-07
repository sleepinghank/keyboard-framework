/* Copyright 2022 @ lokher (https://www.keychron.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

// 充电状态枚举
enum {
    BAT_NOT_CHARGING = 0,    // 未充电
    BAT_CHARGING,            // 充电中
    BAT_CHARGING_FINISHED,   // 充电完成
};

// 电源状态枚举
enum {
    BAT_POWER_NORMAL = 0,      // 正常电量 (>20% 且 >3.7V)
    BAT_POWER_LOW,             // 低电量 (<20% 且 >3.3V)
    BAT_POWER_CRITICAL_LOW,    // 严重低电 (<5% 或 <3.3V)
    BAT_POWER_SHUTDOWN,        // 需要关机 (<3.0V)
};

// 默认电压阈值配置
#ifndef FULL_VOLTAGE_VALUE
#    define FULL_VOLTAGE_VALUE 4200    // 满电电压 4.2V
#endif

#ifndef EMPTY_VOLTAGE_VALUE
#    define EMPTY_VOLTAGE_VALUE 3400    // 空电电压 3.4V
#endif

#ifndef SHUTDOWN_VOLTAGE_VALUE
#    define SHUTDOWN_VOLTAGE_VALUE 3000    // 关机电压 3.0V
#endif

#ifndef CRITICAL_VOLTAGE_VALUE
#    define CRITICAL_VOLTAGE_VALUE 3300    // 严重低电电压 3.3V
#endif

#ifndef NORMAL_VOLTAGE_VALUE
#    define NORMAL_VOLTAGE_VALUE 3700    // 正常电压阈值 3.7V
#endif

// 测量间隔配置
#ifndef VOLTAGE_MEASURE_INTERVAL
#    define VOLTAGE_MEASURE_INTERVAL 3000    // 电压测量间隔 (ms)
#endif

#ifndef VOLTAGE_POWER_ON_MEASURE_COUNT
#    define VOLTAGE_POWER_ON_MEASURE_COUNT 15    // 开机测量次数
#endif

#ifndef BACKLIGHT_OFF_VOLTAGE_MEASURE_INTERVAL
#    define BACKLIGHT_OFF_VOLTAGE_MEASURE_INTERVAL 200    // 背光关闭时的测量间隔 (ms)
#endif

// 电池内阻配置 (mΩ)
#ifndef BATTERY_INTERNAL_RESISTANCE
#    define BATTERY_INTERNAL_RESISTANCE 100
#endif

// LED/RGB补偿电压配置 (mV)
#ifndef VOLTAGE_TRIM_LED_MATRIX
#    define VOLTAGE_TRIM_LED_MATRIX 30
#endif

#ifndef VOLTAGE_TRIM_RGB_MATRIX
#    define VOLTAGE_TRIM_RGB_MATRIX 60
#endif

// 函数声明

/*********************************************************************
 * @fn      battery_init
 *
 * @brief   初始化电池检测模块
 *          配置ADC引脚、充电检测引脚，初始化变量
 *
 * @return  none
 */
void battery_init(void);

/*********************************************************************
 * @fn      battery_stop
 *
 * @brief   停止电池检测
 *          关闭ADC，释放相关引脚
 *
 * @return  none
 */
void battery_stop(void);

/*********************************************************************
 * @fn      battery_get_charging_state
 *
 * @brief   获取当前充电状态
 *
 * @return  uint8_t - 充电状态 (BAT_NOT_CHARGING/BAT_CHARGING/BAT_CHARGING_FINISHED)
 */
uint8_t battery_get_charging_state(void);

/*********************************************************************
 * @fn      battery_get_adc_value
 *
 * @brief   获取电量检测的ADC原始值
 *
 * @return  uint16_t - ADC原始值 (0-1023)
 */
uint16_t battery_get_adc_value(void);

/*********************************************************************
 * @fn      battery_calculate_voltage_from_adc
 *
 * @brief   根据ADC值计算电池电压（包含LED/RGB和负载补偿）
 *
 * @param   adc_value - ADC原始值
 *
 * @return  uint16_t - 计算后的电池电压 (mV)
 */
uint16_t battery_calculate_voltage_from_adc(uint16_t adc_value);

/*********************************************************************
 * @fn      battery_get_voltage
 *
 * @brief   获取存储的电池电压值
 *
 * @return  uint16_t - 电池电压 (mV)
 */
uint16_t battery_get_voltage(void);

/*********************************************************************
 * @fn      battery_voltage_to_percentage
 *
 * @brief   根据电池电压获取电量百分比（查表法）
 *
 * @param   voltage - 电池电压 (mV)
 *
 * @return  uint8_t - 电量百分比 (0-100)
 */
uint8_t battery_voltage_to_percentage(uint16_t voltage);

/*********************************************************************
 * @fn      battery_get_percentage
 *
 * @brief   获取当前存储的电量百分比
 *          如果ADC不可用，返回默认100%
 *
 * @return  uint8_t - 电量百分比 (0-100)
 */
uint8_t battery_get_percentage(void);

/*********************************************************************
 * @fn      battery_get_power_state
 *
 * @brief   根据当前电量百分比和电压判断系统电源状态
 *
 * @return  uint8_t - 电源状态 (BAT_POWER_NORMAL/LOW/CRITICAL_LOW/SHUTDOWN)
 */
uint8_t battery_get_power_state(void);

/*********************************************************************
 * @fn      battery_task
 *
 * @brief   电池检测任务函数
 *          定期执行ADC采样、电压计算、状态更新
 *
 * @return  none
 */
void battery_task(void);

/*********************************************************************
 * @fn      battery_is_critical_low
 *
 * @brief   检查电池是否处于严重低电状态
 *
 * @return  bool - 是否严重低电
 */
bool battery_is_critical_low(void);

/*********************************************************************
 * @fn      battery_is_empty
 *
 * @brief   检查电池是否已经完全耗尽（需要关机）
 *          判断条件：电压 < 关机电压阈值 (3.0V)
 *
 * @return  bool - true表示电池耗尽，false表示仍有电量
 */
bool battery_is_empty(void);

// 保留的兼容性函数
void     battery_measure(void);
bool     battery_power_on_sample(void);
