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
 * @param   channel PWM通道号
 *
 * @return  none
 */
void pwm_start(uint8_t channel);

/*********************************************************************
 * @fn      pwm_stop
 *
 * @brief   停止PWM输出
 *
 * @param   channel PWM通道号
 *
 * @return  none
 */
void pwm_stop(uint8_t channel);

/*********************************************************************
 * @fn      pwm_set_frequency
 *
 * @brief   设置PWM频率
 *
 * @param   channel PWM通道号
 * @param   frequency PWM频率(Hz)
 *
 * @return  none
 */
void pwm_set_frequency(uint8_t channel, uint32_t frequency);

/*********************************************************************
 * @fn      pwm_set_duty_cycle
 *
 * @brief   设置PWM占空比
 *
 * @param   channel PWM通道号
 * @param   duty_cycle 占空比(0-100)
 *
 * @return  none
 */
void pwm_set_duty_cycle(uint8_t channel, uint8_t duty_cycle);

/*********************************************************************
 * @fn      pwm_set_duty_percentage
 *
 * @brief   设置PWM占空比百分比
 *
 * @param   channel PWM通道号
 * @param   percentage 占空比百分比(0.0-100.0)
 *
 * @return  none
 */
void pwm_set_duty_percentage(uint8_t channel, float percentage);

/*********************************************************************
 * @fn      pwm_get_duty_cycle
 *
 * @brief   获取PWM占空比
 *
 * @param   channel PWM通道号
 *
 * @return  占空比(0-100)
 */
uint8_t pwm_get_duty_cycle(uint8_t channel);

/*********************************************************************
 * @fn      pwm_get_duty_percentage
 *
 * @brief   获取PWM占空比百分比
 *
 * @param   channel PWM通道号
 *
 * @return  占空比百分比(0.0-100.0)
 */
float pwm_get_duty_percentage(uint8_t channel);

/*********************************************************************
 * @fn      pwm_set_pulse_width
 *
 * @brief   设置PWM脉冲宽度
 *
 * @param   channel PWM通道号
 * @param   pulse_width_ns 脉冲宽度(纳秒)
 *
 * @return  none
 */
void pwm_set_pulse_width(uint8_t channel, uint32_t pulse_width_ns);

/*********************************************************************
 * @fn      pwm_set_period
 *
 * @brief   设置PWM周期
 *
 * @param   channel PWM通道号
 * @param   period_ns PWM周期(纳秒)
 *
 * @return  none
 */
void pwm_set_period(uint8_t channel, uint32_t period_ns);

/*********************************************************************
 * @fn      pwm_set_polarity
 *
 * @brief   设置PWM极性
 *
 * @param   channel PWM通道号
 * @param   polarity 极性(0:正常, 1:反向)
 *
 * @return  none
 */
void pwm_set_polarity(uint8_t channel, uint8_t polarity);

/*********************************************************************
 * @fn      pwm_enable
 *
 * @brief   使能PWM
 *
 * @param   channel PWM通道号
 *
 * @return  none
 */
void pwm_enable(uint8_t channel);

/*********************************************************************
 * @fn      pwm_disable
 *
 * @brief   禁用PWM
 *
 * @param   channel PWM通道号
 *
 * @return  none
 */
void pwm_disable(uint8_t channel);

/*********************************************************************
 * @fn      pwm_configure
 *
 * @brief   配置PWM通道
 *
 * @param   channel PWM通道号
 * @param   frequency PWM频率(Hz)
 * @param   duty_cycle 占空比(0-100)
 *
 * @return  none
 */
void pwm_configure(uint8_t channel, uint32_t frequency, uint8_t duty_cycle);

/* 便捷宏定义 */

#define pwm_set_level(channel, level)        pwm_set_duty_cycle((channel), (level) ? 100 : 0)
#define pwm_set_brightness(channel, brightness) pwm_set_duty_cycle((channel), (brightness))
#define pwm_get_brightness(channel)          pwm_get_duty_cycle(channel)

#define pwm_led_on(channel)                  pwm_set_level((channel), 1)
#define pwm_led_off(channel)                 pwm_set_level((channel), 0)
#define pwm_led_toggle(channel)              pwm_set_level((channel), !pwm_get_level(channel))
#define pwm_led_set(channel, val)            pwm_set_brightness((channel), (val))
#define pwm_led_get(channel)                 pwm_get_brightness(channel)
