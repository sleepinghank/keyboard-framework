/**
 * @file pwm_interface.h
 * @brief PWM硬件抽象层接口定义
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * 提供跨芯片平台的PWM操作统一接口
 * 支持LED背光控制、指示灯控制、电机控制等应用
 */

#ifndef PWM_INTERFACE_H
#define PWM_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PWM错误码定义 */
typedef enum {
    PWM_OK = 0,                     /* 操作成功 */
    PWM_ERROR_INVALID_CHANNEL,      /* 无效通道 */
    PWM_ERROR_INVALID_FREQUENCY,    /* 无效频率 */
    PWM_ERROR_INVALID_DUTY,         /* 无效占空比 */
    PWM_ERROR_INVALID_POLARITY,     /* 无效极性 */
    PWM_ERROR_INVALID_MODE,         /* 无效模式 */
    PWM_ERROR_NOT_SUPPORTED,        /* 操作不支持 */
    PWM_ERROR_HARDWARE_FAULT,       /* 硬件故障 */
    PWM_ERROR_BUSY,                 /* PWM忙碌 */
    PWM_ERROR_TIMEOUT,              /* 操作超时 */
    PWM_ERROR_ALREADY_INITIALIZED,  /* 已初始化 */
    PWM_ERROR_NOT_INITIALIZED,      /* 未初始化 */
    PWM_ERROR_FREQUENCY_OUT_OF_RANGE, /* 频率超出范围 */
    PWM_ERROR_CHANNEL_CONFLICT,     /* 通道冲突 */
    PWM_ERROR_CLOCK_NOT_READY       /* 时钟未准备好 */
} pwm_result_t;

/* PWM极性定义 */
typedef enum {
    PWM_POLARITY_NORMAL = 0,        /* 正常极性 (高电平有效) */
    PWM_POLARITY_INVERSE            /* 反向极性 (低电平有效) */
} pwm_polarity_t;

/* PWM对齐模式 */
typedef enum {
    PWM_ALIGN_EDGE = 0,             /* 边沿对齐 */
    PWM_ALIGN_CENTER               /* 中心对齐 */
} pwm_align_t;

/* PWM计数模式 */
typedef enum {
    PWM_COUNT_UP = 0,               /* 向上计数 */
    PWM_COUNT_DOWN,                 /* 向下计数 */
    PWM_COUNT_UP_DOWN              /* 上下计数 */
} pwm_count_mode_t;

/* PWM输出模式 */
typedef enum {
    PWM_MODE_SINGLE = 0,            /* 单个输出 */
    PWM_MODE_COMPLEMENTARY,         /* 互补输出 */
    PWM_MODE_PUSH_PULL,             /* 推挽输出 */
    PWM_MODE_BRAKE                  /* 刹车模式 */
} pwm_output_mode_t;

/* PWM死区时间单位 */
typedef enum {
    PWM_DEADTIME_NS = 0,            /* 纳秒 */
    PWM_DEADTIME_US,                /* 微秒 */
    PWM_DEADTIME_CYCLES             /* 时钟周期 */
} pwm_deadtime_unit_t;

/* PWM通道配置结构体 */
typedef struct {
    uint8_t                 channel;        /* PWM通道号 */
    uint32_t                frequency;      /* PWM频率(Hz) */
    uint16_t                duty_cycle;     /* PWM占空比(0-1000) */
    uint16_t                period_ns;      /* PWM周期(纳秒) */
    pwm_polarity_t          polarity;       /* PWM极性 */
    pwm_align_t             alignment;      /* PWM对齐模式 */
    pwm_count_mode_t        count_mode;     /* 计数模式 */
    pwm_output_mode_t       output_mode;    /* 输出模式 */
    bool                    auto_reload;    /* 自动重载 */
    bool                    enable_interrupt; /* 使能中断 */
    uint8_t                 priority;        /* 中断优先级 */
} pwm_channel_config_t;

/* PWM互补通道配置 */
typedef struct {
    uint8_t                 primary_channel;    /* 主通道 */
    uint8_t                 complementary_channel; /* 互补通道 */
    uint32_t                dead_time;         /* 死区时间 */
    pwm_deadtime_unit_t     dead_time_unit;    /* 死区时间单位 */
    bool                    enable_break_input; /* 使能刹车输入 */
    uint8_t                 break_polarity;     /* 刹车极性 */
    bool                    auto_restart;       /* 自动重启 */
} pwm_complementary_config_t;

/* PWM多通道配置结构体 */
typedef struct {
    const pwm_channel_config_t *channels;          /* 通道配置数组 */
    uint8_t                     channel_count;    /* 通道数量 */
    uint32_t                    base_frequency;   /* 基础频率(Hz) */
    bool                        synchronized;     /* 是否同步 */
    uint32_t                    phase_shift_ns;   /* 相位偏移(纳秒) */
    bool                        enable_break_input; /* 使能全局刹车输入 */
    uint8_t                     break_polarity;   /* 刹车极性 */
} pwm_config_t;

/* PWM更新完成回调函数类型 */
typedef void (*pwm_update_callback_t)(uint8_t channel, uint16_t duty_cycle, void *user_data);

/* PWM故障回调函数类型 */
typedef void (*pwm_fault_callback_t)(uint8_t channel, uint8_t fault_code, void *user_data);

/* PWM接口函数指针结构体 */
typedef struct {
    /**
     * @brief 初始化PWM模块
     * @return 初始化结果
     */
    pwm_result_t (*init)(void);

    /**
     * @brief 反初始化PWM模块
     * @return 反初始化结果
     */
    pwm_result_t (*deinit)(void);

    /**
     * @brief 配置PWM
     * @param config PWM配置参数
     * @return 配置结果
     */
    pwm_result_t (*config)(const pwm_config_t *config);

    /**
     * @brief 配置单个PWM通道
     * @param channel_config 通道配置参数
     * @return 配置结果
     */
    pwm_result_t (*config_channel)(const pwm_channel_config_t *channel_config);

    /**
     * @brief 配置PWM互补通道
     * @param complementary_config 互补通道配置参数
     * @return 配置结果
     */
    pwm_result_t (*config_complementary)(const pwm_complementary_config_t *complementary_config);

    /**
     * @brief 启动PWM输出
     * @param channel PWM通道号
     * @return 启动结果
     */
    pwm_result_t (*start)(uint8_t channel);

    /**
     * @brief 停止PWM输出
     * @param channel PWM通道号
     * @return 停止结果
     */
    pwm_result_t (*stop)(uint8_t channel);

    /**
     * @brief 启动所有PWM输出
     * @return 启动结果
     */
    pwm_result_t (*start_all)(void);

    /**
     * @brief 停止所有PWM输出
     * @return 停止结果
     */
    pwm_result_t (*stop_all)(void);

    /**
     * @brief 设置PWM频率
     * @param channel PWM通道号
     * @param frequency PWM频率(Hz)
     * @return 设置结果
     */
    pwm_result_t (*set_frequency)(uint8_t channel, uint32_t frequency);

    /**
     * @brief 获取PWM频率
     * @param channel PWM通道号
     * @param frequency 输出PWM频率(Hz)
     * @return 获取结果
     */
    pwm_result_t (*get_frequency)(uint8_t channel, uint32_t *frequency);

    /**
     * @brief 设置PWM占空比
     * @param channel PWM通道号
     * @param duty_cycle 占空比(0-1000)
     * @return 设置结果
     */
    pwm_result_t (*set_duty_cycle)(uint8_t channel, uint16_t duty_cycle);

    /**
     * @brief 获取PWM占空比
     * @param channel PWM通道号
     * @param duty_cycle 输出占空比(0-1000)
     * @return 获取结果
     */
    pwm_result_t (*get_duty_cycle)(uint8_t channel, uint16_t *duty_cycle);

    /**
     * @brief 设置PWM占空比(浮点数)
     * @param channel PWM通道号
     * @param percentage 占空比百分比(0.0-100.0)
     * @return 设置结果
     */
    pwm_result_t (*set_duty_percentage)(uint8_t channel, float percentage);

    /**
     * @brief 获取PWM占空比(浮点数)
     * @param channel PWM通道号
     * @param percentage 输出占空比百分比(0.0-100.0)
     * @return 获取结果
     */
    pwm_result_t (*get_duty_percentage)(uint8_t channel, float *percentage);

    /**
     * @brief 设置PWM脉冲宽度
     * @param channel PWM通道号
     * @param pulse_width_ns 脉冲宽度(纳秒)
     * @return 设置结果
     */
    pwm_result_t (*set_pulse_width)(uint8_t channel, uint32_t pulse_width_ns);

    /**
     * @brief 获取PWM脉冲宽度
     * @param channel PWM通道号
     * @param pulse_width_ns 输出脉冲宽度(纳秒)
     * @return 获取结果
     */
    pwm_result_t (*get_pulse_width)(uint8_t channel, uint32_t *pulse_width_ns);

    /**
     * @brief 设置PWM周期
     * @param channel PWM通道号
     * @param period_ns PWM周期(纳秒)
     * @return 设置结果
     */
    pwm_result_t (*set_period)(uint8_t channel, uint32_t period_ns);

    /**
     * @brief 获取PWM周期
     * @param channel PWM通道号
     * @param period_ns 输出PWM周期(纳秒)
     * @return 获取结果
     */
    pwm_result_t (*get_period)(uint8_t channel, uint32_t *period_ns);

    /**
     * @brief 设置PWM极性
     * @param channel PWM通道号
     * @param polarity PWM极性
     * @return 设置结果
     */
    pwm_result_t (*set_polarity)(uint8_t channel, pwm_polarity_t polarity);

    /**
     * @brief 获取PWM极性
     * @param channel PWM通道号
     * @param polarity 输出PWM极性
     * @return 获取结果
     */
    pwm_result_t (*get_polarity)(uint8_t channel, pwm_polarity_t *polarity);

    /**
     * @brief 增加PWM占空比
     * @param channel PWM通道号
     * @param increment 增加量(0-1000)
     * @return 设置结果
     */
    pwm_result_t (*increase_duty)(uint8_t channel, uint16_t increment);

    /**
     * @brief 减少PWM占空比
     * @param channel PWM通道号
     * @param decrement 减少量(0-1000)
     * @return 设置结果
     */
    pwm_result_t (*decrease_duty)(uint8_t channel, uint16_t decrement);

    /**
     * @brief 增加PWM频率
     * @param channel PWM通道号
     * @param increment 增加量(Hz)
     * @return 设置结果
     */
    pwm_result_t (*increase_frequency)(uint8_t channel, uint32_t increment);

    /**
     * @brief 减少PWM频率
     * @param channel PWM通道号
     * @param decrement 减少量(Hz)
     * @return 设置结果
     */
    pwm_result_t (*decrease_frequency)(uint8_t channel, uint32_t decrement);

    /**
     * @brief 启用PWM更新中断
     * @param channel PWM通道号
     * @param callback 中断回调函数
     * @param user_data 用户数据
     * @return 使能结果
     */
    pwm_result_t (*enable_update_interrupt)(uint8_t channel, pwm_update_callback_t callback, void *user_data);

    /**
     * @brief 禁用PWM更新中断
     * @param channel PWM通道号
     * @return 禁用结果
     */
    pwm_result_t (*disable_update_interrupt)(uint8_t channel);

    /**
     * @brief 启用PWM故障中断
     * @param channel PWM通道号
     * @param callback 故障回调函数
     * @param user_data 用户数据
     * @return 使能结果
     */
    pwm_result_t (*enable_fault_interrupt)(uint8_t channel, pwm_fault_callback_t callback, void *user_data);

    /**
     * @brief 禁用PWM故障中断
     * @param channel PWM通道号
     * @return 禁用结果
     */
    pwm_result_t (*disable_fault_interrupt)(uint8_t channel);

    /**
     * @brief 触发PWM刹车
     * @param channel PWM通道号
     * @return 刹车结果
     */
    pwm_result_t (*trigger_brake)(uint8_t channel);

    /**
     * @brief 释放PWM刹车
     * @param channel PWM通道号
     * @return 释放结果
     */
    pwm_result_t (*release_brake)(uint8_t channel);

    /**
     * @brief 同步多个PWM通道
     * @param channels 通道数组
     * @param count 通道数量
     * @return 同步结果
     */
    pwm_result_t (*synchronize_channels)(const uint8_t *channels, uint8_t count);

    /**
     * @brief 设置PWM相位偏移
     * @param channel PWM通道号
     * @param phase_shift_ns 相位偏移(纳秒)
     * @return 设置结果
     */
    pwm_result_t (*set_phase_shift)(uint8_t channel, uint32_t phase_shift_ns);

    /**
     * @brief 获取PWM当前状态
     * @param channel PWM通道号
     * @param is_running 输出是否在运行
     * @param is_active 输出当前是否活跃
     * @return 获取结果
     */
    pwm_result_t (*get_state)(uint8_t channel, bool *is_running, bool *is_active);

    /**
     * @brief 检查PWM通道是否有效
     * @param channel PWM通道号
     * @param valid 输出是否有效标志
     * @return 检查结果
     */
    pwm_result_t (*is_valid_channel)(uint8_t channel, bool *valid);

    /**
     * @brief 获取PWM最大通道数
     * @return 最大通道数
     */
    uint8_t (*get_channel_count)(void);

    /**
     * @brief 获取PWM最小频率
     * @return 最小频率(Hz)
     */
    uint32_t (*get_min_frequency)(void);

    /**
     * @brief 获取PWM最大频率
     * @return 最大频率(Hz)
     */
    uint32_t (*get_max_frequency)(void);

    /**
     * @brief 获取PWM频率分辨率
     * @return 频率分辨率(Hz)
     */
    uint32_t (*get_frequency_resolution)(void);

    /**
     * @brief 获取PWM驱动版本信息
     * @return 版本号
     */
    uint32_t (*get_version)(void);
} pwm_interface_t;

/* 全局PWM接口实例 */
extern const pwm_interface_t *pwm_hal;

/* 便捷宏定义 */
#define PWM_DUTY_TO_PERCENTAGE(duty) \
    ((float)(duty) / 10.0f)

#define PWM_PERCENTAGE_TO_DUTY(percentage) \
    ((uint16_t)((percentage) * 10.0f + 0.5f))

#define PWM_FREQUENCY_TO_PERIOD(freq) \
    (1000000000UL / (freq))

#define PWM_PERIOD_TO_FREQUENCY(period_ns) \
    (1000000000UL / (period_ns))

#define PWM_PULSE_TO_DUTY(pulse_ns, period_ns) \
    (((pulse_ns) * 1000U) / (period_ns))

#define PWM_DUTY_TO_PULSE(duty, period_ns) \
    (((duty) * (period_ns)) / 1000U)

/* 常用配置宏 */
#define PWM_CHANNEL_CONFIG_DEFAULT(channel, freq) \
    { (channel), (freq), 0, 0, PWM_POLARITY_NORMAL, PWM_ALIGN_EDGE, PWM_COUNT_UP, PWM_MODE_SINGLE, false, false, 0 }

#define PWM_LED_CONFIG(channel, freq) \
    { (channel), (freq), 0, 0, PWM_POLARITY_NORMAL, PWM_ALIGN_EDGE, PWM_COUNT_UP, PWM_MODE_SINGLE, true, false, 0 }

#define PWM_BACKLIGHT_CONFIG(channel, freq) \
    { (channel), (freq), 0, 0, PWM_POLARITY_NORMAL, PWM_ALIGN_CENTER, PWM_COUNT_UP, PWM_MODE_SINGLE, true, false, 0 }

#define PWM_MOTOR_CONFIG(channel, freq) \
    { (channel), (freq), 0, 0, PWM_POLARITY_NORMAL, PWM_ALIGN_CENTER, PWM_COUNT_UP_DOWN, PWM_MODE_COMPLEMENTARY, true, true, 0 }

/* 快速控制宏 */
#define PWM_SET_LEVEL(channel, level) \
    pwm_hal->set_duty_percentage((channel), (level) ? 100.0f : 0.0f)

#define PWM_SET_BRIGHTNESS(channel, brightness) \
    pwm_hal->set_duty_percentage((channel), (brightness) * 100.0f)

#define PWM_GET_BRIGHTNESS(channel) \
    ({ float _percentage; pwm_hal->get_duty_percentage((channel), &_percentage); _percentage / 100.0f; })

/* LED亮度控制宏 */
#define PWM_LED_ON(channel)       PWM_SET_LEVEL((channel), true)
#define PWM_LED_OFF(channel)      PWM_SET_LEVEL((channel), false)
#define PWM_LED_TOGGLE(channel)   PWM_SET_LEVEL((channel), !PWM_GET_BRIGHTNESS(channel))
#define PWM_LED_SET(channel, val) PWM_SET_BRIGHTNESS((channel), (val))
#define PWM_LED_GET(channel)      PWM_GET_BRIGHTNESS(channel)

/* PWM错误处理宏 */
#define PWM_CHECK(expr) \
    do { \
        pwm_result_t _result = (expr); \
        if (_result != PWM_OK) { \
            return _result; \
        } \
    } while(0)

#define PWM_CHECK_RET(expr, ret_val) \
    do { \
        pwm_result_t _result = (expr); \
        if (_result != PWM_OK) { \
            return (ret_val); \
        } \
    } while(0)

/* 呼吸灯效果宏 */
#define PWM_BREATH_STEP(channel, from_percent, to_percent, step, delay_ms) \
    do { \
        float _current = (from_percent); \
        float _target = (to_percent); \
        float _step_size = ((_target > _current) ? (step) : -(step)); \
        \
        while ((_step_size > 0 && _current < _target) || (_step_size < 0 && _current > _target)) { \
            _current += _step_size; \
            if (_step_size > 0 && _current > _target) _current = _target; \
            if (_step_size < 0 && _current < _target) _current = _target; \
            \
            PWM_SET_BRIGHTNESS((channel), _current / 100.0f); \
            /* 延时 delay_ms 毫秒 */ \
        } \
    } while(0)

#ifdef __cplusplus
}
#endif

#endif /* PWM_INTERFACE_H */