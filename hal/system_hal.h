/**
 * @file system_hal.h
 * @brief 系统硬件抽象层接口
 * @version 1.0
 *
 * 提供跨平台的系统级接口：初始化、关机、休眠、复位等
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 系统错误码 */
typedef enum {
    SYSTEM_OK = 0,                      /* 操作成功 */
    SYSTEM_ERROR_INVALID_PARAM,         /* 无效参数 */
    SYSTEM_ERROR_NOT_SUPPORTED,         /* 操作不支持 */
    SYSTEM_ERROR_TIMEOUT,               /* 操作超时 */
    SYSTEM_ERROR_NOT_INITIALIZED        /* 未初始化 */
} system_result_t;

/* 系统电源模式 */
typedef enum {
    SYSTEM_POWER_MODE_RUN = 0,          /* 正常运行模式 */
    SYSTEM_POWER_MODE_IDLE,             /* 空闲模式，CPU暂停，外设运行 */
    SYSTEM_POWER_MODE_SLEEP,            /* 睡眠模式，低功耗，快速唤醒 */
    SYSTEM_POWER_MODE_DEEP_SLEEP,       /* 深度睡眠，更低功耗，唤醒较慢 */
    SYSTEM_POWER_MODE_SHUTDOWN          /* 关机模式，最低功耗，RAM丢失 */
} system_power_mode_t;

/* 系统唤醒源(可组合使用) */
typedef enum {
    SYSTEM_WAKEUP_NONE     = 0x0000,    /* 无唤醒源 */
    SYSTEM_WAKEUP_GPIO     = 0x0001,    /* GPIO引脚唤醒 */
    SYSTEM_WAKEUP_RTC      = 0x0002,    /* RTC定时唤醒 */
    SYSTEM_WAKEUP_TIMER    = 0x0004,    /* 定时器唤醒 */
    SYSTEM_WAKEUP_USB      = 0x0010,    /* USB插入唤醒 */
    SYSTEM_WAKEUP_BLE      = 0x0020,    /* 蓝牙事件唤醒 */
    SYSTEM_WAKEUP_KEYBOARD = 0x0080,    /* 键盘按键唤醒 */
    SYSTEM_WAKEUP_ALL      = 0xFFFF     /* 所有唤醒源 */
} system_wakeup_source_t;

/*==========================================
 * 核心接口
 * =========================================*/

/**
 * @brief 系统HAL层初始化
 * @note 在系统启动时调用，初始化基础硬件资源
 * @return SYSTEM_OK 成功，其他值表示错误
 */
system_result_t system_hal_init(void);

/**
 * @brief 初始化所有GPIO为默认安全状态
 * @note 将所有未使用的GPIO设置为输入上拉模式，防止悬空漏电
 *       保留引脚(USB、复位等)不会被初始化
 * @return SYSTEM_OK 成功，其他值表示错误
 */
system_result_t system_hal_gpio_init_all(void);

/**
 * @brief 系统关机
 * @note 进入最低功耗模式，RAM内容会丢失，需要外部事件唤醒
 * @return 正常情况不会返回，返回则表示关机失败
 */
system_result_t system_hal_shutdown(void);

/**
 * @brief 进入睡眠模式
 * @param mode 目标睡眠模式
 * @param wakeup_sources 唤醒源掩码，可使用 | 组合多个唤醒源
 * @return SYSTEM_OK 唤醒后返回成功，其他值表示错误
 * @example system_hal_enter_sleep(SYSTEM_POWER_MODE_SLEEP,
 *                                 SYSTEM_WAKEUP_GPIO | SYSTEM_WAKEUP_KEYBOARD);
 */
system_result_t system_hal_enter_sleep(system_power_mode_t mode, uint32_t wakeup_sources);

/**
 * @brief 系统软件复位
 * @note 执行软件复位，系统会重新启动，此函数不会返回
 */
void system_hal_reset(void);


/**
 * @brief 禁用全局中断
 * @return 返回当前中断状态，用于后续恢复
 */
uint32_t system_hal_disable_irq(void);

/**
 * @brief 恢复全局中断状态
 * @param state 之前 system_hal_disable_irq 返回的状态值
 */
void system_hal_restore_irq(uint32_t state);

#ifdef __cplusplus
}
#endif
