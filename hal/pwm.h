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

/* PWM通道枚举 - 使用按位递增的值，支持多通道组合 */
typedef enum {
    PWM_CHANNEL_0  = (1 << 0),  /* 通道0: 1 */
    PWM_CHANNEL_1  = (1 << 1),  /* 通道1: 2 */
    PWM_CHANNEL_2  = (1 << 2),  /* 通道2: 4 */
    PWM_CHANNEL_3  = (1 << 3),  /* 通道3: 8 */
    PWM_CHANNEL_4  = (1 << 4),  /* 通道4: 16 */
    PWM_CHANNEL_5  = (1 << 5),  /* 通道5: 32 */
    PWM_CHANNEL_6  = (1 << 6),  /* 通道6: 64 */
    PWM_CHANNEL_7  = (1 << 7),  /* 通道7: 128 */
    PWM_CHANNEL_ALL = 0xFF       /* 所有通道 */
} pwm_channel_t;

/* PWM操作函数 */

/*********************************************************************
 * @fn      pwm_init
 *
 * @brief   初始化PWM模块
 *
 * @param   none
 *
 * @return  none
 */
void pwm_init(void);

/*********************************************************************
 * @fn      pwm_start
 *
 * @brief   启动PWM输出
 *
 * @param   channel PWM通道号（可使用按位OR组合多个通道，如 PWM_CHANNEL_0 | PWM_CHANNEL_1）
 *
 * @return  none
 */
void pwm_start(pwm_channel_t channel);

/*********************************************************************
 * @fn      pwm_stop
 *
 * @brief   停止PWM输出
 *
 * @param   channel PWM通道号（可使用按位OR组合多个通道）
 *
 * @return  none
 */
void pwm_stop(pwm_channel_t channel);

/*********************************************************************
 * @fn      pwm_set_frequency
 *
 * @brief   设置PWM频率
 *
 * @param   channel PWM通道号（可使用按位OR组合多个通道）
 * @param   frequency PWM频率(Hz)
 *
 * @return  none
 */
void pwm_set_frequency(pwm_channel_t channel, uint32_t frequency);

/*********************************************************************
 * @fn      pwm_set_duty_cycle
 *
 * @brief   设置PWM占空比
 *
 * @param   channel PWM通道号（可使用按位OR组合多个通道）
 * @param   duty_cycle 占空比(0-100)
 *
 * @return  none
 */
void pwm_set_duty_cycle(pwm_channel_t channel, uint8_t duty_cycle);

/*********************************************************************
 * @fn      pwm_set_duty_percentage
 *
 * @brief   设置PWM占空比百分比
 *
 * @param   channel PWM通道号（可使用按位OR组合多个通道）
 * @param   percentage 占空比百分比(0.0-100.0)
 *
 * @return  none
 */
void pwm_set_duty_percentage(pwm_channel_t channel, float percentage);

/*********************************************************************
 * @fn      pwm_get_duty_cycle
 *
 * @brief   获取PWM占空比
 *
 * @param   channel PWM通道号
 *
 * @return  占空比(0-100)
 */
uint8_t pwm_get_duty_cycle(pwm_channel_t channel);

/*********************************************************************
 * @fn      pwm_get_duty_percentage
 *
 * @brief   获取PWM占空比百分比
 *
 * @param   channel PWM通道号
 *
 * @return  占空比百分比(0.0-100.0)
 */
float pwm_get_duty_percentage(pwm_channel_t channel);

/*********************************************************************
 * @fn      pwm_set_pulse_width
 *
 * @brief   设置PWM脉冲宽度
 *
 * @param   channel PWM通道号（可使用按位OR组合多个通道）
 * @param   pulse_width_ns 脉冲宽度(纳秒)
 *
 * @return  none
 */
void pwm_set_pulse_width(pwm_channel_t channel, uint32_t pulse_width_ns);

/*********************************************************************
 * @fn      pwm_set_period
 *
 * @brief   设置PWM周期
 *
 * @param   channel PWM通道号（可使用按位OR组合多个通道）
 * @param   period_ns PWM周期(纳秒)
 *
 * @return  none
 */
void pwm_set_period(pwm_channel_t channel, uint32_t period_ns);

/*********************************************************************
 * @fn      pwm_set_polarity
 *
 * @brief   设置PWM极性
 *
 * @param   channel PWM通道号（可使用按位OR组合多个通道）
 * @param   polarity 极性(0:正常, 1:反向)
 *
 * @return  none
 */
void pwm_set_polarity(pwm_channel_t channel, uint8_t polarity);

/*********************************************************************
 * @fn      pwm_enable
 *
 * @brief   使能PWM
 *
 * @param   channel PWM通道号（可使用按位OR组合多个通道）
 *
 * @return  none
 */
void pwm_enable(pwm_channel_t channel);

/*********************************************************************
 * @fn      pwm_disable
 *
 * @brief   禁用PWM
 *
 * @param   channel PWM通道号（可使用按位OR组合多个通道）
 *
 * @return  none
 */
void pwm_disable(pwm_channel_t channel);

/*********************************************************************
 * @fn      pwm_configure
 *
 * @brief   配置PWM通道
 *
 * @param   channel PWM通道号（可使用按位OR组合多个通道）
 * @param   frequency PWM频率(Hz)
 * @param   duty_cycle 占空比(0-100)
 *
 * @return  none
 */
void pwm_configure(pwm_channel_t channel, uint32_t frequency, uint8_t duty_cycle);

/*==========================================
 * GPIO引脚绑定到PWM通道
 *=========================================*/

/*********************************************************************
 * @fn      pwm_bind_pin
 *
 * @brief   绑定GPIO引脚到PWM通道
 * @param   pin GPIO引脚号
 * @param   channel PWM通道号（可使用按位OR组合多个通道）
 * @return  bool 绑定是否成功
 * @note    此函数将GPIO引脚绑定到指定PWM通道，后续操作直接使用通道号
 */
bool pwm_bind_pin(pin_t pin, pwm_channel_t channel);

/*********************************************************************
 * @fn      pwm_unbind_pin
 *
 * @brief   取消GPIO引脚与PWM通道的绑定
 * @param   pin GPIO引脚号
 * @return  bool 取消绑定是否成功
 */
bool pwm_unbind_pin(pin_t pin);

/*********************************************************************
 * @fn      pwm_get_bound_pin
 *
 * @brief   获取PWM通道绑定的GPIO引脚
 * @param   channel PWM通道号
 * @return  GPIO引脚号（如果未绑定则返回NO_PIN）
 */
pin_t pwm_get_bound_pin(pwm_channel_t channel);

/*********************************************************************
 * @fn      pwm_is_bound
 *
 * @brief   检查PWM通道是否已绑定GPIO引脚
 * @param   channel PWM通道号
 * @return  true表示已绑定，false表示未绑定
 */
bool pwm_is_bound(pwm_channel_t channel);

/*********************************************************************
 * @fn      pwm_bind_enable_pin
 *
 * @brief   绑定PWM使能引脚到通道
 * @param   enable_pin 使能引脚号
 * @param   channel PWM通道号
 * @return  bool 绑定是否成功
 * @note    可选功能，如果PWM通道有独立的使能引脚，可通过此函数绑定
 */
bool pwm_bind_enable_pin(pin_t enable_pin, pwm_channel_t channel);

/*********************************************************************
 * @fn      pwm_get_bound_enable_pin
 *
 * @brief   获取PWM通道绑定的使能引脚
 * @param   channel PWM通道号
 * @return  使能引脚号（如果未绑定则返回NO_PIN）
 */
pin_t pwm_get_bound_enable_pin(pwm_channel_t channel);

/* 便捷宏定义 */

#define pwm_set_level(channel, level)        pwm_set_duty_cycle((channel), (level) ? 100 : 0)
#define pwm_set_brightness(channel, brightness) pwm_set_duty_cycle((channel), (brightness))
#define pwm_get_brightness(channel)          pwm_get_duty_cycle(channel)

#define pwm_led_on(channel)                  pwm_set_level((channel), 1)
#define pwm_led_off(channel)                 pwm_set_level((channel), 0)
#define pwm_led_toggle(channel)              pwm_set_level((channel), !pwm_get_duty_cycle(channel))
#define pwm_led_set(channel, val)            pwm_set_brightness((channel), (val))
#define pwm_led_get(channel)                 pwm_get_brightness(channel)
