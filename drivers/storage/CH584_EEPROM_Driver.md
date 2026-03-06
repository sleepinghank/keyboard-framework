# CH584 EEPROM 驱动实现

## 概述

本文档记录 CH584 平台 EEPROM 驱动的实现细节和验证测试。

## 实现文件

| 文件 | 说明 |
|------|------|
| `drivers/storage/ch584/eeprom.c` | CH584 EEPROM 驱动实现 |
| `drivers/storage/eeprom.h` | EEPROM 抽象接口定义 (QMK 风格) |
| `keyboards/product_config.h` | EEPROM 配置宏定义 |

## 配置宏

在 `product_config.h` 中定义：

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

## 技术特性

### CH584 Data-Flash 规格
- **页大小**: 256 字节
- **块大小**: 4KB
- **最大容量**: 32KB
- **擦除特性**: 写入前必须先擦除（Flash 特性）

### 写入策略
采用 **读-改-写 (Read-Modify-Write)** 策略：
1. 读取整页到 RAM 缓冲区
2. 修改缓冲区中需要更新的部分
3. 擦除整页
4. 写回整个缓冲区

**优点**: 保护同页中其他数据不被意外擦除

## 实现的接口

### 读取函数
| 函数 | 说明 |
|------|------|
| `eeprom_read_byte(addr)` | 读取单字节 |
| `eeprom_read_word(addr)` | 读取 16 位字 |
| `eeprom_read_dword(addr)` | 读取 32 位双字 |
| `eeprom_read_block(buf, addr, len)` | 读取数据块 |

### 写入函数
| 函数 | 说明 |
|------|------|
| `eeprom_write_byte(addr, val)` | 写入单字节 |
| `eeprom_write_word(addr, val)` | 写入 16 位字 |
| `eeprom_write_dword(addr, val)` | 写入 32 位双字 |
| `eeprom_write_block(src, addr, len)` | 写入数据块 |

### 更新函数
与写入函数相同（Flash 特性要求先擦除）

## 硬件验证测试

### 测试环境
- **平台**: CH584M
- **SDK**: WCH CH584 BLE SDK
- **测试日期**: 2026-03-06

### EEPROM 测试结果

```
========== EEPROM Test Start ==========
[EEPROM] Original data at offset 0: 92 8B E8 01 00 00 50 00
[EEPROM] Writing test data: AA BB CC DD 11 22 33 44
[EEPROM] Read back data: AA BB CC DD 11 22 33 44
[EEPROM] Block test: PASSED
[EEPROM] Original data restored
========== EEPROM Test PASSED ==========
```

**测试项目**:
- 块读写测试 (8 字节)
- 单字节读写测试
- 16 位字读写测试
- 32 位双字读写测试
- 原始数据恢复

**结论**: EEPROM 驱动工作正常

### Storage 模块测试结果

```
========== Storage Test Start ==========
[Storage] Module initialized
[Storage] Init check: PASSED - version 1
[Storage] Original values: fn_lock=0, device_type=0, backlight=80, led_mode=0, language=0
[Storage] Modified values: fn_lock=1, device_type=5, backlight=50, led_mode=1, language=2
[Storage] Save: PASSED
[Storage] fn_lock_state: 1 - PASS
[Storage] device_type: 5 - PASS
[Storage] backlight_brightness: 50 - PASS
[Storage] led_mode: 1 - PASS
[Storage] language: 2 - PASS
[Storage] Original config restored: PASSED
========== Storage Test PASSED ==========
```

**测试项目**:
- 模块初始化
- 配置字段修改
- 配置保存
- 配置读取验证
- 原始配置恢复

**结论**: Storage 模块工作正常，CRC16 校验正常

## 集成说明

### 添加到 CH584 项目

1. 将 `drivers/storage/ch584/eeprom.c` 添加到编译配置
2. 确保包含目录中有 `drivers/storage`
3. 确保 WCH SDK 的 `ISP585.h` 可访问

### 初始化顺序

```c
// 在系统初始化时调用
storage_init();  // 这会自动初始化底层 EEPROM
```

## 注意事项

1. **写入延迟**: 每次写入后需要适当延迟（约 10ms）等待 Flash 操作完成
2. **页边界**: 当前实现假设写入数据不跨页，如需跨页写入需扩展实现
3. **磨损均衡**: 当前采用简单策略，频繁写入同一地址会加速磨损

## 版本历史

| 版本 | 日期 | 说明 |
|------|------|------|
| 1.2.0 | 2026-03-06 | 移除条件编译，添加延迟等待 |
| 1.1.0 | 2026-03-06 | 实现读-改-写策略 |
| 1.0.0 | 2026-03-06 | 初始实现 |
