# CountryCode 实现方案设计

> 文档类型：design
> 创建日期：2026-03-21
> 状态：待实施

---

## 1. 需求摘要

### 1.1 背景

基于 2860sdk.xml 中 CountryCode 的实现逻辑，在当前键盘框架项目中实现类似的国家/地区支持机制。

### 1.2 核心需求

| ID | 需求项 | 说明 |
|----|--------|------|
| R1 | 多国支持 | 支持 US/DE/UK/FR/JP 等国家版本，预留扩展机制 |
| R2 | 编译时配置 | 国家语言编译时确定，通过宏定义区分 |
| R3 | 键值映射 | LAYER_BASE 保持不变，LAYER_MACOS/WINDOWS 按国家差异编译 |
| R4 | 组合键差异 | 国家+系统双重差异，使用条件编译实现 |
| R5 | HID Profile | 仅修改 HID Information 的 bCountryCode 字段 |

### 1.3 范围边界

| 包含 | 排除 |
|------|------|
| ✅ HID bCountryCode 字段配置 | ❌ 运行时动态切换国家 |
| ✅ 编译时国家宏定义 | ❌ 用户配置接口（RAW HID/蓝牙写入） |
| ✅ 国家-系统层级映射表 | ❌ Flash 热切换机制 |
| ✅ 组合键国家差异 | ❌ 全量 33 国 HID 代码 |

---

## 2. 现状分析

### 2.1 当前代码差距

| ID | 差距项 | 当前实现 | 需求要求 | 优先级 |
|----|--------|---------|---------|--------|
| G1 | HID bCountryCode | 硬编码 `0x00` | 宏配置 `HID_COUNTRY_CODE` | P0 |
| G2 | 层级国家维度 | 无 | 编译时国家条件编译 | P0 |
| G3 | 国家配置集中化 | 分散于各 `#ifdef` | 统一配置头文件 | P1 |
| G4 | 组合键国家差异 | 原型代码（#ifdef DE/UK） | 统一国家宏 | P1 |

### 2.2 关键文件现状

| 文件 | 当前状态 | 问题 |
|------|---------|------|
| `project/ch584m/Profile/hidkbdservice.c` | bCountryCode 硬编码为 `0x00` | 无宏配置机制 |
| `keyboards/kb904/keymaps/default.c` | LAYER_MACOS/WINDOWS 为 KC_TRNS | 无国家变体 |
| `middleware/keyboard/combo/kb_sys_action.c` | 使用 `#ifdef DE` 原型代码 | 宏定义分散 |
| `keyboards/kb904/config.h` | 无国家配置 | 缺少 KEYBOARD_COUNTRY 宏 |

---

## 3. 方案设计

### 3.1 方案选型

**选定方案**：编译时隔离（方案 A）

**核心思路**：每个国家版本独立编译，共享 LAYER_BASE，仅替换系统层。

**选择理由**：
- 改动最小、Flash 占用低、无运行时开销
- 符合嵌入式项目资源受限的特点
- 与现有 `#ifdef` 模式一致

### 3.2 架构设计

```
配置入口
└── keyboards/kb904/config.h
    └── #define KEYBOARD_COUNTRY  COUNTRY_DE  // 或 COUNTRY_US/COUNTRY_UK

国家代码定义
└── keyboards/country_codes.h
    └── HID 国家代码枚举 + HID_COUNTRY_CODE 宏

键位层（原地修改）
└── keyboards/kb904/keymaps/default.c
    └── keymap_layers[] 用 #ifdef KEYBOARD_COUNTRY_* 条件编译

HID Profile
└── project/ch584m/Profile/hidkbdservice.c
    └── hidInfo[] 使用 HID_COUNTRY_CODE 宏
```

### 3.3 组件设计

#### 3.3.1 country_codes.h（新增）

```c
#pragma once

// HID 国家代码（USB HID Spec Table 6）
typedef enum {
    COUNTRY_UNSPECIFIED = 0x00,
    COUNTRY_ARABIC      = 0x01,
    COUNTRY_BELGIAN     = 0x03,
    COUNTRY_FRENCH      = 0x09,
    COUNTRY_GERMAN      = 0x0C,
    COUNTRY_GREEK       = 0x0D,
    COUNTRY_JAPANESE    = 0x0F,
    COUNTRY_KOREAN      = 0x13,
    COUNTRY_UK          = 0x18,
    COUNTRY_US          = 0x21,
} hid_country_code_t;

// 便捷宏：根据 KEYBOARD_COUNTRY 自动设置 HID 国家代码
#ifdef KEYBOARD_COUNTRY
    #define HID_COUNTRY_CODE KEYBOARD_COUNTRY
#else
    #define HID_COUNTRY_CODE COUNTRY_UNSPECIFIED
#endif

// 国家标识宏（用于条件编译，必须在定义 KEYBOARD_COUNTRY 后使用）
// 注意：KEYBOARD_COUNTRY 必须在 config.h 中定义为整型常量
#if defined(KEYBOARD_COUNTRY) && (KEYBOARD_COUNTRY == COUNTRY_US)
    #define IS_COUNTRY_US    1
    #define IS_COUNTRY_DE    0
    #define IS_COUNTRY_UK    0
#elif defined(KEYBOARD_COUNTRY) && (KEYBOARD_COUNTRY == COUNTRY_GERMAN)
    #define IS_COUNTRY_US    0
    #define IS_COUNTRY_DE    1
    #define IS_COUNTRY_UK    0
#elif defined(KEYBOARD_COUNTRY) && (KEYBOARD_COUNTRY == COUNTRY_UK)
    #define IS_COUNTRY_US    0
    #define IS_COUNTRY_DE    0
    #define IS_COUNTRY_UK    1
#else
    #define IS_COUNTRY_US    1
    #define IS_COUNTRY_DE    0
    #define IS_COUNTRY_UK    0
#endif
```

#### 3.3.2 kb904/config.h（修改）

```c
// ========== 国家/地区配置 ==========
// 取消注释以下之一，仅启用一个国家配置
// #define KEYBOARD_COUNTRY  COUNTRY_US    // 美国
// #define KEYBOARD_COUNTRY  COUNTRY_GERMAN // 德国
// #define KEYBOARD_COUNTRY  COUNTRY_UK    // 英国
// #define KEYBOARD_COUNTRY  COUNTRY_FRENCH // 法国
// #define KEYBOARD_COUNTRY  COUNTRY_JAPANESE // 日本

// 默认值保护
#ifndef KEYBOARD_COUNTRY
#define KEYBOARD_COUNTRY COUNTRY_UNSPECIFIED
#endif
```

#### 3.3.3 keymaps/default.c（修改）

```c
#include "country_codes.h"

const uint16_t keymap_layers[][MATRIX_ROWS][MATRIX_COLS] = {
    // LAYER_BASE 所有国家共用
    [LAYER_BASE] = LAYOUT(
        KC_ESC, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINUS, KC_EQUAL, KC_BSPC,
        KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRACKET, KC_RBRACKET, KC_BSLS,
        ...
    ),

    // 系统层按国家差异编译
    #if IS_COUNTRY_DE
    [LAYER_MACOS]   = LAYOUT(/* DE + macOS 映射 */),
    [LAYER_WINDOWS] = LAYOUT(/* DE + Windows 映射 */),
    #elif IS_COUNTRY_UK
    [LAYER_MACOS]   = LAYOUT(/* UK + macOS 映射 */),
    [LAYER_WINDOWS] = LAYOUT(/* UK + Windows 映射 */),
    #elif IS_COUNTRY_JP
    [LAYER_MACOS]   = LAYOUT(/* JP + macOS 映射 */),
    [LAYER_WINDOWS] = LAYOUT(/* JP + Windows 映射 */),
    #else
    // 默认美国布局
    [LAYER_MACOS]   = LAYOUT(/* US + macOS 映射 */),
    [LAYER_WINDOWS] = LAYOUT(/* US + Windows 映射 */),
    #endif
};
```

#### 3.3.4 hidkbdservice.c（修改）

```c
// 第 81-85 行，替换硬编码值
static const uint8_t hidInfo[HID_INFORMATION_LEN] = {
    LO_UINT16(0x0111), HI_UINT16(0x0111), // bcdHID
    HID_COUNTRY_CODE,                      // bCountryCode (原硬编码 0x00)
    HID_FEATURE_FLAGS                      // Flags
};
```

### 3.4 数据流

```
编译时配置流程：
┌─────────────────────┐
│ kb904/config.h      │
│ KEYBOARD_COUNTRY=X  │
└─────────┬───────────┘
          │
          ▼
┌─────────────────────┐
│ country_codes.h     │
│ HID_COUNTRY_CODE=X  │
└─────────┬───────────┘
          │
    ┌─────┴─────┐
    ▼           ▼
┌───────┐   ┌───────────┐
│keymaps│   │hidkbdsvc  │
│条件编译│   │bCountryCode│
└───────┘   └───────────┘
```

### 3.5 头文件依赖

```
包含顺序：
config.h (定义 KEYBOARD_COUNTRY)
    │
    └─► country_codes.h (定义枚举和辅助宏)
            │
            ├─► keycodes.h (键码定义)
            │
            └─► keymaps/default.c (键位表)

注意：country_codes.h 必须在 keycodes.h 之后、keymaps 之前包含
```

### 3.6 错误处理

```c
// 编译时验证
static_assert(HID_COUNTRY_CODE <= 0x21, "Invalid HID country code");
```

---

## 4. 实施计划

### 4.1 阶段一：基础设施搭建（P0）

| 步骤 | 任务 | 具体操作 | 验证标准 | 依赖 |
|------|------|---------|---------|------|
| **1.1** | 创建国家代码定义 | 新建 `keyboards/country_codes.h` | 编译通过 | 无 |
| **1.2** | 扩展产品配置 | 修改 `keyboards/kb904/config.h` | 宏可被引用 | 1.1 |
| **1.3** | 修改 HID Profile | 修改 `hidkbdservice.c` 第 83 行 | hidInfo[2] 值正确 | 1.1, 1.2 |

### 4.2 阶段二：键位表改造（P0）

| 步骤 | 任务 | 具体操作 | 验证标准 | 依赖 |
|------|------|---------|---------|------|
| **2.1** | 重构键位表结构 | 修改 `keymaps/default.c`，添加条件编译框架 | 编译通过 | 1.2 |
| **2.2** | 实现德国变体 | 添加 `#if KEYBOARD_COUNTRY_DE` 块 | DE 键位正确 | 2.1 |
| **2.3** | 实现英国变体 | 添加 `#elif KEYBOARD_COUNTRY_UK` 块 | UK 键位正确 | 2.1 |

### 4.3 阶段三：组合键适配（P1）

| 步骤 | 任务 | 具体操作 | 验证标准 | 依赖 |
|------|------|---------|---------|------|
| **3.1** | 统一组合键宏 | 在 `country_codes.h` 添加辅助宏 | 宏可被 combo 使用 | 1.1 |
| **3.2** | 重构组合键 | 修改 `kb_sys_action.c` | 原有功能正常 | 3.1 |

### 4.4 阶段四：验证与文档（P2）

| 步骤 | 任务 | 具体操作 | 验证标准 | 依赖 |
|------|------|---------|---------|------|
| **4.1** | 编译验证 | 遍历各国配置编译 | 无警告 | 3.2 |
| **4.2** | 更新文档 | 更新 CLAUDE.md | 文档一致 | 4.1 |

### 4.5 文件变更清单

| 文件路径 | 操作 | 变更范围 |
|---------|------|---------|
| `keyboards/country_codes.h` | 新增 | ~50 行 |
| `keyboards/kb904/config.h` | 修改 | +10 行 |
| `project/ch584m/Profile/hidkbdservice.c` | 修改 | 1 行 |
| `keyboards/kb904/keymaps/default.c` | 修改 | +100 行 |
| `middleware/keyboard/combo/kb_sys_action.c` | 修改 | ~20 行 |

---

## 5. 测试策略

### 5.1 编译验证

修改 `keyboards/kb904/config.h` 中的 `KEYBOARD_COUNTRY` 宏定义后编译：

```bash
# 修改 config.h: #define KEYBOARD_COUNTRY  COUNTRY_GERMAN
# 然后使用 WCH RISC-V 编译命令
cmd.exe /c "cd /d D:\\Code\\C_Project\\keyboard-framework\\project\\ch584m\\obj && \
  set PATH=D:\\Software\\Work\\MounRiver_Studio2\\...\\bin;%PATH% && \
  make.exe clean && make.exe main-build"

# 或使用 /wch-riscv-build skill 自动执行
```

### 5.2 功能验证

| 测试项 | 验证方法 |
|--------|---------|
| HID 国家代码正确性 | 编译后检查 `hidInfo[2]` 值 |
| 键位表切换正确 | 运行时验证 LAYER_MACOS/WINDOWS 键值 |
| 组合键国家差异 | 验证 DE/UK 的特殊组合键输出 |
| 回归测试 | 未定义国家宏时行为不变 |

---

## 6. 风险与缓解

| 风险 | 等级 | 缓解措施 |
|------|------|---------|
| 配置冲突 | 中 | 添加 `#ifndef` 保护，默认值 |
| 编译配置遗漏 | 中 | static_assert 验证 |
| 扩展成本高 | 低 | 长期考虑数据驱动重构 |

---

## 7. 参考资料

- USB HID Usage Tables 1.12v2 - Table 6: Country Codes
- 2860sdk.xml - CountryCode 实现参考
- QMK Firmware - keymap.c 多国布局示例
