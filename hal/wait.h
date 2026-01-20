/**
 * @file wait.h
 * @brief 延时函数统一接口
 * @version 1.0
 *
 * 提供跨平台的延时函数接口
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 平台实现说明：
 * - CH584: 使用SDK的 mDelaymS / mDelayuS
 * - 其他平台: 需要提供对应实现
 */

#if defined(CH584) || defined(CH585)
    /* CH584/CH585 平台 - 使用SDK延时函数 */
    #include "CH58x_common.h"

    /**
     * @brief 毫秒延时
     * @param ms 延时毫秒数
     */
    #define wait_ms(ms)  mDelaymS(ms)

    /**
     * @brief 微秒延时
     * @param us 延时微秒数
     */
    #define wait_us(us)  mDelayuS(us)

#elif defined(NRF52)
    /* nRF52 平台 */
    #include "nrf_delay.h"
    #define wait_ms(ms)  nrf_delay_ms(ms)
    #define wait_us(us)  nrf_delay_us(us)

#elif defined(TEST_PLATFORM)
    /* 测试平台 - 空实现 */
    #define wait_ms(ms)  ((void)(ms))
    #define wait_us(us)  ((void)(us))

#else
    /* 默认实现 - 简单循环延时（不精确） */
    static inline void wait_ms(uint16_t ms) {
        volatile uint32_t count;
        while (ms--) {
            count = 8000;  /* 需要根据CPU频率调整 */
            while (count--);
        }
    }

    static inline void wait_us(uint16_t us) {
        volatile uint32_t count;
        while (us--) {
            count = 8;  /* 需要根据CPU频率调整 */
            while (count--);
        }
    }
#endif

#ifdef __cplusplus
}
#endif
