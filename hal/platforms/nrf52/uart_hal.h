/**
 * @file uart_hal.h
 * @brief NRF52 UART硬件抽象层实现
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * NRF52平台的UART驱动实现
 */

#ifndef UART_HAL_NRF52_H
#define UART_HAL_NRF52_H

#include "../interface/uart_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* NRF52 UART函数声明 */
const uart_interface_t *nrf52_uart_get_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* UART_HAL_NRF52_H */