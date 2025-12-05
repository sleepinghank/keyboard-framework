/**
 * @file report_builder.h
 * @brief HID报告构建器接口定义
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * 提供HID报告构建、组装、序列化等功能
 */

#ifndef REPORT_BUILDER_H
#define REPORT_BUILDER_H

#include <stdint.h>
#include <stdbool.h>
#include "hid_processor.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 报告构建器错误码定义 */
typedef enum {
    REPORT_BUILDER_OK = 0,                /* 构建成功 */
    REPORT_BUILDER_ERROR_INVALID_PARAM,   /* 无效参数 */
    REPORT_BUILDER_ERROR_NOT_SUPPORTED,   /* 操作不支持 */
    REPORT_BUILDER_ERROR_BUFFER_OVERFLOW,/* 缓冲区溢出 */
    REPORT_BUILDER_ERROR_REPORT_TOO_LARGE,/* 报告太大 */
    REPORT_BUILDER_ERROR_INVALID_FORMAT, /* 无效格式 */
    REPORT_BUILDER_ERROR_INVALID_ID,     /* 无效报告ID */
    REPORT_BUILDER_ERROR_INVALID_TYPE,   /* 无效报告类型 */
    REPORT_BUILDER_ERROR_INVALID_DATA,   /* 无效数据 */
    REPORT_BUILDER_ERROR_CONFLICT,       /* 数据冲突 */
    REPORT_BUILDER_ERROR_SEQUENCE,       /* 序列错误 */
    REPORT_BUILDER_ERROR_TIMEOUT,        /* 构建超时 */
    REPORT_BUILDER_ERROR_NOT_INITIALIZED,/* 未初始化 */
    REPORT_BUILDER_ERROR_ALREADY_INITIALIZED, /* 已初始化 */
    REPORT_BUILDER_ERROR_INSUFFICIENT_DATA, /* 数据不足 */
    REPORT_BUILDER_ERROR_CHECKSUM_FAILED, /* 校验失败 */
    REPORT_BUILDER_ERROR_PROTOCOL_VIOLATION /* 协议违规 */
} report_builder_result_t;

/* 报告构建模式定义 */
typedef enum {
    REPORT_BUILD_MODE_SINGLE = 0,         /* 单报告模式 */
    REPORT_BUILD_MODE_BATCH,              /* 批量报告模式 */
    REPORT_BUILD_MODE_STREAM,             /* 流式报告模式 */
    REPORT_BUILD_MODE_BUFFERED,           /* 缓冲报告模式 */
    REPORT_BUILD_MODE_COMPRESSED,         /* 压缩报告模式 */
    REPORT_BUILD_MODE_FRAGMENTED,          /* 分片报告模式 */
    REPORT_BUILD_MODE_ENCRYPTED           /* 加密报告模式 */
} report_build_mode_t;

/* 报告序列化格式定义 */
typedef enum {
    REPORT_FORMAT_RAW = 0,                /* 原始格式 */
    REPORT_FORMAT_HID,                    /* HID格式 */
    REPORT_FORMAT_USB_HID,                /* USB HID格式 */
    REPORT_FORMAT_BLE_HID,                /* BLE HID格式 */
    REPORT_FORMAT_BT_HID,                 /* 蓝牙HID格式 */
    REPORT_FORMAT_CUSTOM,                 /* 自定义格式 */
    REPORT_FORMAT_JSON,                    /* JSON格式 */
    REPORT_FORMAT_BINARY,                  /* 二进制格式 */
    REPORT_FORMAT_COMPRESSED               /* 压缩格式 */
} report_format_t;

/* 报告优先级定义 */
typedef enum {
    REPORT_PRIORITY_LOW = 0,              /* 低优先级 */
    REPORT_PRIORITY_NORMAL,               /* 普通优先级 */
    REPORT_PRIORITY_HIGH,                 /* 高优先级 */
    REPORT_PRIORITY_CRITICAL,             /* 关键优先级 */
    REPORT_PRIORITY_EMERGENCY             /* 紧急优先级 */
} report_priority_t;

/* 报告标志定义 */
typedef enum {
    REPORT_FLAG_NONE = 0x00,             /* 无标志 */
    REPORT_FLAG_IMMEDIATE = 0x01,         /* 立即发送 */
    REPORT_FLAG_QUEUED = 0x02,           /* 队列发送 */
    REPORT_FLAG_RETRY = 0x04,             /* 重试发送 */
    REPORT_FLAG_ACK_REQUIRED = 0x08,      /* 需要确认 */
    REPORT_FLAG_COMPRESSED = 0x10,        /* 压缩标志 */
    REPORT_FLAG_ENCRYPTED = 0x20,         /* 加密标志 */
    REPORT_FLAG_SIGNED = 0x40,            /* 签名标志 */
    REPORT_FLAG_FRAGMETED = 0x80          /* 分片标志 */
} report_flag_t;

/* 报告状态定义 */
typedef enum {
    REPORT_STATE_EMPTY = 0,               /* 空状态 */
    REPORT_STATE_BUILDING,                /* 构建中 */
    REPORT_STATE_READY,                   /* 就绪状态 */
    REPORT_STATE_SENDING,                 /* 发送中 */
    REPORT_STATE_SENT,                    /* 已发送 */
    REPORT_STATE_ACKED,                   /* 已确认 */
    REPORT_STATE_TIMEOUT,                 /* 超时状态 */
    REPORT_STATE_ERROR,                   /* 错误状态 */
    REPORT_STATE_RETRYING,                /* 重试中 */
    REPORT_STATE_CANCELED                 /* 已取消 */
} report_state_t;

/* 报告数据类型定义 */
typedef enum {
    REPORT_DATA_TYPE_KEY = 0,             /* 按键数据 */
    REPORT_DATA_TYPE_MODIFIER,            /* 修饰键数据 */
    REPORT_DATA_TYPE_MOUSE,               /* 鼠标数据 */
    REPORT_DATA_TYPE_CONSUMER,            /* 消费者控制数据 */
    REPORT_DATA_TYPE_SYSTEM,              /* 系统控制数据 */
    REPORT_DATA_TYPE_GAMEPAD,             /* 游戏手柄数据 */
    REPORT_DATA_TYPE_JOYSTICK,            /* 摇杆数据 */
    REPORT_DATA_TYPE_DIGITIZER,           /* 数字化仪数据 */
    REPORT_DATA_TYPE_VENDOR,              /* 厂商特定数据 */
    REPORT_DATA_TYPE_RAW,                 /* 原始数据 */
    REPORT_DATA_TYPE_CUSTOM               /* 自定义数据 */
} report_data_type_t;

/* 报告构建器配置结构体 */
typedef struct {
    report_build_mode_t    mode;           /* 构建模式 */
    report_format_t        format;         /* 输出格式 */
    uint16_t                max_report_size;/* 最大报告大小 */
    uint16_t                buffer_size;    /* 缓冲区大小 */
    uint8_t                 report_queue_size;/* 报告队列大小 */
    report_priority_t       default_priority; /* 默认优先级 */
    bool                    enable_compression; /* 启用压缩 */
    bool                    enable_encryption; /* 启用加密 */
    bool                    enable_signing;   /* 启用签名 */
    bool                    enable_fragmentation; /* 启用分片 */
    bool                    enable_retry;     /* 启用重试 */
    uint8_t                 max_retry_count; /* 最大重试次数 */
    uint32_t                timeout_ms;      /* 超时时间(毫秒) */
    uint32_t                fragment_size;   /* 分片大小 */
    uint16_t                compression_level;/* 压缩级别 */
    uint8_t                 encryption_key[16]; /* 加密密钥 */
    uint8_t                 signing_key[16];   /* 签名密钥 */
    bool                    immediate_send;   /* 立即发送 */
    bool                    batch_mode;       /* 批量模式 */
    uint16_t                batch_timeout_ms; /* 批量超时(毫秒) */
    uint16_t                batch_max_count;  /* 批量最大数量 */
} report_builder_config_t;

/* 报告数据结构体 */
typedef struct {
    report_data_type_t     type;           /* 数据类型 */
    hid_usage_page_t        usage_page;     /* 用法页 */
    uint16_t                usage;          /* 用法 */
    uint32_t                value;          /* 数据值 */
    uint16_t                bit_offset;     /* 位偏移 */
    uint8_t                 bit_count;      /* 位数量 */
    int32_t                 logical_min;    /* 逻辑最小值 */
    int32_t                 logical_max;    /* 逻辑最大值 */
    bool                    is_absolute;    /* 是否绝对值 */
    bool                    is_relative;    /* 是否相对值 */
    bool                    is_array;       /* 是否数组 */
    bool                    is_variable;    /* 是否可变 */
    uint32_t                timestamp;      /* 时间戳 */
} report_data_t;

/* 报告片段结构体 */
typedef struct {
    uint8_t                 fragment_id;    /* 片段ID */
    uint8_t                 total_fragments;/* 总片段数 */
    uint8_t                 fragment_index; /* 片段索引 */
    uint16_t                fragment_size;  /* 片段大小 */
    uint16_t                data_offset;    /* 数据偏移 */
    uint32_t                checksum;       /* 校验和 */
    bool                    is_last;        /* 是否最后一片 */
    uint32_t                timestamp;      /* 时间戳 */
} report_fragment_t;

/* 报告头结构体 */
typedef struct {
    uint16_t                report_id;      /* 报告ID */
    hid_report_type_t       report_type;    /* 报告类型 */
    report_format_t         format;         /* 格式 */
    report_priority_t       priority;       /* 优先级 */
    report_flag_t          flags;           /* 标志 */
    uint16_t                total_size;      /* 总大小 */
    uint16_t                data_size;      /* 数据大小 */
    uint8_t                 fragment_count;  /* 分片数量 */
    uint32_t                sequence_number;/* 序列号 */
    uint32_t                timestamp;      /* 时间戳 */
    uint32_t                checksum;       /* 校验和 */
    uint8_t                 retry_count;    /* 重试次数 */
    report_state_t          state;          /* 状态 */
} report_header_t;

/* 报告结构体 */
typedef struct {
    report_header_t          header;         /* 报告头 */
    uint8_t                *data;           /* 数据缓冲区 */
    report_fragment_t      *fragments;      /* 分片数组 */
    uint32_t                creation_time;  /* 创建时间 */
    uint32_t                send_time;      /* 发送时间 */
    uint32_t                ack_time;       /* 确认时间 */
    void                   *user_data;      /* 用户数据 */
} report_t;

/* 报告构建上下文结构体 */
typedef struct {
    report_build_mode_t    mode;           /* 构建模式 */
    report_format_t        format;         /* 输出格式 */
    uint8_t                *buffer;         /* 构建缓冲区 */
    uint16_t                buffer_size;    /* 缓冲区大小 */
    uint16_t                data_offset;    /* 数据偏移 */
    uint16_t                bit_offset;     /* 位偏移 */
    uint16_t                report_count;   /* 报告数量 */
    report_t              *current_report; /* 当前报告 */
    bool                    building;       /* 构建标志 */
    bool                    auto_fragment;  /* 自动分片 */
    bool                    auto_compress;  /* 自动压缩 */
    bool                    auto_encrypt;   /* 自动加密 */
    uint32_t                start_timestamp;/* 开始时间戳 */
    uint32_t                timeout_ms;     /* 超时时间 */
    void                   *user_data;      /* 用户数据 */
} report_build_context_t;

/* 报告队列项结构体 */
typedef struct {
    report_t               *report;         /* 报告指针 */
    report_priority_t       priority;       /* 优先级 */
    uint32_t                enqueue_time;   /* 入队时间 */
    uint8_t                 retry_count;    /* 重试次数 */
    bool                    urgent;         /* 紧急标志 */
    void                   *next;           /* 下一个项 */
} report_queue_item_t;

/* 报告统计信息结构体 */
typedef struct {
    uint32_t                total_reports_built; /* 构建的总报告数 */
    uint32_t                successful_reports; /* 成功报告数 */
    uint32_t                failed_reports;      /* 失败报告数 */
    uint32_t                reports_sent;       /* 发送的报告数 */
    uint32_t                reports_acked;      /* 确认的报告数 */
    uint32_t                reports_timeout;    /* 超时的报告数 */
    uint32_t                reports_retry;      /* 重试的报告数 */
    uint32_t                reports_fragmented;  /* 分片的报告数 */
    uint32_t                reports_compressed;  /* 压缩的报告数 */
    uint32_t                reports_encrypted;   /* 加密的报告数 */
    uint32_t                bytes_sent;          /* 发送的字节数 */
    uint32_t                bytes_received;      /* 接收的字节数 */
    uint32_t                total_build_time_ms; /* 总构建时间(毫秒) */
    uint32_t                average_build_time_us; /* 平均构建时间(微秒) */
    uint32_t                max_build_time_us;   /* 最大构建时间(微秒) */
    uint32_t                queue_depth;         /* 队列深度 */
    uint32_t                queue_overflow_count; /* 队列溢出次数 */
    uint32_t                buffer_overflow_count; /* 缓冲区溢出次数 */
    uint32_t                checksum_errors;      /* 校验错误次数 */
    uint32_t                fragmentation_errors; /* 分片错误次数 */
    uint32_t                compression_errors;  /* 压缩错误次数 */
    uint32_t                encryption_errors;   /* 加密错误次数 */
    uint32_t                last_activity_time;  /* 最后活动时间 */
    uint32_t                uptime_seconds;      /* 运行时间(秒) */
} report_builder_statistics_t;

/* 报告构建回调函数类型 */
typedef void (*report_build_callback_t)(const report_t *report, void *user_data);

typedef void (*report_send_callback_t)(const report_t *report, bool success, void *user_data);

typedef void (*report_ack_callback_t)(const report_t *report, void *user_data);

typedef void (*report_error_callback_t)(report_builder_result_t error, const char *message, void *user_data);

typedef void (*report_progress_callback_t)(uint32_t bytes_processed, uint32_t total_bytes, void *user_data);

typedef bool (*report_filter_callback_t)(const report_t *report, void *user_data);

/* 报告构建器接口函数指针结构体 */
typedef struct {
    /**
     * @brief 初始化报告构建器
     * @param config 构建器配置
     * @return 初始化结果
     */
    report_builder_result_t (*init)(const report_builder_config_t *config);

    /**
     * @brief 反初始化报告构建器
     * @return 反初始化结果
     */
    report_builder_result_t (*deinit)(void);

    /**
     * @brief 开始构建报告
     * @param context 构建上下文
     * @return 开始结果
     */
    report_builder_result_t (*start_build)(report_build_context_t *context);

    /**
     * @brief 结束构建报告
     * @param context 构建上下文
     * @param report 输出报告
     * @return 结束结果
     */
    report_builder_result_t (*finish_build)(report_build_context_t *context, report_t **report);

    /**
     * @brief 取消构建报告
     * @param context 构建上下文
     * @return 取消结果
     */
    report_builder_result_t (*cancel_build)(report_build_context_t *context);

    /**
     * @brief 添加报告数据
     * @param context 构建上下文
     * @param data 报告数据
     * @return 添加结果
     */
    report_builder_result_t (*add_data)(report_build_context_t *context, const report_data_t *data);

    /**
     * @brief 批量添加报告数据
     * @param context 构建上下文
     * @param data 数据数组
     * @param count 数据数量
     * @return 添加结果
     */
    report_builder_result_t (*add_data_batch)(report_build_context_t *context,
                                                 const report_data_t *data,
                                                 uint16_t count);

    /**
     * @brief 构建键盘报告
     * @param context 构建上下文
     * @param modifier 修饰键
     * @param keys 按键数组
     * @param key_count 按键数量
     * @return 构建结果
     */
    report_builder_result_t (*build_keyboard_report)(report_build_context_t *context,
                                                         uint8_t modifier,
                                                         const uint8_t *keys,
                                                         uint8_t key_count);

    /**
     * @brief 构建鼠标报告
     * @param context 构建上下文
     * @param buttons 鼠标按键
     * @param x X轴移动
     * @param y Y轴移动
     * @param wheel 滚轮滚动
     * @return 构建结果
     */
    report_builder_result_t (*build_mouse_report)(report_build_context_t *context,
                                                      uint8_t buttons,
                                                      int8_t x,
                                                      int8_t y,
                                                      int8_t wheel);

    /**
     * @brief 构建消费者控制报告
     * @param context 构建上下文
     * @param usage 用法
     * @param value 用法值
     * @return 构建结果
     */
    report_builder_result_t (*build_consumer_report)(report_build_context_t *context,
                                                         uint16_t usage,
                                                         int8_t value);

    /**
     * @brief 构建系统控制报告
     * @param context 构建上下文
     * @param usage 系统控制用法
     * @return 构建结果
     */
    report_builder_result_t (*build_system_report)(report_build_context_t *context,
                                                         uint8_t usage);

    /**
     * @brief 构建游戏手柄报告
     * @param context 构建上下文
     * @param buttons 按键
     * @param x X轴
     * @param y Y轴
     * @param z Z轴
     * @param rx X轴旋转
     * @param ry Y轴旋转
     * @param rz Z轴旋转
     * @param slider 滑块
     * @return 构建结果
     */
    report_builder_result_t (*build_gamepad_report)(report_build_context_t *context,
                                                          uint16_t buttons,
                                                          uint8_t x,
                                                          uint8_t y,
                                                          uint8_t z,
                                                          uint8_t rx,
                                                          uint8_t ry,
                                                          uint8_t rz,
                                                          uint8_t slider);

    /**
     * @brief 构建摇杆报告
     * @param context 构建上下文
     * @param buttons 按键
     * @param x X轴
     * @param y Y轴
     * @param z Z轴
     * @param rx X轴旋转
     * @param ry Y轴旋转
     * @param rz Z轴旋转
     * @param slider 滑块
     * @return 构建结果
     */
    report_builder_result_t (*build_joystick_report)(report_build_context_t *context,
                                                           uint16_t buttons,
                                                           uint8_t x,
                                                           uint8_t y,
                                                           uint8_t z,
                                                           uint8_t rx,
                                                           uint8_t ry,
                                                           uint8_t rz,
                                                           uint8_t slider);

    /**
     * @brief 构建数字化仪报告
     * @param context 构建上下文
     * @param report_data 报告数据
     * @param data_size 数据大小
     * @return 构建结果
     */
    report_builder_result_t (*build_digitizer_report)(report_build_context_t *context,
                                                           const void *report_data,
                                                           uint16_t data_size);

    /**
     * @brief 构建厂商特定报告
     * @param context 构建上下文
     * @param report_id 报告ID
     * @param report_data 报告数据
     * @param data_size 数据大小
     * @return 构建结果
     */
    report_builder_result_t (*build_vendor_report)(report_build_context_t *context,
                                                        uint8_t report_id,
                                                        const void *report_data,
                                                        uint16_t data_size);

    /**
     * @brief 构建原始报告
     * @param context 构建上下文
     * @param report_id 报告ID
     * @param report_type 报告类型
     * @param report_data 报告数据
     * @param data_size 数据大小
     * @return 构建结果
     */
    report_builder_result_t (*build_raw_report)(report_build_context_t *context,
                                                     uint8_t report_id,
                                                     hid_report_type_t report_type,
                                                     const void *report_data,
                                                     uint16_t data_size);

    /**
     * @brief 构建组合报告
     * @param context 构建上下文
     * @param reports 报告数组
     * @param report_count 报告数量
     * @return 构建结果
     */
    report_builder_result_t (*build_combined_report)(report_build_context_t *context,
                                                         const report_t **reports,
                                                         uint8_t report_count);

    /**
     * @brief 序列化报告
     * @param report 报告
     * @param format 输出格式
     * @param buffer 输出缓冲区
     * @param buffer_size 缓冲区大小
     * @param serialized_size 输出序列化大小
     * @return 序列化结果
     */
    report_builder_result_t (*serialize_report)(const report_t *report,
                                                     report_format_t format,
                                                     void *buffer,
                                                     uint32_t buffer_size,
                                                     uint32_t *serialized_size);

    /**
     * @brief 反序列化报告
     * @param data 数据
     * @param data_size 数据大小
     * @param format 输入格式
     * @param report 输出报告
     * @return 反序列化结果
     */
    report_builder_result_t (*deserialize_report)(const void *data,
                                                       uint32_t data_size,
                                                       report_format_t format,
                                                       report_t **report);

    /**
     * @brief 压缩报告
     * @param report 报告
     * @param compressed_data 输出压缩数据
     * @param max_size 最大大小
     * @param compressed_size 输出压缩大小
     * @return 压缩结果
     */
    report_builder_result_t (*compress_report)(const report_t *report,
                                                     void *compressed_data,
                                                     uint32_t max_size,
                                                     uint32_t *compressed_size);

    /**
     * @brief 解压缩报告
     * @param compressed_data 压缩数据
     * @param compressed_size 压缩大小
     * @param report 输出报告
     * @return 解压缩结果
     */
    report_builder_result_t (*decompress_report)(const void *compressed_data,
                                                       uint32_t compressed_size,
                                                       report_t **report);

    /**
     * @brief 加密报告
     * @param report 报告
     * @param encrypted_data 输出加密数据
     * @param max_size 最大大小
     * @param encrypted_size 输出加密大小
     * @return 加密结果
     */
    report_builder_result_t (*encrypt_report)(const report_t *report,
                                                   void *encrypted_data,
                                                   uint32_t max_size,
                                                   uint32_t *encrypted_size);

    /**
     * @brief 解密报告
     * @param encrypted_data 加密数据
     * @param encrypted_size 加密大小
     * @param report 输出报告
     * @return 解密结果
     */
    report_builder_result_t (*decrypt_report)(const void *encrypted_data,
                                                   uint32_t encrypted_size,
                                                   report_t **report);

    /**
     * @brief 分片报告
     * @param report 报告
     * @param fragment_size 分片大小
     * @param fragments 输出分片数组
     * @param max_fragments 最大分片数
     * @param fragment_count 输出分片数量
     * @return 分片结果
     */
    report_builder_result_t (*fragment_report)(const report_t *report,
                                                     uint16_t fragment_size,
                                                     report_fragment_t *fragments,
                                                     uint8_t max_fragments,
                                                     uint8_t *fragment_count);

    /**
     * @brief 重组报告
     * @param fragments 分片数组
     * @param fragment_count 分片数量
     * @param report 输出报告
     * @return 重组结果
     */
    report_builder_result_t (*reassemble_report)(const report_fragment_t *fragments,
                                                       uint8_t fragment_count,
                                                       report_t **report);

    /**
     * @brief 验证报告
     * @param report 报告
     * @return 验证结果
     */
    report_builder_result_t (*validate_report)(const report_t *report);

    /**
     * @brief 计算报告校验和
     * @param report 报告
     * @param checksum 输出校验和
     * @return 计算结果
     */
    report_builder_result_t (*calculate_checksum)(const report_t *report, uint32_t *checksum);

    /**
     * @brief 验证报告校验和
     * @param report 报告
     * @param valid 输出是否有效标志
     * @return 验证结果
     */
    report_builder_result_t (*verify_checksum)(const report_t *report, bool *valid);

    /**
     * @brief 克隆报告
     * @param source_report 源报告
     * @param cloned_report 输出克隆报告
     * @return 克隆结果
     */
    report_builder_result_t (*clone_report)(const report_t *source_report, report_t **cloned_report);

    /**
     * @brief 合并报告
     * @param reports 报告数组
     * @param report_count 报告数量
     * @param merged_report 输出合并报告
     * @return 合并结果
     */
    report_builder_result_t (*merge_reports)(const report_t **reports,
                                                  uint8_t report_count,
                                                  report_t **merged_report);

    /**
     * @brief 分离报告
     * @param report 报告
     * @param sub_reports 输出子报告数组
     * @param max_reports 最大报告数
     * @param sub_report_count 输出子报告数量
     * @return 分离结果
     */
    report_builder_result_t (*split_report)(const report_t *report,
                                                 report_t **sub_reports,
                                                 uint8_t max_reports,
                                                 uint8_t *sub_report_count);

    /**
     * @brief 获取报告大小
     * @param report 报告
     * @param size 输出报告大小
     * @return 获取结果
     */
    report_builder_result_t (*get_report_size)(const report_t *report, uint32_t *size);

    /**
     * @brief 获取报告数据
     * @param report 报告
     * @param data 输出数据缓冲区
     * @param max_size 最大大小
     * @param data_size 输出数据大小
     * @return 获取结果
     */
    report_builder_result_t (*get_report_data)(const report_t *report,
                                                     void *data,
                                                     uint32_t max_size,
                                                     uint32_t *data_size);

    /**
     * @brief 设置报告优先级
     * @param report 报告
     * @param priority 优先级
     * @return 设置结果
     */
    report_builder_result_t (*set_report_priority)(report_t *report, report_priority_t priority);

    /**
     * @brief 获取报告优先级
     * @param report 报告
     * @param priority 输出优先级
     * @return 获取结果
     */
    report_builder_result_t (*get_report_priority)(const report_t *report, report_priority_t *priority);

    /**
     * @brief 设置报告标志
     * @param report 报告
     * @param flags 标志
     * @return 设置结果
     */
    report_builder_result_t (*set_report_flags)(report_t *report, report_flag_t flags);

    /**
     * @brief 获取报告标志
     * @param report 报告
     * @param flags 输出标志
     * @return 获取结果
     */
    report_builder_result_t (*get_report_flags)(const report_t *report, report_flag_t *flags);

    /**
     * @brief 检查报告是否已完成
     * @param report 报告
     * @param completed 输出是否完成标志
     * @return 检查结果
     */
    report_builder_result_t (*is_report_complete)(const report_t *report, bool *completed);

    /**
     * @brief 等待报告完成
     * @param report 报告
     * @param timeout_ms 超时时间
     * @return 等待结果
     */
    report_builder_result_t (*wait_report_complete)(const report_t *report, uint32_t timeout_ms);

    /**
     * @brief 取消报告
     * @param report 报告
     * @return 取消结果
     */
    report_builder_result_t (*cancel_report)(report_t *report);

    /**
     * @brief 释放报告
     * @param report 报告
     * @return 释放结果
     */
    report_builder_result_t (*free_report)(report_t *report);

    /**
     * @brief 入队报告
     * @param report 报告
     * @param priority 优先级
     * @return 入队结果
     */
    report_builder_result_t (*enqueue_report)(report_t *report, report_priority_t priority);

    /**
     * @brief 出队报告
     * @param report 输出报告
     * @return 出队结果
     */
    report_builder_result_t (*dequeue_report)(report_t **report);

    /**
     * @brief 查找报告
     * @param report_id 报告ID
     * @param report 输出报告
     * @return 查找结果
     */
    report_builder_result_t (*find_report)(uint16_t report_id, report_t **report);

    /**
     * @brief 删除报告
     * @param report_id 报告ID
     * @return 删除结果
     */
    report_builder_result_t (*remove_report)(uint16_t report_id);

    /**
     * @brief 清空报告队列
     * @return 清空结果
     */
    report_builder_result_t (*clear_queue)(void);

    /**
     * @brief 获取队列深度
     * @param depth 输出队列深度
     * @return 获取结果
     */
    report_builder_result_t (*get_queue_depth)(uint16_t *depth);

    /**
     * @brief 设置构建回调函数
     * @param callback 回调函数
     * @param user_data 用户数据
     * @return 设置结果
     */
    report_builder_result_t (*set_build_callback)(report_build_callback_t callback, void *user_data);

    /**
     * @brief 设置发送回调函数
     * @param callback 回调函数
     * @param user_data 用户数据
     * @return 设置结果
     */
    report_builder_result_t (*set_send_callback)(report_send_callback_t callback, void *user_data);

    /**
     * @brief 设置确认回调函数
     * @param callback 回调函数
     * @param user_data 用户数据
     * @return 设置结果
     */
    report_builder_result_t (*set_ack_callback)(report_ack_callback_t callback, void *user_data);

    /**
     * @brief 设置错误回调函数
     * @param callback 回调函数
     * @param user_data 用户数据
     * @return 设置结果
     */
    report_builder_result_t (*set_error_callback)(report_error_callback_t callback, void *user_data);

    /**
     * @brief 设置进度回调函数
     * @param callback 回调函数
     * @param user_data 用户数据
     * @return 设置结果
     */
    report_builder_result_t (*set_progress_callback)(report_progress_callback_t callback, void *user_data);

    /**
     * @brief 设置过滤回调函数
     * @param callback 回调函数
     * @param user_data 用户数据
     * @return 设置结果
     */
    report_builder_result_t (*set_filter_callback)(report_filter_callback_t callback, void *user_data);

    /**
     * @brief 处理报告队列
     * @return 处理结果
     */
    report_builder_result_t (*process_queue)(void);

    /**
     * @brief 发送报告
     * @param report 报告
     * @return 发送结果
     */
    report_builder_result_t (*send_report)(report_t *report);

    /**
     * @brief 批量发送报告
     * @param reports 报告数组
     * @param report_count 报告数量
     * @return 发送结果
     */
    report_builder_result_t (*send_reports)(report_t **reports, uint8_t report_count);

    /**
     * @brief 异步发送报告
     * @param report 报告
     * @return 发送结果
     */
    report_builder_result_t (*send_report_async)(report_t *report);

    /**
     * @brief 等待发送完成
     * @param timeout_ms 超时时间
     * @return 等待结果
     */
    report_builder_result_t (*wait_send_complete)(uint32_t timeout_ms);

    /**
     * @brief 处理报告确认
     * @param report_id 报告ID
     * @param success 是否成功
     * @return 处理结果
     */
    report_builder_result_t (*handle_report_ack)(uint16_t report_id, bool success);

    /**
     * @brief 处理超时报告
     * @param report_id 报告ID
     * @return 处理结果
     */
    report_builder_result_t (*handle_report_timeout)(uint16_t report_id);

    /**
     * @brief 重试发送报告
     * @param report_id 报告ID
     * @return 重试结果
     */
    report_builder_result_t (*retry_report)(uint16_t report_id);

    /**
     * @brief 获取构建器状态
     * @param initialized 输出是否已初始化标志
     * @param building 输出是否正在构建标志
     * @param queue_empty 输出队列是否为空标志
     * @return 获取结果
     */
    report_builder_result_t (*get_state)(bool *initialized, bool *building, bool *queue_empty);

    /**
     * @brief 获取统计信息
     * @param stats 输出统计信息
     * @return 获取结果
     */
    report_builder_result_t (*get_statistics)(report_builder_statistics_t *stats);

    /**
     * @brief 重置统计信息
     * @return 重置结果
     */
    report_builder_result_t (*reset_statistics)(void);

    /**
     * @brief 重置构建器
     * @return 重置结果
     */
    report_builder_result_t (*reset)(void);

    /**
     * @brief 获取报告构建器版本
     * @return 版本号
     */
    uint32_t (*get_version)(void);
} report_builder_interface_t;

/* 全局报告构建器实例 */
extern const report_builder_interface_t *report_builder;

/* 常用报告构建器配置宏 */
#define REPORT_BUILDER_CONFIG_DEFAULT() \
    { REPORT_BUILD_MODE_SINGLE, REPORT_FORMAT_HID, 64, 1024, 16, \
      REPORT_PRIORITY_NORMAL, false, false, false, false, true, 3, \
      1000, 512, 6, {0}, {0}, false, false, 100, 10 }

#define REPORT_BUILDER_CONFIG_BATCH() \
    { REPORT_BUILD_MODE_BATCH, REPORT_FORMAT_HID, 64, 4096, 64, \
      REPORT_PRIORITY_NORMAL, true, false, false, true, true, 5, \
      2000, 512, 6, {0}, {0}, false, true, 50, 10 }

#define REPORT_BUILDER_CONFIG_SECURE() \
    { REPORT_BUILD_MODE_SINGLE, REPORT_FORMAT_ENCRYPTED, 64, 1024, 16, \
      REPORT_PRIORITY_HIGH, true, true, true, false, true, 3, \
      500, 256, 9, {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08, \
                      0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10}, \
      {0xA1,0xB2,0xC3,0xD4,0xE5,0xF6,0x07,0x18, \
       0x29,0x3A,0x4B,0x5C,0x6D,0x7E,0x8F,0x90}, \
      true, false, 200, 5 }

#define REPORT_BUILDER_CONFIG_STREAM() \
    { REPORT_BUILD_MODE_STREAM, REPORT_FORMAT_HID, 64, 2048, 32, \
      REPORT_PRIORITY_NORMAL, false, false, false, false, false, 0, \
      0, 0, 3, {0}, {0}, true, false, 0, 0 }

/* 报告构建器错误处理宏 */
#define REPORT_BUILDER_CHECK(expr) \
    do { \
        report_builder_result_t _result = (expr); \
        if (_result != REPORT_BUILDER_OK) { \
            return _result; \
        } \
    } while(0)

#define REPORT_BUILDER_CHECK_RET(expr, ret_val) \
    do { \
        report_builder_result_t _result = (expr); \
        if (_result != REPORT_BUILDER_OK) { \
            return (ret_val); \
        } \
    } while(0)

/* 便捷操作宏 */
#define REPORT_BUILD_KEYBOARD(modifier, keys, count) \
    do { \
        report_build_context_t _ctx; \
        report_builder->start_build(&_ctx); \
        report_builder->build_keyboard_report(&_ctx, (modifier), (keys), (count)); \
        report_t *_report; \
        report_builder->finish_build(&_ctx, &_report); \
        report_builder->send_report(_report); \
    } while(0)

#define REPORT_BUILD_MOUSE(buttons, x, y, wheel) \
    do { \
        report_build_context_t _ctx; \
        report_builder->start_build(&_ctx); \
        report_builder->build_mouse_report(&_ctx, (buttons), (x), (y), (wheel)); \
        report_t *_report; \
        report_builder->finish_build(&_ctx, &_report); \
        report_builder->send_report(_report); \
    } while(0)

#define REPORT_BUILD_CONSUMER(usage, value) \
    do { \
        report_build_context_t _ctx; \
        report_builder->start_build(&_ctx); \
        report_builder->build_consumer_report(&_ctx, (usage), (value)); \
        report_t *_report; \
        report_builder->finish_build(&_ctx, &_report); \
        report_builder->send_report(_report); \
    } while(0)

#define REPORT_BUILD_SYSTEM(usage) \
    do { \
        report_build_context_t _ctx; \
        report_builder->start_build(&_ctx); \
        report_builder->build_system_report(&_ctx, (usage)); \
        report_t *_report; \
        report_builder->finish_build(&_ctx, &_report); \
        report_builder->send_report(_report); \
    } while(0)

#define REPORT_BUILD_GAMEPAD(buttons, x, y, z, rx, ry, rz, slider) \
    do { \
        report_build_context_t _ctx; \
        report_builder->start_build(&_ctx); \
        report_builder->build_gamepad_report(&_ctx, (buttons), (x), (y), (z), (rx), (ry), (rz), (slider)); \
        report_t *_report; \
        report_builder->finish_build(&_ctx, &_report); \
        report_builder->send_report(_report); \
    } while(0)

#define REPORT_PROCESS_QUEUE() \
    report_builder->process_queue()

#define REPORT_GET_QUEUE_DEPTH() \
    ({ \
        uint16_t _depth; \
        report_builder->get_queue_depth(&_depth); \
        _depth; \
    })

#define REPORT_CLEAR_QUEUE() \
    report_builder->clear_queue()

#ifdef __cplusplus
}
#endif

#endif /* REPORT_BUILDER_H */