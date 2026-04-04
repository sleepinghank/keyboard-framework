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
#include "battery.h"
#include "gpio.h"
#include "timer.h"
#include "indicator.h"
#include "adc.h"
#include "debug.h"
#include "kb904/config_hw.h"
#include "kb904/config_hw.h"
#include "kb904/config_hw.h"

// 分压电阻配置 (KΩ)
#ifndef RVD_R1
#    define RVD_R1 1000    // 上拉电阻
#endif

#ifndef RVD_R2
#    define RVD_R2 2000    // 下拉电阻
#endif

// 锂电池放电曲线查表 - 电压(mV) 对应 电量百分比(%)
// 从100%到0%，每5%一个电压值
static const uint16_t voltage_to_percentage_table[21] = {
    4200,  // 100% 
    4150,  // 95%
    4100,  // 90%
    4000,  // 85%
    3950,  // 80%
    3900,  // 75%
    3850,  // 70%
    3800,  // 65%
    3750,  // 60%
    3700,  // 55%
    3650,  // 50%
    3600,  // 45%
    3550,  // 40%
    3500,  // 35%
    3450,  // 30%
    3400,  // 25%
    3350,  // 20%
    3300,  // 15%
    3250,  // 10%
    3200,  // 5%
    3150   // 0%
};

// 全局变量
static uint16_t g_adc_value = 0;              // ADC原始值
static uint16_t g_battery_voltage = 4200;     // 存储的电池电压(mV)
static uint8_t g_battery_percentage = 100;    // 存储的电量百分比
static uint8_t g_charging_state = BAT_NOT_CHARGING;  // 充电状态
static uint32_t g_battery_timer = 0;          // 电池检测定时器

/*********************************************************************
 * @fn      battery_init
 *
 * @brief   初始化电池检测模块
 *          配置ADC引脚、充电检测引脚，初始化变量
 *
 * @return  none
 */
void battery_init(void) {
    // 初始化充电状态
    g_charging_state = BAT_NOT_CHARGING;
    // 初始化ADC
    adc_init();
    adc_bind_pin(BATTERY_ADC_PIN, ADC_CHANNEL_4);
    adc_init_channel(ADC_CHANNEL_4, ADC_MODE_SINGLE, 1);
 
    // 初始化变量
    g_adc_value = 0;
    g_battery_voltage = FULL_VOLTAGE_VALUE;
    g_battery_percentage = 100;
    g_battery_timer = timer_read();

}

/*********************************************************************
 * @fn      battery_stop
 *
 * @brief   停止电池检测
 *          关闭ADC，释放相关引脚
 *
 * @return  none
 */
void battery_stop(void) {
    // 关闭ADC使能
#ifdef BAT_ADC_ENABLE_PIN
    gpio_write_pin_low(BAT_ADC_ENABLE_PIN);
#endif

    // 释放ADC引脚
#ifdef BATTERY_ADC_PIN
    gpio_set_pin_input(BATTERY_ADC_PIN);
#endif
}

/*********************************************************************
 * @fn      battery_get_charging_state
 *
 * @brief   获取当前充电状态
 *
 * @return  uint8_t - 充电状态 (BAT_NOT_CHARGING/BAT_CHARGING/BAT_CHARGING_FINISHED)
 */
uint8_t battery_get_charging_state(void) {
    return g_charging_state;
}

/*********************************************************************
 * @fn      battery_get_adc_value
 *
 * @brief   获取电量检测的ADC原始值
 *
 * @return  uint16_t - ADC原始值 (0-1023)
 */
uint16_t battery_get_adc_value(void) {
    return g_adc_value;
}

/*********************************************************************
 * @fn      battery_calculate_voltage_from_adc
 *
 * @brief   根据ADC值计算电池电压（包含LED/RGB和负载补偿）
 *
 * @param   adc_value - ADC原始值
 *
 * @return  uint16_t - 计算后的电池电压 (mV)
 */
uint16_t battery_calculate_voltage_from_adc(uint16_t adc_value) {
    uint16_t voltage;
    uint32_t compensation = 0;

    // ADC参考电压3.3V，12位分辨率 (0-4095)，使用1024计算
    // 通过分压电阻计算实际电池电压
    voltage = ((uint32_t)adc_value * 1050 ) / 1024 - 1050;

    voltage = voltage * (RVD_R1 + RVD_R2) / RVD_R2;

    // LED/RGB亮度补偿 - 背光和指示灯开启时会消耗电流，导致电压下降
    // 需要根据LED亮度补偿电压
#if defined(LED_MATRIX_ENABLE) || defined(RGB_MATRIX_ENABLE)
    if (indicator_any_active()) {
        uint32_t total_pwm = 0;
        uint8_t led_count = 0;

#ifdef LED_MATRIX_ENABLE
        led_count = LED_MATRIX_LED_COUNT;
        extern uint8_t g_pwm_buffer[LED_MATRIX_DRIVER_COUNT][192];
        for (uint8_t i = 0; i < LED_MATRIX_DRIVER_COUNT; i++) {
            for (uint8_t j = 0; j < 192; j++) {
                total_pwm += g_pwm_buffer[i][j];
            }
        }
        // LED矩阵补偿
        compensation += (VOLTAGE_TRIM_LED_MATRIX * total_pwm / led_count / 255);
#endif

#ifdef RGB_MATRIX_ENABLE
        led_count = RGB_MATRIX_LED_COUNT;
        extern uint8_t g_pwm_buffer[RGB_MATRIX_DRIVER_COUNT][192];
        for (uint8_t i = 0; i < RGB_MATRIX_DRIVER_COUNT; i++) {
            for (uint8_t j = 0; j < 192; j++) {
                total_pwm += g_pwm_buffer[i][j];
            }
        }
        // RGB矩阵补偿 (每个LED有3个通道)
        compensation += (VOLTAGE_TRIM_RGB_MATRIX * total_pwm / led_count / 255 / 3);
#endif

        voltage += compensation;
    }
#endif

    // 负载电流补偿 - 根据当前工作模式估算电流消耗
    // 电流越大，电池内阻上的电压降越大，需要补偿
    // 这里可以根据实际工作模式调整补偿值
    // uint16_t estimated_current_ma = 0;

    // // 估算当前电流 (mA) - 根据系统状态
    // if (indicator_any_active()) {
    //     estimated_current_ma += 50;  // LED背光约50mA
    // }

    // // 根据电流和电池内阻计算电压降
    // uint16_t voltage_drop = (estimated_current_ma * BATTERY_INTERNAL_RESISTANCE) / 1000;
    // voltage += voltage_drop;

    return voltage;
}

/*********************************************************************
 * @fn      battery_get_voltage
 *
 * @brief   获取存储的电池电压值
 *
 * @return  uint16_t - 电池电压 (mV)
 */
uint16_t battery_get_voltage(void) {
    return g_battery_voltage;
}

/*********************************************************************
 * @fn      battery_voltage_to_percentage
 *
 * @brief   根据电池电压获取电量百分比（查表法）
 *          不使用线性插值，直接返回5%的整数倍，避免ADC波动导致显示波动
 *
 * @param   voltage - 电池电压 (mV)
 *
 * @return  uint8_t - 电量百分比 (0-100, 5的倍数)
 */
uint8_t battery_voltage_to_percentage(uint16_t voltage) {
    // 电压超出表格范围的处理
    if (voltage >= voltage_to_percentage_table[0]) {
        return 100;
    }

    if (voltage <= voltage_to_percentage_table[20]) {
        return 0;
    }

    // 在表格中查找对应的电量百分比
    // 直接返回表格中的百分比值，不进行插值
    for (uint8_t i = 0; i < 21; i++) {
        if (voltage >= voltage_to_percentage_table[i]) {
            // 返回5的倍数：100, 95, 90, ..., 5, 0
            return 100 - i * 5;
        }
    }

    return 0;
}

/*********************************************************************
 * @fn      battery_get_percentage
 *
 * @brief   获取当前存储的电量百分比
 *          如果ADC不可用，返回默认100%
 *
 * @return  uint8_t - 电量百分比 (0-100)
 */
uint8_t battery_get_percentage(void) {
    return g_battery_percentage;
}

/*********************************************************************
 * @fn      battery_get_power_state
 *
 * @brief   根据当前电量百分比和电压判断系统电源状态
 *
 * @return  uint8_t - 电源状态 (BAT_POWER_NORMAL/LOW/CRITICAL_LOW/SHUTDOWN)
 */
uint8_t battery_get_power_state(void) {
    // 优先检查电压阈值（更可靠）
    if (g_battery_voltage < SHUTDOWN_VOLTAGE_VALUE) {
        return BAT_POWER_SHUTDOWN;
    }

    if (g_battery_percentage < 5) {
        return BAT_POWER_CRITICAL_LOW;
    }

    if (g_battery_percentage < 20) {
        return BAT_POWER_LOW;
    }

    return BAT_POWER_NORMAL;
}

/*********************************************************************
 * @fn      battery_measure
 *
 * @brief   执行ADC采样和电压计算
 *
 * @return  none
 */
void battery_measure(void) {
#ifdef BATTERY_ADC_PIN
    g_adc_value = adc_read_average(ADC_CHANNEL_4, 4);
    dprintf("ADC value (4 times average): %d\n", g_adc_value);

    // 计算电压
    g_battery_voltage = battery_calculate_voltage_from_adc(g_adc_value);
    dprintf("voltage: %d\n", g_battery_voltage);

    // 根据电压计算电量百分比
    g_battery_percentage = battery_voltage_to_percentage(g_battery_voltage);
    dprintf("battery percentage: %d\n", g_battery_percentage);
#else
    // 如果没有ADC引脚，使用默认电压
    g_adc_value = 0;
    g_battery_voltage = FULL_VOLTAGE_VALUE;
    g_battery_percentage = 100;
#endif
}

/*********************************************************************
 * @fn      battery_power_on_sample
 *
 * @brief   检查是否处于开机采样阶段（兼容性函数）
 *
 * @return  bool - 是否在开机采样
 */
bool battery_power_on_sample(void) {
    // 简化处理，始终返回false
    return false;
}

/*********************************************************************
 * @fn      battery_task
 *
 * @brief   电池检测任务函数
 *          定期执行ADC采样、充电状态检测、电压更新
 *
 * @return  none
 */
void battery_task(void) {
#if defined(LED_MATRIX_ENABLE) || defined(RGB_MATRIX_ENABLE)
    // 如果背光开启，使用较短的测量间隔
    if (indicator_any_active()) {
        measure_interval = BACKLIGHT_OFF_VOLTAGE_MEASURE_INTERVAL;
    }
#endif
    // 执行ADC采样
    battery_measure();

    // 检测充电状态
#if defined(BAT_CHARGING_PIN)
    if (gpio_read_pin(BAT_CHARGING_PIN) == BAT_CHARGING_LEVEL) {
        g_charging_state = BAT_CHARGING;
    } else {
        g_charging_state = BAT_NOT_CHARGING;
    }
#endif
}

/*********************************************************************
 * @fn      battery_is_critical_low
 *
 * @brief   检查电池是否处于严重低电状态
 *          判断条件：电量百分比 < 5% 或 电压 < 3.3V
 *
 * @return  bool - true表示严重低电，false表示正常
 */
bool battery_is_critical_low(void) {
    // 检查电量百分比是否低于5%
    if (g_battery_percentage < 5) {
        return true;
    }

    // 检查电压是否低于严重低电阈值 (3.3V)
    if (g_battery_voltage < CRITICAL_VOLTAGE_VALUE) {
        return true;
    }

    return false;
}

/*********************************************************************
 * @fn      battery_is_empty
 *
 * @brief   检查电池是否已经完全耗尽（需要关机）
 *          判断条件：电压 < 关机电压阈值 (3.0V)
 *
 * @return  bool - true表示电池耗尽，false表示仍有电量
 */
bool battery_is_empty(void) {
    // 检查电压是否低于关机电压阈值 (3.0V)
    if (g_battery_voltage < SHUTDOWN_VOLTAGE_VALUE) {
        return true;
    }

    return false;
}
