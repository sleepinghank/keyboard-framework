#pragma once

/**
 * @file keyboard_defaults.h
 * @brief 键盘行为默认配置
 * @note 产品可通过在 config.h 中 #define 覆盖默认值
 */

/* ========== 按键行为参数 ========== */
#ifndef TAPPING_TERM
#define TAPPING_TERM         200
#endif

#ifndef COMBO_TERM
#define COMBO_TERM           50
#endif

#ifndef DEBOUNCE
#define DEBOUNCE             5
#endif

#ifndef ONESHOT_TIMEOUT
#define ONESHOT_TIMEOUT      300
#endif

/* ========== 层级配置 ========== */
#ifndef LAYER_COUNT
#define LAYER_COUNT          4
#endif

#ifndef DEFAULT_LAYER
#define DEFAULT_LAYER        0
#endif

/* ========== 鼠标配置 ========== */
#ifndef MOUSEKEY_DELAY
#define MOUSEKEY_DELAY       10
#endif

#ifndef MOUSEKEY_INTERVAL
#define MOUSEKEY_INTERVAL    20
#endif

#ifndef MOUSEKEY_MAX_SPEED
#define MOUSEKEY_MAX_SPEED   10
#endif

#ifndef MOUSEKEY_TIME_TO_MAX
#define MOUSEKEY_TIME_TO_MAX 30
#endif
