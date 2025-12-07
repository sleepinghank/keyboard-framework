/*
 * Copyright (C) 2024 Keyboard Framework
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Cross-platform keyboard middleware framework
 * Supports multiple chips: PAN32860, CH584, Nordic series
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

// Core modules
#include "keyboard_core.h"
#include "keycode.h"
#include "layer.h"
#include "debounce.h"
#include "action.h"
#include "combo.h"
#include "custom_function.h"

#ifdef __cplusplus
extern "C" {
#endif

// Framework version
#define KEYBOARD_FRAMEWORK_VERSION_MAJOR 1
#define KEYBOARD_FRAMEWORK_VERSION_MINOR 0
#define KEYBOARD_FRAMEWORK_VERSION_PATCH 0

// Supported platform detection
#if defined(__arm__)
    #if defined(STM32F103xB) || defined(STM32F303xC)
        #define PLATFORM_CHIPSTM32
    #elif defined(CH579) || defined(CH584)
        #define PLATFORM_CHIPWCH
    #elif defined(NRF52) || defined(NRF52840) || defined(NRF52833)
        #define PLATFORM_NORDIC
    #else
        #define PLATFORM_UNKNOWN
    #endif
#elif defined(__riscv)
    #define PLATFORM_RISCV
#else
    #define PLATFORM_UNKNOWN
#endif

// Feature flags (from product_config.h)
#ifndef DEBOUNCE_ENABLE
#define DEBOUNCE_ENABLE 1
#endif

#ifndef LAYER_ENABLE
#define LAYER_ENABLE 1
#endif

#ifndef COMBO_ENABLE
#define COMBO_ENABLE 0
#endif

#ifndef TAP_DANCE_ENABLE
#define TAP_DANCE_ENABLE 0
#endif

#ifndef LEADER_KEY_ENABLE
#define LEADER_KEY_ENABLE 0
#endif

#ifndef MACRO_ENABLE
#define MACRO_ENABLE 0
#endif

/**
 * @brief Initialize the entire keyboard framework
 *
 * This function should be called once at system startup
 * It initializes all modules in the correct order
 *
 * @return 0 on success, negative error code on failure
 */
int keyboard_framework_init(void);

/**
 * @brief Main framework task loop
 *
 * This function should be called repeatedly in the main application loop
 * It handles all keyboard processing
 *
 * @note This replaces the individual keyboard_task() call
 */
void keyboard_framework_task(void);

/**
 * @brief Get framework version string
 *
 * @return Version string
 */
const char* keyboard_framework_version(void);

/**
 * @brief Get supported platform name
 *
 * @return Platform name string
 */
const char* keyboard_framework_platform(void);

/**
 * @brief Reset framework to initial state
 *
 * Clears all key states, layers, and internal buffers
 */
void keyboard_framework_reset(void);

// Convenience macros for common operations

/**
 * @brief Send a keycode
 */
#define KEYBOARD_SEND_KEYCODE(code) do { \
    register_code(code); \
    unregister_code(code); \
} while(0)

/**
 * @brief Press and hold a keycode
 */
#define KEYBOARD_PRESS_KEYCODE(code) register_code(code)

/**
 * @brief Release a keycode
 */
#define KEYBOARD_RELEASE_KEYCODE(code) unregister_code(code)

/**
 * @brief Activate a layer
 */
#define KEYBOARD_LAYER_ON(layer) layer_on(layer)

/**
 * @brief Deactivate a layer
 */
#define KEYBOARD_LAYER_OFF(layer) layer_off(layer)

/**
 * @brief Toggle a layer
 */
#define KEYBOARD_LAYER_TOGGLE(layer) layer_invert(layer)

/**
 * @brief Get current layer state
 */
#define KEYBOARD_LAYER_STATE() layer_state_get()

/**
 * @brief Check if a layer is active
 */
#define KEYBOARD_LAYER_IS_ACTIVE(layer) layer_state_is(layer)

/**
 * @brief Clear all layers
 */
#define KEYBOARD_LAYER_CLEAR() layer_clear()

/**
 * @brief Get current modifier state
 */
#define KEYBOARD_GET_MODS() get_mods()

/**
 * @brief Register modifiers
 */
#define KEYBOARD_REGISTER_MODS(mods) register_mods(mods)

/**
 * @brief Unregister modifiers
 */
#define KEYBOARD_UNREGISTER_MODS(mods) unregister_mods(mods)

/**
 * @brief Clear all modifiers
 */
#define KEYBOARD_CLEAR_MODS() clear_mods()

// Platform-specific helpers

#ifdef PLATFORM_CHIPSTM32
    #include "platform_stm32.h"
#elif defined(PLATFORM_CHIPWCH)
    #include "platform_wch.h"
#elif defined(PLATFORM_NORDIC)
    #include "platform_nordic.h"
#endif

// Debug and configuration

/**
 * @brief Enable framework debugging
 */
#ifndef KEYBOARD_DEBUG
#define KEYBOARD_DEBUG 0
#endif

#if KEYBOARD_DEBUG
    #define KEYBOARD_DPRINTF(fmt, ...) dprintf("[KB] " fmt, ##__VA_ARGS__)
#else
    #define KEYBOARD_DPRINTF(fmt, ...) do {} while(0)
#endif

/**
 * @brief Framework statistics
 */
typedef struct {
    uint32_t matrix_scans;          /**< Total matrix scans performed */
    uint32_t key_events_processed;  /**< Total key events processed */
    uint32_t actions_executed;      /**< Total actions executed */
    uint32_t combos_triggered;      /**< Total combos triggered */
    uint32_t layers_activated;      /**< Total layer activations */
} keyboard_framework_stats_t;

/**
 * @brief Get framework statistics
 *
 * @return Pointer to statistics structure
 */
const keyboard_framework_stats_t* keyboard_framework_get_stats(void);

/**
 * @brief Reset framework statistics
 */
void keyboard_framework_reset_stats(void);

#ifdef __cplusplus
}
#endif

// Main API - Simple initialization macro for users
/**
 * @brief Simple initialization for most use cases
 *
 * Usage:
 *   int main(void) {
 *       KEYBOARD_FRAMEWORK_INIT();
 *
 *       while (1) {
 *           KEYBOARD_FRAMEWORK_TASK();
 *       }
 *   }
 */
#define KEYBOARD_FRAMEWORK_INIT() \
    do { \
        if (keyboard_framework_init() != 0) { \
            return -1; \
        } \
    } while(0)

#define KEYBOARD_FRAMEWORK_TASK() \
    do { \
        keyboard_framework_task(); \
    } while(0)
