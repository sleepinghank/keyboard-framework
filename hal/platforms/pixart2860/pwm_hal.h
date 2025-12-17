/**
 * @file pwm_hal.h
 * @brief Pixart2860 PWM硬件抽象层实现
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * Pixart2860平台的PWM驱动实现
 */

#ifndef PWM_HAL_PIXART2860_H
#define PWM_HAL_PIXART2860_H

#include "pwm_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Pixart2860 PWM通道定义 */
#define PIXART2860_PWM_CHANNEL_0        0
#define PIXART2860_PWM_CHANNEL_1        1
#define PIXART2860_PWM_CHANNEL_2        2
#define PIXART2860_PWM_CHANNEL_3        3
#define PIXART2860_PWM_CHANNEL_4        4
#define PIXART2860_PWM_CHANNEL_5        5
#define PIXART2860_PWM_CHANNEL_6        6
#define PIXART2860_PWM_CHANNEL_7        7

/* Pixart2860 PWM通道数量 */
#define PIXART2860_PWM_CHANNEL_COUNT     8

/* Pixart2860 PWM定时器定义 */
#define PIXART2860_PWM_TIMER_0          0
#define PIXART2860_PWM_TIMER_1          1
#define PIXART2860_PWM_TIMER_2          2
#define PIXART2860_PWM_TIMER_3          3

/* Pixart2860 PWM定时器数量 */
#define PIXART2860_PWM_TIMER_COUNT      4

/* Pixart2860 PWM寄存器基址 */
#define PIXART2860_PWM_BASE             0x40002000UL

/* Pixart2860 PWM定时器寄存器基址 */
#define PIXART2860_PWM_TIMER_BASE(timer) (PIXART2860_PWM_BASE + ((timer) << 8))

/* Pixart2860 PWM寄存器偏移 */
#define PIXART2860_PWM_CR_REG           0x00  /* 控制寄存器 */
#define PIXART2860_PWM_SR_REG           0x04  /* 状态寄存器 */
#define PIXART2860_PWM_DIER_REG         0x0C  /* 中断使能寄存器 */
#define PIXART2860_PWM_SR_REG           0x10  /* 状态寄存器 */
#define PIXART2860_PWM_EGR_REG          0x14  /* 事件生成寄存器 */
#define PIXART2860_PWM_CCMR1_REG        0x18  /* 捕获/比较模式寄存器1 */
#define PIXART2860_PWM_CCMR2_REG        0x1C  /* 捕获/比较模式寄存器2 */
#define PIXART2860_PWM_CCER_REG         0x20  /* 捕获/比较使能寄存器 */
#define PIXART2860_PWM_CNT_REG          0x24  /* 计数器寄存器 */
#define PIXART2860_PWM_PSC_REG          0x28  /* 预分频器寄存器 */
#define PIXART2860_PWM_ARR_REG          0x2C  /* 自动重载寄存器 */
#define PIXART2860_PWM_RCR_REG          0x30  /* 重复计数器寄存器 */
#define PIXART2860_PWM_CCR1_REG         0x34  /* 捕获/比较寄存器1 */
#define PIXART2860_PWM_CCR2_REG         0x38  /* 捕获/比较寄存器2 */
#define PIXART2860_PWM_CCR3_REG         0x3C  /* 捕获/比较寄存器3 */
#define PIXART2860_PWM_CCR4_REG         0x40  /* 捕获/比较寄存器4 */
#define PIXART2860_PWM_BDTR_REG         0x44  /* 刹车和死区寄存器 */
#define PIXART2860_PWM_DCR_REG          0x48  /* DMA控制寄存器 */
#define PIXART2860_PWM_DMAR_REG         0x4C  /* DMA地址寄存器 */
#define PIXART2860_PWM_OR_REG           0x50  /* 选项寄存器 */

/* Pixart2860 PWM控制寄存器位定义 */
#define PIXART2860_PWM_CR_CEN           (1 << 0)    /* 计数器使能 */
#define PIXART2860_PWM_CR_UDIS          (1 << 1)    /* 更新禁用 */
#define PIXART2860_PWM_CR_URS           (1 << 2)    /* 更新请求源 */
#define PIXART2860_PWM_CR_OPM           (1 << 3)    /* 单脉冲模式 */
#define PIXART2860_PWM_CR_DIR           (1 << 4)    /* 方向 */
#define PIXART2860_PWM_CR_CMS_SHIFT     5          /* 中央对齐模式选择 */
#define PIXART2860_PWM_CR_CMS_MASK      (3 << 5)
#define PIXART2860_PWM_CR_ARPE          (1 << 7)    /* 自动重载预装载使能 */
#define PIXART2860_PWM_CR_CKD_SHIFT     8          /* 时钟分频 */
#define PIXART2860_PWM_CR_CKD_MASK      (3 << 8)
#define PIXART2860_PWM_CR_UIFREMAP      (1 << 11)   /* UIF重映射 */
#define PIXART2860_PWM_CR_CCPC          (1 << 15)   /* 捕获/比较预装载控制 */
#define PIXART2860_PWM_CR_CCUS          (1 << 16)   /* 捕获/比较控制更新选择 */
#define PIXART2860_PWM_CR_CCDS          (1 << 17)   /* 捕获/比较DMA选择 */

/* Pixart2860 PWM状态寄存器位定义 */
#define PIXART2860_PWM_SR_UIF           (1 << 0)    /* 更新中断标志 */
#define PIXART2860_PWM_SR_CC1IF         (1 << 1)    /* 捕获/比较1中断标志 */
#define PIXART2860_PWM_SR_CC2IF         (1 << 2)    /* 捕获/比较2中断标志 */
#define PIXART2860_PWM_SR_CC3IF         (1 << 3)    /* 捕获/比较3中断标志 */
#define PIXART2860_PWM_SR_CC4IF         (1 << 4)    /* 捕获/比较4中断标志 */
#define PIXART2860_PWM_SR_COMIF         (1 << 5)    /* 通信中断标志 */
#define PIXART2860_PWM_SR_TIF           (1 << 6)    /* 触发中断标志 */
#define PIXART2860_PWM_SR_BIF           (1 << 7)    /* 刹车中断标志 */
#define PIXART2860_PWM_SR_CC1OF         (1 << 9)    /* 捕获/比较1溢出标志 */
#define PIXART2860_PWM_SR_CC2OF         (1 << 10)   /* 捕获/比较2溢出标志 */
#define PIXART2860_PWM_SR_CC3OF         (1 << 11)   /* 捕获/比较3溢出标志 */
#define PIXART2860_PWM_SR_CC4OF         (1 << 12)   /* 捕获/比较4溢出标志 */

/* Pixart2860 PWM捕获/比较模式寄存器位定义 */
#define PIXART2860_PWM_CCMR_CCxS_SHIFT   0          /* 捕获/比较x选择 */
#define PIXART2860_PWM_CCMR_CCxS_MASK    (3 << 0)
#define PIXART2860_PWM_CCMR_ICxPSC_SHIFT 2          /* 输入捕获预分频器 */
#define PIXART2860_PWM_CCMR_ICxPSC_MASK  (3 << 2)
#define PIXART2860_PWM_CCMR_ICxF_SHIFT   4          /* 输入捕获滤波器 */
#define PIXART2860_PWM_CCMR_ICxF_MASK    (15 << 4)
#define PIXART2860_PWM_CCMR_OCxM_SHIFT   4          /* 输出比较模式 */
#define PIXART2860_PWM_CCMR_OCxM_MASK    (7 << 4)
#define PIXART2860_PWM_CCMR_OCxFE        (1 << 2)    /* 输出比较快速使能 */
#define PIXART2860_PWM_CCMR_OCxPE        (1 << 3)    /* 输出比较预装载使能 */

/* Pixart2860 PWM捕获/比较使能寄存器位定义 */
#define PIXART2860_PWM_CCER_CC1E         (1 << 0)    /* 捕获/比较1输出使能 */
#define PIXART2860_PWM_CCER_CC1P         (1 << 1)    /* 捕获/比较1输出极性 */
#define PIXART2860_PWM_CCER_CC1NE        (1 << 2)    /* 捕获/比较1互补输出使能 */
#define PIXART2860_PWM_CCER_CC1NP        (1 << 3)    /* 捕获/比较1互补输出极性 */
#define PIXART2860_PWM_CCER_CC2E         (1 << 4)    /* 捕获/比较2输出使能 */
#define PIXART2860_PWM_CCER_CC2P         (1 << 5)    /* 捕获/比较2输出极性 */
#define PIXART2860_PWM_CCER_CC2NE        (1 << 6)    /* 捕获/比较2互补输出使能 */
#define PIXART2860_PWM_CCER_CC2NP        (1 << 7)    /* 捕获/比较2互补输出极性 */
#define PIXART2860_PWM_CCER_CC3E         (1 << 8)    /* 捕获/比较3输出使能 */
#define PIXART2860_PWM_CCER_CC3P         (1 << 9)    /* 捕获/比较3输出极性 */
#define PIXART2860_PWM_CCER_CC3NE        (1 << 10)   /* 捕获/比较3互补输出使能 */
#define PIXART2860_PWM_CCER_CC3NP        (1 << 11)   /* 捕获/比较3互补输出极性 */
#define PIXART2860_PWM_CCER_CC4E         (1 << 12)   /* 捕获/比较4输出使能 */
#define PIXART2860_PWM_CCER_CC4P         (1 << 13)   /* 捕获/比较4输出极性 */

/* Pixart2860 PWM中断定义 */
#define PIXART2860_PWM_IRQn(timer)      (2 + (timer))
#define PIXART2860_PWM_IRQ_PRIORITY      3

/* Pixart2860 PWM时钟使能 */
#define PIXART2860_PWM_CLK_ENABLE(timer) do { \
    /* 启用PWM时钟 */ \
} while(0)

#define PIXART2860_PWM_CLK_DISABLE(timer) do { \
    /* 禁用PWM时钟 */ \
} while(0)

/* Pixart2860 PWM时钟频率定义 */
#define PIXART2860_PWM_INPUT_CLOCK_HZ    48000000   /* 输入时钟频率48MHz */

/* Pixart2860 PWM最大频率 */
#define PIXART2860_PWM_MAX_FREQUENCY_HZ  24000000   /* 最大频率24MHz */

/* Pixart2860 PWM最小频率 */
#define PIXART2860_PWM_MIN_FREQUENCY_HZ  1          /* 最小频率1Hz */

/* Pixart2860 PWM配置参数 */
typedef struct {
    uint8_t                 channel;        /* PWM通道号 */
    uint8_t                 timer;          /* PWM定时器号 */
    uint32_t                frequency;      /* PWM频率(Hz) */
    uint16_t                duty_cycle;     /* PWM占空比(0-1000) */
    uint16_t                period_ns;      /* PWM周期(纳秒) */
    pwm_polarity_t          polarity;       /* PWM极性 */
    pwm_align_t             alignment;      /* PWM对齐模式 */
    pwm_count_mode_t        count_mode;     /* 计数模式 */
    pwm_output_mode_t       output_mode;    /* 输出模式 */
    bool                    auto_reload;    /* 自动重载 */
    bool                    enable_interrupt; /* 使能中断 */
    uint8_t                 priority;       /* 中断优先级 */
    bool                    complementary;  /* 互补输出 */
    uint16_t                dead_time_ns;   /* 死区时间(纳秒) */
    bool                    brake_enable;   /* 刹车使能 */
    uint8_t                 brake_polarity;  /* 刹车极性 */
    uint32_t                prescaler;      /* 预分频器值 */
    uint16_t                auto_reload_value; /* 自动重载值 */
    uint16_t                capture_compare_value; /* 捕获比较值 */
} pixart2860_pwm_config_t;

/* Pixart2860 PWM实例结构体 */
typedef struct {
    volatile uint32_t      *cr_reg;          /* 控制寄存器 */
    volatile uint32_t      *sr_reg;          /* 状态寄存器 */
    volatile uint32_t      *dier_reg;        /* 中断使能寄存器 */
    volatile uint32_t      *egr_reg;         /* 事件生成寄存器 */
    volatile uint32_t      *ccmr1_reg;       /* 捕获/比较模式寄存器1 */
    volatile uint32_t      *ccmr2_reg;       /* 捕获/比较模式寄存器2 */
    volatile uint32_t      *ccer_reg;        /* 捕获/比较使能寄存器 */
    volatile uint32_t      *cnt_reg;         /* 计数器寄存器 */
    volatile uint32_t      *psc_reg;         /* 预分频器寄存器 */
    volatile uint32_t      *arr_reg;         /* 自动重载寄存器 */
    volatile uint32_t      *rcr_reg;         /* 重复计数器寄存器 */
    volatile uint32_t      *bdtr_reg;        /* 刹车和死区寄存器 */
    volatile uint32_t      *ccr_regs[4];     /* 捕获/比较寄存器数组 */
    pixart2860_pwm_config_t *configs;        /* 通道配置数组 */
    pwm_update_callback_t  *update_callbacks; /* 更新回调函数数组 */
    pwm_fault_callback_t   *fault_callbacks;  /* 故障回调函数数组 */
    void                  **user_data;       /* 用户数据数组 */
    bool                   initialized[4];   /* 通道初始化标志数组 */
    bool                   active[4];         /* 通道活跃标志数组 */
    uint16_t               current_duty[4];  /* 当前占空比数组 */
    uint32_t               current_frequency; /* 当前频率 */
    uint32_t               update_count[4];  /* 更新计数器数组 */
    uint32_t               fault_count;      /* 故障计数器 */
} pixart2860_pwm_instance_t;

/* Pixart2860 PWM函数声明 */

/**
 * @brief 获取Pixart2860 PWM接口
 * @return PWM接口指针
 */
const pwm_interface_t *pixart2860_pwm_get_interface(void);

/**
 * @brief 配置PWM定时器
 * @param timer 定时器号
 * @param frequency 频率(Hz)
 * @param alignment 对齐模式
 * @return 配置结果
 */
pwm_result_t pixart2860_pwm_config_timer(uint8_t timer, uint32_t frequency, pwm_align_t alignment);

/**
 * @brief 配置PWM通道
 * @param channel 通道号
 * @param timer 定时器号
 * @param config 配置参数
 * @return 配置结果
 */
pwm_result_t pixart2860_pwm_config_channel(uint8_t channel, uint8_t timer, const pixart2860_pwm_config_t *config);

/**
 * @brief 配置PWM输出比较
 * @param timer 定时器号
 * @param cc_channel 捕获比较通道
 * @param mode 输出比较模式
 * @param value 比较值
 * @return 配置结果
 */
pwm_result_t pixart2860_pwm_config_output_compare(uint8_t timer, uint8_t cc_channel, uint8_t mode, uint16_t value);

/**
 * @brief 配置PWM死区时间
 * @param timer 定时器号
 * @param dead_time_ns 死区时间(纳秒)
 * @return 配置结果
 */
pwm_result_t pixart2860_pwm_config_dead_time(uint8_t timer, uint32_t dead_time_ns);

/**
 * @brief 配置PWM刹车
 * @param timer 定时器号
 * @param enable 使能
 * @param polarity 极性
 * @return 配置结果
 */
pwm_result_t pixart2860_pwm_config_brake(uint8_t timer, bool enable, uint8_t polarity);

/**
 * @brief 配置PWM互补输出
 * @param timer 定时器号
 * @param cc_channel 捕获比较通道
 * @param enable 使能
 * @param polarity 极性
 * @return 配置结果
 */
pwm_result_t pixart2860_pwm_config_complementary(uint8_t timer, uint8_t cc_channel, bool enable, uint8_t polarity);

/**
 * @brief 启动PWM定时器
 * @param timer 定时器号
 * @return 启动结果
 */
pwm_result_t pixart2860_pwm_start_timer(uint8_t timer);

/**
 * @brief 停止PWM定时器
 * @param timer 定时器号
 * @return 停止结果
 */
pwm_result_t pixart2860_pwm_stop_timer(uint8_t timer);

/**
 * @brief 设置PWM周期
 * @param timer 定时器号
 * @param period_ns 周期(纳秒)
 * @return 设置结果
 */
pwm_result_t pixart2860_pwm_set_period(uint8_t timer, uint32_t period_ns);

/**
 * @brief 设置PWM占空比
 * @param channel 通道号
 * @param timer 定时器号
 * @param duty_cycle 占空比(0-1000)
 * @return 设置结果
 */
pwm_result_t pixart2860_pwm_set_duty_cycle(uint8_t channel, uint8_t timer, uint16_t duty_cycle);

/**
 * @brief 设置PWM脉冲宽度
 * @param channel 通道号
 * @param timer 定时器号
 * @param pulse_width_ns 脉冲宽度(纳秒)
 * @return 设置结果
 */
pwm_result_t pixart2860_pwm_set_pulse_width(uint8_t channel, uint8_t timer, uint32_t pulse_width_ns);

/**
 * @brief 触发PWM更新
 * @param timer 定时器号
 * @return 触发结果
 */
pwm_result_t pixart2860_pwm_trigger_update(uint8_t timer);

/**
 * @brief PWM中断处理函数
 * @param timer 定时器号
 * @return 处理结果
 */
void pixart2860_pwm_irq_handler(uint8_t timer);

/**
 * @brief 计算PWM预分频器
 * @param frequency 目标频率(Hz)
 * @param input_clock 输入时钟(Hz)
 * @return 预分频器值
 */
uint32_t pixart2860_pwm_calculate_prescaler(uint32_t frequency, uint32_t input_clock);

/**
 * @brief 计算PWM自动重载值
 * @param frequency 目标频率(Hz)
 * @param input_clock 输入时钟(Hz)
 * @param prescaler 预分频器值
 * @return 自动重载值
 */
uint32_t pixart2860_pwm_calculate_auto_reload(uint32_t frequency, uint32_t input_clock, uint32_t prescaler);

/**
 * @brief 计算PWM比较值
 * @param duty_cycle 占空比(0-1000)
 * @param auto_reload_value 自动重载值
 * @return 比较值
 */
uint32_t pixart2860_pwm_calculate_compare_value(uint16_t duty_cycle, uint32_t auto_reload_value);

/**
 * @brief 计算PWM死区时间寄存器值
 * @param dead_time_ns 死区时间(纳秒)
 * @param input_clock 输入时钟(Hz)
 * @return 死区时间寄存器值
 */
uint32_t pixart2860_pwm_calculate_dead_time(uint32_t dead_time_ns, uint32_t input_clock);

/* 便捷宏定义 */
#define PIXART2860_PWM_CONFIG(channel, timer, freq, duty) \
    { (channel), (timer), (freq), (duty), 0, PWM_POLARITY_NORMAL, PWM_ALIGN_EDGE, PWM_COUNT_UP, PWM_MODE_SINGLE, true, false, 0, false, 0, false, 0, 0, 0, 0 }

#define PIXART2860_PWM_CONFIG_BACKLIGHT(channel, timer, freq, duty) \
    { (channel), (timer), (freq), (duty), 0, PWM_POLARITY_NORMAL, PWM_ALIGN_CENTER, PWM_COUNT_UP, PWM_MODE_SINGLE, true, true, 0, false, 0, false, 0, 0, 0, 0 }

#define PIXART2860_PWM_CONFIG_MOTOR(channel, timer, freq, duty) \
    { (channel), (timer), (freq), (duty), 0, PWM_POLARITY_NORMAL, PWM_ALIGN_CENTER, PWM_COUNT_UP_DOWN, PWM_MODE_COMPLEMENTARY, true, true, 0, true, 500, true, 0, 0, 0, 0 }

/* 寄存器操作宏 */
#define PIXART2860_PWM_GET_REG(timer, offset) \
    (*((volatile uint32_t *)(PIXART2860_PWM_TIMER_BASE(timer) + (offset))))

#define PIXART2860_PWM_SET_REG(timer, offset, value) \
    (*((volatile uint32_t *)(PIXART2860_PWM_TIMER_BASE(timer) + (offset))) = (value))

#define PIXART2860_PWM_MODIFY_REG(timer, offset, mask, value) \
    (*((volatile uint32_t *)(PIXART2860_PWM_TIMER_BASE(timer) + (offset))) = \
     ((*((volatile uint32_t *)(PIXART2860_PWM_TIMER_BASE(timer) + (offset))) & ~(mask)) | ((value) & (mask))))

#define PIXART2860_PWM_GET_FLAG(timer, flag) \
    (PIXART2860_PWM_GET_REG(timer, PIXART2860_PWM_SR_REG) & (flag))

#define PIXART2860_PWM_SET_FLAG(timer, flag) \
    (PIXART2860_PWM_GET_REG(timer, PIXART2860_PWM_SR_REG) | (flag))

#define PIXART2860_PWM_CLR_FLAG(timer, flag) \
    (PIXART2860_PWM_GET_REG(timer, PIXART2860_PWM_SR_REG) & ~(flag))

/* 常用PWM通道配置宏 */
#define PIXART2860_PWM_BACKLIGHT_CHANNEL   PIXART2860_PWM_CHANNEL_0   /* 背光通道 */
#define PIXART2860_PWM_LED_CHANNEL         PIXART2860_PWM_CHANNEL_1   /* LED指示灯通道 */
#define PIXART2860_PWM_BEEPER_CHANNEL      PIXART2860_PWM_CHANNEL_2   /* 蜂鸣器通道 */
#define PIXART2860_PWM_MOTOR_A_CHANNEL     PIXART2860_PWM_CHANNEL_3   /* 电机A通道 */
#define PIXART2860_PWM_MOTOR_B_CHANNEL     PIXART2860_PWM_CHANNEL_4   /* 电机B通道 */

/* PWM频率计算宏 */
#define PIXART2860_PWM_PERIOD_TO_NS(period) \
    ((1000000000UL) / (period))

#define PIXART2860_PWM_NS_TO_PERIOD(period_ns) \
    ((1000000000UL + (period_ns) / 2) / (period_ns))

#define PIXART2860_PWM_DUTY_TO_NS(duty, period_ns) \
    (((duty) * (period_ns)) / 1000)

#define PIXART2860_PWM_NS_TO_DUTY(pulse_ns, period_ns) \
    (((pulse_ns) * 1000 + (period_ns) / 2) / (period_ns))

/* PWM亮度控制宏 */
#define PIXART2860_PWM_SET_BRIGHTNESS(channel, brightness) \
    pixart2860_pwm_set_duty_percentage((channel), ((brightness) * 1000) / 255)

#define PIXART2860_PWM_GET_BRIGHTNESS(channel) \
    (pixart2860_pwm_get_duty_percentage((channel)) * 255 / 1000)

#ifdef __cplusplus
}
#endif

#endif /* PWM_HAL_PIXART2860_H */