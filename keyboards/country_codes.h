/**
 * @file country_codes.h
 * @brief USB HID 国家代码定义
 *
 * 基于 USB HID Usage Tables 规范定义国家代码枚举和宏。
 * 用于标识键盘的国家/地区布局。
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief USB HID 国家代码枚举
 *
 * 根据 USB HID Usage Tables 规范定义的国家代码。
 * 注意：仅包含本项目支持的常见国家代码。
 */
typedef enum {
    COUNTRY_NOT_SUPPORTED   = 0x00, /* 不支持的国家代码 */
    COUNTRY_ARABIC          = 0x01, /* 阿拉伯 */
    COUNTRY_BELGIAN         = 0x02, /* 比利时 */
    COUNTRY_CANADIAN_BI     = 0x03, /* 加拿大双语 */
    COUNTRY_CANADIAN_FRENCH = 0x04, /* 加拿大法语 */
    COUNTRY_CZECH_REPUBLIC  = 0x05, /* 捷克共和国 */
    COUNTRY_DANISH          = 0x06, /* 丹麦 */
    COUNTRY_FINNISH         = 0x07, /* 芬兰 */
    COUNTRY_FRENCH          = 0x09, /* 法国 */
    COUNTRY_GERMAN          = 0x0C, /* 德国 */
    COUNTRY_GREEK           = 0x0D, /* 希腊 */
    COUNTRY_HEBREW          = 0x0E, /* 希伯来 */
    COUNTRY_JAPANESE        = 0x0F, /* 日本 */
    COUNTRY_HUNGARIAN       = 0x10, /* 匈牙利 */
    COUNTRY_INTERNATIONAL   = 0x11, /* 国际 ISO */
    COUNTRY_ITALIAN         = 0x12, /* 意大利 */
    COUNTRY_NETHERLANDS_DUTCH = 0x13, /* 荷兰 */
    COUNTRY_NORWEGIAN       = 0x14, /* 挪威 */
    COUNTRY_KOREAN          = 0x15, /* 韩国 */
    COUNTRY_PERSIAN_FARSI   = 0x16, /* 波斯法尔斯 */
    COUNTRY_POLAND          = 0x17, /* 波兰 */
    COUNTRY_UK              = 0x18, /* 英国 */
    COUNTRY_PORTUGUESE      = 0x19, /* 葡萄牙 */
    COUNTRY_RUSSIA          = 0x1B, /* 俄罗斯 */
    COUNTRY_SLOVAKIA        = 0x1C, /* 斯洛伐克 */
    COUNTRY_SPANISH         = 0x1D, /* 西班牙 */
    COUNTRY_SWEDISH         = 0x1E, /* 瑞典 */
    COUNTRY_SWISS_FRENCH    = 0x1F, /* 瑞士法语 */
    COUNTRY_SWISS_GERMAN    = 0x20, /* 瑞士德语 */
    COUNTRY_US              = 0x21, /* 美国 */
    COUNTRY_TAIWAN          = 0x24, /* 台湾 */
    COUNTRY_CHINESE_SIMPLIFIED = 0x25, /* 简体中文 */
    COUNTRY_CHINESE_TRADITIONAL = 0x26, /* 繁体中文 */
} hid_country_code_t;

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

#ifdef __cplusplus
}
#endif
