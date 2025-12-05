/**
 * @file input_interface.h
 * @brief 输入设备驱动接口定义
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * 提供键盘矩阵、触控板、额外按键等输入设备的统一接口
 */

#ifndef INPUT_INTERFACE_H
#define INPUT_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 输入设备错误码定义 */
typedef enum {
    INPUT_OK = 0,                    /* 操作成功 */
    INPUT_ERROR_INVALID_DEVICE,      /* 无效设备 */
    INPUT_ERROR_INVALID_PARAM,       /* 无效参数 */
    INPUT_ERROR_NOT_SUPPORTED,       /* 操作不支持 */
    INPUT_ERROR_HARDWARE_FAULT,      /* 硬件故障 */
    INPUT_ERROR_TIMEOUT,             /* 操作超时 */
    INPUT_ERROR_ALREADY_INITIALIZED, /* 已初始化 */
    INPUT_ERROR_NOT_INITIALIZED,     /* 未初始化 */
    INPUT_ERROR_BUFFER_OVERFLOW,     /* 缓冲区溢出 */
    INPUT_ERROR_DEVICE_BUSY,         /* 设备忙碌 */
    INPUT_ERROR_CALIBRATION_FAILED,  /* 校准失败 */
    INPUT_ERROR_NOT_READY            /* 设备未就绪 */
} input_result_t;

/* 输入设备类型定义 */
typedef enum {
    INPUT_TYPE_KEYBOARD = 0,         /* 键盘矩阵 */
    INPUT_TYPE_TOUCHPAD,             /* 触控板 */
    INPUT_TYPE_BUTTONS,              /* 额外按键 */
    INPUT_TYPE_GESTURE,              /* 手势识别 */
    INPUT_TYPE_ENCODER,              /* 旋转编码器 */
    INPUT_TYPE_JOYSTICK,             /* 摇杆 */
    INPUT_TYPE_ACCELEROMETER,        /* 加速度计 */
    INPUT_TYPE_GYROSCOPE            /* 陀螺仪 */
} input_device_type_t;

/* 输入设备状态定义 */
typedef enum {
    INPUT_STATE_DISABLED = 0,        /* 禁用状态 */
    INPUT_STATE_ENABLED,             /* 启用状态 */
    INPUT_STATE_SCANNING,            /* 扫描状态 */
    INPUT_STATE_CALIBRATING,         /* 校准状态 */
    INPUT_STATE_SUSPENDED,           /* 挂起状态 */
    INPUT_STATE_ERROR               /* 错误状态 */
} input_device_state_t;

/* 键盘按键状态定义 */
typedef enum {
    KEY_STATE_RELEASED = 0,          /* 按键释放 */
    KEY_STATE_PRESSED,               /* 按键按下 */
    KEY_STATE_HOLDING,               /* 按键保持 */
    KEY_STATE_REPEAT,               /* 按键重复 */
    KEY_STATE_UNKNOWN               /* 未知状态 */
} key_state_t;

/* 触控点状态定义 */
typedef enum {
    TOUCH_POINT_UP = 0,             /* 触控点抬起 */
    TOUCH_POINT_DOWN,               /* 触控点按下 */
    TOUCH_POINT_MOVE,               /* 触控点移动 */
    TOUCH_POINT_STATIONARY          /* 触控点静止 */
} touch_point_state_t;

/* 手势类型定义 */
typedef enum {
    GESTURE_NONE = 0,               /* 无手势 */
    GESTURE_TAP_SINGLE,            /* 单击 */
    GESTURE_TAP_DOUBLE,            /* 双击 */
    GESTURE_TAP_TRIPLE,            /* 三击 */
    GESTURE_SCROLL_UP,             /* 向上滚动 */
    GESTURE_SCROLL_DOWN,           /* 向下滚动 */
    GESTURE_SCROLL_LEFT,           /* 向左滚动 */
    GESTURE_SCROLL_RIGHT,          /* 向右滚动 */
    GESTURE_SWIPE_UP,              /* 向上滑动 */
    GESTURE_SWIPE_DOWN,            /* 向下滑动 */
    GESTURE_SWIPE_LEFT,            /* 向左滑动 */
    GESTURE_SWIPE_RIGHT,           /* 向右滑动 */
    GESTURE_PINCH_IN,              /* 向内捏合 */
    GESTURE_PINCH_OUT,             /* 向外捏合 */
    GESTURE_ROTATE_CW,             /* 顺时针旋转 */
    GESTURE_ROTATE_CCW,            /* 逆时针旋转 */
    GESTURE_HOLD,                  /* 长按 */
    GESTURE_PALM                   /* 手掌接触 */
} gesture_type_t;

/* 键盘矩阵配置结构体 */
typedef struct {
    uint8_t                 rows;           /* 行数 */
    uint8_t                 cols;           /* 列数 */
    uint8_t                *row_pins;       /* 行引脚数组 */
    uint8_t                *col_pins;       /* 列引脚数组 */
    uint16_t               *keymap;         /* 按键映射表 */
    uint8_t                 debounce_ms;    /* 消抖时间(毫秒) */
    bool                    diode_direction; /* 二极管方向: true=行->列, false=列->行 */
    bool                    enable_rollover; /* 启用按键翻转 */
    bool                    enable_ghosting_prevention; /* 启用防鬼键 */
} keyboard_matrix_config_t;

/* 键盘事件结构体 */
typedef struct {
    uint16_t                key_code;       /* 按键编码 */
    uint8_t                 row;            /* 行号 */
    uint8_t                 col;            /* 列号 */
    key_state_t             state;          /* 按键状态 */
    uint32_t                timestamp;      /* 时间戳(毫秒) */
    uint16_t                duration;       /* 持续时间(毫秒) */
} keyboard_event_t;

/* 触控板配置结构体 */
typedef struct {
    uint16_t                width;          /* 触控区域宽度 */
    uint16_t                height;         /* 触控区域高度 */
    uint8_t                 max_points;     /* 最大触控点数 */
    uint16_t                resolution_x;   /* X轴分辨率 */
    uint16_t                resolution_y;   /* Y轴分辨率 */
    uint8_t                 sensitivity;    /* 灵敏度等级 */
    bool                    enable_gesture; /* 启用手势识别 */
    bool                    enable_scroll;  /* 启用滚动 */
    bool                    enable_tap;     /* 启用点击 */
    bool                    enable_palm;    /* 启用手掌检测 */
} touchpad_config_t;

/* 触控点结构体 */
typedef struct {
    uint8_t                 id;             /* 触控点ID */
    uint16_t                x;              /* X坐标 */
    uint16_t                y;              /* Y坐标 */
    int16_t                 delta_x;        /* X轴增量 */
    int16_t                 delta_y;        /* Y轴增量 */
    uint8_t                 pressure;       /* 压力值 */
    touch_point_state_t     state;          /* 触控状态 */
    uint32_t                timestamp;      /* 时间戳 */
} touch_point_t;

/* 触控板事件结构体 */
typedef struct {
    uint8_t                 point_count;    /* 触控点数量 */
    touch_point_t          *points;        /* 触控点数组 */
    gesture_type_t          gesture;        /* 检测到的手势 */
    int16_t                 scroll_x;       /* X轴滚动量 */
    int16_t                 scroll_y;       /* Y轴滚动量 */
    uint32_t                timestamp;      /* 时间戳 */
} touchpad_event_t;

/* 额外按键配置结构体 */
typedef struct {
    uint8_t                 button_count;   /* 按键数量 */
    uint8_t                *button_pins;    /* 按键引脚数组 */
    uint8_t                *button_codes;   /* 按键编码数组 */
    uint8_t                 debounce_ms;    /* 消抖时间 */
    bool                    enable_interrupt; /* 启用中断模式 */
} buttons_config_t;

/* 按键事件结构体 */
typedef struct {
    uint8_t                 button_id;      /* 按键ID */
    uint16_t                key_code;       /* 按键编码 */
    key_state_t             state;          /* 按键状态 */
    uint32_t                timestamp;      /* 时间戳 */
} button_event_t;

/* 手势识别配置结构体 */
typedef struct {
    bool                    enable_tap;     /* 启用点击手势 */
    bool                    enable_scroll;  /* 启用滚动手势 */
    bool                    enable_swipe;   /* 启用滑动手势 */
    bool                    enable_pinch;   /* 启用捏合手势 */
    bool                    enable_rotate;  /* 启用旋转手势 */
    bool                    enable_hold;    /* 启用长按手势 */
    uint8_t                 tap_threshold;  /* 点击阈值(毫秒) */
    uint8_t                 hold_threshold; /* 长按阈值(毫秒) */
    uint8_t                 scroll_threshold; /* 滚动阈值 */
    uint8_t                 swipe_threshold; /* 滑动阈值 */
    uint8_t                 pinch_threshold; /* 捏合阈值 */
    uint8_t                 rotate_threshold; /* 旋转阈值 */
} gesture_config_t;

/* 输入设备通用配置结构体 */
typedef struct {
    input_device_type_t     type;           /* 设备类型 */
    input_device_state_t    state;          /* 设备状态 */
    uint8_t                 priority;       /* 设备优先级 */
    uint32_t                scan_interval;  /* 扫描间隔(微秒) */
    bool                    enable_interrupt; /* 启用中断模式 */
    bool                    enable_polling; /* 启用轮询模式 */
    union {
        keyboard_matrix_config_t keyboard;  /* 键盘矩阵配置 */
        touchpad_config_t    touchpad;      /* 触控板配置 */
        buttons_config_t     buttons;       /* 额外按键配置 */
        gesture_config_t     gesture;       /* 手势识别配置 */
    } device_config;
} input_device_config_t;

/* 输入设备通用事件结构体 */
typedef struct {
    input_device_type_t     device_type;    /* 设备类型 */
    uint8_t                 device_id;      /* 设备ID */
    union {
        keyboard_event_t    keyboard;       /* 键盘事件 */
        touchpad_event_t    touchpad;       /* 触控板事件 */
        button_event_t      button;         /* 按键事件 */
    } event_data;
    uint32_t                timestamp;      /* 时间戳 */
} input_event_t;

/* 输入事件回调函数类型 */
typedef void (*input_event_callback_t)(const input_event_t *event, void *user_data);

/* 输入设备接口函数指针结构体 */
typedef struct {
    /**
     * @brief 初始化输入设备
     * @param config 设备配置
     * @return 初始化结果
     */
    input_result_t (*init)(const input_device_config_t *config);

    /**
     * @brief 反初始化输入设备
     * @return 反初始化结果
     */
    input_result_t (*deinit)(void);

    /**
     * @brief 启用输入设备
     * @return 启用结果
     */
    input_result_t (*enable)(void);

    /**
     * @brief 禁用输入设备
     * @return 禁用结果
     */
    input_result_t (*disable)(void);

    /**
     * @brief 配置输入设备
     * @param config 设备配置
     * @return 配置结果
     */
    input_result_t (*config)(const input_device_config_t *config);

    /**
     * @brief 开始扫描
     * @return 开始扫描结果
     */
    input_result_t (*start_scan)(void);

    /**
     * @brief 停止扫描
     * @return 停止扫描结果
     */
    input_result_t (*stop_scan)(void);

    /**
     * @brief 读取输入事件
     * @param event 输出事件结构体
     * @return 读取结果
     */
    input_result_t (*read_event)(input_event_t *event);

    /**
     * @brief 检查是否有待处理事件
     * @param has_event 输出是否有事件标志
     * @return 检查结果
     */
    input_result_t (*has_event)(bool *has_event);

    /**
     * @brief 清空事件队列
     * @return 清空结果
     */
    input_result_t (*clear_events)(void);

    /**
     * @brief 注册事件回调函数
     * @param callback 回调函数
     * @param user_data 用户数据
     * @return 注册结果
     */
    input_result_t (*register_callback)(input_event_callback_t callback, void *user_data);

    /**
     * @brief 注销事件回调函数
     * @param callback 回调函数
     * @return 注销结果
     */
    input_result_t (*unregister_callback)(input_event_callback_t callback);

    /**
     * @brief 获取设备状态
     * @param state 输出设备状态
     * @return 获取结果
     */
    input_result_t (*get_state)(input_device_state_t *state);

    /**
     * @brief 设置设备状态
     * @param state 设备状态
     * @return 设置结果
     */
    input_result_t (*set_state)(input_device_state_t state);

    /**
     * @brief 校准设备
     * @return 校准结果
     */
    input_result_t (*calibrate)(void);

    /**
     * @brief 重置设备
     * @return 重置结果
     */
    input_result_t (*reset)(void);

    /**
     * @brief 获取设备信息
     * @param info 输出设备信息
     * @return 获取结果
     */
    input_result_t (*get_info)(char *info, uint16_t max_len);

    /**
     * @brief 获取设备版本
     * @return 版本号
     */
    uint32_t (*get_version)(void);
} input_device_interface_t;

/* 输入管理器接口函数指针结构体 */
typedef struct {
    /**
     * @brief 初始化输入管理器
     * @return 初始化结果
     */
    input_result_t (*init)(void);

    /**
     * @brief 反初始化输入管理器
     * @return 反初始化结果
     */
    input_result_t (*deinit)(void);

    /**
     * @brief 注册输入设备
     * @param device_type 设备类型
     * @param device_interface 设备接口
     * @param config 设备配置
     * @param device_id 输出设备ID
     * @return 注册结果
     */
    input_result_t (*register_device)(input_device_type_t device_type,
                                     const input_device_interface_t *device_interface,
                                     const input_device_config_t *config,
                                     uint8_t *device_id);

    /**
     * @brief 注销输入设备
     * @param device_id 设备ID
     * @return 注销结果
     */
    input_result_t (*unregister_device)(uint8_t device_id);

    /**
     * @brief 启用指定设备
     * @param device_id 设备ID
     * @return 启用结果
     */
    input_result_t (*enable_device)(uint8_t device_id);

    /**
     * @brief 禁用指定设备
     * @param device_id 设备ID
     * @return 禁用结果
     */
    input_result_t (*disable_device)(uint8_t device_id);

    /**
     * @brief 开始所有设备扫描
     * @return 开始扫描结果
     */
    input_result_t (*start_all_scans)(void);

    /**
     * @brief 停止所有设备扫描
     * @return 停止扫描结果
     */
    input_result_t (*stop_all_scans)(void);

    /**
     * @brief 获取合并后的输入事件
     * @param event 输出事件结构体
     * @return 获取结果
     */
    input_result_t (*get_merged_event)(input_event_t *event);

    /**
     * @brief 处理所有输入设备
     * @return 处理结果
     */
    input_result_t (*process_all_devices)(void);

    /**
     * @brief 注册全局事件回调
     * @param callback 回调函数
     * @param user_data 用户数据
     * @return 注册结果
     */
    input_result_t (*register_global_callback)(input_event_callback_t callback, void *user_data);

    /**
     * @brief 注销全局事件回调
     * @param callback 回调函数
     * @return 注销结果
     */
    input_result_t (*unregister_global_callback)(input_event_callback_t callback);

    /**
     * @brief 获取已注册设备数量
     * @return 设备数量
     */
    uint8_t (*get_device_count)(void);

    /**
     * @brief 获取输入管理器版本
     * @return 版本号
     */
    uint32_t (*get_version)(void);
} input_manager_interface_t;

/* 全局输入管理器实例 */
extern const input_manager_interface_t *input_manager;

/* 常用配置宏 */
#define KEYBOARD_MATRIX_CONFIG_DEFAULT(rows, cols, r_pins, c_pins) \
    { (rows), (cols), (r_pins), (c_pins), NULL, 10, true, true, true }

#define TOUCHPAD_CONFIG_DEFAULT(width, height) \
    { (width), (height), 1, 1000, 1000, 5, true, true, true, true }

#define BUTTONS_CONFIG_DEFAULT(count, pins, codes) \
    { (count), (pins), (codes), 10, true }

#define INPUT_DEVICE_CONFIG_DEFAULT(type) \
    { (type), INPUT_STATE_DISABLED, 0, 1000, false, true, {0} }

/* 输入错误处理宏 */
#define INPUT_CHECK(expr) \
    do { \
        input_result_t _result = (expr); \
        if (_result != INPUT_OK) { \
            return _result; \
        } \
    } while(0)

#define INPUT_CHECK_RET(expr, ret_val) \
    do { \
        input_result_t _result = (expr); \
        if (_result != INPUT_OK) { \
            return (ret_val); \
        } \
    } while(0)

/* 便捷操作宏 */
#define INPUT_ENABLE_ALL() \
    input_manager->start_all_scans()

#define INPUT_DISABLE_ALL() \
    input_manager->stop_all_scans()

#define INPUT_PROCESS_ALL() \
    input_manager->process_all_devices()

#ifdef __cplusplus
}
#endif

#endif /* INPUT_INTERFACE_H */