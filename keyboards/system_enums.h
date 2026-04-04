#pragma once

/**
 * @file system_enums.h
 * @brief 跨模块共用枚举定义
 * @note 仅放置被多个模块引用的枚举，模块内部枚举保留在各自头文件
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
/**
 * 错误码定义规则:
 * 1. 所有错误码格式为0xCxxx
 * 2. 按模块分类:
 *    - 0xC0xx: 通信模块错误
 *       - 0xC0Ax: 通信参数错误
 *       - 0xC0Bx: 通信状态错误
 *    - 0xC1xx: 软件相关错误
 *       - 0xC1Ax: 任务管理错误
 *       - 0xC1Bx: 定时器管理错误
 *       - 0xC1Cx: 事件管理错误
 *       - 0xC1Dx: 消息管理错误
 *       - 0xC1Ex: 内存管理错误
 *       - 0xC1Fx: 文件管理错误
 *       - 0xC1Gx: 网络管理错误
 */
typedef enum {
    NO_ERROR = 0,                // 无错误
    /* 通信相关错误码  0xC0-0xFF*/
    ERROR_COMM_PARAM   = 0xC0,           // 参数错误
    ERROR_COMM_NO_MEM  = 0xC1,         // 内存不足
    ERROR_COMM_LENGTH  = 0xC2,         // 长度错误
    ERROR_COMM_CRC     = 0xC3,           // CRC校验错误
    ERROR_COMM_FORMAT  = 0xC4,        // 格式错误
    ERROR_COMM_STATE   = 0xC5,         // 状态错误
    ERROR_COMM_TIMEOUT = 0xC6,       // 超时错误
    ERROR_COMM_SESSION = 0xC7,       // 会话错误
    ERROR_COMM_ENCRYPT = 0xC8,        // 加密错误
    ERROR_COMM_DECRYPT = 0xC9,        // 解密错误
    ERROR_COMM_COMMAND = 0xCA,        // 命令错误
    ERROR_COMM_VERSION = 0xCB,        // 版本错误
    ERROR_COMM_RX_TIMEOUT = 0xCC,     // 接收超时
    ERROR_COMM_BUSY    = 0xCD,        // 设备忙
    ERROR_COMM_NO_RESOURCE = 0xCE,    // 资源不足
    ERROR_COMM_ALREADY_INIT = 0xCF,  // 已初始化
    ERROR_COMM_NOT_FOUND = 0xD0,     // 未找到
    ERROR_COMM_ALREADY_EXIST = 0xD1, // 已存在

    /* TAG管理相关错误: 0xE0-0xE6 */
    ERROR_INVALID_PARAM = 0xE0,             /* 无效参数 */
    ERROR_INVALID_TYPE = 0xE1,              /* 无效类型 */
    ERROR_INVALID_LENGTH = 0xE2,            /* 无效长度 */
    ERROR_ACCESS_DENIED = 0xE3,             /* 访问被拒绝 */
    ERROR_NOT_FOUND = 0xE4,                 /* 未找到TAG */
    ERROR_BUFFER_TOO_SMALL = 0xE5,          /* 缓冲区太小 */
    ERROR_NOT_IMPLEMENTED = 0xE6,           /* 功能未实现 */
    ERROR_NO_MEMORY = 0xE8,                // 内存不足
    ERROR_NO_RESOURCE = 0xE9,              // 资源不足

    /* GPIO相关错误: 0xC0Ax */
    ERROR_GPIO_INVALID_PORT = 0xC0A0,          /* GPIO端口号无效 */
    ERROR_GPIO_INVALID_PIN = 0xC0A1,           /* GPIO引脚号无效 */
    ERROR_GPIO_INVALID_MODE = 0xC0A2,          /* GPIO工作模式配置无效 */
    ERROR_GPIO_INVALID_INTERRUPT_MODE = 0xC0A3, /* GPIO中断模式配置无效 */

    /* UART相关错误: 0xC0Bx */
    ERROR_UART_INVALID_DATA_BITS = 0xC0B0,     /* UART数据位配置无效 */
    ERROR_UART_INVALID_STOP_BITS = 0xC0B1,     /* UART停止位配置无效 */
    ERROR_UART_INVALID_PARITY = 0xC0B2,        /* UART校验位配置无效 */
    ERROR_UART_INVALID_FLOW_CONTROL = 0xC0B3,   /* UART流控制配置无效 */
    ERROR_UART_INVALID_BAUD_RATE = 0xC0B4,     /* UART波特率配置无效 */
    ERROR_UART_INVALID_PORT = 0xC0B5,          /* UART端口号无效 */

    /* 任务管理相关错误: 0xC1Ax */
    ERROR_INVALID_TASK_ID = 0xC1A0,            /* 任务ID无效 */
    ERROR_EVENT_TRIGGER_FAILED = 0xC1A1,       /* 事件触发失败 */
    ERROR_EVENT_CLEAR_FAILED = 0xC1A2,         /* 事件清除失败 */
    ERROR_EVENT_START_DELAYED_FAILED = 0xC1A3, /* 启动定时事件失败 */
    ERROR_EVENT_START_RELOAD_FAILED = 0xC1A4,   /* 启动自动重载定时事件失败 */
    ERROR_NO_TIMER_AVAILABLE = 0xC1A5,         /* 无可用定时器 */
    ERROR_EVENT_SEND_TO_TASK_FAILED = 0xC1A6,   /* 事件发送失败 */
    ERROR_INVALID_MSG_POINTER = 0xC1A7,         /* 无效消息指针 */
    ERROR_EVENT_STOP_FAILED = 0xC1A8,           /* 事件停止失败 */
    ERROR_EVENT_DEALLOCATE_FAILED = 0xC1A9,     /* 事件删除失败 */
    
    ERROR_INVALID_TIMER_ID = 0xC1A10,         /* 无效定时器ID */
    ERROR_TIMER_START_FAILED = 0xC1A11,       /* 定时器启动失败 */
    ERROR_TIMER_STOP_FAILED = 0xC1A12,        /* 定时器停止失败 */
} error_code_t;



/**
 * @brief USB HID 国家代码枚举
 *
 * 根据 USB HID Usage Tables 规范定义的国家代码。
 * 注意：仅包含本项目支持的常见国家代码。
 */
typedef enum {
    COUNTRY_NOT_SUPPORTED   = 0x00, /* 不支持的国家代码 */
    COUNTRY_ARABIC          = 0x01, /* 阿拉伯 */
    COUNTRY_BELGIAN         = 0x02, /* 比利时 */
    COUNTRY_CANADIAN_BI     = 0x03, /* 加拿大双语 */
    COUNTRY_CANADIAN_FRENCH = 0x04, /* 加拿大法语 */
    COUNTRY_CZECH_REPUBLIC  = 0x05, /* 捷克共和国 */
    COUNTRY_DANISH          = 0x06, /* 丹麦 */
    COUNTRY_FINNISH         = 0x07, /* 芬兰 */
    COUNTRY_FRENCH          = 0x09, /* 法国 */
    COUNTRY_GERMAN          = 0x0C, /* 德国 */
    COUNTRY_GREEK           = 0x0D, /* 希腊 */
    COUNTRY_HEBREW          = 0x0E, /* 希伯来 */
    COUNTRY_JAPANESE        = 0x0F, /* 日本 */
    COUNTRY_HUNGARIAN       = 0x10, /* 匈牙利 */
    COUNTRY_INTERNATIONAL   = 0x11, /* 国际 ISO */
    COUNTRY_ITALIAN         = 0x12, /* 意大利 */
    COUNTRY_NETHERLANDS_DUTCH = 0x13, /* 荷兰 */
    COUNTRY_NORWEGIAN       = 0x14, /* 挪威 */
    COUNTRY_KOREAN          = 0x15, /* 韩国 */
    COUNTRY_PERSIAN_FARSI   = 0x16, /* 波斯法尔斯 */
    COUNTRY_POLAND          = 0x17, /* 波兰 */
    COUNTRY_UK              = 0x18, /* 英国 */
    COUNTRY_PORTUGUESE      = 0x19, /* 葡萄牙 */
    COUNTRY_RUSSIA          = 0x1B, /* 俄罗斯 */
    COUNTRY_SLOVAKIA        = 0x1C, /* 斯洛伐克 */
    COUNTRY_SPANISH         = 0x1D, /* 西班牙 */
    COUNTRY_SWEDISH         = 0x1E, /* 瑞典 */
    COUNTRY_SWISS_FRENCH    = 0x1F, /* 瑞士法语 */
    COUNTRY_SWISS_GERMAN    = 0x20, /* 瑞士德语 */
    COUNTRY_US              = 0x21, /* 美国 */
    COUNTRY_TAIWAN          = 0x24, /* 台湾 */
    COUNTRY_CHINESE_SIMPLIFIED = 0x25, /* 简体中文 */
    COUNTRY_CHINESE_TRADITIONAL = 0x26, /* 繁体中文 */
} hid_country_code_t;


/**
 * @file keyboard_defaults.h
 * @brief 键盘行为默认配置
 * @note 产品可通过在 config_product.h 中 #define 覆盖默认值
 */
//系统类型枚举
enum
{
    ANDROID,
    IOS,
    WIN,
    MAC
};
#define ALL_SYSTEM 0xff // for combo map, 0xff means trigger for all system
#define APPLE_SYSTEM (IOS | MAC) // for combo map, 0x0c means trigger for apple system (macOS and iOS), but not trigger for windows and android
#define OTHER_SYSTEM (ALL_SYSTEM & ~APPLE_SYSTEM) // for combo map, 0x03 means trigger for windows and android, but not trigger for macOS and iOS

// 设备类型 用于判断设备是否支持move功能
enum _Origin_DeviceType_
{
    Keyboard = 0x01,
    Mouse,
    Touchpad,
    MulKeyboardTouchpad,
};


#ifdef __cplusplus
}
#endif
