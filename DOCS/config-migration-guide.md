# 配置系统迁移指南

> 版本: V1.0
> 日期: 2026-03-18
> 适用于: keyboard-framework

---

## 概述

本指南描述如何将产品从旧的 `product_config.h` 单文件配置迁移到新的分层配置系统。

---

## 新旧对比

### 旧系统

```
keyboards/
└── product_config.h      # 所有配置混合在一个文件
```

### 新系统

```
keyboards/
├── defaults/             # 通用默认值
│   ├── backlight_defaults.h
│   ├── ble_defaults.h
│   ├── indicator_defaults.h
│   ├── keyboard_defaults.h
│   ├── power_defaults.h
│   └── matrix_defaults.h
│
├── kb904/                # 产品配置目录
│   ├── config.h          # 产品主配置
│   ├── config_hw.h       # GPIO 硬件映射
│   └── README.md         # 产品说明
│
├── config_check.h        # 编译时验证
├── indicator_config.h    # 指示灯配置（引用 config_hw.h）
└── product_config.h      # 过渡期兼容
```

---

## 迁移步骤

### 步骤一：创建产品目录

```bash
mkdir keyboards/my_product
```

### 步骤二：创建 config.h

```c
// keyboards/my_product/config.h
#pragma once

/* ========== 产品标识 ========== */
#define PRODUCT_ID_VALUE     0x0905
#define PRODUCT_NAME         "My Keyboard"
#define MANUFACTURER         "My Company"
#define DEVICE_VERSION       0x0100

/* ========== 功能开关 ========== */
#define BLE_ENABLE_FLAG      TRUE
#define USB_ENABLE_FLAG      TRUE
#define TOUCHPAD_ENABLE      FALSE
#define COMBO_ENABLE         TRUE

/* ========== 覆盖默认值（可选） ========== */
#define BACKLIGHT_COLOR_COUNT 7
#define BLE_HOST_COUNT       3

/* ========== 包含默认配置 ========== */
#include "defaults/backlight_defaults.h"
#include "defaults/ble_defaults.h"
#include "defaults/indicator_defaults.h"
#include "defaults/keyboard_defaults.h"
#include "defaults/power_defaults.h"
#include "defaults/matrix_defaults.h"

/* ========== 包含硬件配置 ========== */
#include "config_hw.h"

/* ========== 配置验证 ========== */
#include "config_check.h"
```

### 步骤三：创建 config_hw.h

```c
// keyboards/my_product/config_hw.h
#pragma once

/* 清除旧定义（如果从 product_config.h 继承） */
#ifdef MATRIX_ROWS
#undef MATRIX_ROWS
#endif
#ifdef MATRIX_COLS
#undef MATRIX_COLS
#endif
#ifdef MATRIX_ROW_PINS
#undef MATRIX_ROW_PINS
#endif
#ifdef MATRIX_COL_PINS
#undef MATRIX_COL_PINS
#endif

/* ========== 矩阵配置 ========== */
#define MATRIX_ROWS           6
#define MATRIX_COLS           14
#define DIODE_DIRECTION       COL2ROW

#define MATRIX_ROW_PINS { A0, A1, A2, A3, A4, A5 }
#define MATRIX_COL_PINS { B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10, B11, B12, B13 }

/* ========== 指示灯 ========== */
#define LED_CAPS_PIN          A15
#define LED_BT_PIN            B22
#define LED_POWER_PIN         B23

/* ========== 背光 ========== */
#define BACKLIGHT_PIN_R       B0
#define BACKLIGHT_PIN_G       B1
#define BACKLIGHT_PIN_B       B2
```

### 步骤四：更新 indicator_config.h

在 `keyboards/indicator_config.h` 中添加新产品分支：

```c
/* 根据产品 ID 加载对应的硬件配置 */
#if PRODUCT_ID == 0x0904
#include "kb904/config_hw.h"
#define IND_LED_COUNT   4
#elif PRODUCT_ID == 0x0905
#include "my_product/config_hw.h"
#define IND_LED_COUNT   3
#endif
```

### 步骤五：编译验证

```bash
# 使用 wch-riscv-build skill 编译
/wch-riscv-build
```

---

## 配置命名规范

### 模块前缀

| 模块 | 前缀 | 示例 |
|------|------|------|
| 背光 | `BACKLIGHT_` | `BACKLIGHT_COLOR_COUNT` |
| 蓝牙 | `BLE_` | `BLE_HOST_COUNT` |
| 指示灯 | `IND_` | `IND_LED_COUNT` |
| 键盘 | - | `TAPPING_TERM`, `COMBO_TERM` |
| 电源 | `POWER_` / `LPM_` | `POWER_DEEP_SLEEP_MS` |
| 矩阵 | `MATRIX_` | `MATRIX_ROWS`, `MATRIX_COLS` |

### 引脚定义

在 `config_hw.h` 中定义，使用语义化命名：

```c
// 好的命名
#define LED_CAPS_PIN     A15
#define BACKLIGHT_PIN_R  B0

// 避免的命名
#define LED1_PIN         A15  // 语义不清
#define PWM_R_PIN        B0   // 功能而非语义
```

---

## 常见问题

### Q: 为什么使用 `#undef` 清除旧定义？

A: 在迁移过渡期，`product_config.h` 和 `config_hw.h` 可能都定义了相同配置项。使用 `#undef` 可以避免编译警告。

### Q: defaults 中的 `#ifndef` 如何与产品覆盖配合？

A: 产品配置先定义覆盖值，然后包含 defaults。defaults 使用 `#ifndef` 保护，已定义的值不会被覆盖。

```c
// 产品 config.h
#define BACKLIGHT_COLOR_COUNT 13  // 先定义
#include "defaults/backlight_defaults.h"  // defaults 使用 #ifndef，不会覆盖
```

### Q: 如何添加新的配置项？

1. 在对应的 `defaults/*.h` 中添加默认值（使用 `#ifndef` 保护）
2. 在产品 `config.h` 中覆盖（如果需要）
3. 在 `config_check.h` 中添加验证（如果需要）

---

## 验收清单

- [ ] 创建了产品目录 `keyboards/<product>/`
- [ ] 创建了 `config.h` 包含产品标识和功能开关
- [ ] 创建了 `config_hw.h` 包含 GPIO 映射
- [ ] 在 `indicator_config.h` 中添加了产品分支
- [ ] 编译通过，无配置相关警告
- [ ] 创建了 `README.md` 产品说明

---

*文档结束*
