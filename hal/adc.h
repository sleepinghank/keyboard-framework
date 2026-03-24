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
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/*==========================================
 * ADC通道定义（平台无关）
 * 支持6个ADC通道，具体引脚映射由平台实现定义
 *=========================================*/
typedef enum {
    ADC_CHANNEL_0 = 0,   /**< ADC通道0 */
    ADC_CHANNEL_1 = 1,   /**< ADC通道1 */
    ADC_CHANNEL_2 = 2,   /**< ADC通道2 */
    ADC_CHANNEL_3 = 3,   /**< ADC通道3 */
    ADC_CHANNEL_4 = 4,   /**< ADC通道4 */
    ADC_CHANNEL_5 = 5,   /**< ADC通道5 */
    ADC_CHANNEL_MAX = 6  /**< ADC通道最大数量 */
} adc_channel_t;

/*==========================================
 * ADC采样模式定义
 *=========================================*/
typedef enum {
    ADC_MODE_SINGLE = 0,    /**< 单次采样模式：每次读取返回单次采样值 */
    ADC_MODE_AVERAGE = 1,   /**< 平均采样模式：多次采样后返回平均值 */
} adc_mode_t;

/*==========================================
 * ADC回调函数类型定义
 * 用于中断模式，当采样完成或达到阈值时调用
 * @param value ADC采样原始值（12位，0-4095）
 *=========================================*/
typedef void (*adc_callback_t)(uint16_t value);

/*==========================================
 * ADC基础初始化
 * @brief 初始化ADC软件结构体模块
 * 调用其他ADC函数前必须先调用此函数
 * @note 此函数只初始化软件结构体，不配置具体硬件
 * @return none
 *=========================================*/
void adc_init(void);

/*==========================================
 * ADC通道初始化
 * @brief 配置指定ADC通道的采样模式和平均次数
 * @param channel ADC通道号 (0-5)
 * @param mode 采样模式：ADC_MODE_SINGLE 或 ADC_MODE_AVERAGE
 * @param avg_samples 平均采样次数 (1-255，仅在ADC_MODE_AVERAGE模式下有效)
 * @return none
 * @note 初始化后通道处于工作状态，可直接进行采样
 *=========================================*/
void adc_init_channel(adc_channel_t channel, adc_mode_t mode, uint8_t avg_samples);

/*==========================================
 * GPIO引脚绑定功能
 * 一个ADC通道可以绑定多个GPIO引脚
 * 采样时会按绑定顺序依次读取所有绑定到该通道的引脚
 *=========================================*/

/**
 * @brief 将GPIO引脚绑定到ADC通道
 * @param pin GPIO引脚号（由gpio.h定义）
 * @param channel 目标ADC通道号
 * @return bool 绑定成功返回true，失败返回false（通道不存在或引脚无效）
 * @note 同一引脚只能绑定到一个通道，但一个通道可以绑定多个引脚
 * @note 绑定后的引脚会自动配置为模拟输入模式
 */
bool adc_bind_pin(pin_t pin, adc_channel_t channel);

/**
 * @brief 解除GPIO引脚与ADC通道的绑定
 * @param pin 要解除绑定的GPIO引脚号
 * @return bool 解除成功返回true，失败返回false（引脚未绑定或无效）
 * @note 解除绑定后，引脚恢复为浮空状态
 */
bool adc_unbind_pin(pin_t pin);

/**
 * @brief 获取ADC通道绑定的第一个GPIO引脚
 * @param channel ADC通道号
 * @return pin_t 绑定的GPIO引脚号，如果未绑定则返回NO_PIN
 * @note 由于通道可绑定多个引脚，此函数返回第一个绑定的引脚
 * @note 如需获取所有绑定引脚，请使用adc_is_bound()遍历
 */
pin_t adc_get_bound_pin(adc_channel_t channel);

/**
 * @brief 检查ADC通道是否已绑定GPIO引脚
 * @param channel ADC通道号
 * @return bool 已绑定返回true，未绑定返回false
 * @note 用于快速检查通道是否可用
 */
bool adc_is_bound(adc_channel_t channel);

/*==========================================
 * ADC采样功能
 *=========================================*/

/**
 * @brief 单次ADC采样
 * @param channel ADC通道号
 * @return uint16_t ADC采样原始值（12位，0-4095范围）
 * @note 此函数会阻塞直到采样完成
 * @note 返回值为ADC原始数字量，需使用adc_to_voltage()转换为电压
 * @warning 通道必须已通过adc_init_channel()初始化
 */
uint16_t adc_read_single(adc_channel_t channel);

/**
 * @brief 平均ADC采样
 * @param channel ADC通道号
 * @param samples 采样次数 (1-255)
 * @return uint16_t 平均采样原始值
 * @note 此函数会执行指定次数的采样并返回平均值
 * @note 采样次数越多，结果越稳定，但耗时越长
 * @note 如果通道初始化时使用了ADC_MODE_AVERAGE，此函数会叠加采样
 * @warning samples为0时行为未定义
 */
uint16_t adc_read_average(adc_channel_t channel, uint8_t samples);

/**
 * @brief 将ADC原始值转换为电压值
 * @param raw_value ADC原始采样值（0-4095）
 * @param vref_mv 参考电压（毫伏），通常为3300（3.3V）或5000（5.0V）
 * @return uint32_t 转换后的电压值（毫伏）
 * @note 公式：voltage = (raw_value * vref_mv) / 4096
 * @note 例如：raw_value=2048, vref_mv=3300时，返回1650mV (1.65V)
 */
uint32_t adc_to_voltage(uint16_t raw_value, uint32_t vref_mv);

/*==========================================
 * ADC中断控制功能
 *=========================================*/

/**
 * @brief 启用ADC通道中断
 * @param channel ADC通道号
 * @param threshold 中断触发阈值（0-4095）
 * @return none
 * @note 当采样值大于等于阈值时触发中断，调用注册的回调函数
 * @note 使用前必须通过adc_set_callback()注册回调函数
 * @note 启用中断后，ADC会持续监控该通道，无需手动调用读取函数
 * @note 中断为电平触发，不是边沿触发
 */
void adc_enable_interrupt(adc_channel_t channel, uint16_t threshold);

/**
 * @brief 禁用ADC通道中断
 * @param channel ADC通道号
 * @return none
 * @note 禁用后，该通道不再产生中断，但仍可手动采样
 * @note 不会影响其他通道的中断设置
 */
void adc_disable_interrupt(adc_channel_t channel);

/**
 * @brief 注册ADC回调函数
 * @param callback 回调函数指针，设置为NULL可取消回调
 * @return none
 * @note 回调函数在ADC中断服务程序中调用
 * @note 回调函数应避免执行耗时操作，建议仅设置标志位或写入缓冲区
 * @note 同一时间只能注册一个回调函数，新注册会覆盖之前的
 * @note 回调函数原型：void callback(uint16_t value);
 */
void adc_set_callback(adc_callback_t callback);

/*==========================================
 * ADC启停控制功能
 *=========================================*/

/**
 * @brief 停止指定ADC通道
 * @param channel ADC通道号
 * @return none
 * @note 停止后通道不再进行采样，节省功耗
 * @note 停止操作不会影响通道配置（绑定、GPIO等）
 * @note 可通过adc_resume()恢复
 * @note 如果通道启用了中断，停止后中断也会被暂停
 */
void adc_stop(adc_channel_t channel);

/**
 * @brief 恢复指定ADC通道
 * @param channel ADC通道号
 * @return none
 * @note 恢复之前通过adc_stop()停止的通道
 * @note 恢复后通道回到停止前的状态（采样模式、中断设置等）
 * @note 如果之前启用了中断，恢复后中断会自动重新启用
 * @note 如果通道未停止，此函数无效果
 */
void adc_resume(adc_channel_t channel);

/*==========================================
 * 便捷宏定义
 *=========================================*/

/**
 * @brief 快速采样宏（默认通道0，单次采样）
 * @return uint16_t 采样值
 * @note 使用前需确保通道0已初始化
 * @deprecated 建议直接使用adc_read_single()以提高代码可读性
 */
#define adc_read() adc_read_single(ADC_CHANNEL_0)

/**
 * @brief 快速平均采样宏（默认通道0）
 * @param samples 采样次数
 * @return uint16_t 平均采样值
 * @note 使用前需确保通道0已初始化
 * @deprecated 建议直接使用adc_read_average()以提高代码可读性
 */
#define adc_read_avg(samples) adc_read_average(ADC_CHANNEL_0, (samples))

/**
 * @brief 快速电压读取宏（默认通道0）
 * @param vref 参考电压（毫伏）
 * @return uint32_t 电压值（毫伏）
 * @note 内部调用adc_read_single()然后转换
 * @deprecated 建议分步调用以提高可读性和灵活性
 */
#define adc_read_voltage(vref) adc_to_voltage(adc_read_single(ADC_CHANNEL_0), (vref))

/**
 * @brief ADC值转电压宏（3.3V参考）
 * @param value ADC原始值
 * @return uint32_t 电压值（毫伏）
 * @note 假设参考电压为3.3V (3300mV)
 * @note 公式：(value * 3300) / 4096
 */
#define ADC_TO_VOLTAGE_3V3(value) ((uint32_t)(value) * 3300 / 4096)

/**
 * @brief ADC值转电压宏（5.0V参考）
 * @param value ADC原始值
 * @return uint32_t 电压值（毫伏）
 * @note 假设参考电压为5.0V (5000mV)
 * @note 公式：(value * 5000) / 4096
 */
#define ADC_TO_VOLTAGE_5V0(value) ((uint32_t)(value) * 5000 / 4096)

#ifdef __cplusplus
}
#endif
