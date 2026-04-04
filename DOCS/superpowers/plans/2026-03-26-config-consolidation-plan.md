# 产品配置整合实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将分散的配置文件整合为单一入口，废弃 sys_config.h，新增 system_enums.h

**Architecture:** 扁平合并方案 — 合并 sys_config.h 到产品 config.h，枚举抽取到独立头文件

**Tech Stack:** C 语言、CMake 构建系统、CH584M 嵌入式平台

**Spec:** `docs/superpowers/specs/2026-03-26-config-consolidation-design.md`

---

## 文件变更清单

| 操作 | 文件路径 |
|------|----------|
| 新建 | `keyboards/system_enums.h` |
| 重写 | `keyboards/kb904/config.h` |
| 修改 | `keyboards/config_check.h` |
| 修改 | `application/main.c` |
| 修改 | `test/test_main.c` |
| 修改 | `application/system/system_init.c` |
| 修改 | `application/system/indicator_test.h` |
| 修改 | `drivers/system/timer_manager.h` |
| 修改 | `drivers/system/timer.c` |
| 修改 | `drivers/system/platforms/test/event_manager.c` |
| 修改 | `drivers/system/event_manager.h` |
| 修改 | `drivers/system/atomic_util.h` |
| 修改 | `middleware/keyboard/keyboard.c` |
| 修改 | `hal/platforms/test/_uart.c` |
| 修改 | `drivers/communication/bluetooth/bt_driver.h` |
| 修改 | `hal/uart.h` |
| 删除 | `application/sys_config.h` |

---

## Task 1: 创建 system_enums.h

**Files:**
- Create: `keyboards/system_enums.h`

- [ ] **Step 1: 创建枚举头文件**

```c
#pragma once

/**
 * @file system_enums.h
 * @brief 跨模块共用枚举定义
 * @note 仅放置被多个模块引用的枚举，模块内部枚举保留在各自头文件
 */

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
    PRINTF_LEVEL_ASSERT = 0,   /**< 断言 */
    PRINTF_LEVEL_DEBUG  = 1,   /**< 调试信息 */
    PRINTF_LEVEL_INFO   = 2,   /**< 普通信息 */
    PRINTF_LEVEL_WARN   = 3,   /**< 警告信息 */
    PRINTF_LEVEL_ERROR  = 4,   /**< 错误信息 */
    PRINTF_LEVEL_VERBOSE = 5   /**< 详细信息 */
} printf_level_t;

/* ========== 其他跨模块枚举 ========== */
// 如有其他共用枚举，按需添加
// 注意：transport_t 已在 middleware/communication/transport.h 定义

#ifdef __cplusplus
}
#endif
```

- [ ] **Step 2: 验证文件创建**

Run: `ls -la keyboards/system_enums.h`
Expected: 文件存在且内容正确

---

## Task 2: 重写 config.h

**Files:**
- Modify: `keyboards/kb904/config.h`

- [ ] **Step 1: 备份原文件**

Run: `cp keyboards/kb904/config.h keyboards/kb904/config.h.bak`

- [ ] **Step 2: 写入合并后的配置**

```c
#pragma once

/**
 * @file config.h
 * @brief KB904 产品配置（合并 sys_config.h）
 * @note 配置入口统一在此文件
 */

#include "system_enums.h"

/* ============================================================
 * 第一部分：芯片与系统配置（原 sys_config.h）
 * ============================================================ */

/* ---------- 芯片型号 ---------- */
#define CHIP_TYPE          CHIP_CH584M

/* ---------- 日志配置 ---------- */
#define PRINTF_ENABLE      TRUE
#define INDICATOR_TEST_ENABLE

#if (PRINTF_ENABLE == TRUE)
    #define PRINTF_LEVEL              PRINTF_LEVEL_DEBUG
    #define PRINTF_UART               PLATFORM_UART_3
    #define PRINTF_BAUDRATE           921600
    #define PRINTF_IO_MAP             PLATFORM_UART_IO_MAP_ENABLE
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

/* ---------- 功能总开关 ---------- */
#define USB_ENABLE         TRUE
#define BLE_ENABLE         TRUE

#if (BLE_ENABLE == TRUE)
    #define BLE_NAME                   "CH584M_BLE"
    #define BLE_ADVERTISE_INTERVAL     1000
#endif

/* ============================================================
 * 第二部分：产品标识
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
#include "defaults/power_defaults.h"
#include "defaults/matrix_defaults.h"

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

---

## Task 3: 更新 include 引用（application/）

**Files:**
- Modify: `application/main.c`
- Modify: `application/system/system_init.c`
- Modify: `application/system/indicator_test.h`

- [ ] **Step 1: 更新 main.c**

将 `#include "sys_config.h"` 改为 `#include "kb904/config.h"`

- [ ] **Step 2: 更新 system_init.c**

将 `#include "sys_config.h"` 改为 `#include "kb904/config.h"`

- [ ] **Step 3: 更新 indicator_test.h**

将 `#include "sys_config.h"` 改为 `#include "kb904/config.h"`

---

## Task 4: 更新 include 引用（drivers/）

**Files:**
- Modify: `drivers/system/timer_manager.h`
- Modify: `drivers/system/timer.c`
- Modify: `drivers/system/event_manager.h`
- Modify: `drivers/system/atomic_util.h`
- Modify: `drivers/system/platforms/test/event_manager.c`
- Modify: `drivers/communication/bluetooth/bt_driver.h`

- [ ] **Step 1: 更新 timer_manager.h**

将 `#include "sys_config.h"` 改为 `#include "kb904/config.h"`

- [ ] **Step 2: 更新 timer.c**

将 `#include "sys_config.h"` 改为 `#include "kb904/config.h"`

- [ ] **Step 3: 更新 event_manager.h**

将 `#include "sys_config.h"` 改为 `#include "kb904/config.h"`

- [ ] **Step 4: 更新 atomic_util.h**

将 `#include "sys_config.h"` 改为 `#include "kb904/config.h"`

- [ ] **Step 5: 更新 platforms/test/event_manager.c**

将 `#include "sys_config.h"` 改为 `#include "kb904/config.h"`

- [ ] **Step 6: 更新 bt_driver.h**

将 `#include "sys_config.h"` 改为 `#include "kb904/config.h"`

---

## Task 5: 更新 include 引用（hal/ 和 middleware/）

**Files:**
- Modify: `hal/uart.h`
- Modify: `hal/platforms/test/_uart.c`
- Modify: `middleware/keyboard/keyboard.c`

- [ ] **Step 1: 更新 hal/uart.h**

将 `#include "sys_config.h"` 改为 `#include "kb904/config.h"`

- [ ] **Step 2: 更新 hal/platforms/test/_uart.c**

将 `#include "sys_config.h"` 改为 `#include "kb904/config.h"`

- [ ] **Step 3: 更新 middleware/keyboard/keyboard.c**

将 `#include "sys_config.h"` 改为 `#include "kb904/config.h"`

---

## Task 6: 更新 include 引用（test/）

**Files:**
- Modify: `test/test_main.c`

- [ ] **Step 1: 更新 test_main.c**

将 `#include "sys_config.h"` 改为 `#include "kb904/config.h"`

---

## Task 7: 更新 config_check.h

**Files:**
- Modify: `keyboards/config_check.h`

- [ ] **Step 1: 添加系统配置验证**

在 `/* ========== 必需配置项验证 ========== */` 部分添加：

```c
/* ========== 系统配置验证 ========== */
#ifndef CHIP_TYPE
    #error "CHIP_TYPE must be defined in config.h"
#endif

#ifndef PRINTF_ENABLE
    #error "PRINTF_ENABLE must be defined in config.h"
#endif

#ifndef USB_ENABLE
    #error "USB_ENABLE must be defined in config.h"
#endif

#ifndef BLE_ENABLE
    #error "BLE_ENABLE must be defined in config.h"
#endif
```

---

## Task 8: 删除 sys_config.h

**Files:**
- Delete: `application/sys_config.h`

- [ ] **Step 1: 确认无引用**

Run: `grep -r "sys_config.h" --include="*.c" --include="*.h" --exclude-dir=docs .`
Expected: 无输出

- [ ] **Step 2: 删除文件**

Run: `rm application/sys_config.h`

- [ ] **Step 3: 删除备份文件**

Run: `rm -f keyboards/kb904/config.h.bak`

---

## Task 9: 验证构建

**Files:**
- None (verification only)

- [ ] **Step 1: CMake 构建验证**

Run: `cmake -S . -B build -G "MinGW Makefiles" && cmake --build build -j4`
Expected: 编译成功，无错误

- [ ] **Step 2: 确认 sys_config.h 引用已清除**

Run: `grep -r "sys_config.h" --include="*.c" --include="*.h" --exclude-dir=docs application/ drivers/ hal/ middleware/ test/`
Expected: 无输出

---

## 验收检查清单

- [ ] `keyboards/system_enums.h` 已创建，包含 `chip_type_t` 和 `printf_level_t`
- [ ] `keyboards/kb904/config.h` 包含完整配置（系统 + 产品）
- [ ] `keyboards/config_check.h` 包含 CHIP_TYPE/PRINTF_ENABLE 验证
- [ ] 所有 14 个文件已更新 include 路径
- [ ] `application/sys_config.h` 已删除
- [ ] CMake 构建通过
