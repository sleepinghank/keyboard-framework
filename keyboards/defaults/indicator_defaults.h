#pragma once

/**
 * @file indicator_defaults.h
 * @brief 指示灯模块默认配置
 * @note 产品可通过在 config.h 中 #define 覆盖默认值
 */

/* ========== 指示灯数量 ========== */
#ifndef IND_LED_COUNT
#define IND_LED_COUNT        2
#endif

/* ========== LED 极性 ========== */
#ifndef IND_LED_ACTIVE_HIGH
#define IND_LED_ACTIVE_HIGH  true
#endif

/* ========== 灯效时间参数 ========== */
#ifndef IND_BLINK_SLOW_MS
#define IND_BLINK_SLOW_MS    1000
#endif

#ifndef IND_BLINK_FAST_MS
#define IND_BLINK_FAST_MS    200
#endif

#ifndef IND_BLINK_PAIRING_MS
#define IND_BLINK_PAIRING_MS 500
#endif

/* ========== 默认灯效 ========== */
#ifndef IND_CAPS_ON_EFFECT
#define IND_CAPS_ON_EFFECT   IND_ON
#endif

#ifndef IND_BT_CONNECTED_EFFECT
#define IND_BT_CONNECTED_EFFECT  IND_ON
#endif

#ifndef IND_LOW_BATTERY_EFFECT
#define IND_LOW_BATTERY_EFFECT   IND_BLINK_SLOW
#endif
