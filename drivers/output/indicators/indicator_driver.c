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
 * @file indicator_driver.c
 * @brief 指示灯驱动层实现
 *
 * 实现状态机引擎和各模式处理逻辑
 */

#include "indicator_driver.h"
#include "indicator_hal.h"
#include "timer.h"
#include "gpio.h"
#include <string.h>

/* ========== LED实例结构定义 ========== */

/**
 * @brief LED实例结构
 *
 * 驱动层需要访问完整结构，在此定义
 */
struct indicator_t {
    // 硬件配置
    struct {
        pin_t  pin;          /**< GPIO引脚 */
        bool   active_high;  /**< true=高电平亮, false=低电平亮 */
    } hw;

    // 运行时状态（驱动层使用）
    struct {
        ind_state_t state;    /**< 当前状态 */
        uint32_t   timer;     /**< 定时器 */
        uint32_t   elapsed;   /**< 已运行时间 */
        uint16_t   count;     /**< 重复计数 */
        bool       is_on;     /**< 当前是否点亮 */
    } runtime;

    // 当前配置
    ind_config_t config;
};

/* ========== 模式处理函数 ========== */

/**
 * @brief 模式处理函数类型
 */
typedef void (*mode_handler_t)(indicator_t* ind, uint32_t delta);

/**
 * @brief 辅助函数：根据active_high设置GPIO状态
 */
static inline void set_led_state(indicator_t* ind, bool is_on) {
    bool level = is_on ? ind->hw.active_high : !ind->hw.active_high;
    ind_hal_set(ind->hw.pin, level);
    ind->runtime.is_on = is_on;
}

/**
 * @brief OFF模式：熄灭
 */
static void mode_off_handler(indicator_t* ind, uint32_t delta) {
    (void)delta;  // 未使用
    set_led_state(ind, false);
}

/**
 * @brief ON模式：常亮
 */
static void mode_on_handler(indicator_t* ind, uint32_t delta) {
    (void)delta;  // 未使用
    set_led_state(ind, true);

    // 检查持续时间
    if (ind->config.duration > 0) {
        if (ind->runtime.elapsed >= ind->config.duration) {
            ind_driver_stop(ind);  // 时间到，停止
        }
    }
}

/**
 * @brief BLINK模式：闪烁
 */
static void mode_blink_handler(indicator_t* ind, uint32_t delta) {
    uint32_t period = ind->runtime.is_on ? ind->config.off_time : ind->config.on_time;

    if (ind->runtime.timer >= period) {
        // 切换状态
        ind->runtime.is_on = !ind->runtime.is_on;
        set_led_state(ind, ind->runtime.is_on);
        ind->runtime.timer = 0;

        // 如果从亮切换到灭，计数一次
        if (!ind->runtime.is_on) {
            ind->runtime.count++;
            if (ind->config.repeat > 0 && ind->runtime.count >= ind->config.repeat) {
                ind_driver_stop(ind);
            }
        }
    }
}

/**
 * @brief HEARTBEAT模式：心跳
 *
 * 心跳效果：短亮 - 短灭 - 长灭 - 循环
 */
static void mode_heartbeat_handler(indicator_t* ind, uint32_t delta) {
    (void)delta;  // 未使用

    // 心跳：短亮 - 短灭，然后根据repeat决定是否继续
    if (ind->runtime.is_on) {
        // 亮状态
        if (ind->runtime.timer >= ind->config.on_time) {
            // 切换到灭
            ind->runtime.is_on = false;
            set_led_state(ind, false);
            ind->runtime.timer = 0;
        }
    } else {
        // 灭状态
        if (ind->runtime.timer >= ind->config.off_time) {
            // 完成一次短灭，检查是否需要继续
            if (ind->config.repeat > 0 && ind->runtime.count >= ind->config.repeat) {
                // 完成所有心跳
                ind_driver_stop(ind);
            } else {
                // 下一次心跳开始
                ind->runtime.count++;
                ind->runtime.is_on = true;
                set_led_state(ind, true);
                ind->runtime.timer = 0;
            }
        }
    }
}

/**
 * @brief 模式处理表
 */
static const mode_handler_t mode_handlers[] = {
    [IND_MODE_OFF]      = mode_off_handler,
    [IND_MODE_ON]       = mode_on_handler,
    [IND_MODE_BLINK]    = mode_blink_handler,
    [IND_MODE_HEARTBEAT]= mode_heartbeat_handler,
};

/* ========== 驱动层API实现 ========== */

void ind_driver_init(void) {
    // 驱动层初始化（预留）
}

void ind_driver_start(indicator_t* ind, const ind_config_t* config) {
    if (ind == NULL || config == NULL) return;

    // 保存配置
    ind->config = *config;

    // 重置状态
    ind->runtime.state   = IND_STATE_RUNNING;
    ind->runtime.timer   = 0;
    ind->runtime.elapsed = 0;
    ind->runtime.count   = 0;
    ind->runtime.is_on   = false;

    // 立即执行一次模式处理
    if (mode_handlers[config->mode] != NULL) {
        mode_handlers[config->mode](ind, 0);
    }

    // 记录起始时间
    ind->runtime.timer = timer_read32();
}

void ind_driver_stop(indicator_t* ind) {
    if (ind == NULL) return;

    ind->runtime.state = IND_STATE_IDLE;
    set_led_state(ind, false);  // 确保LED熄灭
}

bool ind_driver_task(indicator_t* ind) {
    if (ind == NULL || ind->runtime.state != IND_STATE_RUNNING) {
        return false;
    }

    // 计算时间差
    uint32_t now = timer_read32();
    uint32_t delta = timer_elapsed32(ind->runtime.timer);
    ind->runtime.timer = now;
    ind->runtime.elapsed += delta;

    // 调用模式处理函数
    if (mode_handlers[ind->config.mode] != NULL) {
        mode_handlers[ind->config.mode](ind, delta);
    }

    // 检查总持续时间
    if (ind->config.duration > 0 && ind->runtime.elapsed >= ind->config.duration) {
        ind_driver_stop(ind);
        return false;
    }

    return (ind->runtime.state == IND_STATE_RUNNING);
}
