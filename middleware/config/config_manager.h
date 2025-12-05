/**
 * @file config_manager.h
 * @brief 配置管理器接口定义
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * 提供设备配置、用户设置、按键映射等的统一管理接口
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 配置管理器错误码定义 */
typedef enum {
    CONFIG_OK = 0,                   /* 操作成功 */
    CONFIG_ERROR_INVALID_PARAM,      /* 无效参数 */
    CONFIG_ERROR_NOT_SUPPORTED,      /* 操作不支持 */
    CONFIG_ERROR_HARDWARE_FAULT,     /* 硬件故障 */
    CONFIG_ERROR_TIMEOUT,            /* 操作超时 */
    CONFIG_ERROR_ALREADY_INITIALIZED,/* 已初始化 */
    CONFIG_ERROR_NOT_INITIALIZED,    /* 未初始化 */
    CONFIG_ERROR_INVALID_KEY,        /* 无效配置键 */
    CONFIG_ERROR_KEY_NOT_FOUND,       /* 配置键未找到 */
    CONFIG_ERROR_KEY_ALREADY_EXISTS,  /* 配置键已存在 */
    CONFIG_ERROR_INVALID_TYPE,       /* 无效数据类型 */
    CONFIG_ERROR_TYPE_MISMATCH,      /* 数据类型不匹配 */
    CONFIG_ERROR_BUFFER_TOO_SMALL,    /* 缓冲区太小 */
    CONFIG_ERROR_CHECKSUM_FAILED,     /* 校验失败 */
    CONFIG_ERROR_VERSION_MISMATCH,   /* 版本不匹配 */
    CONFIG_ERROR_STORAGE_FAILED,     /* 存储失败 */
    CONFIG_ERROR_LOAD_FAILED,        /* 加载失败 */
    CONFIG_ERROR_SAVE_FAILED,        /* 保存失败 */
    CONFIG_ERROR_BACKUP_FAILED,       /* 备份失败 */
    CONFIG_ERROR_RESTORE_FAILED,      /* 恢复失败 */
    CONFIG_ERROR_VALIDATION_FAILED,   /* 验证失败 */
    CONFIG_ERROR_READONLY,           /* 只读配置 */
    CONFIG_ERROR_WRITE_PROTECTED      /* 写保护 */
} config_result_t;

/* 配置数据类型定义 */
typedef enum {
    CONFIG_TYPE_NONE = 0,            /* 无类型 */
    CONFIG_TYPE_BOOL,                /* 布尔类型 */
    CONFIG_TYPE_INT8,                /* 8位整数 */
    CONFIG_TYPE_UINT8,               /* 8位无符号整数 */
    CONFIG_TYPE_INT16,               /* 16位整数 */
    CONFIG_TYPE_UINT16,              /* 16位无符号整数 */
    CONFIG_TYPE_INT32,               /* 32位整数 */
    CONFIG_TYPE_UINT32,              /* 32位无符号整数 */
    CONFIG_TYPE_INT64,               /* 64位整数 */
    CONFIG_TYPE_UINT64,              /* 64位无符号整数 */
    CONFIG_TYPE_FLOAT,               /* 浮点数 */
    CONFIG_TYPE_DOUBLE,              /* 双精度浮点数 */
    CONFIG_TYPE_STRING,              /* 字符串 */
    CONFIG_TYPE_BINARY,              /* 二进制数据 */
    CONFIG_TYPE_ARRAY,               /* 数组 */
    CONFIG_TYPE_STRUCT,              /* 结构体 */
    CONFIG_TYPE_ENUM                 /* 枚举 */
} config_data_type_t;

/* 配置存储位置定义 */
typedef enum {
    CONFIG_STORAGE_RAM = 0,          /* RAM存储 */
    CONFIG_STORAGE_EEPROM,           /* EEPROM存储 */
    CONFIG_STORAGE_FLASH,            /* Flash存储 */
    CONFIG_STORAGE_EXTERNAL,         /* 外部存储 */
    CONFIG_STORAGE_CACHED,           /* 缓存存储 */
    CONFIG_STORAGE_TEMPORARY,        /* 临时存储 */
    CONFIG_STORAGE_PERSISTENT,       /* 持久存储 */
    CONFIG_STORAGE_VOLATILE          /* 易失存储 */
} config_storage_type_t;

/* 配置访问级别定义 */
typedef enum {
    CONFIG_ACCESS_PUBLIC = 0,       /* 公开访问 */
    CONFIG_ACCESS_USER,             /* 用户访问 */
    CONFIG_ACCESS_ADMIN,            /* 管理员访问 */
    CONFIG_ACCESS_SYSTEM,           /* 系统访问 */
    CONFIG_ACCESS_READONLY,         /* 只读访问 */
    CONFIG_ACCESS_READWRITE,        /* 读写访问 */
    CONFIG_ACCESS_HIDDEN,           /* 隐藏访问 */
    CONFIG_ACCESS_PROTECTED         /* 受保护访问 */
} config_access_level_t;

/* 配置类别定义 */
typedef enum {
    CONFIG_CATEGORY_SYSTEM = 0,      /* 系统配置 */
    CONFIG_CATEGORY_HARDWARE,        /* 硬件配置 */
    CONFIG_CATEGORY_KEYBOARD,        /* 键盘配置 */
    CONFIG_CATEGORY_TOUCHPAD,        /* 触控板配置 */
    CONFIG_CATEGORY_LED,             /* LED配置 */
    CONFIG_CATEGORY_BACKLIGHT,      /* 背光配置 */
    CONFIG_CATEGORY_GESTURE,        /* 手势配置 */
    CONFIG_CATEGORY_COMMUNICATION,   /* 通信配置 */
    CONFIG_CATEGORY_POWER,          /* 电源配置 */
    CONFIG_CATEGORY_USER,           /* 用户配置 */
    CONFIG_CATEGORY_PROFILE,        /* 配置文件 */
    CONFIG_CATEGORY_DEBUG,          /* 调试配置 */
    CONFIG_CATEGORY_CUSTOM          /* 自定义配置 */
} config_category_t;

/* 配置值联合体 */
typedef union {
    bool                    bool_val;       /* 布尔值 */
    int8_t                  int8_val;       /* 8位整数 */
    uint8_t                 uint8_val;      /* 8位无符号整数 */
    int16_t                 int16_val;      /* 16位整数 */
    uint16_t                uint16_val;     /* 16位无符号整数 */
    int32_t                 int32_val;      /* 32位整数 */
    uint32_t                uint32_val;     /* 32位无符号整数 */
    int64_t                 int64_val;      /* 64位整数 */
    uint64_t                uint64_val;     /* 64位无符号整数 */
    float                   float_val;      /* 浮点数 */
    double                  double_val;     /* 双精度浮点数 */
    char                   *string_val;     /* 字符串值 */
    void                   *binary_val;     /* 二进制数据 */
    void                   *array_val;      /* 数组值 */
    void                   *struct_val;     /* 结构体值 */
    uint32_t                enum_val;       /* 枚举值 */
} config_value_t;

/* 配置项属性结构体 */
typedef struct {
    const char             *key;            /* 配置键名 */
    const char             *description;    /* 配置描述 */
    config_data_type_t     type;           /* 数据类型 */
    config_category_t      category;       /* 配置类别 */
    config_access_level_t  access_level;   /* 访问级别 */
    config_storage_type_t   storage_type;   /* 存储类型 */
    uint16_t                size;           /* 数据大小 */
    uint16_t                offset;         /* 偏移量 */
    bool                    readonly;       /* 只读标志 */
    bool                    persistent;     /* 持久化标志 */
    bool                    encrypted;      /* 加密标志 */
    bool                    checksum;       /* 校验和标志 */
    bool                    required;       /* 必需标志 */
    uint32_t                min_value;      /* 最小值 */
    uint32_t                max_value;      /* 最大值 */
    uint32_t                default_value;  /* 默认值 */
    const char             *unit;           /* 单位 */
    const char             *format;         /* 格式化字符串 */
} config_item_attrib_t;

/* 配置项结构体 */
typedef struct {
    const char             *key;            /* 配置键名 */
    config_data_type_t     type;           /* 数据类型 */
    config_value_t          value;          /* 配置值 */
    uint16_t                size;           /* 数据大小 */
    uint32_t                timestamp;      /* 时间戳 */
    uint32_t                version;        /* 版本号 */
    bool                    modified;       /* 修改标志 */
    bool                    dirty;          /* 脏标志 */
    const config_item_attrib_t *attrib;     /* 配置属性 */
} config_item_t;

/* 配置区域结构体 */
typedef struct {
    config_category_t      category;       /* 配置类别 */
    const char             *name;           /* 区域名称 */
    const char             *description;    /* 区域描述 */
    uint32_t                base_address;   /* 基地址 */
    uint32_t                size;           /* 区域大小 */
    uint16_t                item_count;     /* 配置项数量 */
    const config_item_t   *items;          /* 配置项数组 */
    bool                    auto_save;      /* 自动保存 */
    bool                    backup_enabled; /* 启用备份 */
    uint16_t                backup_count;   /* 备份数量 */
    uint32_t                version;        /* 版本号 */
} config_region_t;

/* 配置管理器配置结构体 */
typedef struct {
    bool                    auto_load;      /* 自动加载 */
    bool                    auto_save;      /* 自动保存 */
    bool                    backup_enabled; /* 启用备份 */
    uint16_t                backup_interval_ms; /* 备份间隔(毫秒) */
    uint8_t                 max_backups;    /* 最大备份数量 */
    bool                    compression;    /* 启用压缩 */
    bool                    encryption;     /* 启用加密 */
    uint16_t                checksum_size;  /* 校验和大小 */
    uint32_t                save_timeout_ms; /* 保存超时(毫秒) */
    uint32_t                load_timeout_ms; /* 加载超时(毫秒) */
    uint8_t                 retry_count;    /* 重试次数 */
    bool                    validate_on_load; /* 加载时验证 */
    bool                    validate_on_save; /* 保存时验证 */
} config_manager_config_t;

/* 配置回调函数类型 */
typedef void (*config_change_callback_t)(const char *key, const config_value_t *old_value,
                                         const config_value_t *new_value, void *user_data);

typedef void (*config_load_callback_t)(config_category_t category, bool success, void *user_data);

typedef void (*config_save_callback_t)(config_category_t category, bool success, void *user_data);

/* 配置管理器接口函数指针结构体 */
typedef struct {
    /**
     * @brief 初始化配置管理器
     * @param config 配置管理器配置
     * @return 初始化结果
     */
    config_result_t (*init)(const config_manager_config_t *config);

    /**
     * @brief 反初始化配置管理器
     * @return 反初始化结果
     */
    config_result_t (*deinit)(void);

    /**
     * @brief 注册配置区域
     * @param region 配置区域
     * @return 注册结果
     */
    config_result_t (*register_region)(const config_region_t *region);

    /**
     * @brief 注销配置区域
     * @param category 配置类别
     * @return 注销结果
     */
    config_result_t (*unregister_region)(config_category_t category);

    /**
     * @brief 加载配置
     * @param category 配置类别
     * @return 加载结果
     */
    config_result_t (*load_config)(config_category_t category);

    /**
     * @brief 保存配置
     * @param category 配置类别
     * @return 保存结果
     */
    config_result_t (*save_config)(config_category_t category);

    /**
     * @brief 加载所有配置
     * @return 加载结果
     */
    config_result_t (*load_all_configs)(void);

    /**
     * @brief 保存所有配置
     * @return 保存结果
     */
    config_result_t (*save_all_configs)(void);

    /**
     * @brief 获取配置值
     * @param key 配置键名
     * @param type 数据类型
     * @param value 输出配置值
     * @param size 值大小
     * @return 获取结果
     */
    config_result_t (*get_value)(const char *key, config_data_type_t type,
                                 void *value, uint16_t size);

    /**
     * @brief 设置配置值
     * @param key 配置键名
     * @param type 数据类型
     * @param value 配置值
     * @param size 值大小
     * @return 设置结果
     */
    config_result_t (*set_value)(const char *key, config_data_type_t type,
                                 const void *value, uint16_t size);

    /**
     * @brief 获取默认值
     * @param key 配置键名
     * @param type 数据类型
     * @param value 输出默认值
     * @param size 值大小
     * @return 获取结果
     */
    config_result_t (*get_default_value)(const char *key, config_data_type_t type,
                                         void *value, uint16_t size);

    /**
     * @brief 重置配置为默认值
     * @param key 配置键名
     * @return 重置结果
     */
    config_result_t (*reset_to_default)(const char *key);

    /**
     * @brief 重置区域为默认值
     * @param category 配置类别
     * @return 重置结果
     */
    config_result_t (*reset_region_to_default)(config_category_t category);

    /**
     * @brief 重置所有配置为默认值
     * @return 重置结果
     */
    config_result_t (*reset_all_to_default)(void);

    /**
     * @brief 检查配置键是否存在
     * @param key 配置键名
     * @param exists 输出是否存在标志
     * @return 检查结果
     */
    config_result_t (*key_exists)(const char *key, bool *exists);

    /**
     * @brief 获取配置项信息
     * @param key 配置键名
     * @param item 输出配置项信息
     * @return 获取结果
     */
    config_result_t (*get_item_info)(const char *key, config_item_t *item);

    /**
     * @brief 获取配置项属性
     * @param key 配置键名
     * @param attrib 输出配置项属性
     * @return 获取结果
     */
    config_result_t (*get_item_attrib)(const char *key, const config_item_attrib_t **attrib);

    /**
     * @brief 枚举配置项
     * @param category 配置类别
     * @param callback 枚举回调函数
     * @param user_data 用户数据
     * @return 枚举结果
     */
    config_result_t (*enumerate_items)(config_category_t category,
                                       void (*callback)(const config_item_t *item, void *user_data),
                                       void *user_data);

    /**
     * @brief 搜索配置项
     * @param pattern 搜索模式
     * @param results 输出搜索结果
     * @param max_results 最大结果数量
     * @param result_count 输出结果数量
     * @return 搜索结果
     */
    config_result_t (*search_items)(const char *pattern, const config_item_t **results,
                                     uint16_t max_results, uint16_t *result_count);

    /**
     * @brief 验证配置
     * @param category 配置类别
     * @return 验证结果
     */
    config_result_t (*validate_config)(config_category_t category);

    /**
     * @brief 验证所有配置
     * @return 验证结果
     */
    config_result_t (*validate_all_configs)(void);

    /**
     * @brief 备份配置
     * @param category 配置类别
     * @return 备份结果
     */
    config_result_t (*backup_config)(config_category_t category);

    /**
     * @brief 恢复配置
     * @param category 配置类别
     * @param backup_index 备份索引
     * @return 恢复结果
     */
    config_result_t (*restore_config)(config_category_t category, uint8_t backup_index);

    /**
     * @brief 获取备份数量
     * @param category 配置类别
     * @param count 输出备份数量
     * @return 获取结果
     */
    config_result_t (*get_backup_count)(config_category_t category, uint8_t *count);

    /**
     * @brief 删除备份
     * @param category 配置类别
     * @param backup_index 备份索引
     * @return 删除结果
     */
    config_result_t (*delete_backup)(config_category_t category, uint8_t backup_index);

    /**
     * @brief 清空所有备份
     * @return 清空结果
     */
    config_result_t (*clear_all_backups)(void);

    /**
     * @brief 导出配置
     * @param category 配置类别
     * @param buffer 输出缓冲区
     * @param buffer_size 缓冲区大小
     * @param exported_size 输出导出大小
     * @return 导出结果
     */
    config_result_t (*export_config)(config_category_t category, void *buffer,
                                     uint32_t buffer_size, uint32_t *exported_size);

    /**
     * @brief 导入配置
     * @param category 配置类别
     * @param data 导入数据
     * @param data_size 数据大小
     * @return 导入结果
     */
    config_result_t (*import_config)(config_category_t category, const void *data, uint32_t data_size);

    /**
     * @brief 注册配置变更回调
     * @param key 配置键名(为NULL表示全局回调)
     * @param callback 回调函数
     * @param user_data 用户数据
     * @return 注册结果
     */
    config_result_t (*register_change_callback)(const char *key, config_change_callback_t callback,
                                               void *user_data);

    /**
     * @brief 注销配置变更回调
     * @param key 配置键名
     * @param callback 回调函数
     * @return 注销结果
     */
    config_result_t (*unregister_change_callback)(const char *key, config_change_callback_t callback);

    /**
     * @brief 注册加载回调
     * @param category 配置类别
     * @param callback 回调函数
     * @param user_data 用户数据
     * @return 注册结果
     */
    config_result_t (*register_load_callback)(config_category_t category, config_load_callback_t callback,
                                            void *user_data);

    /**
     * @brief 注销加载回调
     * @param category 配置类别
     * @param callback 回调函数
     * @return 注销结果
     */
    config_result_t (*unregister_load_callback)(config_category_t category, config_load_callback_t callback);

    /**
     * @brief 注册保存回调
     * @param category 配置类别
     * @param callback 回调函数
     * @param user_data 用户数据
     * @return 注册结果
     */
    config_result_t (*register_save_callback)(config_category_t category, config_save_callback_t callback,
                                             void *user_data);

    /**
     * @brief 注销保存回调
     * @param category 配置类别
     * @param callback 回调函数
     * @return 注销结果
     */
    config_result_t (*unregister_save_callback)(config_category_t category, config_save_callback_t callback);

    /**
     * @brief 检查配置是否已修改
     * @param category 配置类别
     * @param modified 输出是否已修改标志
     * @return 检查结果
     */
    config_result_t (*is_modified)(config_category_t category, bool *modified);

    /**
     * @brief 标记配置为已修改
     * @param category 配置类别
     * @return 标记结果
     */
    config_result_t (*mark_modified)(config_category_t category);

    /**
     * @brief 清除修改标志
     * @param category 配置类别
     * @return 清除结果
     */
    config_result_t (*clear_modified)(config_category_t category);

    /**
     * @brief 同步配置到存储
     * @param category 配置类别
     * @return 同步结果
     */
    config_result_t (*sync_config)(config_category_t category);

    /**
     * @brief 获取配置统计信息
     * @param category 配置类别
     * @param item_count 输出配置项数量
     * @param modified_count 输出已修改数量
     * @return 获取结果
     */
    config_result_t (*get_statistics)(config_category_t category, uint16_t *item_count,
                                       uint16_t *modified_count);

    /**
     * @brief 获取配置管理器版本
     * @return 版本号
     */
    uint32_t (*get_version)(void);
} config_manager_interface_t;

/* 全局配置管理器实例 */
extern const config_manager_interface_t *config_manager;

/* 常用配置键定义 */
#define CONFIG_KEY_SYSTEM_LANGUAGE            "system.language"
#define CONFIG_KEY_SYSTEM_TIMEZONE            "system.timezone"
#define CONFIG_KEY_SYSTEM_DEBUG_MODE          "system.debug_mode"
#define CONFIG_KEY_HARDWARE_PLATFORM          "hardware.platform"
#define CONFIG_KEY_HARDWARE_REVISION          "hardware.revision"
#define CONFIG_KEY_KEYBOARD_LAYOUT            "keyboard.layout"
#define CONFIG_KEY_KEYBOARD_DEBOUNCE_MS       "keyboard.debounce_ms"
#define CONFIG_KEY_KEYBOARD_MATRIX_ROWS       "keyboard.matrix_rows"
#define CONFIG_KEY_KEYBOARD_MATRIX_COLS       "keyboard.matrix_cols"
#define CONFIG_KEY_KEYBOARD_ENABLE_rollover   "keyboard.enable_rollover"
#define CONFIG_KEY_TOUCHPAD_ENABLE            "touchpad.enable"
#define CONFIG_KEY_TOUCHPAD_SENSITIVITY       "touchpad.sensitivity"
#define CONFIG_KEY_TOUCHPAD_ENABLE_gesture    "touchpad.enable_gesture"
#define CONFIG_KEY_LED_BRIGHTNESS             "led.brightness"
#define CONFIG_KEY_LED_EFFECT                 "led.effect"
#define CONFIG_KEY_BACKLIGHT_LEVEL            "backlight.level"
#define CONFIG_KEY_BACKLIGHT_AUTO_DIM         "backlight.auto_dim"
#define CONFIG_KEY_COMMUNICATION_MODE         "communication.mode"
#define CONFIG_KEY_COMMUNICATION_BLE_PAIRING "communication.ble_pairing"
#define CONFIG_KEY_POWER_BATTERY_LOW_THRESHOLD "power.battery_low_threshold"
#define CONFIG_KEY_POWER_SLEEP_TIMEOUT        "power.sleep_timeout"
#define CONFIG_KEY_USER_PROFILE_ACTIVE        "user.profile.active"
#define CONFIG_KEY_USER_NAME                  "user.name"

/* 常用配置宏 */
#define CONFIG_GET_BOOL(key, default_val) \
    ({ \
        bool _val = (default_val); \
        config_manager->get_value((key), CONFIG_TYPE_BOOL, &_val, sizeof(_val)); \
        _val; \
    })

#define CONFIG_GET_UINT8(key, default_val) \
    ({ \
        uint8_t _val = (default_val); \
        config_manager->get_value((key), CONFIG_TYPE_UINT8, &_val, sizeof(_val)); \
        _val; \
    })

#define CONFIG_GET_UINT16(key, default_val) \
    ({ \
        uint16_t _val = (default_val); \
        config_manager->get_value((key), CONFIG_TYPE_UINT16, &_val, sizeof(_val)); \
        _val; \
    })

#define CONFIG_GET_UINT32(key, default_val) \
    ({ \
        uint32_t _val = (default_val); \
        config_manager->get_value((key), CONFIG_TYPE_UINT32, &_val, sizeof(_val)); \
        _val; \
    })

#define CONFIG_GET_STRING(key, buffer, size, default_val) \
    do { \
        strncpy((buffer), (default_val), (size)); \
        config_manager->get_value((key), CONFIG_TYPE_STRING, (buffer), (size)); \
    } while(0)

#define CONFIG_SET_BOOL(key, val) \
    config_manager->set_value((key), CONFIG_TYPE_BOOL, &(val), sizeof(val))

#define CONFIG_SET_UINT8(key, val) \
    config_manager->set_value((key), CONFIG_TYPE_UINT8, &(val), sizeof(val))

#define CONFIG_SET_UINT16(key, val) \
    config_manager->set_value((key), CONFIG_TYPE_UINT16, &(val), sizeof(val))

#define CONFIG_SET_UINT32(key, val) \
    config_manager->set_value((key), CONFIG_TYPE_UINT32, &(val), sizeof(val))

#define CONFIG_SET_STRING(key, val) \
    config_manager->set_value((key), CONFIG_TYPE_STRING, (val), strlen(val) + 1)

/* 配置错误处理宏 */
#define CONFIG_CHECK(expr) \
    do { \
        config_result_t _result = (expr); \
        if (_result != CONFIG_OK) { \
            return _result; \
        } \
    } while(0)

#define CONFIG_CHECK_RET(expr, ret_val) \
    do { \
        config_result_t _result = (expr); \
        if (_result != CONFIG_OK) { \
            return (ret_val); \
        } \
    } while(0)

/* 便捷操作宏 */
#define CONFIG_LOAD_ALL() \
    config_manager->load_all_configs()

#define CONFIG_SAVE_ALL() \
    config_manager->save_all_configs()

#define CONFIG_VALIDATE_ALL() \
    config_manager->validate_all_configs()

#define CONFIG_RESET_ALL() \
    config_manager->reset_all_to_default()

#define CONFIG_SYNC_ALL() \
    do { \
        config_category_t _categories[] = { \
            CONFIG_CATEGORY_SYSTEM, CONFIG_CATEGORY_HARDWARE, CONFIG_CATEGORY_KEYBOARD, \
            CONFIG_CATEGORY_TOUCHPAD, CONFIG_CATEGORY_LED, CONFIG_CATEGORY_BACKLIGHT, \
            CONFIG_CATEGORY_GESTURE, CONFIG_CATEGORY_COMMUNICATION, CONFIG_CATEGORY_POWER, \
            CONFIG_CATEGORY_USER, CONFIG_CATEGORY_PROFILE \
        }; \
        for (size_t i = 0; i < sizeof(_categories)/sizeof(_categories[0]); i++) { \
            config_manager->sync_config(_categories[i]); \
        } \
    } while(0)

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_MANAGER_H */