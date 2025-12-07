#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

// Keyboard matrix state
extern matrix_row_t matrix[MATRIX_ROWS];           /**< Current matrix state */
extern matrix_row_t raw_matrix[MATRIX_ROWS];       /**< Raw matrix state */
extern matrix_row_t debounce_matrix[MATRIX_ROWS];  /**< Debounced matrix state */

// Matrix configuration (from product_config.h)
#ifndef MATRIX_ROWS
#define MATRIX_ROWS 8
#endif

#ifndef MATRIX_COLS
#define MATRIX_COLS 13
#endif

#ifndef MATRIX_HAS_GHOST
#define MATRIX_HAS_GHOST 0
#endif

/**
 * @brief Initialize keyboard system
 */
void keyboard_init(void);

/**
 * @brief Main keyboard task loop
 *
 * This function should be called repeatedly in the main loop
 * It handles matrix scanning, debouncing, key processing, and action execution
 */
void keyboard_task(void);

/**
 * @brief Matrix scan function
 *
 * Scans the matrix and updates raw_matrix
 * Should be implemented by hardware-specific driver
 */
void matrix_scan(void);

/**
 * @brief Get a matrix row
 *
 * @param row Row index
 * @return Matrix row data
 */
matrix_row_t matrix_get_row(uint8_t row);

/**
 * @brief Set a matrix row
 *
 * @param row Row index
 * @param row_data Row data
 */
void matrix_set_row(uint8_t row, matrix_row_t row_data);

/**
 * @brief Matrix changed check
 *
 * @return true if matrix has changed
 */
bool matrix_has_changed(void);

/**
 * @brief Get matrix rows count
 *
 * @return Number of matrix rows
 */
uint8_t matrix_rows(void);

/**
 * @brief Get matrix columns count
 *
 * @return Number of matrix columns
 */
uint8_t matrix_cols(void);

/**
 * @brief Process key event from matrix
 *
 * @param row Row of the key
 * @param col Column of the key
 * @param pressed true if key is pressed, false if released
 */
void keyboard_process_key(uint8_t row, uint8_t col, bool pressed);

/**
 * @brief Housekeeping task
 *
 * Called on each iteration of the keyboard task loop
 * Can be used for background tasks and maintenance
 */
void housekeeping_task(void);

/**
 * @brief Print matrix state (for debugging)
 */
void matrix_print(void);

/**
 * @brief Check if matrix ghosting is present
 *
 * @param row Row to check
 * @param rowdata Row data
 * @return true if ghosting is detected
 */
bool matrix_has_ghost_in_row(uint8_t row, matrix_row_t rowdata);

#ifdef __cplusplus
}
#endif
