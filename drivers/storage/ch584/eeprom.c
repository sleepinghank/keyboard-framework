/**
 * @file eeprom.c
 * @brief CH584 EEPROM 驱动实现 - 使用 Data-Flash 模拟 EEPROM
 * @version 1.2.0
 * @date 2026-03-06
 *
 * 实现说明:
 * - 使用 CH584 Data-Flash 作为存储介质
 * - 采用读-改-写策略保护同页其他数据
 * - 支持最大 512 字节存储空间
 */

#include "eeprom.h"
#include "ISP585.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* 读-改-写策略所需的页缓冲区 */
static uint8_t page_buffer[EEPROM_PAGE_SIZE];

/*============================================================================
 * 读取函数实现
 *============================================================================*/

/**
 * @brief 从 EEPROM 读取单字节
 *
 * @param __p 地址指针（作为偏移量使用）
 * @return 读取的字节值
 */
uint8_t eeprom_read_byte(const uint8_t *__p) {
    uint8_t value = 0xFF;
    uintptr_t offset = (uintptr_t)__p;

    if (offset < EEPROM_SIZE) {
        EEPROM_READ(EEPROM_BASE_ADDR + offset, &value, 1);
    }
    return value;
}

/**
 * @brief 从 EEPROM 读取 16 位字
 *
 * @param __p 地址指针（作为偏移量使用）
 * @return 读取的 16 位值（小端序）
 */
uint16_t eeprom_read_word(const uint16_t *__p) {
    const uint8_t *p = (const uint8_t *)__p;
    return (uint16_t)eeprom_read_byte(p) | ((uint16_t)eeprom_read_byte(p + 1) << 8);
}

/**
 * @brief 从 EEPROM 读取 32 位双字
 *
 * @param __p 地址指针（作为偏移量使用）
 * @return 读取的 32 位值（小端序）
 */
uint32_t eeprom_read_dword(const uint32_t *__p) {
    const uint8_t *p = (const uint8_t *)__p;
    return (uint32_t)eeprom_read_byte(p) |
           ((uint32_t)eeprom_read_byte(p + 1) << 8) |
           ((uint32_t)eeprom_read_byte(p + 2) << 16) |
           ((uint32_t)eeprom_read_byte(p + 3) << 24);
}

/**
 * @brief 从 EEPROM 读取数据块
 *
 * @param __dst 目标缓冲区
 * @param __src 源地址（作为偏移量使用）
 * @param __n 读取字节数
 */
void eeprom_read_block(void *__dst, const void *__src, size_t __n) {
    if (__dst == NULL || __n == 0) {
        return;
    }

    uintptr_t offset = (uintptr_t)__src;
    uint8_t *dest = (uint8_t *)__dst;

    if (offset + __n <= EEPROM_SIZE) {
        EEPROM_READ(EEPROM_BASE_ADDR + offset, dest, __n);
    }
}

/*============================================================================
 * 写入函数实现
 *============================================================================*/

/**
 * @brief 向 EEPROM 写入单字节
 *
 * @param __p 地址指针（作为偏移量使用）
 * @param __value 要写入的字节值
 */
void eeprom_write_byte(uint8_t *__p, uint8_t __value) {
    eeprom_write_block(&__value, __p, 1);
}

/**
 * @brief 向 EEPROM 写入 16 位字
 *
 * @param __p 地址指针（作为偏移量使用）
 * @param __value 要写入的 16 位值（小端序）
 */
void eeprom_write_word(uint16_t *__p, uint16_t __value) {
    eeprom_write_block(&__value, __p, 2);
}

/**
 * @brief 向 EEPROM 写入 32 位双字
 *
 * @param __p 地址指针（作为偏移量使用）
 * @param __value 要写入的 32 位值（小端序）
 */
void eeprom_write_dword(uint32_t *__p, uint32_t __value) {
    eeprom_write_block(&__value, __p, 4);
}

/**
 * @brief 向 EEPROM 写入数据块
 *
 * @param __src 源数据缓冲区
 * @param __dst 目标地址（作为偏移量使用）
 * @param __n 写入字节数
 *
 * @note 采用读-改-写策略:
 *       1. 读取整页到 RAM 缓冲区
 *       2. 修改缓冲区中需要更新的部分
 *       3. 擦除整页
 *       4. 写回整个缓冲区
 */
void eeprom_write_block(const void *__src, void *__dst, size_t __n) {
    if (__src == NULL || __n == 0) {
        return;
    }

    uintptr_t offset = (uintptr_t)__dst;

    if (offset + __n > EEPROM_SIZE) {
        return;
    }

    /* 计算所在页的起始偏移 */
    uintptr_t page_offset = offset & ~(EEPROM_PAGE_SIZE - 1);

    /* 1. 读取整页到缓冲区 */
    EEPROM_READ(EEPROM_BASE_ADDR + page_offset, page_buffer, EEPROM_PAGE_SIZE);

    /* 2. 修改缓冲区中需要更新的部分 */
    memcpy(&page_buffer[offset - page_offset], __src, __n);

    /* 3. 擦除整页 */
    EEPROM_ERASE(EEPROM_BASE_ADDR + page_offset, EEPROM_PAGE_SIZE);

    /* 4. 写回整个缓冲区 */
    EEPROM_WRITE(EEPROM_BASE_ADDR + page_offset, page_buffer, EEPROM_PAGE_SIZE);
}

/*============================================================================
 * 更新函数实现
 *============================================================================*/

/**
 * @brief 更新 EEPROM 单字节
 *
 * @param __p 地址指针（作为偏移量使用）
 * @param __value 要写入的字节值
 *
 * @note Flash 特性要求先擦除，update 等同于 write
 */
void eeprom_update_byte(uint8_t *__p, uint8_t __value) {
    eeprom_write_byte(__p, __value);
}

/**
 * @brief 更新 EEPROM 16 位字
 *
 * @param __p 地址指针（作为偏移量使用）
 * @param __value 要写入的 16 位值
 *
 * @note Flash 特性要求先擦除，update 等同于 write
 */
void eeprom_update_word(uint16_t *__p, uint16_t __value) {
    eeprom_write_block(&__value, __p, 2);
}

/**
 * @brief 更新 EEPROM 32 位双字
 *
 * @param __p 地址指针（作为偏移量使用）
 * @param __value 要写入的 32 位值
 *
 * @note Flash 特性要求先擦除，update 等同于 write
 */
void eeprom_update_dword(uint32_t *__p, uint32_t __value) {
    eeprom_write_block(&__value, __p, 4);
}

/**
 * @brief 更新 EEPROM 数据块
 *
 * @param __src 源数据缓冲区
 * @param __dst 目标地址（作为偏移量使用）
 * @param __n 写入字节数
 *
 * @note Flash 特性要求先擦除，update 等同于 write
 */
void eeprom_update_block(const void *__src, void *__dst, size_t __n) {
    eeprom_write_block(__src, __dst, __n);
}
