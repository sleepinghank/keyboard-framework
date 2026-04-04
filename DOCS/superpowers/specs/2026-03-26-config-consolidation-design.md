# 产品配置整合设计

> 状态：待评审
> 日期：2026-03-26
> 作者：Claude Code

## 1. 背景

当前产品配置项分散在多个文件中：

| 文件 | 位置 | 内容 |
|------|------|------|
| `sys_config.h` | application/ | 芯片型号、日志配置、USB/BLE 总开关、枚举定义 |
| `config.h` | keyboards/kb904/ | 产品覆盖值（功能开关、参数覆盖） |
| `config_hw.h` | keyboards/kb904/ | GPIO 硬件映射 |
| `defaults/*.h` | keyboards/defaults/ | 各模块默认值（#ifndef 保护） |
| `config_check.h` | keyboards/ | 编译时配置验证 |

**问题：**
- 配置入口不统一，开发者需要同时关注多个文件
- 枚举定义分散（如 `chip_type_t` 在 sys_config.h，其他枚举在各模块）
- sys_config.h 与 config.h 职责重叠

## 2. 目标

1. 将 `sys_config.h` 合并到产品 `config.h`
2. 新增 `system_enums.h` 存放所有跨模块共用枚举（单一文件）
3. 保持配置入口在产品目录（`keyboards/kb904/config.h`）
4. 废弃 `application/sys_config.h`

## 3. 设计方案

### 3.1 文件结构

```
keyboards/
├── system_enums.h              # 新增：跨模块共用枚举（单一文件）
├── config_check.h              # 保持不变：编译时验证
├── defaults/                   # 保持不变：默认值
│   ├── backlight_defaults.h
│   ├── ble_defaults.h
│   ├── indicator_defaults.h
│   ├── keyboard_defaults.h
│   ├── matrix_defaults.h
│   └── power_defaults.h
└── kb904/
    ├── config.h                # 合并后：产品完整配置入口
    └── config_hw.h             # 保持不变：硬件 GPIO 映射

application/
└── sys_config.h                # 废弃后删除
```

### 3.2 system_enums.h 内容

```c
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ========== 芯片类型 ========== */
typedef enum {
    CHIP_CH584M = 0,
    CHIP_PAR2860 = 1,
} chip_type_t;

/* ========== 日志等级 ========== */
typedef enum {
    PRINTF_LEVEL_ASSERT = 0,
    PRINTF_LEVEL_DEBUG  = 1,
    PRINTF_LEVEL_INFO   = 2,
    PRINTF_LEVEL_WARN   = 3,
    PRINTF_LEVEL_ERROR  = 4,
    PRINTF_LEVEL_VERBOSE = 5,
} printf_level_t;

/* ========== 其他跨模块枚举 ========== */
// 如有其他共用枚举，按需添加
// 注意：transport_mode_t 不在此文件中，已在 middleware/communication/transport.h 定义

#ifdef __cplusplus
}
#endif
```

**原则：** 只放被多个模块引用的枚举，模块内部使用的枚举保留在各自头文件中。
**注意：** `transport_mode_t` 相关枚举已存在于 `middleware/communication/transport.h`（定义为 `transport_t`），不重复定义。

### 3.3 合并后的 config.h 结构

```c
#pragma once

/**
 * @file config.h
 * @brief KB904 产品配置（合并 sys_config.h）
 */

#include "system_enums.h"

/* ============================================================
 * 第一部分：芯片与系统配置（原 sys_config.h）
 * ============================================================ */

/* ---------- 芯片型号 ---------- */
#define CHIP_TYPE          CHIP_CH584M

/* ---------- 日志配置 ---------- */
#define PRINTF_ENABLE      TRUE

#if (PRINTF_ENABLE == TRUE)
    #define PRINTF_LEVEL              PRINTF_LEVEL_DEBUG
    #define PRINTF_UART               PLATFORM_UART_3
    #define PRINTF_BAUDRATE           921600
    #define PRINTF_IO_MAP             PLATFORM_UART_IO_MAP_ENABLE  // 保留原名
    #define LOG_COLOR_ENABLE          TRUE
    #define LOG_FMT_USING_DIR_ENABLE  TRUE
    #define LOG_FMT_USING_LINE_ENABLE TRUE
    #define LOG_FMT_USING_FUNC_ENABLE FALSE
    #define LOG_LINE_BUF_SIZE         1024
    #define LOG_NEWLINE_SIGN          "\r\n"
    #define LOG_LINE_NUM_MAX_LEN      4
    #define LOG_FILTER_TAG_MAX_LEN    20

    #if (CHIP_TYPE == CHIP_CH584M)
        #ifndef __HIGH_CODE
        #define __HIGH_CODE  __attribute__((section(".highcode")))
        #endif
    #else
        #define __HIGH_CODE
    #endif
#endif

#define INDICATOR_TEST_ENABLE

/* ---------- 功能总开关 ---------- */
#define USB_ENABLE         TRUE
#define BLE_ENABLE         TRUE

#if (BLE_ENABLE == TRUE)
    #define BLE_NAME                   "CH584M_BLE"
    #define BLE_ADVERTISE_INTERVAL     1000
#endif

/* ============================================================
 * 第二部分：产品标识（原 config.h）
 * ============================================================ */

#define PRODUCT_ID_VALUE           0x0904
#define ProductModel               "inateck NeoMagic"
#define ChipModel                  "PAR2860"
#define ProductModelCode           (_PRODUCT_inateck_NeoMagic_SE_US)
#define ChipModelCode              (_WCH_584M)
#define HardwareVersion            (0x010000)
#define SoftwareVersion            (0x000001)
#define ProtocolVersion            (0x010500)
#define EncryptFlag                (0x00)

/* ============================================================
 * 第三部分：功能开关与参数覆盖
 * ============================================================ */

#define BLUETOOTH_ENABLE_FLAG      TRUE
#define COMBO_ENABLE               TRUE
#define BACKLIGHT_TYPE_RGB
#define BACKLIGHT_COLOR_COUNT      13
#define BACKLIGHT_DEFAULT_COLOR    BL_COLOR_WHITE
#define BACKLIGHT_DEFAULT_LEVEL    2
#define BACKLIGHT_IDLE_TIMEOUT_MS  5000
#define BLE_PAIRING_NAME           "inateck NeoMagic Pro"
#define BLE_HOST_COUNT             1
#define POWER_DEEP_SLEEP_MS        (10 * 60 * 1000)
#define IND_LED_COUNT              3
#define EEPROM_CUSTOM
#define EEPROM_SIZE                512
#define EEPROM_BASE_ADDR           (0x76000 - FLASH_ROM_MAX_SIZE)

/* ============================================================
 * 第四部分：默认值包含（#ifndef 保护）
 * ============================================================ */

#include "defaults/backlight_defaults.h"
#include "defaults/ble_defaults.h"
#include "defaults/indicator_defaults.h"
#include "defaults/keyboard_defaults.h"
#include "defaults/matrix_defaults.h"
#include "defaults/power_defaults.h"

/* ============================================================
 * 第五部分：硬件配置
 * ============================================================ */

#include "config_hw.h"

/* ============================================================
 * 第六部分：国家/地区配置
 * ============================================================ */

#define KEYBOARD_COUNTRY  COUNTRY_US
#include "country_codes.h"

#define FirmwareCode       "NeoMagic"
#define FirmwareType       "beta"
#define CountryCode        COUNTRY_CODE_STRING
#define ExternalCode       "N0046"
#define MANUFACTURER_NAME_STRING  "Inateck Inc."

/* ============================================================
 * 第七部分：产品 ID 与配置验证
 * ============================================================ */

#ifndef PRODUCT_ID
#define PRODUCT_ID         PRODUCT_ID_VALUE
#endif

#include "config_check.h"
```

**配置层级图：**
```
config.h
  ├── system_enums.h      (枚举定义)
  ├── 芯片/日志/功能开关    (原 sys_config.h)
  ├── 产品标识/参数覆盖
  ├── defaults/*.h        (默认值)
  ├── config_hw.h         (硬件映射)
  ├── country_codes.h     (国家代码)
  └── config_check.h      (验证)
```

## 4. 迁移步骤

### 步骤 1：创建 system_enums.h

在 `keyboards/` 目录下创建 `system_enums.h`，抽取共用枚举。

### 步骤 2：更新 config.h

修改 `keyboards/kb904/config.h`：
- 顶部添加 `#include "system_enums.h"`
- 合并 `application/sys_config.h` 内容到第一部分
- 保持原有产品配置结构
- **修复现有问题**：删除第 2 行的空 `#include ""` 语法错误

### 步骤 3：更新引用

**受影响的文件（共 14 个）：**

| 文件 | 路径 |
|------|------|
| main.c | application/ |
| test_main.c | test/ |
| system_init.c | application/system/ |
| indicator_test.h | application/system/ |
| timer_manager.h | drivers/system/ |
| timer.c | drivers/system/ |
| event_manager.c | drivers/system/platforms/test/ |
| event_manager.h | drivers/system/ |
| atomic_util.h | drivers/system/ |
| keyboard.c | middleware/keyboard/ |
| _uart.c | hal/platforms/test/ |
| bt_driver.h | drivers/communication/bluetooth/ |
| uart.h | hal/ |

**Include 路径策略：**

CMakeLists.txt 已配置 `keyboards/` 为 include 路径，因此所有文件可使用：
```c
// 旧引用
#include "sys_config.h"

// 新引用（统一路径）
#include "kb904/config.h"
```

**注意：** 如果 CMakeLists.txt 未包含 `keyboards/` 路径，需添加：
```cmake
include_directories(${CMAKE_SOURCE_DIR}/keyboards)
```

### 步骤 4：更新 config_check.h

添加对新增必需配置项的验证：
```c
#ifndef CHIP_TYPE
    #error "CHIP_TYPE must be defined in config.h"
#endif

#ifndef PRINTF_ENABLE
    #error "PRINTF_ENABLE must be defined in config.h"
#endif
```

### 步骤 5：删除 sys_config.h

确认无引用后，删除 `application/sys_config.h`。

## 5. 影响范围

| 文件类型 | 改动内容 |
|---------|---------|
| `keyboards/system_enums.h` | 新建 |
| `keyboards/kb904/config.h` | 扩展内容 |
| `keyboards/config_check.h` | 添加新验证规则 |
| 包含 `sys_config.h` 的源文件 | 修改 include 路径 |
| `application/sys_config.h` | 删除 |

## 6. 验收标准

- [ ] 编译通过（CMake + WCH RISC-V）
- [ ] 所有包含 `sys_config.h` 的文件已更新
- [ ] `application/sys_config.h` 已删除
- [ ] `keyboards/system_enums.h` 包含所有跨模块枚举
- [ ] `config_check.h` 包含新的验证规则
- [ ] **功能验证**：键盘启动正常、BLE 广播正常、USB 连接正常、日志输出级别正确
