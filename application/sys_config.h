#pragma once


#ifdef __cplusplus
extern "C" {
#endif
#include "sys_error.h"
#include <stdint.h>

//定义芯片型号
typedef enum {
    CHIP_CH584M = 0,
    CHIP_PAR2860 = 1,
} chip_type_t;

//定义打印等级
typedef enum {
    PRINTF_LEVEL_ASSERT = 0,//断言
    PRINTF_LEVEL_DEBUG = 1,//调试信息
    PRINTF_LEVEL_INFO = 2,//普通信息
    PRINTF_LEVEL_WARN = 3,//警告信息
    PRINTF_LEVEL_ERROR = 4,//错误信息
    PRINTF_LEVEL_VERBOSE = 5//详细信息
} printf_level_t;

/*---------------------------------------------------------用户配置------------------------------------------------------------ */

/*---------------------------------------------------------芯片型号------------------------------------------------------------ */
//## 芯片型号
#define CHIP_TYPE    CHIP_CH584M
//## END

/*---------------------------------------------------------log打印------------------------------------------------------------ */
//## 是否启用打印
#define PRINTF_ENABLE    TRUE
//## END

#define INDICATOR_TEST_ENABLE

#if(PRINTF_ENABLE == TRUE)

//##定义打印等级
#define PRINTF_LEVEL    PRINTF_LEVEL_DEBUG
//## END

//## 打印串口
#define PRINTF_UART    PLATFORM_UART_3
//## END

//## 打印波特率
#define PRINTF_BAUDRATE  115200
//## END

//## 打印IO映射
#define PRINTF_IO_MAP    PLATFORM_UART_IO_MAP_ENABLE
//## END

//## 日志颜色
#define LOG_COLOR_ENABLE   TRUE//使能日志颜色
//## END

//## 日志格式
#define LOG_FMT_USING_DIR_ENABLE    TRUE // 使用文件名
#define LOG_FMT_USING_LINE_ENABLE   TRUE // 使用行号
#define LOG_FMT_USING_FUNC_ENABLE   FALSE // 使用函数名
//## END
/* 每行日志的缓冲区大小 */
#define LOG_LINE_BUF_SIZE                       1024
/* 输出换行符标志 */
#define LOG_NEWLINE_SIGN                        "\r\n"
/* 输出行号的最大长度 */
#define LOG_LINE_NUM_MAX_LEN                    4
/* 输出过滤器标签的最大长度 */
#define LOG_FILTER_TAG_MAX_LEN                  20

#if(CHIP_TYPE == CHIP_CH584M)
#ifndef __HIGH_CODE
#define __HIGH_CODE   __attribute__((section(".highcode")))
#endif
#else
#define __HIGH_CODE
#endif

#endif

/*---------------------------------------------------------USB配置------------------------------------------------------------ */
//## 是否启用USB
#define USB_ENABLE    TRUE
//## END

/*---------------------------------------------------------蓝牙配置------------------------------------------------------------ */
//## 是否启用蓝牙
#define BLE_ENABLE    TRUE
//## END

#if(BLE_ENABLE == TRUE)

//## 蓝牙名称
#define  BLE_NAME  "CH584M_BLE"
//## END

//## 蓝牙广播间隔
#define BLE_ADVERTISE_INTERVAL   1000
//## END

#endif

#ifdef __cplusplus
}
#endif
