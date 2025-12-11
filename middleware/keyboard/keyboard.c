/*
 * Copyright (C) 2024 Keyboard Framework
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "keyboard.h"
#include "../../drivers/input/keyboard/matrix.h"
#include "../../drivers/input/keyboard/debounce.h"
#include "action_layer.h"
#include "action.h"
#include "keymap_common.h"
#include "combo.h"
#include "custom_function.h"
#include "timer.h"
#include "print.h"
#include "debug.h"
#include <string.h>
#include "keymap_introspection.h"

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

    #ifdef COMBO_ENABLE
    // Initialize combo system
    combo_init();
    #endif

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
static matrix_row_t get_real_keys(uint8_t row, matrix_row_t rowdata) {
    matrix_row_t out = 0;
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        // read each key in the row data and check if the keymap defines it as a real key
        if (keycode_at_keymap_location(0, row, col) && (rowdata & (((matrix_row_t)1) << col))) {
            // this creates new row data, if a key is defined in the keymap, it will be set here
            out |= ((matrix_row_t)1) << col;
        }
    }
    return out;
}

static inline bool popcount_more_than_one(matrix_row_t rowdata) {
    rowdata &= rowdata - 1; // if there are less than two bits (keys) set, rowdata will become zero
    return rowdata;
}

static inline bool has_ghost_in_row(uint8_t row, matrix_row_t rowdata) {
    /* No ghost exists when less than 2 keys are down on the row.
    If there are "active" blanks in the matrix, the key can't be pressed by the user,
    there is no doubt as to which keys are really being pressed.
    The ghosts will be ignored, they are KC_NO.   */
    rowdata = get_real_keys(row, rowdata);
    if ((popcount_more_than_one(rowdata)) == 0) {
        return false;
    }
    /* Ghost occurs when the row shares a column line with other row,
    and two columns are read on each row. Blanks in the matrix don't matter,
    so they are filtered out.
    If there are two or more real keys pressed and they match columns with
    at least two of another row's real keys, the row will be ignored. Keep in mind,
    we are checking one row at a time, not all of them at once.
    */
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        if (i != row && popcount_more_than_one(get_real_keys(i, matrix_get_row(i)) & rowdata)) {
            return true;
        }
    }
    return false;
}

/**
 * @brief This task scans the keyboards matrix and processes any key presses
 * that occur.
 *
 * @return true Matrix did change
 * @return false Matrix didn't change
 */
static bool matrix_task(void) {
//    if (!matrix_can_read()) {
////        generate_tick_event();
//        return false;
//    }

    static matrix_row_t matrix_previous[MATRIX_ROWS];

    matrix_scan();
    bool matrix_changed = false;
    for (uint8_t row = 0; row < MATRIX_ROWS && !matrix_changed; row++) {
        matrix_changed |= matrix_previous[row] ^ matrix_get_row(row);
    }

//    matrix_scan_perf_task();

    // Short-circuit the complete matrix processing if it is not necessary
    if (!matrix_changed) {
//        generate_tick_event();
        return matrix_changed;
    }

    if (debug_config.matrix) {
        matrix_print();
    }


    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        const matrix_row_t current_row = matrix_get_row(row);
        const matrix_row_t row_changes = current_row ^ matrix_previous[row];

        if (!row_changes || has_ghost_in_row(row, current_row)) {
            continue;
        }

        matrix_row_t col_mask = 1;
        for (uint8_t col = 0; col < MATRIX_COLS; col++, col_mask <<= 1) {
            if (row_changes & col_mask) {
                const bool key_pressed = current_row & col_mask;

                action_exec(MAKE_KEYEVENT(row, col, key_pressed));

//                switch_events(row, col, key_pressed);
            }
        }

        matrix_previous[row] = current_row;
    }

    return matrix_changed;
}

/**
 * @brief Main keyboard task loop
 */
void keyboard_task(void) {
    __attribute__((unused)) bool activity_has_occurred = false;
    if (matrix_task()) {
//        last_matrix_activity_trigger();
        activity_has_occurred = true;
    }
    #ifdef COMBO_ENABLE
    // Process combos
    combo_task();
    #endif
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

    #ifdef COMBO_ENABLE
    // Process combo events
    combo_event(event);

    #endif
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
