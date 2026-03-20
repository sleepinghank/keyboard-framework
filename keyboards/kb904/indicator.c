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
 * @file indicator.c
 * @brief KB904 指示灯硬件表实现
 *
 * 从 indicator_config.h 迁移，分离硬件表实现到产品级 .c 文件
 * 类型定义和别名宏在 config_hw.h 中
 */

#include "config.h"
#include "config_hw.h"

/**
 * @brief LED 硬件配置表（extern 供 HAL 层访问）
 *
 * 索引与 config_hw.h 中的 LED_* 别名对应：
 *   0: LED_CAPS        - 白灯，大写锁定
 *   1: LED_BT          - 蓝灯，蓝牙状态
 *   2: LED_POWER_RED   - 红灯，电源/低电量
 *   3: LED_POWER_GRN   - 绿灯，充满电（只读）
 */
const ind_led_def_t ind_led_table[IND_LED_COUNT] = {
    /* 0 */ { LED_CAPS_PIN,   false },   /* 白灯：大写锁定 */
    /* 1 */ { LED_BT_PIN,     false },   /* 蓝灯：蓝牙状态 */
    /* 2 */ { LED_POWER_PIN,  false },   /* 红灯：电源 / 低电量 */
};
