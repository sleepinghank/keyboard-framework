#pragma once

#ifdef PLATFORM_NORDIC

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Nordic nRF5x-specific matrix initialization
 */
void platform_matrix_init_nordic(void);

/**
 * @brief Nordic nRF5x-specific matrix scan
 */
void platform_matrix_scan_nordic(matrix_row_t matrix[]);

/**
 * @brief Nordic nRF5x GPIO configuration
 */
void platform_gpio_init_nordic(void);

/**
 * @brief Nordic nRF5x timer configuration
 */
void platform_timer_init_nordic(void);

#ifdef __cplusplus
}
#endif

#endif // PLATFORM_NORDIC
