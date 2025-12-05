/**
 * @file timer_hal.h
 * @brief Pixart2860 Timer硬件抽象层实现
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * Pixart2860平台的Timer驱动实现
 */

#ifndef TIMER_HAL_PIXART2860_H
#define TIMER_HAL_PIXART2860_H

#include "../interface/timer_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Pixart2860 Timer定义 */
#define PIXART2860_TIMER_0            0
#define PIXART2860_TIMER_1            1
#define PIXART2860_TIMER_2            2
#define PIXART2860_TIMER_3            3

/* Pixart2860 Timer数量 */
#define PIXART2860_TIMER_COUNT         4

/* Pixart2860 Timer寄存器基址 */
#define PIXART2860_TIMER_BASE          0x40005000UL

/* Pixart2860 Timer寄存器基址 */
#define PIXART2860_TIMER_BASE(timer)   (PIXART2860_TIMER_BASE + ((timer) << 8))

/* Pixart2860 Timer函数声明 */
const timer_interface_t *pixart2860_timer_get_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* TIMER_HAL_PIXART2860_H */