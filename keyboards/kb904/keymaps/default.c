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
 * @file default.c
 * @brief KB904 默认键位表
 *
 * 迁移自 keyboards/default_keymap.c
 * 基于 Keychron K7 Pro 键值表转换
 */

#include "keymap.h"
#include "keycode.h"

/*
 * 原始键表结构: 16列 × 8行 (KEY_TABLE[16*8])
 * 当前键表结构: 8行 × 16列 (keymap_layers[8][16])
 *
 * 列映射 (原列号 -> 新列号):
 * 原col0  -> 新col10
 * 原col1  -> 新col0
 * 原col2  -> 新col1
 * 原col3  -> 新col2
 * 原col4  -> 新col3
 * 原col5  -> 新col4
 * 原col6  -> 新col5
 * 原col7  -> 新col6
 * 原col8  -> 新col12
 * 原col9  -> 新col11
 * 原col10 -> 新col13
 * 原col11 -> 新col15
 * 原col12 -> 新col8
 * 原col14 -> 新col7
 * 原col15 -> 新col14
 * 原col16 -> 新col9
 */

// 自定义键码（使用不同的宏名避免与 keycode.h 中的 S_FN_KEY 冲突）
#define KEYMAP_FN_KEY    0x5220  // MO(1) - 切换到 Fn 层
#define TOUCH_BUTTON KC_NO  // 触摸板按钮，暂不映射

const uint16_t keymap_layers[LAYER_MAX][MATRIX_ROWS][MATRIX_COLS] = {

    // =====================================================
    // LAYER_BASE - Windows/Android 基础层
    // =====================================================
    [LAYER_BASE] = {
        // 新col:    0        1        2        3        4        5        6        7        8        9        10       11       12       13       14       15
        // 原col:    1        2        3        4        5        6        7        14       12       16       0        9        8        10       15       11
        /* Row 0 */ KC_ESC,   KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,
        /* Row 1 */ KC_NO,    KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_NO,    KC_LALT,  KC_NO,    KC_NO,    KC_NO,    KC_NO,
        /* Row 2 */ KC_TAB,   KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_NO,    KC_NO,    KC_LCTL,  KC_NO,
        /* Row 3 */ KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_NO,    KC_NO,    KC_NO,    KC_RSFT,  KC_NO,    KC_NO,
        /* Row 4 */ KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_LSFT,  KC_NO,    KC_NO,
        /* Row 5 */ KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KEYMAP_FN_KEY, KC_NO, KC_NO,    KC_NO,
        /* Row 6 */ KC_F10,   KC_F11,   KC_MINS,  KC_EQL,   KC_P,     KC_LBRC,  KC_SCLN,  KC_COMM,  KC_RSFT,  KC_RALT,  KC_UP,    KC_DOWN,  KC_NO,    KC_LGUI,  KC_NO,    KC_NO,
        /* Row 7 */ KC_F12,   KC_DEL,   KC_BSPC,  KC_RBRC,  KC_BSLS,  KC_QUOT,  KC_ENT,   KC_SLSH,  KC_LEFT,  KC_RGHT,  KC_NO,    KC_SPC,   KC_NO,    KC_NO,    KC_RGUI,  KC_NO,
    },

    // =====================================================
    // LAYER_MACOS - macOS/iOS 层 (GUI/ALT 互换)
    // =====================================================
    [LAYER_MACOS] = {
        // 新col:    0       1       2       3       4       5       6       7       8       9       10      11      12      13      14      15
        /* Row 0 */  KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,
        /* Row 1 */  KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,
        /* Row 2 */  KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,
        /* Row 3 */  KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_LALT,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,
        /* Row 4 */  KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_RGUI,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,
        /* Row 5 */  KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_RALT,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,
        /* Row 6 */  KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,
        /* Row 7 */  KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,
    },

    // =====================================================
    // LAYER_WINDOWS - Windows/Android 层 (与基础层相同)
    // =====================================================
    [LAYER_WINDOWS] = {
        // 新col:    0       1       2       3       4       5       6       7       8       9       10      11      12      13      14      15
        /* Row 0 */  KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,
        /* Row 1 */  KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,
        /* Row 2 */  KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,
        /* Row 3 */  KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_LGUI,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,
        /* Row 4 */  KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_RALT,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,
        /* Row 5 */  KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_RGUI,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,
        /* Row 6 */  KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,
        /* Row 7 */  KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,
    }
};
