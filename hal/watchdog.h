/**
 * @file watchdog.h
 * @brief 看门狗硬件抽象层接口
 * @version 1.0
 *
 * 提供跨平台的看门狗定时器接口，用于系统异常检测和自动复位
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化看门狗定时器
 * @param timeout_ms 超时时间（毫秒），建议5000ms
 * @note CH584看门狗计数器为8位，超时时间 = (256 - count) * T
 *       其中T取决于系统时钟，实际超时时间会有偏差
 */
void watchdog_init(uint32_t timeout_ms);

/**
 * @brief 启动看门狗定时器
 * @note 启动后必须定期调用 watchdog_feed() 喂狗，否则系统将复位
 */
void watchdog_start(void);

/**
 * @brief 停止看门狗定时器
 * @note 某些平台可能不支持停止看门狗，调用后应检查返回值
 */
void watchdog_stop(void);

/**
 * @brief 喂狗（重置看门狗计数器）
 * @note 应在主循环中定期调用，防止看门狗超时复位
 */
void watchdog_feed(void);

/**
 * @brief 检查上次复位是否由看门狗引起
 * @return true 上次复位由看门狗超时引起
 * @return false 上次复位非看门狗引起
 */
bool watchdog_caused_reset(void);

#ifdef __cplusplus
}
#endif
