/*
 * Copyright (C) 2024 Keyboard Framework
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "debounce.h"
#include "timer.h"
#include "print.h"
#include <string.h>
#include "debug.h"

// Debounce algorithm selection
#ifndef DEBOUNCE_ALGORITHM
#define DEBOUNCE_ALGORITHM DEBOUNCE_SYM_DEFER_PK
#endif

// Debounce delay in milliseconds
#ifndef DEBOUNCE_DELAY
#define DEBOUNCE_DELAY 5
#endif

// Maximum number of rows in the matrix
#ifndef MAX_MATRIX_ROWS
#define MAX_MATRIX_ROWS 16
#endif

// Debounce state tracking
typedef struct {
    matrix_row_t matrix_row[MAX_MATRIX_ROWS];    // Debounced matrix state
    matrix_row_t raw_matrix_row[MAX_MATRIX_ROWS]; // Raw matrix state
    uint16_t last_change_time[MAX_MATRIX_ROWS];   // Last change timestamp
    uint8_t num_rows;                             // Number of matrix rows
    bool changed;                                 // Whether matrix has changed
} debounce_state_t;

static debounce_state_t debounce_state = {0};

/**
 * @brief Initialize debounce module
 *
 * @param num_rows Number of rows in the matrix
 */
void debounce_init(uint8_t num_rows) {
    if (num_rows > MAX_MATRIX_ROWS) {
        dprintf("Debounce: ERROR - num_rows (%d) exceeds MAX_MATRIX_ROWS (%d)\n", num_rows, MAX_MATRIX_ROWS);
        num_rows = MAX_MATRIX_ROWS;
    }

    debounce_state.num_rows = num_rows;
    memset(debounce_state.matrix_row, 0, sizeof(debounce_state.matrix_row));
    memset(debounce_state.raw_matrix_row, 0, sizeof(debounce_state.raw_matrix_row));
    memset(debounce_state.last_change_time, 0, sizeof(debounce_state.last_change_time));
    debounce_state.changed = false;

    dprintf("Debounce: Initialized for %d rows\n", num_rows);
}

/**
 * @brief Free debounce module resources
 */
void debounce_free(void) {
    memset(&debounce_state, 0, sizeof(debounce_state));
    dprintf("Debounce: Freed\n");
}

/**
 * @brief Symmetric defer with per-key tracking algorithm
 *
 * This is the most common debounce algorithm in QMK.
 * Each key is tracked individually and debounced based on its own timing.
 *
 * @param raw The current raw matrix state
 * @param cooked The debounced matrix state
 * @param num_rows Number of rows
 * @param changed Whether raw matrix has changed
 * @return true If cooked matrix has new changes
 */
static bool debounce_symmetric_defer_pk(matrix_row_t raw[], matrix_row_t cooked[], uint8_t num_rows, bool changed) {
    bool matrix_changed = false;
    uint16_t now = timer_read();
    for (uint8_t row = 0; row < num_rows; row++) {
        matrix_row_t row_changed = raw[row] ^ cooked[row];

        // Process each bit (column) in the row
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            matrix_row_t col_mask = (matrix_row_t)1 << col;

            if (row_changed & col_mask) {
                bool raw_pressed = raw[row] & col_mask;
                bool cooked_pressed = cooked[row] & col_mask;

                // Check if enough time has elapsed since last change
                uint16_t elapsed = now - debounce_state.last_change_time[row];

                if (elapsed >= DEBOUNCE_DELAY) {
                    // Update the cooked matrix
                    if (raw_pressed) {
                        cooked[row] |= col_mask;
                    } else {
                        cooked[row] &= ~col_mask;
                    }

                    // Update last change time
                    debounce_state.last_change_time[row] = now;

                    // Mark matrix as changed
                    if (cooked_pressed != raw_pressed) {
                        matrix_changed = true;
                    }
                }
            }
        }
    }

    return matrix_changed;
}

/**
 * @brief Symmetric eager algorithm
 *
 * Updates immediately on press, waits on release.
 * Good for gaming where immediate response on key press is critical.
 *
 * @param raw The current raw matrix state
 * @param cooked The debounced matrix state
 * @param num_rows Number of rows
 * @param changed Whether raw matrix has changed
 * @return true If cooked matrix has new changes
 */
static bool debounce_symmetric_eager_pk(matrix_row_t raw[], matrix_row_t cooked[], uint8_t num_rows, bool changed) {
    bool matrix_changed = false;
    uint16_t now = timer_read();

    for (uint8_t row = 0; row < num_rows; row++) {
        matrix_row_t row_changed = raw[row] ^ cooked[row];

        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            matrix_row_t col_mask = (matrix_row_t)1 << col;

            if (row_changed & col_mask) {
                bool raw_pressed = raw[row] & col_mask;
                bool cooked_pressed = cooked[row] & col_mask;

                // Eager: update immediately on key press
                if (raw_pressed) {
                    cooked[row] |= col_mask;
                    matrix_changed = true;
                } else {
                    // Conservative: wait on key release
                    uint16_t elapsed = now - debounce_state.last_change_time[row];
                    if (elapsed >= DEBOUNCE_DELAY) {
                        cooked[row] &= ~col_mask;
                        matrix_changed = true;
                    }
                }

                debounce_state.last_change_time[row] = now;
            }
        }
    }

    return matrix_changed;
}

/**
 * @brief Asymmetric eager algorithm
 *
 * Different behavior for press vs release, can be configured per requirements.
 *
 * @param raw The current raw matrix state
 * @param cooked The debounced matrix state
 * @param num_rows Number of rows
 * @param changed Whether raw matrix has changed
 * @return true If cooked matrix has new changes
 */
static bool debounce_asymmetric_eager_defer_pk(matrix_row_t raw[], matrix_row_t cooked[], uint8_t num_rows, bool changed) {
    bool matrix_changed = false;
    uint16_t now = timer_read();

    // Define custom debounce delays for press vs release
    #ifndef DEBOUNCE_PRESS_DELAY
    #define DEBOUNCE_PRESS_DELAY 0  // No delay on press
    #endif

    #ifndef DEBOUNCE_RELEASE_DELAY
    #define DEBOUNCE_RELEASE_DELAY DEBOUNCE_DELAY  // Default delay on release
    #endif

    for (uint8_t row = 0; row < num_rows; row++) {
        matrix_row_t row_changed = raw[row] ^ cooked[row];

        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            matrix_row_t col_mask = (matrix_row_t)1 << col;

            if (row_changed & col_mask) {
                bool raw_pressed = raw[row] & col_mask;
                bool cooked_pressed = cooked[row] & col_mask;

                // Check elapsed time
                uint16_t elapsed = now - debounce_state.last_change_time[row];
                uint16_t required_delay = raw_pressed ? DEBOUNCE_PRESS_DELAY : DEBOUNCE_RELEASE_DELAY;

                if (elapsed >= required_delay) {
                    if (raw_pressed) {
                        cooked[row] |= col_mask;
                    } else {
                        cooked[row] &= ~col_mask;
                    }

                    if (cooked_pressed != raw_pressed) {
                        matrix_changed = true;
                    }
                }

                debounce_state.last_change_time[row] = now;
            }
        }
    }

    return matrix_changed;
}

/**
 * @brief Main debounce function
 *
 * @param raw The current raw matrix state
 * @param cooked The debounced matrix state
 * @param num_rows Number of rows to debounce
 * @param changed True if raw has changed since the last call
 * @return true Cooked has new keychanges after debouncing
 * @return false Cooked is the same as before
 */
bool debounce(matrix_row_t raw[], matrix_row_t cooked[], uint8_t num_rows, bool changed) {
    // Validate input
    if (!raw || !cooked || num_rows == 0 || num_rows > MAX_MATRIX_ROWS) {
        dprintf("Debounce: ERROR - Invalid parameters\n");
        return false;
    }

    // Store raw matrix for state tracking
    if (changed) {
        for (uint8_t row = 0; row < num_rows; row++) {
            debounce_state.raw_matrix_row[row] = raw[row];
        }
    }

    // Perform debounce based on selected algorithm
    bool matrix_changed = false;

    switch (DEBOUNCE_ALGORITHM) {
        case DEBOUNCE_NONE:
            // No debouncing - pass raw through
            memcpy(cooked, raw, num_rows * sizeof(matrix_row_t));
            matrix_changed = changed;
            break;

        case DEBOUNCE_SYM_DEFER_PK:
            matrix_changed = debounce_symmetric_defer_pk(raw, cooked, num_rows, changed);
            break;

        case DEBOUNCE_SYM_EAGER_PK:
            matrix_changed = debounce_symmetric_eager_pk(raw, cooked, num_rows, changed);
            break;

        case DEBOUNCE_ASYM_EAGER_DEFER_PK:
            matrix_changed = debounce_asymmetric_eager_defer_pk(raw, cooked, num_rows, changed);
            break;

        default:
            dprintf("Debounce: ERROR - Unknown algorithm %d, using DEBOUNCE_SYM_DEFER_PK\n", DEBOUNCE_ALGORITHM);
            matrix_changed = debounce_symmetric_defer_pk(raw, cooked, num_rows, changed);
            break;
    }

    debounce_state.changed = matrix_changed;

    #ifdef DEBUG_DEBOUNCE
    if (matrix_changed) {
        dprintf("Debounce: Matrix changed\n");
        for (uint8_t row = 0; row < num_rows; row++) {
            if (cooked[row] != 0) {
                dprintf("  Row %d: 0x%04X\n", row, cooked[row]);
            }
        }
    }
    #endif

    return matrix_changed;
}

/**
 * @brief Get the current debounced matrix state
 *
 * @return Pointer to the current debounced matrix
 */
const matrix_row_t* debounce_get_matrix(void) {
    return debounce_state.matrix_row;
}

/**
 * @brief Check if matrix has changed since last debounce
 *
 * @return true if matrix has changed
 */
bool debounce_changed(void) {
    return debounce_state.changed;
}

/**
 * @brief Reset debounce state
 *
 * Clears all matrix states and timing information
 */
void debounce_reset(void) {
    memset(debounce_state.matrix_row, 0, sizeof(debounce_state.matrix_row));
    memset(debounce_state.raw_matrix_row, 0, sizeof(debounce_state.raw_matrix_row));
    memset(debounce_state.last_change_time, 0, sizeof(debounce_state.last_change_time));
    debounce_state.changed = false;
    dprintf("Debounce: Reset\n");
}
