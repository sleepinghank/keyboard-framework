
#pragma once

#include <stdint.h>
#include "hw_timer.h"
#include "sys_error.h"

/*==========================================
 * 矩阵扫描定时器配置
 *=========================================*/

/* 矩阵扫描周期 (毫秒) - 可在 product_config.h 中覆盖 */
#ifndef MATRIX_SCAN_INTERVAL_MS
#define MATRIX_SCAN_INTERVAL_MS     5
#endif

/* 矩阵扫描使用的硬件定时器 ID */
#ifndef MATRIX_SCAN_TIMER_ID
#define MATRIX_SCAN_TIMER_ID        HW_TIMER_0
#endif

/*==========================================
 * 输入服务事件定义
 *=========================================*/

/** 所有输入相关服务事件
 * 矩阵扫描
 * 矩阵数据变化
 * 触控中断
 * 电量变化
 */
typedef enum {
    INPUT_MATRIX_SCAN_EVT     = 1 << 0,  /**< 矩阵扫描事件 */
    INPUT_MATRIX_CHANGED_EVT  = 1 << 1,  /**< 矩阵数据变化事件 */
    INPUT_TOUCH_INT_EVT       = 1 << 2,  /**< 触控中断事件 */
    INPUT_BATTERY_DETE_EVT    = 1 << 3,  /**< 电量变化事件 */
} input_task_event_t;

/*==========================================
 * 矩阵扫描定时器接口
 *=========================================*/

/**
 * @brief 启动矩阵扫描定时器
 * @return error_code_t 错误码
 */
error_code_t matrix_scan_timer_start(void);

/**
 * @brief 停止矩阵扫描定时器
 * @return error_code_t 错误码
 */
error_code_t matrix_scan_timer_stop(void);

/*==========================================
 * 输入服务接口
 *=========================================*/

/**
 * @brief 输入服务初始化
 */
void input_service_init(void);