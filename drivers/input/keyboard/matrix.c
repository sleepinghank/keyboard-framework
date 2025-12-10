/*
Copyright 2012-2018 Jun Wako, Jack Humbert, Yiancar

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
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "util.h"
#include "matrix.h"
#include "debounce.h"
#include "wait.h"
#include "print.h"

#  define ROWS_PER_HAND (MATRIX_ROWS)

pin_t row_pins[MATRIX_ROWS] = MATRIX_ROW_PINS;
pin_t col_pins[MATRIX_COLS] = MATRIX_COL_PINS;

/* matrix state(1:on, 0:off) */
extern matrix_row_t raw_matrix[MATRIX_ROWS]; // raw values
extern matrix_row_t matrix[MATRIX_ROWS];     // debounced values

// user-defined overridable functions
void matrix_init_pins(void);
void matrix_read_cols_on_row(matrix_row_t current_matrix[], uint8_t current_row);
void matrix_read_rows_on_col(matrix_row_t current_matrix[], uint8_t current_col, matrix_row_t row_shifter);


static inline void setPinOutput_writeLow(pin_t pin) {
    setPinOutput(pin);
    writePinLow(pin);
}

static inline void setPinOutput_writeHigh(pin_t pin) {
    setPinOutput(pin);
    writePinHigh(pin);
}

static inline void setPinInputHigh_atomic(pin_t pin) {
    setPinInputHigh(pin);
}

static inline uint8_t readMatrixPin(pin_t pin) {
    if (pin != NO_PIN) {
        return (readPin(pin) == MATRIX_INPUT_PRESSED_STATE) ? 0 : 1;
    } else {
        return 1;
    }
}
static bool select_col(uint8_t col) {
    pin_t pin = col_pins[col];
    if (pin != NO_PIN) {
        setPinOutput_writeLow(pin);
        return true;
    }
    return false;
}

static void unselect_col(uint8_t col) {
    pin_t pin = col_pins[col];
    if (pin != NO_PIN) {
        setPinOutput_writeHigh(pin);
    }
}

static void unselect_cols(void) {
    for (uint8_t x = 0; x < MATRIX_COLS; x++) {
        unselect_col(x);
    }
}

void matrix_init_pins(void) {
    unselect_cols();
    for (uint8_t x = 0; x < ROWS_PER_HAND; x++) {
        if (row_pins[x] != NO_PIN) {
            setPinInputHigh_atomic(row_pins[x]);
        }
    }
}

void matrix_read_rows_on_col(matrix_row_t current_matrix[], uint8_t current_col, matrix_row_t row_shifter) {
    bool key_pressed = false;

    // Select col
    if (!select_col(current_col)) { // select col
        return;                     // skip NO_PIN col
    }
    matrix_output_select_delay();
    // For each row...
    for (uint8_t row_index = 0; row_index < ROWS_PER_HAND; row_index++) {
        // Check row pin state
        if (readMatrixPin(row_pins[row_index]) == 0) {
            // Pin LO, set col bit
            current_matrix[row_index] |= row_shifter;
            key_pressed = true;
        } else {
            // Pin HI, clear col bit
            current_matrix[row_index] &= ~row_shifter;
        }
    }

    // Unselect col
    unselect_col(current_col);
    matrix_output_unselect_delay(current_col, key_pressed); // wait for all Row signals to go HIGH
}

void matrix_init(void) {

    // initialize key pins
    matrix_init_pins();

    // initialize matrix state: all keys off
    memset(matrix, 0, sizeof(matrix));
    memset(raw_matrix, 0, sizeof(raw_matrix));

    debounce_init(ROWS_PER_HAND);

}


uint8_t matrix_scan(void) {
    matrix_row_t curr_matrix[MATRIX_ROWS] = {0};

    // Set col, read rows
    matrix_row_t row_shifter = MATRIX_ROW_SHIFTER;
    for (uint8_t current_col = 0; current_col < MATRIX_COLS; current_col++, row_shifter <<= 1) {
        matrix_read_rows_on_col(curr_matrix, current_col, row_shifter);
    }

    bool changed = memcmp(raw_matrix, curr_matrix, sizeof(curr_matrix)) != 0;
    if (changed) memcpy(raw_matrix, curr_matrix, sizeof(curr_matrix));

    changed = debounce(raw_matrix, matrix, ROWS_PER_HAND, changed);
    // matrix_scan_kb();

    return (uint8_t)changed;
}




#    define ROWS_PER_HAND (MATRIX_ROWS)


#ifndef MATRIX_IO_DELAY
#    define MATRIX_IO_DELAY 30
#endif

/* matrix state(1:on, 0:off) */
matrix_row_t raw_matrix[MATRIX_ROWS];
matrix_row_t matrix[MATRIX_ROWS];

#ifdef SPLIT_KEYBOARD
// row offsets for each hand
uint8_t thisHand, thatHand;
#endif

#ifdef MATRIX_MASKED
extern const matrix_row_t matrix_mask[];
#endif

// user-defined overridable functions

//__attribute__((weak)) void matrix_init_kb(void) {
//    matrix_init_user();
//}
//
//__attribute__((weak)) void matrix_scan_kb(void) {
//    matrix_scan_user();
//}
//
//__attribute__((weak)) void matrix_init_user(void) {}
//
//__attribute__((weak)) void matrix_scan_user(void) {}

// helper functions

inline uint8_t matrix_rows(void) {
    return MATRIX_ROWS;
}

inline uint8_t matrix_cols(void) {
    return MATRIX_COLS;
}

inline bool matrix_is_on(uint8_t row, uint8_t col) {
    return (matrix[row] & ((matrix_row_t)1 << col));
}

inline matrix_row_t matrix_get_row(uint8_t row) {
    // Matrix mask lets you disable switches in the returned matrix data. For example, if you have a
    // switch blocker installed and the switch is always pressed.
#ifdef MATRIX_MASKED
    return matrix[row] & matrix_mask[row];
#else
    return matrix[row];
#endif
}

#if (MATRIX_COLS <= 8)
#    define print_matrix_header() print("\nr/c 01234567\n")
#    define print_matrix_row(row) print_bin_reverse8(matrix_get_row(row))
#elif (MATRIX_COLS <= 16)
#    define print_matrix_header() print("\nr/c 0123456789ABCDEF\n")
#    define print_matrix_row(row) print_bin_reverse16(matrix_get_row(row))
#elif (MATRIX_COLS <= 32)
#    define print_matrix_header() print("\nr/c 0123456789ABCDEF0123456789ABCDEF\n")
#    define print_matrix_row(row) print_bin_reverse32(matrix_get_row(row))
#endif

void matrix_print(void) {
    print_matrix_header();

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        print_hex8(row);
        print(": ");
        print_matrix_row(row);
        print("\n");
    }
}


/* `matrix_io_delay ()` exists for backwards compatibility. From now on, use matrix_output_unselect_delay(). */
void matrix_io_delay(void) {
    wait_us(MATRIX_IO_DELAY);
}
void matrix_output_select_delay(void) {
    matrix_io_delay();
}
void matrix_output_unselect_delay(uint8_t line, bool key_pressed) {
    matrix_io_delay();
}

// CUSTOM MATRIX 'LITE'
void matrix_init_custom(void) {}
bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    return true;
}

bool peek_matrix(uint8_t row_index, uint8_t col_index, bool raw) {
    return 0 != ((raw ? raw_matrix[row_index] : matrix[row_index]) & (MATRIX_ROW_SHIFTER << col_index));
}
