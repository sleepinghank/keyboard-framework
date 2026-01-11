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
 * @file indicator_driver.h
 * @brief 指示灯驱动层
 *
 * 提供状态机引擎和灯效处理逻辑
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief LED模式
 */
typedef enum {
    IND_MODE_OFF      = 0,    /**< 熄灭 */
    IND_MODE_ON,              /**< 常亮 */
    IND_MODE_BLINK,           /**< 闪烁 */
    IND_MODE_HEARTBEAT,       /**< 心跳（短亮短灭，周期性） */
} ind_mode_t;

/**
 * @brief LED状态（内部使用）
 */
typedef enum {
    IND_STATE_IDLE     = 0,   /**< 空闲 */
    IND_STATE_RUNNING,        /**< 运行中 */
} ind_state_t;

/**
 * @brief LED配置参数
 */
typedef struct {
    ind_mode_t mode;         /**< 模式 */
    uint32_t on_time;        /**< 亮持续时间 (ms) */
    uint32_t off_time;       /**< 灭持续时间 (ms) */
    uint32_t duration;       /**< 总持续时间 (ms), 0=无限 */
    uint16_t repeat;         /**< 重复次数, 0=无限 */
} ind_config_t;

/**
 * @brief LED实例（驱动层使用的前置声明）
 */
typedef struct indicator_t indicator_t;

/**
 * @brief 驱动层初始化
 */
void ind_driver_init(void);

/**
 * @brief 启动LED指示
 * @param ind LED实例
 * @param config 配置参数
 */
void ind_driver_start(indicator_t* ind, const ind_config_t* config);

/**
 * @brief 停止LED指示
 * @param ind LED实例
 */
void ind_driver_stop(indicator_t* ind);

/**
 * @brief LED任务函数（主循环调用）
 * @param ind LED实例
 * @return true=LED仍在运行, false=LED已完成
 */
bool ind_driver_task(indicator_t* ind);
