/**
 * @file config_validator.h
 * @brief 配置验证器接口定义
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * 提供配置数据验证、范围检查、类型验证等功能
 */

#ifndef CONFIG_VALIDATOR_H
#define CONFIG_VALIDATOR_H

#include <stdint.h>
#include <stdbool.h>
#include "config_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 配置验证器错误码定义 */
typedef enum {
    CONFIG_VALIDATOR_OK = 0,               /* 验证成功 */
    CONFIG_VALIDATOR_ERROR_INVALID_PARAM,   /* 无效参数 */
    CONFIG_VALIDATOR_ERROR_NOT_SUPPORTED,   /* 操作不支持 */
    CONFIG_VALIDATOR_ERROR_TYPE_MISMATCH,   /* 类型不匹配 */
    CONFIG_VALIDATOR_ERROR_RANGE_VIOLATION, /* 范围违规 */
    CONFIG_VALIDATOR_ERROR_VALUE_INVALID,   /* 值无效 */
    CONFIG_VALIDATOR_ERROR_FORMAT_INVALID,  /* 格式无效 */
    CONFIG_VALIDATOR_ERROR_CHECKSUM_FAILED, /* 校验失败 */
    CONFIG_VALIDATOR_ERROR_DEPENDENCY_FAILED, /* 依赖验证失败 */
    CONFIG_VALIDATOR_ERROR_CONSTRAINT_FAILED, /* 约束验证失败 */
    CONFIG_VALIDATOR_ERROR_SCHEMA_FAILED,    /* 架构验证失败 */
    CONFIG_VALIDATOR_ERROR_REFERENCE_INVALID, /* 引用无效 */
    CONFIG_VALIDATOR_ERROR_EXPRESSION_FAILED, /* 表达式验证失败 */
    CONFIG_VALIDATOR_ERROR_MEMORY_ALLOCATION, /* 内存分配失败 */
    CONFIG_VALIDATOR_ERROR_TIMEOUT,           /* 验证超时 */
    CONFIG_VALIDATOR_ERROR_ABORTED            /* 验证中断 */
} config_validator_result_t;

/* 验证规则类型定义 */
typedef enum {
    VALIDATOR_RULE_TYPE_REQUIRED = 0,      /* 必需规则 */
    VALIDATOR_RULE_TYPE_OPTIONAL,          /* 可选规则 */
    VALIDATOR_RULE_TYPE_CONDITIONAL,       /* 条件规则 */
    VALIDATOR_RULE_TYPE_TYPE_CHECK,         /* 类型检查 */
    VALIDATOR_RULE_TYPE_RANGE_CHECK,       /* 范围检查 */
    VALIDATOR_RULE_TYPE_FORMAT_CHECK,       /* 格式检查 */
    VALIDATOR_RULE_TYPE_LENGTH_CHECK,       /* 长度检查 */
    VALIDATOR_RULE_TYPE_ENUM_CHECK,        /* 枚举检查 */
    VALIDATOR_RULE_TYPE_REGEX_CHECK,       /* 正则检查 */
    VALIDATOR_RULE_TYPE_CUSTOM,            /* 自定义检查 */
    VALIDATOR_RULE_TYPE_DEPENDENCY,        /* 依赖检查 */
    VALIDATOR_RULE_TYPE_CONSTRAINT,        /* 约束检查 */
    VALIDATOR_RULE_TYPE_SCHEMA,            /* 架构检查 */
    VALIDATOR_RULE_TYPE_EXPRESSION         /* 表达式检查 */
} validator_rule_type_t;

/* 验证操作符定义 */
typedef enum {
    VALIDATOR_OP_EQUAL = 0,               /* 等于 */
    VALIDATOR_OP_NOT_EQUAL,                /* 不等于 */
    VALIDATOR_OP_GREATER_THAN,             /* 大于 */
    VALIDATOR_OP_GREATER_EQUAL,            /* 大于等于 */
    VALIDATOR_OP_LESS_THAN,                /* 小于 */
    VALIDATOR_OP_LESS_EQUAL,               /* 小于等于 */
    VALIDATOR_OP_CONTAINS,                 /* 包含 */
    VALIDATOR_OP_NOT_CONTAINS,             /* 不包含 */
    VALIDATOR_OP_STARTS_WITH,              /* 开始于 */
    VALIDATOR_OP_ENDS_WITH,                /* 结束于 */
    VALIDATOR_OP_MATCHES,                  /* 匹配 */
    VALIDATOR_OP_IN,                       /* 在范围内 */
    VALIDATOR_OP_NOT_IN,                   /* 不在范围内 */
    VALIDATOR_OP_IS_NULL,                  /* 为空 */
    VALIDATOR_OP_IS_NOT_NULL,              /* 不为空 */
    VALIDATOR_OP_IS_EMPTY,                 /* 为空字符串 */
    VALIDATOR_OP_IS_NOT_EMPTY,             /* 不为空字符串 */
    VALIDATOR_OP_IS_TRUE,                  /* 为真 */
    VALIDATOR_OP_IS_FALSE,                 /* 为假 */
    VALIDATOR_OP_CUSTOM                    /* 自定义操作 */
} validator_operator_t;

/* 验证严重级别定义 */
typedef enum {
    VALIDATOR_SEVERITY_INFO = 0,          /* 信息级别 */
    VALIDATOR_SEVERITY_WARNING,            /* 警告级别 */
    VALIDATOR_SEVERITY_ERROR,              /* 错误级别 */
    VALIDATOR_SEVERITY_CRITICAL,           /* 关键级别 */
    VALIDATOR_SEVERITY_FATAL               /* 致命级别 */
} validator_severity_t;

/* 验证条件类型定义 */
typedef enum {
    VALIDATOR_CONDITION_AND = 0,          /* AND条件 */
    VALIDATOR_CONDITION_OR,               /* OR条件 */
    VALIDATOR_CONDITION_NOT,              /* NOT条件 */
    VALIDATOR_CONDITION_XOR,              /* XOR条件 */
    VALIDATOR_CONDITION_IMPLIES,          /* 蕴含条件 */
    VALIDATOR_CONDITION_IF_THEN,          /* IF-THEN条件 */
    VALIDATOR_CONDITION_SWITCH,           /* SWITCH条件 */
    VALIDATOR_CONDITION_CASE,            /* CASE条件 */
    VALIDATOR_CONDITION_DEFAULT          /* DEFAULT条件 */
} validator_condition_type_t;

/* 验证值类型定义 */
typedef enum {
    VALIDATOR_VALUE_TYPE_CONSTANT = 0,    /* 常量值 */
    VALIDATOR_VALUE_TYPE_REFERENCE,       /* 引用值 */
    VALIDATOR_VALUE_TYPE_EXPRESSION,      /* 表达式值 */
    VALIDATOR_VALUE_TYPE_FUNCTION,        /* 函数值 */
    VALIDATOR_VALUE_TYPE_ARRAY,           /* 数组值 */
    VALIDATOR_VALUE_TYPE_RANGE,           /* 范围值 */
    VALIDATOR_VALUE_TYPE_PATTERN,         /* 模式值 */
    VALIDATOR_VALUE_TYPE_VARIABLE         /* 变量值 */
} validator_value_type_t;

/* 验证规则结构体 */
typedef struct {
    validator_rule_type_t    type;           /* 规则类型 */
    const char              *name;           /* 规则名称 */
    const char              *description;    /* 规则描述 */
    validator_operator_t     operator;       /* 验证操作符 */
    validator_value_type_t   value_type;     /* 值类型 */
    config_data_type_t       expected_type;  /* 期望数据类型 */
    union {
        bool                bool_val;        /* 布尔值 */
        int32_t             int32_val;       /* 32位整数值 */
        uint32_t            uint32_val;      /* 32位无符号整数值 */
        float               float_val;       /* 浮点数值 */
        const char         *string_val;     /* 字符串值 */
        const void         *binary_val;     /* 二进制值 */
        const int32_t      *int_array_val;  /* 整数数组值 */
        const char        **string_array_val; /* 字符串数组值 */
    } expected_value;
    union {
        struct {
            int32_t         min_value;       /* 最小值 */
            int32_t         max_value;       /* 最大值 */
        } int_range;
        struct {
            uint32_t        min_value;       /* 最小值 */
            uint32_t        max_value;       /* 最大值 */
        } uint_range;
        struct {
            float           min_value;       /* 最小值 */
            float           max_value;       /* 最大值 */
        } float_range;
        struct {
            uint16_t        min_length;      /* 最小长度 */
            uint16_t        max_length;      /* 最大长度 */
        } length_range;
        struct {
            const char     *pattern;        /* 正则表达式 */
            bool            case_sensitive;  /* 区分大小写 */
            bool            multiline;       /* 多行模式 */
        } regex;
        struct {
            uint32_t        enum_count;     /* 枚举值数量 */
            const int32_t  *enum_values;    /* 枚举值数组 */
        } enum_values;
        struct {
            const char     *format_string;  /* 格式化字符串 */
            const char     *example;        /* 示例 */
        } format;
    } rule_params;
    validator_severity_t    severity;       /* 严重级别 */
    bool                    enabled;        /* 启用标志 */
    uint32_t                priority;       /* 优先级 */
} validator_rule_t;

/* 验证条件结构体 */
typedef struct {
    validator_condition_type_t type;        /* 条件类型 */
    uint8_t                 condition_count; /* 条件数量 */
    const validator_rule_t **conditions;    /* 条件数组 */
    const char             *condition_key;  /* 条件键名 */
    config_value_t          condition_value; /* 条件值 */
    config_data_type_t      condition_type;  /* 条件类型 */
} validator_condition_t;

/* 验证约束结构体 */
typedef struct {
    const char             *constraint_name;     /* 约束名称 */
    const char             *constraint_desc;     /* 约束描述 */
    const validator_rule_t **rules;              /* 规则数组 */
    uint8_t                 rule_count;         /* 规则数量 */
    const validator_condition_t *condition;      /* 约束条件 */
    validator_severity_t    severity;           /* 严重级别 */
    bool                    enabled;            /* 启用标志 */
    uint32_t                priority;           /* 优先级 */
} validator_constraint_t;

/* 验证错误结构体 */
typedef struct {
    validator_severity_t    severity;           /* 错误严重级别 */
    const char             *error_code;         /* 错误代码 */
    const char             *error_message;      /* 错误消息 */
    const char             *config_key;         /* 配置键名 */
    const char             *rule_name;          /* 违反的规则名称 */
    const void             *actual_value;       /* 实际值 */
    config_data_type_t      value_type;         /* 值类型 */
    const void             *expected_value;     /* 期望值 */
    uint32_t                line_number;        /* 行号(如果有) */
    uint32_t                column_number;      /* 列号(如果有) */
    uint32_t                timestamp;          /* 错误时间戳 */
    const char             *suggestion;         /* 修复建议 */
} validator_error_t;

/* 验证报告结构体 */
typedef struct {
    uint32_t                total_items;        /* 总验证项数 */
    uint32_t                valid_items;        /* 有效项数 */
    uint32_t                invalid_items;      /* 无效项数 */
    uint32_t                warning_items;      /* 警告项数 */
    uint32_t                error_items;        /* 错误项数 */
    uint32_t                critical_items;     /* 关键项数 */
    uint32_t                fatal_items;        /* 致命项数 */
    uint32_t                skipped_items;      /* 跳过项数 */
    uint32_t                error_count;        /* 错误数量 */
    const validator_error_t *errors;             /* 错误数组 */
    uint32_t                validate_time_ms;   /* 验证耗时(毫秒) */
    bool                    success;            /* 验证是否成功 */
    char                    summary[256];       /* 验证摘要 */
} validator_report_t;

/* 验证上下文结构体 */
typedef struct {
    config_category_t       category;           /* 配置类别 */
    const char             *config_version;     /* 配置版本 */
    const char             *schema_version;    /* 架构版本 */
    const validator_rule_t **global_rules;      /* 全局规则 */
    uint8_t                 global_rule_count;  /* 全局规则数量 */
    const validator_constraint_t **constraints; /* 约束数组 */
    uint8_t                 constraint_count;   /* 约束数量 */
    bool                    strict_mode;        /* 严格模式 */
    bool                    allow_unknown_keys; /* 允许未知键 */
    bool                    auto_fix_errors;    /* 自动修复错误 */
    uint32_t                timeout_ms;         /* 验证超时(毫秒) */
    uint32_t                max_errors;         /* 最大错误数 */
    void                   *user_data;          /* 用户数据 */
} validator_context_t;

/* 验证回调函数类型 */
typedef void (*validator_progress_callback_t)(const char *config_key,
                                               uint32_t validated_items,
                                               uint32_t total_items,
                                               void *user_data);

typedef void (*validator_error_callback_t)(const validator_error_t *error,
                                            void *user_data);

typedef bool (*validator_custom_check_t)(const char *config_key,
                                         const config_value_t *value,
                                         config_data_type_t type,
                                         void *user_data);

/* 配置验证器接口函数指针结构体 */
typedef struct {
    /**
     * @brief 初始化配置验证器
     * @return 初始化结果
     */
    config_validator_result_t (*init)(void);

    /**
     * @brief 反初始化配置验证器
     * @return 反初始化结果
     */
    config_validator_result_t (*deinit)(void);

    /**
     * @brief 注册验证规则
     * @param rule 验证规则
     * @return 注册结果
     */
    config_validator_result_t (*register_rule)(const validator_rule_t *rule);

    /**
     * @brief 注销验证规则
     * @param rule_name 规则名称
     * @return 注销结果
     */
    config_validator_result_t (*unregister_rule)(const char *rule_name);

    /**
     * @brief 注册验证约束
     * @param constraint 验证约束
     * @return 注册结果
     */
    config_validator_result_t (*register_constraint)(const validator_constraint_t *constraint);

    /**
     * @brief 注销验证约束
     * @param constraint_name 约束名称
     * @return 注销结果
     */
    config_validator_result_t (*unregister_constraint)(const char *constraint_name);

    /**
     * @brief 注册自定义验证函数
     * @param rule_name 规则名称
     * @param check_func 验证函数
     * @return 注册结果
     */
    config_validator_result_t (*register_custom_check)(const char *rule_name,
                                                        validator_custom_check_t check_func);

    /**
     * @brief 注销自定义验证函数
     * @param rule_name 规则名称
     * @return 注销结果
     */
    config_validator_result_t (*unregister_custom_check)(const char *rule_name);

    /**
     * @brief 验证配置项
     * @param config_key 配置键名
     * @param value 配置值
     * @param type 数据类型
     * @param context 验证上下文
     * @param report 输出验证报告
     * @return 验证结果
     */
    config_validator_result_t (*validate_item)(const char *config_key,
                                                  const config_value_t *value,
                                                  config_data_type_t type,
                                                  const validator_context_t *context,
                                                  validator_report_t *report);

    /**
     * @brief 验证配置区域
     * @param category 配置类别
     * @param context 验证上下文
     * @param report 输出验证报告
     * @return 验证结果
     */
    config_validator_result_t (*validate_category)(config_category_t category,
                                                      const validator_context_t *context,
                                                      validator_report_t *report);

    /**
     * @brief 验证所有配置
     * @param context 验证上下文
     * @param report 输出验证报告
     * @return 验证结果
     */
    config_validator_result_t (*validate_all)(const validator_context_t *context,
                                               validator_report_t *report);

    /**
     * @brief 快速验证配置
     * @param config_key 配置键名
     * @param value 配置值
     * @param type 数据类型
     * @return 验证结果
     */
    config_validator_result_t (*quick_validate)(const char *config_key,
                                                   const config_value_t *value,
                                                   config_data_type_t type);

    /**
     * @brief 批量验证配置
     * @param items 配置项数组
     * @param item_count 配置项数量
     * @param context 验证上下文
     * @param report 输出验证报告
     * @return 验证结果
     */
    config_validator_result_t (*batch_validate)(const config_item_t *items,
                                                   uint16_t item_count,
                                                   const validator_context_t *context,
                                                   validator_report_t *report);

    /**
     * @brief 异步验证配置
     * @param category 配置类别
     * @param context 验证上下文
     * @param progress_callback 进度回调函数
     * @param error_callback 错误回调函数
     * @param user_data 用户数据
     * @return 验证结果
     */
    config_validator_result_t (*async_validate)(config_category_t category,
                                                   const validator_context_t *context,
                                                   validator_progress_callback_t progress_callback,
                                                   validator_error_callback_t error_callback,
                                                   void *user_data);

    /**
     * @brief 检查验证是否完成
     * @param completed 输出是否完成标志
     * @return 检查结果
     */
    config_validator_result_t (*is_validation_complete)(bool *completed);

    /**
     * @brief 等待验证完成
     * @param timeout_ms 超时时间(毫秒)
     * @return 等待结果
     */
    config_validator_result_t (*wait_validation_complete)(uint32_t timeout_ms);

    /**
     * @brief 取消异步验证
     * @return 取消结果
     */
    config_validator_result_t (*cancel_validation)(void);

    /**
     * @brief 获取验证进度
     * @param validated_items 输出已验证项数
     * @param total_items 输出总项数
     * @param progress_percent 输出进度百分比
     * @return 获取结果
     */
    config_validator_result_t (*get_validation_progress)(uint32_t *validated_items,
                                                           uint32_t *total_items,
                                                           float *progress_percent);

    /**
     * @brief 获取最后验证报告
     * @param report 输出验证报告
     * @return 获取结果
     */
    config_validator_result_t (*get_last_report)(validator_report_t *report);

    /**
     * @brief 清除验证报告
     * @return 清除结果
     */
    config_validator_result_t (*clear_report)(void);

    /**
     * @brief 导出验证报告
     * @param format 导出格式
     * @param buffer 输出缓冲区
     * @param buffer_size 缓冲区大小
     * @param exported_size 输出导出大小
     * @return 导出结果
     */
    config_validator_result_t (*export_report)(config_format_t format,
                                                  void *buffer,
                                                  uint32_t buffer_size,
                                                  uint32_t *exported_size);

    /**
     * @brief 自动修复验证错误
     * @param report 验证报告
     * @param fixed_report 输出修复后报告
     * @return 修复结果
     */
    config_validator_result_t (*auto_fix_errors)(const validator_report_t *report,
                                                    validator_report_t *fixed_report);

    /**
     * @brief 获取已注册规则数量
     * @return 规则数量
     */
    uint16_t (*get_rule_count)(void);

    /**
     * @brief 获取已注册约束数量
     * @return 约束数量
     */
    uint8_t (*get_constraint_count)(void);

    /**
     * @brief 枚举验证规则
     * @param callback 枚举回调函数
     * @param user_data 用户数据
     * @return 枚举结果
     */
    config_validator_result_t (*enumerate_rules)(void (*callback)(const validator_rule_t *rule,
                                                                  void *user_data),
                                                   void *user_data);

    /**
     * @brief 枚举验证约束
     * @param callback 枚举回调函数
     * @param user_data 用户数据
     * @return 枚举结果
     */
    config_validator_result_t (*enumerate_constraints)(void (*callback)(const validator_constraint_t *constraint,
                                                                        void *user_data),
                                                        void *user_data);

    /**
     * @brief 查找验证规则
     * @param rule_name 规则名称
     * @param rule 输出规则指针
     * @return 查找结果
     */
    config_validator_result_t (*find_rule)(const char *rule_name,
                                              const validator_rule_t **rule);

    /**
     * @brief 查找验证约束
     * @param constraint_name 约束名称
     * @param constraint 输出约束指针
     * @return 查找结果
     */
    config_validator_result_t (*find_constraint)(const char *constraint_name,
                                                    const validator_constraint_t **constraint);

    /**
     * @brief 设置验证器配置
     * @param context 验证上下文
     * @return 设置结果
     */
    config_validator_result_t (*set_context)(const validator_context_t *context);

    /**
     * @brief 获取验证器配置
     * @param context 输出验证上下文
     * @return 获取结果
     */
    config_validator_result_t (*get_context)(validator_context_t *context);

    /**
     * @brief 重置验证器状态
     * @return 重置结果
     */
    config_validator_result_t (*reset)(void);

    /**
     * @brief 获取验证器统计信息
     * @param total_validations 输出总验证次数
     * @param successful_validations 输出成功验证次数
     * @param failed_validations 输出失败验证次数
     * @param total_errors 输出总错误数
     * @return 获取结果
     */
    config_validator_result_t (*get_statistics)(uint32_t *total_validations,
                                                   uint32_t *successful_validations,
                                                   uint32_t *failed_validations,
                                                   uint32_t *total_errors);

    /**
     * @brief 重置验证器统计信息
     * @return 重置结果
     */
    config_validator_result_t (*reset_statistics)(void);

    /**
     * @brief 获取配置验证器版本
     * @return 版本号
     */
    uint32_t (*get_version)(void);
} config_validator_interface_t;

/* 全局配置验证器实例 */
extern const config_validator_interface_t *config_validator;

/* 常用验证规则宏 */
#define VALIDATOR_RULE_REQUIRED(key) \
    { VALIDATOR_RULE_TYPE_REQUIRED, (key), "Required field", VALIDATOR_OP_IS_NOT_NULL, \
      VALIDATOR_VALUE_TYPE_CONSTANT, CONFIG_TYPE_NONE, { .bool_val = true }, {0}, \
      VALIDATOR_SEVERITY_ERROR, true, 100 }

#define VALIDATOR_RULE_TYPE_CHECK(key, type) \
    { VALIDATOR_RULE_TYPE_TYPE_CHECK, (key), "Type check", VALIDATOR_OP_EQUAL, \
      VALIDATOR_VALUE_TYPE_CONSTANT, (type), { .uint32_val = (type) }, {0}, \
      VALIDATOR_SEVERITY_ERROR, true, 100 }

#define VALIDATOR_RULE_RANGE_CHECK(key, min_val, max_val) \
    { VALIDATOR_RULE_TYPE_RANGE_CHECK, (key), "Range check", VALIDATOR_OP_IN, \
      VALIDATOR_VALUE_TYPE_RANGE, CONFIG_TYPE_UINT32, {0}, \
      { .int_range = { (min_val), (max_val) } }, VALIDATOR_SEVERITY_ERROR, true, 90 }

#define VALIDATOR_RULE_LENGTH_CHECK(key, min_len, max_len) \
    { VALIDATOR_RULE_TYPE_LENGTH_CHECK, (key), "Length check", VALIDATOR_OP_IN, \
      VALIDATOR_VALUE_TYPE_RANGE, CONFIG_TYPE_STRING, {0}, \
      { .length_range = { (min_len), (max_len) } }, VALIDATOR_SEVERITY_ERROR, true, 90 }

#define VALIDATOR_RULE_ENUM_CHECK(key, values, count) \
    { VALIDATOR_RULE_TYPE_ENUM_CHECK, (key), "Enum check", VALIDATOR_OP_IN, \
      VALIDATOR_VALUE_TYPE_ARRAY, CONFIG_TYPE_INT32, {0}, \
      { .enum_values = { (count), (values) } }, VALIDATOR_SEVERITY_ERROR, true, 90 }

#define VALIDATOR_RULE_REGEX_CHECK(key, pattern, case_sensitive) \
    { VALIDATOR_RULE_TYPE_REGEX_CHECK, (key), "Regex check", VALIDATOR_OP_MATCHES, \
      VALIDATOR_VALUE_TYPE_PATTERN, CONFIG_TYPE_STRING, { .string_val = (pattern) }, \
      { .regex = { (pattern), (case_sensitive), false } }, VALIDATOR_SEVERITY_WARNING, true, 80 }

#define VALIDATOR_RULE_FORMAT_CHECK(key, format, example) \
    { VALIDATOR_RULE_TYPE_FORMAT_CHECK, (key), "Format check", VALIDATOR_OP_MATCHES, \
      VALIDATOR_VALUE_TYPE_PATTERN, CONFIG_TYPE_STRING, { .string_val = (format) }, \
      { .format = { (format), (example) } }, VALIDATOR_SEVERITY_WARNING, true, 80 }

/* 验证上下文宏 */
#define VALIDATOR_CONTEXT_DEFAULT(category) \
    { (category), "1.0", "1.0", NULL, 0, NULL, 0, true, false, false, 10000, 100, NULL }

#define VALIDATOR_CONTEXT_STRICT(category) \
    { (category), "1.0", "1.0", NULL, 0, NULL, 0, true, false, true, 10000, 100, NULL }

#define VALIDATOR_CONTEXT_LENIENT(category) \
    { (category), "1.0", "1.0", NULL, 0, NULL, 0, false, true, false, 30000, 1000, NULL }

/* 配置验证错误处理宏 */
#define CONFIG_VALIDATOR_CHECK(expr) \
    do { \
        config_validator_result_t _result = (expr); \
        if (_result != CONFIG_VALIDATOR_OK) { \
            return _result; \
        } \
    } while(0)

#define CONFIG_VALIDATOR_CHECK_RET(expr, ret_val) \
    do { \
        config_validator_result_t _result = (expr); \
        if (_result != CONFIG_VALIDATOR_OK) { \
            return (ret_val); \
        } \
    } while(0)

/* 便捷操作宏 */
#define CONFIG_VALIDATE_ITEM(key, value, type) \
    config_validator->validate_item((key), (value), (type), NULL, NULL)

#define CONFIG_VALIDATE_CATEGORY(category) \
    config_validator->validate_category((category), NULL, NULL)

#define CONFIG_VALIDATE_ALL() \
    config_validator->validate_all(NULL, NULL)

#define CONFIG_QUICK_VALIDATE(key, value, type) \
    config_validator->quick_validate((key), (value), (type))

#define CONFIG_ASYNC_VALIDATE(category, progress_cb, error_cb, user_data) \
    config_validator->async_validate((category), NULL, (progress_cb), (error_cb), (user_data))

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_VALIDATOR_H */