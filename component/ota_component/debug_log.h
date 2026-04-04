/**
 * @file debug_log.h
 * @brief OTA 组件日志适配层
 *
 * 将 OTA 组件的日志宏适配到 CH584 平台的日志系统
 */

#pragma once

#include "log.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 调试日志 - 使用 PRINT 宏输出 */
#ifndef DEBUG_log
#define DEBUG_log(fmt, ...)     PRINT(fmt "\r\n", ##__VA_ARGS__)
#endif

/* 调试日志 - 字符串版本 */
#ifndef DEBUG_logs
#define DEBUG_logs(fmt, ...)    PRINT(fmt, ##__VA_ARGS__)
#endif

/* 错误日志 */
#ifndef ERROR_log
#define ERROR_log(fmt, ...)     PRINT("[ERROR] " fmt "\r\n", ##__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif
