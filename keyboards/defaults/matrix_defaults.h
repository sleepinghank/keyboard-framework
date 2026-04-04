#pragma once

/**
 * @file matrix_defaults.h
 * @brief 矩阵扫描默认配置
 * @note 产品可通过在 config_product.h 中 #define 覆盖默认值
 */

/* ========== 矩阵扫描参数 ========== */
#ifndef MATRIX_SCAN_RATE
#define MATRIX_SCAN_RATE     1000
#endif

#ifndef MATRIX_GHOST_THRESHOLD
#define MATRIX_GHOST_THRESHOLD  3
#endif

/* ========== 鬼键检测 ========== */
// 默认不启用。无二极管矩阵的产品请在 config_product.h 中添加：
//   #define MATRIX_HAS_GHOST
// 有二极管矩阵的产品（不会产生幽灵键）无需定义此宏。
// #define MATRIX_HAS_GHOST

/* ========== 二极管方向 ========== */
#ifndef DIODE_DIRECTION
#define DIODE_DIRECTION      COL2ROW
#endif

/* ========== 矩阵输入状态 ========== */
#ifndef MATRIX_INPUT_PRESSED_STATE
#define MATRIX_INPUT_PRESSED_STATE  0  // 按键按下时的引脚状态（0 = 低电平）
#endif

/* ========== 防抖配置 ========== */
#ifndef DEBOUNCE_ALGORITHM
#define DEBOUNCE_ALGORITHM   DEBOUNCE_SYM_DEFER_PK
#endif

/* ========== 矩阵尺寸默认值（通常由产品覆盖） ========== */
// MATRIX_ROWS 和 MATRIX_COLS 必须在产品 config_hw.h 中定义
