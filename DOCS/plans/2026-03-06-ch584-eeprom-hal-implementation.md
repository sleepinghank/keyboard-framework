# CH584 EEPROM HAL 实现计划

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 为 CH584 平台实现 EEPROM HAL 层，使用 Data-Flash 模拟 EEPROM 存储功能。

**Architecture:** 在 HAL 层创建 `_eeprom.c`，封装 WCH SDK 的 `EEPROM_READ/WRITE/ERASE` 接口，实现 `eeprom.h` 定义的所有函数。写入前先擦除整页，再写入数据。

**Tech Stack:** C11, WCH CH584 SDK, CMake

---

## 前置条件

- WCH SDK 已包含 `ISP585.h` 头文件
- 项目已配置 CH584 平台编译环境
- `FLASH_ROM_MAX_SIZE` 宏已定义

---

## Task 1: 添加 EEPROM 配置宏

**Files:**
- Modify: `keyboards/product_config.h:116-117`

**Step 1: 查看当前 EEPROM 配置位置**

当前配置：
```c
#define EEPROM_CUSTOM
#define EEPROM_SIZE 32
```

**Step 2: 添加 CH584 EEPROM 配置**

在现有 EEPROM 配置下方添加 CH584 平台专用配置：

```c
/* EEPROM 配置 */
#define EEPROM_CUSTOM

#ifdef CHIP_TYPE_CH584
    // CH584 平台配置
    #define EEPROM_SIZE              512        // 分配 2 页 (2 × 256)
    #define EEPROM_PAGE_SIZE         256        // CH584 页大小
    #ifndef FLASH_ROM_MAX_SIZE
        #define FLASH_ROM_MAX_SIZE   0x070000  // Flash-ROM 最大程序大小, 448KB
    #endif
    #define EEPROM_BASE_ADDR         (0x77000 - FLASH_ROM_MAX_SIZE)  // Data-Flash 起始地址
#else
    // Test 平台配置
    #define EEPROM_SIZE 32
#endif
```

**Step 3: 验证配置**

确保 `eeprom.h` 中的 `TOTAL_EEPROM_BYTE_COUNT` 宏能正确使用 `EEPROM_SIZE`。

---

## Task 2: 创建 CH584 EEPROM HAL 实现文件

**Files:**
- Create: `hal/platforms/ch584/_eeprom.c`

**Step 1: 创建文件头部和包含**

```c
/**
 * @file _eeprom.c
 * @brief CH584 EEPROM HAL 实现 - 使用 Data-Flash 模拟 EEPROM
 * @version 1.0.0
 * @date 2026-03-06
 *
 * 实现说明:
 * - 使用 CH584 Data-Flash 作为存储介质
 * - 写入前先擦除整页 (256 字节)
 * - 支持最大 512 字节存储空间
 */

#include "eeprom.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

// WCH SDK EEPROM 接口 (根据实际 SDK 路径调整)
#ifdef CHIP_TYPE_CH584
    #include "ISP585.h"
#endif
```

**Step 2: 实现读取函数**

```c
/**
 * @brief 从 EEPROM 读取单字节
 */
uint8_t eeprom_read_byte(const uint8_t *__p) {
    uint8_t value = 0xFF;
    uintptr_t offset = (uintptr_t)__p;

    if (offset < EEPROM_SIZE) {
#ifdef CHIP_TYPE_CH584
        EEPROM_READ(EEPROM_BASE_ADDR + offset, &value, 1);
#else
        // Test 平台: 返回默认值
        value = 0xFF;
#endif
    }
    return value;
}

/**
 * @brief 从 EEPROM 读取 16 位字
 */
uint16_t eeprom_read_word(const uint16_t *__p) {
    const uint8_t *p = (const uint8_t *)__p;
    return (uint16_t)eeprom_read_byte(p) | ((uint16_t)eeprom_read_byte(p + 1) << 8);
}

/**
 * @brief 从 EEPROM 读取 32 位双字
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
 */
void eeprom_read_block(void *__dst, const void *__src, size_t __n) {
    if (__dst == NULL || __n == 0) return;

    uintptr_t offset = (uintptr_t)__src;
    uint8_t *dest = (uint8_t *)__dst;

    if (offset + __n <= EEPROM_SIZE) {
#ifdef CHIP_TYPE_CH584
        EEPROM_READ(EEPROM_BASE_ADDR + offset, dest, __n);
#else
        // Test 平台: 填充默认值
        memset(dest, 0xFF, __n);
#endif
    }
}
```

**Step 3: 实现写入函数**

```c
/**
 * @brief 向 EEPROM 写入单字节
 */
void eeprom_write_byte(uint8_t *__p, uint8_t __value) {
    eeprom_write_block(&__value, __p, 1);
}

/**
 * @brief 向 EEPROM 写入 16 位字
 */
void eeprom_write_word(uint16_t *__p, uint16_t __value) {
    uint8_t *p = (uint8_t *)__p;
    eeprom_write_byte(p++, (uint8_t)(__value & 0xFF));
    eeprom_write_byte(p, (uint8_t)((__value >> 8) & 0xFF));
}

/**
 * @brief 向 EEPROM 写入 32 位双字
 */
void eeprom_write_dword(uint32_t *__p, uint32_t __value) {
    uint8_t *p = (uint8_t *)__p;
    eeprom_write_byte(p++, (uint8_t)(__value & 0xFF));
    eeprom_write_byte(p++, (uint8_t)((__value >> 8) & 0xFF));
    eeprom_write_byte(p++, (uint8_t)((__value >> 16) & 0xFF));
    eeprom_write_byte(p, (uint8_t)((__value >> 24) & 0xFF));
}

/**
 * @brief 向 EEPROM 写入数据块
 *
 * @note Flash 写入前需要先擦除，这里采用整页擦除策略
 */
void eeprom_write_block(const void *__src, void *__dst, size_t __n) {
    if (__src == NULL || __n == 0) return;

    uintptr_t offset = (uintptr_t)__dst;

    if (offset + __n > EEPROM_SIZE) return;

#ifdef CHIP_TYPE_CH584
    // 擦除整页后再写入
    EEPROM_ERASE(EEPROM_BASE_ADDR, EEPROM_PAGE_SIZE);
    EEPROM_WRITE(EEPROM_BASE_ADDR + offset, (void *)__src, __n);
#else
    // Test 平台: 不执行实际写入
    (void)__src;
    (void)offset;
#endif
}
```

**Step 4: 实现 update 系列函数**

```c
/**
 * @brief 更新 EEPROM 单字节 (等同于 write)
 * @note Flash 特性要求先擦除，update 等同于 write
 */
void eeprom_update_byte(uint8_t *__p, uint8_t __value) {
    eeprom_write_byte(__p, __value);
}

/**
 * @brief 更新 EEPROM 16 位字 (等同于 write)
 */
void eeprom_update_word(uint16_t *__p, uint16_t __value) {
    eeprom_write_word(__p, __value);
}

/**
 * @brief 更新 EEPROM 32 位双字 (等同于 write)
 */
void eeprom_update_dword(uint32_t *__p, uint32_t __value) {
    eeprom_write_dword(__p, __value);
}

/**
 * @brief 更新 EEPROM 数据块 (等同于 write)
 */
void eeprom_update_block(const void *__src, void *__dst, size_t __n) {
    eeprom_write_block(__src, __dst, __n);
}
```

---

## Task 3: 更新 CMakeLists.txt 编译配置

**Files:**
- Modify: `CMakeLists.txt:82-84`

**Step 1: 查看当前 Storage 编译配置**

当前配置：
```cmake
# Drivers - Storage (Test Platform Only)
./drivers/storage/storage.c
./drivers/storage/test/eeprom.c
```

**Step 2: 添加平台条件编译**

将 EEPROM 源文件改为条件编译：

```cmake
# Drivers - Storage
./drivers/storage/storage.c

# EEPROM 驱动 - 根据平台选择
if(DEFINED CHIP_TYPE_CH584)
    # CH584 平台使用 HAL 层实现
    ./hal/platforms/ch584/_eeprom.c
else()
    # Test 平台使用 RAM 模拟
    ./drivers/storage/test/eeprom.c
endif()
```

**注意:** 由于当前 CMakeLists.txt 使用硬编码源文件列表，实际可能需要使用 `set()` 变量或 `option()` 来控制平台选择。

---

## Task 4: 验证编译

**Files:**
- Verify: 编译输出

**Step 1: 清理并重新构建**

```bash
rm -rf build/
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build -j4
```

**Step 2: 检查编译输出**

预期结果：
- 无编译错误
- 无链接错误
- 生成 `keyboard-framework.exe`

---

## 验收标准

1. [ ] `hal/platforms/ch584/_eeprom.c` 文件已创建
2. [ ] 实现 `eeprom.h` 定义的所有 12 个接口函数
3. [ ] `product_config.h` 包含 CH584 EEPROM 配置宏
4. [ ] Test 平台编译通过（使用 `drivers/storage/test/eeprom.c`）
5. [ ] CH584 平台编译配置已准备（需要 WCH SDK 支持）

---

## 风险与注意事项

1. **SDK 依赖:** CH584 编译需要 WCH SDK 的 `ISP585.h`，Test 平台可能没有此文件
2. **条件编译:** 使用 `#ifdef CHIP_TYPE_CH584` 区分平台代码
3. **测试:** 实际 Flash 读写需要在 CH584 硬件上验证
