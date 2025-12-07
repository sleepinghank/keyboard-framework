#pragma once

#ifdef PLATFORM_CHIPWCH

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief WCH CH58x-specific matrix initialization
 */
void platform_matrix_init_wch(void);

/**
 * @brief WCH CH58x-specific matrix scan
 */
void platform_matrix_scan_wch(matrix_row_t matrix[]);

/**
 * @brief WCH CH58x GPIO configuration
 */
void platform_gpio_init_wch(void);

/**
 * @brief WCH CH58x timer configuration
 */
void platform_timer_init_wch(void);

#ifdef __cplusplus
}
#endif

#endif // PLATFORM_CHIPWCH
