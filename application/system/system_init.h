/**
 * @file system_init.h
 * @brief 系统初始化协调器
 * @version 2.0.0
 * @date 2026-03-19
 *
 * 设计说明:
 * - 基于TMOS事件系统的统一初始化协调
 * - 按HAL → Driver → Middleware → Application的层级顺序初始化
 * - 简化的初始化流程: 仅_init阶段，删除空的_setup阶段
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*==========================================
 * 系统初始化状态枚举
 * =========================================*/

/**
 * @brief 系统初始化状态
 */
typedef enum {
    SYSTEM_INIT_STATUS_NOT_STARTED = 0,    // 未开始
    SYSTEM_INIT_STATUS_HAL,                // HAL init 完成
    SYSTEM_INIT_STATUS_DRIVER,             // Driver init 完成
    SYSTEM_INIT_STATUS_MIDDLEWARE,         // Middleware init 完成
    SYSTEM_INIT_STATUS_APPLICATION,        // Application init 完成
    SYSTEM_INIT_STATUS_TASK,               // 进入主循环
    SYSTEM_INIT_STATUS_COMPLETED           // 完全初始化完成
} system_init_status_t;

/*==========================================
 * 系统初始化函数声明
 * =========================================*/

/**
 * @brief 初始化阶段 - 按层级顺序执行
 * 主要任务:
 * - HAL层初始化 (UART、GPIO、硬件定时器)
 * - 设备驱动初始化 (Timer、Storage、Battery、Backlight、Indicator)
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

/**
 * @brief 选择默认传输通道
 *
 * 根据编译配置选择默认的传输通道，优先使用蓝牙通道。
 * set_transport() 内部会调用相应的驱动切换函数来初始化无线传输函数表。
 *
 * @note 如果编译时未启用任何传输通道，default_transport 将保持为 TRANSPORT_NONE，
 *       此时不会调用 set_transport()。
 */
void system_select_default_transport(void);

#ifdef __cplusplus
}
#endif
