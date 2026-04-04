#pragma once



#ifndef DEVICE_TYPE
#define DEVICE_TYPE      MulKeyboardTouchpad
#endif

#ifndef DEFAULT_SYSTEM
#define DEFAULT_SYSTEM      IOS
#endif


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

/**
 * @brief 当前设备配置的 HID 国家代码
 *
 * 通过编译时 KEYBOARD_COUNTRY 宏配置。
 * 如果未定义 KEYBOARD_COUNTRY，则默认为 COUNTRY_NOT_SUPPORTED。
 */
#ifdef KEYBOARD_COUNTRY
    #define HID_COUNTRY_CODE KEYBOARD_COUNTRY
#else
    #define HID_COUNTRY_CODE COUNTRY_NOT_SUPPORTED
#endif

/**
 * @brief 国家代码识别宏（可用于 #if 预处理指令）
 *
 * 根据 KEYBOARD_COUNTRY 定义返回 0 或 1 的常量值。
 * 这些宏可以在编译时条件判断中使用。
 */
#if defined(KEYBOARD_COUNTRY) && (KEYBOARD_COUNTRY == COUNTRY_GERMAN)
    #define IS_COUNTRY_DE    1
    #define IS_COUNTRY_UK    0
    #define IS_COUNTRY_FR    0
    #define IS_COUNTRY_JP    0
    #define IS_COUNTRY_US    0
#elif defined(KEYBOARD_COUNTRY) && (KEYBOARD_COUNTRY == COUNTRY_UK)
    #define IS_COUNTRY_DE    0
    #define IS_COUNTRY_UK    1
    #define IS_COUNTRY_FR    0
    #define IS_COUNTRY_JP    0
    #define IS_COUNTRY_US    0
#elif defined(KEYBOARD_COUNTRY) && (KEYBOARD_COUNTRY == COUNTRY_FRENCH)
    #define IS_COUNTRY_DE    0
    #define IS_COUNTRY_UK    0
    #define IS_COUNTRY_FR    1
    #define IS_COUNTRY_JP    0
    #define IS_COUNTRY_US    0
#elif defined(KEYBOARD_COUNTRY) && (KEYBOARD_COUNTRY == COUNTRY_JAPANESE)
    #define IS_COUNTRY_DE    0
    #define IS_COUNTRY_UK    0
    #define IS_COUNTRY_FR    0
    #define IS_COUNTRY_JP    1
    #define IS_COUNTRY_US    0
#elif defined(KEYBOARD_COUNTRY) && (KEYBOARD_COUNTRY == COUNTRY_US)
    #define IS_COUNTRY_DE    0
    #define IS_COUNTRY_UK    0
    #define IS_COUNTRY_FR    0
    #define IS_COUNTRY_JP    0
    #define IS_COUNTRY_US    1
#else
    /* 默认：未配置或不支持的国家 */
    #define IS_COUNTRY_DE    0
    #define IS_COUNTRY_UK    0
    #define IS_COUNTRY_FR    0
    #define IS_COUNTRY_JP    0
    #define IS_COUNTRY_US    0
#endif

/**
 * @brief 国家代码字符串标识
 *
 * 用于调试和日志输出，根据当前国家配置返回对应的字符串。
 */
#if IS_COUNTRY_DE
    #define COUNTRY_CODE_STRING "DE"
#elif IS_COUNTRY_UK
    #define COUNTRY_CODE_STRING "UK"
#elif IS_COUNTRY_FR
    #define COUNTRY_CODE_STRING "FR"
#elif IS_COUNTRY_JP
    #define COUNTRY_CODE_STRING "JP"
#elif IS_COUNTRY_US
    #define COUNTRY_CODE_STRING "US"
#else
    #define COUNTRY_CODE_STRING "N/A"
#endif

