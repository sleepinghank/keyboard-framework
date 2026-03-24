# CountryCode 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 实现键盘框架的国家/地区支持，支持编译时配置多国版本（US/DE/UK/FR/JP）。

**Architecture:** 编译时隔离方案。每个国家版本独立编译，共享 LAYER_BASE，仅按国家差异替换 LAYER_MACOS/LAYER_WINDOWS。HID Profile 通过宏配置 bCountryCode 字段。

**Tech Stack:** C 语言、GCC 预处理器、WCH CH584M 平台

---

## 文件结构

```
新增文件：
├── keyboards/country_codes.h           # HID 国家代码枚举和辅助宏

修改文件：
├── keyboards/kb904/config.h            # 添加国家配置宏
├── project/ch584m/Profile/hidkbdservice.c  # 使用 HID_COUNTRY_CODE 宏
├── keyboards/kb904/keymaps/default.c   # 按国家条件编译系统层
├── middleware/keyboard/combo/kb_sys_action.c  # 统一使用 IS_COUNTRY_* 宏
```

---

## Task 1: 创建国家代码定义头文件

**Files:**
- Create: `keyboards/country_codes.h`

### Step 1.1: 创建 country_codes.h 文件

在 `keyboards/` 目录下创建 `country_codes.h`：

```c
/**
 * @file country_codes.h
 * @brief HID 国家代码定义
 *
 * 基于 USB HID Usage Tables 1.12v2 - Table 6: Country Codes
 * 用于编译时配置键盘国家/地区版本
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* =====================================================
 * HID 国家代码枚举（USB HID Spec Table 6）
 * ===================================================== */
typedef enum {
    COUNTRY_NOT_SUPPORTED   = 0x00,  // 不支持特定国家
    COUNTRY_ARABIC          = 0x01,  // 阿拉伯
    COUNTRY_BELGIAN         = 0x03,  // 比利时
    COUNTRY_CANADIAN_BILINGUAL = 0x04,  // 加拿大双语
    COUNTRY_CANADIAN_FRENCH = 0x05,  // 加拿大法语
    COUNTRY_CZECH           = 0x06,  // 捷克
    COUNTRY_DANISH          = 0x07,  // 丹麦
    COUNTRY_FINNISH         = 0x08,  // 芬兰
    COUNTRY_FRENCH          = 0x09,  // 法国
    COUNTRY_GERMAN          = 0x0C,  // 德国
    COUNTRY_GREEK           = 0x0D,  // 希腊
    COUNTRY_HEBREW          = 0x0E,  // 希伯来
    COUNTRY_JAPANESE        = 0x0F,  // 日本
    COUNTRY_KOREAN          = 0x13,  // 韩国
    COUNTRY_NORWEGIAN       = 0x14,  // 挪威
    COUNTRY_POLISH          = 0x16,  // 波兰
    COUNTRY_PORTUGUESE      = 0x17,  // 葡萄牙
    COUNTRY_UK              = 0x18,  // 英国
    COUNTRY_SPANISH         = 0x1D,  // 西班牙
    COUNTRY_SWISS           = 0x1E,  // 瑞士
    COUNTRY_TAIWANESE       = 0x20,  // 台湾
    COUNTRY_TURKISH         = 0x21,  // 土耳其
    COUNTRY_US              = 0x21,  // 美国（注意：与土耳其相同值）
} hid_country_code_t;

/* =====================================================
 * HID 国家代码宏（根据 KEYBOARD_COUNTRY 自动设置）
 * ===================================================== */
#ifdef KEYBOARD_COUNTRY
    #define HID_COUNTRY_CODE KEYBOARD_COUNTRY
#else
    #define HID_COUNTRY_CODE COUNTRY_NOT_SUPPORTED
#endif

/* =====================================================
 * 国家标识宏（用于条件编译）
 * 这些宏返回 0 或 1，可用于 #if 条件编译
 * ===================================================== */
#if defined(KEYBOARD_COUNTRY) && (KEYBOARD_COUNTRY == COUNTRY_GERMAN)
    #define IS_COUNTRY_DE    1
    #define IS_COUNTRY_UK    0
    #define IS_COUNTRY_FR    0
    #define IS_COUNTRY_JP    0
    #define IS_COUNTRY_US    0
#elif defined(KEYBOARD_COUNTRY) && (KEYBOARD_COUNTRY == COUNTRY_UK)
    #define IS_COUNTRY_DE    0
    #define IS_COUNTRY_UK    1
    #define IS_COUNTRY_FR    0
    #define IS_COUNTRY_JP    0
    #define IS_COUNTRY_US    0
#elif defined(KEYBOARD_COUNTRY) && (KEYBOARD_COUNTRY == COUNTRY_FRENCH)
    #define IS_COUNTRY_DE    0
    #define IS_COUNTRY_UK    0
    #define IS_COUNTRY_FR    1
    #define IS_COUNTRY_JP    0
    #define IS_COUNTRY_US    0
#elif defined(KEYBOARD_COUNTRY) && (KEYBOARD_COUNTRY == COUNTRY_JAPANESE)
    #define IS_COUNTRY_DE    0
    #define IS_COUNTRY_UK    0
    #define IS_COUNTRY_FR    0
    #define IS_COUNTRY_JP    1
    #define IS_COUNTRY_US    0
#else
    // 默认美国布局
    #define IS_COUNTRY_DE    0
    #define IS_COUNTRY_UK    0
    #define IS_COUNTRY_FR    0
    #define IS_COUNTRY_JP    0
    #define IS_COUNTRY_US    1
#endif

/* =====================================================
 * 国家字符串标识（用于固件版本）
 * ===================================================== */
#if IS_COUNTRY_DE
    #define COUNTRY_CODE_STRING "DE"
#elif IS_COUNTRY_UK
    #define COUNTRY_CODE_STRING "UK"
#elif IS_COUNTRY_FR
    #define COUNTRY_CODE_STRING "FR"
#elif IS_COUNTRY_JP
    #define COUNTRY_CODE_STRING "JP"
#else
    #define COUNTRY_CODE_STRING "US"
#endif

#ifdef __cplusplus
}
#endif
```

- [ ] **Step 1.2: 验证头文件创建**

```bash
# 检查文件是否存在
ls -la keyboards/country_codes.h
```

Expected: 文件存在，大小约 2.5KB

- [ ] **Step 1.3: 建议提交**

```bash
git add keyboards/country_codes.h
git commit -m "feat(keyboard): add HID country code definitions"
```

---

## Task 2: 扩展产品配置

**Files:**
- Modify: `keyboards/kb904/config.h`

### Step 2.1: 在 config.h 添加国家配置区

在 `keyboards/kb904/config.h` 文件末尾、`#include "config_check.h"` 之前添加：

```c
/* ========== 国家/地区配置 ==========
 * 取消注释以下之一，仅启用一个国家配置
 * 配置值参考 country_codes.h 中的 hid_country_code_t 枚举
 * ===================================================== */
// #define KEYBOARD_COUNTRY  COUNTRY_US        // 美国（默认）
// #define KEYBOARD_COUNTRY  COUNTRY_GERMAN    // 德国
// #define KEYBOARD_COUNTRY  COUNTRY_UK        // 英国
// #define KEYBOARD_COUNTRY  COUNTRY_FRENCH    // 法国
// #define KEYBOARD_COUNTRY  COUNTRY_JAPANESE  // 日本

/* 国家配置默认值（如未定义则默认美国） */
#ifndef KEYBOARD_COUNTRY
// #define KEYBOARD_COUNTRY  COUNTRY_NOT_SUPPORTED  // 或显式定义为不支持
#endif
```

**注意**：需要在 `#include "defaults/*.h"` 之后添加，因为这些默认头文件可能需要引用 `KEYBOARD_COUNTRY`。

- [ ] **Step 2.2: 验证修改**

```bash
# 检查 config.h 的修改
git diff keyboards/kb904/config.h
```

Expected: 显示新增的国家配置区，**不包含** `#include "country_codes.h"`（避免循环依赖）

> **注意**：country_codes.h 应该由使用它的文件（keymaps/default.c、hidkbdservice.c 等）自行包含，而非在 config.h 中包含。

- [ ] **Step 2.3: 建议提交**

```bash
git add keyboards/kb904/config.h
git commit -m "feat(kb904): add country configuration section"
```

---

## Task 3: 修改 HID Profile

**Files:**
- Modify: `project/ch584m/Profile/hidkbdservice.c:81-85`

### Step 3.1: 在 hidkbdservice.c 包含 country_codes.h

在文件头部的 `#include` 区域添加：

```c
// 在 #include "battservice.h" 之后添加
#include "country_codes.h"
```

- [ ] **Step 3.2: 替换硬编码的 bCountryCode**

将第 81-85 行的 `hidInfo` 数组：

```c
// 原代码
static const uint8_t hidInfo[HID_INFORMATION_LEN] = {
    LO_UINT16(0x0111), HI_UINT16(0x0111), // bcdHID (USB HID version)
    0x00,                                 // bCountryCode
    HID_FEATURE_FLAGS                     // Flags
};
```

修改为：

```c
// 修改后
static const uint8_t hidInfo[HID_INFORMATION_LEN] = {
    LO_UINT16(0x0111), HI_UINT16(0x0111), // bcdHID (USB HID version)
    HID_COUNTRY_CODE,                     // bCountryCode（编译时配置）
    HID_FEATURE_FLAGS                     // Flags
};
```

- [ ] **Step 3.3: 验证修改**

```bash
# 检查修改
git diff project/ch584m/Profile/hidkbdservice.c
```

Expected: 显示 hidInfo 数组中 bCountryCode 从 `0x00` 改为 `HID_COUNTRY_CODE`

- [ ] **Step 3.4: 建议提交**

```bash
git add project/ch584m/Profile/hidkbdservice.c
git commit -m "feat(ble): use configurable HID country code"
```

---

## Task 4: 重构键位表条件编译

**Files:**
- Modify: `keyboards/kb904/keymaps/default.c`

### Step 4.1: 包含 country_codes.h

在文件头部的 `#include` 区域添加：

```c
#include "country_codes.h"
```

- [ ] **Step 4.2: 添加国家差异的系统层模板**

在 `keymap_layers` 数组中，将 `[LAYER_WINDOWS]` 部分替换为条件编译结构：

```c
    // =====================================================
    // LAYER_WINDOWS - Windows/Android 层
    // 按国家差异编译
    // =====================================================
#if IS_COUNTRY_DE
    [LAYER_WINDOWS] = {
        // 德国布局差异示例：Z/Y 互换（QWERTZ 布局）
        // TODO: 根据实际产品需求填充完整的德国布局键位
        //          0        1        2        3        4        5        6        7        8        9       10       11       12       13       14
        /* Row 0 */  KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        /* Row 1 */  KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_Z,    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        /* Row 2 */  KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        /* Row 3 */  KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        /* Row 4 */  KC_Y,     KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        /* Row 5 */  KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        /* Row 6 */  KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        /* Row 7 */  KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
    },
#elif IS_COUNTRY_UK
    [LAYER_WINDOWS] = {
        // 英国布局差异（待填充具体键值）
        //          0        1        2        3        4        5        6        7        8        9       10       11       12       13       14
        /* Row 0 */  KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        /* Row 1 */  KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        /* Row 2 */  KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        /* Row 3 */  KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        /* Row 4 */  KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        /* Row 5 */  KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        /* Row 6 */  KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        /* Row 7 */  KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
    },
#else
    // 默认美国布局（保持原有 LAYER_WINDOWS 内容）
    [LAYER_WINDOWS] = {
        //          0        1        2        3        4        5        6        7        8        9       10       11       12       13       14
        /* Row 0 */  KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        /* Row 1 */  KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        /* Row 2 */  KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        /* Row 3 */  KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        /* Row 4 */  KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        /* Row 5 */  KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        /* Row 6 */  KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        /* Row 7 */  KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
    },
#endif
```

**注意**：德国布局的 Z/Y 互换已在上述示例中体现（Row 1 的 Y 位置放 Z，Row 4 的 Z 位置放 Y）。

- [ ] **Step 4.3: 验证修改**

```bash
# 检查修改
git diff keyboards/kb904/keymaps/default.c
```

Expected: 显示新增的条件编译结构

- [ ] **Step 4.4: 建议提交**

```bash
git add keyboards/kb904/keymaps/default.c
git commit -m "feat(keymap): add country-specific layer configuration"
```

---

## Task 5: 重构组合键国家差异

**Files:**
- Modify: `middleware/keyboard/combo/kb_sys_action.c`

### Step 5.1: 包含 country_codes.h

在文件头部添加：

```c
#include "country_codes.h"
```

- [ ] **Step 5.2: 替换现有的 #ifdef DE / #ifdef UK**

将第 84-145 行的 `#ifdef DE` 和 `#ifdef UK` 块替换为统一的条件编译：

```c
/* =====================================================
 * 特殊组合键（按国家差异）
 * X 键组合：输出特殊字符
 * ===================================================== */
#if IS_COUNTRY_DE
// 德国布局：Shift + ` (grave) 或 Shift + < (Europe 2)
uint8_t X_Special_Combo(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (keycode_type == IOS || keycode_type == MAC) {
        add_keys[idx++] = KB_L_SHIFT;
        add_keys[idx++] = KB_GRAVE_ACCENT_N_TILDE;
    } else {
        add_keys[idx++] = KB_L_SHIFT;
        add_keys[idx++] = KB_EUROPE_2;
    }
    return idx;
}

// Z 键组合：Y/Z 互换布局的特殊处理
uint8_t Z_Special_Combo(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (keycode_type == IOS || keycode_type == MAC) {
        add_keys[idx++] = KB_L_SHIFT;
        add_keys[idx++] = KB_GRAVE_ACCENT_N_TILDE;
    } else {
        add_keys[idx++] = KB_L_SHIFT;
        add_keys[idx++] = KB_EUROPE_2;
    }
    return idx;
}

#elif IS_COUNTRY_UK
// 英国布局：` 和 # 位置差异
uint8_t X_Special_Combo(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (keycode_type == IOS) {
        add_keys[idx++] = KB_L_SHIFT;
        add_keys[idx++] = KB_BACK_SPLASH_N_VERTICAL_BAR;
    } else if (keycode_type == WINDOWS) {
        add_keys[idx++] = KB_EUROPE_2;
    }
    return idx;
}

uint8_t Z_Special_Combo(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (keycode_type == IOS) {
        add_keys[idx++] = KB_BACK_SPLASH_N_VERTICAL_BAR;
    } else if (keycode_type == WINDOWS) {
        add_keys[idx++] = KB_L_SHIFT;
        add_keys[idx++] = KB_EUROPE_2;
    }
    return idx;
}

#else
// 默认美国布局（无特殊组合键差异）
// 如需添加美国特定的组合键处理，在此处实现
#endif
```

- [ ] **Step 5.3: 验证修改**

```bash
# 检查修改
git diff middleware/keyboard/combo/kb_sys_action.c
```

Expected: 显示从 `#ifdef DE/#ifdef UK` 改为 `#if IS_COUNTRY_DE/#elif IS_COUNTRY_UK`

- [ ] **Step 5.4: 建议提交**

```bash
git add middleware/keyboard/combo/kb_sys_action.c
git commit -m "refactor(combo): unify country-specific combo using IS_COUNTRY_* macros"
```

---

## Task 6: 编译验证

**Files:**
- 无新文件（仅修改 config.h 进行测试）

> **注意**：本任务需要多次修改 `keyboards/kb904/config.h` 并重新编译。每次修改后请确保编译成功。

### Step 6.1: 验证默认配置（美国）编译

确保 `keyboards/kb904/config.h` 中 `KEYBOARD_COUNTRY` 处于注释状态：

```c
// 检查：以下行应为注释状态
// #define KEYBOARD_COUNTRY  COUNTRY_GERMAN
// #define KEYBOARD_COUNTRY  COUNTRY_UK
```

运行编译：
```bash
/wch-riscv-build
```

Expected: 编译成功，无错误和警告

- [ ] **Step 6.2: 验证德国配置编译**

**操作**：编辑 `keyboards/kb904/config.h`，取消德国配置的注释：

```c
// 修改前：
// #define KEYBOARD_COUNTRY  COUNTRY_GERMAN

// 修改后：
#define KEYBOARD_COUNTRY  COUNTRY_GERMAN
```

保存后运行编译：
```bash
/wch-riscv-build
```

Expected: 编译成功

- [ ] **Step 6.3: 验证英国配置编译**

**操作**：编辑 `keyboards/kb904/config.h`，将德国配置注释，英国配置取消注释：

```c
// 修改后：
// #define KEYBOARD_COUNTRY  COUNTRY_GERMAN
#define KEYBOARD_COUNTRY  COUNTRY_UK
```

保存后运行编译：
```bash
/wch-riscv-build
```

Expected: 编译成功

- [ ] **Step 6.4: 恢复默认配置**

**操作**：将所有国家配置恢复为注释状态：

```c
// 恢复后：
// #define KEYBOARD_COUNTRY  COUNTRY_GERMAN
// #define KEYBOARD_COUNTRY  COUNTRY_UK
```

保存文件。此时配置恢复为默认美国布局。

> **提示**：所有配置验证完成后，可以提交 Task 1-5 的修改（如未提交）。

---

## Task 7: 更新文档

**Files:**
- Modify: `CLAUDE.md`

### Step 7.1: 在 CLAUDE.md 添加国家配置说明

在 `## Configuration` 部分添加：

```markdown
### 国家/地区配置 (keyboards/kb904/config.h)

```c
// 取消注释以下之一，仅启用一个国家配置
// #define KEYBOARD_COUNTRY  COUNTRY_US        // 美国（默认）
// #define KEYBOARD_COUNTRY  COUNTRY_GERMAN    // 德国
// #define KEYBOARD_COUNTRY  COUNTRY_UK        // 英国
// #define KEYBOARD_COUNTRY  COUNTRY_FRENCH    // 法国
// #define KEYBOARD_COUNTRY  COUNTRY_JAPANESE  // 日本
```

**国家代码映射**：
- `COUNTRY_NOT_SUPPORTED` (0x00) - 不支持特定国家
- `COUNTRY_GERMAN` (0x0C) - 德国（Z/Y 互换）
- `COUNTRY_UK` (0x18) - 英国
- `COUNTRY_FRENCH` (0x09) - 法国
- `COUNTRY_JAPANESE` (0x0F) - 日本
- `COUNTRY_US` (0x21) - 美国（默认）
```

- [ ] **Step 7.2: 建议提交**

```bash
git add CLAUDE.md
git commit -m "docs: add country configuration documentation"
```

---

## 完成检查

- [ ] 所有文件修改完成
- [ ] 至少 3 个国家配置编译通过（US/DE/UK）
- [ ] 文档更新完成
- [ ] 所有 commit 已创建

---

## 参考文档

- 设计文档: `docs/plans/2026-03-21-country-code-design.md`
- USB HID Usage Tables 1.12v2 - Table 6: Country Codes
