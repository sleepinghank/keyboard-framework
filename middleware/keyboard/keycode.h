/*
Copyright 2011,2012 Jun Wako <wakojun@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * Keycodes based on HID Keyboard/Keypad Usage Page (0x07) plus media keys from Generic Desktop Page (0x01) and Consumer Page (0x0C)
 *
 * See https://web.archive.org/web/20060218214400/http://www.usb.org/developers/devclass_docs/Hut1_12.pdf
 * or http://www.usb.org/developers/hidpage/Hut1_12v2.pdf (older)
 */

#pragma once

/* FIXME: Add doxygen comments here */

#define IS_ANY(code) (KC_A <= (code) && (code) <= 0xFF)

#define IS_MOUSEKEY(code) IS_MOUSE_KEYCODE(code)
#define IS_MOUSEKEY_MOVE(code) (KC_MS_UP <= (code) && (code) <= KC_MS_RIGHT)
#define IS_MOUSEKEY_BUTTON(code) (KC_MS_BTN1 <= (code) && (code) <= KC_MS_BTN8)
#define IS_MOUSEKEY_WHEEL(code) (KC_MS_WH_UP <= (code) && (code) <= KC_MS_WH_RIGHT)
#define IS_MOUSEKEY_ACCEL(code) (KC_MS_ACCEL0 <= (code) && (code) <= KC_MS_ACCEL2)

#define MOD_BIT(code) (1 << ((code)&0x07))

// clang-format off

// TODO: dd keycodes
#include "keycodes.h"
#include "modifiers.h"

// ============================================================================
// 新增：kb-04122 兼容定义
// ============================================================================

// 特殊功能键（Fn 键）
#ifndef S_FN_KEY
#define S_FN_KEY 0xF0
#endif

// 媒体键类型标记
#ifndef M_KEY_TYPE
#define M_KEY_TYPE 0x1000
#endif

// 媒体键定义
#ifndef M_BACKLIGHT_DOWN
#define M_BACKLIGHT_DOWN    (M_KEY_TYPE | 0x0070)
#endif

#ifndef M_BACKLIGHT_UP
#define M_BACKLIGHT_UP      (M_KEY_TYPE | 0x006F)
#endif

#ifndef M_VOLUME_DOWN
#define M_VOLUME_DOWN       (M_KEY_TYPE | 0x00EA)
#endif

#ifndef M_VOLUME_UP
#define M_VOLUME_UP         (M_KEY_TYPE | 0x00E9)
#endif

#ifndef M_MUTE
#define M_MUTE              (M_KEY_TYPE | 0x00E2)
#endif

#ifndef M_PLAY_N_PAUSE
#define M_PLAY_N_PAUSE      (M_KEY_TYPE | 0x00CD)
#endif

#ifndef M_SCAN_NEXT_TRACK
#define M_SCAN_NEXT_TRACK   (M_KEY_TYPE | 0x00B5)
#endif

#ifndef M_SCAN_PREVIOUS_TRACK
#define M_SCAN_PREVIOUS_TRACK (M_KEY_TYPE | 0x00B6)
#endif

#ifndef M_WWW_SEARCH
#define M_WWW_SEARCH        (M_KEY_TYPE | 0x0221)
#endif

#ifndef M_EARTH
#define M_EARTH             (M_KEY_TYPE | 0x029D)
#endif

// KB_* 别名（兼容 kb-04122 代码风格）
#define KB_A            KC_A
#define KB_B            KC_B
#define KB_C            KC_C
#define KB_D            KC_D
#define KB_E            KC_E
#define KB_F            KC_F
#define KB_G            KC_G
#define KB_H            KC_H
#define KB_I            KC_I
#define KB_J            KC_J
#define KB_K            KC_K
#define KB_L            KC_L
#define KB_M            KC_M
#define KB_N            KC_N
#define KB_O            KC_O
#define KB_P            KC_P
#define KB_Q            KC_Q
#define KB_R            KC_R
#define KB_S            KC_S
#define KB_T            KC_T
#define KB_U            KC_U
#define KB_V            KC_V
#define KB_W            KC_W
#define KB_X            KC_X
#define KB_Y            KC_Y
#define KB_Z            KC_Z

#define KB_1            KC_1
#define KB_2            KC_2
#define KB_3            KC_3
#define KB_4            KC_4
#define KB_5            KC_5
#define KB_6            KC_6
#define KB_7            KC_7
#define KB_8            KC_8
#define KB_9            KC_9
#define KB_0            KC_0

#define KB_F1           KC_F1
#define KB_F2           KC_F2
#define KB_F3           KC_F3
#define KB_F4           KC_F4
#define KB_F5           KC_F5
#define KB_F6           KC_F6
#define KB_F7           KC_F7
#define KB_F8           KC_F8
#define KB_F9           KC_F9
#define KB_F10          KC_F10
#define KB_F11          KC_F11
#define KB_F12          KC_F12

#define KB_ESCAPE       KC_ESCAPE
#define KB_TAB          KC_TAB
#define KB_SPACE        KC_SPACE
#define KB_ENTER        KC_ENTER

#define KB_L_CTRL       KC_LEFT_CTRL
#define KB_L_SHIFT      KC_LEFT_SHIFT
#define KB_L_ALT        KC_LEFT_ALT
#define KB_L_GUI        KC_LEFT_GUI
#define KB_R_CTRL       KC_RIGHT_CTRL
#define KB_R_SHIFT      KC_RIGHT_SHIFT
#define KB_R_ALT        KC_RIGHT_ALT
#define KB_R_GUI        KC_RIGHT_GUI

#define KB_DELETE_FORWARD KC_DELETE
#define KB_EQUAL_N_PLUS   KC_EQUAL
