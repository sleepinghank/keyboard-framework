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

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "gpio.h"
#include "pin_mapper.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ADC通道枚举 - CH584支持14个外部通道 */
typedef enum {
    ADC_CHANNEL_0 = 0,   /* PA4,AIN0 */
    ADC_CHANNEL_1 = 1,   /* PA5,AIN1 */
    ADC_CHANNEL_2 = 2,   /* PA12,AIN2 */
    ADC_CHANNEL_3 = 3,   /* PA13,AIN3 */
    ADC_CHANNEL_4 = 4,   /* PA14,AIN4 */
    ADC_CHANNEL_5 = 5,   /* PA15,AIN5 */
    ADC_CHANNEL_6 = 6,   /* PA3,AIN6 */
    ADC_CHANNEL_7 = 7,   /* PA2,AIN7 */
    ADC_CHANNEL_8 = 8,   /* PA1,AIN8 */
    ADC_CHANNEL_9 = 9,   /* PA0,AIN9 */
    ADC_CHANNEL_10 = 10, /* PA6,AIN10 */
    ADC_CHANNEL_11 = 11, /* PA7,AIN11 */
    ADC_CHANNEL_12 = 12, /* PA8,AIN12 */
    ADC_CHANNEL_13 = 13, /* PA9,AIN13 */
    ADC_CHANNEL_MAX = 14
} adc_channel_t;

/* ADC采样频率枚举 */
typedef enum {
    ADC_SAMPLE_FREQ_8MHZ = 0,  /* 8MHz采样频率 (或4MHz) */
    ADC_SAMPLE_FREQ_4MHZ = 1,  /* 4MHz采样频率 (或2MHz) */
} adc_sample_freq_t;

/* ADC增益枚举 */
typedef enum {
    ADC_PGA_0 = 0,   /* 0dB增益 */
    ADC_PGA_6 = 1,   /* 6dB增益 */
    ADC_PGA_12 = 2,  /* 12dB增益 */
    ADC_PGA_18 = 3,  /* 18dB增益 */
} adc_pga_t;

/* ADC模式枚举 */
typedef enum {
    ADC_MODE_SINGLE,      /* 单端模式 */
    ADC_MODE_DIFFERENTIAL /* 差分模式 */
} adc_mode_t;

/* ADC工作模式 */
typedef enum {
    ADC_WORK_MODE_POLLING,    /* 轮询模式 */
    ADC_WORK_MODE_INTERRUPT,  /* 中断模式 */
    ADC_WORK_MODE_DMA         /* DMA模式 */
} adc_work_mode_t;

/* ADC转换完成回调函数类型 */
typedef void (*adc_callback_t)(uint16_t value);

/*==========================================
 * 基础ADC函数
 *=========================================*/

/**
 * @brief   初始化ADC模块（单端模式）
 * @param   channel ADC通道号
 * @param   freq 采样频率
 * @param   gain 增益设置
 * @return  none
 */
void adc_init_single(adc_channel_t channel, adc_sample_freq_t freq, adc_pga_t gain);

/**
 * @brief   初始化ADC模块（差分模式）
 * @param   channel ADC通道号（正输入通道）
 * @param   freq 采样频率
 * @param   gain 增益设置
 * @return  none
 */
void adc_init_diff(adc_channel_t channel, adc_sample_freq_t freq, adc_pga_t gain);

/**
 * @brief   初始化温度传感器采样
 * @return  none
 */
void adc_init_temperature(void);

/**
 * @brief   初始化TouchKey采样
 * @return  none
 */
void adc_init_touchkey(void);

/**
 * @brief   启动单次ADC转换
 * @return  ADC采样值（12位）
 */
uint16_t adc_start_single_conversion(void);

/**
 * @brief   启动TouchKey单次转换
 * @param   charge_time 充电时间
 * @param   discharge_time 放电时间
 * @return  TouchKey采样值
 */
uint16_t adc_start_touchkey_conversion(uint8_t charge_time, uint8_t discharge_time);

/**
 * @brief   读取ADC转换值
 * @return  ADC采样值
 */
uint16_t adc_read_value(void);

/**
 * @brief   执行多次采样并返回平均值
 * @param   channel ADC通道号
 * @param   samples 采样次数
 * @return  平均值
 */
uint16_t adc_read_average(adc_channel_t channel, uint8_t samples);

/**
 * @brief   读取电压值（毫伏）
 * @param   channel ADC通道号
 * @return  电压值（毫伏）
 */
uint32_t adc_read_voltage(adc_channel_t channel);

/**
 * @brief   ADC数据校准
 * @return  校准值（用于修正ADC偏差）
 */
int16_t adc_calibrate(void);

/*==========================================
 * 中断模式ADC函数
 *=========================================*/

/**
 * @brief   配置ADC中断模式
 * @param   channel ADC通道号
 * @param   callback 转换完成回调函数
 * @return  none
 */
void adc_config_interrupt(adc_channel_t channel, adc_callback_t callback);

/**
 * @brief   启动ADC中断转换
 * @return  none
 */
void adc_start_interrupt(void);

/**
 * @brief   停止ADC中断转换
 * @return  none
 */
void adc_stop_interrupt(void);

/**
 * @brief   检查ADC中断状态
 * @return  true表示有中断请求
 */
bool adc_get_interrupt_status(void);

/**
 * @brief   清除ADC中断标志
 * @return  none
 */
void adc_clear_interrupt_flag(void);

/*==========================================
 * DMA模式ADC函数
 *=========================================*/

/**
 * @brief   配置ADC DMA模式
 * @param   buffer 接收缓冲区
 * @param   size 缓冲区大小
 * @return  none
 */
void adc_config_dma(uint16_t* buffer, uint16_t size);

/**
 * @brief   启动ADC DMA传输
 * @return  none
 */
void adc_start_dma(void);

/**
 * @brief   停止ADC DMA传输
 * @return  none
 */
void adc_stop_dma(void);

/**
 * @brief   检查DMA状态
 * @return  true表示DMA传输完成
 */
bool adc_get_dma_status(void);

/**
 * @brief   清除DMA标志
 * @return  none
 */
void adc_clear_dma_flag(void);

/*==========================================
 * 扫描模式ADC函数
 *=========================================*/

/**
 * @brief   配置ADC自动扫描模式
 * @param   channels 通道数组
 * @param   num_channels 通道数量
 * @param   buffer 接收缓冲区
 * @param   buffer_size 缓冲区大小
 * @return  none
 */
void adc_config_scan(const adc_channel_t* channels, uint8_t num_channels, uint16_t* buffer, uint16_t buffer_size);

/**
 * @brief   启动ADC扫描
 * @return  none
 */
void adc_start_scan(void);

/**
 * @brief   停止ADC扫描
 * @return  none
 */
void adc_stop_scan(void);

/*==========================================
 * GPIO引脚绑定函数
 *=========================================*/

/**
 * @brief   绑定GPIO引脚到ADC通道
 * @param   pin GPIO引脚号
 * @param   channel ADC通道号
 * @return  bool 绑定是否成功
 */
bool adc_bind_pin(pin_t pin, adc_channel_t channel);

/**
 * @brief   获取ADC通道绑定的GPIO引脚
 * @param   channel ADC通道号
 * @return  GPIO引脚号（如果未绑定则返回NO_PIN）
 */
pin_t adc_get_bound_pin(adc_channel_t channel);

/**
 * @brief   检查ADC通道是否已绑定GPIO引脚
 * @param   channel ADC通道号
 * @return  true表示已绑定，false表示未绑定
 */
bool adc_is_bound(adc_channel_t channel);

/*==========================================
 * 便捷宏和快捷函数
 *=========================================*/

/* 便捷宏 - 默认使用通道0 */
#define adc_init()              adc_init_single(ADC_CHANNEL_0, ADC_SAMPLE_FREQ_8MHZ, ADC_PGA_0)
#define adc_read()              adc_start_single_conversion()
#define adc_read_average(samples) adc_read_average(ADC_CHANNEL_0, (samples))
#define adc_read_voltage()      adc_read_voltage(ADC_CHANNEL_0)

/* 快速读取宏 */
#define ADC_READ(channel)               adc_start_single_conversion()
#define ADC_READ_AVG(channel, samples)  adc_read_average(channel, samples)
#define ADC_READ_VOLT(channel)          adc_read_voltage(channel)

/* 温度转换宏 */
#define adc_to_temperature_celsius(value) ((int)((int32_t)(value) - *((PUINT32)ROM_CFG_TMP_25C)) / 118)

/* 电压转换宏 - 假设3.3V参考电压，12位ADC */
#define ADC_TO_VOLTAGE(value) ((uint32_t)(value) * 3300 / 4096)

#ifdef __cplusplus
}
#endif