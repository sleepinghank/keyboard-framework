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
#define MATRIX_COLS           16
#define DIODE_DIRECTION       COL2ROW

/* 行线 (ROW) - 输出
 * 对应原理图 FPC1 连接器
 * 网络名: R0-R7 (U2.7-22)
 */
#define MATRIX_ROW_PINS {     \
    B9,  /* R0 - U2.7  */     \
    B8,  /* R1 - U2.8  */     \
    B7,  /* R2 - U2.17 */     \
    B6,  /* R3 - U2.18 */     \
    B5,  /* R4 - U2.19 */     \
    B4,  /* R5 - U2.20 */     \
    B3,  /* R6 - U2.21 */     \
    B2   /* R7 - U2.22 */     \
}

/* 列线 (COL) - 输入带上拉
 * 对应原理图 FPC1 连接器
 * 网络名: C0-C15
 */
#define MATRIX_COL_PINS {     \
    B17, /* C0  - U2.9  */    \
    B16, /* C1  - U2.10 */    \
    B15, /* C2  - U2.11 */    \
    B14, /* C3  - U2.12 */    \
    B13, /* C4  - U2.13 */    \
    B12, /* C5  - U2.14 */    \
    B11, /* C6  - U2.15 */    \
    B10, /* C7  - U2.16 */    \
    A7,  /* C8  - U2.4  */    \
    A6,  /* C9  - U2.38 */    \
    A5,  /* C10 - U2.37 */    \
    A4,  /* C11 - U2.36 */    \
    A1,  /* C12 - U2.40 */    \
    A0,  /* C13 - U2.39 */    \
    B1,  /* C14 - U2.23 */    \
    A15  /* C15 - U2.43 */    \
}

/* ========== 背光 RGB ==========
 * 网络名: PB0_PWM_R, PA12_PWM_G, PA13_PWM_B
 */
#define BACKLIGHT_PIN_R       B0   /* PB0_PWM_R - U2.24 */
#define BACKLIGHT_PIN_G       A12  /* PA12_PWM_G - U2.46 */
#define BACKLIGHT_PIN_B       A13  /* PA13_PWM_B - U2.45 */

/* ========== 指示灯 ==========
 * 网络名: PA15_LED1, PB22_LED2, PB23_LED3, PA14_LED4
 * 对应原理图: 4颗独立LED
 */
#define LED_CAPS_PIN          A15  /* 白灯 - 大写锁定 - U2.43 */
#define LED_BT_PIN            B22  /* 蓝灯 - 蓝牙状态 - U2.30 */
#define LED_POWER_PIN         B23  /* 红灯 - 电源/低电量 - U2.31 */
#define LED_CHARGE_PIN        A14  /* 绿灯 - 充满电（只读）- U2.45 */

/* 兼容别名 */
#define LED_POWER_RED_PIN     LED_POWER_PIN
#define LED_POWER_GRN_PIN     LED_CHARGE_PIN

/* ========== 触控板 I2C ==========
 * 芯片: PCT1336QN
 * 网络名: PB20_SDA, PB21_SCL, PA3_INT
 */
#define TOUCHPAD_SDA          B20  /* PB20_SDA - U2.28 */
#define TOUCHPAD_SCL          B21  /* PB21_SCL - U2.29 */
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
