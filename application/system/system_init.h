/**
 * @file system_init.h
 * @brief 系统初始化协调器
 * @version 1.0.0
 * @date 2025-12-16
 *
 * 设计说明:
 * - 基于TMOS事件系统的统一初始化协调
 * - 按HAL → Driver → Middleware → Application的层级顺序初始化
 * - 遵循_setup → _init → _task的生命周期模式
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*==========================================
 * 系统初始化阶段定义
 * =========================================*/

/**
 * @brief 早期启动阶段 - 在_init之前运行
 * 主要任务:
 * - HAL层基础初始化 (GPIO、Platform抽象)
 * - 系统核心驱动初始化 (Timer、基础驱动)
 */
void system_setup_hal(void);
void system_setup_drivers(void);
void system_setup_middleware(void);
void system_setup_application(void);

/**
 * @brief 初始化阶段 - 在主机协议、调试和MCU外设初始化后运行
 * 主要任务:
 * - 设备驱动初始化 (Matrix、Storage、Battery、Indicator)
 * - 中间件初始化 (Keyboard、Communication、Transport)
 * - 应用服务初始化 (Input/Output/Communication/Storage服务)
 */
void system_init_hal(void);
void system_init_drivers(void);
void system_init_middleware(void);
void system_init_application(void);

/**
 * @brief 系统初始化协调器主函数
 * @return 错误码 (0表示成功)
 */
uint32_t system_init_coordinator(void);

/**
 * @brief 检查系统是否完全初始化
 * @return true表示已完全初始化，false表示未完成
 */
bool system_is_initialized(void);

/**
 * @brief 获取系统初始化状态
 * @return 当前初始化阶段 (0-7)
 */
uint8_t system_get_init_status(void);

#ifdef __cplusplus
}
#endif
