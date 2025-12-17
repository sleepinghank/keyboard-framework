/**
 * @file pwm_hal.h
 * @brief CH584 PWM硬件抽象层实现
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * CH584平台的PWM驱动实现
 */

#ifndef PWM_HAL_CH584_H
#define PWM_HAL_CH584_H

#include "pwm_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CH584 PWM函数声明 */
const pwm_interface_t *ch584_pwm_get_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* PWM_HAL_CH584_H */