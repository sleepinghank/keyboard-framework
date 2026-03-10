#pragma once

/**
 * @file config_gpio.h
 * @brief N0046 GPIO 配置
 */

/* ========== 矩阵行线 (ROW) - 输出 ========== */
#define MATRIX_ROW_PINS { A4, A5, A6, A0, A1, A8, A9, B9 }

/* ========== 矩阵列线 (COL) - 输入带上拉 ========== */
#define MATRIX_COL_PINS { B5, B8, B17, B16, B15, B14, B13, B12, \
                          B3, B4, B2, A7, B7, B18, B1, B6 }

/* ========== 指示灯 GPIO ========== */
#define IND_LED_WHITE_PIN    A15    // 白灯：大写锁定/蓝牙状态
#define IND_LED_RED_PIN      B23    // 红灯：充电/低电量

/* ========== 背光 GPIO ========== */
#define BACKLIGHT_EN_PIN     B22    // 背光电源使能
#define BACKLIGHT_R_PIN      B0     // 红色 PWM
#define BACKLIGHT_G_PIN      A12    // 绿色 PWM
#define BACKLIGHT_B_PIN      A13    // 蓝色 PWM

/* ========== 电源管理 GPIO ========== */
#define BAT_ADC_PIN          A14    // 电池电压 ADC
