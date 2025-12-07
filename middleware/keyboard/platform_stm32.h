#pragma once

#ifdef PLATFORM_CHIPSTM32

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief STM32-specific matrix initialization
 */
void platform_matrix_init_stm32(void);

/**
 * @brief STM32-specific matrix scan
 *
 * Reads GPIO pins and updates the matrix state
 */
void platform_matrix_scan_stm32(matrix_row_t matrix[]);

/**
 * @brief STM32 GPIO configuration for keyboard matrix
 */
void platform_gpio_init_stm32(void);

/**
 * @brief STM32 timer configuration
 */
void platform_timer_init_stm32(void);

#ifdef __cplusplus
}
#endif

#endif // PLATFORM_CHIPSTM32
