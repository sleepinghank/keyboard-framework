# N0046 键盘固件开发实施计划

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 在 keyboard-framework 基础上实现 N0046 iPad 键盘完整固件功能

**Architecture:** 基于 CH584M 主控，复用现有 HAL/Driver/Middleware 层，新增 keyboards/n0046/ 产品目录，实现产品特定的配置、键位映射和业务逻辑

**Tech Stack:** C 语言、CMake、CH584M (RISC-V)、BLE 5.3、PWM、ADC

---

## 项目概述

### 产品规格

| 项目 | 规格 |
|------|------|
| 产品型号 | N0046 |
| 主控芯片 | CH584M (沁恒) |
| 蓝牙版本 | BLE 5.3 |
| 连接方式 | 蓝牙单通道 |
| 目标平台 | iPadOS 16/17/18 |
| 键盘布局 | US (本次优先) |
| 蓝牙名称 | "inateck AceTouch" |
| 电池 | 3.7V 500mAh 锂电池 |

### 开发范围

**本次实现：**
- 键盘矩阵扫描与防抖
- 蓝牙配对/回连/连接
- 指示灯业务逻辑（蓝牙状态、大写锁定、低电量、充电状态）
- 背光业务逻辑（七彩切换、亮度调节、5秒休眠、低电量关闭）
- Fn 组合键功能（配对、颜色、亮度、电量查询）
- 恢复出厂设置（ESC+= 3秒）
- 多媒体/系统快捷键（F1-F12）
- F6 默认 Siri 功能

**本次不实现：**
- 触控板模块（后续专项）
- OTA 升级功能（后续专项）
- F6 自定义功能（后续随 APP）
- 软件开关机功能（后续专项实现）
- 电源键功能（后续专项实现）

### 分支规划

| 分支名 | 功能范围 | 依赖 | 优先级 |
|--------|----------|------|--------|
| `feature/n0046-product-config` | 产品基础配置 | 无 | P0 |
| `feature/n0046-keyboard-input` | 键盘输入 | 分支一 | P0 |
| `feature/n0046-bluetooth-connection` | 蓝牙连接 | 分支一、二 | P0 |
| `feature/n0046-indicator-system` | 指示灯系统 | 分支一、三 | P0 |
| `feature/n0046-backlight-system` | 背光系统 | 分支一 | P1 |
| `feature/n0046-special-functions` | 特殊功能 | 分支一、三、四、五 | P0/P1 |

---

## 分支一：feature/n0046-product-config

### 需求概述

**目标：** 创建 N0046 产品的配置目录和基础文件，为后续功能开发提供配置基础。

**硬件上下文：**

| 项目 | 规格 |
|------|------|
| 主控 | CH584M (QFN-48, RISC-V, BLE 5.3) |
| 工作电压 | 3.3V |
| 晶振 | 32MHz |

**GPIO 配置（来自原理图）：**

| 功能 | GPIO |
|------|------|
| 矩阵行线 R0-R7 | PA4, PA5, PA6, PA0, PA1, PA8, PA9, PB9 |
| 矩阵列线 C0-C15 | PB5, PB8, PB17, PB16, PB15, PB14, PB13, PB12, PB3, PB4, PB2, PA7, PB7, PB18, PB1, PB6 |
| 白灯 (CAPS/蓝牙) | PA15 (高电平) |
| 红灯 (电源/充电) | PB23 (高电平) |
| 背光使能 | PB22 |
| 背光 R/G/B | PB0, PA12, PA13 (PWM) |
| 电池 ADC | PA14 |

**需要创建的文件：**

| 文件 | 说明 |
|------|------|
| `keyboards/n0046/config.h` | 产品主配置 |
| `keyboards/n0046/config_gpio.h` | GPIO 映射 |
| `keyboards/n0046/config_matrix.h` | 矩阵配置 |
| `keyboards/n0046/config_led.h` | LED 配置 |
| `keyboards/n0046/rules.mk` | 编译规则 |
| `keyboards/n0046/n0046.c` | 产品入口框架 |

**验收标准：**
- [ ] 编译通过，无警告
- [ ] GPIO 配置与原理图一致
- [ ] 矩阵尺寸正确 (8x16)

### Task 1.1: 创建产品目录结构

**Files:**
- Create: `keyboards/n0046/` 目录

```bash
mkdir -p keyboards/n0046/keymaps
```

### Task 1.2: 创建 GPIO 配置文件

**Files:**
- Create: `keyboards/n0046/config_gpio.h`

```c
#pragma once

/**
 * @file config_gpio.h
 * @brief N0046 GPIO 配置
 */

/* ========== 矩阵行线 (ROW) - 输出 ========== */
#define MATRIX_ROW_PINS { A4, A5, A6, A0, A1, A8, A9, B9 }

/* ========== 矩阵列线 (COL) - 输入带上拉 ========== */
#define MATRIX_COL_PINS { B5, B8, B17, B16, B15, B14, B13, B12, \
                          B3, B4, B2, A7, B7, B18, B1, B6 }

/* ========== 指示灯 GPIO ========== */
#define IND_LED_WHITE_PIN    A15    // 白灯：大写锁定/蓝牙状态
#define IND_LED_RED_PIN      B23    // 红灯：充电/低电量

/* ========== 背光 GPIO ========== */
#define BACKLIGHT_EN_PIN     B22    // 背光电源使能
#define BACKLIGHT_R_PIN      B0     // 红色 PWM
#define BACKLIGHT_G_PIN      A12    // 绿色 PWM
#define BACKLIGHT_B_PIN      A13    // 蓝色 PWM

/* ========== 电源管理 GPIO ========== */
#define BAT_ADC_PIN          A14    // 电池电压 ADC
```

### Task 1.3: 创建矩阵配置文件

**Files:**
- Create: `keyboards/n0046/config_matrix.h`

```c
#pragma once

/**
 * @file config_matrix.h
 * @brief N0046 矩阵配置
 */

#define MATRIX_ROWS          8
#define MATRIX_COLS          16
#define DIODE_DIRECTION      ROW2COL
#define MATRIX_IO_DELAY      30
#define MATRIX_SCAN_TIMER    5
#define DEBOUNCE             5
#define DEBOUNCE_ALGORITHM   DEBOUNCE_SYM_DEFER_PK
#define MAX_MATRIX_ROWS      16
#define MATRIX_INPUT_PRESSED_STATE  0
```

### Task 1.4: 创建产品主配置文件

**Files:**
- Create: `keyboards/n0046/config.h`

```c
#pragma once

/**
 * @file config.h
 * @brief N0046 产品配置
 */

/* ========== 产品信息 ========== */
#define PRODUCT_ID           0x0046
#define VENDOR_ID            0x3554
#define MANUFACTURER         "inateck"
#define PRODUCT              "inateck AceTouch"
#define DEVICE_VER           0x0100

#include "config_gpio.h"
#include "config_matrix.h"

/* ========== 功能开关 ========== */
#define USB_ENABLE_FLAG      FALSE
#define BLUETOOTH_ENABLE     TRUE
#define BLUETOOTH_HOST_COUNT 1

/* ========== 蓝牙配置 ========== */
#define BLE_DEVICE_NAME      "inateck AceTouch"
#define BLE_PAIRING_TIMEOUT  60000
#define BLE_RECONN_TIMEOUT   60000
#define BLE_IDLE_TIMEOUT     600000

/* ========== 电池配置 ========== */
#define RVD_R1               560
#define RVD_R2               2000
#define BATTERY_LOW_THRESHOLD 20

/* ========== 按键配置 ========== */
#define TAPPING_TERM         200
#define COMBO_TERM           200
#define COMBO_ENABLE
#define EXTRAKEY_ENABLE
```

### Task 1.5: 创建 LED 配置文件

**Files:**
- Create: `keyboards/n0046/config_led.h`

```c
#pragma once

/**
 * @file config_led.h
 * @brief N0046 LED 配置
 */

/* ========== 指示灯配置 ========== */
#define IND_LED_COUNT        2
#define IND_LED_CAPS         0
#define IND_LED_POWER        1

/* ========== 背光配置 ========== */
#define BACKLIGHT_TYPE_RGB
#define BACKLIGHT_DEFAULT_ON          TRUE
#define BACKLIGHT_DEFAULT_BRIGHTNESS  66
#define BACKLIGHT_SLEEP_TIMEOUT       5000

typedef enum {
    BL_COLOR_RED, BL_COLOR_YELLOW, BL_COLOR_CYAN,
    BL_COLOR_BLUE, BL_COLOR_GREEN, BL_COLOR_WHITE, BL_COLOR_MAGENTA,
    BL_COLOR_COUNT
} bl_preset_color_t;

typedef enum {
    BL_LEVEL_OFF, BL_LEVEL_LOW, BL_LEVEL_MEDIUM, BL_LEVEL_HIGH, BL_LEVEL_COUNT
} bl_preset_level_t;
```

### Task 1.6: 创建编译规则文件

**Files:**
- Create: `keyboards/n0046/rules.mk`

```makefile
# N0046 编译规则
PRODUCT_NAME = n0046
SRC += keyboards/n0046/n0046.c keyboards/n0046/n0046_indicators.c \
       keyboards/n0046/n0046_backlight.c keyboards/n0046/keymaps/default.c
INC += keyboards/n0046
BACKLIGHT_ENABLE = yes
INDICATOR_ENABLE = yes
BLUETOOTH_ENABLE = yes
```

### Task 1.7: 创建产品入口文件框架

**Files:**
- Create: `keyboards/n0046/n0046.c`

```c
/**
 * @file n0046.c
 * @brief N0046 产品入口
 */

#include "config.h"
#include "config_led.h"
#include "keyboard.h"
#include "wireless.h"
#include "indicator.h"
#include "backlight.h"
#include "battery.h"

void n0046_setup(void) {}
void n0046_init(void) { indicator_init(); backlight_init(NULL); battery_init(); }
void n0046_task(void) { indicator_task(); backlight_task(); battery_task(); }
bool process_record_kb(uint16_t keycode, keyrecord_t *record) { return true; }
void wireless_enter_connected_kb(uint8_t host_idx) {}
void wireless_enter_disconnected_kb(uint8_t host_idx, uint8_t reason) {}
void wireless_enter_discoverable_kb(uint8_t host_idx) {}
void wireless_enter_reconnecting_kb(uint8_t host_idx) {}
```

### Task 1.8: 提交基础配置

```bash
git add keyboards/n0046/
git commit -m "$(cat <<'EOF'
feat(n0046): add product configuration

- Add GPIO configuration for matrix, indicators, backlight
- Add matrix configuration (8x16, ROW2COL)
- Add LED configuration (RGB backlight, 2 indicators)
- Add product entry file skeleton

Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>
EOF
)"
```

---

## 分支二：feature/n0046-keyboard-input

### 需求概述

**目标：** 实现 N0046 键盘的 US 布局键位映射，确保所有按键正确识别和发送。

**键盘规格：**
- 矩阵: 8行 x 16列
- 总键数: 78键

**键位布局：**
```
┌─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐
│ ESC │ F1  │ F2  │ F3  │ F4  │ F5  │ F6  │ F7  │ F8  │ F9  │ F10 │ F11 │ F12 │ Del │
├─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┤
│  `  │  1  │  2  │  3  │  4  │  5  │  6  │  7  │  8  │  9  │  0  │  -  │  =  │ Bsp │
├─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┤
│ Tab │  Q  │  W  │  E  │  R  │  T  │  Y  │  U  │  I  │  O  │  P  │  [  │  ]  │  \  │
├─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┤
│Caps │  A  │  S  │  D  │  F  │  G  │  H  │  J  │  K  │  L  │  ;  │  '  │ Enter│
├─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┤
│Shift│  Z  │  X  │  C  │  V  │  B  │  N  │  M  │  ,  │  .  │  /  │ Shift│
├─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┤
│Ctrl │ Alt │ GUI │                 Space                │ GUI │ Alt │ Fn  │Ctrl │
└─────┴─────┴─────┴──────────────────────────────────────┴─────┴─────┴─────┴─────┘
```

**Fn 层多媒体键：**
| 按键 | 功能 |
|------|------|
| F2 | 屏幕亮度减 |
| F3 | 屏幕亮度加 |
| F4 | 搜索 |
| F5 | 截屏 |
| F6 | Siri |
| F7 | 上一首 |
| F8 | 播放/暂停 |
| F9 | 下一首 |
| F10 | 静音 |
| F11 | 音量减 |
| F12 | 音量加 |

**验收标准：**
- [ ] 所有 78 键位置正确映射
- [ ] 基本打字功能正常
- [ ] Fn 层多媒体键正确映射

### Task 2.1: 创建键位映射文件

**Files:**
- Create: `keyboards/n0046/keymaps/default.c`

```c
/**
 * @file default.c
 * @brief N0046 US 布局键位映射
 */

#include "keycode.h"
#include "keymap_common.h"

const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = KEYMAP(
        KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,
        KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_NO,   KC_DEL,  KC_NO,
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,
        KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC, KC_NO,   KC_NO,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,
        KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS, KC_NO,   KC_NO,
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,
        KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_ENT,  KC_NO,   KC_NO,   KC_NO,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,
        KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT, KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_LCTL, KC_LALT, KC_LGUI, KC_SPC,  KC_RGUI, KC_RALT, MO(1),   KC_RCTL,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO
    ),
    [1] = KEYMAP(
        KC_ESC,  KC_BRID, KC_BRIU, KC_FIND, KC_PSCR, KC_SIRI, KC_MPRV, KC_MPLY,
        KC_MNXT, KC_MUTE, KC_VOLD, KC_VOLU, KC_NO,   KC_NO,   RESET,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   BL_STEP, KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   BL_TOGG, KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,   BL_UP,   BL_DOWN, KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_BAT,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO
    )
};

enum custom_keycodes {
    KC_SIRI = SAFE_RANGE,
    KC_BAT,
    BL_STEP,
    BL_UP,
    BL_DOWN,
    BL_TOGG
};
```

### Task 2.2: 提交键位映射

```bash
git add keyboards/n0046/keymaps/
git commit -m "$(cat <<'EOF'
feat(n0046): add US layout keymap

- Add 78-key US layout mapping
- Add Fn layer with multimedia keys
- Define custom keycodes for special functions

Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>
EOF
)"
```

---

## 分支三：feature/n0046-bluetooth-connection

### 需求概述

**目标：** 实现 N0046 键盘的蓝牙连接功能，包括配对、回连、连接状态管理。

**蓝牙规格：**
- BLE 5.3, 单通道
- 设备名称: "inateck AceTouch"
- 有效距离: 10 米

**状态机：**

| 状态 | 白灯行为 | 超时 |
|------|----------|------|
| WT_PARING | 快闪 (3次/秒) | 60秒 → 休眠 |
| WT_RECONNECTING | 慢闪 (1次/秒) | 60秒 → 休眠 |
| WT_CONNECTED | 熄灭 | - |
| WT_DISCONNECTED | 闪烁 | 10分钟 → 深度休眠 |

**配对功能：**
- 首次开机自动进入配对模式
- Fn+Del 长按 3 秒手动配对

**验收标准：**
- [ ] 蓝牙名称正确显示 "inateck AceTouch"
- [ ] 首次开机自动配对
- [ ] Fn+Del 长按3秒进入配对
- [ ] 回连功能正常

### Task 3.1: 实现蓝牙状态机回调

**Files:**
- Modify: `keyboards/n0046/n0046.c`

```c
#include "indicator.h"

void wireless_enter_connected_kb(uint8_t host_idx) {
    indicator_off(IND_LED_CAPS);
}

void wireless_enter_disconnected_kb(uint8_t host_idx, uint8_t reason) {
    indicator_set(IND_LED_CAPS, &IND_BLINK_SLOW);
}

void wireless_enter_discoverable_kb(uint8_t host_idx) {
    indicator_set(IND_LED_CAPS, &IND_BLINK_FAST);
}

void wireless_enter_reconnecting_kb(uint8_t host_idx) {
    indicator_set(IND_LED_CAPS, &IND_BLINK_SLOW);
}
```

### Task 3.2: 实现配对触发逻辑

**Files:**
- Modify: `keyboards/n0046/n0046.c`

```c
#include "wireless.h"
#include "timer.h"

#define PAIRING_HOLD_TIME 3000

static uint16_t pairing_keycode = KC_NO;
static uint32_t pairing_timer = 0;

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (keycode == KC_DEL && (get_mods() & MOD_BIT(KC_FN))) {
        if (record->event.pressed) {
            pairing_keycode = keycode;
            pairing_timer = timer_read32();
        } else {
            pairing_keycode = KC_NO;
        }
        return false;
    }

    if (pairing_keycode != KC_NO &&
        timer_elapsed32(pairing_timer) >= PAIRING_HOLD_TIME) {
        wireless_pairing();
        pairing_keycode = KC_NO;
    }

    return true;
}
```

### Task 3.3: 提交蓝牙连接功能

```bash
git add keyboards/n0046/n0046.c
git commit -m "$(cat <<'EOF'
feat(n0046): add bluetooth connection logic

- Add wireless state callbacks for indicator
- Add Fn+Del long press 3s for pairing trigger
- Add pairing hold time detection

Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>
EOF
)"
```

---

## 分支四：feature/n0046-indicator-system

### 需求概述

**目标：** 实现指示灯业务逻辑，包括大写锁定、蓝牙状态、充电状态、低电量警告和电量查询。

**指示灯定义：**

| LED | GPIO | 功能 |
|-----|------|------|
| 白灯 | PA15 | 大写锁定 / 蓝牙状态 |
| 红灯 | PB23 | 充电 / 低电量 |

**灯效逻辑：**

| 事件 | 白灯 | 红灯 |
|------|------|------|
| 大写锁定 | 常亮 | - |
| 蓝牙配对 | 快闪 | - |
| 蓝牙回连 | 慢闪 | - |
| 充电中 | - | 常亮 |
| 低电量 (<20%) | - | 闪烁 |
| 电量查询 | - | 闪烁 N 次 (25% 档位) |

**验收标准：**
- [ ] 大写锁定指示正确
- [ ] 蓝牙状态指示正确
- [ ] 低电量警告正确
- [ ] 电量查询灯效正确

### Task 4.1: 创建指示灯业务逻辑文件

**Files:**
- Create: `keyboards/n0046/n0046_indicators.h`
- Create: `keyboards/n0046/n0046_indicators.c`

```c
// n0046_indicators.h
#pragma once
#include <stdbool.h>

void n0046_indicator_caps_lock(bool enabled);
void n0046_indicator_charging(bool charging);
void n0046_indicator_low_battery(bool low);
void n0046_indicator_battery_query(void);
void n0046_indicator_factory_reset(void);
```

```c
// n0046_indicators.c
#include "n0046_indicators.h"
#include "indicator.h"
#include "battery.h"
#include "config_led.h"

void n0046_indicator_caps_lock(bool enabled) {
    if (enabled) indicator_set(IND_LED_CAPS, &IND_ON);
    else indicator_off(IND_LED_CAPS);
}

void n0046_indicator_charging(bool charging) {
    if (charging) indicator_set(IND_LED_POWER, &IND_ON);
    else indicator_off(IND_LED_POWER);
}

void n0046_indicator_low_battery(bool low) {
    if (low) indicator_set(IND_LED_POWER, &IND_BLINK_SLOW);
    else indicator_off(IND_LED_POWER);
}

void n0046_indicator_battery_query(void) {
    uint8_t percentage = battery_get_percentage();
    uint8_t blinks = (percentage + 24) / 25;
    if (blinks > 4) blinks = 4;
    if (blinks < 1) blinks = 1;

    ind_effect_t effect = {
        .mode = IND_MODE_BLINK, .on_ms = 200, .off_ms = 200,
        .delay_ms = 0, .duration_ms = 0, .repeat = blinks
    };
    indicator_set(IND_LED_POWER, &effect);
}

void n0046_indicator_factory_reset(void) {
    ind_effect_t effect = {
        .mode = IND_MODE_BLINK, .on_ms = 200, .off_ms = 200,
        .delay_ms = 0, .duration_ms = 0, .repeat = 3
    };
    indicator_set(IND_LED_CAPS, &effect);
    indicator_set(IND_LED_POWER, &effect);
}
```

### Task 4.2: 集成指示灯逻辑到主文件

**Files:**
- Modify: `keyboards/n0046/n0046.c`

```c
#include "n0046_indicators.h"

void n0046_task(void) {
    indicator_task();
    backlight_task();
    battery_task();

    if (battery_get_percentage() < BATTERY_LOW_THRESHOLD) {
        static bool low_battery_indicated = false;
        if (!low_battery_indicated) {
            n0046_indicator_low_battery(true);
            low_battery_indicated = true;
        }
    }
}
```

### Task 4.3: 提交指示灯系统

```bash
git add keyboards/n0046/
git commit -m "$(cat <<'EOF'
feat(n0046): add indicator system

- Add caps lock indicator
- Add charging/low battery indicator
- Add battery query blink effect
- Add factory reset blink effect

Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>
EOF
)"
```

---

## 分支五：feature/n0046-backlight-system

### 需求概述

**目标：** 实现 RGB 七彩背光功能，包括颜色切换、亮度调节、自动休眠和低电量保护。

**功能需求：**

| 功能 | 实现 |
|------|------|
| 颜色切换 | 循环：红→黄→青→蓝→绿→白→洋红 |
| 亮度调节 | 循环：关→1档→2档→3档 |
| 默认值 | 白光、2档 (66%) |
| 5秒休眠 | 定时器 + 按键唤醒 |
| 低电量关闭 | 电量 < 20% 时强制关闭 |

**颜色表：**

| 颜色 | R | G | B |
|------|---|---|---|
| 红 | 100 | 0 | 0 |
| 黄 | 100 | 100 | 0 |
| 青 | 0 | 100 | 100 |
| 蓝 | 0 | 0 | 100 |
| 绿 | 0 | 100 | 0 |
| 白 | 100 | 100 | 100 |
| 洋红 | 100 | 0 | 100 |

**验收标准：**
- [ ] Fn+右Enter 切换颜色
- [ ] Fn+右Shift 切换亮度
- [ ] 5秒无操作熄灭
- [ ] 低电量强制关闭

### Task 5.1: 创建背光业务逻辑文件

**Files:**
- Create: `keyboards/n0046/n0046_backlight.h`
- Create: `keyboards/n0046/n0046_backlight.c`

```c
// n0046_backlight.h
#pragma once

void n0046_backlight_init(void);
void n0046_backlight_apply(void);
void n0046_backlight_color_step(void);
void n0046_backlight_level_step(void);
void n0046_backlight_wake(void);
void n0046_backlight_low_battery(bool low);
void n0046_backlight_task(void);
void n0046_backlight_on_key(void);
```

```c
// n0046_backlight.c
#include "n0046_backlight.h"
#include "backlight.h"
#include "timer.h"
#include "battery.h"
#include "config.h"
#include "config_led.h"

static bl_preset_color_t current_color = BL_COLOR_WHITE;
static bl_preset_level_t current_level = BL_LEVEL_MEDIUM;
static bool backlight_enabled = true;
static bool low_battery_disabled = false;
static uint32_t sleep_timer = 0;

static const bl_rgb_t color_table[BL_COLOR_COUNT] = {
    [BL_COLOR_RED]     = {100,   0,   0},
    [BL_COLOR_YELLOW]  = {100, 100,   0},
    [BL_COLOR_CYAN]    = {  0, 100, 100},
    [BL_COLOR_BLUE]    = {  0,   0, 100},
    [BL_COLOR_GREEN]   = {  0, 100,   0},
    [BL_COLOR_WHITE]   = {100, 100, 100},
    [BL_COLOR_MAGENTA] = {100,   0, 100}
};

static const uint8_t level_table[BL_LEVEL_COUNT] = {
    [BL_LEVEL_OFF] = 0, [BL_LEVEL_LOW] = 33, [BL_LEVEL_MEDIUM] = 66, [BL_LEVEL_HIGH] = 100
};

void n0046_backlight_init(void) {
    current_color = BL_COLOR_WHITE;
    current_level = BL_LEVEL_MEDIUM;
    backlight_enabled = true;
    sleep_timer = timer_read32();
    n0046_backlight_apply();
}

void n0046_backlight_apply(void) {
    if (!backlight_enabled || current_level == BL_LEVEL_OFF) {
        backlight_disable();
        return;
    }
    bl_rgb_t color = color_table[current_color];
    uint8_t brightness = level_table[current_level];
    backlight_set_rgb(
        (uint16_t)color.r * brightness / 100,
        (uint16_t)color.g * brightness / 100,
        (uint16_t)color.b * brightness / 100
    );
    backlight_enable();
}

void n0046_backlight_color_step(void) {
    if (low_battery_disabled) return;
    current_color++;
    if (current_color >= BL_COLOR_COUNT) current_color = BL_COLOR_RED;
    backlight_enabled = true;
    n0046_backlight_apply();
    sleep_timer = timer_read32();
}

void n0046_backlight_level_step(void) {
    if (low_battery_disabled) return;
    current_level++;
    if (current_level >= BL_LEVEL_COUNT) current_level = BL_LEVEL_OFF;
    if (current_level == BL_LEVEL_OFF) {
        backlight_enabled = false;
        backlight_disable();
    } else {
        backlight_enabled = true;
        n0046_backlight_apply();
    }
    sleep_timer = timer_read32();
}

void n0046_backlight_wake(void) {
    if (low_battery_disabled || current_level == BL_LEVEL_OFF) return;
    backlight_enabled = true;
    n0046_backlight_apply();
    sleep_timer = timer_read32();
}

void n0046_backlight_low_battery(bool low) {
    if (low && !low_battery_disabled) {
        low_battery_disabled = true;
        backlight_disable();
    } else if (!low) {
        low_battery_disabled = false;
        n0046_backlight_wake();
    }
}

void n0046_backlight_task(void) {
    if (battery_get_percentage() < BATTERY_LOW_THRESHOLD) {
        n0046_backlight_low_battery(true);
    }
    if (backlight_enabled && timer_elapsed32(sleep_timer) >= BACKLIGHT_SLEEP_TIMEOUT) {
        backlight_disable();
        backlight_enabled = false;
    }
}

void n0046_backlight_on_key(void) {
    sleep_timer = timer_read32();
    n0046_backlight_wake();
}
```

### Task 5.2: 集成背光逻辑到主文件

**Files:**
- Modify: `keyboards/n0046/n0046.c`

```c
#include "n0046_backlight.h"

void n0046_init(void) {
    indicator_init();
    n0046_backlight_init();
    battery_init();
}

void n0046_task(void) {
    indicator_task();
    n0046_backlight_task();
    battery_task();
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) n0046_backlight_on_key();

    if (keycode == BL_STEP && record->event.pressed) {
        n0046_backlight_color_step();
        return false;
    }
    if ((keycode == BL_UP || keycode == BL_DOWN) && record->event.pressed) {
        n0046_backlight_level_step();
        return false;
    }
    return true;
}
```

### Task 5.3: 提交背光系统

```bash
git add keyboards/n0046/
git commit -m "$(cat <<'EOF'
feat(n0046): add backlight system

- Add 7-color cycling
- Add 4-level brightness
- Add 5-second sleep timer
- Add low battery force disable
- Default: white light, medium level

Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>
EOF
)"
```

---

## 分支六：feature/n0046-special-functions

### 需求概述

**目标：** 实现特殊功能，包括 Fn 组合键、恢复出厂设置、电量查询和多媒体键。

**组合键列表：**

| 组合键 | 功能 |
|--------|------|
| Fn+Del 长按3s | 蓝牙配对 |
| Fn+右Enter | 背光颜色切换 |
| Fn+右Shift | 背光亮度切换 |
| Fn+右Cmd | 电量查询 |
| ESC+= 长按3s | 恢复出厂设置 |

**恢复出厂流程：**
1. 清除蓝牙配对记录
2. 重置背光为默认值
3. 指示灯闪烁 3 次
4. 进入配对模式

**验收标准：**
- [ ] Fn+Del 长按3秒配对
- [ ] ESC+= 长按3秒恢复出厂
- [ ] Fn+右Cmd 电量查询
- [ ] F6 触发 Siri

### Task 6.1: 实现恢复出厂设置

**Files:**
- Modify: `keyboards/n0046/n0046.c`

```c
#include "wireless.h"
#include "n0046_indicators.h"
#include "n0046_backlight.h"

#define FACTORY_RESET_HOLD_TIME 3000
#define FACTORY_RESET_KEY_COUNT 2

static const uint16_t factory_reset_keys[2] = { KC_ESC, KC_EQL };
static bool factory_reset_pressed[2] = { false, false };
static uint32_t factory_reset_timer = 0;

static bool check_factory_reset_keys(void) {
    for (int i = 0; i < 2; i++) {
        if (!factory_reset_pressed[i]) return false;
    }
    return true;
}

static void do_factory_reset(void) {
    wireless_disconnect();
    n0046_backlight_init();
    n0046_indicator_factory_reset();
    wireless_pairing();
}
```

### Task 6.2: 实现电量查询

```c
bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    // ... 恢复出厂检测代码

    if (keycode == KC_BAT && record->event.pressed) {
        n0046_indicator_battery_query();
        return false;
    }
    return true;
}
```

### Task 6.3: 提交特殊功能

```bash
git add keyboards/n0046/
git commit -m "$(cat <<'EOF'
feat(n0046): add special functions

- Add ESC+= long press 3s for factory reset
- Add Fn+RightCmd for battery query
- Add Fn layer multimedia keys (F1-F12)
- Add F6 Siri trigger

Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>
EOF
)"
```

---

## 验收测试清单

### 编译验证

```bash
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build -j4
```

### 功能测试

| 功能 | 测试方法 | 预期结果 |
|------|----------|----------|
| 基本打字 | 连接蓝牙后打字 | 字符正确显示 |
| 大写锁定 | 按 Caps 键 | 白灯亮/灭 |
| 蓝牙配对 | Fn+Del 长按3秒 | 白灯快闪 |
| 背光颜色 | Fn+右Enter | 颜色循环 |
| 背光亮度 | Fn+右Shift | 亮度循环 |
| 背光休眠 | 5秒无操作 | 背光熄灭 |
| 低电量 | 电量<20% | 红灯闪烁 |
| 电量查询 | Fn+右Cmd | 红灯闪烁N次 |
| 恢复出厂 | ESC+= 长按3秒 | 灯闪烁3次 |
| 多媒体键 | F1-F12 | 功能正确 |

---

## 修订历史

| 版本 | 日期 | 说明 |
|------|------|------|
| V1.0 | 2026-03-06 | 初始版本 |
