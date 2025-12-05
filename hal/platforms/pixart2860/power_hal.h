/**
 * @file power_hal.h
 * @brief Pixart2860 Power管理硬件抽象层实现
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * Pixart2860平台的Power管理驱动实现
 */

#ifndef POWER_HAL_PIXART2860_H
#define POWER_HAL_PIXART2860_H

#include "../interface/power_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Pixart2860 Power管理函数声明 */
const power_interface_t *pixart2860_power_get_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* POWER_HAL_PIXART2860_H */