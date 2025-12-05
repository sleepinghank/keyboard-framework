/**
 * @file system_interface.h
 * @brief 系统驱动接口定义
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * 提供定时器、看门狗、时钟、原子操作、事件处理、等待等系统服务的统一接口
 */

#ifndef SYSTEM_INTERFACE_H
#define SYSTEM_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 系统错误码定义 */
typedef enum {
    SYSTEM_OK = 0,                  /* 操作成功 */
    SYSTEM_ERROR_INVALID_PARAM,     /* 无效参数 */
    SYSTEM_ERROR_NOT_SUPPORTED,     /* 操作不支持 */
    SYSTEM_ERROR_HARDWARE_FAULT,    /* 硬件故障 */
    SYSTEM_ERROR_TIMEOUT,           /* 操作超时 */
    SYSTEM_ERROR_ALREADY_INITIALIZED,/* 已初始化 */
    SYSTEM_ERROR_NOT_INITIALIZED,   /* 未初始化 */
    SYSTEM_ERROR_INVALID_ID,        /* 无效ID */
    SYSTEM_ERROR_ID_IN_USE,         /* ID已被使用 */
    SYSTEM_ERROR_NO_MEMORY,         /* 内存不足 */
    SYSTEM_ERROR_TIMER_EXPIRED,     /* 定时器已过期 */
    SYSTEM_ERROR_WATCHDOG_TRIGGERED, /* 看门狗触发 */
    SYSTEM_ERROR_CONTEXT_SWITCH,    /* 上下文切换失败 */
    SYSTEM_ERROR_INTERRUPT_LEVEL,   /* 中断级别错误 */
    SYSTEM_ERROR_INVALID_STATE,     /* 无效状态 */
    SYSTEM_ERROR_QUEUE_FULL,        /* 队列满 */
    SYSTEM_ERROR_QUEUE_EMPTY,       /* 队列空 */
    SYSTEM_ERROR_EVENT_NOT_SET,     /* 事件未设置 */
    SYSTEM_ERROR_EVENT_ALREADY_SET  /* 事件已设置 */
} system_result_t;

/* 定时器类型定义 */
typedef enum {
    TIMER_TYPE_ONE_SHOT = 0,        /* 单次定时器 */
    TIMER_TYPE_PERIODIC,            /* 周期定时器 */
    TIMER_TYPE_CONTINUOUS,         /* 连续定时器 */
    TIMER_TYPE_DELAY,              /* 延迟定时器 */
    TIMER_TYPE_COUNTDOWN,          /* 倒计时定时器 */
    TIMER_TYPE_STOPWATCH           /* 秒表定时器 */
} timer_type_t;

/* 定时器状态定义 */
typedef enum {
    TIMER_STATE_STOPPED = 0,        /* 停止状态 */
    TIMER_STATE_RUNNING,           /* 运行状态 */
    TIMER_STATE_PAUSED,            /* 暂停状态 */
    TIMER_STATE_EXPIRED,           /* 过期状态 */
    TIMER_STATE_ERROR              /* 错误状态 */
} timer_state_t;

/* 定时器精度定义 */
typedef enum {
    TIMER_PRECISION_MS = 0,         /* 毫秒精度 */
    TIMER_PRECISION_US,            /* 微秒精度 */
    TIMER_PRECISION_NS,            /* 纳秒精度 */
    TIMER_PRECISION_TICKS          /* 时钟滴答精度 */
} timer_precision_t;

/* 时钟源定义 */
typedef enum {
    CLOCK_SOURCE_INTERNAL = 0,      /* 内部时钟源 */
    CLOCK_SOURCE_EXTERNAL,         /* 外部时钟源 */
    CLOCK_SOURCE_LSI,              /* 内部低速时钟 */
    CLOCK_SOURCE_LSE,              /* 外部低速时钟 */
    CLOCK_SOURCE_HSI,              /* 内部高速时钟 */
    CLOCK_SOURCE_HSE,              /* 外部高速时钟 */
    CLOCK_SOURCE_PLL,              /* 锁相环时钟 */
    CLOCK_SOURCE_RTC               /* RTC时钟 */
} clock_source_t;

/* 看门狗类型定义 */
typedef enum {
    WATCHDOG_TYPE_IWDG = 0,        /* 独立看门狗 */
    WATCHDOG_TYPE_WWDG,            /* 窗口看门狗 */
    WATCHDOG_TYPE_SOFTWARE         /* 软件看门狗 */
} watchdog_type_t;

/* 看门狗状态定义 */
typedef enum {
    WATCHDOG_STATE_STOPPED = 0,    /* 停止状态 */
    WATCHDOG_STATE_RUNNING,        /* 运行状态 */
    WATCHDOG_STATE_TRIGGERED,      /* 触发状态 */
    WATCHDOG_STATE_REFRESHED,      /* 已刷新状态 */
    WATCHDOG_STATE_ERROR           /* 错误状态 */
} watchdog_state_t;

/* 事件类型定义 */
typedef enum {
    EVENT_TYPE_FLAG = 0,           /* 标志事件 */
    EVENT_TYPE_SEMAPHORE,          /* 信号量事件 */
    EVENT_TYPE_QUEUE,              /* 队列事件 */
    EVENT_TYPE_MESSAGE,            /* 消息事件 */
    EVENT_TYPE_SIGNAL,            /* 信号事件 */
    EVENT_TYPE_CONDITION,          /* 条件事件 */
    EVENT_TYPE_TIMEOUT             /* 超时事件 */
} event_type_t;

/* 中断级别定义 */
typedef enum {
    INTERRUPT_LEVEL_LOW = 0,       /* 低中断级别 */
    INTERRUPT_LEVEL_MEDIUM,        /* 中中断级别 */
    INTERRUPT_LEVEL_HIGH,          /* 高中断级别 */
    INTERRUPT_LEVEL_CRITICAL,      /* 关键中断级别 */
    INTERRUPT_LEVEL_NONMASKABLE    /* 不可屏蔽中断 */
} interrupt_level_t;

/* 定时器配置结构体 */
typedef struct {
    timer_type_t            type;           /* 定时器类型 */
    timer_precision_t       precision;      /* 定时器精度 */
    clock_source_t          clock_source;   /* 时钟源 */
    uint32_t                timeout;        /* 超时时间 */
    uint32_t                period;         /* 周期时间 */
    bool                    auto_reload;    /* 自动重载 */
    bool                    interrupt_enabled; /* 启用中断 */
    interrupt_level_t       interrupt_level; /* 中断级别 */
    void                   *user_data;      /* 用户数据 */
} timer_config_t;

/* 定时器信息结构体 */
typedef struct {
    uint8_t                 timer_id;       /* 定时器ID */
    timer_type_t            type;           /* 定时器类型 */
    timer_state_t           state;          /* 定时器状态 */
    uint32_t                timeout;        /* 超时时间 */
    uint32_t                remaining_time; /* 剩余时间 */
    uint32_t                elapsed_time;   /* 已用时间 */
    uint32_t                period_count;   /* 周期计数 */
    uint32_t                start_timestamp; /* 启动时间戳 */
    uint32_t                expire_timestamp; /* 过期时间戳 */
    bool                    auto_reload;    /* 自动重载 */
    void                   *user_data;      /* 用户数据 */
} timer_info_t;

/* 看门狗配置结构体 */
typedef struct {
    watchdog_type_t         type;           /* 看门狗类型 */
    uint32_t                timeout_ms;     /* 超时时间(毫秒) */
    uint16_t                window_ms;      /* 窗口时间(毫秒) */
    bool                    auto_start;     /* 自动启动 */
    bool                    interrupt_enabled; /* 启用中断 */
    bool                    reset_enabled;  /* 启用复位 */
    interrupt_level_t       interrupt_level; /* 中断级别 */
    void                   *user_data;      /* 用户数据 */
} watchdog_config_t;

/* 看门狗信息结构体 */
typedef struct {
    uint8_t                 watchdog_id;    /* 看门狗ID */
    watchdog_type_t         type;           /* 看门狗类型 */
    watchdog_state_t        state;          /* 看门狗状态 */
    uint32_t                timeout_ms;     /* 超时时间 */
    uint32_t                remaining_time; /* 剩余时间 */
    uint32_t                last_refresh_time; /* 最后刷新时间 */
    uint32_t                trigger_count;  /* 触发次数 */
    uint32_t                refresh_count;  /* 刷新次数 */
    void                   *user_data;      /* 用户数据 */
} watchdog_info_t;

/* 时钟配置结构体 */
typedef struct {
    clock_source_t          source;         /* 时钟源 */
    uint32_t                frequency;      /* 时钟频率(Hz) */
    uint16_t                prescaler;      /* 预分频器 */
    bool                    auto_calibration; /* 自动校准 */
    bool                    low_power_mode; /* 低功耗模式 */
    uint32_t                accuracy_ppm;   /* 精度(PPM) */
} clock_config_t;

/* 事件配置结构体 */
typedef struct {
    event_type_t            type;           /* 事件类型 */
    uint32_t                max_items;      /* 最大项目数 */
    uint32_t                item_size;      /* 项目大小 */
    bool                    auto_clear;     /* 自动清除 */
    bool                    priority_order; /* 优先级排序 */
    interrupt_level_t       interrupt_level; /* 中断级别 */
    void                   *user_data;      /* 用户数据 */
} event_config_t;

/* 事件信息结构体 */
typedef struct {
    event_type_t            type;           /* 事件类型 */
    uint32_t                event_id;       /* 事件ID */
    uint32_t                item_count;     /* 项目数量 */
    uint32_t                max_items;      /* 最大项目数 */
    uint32_t                wait_count;     /* 等待数量 */
    bool                    is_set;         /* 是否已设置 */
    uint32_t                last_set_time;  /* 最后设置时间 */
    void                   *user_data;      /* 用户数据 */
} event_info_t;

/* 等待类型定义 */
typedef enum {
    WAIT_TYPE_FOREVER = 0,             /* 永远等待 */
    WAIT_TYPE_TIMEOUT,                /* 超时等待 */
    WAIT_TYPE_POLLING,                /* 轮询等待 */
    WAIT_TYPE_CONDITION,              /* 条件等待 */
    WAIT_TYPE_INTERRUPTIBLE           /* 可中断等待 */
} wait_type_t;

/* 等待配置结构体 */
typedef struct {
    wait_type_t             type;           /* 等待类型 */
    uint32_t                timeout_ms;     /* 超时时间(毫秒) */
    uint32_t                poll_interval_ms; /* 轮询间隔(毫秒) */
    bool                    interruptible;  /* 可中断 */
    interrupt_level_t       interrupt_level; /* 中断级别 */
    void                   *user_data;      /* 用户数据 */
} wait_config_t;

/* 系统统计信息结构体 */
typedef struct {
    uint32_t                uptime_seconds; /* 运行时间(秒) */
    uint32_t                uptime_milliseconds; /* 运行时间(毫秒) */
    uint32_t                cpu_usage_percent; /* CPU使用率(%) */
    uint32_t                heap_used_bytes; /* 堆使用字节 */
    uint32_t                heap_free_bytes; /* 堆空闲字节 */
    uint32_t                stack_used_bytes; /* 栈使用字节 */
    uint32_t                stack_free_bytes; /* 栈空闲字节 */
    uint32_t                timer_count;    /* 定时器数量 */
    uint32_t                event_count;    /* 事件数量 */
    uint32_t                interrupt_count; /* 中断次数 */
    uint32_t                context_switch_count; /* 上下文切换次数 */
    float                   temperature_celsius; /* 温度(摄氏度) */
    uint32_t                watchdog_trigger_count; /* 看门狗触发次数 */
} system_statistics_t;

/* 定时器回调函数类型 */
typedef void (*timer_callback_t)(uint8_t timer_id, void *user_data);

/* 看门狗回调函数类型 */
typedef void (*watchdog_callback_t)(uint8_t watchdog_id, void *user_data);

/* 事件回调函数类型 */
typedef void (*event_callback_t)(uint32_t event_id, void *item, void *user_data);

/* 系统回调函数类型 */
typedef void (*system_callback_t)(uint32_t event, void *user_data);

/* 定时器管理器接口函数指针结构体 */
typedef struct {
    /**
     * @brief 初始化定时器管理器
     * @return 初始化结果
     */
    system_result_t (*init)(void);

    /**
     * @brief 反初始化定时器管理器
     * @return 反初始化结果
     */
    system_result_t (*deinit)(void);

    /**
     * @brief 创建定时器
     * @param config 定时器配置
     * @param timer_id 输出定时器ID
     * @return 创建结果
     */
    system_result_t (*create_timer)(const timer_config_t *config, uint8_t *timer_id);

    /**
     * @brief 删除定时器
     * @param timer_id 定时器ID
     * @return 删除结果
     */
    system_result_t (*delete_timer)(uint8_t timer_id);

    /**
     * @brief 启动定时器
     * @param timer_id 定时器ID
     * @param callback 回调函数
     * @return 启动结果
     */
    system_result_t (*start_timer)(uint8_t timer_id, timer_callback_t callback);

    /**
     * @brief 停止定时器
     * @param timer_id 定时器ID
     * @return 停止结果
     */
    system_result_t (*stop_timer)(uint8_t timer_id);

    /**
     * @brief 重启定时器
     * @param timer_id 定时器ID
     * @return 重启结果
     */
    system_result_t (*restart_timer)(uint8_t timer_id);

    /**
     * @brief 暂停定时器
     * @param timer_id 定时器ID
     * @return 暂停结果
     */
    system_result_t (*pause_timer)(uint8_t timer_id);

    /**
     * @brief 恢复定时器
     * @param timer_id 定时器ID
     * @return 恢复结果
     */
    system_result_t (*resume_timer)(uint8_t timer_id);

    /**
     * @brief 设置定时器超时时间
     * @param timer_id 定时器ID
     * @param timeout_ms 超时时间(毫秒)
     * @return 设置结果
     */
    system_result_t (*set_timer_timeout)(uint8_t timer_id, uint32_t timeout_ms);

    /**
     * @brief 获取定时器超时时间
     * @param timer_id 定时器ID
     * @param timeout_ms 输出超时时间
     * @return 获取结果
     */
    system_result_t (*get_timer_timeout)(uint8_t timer_id, uint32_t *timeout_ms);

    /**
     * @brief 获取定时器剩余时间
     * @param timer_id 定时器ID
     * @param remaining_ms 输出剩余时间
     * @return 获取结果
     */
    system_result_t (*get_timer_remaining)(uint8_t timer_id, uint32_t *remaining_ms);

    /**
     * @brief 获取定时器状态
     * @param timer_id 定时器ID
     * @param state 输出定时器状态
     * @return 获取结果
     */
    system_result_t (*get_timer_state)(uint8_t timer_id, timer_state_t *state);

    /**
     * @brief 获取定时器信息
     * @param timer_id 定时器ID
     * @param info 输出定时器信息
     * @return 获取结果
     */
    system_result_t (*get_timer_info)(uint8_t timer_id, timer_info_t *info);

    /**
     * @brief 处理所有定时器
     * @return 处理结果
     */
    system_result_t (*process_timers)(void);

    /**
     * @brief 获取活跃定时器数量
     * @return 定时器数量
     */
    uint8_t (*get_active_timer_count)(void);

    /**
     * @brief 获取定时器管理器版本
     * @return 版本号
     */
    uint32_t (*get_version)(void);
} timer_manager_interface_t;

/* 看门狗管理器接口函数指针结构体 */
typedef struct {
    /**
     * @brief 初始化看门狗管理器
     * @return 初始化结果
     */
    system_result_t (*init)(void);

    /**
     * @brief 反初始化看门狗管理器
     * @return 反初始化结果
     */
    system_result_t (*deinit)(void);

    /**
     * @brief 创建看门狗
     * @param config 看门狗配置
     * @param watchdog_id 输出看门狗ID
     * @return 创建结果
     */
    system_result_t (*create_watchdog)(const watchdog_config_t *config, uint8_t *watchdog_id);

    /**
     * @brief 删除看门狗
     * @param watchdog_id 看门狗ID
     * @return 删除结果
     */
    system_result_t (*delete_watchdog)(uint8_t watchdog_id);

    /**
     * @brief 启动看门狗
     * @param watchdog_id 看门狗ID
     * @param callback 回调函数
     * @return 启动结果
     */
    system_result_t (*start_watchdog)(uint8_t watchdog_id, watchdog_callback_t callback);

    /**
     * @brief 停止看门狗
     * @param watchdog_id 看门狗ID
     * @return 停止结果
     */
    system_result_t (*stop_watchdog)(uint8_t watchdog_id);

    /**
     * @brief 刷新看门狗
     * @param watchdog_id 看门狗ID
     * @return 刷新结果
     */
    system_result_t (*refresh_watchdog)(uint8_t watchdog_id);

    /**
     * @brief 设置看门狗超时时间
     * @param watchdog_id 看门狗ID
     * @param timeout_ms 超时时间(毫秒)
     * @return 设置结果
     */
    system_result_t (*set_watchdog_timeout)(uint8_t watchdog_id, uint32_t timeout_ms);

    /**
     * @brief 获取看门狗超时时间
     * @param watchdog_id 看门狗ID
     * @param timeout_ms 输出超时时间
     * @return 获取结果
     */
    system_result_t (*get_watchdog_timeout)(uint8_t watchdog_id, uint32_t *timeout_ms);

    /**
     * @brief 获取看门狗剩余时间
     * @param watchdog_id 看门狗ID
     * @param remaining_ms 输出剩余时间
     * @return 获取结果
     */
    system_result_t (*get_watchdog_remaining)(uint8_t watchdog_id, uint32_t *remaining_ms);

    /**
     * @brief 获取看门狗状态
     * @param watchdog_id 看门狗ID
     * @param state 输出看门狗状态
     * @return 获取结果
     */
    system_result_t (*get_watchdog_state)(uint8_t watchdog_id, watchdog_state_t *state);

    /**
     * @brief 获取看门狗信息
     * @param watchdog_id 看门狗ID
     * @param info 输出看门狗信息
     * @return 获取结果
     */
    system_result_t (*get_watchdog_info)(uint8_t watchdog_id, watchdog_info_t *info);

    /**
     * @brief 处理所有看门狗
     * @return 处理结果
     */
    system_result_t (*process_watchdogs)(void);

    /**
     * @brief 获取活跃看门狗数量
     * @return 看门狗数量
     */
    uint8_t (*get_active_watchdog_count)(void);

    /**
     * @brief 获取看门狗管理器版本
     * @return 版本号
     */
    uint32_t (*get_version)(void);
} watchdog_manager_interface_t;

/* 时钟管理器接口函数指针结构体 */
typedef struct {
    /**
     * @brief 初始化时钟管理器
     * @param config 时钟配置
     * @return 初始化结果
     */
    system_result_t (*init)(const clock_config_t *config);

    /**
     * @brief 反初始化时钟管理器
     * @return 反初始化结果
     */
    system_result_t (*deinit)(void);

    /**
     * @brief 获取系统时钟频率
     * @param frequency 输出时钟频率
     * @return 获取结果
     */
    system_result_t (*get_system_clock)(uint32_t *frequency);

    /**
     * @brief 获取时钟源频率
     * @param source 时钟源
     * @param frequency 输出时钟频率
     * @return 获取结果
     */
    system_result_t (*get_source_frequency)(clock_source_t source, uint32_t *frequency);

    /**
     * @brief 设置时钟源
     * @param source 时钟源
     * @return 设置结果
     */
    system_result_t (*set_clock_source)(clock_source_t source);

    /**
     * @brief 获取当前时钟源
     * @param source 输出时钟源
     * @return 获取结果
     */
    system_result_t (*get_clock_source)(clock_source_t *source);

    /**
     * @brief 校准时钟
     * @param reference_freq 参考频率
     * @return 校准结果
     */
    system_result_t (*calibrate_clock)(uint32_t reference_freq);

    /**
     * @brief 进入低功耗模式
     * @return 进入结果
     */
    system_result_t (*enter_low_power_mode)(void);

    /**
     * @brief 退出低功耗模式
     * @return 退出结果
     */
    system_result_t (*exit_low_power_mode)(void);

    /**
     * @brief 获取当前时间戳(毫秒)
     * @return 时间戳
     */
    uint32_t (*get_timestamp_ms)(void);

    /**
     * @brief 获取当前时间戳(微秒)
     * @return 时间戳
     */
    uint64_t (*get_timestamp_us)(void);

    /**
     * @brief 延时指定毫秒
     * @param delay_ms 延时时间(毫秒)
     */
    void (*delay_ms)(uint32_t delay_ms);

    /**
     * @brief 延时指定微秒
     * @param delay_us 延时时间(微秒)
     */
    void (*delay_us)(uint32_t delay_us);

    /**
     * @brief 获取时钟管理器版本
     * @return 版本号
     */
    uint32_t (*get_version)(void);
} clock_manager_interface_t;

/* 原子操作接口函数指针结构体 */
typedef struct {
    /**
     * @brief 原子读取32位值
     * @param ptr 指针
     * @return 读取值
     */
    uint32_t (*atomic_read_32)(volatile uint32_t *ptr);

    /**
     * @brief 原子写入32位值
     * @param ptr 指针
     * @param value 写入值
     */
    void (*atomic_write_32)(volatile uint32_t *ptr, uint32_t value);

    /**
     * @brief 原子增加32位值
     * @param ptr 指针
     * @param value 增加值
     * @return 原值
     */
    uint32_t (*atomic_add_32)(volatile uint32_t *ptr, uint32_t value);

    /**
     * @brief 原子减少32位值
     * @param ptr 指针
     * @param value 减少值
     * @return 原值
     */
    uint32_t (*atomic_sub_32)(volatile uint32_t *ptr, uint32_t value);

    /**
     * @brief 原子交换32位值
     * @param ptr 指针
     * @param value 交换值
     * @return 原值
     */
    uint32_t (*atomic_swap_32)(volatile uint32_t *ptr, uint32_t value);

    /**
     * @brief 原子比较并交换32位值
     * @param ptr 指针
     * @param expected 期望值
     * @param desired 新值
     * @return 是否成功
     */
    bool (*atomic_cas_32)(volatile uint32_t *ptr, uint32_t expected, uint32_t desired);

    /**
     * @brief 原子读取32位值并设置标志
     * @param ptr 指针
     * @param mask 标志掩码
     * @return 原值
     */
    uint32_t (*atomic_read_set_flags_32)(volatile uint32_t *ptr, uint32_t mask);

    /**
     * @brief 原子读取32位值并清除标志
     * @param ptr 指针
     * @param mask 标志掩码
     * @return 原值
     */
    uint32_t (*atomic_read_clear_flags_32)(volatile uint32_t *ptr, uint32_t mask);

    /**
     * @brief 禁用中断
     * @return 中断状态
     */
    uint32_t (*disable_interrupts)(void);

    /**
     * @brief 启用中断
     * @param state 中断状态
     */
    void (*enable_interrupts)(uint32_t state);

    /**
     * @brief 获取中断状态
     * @return 中断状态
     */
    bool (*are_interrupts_enabled)(void);

    /**
     * @brief 进入关键区
     * @return 关键区状态
     */
    uint32_t (*enter_critical)(void);

    /**
     * @brief 退出关键区
     * @param state 关键区状态
     */
    void (*exit_critical)(uint32_t state);

    /**
     * @brief 获取原子操作管理器版本
     * @return 版本号
     */
    uint32_t (*get_version)(void);
} atomic_interface_t;

/* 事件管理器接口函数指针结构体 */
typedef struct {
    /**
     * @brief 初始化事件管理器
     * @return 初始化结果
     */
    system_result_t (*init)(void);

    /**
     * @brief 反初始化事件管理器
     * @return 反初始化结果
     */
    system_result_t (*deinit)(void);

    /**
     * @brief 创建事件
     * @param config 事件配置
     * @param event_id 输出事件ID
     * @return 创建结果
     */
    system_result_t (*create_event)(const event_config_t *config, uint32_t *event_id);

    /**
     * @brief 删除事件
     * @param event_id 事件ID
     * @return 删除结果
     */
    system_result_t (*delete_event)(uint32_t event_id);

    /**
     * @brief 设置事件
     * @param event_id 事件ID
     * @return 设置结果
     */
    system_result_t (*set_event)(uint32_t event_id);

    /**
     * @brief 清除事件
     * @param event_id 事件ID
     * @return 清除结果
     */
    system_result_t (*clear_event)(uint32_t event_id);

    /**
     * @brief 等待事件
     * @param event_id 事件ID
     * @param timeout_ms 超时时间(毫秒)
     * @return 等待结果
     */
    system_result_t (*wait_event)(uint32_t event_id, uint32_t timeout_ms);

    /**
     * @brief 发送事件项目
     * @param event_id 事件ID
     * @param item 项目数据
     * @param item_size 项目大小
     * @return 发送结果
     */
    system_result_t (*send_event_item)(uint32_t event_id, const void *item, uint32_t item_size);

    /**
     * @brief 接收事件项目
     * @param event_id 事件ID
     * @param item 输出项目数据
     * @param item_size 项目大小
     * @param timeout_ms 超时时间(毫秒)
     * @return 接收结果
     */
    system_result_t (*receive_event_item)(uint32_t event_id, void *item, uint32_t item_size, uint32_t timeout_ms);

    /**
     * @brief 检查事件是否设置
     * @param event_id 事件ID
     * @param is_set 输出是否设置标志
     * @return 检查结果
     */
    system_result_t (*is_event_set)(uint32_t event_id, bool *is_set);

    /**
     * @brief 获取事件信息
     * @param event_id 事件ID
     * @param info 输出事件信息
     * @return 获取结果
     */
    system_result_t (*get_event_info)(uint32_t event_id, event_info_t *info);

    /**
     * @brief 注册事件回调
     * @param event_id 事件ID
     * @param callback 回调函数
     * @return 注册结果
     */
    system_result_t (*register_event_callback)(uint32_t event_id, event_callback_t callback);

    /**
     * @brief 注销事件回调
     * @param event_id 事件ID
     * @return 注销结果
     */
    system_result_t (*unregister_event_callback)(uint32_t event_id);

    /**
     * @brief 处理所有事件
     * @return 处理结果
     */
    system_result_t (*process_events)(void);

    /**
     * @brief 获取活跃事件数量
     * @return 事件数量
     */
    uint8_t (*get_active_event_count)(void);

    /**
     * @brief 获取事件管理器版本
     * @return 版本号
     */
    uint32_t (*get_version)(void);
} event_manager_interface_t;

/* 等待管理器接口函数指针结构体 */
typedef struct {
    /**
     * @brief 初始化等待管理器
     * @return 初始化结果
     */
    system_result_t (*init)(void);

    /**
     * @brief 反初始化等待管理器
     * @return 反初始化结果
     */
    system_result_t (*deinit)(void);

    /**
     * @brief 等待条件
     * @param condition 条件函数
     * @param timeout_ms 超时时间(毫秒)
     * @return 等待结果
     */
    system_result_t (*wait_condition)(bool (*condition)(void *), void *user_data, uint32_t timeout_ms);

    /**
     * @brief 等待标志
     * @param flag 标志指针
     * @param expected_value 期望值
     * @param timeout_ms 超时时间(毫秒)
     * @return 等待结果
     */
    system_result_t (*wait_flag)(volatile bool *flag, bool expected_value, uint32_t timeout_ms);

    /**
     * @brief 等待32位值
     * @param value 值指针
     * @param expected_value 期望值
     * @param timeout_ms 超时时间(毫秒)
     * @return 等待结果
     */
    system_result_t (*wait_value_32)(volatile uint32_t *value, uint32_t expected_value, uint32_t timeout_ms);

    /**
     * @brief 等待任意标志
     * @param flags 标志指针
     * @param mask 标志掩码
     * @param any_true 是否任一标志为真
     * @param timeout_ms 超时时间(毫秒)
     * @return 等待结果
     */
    system_result_t (*wait_any_flags)(volatile uint32_t *flags, uint32_t mask, bool any_true, uint32_t timeout_ms);

    /**
     * @brief 等待超时
     * @param delay_ms 延时时间(毫秒)
     */
    void (*wait_timeout)(uint32_t delay_ms);

    /**
     * @brief 可中断等待
     * @param condition 条件函数
     * @param timeout_ms 超时时间(毫秒)
     * @param user_data 用户数据
     * @return 等待结果
     */
    system_result_t (*wait_interruptible)(bool (*condition)(void *), void *user_data, uint32_t timeout_ms);

    /**
     * @brief 轮询等待
     * @param condition 条件函数
     * @param timeout_ms 超时时间(毫秒)
     * @param poll_interval_ms 轮询间隔
     * @param user_data 用户数据
     * @return 等待结果
     */
    system_result_t (*wait_polling)(bool (*condition)(void *), void *user_data, uint32_t timeout_ms, uint32_t poll_interval_ms);

    /**
     * @brief 取消等待
     * @return 取消结果
     */
    system_result_t (*cancel_wait)(void);

    /**
     * @brief 获取等待管理器版本
     * @return 版本号
     */
    uint32_t (*get_version)(void);
} wait_manager_interface_t;

/* 全局系统接口实例 */
extern const timer_manager_interface_t *timer_manager;
extern const watchdog_manager_interface_t *watchdog_manager;
extern const clock_manager_interface_t *clock_manager;
extern const atomic_interface_t *atomic_ops;
extern const event_manager_interface_t *event_manager;
extern const wait_manager_interface_t *wait_manager;

/* 系统统计信息接口 */
extern system_statistics_t *system_stats;

/* 常用配置宏 */
#define TIMER_CONFIG_DEFAULT(type, timeout) \
    { (type), TIMER_PRECISION_MS, CLOCK_SOURCE_INTERNAL, (timeout), (timeout), false, false, INTERRUPT_LEVEL_LOW, NULL }

#define WATCHDOG_CONFIG_DEFAULT(timeout) \
    { WATCHDOG_TYPE_IWDG, (timeout), 0, true, true, true, INTERRUPT_LEVEL_CRITICAL, NULL }

#define CLOCK_CONFIG_DEFAULT(source, freq) \
    { (source), (freq), 1, true, false, 100 }

#define EVENT_CONFIG_DEFAULT(type, max_items) \
    { (type), (max_items), sizeof(void*), false, false, INTERRUPT_LEVEL_LOW, NULL }

#define WAIT_CONFIG_DEFAULT(timeout) \
    { WAIT_TYPE_TIMEOUT, (timeout), 10, true, INTERRUPT_LEVEL_MEDIUM, NULL }

/* 系统错误处理宏 */
#define SYSTEM_CHECK(expr) \
    do { \
        system_result_t _result = (expr); \
        if (_result != SYSTEM_OK) { \
            return _result; \
        } \
    } while(0)

#define SYSTEM_CHECK_RET(expr, ret_val) \
    do { \
        system_result_t _result = (expr); \
        if (_result != SYSTEM_OK) { \
            return (ret_val); \
        } \
    } while(0)

/* 便捷操作宏 */
#define SYSTEM_DELAY_MS(ms) \
    clock_manager->delay_ms((ms))

#define SYSTEM_DELAY_US(us) \
    clock_manager->delay_us((us))

#define SYSTEM_GET_TIMESTAMP_MS() \
    clock_manager->get_timestamp_ms()

#define SYSTEM_GET_TIMESTAMP_US() \
    clock_manager->get_timestamp_us()

#define SYSTEM_ENTER_CRITICAL() \
    atomic_ops->enter_critical()

#define SYSTEM_EXIT_CRITICAL(state) \
    atomic_ops->exit_critical((state))

#define SYSTEM_DISABLE_INTERRUPTS() \
    atomic_ops->disable_interrupts()

#define SYSTEM_ENABLE_INTERRUPTS(state) \
    atomic_ops->enable_interrupts((state))

#define SYSTEM_PROCESS_ALL() \
    do { \
        timer_manager->process_timers(); \
        watchdog_manager->process_watchdogs(); \
        event_manager->process_events(); \
    } while(0)

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_INTERFACE_H */