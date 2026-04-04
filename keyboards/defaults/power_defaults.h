#pragma once

/**
 * @file power_defaults.h
 * @brief 电源管理默认配置
 * @note 产品可通过在 config_product.h 中 #define 覆盖默认值
 */

/* ========== 休眠时间 ========== */
#ifndef POWER_DEEP_SLEEP_MS
#define POWER_DEEP_SLEEP_MS  (10 * 60 * 1000)  // 10分钟
#endif

#ifndef POWER_IDLE_SLEEP_MS
#define POWER_IDLE_SLEEP_MS (5 * 60 * 1000)   // 5分钟
#endif

/* ========== 低电量阈值 ========== */
#ifndef POWER_LOW_BATTERY_PERCENT
#define POWER_LOW_BATTERY_PERCENT  20
#endif

#ifndef POWER_CRITICAL_BATTERY_PERCENT
#define POWER_CRITICAL_BATTERY_PERCENT  5
#endif

/* ========== 电池参数 ========== */
#ifndef BATTERY_CAPACITY_MAH
#define BATTERY_CAPACITY_MAH  500
#endif

/* ========== 充电检测 ========== */
#ifndef BATTERY_ADC_SAMPLES
#define BATTERY_ADC_SAMPLES   8
#endif

#ifndef BATTERY_CHECK_INTERVAL_MS
#define BATTERY_CHECK_INTERVAL_MS  60000
#endif

/* ========== 背光关闭超时 ========== */
#ifndef CONNECTED_BACKLIGHT_DISABLE_TIMEOUT
#define CONNECTED_BACKLIGHT_DISABLE_TIMEOUT  60  // 连接状态下背光关闭超时（秒）
#endif

#ifndef DISCONNECTED_BACKLIGHT_DISABLE_TIMEOUT
#define DISCONNECTED_BACKLIGHT_DISABLE_TIMEOUT  30  // 断开状态下背光关闭超时（秒）
#endif

/* ========== 兼容旧命名 ========== */
#ifndef CONNECTED_IDLE_TIME
#define CONNECTED_IDLE_TIME  (POWER_DEEP_SLEEP_MS / 1000)
#endif
