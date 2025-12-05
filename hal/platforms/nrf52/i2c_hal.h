/**
 * @file i2c_hal.h
 * @brief NRF52 I2C硬件抽象层实现
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * NRF52平台的I2C驱动实现
 */

#ifndef I2C_HAL_NRF52_H
#define I2C_HAL_NRF52_H

#include "../interface/i2c_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* NRF52 I2C函数声明 */
const i2c_interface_t *nrf52_i2c_get_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* I2C_HAL_NRF52_H */