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

