/*
 * Copyright (C) 2024 Keyboard Framework
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "keyboard_core.h"
#include "matrix.h"
#include "debounce.h"
#include "layer.h"
#include "action.h"
#include "combo.h"
#include "custom_function.h"
#include "timer.h"
#include "print.h"
#include "debug.h"
#include <string.h>

// Matrix state
matrix_row_t matrix[MATRIX_ROWS] = {0};
matrix_row_t raw_matrix[MATRIX_ROWS] = {0};
matrix_row_t debounce_matrix[MATRIX_ROWS] = {0};

// Matrix state tracking
static matrix_row_t matrix_previous[MATRIX_ROWS] = {0};
static bool matrix_has_changed = false;

/**
 * @brief Initialize keyboard system
 */
void keyboard_init(void) {
    dprintf("Keyboard: Initializing...\n");

    // Initialize timer
    timer_init();

    // Initialize debounce
    debounce_init(MATRIX_ROWS);

    // Initialize layer system
    layer_init();

    // Initialize combo system
    combo_init();

    // Initialize custom functions
    #if TAP_DANCE_ENABLE
    tap_dance_init();
    #endif

    #if LEADER_KEY_ENABLE
    leader_init();
    #endif

    #if MACRO_ENABLE
    macro_init();
    #endif

    // Initialize matrix (hardware-specific)
    matrix_init();

    dprintf("Keyboard: Initialization complete\n");
}

/**
 * @brief Main keyboard task loop
 */
void keyboard_task(void) {
    static uint16_t last_tick = 0;

    // Generate tick event for internal state machine
    uint16_t now = timer_read();
    if (TIMER_DIFF_16(now, last_tick) > 0) {
        // Process tick event
        keyevent_t tick_event = MAKE_TICK_EVENT;
        action_exec(tick_event);
        last_tick = now;
    }

    // Scan matrix
    bool matrix_changed = false;
    matrix_scan();

    // Check for matrix changes
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        matrix_changed |= (matrix_previous[row] ^ matrix_get_row(row));
    }

    // Apply debouncing
    bool debounce_changed = false;
    if (matrix_changed) {
        // Copy current matrix to raw_matrix
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            raw_matrix[row] = matrix_get_row(row);
        }

        // Apply debouncing
        debounce_changed = debounce(raw_matrix, debounce_matrix, MATRIX_ROWS, matrix_changed);

        // Update previous matrix
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            matrix_previous[row] = matrix_get_row(row);
            matrix[row] = debounce_matrix[row];
        }

        matrix_has_changed = true;

        dprintf("Keyboard: Matrix changed, debounced: %s\n", debounce_changed ? "yes" : "no");
    } else {
        matrix_has_changed = false;
    }

    // Process matrix changes into key events
    if (matrix_has_changed) {
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            matrix_row_t row_changes = matrix[row] ^ matrix_previous[row];

            if (!row_changes || matrix_has_ghost_in_row(row, matrix[row])) {
                continue;
            }

            matrix_row_t col_mask = 1;
            for (uint8_t col = 0; col < MATRIX_COLS; col++, col_mask <<= 1) {
                if (row_changes & col_mask) {
                    bool key_pressed = matrix[row] & col_mask;
                    keyboard_process_key(row, col, key_pressed);
                }
            }

            matrix_previous[row] = matrix[row];
        }
    }

    // Process combos
    combo_task();

    // Process custom functions
    #if TAP_DANCE_ENABLE
    tap_dance_task();
    #endif

    #if LEADER_KEY_ENABLE
    leader_task();
    #endif

    #if MACRO_ENABLE
    macro_task();
    #endif

    // Run housekeeping tasks
    housekeeping_task();
}

/**
 * @brief Process a key event from the matrix
 *
 * @param row Row of the key
 * @param col Column of the key
 * @param pressed true if key is pressed, false if released
 */
void keyboard_process_key(uint8_t row, uint8_t col, bool pressed) {
    if (row >= MATRIX_ROWS || col >= MATRIX_COLS) {
        dprintf("Keyboard: ERROR - Invalid key position (%d, %d)\n", row, col);
        return;
    }

    keypos_t key = MAKE_KEYPOS(row, col);
    keyevent_t event = {
        .key = key,
        .time = timer_read(),
        .type = KEY_EVENT,
        .pressed = pressed
    };

    dprintf("Keyboard: Key event at (%d, %d) - %s\n", row, col, pressed ? "pressed" : "released");

    // Process combo events
    combo_event(event);

    // Process action
    action_exec(event);
}

/**
 * @brief Housekeeping task
 *
 * Background tasks and maintenance
 */
void housekeeping_task(void) {
    // This can be extended with additional background tasks
    // such as LED updates, display updates, etc.
}

/**
 * @brief Get a matrix row
 *
 * @param row Row index
 * @return Matrix row data
 */
matrix_row_t matrix_get_row(uint8_t row) {
    if (row >= MATRIX_ROWS) {
        return 0;
    }
    return matrix[row];
}

/**
 * @brief Set a matrix row
 *
 * @param row Row index
 * @param row_data Row data
 */
void matrix_set_row(uint8_t row, matrix_row_t row_data) {
    if (row >= MATRIX_ROWS) {
        return;
    }
    matrix[row] = row_data;
}

/**
 * @brief Matrix changed check
 *
 * @return true if matrix has changed
 */
bool matrix_has_changed(void) {
    return matrix_has_changed;
}

/**
 * @brief Get matrix rows count
 *
 * @return Number of matrix rows
 */
uint8_t matrix_rows(void) {
    return MATRIX_ROWS;
}

/**
 * @brief Get matrix columns count
 *
 * @return Number of matrix columns
 */
uint8_t matrix_cols(void) {
    return MATRIX_COLS;
}

/**
 * @brief Print matrix state for debugging
 */
void matrix_print(void) {
    dprintf("\nMatrix State:\n");
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        dprintf("Row %d: 0x%04X\n", row, matrix[row]);
    }
}

/**
 * @brief Check if matrix ghosting is present in a row
 *
 * @param row Row to check
 * @param rowdata Row data
 * @return true if ghosting is detected
 */
bool matrix_has_ghost_in_row(uint8_t row, matrix_row_t rowdata) {
    #if MATRIX_HAS_GHOST
    // Ghost occurs when a row has multiple keys pressed and
    // those keys share columns with keys pressed in other rows

    // Check if more than one key is pressed in this row
    matrix_row_t pressed_keys = rowdata;
    if (!(pressed_keys & (pressed_keys - 1))) {
        // Only one key or no keys pressed - no ghosting
        return false;
    }

    // Check for column conflicts with other rows
    for (uint8_t other_row = 0; other_row < MATRIX_ROWS; other_row++) {
        if (other_row == row) {
            continue;
        }

        matrix_row_t other_row_data = matrix[other_row];
        if (other_row_data & (other_row_data - 1)) {
            // Other row has multiple keys pressed
            if (pressed_keys & other_row_data) {
                // Found shared column - possible ghosting
                dprintf("Keyboard: Ghost detected at row %d\n", row);
                return true;
            }
        }
    }
    #endif

    return false;
}

// Weak matrix scan implementation (to be overridden by hardware driver)
__attribute__((weak)) void matrix_scan(void) {
    // This is a weak implementation
    // Hardware-specific drivers should override this
    // For now, just copy from previous state
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        matrix[row] = matrix_previous[row];
    }
}

__attribute__((weak)) void matrix_init(void) {
    // Hardware-specific initialization
    dprintf("Matrix: Using default init\n");
}
