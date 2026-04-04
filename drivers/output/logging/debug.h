/*
Copyright 2011 Jun Wako <wakojun@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <stdbool.h>
#include "print.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

/* for backward compatibility */
#define debug_enable true//(debug_config.enable)
#define debug_matrix true//(debug_config.matrix)
#define debug_keyboard true//(debug_config.keyboard)
#define debug_mouse true//(debug_config.mouse)

/*
 * Debug print utils
 */
#if (PRINTF_ENABLE == TRUE)

#    define dprint(s)                   \
        do {                            \
            if (debug_enable) print(s); \
        } while (0)
#    define dprintln(s)                   \
        do {                              \
            if (debug_enable) println(s); \
        } while (0)
#    define dprintf(fmt, ...)                              \
        do {                                               \
            if (debug_enable) xprintf(fmt, ##__VA_ARGS__); \
        } while (0)
#    define dmsg(s) dprintf("%s at %d: %s\n", __FILE__, __LINE__, s)

/* Deprecated. DO NOT USE these anymore, use dprintf instead. */
#    define debug(s)                    \
        do {                            \
            if (debug_enable) print(s); \
        } while (0)
#    define debugln(s)                    \
        do {                              \
            if (debug_enable) println(s); \
        } while (0)
#    define debug_msg(s)             \
        do {                         \
            if (debug_enable) {      \
                print(__FILE__);     \
                print(" at ");       \
                print_dec(__LINE__); \
                print(" in ");       \
                print(": ");         \
                print(s);            \
            }                        \
        } while (0)
#    define debug_dec(data)                    \
        do {                                   \
            if (debug_enable) print_dec(data); \
        } while (0)
#    define debug_decs(data)                    \
        do {                                    \
            if (debug_enable) print_decs(data); \
        } while (0)
#    define debug_hex4(data)                    \
        do {                                    \
            if (debug_enable) print_hex4(data); \
        } while (0)
#    define debug_hex8(data)                    \
        do {                                    \
            if (debug_enable) print_hex8(data); \
        } while (0)
#    define debug_hex16(data)                    \
        do {                                     \
            if (debug_enable) print_hex16(data); \
        } while (0)
#    define debug_hex32(data)                    \
        do {                                     \
            if (debug_enable) print_hex32(data); \
        } while (0)
#    define debug_bin8(data)                    \
        do {                                    \
            if (debug_enable) print_bin8(data); \
        } while (0)
#    define debug_bin16(data)                    \
        do {                                     \
            if (debug_enable) print_bin16(data); \
        } while (0)
#    define debug_bin32(data)                    \
        do {                                     \
            if (debug_enable) print_bin32(data); \
        } while (0)
#    define debug_bin_reverse8(data)                    \
        do {                                            \
            if (debug_enable) print_bin_reverse8(data); \
        } while (0)
#    define debug_bin_reverse16(data)                    \
        do {                                             \
            if (debug_enable) print_bin_reverse16(data); \
        } while (0)
#    define debug_bin_reverse32(data)                    \
        do {                                             \
            if (debug_enable) print_bin_reverse32(data); \
        } while (0)
#    define debug_hex(data) debug_hex8(data)
#    define debug_bin(data) debug_bin8(data)
#    define debug_bin_reverse(data) debug_bin8(data)

#else

#    define dprint(s)
#    define dprintln(s)
#    define dprintf(fmt, ...)
#    define dmsg(s)
#    define debug(s)
#    define debugln(s)
#    define debug_msg(s)
#    define debug_dec(data)
#    define debug_decs(data)
#    define debug_hex4(data)
#    define debug_hex8(data)
#    define debug_hex16(data)
#    define debug_hex32(data)
#    define debug_bin8(data)
#    define debug_bin16(data)
#    define debug_bin32(data)
#    define debug_bin_reverse8(data)
#    define debug_bin_reverse16(data)
#    define debug_bin_reverse32(data)
#    define debug_hex(data)
#    define debug_bin(data)
#    define debug_bin_reverse(data)

#endif /* PRINTF_ENABLE */

/* === 统一日志宏 (LOG_E / LOG_W / LOG_I / LOG_D) === */
/* 日志级别定义，由 config_product.h 中的 LOG_LEVEL 选择 */
#ifndef LOG_LEVEL_NONE
#define LOG_LEVEL_NONE    0
#endif
#ifndef LOG_LEVEL_ERROR
#define LOG_LEVEL_ERROR   1
#endif
#ifndef LOG_LEVEL_WARN
#define LOG_LEVEL_WARN    2
#endif
#ifndef LOG_LEVEL_INFO
#define LOG_LEVEL_INFO    3
#endif
#ifndef LOG_LEVEL_DEBUG
#define LOG_LEVEL_DEBUG   4
#endif

/* 默认级别：未配置时仅输出 WARN 和 ERROR */
#ifndef LOG_LEVEL
#define LOG_LEVEL  LOG_LEVEL_WARN
#endif

#if (PRINTF_ENABLE == TRUE) && (LOG_LEVEL >= LOG_LEVEL_ERROR)
#define LOG_E(fmt, ...) xprintf("[E] " fmt LOG_NEWLINE_SIGN, ##__VA_ARGS__)
#else
#define LOG_E(fmt, ...)
#endif

#if (PRINTF_ENABLE == TRUE) && (LOG_LEVEL >= LOG_LEVEL_WARN)
#define LOG_W(fmt, ...) xprintf("[W] " fmt LOG_NEWLINE_SIGN, ##__VA_ARGS__)
#else
#define LOG_W(fmt, ...)
#endif

#if (PRINTF_ENABLE == TRUE) && (LOG_LEVEL >= LOG_LEVEL_INFO)
#define LOG_I(fmt, ...) xprintf("[I] " fmt LOG_NEWLINE_SIGN, ##__VA_ARGS__)
#else
#define LOG_I(fmt, ...)
#endif

#if (PRINTF_ENABLE == TRUE) && (LOG_LEVEL >= LOG_LEVEL_DEBUG)
#define LOG_D(fmt, ...) xprintf("[D] " fmt LOG_NEWLINE_SIGN, ##__VA_ARGS__)
#else
#define LOG_D(fmt, ...)
#endif


