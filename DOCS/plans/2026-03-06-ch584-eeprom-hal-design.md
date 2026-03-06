# CH584 EEPROM HAL 实现设计文档

## 概述

为 CH584 平台实现 EEPROM HAL 层，使用 Data-Flash 模拟 EEPROM 存储功能，支持键盘配置数据的持久化存储。

## 设计决策

| 项目 | 决策 | 原因 |
|------|------|------|
| 层级位置 | HAL 层 `hal/platforms/ch584/_eeprom.c` | 符合项目分层架构，与平台相关代码隔离 |
| 存储布局 | 单区存储，512 字节（2 页） | 实现简洁，满足当前配置存储需求 |
| 擦写策略 | 整页擦除 + 写入 | 与 CH584 Flash 特性匹配，实现简单可靠 |
| 容量配置 | 页对齐，256 字节倍数 | 与 Flash 页大小对齐，方便操作 |
| 缓存管理 | 无 | 上层 storage.c 负责 |
| 接口实现 | 完整实现 `eeprom.h` 所有接口 | 兼容现有 storage 模块 |

## 技术背景

### CH584 Data-Flash 特性
- 页大小：256 字节
- 块大小：4KB
- 最大容量：32KB
- 擦除后才能写入（Flash 特性）
- Data-Flash 起始地址：`0x77000 - FLASH_ROM_MAX_SIZE`

### WCH SDK EEPROM 接口
```c
EEPROM_READ(StartAddr, Buffer, Length)    // 读取
EEPROM_ERASE(StartAddr, Length)           // 擦除（页或块）
EEPROM_WRITE(StartAddr, Buffer, Length)   // 写入
```

## 文件结构

```
hal/platforms/ch584/
└── _eeprom.c          # 新增：EEPROM HAL 实现

keyboards/
└── product_config.h   # 修改：添加 EEPROM 配置宏

CMakeLists.txt         # 修改：添加编译条件
```

## 配置宏定义

在 `keyboards/product_config.h` 中添加：

```c
/* EEPROM 配置 - CH584 Data-Flash */
#define EEPROM_SIZE              512        // 分配 2 页 (2 × 256)
#define EEPROM_PAGE_SIZE         256        // CH584 页大小
#define EEPROM_BASE_ADDR         (0x77000 - FLASH_ROM_MAX_SIZE)  // Data-Flash 起始地址
```

## 接口实现

### 需实现的接口（来自 eeprom.h）

```c
// 基础读写
uint8_t  eeprom_read_byte(const uint8_t *addr);
uint16_t eeprom_read_word(const uint16_t *addr);
uint32_t eeprom_read_dword(const uint32_t *addr);
void     eeprom_read_block(void *buf, const void *addr, size_t len);

void     eeprom_write_byte(uint8_t *addr, uint8_t value);
void     eeprom_write_word(uint16_t *addr, uint16_t value);
void     eeprom_write_dword(uint32_t *addr, uint32_t value);
void     eeprom_write_block(const void *buf, void *addr, size_t len);

// update 系列
void     eeprom_update_byte(uint8_t *addr, uint8_t value);
void     eeprom_update_word(uint16_t *addr, uint16_t value);
void     eeprom_update_dword(uint32_t *addr, uint32_t value);
void     eeprom_update_block(const void *buf, void *addr, size_t len);
```

### 实现策略

1. **读取操作**：直接调用 `EEPROM_READ`
2. **写入操作**：先 `EEPROM_ERASE` 擦除页，再 `EEPROM_WRITE` 写入
3. **update 系列**：等同于 write（Flash 特性要求先擦除）
4. **word/dword**：基于 byte/block 实现

## 与现有模块的集成

### storage.c 集成点
- `storage_read_from_eeprom()` → 调用 `eeprom_read_block()`
- `storage_write_to_eeprom()` → 调用 `eeprom_write_block()`

无需修改 `storage.c`，现有接口直接对接。

### CMakeLists.txt 集成
```cmake
# CH584 平台 EEPROM 驱动
if(CHIP_TYPE STREQUAL "CH584")
    list(APPEND SOURCES
        hal/platforms/ch584/_eeprom.c
    )
endif()
```

## 风险与注意事项

1. **Flash 寿命**：每次写入都擦除整页，频繁写入会加速磨损。上层 storage.c 应避免不必要的写入。
2. **原子性**：擦除-写入非原子操作，掉电可能导致数据丢失。上层 CRC 校验可检测。
3. **跨页写入**：当前设计假设写入数据在单页内，跨页需要特殊处理。

## 参考文档

- 参考实现：`D:\Code\CLionProjects\kb04122-13a-wch\BLE\3mode\KEYBOARD\nvs_flash.c`
- WCH SDK：`ISP585.h`
- 现有接口：`drivers/storage/eeprom.h`
