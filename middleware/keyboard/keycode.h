/*
 * Copyright (C) 2024 Keyboard Framework
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Basic keycode definitions (HID usage codes)
// These follow the HID keyboard/keypad page (0x07)
#define KC_NO 0x00        /**< No key pressed */
#define KC_ROLL_OVER 0x01 /**< Roll over error */
#define KC_POST_FAIL 0x02 /**< POST Fail */
#define KC_ERROR_UNDEFINED 0x03 /**< ErrorUndefined */
#define KC_A 0x04         /**< Key 'a' or 'A' */
#define KC_B 0x05         /**< Key 'b' or 'B' */
#define KC_C 0x06         /**< Key 'c' or 'C' */
#define KC_D 0x07         /**< Key 'd' or 'D' */
#define KC_E 0x08         /**< Key 'e' or 'E' */
#define KC_F 0x09         /**< Key 'f' or 'F' */
#define KC_G 0x0A         /**< Key 'g' or 'G' */
#define KC_H 0x0B         /**< Key 'h' or 'H' */
#define KC_I 0x0C         /**< Key 'i' or 'I' */
#define KC_J 0x0D         /**< Key 'j' or 'J' */
#define KC_K 0x0E         /**< Key 'k' or 'K' */
#define KC_L 0x0F         /**< Key 'l' or 'L' */
#define KC_M 0x10         /**< Key 'm' or 'M' */
#define KC_N 0x11         /**< Key 'n' or 'N' */
#define KC_O 0x12         /**< Key 'o' or 'O' */
#define KC_P 0x13         /**< Key 'p' or 'P' */
#define KC_Q 0x14         /**< Key 'q' or 'Q' */
#define KC_R 0x15         /**< Key 'r' or 'R' */
#define KC_S 0x16         /**< Key 's' or 'S' */
#define KC_T 0x17         /**< Key 't' or 'T' */
#define KC_U 0x18         /**< Key 'u' or 'U' */
#define KC_V 0x19         /**< Key 'v' or 'V' */
#define KC_W 0x1A         /**< Key 'w' or 'W' */
#define KC_X 0x1B         /**< Key 'x' or 'X' */
#define KC_Y 0x1C         /**< Key 'y' or 'Y' */
#define KC_Z 0x1D         /**< Key 'z' or 'Z' */
#define KC_1 0x1E         /**< Key '1' or '!' */
#define KC_2 0x1F         /**< Key '2' or '@' */
#define KC_3 0x20         /**< Key '3' or '#' */
#define KC_4 0x21         /**< Key '4' or '$' */
#define KC_5 0x22         /**< Key '5' or '%' */
#define KC_6 0x23         /**< Key '6' or '^' */
#define KC_7 0x24         /**< Key '7' or '&' */
#define KC_8 0x25         /**< Key '8' or '*' */
#define KC_9 0x26         /**< Key '9' or '(' */
#define KC_0 0x27         /**< Key '0' or ')' */
#define KC_ENTER 0x28     /**< Return (Enter) */
#define KC_ESCAPE 0x29    /**< Escape */
#define KC_BACKSPACE 0x2A /**< Backspace */
#define KC_TAB 0x2B       /**< Tab */
#define KC_SPACE 0x2C     /**< Spacebar */
#define KC_MINUS 0x2D     /**< '-' or '_' */
#define KC_EQUAL 0x2E     /**< '=' or '+' */
#define KC_LEFT_BRACKET 0x2F /**< '[' or '{' */
#define KC_RIGHT_BRACKET 0x30 /**< ']' or '}' */
#define KC_BACKSLASH 0x31     /**< '\' or '|' */
#define KC_NONUS_HASH 0x32    /**< Non-US '#' and '~' */
#define KC_SEMICOLON 0x33     /**< ';' or ':' */
#define KC_QUOTE 0x34         /**< ''' and '"' */
#define KC_GRAVE 0x35         /**< '`' and '~' */
#define KC_COMMA 0x36         /**< ',' and '<' */
#define KC_DOT 0x37           /**< '.' and '>' */
#define KC_SLASH 0x38         /**< '/' and '?' */
#define KC_CAPS_LOCK 0x39     /**< Caps Lock */
#define KC_F1 0x3A            /**< F1 */
#define KC_F2 0x3B            /**< F2 */
#define KC_F3 0x3C            /**< F3 */
#define KC_F4 0x3D            /**< F4 */
#define KC_F5 0x3E            /**< F5 */
#define KC_F6 0x3F            /**< F6 */
#define KC_F7 0x40            /**< F7 */
#define KC_F8 0x41            /**< F8 */
#define KC_F9 0x42            /**< F9 */
#define KC_F10 0x43           /**< F10 */
#define KC_F11 0x44           /**< F11 */
#define KC_F12 0x45           /**< F12 */
#define KC_PRINT_SCREEN 0x46  /**< Print Screen */
#define KC_SCROLL_LOCK 0x47   /**< Scroll Lock */
#define KC_PAUSE 0x48         /**< Pause */
#define KC_INSERT 0x49        /**< Insert */
#define KC_HOME 0x4A          /**< Home */
#define KC_PAGE_UP 0x4B       /**< Page Up */
#define KC_DELETE 0x4C        /**< Delete */
#define KC_END 0x4D           /**< End */
#define KC_PAGE_DOWN 0x4E     /**< Page Down */
#define KC_RIGHT 0x4F         /**< Right Arrow */
#define KC_LEFT 0x50          /**< Left Arrow */
#define KC_DOWN 0x51          /**< Down Arrow */
#define KC_UP 0x52            /**< Up Arrow */
#define KC_NUM_LOCK 0x53      /**< Num Lock and Clear */

// Modifiers (bitmask)
#define KC_LCTRL 0xE0         /**< Left Control */
#define KC_LSHIFT 0xE1        /**< Left Shift */
#define KC_LALT 0xE2          /**< Left Alt */
#define KC_LGUI 0xE3          /**< Left GUI (Windows/Command) */
#define KC_RCTRL 0xE4         /**< Right Control */
#define KC_RSHIFT 0xE5        /**< Right Shift */
#define KC_RALT 0xE6          /**< Right Alt */
#define KC_RGUI 0xE7          /**< Right GUI (Windows/Command) */

// Function keycodes (non-standard keys)
#define KC_APPLICATION 0x65   /**< Application (Menu) */
#define KC_POWER 0x66         /**< Power */
#define KC_F13 0x68           /**< F13 */
#define KC_F14 0x69           /**< F14 */
#define KC_F15 0x6A           /**< F15 */
#define KC_F16 0x6B16 */
#define KC_F17 0x6C           /**< F17           /**< F */
#define KC_F18 0x6D           /**< F18 */
#define KC_F19 0x6E           /**< F19 */
#define KC_F20 0x6F           /**< F20 */
#define KC_F21 0x70           /**< F21 */
#define KC_F22 0x71           /**< F22 */
#define KC_F23 0x72           /**< F23 */
#define KC_F24 0x73           /**< F24 */

// Extended keycodes
#define KC_EXECUTE 0x74       /**< Execute */
#define KC_HELP 0x75          /**< Help */
#define KC_MENU 0x76          /**< Menu */
#define KC_SELECT 0x77        /**< Select */
#define KC_STOP 0x78          /**< Stop */
#define KC_AGAIN 0x79         /**< Again */
#define KC_UNDO 0x7A          /**< Undo */
#define KC_CUT 0x7B           /**< Cut */
#define KC_COPY 0x7C          /**< Copy */
#define KC_PASTE 0x7D         /**< Paste */
#define KC_FIND 0x7E          /**< Find */

// Numpad keycodes
#define KC_KP_0 0x80          /**< Keypad '0' and Insert */
#define KC_KP_1 0x81          /**< Keypad '1' and End */
#define KC_KP_2 0x82          /**< Keypad '2' and Down Arrow */
#define KC_KP_3 0x83          /**< Keypad '3' and Page Down */
#define KC_KP_4 0x84          /**< Keypad '4' and Left Arrow */
#define KC_KP_5 0x85          /**< Keypad '5' */
#define KC_KP_6 0x86          /**< Keypad '6' and Right Arrow */
#define KC_KP_7 0x87          /**< Keypad '7' and Home */
#define KC_KP_8 0x88          /**< Keypad '8' and Up Arrow */
#define KC_KP_9 0x89          /**< Keypad '9' and Page Up */
#define KC_KP_DECIMAL 0x8A    /**< Keypad '.' and Delete */
#define KC_KP_ENTER 0x8C      /**< Keypad Enter */
#define KC_KP_EQUAL 0x8D      /**< Keypad '=' */

// Modifier masks (for bit operations)
#define MOD_LCTRL  (1 << 0)
#define MOD_LSHIFT (1 << 1)
#define MOD_LALT   (1 << 2)
#define MOD_LGUI   (1 << 3)
#define MOD_RCTRL  (1 << 4)
#define MOD_RSHIFT (1 << 5)
#define MOD_RALT   (1 << 6)
#define MOD_RGUI   (1 << 7)
#define MOD_HYPR   (1 << 8)   /**< Hyper (Ctrl+Alt+Shift+GUI) */
#define MOD_MEH    (1 << 9)   /**< Meh (Ctrl+Alt+Shift) */

// Convenience macros for combining mods
#define MOD_MASK(code) (((code) >> 8) & 0xFF)
#define MOD_BIT(code) (1 << ((code)-0xE0))
#define CHECK_MODS(code) (((code) & 0xF000) == 0xF000)

// Layer switching keycodes (Quantum)
#define KC_TG(n)  (QK_TOGGLE_LAYER | ((n) & 0xFF))
#define KC_TO(n)  (QK_TO_LAYER | ((n) & 0xFF))
#define KC_MO(n)  (QK_MOMENTARY_LAYER | ((n) & 0xFF))
#define KC_OSL(n) (QK_ONE_SHOT_LAYER | ((n) & 0xFF))

// Quantum keycode ranges
#define QK_BASIC_MAX 0x00FF
#define QK_BASIC (0x0100) /**< Basic keycode range marker */
#define QK_BASIC_MAX (0x01FF)

#define QK_LAYER_TAP_MAX (0x23FF)
#define QK_LAYER_TAP (0x2400) /**< Layer tap keycode range */
#define QK_LAYER_TAP_MAX (0x27FF)

#define QK_LAYER_MOD_MAX (0x2BFF)
#define QK_LAYER_MOD (0x2C00) /**< Layer mod keycode range */
#define QK_LAYER_MOD_MAX (0x2FFF)

#define QK_TAP_DANCE_MAX (0x3FFF)
#define QK_TAP_DANCE (0x4000) /**< Tap dance keycode range */
#define QK_TAP_DANCE_MAX (0x43FF)

#define QK_MOD_TAP_MAX (0x4FFF)
#define QK_MOD_TAP (0x5000) /**< Mod tap keycode range */
#define QK_MOD_TAP_MAX (0x53FF)

#define QK_FUNCTION_MAX (0xFFFF)
#define QK_FUNCTION (0x8000) /**< Function keycode range */
#define QK_FUNCTION_MAX (0x83FF)

// Layer control keycodes
#define QK_TOGGLE_LAYER (0x2000) /**< Toggle layer on/off */
#define QK_TO_LAYER (0x4000)     /**< Activate layer (momentary when held) */
#define QK_MOMENTARY_LAYER (0x6000) /**< Activate layer while held */
#define QK_ONE_SHOT_LAYER (0xA000)  /**< One-shot layer for one keypress */

// Get layer from a quantum keycode
#define GET_LAYER(code) ((code) & 0xFF)
#define IS_QK_LAYER_TAP(code) (((code) & 0xF800) == QK_LAYER_TAP)
#define IS_QK_LAYER_MOD(code) (((code) & 0xF800) == QK_LAYER_MOD)

// Type checking
#define IS_KEYCODE(code) (((code) & 0xF000) == 0x0000)
#define IS_LAYER_KEY(code) (((code) & 0xF800) >= 0x2000)
#define IS_FUNCTION_KEY(code) (((code) & 0xFC00) == 0x8000)

#ifdef __cplusplus
}
#endif
