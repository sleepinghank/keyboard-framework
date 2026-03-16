#include "timer.h"
#include "sys_config.h"
#include "atomic_util.h"
#include <stdint.h>

#if(CHIP_TYPE == CHIP_CH584M)
    // #include "CH58x_common.h"
#endif

// RTC频率：32kHz = 32768 Hz
#if defined(CLK_OSC32K) && (CLK_OSC32K == 1)
#define RTC_FREQUENCY 32000UL
#else
#define RTC_FREQUENCY 32768UL
#endif
// 毫秒转换：cycles * 1000 / 32768
#define CYCLES_TO_MS(cycles) ((uint32_t)(((uint64_t)(cycles) * 1000UL) / RTC_FREQUENCY))

// 溢出调整：最大的32768的倍数，使得结果不超过UINT32_MAX
// 这样可以确保在转换为毫秒时不会溢出
#if(CHIP_TYPE == CHIP_CH584M)
#define RTC_COUNTER_MOD 0xA8C00000UL
#define RTC_WRAP_CYCLES UINT64_C(0xA8C00000)
#else
#define RTC_WRAP_CYCLES (UINT64_C(1) << 32)
#endif
#define RTC_WRAP_MS (CYCLES_TO_MS(RTC_WRAP_CYCLES))
// 对应的毫秒数

// 静态变量
static uint32_t cycles_offset = 0;  // RTC周期偏移量
static uint32_t last_cycles    = 0;  // 上次读取的周期数
static uint32_t ms_offset     = 0;  // 毫秒偏移量（用于处理溢出）

// 外部变量声明（为了保持接口一致性）
volatile uint32_t timer_count = 0;

// 获取当前RTC周期数（32位）
// 此函数必须在原子操作保护下调用
static inline uint32_t get_rtc_cycles(void) {
    return RTC_GetCycle32k();
    // return 1;
}

static inline uint32_t rtc_cycles_diff(uint32_t current, uint32_t base) {
#if(CHIP_TYPE == CHIP_CH584M)
    if (current >= base) {
        return current - base;
    }
    return (RTC_COUNTER_MOD - base) + current;
#else
    return current - base;
#endif
}

void timer_init(void) {
    timer_clear();
}

void timer_clear(void) {
    ATOMIC_BLOCK_FORCEON {
        cycles_offset = get_rtc_cycles();
        last_cycles   = 0;
        ms_offset     = 0;
    }
}

uint16_t timer_read(void) {
    return (uint16_t)timer_read32();
}

uint32_t timer_read32(void) {
    uint32_t cycles_elapsed;
    uint32_t ms_offset_copy;
    
    ATOMIC_BLOCK_FORCEON {
        // 获取当前RTC周期数
        uint32_t cycles = get_rtc_cycles();
        
        // 计算相对于偏移量的周期数
        cycles_elapsed = rtc_cycles_diff(cycles, cycles_offset);
        
        // 检测溢出：如果当前周期数小于上次读取的值，说明发生了32位溢出
        if (cycles_elapsed < last_cycles) {
            // 32位周期计数器溢出并回绕。我们不能简单地扩展到64位，
            // 因为在转换为毫秒时可能会遇到溢出问题。
            // 解决方案是：从周期计数器中减去一个合理的大数值，使其值低于32位限制，
            // 然后将等价的毫秒数加到转换后的值中。
            ms_offset += RTC_WRAP_MS;
        }
        
        last_cycles = cycles_elapsed;
        ms_offset_copy = ms_offset; // 在持有锁时读取以确保值的一致性
    }
    
    // 将周期数转换为毫秒并加上偏移量
    return CYCLES_TO_MS(cycles_elapsed) + ms_offset_copy;
}

uint16_t timer_elapsed(uint16_t last) {
    return TIMER_DIFF_16(timer_read(), last);
}

uint32_t timer_elapsed32(uint32_t last) {
    return TIMER_DIFF_32(timer_read32(), last);
}
