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

#include "pin_defs.h"
#include "stdint.h"

typedef uint8_t pin_t;

/* Operation of GPIO by pin. */

/*********************************************************************
 * @fn      gpio_set_pin_input
 * 
 * @brief   设置引脚为输入模式
 * 
 * @param   pin 引脚编号
 * 
 * @return  none
 */
void gpio_set_pin_input(pin_t pin);

/*********************************************************************
 * @fn      gpio_set_pin_input_high
 * 
 * @brief   设置引脚为高电平输入模式
 * 
 * @param   pin 引脚编号
 * 
 * @return  none
 */
void gpio_set_pin_input_high(pin_t pin);

/*********************************************************************
 * @fn      gpio_set_pin_input_low
 * 
 * @brief   设置引脚为低电平输入模式
 * 
 * @param   pin 引脚编号
 * 
 * @return  none
 */
void gpio_set_pin_input_low(pin_t pin);

/*********************************************************************
 * @fn      gpio_set_pin_output_push_pull
 * 
 * @brief   设置引脚为推挽输出模式
 * 
 * @param   pin 引脚编号
 * 
 * @return  none
 */
void gpio_set_pin_output_push_pull(pin_t pin);

/*********************************************************************
 * @fn      gpio_set_pin_output_open_drain
 * 
 * @brief   设置引脚为开漏输出模式
 * 
 * @param   pin 引脚编号
 * 
 * @return  none
 */
void gpio_set_pin_output_open_drain(pin_t pin);

/*********************************************************************
 * @fn      gpio_set_pin_output
 * 
 * @brief   设置引脚为输出模式(默认推挽输出)
 * 
 * @param   pin 引脚编号
 * 
 * @return  none
 */
void gpio_set_pin_output(pin_t pin);

/*********************************************************************
 * @fn      gpio_write_pin_high
 * 
 * @brief   将引脚设置为高电平
 * 
 * @param   pin 引脚编号
 * 
 * @return  none
 */
void gpio_write_pin_high(pin_t pin);

/*********************************************************************
 * @fn      gpio_write_pin_low
 * 
 * @brief   将引脚设置为低电平
 * 
 * @param   pin 引脚编号
 * 
 * @return  none
 */
void gpio_write_pin_low(pin_t pin);

/*********************************************************************
 * @fn      gpio_write_pin
 * 
 * @brief   根据电平值设置引脚状态
 * 
 * @param   pin 引脚编号
 * @param   level 电平值，1为高电平，0为低电平
 * 
 * @return  none
 */
void gpio_write_pin(pin_t pin, uint8_t level);

/*********************************************************************
 * @fn      gpio_read_pin
 * 
 * @brief   读取引脚电平状态
 * 
 * @param   pin 引脚编号
 * 
 * @return  引脚当前电平状态
 */
uint8_t gpio_read_pin(pin_t pin);

/*********************************************************************
 * @fn      gpio_toggle_pin
 * 
 * @brief   切换引脚电平状态
 * 
 * @param   pin 引脚编号
 * 
 * @return  none
 */
void gpio_toggle_pin(pin_t pin);


#define setPinInput(pin)             gpio_set_pin_input(pin)
#define setPinInputHigh(pin)         gpio_set_pin_input_high(pin)
#define setPinInputLow(pin)          gpio_set_pin_input_low(pin)
#define setPinOutputPushPull(pin)    gpio_set_pin_output_push_pull(pin)
#define setPinOutputOpenDrain(pin)   gpio_set_pin_output_open_drain(pin)
#define setPinOutput(pin)            setPinOutputPushPull(pin)

#define writePinHigh(pin)            gpio_write_pin_high(pin)
#define writePinLow(pin)             gpio_write_pin_low(pin)
#define writePin(pin, level)         gpio_write_pin(pin, level)

#define readPin(pin)                 gpio_read_pin(pin)

#define togglePin(pin)               gpio_toggle_pin(pin)

/*********************************************************************
 * GPIO 中断接口
 *********************************************************************/

/* GPIO 中断触发类型 */
typedef enum {
    GPIO_INT_FALLING = 0,   /**< 下降沿触发 */
    GPIO_INT_RISING,        /**< 上升沿触发 */
    GPIO_INT_LOW_LEVEL,     /**< 低电平触发 */
    GPIO_INT_HIGH_LEVEL     /**< 高电平触发 */
} gpio_int_trigger_t;

/* GPIO 中断回调类型 */
typedef void (*gpio_int_callback_t)(void);

/*********************************************************************
 * @fn      gpio_enable_interrupt
 *
 * @brief   启用 GPIO 中断
 *
 * @param   pin 引脚编号
 * @param   trigger 触发类型
 * @param   callback 中断回调函数
 *
 * @return  none
 */
void gpio_enable_interrupt(pin_t pin, gpio_int_trigger_t trigger, gpio_int_callback_t callback);

/*********************************************************************
 * @fn      gpio_disable_interrupt
 *
 * @brief   禁用 GPIO 中断
 *
 * @param   pin 引脚编号
 *
 * @return  none
 */
void gpio_disable_interrupt(pin_t pin);

