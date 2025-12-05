/**
 * @file config_loader.h
 * @brief 配置加载器接口定义
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * 提供配置数据加载、解析、转换等功能
 */

#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H

#include <stdint.h>
#include <stdbool.h>
#include "config_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 配置加载器错误码定义 */
typedef enum {
    CONFIG_LOADER_OK = 0,               /* 加载成功 */
    CONFIG_LOADER_ERROR_INVALID_FORMAT,   /* 无效格式 */
    CONFIG_LOADER_ERROR_PARSE_FAILED,     /* 解析失败 */
    CONFIG_LOADER_ERROR_INVALID_DATA,     /* 无效数据 */
    CONFIG_LOADER_ERROR_VERSION_MISMATCH, /* 版本不匹配 */
    CONFIG_LOADER_ERROR_CHECKSUM_FAILED,   /* 校验失败 */
    CONFIG_LOADER_ERROR_BUFFER_TOO_SMALL, /* 缓冲区太小 */
    CONFIG_LOADER_ERROR_MEMORY_ALLOCATION, /* 内存分配失败 */
    CONFIG_LOADER_ERROR_FILE_NOT_FOUND,   /* 文件未找到 */
    CONFIG_LOADER_ERROR_READ_FAILED,      /* 读取失败 */
    CONFIG_LOADER_ERROR_DECOMPRESSION_FAILED, /* 解压失败 */
    CONFIG_LOADER_ERROR_DECRYPTION_FAILED, /* 解密失败 */
    CONFIG_LOADER_ERROR_UNKNOWN_KEY,       /* 未知配置键 */
    CONFIG_LOADER_ERROR_TYPE_MISMATCH,    /* 数据类型不匹配 */
    CONFIG_LOADER_ERROR_RANGE_VIOLATION,   /* 范围违规 */
    CONFIG_LOADER_ERROR_CORRUPTED_DATA     /* 数据损坏 */
} config_loader_result_t;

/* 配置文件格式定义 */
typedef enum {
    CONFIG_FORMAT_BINARY = 0,             /* 二进制格式 */
    CONFIG_FORMAT_JSON,                   /* JSON格式 */
    CONFIG_FORMAT_XML,                     /* XML格式 */
    CONFIG_FORMAT_INI,                     /* INI格式 */
    CONFIG_FORMAT_CSV,                     /* CSV格式 */
    CONFIG_FORMAT_YAML,                    /* YAML格式 */
    CONFIG_FORMAT_TOML,                    /* TOML格式 */
    CONFIG_FORMAT_CUSTOM,                  /* 自定义格式 */
    CONFIG_FORMAT_COMPRESSED,              /* 压缩格式 */
    CONFIG_FORMAT_ENCRYPTED               /* 加密格式 */
} config_format_t;

/* 配置加载模式定义 */
typedef enum {
    CONFIG_LOAD_MODE_REPLACE = 0,          /* 替换模式 */
    CONFIG_LOAD_MODE_MERGE,               /* 合并模式 */
    CONFIG_LOAD_MODE_UPDATE,              /* 更新模式 */
    CONFIG_LOAD_MODE_APPEND,               /* 追加模式 */
    CONFIG_LOAD_MODE_OVERRIDE              /* 覆盖模式 */
} config_load_mode_t;

/* 配置压缩算法定义 */
typedef enum {
    CONFIG_COMPRESSION_NONE = 0,           /* 无压缩 */
    CONFIG_COMPRESSION_ZLIB,               /* ZLIB压缩 */
    CONFIG_COMPRESSION_GZIP,               /* GZIP压缩 */
    CONFIG_COMPRESSION_LZMA,               /* LZMA压缩 */
    CONFIG_COMPRESSION_LZ4,                /* LZ4压缩 */
    CONFIG_COMPRESSION_CUSTOM              /* 自定义压缩 */
} config_compression_t;

/* 配置加密算法定义 */
typedef enum {
    CONFIG_ENCRYPTION_NONE = 0,            /* 无加密 */
    CONFIG_ENCRYPTION_AES128,              /* AES128加密 */
    CONFIG_ENCRYPTION_AES256,              /* AES256加密 */
    CONFIG_ENCRYPTION_XOR,                 /* XOR加密 */
    CONFIG_ENCRYPTION_CUSTOM              /* 自定义加密 */
} config_encryption_t;

/* 配置加载器配置结构体 */
typedef struct {
    config_format_t         format;         /* 配置格式 */
    config_load_mode_t      load_mode;      /* 加载模式 */
    config_compression_t    compression;    /* 压缩算法 */
    config_encryption_t     encryption;     /* 加密算法 */
    bool                    strict_parsing; /* 严格解析模式 */
    bool                    allow_unknown_keys; /* 允许未知键 */
    bool                    auto_convert;   /* 自动类型转换 */
    bool                    validate_values; /* 验证配置值 */
    bool                    backup_existing; /* 备份现有配置 */
    uint32_t                max_file_size;   /* 最大文件大小 */
    uint32_t                max_nesting_level; /* 最大嵌套层级 */
    const char             *encoding;        /* 字符编码 */
    const uint8_t          *encryption_key;  /* 加密密钥 */
    uint16_t                key_length;     /* 密钥长度 */
} config_loader_config_t;

/* 配置文件头结构体 */
typedef struct {
    char                    magic[4];       /* 魔数 */
    uint8_t                 version;        /* 版本号 */
    uint8_t                 format;         /* 格式类型 */
    uint8_t                 compression;    /* 压缩算法 */
    uint8_t                 encryption;     /* 加密算法 */
    uint32_t                data_size;      /* 数据大小 */
    uint32_t                checksum;       /* 校验和 */
    uint32_t                timestamp;      /* 时间戳 */
} config_file_header_t;

/* 配置加载信息结构体 */
typedef struct {
    config_format_t         format;         /* 加载的格式 */
    uint32_t                total_size;     /* 总大小 */
    uint32_t                data_size;      /* 数据大小 */
    uint32_t                items_loaded;   /* 加载的配置项数量 */
    uint32_t                items_skipped;   /* 跳过的配置项数量 */
    uint32_t                items_updated;  /* 更新的配置项数量 */
    uint32_t                items_added;    /* 新增的配置项数量 */
    uint32_t                parse_errors;   /* 解析错误数量 */
    uint32_t                validation_errors; /* 验证错误数量 */
    uint32_t                conversion_errors; /* 转换错误数量 */
    uint32_t                load_time_ms;   /* 加载时间(毫秒) */
    bool                    success;        /* 是否成功 */
    char                    error_message[128]; /* 错误消息 */
} config_load_info_t;

/* 配置解析器接口函数指针结构体 */
typedef struct {
    /**
     * @brief 初始化解析器
     * @param config 解析器配置
     * @return 初始化结果
     */
    config_loader_result_t (*init)(const config_loader_config_t *config);

    /**
     * @brief 反初始化解析器
     * @return 反初始化结果
     */
    config_loader_result_t (*deinit)(void);

    /**
     * @brief 解析配置数据
     * @param data 配置数据
     * @param size 数据大小
     * @param items 输出配置项数组
     * @param max_items 最大配置项数量
     * @param item_count 输出配置项数量
     * @return 解析结果
     */
    config_loader_result_t (*parse)(const void *data, uint32_t size,
                                     config_item_t *items, uint16_t max_items,
                                     uint16_t *item_count);

    /**
     * @brief 从文件解析配置
     * @param filename 文件名
     * @param items 输出配置项数组
     * @param max_items 最大配置项数量
     * @param item_count 输出配置项数量
     * @return 解析结果
     */
    config_loader_result_t (*parse_file)(const char *filename,
                                         config_item_t *items, uint16_t max_items,
                                         uint16_t *item_count);

    /**
     * @brief 验证配置数据
     * @param data 配置数据
     * @param size 数据大小
     * @return 验证结果
     */
    config_loader_result_t (*validate)(const void *data, uint32_t size);

    /**
     * @brief 获取支持的格式
     * @param formats 输出支持格式数组
     * @param max_formats 最大格式数量
     * @param format_count 输出格式数量
     * @return 获取结果
     */
    config_loader_result_t (*get_supported_formats)(config_format_t *formats,
                                                     uint8_t max_formats,
                                                     uint8_t *format_count);

    /**
     * @brief 检查格式是否支持
     * @param format 格式类型
     * @return 是否支持
     */
    bool (*is_format_supported)(config_format_t format);

    /**
     * @brief 获取解析器版本
     * @return 版本号
     */
    uint32_t (*get_version)(void);
} config_parser_interface_t;

/* 配置转换器接口函数指针结构体 */
typedef struct {
    /**
     * @brief 初始化转换器
     * @return 初始化结果
     */
    config_loader_result_t (*init)(void);

    /**
     * @brief 反初始化转换器
     * @return 反初始化结果
     */
    config_loader_result_t (*deinit)(void);

    /**
     * @brief 转换配置格式
     * @param source_data 源数据
     * @param source_size 源数据大小
     * @param source_format 源格式
     * @param target_data 输出目标数据
     * @param target_size 目标数据大小
     * @param target_format 目标格式
     * @param actual_size 输出实际大小
     * @return 转换结果
     */
    config_loader_result_t (*convert)(const void *source_data, uint32_t source_size,
                                       config_format_t source_format,
                                       void *target_data, uint32_t target_size,
                                       config_format_t target_format,
                                       uint32_t *actual_size);

    /**
     * @brief 转换数据类型
     * @param source_value 源值
     * @param source_type 源类型
     * @param target_value 输出目标值
     * @param target_type 目标类型
     * @return 转换结果
     */
    config_loader_result_t (*convert_type)(const config_value_t *source_value,
                                            config_data_type_t source_type,
                                            config_value_t *target_value,
                                            config_data_type_t target_type);

    /**
     * @brief 获取转换器版本
     * @return 版本号
     */
    uint32_t (*get_version)(void);
} config_converter_interface_t;

/* 配置加载器接口函数指针结构体 */
typedef struct {
    /**
     * @brief 初始化配置加载器
     * @param config 加载器配置
     * @return 初始化结果
     */
    config_loader_result_t (*init)(const config_loader_config_t *config);

    /**
     * @brief 反初始化配置加载器
     * @return 反初始化结果
     */
    config_loader_result_t (*deinit)(void);

    /**
     * @brief 注册解析器
     * @param format 支持的格式
     * @param parser 解析器接口
     * @return 注册结果
     */
    config_loader_result_t (*register_parser)(config_format_t format,
                                                const config_parser_interface_t *parser);

    /**
     * @brief 注销解析器
     * @param format 格式类型
     * @return 注销结果
     */
    config_loader_result_t (*unregister_parser)(config_format_t format);

    /**
     * @brief 从内存加载配置
     * @param data 配置数据
     * @param size 数据大小
     * @param category 配置类别
     * @param load_info 输出加载信息
     * @return 加载结果
     */
    config_loader_result_t (*load_from_memory)(const void *data, uint32_t size,
                                                config_category_t category,
                                                config_load_info_t *load_info);

    /**
     * @brief 从文件加载配置
     * @param filename 文件名
     * @param category 配置类别
     * @param load_info 输出加载信息
     * @return 加载结果
     */
    config_loader_result_t (*load_from_file)(const char *filename,
                                              config_category_t category,
                                              config_load_info_t *load_info);

    /**
     * @brief 从多个文件加载配置
     * @param filenames 文件名数组
     * @param file_count 文件数量
     * @param category 配置类别
     * @param load_info 输出加载信息
     * @return 加载结果
     */
    config_loader_result_t (*load_from_files)(const char **filenames, uint8_t file_count,
                                               config_category_t category,
                                               config_load_info_t *load_info);

    /**
     * @brief 自动检测并加载配置
     * @param directory 目录路径
     * @param pattern 文件模式
     * @param category 配置类别
     * @param load_info 输出加载信息
     * @return 加载结果
     */
    config_loader_result_t (*auto_load)(const char *directory, const char *pattern,
                                          config_category_t category,
                                          config_load_info_t *load_info);

    /**
     * @brief 检测配置文件格式
     * @param data 配置数据
     * @param size 数据大小
     * @param format 输出检测到的格式
     * @return 检测结果
     */
    config_loader_result_t (*detect_format)(const void *data, uint32_t size,
                                              config_format_t *format);

    /**
     * @brief 从文件检测格式
     * @param filename 文件名
     * @param format 输出检测到的格式
     * @return 检测结果
     */
    config_loader_result_t (*detect_format_from_file)(const char *filename,
                                                        config_format_t *format);

    /**
     * @brief 验证配置文件
     * @param filename 文件名
     * @param load_info 输出加载信息
     * @return 验证结果
     */
    config_loader_result_t (*validate_file)(const char *filename,
                                              config_load_info_t *load_info);

    /**
     * @brief 预加载配置文件
     * @param filename 文件名
     * @param file_handle 输出文件句柄
     * @return 预加载结果
     */
    config_loader_result_t (*preload_file)(const char *filename, void **file_handle);

    /**
     * @brief 完成加载
     * @param file_handle 文件句柄
     * @param category 配置类别
     * @param load_info 输出加载信息
     * @return 完成结果
     */
    config_loader_result_t (*finish_load)(void *file_handle,
                                           config_category_t category,
                                           config_load_info_t *load_info);

    /**
     * @brief 取消加载
     * @param file_handle 文件句柄
     * @return 取消结果
     */
    config_loader_result_t (*cancel_load)(void *file_handle);

    /**
     * @brief 设置加载进度回调
     * @param callback 回调函数
     * @param user_data 用户数据
     * @return 设置结果
     */
    config_loader_result_t (*set_progress_callback)(void (*callback)(uint32_t loaded, uint32_t total, void *user_data),
                                                    void *user_data);

    /**
     * @brief 获取支持的格式
     * @param formats 输出支持格式数组
     * @param max_formats 最大格式数量
     * @param format_count 输出格式数量
     * @return 获取结果
     */
    config_loader_result_t (*get_supported_formats)(config_format_t *formats,
                                                    uint8_t max_formats,
                                                    uint8_t *format_count);

    /**
     * @brief 检查格式是否支持
     * @param format 格式类型
     * @return 是否支持
     */
    bool (*is_format_supported)(config_format_t format);

    /**
     * @brief 获取加载统计信息
     * @param total_files 总文件数
     * @param successful_loads 成功加载数
     * @param failed_loads 失败加载数
     * @param total_time_ms 总耗时
     * @return 获取结果
     */
    config_loader_result_t (*get_load_statistics)(uint32_t *total_files,
                                                   uint32_t *successful_loads,
                                                   uint32_t *failed_loads,
                                                   uint32_t *total_time_ms);

    /**
     * @brief 重置加载统计信息
     * @return 重置结果
     */
    config_loader_result_t (*reset_statistics)(void);

    /**
     * @brief 获取配置加载器版本
     * @return 版本号
     */
    uint32_t (*get_version)(void);
} config_loader_interface_t;

/* 全局配置加载器实例 */
extern const config_loader_interface_t *config_loader;

/* 常用配置加载器宏 */
#define CONFIG_LOADER_CONFIG_DEFAULT() \
    { CONFIG_FORMAT_BINARY, CONFIG_LOAD_MODE_REPLACE, CONFIG_COMPRESSION_NONE, CONFIG_ENCRYPTION_NONE, \
      true, false, true, true, false, 65536, 10, "UTF-8", NULL, 0 }

#define CONFIG_LOADER_CONFIG_JSON() \
    { CONFIG_FORMAT_JSON, CONFIG_LOAD_MODE_MERGE, CONFIG_COMPRESSION_NONE, CONFIG_ENCRYPTION_NONE, \
      true, false, true, true, false, 1048576, 20, "UTF-8", NULL, 0 }

#define CONFIG_LOADER_CONFIG_COMPRESSED() \
    { CONFIG_FORMAT_BINARY, CONFIG_LOAD_MODE_REPLACE, CONFIG_COMPRESSION_ZLIB, CONFIG_ENCRYPTION_NONE, \
      true, false, true, true, false, 262144, 10, "UTF-8", NULL, 0 }

#define CONFIG_LOADER_CONFIG_ENCRYPTED(key, key_len) \
    { CONFIG_FORMAT_BINARY, CONFIG_LOAD_MODE_REPLACE, CONFIG_COMPRESSION_NONE, CONFIG_ENCRYPTION_AES256, \
      true, false, true, true, false, 65536, 10, "UTF-8", (key), (key_len) }

/* 配置加载错误处理宏 */
#define CONFIG_LOADER_CHECK(expr) \
    do { \
        config_loader_result_t _result = (expr); \
        if (_result != CONFIG_LOADER_OK) { \
            return _result; \
        } \
    } while(0)

#define CONFIG_LOADER_CHECK_RET(expr, ret_val) \
    do { \
        config_loader_result_t _result = (expr); \
        if (_result != CONFIG_LOADER_OK) { \
            return (ret_val); \
        } \
    } while(0)

/* 便捷操作宏 */
#define CONFIG_LOAD_FROM_FILE(filename, category) \
    config_loader->load_from_file((filename), (category), NULL)

#define CONFIG_LOAD_FROM_MEMORY(data, size, category) \
    config_loader->load_from_memory((data), (size), (category), NULL)

#define CONFIG_AUTO_LOAD(directory, pattern, category) \
    config_loader->auto_load((directory), (pattern), (category), NULL)

#define CONFIG_VALIDATE_FILE(filename) \
    config_loader->validate_file((filename), NULL)

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_LOADER_H */