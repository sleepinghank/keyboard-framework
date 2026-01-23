/**
 * @file hw_timer.h
 * @brief 通用硬件定时器 HAL 接口 (简化版)
 * @version 2.0.0
 *
 * 设计说明:
 * - 提供跨平台的硬件定时器抽象接口
 * - 支持 CH584、PAR2860、nRF52 等平台
 * - 时间单位为毫秒 (ms)
 * - 仅支持周期模式，长时间定时请使用 OSAL 软件定时器
 * - 硬件限制: 最大定时周期约 860ms (78MHz 时钟)
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../application/sys_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/*==========================================
 * 常量定义
 *=========================================*/

// 硬件定时器数量
#define HW_TIMER_COUNT          3

// 无效定时器 ID
#define HW_TIMER_INVALID_ID     0xFF

// 硬件定时器最大定时周期 (毫秒)
// 78MHz 时钟，26位计数器: 67108864 / 78000 ≈ 860ms
#define HW_TIMER_MAX_MS         860

/*==========================================
 * 类型定义
 *=========================================*/

// 硬件定时器 ID 枚举
typedef enum {
    HW_TIMER_0 = 0,
    HW_TIMER_1 = 1,
    HW_TIMER_2 = 2,
    HW_TIMER_MAX
} hw_timer_id_t;

// 定时器回调函数类型
typedef void (*hw_timer_callback_t)(void);

// 定时器状态枚举
typedef enum {
    HW_TIMER_STATE_IDLE    = 0,  // 空闲/未初始化
    HW_TIMER_STATE_RUNNING = 1   // 运行中
} hw_timer_state_t;

/*==========================================
 * 初始化接口
 *=========================================*/

/**
 * @brief 初始化硬件定时器模块
 * @return error_code_t 错误码
 */
error_code_t hw_timer_init(void);

/**
 * @brief 反初始化硬件定时器模块
 * @return error_code_t 错误码
 */
error_code_t hw_timer_deinit(void);

/*==========================================
 * 主要接口
 *=========================================*/

/**
 * @brief 启动周期定时器
 * @param timer_id 定时器 ID
 * @param interval_ms 定时间隔 (毫秒, 最大 860ms)
 * @param callback 回调函数
 * @return error_code_t 错误码
 * @note 超过 860ms 的定时请使用 OSAL 软件定时器
 */
error_code_t hw_timer_start(hw_timer_id_t timer_id, uint32_t interval_ms, hw_timer_callback_t callback);

/**
 * @brief 停止定时器
 * @param timer_id 定时器 ID
 * @return error_code_t 错误码
 */
error_code_t hw_timer_stop(hw_timer_id_t timer_id);

/*==========================================
 * 动态配置接口
 *=========================================*/

/**
 * @brief 修改定时间隔
 * @param timer_id 定时器 ID
 * @param interval_ms 定时间隔 (毫秒, 最大 860ms)
 * @return error_code_t 错误码
 */
error_code_t hw_timer_set_interval(hw_timer_id_t timer_id, uint32_t interval_ms);

/**
 * @brief 修改回调函数
 * @param timer_id 定时器 ID
 * @param callback 回调函数
 * @return error_code_t 错误码
 */
error_code_t hw_timer_set_callback(hw_timer_id_t timer_id, hw_timer_callback_t callback);

/*==========================================
 * 状态查询接口
 *=========================================*/

/**
 * @brief 获取定时器状态
 * @param timer_id 定时器 ID
 * @return hw_timer_state_t 定时器状态
 */
hw_timer_state_t hw_timer_get_state(hw_timer_id_t timer_id);

/**
 * @brief 检查定时器是否正在运行
 * @param timer_id 定时器 ID
 * @return true 正在运行, false 未运行
 */
bool hw_timer_is_running(hw_timer_id_t timer_id);

#ifdef __cplusplus
}
#endif
