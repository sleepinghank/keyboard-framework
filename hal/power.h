/**
 * @file power_interface.h
 * @brief Power管理硬件抽象层接口定义
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * 提供跨芯片平台的Power管理统一接口
 * 支持低功耗模式、电池管理、电源控制等应用
 */

#ifndef POWER_INTERFACE_H
#define POWER_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Power管理错误码定义 */
typedef enum {
    POWER_OK = 0,                  /* 操作成功 */
    POWER_ERROR_INVALID_MODE,       /* 无效模式 */
    POWER_ERROR_INVALID_VOLTAGE,    /* 无效电压 */
    POWER_ERROR_INVALID_CURRENT,    /* 无效电流 */
    POWER_ERROR_INVALID_TEMP,       /* 无效温度 */
    POWER_ERROR_INVALID_SOURCE,     /* 无效电源源 */
    POWER_ERROR_NOT_SUPPORTED,      /* 操作不支持 */
    POWER_ERROR_HARDWARE_FAULT,     /* 硬件故障 */
    POWER_ERROR_BUSY,               /* Power管理忙碌 */
    POWER_ERROR_TIMEOUT,            /* 操作超时 */
    POWER_ERROR_ALREADY_INITIALIZED,/* 已初始化 */
    POWER_ERROR_NOT_INITIALIZED,    /* 未初始化 */
    POWER_ERROR_WAKEUP_SOURCE,      /* 唤醒源错误 */
    POWER_ERROR_BROWNOUT,           /* 欠压错误 */
    POWER_ERROR_OVERVOLTAGE,        /* 过压错误 */
    POWER_ERROR_UNDERVOLTAGE,       /* 欠压错误 */
    POWER_ERROR_OVERCURRENT,        /* 过流错误 */
    POWER_ERROR_SHORT_CIRCUIT,      /* 短路错误 */
    POWER_ERROR_OVERTEMPERATURE,    /* 过温错误 */
    POWER_ERROR_UNDERTEMPERATURE,    /* 欠温错误 */
    POWER_ERROR_BATTERY_LOW,        /* 电池电量低 */
    POWER_ERROR_BATTERY_EMPTY,      /* 电池电量耗尽 */
    POWER_ERROR_BATTERY_FAULT       /* 电池故障 */
} power_result_t;

/* Power模式定义 */
typedef enum {
    POWER_MODE_RUN = 0,             /* 运行模式 */
    POWER_MODE_SLEEP,               /* 睡眠模式 */
    POWER_MODE_STOP,                /* 停止模式 */
    POWER_MODE_STANDBY,             /* 待机模式 */
    POWER_MODE_SHUTDOWN,            /* 关机模式 */
    POWER_MODE_DEEP_SLEEP,          /* 深度睡眠模式 */
    POWER_MODE_HIBERNATE,           /* 休眠模式 */
    POWER_MODE_ULTRA_LOW_POWER,     /* 超低功耗模式 */
    POWER_MODE_RETENTION,           /* 数据保持模式 */
    POWER_MODE_BACKUP               /* 备份模式 */
} power_mode_t;

/* Power唤醒源定义 */
typedef enum {
    POWER_WAKEUP_NONE = 0,          /* 无唤醒源 */
    POWER_WAKEUP_GPIO,              /* GPIO唤醒 */
    POWER_WAKEUP_RTC,               /* RTC唤醒 */
    POWER_WAKEUP_TIMER,             /* 定时器唤醒 */
    POWER_WAKEUP_UART,              /* 串口唤醒 */
    POWER_WAKEUP_I2C,               /* I2C唤醒 */
    POWER_WAKEUP_SPI,               /* SPI唤醒 */
    POWER_WAKEUP_USB,               /* USB唤醒 */
    POWER_WAKEUP_BLE,               /* 蓝牙唤醒 */
    POWER_WAKEUP_2_4G,              /* 2.4G唤醒 */
    POWER_WAKEUP_ADC,               /* ADC唤醒 */
    POWER_WAKEUP_COMPARATOR,        /* 比较器唤醒 */
    POWER_WAKEUP_TOUCHPAD,          /* 触控板唤醒 */
    POWER_WAKEUP_KEYBOARD,          /* 键盘唤醒 */
    POWER_WAKEUP_BUTTON,            /* 按键唤醒 */
    POWER_WAKEUP_BATTERY,           /* 电池唤醒 */
    POWER_WAKEUP_CHARGER,           /* 充电器唤醒 */
    POWER_WAKEUP_WATCHDOG,          /* 看门狗唤醒 */
    POWER_WAKEUP_DEBUG,             /* 调试唤醒 */
    POWER_WAKEUP_ALL               /* 所有唤醒源 */
} power_wakeup_source_t;

/* Power电压调节器状态定义 */
typedef enum {
    POWER_REGULATOR_OFF = 0,        /* 关闭 */
    POWER_REGULATOR_LOW_POWER,      /* 低功耗模式 */
    POWER_REGULATOR_NORMAL,         /* 正常模式 */
    POWER_REGULATOR_HIGH_PERFORMANCE /* 高性能模式 */
} power_regulator_state_t;

/* Power时钟源定义 */
typedef enum {
    POWER_CLOCK_INTERNAL = 0,       /* 内部时钟 */
    POWER_CLOCK_EXTERNAL,          /* 外部时钟 */
    POWER_CLOCK_PLL,               /* PLL时钟 */
    POWER_CLOCK_HSI,               /* 高速内部时钟 */
    POWER_CLOCK_HSE,               /* 高速外部时钟 */
    POWER_CLOCK_LSI,               /* 低速内部时钟 */
    POWER_CLOCK_LSE,               /* 低速外部时钟 */
    POWER_CLOCK_MSI,               /* 多速率内部时钟 */
    POWER_CLOCK_RC,                /* RC时钟 */
    POWER_CLOCK_CRYSTAL            /* 晶体时钟 */
} power_clock_source_t;

/* Power充电状态定义 */
typedef enum {
    POWER_CHARGE_IDLE = 0,          /* 空闲 */
    POWER_CHARGE_CHARGING,         /* 充电中 */
    POWER_CHARGE_COMPLETE,         /* 充电完成 */
    POWER_CHARGE_ERROR,            /* 充电错误 */
    POWER_CHARGE_PRE_CHARGE,       /* 预充电 */
    POWER_CHARGE_FAST_CHARGE,      /* 快速充电 */
    POWER_CHARGE_CONSTANT_VOLTAGE, /* 恒压充电 */
    POWER_CHARGE_CONSTANT_CURRENT, /* 恒流充电 */
    POWER_CHARGE_TRICKLE_CHARGE,   /* 涓流充电 */
    POWER_CHARGE_THERMAL_SHUTDOWN  /* 热关断 */
} power_charge_state_t;

/* Power电池类型定义 */
typedef enum {
    POWER_BATTERY_UNKNOWN = 0,      /* 未知电池 */
    POWER_BATTERY_LITHIUM_ION,     /* 锂离子电池 */
    POWER_BATTERY_LITHIUM_POLYMER, /* 锂聚合物电池 */
    POWER_BATTERY_NIMH,             /* 镍氢电池 */
    POWER_BATTERY_NICD,             /* 镍镉电池 */
    POWER_BATTERY_LEAD_ACID,        /* 铅酸电池 */
    POWER_BATTERY_LIFEP04,          /* 磷酸铁锂电池 */
    POWER_BATTERY_ALKALINE,         /* 碱性电池 */
    POWER_BATTERY_ZINC_CARBON,      /* 碳性电池 */
    POWER_BATTERY_RECHARGEABLE,     /* 可充电电池 */
    POWER_BATTERY_PRIMARY           /* 一次性电池 */
} power_battery_type_t;

/* Power系统状态定义 */
typedef enum {
    POWER_SYSTEM_STATE_INIT = 0,    /* 初始化状态 */
    POWER_SYSTEM_STATE_NORMAL,      /* 正常状态 */
    POWER_SYSTEM_STATE_LOW_POWER,   /* 低功耗状态 */
    POWER_SYSTEM_STATE_CRITICAL,    /* 临界状态 */
    POWER_SYSTEM_STATE_EMERGENCY,   /* 紧急状态 */
    POWER_SYSTEM_STATE_SHUTDOWN,    /* 关机状态 */
    POWER_SYSTEM_STATE_RECOVERY,    /* 恢复状态 */
    POWER_SYSTEM_STATE_TEST,         /* 测试状态 */
    POWER_SYSTEM_STATE_CALIBRATION   /* 校准状态 */
} power_system_state_t;

/* Power事件类型定义 */
typedef enum {
    POWER_EVENT_MODE_CHANGE = 0,    /* 模式改变事件 */
    POWER_EVENT_WAKEUP,             /* 唤醒事件 */
    POWER_EVENT_SLEEP,              /* 睡眠事件 */
    POWER_EVENT_BATTERY_LOW,        /* 电池低电事件 */
    POWER_EVENT_BATTERY_CRITICAL,   /* 电池临界事件 */
    POWER_EVENT_CHARGE_START,       /* 充电开始事件 */
    POWER_EVENT_CHARGE_COMPLETE,    /* 充电完成事件 */
    POWER_EVENT_CHARGE_ERROR,       /* 充电错误事件 */
    POWER_EVENT_OVERVOLTAGE,        /* 过压事件 */
    POWER_EVENT_UNDERVOLTAGE,       /* 欠压事件 */
    POWER_EVENT_OVERCURRENT,        /* 过流事件 */
    POWER_EVENT_OVERTEMPERATURE,    /* 过温事件 */
    POWER_EVENT_UNDERTEMPERATURE,   /* 欠温事件 */
    POWER_EVENT_BROWNOUT,           /* 欠压保护事件 */
    POWER_EVENT_RESET,              /* 复位事件 */
    POWER_EVENT_WATCHDOG,           /* 看门狗事件 */
    POWER_EVENT_FAULT,              /* 故障事件 */
    POWER_EVENT_WARNING,            /* 警告事件 */
    POWER_EVENT_INFO               /* 信息事件 */
} power_event_t;

/* Power低功耗配置结构体 */
typedef struct {
    power_mode_t            target_mode;        /* 目标低功耗模式 */
    uint32_t                sleep_duration_ms;  /* 睡眠持续时间(毫秒) */
    uint32_t                wakeup_timeout_ms;  /* 唤醒超时时间(毫秒) */
    power_wakeup_source_t   wakeup_source;      /* 唤醒源 */
    uint32_t                wakeup_pin_mask;    /* 唤醒引脚掩码 */
    bool                    enable_retention;   /* 使能数据保持 */
    bool                    enable_backup;      /* 使能备份域 */
    bool                    enable_wakeup_timer;/* 使能唤醒定时器 */
    bool                    enable_wakeup_gpio;  /* 使能GPIO唤醒 */
    bool                    enable_wakeup_rtc;   /* 使能RTC唤醒 */
    bool                    enable_wakeup_uart;  /* 使能串口唤醒 */
    bool                    enable_wakeup_ble;   /* 使能蓝牙唤醒 */
    bool                    enable_wakeup_usb;   /* 使能USB唤醒 */
    uint32_t                wakeup_timer_period_ms; /* 唤醒定时器周期 */
    uint32_t                rtc_wakeup_time;     /* RTC唤醒时间 */
    power_clock_source_t    wakeup_clock;        /* 唤醒时钟源 */
    power_regulator_state_t  regulator_state;     /* 电压调节器状态 */
} power_low_power_config_t;

/* Power电池监控配置结构体 */
typedef struct {
    power_battery_type_t    battery_type;        /* 电池类型 */
    float                   nominal_voltage_v;    /* 标称电压(V) */
    float                   max_voltage_v;        /* 最大电压(V) */
    float                   min_voltage_v;        /* 最小电压(V) */
    float                   cutoff_voltage_v;     /* 截止电压(V) */
    float                   capacity_mah;         /* 电池容量(mAh) */
    uint16_t                impedance_ohm;        /* 内阻(欧姆) */
    float                   temperature_coefficient; /* 温度系数 */
    uint16_t                charge_cycles;        /* 充电循环次数 */
    uint32_t                calibration_date;     /* 校准日期 */
    float                   calibration_offset;   /* 校准偏移 */
    float                   calibration_scale;    /* 校准比例 */
    uint16_t                low_voltage_threshold_mv; /* 低电压阈值(mV) */
    uint16_t                critical_voltage_threshold_mv; /* 临界电压阈值(mV) */
    uint8_t                 low_battery_percentage; /* 低电量百分比 */
    uint8_t                 critical_battery_percentage; /* 临界电量百分比 */
    uint16_t                sample_interval_ms;   /* 采样间隔(毫秒) */
    uint8_t                 filter_size;          /* 滤波器大小 */
    bool                    enable_temperature_compensation; /* 使能温度补偿 */
    bool                    enable_aging_compensation; /* 使能老化补偿 */
    bool                    enable_self_discharge_compensation; /* 使能自放电补偿 */
} power_battery_config_t;

/* Power充电管理配置结构体 */
typedef struct {
    float                   charge_voltage_v;     /* 充电电压(V) */
    float                   charge_current_a;     /* 充电电流(A) */
    float                   pre_charge_current_a; /* 预充电电流(A) */
    float                   termination_current_a;/* 终止电流(A) */
    float                   max_charge_voltage_v; /* 最大充电电压(V) */
    float                   max_charge_temperature_c; /* 最大充电温度(°C) */
    float                   min_charge_temperature_c; /* 最小充电温度(°C) */
    uint32_t                charge_timeout_s;    /* 充电超时时间(秒) */
    uint32_t                pre_charge_timeout_s; /* 预充电超时时间(秒) */
    uint32_t                charge_complete_timeout_s; /* 充电完成超时时间(秒) */
    uint8_t                 charge_profile;      /* 充电配置文件 */
    bool                    enable_termination;   /* 使能充电终止 */
    bool                    enable_temperature_monitoring; /* 使能温度监控 */
    bool                    enable_timer_protection; /* 使能定时保护 */
    bool                    enable_voltage_protection; /* 使能电压保护 */
    bool                    enable_current_protection; /* 使能电流保护 */
    bool                    enable_temperature_protection; /* 使能温度保护 */
    bool                    enable_reverse_polarity_protection; /* 使能极性反接保护 */
    bool                    enable_trickle_charge; /* 使能涓流充电 */
    bool                    enable_boost_mode;   /* 使能升压模式 */
} power_charge_config_t;

/* Power状态信息结构体 */
typedef struct {
    power_mode_t            current_mode;        /* 当前模式 */
    power_system_state_t    system_state;        /* 系统状态 */
    float                   battery_voltage_v;    /* 电池电压(V) */
    float                   battery_current_a;    /* 电池电流(A) */
    float                   battery_temperature_c; /* 电池温度(°C) */
    uint8_t                 battery_percentage;   /* 电池电量百分比 */
    power_charge_state_t    charge_state;        /* 充电状态 */
    float                   charge_voltage_v;     /* 充电电压(V) */
    float                   charge_current_a;     /* 充电电流(A) */
    float                   system_voltage_v;     /* 系统电压(V) */
    float                   system_current_a;     /* 系统电流(A) */
    float                   system_power_w;      /* 系统功率(W) */
    float                   temperature_c;       /* 系统温度(°C) */
    uint32_t                uptime_seconds;      /* 运行时间(秒) */
    uint32_t                sleep_time_seconds;  /* 睡眠时间(秒) */
    uint32_t                active_time_seconds; /* 活动时间(秒) */
    uint32_t                total_charge_time_seconds; /* 总充电时间(秒) */
    uint32_t                total_discharge_time_seconds; /* 总放电时间(秒) */
    uint16_t                charge_cycles;        /* 充电循环次数 */
    uint64_t                total_charge_mah;    /* 总充电量(mAh) */
    uint64_t                total_discharge_mah;  /* 总放电量(mAh) */
    float                   energy_consumed_wh;  /* 已消耗能量(Wh) */
    float                   efficiency_percentage; /* 效率百分比 */
    bool                    is_charging;         /* 是否在充电 */
    bool                    is_discharging;      /* 是否在放电 */
    bool                    is_battery_present;  /* 电池是否存在 */
    bool                    is_charger_present;  /* 充电器是否存在 */
    bool                    is_usb_connected;    /* USB是否连接 */
    bool                    is_power_good;       /* 电源是否正常 */
    bool                    is_temperature_ok;   /* 温度是否正常 */
    bool                    is_voltage_ok;       /* 电压是否正常 */
    bool                    is_current_ok;       /* 电流是否正常 */
    uint32_t                last_wakeup_source;   /* 上次唤醒源 */
    uint32_t                last_sleep_time;     /* 上次睡眠时间 */
    uint32_t                last_wakeup_time;    /* 上次唤醒时间 */
} power_status_t;

/* Power回调函数类型 */
typedef void (*power_callback_t)(power_event_t event, void *user_data);

/* Power接口函数指针结构体 */
typedef struct {
    /**
     * @brief 初始化Power管理模块
     * @return 初始化结果
     */
    power_result_t (*init)(void);

    /**
     * @brief 反初始化Power管理模块
     * @return 反初始化结果
     */
    power_result_t (*deinit)(void);

    /**
     * @brief 配置低功耗模式
     * @param config 低功耗配置参数
     * @return 配置结果
     */
    power_result_t (*config_low_power)(const power_low_power_config_t *config);

    /**
     * @brief 配置电池监控
     * @param config 电池监控配置参数
     * @return 配置结果
     */
    power_result_t (*config_battery_monitoring)(const power_battery_config_t *config);

    /**
     * @brief 配置充电管理
     * @param config 充电管理配置参数
     * @return 配置结果
     */
    power_result_t (*config_charge_management)(const power_charge_config_t *config);

    /**
     * @brief 进入睡眠模式
     * @param mode 睡眠模式
     * @param wakeup_source 唤醒源
     * @return 进入结果
     */
    power_result_t (*enter_sleep)(power_mode_t mode, power_wakeup_source_t wakeup_source);

    /**
     * @brief 进入深度睡眠模式
     * @param duration_ms 睡眠持续时间(毫秒)
     * @return 进入结果
     */
    power_result_t (*enter_deep_sleep)(uint32_t duration_ms);

    /**
     * @brief 进入待机模式
     * @param wakeup_time_ms 唤醒时间(毫秒)
     * @return 进入结果
     */
    power_result_t (*enter_standby)(uint32_t wakeup_time_ms);

    /**
     * @brief 进入关机模式
     * @return 进入结果
     */
    power_result_t (*enter_shutdown)(void);

    /**
     * @brief 进入休眠模式
     * @return 进入结果
     */
    power_result_t (*enter_hibernate)(void);

    /**
     * @brief 退出睡眠模式
     * @return 退出结果
     */
    power_result_t (*exit_sleep)(void);

    /**
     * @brief 退出深度睡眠模式
     * @return 退出结果
     */
    power_result_t (*exit_deep_sleep)(void);

    /**
     * @brief 退出待机模式
     * @return 退出结果
     */
    power_result_t (*exit_standby)(void);

    /**
     * @brief 退出休眠模式
     * @return 退出结果
     */
    power_result_t (*exit_hibernate)(void);

    /**
     * @brief 系统复位
     * @return 复位结果
     */
    power_result_t (*system_reset)(void);

    /**
     * @brief 软件复位
     * @return 复位结果
     */
    power_result_t (*software_reset)(void);

    /**
     * @brief 看门狗复位
     * @return 复位结果
     */
    power_result_t (*watchdog_reset)(void);

    /**
     * @brief 配置唤醒源
     * @param wakeup_source 唤醒源
     * @param enable 是否使能
     * @return 配置结果
     */
    power_result_t (*configure_wakeup_source)(power_wakeup_source_t wakeup_source, bool enable);

    /**
     * @brief 配置GPIO唤醒
     * @param gpio_mask GPIO掩码
     * @param trigger_mode 触发模式
     * @return 配置结果
     */
    power_result_t (*configure_gpio_wakeup)(uint32_t gpio_mask, uint8_t trigger_mode);

    /**
     * @brief 配置RTC唤醒
     * @param wakeup_time 唤醒时间
     * @return 配置结果
     */
    power_result_t (*configure_rtc_wakeup)(uint32_t wakeup_time);

    /**
     * @brief 配置定时器唤醒
     * @param period_ms 定时器周期(毫秒)
     * @return 配置结果
     */
    power_result_t (*configure_timer_wakeup)(uint32_t period_ms);

    /**
     * @brief 获取唤醒源
     * @param wakeup_source 输出唤醒源
     * @return 获取结果
     */
    power_result_t (*get_wakeup_source)(power_wakeup_source_t *wakeup_source);

    /**
     * @brief 清除唤醒源
     * @param wakeup_source 要清除的唤醒源
     * @return 清除结果
     */
    power_result_t (*clear_wakeup_source)(power_wakeup_source_t wakeup_source);

    /**
     * @brief 使能电源事件中断
     * @param callback 事件回调函数
     * @param user_data 用户数据
     * @return 使能结果
     */
    power_result_t (*enable_power_interrupt)(power_callback_t callback, void *user_data);

    /**
     * @brief 禁用电源事件中断
     * @return 禁用结果
     */
    power_result_t (*disable_power_interrupt)(void);

    /**
     * @brief 获取电池电压
     * @param voltage 输出电池电压(V)
     * @return 获取结果
     */
    power_result_t (*get_battery_voltage)(float *voltage);

    /**
     * @brief 获取电池电压(mV)
     * @param voltage_mv 输出电池电压(mV)
     * @return 获取结果
     */
    power_result_t (*get_battery_voltage_mv)(uint16_t *voltage_mv);

    /**
     * @brief 获取电池电流
     * @param current 输出电池电流(A)
     * @return 获取结果
     */
    power_result_t (*get_battery_current)(float *current);

    /**
     * @brief 获取电池电流(mA)
     * @param current_ma 输出电池电流(mA)
     * @return 获取结果
     */
    power_result_t (*get_battery_current_ma)(int16_t *current_ma);

    /**
     * @brief 获取电池温度
     * @param temperature 输出电池温度(°C)
     * @return 获取结果
     */
    power_result_t (*get_battery_temperature)(float *temperature);

    /**
     * @brief 获取电池电量百分比
     * @param percentage 输出电量百分比
     * @return 获取结果
     */
    power_result_t (*get_battery_percentage)(uint8_t *percentage);

    /**
     * @brief 获取电池剩余容量
     * @param capacity 输出剩余容量(mAh)
     * @return 获取结果
     */
    power_result_t (*get_remaining_capacity)(float *capacity);

    /**
     * @brief 获取充电状态
     * @param charge_state 输出充电状态
     * @return 获取结果
     */
    power_result_t (*get_charge_state)(power_charge_state_t *charge_state);

    /**
     * @brief 开始充电
     * @return 开始结果
     */
    power_result_t (*start_charging)(void);

    /**
     * @brief 停止充电
     * @return 停止结果
     */
    power_result_t (*stop_charging)(void);

    /**
     * @brief 暂停充电
     * @return 暂停结果
     */
    power_result_t (*pause_charging)(void);

    /**
     * @brief 恢复充电
     * @return 恢复结果
     */
    power_result_t (*resume_charging)(void);

    /**
     * @brief 设置充电电流
     * @param current 充电电流(A)
     * @return 设置结果
     */
    power_result_t (*set_charge_current)(float current);

    /**
     * @brief 设置充电电压
     * @param voltage 充电电压(V)
     * @return 设置结果
     */
    power_result_t (*set_charge_voltage)(float voltage);

    /**
     * @brief 获取系统电压
     * @param voltage 输出系统电压(V)
     * @return 获取结果
     */
    power_result_t (*get_system_voltage)(float *voltage);

    /**
     * @brief 获取系统电流
     * @param current 输出系统电流(A)
     * @return 获取结果
     */
    power_result_t (*get_system_current)(float *current);

    /**
     * @brief 获取系统功率
     * @param power 输出系统功率(W)
     * @return 获取结果
     */
    power_result_t (*get_system_power)(float *power);

    /**
     * @brief 获取系统温度
     * @param temperature 输出系统温度(°C)
     * @return 获取结果
     */
    power_result_t (*get_system_temperature)(float *temperature);

    /**
     * @brief 设置电压调节器状态
     * @param regulator 电压调节器
     * @param state 调节器状态
     * @return 设置结果
     */
    power_result_t (*set_regulator_state)(uint8_t regulator, power_regulator_state_t state);

    /**
     * @brief 获取电压调节器状态
     * @param regulator 电压调节器
     * @param state 输出调节器状态
     * @return 获取结果
     */
    power_result_t (*get_regulator_state)(uint8_t regulator, power_regulator_state_t *state);

    /**
     * @brief 使能电源域
     * @param domain 电源域
     * @return 使能结果
     */
    power_result_t (*enable_power_domain)(uint8_t domain);

    /**
     * @brief 禁用电源域
     * @param domain 电源域
     * @return 禁用结果
     */
    power_result_t (*disable_power_domain)(uint8_t domain);

    /**
     * @brief 配置时钟源
     * @param clock_source 时钟源
     * @return 配置结果
     */
    power_result_t (*configure_clock_source)(power_clock_source_t clock_source);

    /**
     * @brief 获取时钟源
     * @param clock_source 输出时钟源
     * @return 获取结果
     */
    power_result_t (*get_clock_source)(power_clock_source_t *clock_source);

    /**
     * @brief 设置系统时钟频率
     * @param frequency 时钟频率(Hz)
     * @return 设置结果
     */
    power_result_t (*set_system_clock_frequency)(uint32_t frequency);

    /**
     * @brief 获取系统时钟频率
     * @param frequency 输出时钟频率(Hz)
     * @return 获取结果
     */
    power_result_t (*get_system_clock_frequency)(uint32_t *frequency);

    /**
     * @brief 获取运行时间
     * @param uptime 输出运行时间(秒)
     * @return 获取结果
     */
    power_result_t (*get_uptime)(uint32_t *uptime);

    /**
     * @brief 获取运行时间(毫秒)
     * @param uptime_ms 输出运行时间(毫秒)
     * @return 获取结果
     */
    power_result_t (*get_uptime_ms)(uint64_t *uptime_ms);

    /**
     * @brief 获取运行时间(微秒)
     * @param uptime_us 输出运行时间(微秒)
     * @return 获取结果
     */
    power_result_t (*get_uptime_us)(uint64_t *uptime_us);

    /**
     * @brief 设置运行时间
     * @param uptime 运行时间(秒)
     * @return 设置结果
     */
    power_result_t (*set_uptime)(uint32_t uptime);

    /**
     * @brief 重置运行时间
     * @return 重置结果
     */
    power_result_t (*reset_uptime)(void);

    /**
     * @brief 获取电源管理状态
     * @param status 输出状态信息
     * @return 获取结果
     */
    power_result_t (*get_power_status)(power_status_t *status);

    /**
     * @brief 获取电池状态
     * @param is_present 输出电池是否存在
     * @param is_charging 输出是否在充电
     * @param percentage 输出电量百分比
     * @return 获取结果
     */
    power_result_t (*get_battery_status)(bool *is_present, bool *is_charging, uint8_t *percentage);

    /**
     * @brief 获取充电器状态
     * @param is_present 输出充电器是否存在
     * @param charge_state 输出充电状态
     * @return 获取结果
     */
    power_result_t (*get_charger_status)(bool *is_present, power_charge_state_t *charge_state);

    /**
     * @brief 检查电源是否正常
     * @param power_good 输出电源是否正常
     * @return 检查结果
     */
    power_result_t (*is_power_good)(bool *power_good);

    /**
     * @brief 检查电池是否需要充电
     * @param need_charge 输出是否需要充电
     * @return 检查结果
     */
    power_result_t (*need_charge)(bool *need_charge);

    /**
     * @brief 检查电池是否电量低
     * @param low_battery 输出是否电量低
     * @return 检查结果
     */
    power_result_t (*is_low_battery)(bool *low_battery);

    /**
     * @brief 检查电池是否电量临界
     * @param critical_battery 输出是否电量临界
     * @return 检查结果
     */
    power_result_t (*is_critical_battery)(bool *critical_battery);

    /**
     * @brief 估算电池剩余时间
     * @param remaining_hours 输出剩余小时数
     * @return 估算结果
     */
    power_result_t (*estimate_remaining_time)(float *remaining_hours);

    /**
     * @brief 估算充电完成时间
     * @param hours_to_full 输出充电到满电的小时数
     * @return 估算结果
     */
    power_result_t (*estimate_charge_time(float battery_percentage, float *hours_to_full);

    /**
     * @brief 校准电池电量
     * @return 校准结果
     */
    power_result_t (*calibrate_battery)(void);

    /**
     * @brief 重置电池统计
     * @return 重置结果
     */
    power_result_t (*reset_battery_statistics)(void);

    /**
     * @brief 获取电池健康度
     * @param health_percentage 输出电池健康度百分比
     * @return 获取结果
     */
    power_result_t (*get_battery_health)(uint8_t *health_percentage);

    /**
     * @brief 设置低电量警告阈值
     * @param percentage 电量百分比阈值
     * @return 设置结果
     */
    power_result_t (*set_low_battery_threshold)(uint8_t percentage);

    /**
     * @brief 设置临界电量警告阈值
     * @param percentage 电量百分比阈值
     * @return 设置结果
     */
    power_result_t (*set_critical_battery_threshold)(uint8_t percentage);

    /**
     * @brief 获取低电量警告阈值
     * @param percentage 输出电量百分比阈值
     * @return 获取结果
     */
    power_result_t (*get_low_battery_threshold)(uint8_t *percentage);

    /**
     * @brief 获取临界电量警告阈值
     * @param percentage 输出电量百分比阈值
     * @return 获取结果
     */
    power_result_t (*get_critical_battery_threshold)(uint8_t *percentage);

    /**
     * @brief 配置温度保护
     * @param max_temperature_c 最大温度(°C)
     * @param min_temperature_c 最小温度(°C)
     * @return 配置结果
     */
    power_result_t (*configure_temperature_protection)(float max_temperature_c, float min_temperature_c);

    /**
     * @brief 配置电压保护
     * @param max_voltage_v 最大电压(V)
     * @param min_voltage_v 最小电压(V)
     * @return 配置结果
     */
    power_result_t (*configure_voltage_protection)(float max_voltage_v, float min_voltage_v);

    /**
     * @brief 配置电流保护
     * @param max_current_a 最大电流(A)
     * @return 配置结果
     */
    power_result_t (*configure_current_protection)(float max_current_a);

    /**
     * @brief 使能电源保护
     * @return 使能结果
     */
    power_result_t (*enable_power_protection)(void);

    /**
     * @brief 禁用电源保护
     * @return 禁用结果
     */
    power_result_t (*disable_power_protection)(void);

    /**
     * @brief 设置功率限制
     * @param max_power_w 最大功率(W)
     * @return 设置结果
     */
    power_result_t (*set_power_limit)(float max_power_w);

    /**
     * @brief 获取当前功耗
     * @param current_power_w 输出当前功耗(W)
     * @return 获取结果
     */
    power_result_t (*get_current_power_consumption)(float *current_power_w);

    /**
     * @brief 计算系统效率
     * @param efficiency_percentage 输出效率百分比
     * @return 计算结果
     */
    power_result_t (*calculate_system_efficiency)(float *efficiency_percentage);

    /**
     * @brief 优化电源管理
     * @return 优化结果
     */
    power_result_t (*optimize_power_management)(void);

    /**
     * @brief 启用动态电源管理
     * @return 使能结果
     */
    power_result_t (*enable_dynamic_power_management)(void);

    /**
     * @brief 禁用动态电源管理
     * @return 禁用结果
     */
    power_result_t (*disable_dynamic_power_management)(void);

    /**
     * @brief 获取电源管理统计信息
     * @param total_energy_wh 输出总能耗(Wh)
     * @param average_power_w 输出平均功耗(W)
     * @param peak_power_w 输出峰值功耗(W)
     * @return 获取结果
     */
    power_result_t (*get_power_statistics)(float *total_energy_wh, float *average_power_w, float *peak_power_w);

    /**
     * @brief 导出电源管理数据
     * @param data_buffer 数据缓冲区
     * @param buffer_size 缓冲区大小
     * @param data_size 输出数据大小
     * @return 导出结果
     */
    power_result_t (*export_power_data)(uint8_t *data_buffer, uint16_t buffer_size, uint16_t *data_size);

    /**
     * @brief 导入电源管理数据
     * @param data_buffer 数据缓冲区
     * @param data_size 数据大小
     * @return 导入结果
     */
    power_result_t (*import_power_data)(const uint8_t *data_buffer, uint16_t data_size);

    /**
     * @brief 执行电源管理自检
     * @return 自检结果
     */
    power_result_t (*self_test)(void);

    /**
     * @brief 获取电源管理驱动版本信息
     * @return 版本号
     */
    uint32_t (*get_version)(void);
} power_interface_t;

/* 全局Power接口实例 */
extern const power_interface_t *power_hal;

/* 便捷宏定义 */
#define POWER_GET_BATTERY_VOLTAGE() \
    ({ float _voltage; power_hal->get_battery_voltage(&_voltage); _voltage; })

#define POWER_GET_BATTERY_VOLTAGE_MV() \
    ({ uint16_t _voltage; power_hal->get_battery_voltage_mv(&_voltage); _voltage; })

#define POWER_GET_BATTERY_CURRENT() \
    ({ float _current; power_hal->get_battery_current(&_current); _current; })

#define POWER_GET_BATTERY_CURRENT_MA() \
    ({ int16_t _current; power_hal->get_battery_current_ma(&_current); _current; })

#define POWER_GET_BATTERY_PERCENTAGE() \
    ({ uint8_t _percentage; power_hal->get_battery_percentage(&_percentage); _percentage; })

#define POWER_GET_SYSTEM_VOLTAGE() \
    ({ float _voltage; power_hal->get_system_voltage(&_voltage); _voltage; })

#define POWER_GET_SYSTEM_CURRENT() \
    ({ float _current; power_hal->get_system_current(&_current); _current; })

#define POWER_GET_SYSTEM_POWER() \
    ({ float _power; power_hal->get_system_power(&_power); _power; })

#define POWER_GET_UPTIME() \
    ({ uint32_t _uptime; power_hal->get_uptime(&_uptime); _uptime; })

#define POWER_GET_UPTIME_MS() \
    ({ uint64_t _uptime; power_hal->get_uptime_ms(&_uptime); _uptime; })

#define POWER_GET_UPTIME_US() \
    ({ uint64_t _uptime; power_hal->get_uptime_us(&_uptime); _uptime; })

#define POWER_IS_POWER_GOOD() \
    ({ bool _power_good; power_hal->is_power_good(&_power_good); _power_good; })

#define POWER_NEED_CHARGE() \
    ({ bool _need_charge; power_hal->need_charge(&_need_charge); _need_charge; })

#define POWER_IS_LOW_BATTERY() \
    ({ bool _low_battery; power_hal->is_low_battery(&_low_battery); _low_battery; })

#define POWER_IS_CRITICAL_BATTERY() \
    ({ bool _critical_battery; power_hal->is_critical_battery(&_critical_battery); _critical_battery; })

/* 常用配置宏 */
#define POWER_LOW_POWER_CONFIG_DEFAULT() \
    { POWER_MODE_SLEEP, 0, 0, POWER_WAKEUP_GPIO, 0, true, false, true, true, false, false, true, false, 0, 0, POWER_CLOCK_INTERNAL, POWER_REGULATOR_LOW_POWER }

#define POWER_BATTERY_CONFIG_LITHIUM_ION() \
    { POWER_BATTERY_LITHIUM_ION, 3.7f, 4.2f, 2.7f, 2.5f, 3000.0f, 100, 0.01f, 0, 0, 0.0f, 1.0f, 3200, 1500, 10, 1000, 8, true, true, true }

#define POWER_CHARGE_CONFIG_DEFAULT() \
    { 4.2f, 1.0f, 0.1f, 0.1f, 4.2f, 45.0f, 0.0f, 21600, 1800, 7200, 0, true, true, true, true, true, true, false, false, false }

/* Power错误处理宏 */
#define POWER_CHECK(expr) \
    do { \
        power_result_t _result = (expr); \
        if (_result != POWER_OK) { \
            return _result; \
        } \
    } while(0)

#define POWER_CHECK_RET(expr, ret_val) \
    do { \
        power_result_t _result = (expr); \
        if (_result != POWER_OK) { \
            return (ret_val); \
        } \
    } while(0)

/* 电池电量判断宏 */
#define POWER_BATTERY_IS_FULL(percentage) ((percentage) >= 95)
#define POWER_BATTERY_IS_HIGH(percentage) ((percentage) >= 80 && (percentage) < 95)
#define POWER_BATTERY_IS_MEDIUM(percentage) ((percentage) >= 30 && (percentage) < 80)
#define POWER_BATTERY_IS_LOW(percentage) ((percentage) >= 15 && (percentage) < 30)
#define POWER_BATTERY_IS_CRITICAL(percentage) ((percentage) < 15)

#define POWER_BATTERY_STATUS_STR(percentage) \
    ((percentage) >= 95 ? "FULL" : \
     (percentage) >= 80 ? "HIGH" : \
     (percentage) >= 30 ? "MEDIUM" : \
     (percentage) >= 15 ? "LOW" : "CRITICAL")

/* 温度保护宏 */
#define POWER_TEMPERATURE_IS_OK(temp_c) ((temp_c) >= 0.0f && (temp_c) <= 45.0f)
#define POWER_TEMPERATURE_IS_HIGH(temp_c) ((temp_c) > 45.0f && (temp_c) <= 60.0f)
#define POWER_TEMPERATURE_IS_CRITICAL(temp_c) ((temp_c) > 60.0f)
#define POWER_TEMPERATURE_IS_LOW(temp_c) ((temp_c) < 0.0f)

#define POWER_TEMPERATURE_STATUS_STR(temp_c) \
    ((temp_c) > 60.0f ? "CRITICAL" : \
     (temp_c) > 45.0f ? "HIGH" : \
     (temp_c) < 0.0f ? "LOW" : "OK")

/* 充电状态判断宏 */
#define POWER_IS_CHARGING(state) ((state) == POWER_CHARGE_CHARGING || (state) == POWER_CHARGE_FAST_CHARGE || (state) == POWER_CHARGE_CONSTANT_VOLTAGE || (state) == POWER_CHARGE_CONSTANT_CURRENT)
#define POWER_IS_CHARGE_COMPLETE(state) ((state) == POWER_CHARGE_COMPLETE)
#define POWER_IS_CHARGE_ERROR(state) ((state) == POWER_CHARGE_ERROR)
#define POWER_IS_CHARGE_IDLE(state) ((state) == POWER_CHARGE_IDLE)

#define POWER_CHARGE_STATUS_STR(state) \
    ((state) == POWER_CHARGE_CHARGING ? "CHARGING" : \
     (state) == POWER_CHARGE_COMPLETE ? "COMPLETE" : \
     (state) == POWER_CHARGE_ERROR ? "ERROR" : \
     (state) == POWER_CHARGE_PRE_CHARGE ? "PRE-CHARGE" : \
     (state) == POWER_CHARGE_FAST_CHARGE ? "FAST-CHARGE" : \
     (state) == POWER_CHARGE_CONSTANT_VOLTAGE ? "CONSTANT-V" : \
     (state) == POWER_CHARGE_CONSTANT_CURRENT ? "CONSTANT-C" : "IDLE")

/* 电源管理策略宏 */
#define POWER_AUTO_SLEEP_TIMEOUT_MS 30000    /* 30秒自动睡眠 */
#define POWER_DEEP_SLEEP_TIMEOUT_MS 300000   /* 5分钟深度睡眠 */
#define POWER_STANDBY_TIMEOUT_MS 1800000    /* 30分钟待机 */
#define POWER_SHUTDOWN_TIMEOUT_MS 7200000    /* 2小时关机 */

#define POWER_LOW_BATTERY_SLEEP_THRESHOLD 15 /* 低电量15%睡眠 */
#define POWER_CRITICAL_BATTERY_SHUTDOWN_THRESHOLD 5 /* 临界5%关机 */

#ifdef __cplusplus
}
#endif

#endif /* POWER_INTERFACE_H */