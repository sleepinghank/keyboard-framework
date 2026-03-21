
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
 * 触控板看门狗检查
 * LPM prepare/resume
 */
typedef enum {
    // INPUT_MATRIX_SCAN_EVT  = 1 << 0,  // 移除：改用 main loop 标志位
    INPUT_MATRIX_CHANGED_EVT  = 1 << 1,  /**< 矩阵数据变化事件 */
    INPUT_BATTERY_DETE_EVT    = 1 << 3,  /**< 电量变化事件 */
    INPUT_LPM_PREPARE_EVT     = 1 << 4,  /**< LPM prepare（Idle/Deep，mode 通过 lpm_get_mode() 查询） */
    INPUT_LPM_RESUME_EVT      = 1 << 5,  /**< LPM resume（Idle/Deep 唤醒后恢复） */
} input_task_event_t;

/*==========================================
 * 唤醒原因枚举
 *=========================================*/

typedef enum {
    LPM_WAKEUP_NONE    = 0,
    LPM_WAKEUP_MATRIX  = 1,    /* 矩阵按键触发 */
    LPM_WAKEUP_PA2     = 2,    /* 电源键 PA2 触发 */
    LPM_WAKEUP_TOUCH   = 3,    /* 触控板 INT 触发 */
    LPM_WAKEUP_RTC     = 4,    /* RTC 定时触发 */
} lpm_wakeup_source_t;

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

/*==========================================
 * 矩阵扫描标志位接口
 *=========================================*/

/**
 * @brief 检查是否需要执行矩阵扫描
 * @return true 需要执行, false 不需要
 * @note 非原子操作，仅供调试或非关键场景使用
 */
bool input_get_matrix_scan_flag(void);

/**
 * @brief 清除矩阵扫描标志位
 * @note 非原子操作，仅供调试或非关键场景使用
 */
void input_clear_matrix_scan_flag(void);

/*==========================================
 * 唤醒原因查询接口
 *=========================================*/

/**
 * @brief 获取最近一次唤醒原因
 * @return 唤醒源枚举值
 */
lpm_wakeup_source_t input_get_last_wakeup_source(void);

/* task ID 外部声明 */
extern uint8_t input_taskID;