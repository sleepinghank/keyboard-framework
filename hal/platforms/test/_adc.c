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

#include "adc.h"
#include "pin_defs.h"
#include "pin_mapper.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

// 调试输出宏
#define ADC_DEBUG_PRINT(fmt, ...) printf("[ADC] " fmt "\n", ##__VA_ARGS__)

// ADC channel state structure
typedef struct {
    pin_t pin;           // GPIO pin bound to this channel
    bool initialized;    // Channel initialization flag
} adc_channel_state_t;

// ADC channel state array
static adc_channel_state_t adc_channels[ADC_CHANNEL_MAX] = {0};

// Current active ADC channel
static adc_channel_t adc_current_channel = ADC_CHANNEL_0;

/*==========================================
 * Channel-based ADC functions
 *=========================================*/

void adc_init(adc_channel_t channel) {
    if (channel >= ADC_CHANNEL_MAX) {
        ADC_DEBUG_PRINT("adc_init: Invalid channel %d", channel);
        return;
    }

    // Initialize the ADC channel
    adc_channels[channel].initialized = true;
    ADC_DEBUG_PRINT("adc_init: Initialized channel %d", channel);
}

void adc_init_all(void) {
    adc_channel_t i;
    for (i = 0; i < ADC_CHANNEL_MAX; i++) {
        adc_init(i);
    }
    ADC_DEBUG_PRINT("adc_init_all: Initialized all %d channels", ADC_CHANNEL_MAX);
}

uint16_t adc_read(adc_channel_t channel) {
    if (channel >= ADC_CHANNEL_MAX || !adc_channels[channel].initialized) {
        ADC_DEBUG_PRINT("adc_read: Invalid channel %d", channel);
        return 0;
    }

    adc_current_channel = channel;

    // Test返回值（模拟ADC采样值）
    static uint16_t test_value = 2048; // 模拟中间值

    ADC_DEBUG_PRINT("adc_read(channel: %d) - 读取ADC值: %d",
                    channel, test_value);

    // 测试代码：返回模拟值（每次递增，模拟变化）
    test_value = (test_value + 1) % 4096;

    return test_value;
}

uint16_t adc_read_average(adc_channel_t channel, uint8_t samples) {
    if (channel >= ADC_CHANNEL_MAX || !adc_channels[channel].initialized) {
        ADC_DEBUG_PRINT("adc_read_average: Invalid channel %d", channel);
        return 0;
    }

    uint32_t sum = 0;
    uint8_t i;

    for (i = 0; i < samples; i++) {
        sum += adc_read(channel);
    }

    uint16_t average = sum / samples;
    ADC_DEBUG_PRINT("adc_read_average(channel: %d, samples: %d) - 平均值: %d",
                    channel, samples, average);

    return average;
}

uint32_t adc_read_voltage(adc_channel_t channel) {
    if (channel >= ADC_CHANNEL_MAX || !adc_channels[channel].initialized) {
        ADC_DEBUG_PRINT("adc_read_voltage: Invalid channel %d", channel);
        return 0;
    }

    // 假设参考电压为3300mV (3.3V)，12位ADC (4096个刻度)
    uint16_t adc_value = adc_read(channel);
    uint32_t voltage_mv = (adc_value * 3300) / 4096;

    ADC_DEBUG_PRINT("adc_read_voltage(channel: %d) - 电压值: %d mV",
                    channel, voltage_mv);

    return voltage_mv;
}

uint16_t adc_read_advanced(adc_channel_t channel, uint8_t samples, uint32_t reference_mv) {
    if (channel >= ADC_CHANNEL_MAX || !adc_channels[channel].initialized) {
        ADC_DEBUG_PRINT("adc_read_advanced: Invalid channel %d", channel);
        return 0;
    }

    uint16_t adc_value = adc_read_average(channel, samples);
    ADC_DEBUG_PRINT("adc_read_advanced(channel: %d, samples: %d, ref: %d mV) - ADC值: %d",
                    channel, samples, reference_mv, adc_value);

    return adc_value;
}

/*==========================================
 * GPIO pin binding functions
 *=========================================*/

bool adc_bind_pin(pin_t pin, adc_channel_t channel) {
    if (channel >= ADC_CHANNEL_MAX) {
        ADC_DEBUG_PRINT("adc_bind_pin: Invalid channel %d", channel);
        return false;
    }

    // Store the pin assignment
    adc_channels[channel].pin = pin;
    adc_channels[channel].initialized = true;

    ADC_DEBUG_PRINT("adc_bind_pin: Bound pin 0x%02X to channel %d", pin, channel);

    // TODO: In a real implementation, you would:
    // 1. Configure the GPIO pin as analog input
    // 2. Configure the ADC channel
    // 3. Initialize the ADC peripheral if not already done

    return true;
}

pin_t adc_get_bound_pin(adc_channel_t channel) {
    if (channel >= ADC_CHANNEL_MAX) {
        return NO_PIN;
    }

    return adc_channels[channel].pin;
}

bool adc_is_bound(adc_channel_t channel) {
    if (channel >= ADC_CHANNEL_MAX) {
        return false;
    }

    return adc_channels[channel].initialized;
}

/*==========================================
 * Legacy pin-based functions (for backward compatibility)
 *=========================================*/

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
void adc_set_pin(pin_t pin) {
    ADC_DEBUG_PRINT("adc_set_pin(pin: 0x%02X) - 设置引脚为ADC输入模式", 
                    pin);
    
    // TODO: 实际实现时，需要：
    // 1. 配置GPIO为模拟输入模式
    // 2. 配置ADC通道
    // 3. 初始化ADC外设（如果尚未初始化）
    
    // 测试代码：调用GPIO设置函数（模拟）
    // gpio_set_pin_input(pin);
}


