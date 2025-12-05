/**
 * @file i2c_hal.h
 * @brief CH584 I2C硬件抽象层实现
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * CH584平台的I2C驱动实现
 */

#ifndef I2C_HAL_CH584_H
#define I2C_HAL_CH584_H

#include "../interface/i2c_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CH584 I2C函数声明 */
const i2c_interface_t *ch584_i2c_get_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* I2C_HAL_CH584_H */