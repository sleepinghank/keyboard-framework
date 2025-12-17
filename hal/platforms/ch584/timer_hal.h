/**
 * @file timer_hal.h
 * @brief CH584 Timer硬件抽象层实现
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * CH584平台的Timer驱动实现
 */

#ifndef TIMER_HAL_CH584_H
#define TIMER_HAL_CH584_H

#include "timer_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CH584 Timer函数声明 */
const timer_interface_t *ch584_timer_get_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* TIMER_HAL_CH584_H */