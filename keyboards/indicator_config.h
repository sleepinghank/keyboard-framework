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
 * @file indicator_config.h
 * @brief 指示灯产品配置
 *
 * 定义产品的 LED 硬件配置和业务别名
 * 不同产品需要根据实际硬件修改此文件
 */

#pragma once

#include "gpio.h"
#include <stdbool.h>

/* ============ LED 硬件定义 ============ */

/**
 * @brief LED 定义结构
 */
typedef struct {
    pin_t pin;          /**< GPIO 引脚 */
    bool  active_high;  /**< true=高电平亮, false=低电平亮 */
} ind_led_def_t;

/**
 * @brief LED 数量
 *
 * 根据产品实际 LED 数量修改
 */
#define IND_LED_COUNT   2

/**
 * @brief LED 硬件配置表
 *
 * 根据产品实际硬件修改引脚和极性
 * 索引顺序与业务别名对应
 */
static const ind_led_def_t ind_led_table[IND_LED_COUNT] = {
    /* 索引 0 */ {A15, true},   // 白灯：大小写 / 蓝牙状态
    /* 索引 1 */ {B23, true},   // 红灯：电源 / 电量状态
};

/* ============ 业务别名 ============ */

/* N0046 实际只有两颗指示灯 */
#define LED_WHITE       0
#define LED_RED         1

/**
 * @brief 大小写指示灯
 *
 * N0046 白灯：A15
 */
#define LED_CAPS        LED_WHITE

/**
 * @brief 电量指示灯
 *
 * N0046 红灯：B23
 */
#define LED_BAT         LED_RED

/**
 * @brief 充电指示灯
 *
 * N0046 红灯：B23
 */
#define LED_CHARGE      LED_RED

/* ============ 使用示例 ============ */

/*
 * 基础使用：
 *
 * #include "indicator.h"
 * #include "indicator_config.h"
 *
 * // 初始化
 * indicator_init();
 *
 * // 白灯常亮 2 秒
 * indicator_set(LED_WHITE, &IND_ON_2S);
 *
 * // 白灯慢闪
 * indicator_set(LED_WHITE, &IND_BLINK_SLOW);
 *
 * // 红灯快闪
 * indicator_set(LED_RED, &IND_BLINK_FAST);
 *
 * // 大小写开启（白灯）
 * indicator_set(LED_CAPS, &IND_ON);
 *
 * // 大小写关闭
 * indicator_set(LED_CAPS, &IND_OFF);
 *
 * // 低电警告，闪烁 3 次
 * indicator_set(LED_BAT, &IND_BLINK_3);
 *
 * // 充电中常亮
 * indicator_set(LED_CHARGE, &IND_ON);
 *
 * // 充满熄灭
 * indicator_set(LED_CHARGE, &IND_OFF);
 *
 * // 自定义灯效：亮 100ms 灭 900ms，闪烁 5 次
 * ind_effect_t custom = IND_BLINK_CUSTOM(100, 900, 5);
 * indicator_set(LED_BAT, &custom);
 */
