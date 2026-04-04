/**
 * @file storage.h
 * @brief 统一存储模块 - 大结构体存储方案
 * @version 1.0.0
 * @date 2025-12-15
 *
 * 设计特点:
 * - 使用大结构体存储所有配置，易于扩展
 * - 统一的读写机制，基于CRC16校验
 * - 读写锁防止并发访问冲突
 * - 支持出厂设置重置
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* 存储配置 */
#define STORAGE_MAGIC_NUMBER      0xABCD // 存储魔术数字
#define STORAGE_VERSION           2     // 配置版本号




#define LINK_NUM_MAX            2

/*********************************************************************
 * TYPEDEFS
 */

#define USB_VID     0x05AC
#define USB_PID     0x024F
#define USB_REV     0x0100

#define  RF_ROLE_ID_INVALD       (0x07)

#define LED_MODE_ON             0
#define LED_MODE_1Hz            1
#define LED_MODE_2Hz            2
#define LED_MODE_MAX            3

#define LED_LIGHT_0             0
#define LED_LIGHT_25            1
#define LED_LIGHT_50            2
#define LED_LIGHT_75            3
// #define LED_LIGHT_100           4
#define LED_LIGHT_MAX           4//对4取余

#define LED_SPEED_1            0
#define LED_SPEED_2            1
#define LED_SPEED_3            2
#define LED_SPEED_4            3
#define LED_SPEED_5            4
#define LED_SPEED_MAX          5

#define SYS_WIN            0
#define SYS_MAC            1

typedef enum
{
    BLE_INDEX_IDEL = 0,
    BLE_INDEX_1,
    BLE_INDEX_2,
    BLE_INDEX_3,
    BLE_INDEX_MAX,
}access_ble_idx_t;

//#define MODE_USB            0
//#define MODE_2_4G           1
//#define MODE_BT             2
//
//#define SYS_WIN            0
//#define SYS_MAC            1

#define BLE_BOND_FLAG_1             (1<<0)
#define BLE_BOND_FLAG_2             (1<<1)
#define BLE_BOND_FLAG_3             (1<<2)

/* 数据结构定义 */

// 存储头部
typedef struct {
    uint16_t crc16;        // CRC16校验和
    uint8_t  length;       // 数据长度 (0xFF表示无效)
    uint8_t  version;      // 版本号
} storage_header_t;

// 所有配置的集合结构体
typedef struct {
    // 基础配置
    uint8_t   ble_idx;
    uint8_t   ble_bond_flag;
    uint8_t   ble_mac_flag;
    uint8_t   ble_addr_ver[3];
    uint8_t   ble_irk_flag[5];
    uint8_t   ble_name_len;
    uint8_t   ble_name_data[22];
    uint8_t   work_mode;
    // 系统配置
    uint8_t device_type;                // 设备类型 (0-7)
    uint8_t backlight_brightness;       // 背光亮度档位 (BL_LEVEL_*)
    uint8_t backlight_color;            // bit7:开关, bit0~6:颜色索引
    uint8_t language;                   // 语言设置 (0-255)
    // 用户配置
    uint16_t gesture_map[32];           // 手势映射数组 (64字节)
    uint8_t macro_data[32];             // 宏数据 (32字节)
    uint16_t shortcuts[16];             // 快捷键数组 (32字节)
    uint32_t user_preferences[4];       // 用户偏好设置 (16字节)
    // 扩展
    // uint8_t   rf_device_id;
    // uint8_t   peer_mac[6];
    // uint8_t   usb_vid_pid[4];
    // uint8_t   usb_prod_info_len;
    // uint8_t   usb_prod_info[31];
} storage_config_t;

// 存储池结构
typedef struct {
    storage_header_t header;            // 存储头部 (4字节)
    storage_config_t config;            // 配置数据区 (256字节)
} storage_pool_t;

// 锁结构
typedef struct {
    bool write_lock;       // 写入锁状态
    uint8_t owner_id;      // 持有锁的任务ID
    bool pending_write;    // 是否有待写入的数据
} storage_lock_t;

// 写入回调函数类型
typedef void (*storage_write_callback_t)(bool success);

/* API接口 */

// 初始化存储模块
void storage_init(void);

// 读取配置到指定结构体
bool storage_load_config(storage_config_t *config);

// 保存配置到EEPROM
bool storage_save_config(const storage_config_t *config);

// 直接保存当前配置到EEPROM (无需传参，直接保存g_storage_pool)
bool storage_save(void);

// 直接获取配置指针 (最高效)
storage_config_t* storage_get_config_ptr(void);

// 验证配置数据完整性
bool storage_validate_config(const storage_config_t *config);

// 验证EEPROM中的数据
bool storage_validate_eeprom_data(void);

// 重置为出厂设置
void storage_factory_reset(void);

// 设置写入完成回调
void storage_set_write_callback(storage_write_callback_t callback);

// 检查存储是否已初始化
bool storage_is_initialized(void);

// 获取存储版本号
uint8_t storage_get_version(void);

// 计算配置数据的CRC16
uint16_t storage_calculate_crc16(const storage_config_t *config);

/* 便捷宏 */

// 快速访问配置字段

#define STORAGE_GET_DEVICE_TYPE()       (storage_get_config_ptr()->device_type)
#define STORAGE_SET_DEVICE_TYPE(val)    (storage_get_config_ptr()->device_type = (val))

#define STORAGE_GET_BACKLIGHT()         (storage_get_config_ptr()->backlight_brightness)
#define STORAGE_SET_BACKLIGHT(val)      (storage_get_config_ptr()->backlight_brightness = (val))

#define STORAGE_GET_LANGUAGE()          (storage_get_config_ptr()->language)
#define STORAGE_SET_LANGUAGE(val)       (storage_get_config_ptr()->language = (val))

#define STORAGE_GET_BLE_IDX()           (storage_get_config_ptr()->ble_idx)
#define STORAGE_SET_BLE_IDX(val)        (storage_get_config_ptr()->ble_idx = (val))

// 获取手势映射数组指针
#define STORAGE_GET_GESTURE_MAP_PTR()   (storage_get_config_ptr()->gesture_map)

// 获取宏数据指针
#define STORAGE_GET_MACRO_DATA_PTR()    (storage_get_config_ptr()->macro_data)

// 获取快捷键数组指针
#define STORAGE_GET_SHORTCUTS_PTR()     (storage_get_config_ptr()->shortcuts)

// 获取用户偏好数组指针
#define STORAGE_GET_PREFERENCES_PTR()   (storage_get_config_ptr()->user_preferences)
