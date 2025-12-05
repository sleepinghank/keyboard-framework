#ifndef SYS_ERROR_H
#define SYS_ERROR_H



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

#endif /* SYS_ERROR_H */ 