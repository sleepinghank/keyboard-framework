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
 * @brief 指示灯驱动接口
 *
 * 提供简洁的 LED 控制接口，支持多种灯效模式
 * 使用 OSAL 事件驱动，无需主循环轮询
 *
 * @example
 * // 初始化
 * indicator_init();
 *
 * // 设置灯效
 * indicator_set(LED_BT1, &IND_BLINK_SLOW);
 *
 * // 关闭
 * indicator_off(LED_BT1);
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

/* ============ 类型定义 ============ */

/**
 * @brief 灯效模式
 */
typedef enum {
    IND_MODE_OFF = 0,   /**< 熄灭 */
    IND_MODE_ON,        /**< 常亮 */
    IND_MODE_BLINK,     /**< 闪烁 */
} ind_mode_t;

/**
 * @brief 灯效配置
 */
typedef struct {
    ind_mode_t mode;        /**< 灯效模式 */
    uint16_t   on_ms;       /**< 亮持续时间 (ms) */
    uint16_t   off_ms;      /**< 灭持续时间 (ms) */
    uint16_t   delay_ms;    /**< 延迟启动时间 (ms), 0=立即 */
    uint16_t   duration_ms; /**< 总持续时间 (ms), 0=由repeat决定 */
    uint8_t    repeat;      /**< 重复次数, 0=无限 */
} ind_effect_t;

/**
 * @brief 低功耗回调函数类型
 * @param all_off true=所有灯已熄灭, false=有灯在运行
 *
 * 当所有灯都熄灭时调用此回调，用于通知系统可进入低功耗
 */
typedef void (*ind_lpm_callback_t)(bool all_off);

/* ============ 核心接口 ============ */

/**
 * @brief 初始化指示灯模块
 *
 * 初始化硬件和 OSAL 任务，必须在使用其他接口前调用
 */
void indicator_init(void);

/**
 * @brief 反初始化指示灯模块
 *
 * 停止所有灯效，释放资源
 */
void indicator_deinit(void);

/**
 * @brief 设置 LED 灯效
 * @param led_id LED 索引（使用别名如 LED_BT1）
 * @param effect 灯效配置指针
 *
 * 后设置的灯效会覆盖前面的
 *
 * @example
 * indicator_set(LED_BT1, &IND_BLINK_SLOW);
 */
void indicator_set(uint8_t led_id, const ind_effect_t* effect);

/**
 * @brief 关闭指定 LED
 * @param led_id LED 索引
 */
void indicator_off(uint8_t led_id);

/**
 * @brief 关闭所有 LED
 */
void indicator_off_all(void);

/**
 * @brief 检查指定 LED 是否在运行
 * @param led_id LED 索引
 * @return true=运行中, false=空闲
 */
bool indicator_is_active(uint8_t led_id);

/**
 * @brief 检查是否有任意 LED 在运行
 * @return true=有灯在运行, false=全部熄灭
 */
bool indicator_any_active(void);

/**
 * @brief 注册低功耗回调
 * @param callback 回调函数
 *
 * 当所有灯都熄灭时会调用此回调
 */
void indicator_set_lpm_callback(ind_lpm_callback_t callback);

/**
 * @brief 指示灯任务函数
 *
 * 需要在主循环中定期调用，用于驱动状态机
 *
 * @example
 * while (1) {
 *     indicator_task();
 *     // ... 其他任务
 * }
 */
void indicator_task(void);

/* ============ 预定义灯效 ============ */

/** 熄灭 */
extern const ind_effect_t IND_OFF;

/** 常亮（无限） */
extern const ind_effect_t IND_ON;

/** 常亮 1 秒后熄灭 */
extern const ind_effect_t IND_ON_1S;

/** 常亮 2 秒后熄灭 */
extern const ind_effect_t IND_ON_2S;

/** 常亮 3 秒后熄灭 */
extern const ind_effect_t IND_ON_3S;

/** 慢闪 (亮 1s + 灭 1s) */
extern const ind_effect_t IND_BLINK_SLOW;

/** 快闪 (亮 500ms + 灭 500ms) */
extern const ind_effect_t IND_BLINK_FAST;

/** 闪烁 1 次 */
extern const ind_effect_t IND_BLINK_1;

/** 闪烁 2 次 */
extern const ind_effect_t IND_BLINK_2;

/** 闪烁 3 次 */
extern const ind_effect_t IND_BLINK_3;

/** 延迟 500ms 后常亮 */
extern const ind_effect_t IND_DELAY_ON;

/** 延迟 500ms 后闪烁 */
extern const ind_effect_t IND_DELAY_BLINK;

/* ============ 便捷宏 ============ */

/**
 * @brief 创建自定义闪烁灯效
 * @param on_time 亮时间 (ms)
 * @param off_time 灭时间 (ms)
 * @param cnt 闪烁次数, 0=无限
 */
#define IND_BLINK_CUSTOM(on_time, off_time, cnt) \
    ((ind_effect_t){IND_MODE_BLINK, (on_time), (off_time), 0, 0, (cnt)})

/**
 * @brief 创建延迟亮灯效
 * @param delay 延迟时间 (ms)
 * @param duration 持续时间 (ms), 0=无限
 */
#define IND_DELAY_ON_CUSTOM(delay, duration) \
    ((ind_effect_t){IND_MODE_ON, 0, 0, (delay), (duration), 0})
