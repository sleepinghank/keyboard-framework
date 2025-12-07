/*
 * Copyright (C) 2024 Keyboard Framework
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Cross-platform keyboard middleware framework
 * Supports multiple chips: PAN32860, CH584, Nordic series
 */

#include "keyboard_framework.h"
#include "timer.h"
#include "print.h"
#include <string.h>

// Framework statistics
static keyboard_framework_stats_t framework_stats = {0};

/**
 * @brief Initialize the entire keyboard framework
 *
 * @return 0 on success, negative error code on failure
 */
int keyboard_framework_init(void) {
    dprintf("\n");
    dprintf("========================================\n");
    dprintf("Keyboard Framework v%d.%d.%d\n",
            KEYBOARD_FRAMEWORK_VERSION_MAJOR,
            KEYBOARD_FRAMEWORK_VERSION_MINOR,
            KEYBOARD_FRAMEWORK_VERSION_PATCH);
    dprintf("Platform: ");
    #ifdef PLATFORM_CHIPSTM32
        dprintf("STM32\n");
    #elif defined(PLATFORM_CHIPWCH)
        dprintf("WCH CH58x\n");
    #elif defined(PLATFORM_NORDIC)
        dprintf("Nordic nRF5x\n");
    #else
        dprintf("Unknown\n");
    #endif
    dprintf("========================================\n");

    // Initialize timer
    timer_init();
    dprintf("[INIT] Timer initialized\n");

    // Initialize core systems
    #if DEBOUNCE_ENABLE
    debounce_init(MATRIX_ROWS);
    dprintf("[INIT] Debounce module initialized\n");
    #endif

    #if LAYER_ENABLE
    layer_init();
    dprintf("[INIT] Layer module initialized\n");
    #endif

    #if COMBO_ENABLE
    combo_init();
    dprintf("[INIT] Combo module initialized\n");
    #endif

    #if TAP_DANCE_ENABLE
    tap_dance_init();
    dprintf("[INIT] Tap Dance module initialized\n");
    #endif

    #if LEADER_KEY_ENABLE
    leader_init();
    dprintf("[INIT] Leader Key module initialized\n");
    #endif

    #if MACRO_ENABLE
    macro_init();
    dprintf("[INIT] Macro module initialized\n");
    #endif

    // Initialize matrix hardware
    matrix_init();
    dprintf("[INIT] Matrix hardware initialized\n");

    dprintf("========================================\n");
    dprintf("Framework initialization complete!\n");
    dprintf("========================================\n\n");

    return 0;
}

/**
 * @brief Main framework task loop
 */
void keyboard_framework_task(void) {
    static uint32_t last_scan_count = 0;

    // Scan matrix
    matrix_scan();
    framework_stats.matrix_scans++;

    // Check for changes
    bool matrix_changed = false;
    static matrix_row_t previous_matrix[MATRIX_ROWS];

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        if (matrix[row] != previous_matrix[row]) {
            matrix_changed = true;
            previous_matrix[row] = matrix[row];
        }
    }

    // Apply debouncing if enabled
    #if DEBOUNCE_ENABLE
    bool debounce_result = false;
    if (matrix_changed) {
        // Copy to raw matrix
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            raw_matrix[row] = matrix[row];
        }

        // Debounce
        debounce_result = debounce(raw_matrix, debounce_matrix, MATRIX_ROWS, matrix_changed);

        // Update current matrix
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            matrix[row] = debounce_matrix[row];
        }
    }
    #else
    // No debouncing - use raw matrix
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        raw_matrix[row] = matrix[row];
    }
    #endif

    // Process key events
    if (matrix_changed) {
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            matrix_row_t changes = matrix[row] ^ previous_matrix[row];

            if (!changes) {
                continue;
            }

            // Check for ghosting
            #if MATRIX_HAS_GHOST
            if (matrix_has_ghost_in_row(row, matrix[row])) {
                continue;
            }
            #endif

            // Process column changes
            matrix_row_t col_mask = 1;
            for (uint8_t col = 0; col < MATRIX_COLS; col++, col_mask <<= 1) {
                if (changes & col_mask) {
                    bool pressed = matrix[row] & col_mask;

                    #if COMBO_ENABLE
                    keyevent_t event = MAKE_KEYEVENT(row, col, pressed);
                    combo_event(event);
                    #endif

                    action_exec(MAKE_KEYEVENT(row, col, pressed));
                    framework_stats.key_events_processed++;
                }
            }
        }
    }

    // Run module tasks
    #if COMBO_ENABLE
    combo_task();
    #endif

    #if TAP_DANCE_ENABLE
    tap_dance_task();
    #endif

    #if LEADER_KEY_ENABLE
    leader_task();
    #endif

    #if MACRO_ENABLE
    macro_task();
    #endif

    // Housekeeping
    housekeeping_task();
}

/**
 * @brief Get framework version string
 *
 * @return Version string
 */
const char* keyboard_framework_version(void) {
    static char version[32];
    snprintf(version, sizeof(version), "%d.%d.%d",
             KEYBOARD_FRAMEWORK_VERSION_MAJOR,
             KEYBOARD_FRAMEWORK_VERSION_MINOR,
             KEYBOARD_FRAMEWORK_VERSION_PATCH);
    return version;
}

/**
 * @brief Get supported platform name
 *
 * @return Platform name string
 */
const char* keyboard_framework_platform(void) {
    #ifdef PLATFORM_CHIPSTM32
        return "STM32";
    #elif defined(PLATFORM_CHIPWCH)
        return "WCH CH58x";
    #elif defined(PLATFORM_NORDIC)
        return "Nordic nRF5x";
    #else
        return "Unknown";
    #endif
}

/**
 * @brief Reset framework to initial state
 */
void keyboard_framework_reset(void) {
    // Reset all modules
    #if DEBOUNCE_ENABLE
    debounce_reset();
    #endif

    #if LAYER_ENABLE
    layer_clear();
    #endif

    #if COMBO_ENABLE
    combo_clear();
    #endif

    #if TAP_DANCE_ENABLE
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            tap_dance_reset(MAKE_KEYPOS(row, col));
        }
    }
    #endif

    // Clear matrix
    memset(matrix, 0, sizeof(matrix));
    memset(raw_matrix, 0, sizeof(raw_matrix));
    memset(debounce_matrix, 0, sizeof(debounce_matrix));

    // Clear modifiers and key states
    clear_keyboard();

    KEYBOARD_DPRINTF("Framework reset complete\n");
}

/**
 * @brief Get framework statistics
 *
 * @return Pointer to statistics structure
 */
const keyboard_framework_stats_t* keyboard_framework_get_stats(void) {
    return &framework_stats;
}

/**
 * @brief Reset framework statistics
 */
void keyboard_framework_reset_stats(void) {
    memset(&framework_stats, 0, sizeof(framework_stats));
    KEYBOARD_DPRINTF("Framework statistics reset\n");
}
