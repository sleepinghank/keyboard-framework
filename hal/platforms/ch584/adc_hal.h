/**
 * @file adc_hal.h
 * @brief CH584 ADC硬件抽象层实现
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * CH584平台的ADC驱动实现
 */

#ifndef ADC_HAL_CH584_H
#define ADC_HAL_CH584_H

#include "../interface/adc_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CH584 ADC函数声明 */
const adc_interface_t *ch584_adc_get_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* ADC_HAL_CH584_H */