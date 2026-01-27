# N0046 键盘固件实现计划

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 在 keyboard-framework 基础上实现 N0046 产品的完整固件，支持蓝牙连接、RGB背光、电源管理等功能。

**Architecture:** 采用分层架构，复用现有 HAL/驱动层，新增 N0046 产品配置目录。电源管理采用硬件开关机方案（B19 供电保持），蓝牙单通道连接，背光 7 色循环。

**Tech Stack:** C语言, CH584M SDK, 嵌入式开发

**Worktree:** `.worktrees/n0046-firmware` (分支: `feature/n0046-firmware`)

---

## Task 1: 创建 N0046 产品配置目录结构

**Files:**
- Create: `keyboards/n0046/n0046_config.h`
- Create: `keyboards/n0046/n0046_indicator.h`
- Create: `keyboards/n0046/n0046_backlight.h`
- Create: `keyboards/n0046/keymaps/keymap_us.h`

**Step 1: 创建目录结构**

Run:
```bash
mkdir -p keyboards/n0046/keymaps
```

**Step 2: 创建 n0046_config.h**

```c
/* Copyright 2025 @ keyboard-framework
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include "gpio.h"

/*============================================================================
 * N0046 产品配置
 *===========================================================================*/

/* 产品标识 */
#define PRODUCT_N0046
#define PRODUCT_ID          0x0046
#define VENDOR_ID           0x0001
#define MANUFACTURER        "inateck"
#define PRODUCT             "AceTouch"
#define DEVICE_VER          0x0100

/* 蓝牙配置 */
#define BT_DEVICE_NAME      "inateck AceTouch"
#define BT_HOST_DEVICES_COUNT   1       /* 单通道 */

/*============================================================================
 * 矩阵配置 (8行 x 16列)
 *===========================================================================*/

#define MATRIX_ROWS         8
#define MATRIX_COLS         16
#define MATRIX_IO_DELAY     30
#define DIODE_DIRECTION     ROW2COL
#define MATRIX_SCAN_TIMER   5

#define MATRIX_ROW_PINS     { A4, A5, A6, A0, A1, A8, A9, B9 }
#define MATRIX_COL_PINS     { B5, B8, B17, B16, B15, B14, B13, B12, B3, B4, B2, A7, B7, B18, B1, B6 }

#define MATRIX_INPUT_PRESSED_STATE  0

/*============================================================================
 * 指示灯配置
 *===========================================================================*/

#define LED_CAPS_BT_PIN     A15     /* 白灯：大写/蓝牙状态，高电平亮 */
#define LED_BATTERY_PIN     B23     /* 红灯：电量/充电，高电平亮 */

/*============================================================================
 * 背光 RGB 配置
 *===========================================================================*/

#define BACKLIGHT_POWER_PIN B22     /* 背光电源控制 */
#define BACKLIGHT_R_PIN     B0      /* 红色通道，高电平有效 */
#define BACKLIGHT_G_PIN     A12     /* 绿色通道，高电平有效 */
#define BACKLIGHT_B_PIN     A13     /* 蓝色通道，高电平有效 */

/*============================================================================
 * 触控板配置 (本次不实现，仅定义引脚)
 *===========================================================================*/

#define TOUCH_POWER_PIN     A11     /* 电源控制，低电平有效 */
#define TOUCH_INT_PIN       A3      /* 中断，下降沿触发 */
#define TOUCH_SDA_PIN       B20
#define TOUCH_SCL_PIN       B21

/*============================================================================
 * 电量检测配置
 *===========================================================================*/

#define BAT_ADC_PIN             A14
#define BATTERY_DIVIDER_R1      560     /* 上拉电阻 560kΩ */
#define BATTERY_DIVIDER_R2      2000    /* 下拉电阻 2MΩ */
/* 分压比: R2/(R1+R2) = 2000/(560+2000) = 0.781 */

#define BATTERY_LOW_THRESHOLD   20      /* 低电警告阈值 20% */

/*============================================================================
 * 电源控制配置
 *===========================================================================*/

#define POWER_BUTTON_PIN    A2      /* 电源按钮检测，低电平按下 */
#define POWER_HOLD_PIN      B19     /* 供电保持，高电平保持供电 */

#define POWER_LONG_PRESS_MS 3000    /* 长按 3 秒触发开/关机 */

/*============================================================================
 * 超时配置
 *===========================================================================*/

#define BT_PAIRING_TIMEOUT_SEC      60      /* 配对超时 60 秒 */
#define BT_RECONNECT_TIMEOUT_SEC    60      /* 回连超时 60 秒 */
#define IDLE_SLEEP_TIMEOUT_SEC      600     /* 无操作 10 分钟进入休眠 */
#define BACKLIGHT_IDLE_TIMEOUT_SEC  5       /* 背光 5 秒无操作熄灭 */

/*============================================================================
 * 防抖配置
 *===========================================================================*/

#define DEBOUNCE            5
#define DEBOUNCE_ALGORITHM  DEBOUNCE_SYM_DEFER_PK
#define DEBOUNCE_DELAY      5
#define MAX_MATRIX_ROWS     16

/*============================================================================
 * 功能开关
 *===========================================================================*/

#define USB_ENABLE_FLAG         FALSE
#define BLUETOOTH_ENABLE_FLAG   TRUE
#define LK_WIRELESS_ENABLE
#define EXTRAKEY_ENABLE
#define COMBO_ENABLE

/*============================================================================
 * 语种配置 (编译时选择)
 *===========================================================================*/

#ifndef KEYBOARD_LAYOUT
#define KEYBOARD_LAYOUT     LAYOUT_US
#endif

#define LAYOUT_US   0
#define LAYOUT_DE   1
#define LAYOUT_UK   2
#define LAYOUT_JP   3
#define LAYOUT_FR   4
```

**Step 3: 创建 n0046_indicator.h**

```c
/* Copyright 2025 @ keyboard-framework */

#pragma once

#include "gpio.h"
#include "n0046_config.h"
#include <stdbool.h>

/*============================================================================
 * N0046 指示灯配置 (2灯)
 *===========================================================================*/

typedef struct {
    pin_t pin;
    bool  active_high;
} ind_led_def_t;

#define IND_LED_COUNT   2

static const ind_led_def_t ind_led_table[IND_LED_COUNT] = {
    /* 索引 0: 白灯 - 大写/蓝牙状态 */
    { LED_CAPS_BT_PIN, true },
    /* 索引 1: 红灯 - 电量/充电 */
    { LED_BATTERY_PIN, true },
};

/*============================================================================
 * 业务别名
 *===========================================================================*/

#define LED_CAPS        0       /* 大写锁定指示 */
#define LED_BT_STATUS   0       /* 蓝牙状态指示 (与 CAPS 共用) */
#define LED_BATTERY     1       /* 电量/充电指示 */

/*============================================================================
 * 蓝牙状态灯效定义
 *===========================================================================*/

/* 配对中: 快闪 (3次/秒, 约 333ms 周期) */
#define IND_BT_PAIRING      IND_BLINK_CUSTOM(100, 233, 0)

/* 回连中: 慢闪 (1次/秒, 1000ms 周期) */
#define IND_BT_RECONNECTING IND_BLINK_CUSTOM(500, 500, 0)

/* 已连接: 熄灭 */
#define IND_BT_CONNECTED    IND_OFF

/*============================================================================
 * 电量状态灯效定义
 *===========================================================================*/

/* 低电警告: 闪烁 */
#define IND_BAT_LOW         IND_BLINK_SLOW

/* 充电中: 常亮 */
#define IND_BAT_CHARGING    IND_ON

/* 正常/充满: 熄灭 */
#define IND_BAT_NORMAL      IND_OFF

/* 电量查询闪烁 N 次 */
#define IND_BAT_QUERY(n)    IND_BLINK_CUSTOM(200, 300, (n))
```

**Step 4: 创建 n0046_backlight.h**

```c
/* Copyright 2025 @ keyboard-framework */

#pragma once

#include "n0046_config.h"

/*============================================================================
 * N0046 背光配置 (7色 RGB)
 *===========================================================================*/

#define BACKLIGHT_TYPE_RGB

/* RGB 引脚定义 */
#define BL_PIN_R            BACKLIGHT_R_PIN
#define BL_PIN_G            BACKLIGHT_G_PIN
#define BL_PIN_B            BACKLIGHT_B_PIN
#define BL_PIN_POWER        BACKLIGHT_POWER_PIN

/* 引脚极性: 高电平有效 */
#define BL_PIN_ACTIVE_HIGH  1

/*============================================================================
 * 亮度档位配置
 *===========================================================================*/

#define BACKLIGHT_LEVEL_LOW     33      /* 一档: 33% */
#define BACKLIGHT_LEVEL_MEDIUM  66      /* 二档: 66% */
#define BACKLIGHT_LEVEL_HIGH    100     /* 三档: 100% */

/*============================================================================
 * 默认值
 *===========================================================================*/

#define BACKLIGHT_DEFAULT_ON            true
#define BACKLIGHT_DEFAULT_BRIGHTNESS    BACKLIGHT_LEVEL_MEDIUM  /* 默认二档 */
#define BACKLIGHT_DEFAULT_COLOR_INDEX   BL_COLOR_WHITE          /* 默认白色 */

/*============================================================================
 * 低电量背光控制
 *===========================================================================*/

#define BACKLIGHT_LOW_BATTERY_PCT       20  /* 低于 20% 强制关闭背光 */
```

**Step 5: 创建基础 keymap_us.h**

```c
/* Copyright 2025 @ keyboard-framework */

#pragma once

#include "keycode.h"

/*============================================================================
 * N0046 US 布局 - 8x16 矩阵
 *===========================================================================*/

/*
 * 物理布局参考:
 *
 * ESC  F2   F3   F4   F5   F6   F7   F8   F9   F10  F11  F12  PWR
 * `    1    2    3    4    5    6    7    8    9    0    -    =    BSPC
 * TAB  Q    W    E    R    T    Y    U    I    O    P    [    ]    \
 * CAPS A    S    D    F    G    H    J    K    L    ;    '    ENTER
 * LSFT Z    X    C    V    B    N    M    ,    .    /    RSFT  UP   DEL
 * LCTL LOPT LCMD          SPACE              RCMD  FN   LEFT DOWN RIGHT
 */

/* 多媒体键定义 (F1-F12 位置) */
#define KC_F1_FUNC  KC_ESC              /* ESC */
#define KC_F2_FUNC  KC_BRIGHTNESS_DOWN  /* 亮度减 */
#define KC_F3_FUNC  KC_BRIGHTNESS_UP    /* 亮度加 */
#define KC_F4_FUNC  KC_SPOTLIGHT        /* 搜索 */
#define KC_F5_FUNC  KC_SCREENSHOT       /* 截屏 */
#define KC_F6_FUNC  KC_SIRI             /* Siri (默认) */
#define KC_F7_FUNC  KC_MEDIA_PREV       /* 上一首 */
#define KC_F8_FUNC  KC_MEDIA_PLAY_PAUSE /* 暂停/播放 */
#define KC_F9_FUNC  KC_MEDIA_NEXT       /* 下一首 */
#define KC_F10_FUNC KC_MUTE             /* 静音 */
#define KC_F11_FUNC KC_VOLUME_DOWN      /* 音量减 */
#define KC_F12_FUNC KC_VOLUME_UP        /* 音量加 */

/* 自定义功能键码 */
#define KC_BL_COLOR     KC_USER_0       /* 背光颜色切换 */
#define KC_BL_LEVEL     KC_USER_1       /* 背光亮度切换 */
#define KC_BAT_QUERY    KC_USER_2       /* 电量查询 */
#define KC_BT_PAIR      KC_USER_3       /* 蓝牙配对 */

/*
 * 矩阵到物理位置映射
 * 需要根据实际 PCB 走线调整
 *
 * 格式: LAYOUT(row0_keys, row1_keys, ...)
 */

#define KEYMAP_US_LAYER_0 { \
    /* Row 0: Function row */ \
    KC_F1_FUNC, KC_F2_FUNC, KC_F3_FUNC, KC_F4_FUNC, KC_F5_FUNC, KC_F6_FUNC, \
    KC_F7_FUNC, KC_F8_FUNC, KC_F9_FUNC, KC_F10_FUNC, KC_F11_FUNC, KC_F12_FUNC, \
    KC_POWER, KC_NO, KC_NO, KC_NO, \
    \
    /* Row 1: Number row */ \
    KC_GRAVE, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, \
    KC_MINUS, KC_EQUAL, KC_BSPACE, KC_NO, KC_NO, \
    \
    /* Row 2: QWERTY top */ \
    KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, \
    KC_LBRACKET, KC_RBRACKET, KC_BSLASH, KC_NO, KC_NO, \
    \
    /* Row 3: Home row */ \
    KC_CAPS, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, \
    KC_SCOLON, KC_QUOTE, KC_ENTER, KC_NO, KC_NO, KC_NO, \
    \
    /* Row 4: Bottom row */ \
    KC_LSHIFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMMA, KC_DOT, \
    KC_SLASH, KC_RSHIFT, KC_UP, KC_DELETE, KC_NO, KC_NO, \
    \
    /* Row 5: Modifier row */ \
    KC_LCTRL, KC_LALT, KC_LGUI, KC_SPACE, KC_SPACE, KC_SPACE, KC_SPACE, \
    KC_RGUI, KC_FN, KC_LEFT, KC_DOWN, KC_RIGHT, KC_NO, KC_NO, KC_NO, KC_NO, \
    \
    /* Row 6-7: 未使用 */ \
    KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, \
    KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, \
    KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, \
    KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO  \
}

/* Fn 层 */
#define KEYMAP_US_LAYER_FN { \
    /* Row 0: Fn + Function row = 标准 F 键 */ \
    KC_ESCAPE, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, \
    KC_F10, KC_F11, KC_F12, KC_NO, KC_NO, KC_NO, \
    \
    /* Row 1 */ \
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, \
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_NO, KC_NO, \
    \
    /* Row 2 */ \
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, \
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_NO, KC_NO, \
    \
    /* Row 3 */ \
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, \
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_BL_COLOR, KC_NO, KC_NO, KC_NO, \
    \
    /* Row 4: Fn+Del=配对, Fn+RShift=亮度 */ \
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, \
    KC_TRNS, KC_TRNS, KC_TRNS, KC_BL_LEVEL, KC_TRNS, KC_BT_PAIR, KC_NO, KC_NO, \
    \
    /* Row 5: Fn+RCmd=电量查询 */ \
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, \
    KC_BAT_QUERY, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_NO, KC_NO, KC_NO, KC_NO, \
    \
    /* Row 6-7 */ \
    KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, \
    KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, \
    KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, \
    KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO  \
}
```

**Step 6: 提交**

Run:
```bash
git add keyboards/n0046/
git commit -m "feat(n0046): add product configuration files

- n0046_config.h: GPIO pins, matrix, bluetooth, power settings
- n0046_indicator.h: 2-LED configuration (caps/bt, battery)
- n0046_backlight.h: 7-color RGB backlight settings
- keymaps/keymap_us.h: US layout with multimedia keys

Co-Authored-By: Claude Opus 4.5 <noreply@anthropic.com>"
```

---

## Task 2: 实现电源管理模块 (power_switch)

**Files:**
- Create: `drivers/power/power_switch.h`
- Create: `drivers/power/power_switch.c`

**Step 1: 创建 power_switch.h**

```c
/* Copyright 2025 @ keyboard-framework */

#pragma once

#include <stdint.h>
#include <stdbool.h>

/*============================================================================
 * 电源状态枚举
 *===========================================================================*/

typedef enum {
    POWER_STATE_OFF = 0,        /* 断电状态 */
    POWER_STATE_BOOT_CHECK,     /* 上电检测中 */
    POWER_STATE_RUNNING,        /* 正常运行 */
    POWER_STATE_SLEEP,          /* 深度休眠 */
} power_state_t;

/*============================================================================
 * 电源按钮事件
 *===========================================================================*/

typedef enum {
    POWER_BTN_NONE = 0,         /* 无事件 */
    POWER_BTN_SHORT_PRESS,      /* 短按 (锁屏) */
    POWER_BTN_LONG_PRESS,       /* 长按 3 秒 (开/关机) */
} power_btn_event_t;

/*============================================================================
 * 接口函数
 *===========================================================================*/

/**
 * @brief 初始化电源管理模块
 *        配置电源按钮检测引脚和供电保持引脚
 */
void power_switch_init(void);

/**
 * @brief 电源管理任务
 *        在主循环中调用，处理电源按钮事件和状态转换
 */
void power_switch_task(void);

/**
 * @brief 获取当前电源状态
 * @return 电源状态枚举值
 */
power_state_t power_get_state(void);

/**
 * @brief 检查电源按钮是否按下
 * @return true=按下, false=松开
 */
bool power_button_pressed(void);

/**
 * @brief 获取电源按钮事件
 * @return 按钮事件类型
 */
power_btn_event_t power_get_button_event(void);

/**
 * @brief 执行关机
 *        拉低供电保持引脚，系统断电
 */
void power_shutdown(void);

/**
 * @brief 进入深度休眠
 *        保持供电但进入低功耗模式
 */
void power_enter_sleep(void);

/**
 * @brief 退出深度休眠
 *        从低功耗模式恢复
 */
void power_exit_sleep(void);

/**
 * @brief 重置空闲计时器
 *        有按键活动时调用
 */
void power_reset_idle_timer(void);

/**
 * @brief 开机检测
 *        在系统启动时调用，检测是否应该保持开机
 * @return true=保持开机, false=关机
 */
bool power_boot_check(void);
```

**Step 2: 创建 power_switch.c**

```c
/* Copyright 2025 @ keyboard-framework */

#include "power_switch.h"
#include "gpio.h"
#include "timer.h"

#ifdef PRODUCT_N0046
#include "n0046_config.h"
#endif

/*============================================================================
 * 配置
 *===========================================================================*/

#ifndef POWER_BUTTON_PIN
#error "POWER_BUTTON_PIN must be defined"
#endif

#ifndef POWER_HOLD_PIN
#error "POWER_HOLD_PIN must be defined"
#endif

#ifndef POWER_LONG_PRESS_MS
#define POWER_LONG_PRESS_MS     3000
#endif

#ifndef IDLE_SLEEP_TIMEOUT_SEC
#define IDLE_SLEEP_TIMEOUT_SEC  600
#endif

/*============================================================================
 * 内部状态
 *===========================================================================*/

static power_state_t g_power_state = POWER_STATE_OFF;
static power_btn_event_t g_btn_event = POWER_BTN_NONE;

static uint32_t g_btn_press_start = 0;      /* 按钮按下开始时间 */
static bool g_btn_was_pressed = false;      /* 上一次按钮状态 */
static bool g_long_press_triggered = false; /* 长按已触发标志 */

static uint32_t g_idle_timer = 0;           /* 空闲计时器 */

/*============================================================================
 * 内部函数
 *===========================================================================*/

static void power_hold_enable(void) {
    gpio_write_pin_high(POWER_HOLD_PIN);
}

static void power_hold_disable(void) {
    gpio_write_pin_low(POWER_HOLD_PIN);
}

/*============================================================================
 * 接口实现
 *===========================================================================*/

void power_switch_init(void) {
    /* 配置电源按钮引脚: 输入上拉 */
    gpio_set_pin_input(POWER_BUTTON_PIN, GPIO_PULL_UP);

    /* 配置供电保持引脚: 输出，初始低电平 */
    gpio_set_pin_output_push_pull(POWER_HOLD_PIN);
    gpio_write_pin_low(POWER_HOLD_PIN);

    /* 初始化状态 */
    g_power_state = POWER_STATE_BOOT_CHECK;
    g_btn_event = POWER_BTN_NONE;
    g_btn_press_start = 0;
    g_btn_was_pressed = false;
    g_long_press_triggered = false;
    g_idle_timer = timer_read32();
}

bool power_boot_check(void) {
    uint32_t start_time = timer_read32();

    /* 检测电源按钮是否持续按住 3 秒 */
    while (power_button_pressed()) {
        if (timer_elapsed32(start_time) >= POWER_LONG_PRESS_MS) {
            /* 长按 3 秒，保持供电 */
            power_hold_enable();
            g_power_state = POWER_STATE_RUNNING;
            g_idle_timer = timer_read32();
            return true;
        }
    }

    /* 按钮提前松开，不保持供电 */
    g_power_state = POWER_STATE_OFF;
    return false;
}

void power_switch_task(void) {
    bool btn_pressed = power_button_pressed();
    uint32_t now = timer_read32();

    /* 清除上一次事件 */
    g_btn_event = POWER_BTN_NONE;

    /* 运行状态下的按钮检测 */
    if (g_power_state == POWER_STATE_RUNNING) {
        /* 按钮按下边沿 */
        if (btn_pressed && !g_btn_was_pressed) {
            g_btn_press_start = now;
            g_long_press_triggered = false;
        }

        /* 按钮持续按下 */
        if (btn_pressed && g_btn_press_start > 0) {
            if (!g_long_press_triggered &&
                timer_elapsed32(g_btn_press_start) >= POWER_LONG_PRESS_MS) {
                /* 长按触发关机 */
                g_btn_event = POWER_BTN_LONG_PRESS;
                g_long_press_triggered = true;
            }
        }

        /* 按钮释放边沿 */
        if (!btn_pressed && g_btn_was_pressed) {
            if (!g_long_press_triggered && g_btn_press_start > 0) {
                /* 短按触发锁屏 */
                g_btn_event = POWER_BTN_SHORT_PRESS;
            }
            g_btn_press_start = 0;
        }

        /* 检查空闲超时 */
        if (timer_elapsed32(g_idle_timer) >= (IDLE_SLEEP_TIMEOUT_SEC * 1000)) {
            power_enter_sleep();
        }
    }

    g_btn_was_pressed = btn_pressed;
}

power_state_t power_get_state(void) {
    return g_power_state;
}

bool power_button_pressed(void) {
    /* 低电平表示按下 */
    return gpio_read_pin(POWER_BUTTON_PIN) == 0;
}

power_btn_event_t power_get_button_event(void) {
    return g_btn_event;
}

void power_shutdown(void) {
    g_power_state = POWER_STATE_OFF;
    power_hold_disable();
    /* 系统将在此后断电 */
    while (1) {
        /* 等待断电 */
    }
}

void power_enter_sleep(void) {
    g_power_state = POWER_STATE_SLEEP;
    /* TODO: 调用平台低功耗接口 */
}

void power_exit_sleep(void) {
    g_power_state = POWER_STATE_RUNNING;
    g_idle_timer = timer_read32();
}

void power_reset_idle_timer(void) {
    g_idle_timer = timer_read32();
}
```

**Step 3: 提交**

Run:
```bash
git add drivers/power/power_switch.h drivers/power/power_switch.c
git commit -m "feat(power): add power switch module for N0046

- Hardware power on/off via B19 hold pin
- Power button detection (A2, active low)
- Boot check: 3s long press to keep power
- Short press: lock screen event
- Long press: shutdown event
- Idle timeout: 10 minutes to sleep

Co-Authored-By: Claude Opus 4.5 <noreply@anthropic.com>"
```

---

## Task 3: 更新背光颜色顺序

**Files:**
- Modify: `drivers/output/backlight/backlight.c:28-36`

**Step 1: 修改颜色顺序**

将 `preset_colors` 数组的顺序改为：红→黄→青→蓝→绿→白→洋红

```c
static const bl_rgb_t preset_colors[BL_COLOR_COUNT] = {
    [BL_COLOR_RED]     = {100,   0,   0},
    [BL_COLOR_YELLOW]  = {100, 100,   0},
    [BL_COLOR_CYAN]    = {  0, 100, 100},
    [BL_COLOR_BLUE]    = {  0,   0, 100},
    [BL_COLOR_GREEN]   = {  0, 100,   0},
    [BL_COLOR_WHITE]   = {100, 100, 100},
    [BL_COLOR_MAGENTA] = {100,   0, 100},
};
```

**Step 2: 同步更新 backlight.h 中的枚举**

修改 `drivers/output/backlight/backlight.h:44-53`:

```c
typedef enum {
    BL_COLOR_RED = 0,
    BL_COLOR_YELLOW,
    BL_COLOR_CYAN,
    BL_COLOR_BLUE,
    BL_COLOR_GREEN,
    BL_COLOR_WHITE,
    BL_COLOR_MAGENTA,
    BL_COLOR_COUNT
} bl_preset_color_t;
```

**Step 3: 提交**

Run:
```bash
git add drivers/output/backlight/backlight.c drivers/output/backlight/backlight.h
git commit -m "feat(backlight): update color cycle order for N0046

Order: Red -> Yellow -> Cyan -> Blue -> Green -> White -> Magenta

Co-Authored-By: Claude Opus 4.5 <noreply@anthropic.com>"
```

---

## Task 4: 扩展电量检测模块

**Files:**
- Modify: `drivers/power/battery.h`
- Modify: `drivers/power/battery.c`

**Step 1: 在 battery.h 添加电量查询函数声明**

在文件末尾 `battery_is_empty()` 声明后添加:

```c
/*********************************************************************
 * @fn      battery_show_level
 *
 * @brief   通过指示灯闪烁显示电量档位
 *          1次=0-25%, 2次=26-50%, 3次=51-75%, 4次=76-100%
 *
 * @return  none
 */
void battery_show_level(void);

/*********************************************************************
 * @fn      battery_is_low
 *
 * @brief   检查电池是否处于低电状态 (<20%)
 *
 * @return  bool - true表示低电
 */
bool battery_is_low(void);
```

**Step 2: 在 battery.c 实现电量查询函数**

在文件末尾添加:

```c
/*********************************************************************
 * @fn      battery_show_level
 *
 * @brief   通过指示灯闪烁显示电量档位
 */
void battery_show_level(void) {
    uint8_t pct = battery_get_percentage();
    uint8_t blinks;

    if (pct == 0) {
        blinks = 1;
    } else if (pct <= 25) {
        blinks = 1;
    } else if (pct <= 50) {
        blinks = 2;
    } else if (pct <= 75) {
        blinks = 3;
    } else {
        blinks = 4;
    }

    /* 使用电量指示灯闪烁 */
    ind_effect_t effect = IND_BLINK_CUSTOM(200, 300, blinks);
    indicator_set(LED_BATTERY, &effect);
}

/*********************************************************************
 * @fn      battery_is_low
 *
 * @brief   检查电池是否处于低电状态 (<20%)
 */
bool battery_is_low(void) {
    return g_battery_percentage < 20;
}
```

**Step 3: 添加 LED_BATTERY 包含**

在 battery.c 顶部添加:

```c
#ifdef PRODUCT_N0046
#include "n0046_indicator.h"
#endif
```

**Step 4: 提交**

Run:
```bash
git add drivers/power/battery.h drivers/power/battery.c
git commit -m "feat(battery): add level query and low battery check

- battery_show_level(): show 1-4 blinks for 25% increments
- battery_is_low(): check if below 20%

Co-Authored-By: Claude Opus 4.5 <noreply@anthropic.com>"
```

---

## Task 5: 实现恢复出厂设置功能

**Files:**
- Create: `application/service/factory_reset.h`
- Create: `application/service/factory_reset.c`

**Step 1: 创建 factory_reset.h**

```c
/* Copyright 2025 @ keyboard-framework */

#pragma once

#include <stdint.h>
#include <stdbool.h>

/*============================================================================
 * 恢复出厂设置模块
 *===========================================================================*/

/**
 * @brief 初始化恢复出厂设置模块
 */
void factory_reset_init(void);

/**
 * @brief 恢复出厂设置检测任务
 *        在主循环中调用，检测 ESC + = 长按 3 秒
 */
void factory_reset_task(void);

/**
 * @brief 执行恢复出厂设置
 *        清除配对、重置背光、指示灯闪烁、进入配对模式
 */
void factory_reset_execute(void);

/**
 * @brief 检查是否正在执行恢复出厂
 * @return true=执行中
 */
bool factory_reset_in_progress(void);
```

**Step 2: 创建 factory_reset.c**

```c
/* Copyright 2025 @ keyboard-framework */

#include "factory_reset.h"
#include "matrix.h"
#include "timer.h"
#include "indicator.h"
#include "backlight.h"
#include "wireless.h"
#include "storage.h"

#ifdef PRODUCT_N0046
#include "n0046_config.h"
#include "n0046_indicator.h"
#include "n0046_backlight.h"
#endif

/*============================================================================
 * 配置
 *===========================================================================*/

#ifndef FACTORY_RESET_HOLD_MS
#define FACTORY_RESET_HOLD_MS   3000    /* 长按 3 秒触发 */
#endif

/* ESC 和 = 键在矩阵中的位置 (需根据实际 keymap 调整) */
#ifndef FACTORY_RESET_KEY1_ROW
#define FACTORY_RESET_KEY1_ROW  0       /* ESC 行 */
#endif
#ifndef FACTORY_RESET_KEY1_COL
#define FACTORY_RESET_KEY1_COL  0       /* ESC 列 */
#endif
#ifndef FACTORY_RESET_KEY2_ROW
#define FACTORY_RESET_KEY2_ROW  1       /* = 行 */
#endif
#ifndef FACTORY_RESET_KEY2_COL
#define FACTORY_RESET_KEY2_COL  12      /* = 列 */
#endif

/*============================================================================
 * 内部状态
 *===========================================================================*/

static uint32_t g_combo_start_time = 0;
static bool g_reset_in_progress = false;

/*============================================================================
 * 接口实现
 *===========================================================================*/

void factory_reset_init(void) {
    g_combo_start_time = 0;
    g_reset_in_progress = false;
}

void factory_reset_task(void) {
    if (g_reset_in_progress) {
        return;
    }

    bool key1_pressed = matrix_is_on(FACTORY_RESET_KEY1_ROW, FACTORY_RESET_KEY1_COL);
    bool key2_pressed = matrix_is_on(FACTORY_RESET_KEY2_ROW, FACTORY_RESET_KEY2_COL);

    if (key1_pressed && key2_pressed) {
        if (g_combo_start_time == 0) {
            g_combo_start_time = timer_read32();
        } else if (timer_elapsed32(g_combo_start_time) >= FACTORY_RESET_HOLD_MS) {
            factory_reset_execute();
            g_combo_start_time = 0;
        }
    } else {
        g_combo_start_time = 0;
    }
}

void factory_reset_execute(void) {
    g_reset_in_progress = true;

    /* 1. 清除蓝牙配对记录 */
#ifdef LK_WIRELESS_ENABLE
    wireless_clear_pairing();
#endif

    /* 2. 重置背光为默认值 (白光二档) */
    bl_state_t default_bl = {
        .enable = BACKLIGHT_DEFAULT_ON,
        .brightness = BACKLIGHT_DEFAULT_BRIGHTNESS,
        .color = {100, 100, 100}  /* 白色 */
    };
    backlight_restore_state(&default_bl);

    /* 3. 保存默认设置到存储 */
#ifdef STORAGE_ENABLE
    storage_save_backlight(&default_bl);
    storage_clear_user_settings();
#endif

    /* 4. 指示灯闪烁 3 次确认 */
    indicator_set(LED_CAPS, &IND_BLINK_3);
    indicator_set(LED_BATTERY, &IND_BLINK_3);

    /* 5. 延迟后进入蓝牙配对模式 */
#ifdef LK_WIRELESS_ENABLE
    /* 等待指示灯闪烁完成 (约 1.5 秒) */
    uint32_t wait_start = timer_read32();
    while (timer_elapsed32(wait_start) < 1500) {
        indicator_task();
    }
    wireless_start_pairing();
#endif

    g_reset_in_progress = false;
}

bool factory_reset_in_progress(void) {
    return g_reset_in_progress;
}
```

**Step 3: 提交**

Run:
```bash
git add application/service/factory_reset.h application/service/factory_reset.c
git commit -m "feat(factory): add factory reset module

- Trigger: ESC + = held for 3 seconds
- Actions: clear pairing, reset backlight to white/medium,
  flash indicators 3 times, enter pairing mode

Co-Authored-By: Claude Opus 4.5 <noreply@anthropic.com>"
```

---

## Task 6: 更新指示灯配置

**Files:**
- Modify: `keyboards/indicator_config.h`

**Step 1: 添加 N0046 条件编译**

在 `keyboards/indicator_config.h` 中添加 N0046 专用配置:

```c
#ifdef PRODUCT_N0046
#include "n0046_indicator.h"
#else
/* 原有的默认配置保持不变 */
// ... existing code ...
#endif
```

**Step 2: 提交**

Run:
```bash
git add keyboards/indicator_config.h
git commit -m "feat(indicator): add N0046 conditional include

Co-Authored-By: Claude Opus 4.5 <noreply@anthropic.com>"
```

---

## Task 7: 创建主应用入口整合

**Files:**
- Create: `application/n0046_main.c`

**Step 1: 创建 N0046 主应用**

```c
/* Copyright 2025 @ keyboard-framework */

#ifdef PRODUCT_N0046

#include "n0046_config.h"
#include "n0046_indicator.h"
#include "n0046_backlight.h"

#include "matrix.h"
#include "debounce.h"
#include "indicator.h"
#include "backlight.h"
#include "battery.h"
#include "power_switch.h"
#include "factory_reset.h"
#include "wireless.h"
#include "keyboard.h"
#include "timer.h"

/*============================================================================
 * 模块初始化
 *===========================================================================*/

void n0046_init(void) {
    /* 电源管理初始化 */
    power_switch_init();

    /* 开机检测 - 必须长按 3 秒才能开机 */
    if (!power_boot_check()) {
        /* 未通过开机检测，系统将断电 */
        return;
    }

    /* 定时器初始化 */
    timer_init();

    /* 矩阵扫描初始化 */
    matrix_init();

    /* 指示灯初始化 */
    indicator_init();

    /* 背光初始化 (使用默认值) */
    backlight_init(NULL);

    /* 电量检测初始化 */
    battery_init();

    /* 恢复出厂设置模块初始化 */
    factory_reset_init();

    /* 无线模块初始化 */
#ifdef LK_WIRELESS_ENABLE
    wireless_init();
#endif

    /* 键盘处理初始化 */
    keyboard_init();
}

/*============================================================================
 * 主循环任务
 *===========================================================================*/

void n0046_task(void) {
    /* 电源管理任务 */
    power_switch_task();

    /* 处理电源按钮事件 */
    power_btn_event_t btn_event = power_get_button_event();
    if (btn_event == POWER_BTN_LONG_PRESS) {
        power_shutdown();
    } else if (btn_event == POWER_BTN_SHORT_PRESS) {
        /* 发送锁屏键码 */
        keyboard_send_keycode(KC_POWER);
    }

    /* 恢复出厂设置检测 */
    factory_reset_task();

    /* 矩阵扫描 */
    if (matrix_task()) {
        /* 有按键活动，重置空闲计时器 */
        power_reset_idle_timer();
    }

    /* 键盘处理 */
    keyboard_task();

    /* 指示灯任务 */
    indicator_task();

    /* 电量检测任务 */
    battery_task();

    /* 低电量处理 */
    if (battery_is_low()) {
        /* 强制关闭背光 */
        if (backlight_is_enabled()) {
            backlight_disable();
        }
        /* 显示低电警告 */
        static bool low_bat_indicated = false;
        if (!low_bat_indicated) {
            indicator_set(LED_BATTERY, &IND_BAT_LOW);
            low_bat_indicated = true;
        }
    }

    /* 充电状态指示 */
    uint8_t charging = battery_get_charging_state();
    if (charging == BAT_CHARGING) {
        indicator_set(LED_BATTERY, &IND_BAT_CHARGING);
    } else if (charging == BAT_NOT_CHARGING && !battery_is_low()) {
        indicator_set(LED_BATTERY, &IND_BAT_NORMAL);
    }

    /* 无线任务 */
#ifdef LK_WIRELESS_ENABLE
    wireless_task();
#endif
}

#endif /* PRODUCT_N0046 */
```

**Step 2: 提交**

Run:
```bash
git add application/n0046_main.c
git commit -m "feat(n0046): add main application entry point

Integrates all modules:
- Power management with boot check
- Matrix scanning
- Indicator and backlight
- Battery monitoring with low battery handling
- Factory reset detection
- Wireless management

Co-Authored-By: Claude Opus 4.5 <noreply@anthropic.com>"
```

---

## Task 8: 更新 CMakeLists.txt

**Files:**
- Modify: `CMakeLists.txt`

**Step 1: 添加 N0046 相关源文件和编译选项**

在 CMakeLists.txt 中添加:

```cmake
# N0046 产品配置
option(PRODUCT_N0046 "Build for N0046 keyboard" OFF)
option(KEYBOARD_LAYOUT "Keyboard layout (US/DE/UK/JP/FR)" "US")

if(PRODUCT_N0046)
    add_definitions(-DPRODUCT_N0046)
    add_definitions(-DKEYBOARD_LAYOUT=LAYOUT_${KEYBOARD_LAYOUT})

    list(APPEND SOURCES
        keyboards/n0046/keymaps/keymap_us.c
        drivers/power/power_switch.c
        application/service/factory_reset.c
        application/n0046_main.c
    )

    include_directories(keyboards/n0046)
endif()
```

**Step 2: 提交**

Run:
```bash
git add CMakeLists.txt
git commit -m "build: add N0046 product configuration to CMake

- PRODUCT_N0046 option
- KEYBOARD_LAYOUT option (US/DE/UK/JP/FR)
- Include N0046 source files when enabled

Co-Authored-By: Claude Opus 4.5 <noreply@anthropic.com>"
```

---

## 验证清单

完成所有任务后，执行以下验证:

1. **编译检查**:
```bash
mkdir -p build && cd build
cmake .. -DPRODUCT_N0046=ON -DKEYBOARD_LAYOUT=US
make
```

2. **文件结构检查**:
```bash
ls -la keyboards/n0046/
ls -la keyboards/n0046/keymaps/
ls -la drivers/power/power_switch.*
ls -la application/service/factory_reset.*
```

3. **Git 提交历史检查**:
```bash
git log --oneline -10
```

---

**Plan complete and saved to `docs/plans/2026-01-25-n0046-implementation-plan.md`.**

