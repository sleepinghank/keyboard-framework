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
 * @file backlight.h
 * @brief 背光灯驱动接口
 *
 * 支持单色灯和RGB三色灯两种模式
 * 提供亮度调节、颜色设置等功能
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

/* ============ 类型定义 ============ */

/**
 * @brief RGB颜色结构
 */
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} bl_rgb_t;

/**
 * @brief 预设颜色枚举
 */
typedef enum {
    BL_COLOR_RED = 0,
    BL_COLOR_DARK_RED,
    BL_COLOR_PINK,
    BL_COLOR_PURPLE,
    BL_COLOR_INDIGO,
    BL_COLOR_DARK_BLUE,
    BL_COLOR_BLUE,
    BL_COLOR_CYAN_BLUE,
    BL_COLOR_CYAN_GREEN,
    BL_COLOR_GREEN,
    BL_COLOR_LIGHT_YELLOW,
    BL_COLOR_ORANGE,
    BL_COLOR_WHITE,
    BL_COLOR_COUNT
} bl_preset_color_t;

/* 旧枚举兼容别名 */
#define BL_COLOR_YELLOW  BL_COLOR_LIGHT_YELLOW
#define BL_COLOR_CYAN    BL_COLOR_CYAN_GREEN
#define BL_COLOR_MAGENTA BL_COLOR_PURPLE

/**
 * @brief 预设亮度枚举
 */
typedef enum {
    BL_LEVEL_OFF = 0,
    BL_LEVEL_LOW,
    BL_LEVEL_MEDIUM,
    BL_LEVEL_HIGH,
    BL_LEVEL_COUNT
} bl_preset_level_t;

/**
 * @brief 背光灯状态结构 (用于外部存储)
 */
typedef struct {
    bool     enable;
    uint8_t  brightness;
    bl_rgb_t color;
} bl_state_t;

/* ============ 初始化 ============ */

/**
 * @brief 初始化背光模块
 * @param state 初始状态指针，NULL则使用默认值
 */
void backlight_init(const bl_state_t* state);

/**
 * @brief 反初始化背光模块
 */
void backlight_deinit(void);

/* ============ 开关控制 ============ */

void backlight_enable(void);
void backlight_disable(void);
void backlight_toggle(void);
bool backlight_is_enabled(void);

/* ============ 亮度控制 ============ */

/**
 * @brief 设置亮度
 * @param brightness 亮度值 0-100
 */
void backlight_set_brightness(uint8_t brightness);

/**
 * @brief 获取当前亮度
 * @return 亮度值 0-100
 */
uint8_t backlight_get_brightness(void);

/**
 * @brief 亮度增加
 * @param step 增加步进值
 */
void backlight_brightness_increase(uint8_t step);

/**
 * @brief 亮度减少
 * @param step 减少步进值
 */
void backlight_brightness_decrease(uint8_t step);

/**
 * @brief 设置预设亮度档位
 * @param level 预设档位
 */
void backlight_set_preset_level(bl_preset_level_t level);

/**
 * @brief 切换到下一档亮度
 */
void backlight_level_step(void);

/* ============ 颜色控制 (仅RGB模式) ============ */

/**
 * @brief 设置RGB颜色
 * @param rgb 颜色结构指针
 */
void backlight_set_color(const bl_rgb_t* rgb);

/**
 * @brief 设置RGB颜色
 * @param r 红色分量 0-100
 * @param g 绿色分量 0-100
 * @param b 蓝色分量 0-100
 */
void backlight_set_rgb(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief 获取当前颜色
 * @param rgb 输出颜色结构指针
 */
void backlight_get_color(bl_rgb_t* rgb);

/**
 * @brief 设置单通道亮度
 * @param channel 通道: 'r', 'g', 'b'
 * @param value 亮度值 0-100
 */
void backlight_set_channel(char channel, uint8_t value);

/**
 * @brief 设置预设颜色
 * @param color 预设颜色枚举
 */
void backlight_set_preset_color(bl_preset_color_t color);

/**
 * @brief 切换到下一个预设颜色
 */
void backlight_color_step(void);

/* ============ 状态管理 ============ */

/**
 * @brief 获取当前状态 (供外部存储使用)
 * @param state 输出状态结构指针
 */
void backlight_get_state(bl_state_t* state);

/**
 * @brief 恢复状态 (从外部存储加载)
 * @param state 状态结构指针
 */
void backlight_restore_state(const bl_state_t* state);

/* ============ 便捷宏 ============ */

#define backlight_on()   backlight_enable()
#define backlight_off()  backlight_disable()
