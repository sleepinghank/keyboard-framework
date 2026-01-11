/* Copyright 2025 @ keyboard-framework
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

/**
 * @file indicator.h
 * @brief 指示灯应用层API接口
 *
 * 提供简洁的LED控制接口，业务层通过此接口控制LED
 */

#pragma once

#include "indicator_driver.h"
#include "gpio.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief LED实例结构
 *
 * 业务层定义此结构的实例来控制LED
 */
typedef struct indicator_t indicator_t;

/* ========== 初始化 ========== */

/**
 * @brief 初始化LED实例
 * @param ind LED实例指针
 * @param pin GPIO引脚
 * @param active_high true=高电平亮, false=低电平亮
 * @return 0=成功, <0=失败
 *
 * @example
 * indicator_t bt_led;
 * indicator_init(&bt_led, PIN_A1, true);
 */
int indicator_init(indicator_t* ind, pin_t pin, bool active_high);

/**
 * @brief 反初始化LED实例
 * @param ind LED实例指针
 *
 * 停止指示并释放GPIO资源
 */
void indicator_deinit(indicator_t* ind);

/* ========== 控制 ========== */

/**
 * @brief 启动LED指示
 * @param ind LED实例指针
 * @param config 配置参数指针
 * @return 0=成功, <0=失败
 *
 * @example
 * ind_config_t cfg = {IND_MODE_BLINK, 500, 500, 0, 0};
 * indicator_start(&bt_led, &cfg);
 */
int indicator_start(indicator_t* ind, const ind_config_t* config);

/**
 * @brief 停止LED指示
 * @param ind LED实例指针
 *
 * 立即停止指示并熄灭LED
 */
void indicator_stop(indicator_t* ind);

/**
 * @brief 更新配置
 * @param ind LED实例指针
 * @param config 新配置参数指针
 * @return 0=成功, <0=失败
 *
 * 停止当前指示并启动新配置
 */
int indicator_update(indicator_t* ind, const ind_config_t* config);

/* ========== 状态查询 ========== */

/**
 * @brief 检查LED是否正在运行
 * @param ind LED实例指针
 * @return true=运行中, false=空闲
 */
bool indicator_is_running(const indicator_t* ind);

/* ========== 任务 ========== */

/**
 * @brief LED任务函数
 * @param ind LED实例指针
 * @return true=LED仍在运行, false=LED已完成
 *
 * 需要在主循环中定期调用
 *
 * @example
 * while (1) {
 *     indicator_task(&bt_led);
 *     // ... 其他任务
 * }
 */
bool indicator_task(indicator_t* ind);

/* ========== 常用配置宏 ========== */

/**
 * @brief 常用配置宏定义
 *
 * @example
 * indicator_start(&led, &(ind_config_t)IND_BLINK_SLOW);
 */

#define IND_OFF          {IND_MODE_OFF, 0, 0, 0, 0}
#define IND_ON_FOREVER   {IND_MODE_ON, 0, 0, 0, 0}
#define IND_ON_1S        {IND_MODE_ON, 0, 0, 1000, 0}
#define IND_ON_2S        {IND_MODE_ON, 0, 0, 2000, 0}
#define IND_BLINK_SLOW   {IND_MODE_BLINK, 500, 500, 0, 0}
#define IND_BLINK_FAST   {IND_MODE_BLINK, 100, 100, 0, 0}
#define IND_BLINK_3_TIMES {IND_MODE_BLINK, 500, 500, 0, 3}
#define IND_HEARTBEAT    {IND_MODE_HEARTBEAT, 200, 200, 0, 0}
