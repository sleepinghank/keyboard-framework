# 统一存储模块

## 概述

本存储模块提供了一个简单、可靠、易于扩展的配置存储解决方案。使用大结构体存储所有配置，基于CRC16校验确保数据完整性，支持读写锁防止并发访问冲突。

## 特性

✅ **易于扩展** - 添加新配置只需在结构体中添加字段
✅ **数据完整** - 基于CRC16的完整性校验
✅ **并发安全** - 读写锁防止访问冲突
✅ **简单易用** - 统一的API接口，便捷宏定义
✅ **高性能** - 内存直接访问，快速读写

## 文件结构

```
drivers/storage/
├── storage.h          # 头文件，包含所有定义和API声明
├── storage.c          # 实现文件，包含所有函数实现
└── README.md          # 说明文档
```

## 快速开始

### 1. 初始化

```c
#include "storage.h"

int main(void) {
    // 初始化存储模块
    storage_init();
    return 0;
}
```

### 2. 读取配置

```c
// 读取完整配置
storage_config_t config;
storage_load_config(&config);

// 或者直接获取指针 (推荐)
storage_config_t* config = storage_get_config_ptr();
uint8_t fn_lock = config->fn_lock_state;
```

### 3. 修改配置

```c
// 方法1: 使用便捷宏
STORAGE_SET_FN_LOCK(1);
STORAGE_SET_BACKLIGHT(80);

// 方法2: 修改结构体
storage_config_t* config = storage_get_config_ptr();
config->fn_lock_state = 1;
config->backlight_brightness = 80;
```

### 4. 保存配置

```c
// 保存完整配置
storage_save_config(&config);

// 或保存当前内存中的配置
storage_save_config(storage_get_config_ptr());
```

## 详细API说明

### 基础函数

#### `void storage_init(void)`
初始化存储模块。加载EEPROM中的配置，如果数据无效则使用默认配置。

#### `bool storage_load_config(storage_config_t *config)`
读取配置到指定结构体。

**参数:**
- `config` - 目标配置结构体指针

**返回值:**
- `true` - 读取成功
- `false` - 读取失败

#### `bool storage_save_config(const storage_config_t *config)`
保存配置到EEPROM。

**参数:**
- `config` - 要保存的配置结构体指针

**返回值:**
- `true` - 保存成功
- `false` - 保存失败（锁被占用或其他错误）

#### `storage_config_t* storage_get_config_ptr(void)`
直接获取配置指针，推荐使用此方式获得最高性能。

**返回值:**
- 配置结构体指针

### 便捷宏

#### 读取配置
```c
uint8_t fn_lock = STORAGE_GET_FN_LOCK();
uint8_t brightness = STORAGE_GET_BACKLIGHT();
uint8_t language = STORAGE_GET_LANGUAGE();
```

#### 修改配置
```c
STORAGE_SET_FN_LOCK(1);
STORAGE_SET_BACKLIGHT(90);
STORAGE_SET_LANGUAGE(2);
```

#### 访问复杂数据
```c
uint16_t* gesture_map = STORAGE_GET_GESTURE_MAP_PTR();
uint8_t* macro_data = STORAGE_GET_MACRO_DATA_PTR();
uint16_t* shortcuts = STORAGE_GET_SHORTCUTS_PTR();
uint32_t* preferences = STORAGE_GET_PREFERENCES_PTR();
```

### 高级功能

#### `void storage_factory_reset(void)`
重置为出厂设置，加载默认配置并保存到EEPROM。

#### `bool storage_validate_config(const storage_config_t *config)`
验证配置数据完整性（CRC校验）。

#### `void storage_set_write_callback(storage_write_callback_t callback)`
设置写入完成回调函数。

```c
void my_callback(bool success) {
    if (success) {
        printf("保存成功\n");
    } else {
        printf("保存失败\n");
    }
}

storage_set_write_callback(my_callback);
```

## 配置结构体

### 系统配置字段

| 字段 | 类型 | 大小 | 说明 |
|------|------|------|------|
| `fn_lock_state` | uint8_t | 1字节 | Fn锁状态 (0-1) |
| `device_type` | uint8_t | 1字节 | 设备类型 (0-7) |
| `backlight_brightness` | uint8_t | 1字节 | 背光亮度 (0-100) |
| `language` | uint8_t | 1字节 | 语言设置 (0-255) |
| `led_mode` | uint8_t | 1字节 | LED模式 (0-255) |

### 用户配置字段

| 字段 | 类型 | 大小 | 说明 |
|------|------|------|------|
| `gesture_map` | uint16_t[32] | 64字节 | 手势映射数组 |
| `macro_data` | uint8_t[32] | 32字节 | 宏数据 |
| `shortcuts` | uint16_t[16] | 32字节 | 快捷键数组 |
| `user_preferences` | uint32_t[4] | 16字节 | 用户偏好设置 |

### 保留空间

- `reserved[128]` - 128字节保留空间，供未来扩展使用

## 添加新配置

### 步骤1: 在storage_config_t中添加字段

```c
typedef struct {
    // ... 现有字段 ...

    // 添加新配置
    uint8_t new_feature_setting;    // 新功能设置
    uint16_t custom_value;           // 自定义值

    // 保留空间相应减少
    uint8_t reserved[126];           // 原来是128，减少2字节
} storage_config_t;
```

### 步骤2: 使用新配置

```c
// 设置新配置
storage_config_t* config = storage_get_config_ptr();
config->new_feature_setting = 1;
config->custom_value = 1234;

// 保存配置
storage_save_config(config);

// 使用便捷宏 (如果需要)
#define STORAGE_GET_NEW_FEATURE() (storage_get_config_ptr()->new_feature_setting)
#define STORAGE_SET_NEW_FEATURE(val) (storage_get_config_ptr()->new_feature_setting = (val))
```

## 使用示例

详见 `examples/storage_example.c` 文件。

## 性能优化

### 推荐做法

1. **使用便捷宏** - 避免重复的指针解引用
2. **直接访问指针** - 使用 `storage_get_config_ptr()` 获得最高性能
3. **批量操作** - 一次性修改多个字段，然后调用一次 `storage_save_config()`
4. **避免频繁保存** - 只在必要时保存，减少EEPROM写入次数

### 示例: 批量更新配置

```c
storage_config_t* config = storage_get_config_ptr();

// 批量修改
config->fn_lock_state = 1;
config->backlight_brightness = 80;
config->language = 1;

// 只保存一次
storage_save_config(config);
```

## 注意事项

1. **EEPROM写入次数有限** - 避免频繁调用 `storage_save_config()`
2. **并发访问** - 写入操作是原子的，读操作可以安全并发
3. **数据完整性** - 系统会自动验证CRC，如果校验失败会加载默认配置
4. **版本兼容性** - 配置结构体版本不匹配时会加载默认配置

## 错误处理

### 常见问题

1. **保存失败** - 检查EEPROM驱动是否正确实现
2. **数据校验失败** - 系统会自动恢复默认配置
3. **锁被占用** - 等待之前的写入操作完成

### 调试建议

1. 启用EEPROM调试输出
2. 检查 `storage_validate_eeprom_data()` 返回值
3. 查看 `storage_write_callback()` 中的成功/失败状态

## 许可证

本模块基于项目整体许可证发布。
