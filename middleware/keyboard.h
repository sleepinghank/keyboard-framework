#pragma once

/**
 * @file keyboard.h
 * @brief Cross-Platform Keyboard Middleware Framework
 *
 * This is the main header for the keyboard middleware framework.
 * Include this file to access all keyboard framework functionality.
 *
 * @version 1.0.0
 * @date 2024-12-06
 * @copyright GPL-3.0-or-later
 */

// Core framework
#include "keyboard/keyboard_framework.h"

// Individual modules (if needed separately)
#include "keyboard/debounce.h"
#include "keyboard/layer.h"
#include "keyboard/keycode.h"
#include "keyboard/action.h"
#include "keyboard/combo.h"
#include "keyboard/custom_function.h"
#include "keyboard/keyboard_core.h"

// Platform support
#include "keyboard/platform_stm32.h"
#include "keyboard/platform_wch.h"
#include "keyboard/platform_nordic.h"

// Quick start macros for common use cases
/**
 * @defgroup QuickStart Quick Start Macros
 * @{
 */

/** @brief Initialize keyboard with default settings */
#define KEYBOARD_INIT() keyboard_framework_init()

/** @brief Main keyboard processing loop */
#define KEYBOARD_TASK() keyboard_framework_task()

/** @brief Reset all keyboard state */
#define KEYBOARD_RESET() keyboard_framework_reset()

/** @brief Get framework version */
#define KEYBOARD_VERSION() keyboard_framework_version()

/** @brief Get current platform */
#define KEYBOARD_PLATFORM() keyboard_framework_platform()

/** @brief Send a key (press and release) */
#define KEYBOARD_SEND_KEY(keycode) tap_code(keycode)

/** @brief Press and hold a key */
#define KEYBOARD_PRESS_KEY(keycode) register_code(keycode)

/** @brief Release a key */
#define KEYBOARD_RELEASE_KEY(keycode) unregister_code(keycode)

/** @brief Check if layer is active */
#define KEYBOARD_IS_LAYER_ACTIVE(layer) layer_state_is(layer)

/** @brief Activate a layer */
#define KEYBOARD_ACTIVATE_LAYER(layer) layer_on(layer)

/** @brief Deactivate a layer */
#define KEYBOARD_DEACTIVATE_LAYER(layer) layer_off(layer)

/** @brief Get current modifier state */
#define KEYBOARD_GET_MODS() get_mods()

/** @} */

// Common keycode groups for convenience
/**
 * @defgroup KeycodeGroups Keycode Groups
 * @{
 */

/** @name Letter Keys */
#define KEY_A KC_A
#define KEY_B KC_B
#define KEY_C KC_C
#define KEY_D KC_D
#define KEY_E KC_E
#define KEY_F KC_F
#define KEY_G KC_G
#define KEY_H KC_H
#define KEY_I KC_I
#define KEY_J KC_J
#define KEY_K KC_K
#define KEY_L KC_L
#define KEY_M KC_M
#define KEY_N KC_N
#define KEY_O KC_O
#define KEY_P KC_P
#define KEY_Q KC_Q
#define KEY_R KC_R
#define KEY_S KC_S
#define KEY_T KC_T
#define KEY_U KC_U
#define KEY_V KC_V
#define KEY_W KC_W
#define KEY_X KC_X
#define KEY_Y KC_Y
#define KEY_Z KC_Z

/** @name Number Keys */
#define KEY_0 KC_0
#define KEY_1 KC_1
#define KEY_2 KC_2
#define KEY_3 KC_3
#define KEY_4 KC_4
#define KEY_5 KC_5
#define KEY_6 KC_6
#define KEY_7 KC_7
#define KEY_8 KC_8
#define KEY_9 KC_9

/** @name Function Keys */
#define KEY_F1 KC_F1
#define KEY_F2 KC_F2
#define KEY_F3 KC_F3
#define KEY_F4 KC_F4
#define KEY_F5 KC_F5
#define KEY_F6 KC_F6
#define KEY_F7 KC_F7
#define KEY_F8 KC_F8
#define KEY_F9 KC_F9
#define KEY_F10 KC_F10
#define KEY_F11 KC_F11
#define KEY_F12 KC_F12

/** @name Modifier Keys */
#define KEY_LCTRL KC_LCTRL
#define KEY_LSHIFT KC_LSHIFT
#define KEY_LALT KC_LALT
#define KEY_LGUI KC_LGUI
#define KEY_RCTRL KC_RCTRL
#define KEY_RSHIFT KC_RSHIFT
#define KEY_RALT KC_RALT
#define KEY_RGUI KC_RGUI

/** @name Special Keys */
#define KEY_ENTER KC_ENTER
#define KEY_ESCAPE KC_ESCAPE
#define KEY_BACKSPACE KC_BACKSPACE
#define KEY_TAB KC_TAB
#define KEY_SPACE KC_SPACE
#define KEY_CAPS_LOCK KC_CAPS_LOCK
#define KEY_ESC KC_ESCAPE

/** @name Arrow Keys */
#define KEY_UP KC_UP
#define KEY_DOWN KC_DOWN
#define KEY_LEFT KC_LEFT
#define KEY_RIGHT KC_RIGHT

/** @name Navigation Keys */
#define KEY_HOME KC_HOME
#define KEY_END KC_END
#define KEY_PAGE_UP KC_PAGE_UP
#define KEY_PAGE_DOWN KC_PAGE_DOWN
#define KEY_INSERT KC_INSERT
#define KEY_DELETE KC_DELETE

/** @} */

// Layer shortcuts
/**
 * @defgroup LayerShortcuts Layer Shortcuts
 * @{
 */

/** @brief Momentarily activate layer (when held) */
#define LAYER_MO(n) MO(n)

/** @brief Toggle layer on/off */
#define LAYER_TG(n) TG(n)

/** @brief Activate layer (momentary when held) */
#define LAYER_TO(n) TO(n)

/** @brief One-shot layer for one keypress */
#define LAYER_OSL(n) OSL(n)

/** @} */

// Combo shortcuts
/**
 * @defgroup ComboShortcuts Combo Shortcuts
 * @{
 */

/** @brief Define a 2-key combo */
#define COMBO_2(key1, key2, output) \
    { .key = {{(key1).row, (key1).col}, {(key2).row, (key2).col}}, \
      .keycode = output, .count = 2 }

/** @brief Define a 3-key combo */
#define COMBO_3(key1, key2, key3, output) \
    { .key = {{(key1).row, (key1).col}, {(key2).row, (key2).col}, {(key3).row, (key3).col}}, \
      .keycode = output, .count = 3 }

/** @brief Define a 4-key combo */
#define COMBO_4(key1, key2, key3, key4, output) \
    { .key = {{(key1).row, (key1).col}, {(key2).row, (key2).col}, {(key3).row, (key3).col}, {(key4).row, (key4).col}}, \
      .keycode = output, .count = 4 }

/** @} */

// Debug macros
/**
 * @defgroup DebugMacros Debug Macros
 * @{
 */

#if KEYBOARD_DEBUG
    /** @brief Print matrix state */
    #define KEYBOARD_DBG_MATRIX() matrix_print()

    /** @brief Print layer state */
    #define KEYBOARD_DBG_LAYER() layer_debug()

    /** @brief Print combo state */
    #define KEYBOARD_DBG_COMBO() combo_debug()

    /** @brief Print framework statistics */
    #define KEYBOARD_DBG_STATS() \
        do { \
            keyboard_framework_stats_t* stats = keyboard_framework_get_stats(); \
            dprintf("Matrix scans: %lu\n", stats->matrix_scans); \
            dprintf("Key events: %lu\n", stats->key_events_processed); \
            dprintf("Actions: %lu\n", stats->actions_executed); \
        } while(0)
#else
    #define KEYBOARD_DBG_MATRIX()
    #define KEYBOARD_DBG_LAYER()
    #define KEYBOARD_DBG_COMBO()
    #define KEYBOARD_DBG_STATS()
#endif

/** @} */

// Backward compatibility with existing code
/**
 * @defgroup Compatibility Compatibility Macros
 * @{
 */

// If someone is coming from QMK, provide some familiar macros
#ifdef QMK_COMPAT
    #define get_mods() get_mods()
    #define get_weak_mods() get_weak_mods()
    #define get_real_mods() get_real_mods()
    #define clear_mods() clear_mods()
    #define layer_state layer_state_get()
    #define default_layer_state default_layer_state_get()
#endif

/** @} */

// Version check
#if !defined(MATRIX_ROWS) || !defined(MATRIX_COLS)
    #error "MATRIX_ROWS and MATRIX_COLS must be defined before including keyboard.h"
#endif

// Feature requirements
#if defined(COMBO_ENABLE) && !defined(COMBO_COUNT)
    #warning "COMBO_ENABLE is defined but COMBO_COUNT is not defined. No combos will be processed."
#endif
