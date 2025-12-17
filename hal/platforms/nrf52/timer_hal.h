/**
 * @file timer_hal.h
 * @brief NRF52 Timer硬件抽象层实现
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * NRF52平台的Timer驱动实现
 */

#ifndef TIMER_HAL_NRF52_H
#define TIMER_HAL_NRF52_H

#include "timer_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* NRF52 Timer函数声明 */
const timer_interface_t *nrf52_timer_get_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* TIMER_HAL_NRF52_H */