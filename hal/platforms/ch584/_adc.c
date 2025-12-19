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
 *
 *  Based on WCH CH58x Standard Peripheral Library
 *  Reference: code_example\CH585EVT_2\EVT\EXAM\ADC\src\Main.c
 */

#include "adc.h"
#include "pin_defs.h"
#include "pin_mapper.h"
#include "CH58x_common.h"
#include <stdint.h>
#include <stdbool.h>

/* ADC引脚映射表 - 对应官方例程中的ADC_PIN数组 */
static const uint32_t adc_pin_map[ADC_CHANNEL_MAX] = {
    GPIO_Pin_4,   /* PA4,AIN0 */
    GPIO_Pin_5,   /* PA5,AIN1 */
    GPIO_Pin_12,  /* PA12,AIN2 */
    GPIO_Pin_13,  /* PA13,AIN3 */
    GPIO_Pin_14,  /* PA14,AIN4 */
    GPIO_Pin_15,  /* PA15,AIN5 */
    GPIO_Pin_3,   /* PA3,AIN6 */
    GPIO_Pin_2,   /* PA2,AIN7 */
    GPIO_Pin_1,   /* PA1,AIN8 */
    GPIO_Pin_0,   /* PA0,AIN9 */
    GPIO_Pin_6,   /* PA6,AIN10 */
    GPIO_Pin_7,   /* PA7,AIN11 */
    GPIO_Pin_8,   /* PA8,AIN12 */
    GPIO_Pin_9    /* PA9,AIN13 */
};

/* ADC通道状态结构体 */
typedef struct {
    pin_t pin;              /* 绑定的GPIO引脚 */
    bool initialized;       /* 初始化标志 */
    adc_sample_freq_t freq; /* 采样频率 */
    adc_pga_t gain;         /* 增益设置 */
    adc_mode_t mode;        /* 工作模式 */
} adc_channel_state_t;

/* ADC通道状态数组 */
static adc_channel_state_t adc_channels[ADC_CHANNEL_MAX] = {0};

/* 当前活动的ADC通道 */
static adc_channel_t adc_current_channel = ADC_CHANNEL_0;

/* ADC校准值 */
static int16_t g_adc_calib_value = 0;

/* 中断模式相关变量 */
static volatile uint8_t g_adc_sample_count = 0;
static volatile uint16_t g_adc_sample_buffer[64];
static adc_callback_t g_adc_callback = NULL;

/* DMA模式相关变量 */
static volatile bool g_adc_dma_end = FALSE;
static uint16_t* g_adc_dma_buffer = NULL;
static uint16_t g_adc_buffer_size = 0;

/* 扫描模式相关变量 */
static const adc_channel_t* g_scan_channels = NULL;
static uint8_t g_scan_channel_count = 0;
static uint16_t* g_scan_buffer = NULL;

/*==========================================
 * 内部辅助函数
 *=========================================*/

/**
 * @brief   配置GPIO为ADC输入模式
 * @param   channel ADC通道号
 * @return  none
 */
static void adc_configure_gpio_pin(adc_channel_t channel) {
    if (channel >= ADC_CHANNEL_MAX) {
        return;
    }

    uint32_t pin = adc_pin_map[channel];

    /* 配置GPIO为浮空输入模式 */
    GPIOA_ModeCfg(pin, GPIO_ModeIN_Floating);
}

/**
 * @brief   转换采样频率枚举为CH58x库参数
 * @param   freq 采样频率枚举
 * @return  CH58x库采样频率参数
 */
static adc_sample_freq_t adc_convert_freq_param(adc_sample_freq_t freq) {
    return (freq == ADC_SAMPLE_FREQ_8MHZ) ? SampleFreq_8_or_4 : SampleFreq_4_or_2;
}

/*==========================================
 * 基础ADC函数实现
 *=========================================*/

void adc_init_single(adc_channel_t channel, adc_sample_freq_t freq, adc_pga_t gain) {
    if (channel >= ADC_CHANNEL_MAX) {
        return;
    }

    /* 保存配置 */
    adc_channels[channel].initialized = TRUE;
    adc_channels[channel].freq = freq;
    adc_channels[channel].gain = gain;
    adc_channels[channel].mode = ADC_MODE_SINGLE;
    adc_channels[channel].pin = PA0 + channel; /* 记录绑定的引脚 */

    /* 配置GPIO引脚 */
    adc_configure_gpio_pin(channel);

    /* 初始化ADC（单端模式） */
    ADC_ExtSingleChSampInit(adc_convert_freq_param(freq), (uint8_t)gain);

    /* 配置通道 */
    ADC_ChannelCfg(channel);

    adc_current_channel = channel;
}

void adc_init_diff(adc_channel_t channel, adc_sample_freq_t freq, adc_pga_t gain) {
    if (channel >= ADC_CHANNEL_MAX) {
        return;
    }

    /* 保存配置 */
    adc_channels[channel].initialized = TRUE;
    adc_channels[channel].freq = freq;
    adc_channels[channel].gain = gain;
    adc_channels[channel].mode = ADC_MODE_DIFFERENTIAL;
    adc_channels[channel].pin = PA0 + channel;

    /* 配置GPIO引脚（差分模式需要配置两个引脚） */
    if (channel == 0) {
        /* 通道0对应PA4(AIN0)、PA12(AIN2) */
        GPIOA_ModeCfg(GPIO_Pin_4 | GPIO_Pin_12, GPIO_ModeIN_Floating);
    } else {
        adc_configure_gpio_pin(channel);
    }

    /* 初始化ADC（差分模式） */
    ADC_ExtDiffChSampInit(adc_convert_freq_param(freq), (uint8_t)gain);

    /* 配置通道 */
    ADC_ChannelCfg(channel);

    adc_current_channel = channel;
}

void adc_init_temperature(void) {
    /* 初始化温度传感器采样 */
    ADC_InterTSSampInit();
}

void adc_init_touchkey(void) {
    /* TouchKey通道采样初始化 */
    TouchKey_ChSampInit();
}

uint16_t adc_start_single_conversion(void) {
    /* 启动单次转换并丢弃首次数据（官方例程推荐） */
    ADC_ExcutSingleConver();

    /* 执行转换 */
    uint16_t value = ADC_ExcutSingleConver();

    /* 应用校准值 */
    value += g_adc_calib_value;

    return value;
}

uint16_t adc_start_touchkey_conversion(uint8_t charge_time, uint8_t discharge_time) {
    /* TouchKey单次转换 */
    return TouchKey_ExcutSingleConver(charge_time, discharge_time);
}

uint16_t adc_read_value(void) {
    /* 读取转换值 */
    return ADC_ReadConverValue();
}

uint16_t adc_read_average(adc_channel_t channel, uint8_t samples) {
    if (channel >= ADC_CHANNEL_MAX || !adc_channels[channel].initialized) {
        return 0;
    }

    uint32_t sum = 0;
    uint8_t i;

    /* 保存当前通道 */
    adc_channel_t saved_channel = adc_current_channel;

    /* 切换到目标通道 */
    adc_current_channel = channel;
    ADC_ChannelCfg(channel);

    /* 丢弃首次转换 */
    ADC_ExcutSingleConver();

    /* 执行多次采样 */
    for (i = 0; i < samples; i++) {
        sum += adc_start_single_conversion();
    }

    /* 恢复原通道 */
    adc_current_channel = saved_channel;
    ADC_ChannelCfg(saved_channel);

    return (uint16_t)(sum / samples);
}

uint32_t adc_read_voltage(adc_channel_t channel) {
    uint16_t adc_value = adc_read_average(channel, 1);

    /* 假设参考电压为3.3V，12位ADC（4096个刻度） */
    return ((uint32_t)adc_value * 3300) / 4096;
}

int16_t adc_calibrate(void) {
    /* 执行ADC数据校准 */
    g_adc_calib_value = ADC_DataCalib_Rough();

    return g_adc_calib_value;
}

/*==========================================
 * 中断模式ADC函数实现
 *=========================================*/

void adc_config_interrupt(adc_channel_t channel, adc_callback_t callback) {
    if (channel >= ADC_CHANNEL_MAX) {
        return;
    }

    g_adc_callback = callback;
    g_adc_sample_count = 0;

    /* 配置ADC中断模式 */
    ADC_ChannelCfg(channel);

    /* 清除中断标志 */
    ADC_ClearITFlag();

    /* 使能ADC中断 */
    PFIC_EnableIRQ(ADC_IRQn);
}

void adc_start_interrupt(void) {
    /* 启动ADC */
    ADC_StartUp();
}

void adc_stop_interrupt(void) {
    /* 禁用ADC中断 */
    PFIC_DisableIRQ(ADC_IRQn);
}

bool adc_get_interrupt_status(void) {
    /* 获取中断状态 */
    return (bool)ADC_GetITStatus();
}

void adc_clear_interrupt_flag(void) {
    /* 清除中断标志 */
    ADC_ClearITFlag();
}

/*==========================================
 * DMA模式ADC函数实现
 *=========================================*/

void adc_config_dma(uint16_t* buffer, uint16_t size) {
    if (!buffer || size == 0) {
        return;
    }

    g_adc_dma_buffer = buffer;
    g_adc_buffer_size = size;
    g_adc_dma_end = FALSE;

    /* 设置自动转换周期 */
    ADC_AutoConverCycle(192);

    /* 配置DMA */
    ADC_DMACfg(ENABLE, (uint32_t)buffer, (uint32_t)(buffer + size), ADC_Mode_Single);

    /* 使能ADC中断 */
    PFIC_EnableIRQ(ADC_IRQn);
}

void adc_start_dma(void) {
    /* 启动自动DMA传输 */
    ADC_StartAutoDMA();
}

void adc_stop_dma(void) {
    /* 停止自动DMA传输 */
    ADC_StopAutoDMA();

    /* 禁用DMA配置 */
    ADC_DMACfg(DISABLE, 0, 0, 0);
}

bool adc_get_dma_status(void) {
    /* 获取DMA状态 */
    return g_adc_dma_end;
}

void adc_clear_dma_flag(void) {
    /* 清除DMA标志 */
    ADC_ClearDMAFlag();
}

/*==========================================
 * 扫描模式ADC函数实现
 *=========================================*/

void adc_config_scan(const adc_channel_t* channels, uint8_t num_channels, uint16_t* buffer, uint16_t buffer_size) {
    if (!channels || !buffer || num_channels == 0 || buffer_size < num_channels) {
        return;
    }

    g_scan_channels = channels;
    g_scan_channel_count = num_channels;
    g_scan_buffer = buffer;

    uint32_t temp;
    uint8_t i;

    /* 配置所有扫描通道的GPIO为浮空输入 */
    uint32_t all_pins = 0;
    for (i = 0; i < num_channels; i++) {
        if (channels[i] < ADC_CHANNEL_MAX) {
            all_pins |= adc_pin_map[channels[i]];
        }
    }
    GPIOA_ModeCfg(all_pins, GPIO_ModeIN_Floating);

    /* 初始化ADC */
    ADC_ExtSingleChSampInit(SampleFreq_4_or_2, ADC_PGA_0);

    /* 设置采样周期 */
    R8_ADC_CONVERT |= RB_ADC_SAMPLE_TIME; /* 7个采样周期 */

    /* 配置第一个通道 */
    ADC_ChannelCfg(channels[0]);

    /* 配置扫描通道1-8 */
    temp = 0;
    for (i = 1; i < num_channels && i < 9; i++) {
        temp |= (channels[i] << (i * 4));
    }
    R32_ADC_SCAN_CFG1 = temp;

    /* 配置扫描通道9-14 */
    temp = 0;
    for (i = 9; i < num_channels; i++) {
        temp |= (channels[i] << ((i - 8) * 4));
    }
    temp |= RB_ADC_SCAN_SEL | RB_ADC_IE_SCAN_END | ((num_channels - 1) << 24);
    R32_ADC_SCAN_CFG2 = temp;

    /* 配置DMA缓冲区 */
    R32_ADC_DMA_BEG = ((uint32_t)buffer) & 0x1FFFF;
    R32_ADC_DMA_END = ((uint32_t)(buffer + buffer_size)) & 0x1FFFF;

    /* 使能DMA */
    R8_ADC_CTRL_DMA |= RB_ADC_DMA_ENABLE | RB_SCAN_AUTO_TYPE;

    /* 使能ADC中断 */
    PFIC_EnableIRQ(ADC_IRQn);
}

void adc_start_scan(void) {
    /* 启动ADC扫描 */
    R8_ADC_CONVERT = RB_ADC_START;
}

void adc_stop_scan(void) {
    /* 停止ADC扫描（通过清除启动位） */
    R8_ADC_CONVERT &= ~RB_ADC_START;
}

/*==========================================
 * GPIO引脚绑定函数实现
 *=========================================*/

bool adc_bind_pin(pin_t pin, adc_channel_t channel) {
    if (channel >= ADC_CHANNEL_MAX) {
        return FALSE;
    }

    adc_channels[channel].pin = pin;
    adc_channels[channel].initialized = TRUE;

    return TRUE;
}

pin_t adc_get_bound_pin(adc_channel_t channel) {
    if (channel >= ADC_CHANNEL_MAX) {
        return NO_PIN;
    }

    return adc_channels[channel].pin;
}

bool adc_is_bound(adc_channel_t channel) {
    if (channel >= ADC_CHANNEL_MAX) {
        return FALSE;
    }

    return adc_channels[channel].initialized;
}

/*==========================================
 * ADC中断服务程序
 *=========================================*/

__INTERRUPT
__HIGH_CODE
void ADC_IRQHandler(void) {
    /* 处理DMA传输完成 */
    if (ADC_GetDMAStatus()) {
        ADC_StopAutoDMA();
        R32_ADC_DMA_BEG = ((uint32_t)g_adc_dma_buffer) & 0x1FFFF;
        ADC_ClearDMAFlag();
        g_adc_dma_end = TRUE;
    }

    /* 处理转换完成中断 */
    if (ADC_GetITStatus()) {
        ADC_ClearITFlag();

        /* 读取转换值 */
        uint16_t value = ADC_ReadConverValue();

        /* 如果有回调函数，调用它 */
        if (g_adc_callback) {
            g_adc_callback(value);
        }

        /* 保存到缓冲区（如果缓冲区存在） */
        if (g_adc_sample_count < sizeof(g_adc_sample_buffer) / sizeof(g_adc_sample_buffer[0])) {
            g_adc_sample_buffer[g_adc_sample_count] = value;
            g_adc_sample_count++;
        }

        /* 启动新一轮采样 */
        ADC_StartUp();
    }

    /* 处理扫描模式中断 */
    if (R32_ADC_SCAN_CFG2 & RB_ADC_IF_SCAN_END) {
        R32_ADC_SCAN_CFG2 |= RB_ADC_IF_SCAN_END;
        R32_ADC_DMA_BEG = ((uint32_t)g_scan_buffer) & 0x1FFFF;
        g_adc_dma_end = TRUE;
    }
}


