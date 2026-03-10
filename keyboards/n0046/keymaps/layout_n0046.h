/**
 * @file layout_n0046.h
 * @brief N0046 78键 US 布局定义
 *
 * N0046 是 78 键 iPad 蓝牙键盘，使用 8x16 矩阵
 * 物理布局:
 *   Row 0: ESC, F1-F12, DEL (14 keys)
 *   Row 1: `~, 1-0, -, =, BSPC (14 keys)
 *   Row 2: TAB, Q-P, [, ], \ (14 keys)
 *   Row 3: CAPS, A-L, ;, ', ENTER (13 keys)
 *   Row 4: LSHIFT, Z-M, ,, ., /, RSHIFT (12 keys)
 *   Row 5: LCTRL, LALT, LGUI, SPACE, RGUI, RALT, FN, RCTRL (8 keys)
 *
 * 注意: 矩阵位置映射是假设的，需要根据实际原理图调整
 */

#pragma once

#include "keycode.h"

/**
 * LAYOUT_n0046 - N0046 78键 US 布局宏
 *
 * 参数按物理位置从左到右、从上到下排列
 * 映射到 8x16 矩阵位置
 */
#define LAYOUT_n0046( \
    /* Row 0: ESC, F1-F12, DEL */ \
    K_ESC,  K_F1,   K_F2,   K_F3,   K_F4,   K_F5,   K_F6,   K_F7,   K_F8,   K_F9,   K_F10,  K_F11,  K_F12,  K_DEL, \
    /* Row 1: `~, 1-0, -, =, BSPC */ \
    K_GRV,  K_1,    K_2,    K_3,    K_4,    K_5,    K_6,    K_7,    K_8,    K_9,    K_0,    K_MINS, K_EQL,  K_BSPC, \
    /* Row 2: TAB, Q-P, [, ], \ */ \
    K_TAB,  K_Q,    K_W,    K_E,    K_R,    K_T,    K_Y,    K_U,    K_I,    K_O,    K_P,    K_LBRC, K_RBRC, K_BSLS, \
    /* Row 3: CAPS, A-L, ;, ', ENTER */ \
    K_CAPS, K_A,    K_S,    K_D,    K_F,    K_G,    K_H,    K_J,    K_K,    K_L,    K_SCLN, K_QUOT, K_ENT, \
    /* Row 4: LSHIFT, Z-M, ,, ., /, RSHIFT */ \
    K_LSFT, K_Z,    K_X,    K_C,    K_V,    K_B,    K_N,    K_M,    K_COMM, K_DOT,  K_SLSH, K_RSFT, \
    /* Row 5: LCTRL, LALT, LGUI, SPACE, RGUI, RALT, FN, RCTRL */ \
    K_LCTL, K_LALT, K_LGUI,                 K_SPC,                          K_RGUI, K_RALT, K_FN,   K_RCTL \
) { \
    /* Matrix Row 0: ESC, F1-F12, DEL, +2 unused */ \
    { K_ESC,   K_F1,    K_F2,   K_F3,   K_F4,   K_F5,   K_F6,   K_F7,   K_F8,   K_F9,   K_F10,  K_F11,  K_F12,  K_DEL,  KC_NO,  KC_NO }, \
    /* Matrix Row 1: `~, 1-0, -, =, BSPC, +2 unused */ \
    { K_GRV,   K_1,     K_2,    K_3,    K_4,    K_5,    K_6,    K_7,    K_8,    K_9,    K_0,    K_MINS, K_EQL,  K_BSPC, KC_NO,  KC_NO }, \
    /* Matrix Row 2: TAB, Q-P, [, ], \, +2 unused */ \
    { K_TAB,   K_Q,     K_W,    K_E,    K_R,    K_T,    K_Y,    K_U,    K_I,    K_O,    K_P,    K_LBRC, K_RBRC, K_BSLS, KC_NO,  KC_NO }, \
    /* Matrix Row 3: CAPS, A-L, ;, ', ENTER, +3 unused */ \
    { K_CAPS,  K_A,     K_S,    K_D,    K_F,    K_G,    K_H,    K_J,    K_K,    K_L,    K_SCLN, K_QUOT, K_ENT,  KC_NO,  KC_NO,  KC_NO }, \
    /* Matrix Row 4: LSHIFT, Z-M, ,, ., /, RSHIFT, +4 unused */ \
    { K_LSFT,  K_Z,     K_X,    K_C,    K_V,    K_B,    K_N,    K_M,    K_COMM, K_DOT,  K_SLSH, K_RSFT, KC_NO,  KC_NO,  KC_NO,  KC_NO }, \
    /* Matrix Row 5: LCTRL, LALT, LGUI, 3 unused, SPACE, 3 unused, RGUI, RALT, FN, RCTRL, +2 unused */ \
    { K_LCTL,  K_LALT,  K_LGUI, KC_NO,  KC_NO,  KC_NO,  K_SPC,  KC_NO,  KC_NO,  KC_NO,  K_RGUI, K_RALT, K_FN,   K_RCTL, KC_NO,  KC_NO }, \
    /* Matrix Row 6: 未使用 */ \
    { KC_NO,   KC_NO,   KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO }, \
    /* Matrix Row 7: 未使用 */ \
    { KC_NO,   KC_NO,   KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO }  \
}

/**
 * 矩阵配置
 */
#define MATRIX_ROWS 8
#define MATRIX_COLS 16
