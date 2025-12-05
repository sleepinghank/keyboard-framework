/**
 * @file output_interface.h
 * @brief 输出设备驱动接口定义
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * 提供LED、背光、指示灯、振动等输出设备的统一接口
 */

#ifndef OUTPUT_INTERFACE_H
#define OUTPUT_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 输出设备错误码定义 */
typedef enum {
    OUTPUT_OK = 0,                   /* 操作成功 */
    OUTPUT_ERROR_INVALID_DEVICE,     /* 无效设备 */
    OUTPUT_ERROR_INVALID_PARAM,      /* 无效参数 */
    OUTPUT_ERROR_NOT_SUPPORTED,     /* 操作不支持 */
    OUTPUT_ERROR_HARDWARE_FAULT,    /* 硬件故障 */
    OUTPUT_ERROR_TIMEOUT,           /* 操作超时 */
    OUTPUT_ERROR_ALREADY_INITIALIZED,/* 已初始化 */
    OUTPUT_ERROR_NOT_INITIALIZED,   /* 未初始化 */
    OUTPUT_ERROR_INVALID_VALUE,     /* 无效值 */
    OUTPUT_ERROR_DEVICE_BUSY,        /* 设备忙碌 */
    OUTPUT_ERROR_PWM_CHANNEL_UNAVAIL, /* PWM通道不可用 */
    OUTPUT_ERROR_TIMER_UNAVAIL      /* 定时器不可用 */
} output_result_t;

/* 输出设备类型定义 */
typedef enum {
    OUTPUT_TYPE_LED = 0,             /* LED指示灯 */
    OUTPUT_TYPE_BACKLIGHT,          /* 背光 */
    OUTPUT_TYPE_RGB,                /* RGB LED */
    OUTPUT_TYPE_MATRIX_LED,         /* LED矩阵 */
    OUTPUT_TYPE_INDICATOR,          /* 状态指示灯 */
    OUTPUT_TYPE_BUZZER,             /* 蜂鸣器 */
    OUTPUT_TYPE_VIBRATION,          /* 振动马达 */
    OUTPUT_TYPE_DISPLAY             /* 显示屏 */
} output_device_type_t;

/* 输出设备状态定义 */
typedef enum {
    OUTPUT_STATE_OFF = 0,            /* 关闭状态 */
    OUTPUT_STATE_ON,                /* 开启状态 */
    OUTPUT_STATE_DIMMING,           /* 调光状态 */
    OUTPUT_STATE_BLINKING,          /* 闪烁状态 */
    OUTPUT_STATE_BREATHING,         /* 呼吸状态 */
    OUTPUT_STATE_FADING,            /* 渐变状态 */
    OUTPUT_STATE_ANIMATING,         /* 动画状态 */
    OUTPUT_STATE_ERROR             /* 错误状态 */
} output_device_state_t;

/* LED颜色定义 */
typedef struct {
    uint8_t red;                     /* 红色分量 (0-255) */
    uint8_t green;                   /* 绿色分量 (0-255) */
    uint8_t blue;                    /* 蓝色分量 (0-255) */
} rgb_color_t;

/* HSV颜色定义 */
typedef struct {
    float hue;                      /* 色相 (0-360) */
    float saturation;               /* 饱和度 (0-1) */
    float value;                    /* 明度 (0-1) */
} hsv_color_t;

/* LED配置结构体 */
typedef struct {
    uint8_t                 pin;            /* LED引脚 */
    bool                    active_low;     /* 低电平有效 */
    bool                    pwm_enabled;    /* 启用PWM控制 */
    uint8_t                 pwm_channel;    /* PWM通道 */
    uint8_t                 timer_id;       /* 定时器ID */
    uint8_t                 brightness;     /* 默认亮度 (0-255) */
    bool                    blink_enabled;  /* 启用闪烁 */
    uint16_t                blink_period;   /* 闪烁周期(毫秒) */
    float                   blink_duty;     /* 闪烁占空比 (0.0-1.0) */
} led_config_t;

/* 背光配置结构体 */
typedef struct {
    uint8_t                 pin;            /* 背光控制引脚 */
    uint8_t                 pwm_channel;    /* PWM通道 */
    uint8_t                 timer_id;       /* 定时器ID */
    uint8_t                 brightness;     /* 默认亮度 (0-100%) */
    uint8_t                 min_brightness; /* 最小亮度 */
    uint8_t                 max_brightness; /* 最大亮度 */
    uint8_t                 fade_steps;     /* 渐变步数 */
    uint16_t                fade_delay;     /* 渐变延迟(毫秒) */
    bool                    auto_dim;       /* 自动调光 */
    uint16_t                auto_dim_delay; /* 自动调光延迟(秒) */
    bool                    sleep_mode;     /* 睡眠模式 */
} backlight_config_t;

/* RGB LED配置结构体 */
typedef struct {
    uint8_t                 red_pin;        /* 红色LED引脚 */
    uint8_t                 green_pin;      /* 绿色LED引脚 */
    uint8_t                 blue_pin;       /* 蓝色LED引脚 */
    uint8_t                 red_pwm;        /* 红色PWM通道 */
    uint8_t                 green_pwm;      /* 绿色PWM通道 */
    uint8_t                 blue_pwm;       /* 蓝色PWM通道 */
    uint8_t                 timer_id;       /* 定时器ID */
    rgb_color_t             default_color;  /* 默认颜色 */
    uint8_t                 brightness;     /* 亮度 (0-255) */
    bool                    common_anode;   /* 共阳极 */
    bool                    enable_gamma;   /* 启用伽马校正 */
    bool                    enable_temp;    /* 启用色温调整 */
} rgb_config_t;

/* LED矩阵配置结构体 */
typedef struct {
    uint8_t                 rows;           /* 行数 */
    uint8_t                 cols;           /* 列数 */
    uint8_t                *row_pins;       /* 行引脚数组 */
    uint8_t                *col_pins;       /* 列引脚数组 */
    bool                    common_anode;   /* 共阳极 */
    uint8_t                 scan_interval;  /* 扫描间隔(微秒) */
    uint8_t                 brightness;     /* 亮度 (0-255) */
    bool                    enable_gamma;   /* 启用伽马校正 */
    bool                    enable_fade;    /* 启用渐变效果 */
} matrix_led_config_t;

/* 指示灯配置结构体 */
typedef struct {
    uint8_t                 pin;            /* 指示灯引脚 */
    bool                    active_low;     /* 低电平有效 */
    uint8_t                 pwm_channel;    /* PWM通道 */
    uint8_t                 timer_id;       /* 定时器ID */
    bool                    rgb_enabled;    /* RGB支持 */
    uint8_t                 red_pin;        /* 红色引脚 */
    uint8_t                 green_pin;      /* 绿色引脚 */
    uint8_t                 blue_pin;       /* 蓝色引脚 */
    uint8_t                 brightness;     /* 亮度 (0-255) */
} indicator_config_t;

/* 蜂鸣器配置结构体 */
typedef struct {
    uint8_t                 pin;            /* 蜂鸣器引脚 */
    uint8_t                 pwm_channel;    /* PWM通道 */
    uint8_t                 timer_id;       /* 定时器ID */
    bool                    active_low;     /* 低电平有效 */
    uint16_t                frequency;      /* 默认频率(Hz) */
    uint8_t                 volume;         /* 音量 (0-100%) */
    bool                    enable_melody;  /* 启用旋律播放 */
} buzzer_config_t;

/* 振动马达配置结构体 */
typedef struct {
    uint8_t                 pin;            /* 振动引脚 */
    uint8_t                 pwm_channel;    /* PWM通道 */
    uint8_t                 timer_id;       /* 定时器ID */
    bool                    active_low;     /* 低电平有效 */
    uint8_t                 intensity;      /* 振动强度 (0-100%) */
    uint16_t                duration;       /* 默认持续时间(毫秒) */
    bool                    enable_patterns; /* 启用振动模式 */
} vibration_config_t;

/* LED特效类型定义 */
typedef enum {
    LED_EFFECT_NONE = 0,             /* 无特效 */
    LED_EFFECT_BLINK,                /* 闪烁 */
    LED_EFFECT_BREATHING,            /* 呼吸 */
    LED_EFFECT_FADE_IN,              /* 渐入 */
    LED_EFFECT_FADE_OUT,             /* 渐出 */
    LED_EFFECT_RAINBOW,              /* 彩虹 */
    LED_EFFECT_CHASE,                /* 追逐 */
    LED_EFFECT_RANDOM,               /* 随机 */
    LED_EFFECT_WAVE,                 /* 波浪 */
    LED_EFFECT_SPARKLE,              /* 闪烁 */
    LED_EFFECT_PULSE,                /* 脉冲 */
    LED_EFFECT_GRADIENT              /* 渐变 */
} led_effect_type_t;

/* LED特效配置结构体 */
typedef struct {
    led_effect_type_t      effect;         /* 特效类型 */
    rgb_color_t            start_color;    /* 起始颜色 */
    rgb_color_t            end_color;      /* 结束颜色 */
    uint16_t                period;         /* 特效周期(毫秒) */
    uint8_t                 steps;          /* 特效步数 */
    bool                    repeat;         /* 重复播放 */
    uint8_t                 repeat_count;   /* 重复次数 */
    bool                    reverse;        /* 反向播放 */
} led_effect_config_t;

/* 输出设备通用配置结构体 */
typedef struct {
    output_device_type_t    type;           /* 设备类型 */
    output_device_state_t   state;          /* 设备状态 */
    uint8_t                 priority;       /* 设备优先级 */
    bool                    enable_interrupt; /* 启用中断模式 */
    bool                    enable_timer;    /* 启用定时器模式 */
    union {
        led_config_t        led;            /* LED配置 */
        backlight_config_t  backlight;      /* 背光配置 */
        rgb_config_t        rgb;            /* RGB LED配置 */
        matrix_led_config_t matrix;         /* LED矩阵配置 */
        indicator_config_t  indicator;      /* 指示灯配置 */
        buzzer_config_t     buzzer;         /* 蜂鸣器配置 */
        vibration_config_t  vibration;      /* 振动配置 */
    } device_config;
} output_device_config_t;

/* 输出设备接口函数指针结构体 */
typedef struct {
    /**
     * @brief 初始化输出设备
     * @param config 设备配置
     * @return 初始化结果
     */
    output_result_t (*init)(const output_device_config_t *config);

    /**
     * @brief 反初始化输出设备
     * @return 反初始化结果
     */
    output_result_t (*deinit)(void);

    /**
     * @brief 启用输出设备
     * @return 启用结果
     */
    output_result_t (*enable)(void);

    /**
     * @brief 禁用输出设备
     * @return 禁用结果
     */
    output_result_t (*disable)(void);

    /**
     * @brief 设置输出状态
     * @param state 输出状态
     * @return 设置结果
     */
    output_result_t (*set_state)(output_device_state_t state);

    /**
     * @brief 获取输出状态
     * @param state 输出状态
     * @return 获取结果
     */
    output_result_t (*get_state)(output_device_state_t *state);

    /**
     * @brief 设置亮度
     * @param brightness 亮度值 (0-255)
     * @return 设置结果
     */
    output_result_t (*set_brightness)(uint8_t brightness);

    /**
     * @brief 获取亮度
     * @param brightness 输出亮度值
     * @return 获取结果
     */
    output_result_t (*get_brightness)(uint8_t *brightness);

    /**
     * @brief 设置颜色 (RGB设备)
     * @param color RGB颜色值
     * @return 设置结果
     */
    output_result_t (*set_color)(rgb_color_t color);

    /**
     * @brief 获取颜色 (RGB设备)
     * @param color 输出RGB颜色值
     * @return 获取结果
     */
    output_result_t (*get_color)(rgb_color_t *color);

    /**
     * @brief 设置HSV颜色
     * @param hsv HSV颜色值
     * @return 设置结果
     */
    output_result_t (*set_hsv_color)(hsv_color_t hsv);

    /**
     * @brief 获取HSV颜色
     * @param hsv 输出HSV颜色值
     * @return 获取结果
     */
    output_result_t (*get_hsv_color)(hsv_color_t *hsv);

    /**
     * @brief 开始播放特效
     * @param effect 特效配置
     * @return 开始播放结果
     */
    output_result_t (*start_effect)(const led_effect_config_t *effect);

    /**
     * @brief 停止播放特效
     * @return 停止播放结果
     */
    output_result_t (*stop_effect)(void);

    /**
     * @brief 设置PWM频率
     * @param frequency PWM频率(Hz)
     * @return 设置结果
     */
    output_result_t (*set_pwm_frequency)(uint16_t frequency);

    /**
     * @brief 获取PWM频率
     * @param frequency 输出PWM频率
     * @return 获取结果
     */
    output_result_t (*get_pwm_frequency)(uint16_t *frequency);

    /**
     * @brief 设置PWM占空比
     * @param duty 占空比 (0.0-1.0)
     * @return 设置结果
     */
    output_result_t (*set_pwm_duty)(float duty);

    /**
     * @brief 获取PWM占空比
     * @param duty 输出占空比
     * @return 获取结果
     */
    output_result_t (*get_pwm_duty)(float *duty);

    /**
     * @brief 开始渐变
     * @param target_color 目标颜色
     * @param duration 渐变时间(毫秒)
     * @return 开始渐变结果
     */
    output_result_t (*start_fade)(rgb_color_t target_color, uint16_t duration);

    /**
     * @brief 停止渐变
     * @return 停止渐变结果
     */
    output_result_t (*stop_fade)(void);

    /**
     * @brief 设备更新处理
     * @return 更新结果
     */
    output_result_t (*update)(void);

    /**
     * @brief 重置设备
     * @return 重置结果
     */
    output_result_t (*reset)(void);

    /**
     * @brief 获取设备信息
     * @param info 输出设备信息
     * @return 获取结果
     */
    output_result_t (*get_info)(char *info, uint16_t max_len);

    /**
     * @brief 获取设备版本
     * @return 版本号
     */
    uint32_t (*get_version)(void);
} output_device_interface_t;

/* 输出管理器接口函数指针结构体 */
typedef struct {
    /**
     * @brief 初始化输出管理器
     * @return 初始化结果
     */
    output_result_t (*init)(void);

    /**
     * @brief 反初始化输出管理器
     * @return 反初始化结果
     */
    output_result_t (*deinit)(void);

    /**
     * @brief 注册输出设备
     * @param device_type 设备类型
     * @param device_interface 设备接口
     * @param config 设备配置
     * @param device_id 输出设备ID
     * @return 注册结果
     */
    output_result_t (*register_device)(output_device_type_t device_type,
                                       const output_device_interface_t *device_interface,
                                       const output_device_config_t *config,
                                       uint8_t *device_id);

    /**
     * @brief 注销输出设备
     * @param device_id 设备ID
     * @return 注销结果
     */
    output_result_t (*unregister_device)(uint8_t device_id);

    /**
     * @brief 启用指定设备
     * @param device_id 设备ID
     * @return 启用结果
     */
    output_result_t (*enable_device)(uint8_t device_id);

    /**
     * @brief 禁用指定设备
     * @param device_id 设备ID
     * @return 禁用结果
     */
    output_result_t (*disable_device)(uint8_t device_id);

    /**
     * @brief 启用所有设备
     * @return 启用结果
     */
    output_result_t (*enable_all_devices)(void);

    /**
     * @brief 禁用所有设备
     * @return 禁用结果
     */
    output_result_t (*disable_all_devices)(void);

    /**
     * @brief 设置全局亮度
     * @param brightness 亮度值 (0-255)
     * @return 设置结果
     */
    output_result_t (*set_global_brightness)(uint8_t brightness);

    /**
     * @brief 获取全局亮度
     * @param brightness 输出亮度值
     * @return 获取结果
     */
    output_result_t (*get_global_brightness)(uint8_t *brightness);

    /**
     * @brief 设置全局颜色
     * @param color RGB颜色值
     * @return 设置结果
     */
    output_result_t (*set_global_color)(rgb_color_t color);

    /**
     * @brief 处理所有设备更新
     * @return 处理结果
     */
    output_result_t (*update_all_devices)(void);

    /**
     * @brief 进入睡眠模式
     * @return 进入结果
     */
    output_result_t (*enter_sleep_mode)(void);

    /**
     * @brief 退出睡眠模式
     * @return 退出结果
     */
    output_result_t (*exit_sleep_mode)(void);

    /**
     * @brief 获取已注册设备数量
     * @return 设备数量
     */
    uint8_t (*get_device_count)(void);

    /**
     * @brief 获取输出管理器版本
     * @return 版本号
     */
    uint32_t (*get_version)(void);
} output_manager_interface_t;

/* 全局输出管理器实例 */
extern const output_manager_interface_t *output_manager;

/* 颜色转换函数 */
rgb_color_t hsv_to_rgb(hsv_color_t hsv);
hsv_color_t rgb_to_hsv(rgb_color_t rgb);
rgb_color_t fade_color(rgb_color_t start, rgb_color_t end, float progress);

/* 常用颜色定义 */
#define COLOR_BLACK     {0, 0, 0}
#define COLOR_WHITE     {255, 255, 255}
#define COLOR_RED       {255, 0, 0}
#define COLOR_GREEN     {0, 255, 0}
#define COLOR_BLUE      {0, 0, 255}
#define COLOR_YELLOW    {255, 255, 0}
#define COLOR_CYAN      {0, 255, 255}
#define COLOR_MAGENTA   {255, 0, 255}
#define COLOR_ORANGE    {255, 165, 0}
#define COLOR_PURPLE    {128, 0, 128}

/* 常用配置宏 */
#define LED_CONFIG_DEFAULT(pin) \
    { (pin), true, false, 0, 0, 255, false, 0, 0.5f }

#define BACKLIGHT_CONFIG_DEFAULT(pin) \
    { (pin), 0, 0, 50, 5, 95, 10, 50, false, 0, true }

#define RGB_CONFIG_DEFAULT(r_pin, g_pin, b_pin) \
    { (r_pin), (g_pin), (b_pin), 0, 0, 0, 0, COLOR_WHITE, 255, true, false, false }

#define INDICATOR_CONFIG_DEFAULT(pin) \
    { (pin), true, 0, 0, false, 0, 0, 0, 255 }

/* 输出错误处理宏 */
#define OUTPUT_CHECK(expr) \
    do { \
        output_result_t _result = (expr); \
        if (_result != OUTPUT_OK) { \
            return _result; \
        } \
    } while(0)

#define OUTPUT_CHECK_RET(expr, ret_val) \
    do { \
        output_result_t _result = (expr); \
        if (_result != OUTPUT_OK) { \
            return (ret_val); \
        } \
    } while(0)

/* 便捷操作宏 */
#define OUTPUT_ENABLE_ALL() \
    output_manager->enable_all_devices()

#define OUTPUT_DISABLE_ALL() \
    output_manager->disable_all_devices()

#define OUTPUT_UPDATE_ALL() \
    output_manager->update_all_devices()

#define OUTPUT_SET_GLOBAL_COLOR(r, g, b) \
    output_manager->set_global_color((rgb_color_t){(r), (g), (b)})

#define OUTPUT_ENTER_SLEEP() \
    output_manager->enter_sleep_mode()

#define OUTPUT_EXIT_SLEEP() \
    output_manager->exit_sleep_mode()

#ifdef __cplusplus
}
#endif

#endif /* OUTPUT_INTERFACE_H */