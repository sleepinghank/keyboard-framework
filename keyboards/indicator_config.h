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
#define IND_LED_COUNT   5

/**
 * @brief LED 硬件配置表
 *
 * 根据产品实际硬件修改引脚和极性
 * 索引顺序与业务别名对应
 */
static const ind_led_def_t ind_led_table[IND_LED_COUNT] = {
    /* 索引 0 */ {0, true },   // BT1 / CAPS 共用
    /* 索引 1 */ {1, true },   // BT2
    /* 索引 2 */ {2, true },   // BT3
    /* 索引 3 */ {3, true },   // 电量灯
    /* 索引 4 */ {4, true },   // 充电灯
};

/* ============ 业务别名 ============ */

/**
 * @brief 蓝牙通道指示灯
 *
 * 3 个独立 LED 分别指示 3 个蓝牙通道
 */
#define LED_BT1         0
#define LED_BT2         1
#define LED_BT3         2

/**
 * @brief 大小写指示灯
 *
 * 与 BT1 共用同一个 LED
 * 后设置的状态会覆盖前面的
 */
#define LED_CAPS        LED_BT1

/**
 * @brief 电量指示灯
 *
 * 正常时熄灭，低电时闪烁
 */
#define LED_BAT         3

/**
 * @brief 充电指示灯
 *
 * 充电中点亮，充满熄灭
 */
#define LED_CHARGE      4

/* ============ 多灯电量指示（可选）============ */

#if 0  // 如需启用多灯电量显示，将 0 改为 1

/**
 * @brief 多灯电量显示配置
 *
 * 4 个 LED 分段显示电量：25% / 50% / 75% / 100%
 */
#define LED_BAT_25      5
#define LED_BAT_50      6
#define LED_BAT_75      7
#define LED_BAT_100     8

// 需要将 IND_LED_COUNT 修改为 9
// 并在 ind_led_table 中添加对应的引脚配置

#endif

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
 * // 蓝牙连接成功，通道 1 亮 2 秒
 * indicator_set(LED_BT1, &IND_ON_2S);
 *
 * // 蓝牙配对中，通道 2 慢闪
 * indicator_set(LED_BT2, &IND_BLINK_SLOW);
 *
 * // 蓝牙重连中，通道 3 快闪
 * indicator_set(LED_BT3, &IND_BLINK_FAST);
 *
 * // 大小写开启（会覆盖 BT1 状态）
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
