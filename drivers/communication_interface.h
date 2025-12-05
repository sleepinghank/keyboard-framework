/**
 * @file communication_interface.h
 * @brief 通信驱动接口定义
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * 提供通信驱动的统一接口定义
 * 支持BLE、USB、2.4G等多种通信方式
 */

#ifndef COMMUNICATION_INTERFACE_H
#define COMMUNICATION_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 通信错误码定义 */
typedef enum {
    COMM_OK = 0,                       /* 操作成功 */
    COMM_ERROR_INVALID_INTERFACE,      /* 无效接口 */
    COMM_ERROR_INVALID_CHANNEL,        /* 无效通道 */
    COMM_ERROR_INVALID_PARAMETER,      /* 无效参数 */
    COMM_ERROR_NOT_SUPPORTED,          /* 操作不支持 */
    COMM_ERROR_HARDWARE_FAULT,         /* 硬件故障 */
    COMM_ERROR_BUSY,                   /* 通信忙碌 */
    COMM_ERROR_TIMEOUT,                /* 操作超时 */
    COMM_ERROR_ALREADY_INITIALIZED,    /* 已初始化 */
    COMM_ERROR_NOT_INITIALIZED,        /* 未初始化 */
    COMM_ERROR_ALREADY_CONNECTED,      /* 已连接 */
    COMM_ERROR_NOT_CONNECTED,          /* 未连接 */
    COMM_ERROR_CONNECTION_LOST,        /* 连接丢失 */
    COMM_ERROR_CONNECTION_FAILED,      /* 连接失败 */
    COMM_ERROR_DISCONNECT_FAILED,       /* 断开连接失败 */
    COMM_ERROR_TRANSMIT_FAILED,        /* 发送失败 */
    COMM_ERROR_RECEIVE_FAILED,         /* 接收失败 */
    COMM_ERROR_BUFFER_FULL,            /* 缓冲区满 */
    COMM_ERROR_BUFFER_EMPTY,           /* 缓冲区空 */
    COMM_ERROR_INVALID_DATA,           /* 无效数据 */
    COMM_ERROR_DATA_CORRUPTED,         /* 数据损坏 */
    COMM_ERROR_CHECKSUM_FAILED,        /* 校验失败 */
    COMM_ERROR_ENCRYPTION_FAILED,      /* 加密失败 */
    COMM_ERROR_DECRYPTION_FAILED,      /* 解密失败 */
    COMM_ERROR_AUTHENTICATION_FAILED,  /* 认证失败 */
    COMM_ERROR_PAIRING_FAILED,         /* 配对失败 */
    COMM_ERROR_BONDING_FAILED,         /* 绑定失败 */
    COMM_ERROR_SECURITY_FAILED,        /* 安全失败 */
    COMM_ERROR_LICENSE_REQUIRED,       /* 需要许可证 */
    COMM_ERROR_LICENSE_INVALID,        /* 许可证无效 */
    COMM_ERROR_LICENSE_EXPIRED,        /* 许可证过期 */
    COMM_ERROR_FEATURE_DISABLED,       /* 功能禁用 */
    COMM_ERROR_INSUFFICIENT_RESOURCES, /* 资源不足 */
    COMM_ERROR_INSUFFICIENT_POWER,    /* 电源不足 */
    COMM_ERROR_INSUFFICIENT_BANDWIDTH, /* 带宽不足 */
    COMM_ERROR_SIGNAL_TOO_WEAK,       /* 信号太弱 */
    COMM_ERROR_SIGNAL_TOO_STRONG,     /* 信号太强 */
    COMM_ERROR_INTERFERENCE,          /* 干扰 */
    COMM_ERROR_FREQUENCY_MISMATCH,    /* 频率不匹配 */
    COMM_ERROR_PROTOCOL_MISMATCH,     /* 协议不匹配 */
    COMM_ERROR_VERSION_MISMATCH,      /* 版本不匹配 */
    COMM_ERROR_CONFIGURATION_ERROR,    /* 配置错误 */
    COMM_ERROR_FIRMWARE_INCOMPATIBLE,  /* 固件不兼容 */
    COMM_ERROR_HARDWARE_INCOMPATIBLE,  /* 硬件不兼容 */
    COMM_ERROR_CALIBRATION_FAILED,     /* 校准失败 */
    COMM_ERROR_TEST_FAILED,           /* 测试失败 */
    COMM_ERROR_MAINTENANCE_MODE,      /* 维护模式 */
    COMM_ERROR_EMERGENCY_MODE,        /* 紧急模式 */
    COMM_ERROR_BOOTLOADER_MODE,       /* 引导加载模式 */
    COMM_ERROR_FLASHING_MODE,         /* 固件更新模式 */
    COMM_ERROR_DEBUG_MODE,            /* 调试模式 */
    COMM_ERROR_POWER_SAVE_MODE,       /* 省电模式 */
    COMM_ERROR_STANDBY_MODE           /* 待机模式 */
} comm_result_t;

/* 通信接口类型定义 */
typedef enum {
    COMM_INTERFACE_BLE = 0,           /* 蓝牙接口 */
    COMM_INTERFACE_USB,               /* USB接口 */
    COMM_INTERFACE_WIRELESS_24,       /* 2.4G无线接口 */
    COMM_INTERFACE_UART,              /* 串口接口 */
    COMM_INTERFACE_I2C,               /* I2C接口 */
    COMM_INTERFACE_SPI,               /* SPI接口 */
    COMM_INTERFACE_CAN,               /* CAN接口 */
    COMM_INTERFACE_ETHERNET,          /* 以太网接口 */
    COMM_INTERFACE_WIFI,              /* WiFi接口 */
    COMM_INTERFACE_ZIGBEE,            /* ZigBee接口 */
    COMM_INTERFACE_NFC,               /* NFC接口 */
    COMM_INTERFACE_IR,                /* 红外接口 */
    COMM_INTERFACE_MAX                 /* 最大接口数 */
} comm_interface_type_t;

/* 通信协议类型定义 */
typedef enum {
    COMM_PROTOCOL_HID = 0,           /* HID协议 */
    COMM_PROTOCOL_CUSTOM,             /* 自定义协议 */
    COMM_PROTOCOL_VENDOR_SPECIFIC,    /* 厂商特定协议 */
    COMM_PROTOCOL_KEYBOARD,           /* 键盘协议 */
    COMM_PROTOCOL_MOUSE,              /* 鼠标协议 */
    COMM_PROTOCOL_GAMEPAD,            /* 游戏手柄协议 */
    COMM_PROTOCOL_JOYSTICK,           /* 摇杆协议 */
    COMM_PROTOCOL_TOUCH,              /* 触摸协议 */
    COMM_PROTOCOL_MOTION,             /* 运动协议 */
    COMM_PROTOCOL_GESTURE,            /* 手势协议 */
    COMM_PROTOCOL_AUDIO,              /* 音频协议 */
    COMM_PROTOCOL_VIDEO,              /* 视频协议 */
    COMM_PROTOCOL_DATA,               /* 数据协议 */
    COMM_PROTOCOL_CONTROL,            /* 控制协议 */
    COMM_PROTOCOL_STATUS,             /* 状态协议 */
    COMM_PROTOCOL_DIAGNOSTIC,         /* 诊断协议 */
    COMM_PROTOCOL_FIRMWARE_UPDATE,    /* 固件更新协议 */
    COMM_PROTOCOL_BOOTLOADER,         /* 引导加载协议 */
    COMM_PROTOCOL_DEBUG,              /* 调试协议 */
    COMM_PROTOCOL_CONFIGURATION,       /* 配置协议 */
    COMM_PROTOCOL_PAIRING,            /* 配对协议 */
    COMM_PROTOCOL_BONDING,            /* 绑定协议 */
    COMM_PROTOCOL_AUTHENTICATION,     /* 认证协议 */
    COMM_PROTOCOL_ENCRYPTION,         /* 加密协议 */
    COMM_PROTOCOL_POWER,              /* 电源管理协议 */
    COMM_PROTOCOL_BATTERY,            /* 电池协议 */
    COMM_PROTOCOL_INDICATOR,          /* 指示器协议 */
    COMM_PROTOCOL_NOTIFICATION,       /* 通知协议 */
    COMM_PROTOCOL_RAW                 /* 原始数据协议 */
} comm_protocol_type_t;

/* 通信状态定义 */
typedef enum {
    COMM_STATE_INITIALIZING = 0,     /* 初始化中 */
    COMM_STATE_READY,                /* 就绪 */
    COMM_STATE_SCANNING,             /* 扫描中 */
    COMM_STATE_ADVERTISING,          /* 广播中 */
    COMM_STATE_CONNECTING,           /* 连接中 */
    COMM_STATE_CONNECTED,            /* 已连接 */
    COMM_STATE_AUTHENTICATING,        /* 认证中 */
    COMM_STATE_ENCRYPTING,           /* 加密中 */
    COMM_STATE_PAIRED,               /* 已配对 */
    COMM_STATE_BONDED,               /* 已绑定 */
    COMM_STATE_TRANSMITTING,         /* 发送中 */
    COMM_STATE_RECEIVING,             /* 接收中 */
    COMM_STATE_IDLE,                 /* 空闲 */
    COMM_STATE_SLEEP,                /* 睡眠 */
    COMM_STATE_SUSPENDED,            /* 挂起 */
    COMM_STATE_ERROR,                /* 错误 */
    COMM_STATE_FAULT,                /* 故障 */
    COMM_STATE_RECOVERY,             /* 恢复中 */
    COMM_STATE_MAINTENANCE,          /* 维护中 */
    COMM_STATE_UPDATING,             /* 更新中 */
    COMM_STATE_DEBUGGING,            /* 调试中 */
    COMM_STATE_TESTING,              /* 测试中 */
    COMM_STATE_CALIBRATING,          /* 校准中 */
    COMM_STATE_POWER_SAVE,           /* 省电模式 */
    COMM_STATE_STANDBY,             /* 待机模式 */
    COMM_STATE_SHUTTING_DOWN,        /* 关机中 */
    COMM_STATE_SHUTDOWN              /* 已关机 */
} comm_state_t;

/* 通信事件类型定义 */
typedef enum {
    COMM_EVENT_INTERFACE_READY = 0,  /* 接口就绪事件 */
    COMM_EVENT_INTERFACE_ERROR,      /* 接口错误事件 */
    COMM_EVENT_SCAN_COMPLETE,        /* 扫描完成事件 */
    COMM_EVENT_DEVICE_FOUND,         /* 设备发现事件 */
    COMM_EVENT_DEVICE_LOST,          /* 设备丢失事件 */
    COMM_EVENT_CONNECTING,           /* 连接开始事件 */
    COMM_EVENT_CONNECTED,            /* 连接成功事件 */
    COMM_EVENT_DISCONNECTED,         /* 连接断开事件 */
    COMM_EVENT_CONNECTION_FAILED,    /* 连接失败事件 */
    COMM_EVENT_AUTHENTICATION_STARTED, /* 认证开始事件 */
    COMM_EVENT_AUTHENTICATION_SUCCESS, /* 认证成功事件 */
    COMM_EVENT_AUTHENTICATION_FAILED, /* 认证失败事件 */
    COMM_EVENT_PAIRING_STARTED,     /* 配对开始事件 */
    COMM_EVENT_PAIRING_SUCCESS,     /* 配对成功事件 */
    COMM_EVENT_PAIRING_FAILED,      /* 配对失败事件 */
    COMM_EVENT_BONDING_STARTED,     /* 绑定开始事件 */
    COMM_EVENT_BONDING_SUCCESS,     /* 绑定成功事件 */
    COMM_EVENT_BONDING_FAILED,      /* 绑定失败事件 */
    COMM_EVENT_ENCRYPTION_STARTED,   /* 加密开始事件 */
    COMM_EVENT_ENCRYPTION_SUCCESS,   /* 加密成功事件 */
    COMM_EVENT_ENCRYPTION_FAILED,    /* 加密失败事件 */
    COMM_EVENT_DATA_TRANSMITTED,    /* 数据发送事件 */
    COMM_EVENT_DATA_RECEIVED,        /* 数据接收事件 */
    COMM_EVENT_TRANSMIT_FAILED,     /* 发送失败事件 */
    COMM_EVENT_RECEIVE_FAILED,      /* 接收失败事件 */
    COMM_EVENT_BUFFER_FULL,          /* 缓冲区满事件 */
    COMM_EVENT_BUFFER_EMPTY,         /* 缓冲区空事件 */
    COMM_EVENT_SIGNAL_STRENGTH_CHANGED, /* 信号强度变化事件 */
    COMM_EVENT_BATTERY_LEVEL_CHANGED, /* 电池电量变化事件 */
    COMM_EVENT_POWER_STATE_CHANGED,  /* 电源状态变化事件 */
    COMM_EVENT_MODE_CHANGED,         /* 模式变化事件 */
    COMM_EVENT_CONFIGURATION_CHANGED, /* 配置变化事件 */
    COMM_EVENT_ERROR_OCCURRED,       /* 错误发生事件 */
    COMM_EVENT_ERROR_CLEARED,        /* 错误清除事件 */
    COMM_EVENT_RECOVERY_STARTED,     /* 恢复开始事件 */
    COMM_EVENT_RECOVERY_COMPLETED,   /* 恢复完成事件 */
    COMM_EVENT_UPDATE_STARTED,       /* 更新开始事件 */
    COMM_EVENT_UPDATE_PROGRESS,      /* 更新进度事件 */
    COMM_EVENT_UPDATE_COMPLETED,     /* 更新完成事件 */
    COMM_EVENT_UPDATE_FAILED,        /* 更新失败事件 */
    COMM_EVENT_TEST_STARTED,         /* 测试开始事件 */
    COMM_EVENT_TEST_COMPLETED,       /* 测试完成事件 */
    COMM_EVENT_TEST_FAILED,          /* 测试失败事件 */
    COMM_EVENT_CALIBRATION_STARTED,  /* 校准开始事件 */
    COMM_EVENT_CALIBRATION_COMPLETED, /* 校准完成事件 */
    COMM_EVENT_CALIBRATION_FAILED,   /* 校准失败事件 */
    COMM_EVENT_WAKEUP,               /* 唤醒事件 */
    COMM_EVENT_SLEEP,                /* 睡眠事件 */
    COMM_EVENT_RESUME,               /* 恢复事件 */
    COMM_EVENT_SUSPEND,              /* 挂起事件 */
    COMM_EVENT_MAX                   /* 最大事件数 */
} comm_event_type_t;

/* 通信数据类型定义 */
typedef enum {
    COMM_DATA_TYPE_KEYBOARD = 0,      /* 键盘数据 */
    COMM_DATA_TYPE_MOUSE,             /* 鼠标数据 */
    COMM_DATA_TYPE_GAMEPAD,           /* 游戏手柄数据 */
    COMM_DATA_TYPE_JOYSTICK,          /* 摇杆数据 */
    COMM_DATA_TYPE_TOUCH,             /* 触摸数据 */
    COMM_DATA_TYPE_GESTURE,           /* 手势数据 */
    COMM_DATA_TYPE_MOTION,            /* 运动数据 */
    COMM_DATA_TYPE_AUDIO,             /* 音频数据 */
    COMM_DATA_TYPE_VIDEO,             /* 视频数据 */
    COMM_DATA_TYPE_CONTROL,           /* 控制数据 */
    COMM_DATA_TYPE_STATUS,            /* 状态数据 */
    COMM_DATA_TYPE_DIAGNOSTIC,        /* 诊断数据 */
    COMM_DATA_TYPE_FIRMWARE_UPDATE,   /* 固件更新数据 */
    COMM_DATA_TYPE_BOOTLOADER,        /* 引导加载数据 */
    COMM_DATA_TYPE_DEBUG,             /* 调试数据 */
    COMM_DATA_TYPE_CONFIGURATION,      /* 配置数据 */
    COMM_DATA_TYPE_PAIRING,           /* 配对数据 */
    COMM_DATA_TYPE_BONDING,           /* 绑定数据 */
    COMM_DATA_TYPE_AUTHENTICATION,    /* 认证数据 */
    COMM_DATA_TYPE_ENCRYPTION,        /* 加密数据 */
    COMM_DATA_TYPE_POWER,             /* 电源管理数据 */
    COMM_DATA_TYPE_BATTERY,           /* 电池数据 */
    COMM_DATA_TYPE_INDICATOR,         /* 指示器数据 */
    COMM_DATA_TYPE_NOTIFICATION,      /* 通知数据 */
    COMM_DATA_TYPE_RAW,               /* 原始数据 */
    COMM_DATA_TYPE_CUSTOM             /* 自定义数据 */
} comm_data_type_t;

/* 通信优先级定义 */
typedef enum {
    COMM_PRIORITY_LOW = 0,            /* 低优先级 */
    COMM_PRIORITY_NORMAL,            /* 正常优先级 */
    COMM_PRIORITY_HIGH,              /* 高优先级 */
    COMM_PRIORITY_CRITICAL,          /* 关键优先级 */
    COMM_PRIORITY_EMERGENCY,         /* 紧急优先级 */
    COMM_PRIORITY_MAX                 /* 最大优先级 */
} comm_priority_t;

/* 通信质量等级定义 */
typedef enum {
    COMM_QUALITY_EXCELLENT = 0,      /* 优秀质量 */
    COMM_QUALITY_GOOD,               /* 良好质量 */
    COMM_QUALITY_FAIR,               /* 一般质量 */
    COMM_QUALITY_POOR,               /* 较差质量 */
    COMM_QUALITY_BAD,                /* 很差质量 */
    COMM_QUALITY_NONE,               /* 无信号 */
    COMM_QUALITY_UNKNOWN             /* 未知质量 */
} comm_quality_t;

/* 通信安全等级定义 */
typedef enum {
    COMM_SECURITY_NONE = 0,           /* 无安全 */
    COMM_SECURITY_BASIC,             /* 基本安全 */
    COMM_SECURITY_STANDARD,           /* 标准安全 */
    COMM_SECURITY_HIGH,              /* 高级安全 */
    COMM_SECURITY_MAXIMUM,           /* 最高安全 */
    COMM_SECURITY_ENTERPRISE,        /* 企业级安全 */
    COMM_SECURITY_MILITARY,          /* 军用级安全 */
    COMM_SECURITY_UNKNOWN            /* 未知安全等级 */
} comm_security_level_t;

/* 通信配置结构体 */
typedef struct {
    comm_interface_type_t  interface_type;  /* 接口类型 */
    comm_protocol_type_t   protocol_type;   /* 协议类型 */
    uint32_t                baud_rate;       /* 波特率(适用于串口) */
    uint32_t                timeout_ms;      /* 超时时间(毫秒) */
    uint16_t                max_retries;     /* 最大重试次数 */
    bool                    auto_reconnect;  /* 自动重连 */
    bool                    auto_discover;   /* 自动发现 */
    bool                    auto_pair;       /* 自动配对 */
    bool                    auto_bond;       /* 自动绑定 */
    bool                    encryption_enable; /* 加密使能 */
    comm_security_level_t   security_level;  /* 安全等级 */
    comm_priority_t         priority;        /* 优先级 */
    uint16_t                max_connections; /* 最大连接数 */
    uint16_t                max_packet_size; /* 最大包大小 */
    uint8_t                 channel;         /* 通道号 */
    uint8_t                 power_level;     /* 功率等级 */
    uint8_t                 advertisement_interval; /* 广播间隔 */
    uint8_t                 scan_interval;   /* 扫描间隔 */
    uint8_t                 scan_window;     /* 扫描窗口 */
    bool                    whitelist_enable;/* 白名单使能 */
    uint8_t                 whitelist_size;  /* 白名单大小 */
    bool                    filter_enable;   /* 过滤使能 */
    uint8_t                 filter_policy;   /* 过滤策略 */
    bool                    multi_device_support; /* 多设备支持 */
    uint8_t                 multi_device_count; /* 多设备数量 */
    bool                    low_power_mode;  /* 低功耗模式 */
    uint8_t                 sleep_timeout;   /* 睡眠超时 */
    bool                    wake_on_activity; /* 活动唤醒 */
    bool                    debug_enable;    /* 调试使能 */
    uint8_t                 log_level;       /* 日志级别 */
    uint32_t                mtu_size;        /* MTU大小 */
    uint16_t                max_throughput;  /* 最大吞吐量 */
    uint16_t                latency_ms;      /* 延迟(毫秒) */
    uint8_t                 reliability_level; /* 可靠性等级 */
    bool                    flow_control;    /* 流控制 */
    uint8_t                 error_correction; /* 错误纠正 */
    uint16_t                buffer_size;     /* 缓冲区大小 */
    bool                    dma_enable;      /* DMA使能 */
    uint8_t                 dma_priority;    /* DMA优先级 */
    uint8_t                 interrupt_priority; /* 中断优先级 */
} comm_config_t;

/* 通信状态信息结构体 */
typedef struct {
    comm_interface_type_t  interface_type;  /* 接口类型 */
    comm_protocol_type_t   protocol_type;   /* 协议类型 */
    comm_state_t            current_state;   /* 当前状态 */
    comm_quality_t          quality;         /* 信号质量 */
    int8_t                  rssi;            /* 信号强度(dBm) */
    uint8_t                 link_quality;    /* 链路质量百分比 */
    uint16_t                connection_count; /* 连接数量 */
    uint16_t                active_channels; /* 活跃通道数 */
    uint32_t                bytes_transmitted;/* 已发送字节数 */
    uint32_t                bytes_received;  /* 已接收字节数 */
    uint32_t                packets_transmitted;/* 已发送包数 */
    uint32_t                packets_received;  /* 已接收包数 */
    uint32_t                error_count;     /* 错误计数 */
    uint32_t                timeout_count;   /* 超时计数 */
    uint32_t                retry_count;     /* 重试计数 */
    uint32_t                reconnect_count; /* 重连计数 */
    uint16_t                tx_buffer_usage;  /* 发送缓冲区使用率 */
    uint16_t                rx_buffer_usage;  /* 接收缓冲区使用率 */
    uint16_t                buffer_peak_usage;/* 缓冲区峰值使用率 */
    uint32_t                throughput_bps;  /* 吞吐量(bps) */
    uint16_t                latency_ms;      /* 延迟(毫秒) */
    uint16_t                jitter_ms;       /* 抖动(毫秒) */
    uint8_t                 battery_level;   /* 电池电量百分比 */
    uint32_t                uptime_seconds;   /* 运行时间(秒) */
    uint32_t                total_active_time_seconds; /* 总活动时间(秒) */
    uint32_t                total_sleep_time_seconds;  /* 总睡眠时间(秒) */
    comm_security_level_t   security_level;  /* 当前安全等级 */
    bool                    encryption_active; /* 加密激活状态 */
    bool                    authentication_active; /* 认证激活状态 */
    uint8_t                 connected_devices;/* 已连接设备数 */
    uint8_t                 paired_devices;   /* 已配对设备数 */
    uint8_t                 bonded_devices;   /* 已绑定设备数 */
    uint16_t                max_connections_supported; /* 支持的最大连接数 */
    uint16_t                max_packet_size_supported; /* 支持的最大包大小 */
    uint32_t                current_mtu_size; /* 当前MTU大小 */
    uint8_t                 current_power_level; /* 当前功率等级 */
    bool                    low_power_mode_active; /* 低功耗模式激活状态 */
    uint8_t                 sleep_mode;      /* 睡眠模式 */
    uint32_t                last_activity_timestamp; /* 最后活动时间戳 */
    uint32_t                connection_timestamp; /* 连接时间戳 */
    uint32_t                last_error_timestamp; /* 最后错误时间戳 */
    uint32_t                last_reconnect_timestamp; /* 最后重连时间戳 */
} comm_status_t;

/* 通信数据包结构体 */
typedef struct {
    comm_data_type_t       data_type;        /* 数据类型 */
    uint8_t                 channel;         /* 通道号 */
    uint8_t                 device_id;        /* 设备ID */
    uint16_t                packet_id;        /* 包ID */
    uint16_t                sequence_number;  /* 序列号 */
    bool                    acknowledgment;  /* 应答标志 */
    bool                    fragment;         /* 分片标志 */
    bool                    last_fragment;    /* 最后分片标志 */
    uint8_t                 fragment_index;   /* 分片索引 */
    uint8_t                 fragment_count;   /* 分片总数 */
    uint16_t                data_length;      /* 数据长度 */
    uint16_t                checksum;         /* 校验和 */
    uint8_t                 priority;        /* 优先级 */
    uint32_t                timestamp;        /* 时间戳 */
    uint8_t                *data;             /* 数据指针 */
} comm_packet_t;

/* 通信设备信息结构体 */
typedef struct {
    uint8_t                 device_id;        /* 设备ID */
    uint8_t                 device_type;      /* 设备类型 */
    char                    device_name[32];  /* 设备名称 */
    char                    manufacturer_name[32]; /* 制造商名称 */
    char                    model_name[32];   /* 型号名称 */
    char                    serial_number[32]; /* 序列号 */
    char                    firmware_version[16]; /* 固件版本 */
    char                    hardware_version[16]; /* 硬件版本 */
    char                    mac_address[18];  /* MAC地址 */
    char                    bluetooth_address[18]; /* 蓝牙地址 */
    uint16_t                vendor_id;       /* 厂商ID */
    uint16_t                product_id;      /* 产品ID */
    uint16_t                version_number;   /* 版本号 */
    uint8_t                 capabilities;    /* 能力标志 */
    bool                    paired;          /* 已配对标志 */
    bool                    bonded;          /* 已绑定标志 */
    bool                    trusted;         /* 信任标志 */
    bool                    blocked;         /* 阻止标志 */
    bool                    favorite;        /* 收藏标志 */
    int8_t                  rssi;            /* 信号强度 */
    uint8_t                 battery_level;   /* 电池电量 */
    bool                    connected;       /* 连接状态 */
    uint32_t                last_seen_timestamp; /* 最后发现时间戳 */
    uint32_t                connection_timestamp; /* 连接时间戳 */
    uint32_t                disconnection_timestamp; /* 断开连接时间戳 */
    uint32_t                total_connection_time; /* 总连接时间 */
    uint32_t                total_data_transferred; /* 总传输数据量 */
    comm_protocol_type_t    supported_protocols; /* 支持的协议 */
    uint8_t                 max_connections;   /* 最大连接数 */
    uint8_t                 current_connections; /* 当前连接数 */
} comm_device_info_t;

/* 通信统计信息结构体 */
typedef struct {
    uint32_t                total_connections;     /* 总连接数 */
    uint32_t                successful_connections;/* 成功连接数 */
    uint32_t                failed_connections;    /* 失败连接数 */
    uint32_t                connection_attempts;   /* 连接尝试次数 */
    uint32_t                reconnections;         /* 重连次数 */
    uint32_t                disconnections;       /* 断开连接次数 */
    uint32_t                pairing_attempts;     /* 配对尝试次数 */
    uint32_t                successful_pairings;  /* 成功配对次数 */
    uint32_t                failed_pairings;      /* 失败配对次数 */
    uint32_t                bonding_attempts;     /* 绑定尝试次数 */
    uint32_t                successful_bondings;  /* 成功绑定次数 */
    uint32_t                failed_bondings;      /* 失败绑定次数 */
    uint32_t                authentication_attempts;/* 认证尝试次数 */
    uint32_t                successful_authentications; /* 成功认证次数 */
    uint32_t                failed_authentications; /* 失败认证次数 */
    uint32_t                encryption_attempts; /* 加密尝试次数 */
    uint32_t                successful_encryptions; /* 成功加密次数 */
    uint32_t                failed_encryptions;  /* 失败加密次数 */
    uint32_t                bytes_transmitted;    /* 总发送字节数 */
    uint32_t                bytes_received;       /* 总接收字节数 */
    uint32_t                packets_transmitted;  /* 总发送包数 */
    uint32_t                packets_received;     /* 总接收包数 */
    uint32_t                transmission_errors;  /* 发送错误数 */
    uint32_t                reception_errors;     /* 接收错误数 */
    uint32_t                checksum_errors;     /* 校验错误数 */
    uint32_t                timeout_errors;      /* 超时错误数 */
    uint32_t                buffer_overflow_errors; /* 缓冲区溢出错误数 */
    uint32_t                buffer_underflow_errors;/* 缓冲区下溢错误数 */
    uint32_t                protocol_errors;      /* 协议错误数 */
    uint32_t                hardware_errors;      /* 硬件错误数 */
    uint32_t                software_errors;      /* 软件错误数 */
    uint32_t                security_errors;     /* 安全错误数 */
    uint32_t                encryption_errors;   /* 加密错误数 */
    uint32_t                decryption_errors;   /* 解密错误数 */
    uint32_t                key_exchange_errors; /* 密钥交换错误数 */
    uint32_t                certificate_errors; /* 证书错误数 */
    uint32_t                retry_count;         /* 重试次数 */
    uint32_t                total_uptime;        /* 总运行时间(秒) */
    uint32_t                total_downtime;      /* 总停机时间(秒) */
    uint32_t                total_active_time;   /* 总活动时间(秒) */
    uint32_t                total_sleep_time;    /* 总睡眠时间(秒) */
    float                   average_throughput;  /* 平均吞吐量(bps) */
    float                   peak_throughput;     /* 峰值吞吐量(bps) */
    uint16_t                average_latency;     /* 平均延迟(毫秒) */
    uint16_t                min_latency;         /* 最小延迟(毫秒) */
    uint16_t                max_latency;         /* 最大延迟(毫秒) */
    uint16_t                average_jitter;      /* 平均抖动(毫秒) */
    int8_t                  average_rssi;        /* 平均信号强度 */
    int8_t                  min_rssi;            /* 最小信号强度 */
    int8_t                  max_rssi;            /* 最大信号强度 */
    uint8_t                 average_link_quality; /* 平均链路质量 */
    uint8_t                 min_link_quality;    /* 最小链路质量 */
    uint8_t                 max_link_quality;    /* 最大链路质量 */
    uint32_t                battery_drain;       /* 电池消耗量 */
    uint32_t                power_consumption;   /* 功耗量 */
    uint32_t                last_reset_timestamp; /* 最后复位时间戳 */
    uint32_t                session_start_timestamp; /* 会话开始时间戳 */
    uint32_t                session_duration;    /* 会话持续时间 */
} comm_statistics_t;

/* 通信回调函数类型 */
typedef void (*comm_callback_t)(comm_event_type_t event, void *data, void *user_data);
typedef void (*comm_data_callback_t)(comm_data_type_t data_type, const uint8_t *data, uint16_t length, void *user_data);
typedef void (*comm_device_callback_t)(comm_device_info_t *device_info, void *user_data);
typedef void (*comm_error_callback_t)(comm_result_t error, const char *error_message, void *user_data);
typedef void (*comm_connection_callback_t)(uint8_t device_id, bool connected, void *user_data);
typedef void (*comm_pairing_callback_t)(uint8_t device_id, bool paired, void *user_data);
typedef void (*comm_bonding_callback_t)(uint8_t device_id, bool bonded, void *user_data);
typedef void (*comm_authentication_callback_t)(uint8_t device_id, bool authenticated, void *user_data);
typedef void (*comm_encryption_callback_t)(uint8_t device_id, bool encrypted, void *user_data);
typedef void (*comm_notification_callback_t)(uint8_t device_id, const char *notification, void *user_data);
typedef void (*comm_battery_callback_t)(uint8_t device_id, uint8_t battery_level, void *user_data);
typedef void (*comm_signal_callback_t)(uint8_t device_id, int8_t rssi, uint8_t link_quality, void *user_data);
typedef void (*comm_progress_callback_t)(uint32_t current, uint32_t total, void *user_data);
typedef void (*comm_firmware_update_callback_t)(uint8_t device_id, bool success, const char *message, void *user_data);

/* 通信接口函数指针结构体 */
typedef struct {
    /**
     * @brief 初始化通信接口
     * @param config 通信配置参数
     * @return 初始化结果
     */
    comm_result_t (*init)(const comm_config_t *config);

    /**
     * @brief 反初始化通信接口
     * @return 反初始化结果
     */
    comm_result_t (*deinit)(void);

    /**
     * @brief 启动通信接口
     * @return 启动结果
     */
    comm_result_t (*start)(void);

    /**
     * @brief 停止通信接口
     * @return 停止结果
     */
    comm_result_t (*stop)(void);

    /**
     * @brief 暂停通信接口
     * @return 暂停结果
     */
    comm_result_t (*pause)(void);

    /**
     * @brief 恢复通信接口
     * @return 恢复结果
     */
    comm_result_t (*resume)(void);

    /**
     * @brief 重置通信接口
     * @return 重置结果
     */
    comm_result_t (*reset)(void);

    /**
     * @brief 配置通信接口
     * @param config 通信配置参数
     * @return 配置结果
     */
    comm_result_t (*configure)(const comm_config_t *config);

    /**
     * @brief 获取通信配置
     * @param config 输出通信配置参数
     * @return 获取结果
     */
    comm_result_t (*get_config)(comm_config_t *config);

    /**
     * @brief 扫描设备
     * @param scan_timeout_ms 扫描超时时间(毫秒)
     * @param device_list 输出设备列表
     * @param max_devices 最大设备数量
     * @param device_count 输出设备数量
     * @return 扫描结果
     */
    comm_result_t (*scan_devices)(uint32_t scan_timeout_ms, comm_device_info_t *device_list, uint8_t max_devices, uint8_t *device_count);

    /**
     * @brief 停止扫描设备
     * @return 停止扫描结果
     */
    comm_result_t (*stop_scan)(void);

    /**
     * @brief 连接设备
     * @param device_info 设备信息
     * @param timeout_ms 连接超时时间(毫秒)
     * @return 连接结果
     */
    comm_result_t (*connect_device)(const comm_device_info_t *device_info, uint32_t timeout_ms);

    /**
     * @brief 断开设备连接
     * @param device_id 设备ID
     * @return 断开连接结果
     */
    comm_result_t (*disconnect_device)(uint8_t device_id);

    /**
     * @brief 断开所有设备连接
     * @return 断开所有连接结果
     */
    comm_result_t (*disconnect_all_devices)(void);

    /**
     * @brief 配对设备
     * @param device_info 设备信息
     * @param timeout_ms 配对超时时间(毫秒)
     * @return 配对结果
     */
    comm_result_t (*pair_device)(const comm_device_info_t *device_info, uint32_t timeout_ms);

    /**
     * @brief 取消设备配对
     * @param device_id 设备ID
     * @return 取消配对结果
     */
    comm_result_t (*unpair_device)(uint8_t device_id);

    /**
     * @brief 绑定设备
     * @param device_info 设备信息
     * @param timeout_ms 绑定超时时间(毫秒)
     * @return 绑定结果
     */
    comm_result_t (*bond_device)(const comm_device_info_t *device_info, uint32_t timeout_ms);

    /**
     * @brief 取消设备绑定
     * @param device_id 设备ID
     * @return 取消绑定结果
     */
    comm_result_t (*unbond_device)(uint8_t device_id);

    /**
     * @brief 认证设备
     * @param device_id 设备ID
     * @param auth_data 认证数据
     * @param auth_data_len 认证数据长度
     * @return 认证结果
     */
    comm_result_t (*authenticate_device)(uint8_t device_id, const uint8_t *auth_data, uint16_t auth_data_len);

    /**
     * @brief 加密连接
     * @param device_id 设备ID
     * @param encryption_key 加密密钥
     * @param key_length 密钥长度
     * @return 加密结果
     */
    comm_result_t (*encrypt_connection)(uint8_t device_id, const uint8_t *encryption_key, uint16_t key_length);

    /**
     * @brief 解密连接
     * @param device_id 设备ID
     * @return 解密结果
     */
    comm_result_t (*decrypt_connection)(uint8_t device_id);

    /**
     * @brief 发送数据
     * @param device_id 设备ID
     * @param data_type 数据类型
     * @param data 数据指针
     * @param data_length 数据长度
     * @return 发送结果
     */
    comm_result_t (*send_data)(uint8_t device_id, comm_data_type_t data_type, const uint8_t *data, uint16_t data_length);

    /**
     * @brief 接收数据
     * @param device_id 设备ID
     * @param data_type 数据类型
     * @param data 输出数据缓冲区
     * @param max_length 最大长度
     * @param received_length 输出接收长度
     * @return 接收结果
     */
    comm_result_t (*receive_data)(uint8_t device_id, comm_data_type_t data_type, uint8_t *data, uint16_t max_length, uint16_t *received_length);

    /**
     * @brief 发送数据包
     * @param packet 数据包指针
     * @return 发送结果
     */
    comm_result_t (*send_packet)(const comm_packet_t *packet);

    /**
     * @brief 接收数据包
     * @param packet 输出数据包
     * @return 接收结果
     */
    comm_result_t (*receive_packet)(comm_packet_t *packet);

    /**
     * @brief 广播数据
     * @param data_type 数据类型
     * @param data 数据指针
     * @param data_length 数据长度
     * @return 广播结果
     */
    comm_result_t (*broadcast_data)(comm_data_type_t data_type, const uint8_t *data, uint16_t data_length);

    /**
     * @brief 多播数据
     * @param device_ids 设备ID数组
     * @param device_count 设备数量
     * @param data_type 数据类型
     * @param data 数据指针
     * @param data_length 数据长度
     * @return 多播结果
     */
    comm_result_t (*multicast_data)(const uint8_t *device_ids, uint8_t device_count, comm_data_type_t data_type, const uint8_t *data, uint16_t data_length);

    /**
     * @brief 异步发送数据
     * @param device_id 设备ID
     * @param data_type 数据类型
     * @param data 数据指针
     * @param data_length 数据长度
     * @param callback 发送完成回调函数
     * @param user_data 用户数据
     * @return 发送结果
     */
    comm_result_t (*send_data_async)(uint8_t device_id, comm_data_type_t data_type, const uint8_t *data, uint16_t data_length, comm_progress_callback_t callback, void *user_data);

    /**
     * @brief 异步接收数据
     * @param device_id 设备ID
     * @param data_type 数据类型
     * @param buffer 接收缓冲区
     * @param buffer_size 缓冲区大小
     * @param callback 接收完成回调函数
     * @param user_data 用户数据
     * @return 接收结果
     */
    comm_result_t (*receive_data_async)(uint8_t device_id, comm_data_type_t data_type, uint8_t *buffer, uint16_t buffer_size, comm_progress_callback_t callback, void *user_data);

    /**
     * @brief 获取设备信息
     * @param device_id 设备ID
     * @param device_info 输出设备信息
     * @return 获取结果
     */
    comm_result_t (*get_device_info)(uint8_t device_id, comm_device_info_t *device_info);

    /**
     * @brief 获取已连接设备列表
     * @param device_list 输出设备列表
     * @param max_devices 最大设备数量
     * @param device_count 输出设备数量
     * @return 获取结果
     */
    comm_result_t (*get_connected_devices)(comm_device_info_t *device_list, uint8_t max_devices, uint8_t *device_count);

    /**
     * @brief 获取已配对设备列表
     * @param device_list 输出设备列表
     * @param max_devices 最大设备数量
     * @param device_count 输出设备数量
     * @return 获取结果
     */
    comm_result_t (*get_paired_devices)(comm_device_info_t *device_list, uint8_t max_devices, uint8_t *device_count);

    /**
     * @brief 获取已绑定设备列表
     * @param device_list 输出设备列表
     * @param max_devices 最大设备数量
     * @param device_count 输出设备数量
     * @return 获取结果
     */
    comm_result_t (*get_bonded_devices)(comm_device_info_t *device_list, uint8_t max_devices, uint8_t *device_count);

    /**
     * @brief 获取通信状态
     * @param status 输出通信状态信息
     * @return 获取结果
     */
    comm_result_t (*get_status)(comm_status_t *status);

    /**
     * @brief 获取统计信息
     * @param statistics 输出统计信息
     * @return 获取结果
     */
    comm_result_t (*get_statistics)(comm_statistics_t *statistics);

    /**
     * @brief 清除统计信息
     * @return 清除结果
     */
    comm_result_t (*clear_statistics)(void);

    /**
     * @brief 获取信号强度
     * @param device_id 设备ID
     * @param rssi 输出信号强度
     * @return 获取结果
     */
    comm_result_t (*get_signal_strength)(uint8_t device_id, int8_t *rssi);

    /**
     * @brief 获取链路质量
     * @param device_id 设备ID
     * @param link_quality 输出链路质量
     * @return 获取结果
     */
    comm_result_t (*get_link_quality)(uint8_t device_id, uint8_t *link_quality);

    /**
     * @brief 获取电池电量
     * @param device_id 设备ID
     * @param battery_level 输出电池电量
     * @return 获取结果
     */
    comm_result_t (*get_battery_level)(uint8_t device_id, uint8_t *battery_level);

    /**
     * @brief 设置信号强度
     * @param power_level 功率等级
     * @return 设置结果
     */
    comm_result_t (*set_power_level)(uint8_t power_level);

    /**
     * @brief 获取信号强度
     * @param power_level 输出功率等级
     * @return 获取结果
     */
    comm_result_t (*get_power_level)(uint8_t *power_level);

    /**
     * @brief 设置广告参数
     * @param interval 广告间隔
     * @param timeout 超时时间
     * @return 设置结果
     */
    comm_result_t (*set_advertisement_parameters)(uint8_t interval, uint16_t timeout);

    /**
     * @brief 开始广告
     * @return 开始广告结果
     */
    comm_result_t (*start_advertisement)(void);

    /**
     * @brief 停止广告
     * @return 停止广告结果
     */
    comm_result_t (*stop_advertisement)(void);

    /**
     * @brief 设置扫描参数
     * @param interval 扫描间隔
     * @param window 扫描窗口
     * @return 设置结果
     */
    comm_result_t (*set_scan_parameters)(uint8_t interval, uint8_t window);

    /**
     * @brief 开始扫描
     * @return 开始扫描结果
     */
    comm_result_t (*start_scan)(void);

    /**
     * @brief 更新固件
     * @param device_id 设备ID
     * @param firmware_data 固件数据
     * @param firmware_size 固件大小
     * @param callback 更新进度回调函数
     * @param user_data 用户数据
     * @return 更新结果
     */
    comm_result_t (*update_firmware)(uint8_t device_id, const uint8_t *firmware_data, uint32_t firmware_size, comm_progress_callback_t callback, void *user_data);

    /**
     * @brief 获取固件版本
     * @param device_id 设备ID
     * @param version 输出版本字符串
     * @param max_length 最大长度
     * @return 获取结果
     */
    comm_result_t (*get_firmware_version)(uint8_t device_id, char *version, uint16_t max_length);

    /**
     * @brief 进入引导模式
     * @param device_id 设备ID
     * @return 进入结果
     */
    comm_result_t (*enter_bootloader_mode)(uint8_t device_id);

    /**
     * @brief 退出引导模式
     * @param device_id 设备ID
     * @return 退出结果
     */
    comm_result_t (*exit_bootloader_mode)(uint8_t device_id);

    /**
     * @brief 发送自定义命令
     * @param device_id 设备ID
     * @param command 命令
     * @param parameters 参数
     * @param param_length 参数长度
     * @param response 输出响应
     * @param response_length 响应长度
     * @return 发送结果
     */
    comm_result_t (*send_custom_command)(uint8_t device_id, uint16_t command, const uint8_t *parameters, uint16_t param_length, uint8_t *response, uint16_t *response_length);

    /**
     * @brief 启用事件通知
     * @param callback 事件回调函数
     * @param user_data 用户数据
     * @return 启用结果
     */
    comm_result_t (*enable_event_notification)(comm_callback_t callback, void *user_data);

    /**
     * @brief 禁用事件通知
     * @return 禁用结果
     */
    comm_result_t (*disable_event_notification)(void);

    /**
     * @brief 启用数据通知
     * @param callback 数据回调函数
     * @param user_data 用户数据
     * @return 启用结果
     */
    comm_result_t (*enable_data_notification)(comm_data_callback_t callback, void *user_data);

    /**
     * @brief 禁用数据通知
     * @return 禁用结果
     */
    comm_result_t (*disable_data_notification)(void);

    /**
     * @brief 启用设备通知
     * @param callback 设备回调函数
     * @param user_data 用户数据
     * @return 启用结果
     */
    comm_result_t (*enable_device_notification)(comm_device_callback_t callback, void *user_data);

    /**
     * @brief 禁用设备通知
     * @return 禁用结果
     */
    comm_result_t (*disable_device_notification)(void);

    /**
     * @brief 启用错误通知
     * @param callback 错误回调函数
     * @param user_data 用户数据
     * @return 启用结果
     */
    comm_result_t (*enable_error_notification)(comm_error_callback_t callback, void *user_data);

    /**
     * @brief 禁用错误通知
     * @return 禁用结果
     */
    comm_result_t (*disable_error_notification)(void);

    /**
     * @brief 导出配置
     * @param buffer 输出缓冲区
     * @param buffer_size 缓冲区大小
     * @param data_size 输出数据大小
     * @return 导出结果
     */
    comm_result_t (*export_config)(uint8_t *buffer, uint16_t buffer_size, uint16_t *data_size);

    /**
     * @brief 导入配置
     * @param buffer 输入缓冲区
     * @param data_size 数据大小
     * @return 导入结果
     */
    comm_result_t (*import_config)(const uint8_t *buffer, uint16_t data_size);

    /**
     * @brief 导出设备列表
     * @param buffer 输出缓冲区
     * @param buffer_size 缓冲区大小
     * @param data_size 输出数据大小
     * @return 导出结果
     */
    comm_result_t (*export_device_list)(uint8_t *buffer, uint16_t buffer_size, uint16_t *data_size);

    /**
     * @brief 导入设备列表
     * @param buffer 输入缓冲区
     * @param data_size 数据大小
     * @return 导入结果
     */
    comm_result_t (*import_device_list)(const uint8_t *buffer, uint16_t data_size);

    /**
     * @brief 检查接口是否有效
     * @param interface_type 接口类型
     * @param valid 输出是否有效标志
     * @return 检查结果
     */
    comm_result_t (*is_interface_valid)(comm_interface_type_t interface_type, bool *valid);

    /**
     * @brief 检查协议是否支持
     * @param protocol_type 协议类型
     * @param supported 输出是否支持标志
     * @return 检查结果
     */
    comm_result_t (*is_protocol_supported)(comm_protocol_type_t protocol_type, bool *supported);

    /**
     * @brief 获取最大连接数
     * @return 最大连接数
     */
    uint8_t (*get_max_connections)(void);

    /**
     * @brief 获取最大包大小
     * @return 最大包大小
     */
    uint16_t (*get_max_packet_size)(void);

    /**
     * @brief 获取最大传输速率
     * @return 最大传输速率(bps)
     */
    uint32_t (*get_max_throughput)(void);

    /**
     * @brief 获取支持的接口类型
     * @param interface_types 输出接口类型数组
     * @param max_types 最大类型数量
     * @param type_count 输出类型数量
     * @return 获取结果
     */
    comm_result_t (*get_supported_interface_types)(comm_interface_type_t *interface_types, uint8_t max_types, uint8_t *type_count);

    /**
     * @brief 获取支持的协议类型
     * @param protocol_types 输出协议类型数组
     * @param max_types 最大类型数量
     * @param type_count 输出类型数量
     * @return 获取结果
     */
    comm_result_t (*get_supported_protocol_types)(comm_protocol_type_t *protocol_types, uint8_t max_types, uint8_t *type_count);

    /**
     * @brief 获取支持的特性
     * @param features 输出特性数组
     * @param max_features 最大特性数量
     * @param feature_count 输出特性数量
     * @return 获取结果
     */
    comm_result_t (*get_supported_features)(uint32_t *features, uint8_t max_features, uint8_t *feature_count);

    /**
     * @brief 执行自检
     * @return 自检结果
     */
    comm_result_t (*self_test)(void);

    /**
     * @brief 获取驱动版本信息
     * @return 版本号
     */
    uint32_t (*get_version)(void);
} comm_interface_t;

/* 便捷宏定义 */
#define COMM_SEND_KEYBOARD_DATA(device_id, keys, length) \
    comm_hal->send_data((device_id), COMM_DATA_TYPE_KEYBOARD, (keys), (length))

#define COMM_SEND_MOUSE_DATA(device_id, buttons, x, y, wheel) \
    do { \
        uint8_t mouse_data[6]; \
        mouse_data[0] = (buttons) & 0xFF; \
        mouse_data[1] = (x) & 0xFF; \
        mouse_data[2] = ((x) >> 8) & 0xFF; \
        mouse_data[3] = (y) & 0xFF; \
        mouse_data[4] = ((y) >> 8) & 0xFF; \
        mouse_data[5] = (wheel) & 0xFF; \
        comm_hal->send_data((device_id), COMM_DATA_TYPE_MOUSE, mouse_data, 6); \
    } while(0)

#define COMM_SEND_TOUCH_DATA(device_id, touches, count) \
    comm_hal->send_data((device_id), COMM_DATA_TYPE_TOUCH, (touches), (count) * sizeof(comm_touch_t))

#define COMM_SEND_GESTURE_DATA(device_id, gesture, params, param_length) \
    comm_hal->send_data((device_id), COMM_DATA_TYPE_GESTURE, (params), (param_length))

#define COMM_BROADCAST_KEYBOARD_DATA(keys, length) \
    comm_hal->broadcast_data(COMM_DATA_TYPE_KEYBOARD, (keys), (length))

#define COMM_GET_STATUS() \
    ({ comm_status_t _status; comm_hal->get_status(&_status); _status; })

#define COMM_GET_STATISTICS() \
    ({ comm_statistics_t _stats; comm_hal->get_statistics(&_stats); _stats; })

#define COMM_IS_CONNECTED(device_id) \
    ({ comm_status_t _status = COMM_GET_STATUS(); \
       /* 检查设备是否连接 */ \
       false; })

#define COMM_GET_SIGNAL_STRENGTH(device_id) \
    ({ int8_t _rssi; comm_hal->get_signal_strength((device_id), &_rssi); _rssi; })

#define COMM_GET_BATTERY_LEVEL(device_id) \
    ({ uint8_t _level; comm_hal->get_battery_level((device_id), &_level); _level; })

/* 常用配置宏 */
#define COMM_CONFIG_DEFAULT(interface, protocol) \
    { (interface), (protocol), 115200, 5000, 3, true, true, true, true, false, COMM_SECURITY_NONE, COMM_PRIORITY_NORMAL, 1, 64, 0, 0, 100, 100, 100, false, 0, false, 0, false, false, 3, false, 0, 0, 1000, 128, 1024, 500, 0, false, 0, 0, 0 }

#define COMM_CONFIG_BLE_HIGH_SPEED() \
    { COMM_INTERFACE_BLE, COMM_PROTOCOL_HID, 0, 5000, 3, true, true, true, true, true, COMM_SECURITY_STANDARD, COMM_PRIORITY_HIGH, 5, 64, 0, 3, 160, 80, 20, true, 10, true, 2, true, 5, false, 2, 0, 500, 256, 1024, 100, 1, false, 1, 0, 0 }

#define COMM_CONFIG_USB_HID() \
    { COMM_INTERFACE_USB, COMM_PROTOCOL_HID, 0, 1000, 1, false, false, false, false, false, COMM_SECURITY_NONE, COMM_PRIORITY_HIGH, 1, 64, 0, 0, 0, 0, 0, false, 0, false, 0, false, 1, false, 0, false, 0, 0, 128, 1024, 1, 0, true, 1, 0, 0 }

#define COMM_CONFIG_WIRELESS_24_LOW_POWER() \
    { COMM_INTERFACE_WIRELESS_24, COMM_PROTOCOL_KEYBOARD, 0, 10000, 5, true, false, false, false, false, COMM_SECURITY_BASIC, COMM_PRIORITY_LOW, 1, 32, 0, 2, 200, 200, 10, false, 0, false, 0, false, 1, true, 1, 0, 2000, 64, 512, 10, 0, false, 0, 0, 0 }

#define COMM_CONFIG_UART_DEBUG() \
    { COMM_INTERFACE_UART, COMM_PROTOCOL_CUSTOM, 115200, 1000, 3, false, false, false, false, false, COMM_SECURITY_NONE, COMM_PRIORITY_NORMAL, 1, 128, 0, 0, 0, 0, 0, false, 0, false, 0, false, 1, false, 0, false, 0, 0, 256, 1024, 1, 0, false, 0, 0, 0 }

/* 通信错误处理宏 */
#define COMM_CHECK(expr) \
    do { \
        comm_result_t _result = (expr); \
        if (_result != COMM_OK) { \
            return _result; \
        } \
    } while(0)

#define COMM_CHECK_RET(expr, ret_val) \
    do { \
        comm_result_t _result = (expr); \
        if (_result != COMM_OK) { \
            return (ret_val); \
        } \
    } while(0)

#define COMM_CHECK_TIMEOUT(start_time, timeout_ms) \
    (timer_hal->get_elapsed_ms(start_time, timer_hal->get_tick()) > (timeout_ms))

#define COMM_WAIT_FOR_OPERATION(expr, timeout_ms) \
    do { \
        uint32_t _start_time = timer_hal->get_tick(); \
        while ((expr) != COMM_OK) { \
            if (COMM_CHECK_TIMEOUT(_start_time, timeout_ms)) { \
                return COMM_ERROR_TIMEOUT; \
            } \
        } \
    } while(0)

/* 通信状态检查宏 */
#define COMM_IS_STATE(state) \
    ({ comm_status_t _status = COMM_GET_STATUS(); _status.current_state == (state); })

#define COMM_IS_READY() COMM_IS_STATE(COMM_STATE_READY)
#define COMM_IS_CONNECTED() COMM_IS_STATE(COMM_STATE_CONNECTED)
#define COMM_IS_SCANNING() COMM_IS_STATE(COMM_STATE_SCANNING)
#define COMM_IS_ERROR() COMM_IS_STATE(COMM_STATE_ERROR)
#define COMM_IS_SLEEPING() COMM_IS_STATE(COMM_STATE_SLEEP)
#define COMM_IS_LOW_POWER() COMM_IS_STATE(COMM_STATE_POWER_SAVE)

/* 信号质量判断宏 */
#define COMM_SIGNAL_EXCELLENT(rssi) ((rssi) >= -50)
#define COMM_SIGNAL_GOOD(rssi) ((rssi) >= -60 && (rssi) < -50)
#define COMM_SIGNAL_FAIR(rssi) ((rssi) >= -70 && (rssi) < -60)
#define COMM_SIGNAL_POOR(rssi) ((rssi) >= -80 && (rssi) < -70)
#define COMM_SIGNAL_BAD(rssi) ((rssi) < -80)

#define COMM_SIGNAL_QUALITY_STR(rssi) \
    ((rssi) >= -50 ? "EXCELLENT" : \
     (rssi) >= -60 ? "GOOD" : \
     (rssi) >= -70 ? "FAIR" : \
     (rssi) >= -80 ? "POOR" : "BAD")

/* 电池电量判断宏 */
#define COMM_BATTERY_FULL(level) ((level) >= 90)
#define COMM_BATTERY_HIGH(level) ((level) >= 70 && (level) < 90)
#define COMM_BATTERY_MEDIUM(level) ((level) >= 40 && (level) < 70)
#define COMM_BATTERY_LOW(level) ((level) >= 20 && (level) < 40)
#define COMM_BATTERY_CRITICAL(level) ((level) < 20)

#define COMM_BATTERY_STATUS_STR(level) \
    ((level) >= 90 ? "FULL" : \
     (level) >= 70 ? "HIGH" : \
     (level) >= 40 ? "MEDIUM" : \
     (level) >= 20 ? "LOW" : "CRITICAL")

/* 安全等级字符串宏 */
#define COMM_SECURITY_LEVEL_STR(level) \
    ((level) == COMM_SECURITY_NONE ? "NONE" : \
     (level) == COMM_SECURITY_BASIC ? "BASIC" : \
     (level) == COMM_SECURITY_STANDARD ? "STANDARD" : \
     (level) == COMM_SECURITY_HIGH ? "HIGH" : \
     (level) == COMM_SECURITY_MAXIMUM ? "MAXIMUM" : \
     (level) == COMM_SECURITY_ENTERPRISE ? "ENTERPRISE" : \
     (level) == COMM_SECURITY_MILITARY ? "MILITARY" : "UNKNOWN")

/* 错误字符串宏 */
#define COMM_ERROR_STRING(result) \
    ((result) == COMM_OK ? "OK" : \
     (result) == COMM_ERROR_INVALID_INTERFACE ? "INVALID_INTERFACE" : \
     (result) == COMM_ERROR_INVALID_CHANNEL ? "INVALID_CHANNEL" : \
     (result) == COMM_ERROR_INVALID_PARAMETER ? "INVALID_PARAMETER" : \
     (result) == COMM_ERROR_NOT_SUPPORTED ? "NOT_SUPPORTED" : \
     (result) == COMM_ERROR_HARDWARE_FAULT ? "HARDWARE_FAULT" : \
     (result) == COMM_ERROR_BUSY ? "BUSY" : \
     (result) == COMM_ERROR_TIMEOUT ? "TIMEOUT" : \
     (result) == COMM_ERROR_ALREADY_INITIALIZED ? "ALREADY_INITIALIZED" : \
     (result) == COMM_ERROR_NOT_INITIALIZED ? "NOT_INITIALIZED" : \
     (result) == COMM_ERROR_ALREADY_CONNECTED ? "ALREADY_CONNECTED" : \
     (result) == COMM_ERROR_NOT_CONNECTED ? "NOT_CONNECTED" : \
     (result) == COMM_ERROR_CONNECTION_LOST ? "CONNECTION_LOST" : \
     (result) == COMM_ERROR_CONNECTION_FAILED ? "CONNECTION_FAILED" : \
     (result) == COMM_ERROR_TRANSMIT_FAILED ? "TRANSMIT_FAILED" : \
     (result) == COMM_ERROR_RECEIVE_FAILED ? "RECEIVE_FAILED" : \
     (result) == COMM_ERROR_BUFFER_FULL ? "BUFFER_FULL" : \
     (result) == COMM_ERROR_BUFFER_EMPTY ? "BUFFER_EMPTY" : "UNKNOWN_ERROR")

/* 性能监控宏 */
#define COMM_PERFORMANCE_START(device_id) \
    ({ uint32_t _start_time = timer_hal->get_tick(); \
       uint32_t _start_bytes = 0; \
       comm_hal->get_status(&_status); _start_bytes = _status.bytes_transmitted; \
       (_start_time), (_start_bytes); })

#define COMM_PERFORMANCE_STOP(device_id, start_time, start_bytes) \
    ({ comm_status_t _status; \
       uint32_t _stop_time = timer_hal->get_tick(); \
       uint32_t _duration_ms = timer_hal->get_elapsed_ms(start_time, _stop_time); \
       comm_hal->get_status(&_status); \
       uint32_t _bytes_transferred = _status.bytes_transmitted - start_bytes; \
       uint32_t _throughput_bps = (_duration_ms > 0) ? (_bytes_transferred * 1000 / _duration_ms) : 0; \
       (_duration_ms), (_bytes_transferred), (_throughput_bps); })

#ifdef __cplusplus
}
#endif

#endif /* COMMUNICATION_INTERFACE_H */