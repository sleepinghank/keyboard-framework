/**
 * @file storage_interface.h
 * @brief 存储设备驱动接口定义
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * 提供EEPROM、Flash、外部存储等存储设备的统一接口
 */

#ifndef STORAGE_INTERFACE_H
#define STORAGE_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 存储设备错误码定义 */
typedef enum {
    STORAGE_OK = 0,                 /* 操作成功 */
    STORAGE_ERROR_INVALID_DEVICE,   /* 无效设备 */
    STORAGE_ERROR_INVALID_PARAM,    /* 无效参数 */
    STORAGE_ERROR_NOT_SUPPORTED,   /* 操作不支持 */
    STORAGE_ERROR_HARDWARE_FAULT,   /* 硬件故障 */
    STORAGE_ERROR_TIMEOUT,          /* 操作超时 */
    STORAGE_ERROR_ALREADY_INITIALIZED, /* 已初始化 */
    STORAGE_ERROR_NOT_INITIALIZED,   /* 未初始化 */
    STORAGE_ERROR_INVALID_ADDRESS,   /* 无效地址 */
    STORAGE_ERROR_ADDRESS_OVERFLOW,  /* 地址溢出 */
    STORAGE_ERROR_WRITE_PROTECTED,   /* 写保护 */
    STORAGE_ERROR_WRITE_FAILED,      /* 写入失败 */
    STORAGE_ERROR_READ_FAILED,       /* 读取失败 */
    STORAGE_ERROR_ERASE_FAILED,       /* 擦除失败 */
    STORAGE_ERROR_VERIFY_FAILED,      /* 校验失败 */
    STORAGE_ERROR_BUFFER_TOO_SMALL,   /* 缓冲区太小 */
    STORAGE_ERROR_NO_MEMORY,         /* 内存不足 */
    STORAGE_ERROR_CORRUPTED,         /* 数据损坏 */
    STORAGE_ERROR_NOT_FOUND,         /* 数据未找到 */
    STORAGE_ERROR_NOT_EMPTY,         /* 存储非空 */
    STORAGE_ERROR_ALREADY_EXISTS     /* 数据已存在 */
} storage_result_t;

/* 存储设备类型定义 */
typedef enum {
    STORAGE_TYPE_EEPROM = 0,         /* EEPROM存储器 */
    STORAGE_TYPE_FLASH,              /* 内部Flash */
    STORAGE_TYPE_EXTERNAL_FLASH,      /* 外部Flash */
    STORAGE_TYPE_FRAM,               /* FRAM存储器 */
    STORAGE_TYPE_SDCARD,             /* SD卡 */
    STORAGE_TYPE_SPI_NOR,            /* SPI NOR Flash */
    STORAGE_TYPE_I2C_EEPROM,         /* I2C EEPROM */
    STORAGE_TYPE_USB_STORAGE,        /* USB存储 */
    STORAGE_TYPE_VIRTUAL             /* 虚拟存储 */
} storage_device_type_t;

/* 存储设备状态定义 */
typedef enum {
    STORAGE_STATE_UNINITIALIZED = 0,  /* 未初始化 */
    STORAGE_STATE_READY,             /* 就绪状态 */
    STORAGE_STATE_BUSY,              /* 忙碌状态 */
    STORAGE_STATE_WRITE_PROTECTED,   /* 写保护状态 */
    STORAGE_STATE_ERROR,             /* 错误状态 */
    STORAGE_STATE_SUSPENDED,         /* 挂起状态 */
    STORAGE_STATE_FORMATTING,        /* 格式化状态 */
    STORAGE_STATE_ERASING,           /* 擦除状态 */
    STORAGE_STATE_SCANNING           /* 扫描状态 */
} storage_device_state_t;

/* 存储器操作类型定义 */
typedef enum {
    STORAGE_OP_READ = 0,             /* 读操作 */
    STORAGE_OP_WRITE,                /* 写操作 */
    STORAGE_OP_ERASE,                /* 擦除操作 */
    STORAGE_OP_VERIFY,               /* 校验操作 */
    STORAGE_OP_FORMAT,               /* 格式化操作 */
    STORAGE_OP_SCAN,                 /* 扫描操作 */
    STORAGE_OP_MOUNT,                /* 挂载操作 */
    STORAGE_OP_UNMOUNT,              /* 卸载操作 */
    STORAGE_OP_BACKUP,               /* 备份操作 */
    STORAGE_OP_RESTORE               /* 恢复操作 */
} storage_operation_type_t;

/* 存储区域类型定义 */
typedef enum {
    STORAGE_AREA_CONFIG = 0,         /* 配置数据区域 */
    STORAGE_AREA_KEYMAP,             /* 按键映射区域 */
    STORAGE_AREA_PROFILE,            /* 用户配置区域 */
    STORAGE_AREA_LOG,                /* 日志数据区域 */
    STORAGE_AREA_TEMP,               /* 临时数据区域 */
    STORAGE_AREA_BACKUP,             /* 备份数据区域 */
    STORAGE_AREA_SYSTEM,             /* 系统数据区域 */
    STORAGE_AREA_FIRMWARE,           /* 固件数据区域 */
    STORAGE_AREA_USER                /* 用户数据区域 */
} storage_area_type_t;

/* EEPROM配置结构体 */
typedef struct {
    uint16_t                size;           /* 存储容量(字节) */
    uint8_t                 page_size;      /* 页大小(字节) */
    uint8_t                 write_cycle;    /* 写周期时间(毫秒) */
    uint16_t                max_erase_cycles; /* 最大擦写次数 */
    bool                    write_protect;  /* 写保护 */
    uint8_t                 address_width;  /* 地址宽度(位) */
    uint32_t                base_address;   /* 基地址 */
} eeprom_config_t;

/* Flash配置结构体 */
typedef struct {
    uint32_t                size;           /* 存储容量(字节) */
    uint16_t                sector_size;    /* 扇区大小(字节) */
    uint8_t                 page_size;      /* 页大小(字节) */
    uint16_t                max_erase_cycles; /* 最大擦写次数 */
    bool                    quad_mode;      /* 四线模式 */
    uint8_t                 address_width;  /* 地址宽度(位) */
    uint32_t                base_address;   /* 基地址 */
    bool                    auto_erase;     /* 自动擦除 */
} flash_config_t;

/* SD卡配置结构体 */
typedef struct {
    uint8_t                 spi_cs_pin;     /* SPI片选引脚 */
    uint8_t                 spi_clk_pin;    /* SPI时钟引脚 */
    uint8_t                 spi_mosi_pin;   /* SPI MOSI引脚 */
    uint8_t                 spi_miso_pin;   /* SPI MISO引脚 */
    uint32_t                clock_speed;    /* SPI时钟频率 */
    uint8_t                 timeout_ms;     /* 操作超时(毫秒) */
    bool                    auto_retries;   /* 自动重试 */
    uint8_t                 max_retries;    /* 最大重试次数 */
} sdcard_config_t;

/* 存储区域配置结构体 */
typedef struct {
    storage_area_type_t     area_type;      /* 区域类型 */
    uint32_t                start_address;  /* 起始地址 */
    uint32_t                size;           /* 区域大小 */
    uint8_t                 access_level;   /* 访问级别 */
    bool                    write_protect;  /* 写保护 */
    bool                    encrypted;      /* 加密 */
    bool                    checksum;       /* 校验和 */
    uint16_t                checksum_size;  /* 校验和大小 */
} storage_area_config_t;

/* 存储设备通用配置结构体 */
typedef struct {
    storage_device_type_t   device_type;    /* 设备类型 */
    storage_device_state_t  state;          /* 设备状态 */
    uint8_t                 priority;       /* 设备优先级 */
    bool                    auto_init;      /* 自动初始化 */
    bool                    auto_mount;     /* 自动挂载 */
    bool                    verify_write;   /* 写入校验 */
    uint8_t                 retry_count;    /* 重试次数 */
    uint32_t                timeout_ms;     /* 操作超时(毫秒) */
    storage_area_config_t  *areas;          /* 存储区域配置 */
    uint8_t                 area_count;     /* 区域数量 */
    union {
        eeprom_config_t     eeprom;         /* EEPROM配置 */
        flash_config_t      flash;          /* Flash配置 */
        sdcard_config_t     sdcard;         /* SD卡配置 */
    } device_config;
} storage_device_config_t;

/* 存储操作信息结构体 */
typedef struct {
    storage_operation_type_t operation;      /* 操作类型 */
    uint32_t                address;        /* 操作地址 */
    uint32_t                size;           /* 操作大小 */
    uint32_t                processed_size; /* 已处理大小 */
    uint32_t                timestamp;      /* 时间戳 */
    storage_result_t        result;         /* 操作结果 */
    uint32_t                duration;       /* 操作耗时(毫秒) */
} storage_operation_info_t;

/* 存储统计信息结构体 */
typedef struct {
    uint32_t                total_reads;    /* 总读取次数 */
    uint32_t                total_writes;   /* 总写入次数 */
    uint32_t                total_erases;   /* 总擦除次数 */
    uint32_t                total_errors;   /* 总错误次数 */
    uint32_t                read_bytes;     /* 读取字节数 */
    uint32_t                write_bytes;    /* 写入字节数 */
    uint32_t                erase_bytes;    /* 擦除字节数 */
    uint32_t                uptime_seconds; /* 运行时间(秒) */
    float                   temperature;    /* 温度(摄氏度) */
    uint32_t                wear_level;     /* 磨损等级 */
} storage_statistics_t;

/* 存储操作回调函数类型 */
typedef void (*storage_operation_callback_t)(storage_operation_info_t *info, void *user_data);

/* 存储设备接口函数指针结构体 */
typedef struct {
    /**
     * @brief 初始化存储设备
     * @param config 设备配置
     * @return 初始化结果
     */
    storage_result_t (*init)(const storage_device_config_t *config);

    /**
     * @brief 反初始化存储设备
     * @return 反初始化结果
     */
    storage_result_t (*deinit)(void);

    /**
     * @brief 挂载存储设备
     * @return 挂载结果
     */
    storage_result_t (*mount)(void);

    /**
     * @brief 卸载存储设备
     * @return 卸载结果
     */
    storage_result_t (*unmount)(void);

    /**
     * @brief 格式化存储设备
     * @return 格式化结果
     */
    storage_result_t (*format)(void);

    /**
     * @brief 读取数据
     * @param address 读取地址
     * @param data 输出数据缓冲区
     * @param size 读取大小
     * @return 读取结果
     */
    storage_result_t (*read)(uint32_t address, void *data, uint32_t size);

    /**
     * @brief 写入数据
     * @param address 写入地址
     * @param data 写入数据
     * @param size 写入大小
     * @return 写入结果
     */
    storage_result_t (*write)(uint32_t address, const void *data, uint32_t size);

    /**
     * @brief 擦除数据
     * @param address 擦除地址
     * @param size 擦除大小
     * @return 擦除结果
     */
    storage_result_t (*erase)(uint32_t address, uint32_t size);

    /**
     * @brief 校验数据
     * @param address 校验地址
     * @param data 参考数据
     * @param size 校验大小
     * @return 校验结果
     */
    storage_result_t (*verify)(uint32_t address, const void *data, uint32_t size);

    /**
     * @brief 同步缓存
     * @return 同步结果
     */
    storage_result_t (*sync)(void);

    /**
     * @brief 刷新缓存
     * @return 刷新结果
     */
    storage_result_t (*flush)(void);

    /**
     * @brief 获取设备大小
     * @return 设备大小
     */
    uint32_t (*get_size)(void);

    /**
     * @brief 获取空闲大小
     * @return 空闲大小
     */
    uint32_t (*get_free_size)(void);

    /**
     * @brief 获取扇区大小
     * @return 扇区大小
     */
    uint32_t (*get_sector_size)(void);

    /**
     * @brief 获取页大小
     * @return 页大小
     */
    uint32_t (*get_page_size)(void);

    /**
     * @brief 获取设备状态
     * @param state 输出设备状态
     * @return 获取结果
     */
    storage_result_t (*get_state)(storage_device_state_t *state);

    /**
     * @brief 获取统计信息
     * @param stats 输出统计信息
     * @return 获取结果
     */
    storage_result_t (*get_statistics)(storage_statistics_t *stats);

    /**
     * @brief 重置统计信息
     * @return 重置结果
     */
    storage_result_t (*reset_statistics)(void);

    /**
     * @brief 检查地址有效性
     * @param address 检查地址
     * @param size 检查大小
     * @return 检查结果
     */
    bool (*is_address_valid)(uint32_t address, uint32_t size);

    /**
     * @brief 检查设备是否可写
     * @param address 检查地址
     * @return 检查结果
     */
    bool (*is_writable)(uint32_t address);

    /**
     * @brief 设置写保护
     * @param enable 启用写保护
     * @return 设置结果
     */
    storage_result_t (*set_write_protect)(bool enable);

    /**
     * @brief 获取写保护状态
     * @param protected 输出写保护状态
     * @return 获取结果
     */
    storage_result_t (*get_write_protect)(bool *protected);

    /**
     * @brief 开始擦除操作
     * @param address 擦除地址
     * @param size 擦除大小
     * @param callback 操作回调
     * @param user_data 用户数据
     * @return 开始擦除结果
     */
    storage_result_t (*start_erase_async)(uint32_t address, uint32_t size,
                                           storage_operation_callback_t callback, void *user_data);

    /**
     * @brief 开始写入操作
     * @param address 写入地址
     * @param data 写入数据
     * @param size 写入大小
     * @param callback 操作回调
     * @param user_data 用户数据
     * @return 开始写入结果
     */
    storage_result_t (*start_write_async)(uint32_t address, const void *data, uint32_t size,
                                          storage_operation_callback_t callback, void *user_data);

    /**
     * @brief 取消异步操作
     * @return 取消结果
     */
    storage_result_t (*cancel_async_operation)(void);

    /**
     * @brief 检查异步操作状态
     * @param completed 输出是否完成标志
     * @return 检查结果
     */
    storage_result_t (*is_async_operation_complete)(bool *completed);

    /**
     * @brief 等待异步操作完成
     * @param timeout_ms 超时时间(毫秒)
     * @return 等待结果
     */
    storage_result_t (*wait_async_operation_complete)(uint32_t timeout_ms);

    /**
     * @brief 设备更新处理
     * @return 更新结果
     */
    storage_result_t (*update)(void);

    /**
     * @brief 重置设备
     * @return 重置结果
     */
    storage_result_t (*reset)(void);

    /**
     * @brief 获取设备信息
     * @param info 输出设备信息
     * @return 获取结果
     */
    storage_result_t (*get_info)(char *info, uint16_t max_len);

    /**
     * @brief 获取设备版本
     * @return 版本号
     */
    uint32_t (*get_version)(void);
} storage_device_interface_t;

/* 存储管理器接口函数指针结构体 */
typedef struct {
    /**
     * @brief 初始化存储管理器
     * @return 初始化结果
     */
    storage_result_t (*init)(void);

    /**
     * @brief 反初始化存储管理器
     * @return 反初始化结果
     */
    storage_result_t (*deinit)(void);

    /**
     * @brief 注册存储设备
     * @param device_type 设备类型
     * @param device_interface 设备接口
     * @param config 设备配置
     * @param device_id 输出设备ID
     * @return 注册结果
     */
    storage_result_t (*register_device)(storage_device_type_t device_type,
                                         const storage_device_interface_t *device_interface,
                                         const storage_device_config_t *config,
                                         uint8_t *device_id);

    /**
     * @brief 注销存储设备
     * @param device_id 设备ID
     * @return 注销结果
     */
    storage_result_t (*unregister_device)(uint8_t device_id);

    /**
     * @brief 挂载指定设备
     * @param device_id 设备ID
     * @return 挂载结果
     */
    storage_result_t (*mount_device)(uint8_t device_id);

    /**
     * @brief 卸载指定设备
     * @param device_id 设备ID
     * @return 卸载结果
     */
    storage_result_t (*unmount_device)(uint8_t device_id);

    /**
     * @brief 挂载所有设备
     * @return 挂载结果
     */
    storage_result_t (*mount_all_devices)(void);

    /**
     * @brief 卸载所有设备
     * @return 卸载结果
     */
    storage_result_t (*unmount_all_devices)(void);

    /**
     * @brief 读取数据到指定区域
     * @param area_type 区域类型
     * @param offset 区域内偏移
     * @param data 输出数据缓冲区
     * @param size 读取大小
     * @return 读取结果
     */
    storage_result_t (*read_area)(storage_area_type_t area_type, uint32_t offset,
                                  void *data, uint32_t size);

    /**
     * @brief 写入数据到指定区域
     * @param area_type 区域类型
     * @param offset 区域内偏移
     * @param data 写入数据
     * @param size 写入大小
     * @return 写入结果
     */
    storage_result_t (*write_area)(storage_area_type_t area_type, uint32_t offset,
                                   const void *data, uint32_t size);

    /**
     * @brief 擦除指定区域
     * @param area_type 区域类型
     * @param offset 区域内偏移
     * @param size 擦除大小
     * @return 擦除结果
     */
    storage_result_t (*erase_area)(storage_area_type_t area_type, uint32_t offset,
                                   uint32_t size);

    /**
     * @brief 获取区域大小
     * @param area_type 区域类型
     * @param size 输出区域大小
     * @return 获取结果
     */
    storage_result_t (*get_area_size)(storage_area_type_t area_type, uint32_t *size);

    /**
     * @brief 获取区域信息
     * @param area_type 区域类型
     * @param area_config 输出区域配置
     * @return 获取结果
     */
    storage_result_t (*get_area_info)(storage_area_type_t area_type,
                                      storage_area_config_t *area_config);

    /**
     * @brief 同步所有设备
     * @return 同步结果
     */
    storage_result_t (*sync_all_devices)(void);

    /**
     * @brief 备份数据
     * @param source_device 源设备ID
     * @param target_device 目标设备ID
     * @param area_type 区域类型
     * @return 备份结果
     */
    storage_result_t (*backup_area)(uint8_t source_device, uint8_t target_device,
                                     storage_area_type_t area_type);

    /**
     * @brief 恢复数据
     * @param source_device 源设备ID
     * @param target_device 目标设备ID
     * @param area_type 区域类型
     * @return 恢复结果
     */
    storage_result_t (*restore_area)(uint8_t source_device, uint8_t target_device,
                                      storage_area_type_t area_type);

    /**
     * @brief 获取最佳存储设备
     * @param size 所需存储大小
     * @param writable 是否需要可写
     * @param device_id 输出设备ID
     * @return 获取结果
     */
    storage_result_t (*get_best_device)(uint32_t size, bool writable, uint8_t *device_id);

    /**
     * @brief 处理所有设备更新
     * @return 处理结果
     */
    storage_result_t (*update_all_devices)(void);

    /**
     * @brief 获取已注册设备数量
     * @return 设备数量
     */
    uint8_t (*get_device_count)(void);

    /**
     * @brief 获取存储管理器版本
     * @return 版本号
     */
    uint32_t (*get_version)(void);
} storage_manager_interface_t;

/* 全局存储管理器实例 */
extern const storage_manager_interface_t *storage_manager;

/* 常用配置宏 */
#define EEPROM_CONFIG_DEFAULT(size) \
    { (size), 64, 5, 100000, false, 8, 0 }

#define FLASH_CONFIG_DEFAULT(size) \
    { (size), 4096, 256, 10000, false, 24, 0, false }

#define SDCARD_CONFIG_DEFAULT(cs, clk, mosi, miso) \
    { (cs), (clk), (mosi), (miso), 4000000, 1000, true, 3 }

#define STORAGE_AREA_CONFIG_DEFAULT(type, addr, size) \
    { (type), (addr), (size), 0, false, false, true, 4 }

#define STORAGE_DEVICE_CONFIG_DEFAULT(type) \
    { (type), STORAGE_STATE_UNINITIALIZED, 0, true, true, true, 3, 1000, NULL, 0, {0} }

/* 存储错误处理宏 */
#define STORAGE_CHECK(expr) \
    do { \
        storage_result_t _result = (expr); \
        if (_result != STORAGE_OK) { \
            return _result; \
        } \
    } while(0)

#define STORAGE_CHECK_RET(expr, ret_val) \
    do { \
        storage_result_t _result = (expr); \
        if (_result != STORAGE_OK) { \
            return (ret_val); \
        } \
    } while(0)

/* 便捷操作宏 */
#define STORAGE_READ_AREA(area, offset, data, size) \
    storage_manager->read_area((area), (offset), (data), (size))

#define STORAGE_WRITE_AREA(area, offset, data, size) \
    storage_manager->write_area((area), (offset), (data), (size))

#define STORAGE_MOUNT_ALL() \
    storage_manager->mount_all_devices()

#define STORAGE_UNMOUNT_ALL() \
    storage_manager->unmount_all_devices()

#define STORAGE_SYNC_ALL() \
    storage_manager->sync_all_devices()

#define STORAGE_UPDATE_ALL() \
    storage_manager->update_all_devices()

#ifdef __cplusplus
}
#endif

#endif /* STORAGE_INTERFACE_H */