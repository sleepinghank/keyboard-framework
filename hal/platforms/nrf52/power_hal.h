/**
 * @file power_hal.h
 * @brief NRF52 Power管理硬件抽象层实现
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * NRF52平台的Power管理驱动实现
 */

#ifndef POWER_HAL_NRF52_H
#define POWER_HAL_NRF52_H

#include "../interface/power_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* NRF52 Power管理函数声明 */
const power_interface_t *nrf52_power_get_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* POWER_HAL_NRF52_H */