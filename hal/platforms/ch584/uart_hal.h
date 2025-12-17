/**
 * @file uart_hal.h
 * @brief CH584 UART硬件抽象层实现
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * CH584平台的UART驱动实现
 */

#ifndef UART_HAL_CH584_H
#define UART_HAL_CH584_H

#include "uart_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CH584 UART函数声明 */
const uart_interface_t *ch584_uart_get_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* UART_HAL_CH584_H */