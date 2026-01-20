/**
 * @file storage.c
 * @brief 统一存储模块实现 - 大结构体存储方案
 */

#include "storage.h"
#include "eeprom.h"  // 使用eeprom.h中的底层读写函数

/* 全局变量 */
static storage_pool_t g_storage_pool;
static storage_lock_t g_storage_lock;
static storage_write_callback_t g_write_callback = NULL;
static bool g_initialized = false;

/* 锁管理函数 */

// 获取写入锁
static bool storage_lock_acquire(uint8_t owner_id) {
    if (g_storage_lock.write_lock) {
        g_storage_lock.pending_write = true;
        return false;
    }
    g_storage_lock.write_lock = true;
    g_storage_lock.owner_id = owner_id;
    return true;
}

// 释放写入锁
static void storage_lock_release(uint8_t owner_id) {
    if (g_storage_lock.owner_id == owner_id) {
        g_storage_lock.write_lock = false;
        g_storage_lock.owner_id = 0;

        if (g_storage_lock.pending_write) {
            g_storage_lock.pending_write = false;
            // 自动触发下一次写入
            storage_save_config(&g_storage_pool.config);
        }
    }
}

/* CRC16计算 */

// CRC16多项式
#define CRC16_POLYNOMIAL 0x1021

// 计算CRC16校验和
static uint16_t calculate_crc16(const uint8_t *data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    uint16_t i, j;

    for (i = 0; i < length; i++) {
        crc ^= ((uint16_t)data[i] << 8);
        for (j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ CRC16_POLYNOMIAL;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

/* 默认配置 */

static const storage_config_t default_config = {
    // 系统配置默认值
    .fn_lock_state = 0,
    .device_type = 0,
    .backlight_brightness = 80,
    .language = 0,
    .led_mode = 1,

    // 用户配置默认值
    .gesture_map = {0},
    .macro_data = {0},
    .shortcuts = {0},
    .user_preferences = {0}
};

/* 存储读写函数 */

// 从EEPROM读取数据
static bool storage_read_from_eeprom(void) {
    eeprom_read_block(&g_storage_pool, (void*)STORAGE_EEPROM_BASE_ADDR, sizeof(storage_pool_t));
    return true;
}

// 向EEPROM写入数据
static bool storage_write_to_eeprom(void) {
    eeprom_write_block(&g_storage_pool, (void*)STORAGE_EEPROM_BASE_ADDR, sizeof(storage_pool_t));
    return true;
}

/* API接口实现 */

// 初始化存储模块
void storage_init(void) {
    // 初始化锁
    g_storage_lock.write_lock = false;
    g_storage_lock.owner_id = 0;
    g_storage_lock.pending_write = false;

    // 加载默认配置
    memcpy(&g_storage_pool.config, &default_config, sizeof(storage_config_t));

    // 设置默认头部
    g_storage_pool.header.length = sizeof(storage_config_t);
    g_storage_pool.header.version = STORAGE_VERSION;

    // 读取EEPROM数据
    storage_read_from_eeprom();

    // 验证数据有效性
    if (!storage_validate_eeprom_data()) {
        // 数据无效，加载默认配置
        storage_factory_reset();
    }

    g_initialized = true;
}

// 读取配置到指定结构体
bool storage_load_config(storage_config_t *config) {
    if (config == NULL) {
        return false;
    }

    memcpy(config, &g_storage_pool.config, sizeof(storage_config_t));
    return true;
}

// 保存配置到EEPROM
bool storage_save_config(const storage_config_t *config) {
    if (config == NULL) {
        return false;
    }

    // 获取锁 (假设当前任务ID为1)
    if (!storage_lock_acquire(1)) {
        return false;  // 锁被占用
    }

    // 更新内存中的配置
    memcpy(&g_storage_pool.config, config, sizeof(storage_config_t));

    // 计算并更新CRC
    g_storage_pool.header.crc16 = storage_calculate_crc16(config);

    // 写入EEPROM
    bool success = storage_write_to_eeprom();

    // 调用回调并释放锁
    if (g_write_callback != NULL) {
        g_write_callback(success);
    }
    storage_lock_release(1);

    return success;
}

// 直接保存当前配置到EEPROM
bool storage_save(void) {
    // 获取锁 (假设当前任务ID为1)
    if (!storage_lock_acquire(1)) {
        return false;  // 锁被占用
    }

    // 计算并更新CRC
    g_storage_pool.header.crc16 = storage_calculate_crc16(&g_storage_pool.config);

    // 写入EEPROM
    bool success = storage_write_to_eeprom();

    // 调用回调并释放锁
    if (g_write_callback != NULL) {
        g_write_callback(success);
    }
    storage_lock_release(1);

    return success;
}

// 获取默认配置
void storage_get_default_config(storage_config_t *config) {
    if (config != NULL) {
        memcpy(config, &default_config, sizeof(storage_config_t));
    }
}

// 直接获取配置指针
storage_config_t* storage_get_config_ptr(void) {
    return &g_storage_pool.config;
}

// 验证配置数据完整性
bool storage_validate_config(const storage_config_t *config) {
    if (config == NULL) {
        return false;
    }

    // 验证数据长度
    if (g_storage_pool.header.length != sizeof(storage_config_t)) {
        return false;
    }

    // 验证版本号
    if (g_storage_pool.header.version != STORAGE_VERSION) {
        return false;
    }

    // 验证CRC
    uint16_t calculated_crc = storage_calculate_crc16(config);
    if (calculated_crc != g_storage_pool.header.crc16) {
        return false;
    }

    return true;
}

// 验证EEPROM中的数据
bool storage_validate_eeprom_data(void) {
    // 检查长度是否为无效值
    if (g_storage_pool.header.length == 0xFF) {
        return false;
    }

    // 检查长度是否正确
    if (g_storage_pool.header.length != sizeof(storage_config_t)) {
        return false;
    }

    // 检查版本号
    if (g_storage_pool.header.version != STORAGE_VERSION) {
        return false;
    }

    // 验证CRC
    uint16_t calculated_crc = storage_calculate_crc16(&g_storage_pool.config);
    if (calculated_crc != g_storage_pool.header.crc16) {
        return false;
    }

    return true;
}

// 重置为出厂设置
void storage_factory_reset(void) {
    // 重置为默认配置
    memcpy(&g_storage_pool.config, &default_config, sizeof(storage_config_t));

    // 更新头部
    g_storage_pool.header.length = sizeof(storage_config_t);
    g_storage_pool.header.version = STORAGE_VERSION;
    g_storage_pool.header.crc16 = storage_calculate_crc16(&g_storage_pool.config);

    // 写入EEPROM
    storage_write_to_eeprom();
}

// 设置写入完成回调
void storage_set_write_callback(storage_write_callback_t callback) {
    g_write_callback = callback;
}

// 检查存储是否已初始化
bool storage_is_initialized(void) {
    return g_initialized;
}

// 获取存储版本号
uint8_t storage_get_version(void) {
    return g_storage_pool.header.version;
}

// 计算配置数据的CRC16
uint16_t storage_calculate_crc16(const storage_config_t *config) {
    if (config == NULL) {
        return 0;
    }
    return calculate_crc16((const uint8_t*)config, sizeof(storage_config_t));
}
