#pragma once

/**
 * @file matrix_defaults.h
 * @brief 矩阵扫描默认配置
 * @note 产品可通过在 config.h 中 #define 覆盖默认值
 */

/* ========== 矩阵扫描参数 ========== */
#ifndef MATRIX_SCAN_RATE
#define MATRIX_SCAN_RATE     1000
#endif

#ifndef MATRIX_GHOST_THRESHOLD
#define MATRIX_GHOST_THRESHOLD  3
#endif

/* ========== 二极管方向 ========== */
#ifndef DIODE_DIRECTION
#define DIODE_DIRECTION      COL2ROW
#endif

/* ========== 防抖配置 ========== */
#ifndef DEBOUNCE_ALGORITHM
#define DEBOUNCE_ALGORITHM   DEBOUNCE_SYM_DEFER_PK
#endif

/* ========== 矩阵尺寸默认值（通常由产品覆盖） ========== */
// MATRIX_ROWS 和 MATRIX_COLS 必须在产品 config_hw.h 中定义
