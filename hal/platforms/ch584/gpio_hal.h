/**
 * @file gpio_hal.h
 * @brief CH584 GPIO硬件抽象层实现
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * CH584平台的GPIO驱动实现
 */

#ifndef GPIO_HAL_CH584_H
#define GPIO_HAL_CH584_H

#include "gpio_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CH584 GPIO函数声明 */
const gpio_interface_t *ch584_gpio_get_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* GPIO_HAL_CH584_H */