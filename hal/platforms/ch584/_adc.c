/*  Copyright (C) 2024
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
 *
 *  Based on WCH CH58x Standard Peripheral Library
 *  Reference: project\ch584m\StdPeriphDriver\CH58x_adc.c
 */

#include "adc.h"
#include "pin_defs.h"
#include "pin_mapper.h"
#include "CH58x_common.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/*==========================================
 * ADC通道状态结构体
 *=========================================*/
typedef struct {
    bool initialized;           /**< 通道是否已初始化 */
    bool enabled;               /**< 通道是否启用 */
    adc_mode_t mode;            /**< 采样模式 */
    uint8_t avg_samples;        /**< 平均采样次数 */
    pin_t bound_pin;            /**< 绑定的GPIO引脚（NO_PIN表示未绑定） */
    bool interrupt_enabled;     /**< 中断是否启用 */
    uint16_t interrupt_threshold; /**< 中断阈值 */
} adc_channel_state_t;

/*==========================================
 * ADC全局状态
 *=========================================*/
static adc_channel_state_t adc_channels[ADC_CHANNEL_MAX];
static adc_callback_t adc_callback = NULL;  /**< 全局ADC回调函数 */
static bool adc_initialized = FALSE;        /**< ADC模块是否已初始化 */
static int16_t adc_calib_value = 0;         /**< ADC校准值（内部使用） */

/*==========================================
 * 内部辅助函数
 *=========================================*/

/**
 * @brief 配置GPIO引脚为ADC输入模式
 * @param pin GPIO引脚号
 * @return none
 */
static void adc_configure_gpio(pin_t pin) {
    if (pin == NO_PIN) {
        return;
    }

    /* 根据引脚号配置对应的GPIO端口 */
    /* 这里简化处理，实际需要根据具体引脚映射 */
    GPIOA_ModeCfg(1 << (pin & 0xFF), GPIO_ModeIN_Floating);
}

/**
 * @brief 获取CH584采样频率参数
 * @return CH584采样频率参数
 * @note 根据电池检测案例，使用4MHz采样频率（SampleFreq_4_or_2）
 */
static ADC_SampClkTypeDef adc_get_sample_freq(void) {
    return SampleFreq_4_or_2;
}

/**
 * @brief 获取CH584 PGA增益参数
 * @return PGA增益参数
 * @note 根据电池检测案例，使用1/2增益（ADC_PGA_1_2）
 */
static ADC_SignalPGATypeDef adc_get_pga_gain(void) {
    return ADC_PGA_1_2;
}

/**
 * @brief 执行ADC采样（包含校准）
 * @param channel ADC通道号
 * @return uint16_t 采样值（已应用校准）
 * @note 执行单次ADC转换并应用校准值
 */
static uint16_t adc_perform_conversion(adc_channel_t channel) {
    /* 执行转换 */
    uint16_t value = ADC_ExcutSingleConver();

    /* 应用校准值（电池检测案例使用的方法） */
    value += adc_calib_value;

    return value;
}

/*==========================================
 * ADC基础初始化
 *=========================================*/

void adc_init(void) {
    if (adc_initialized) {
        return;  /* 避免重复初始化 */
    }

    /* 清零所有通道状态 */
    memset(adc_channels, 0, sizeof(adc_channels));

    /* 初始化每个通道的默认状态 */
    uint8_t i;
    for (i = 0; i < ADC_CHANNEL_MAX; i++) {
        adc_channels[i].initialized = FALSE;
        adc_channels[i].enabled = FALSE;
        adc_channels[i].bound_pin = NO_PIN;
        adc_channels[i].interrupt_enabled = FALSE;
    }

    adc_initialized = TRUE;
}

/*==========================================
 * ADC通道初始化
 *=========================================*/

void adc_init_channel(adc_channel_t channel, adc_mode_t mode, uint8_t avg_samples) {
    /* 检查参数有效性 */
    if (channel >= ADC_CHANNEL_MAX) {
        return;
    }

    if (avg_samples == 0) {
        avg_samples = 1;  /* 最小采样次数为1 */
    }

    if (avg_samples > 255) {
        avg_samples = 255;  /* 最大采样次数限制 */
    }

    /* 更新通道状态 */
    adc_channels[channel].initialized = TRUE;
    adc_channels[channel].enabled = TRUE;
    adc_channels[channel].mode = mode;
    adc_channels[channel].avg_samples = avg_samples;

    /* 如果通道已绑定GPIO，配置GPIO为浮空输入 */
    if (adc_channels[channel].bound_pin != NO_PIN) {
        adc_configure_gpio(adc_channels[channel].bound_pin);
    }

    /* 初始化ADC硬件（单端模式） */
    /* 使用电池检测案例中的参数：4MHz采样频率，1/2增益 */
    ADC_ExtSingleChSampInit(adc_get_sample_freq(), adc_get_pga_gain());

    /* 配置通道 */
    ADC_ChannelCfg(channel);

    /* 执行ADC校准（仅在首次初始化时） */
    if (adc_calib_value == 0) {
        adc_calib_value = ADC_DataCalib_Rough();
    }
}

/*==========================================
 * GPIO引脚绑定功能
 *=========================================*/

bool adc_bind_pin(pin_t pin, adc_channel_t channel) {
    /* 检查参数有效性 */
    if (channel >= ADC_CHANNEL_MAX || pin == NO_PIN) {
        return FALSE;
    }

    adc_channel_state_t *ch = &adc_channels[channel];

    /* 检查通道是否已经绑定了其他引脚 */
    if (ch->bound_pin != NO_PIN && ch->bound_pin != pin) {
        return FALSE;  /* 通道已绑定其他引脚 */
    }

    /* 绑定引脚 */
    ch->bound_pin = pin;

    /* 如果通道已初始化，配置GPIO */
    if (ch->initialized) {
        adc_configure_gpio(pin);
    }

    return TRUE;
}

bool adc_unbind_pin(pin_t pin) {
    adc_channel_t channel;

    /* 遍历所有通道查找引脚 */
    for (channel = 0; channel < ADC_CHANNEL_MAX; channel++) {
        adc_channel_state_t *ch = &adc_channels[channel];

        /* 查找匹配的引脚 */
        if (ch->bound_pin == pin) {
            ch->bound_pin = NO_PIN;  /* 解绑引脚 */
            return TRUE;
        }
    }

    return FALSE;  /* 未找到引脚 */
}

pin_t adc_get_bound_pin(adc_channel_t channel) {
    /* 检查参数有效性 */
    if (channel >= ADC_CHANNEL_MAX) {
        return NO_PIN;
    }

    adc_channel_state_t *ch = &adc_channels[channel];

    /* 返回绑定的引脚 */
    return ch->bound_pin;
}

bool adc_is_bound(adc_channel_t channel) {
    /* 检查参数有效性 */
    if (channel >= ADC_CHANNEL_MAX) {
        return FALSE;
    }

    return (adc_channels[channel].bound_pin != NO_PIN);
}

/*==========================================
 * ADC采样功能
 *=========================================*/

uint16_t adc_read_single(adc_channel_t channel) {
    /* 检查参数有效性 */
    if (channel >= ADC_CHANNEL_MAX) {
        return 0;
    }

    adc_channel_state_t *ch = &adc_channels[channel];

    /* 检查通道是否已初始化 */
    if (!ch->initialized) {
        return 0;
    }

    /* 检查通道是否已启用 */
    if (!ch->enabled) {
        return 0;
    }

    /* 切换到目标通道 */
    ADC_ChannelCfg(channel);

    /* 执行单次转换 */
    uint16_t value = adc_perform_conversion(channel);

    return value;
}

uint16_t adc_read_average(adc_channel_t channel, uint8_t samples) {
    /* 检查参数有效性 */
    if (channel >= ADC_CHANNEL_MAX) {
        return 0;
    }

    if (samples == 0) {
        return 0;
    }

    adc_channel_state_t *ch = &adc_channels[channel];

    /* 检查通道是否已初始化 */
    if (!ch->initialized) {
        return 0;
    }

    /* 检查通道是否已启用 */
    if (!ch->enabled) {
        return 0;
    }

    /* 切换到目标通道 */
    ADC_ChannelCfg(channel);

    /* 根据电池检测案例，先丢弃前4次采样以提高稳定性 */
    uint8_t i;
    for (i = 0; i < 4; i++) {
        adc_perform_conversion(channel);
    }

    /* 执行指定次数的采样并求平均 */
    uint32_t sum = 0;
    for (i = 0; i < samples; i++) {
        sum += adc_perform_conversion(channel);
    }

    return (uint16_t)(sum / samples);
}

uint32_t adc_to_voltage(uint16_t raw_value, uint32_t vref_mv) {
    /* 转换公式：voltage = (raw_value * vref_mv) / 4096 */
    /* CH584 ADC为12位，范围0-4095 */

    /* 根据电池检测案例，测量点电压转换（使用3000mV参考） */
    uint32_t voltage = ((uint32_t)raw_value * 3000) / 4096;

    /* 如果使用1/2增益，需要补偿（ADC_PGA_1_2） */
    /* 电池检测案例中测量点电压需要除以0.747得到电池电压 */
    /* 这里返回测量点电压，实际电池电压需要除以0.747 */
    return voltage;
}

/*==========================================
 * ADC中断控制功能
 *=========================================*/

void adc_enable_interrupt(adc_channel_t channel, uint16_t threshold) {
    /* 检查参数有效性 */
    if (channel >= ADC_CHANNEL_MAX) {
        return;
    }

    if (threshold > 4095) {
        threshold = 4095;  /* 限制在12位范围内 */
    }

    adc_channel_state_t *ch = &adc_channels[channel];

    /* 检查通道是否已初始化 */
    if (!ch->initialized) {
        return;
    }

    /* 启用中断并设置阈值 */
    ch->interrupt_enabled = TRUE;
    ch->interrupt_threshold = threshold;

    /* TODO: 配置CH584硬件中断（需要根据具体需求实现） */
    /* CH584支持ADC自动转换周期，可用于实现阈值检测 */
    /* 此处简化处理，实际应用中需要在ADC中断服务程序中检查阈值 */
}

void adc_disable_interrupt(adc_channel_t channel) {
    /* 检查参数有效性 */
    if (channel >= ADC_CHANNEL_MAX) {
        return;
    }

    adc_channel_state_t *ch = &adc_channels[channel];

    /* 禁用中断 */
    ch->interrupt_enabled = FALSE;

    /* TODO: 关闭CH584硬件中断 */
}

void adc_set_callback(adc_callback_t callback) {
    /* 注册全局回调函数 */
    adc_callback = callback;
}

/*==========================================
 * ADC启停控制功能
 *=========================================*/

void adc_stop(adc_channel_t channel) {
    /* 检查参数有效性 */
    if (channel >= ADC_CHANNEL_MAX) {
        return;
    }

    adc_channel_state_t *ch = &adc_channels[channel];

    /* 禁用通道 */
    ch->enabled = FALSE;

    /* 禁用中断 */
    if (ch->interrupt_enabled) {
        ch->interrupt_enabled = FALSE;
        /* TODO: 关闭硬件中断 */
    }
}

void adc_resume(adc_channel_t channel) {
    /* 检查参数有效性 */
    if (channel >= ADC_CHANNEL_MAX) {
        return;
    }

    adc_channel_state_t *ch = &adc_channels[channel];

    /* 检查通道是否已初始化 */
    if (!ch->initialized) {
        return;
    }

    /* 恢复通道 */
    ch->enabled = TRUE;

    /* 恢复中断（如果之前启用） */
    /* TODO: 重新启用硬件中断（如果需要） */
}
