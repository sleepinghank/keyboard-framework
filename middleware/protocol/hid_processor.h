/**
 * @file hid_processor.h
 * @brief HID协议处理器接口定义
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * 提供HID协议处理、报告构建、数据转换等功能
 */

#ifndef HID_PROCESSOR_H
#define HID_PROCESSOR_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HID协议处理器错误码定义 */
typedef enum {
    HID_PROCESSOR_OK = 0,                /* 处理成功 */
    HID_PROCESSOR_ERROR_INVALID_PARAM,   /* 无效参数 */
    HID_PROCESSOR_ERROR_NOT_SUPPORTED,   /* 操作不支持 */
    HID_PROCESSOR_ERROR_INVALID_REPORT, /* 无效报告 */
    HID_PROCESSOR_ERROR_INVALID_USAGE,   /* 无效用法 */
    HID_PROCESSOR_ERROR_INVALID_VALUE,   /* 无效值 */
    HID_PROCESSOR_ERROR_BUFFER_TOO_SMALL,/* 缓冲区太小 */
    HID_PROCESSOR_ERROR_REPORT_OVERFLOW, /* 报告溢出 */
    HID_PROCESSOR_ERROR_PROTOCOL_ERROR, /* 协议错误 */
    HID_PROCESSOR_ERROR_CHECKSUM_FAILED, /* 校验失败 */
    HID_PROCESSOR_ERROR_TIMEOUT,        /* 处理超时 */
    HID_PROCESSOR_ERROR_BUSY,            /* 处理器忙碌 */
    HID_PROCESSOR_ERROR_NOT_INITIALIZED, /* 未初始化 */
    HID_PROCESSOR_ERROR_ALREADY_INITIALIZED, /* 已初始化 */
    HID_PROCESSOR_ERROR_FEATURE_DISABLED, /* 功能禁用 */
    HID_PROCESSOR_ERROR_LICENSE_LIMIT   /* 许可限制 */
} hid_processor_result_t;

/* HID报告类型定义 */
typedef enum {
    HID_REPORT_TYPE_INPUT = 0,           /* 输入报告 */
    HID_REPORT_TYPE_OUTPUT,              /* 输出报告 */
    HID_REPORT_TYPE_FEATURE              /* 特性报告 */
} hid_report_type_t;

/* HID报告ID定义 */
typedef enum {
    HID_REPORT_ID_KEYBOARD = 1,          /* 键盘报告ID */
    HID_REPORT_ID_MOUSE = 2,             /* 鼠标报告ID */
    HID_REPORT_ID_CONSUMER = 3,           /* 消费者控制报告ID */
    HID_REPORT_ID_SYSTEM = 4,            /* 系统控制报告ID */
    HID_REPORT_ID_GAMEPAD = 5,            /* 游戏手柄报告ID */
    HID_REPORT_ID_JOYSTICK = 6,           /* 摇杆报告ID */
    HID_REPORT_ID_DIGITIZER = 7,          /* 数字化仪报告ID */
    HID_REPORT_ID_VENDOR = 0xFF           /* 厂商特定报告ID */
} hid_report_id_t;

/* HID用法页定义 */
typedef enum {
    HID_USAGE_PAGE_GENERIC_DESKTOP = 0x01, /* 通用桌面控制 */
    HID_USAGE_PAGE_SIMULATION = 0x02,     /* 模拟控制 */
    HID_USAGE_PAGE_VR = 0x03,             /* VR控制 */
    HID_USAGE_PAGE_SPORT = 0x04,           /* 体育控制 */
    HID_USAGE_PAGE_GAME = 0x05,            /* 游戏控制 */
    HID_USAGE_PAGE_GENERIC_DEVICE = 0x06,  /* 通用设备控制 */
    HID_USAGE_PAGE_KEYBOARD = 0x07,        /* 键盘/按键 */
    HID_USAGE_PAGE_LED = 0x08,             /* LED */
    HID_USAGE_PAGE_BUTTON = 0x09,           /* 按钮 */
    HID_USAGE_PAGE_ORDINAL = 0x0A,         /* 序数 */
    HID_USAGE_PAGE_TELEPHONY = 0x0B,       /* 电话 */
    HID_USAGE_PAGE_CONSUMER = 0x0C,        /* 消费者控制 */
    HID_USAGE_PAGE_DIGITIZER = 0x0D,        /* 数字化仪 */
    HID_USAGE_PAGE_UNICODE = 0x10,          /* Unicode */
    HID_USAGE_PAGE_ALPHANUMERIC = 0x14,    /* 字母数字显示 */
    HID_USAGE_PAGE_MEDICAL = 0x40,          /* 医疗仪器 */
    HID_USAGE_PAGE_BARCODE = 0x8C,          /* 条码扫描仪 */
    HID_USAGE_PAGE_SCALE = 0x8D,            /* 规模 */
    HID_USAGE_PAGE_MAGNETIC_STRIPE = 0x8E,  /* 磁条阅读器 */
    HID_USAGE_PAGE_CAMERA_CONTROL = 0x90,  /* 相机控制 */
    HID_USAGE_PAGE_ARCADE = 0x91,           /* 街机 */
    HID_USAGE_PAGE_VENDOR_DEFINED = 0xFF00  /* 厂商定义 */
} hid_usage_page_t;

/* HID键盘用法定义 */
typedef enum {
    HID_USAGE_KEYBOARD_NONE = 0x00,         /* 无事件 */
    HID_USAGE_KEYBOARD_A = 0x04,           /* A键 */
    HID_USAGE_KEYBOARD_B = 0x05,           /* B键 */
    HID_USAGE_KEYBOARD_C = 0x06,           /* C键 */
    HID_USAGE_KEYBOARD_D = 0x07,           /* D键 */
    HID_USAGE_KEYBOARD_E = 0x08,           /* E键 */
    HID_USAGE_KEYBOARD_F = 0x09,           /* F键 */
    HID_USAGE_KEYBOARD_G = 0x0A,           /* G键 */
    HID_USAGE_KEYBOARD_H = 0x0B,           /* H键 */
    HID_USAGE_KEYBOARD_I = 0x0C,           /* I键 */
    HID_USAGE_KEYBOARD_J = 0x0D,           /* J键 */
    HID_USAGE_KEYBOARD_K = 0x0E,           /* K键 */
    HID_USAGE_KEYBOARD_L = 0x0F,           /* L键 */
    HID_USAGE_KEYBOARD_M = 0x10,           /* M键 */
    HID_USAGE_KEYBOARD_N = 0x11,           /* N键 */
    HID_USAGE_KEYBOARD_O = 0x12,           /* O键 */
    HID_USAGE_KEYBOARD_P = 0x13,           /* P键 */
    HID_USAGE_KEYBOARD_Q = 0x14,           /* Q键 */
    HID_USAGE_KEYBOARD_R = 0x15,           /* R键 */
    HID_USAGE_KEYBOARD_S = 0x16,           /* S键 */
    HID_USAGE_KEYBOARD_T = 0x17,           /* T键 */
    HID_USAGE_KEYBOARD_U = 0x18,           /* U键 */
    HID_USAGE_KEYBOARD_V = 0x19,           /* V键 */
    HID_USAGE_KEYBOARD_W = 0x1A,           /* W键 */
    HID_USAGE_KEYBOARD_X = 0x1B,           /* X键 */
    HID_USAGE_KEYBOARD_Y = 0x1C,           /* Y键 */
    HID_USAGE_KEYBOARD_Z = 0x1D,           /* Z键 */
    HID_USAGE_KEYBOARD_1 = 0x1E,          /* 1键 */
    HID_USAGE_KEYBOARD_2 = 0x1F,          /* 2键 */
    HID_USAGE_KEYBOARD_3 = 0x20,          /* 3键 */
    HID_USAGE_KEYBOARD_4 = 0x21,          /* 4键 */
    HID_USAGE_KEYBOARD_5 = 0x22,          /* 5键 */
    HID_USAGE_KEYBOARD_6 = 0x23,          /* 6键 */
    HID_USAGE_KEYBOARD_7 = 0x24,          /* 7键 */
    HID_USAGE_KEYBOARD_8 = 0x25,          /* 8键 */
    HID_USAGE_KEYBOARD_9 = 0x26,          /* 9键 */
    HID_USAGE_KEYBOARD_0 = 0x27,          /* 0键 */
    HID_USAGE_KEYBOARD_ENTER = 0x28,       /* 回车键 */
    HID_USAGE_KEYBOARD_ESCAPE = 0x29,      /* ESC键 */
    HID_USAGE_KEYBOARD_DELETE = 0x2A,      /* DELETE键 */
    HID_USAGE_KEYBOARD_TAB = 0x2B,         /* TAB键 */
    HID_USAGE_KEYBOARD_SPACEBAR = 0x2C,    /* 空格键 */
    HID_USAGE_KEYBOARD_CAPS_LOCK = 0x39,   /* CAPS LOCK键 */
    HID_USAGE_KEYBOARD_F1 = 0x3A,         /* F1键 */
    HID_USAGE_KEYBOARD_F2 = 0x3B,         /* F2键 */
    HID_USAGE_KEYBOARD_F3 = 0x3C,         /* F3键 */
    HID_USAGE_KEYBOARD_F4 = 0x3D,         /* F4键 */
    HID_USAGE_KEYBOARD_F5 = 0x3E,         /* F5键 */
    HID_USAGE_KEYBOARD_F6 = 0x3F,         /* F6键 */
    HID_USAGE_KEYBOARD_F7 = 0x40,         /* F7键 */
    HID_USAGE_KEYBOARD_F8 = 0x41,         /* F8键 */
    HID_USAGE_KEYBOARD_F9 = 0x42,         /* F9键 */
    HID_USAGE_KEYBOARD_F10 = 0x43,        /* F10键 */
    HID_USAGE_KEYBOARD_F11 = 0x44,        /* F11键 */
    HID_USAGE_KEYBOARD_F12 = 0x45,        /* F12键 */
    HID_USAGE_KEYBOARD_PRINT_SCREEN = 0x46, /* PRINT SCREEN键 */
    HID_USAGE_KEYBOARD_SCROLL_LOCK = 0x47,  /* SCROLL LOCK键 */
    HID_USAGE_KEYBOARD_PAUSE = 0x48,        /* PAUSE键 */
    HID_USAGE_KEYBOARD_INSERT = 0x49,       /* INSERT键 */
    HID_USAGE_KEYBOARD_HOME = 0x4A,        /* HOME键 */
    HID_USAGE_KEYBOARD_PAGE_UP = 0x4B,     /* PAGE UP键 */
    HID_USAGE_KEYBOARD_DELETE_FORWARD = 0x4C, /* DELETE键 */
    HID_USAGE_KEYBOARD_END = 0x4D,          /* END键 */
    HID_USAGE_KEYBOARD_PAGE_DOWN = 0x4E,    /* PAGE DOWN键 */
    HID_USAGE_KEYBOARD_RIGHT_ARROW = 0x4F,  /* 右箭头键 */
    HID_USAGE_KEYBOARD_LEFT_ARROW = 0x50,   /* 左箭头键 */
    HID_USAGE_KEYBOARD_DOWN_ARROW = 0x51,   /* 下箭头键 */
    HID_USAGE_KEYBOARD_UP_ARROW = 0x52,     /* 上箭头键 */
    HID_USAGE_KEYPAD_NUM_LOCK = 0x53,       /* NUM LOCK键 */
    HID_USAGE_KEYPAD_DIVIDE = 0x54,         /* 小键盘除法键 */
    HID_USAGE_KEYPAD_MULTIPLY = 0x55,       /* 小键盘乘法键 */
    HID_USAGE_KEYPAD_SUBTRACT = 0x56,       /* 小键盘减法键 */
    HID_USAGE_KEYPAD_ADD = 0x57,            /* 小键盘加法键 */
    HID_USAGE_KEYPAD_ENTER = 0x58,         /* 小键盘回车键 */
    HID_USAGE_KEYPAD_1 = 0x59,              /* 小键盘1键 */
    HID_USAGE_KEYPAD_2 = 0x5A,              /* 小键盘2键 */
    HID_USAGE_KEYPAD_3 = 0x5B,              /* 小键盘3键 */
    HID_USAGE_KEYPAD_4 = 0x5C,              /* 小键盘4键 */
    HID_USAGE_KEYPAD_5 = 0x5D,              /* 小键盘5键 */
    HID_USAGE_KEYPAD_6 = 0x5E,              /* 小键盘6键 */
    HID_USAGE_KEYPAD_7 = 0x5F,              /* 小键盘7键 */
    HID_USAGE_KEYPAD_8 = 0x60,              /* 小键盘8键 */
    HID_USAGE_KEYPAD_9 = 0x61,              /* 小键盘9键 */
    HID_USAGE_KEYPAD_0 = 0x62,              /* 小键盘0键 */
    HID_USAGE_KEYPAD_DECIMAL = 0x63,        /* 小键盘小数点键 */
    HID_USAGE_KEYBOARD_LEFT_CONTROL = 0xE0,  /* 左CTRL键 */
    HID_USAGE_KEYBOARD_LEFT_SHIFT = 0xE1,   /* 左SHIFT键 */
    HID_USAGE_KEYBOARD_LEFT_ALT = 0xE2,      /* 左ALT键 */
    HID_USAGE_KEYBOARD_LEFT_GUI = 0xE3,      /* 左GUI键 */
    HID_USAGE_KEYBOARD_RIGHT_CONTROL = 0xE4, /* 右CTRL键 */
    HID_USAGE_KEYBOARD_RIGHT_SHIFT = 0xE5,  /* 右SHIFT键 */
    HID_USAGE_KEYBOARD_RIGHT_ALT = 0xE6,   /* 右ALT键 */
    HID_USAGE_KEYBOARD_RIGHT_GUI = 0xE7     /* 右GUI键 */
} hid_keyboard_usage_t;

/* HID修饰键标志定义 */
typedef enum {
    HID_KEYBOARD_MODIFIER_LEFT_CONTROL = 0x01,   /* 左CTRL */
    HID_KEYBOARD_MODIFIER_LEFT_SHIFT = 0x02,     /* 左SHIFT */
    HID_KEYBOARD_MODIFIER_LEFT_ALT = 0x04,        /* 左ALT */
    HID_KEYBOARD_MODIFIER_LEFT_GUI = 0x08,        /* 左GUI */
    HID_KEYBOARD_MODIFIER_RIGHT_CONTROL = 0x10,  /* 右CTRL */
    HID_KEYBOARD_MODIFIER_RIGHT_SHIFT = 0x20,    /* 右SHIFT */
    HID_KEYBOARD_MODIFIER_RIGHT_ALT = 0x40,       /* 右ALT */
    HID_KEYBOARD_MODIFIER_RIGHT_GUI = 0x80        /* 右GUI */
} hid_keyboard_modifier_t;

/* HID LED标志定义 */
typedef enum {
    HID_LED_NUM_LOCK = 0x01,           /* NUM LOCK指示灯 */
    HID_LED_CAPS_LOCK = 0x02,         /* CAPS LOCK指示灯 */
    HID_LED_SCROLL_LOCK = 0x04,        /* SCROLL LOCK指示灯 */
    HID_LED_COMPOSE = 0x08,            /* COMPOSE指示灯 */
    HID_LED_KANA = 0x10,               /* KANA指示灯 */
    HID_LED_POWER = 0x20,              /* 电源指示灯 */
    HID_LED_SHIFT = 0x40,              /* SHIFT指示灯 */
    HID_LED_DO_NOT_DISTURB = 0x80      /* 请勿打扰指示灯 */
} hid_led_t;

/* HID键盘报告结构体 */
typedef struct {
    uint8_t                 modifier;       /* 修饰键状态 */
    uint8_t                 reserved;       /* 保留字节 */
    uint8_t                 keys[6];        /* 按键数组 */
} hid_keyboard_report_t;

/* HID鼠标报告结构体 */
typedef struct {
    uint8_t                 buttons;        /* 鼠标按键 */
    int8_t                  x;              /* X轴移动 */
    int8_t                  y;              /* Y轴移动 */
    int8_t                  wheel;          /* 滚轮滚动 */
} hid_mouse_report_t;

/* HID消费者控制报告结构体 */
typedef struct {
    uint16_t                usage;          /* 用法值 */
    int8_t                  value;          /* 用法值 */
} hid_consumer_report_t;

/* HID系统控制报告结构体 */
typedef struct {
    uint8_t                 usage;          /* 系统控制用法 */
} hid_system_report_t;

/* HID游戏手柄报告结构体 */
typedef struct {
    uint16_t                buttons;        /* 按键状态 */
    uint8_t                 x;              /* X轴 */
    uint8_t                 y;              /* Y轴 */
    uint8_t                 z;              /* Z轴 */
    uint8_t                 rx;             /* X轴旋转 */
    uint8_t                 ry;             /* Y轴旋转 */
    uint8_t                 rz;             /* Z轴旋转 */
    uint8_t                 slider;         /* 滑块 */
} hid_gamepad_report_t;

/* HID报告映射表结构体 */
typedef struct {
    hid_usage_page_t        usage_page;     /* 用法页 */
    uint16_t                usage;          /* 用法 */
    uint8_t                 report_id;      /* 报告ID */
    hid_report_type_t       report_type;    /* 报告类型 */
    uint8_t                 bit_offset;     /* 位偏移 */
    uint8_t                 bit_count;      /* 位数量 */
    int32_t                 logical_min;    /* 逻辑最小值 */
    int32_t                 logical_max;    /* 逻辑最大值 */
    int32_t                 physical_min;   /* 物理最小值 */
    int32_t                 physical_max;   /* 物理最大值 */
    uint16_t                unit_exponent;  /* 单位指数 */
    uint16_t                unit;           /* 单位 */
    bool                    is_absolute;    /* 是否绝对值 */
    bool                    is_array;       /* 是否数组 */
    bool                    is_variable;    /* 是否可变 */
    bool                    is_relative;    /* 是否相对值 */
    bool                    is_wrap;        /* 是否环绕 */
    bool                    is_non_linear;  /* 是否非线性 */
    bool                    is_no_preferred;/* 是否无偏好状态 */
    bool                    is_null_state;  /* 是否空状态 */
    bool                    is_volatile;    /* 是否易失性 */
    bool                    is_buffered;    /* 是否缓冲 */
} hid_report_map_t;

/* HID报告描述符结构体 */
typedef struct {
    uint8_t                *data;           /* 描述符数据 */
    uint16_t                size;           /* 描述符大小 */
    uint16_t                item_count;     /* 项目数量 */
    hid_report_map_t       *report_maps;    /* 报告映射表 */
    uint8_t                 report_map_count; /* 报告映射数量 */
    uint8_t                 input_report_count; /* 输入报告数量 */
    uint8_t                 output_report_count; /* 输出报告数量 */
    uint8_t                 feature_report_count; /* 特性报告数量 */
    uint16_t                max_input_report_size; /* 最大输入报告大小 */
    uint16_t                max_output_report_size; /* 最大输出报告大小 */
    uint16_t                max_feature_report_size; /* 最大特性报告大小 */
} hid_report_descriptor_t;

/* HID协议处理器配置结构体 */
typedef struct {
    bool                    enable_keyboard; /* 启用键盘报告 */
    bool                    enable_mouse;    /* 启用鼠标报告 */
    bool                    enable_consumer; /* 启用消费者控制报告 */
    bool                    enable_system;   /* 启用系统控制报告 */
    bool                    enable_gamepad;  /* 启用游戏手柄报告 */
    bool                    enable_joystick; /* 启用摇杆报告 */
    bool                    enable_digitizer;/* 启用数字化仪报告 */
    bool                    enable_vendor;   /* 启用厂商特定报告 */
    uint8_t                 keyboard_led_support; /* 键盘LED支持 */
    uint8_t                 mouse_button_count;   /* 鼠标按键数量 */
    uint8_t                 mouse_resolution;     /* 鼠标分辨率 */
    uint8_t                 consumer_button_count; /* 消费者控制按键数量 */
    uint8_t                 gamepad_button_count; /* 游戏手柄按键数量 */
    uint8_t                 joystick_button_count; /* 摇杆按键数量 */
    uint16_t                report_interval_ms; /* 报告间隔(毫秒) */
    bool                    auto_repeat;     /* 自动重复 */
    uint16_t                auto_repeat_delay_ms; /* 自动重复延迟(毫秒) */
    uint16_t                auto_repeat_rate_ms; /* 自动重复速率(毫秒) */
    bool                    nkro_support;    /* N-Key Rollover支持 */
    bool                    rollover_protection; /* 按键翻转保护 */
    bool                    ghosting_prevention; /* 防鬼键 */
    uint8_t                 max_concurrent_keys; /* 最大并发按键数 */
    bool                    power_save_mode; /* 省电模式 */
    uint32_t                sleep_timeout_ms; /* 睡眠超时(毫秒) */
} hid_processor_config_t;

/* HID处理器统计信息结构体 */
typedef struct {
    uint32_t                total_reports_sent;    /* 发送的总报告数 */
    uint32_t                total_reports_received; /* 接收的总报告数 */
    uint32_t                keyboard_reports_sent;  /* 发送的键盘报告数 */
    uint32_t                mouse_reports_sent;     /* 发送的鼠标报告数 */
    uint32_t                consumer_reports_sent;  /* 发送的消费者控制报告数 */
    uint32_t                system_reports_sent;    /* 发送的系统控制报告数 */
    uint32_t                gamepad_reports_sent;   /* 发送的游戏手柄报告数 */
    uint32_t                joystick_reports_sent;  /* 发送的摇杆报告数 */
    uint32_t                digitizer_reports_sent; /* 发送的数字化仪报告数 */
    uint32_t                vendor_reports_sent;    /* 发送的厂商特定报告数 */
    uint32_t                report_errors;         /* 报告错误数 */
    uint32_t                buffer_overflows;      /* 缓冲区溢出数 */
    uint32_t                protocol_errors;       /* 协议错误数 */
    uint32_t                checksum_errors;       /* 校验错误数 */
    uint32_t                timeout_errors;        /* 超时错误数 */
    uint32_t                key_press_count;      /* 按键按下次数 */
    uint32_t                key_release_count;    /* 按键释放次数 */
    uint32_t                mouse_click_count;     /* 鼠标点击次数 */
    uint32_t                mouse_move_count;     /* 鼠标移动次数 */
    uint32_t                wheel_scroll_count;    /* 滚轮滚动次数 */
    uint32_t                last_activity_time;   /* 最后活动时间 */
    uint32_t                uptime_seconds;       /* 运行时间(秒) */
} hid_processor_statistics_t;

/* HID报告回调函数类型 */
typedef void (*hid_report_callback_t)(hid_report_id_t report_id,
                                       const void *report_data,
                                       uint16_t report_size,
                                       void *user_data);

typedef void (*hid_led_callback_t)(uint8_t led_state, void *user_data);

typedef void (*hid_error_callback_t)(hid_processor_result_t error,
                                       const char *error_message,
                                       void *user_data);

/* HID协议处理器接口函数指针结构体 */
typedef struct {
    /**
     * @brief 初始化HID协议处理器
     * @param config 处理器配置
     * @return 初始化结果
     */
    hid_processor_result_t (*init)(const hid_processor_config_t *config);

    /**
     * @brief 反初始化HID协议处理器
     * @return 反初始化结果
     */
    hid_processor_result_t (*deinit)(void);

    /**
     * @brief 设置报告描述符
     * @param descriptor 报告描述符
     * @return 设置结果
     */
    hid_processor_result_t (*set_report_descriptor)(const hid_report_descriptor_t *descriptor);

    /**
     * @brief 获取报告描述符
     * @param descriptor 输出报告描述符
     * @return 获取结果
     */
    hid_processor_result_t (*get_report_descriptor)(hid_report_descriptor_t *descriptor);

    /**
     * @brief 发送键盘报告
     * @param modifier 修饰键状态
     * @param keys 按键数组
     * @param key_count 按键数量
     * @return 发送结果
     */
    hid_processor_result_t (*send_keyboard_report)(uint8_t modifier,
                                                        const uint8_t *keys,
                                                        uint8_t key_count);

    /**
     * @brief 发送鼠标报告
     * @param buttons 鼠标按键
     * @param x X轴移动
     * @param y Y轴移动
     * @param wheel 滚轮滚动
     * @return 发送结果
     */
    hid_processor_result_t (*send_mouse_report)(uint8_t buttons,
                                                     int8_t x,
                                                     int8_t y,
                                                     int8_t wheel);

    /**
     * @brief 发送消费者控制报告
     * @param usage 用法值
     * @param value 用法值
     * @return 发送结果
     */
    hid_processor_result_t (*send_consumer_report)(uint16_t usage,
                                                         int8_t value);

    /**
     * @brief 发送系统控制报告
     * @param usage 系统控制用法
     * @return 发送结果
     */
    hid_processor_result_t (*send_system_report)(uint8_t usage);

    /**
     * @brief 发送游戏手柄报告
     * @param buttons 按键状态
     * @param x X轴
     * @param y Y轴
     * @param z Z轴
     * @param rx X轴旋转
     * @param ry Y轴旋转
     * @param rz Z轴旋转
     * @param slider 滑块
     * @return 发送结果
     */
    hid_processor_result_t (*send_gamepad_report)(uint16_t buttons,
                                                        uint8_t x,
                                                        uint8_t y,
                                                        uint8_t z,
                                                        uint8_t rx,
                                                        uint8_t ry,
                                                        uint8_t rz,
                                                        uint8_t slider);

    /**
     * @brief 发送摇杆报告
     * @param buttons 按键状态
     * @param x X轴
     * @param y Y轴
     * @param z Z轴
     * @param rx X轴旋转
     * @param ry Y轴旋转
     * @param rz Z轴旋转
     * @param slider 滑块
     * @return 发送结果
     */
    hid_processor_result_t (*send_joystick_report)(uint16_t buttons,
                                                         uint8_t x,
                                                         uint8_t y,
                                                         uint8_t z,
                                                         uint8_t rx,
                                                         uint8_t ry,
                                                         uint8_t rz,
                                                         uint8_t slider);

    /**
     * @brief 发送数字化仪报告
     * @param report_data 报告数据
     * @param report_size 报告大小
     * @return 发送结果
     */
    hid_processor_result_t (*send_digitizer_report)(const void *report_data,
                                                         uint16_t report_size);

    /**
     * @brief 发送厂商特定报告
     * @param report_id 报告ID
     * @param report_data 报告数据
     * @param report_size 报告大小
     * @return 发送结果
     */
    hid_processor_result_t (*send_vendor_report)(uint8_t report_id,
                                                       const void *report_data,
                                                       uint16_t report_size);

    /**
     * @brief 发送原始HID报告
     * @param report_id 报告ID
     * @param report_type 报告类型
     * @param report_data 报告数据
     * @param report_size 报告大小
     * @return 发送结果
     */
    hid_processor_result_t (*send_raw_report)(uint8_t report_id,
                                                   hid_report_type_t report_type,
                                                   const void *report_data,
                                                   uint16_t report_size);

    /**
     * @brief 接收HID报告
     * @param report_id 输出报告ID
     * @param report_type 输出报告类型
     * @param report_data 输出报告数据
     * @param report_size 输入报告大小
     * @param max_size 最大报告大小
     * @return 接收结果
     */
    hid_processor_result_t (*receive_report)(uint8_t *report_id,
                                                  hid_report_type_t *report_type,
                                                  void *report_data,
                                                  uint16_t *report_size,
                                                  uint16_t max_size);

    /**
     * @brief 处理输出报告
     * @param report_id 报告ID
     * @param report_data 报告数据
     * @param report_size 报告大小
     * @return 处理结果
     */
    hid_processor_result_t (*handle_output_report)(uint8_t report_id,
                                                        const void *report_data,
                                                        uint16_t report_size);

    /**
     * @brief 处理特性报告
     * @param report_id 报告ID
     * @param report_data 报告数据
     * @param report_size 报告大小
     * @param response_data 响应数据
     * @param response_size 响应大小
     * @return 处理结果
     */
    hid_processor_result_t (*handle_feature_report)(uint8_t report_id,
                                                         const void *report_data,
                                                         uint16_t report_size,
                                                         void *response_data,
                                                         uint16_t *response_size);

    /**
     * @brief 获取LED状态
     * @param led_state 输出LED状态
     * @return 获取结果
     */
    hid_processor_result_t (*get_led_state)(uint8_t *led_state);

    /**
     * @brief 设置LED状态
     * @param led_state LED状态
     * @return 设置结果
     */
    hid_processor_result_t (*set_led_state)(uint8_t led_state);

    /**
     * @brief 设置LED状态回调
     * @param callback 回调函数
     * @param user_data 用户数据
     * @return 设置结果
     */
    hid_processor_result_t (*set_led_callback)(hid_led_callback_t callback,
                                                    void *user_data);

    /**
     * @brief 注册报告回调函数
     * @param report_id 报告ID
     * @param report_type 报告类型
     * @param callback 回调函数
     * @param user_data 用户数据
     * @return 注册结果
     */
    hid_processor_result_t (*register_report_callback)(hid_report_id_t report_id,
                                                             hid_report_type_t report_type,
                                                             hid_report_callback_t callback,
                                                             void *user_data);

    /**
     * @brief 注销报告回调函数
     * @param report_id 报告ID
     * @param report_type 报告类型
     * @return 注销结果
     */
    hid_processor_result_t (*unregister_report_callback)(hid_report_id_t report_id,
                                                               hid_report_type_t report_type);

    /**
     * @brief 注册错误回调函数
     * @param callback 回调函数
     * @param user_data 用户数据
     * @return 注册结果
     */
    hid_processor_result_t (*register_error_callback)(hid_error_callback_t callback,
                                                          void *user_data);

    /**
     * @brief 注销错误回调函数
     * @return 注销结果
     */
    hid_processor_result_t (*unregister_error_callback)(void);

    /**
     * @brief 启用/禁用按键
     * @param usage 按键用法
     * @param enable 是否启用
     * @return 设置结果
     */
    hid_processor_result_t (*enable_key)(hid_keyboard_usage_t usage, bool enable);

    /**
     * @brief 启用/禁用按键组
     * @param usages 按键用法数组
     * @param count 按键数量
     * @param enable 是否启用
     * @return 设置结果
     */
    hid_processor_result_t (*enable_key_group)(const hid_keyboard_usage_t *usages,
                                                     uint8_t count,
                                                     bool enable);

    /**
     * @brief 检查按键是否启用
     * @param usage 按键用法
     * @param enabled 输出是否启用标志
     * @return 检查结果
     */
    hid_processor_result_t (*is_key_enabled)(hid_keyboard_usage_t usage, bool *enabled);

    /**
     * @brief 设置按键映射
     * @param src_usage 源按键用法
     * @param dst_usage 目标按键用法
     * @return 设置结果
     */
    hid_processor_result_t (*set_key_mapping)(hid_keyboard_usage_t src_usage,
                                                      hid_keyboard_usage_t dst_usage);

    /**
     * @brief 获取按键映射
     * @param usage 按键用法
     * @param mapped_usage 输出映射按键用法
     * @return 获取结果
     */
    hid_processor_result_t (*get_key_mapping)(hid_keyboard_usage_t usage,
                                                     hid_keyboard_usage_t *mapped_usage);

    /**
     * @brief 清除按键映射
     * @param usage 按键用法
     * @return 清除结果
     */
    hid_processor_result_t (*clear_key_mapping)(hid_keyboard_usage_t usage);

    /**
     * @brief 清除所有按键映射
     * @return 清除结果
     */
    hid_processor_result_t (*clear_all_key_mappings)(void);

    /**
     * @brief 启用/禁用自动重复
     * @param enable 是否启用
     * @return 设置结果
     */
    hid_processor_result_t (*enable_auto_repeat)(bool enable);

    /**
     * @brief 设置自动重复参数
     * @param delay_ms 重复延迟(毫秒)
     * @param rate_ms 重复速率(毫秒)
     * @return 设置结果
     */
    hid_processor_result_t (*set_auto_repeat_params)(uint16_t delay_ms, uint16_t rate_ms);

    /**
     * @brief 启用/禁用N-Key Rollover
     * @param enable 是否启用
     * @return 设置结果
     */
    hid_processor_result_t (*enable_nkro)(bool enable);

    /**
     * @brief 获取处理器状态
     * @param initialized 输出是否已初始化标志
     * @param active 输出是否活跃标志
     * @return 获取结果
     */
    hid_processor_result_t (*get_state)(bool *initialized, bool *active);

    /**
     * @brief 处理器更新处理
     * @return 更新结果
     */
    hid_processor_result_t (*update)(void);

    /**
     * @brief 重置处理器
     * @return 重置结果
     */
    hid_processor_result_t (*reset)(void);

    /**
     * @brief 进入睡眠模式
     * @return 进入结果
     */
    hid_processor_result_t (*enter_sleep_mode)(void);

    /**
     * @brief 退出睡眠模式
     * @return 退出结果
     */
    hid_processor_result_t (*exit_sleep_mode)(void);

    /**
     * @brief 获取处理器统计信息
     * @param stats 输出统计信息
     * @return 获取结果
     */
    hid_processor_result_t (*get_statistics)(hid_processor_statistics_t *stats);

    /**
     * @brief 重置处理器统计信息
     * @return 重置结果
     */
    hid_processor_result_t (*reset_statistics)(void);

    /**
     * @brief 获取处理器版本
     * @return 版本号
     */
    uint32_t (*get_version)(void);
} hid_processor_interface_t;

/* 全局HID协议处理器实例 */
extern const hid_processor_interface_t *hid_processor;

/* 常用HID配置宏 */
#define HID_PROCESSOR_CONFIG_DEFAULT() \
    { true, true, true, false, false, false, false, false, \
      HID_LED_NUM_LOCK | HID_LED_CAPS_LOCK | HID_LED_SCROLL_LOCK, \
      3, 800, 0, 0, 0, 0, 10, false, 500, 50, true, true, true, 6, \
      false, 30000 }

#define HID_PROCESSOR_CONFIG_KEYBOARD_ONLY() \
    { true, false, false, false, false, false, false, false, \
      HID_LED_NUM_LOCK | HID_LED_CAPS_LOCK | HID_LED_SCROLL_LOCK, \
      0, 0, 0, 0, 0, 0, 10, false, 500, 50, true, true, true, 6, \
      false, 30000 }

#define HID_PROCESSOR_CONFIG_MOUSE_ONLY() \
    { false, true, false, false, false, false, false, false, \
      0, 3, 800, 0, 0, 0, 10, false, 0, 0, false, false, false, 0, \
      false, 30000 }

#define HID_PROCESSOR_CONFIG_COMBO() \
    { true, true, true, false, false, false, false, false, \
      HID_LED_NUM_LOCK | HID_LED_CAPS_LOCK | HID_LED_SCROLL_LOCK, \
      3, 800, 8, 0, 0, 0, 10, false, 500, 50, true, true, true, 6, \
      false, 30000 }

/* HID错误处理宏 */
#define HID_PROCESSOR_CHECK(expr) \
    do { \
        hid_processor_result_t _result = (expr); \
        if (_result != HID_PROCESSOR_OK) { \
            return _result; \
        } \
    } while(0)

#define HID_PROCESSOR_CHECK_RET(expr, ret_val) \
    do { \
        hid_processor_result_t _result = (expr); \
        if (_result != HID_PROCESSOR_OK) { \
            return (ret_val); \
        } \
    } while(0)

/* 便捷操作宏 */
#define HID_SEND_KEYBOARD(modifier, keys, count) \
    hid_processor->send_keyboard_report((modifier), (keys), (count))

#define HID_SEND_MOUSE(buttons, x, y, wheel) \
    hid_processor->send_mouse_report((buttons), (x), (y), (wheel))

#define HID_SEND_CONSUMER(usage, value) \
    hid_processor->send_consumer_report((usage), (value))

#define HID_SEND_SYSTEM(usage) \
    hid_processor->send_system_report((usage))

#define HID_GET_LED_STATE(state) \
    hid_processor->get_led_state((state))

#define HID_SET_LED_STATE(state) \
    hid_processor->set_led_state((state))

#define HID_ENABLE_NKRO() \
    hid_processor->enable_nkro(true)

#define HID_DISABLE_NKRO() \
    hid_processor->enable_nkro(false)

#define HID_ENABLE_AUTO_REPEAT() \
    hid_processor->enable_auto_repeat(true)

#define HID_DISABLE_AUTO_REPEAT() \
    hid_processor->enable_auto_repeat(false)

#define HID_PROCESSOR_UPDATE() \
    hid_processor->update()

#ifdef __cplusplus
}
#endif

#endif /* HID_PROCESSOR_H */