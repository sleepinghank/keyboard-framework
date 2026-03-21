/*
 * 默认键位映射 - 使用 JIS 113 键布局
 */

#pragma once

#include "layout_8x16.h"
#include "keycode.h"
#include "progmem.h"
#include <stdint.h>
#include "combo.h"
#include "kb904/config.h"

enum layers {
    MAC_BASE,
    WIN_BASE,
    WIN_FN,
};

#include "stdint.h"

// clang-format off
enum {
    KC_LOPTN = QK_KB_0,
    KC_ROPTN,
    KC_LCMMD,
    KC_RCMMD,
    KC_MCTRL,
    KC_LNPAD,
    KC_TASK_VIEW,
    KC_FILE_EXPLORER,
    KC_SCREEN_SHOT,
    KC_CORTANA,
#ifdef WIN_LOCK_SCREEN_ENABLE
    KC_WIN_LOCK_SCREEN,
    __KC_WIN_LOCK_SCREEN_NEXT,
#else
    __KC_WIN_LOCK_SCREEN_NEXT = KC_CORTANA + 1,
#endif
#ifdef MAC_LOCK_SCREEN_ENABLE
    KC_MAC_LOCK_SCREEN = __KC_WIN_LOCK_SCREEN_NEXT,
    __KC_MAC_LOCK_SCREEN_NEXT,
#else
    __KC_MAC_LOCK_SCREEN_NEXT = __KC_WIN_LOCK_SCREEN_NEXT,
#endif
    KC_SIRI = __KC_MAC_LOCK_SCREEN_NEXT,
#ifdef LK_WIRELESS_ENABLE
    BT_HST1,
    BT_HST2,
    BT_HST3,
    P2P4G,
    BAT_LVL,
#endif
#ifdef ANANLOG_MATRIX
    PROF1,
    PROF2,
    PROF3,
#endif
#ifdef LED_MATRIX_ENABLE
    BL_SPI,
    BL_SPD,
#endif
    NEW_SAFE_RANGE,

#ifdef TOUCHPAD_ENABLE
    TOUCH_LOCK,
#endif
};
#ifndef LK_WIRELESS_ENABLE
    #define BT_HST1     KC_TRANS
    #define BT_HST2     KC_TRANS
    #define BT_HST3     KC_TRANS
    #define P2P4G       KC_TRANS
    #define BAT_LVL     KC_TRANS
#endif
#ifndef ANANLOG_MATRIX
    #define PROF1 KC_TRANS
    #define PROF2 KC_TRANS
    #define PROF3 KC_TRANS
#endif

#define KC_TASK KC_TASK_VIEW
#define KC_FILE KC_FILE_EXPLORER
#define KC_SNAP KC_SCREEN_SHOT
#define KC_CTANA KC_CORTANA
#define KC_WLCK KC_WIN_LOCK_SCREEN
#define KC_MLCK KC_MAC_LOCK_SCREEN


#define FN_MAC MO(MAC_FN)
#define FN_WIN MO(WIN_FN)

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
        [MAC_BASE] = LAYOUT_8x16(
    KC_Q,        KC_TAB,          KC_A,         KC_ESCAPE,      KC_Z,         _______,    KC_GRAVE,      KC_1,         //col1        
    KC_W,        KC_CAPS_LOCK,    KC_S,         _______,        KC_X,         _______,    KC_BRID,         KC_2,         //col2       
    KC_E,        _______,         KC_D,         _______,        KC_C,         _______,    KC_BRIU,         KC_3,         //col3       
    KC_R,        KC_T,            KC_F,         KC_G,           KC_V,         KC_B,       KC_5,          KC_4,         //col4       
    KC_U,        KC_Y,            KC_J,         KC_H,           KC_M,         KC_N,       KC_6,          KC_7,         //col5       
    KC_I,        KC_RBRC,         KC_K,         KC_MPRV,          KC_COMM,      _______,    KC_EQUAL,      KC_8,         //col6        
    KC_O,        KC_MPLY,           KC_L,         _______,        KC_DOT,       _______,    KC_MNXT,         KC_9,         //col7       
    _______,     KC_SNAP,           KC_WSCH,        KC_UP,          _______,      KC_LEFT,    _______,       _______,      //col14       
    _______,     _______,         _______,      _______,        _______,      KC_RIGHT,   _______,       _______,      //col12       
    _______,     KC_LALT,         _______,      _______,        _______,      _______,     _______,      _______,      //col16        
    _______,     _______,         _______,      _______,        KC_RALT,      _______,    MO(MAC_FN),    QK_USER_0,        //col0        
    _______,     _______,         KC_LCTL,      KC_LCMD,        _______,      KC_RCMD,    _______,       _______,       //col9       
    KC_P,        KC_LBRC,         KC_SCLN,      KC_QUOTE,       _______,      KC_SLASH,   KC_MINUS,      KC_0,         //col8      
    KC_BSLS,     KC_DELETE,       _______,      _______,        KC_ENTER,     _______,    KC_MUTE,         _______,      //col10       
    _______,     KC_LSFT,         KC_RSFT,      _______,        _______,      KC_VOLU,     _______,       KC_VOLD,       //col15
    KC_SPC,      _______,         _______,      _______,        _______,      KC_DOWN,    _______,       _______    ), //col15
    [MAC_FN] = LAYOUT_8x16(
    _______,        _______,          _______,         _______,      _______,         _______,    _______,      _______,    //col1        
    _______,        _______,          _______,         _______,      _______,         _______,    _______,      _______,    //col2       
    _______,        _______,          _______,         _______,      _______,         _______,    _______,      _______,    //col3       
    _______,        TOUCH_LOCK,          _______,         _______,      _______,         _______,    _______,      _______,    //col4       
    _______,        _______,          _______,         _______,      _______,         _______,    _______,      _______,    //col5       
    _______,        _______,          _______,         _______,      _______,         _______,    _______,      _______,    //col6        
    _______,        _______,          _______,         _______,      _______,         _______,    _______,      _______,    //col7       
    _______,        _______,          _______,         _______,      _______,         _______,    _______,      _______,    //col14       
    _______,        _______,          _______,         _______,      _______,         _______,    _______,      _______,    //col12       
    _______,        _______,          _______,         _______,      _______,         _______,    _______,      _______,    //col16        
    _______,        _______,          _______,         _______,      _______,         _______,    _______,      _______,    //col0        
    _______,        _______,          _______,         _______,      _______,         BAT_LVL,    _______,      _______,    //col9       
    _______,        _______,          _______,         _______,      _______,         _______,    _______,      _______,    //col8      
    _______,        BT_HST1,          _______,         _______,      BL_STEP,         _______,    _______,      _______,    //col10       
    _______,        _______,          BL_STEP,         _______,      _______,         _______,    _______,      _______,    //col15
    _______,        _______,          _______,         _______,      _______,         _______,    _______,      _______),   //col15
};


enum combo_events { AB_COMBO, BC_COMBO, AD_COMBO, DE_COMBO, FGHI_COMBO };

const uint16_t ab_combo[] PROGMEM   = {KC_A, KC_B, COMBO_END};
const uint16_t bc_combo[] PROGMEM   = {KC_B, KC_C, COMBO_END};
const uint16_t ad_combo[] PROGMEM   = {KC_A, LCTL_T(KC_D), COMBO_END};
const uint16_t de_combo[] PROGMEM   = {LCTL_T(KC_D), LT(1, KC_E), COMBO_END};
const uint16_t fghi_combo[] PROGMEM = {KC_F, KC_G, KC_H, KC_I, COMBO_END};

// clang-format off
combo_t key_combos[] = {
    [AB_COMBO] = COMBO(ab_combo, KC_SPC),  // KC_A + KC_B = KC_SPC
    [BC_COMBO] = COMBO(bc_combo, KC_X),    // KC_B + KC_C = KC_X
    [AD_COMBO] = COMBO(ad_combo, KC_Y),    // KC_A + LCTL_T(KC_D) = KC_Y
    [DE_COMBO] = COMBO(de_combo, KC_Z),    // LCTL_T(KC_D) + LT(1, KC_E) = KC_Z
    [FGHI_COMBO] = COMBO(fghi_combo, KC_W) // KC_F + KC_G + KC_H + KC_I = KC_W
};
