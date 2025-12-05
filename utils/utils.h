/**
 * @file utils.h
 * @brief 通用工具函数接口定义
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * 提供通用的工具函数，如数学运算、字符串处理、数据转换等
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 工具函数错误码定义 */
typedef enum {
    UTILS_OK = 0,                     /* 操作成功 */
    UTILS_ERROR_INVALID_PARAM,         /* 无效参数 */
    UTILS_ERROR_NOT_SUPPORTED,         /* 操作不支持 */
    UTILS_ERROR_BUFFER_TOO_SMALL,      /* 缓冲区太小 */
    UTILS_ERROR_BUFFER_OVERFLOW,       /* 缓冲区溢出 */
    UTILS_ERROR_OUT_OF_MEMORY,         /* 内存不足 */
    UTILS_ERROR_DIVISION_BY_ZERO,     /* 除零错误 */
    UTILS_ERROR_RANGE_VIOLATION,      /* 范围违规 */
    UTILS_ERROR_FORMAT_ERROR,          /* 格式错误 */
    UTILS_ERROR_PARSE_ERROR,           /* 解析错误 */
    UTILS_ERROR_CONVERSION_ERROR,      /* 转换错误 */
    UTILS_ERROR_TIMEOUT,               /* 超时错误 */
    UTILS_ERROR_CRC_FAILED,            /* CRC校验失败 */
    UTILS_ERROR_CHECKSUM_FAILED,       /* 校验和失败 */
    UTILS_ERROR_DATA_CORRUPTED        /* 数据损坏 */
} utils_result_t;

/* 数学函数工具 */
typedef struct {
    /**
     * @brief 限制数值在指定范围内
     * @param value 输入值
     * @param min_val 最小值
     * @param max_val 最大值
     * @return 限制后的值
     */
    int32_t (*clamp_int32)(int32_t value, int32_t min_val, int32_t max_val);

    /**
     * @brief 限制浮点数在指定范围内
     * @param value 输入值
     * @param min_val 最小值
     * @param max_val 最大值
     * @return 限制后的值
     */
    float (*clamp_float)(float value, float min_val, float max_val);

    /**
     * @brief 线性插值
     * @param start 起始值
     * @param end 结束值
     * @param factor 插值因子(0.0-1.0)
     * @return 插值结果
     */
    float (*lerp)(float start, float end, float factor);

    /**
     * @brief 反线性插值
     * @param start 起始值
     * @param end 结束值
     * @param value 当前值
     * @return 插值因子
     */
    float (*inverse_lerp)(float start, float end, float value);

    /**
     * @brief 平滑阶跃函数
     * @param edge 阈值
     * @param x 输入值
     * @return 阶跃结果(0.0-1.0)
     */
    float (*smoothstep)(float edge, float x);

    /**
     * @brief 映射数值范围
     * @param value 输入值
     * @param from_min 输入最小值
     * @param from_max 输入最大值
     * @param to_min 输出最小值
     * @param to_max 输出最大值
     * @return 映射后的值
     */
    float (*map_range)(float value, float from_min, float from_max,
                       float to_min, float to_max);

    /**
     * @brief 计算绝对值
     * @param value 输入值
     * @return 绝对值
     */
    int32_t (*abs_int32)(int32_t value);

    /**
     * @brief 计算浮点数绝对值
     * @param value 输入值
     * @return 绝对值
     */
    float (*abs_float)(float value);

    /**
     * @brief 计算最小值
     * @param a 值A
     * @param b 值B
     * @return 最小值
     */
    int32_t (*min_int32)(int32_t a, int32_t b);

    /**
     * @brief 计算最大值
     * @param a 值A
     * @param b 值B
     * @return 最大值
     */
    int32_t (*max_int32)(int32_t a, int32_t b);

    /**
     * @brief 计算浮点数最小值
     * @param a 值A
     * @param b 值B
     * @return 最小值
     */
    float (*min_float)(float a, float b);

    /**
     * @brief 计算浮点数最大值
     * @param a 值A
     * @param b 值B
     * @return 最大值
     */
    float (*max_float)(float a, float b);

    /**
     * @brief 计算数组的最大值
     * @param array 数组指针
     * @param length 数组长度
     * @return 最大值
     */
    int32_t (*max_array_int32)(const int32_t *array, size_t length);

    /**
     * @brief 计算数组的最小值
     * @param array 数组指针
     * @param length 数组长度
     * @return 最小值
     */
    int32_t (*min_array_int32)(const int32_t *array, size_t length);

    /**
     * @brief 计算数组的平均值
     * @param array 数组指针
     * @param length 数组长度
     * @return 平均值
     */
    int32_t (*average_array_int32)(const int32_t *array, size_t length);

    /**
     * @brief 计算浮点数数组的平均值
     * @param array 数组指针
     * @param length 数组长度
     * @return 平均值
     */
    float (*average_array_float)(const float *array, size_t length);

    /**
     * @brief 计算数组的和
     * @param array 数组指针
     * @param length 数组长度
     * @return 和
     */
    int32_t (*sum_array_int32)(const int32_t *array, size_t length);

    /**
     * @brief 计算浮点数数组的和
     * @param array 数组指针
     * @param length 数组长度
     * @return 和
     */
    float (*sum_array_float)(const float *array, size_t length);

    /**
     * @brief 快速平方根倒数
     * @param x 输入值
     * @return 平方根倒数
     */
    float (*fast_inv_sqrt)(float x);

    /**
     * @brief 快速平方根
     * @param x 输入值
     * @return 平方根
     */
    float (*fast_sqrt)(float x);

    /**
     * @brief 快速绝对值
     * @param x 输入值
     * @return 绝对值
     */
    float (*fast_abs)(float x);

    /**
     * @brief 快速取整
     * @param x 输入值
     * @return 整数部分
     */
    int32_t (*fast_floor)(float x);

    /**
     * @brief 快速向上取整
     * @param x 输入值
     * @return 向上取整结果
     */
    int32_t (*fast_ceil)(float x);

    /**
     * @brief 快速四舍五入
     * @param x 输入值
     * @return 四舍五入结果
     */
    int32_t (*fast_round)(float x);
} utils_math_t;

/* 字符串处理工具 */
typedef struct {
    /**
     * @brief 计算字符串长度
     * @param str 字符串指针
     * @return 字符串长度
     */
    size_t (*strlen)(const char *str);

    /**
     * @brief 字符串复制
     * @param dest 目标缓冲区
     * @param src 源字符串
     * @param max_size 最大大小
     * @return 复制结果
     */
    utils_result_t (*strcpy)(char *dest, const char *src, size_t max_size);

    /**
     * @brief 字符串连接
     * @param dest 目标缓冲区
     * @param src 源字符串
     * @param max_size 最大大小
     * @return 连接结果
     */
    utils_result_t (*strcat)(char *dest, const char *src, size_t max_size);

    /**
     * @brief 字符串比较
     * @param str1 字符串1
     * @param str2 字符串2
     * @param ignore_case 是否忽略大小写
     * @return 比较结果
     */
    int32_t (*strcmp)(const char *str1, const char *str2, bool ignore_case);

    /**
     * @brief 字符串搜索
     * @param str 源字符串
     * @param substr 子字符串
     * @param ignore_case 是否忽略大小写
     * @return 子字符串位置，未找到返回-1
     */
    int32_t (*strfind)(const char *str, const char *substr, bool ignore_case);

    /**
     * @brief 字符串分割
     * @param str 源字符串
     * @param delimiter 分隔符
     * @param tokens 输出令牌数组
     * @param max_tokens 最大令牌数
     * @return 实际令牌数
     */
    size_t (*strsplit)(const char *str, char delimiter, char **tokens, size_t max_tokens);

    /**
     * @brief 字符串转整数
     * @param str 字符串
     * @param value 输出整数值
     * @param base 进制(10或16)
     * @return 转换结果
     */
    utils_result_t (*atoi)(const char *str, int32_t *value, int32_t base);

    /**
     * @brief 字符串转浮点数
     * @param str 字符串
     * @param value 输出浮点数值
     * @return 转换结果
     */
    utils_result_t (*atof)(const char *str, float *value);

    /**
     * @brief 整数转字符串
     * @param value 整数值
     * @param str 输出字符串
     * @param max_size 最大大小
     * @param base 进制(10或16)
     * @return 转换结果
     */
    utils_result_t (*itoa)(int32_t value, char *str, size_t max_size, int32_t base);

    /**
     * @brief 浮点数转字符串
     * @param value 浮点数值
     * @param str 输出字符串
     * @param max_size 最大大小
     * @param decimal_places 小数位数
     * @return 转换结果
     */
    utils_result_t (*ftoa)(float value, char *str, size_t max_size, uint8_t decimal_places);

    /**
     * @brief 字符串是否为空
     * @param str 字符串
     * @return 是否为空
     */
    bool (*is_empty)(const char *str);

    /**
     * @brief 字符串是否为数字
     * @param str 字符串
     * @return 是否为数字
     */
    bool (*is_numeric)(const char *str);

    /**
     * @brief 字符串是否为字母
     * @param str 字符串
     * @return 是否为字母
     */
    bool (*is_alpha)(const char *str);

    /**
     * @brief 字符串是否为字母数字
     * @param str 字符串
     * @return 是否为字母数字
     */
    bool (*is_alnum)(const char *str);

    /**
     * @brief 字符串转大写
     * @param str 字符串
     * @param max_size 最大大小
     * @return 转换结果
     */
    utils_result_t (*to_upper)(char *str, size_t max_size);

    /**
     * @brief 字符串转小写
     * @param str 字符串
     * @param max_size 最大大小
     * @return 转换结果
     */
    utils_result_t (*to_lower)(char *str, size_t max_size);

    /**
     * @brief 字符串修剪空白字符
     * @param str 字符串
     * @param max_size 最大大小
     * @return 修剪结果
     */
    utils_result_t (*trim)(char *str, size_t max_size);

    /**
     * @brief 字符串左修剪
     * @param str 字符串
     * @param max_size 最大大小
     * @return 修剪结果
     */
    utils_result_t (*ltrim)(char *str, size_t max_size);

    /**
     * @brief 字符串右修剪
     * @param str 字符串
     * @param max_size 最大大小
     * @return 修剪结果
     */
    utils_result_t (*rtrim)(char *str, size_t max_size);

    /**
     * @brief 字符串替换
     * @param str 源字符串
     * @param old_str 旧字符串
     * @param new_str 新字符串
     * @param result 输出结果字符串
     * @param max_size 最大大小
     * @return 替换结果
     */
    utils_result_t (*replace)(const char *str, const char *old_str, const char *new_str,
                              char *result, size_t max_size);

    /**
     * @brief 字符串格式化
     * @param buffer 输出缓冲区
     * @param size 缓冲区大小
     * @param format 格式字符串
     * @param ... 可变参数
     * @return 格式化结果
     */
    int32_t (*sprintf)(char *buffer, size_t size, const char *format, ...);

    /**
     * @brief 安全的字符串复制
     * @param dest 目标缓冲区
     * @param src 源字符串
     * @param dest_size 目标缓冲区大小
     * @return 复制结果
     */
    utils_result_t (*safe_strcpy)(char *dest, const char *src, size_t dest_size);

    /**
     * @brief 安全的字符串连接
     * @param dest 目标缓冲区
     * @param src 源字符串
     * @param dest_size 目标缓冲区大小
     * @return 连接结果
     */
    utils_result_t (*safe_strcat)(char *dest, const char *src, size_t dest_size);

    /**
     * @brief 安全的字符串格式化
     * @param buffer 输出缓冲区
     * @param buffer_size 缓冲区大小
     * @param format 格式字符串
     * @param ... 可变参数
     * @return 格式化结果
     */
    utils_result_t (*safe_sprintf)(char *buffer, size_t buffer_size, const char *format, ...);
} utils_string_t;

/* 数据转换工具 */
typedef struct {
    /**
     * @brief 字节序转换 - 16位
     * @param value 输入值
     * @return 转换后的值
     */
    uint16_t (*swap16)(uint16_t value);

    /**
     * @brief 字节序转换 - 32位
     * @param value 输入值
     * @return 转换后的值
     */
    uint32_t (*swap32)(uint32_t value);

    /**
     * @brief 字节序转换 - 64位
     * @param value 输入值
     * @return 转换后的值
     */
    uint64_t (*swap64)(uint64_t value);

    /**
     * @brief 主机字节序转网络字节序 - 16位
     * @param value 输入值
     * @return 转换后的值
     */
    uint16_t (*htons)(uint16_t value);

    /**
     * @brief 主机字节序转网络字节序 - 32位
     * @param value 输入值
     * @return 转换后的值
     */
    uint32_t (*htonl)(uint32_t value);

    /**
     * @brief 网络字节序转主机字节序 - 16位
     * @param value 输入值
     * @return 转换后的值
     */
    uint16_t (*ntohs)(uint16_t value);

    /**
     * @brief 网络字节序转主机字节序 - 32位
     * @param value 输入值
     * @return 转换后的值
     */
    uint32_t (*ntohl)(uint32_t value);

    /**
     * @brief BCD转二进制
     * @param bcd_value BCD值
     * @return 二进制值
     */
    uint8_t (*bcd_to_bin)(uint8_t bcd_value);

    /**
     * @brief 二进制转BCD
     * @param bin_value 二进制值
     * @return BCD值
     */
    uint8_t (*bin_to_bcd)(uint8_t bin_value);

    /**
     * @brief 十六进制字符串转字节数组
     * @param hex_str 十六进制字符串
     * @param bytes 输出字节数组
     * @param max_bytes 最大字节数
     * @return 转换结果
     */
    utils_result_t (*hex_to_bytes)(const char *hex_str, uint8_t *bytes, size_t max_bytes);

    /**
     * @brief 字节数组转十六进制字符串
     * @param bytes 字节数组
     * @param length 字节长度
     * @param hex_str 输出十六进制字符串
     * @param max_length 最大字符串长度
     * @return 转换结果
     */
    utils_result_t (*bytes_to_hex)(const uint8_t *bytes, size_t length,
                                 char *hex_str, size_t max_length);

    /**
     * @brief Base64编码
     * @param data 输入数据
     * @param data_length 数据长度
     * @param encoded_data 输出编码数据
     * @param max_encoded_length 最大编码长度
     * @param encoded_length 输出编码长度
     * @return 编码结果
     */
    utils_result_t (*base64_encode)(const uint8_t *data, size_t data_length,
                                  char *encoded_data, size_t max_encoded_length,
                                  size_t *encoded_length);

    /**
     * @brief Base64解码
     * @param encoded_data 编码数据
     * @param encoded_length 编码长度
     * @param decoded_data 输出解码数据
     * @param max_decoded_length 最大解码长度
     * @param decoded_length 输出解码长度
     * @return 解码结果
     */
    utils_result_t (*base64_decode)(const char *encoded_data, size_t encoded_length,
                                  uint8_t *decoded_data, size_t max_decoded_length,
                                  size_t *decoded_length);

    /**
     * @brief URL编码
     * @param str 输入字符串
     * @param encoded_str 输出编码字符串
     * @param max_length 最大长度
     * @return 编码结果
     */
    utils_result_t (*url_encode)(const char *str, char *encoded_str, size_t max_length);

    /**
     * @brief URL解码
     * @param encoded_str 编码字符串
     * @param str 输出解码字符串
     * @param max_length 最大长度
     * @return 解码结果
     */
    utils_result_t (*url_decode)(const char *encoded_str, char *str, size_t max_length);

    /**
     * @brief 整数转ASCII字符串
     * @param value 整数值
     * @param str 输出字符串
     * @param max_length 最大长度
     * @return 转换结果
     */
    utils_result_t (*int_to_ascii)(int32_t value, char *str, size_t max_length);

    /**
     * @brief ASCII字符串转整数
     * @param str 输入字符串
     * @param value 输出整数值
     * @return 转换结果
     */
    utils_result_t (*ascii_to_int)(const char *str, int32_t *value);

    /**
     * @brief Unicode转UTF-8
     * @param unicode Unicode码点
     * @param utf8 输出UTF-8字符串
     * @param max_length 最大长度
     * @return 转换结果
     */
    utils_result_t (*unicode_to_utf8)(uint32_t unicode, char *utf8, size_t max_length);

    /**
     * @brief UTF-8转Unicode
     * @param utf8 UTF-8字符串
     * @param unicode 输出Unicode码点
     * @return 转换结果
     */
    utils_result_t (*utf8_to_unicode)(const char *utf8, uint32_t *unicode);

    /**
     * @brief 颜色转换：RGB转HSV
     * @param r 红色分量
     * @param g 绿色分量
     * @param b 蓝色分量
     * @param h 输出色相
     * @param s 输出饱和度
     * @param v 输出明度
     */
    void (*rgb_to_hsv)(uint8_t r, uint8_t g, uint8_t b, float *h, float *s, float *v);

    /**
     * @brief 颜色转换：HSV转RGB
     * @param h 色相
     * @param s 饱和度
     * @param v 明度
     * @param r 输出红色分量
     * @param g 输出绿色分量
     * @param b 输出蓝色分量
     */
    void (*hsv_to_rgb)(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b);
} utils_convert_t;

/* 数据验证工具 */
typedef struct {
    /**
     * @brief 验证整数范围
     * @param value 值
     * @param min_val 最小值
     * @param max_val 最大值
     * @return 是否有效
     */
    bool (*validate_int_range)(int32_t value, int32_t min_val, int32_t max_val);

    /**
     * @brief 验证浮点数范围
     * @param value 值
     * @param min_val 最小值
     * @param max_val 最大值
     * @return 是否有效
     */
    bool (*validate_float_range)(float value, float min_val, float max_val);

    /**
     * @brief 验证字符串长度
     * @param str 字符串
     * @param min_length 最小长度
     * @param max_length 最大长度
     * @return 是否有效
     */
    bool (*validate_string_length)(const char *str, size_t min_length, size_t max_length);

    /**
     * @brief 验证IP地址
     * @param ip_str IP地址字符串
     * @return 是否有效
     */
    bool (*validate_ip_address)(const char *ip_str);

    /**
     * @brief 验证MAC地址
     * @param mac_str MAC地址字符串
     * @return 是否有效
     */
    bool (*validate_mac_address)(const char *mac_str);

    /**
     * @brief 验证邮箱地址
     * @param email_str 邮箱地址字符串
     * @return 是否有效
     */
    bool (*validate_email)(const char *email_str);

    /**
     * @brief 验证URL
     * @param url_str URL字符串
     * @return 是否有效
     */
    bool (*validate_url)(const char *url_str);

    /**
     * @brief 验证日期时间
     * @param datetime_str 日期时间字符串
     * @return 是否有效
     */
    bool (*validate_datetime)(const char *datetime_str);

    /**
     * @brief 验证校验和
     * @param data 数据
     * @param length 数据长度
     * @param expected_checksum 期望校验和
     * @return 是否有效
     */
    bool (*validate_checksum)(const uint8_t *data, size_t length, uint32_t expected_checksum);

    /**
     * @brief 验证CRC
     * @param data 数据
     * @param length 数据长度
     * @param expected_crc 期望CRC值
     * @return 是否有效
     */
    bool (*validate_crc)(const uint8_t *data, size_t length, uint32_t expected_crc);

    /**
     * @brief 验证数据完整性
     * @param data 数据
     * @param length 数据长度
     * @param hash_type 哈希类型
     * @param expected_hash 期望哈希值
     * @return 是否有效
     */
    bool (*validate_integrity)(const uint8_t *data, size_t length, uint32_t hash_type,
                               const uint8_t *expected_hash);

    /**
     * @brief 验证数据模式
     * @param data 数据
     * @param length 数据长度
     * @param pattern 模式
     * @param pattern_length 模式长度
     * @return 是否有效
     */
    bool (*validate_pattern)(const uint8_t *data, size_t length,
                           const uint8_t *pattern, size_t pattern_length);

    /**
     * @brief 验证数据奇偶性
     * @param data 数据
     * @param length 数据长度
     * @param parity 奇偶性(0=偶,1=奇)
     * @return 是否有效
     */
    bool (*validate_parity)(const uint8_t *data, size_t length, uint8_t parity);
} utils_validate_t;

/* 加密解密工具 */
typedef struct {
    /**
     * @brief 简单XOR加密
     * @param data 输入数据
     * @param length 数据长度
     * @param key 密钥
     * @param key_length 密钥长度
     * @param output 输出数据
     * @return 加密结果
     */
    utils_result_t (*xor_encrypt)(const uint8_t *data, size_t length,
                                 const uint8_t *key, size_t key_length,
                                 uint8_t *output);

    /**
     * @brief 简单XOR解密
     * @param data 输入数据
     * @param length 数据长度
     * @param key 密钥
     * @param key_length 密钥长度
     * @param output 输出数据
     * @return 解密结果
     */
    utils_result_t (*xor_decrypt)(const uint8_t *data, size_t length,
                                 const uint8_t *key, size_t key_length,
                                 uint8_t *output);

    /**
     * @brief 计算CRC32
     * @param data 数据
     * @param length 数据长度
     * @return CRC32值
     */
    uint32_t (*calculate_crc32)(const uint8_t *data, size_t length);

    /**
     * @brief 计算CRC16
     * @param data 数据
     * @param length 数据长度
     * @return CRC16值
     */
    uint16_t (*calculate_crc16)(const uint8_t *data, size_t length);

    /**
     * @brief 计算CRC8
     * @param data 数据
     * @param length 数据长度
     * @return CRC8值
     */
    uint8_t (*calculate_crc8)(const uint8_t *data, size_t length);

    /**
     * @brief 计算校验和
     * @param data 数据
     * @param length 数据长度
     * @return 校验和
     */
    uint32_t (*calculate_checksum)(const uint8_t *data, size_t length);

    /**
     * @brief 计算简单哈希
     * @param data 数据
     * @param length 数据长度
     * @return 哈希值
     */
    uint32_t (*calculate_hash)(const uint8_t *data, size_t length);

    /**
     * @brief 计算字符串哈希
     * @param str 字符串
     * @return 哈希值
     */
    uint32_t (*calculate_string_hash)(const char *str);

    /**
     * @brief 计算数据指纹
     * @param data 数据
     * @param length 数据长度
     * @param fingerprint 输出指纹
     * @param fingerprint_size 指纹大小
     * @return 计算结果
     */
    utils_result_t (*calculate_fingerprint)(const uint8_t *data, size_t length,
                                         uint8_t *fingerprint, size_t fingerprint_size);

    /**
     * @brief 生成随机数
     * @param min_val 最小值
     * @param max_val 最大值
     * @return 随机数
     */
    uint32_t (*generate_random)(uint32_t min_val, uint32_t max_val);

    /**
     * @brief 生成随机字节
     * @param data 输出数据
     * @param length 数据长度
     * @return 生成结果
     */
    utils_result_t (*generate_random_bytes)(uint8_t *data, size_t length);

    /**
     * @brief 生成随机字符串
     * @param str 输出字符串
     * @param length 字符串长度
     * @param character_set 字符集
     * @return 生成结果
     */
    utils_result_t (*generate_random_string)(char *str, size_t length,
                                          const char *character_set);

    /**
     * @brief 生成UUID
     * @param uuid 输出UUID
     * @return 生成结果
     */
    utils_result_t (*generate_uuid)(char *uuid);
} utils_crypto_t;

/* 工具接口集合 */
typedef struct {
    utils_math_t           *math;          /* 数学工具 */
    utils_string_t         *string;        /* 字符串工具 */
    utils_convert_t        *convert;       /* 转换工具 */
    utils_validate_t       *validate;      /* 验证工具 */
    utils_crypto_t         *crypto;        /* 加密工具 */
} utils_interface_t;

/* 全局工具接口实例 */
extern const utils_interface_t *utils;

/* 常用工具宏 */
#define CLAMP(value, min_val, max_val) \
    (((value) < (min_val)) ? (min_val) : ((value) > (max_val)) ? (max_val) : (value)))

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define ABS(value) (((value) < 0) ? -(value) : (value))

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

#define OFFSET_OF(type, member) ((size_t)&(((type *)0)->member))

#define CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr) - OFFSET_OF(type, member)))

#define IS_ALIGNED(addr, align) (((uintptr_t)(addr) & ((align) - 1)) == 0)

#define ALIGN_UP(addr, align) (((uintptr_t)(addr) + ((align) - 1)) & ~((align) - 1))
#define ALIGN_DOWN(addr, align) ((uintptr_t)(addr) & ~((align) - 1))

#define BIT_MASK(bit) (1U << (bit))
#define BIT_SET(value, bit) ((value) |= BIT_MASK(bit))
#define BIT_CLEAR(value, bit) ((value) &= ~BIT_MASK(bit))
#define BIT_TOGGLE(value, bit) ((value) ^= BIT_MASK(bit))
#define BIT_TEST(value, bit) (((value) & BIT_MASK(bit)) != 0)

#define SWAP16(value) (((value) >> 8) | ((value) << 8))
#define SWAP32(value) (((value) >> 24) | (((value) & 0x00FF0000) >> 8) | \
                       (((value) & 0x0000FF00) << 8) | ((value) << 24))

#define SAFE_STRCPY(dest, src, size) \
    do { \
        strncpy((dest), (src), (size) - 1); \
        (dest)[(size) - 1] = '\0'; \
    } while(0)

#define SAFE_STRCAT(dest, src, size) \
    do { \
        size_t _len = strlen((dest)); \
        if (_len < (size) - 1) { \
            strncpy((dest) + _len, (src), (size) - _len - 1); \
            (dest)[(size) - 1] = '\0'; \
        } \
    } while(0)

#define IS_EMPTY_STR(str) ((str) == NULL || *(str) == '\0')

#define STR_EQ(str1, str2) (strcmp((str1), (str2)) == 0)
#define STR_CASE_EQ(str1, str2) (strcasecmp((str1), (str2)) == 0)

#define UTILS_CHECK(expr) \
    do { \
        utils_result_t _result = (expr); \
        if (_result != UTILS_OK) { \
            return _result; \
        } \
    } while(0)

#define UTILS_CHECK_RET(expr, ret_val) \
    do { \
        utils_result_t _result = (expr); \
        if (_result != UTILS_OK) { \
            return (ret_val); \
        } \
    } while(0)

/* 便捷操作宏 */
#define UTILS_CLAMP(value, min_val, max_val) \
    utils->math->clamp_int32((value), (min_val), (max_val))

#define UTILS_CLAMP_FLOAT(value, min_val, max_val) \
    utils->math->clamp_float((value), (min_val), (max_val))

#define UTILS_LERP(start, end, factor) \
    utils->math->lerp((start), (end), (factor))

#define UTILS_MAP_RANGE(value, from_min, from_max, to_min, to_max) \
    utils->math->map_range((value), (from_min), (from_max), (to_min), (to_max))

#define UTILS_ABS(value) \
    utils->math->abs_int32((value))

#define UTILS_MIN(a, b) \
    utils->math->min_int32((a), (b))

#define UTILS_MAX(a, b) \
    utils->math->max_int32((a), (b))

#define UTILS_STRLEN(str) \
    utils->string->strlen((str))

#define UTILS_STRCPY(dest, src, max_size) \
    utils->string->strcpy((dest), (src), (max_size))

#define UTILS_STRCAT(dest, src, max_size) \
    utils->string->strcat((dest), (src), (max_size))

#define UTILS_STRCMP(str1, str2, ignore_case) \
    utils->string->strcmp((str1), (str2), (ignore_case))

#define UTILS_STRFIND(str, substr, ignore_case) \
    utils->string->strfind((str), (substr), (ignore_case))

#define UTILS_ATOI(str, value, base) \
    utils->string->atoi((str), (value), (base))

#define UTILS_ATOF(str, value) \
    utils->string->atof((str), (value))

#define UTILS_ITOA(value, str, max_size, base) \
    utils->string->itoa((value), (str), (max_size), (base))

#define UTILS_FTOA(value, str, max_size, decimal_places) \
    utils->string->ftoa((value), (str), (max_size), (decimal_places))

#define UTILS_IS_EMPTY(str) \
    utils->string->is_empty((str))

#define UTILS_IS_NUMERIC(str) \
    utils->string->is_numeric((str))

#define UTILS_SWAP16(value) \
    utils->convert->swap16((value))

#define UTILS_SWAP32(value) \
    utils->convert->swap32((value))

#define UTILS_HTONS(value) \
    utils->convert->htons((value))

#define UTILS_HTONL(value) \
    utils->convert->htonl((value))

#define UTILS_NTOHS(value) \
    utils->convert->ntohs((value))

#define UTILS_NTOHL(value) \
    utils->convert->ntohl((value))

#define UTILS_HEX_TO_BYTES(hex_str, bytes, max_bytes) \
    utils->convert->hex_to_bytes((hex_str), (bytes), (max_bytes))

#define UTILS_BYTES_TO_HEX(bytes, length, hex_str, max_length) \
    utils->convert->bytes_to_hex((bytes), (length), (hex_str), (max_length))

#define UTILS_BASE64_ENCODE(data, data_length, encoded_data, max_length, encoded_length) \
    utils->convert->base64_encode((data), (data_length), (encoded_data), (max_length), (encoded_length))

#define UTILS_BASE64_DECODE(encoded_data, encoded_length, decoded_data, max_length, decoded_length) \
    utils->convert->base64_decode((encoded_data), (encoded_length), (decoded_data), (max_length), (decoded_length))

#define UTILS_VALIDATE_INT_RANGE(value, min_val, max_val) \
    utils->validate->validate_int_range((value), (min_val), (max_val))

#define UTILS_VALIDATE_FLOAT_RANGE(value, min_val, max_val) \
    utils->validate->validate_float_range((value), (min_val), (max_val))

#define UTILS_VALIDATE_STRING_LENGTH(str, min_length, max_length) \
    utils->validate->validate_string_length((str), (min_length), (max_length))

#define UTILS_VALIDATE_IP_ADDRESS(ip_str) \
    utils->validate->validate_ip_address((ip_str))

#define UTILS_VALIDATE_MAC_ADDRESS(mac_str) \
    utils->validate->validate_mac_address((mac_str))

#define UTILS_CALCULATE_CRC32(data, length) \
    utils->crypto->calculate_crc32((data), (length))

#define UTILS_CALCULATE_CRC16(data, length) \
    utils->crypto->calculate_crc16((data), (length))

#define UTILS_CALCULATE_CRC8(data, length) \
    utils->crypto->calculate_crc8((data), (length))

#define UTILS_CALCULATE_CHECKSUM(data, length) \
    utils->crypto->calculate_checksum((data), (length))

#define UTILS_CALCULATE_HASH(data, length) \
    utils->crypto->calculate_hash((data), (length))

#define UTILS_GENERATE_RANDOM(min_val, max_val) \
    utils->crypto->generate_random((min_val), (max_val))

#define UTILS_GENERATE_RANDOM_BYTES(data, length) \
    utils->crypto->generate_random_bytes((data), (length))

#define UTILS_GENERATE_RANDOM_STRING(str, length, character_set) \
    utils->crypto->generate_random_string((str), (length), (character_set))

#define UTILS_GENERATE_UUID(uuid) \
    utils->crypto->generate_uuid((uuid))

#ifdef __cplusplus
}
#endif

#endif /* UTILS_H */