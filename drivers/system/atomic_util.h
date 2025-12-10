#pragma once

#include "sys_config.h"
#include <stdint.h>

// 编译器兼容性处理：inline 关键字
#ifndef __INLINE
    #if defined(__GNUC__) || defined(__clang__)
        #define __INLINE static inline
    #elif defined(_MSC_VER)
        #define __INLINE static __inline
    #else
        #define __INLINE static inline
    #endif
#endif

#if(CHIP_TYPE == CHIP_CH584M)
    // 沁恒CH584平台
    // #include "CH58x_common.h"
    
    // 中断状态保存结构体
    typedef struct {
        uint32_t irqv;
        uint8_t done;
    } __interrupt_state_t;
    
    __INLINE __interrupt_state_t __interrupt_disable__(void) {
        __interrupt_state_t state;
        // SYS_DisableAllIrq(&state.irqv);
        state.done = 0;
        return state;
    }
    
    __INLINE void __interrupt_enable__(__interrupt_state_t *__s) {
        // if (__s != NULL) {
            // SYS_RecoverIrq(__s->irqv);
        // }
        // __asm__ volatile("" ::: "memory");
    }
    
    #define ATOMIC_BLOCK(type) for (type, __ToDo = __interrupt_disable__(); !__ToDo.done; __ToDo.done = 1)
    #define ATOMIC_FORCEON __interrupt_state_t sreg_save __attribute__((__cleanup__(__interrupt_enable__))) = {0, 0}

#elif(CHIP_TYPE == CHIP_PAR2860)
    // 原相2860平台
    #include "core_cm0.h"
    
    __INLINE uint8_t __interrupt_disable__(void) {
        __disable_irq();
        return 1;
    }
    
    __INLINE void __interrupt_enable__(const uint8_t *__s) {
        __enable_irq();
        __asm__ volatile("" ::: "memory");
        (void)__s;
    }
    
    #define ATOMIC_BLOCK(type) for (type, __ToDo = __interrupt_disable__(); __ToDo; __ToDo = 0)
    #define ATOMIC_FORCEON uint8_t sreg_save __attribute__((__cleanup__(__interrupt_enable__))) = 0

#else
    #error "Unsupported chip type for atomic operations"
#endif

#define ATOMIC_BLOCK_RESTORESTATE _Static_assert(0, "ATOMIC_BLOCK_RESTORESTATE not implemented")
#define ATOMIC_BLOCK_FORCEON ATOMIC_BLOCK(ATOMIC_FORCEON)