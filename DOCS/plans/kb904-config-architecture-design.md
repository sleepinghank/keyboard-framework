# KB904 配置项管理架构设计文档

> 文档版本: V1.0
> 创建日期: 2026-03-18
> 完成日期: 2026-03-18
> 状态: ✅ 已实施

---

## 1. 设计目标

| 目标 | 描述 |
|------|------|
| **多产品复用** | 支持多个产品型号使用同一代码库 |
| **参数可调** | 单产品通过配置实现功能参数可调 |
| **编译时确定** | 所有配置在编译时确定，无运行时开销 |
| **按产品集中** | 每个产品的配置集中在独立目录 |
| **低耦合高内聚** | 模块间配置独立，模块内配置聚合 |

---

## 2. 架构设计

### 2.1 目录结构

```
keyboards/
├── defaults/                    # 通用默认配置层
│   ├── backlight_defaults.h     # 背光模块默认值
│   ├── ble_defaults.h           # 蓝牙模块默认值
│   ├── indicator_defaults.h     # 指示灯模块默认值
│   ├── keyboard_defaults.h      # 键盘行为默认值
│   ├── power_defaults.h         # 电源管理默认值
│   └── matrix_defaults.h        # 矩阵扫描默认值
│
├── kb904/                       # KB904 产品配置
│   ├── config.h                 # 产品级配置（覆盖默认值）
│   ├── config_hw.h              # GPIO 硬件映射（对应原理图）
│   ├── keymaps/
│   │   └── layout.h             # 键位表
│   └── README.md                # 产品说明
│
└── n0046/                       # N0046 产品配置（示例）
    └── config.h                 # 仅定义与 KB904 不同的配置
```

### 2.2 配置分层逻辑

```
┌──────────────────────────────────────────────────────────────┐
│                    产品配置 (kb904/config.h)                  │
│         只定义与默认值不同的配置项，使用 #define 覆盖         │
├──────────────────────────────────────────────────────────────┤
│                   默认配置层 (defaults/)                      │
│  使用 #ifndef 保护，允许产品配置覆盖                          │
├──────────────────────────────────────────────────────────────┤
│                   硬件配置层 (kb904/config_hw.h)             │
│              GPIO 引脚映射，直接对应原理图                    │
└──────────────────────────────────────────────────────────────┘
```

### 2.3 配置加载顺序

```c
// 模块驱动文件 (例如 backlight.c)
#include "backlight_config.h"

// backlight_config.h 内容
#pragma once
#include "defaults/backlight_defaults.h"  // 先加载默认值
// 产品配置通过 -D 或包含产品 config.h 在此之前完成覆盖
```

---

## 3. 详细设计

### 3.1 defaults/backlight_defaults.h

```c
#pragma once

/* ========== 背光类型 ========== */
#ifndef BACKLIGHT_TYPE
#define BACKLIGHT_TYPE_SINGLE
#endif

/* ========== 背光颜色 ========== */
#ifndef BACKLIGHT_COLOR_COUNT
#define BACKLIGHT_COLOR_COUNT    7
#endif

/* ========== 亮度档位 ========== */
#ifndef BACKLIGHT_LEVEL_COUNT
#define BACKLIGHT_LEVEL_COUNT    4
#endif

/* ========== 默认值 ========== */
#ifndef BACKLIGHT_DEFAULT_ON
#define BACKLIGHT_DEFAULT_ON     true
#endif

#ifndef BACKLIGHT_DEFAULT_COLOR
#define BACKLIGHT_DEFAULT_COLOR  BL_COLOR_WHITE
#endif

#ifndef BACKLIGHT_DEFAULT_LEVEL
#define BACKLIGHT_DEFAULT_LEVEL  2
#endif

/* ========== 休眠配置 ========== */
#ifndef BACKLIGHT_IDLE_TIMEOUT_MS
#define BACKLIGHT_IDLE_TIMEOUT_MS   5000
#endif

/* ========== 低电量 ========== */
#ifndef BACKLIGHT_LOW_BATTERY_THRESHOLD
#define BACKLIGHT_LOW_BATTERY_THRESHOLD  20
#endif
```

### 3.2 defaults/ble_defaults.h

```c
#pragma once

/* ========== 蓝牙基础参数 ========== */
#ifndef BLE_PAIRING_NAME
#define BLE_PAIRING_NAME     "Keyboard"
#endif

#ifndef BLE_PAIRING_TIMEOUT_SEC
#define BLE_PAIRING_TIMEOUT_SEC  60
#endif

#ifndef BLE_RECONNECT_TIMEOUT_SEC
#define BLE_RECONNECT_TIMEOUT_SEC 60
#endif

#ifndef BLE_ADVERTISE_INTERVAL_MS
#define BLE_ADVERTISE_INTERVAL_MS 1000
#endif

/* ========== 通道配置 ========== */
#ifndef BLE_HOST_COUNT
#define BLE_HOST_COUNT       3
#endif
```

### 3.3 defaults/power_defaults.h

```c
#pragma once

/* ========== 休眠时间 ========== */
#ifndef POWER_DEEP_SLEEP_MS
#define POWER_DEEP_SLEEP_MS  (10 * 60 * 1000)  // 10分钟
#endif

#ifndef POWER_IDLE_SLEEP_MS
#define POWER_IDLE_SLEEP_MS (5 * 60 * 1000)   // 5分钟
#endif

/* ========== 低电量阈值 ========== */
#ifndef POWER_LOW_BATTERY_PERCENT
#define POWER_LOW_BATTERY_PERCENT  20
#endif

#ifndef POWER_CRITICAL_BATTERY_PERCENT
#define POWER_CRITICAL_BATTERY_PERCENT  5
#endif

/* ========== 电池参数 ========== */
#ifndef BATTERY_CAPACITY_MAH
#define BATTERY_CAPACITY_MAH  500
#endif
```

### 3.4 kb904/config.h

```c
#pragma once

/* ========================================
 * KB904 产品配置
 * 仅定义与默认值不同的配置项
 * ======================================== */

/* ========== 产品标识 ========== */
#define PRODUCT_ID       0x0904
#define PRODUCT_NAME     "inateck NeoMagic Pro"
#define VENDOR_ID        0x0002
#define MANUFACTURER     "inateck"
#define DEVICE_VERSION   0x0100

/* ========== 功能开关 ========== */
#define BLE_ENABLE_FLAG      TRUE
#define USB_ENABLE_FLAG      FALSE
#define TOUCHPAD_ENABLE      TRUE
#define COMBO_ENABLE         TRUE

/* ========== 覆盖背光默认值 ========== */
#define BACKLIGHT_TYPE       BACKLIGHT_TYPE_RGB
#define BACKLIGHT_COLOR_COUNT 13
#define BACKLIGHT_DEFAULT_COLOR  BL_COLOR_WHITE
#define BACKLIGHT_DEFAULT_LEVEL  2

/* ========== 覆盖蓝牙默认值 ========== */
#define BLE_PAIRING_NAME     "inateck NeoMagic Pro"
#define BLE_HOST_COUNT       1  // 单通道

/* ========== 覆盖电源默认值 ========== */
#define POWER_DEEP_SLEEP_MS  (10 * 60 * 1000)  // 10分钟

/* ========== 包含硬件配置 ========== */
#include "config_hw.h"
```

### 3.5 kb904/config_hw.h

```c
#pragma once

/* ========================================
 * KB904 硬件 GPIO 映射
 * 直接对应 KB904_原理图.md
 * ======================================== */

/* ========== 矩阵配置 ========== */
#define MATRIX_ROWS           8
#define MATRIX_COLS           16
#define DIODE_DIRECTION       COL2ROW

/* 行线 (ROW) - 输出 */
#define MATRIX_ROW_PINS {     \
    B9,  /* R0 - U2.7  */     \
    B8,  /* R1 - U2.8  */     \
    B7,  /* R2 - U2.17 */     \
    B6,  /* R3 - U2.18 */     \
    B5,  /* R4 - U2.19 */     \
    B4,  /* R5 - U2.20 */     \
    B3,  /* R6 - U2.21 */     \
    B2   /* R7 - U2.22 */     \
}

/* 列线 (COL) - 输入带上拉 */
#define MATRIX_COL_PINS {     \
    B17, /* C0  - U2.9  */    \
    B16, /* C1  - U2.10 */    \
    B15, /* C2  - U2.11 */    \
    B14, /* C3  - U2.12 */    \
    B13, /* C4  - U2.13 */    \
    B12, /* C5  - U2.14 */    \
    B11, /* C6  - U2.15 */    \
    B10, /* C7  - U2.16 */    \
    A7,  /* C8  - U2.4  */    \
    A6,  /* C9  - U2.38 */    \
    A5,  /* C10 - U2.37 */    \
    A4,  /* C11 - U2.36 */    \
    A1,  /* C12 - U2.40 */    \
    A0,  /* C13 - U2.39 */    \
    B1,  /* C14 - U2.23 */    \
    A15  /* C15 - U2.43 */    \
}

/* ========== 背光 RGB ========== */
#define BACKLIGHT_PIN_R       B0   /* PB0_PWM_R - U2.24 */
#define BACKLIGHT_PIN_G       A12  /* PA12_PWM_G - U2.46 */
#define BACKLIGHT_PIN_B       A13  /* PA13_PWM_B - U2.45 */

/* ========== 指示灯 ========== */
#define LED_CAPS_PIN          A15  /* 白灯 - 大写锁定 */
#define LED_BT_PIN            B22  /* 蓝灯 - 蓝牙状态 */
#define LED_POWER_PIN         B23  /* 红灯 - 电源/低电量 */
#define LED_CHARGE_PIN        A14  /* 绿灯 - 充满电（只读） */

/* ========== 触控板 I2C ========== */
#define TOUCHPAD_SDA          B20  /* PB20_SDA */
#define TOUCHPAD_SCL          B21  /* PB21_SCL */
#define TOUCHPAD_INT          A3   /* PA3_INT - 下降沿触发 */

/* ========== 触控板尺寸 ========== */
#define TOUCHPAD_MAX_X        2048
#define TOUCHPAD_MAX_Y        1024

/* ========== 电源管理 ========== */
#define POWER_KEY_PIN         A2   /* PA2_POWER_KEY_IN */
#define POWER_EN_PIN          B19  /* PB19_POWER_EN */
#define BATTERY_ADC_PIN       A14  /* PA14_ADC */

/* ========== UART 调试 ========== */
#define DEBUG_UART_TX         A9   /* PA9_TXD1 */
#define DEBUG_UART_RX         A8   /* PA8_RXD1 */
```

---

## 4. 配置验证机制

### 4.1 编译时检查

```c
/* keyboards/config_check.h */
#pragma once

/* 必需配置项验证 */
#ifndef PRODUCT_ID
    #error "PRODUCT_ID must be defined in product config.h"
#endif

#ifndef BLE_PAIRING_NAME
    #error "BLE_PAIRING_NAME must be defined"
#endif

/* 范围验证 */
#if BACKLIGHT_LEVEL_COUNT < 1 || BACKLIGHT_LEVEL_COUNT > 10
    #error "BACKLIGHT_LEVEL_COUNT must be 1-10"
#endif

#if BACKLIGHT_COLOR_COUNT < 1 || BACKLIGHT_COLOR_COUNT > 20
    #error "BACKLIGHT_COLOR_COUNT must be 1-20"
#endif

/* 依赖验证 */
#ifdef TOUCHPAD_ENABLE
    #ifndef TOUCHPAD_SDA
        #error "TOUCHPAD_ENABLE requires TOUCHPAD_SDA definition"
    #endif
    #ifndef TOUCHPAD_SCL
        #error "TOUCHPAD_ENABLE requires TOUCHPAD_SCL definition"
    #endif
#endif

/* GPIO 冲突检测（示例） */
#if defined(BACKLIGHT_PIN_R) && defined(LED_CAPS_PIN)
    #if BACKLIGHT_PIN_R == LED_CAPS_PIN
        #error "GPIO conflict: BACKLIGHT_PIN_R and LED_CAPS_PIN use the same pin"
    #endif
#endif
```

---

## 5. 迁移计划

### 5.1 阶段一：创建基础结构（1天）

| 任务 | 文件 |
|------|------|
| 创建 defaults 目录和默认配置文件 | `keyboards/defaults/*.h` |
| 创建 config_check.h | `keyboards/config_check.h` |
| 创建 KB904 配置目录 | `keyboards/kb904/` |

### 5.2 阶段二：迁移背光模块（0.5天）

| 任务 | 说明 |
|------|------|
| 创建 kb904/config.h | 产品配置 |
| 创建 kb904/config_hw.h | GPIO 映射 |
| 修改 backlight_config.h | 引用 defaults |
| 修改 backlight.c | 使用新配置 |

### 5.3 阶段三：迁移其他模块（1天）

| 模块 | 配置文件 |
|------|----------|
| 蓝牙 | `defaults/ble_defaults.h` |
| 电源 | `defaults/power_defaults.h` |
| 指示灯 | `defaults/indicator_defaults.h` |
| 键盘行为 | `defaults/keyboard_defaults.h` |

### 5.4 阶段四：清理和验证（0.5天）

| 任务 | 说明 |
|------|------|
| 删除旧的 product_config.h 中的重复配置 | 保留过渡期兼容 |
| 添加迁移文档 | `docs/config-migration-guide.md` |
| 验证编译 | 确保所有模块正常 |

---

## 6. 验收标准

### 6.1 功能验收

- [ ] KB904 配置目录结构完整
- [ ] defaults 目录包含所有模块默认值
- [ ] config_hw.h 与原理图 GPIO 一一对应
- [ ] 编译通过，无警告
- [ ] 配置覆盖机制正常工作

### 6.2 质量验收

- [ ] 新产品只需创建 config.h 和 config_hw.h
- [ ] 配置项命名规范统一（模块前缀）
- [ ] 所有配置项有注释说明
- [ ] config_check.h 验证必需配置

---

## 7. 后续优化

| 优化项 | 说明 |
|--------|------|
| 配置文档生成 | 从配置文件自动生成文档 |
| IDE 支持 | 配置项智能提示 |
| 配置对比工具 | 比较不同产品的配置差异 |

---

*文档结束*
