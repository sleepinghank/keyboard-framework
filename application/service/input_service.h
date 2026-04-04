
#pragma once

#include <stdint.h>
#include "hw_timer.h"
#include "system_enums.h"

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

/* 电量检测周期 (ms) todo：区分正常工作状态的电量检测频率和idle状态的 */
#ifndef BATTERY_DETECT_INTERVAL
#define BATTERY_DETECT_INTERVAL  1600*10  /* 正常工作状态10秒检测一次 */
#define BATTERY_DETECT_INTERVAL_IDLE  1600*15  /* idle状态15秒检测一次 */
#endif

/* 触控板看门狗检查周期 (ms) */
// #ifndef TOUCH_WATCHDOG_INTERVAL
// #define TOUCH_WATCHDOG_INTERVAL  2000   /* 2秒检测一次 */
// #endif

/*==========================================
 * 输入服务事件定义
 *=========================================*/

/* 输入服务事件定义
 * bit 0: 移除（矩阵扫描改用 main loop 标志位）
 */
#define INPUT_MATRIX_CHANGED_EVT  (1 << 1)  /* 矩阵数据变化事件 */
#define INPUT_TOUCH_TOGGLE_EVT    (1 << 2)  /* 触控板锁定/解锁切换事件 */
#define INPUT_BATTERY_DETE_EVT    (1 << 3)  /* 电量变化事件 */
#define INPUT_BATTERY_DETE_EVT    (1 << 3)  /* 电量变化事件 */
#define INPUT_TOUCH_ON_EVT    (1 << 4)  /* 触控板开启事件 */
#define INPUT_TOUCH_OFF_EVT   (1 << 5)  /* 触控板关闭事件 */
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
 * 触控板中断标志位接口
 *=========================================*/

/**
 * @brief 获取触控板中断标志位
 * @return true 有中断待处理, false 无中断
 */
bool input_get_touchpad_int_flag(void);

/**
 * @brief 设置触控板中断标志位
 * @param val 标志位值
 */
void input_set_touchpad_int_flag(bool val);

/**
 * @brief 清除触控板中断标志位
 */
void input_clear_touchpad_int_flag(void);

/*==========================================
 * 唤醒原因查询接口
 *=========================================*/

/**
 * @brief 获取最近一次唤醒原因
 * @return 唤醒源枚举值
 */
lpm_wakeup_source_t input_get_last_wakeup_source(void);

/**
 * @brief 设置最近一次唤醒原因
 * @param source 唤醒源枚举值
 */
void input_set_last_wakeup_source(lpm_wakeup_source_t source);

void matrix_scan_timer_callback(void);

/* task ID 外部声明 */
extern uint8_t input_taskID;
