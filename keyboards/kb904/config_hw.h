#pragma once

/**
 * @file config_hw.h
 * @brief KB904 硬件 GPIO 映射
 * @note 直接对应 KB904_原理图.md
 */

/* ========================================
 * KB904 硬件 GPIO 映射
 * 直接对应 KB904_原理图.md
 * ======================================== */

/* ========== 矩阵配置 ========== */
/* 取消旧定义（来自 product_config.h）以避免重定义警告 */
#ifdef MATRIX_ROWS
#undef MATRIX_ROWS
#endif
#ifdef MATRIX_COLS
#undef MATRIX_COLS
#endif
#ifdef DIODE_DIRECTION
#undef DIODE_DIRECTION
#endif
#ifdef MATRIX_ROW_PINS
#undef MATRIX_ROW_PINS
#endif
#ifdef MATRIX_COL_PINS
#undef MATRIX_COL_PINS
#endif

#define MATRIX_ROWS           8
#define MATRIX_COLS           15
#define DIODE_DIRECTION       COL2ROW

#define MATRIX_ROW_PINS {     \
    A9,  /* R0 - U2.7  */     \
    A8,  /* R1 - U2.8  */     \
    A7,  /* R2 - U2.17 */     \
    A1,  /* R3 - U2.18 */     \
    A0,  /* R4 - U2.19 */     \
    A6,  /* R5 - U2.20 */     \
    A5,  /* R6 - U2.21 */     \
    A4   /* R7 - U2.22 */     \
}

#define MATRIX_COL_PINS {     \
    B9, /* C0  - U2.9  */    \
    B8, /* C1  - U2.10 */    \
    B17, /* C2  - U2.11 */    \
    B16, /* C3  - U2.12 */    \
    B15, /* C4  - U2.13 */    \
    B14, /* C5  - U2.14 */    \
    B13, /* C6  - U2.15 */    \
    B1, /* C7  - U2.16 */    \
    B7,  /* C8  - U2.4  */    \
    B12,  /* C9  - U2.38 */    \
    B6,  /* C10 - U2.37 */    \
    B5,  /* C11 - U2.36 */    \
    B4,  /* C12 - U2.40 */    \
    B3,  /* C13 - U2.39 */    \
    B2  /* C14 - U2.23 */    \
}

/* ========== 背光 RGB ==========
 * 网络名: PB0_PWM_R, PA12_PWM_G, PA13_PWM_B
 */
#define BACKLIGHT_PIN_R       B0   /* PB0_PWM_R - U2.24 */
#define BACKLIGHT_PIN_G       A12  /* PA12_PWM_G - U2.46 */
#define BACKLIGHT_PIN_B       A13  /* PA13_PWM_B - U2.45 */

/* ========== 指示灯 GPIO 映射 ==========
 * 网络名: PA15_LED1, PB22_LED2, PB23_LED3, PA14_LED4
 * 对应原理图: 4颗独立LED
 */
#define LED_CAPS_PIN          A15  /* 白灯 - 大写锁定 - U2.43 */
#define LED_BT_PIN            B22  /* 蓝灯 - 蓝牙状态 - U2.30 */
#define LED_POWER_PIN         B23  /* 红灯 - 电源/低电量 - U2.31 */

/* 兼容别名 */
#define LED_POWER_RED_PIN     LED_POWER_PIN

/* ========== 指示灯业务别名（供应用层使用）========== */
#define LED_CAPS          0    /* 白灯：大写锁定 */
#define LED_BT            1    /* 蓝灯：蓝牙状态 */
#define LED_POWER_RED     2    /* 红灯：电源 / 低电量 */

/* ========== 指示灯硬件表类型 ============ */
#include <stdbool.h>
#include "gpio.h"

typedef struct {
    pin_t pin;          /**< GPIO 引脚 */
    bool  active_high;  /**< true=高电平亮, false=低电平亮 */
} ind_led_def_t;

/* extern 声明（实现在 kb904/indicator.c）*/
extern const ind_led_def_t ind_led_table[IND_LED_COUNT];

/* ========== 触控板 I2C ==========
 * 芯片: PCT1336QN
 * 网络名: PB20_SDA, PB21_SCL, PA3_INT
 */
#define TOUCHPAD_SDA          NO_PIN  /* PB20_SDA - U2.28 */
#define TOUCHPAD_SCL          NO_PIN  /* PB21_SCL - U2.29 */
#define TOUCHPAD_INT          A3   /* PA3_INT - 下降沿触发 - U2.42 */

/* ========== 触控板尺寸 ========== */
#define TOUCHPAD_MAX_X        2048
#define TOUCHPAD_MAX_Y        1024

/* ========== 电源管理 ==========
 * 网络名: PA2_POWER_KEY_IN, PB19_POWER_EN, PA14_ADC
 */
#define POWER_KEY_PIN         A2   /* PA2_POWER_KEY_IN - U2.41 */
#define POWER_EN_PIN          B19  /* PB19_POWER_EN - U2.27 */
#define BATTERY_ADC_PIN       A14  /* PA14_ADC - U2.45 (与 LED_CHARGE_PIN 共用) */

/* ========== UART 调试 ==========
 * 网络名: PA9_TXD1, PA8_RXD1
 */
#define DEBUG_UART_TX         A9   /* PA9_TXD1 - U2.47 */
#define DEBUG_UART_RX         A8   /* PA8_RXD1 - U2.48 */
