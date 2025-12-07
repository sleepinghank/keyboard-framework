#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

// Debounce algorithm types
typedef enum {
    DEBOUNCE_NONE = 0,                /**< No debouncing */
    DEBOUNCE_SYM_DEFER_PK,            /**< Symmetric defer with per-key tracking */
    DEBOUNCE_SYM_EAGER_PK,            /**< Symmetric eager with per-key tracking */
    DEBOUNCE_ASYM_EAGER_DEFER_PK,     /**< Asymmetric eager/defer with per-key tracking */
    DEBOUNCE_SYM_DEFER_G,             /**< Symmetric defer with global tracking */
    DEBOUNCE_SYM_DEFER_PR,            /**< Symmetric defer with per-row tracking */
    DEBOUNCE_SYM_EAGER_PR,            /**< Symmetric eager with per-row tracking */
} debounce_algorithm_t;

/**
 * @brief Debounce raw matrix events according to the chosen debounce algorithm.
 *
 * This function processes raw matrix data and applies debouncing to produce
 * a clean, debounced matrix state. Different algorithms can be selected to
 * optimize for different use cases (gaming, typing, etc.).
 *
 * @param raw The current raw matrix state (before debouncing)
 * @param cooked The debounced matrix state (after debouncing)
 * @param num_rows Number of rows in the matrix
 * @param changed True if raw matrix has changed since the last call
 * @return true If cooked matrix has new key changes after debouncing
 * @return false If cooked matrix is unchanged
 *
 * @note This function should be called on every matrix scan cycle
 * @note The cooked array must be pre-initialized with debounce_init()
 */
bool debounce(matrix_row_t raw[], matrix_row_t cooked[], uint8_t num_rows, bool changed);

/**
 * @brief Initialize the debounce module
 *
 * @param num_rows Number of rows in the matrix
 *
 * @note Must be called before using debounce()
 * @note Allocates internal state for debounce tracking
 */
void debounce_init(uint8_t num_rows);

/**
 * @brief Free debounce module resources
 *
 * @note Call when debounce is no longer needed
 * @note Clears all internal state
 */
void debounce_free(void);

/**
 * @brief Get the current debounced matrix state
 *
 * @return Pointer to the internal debounced matrix buffer
 *
 * @note This provides direct access to the internal debounced matrix
 * @note The returned pointer is only valid until debounce_free() is called
 * @note Useful for debugging and monitoring
 */
const matrix_row_t* debounce_get_matrix(void);

/**
 * @brief Check if the matrix has changed since the last debounce cycle
 *
 * @return true If the matrix has changed
 * @return false If the matrix is stable
 *
 * @note This checks if debounce has detected any key state changes
 * @note Useful for optimizing the key processing pipeline
 */
bool debounce_changed(void);

/**
 * @brief Reset the debounce state
 *
 * @note Clears all matrix states and timing information
 * @note Useful for reconnect scenarios or mode switches
 * @note Call when leaving deep sleep or after hardware reset
 */
void debounce_reset(void);

#ifdef __cplusplus
}
#endif
