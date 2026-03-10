/**
 * @file default.c
 * @brief N0046 默认键位映射
 *
 * Layer 0: US 标准布局，F1-F12 多媒体功能
 * Layer 1: Fn 层，系统控制和背光调节
 */

#include "layout_n0046.h"
#include "keycode.h"
#include "progmem.h"
#include <stdint.h>
#include "config.h"

/* 层级定义 */
enum layers {
    LAYER_BASE = 0,    /* 默认层 - US 标准布局 */
    LAYER_FN,          /* Fn 层 - 多媒体和系统控制 */
    LAYER_COUNT
};

/* 自定义键码定义 */
enum n0046_keycodes {
    KC_SIRI = QK_KB_0,     /* Siri 唤起 (Fn+F6) */
    KC_BAT,                 /* 电量查询 (Fn+右Cmd) */
    BL_COLOR,               /* 背光颜色切换 (Fn+右Enter) */
    BL_LEVEL,               /* 背光亮度切换 (Fn+右Shift) */
    BT_PAIR,                /* 蓝牙配对 (Fn+Del) */
    FACTORY_RST,            /* 恢复出厂 (ESC+= 长按) */
    NEW_SAFE_RANGE
};

/* Fn 键定义 */
#define FN_MO MO(LAYER_FN)

/* KC_TRNS 别名 */
#define _______ KC_TRNS
#define XXXXXXX KC_NO

/*
 * Layer 0: 默认层 - US 标准布局
 * F1-F12 在默认层为标准功能键
 */
const uint16_t PROGMEM keymaps[LAYER_COUNT][MATRIX_ROWS][MATRIX_COLS] = {
    [LAYER_BASE] = LAYOUT_n0046(
        /* ESC, F1-F12, DEL */
        KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_DEL,
        /* `~, 1-0, -, =, BSPC */
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC,
        /* TAB, Q-P, [, ], \ */
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS,
        /* CAPS, A-L, ;, ', ENTER */
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_ENT,
        /* LSHIFT, Z-M, ,, ., /, RSHIFT */
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,
        /* LCTRL, LALT, LGUI, SPACE, RGUI, RALT, FN, RCTRL */
        KC_LCTL, KC_LALT, KC_LGUI,                   KC_SPC,                          KC_RGUI, KC_RALT, FN_MO,   KC_RCTL
    ),

    /*
     * Layer 1: Fn 层 - 多媒体和系统控制
     * F1: ESC (防止误触)
     * F2/F3: 屏幕亮度减/加
     * F4: 搜索 (Find)
     * F5: 截屏
     * F6: Siri
     * F7-F9: 媒体控制
     * F10-F12: 音量控制
     * Del: 蓝牙配对
     * 右Enter: 背光颜色
     * 右Shift: 背光亮度
     * 右Cmd: 电量查询
     */
    [LAYER_FN] = LAYOUT_n0046(
        /* ESC -> TRNS, F1=ESC, F2-F12 多媒体, DEL=BT_PAIR */
        KC_TRNS, KC_ESC,  KC_BRID, KC_BRIU, KC_FIND, KC_PSCR, KC_SIRI, KC_MPRV, KC_MPLY, KC_MNXT, KC_MUTE, KC_VOLD, KC_VOLU, BT_PAIR,
        /* 数字行透传 */
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        /* QWERTY 上行透传 */
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        /* ASDF 行透传，ENTER=背光颜色 */
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, BL_COLOR,
        /* ZXCV 行透传，RSHIFT=背光亮度 */
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, BL_LEVEL,
        /* 底行透传，右Cmd=电量查询 */
        KC_TRNS, KC_TRNS, KC_TRNS,                   KC_TRNS,                         KC_BAT,  KC_TRNS, KC_TRNS, KC_TRNS
    )
};
