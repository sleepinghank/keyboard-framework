/*
 * LAYOUT_jis_113 - JIS 113键键盘布局定义
 * 基于日本工业标准 (JIS X 4052)
 */

#pragma once

#include "keycode.h"

// 矩阵配置
#define MATRIX_ROWS 8
#define MATRIX_COLS 15

// JIS 113键布局 - 标准键位定义
#define LAYOUT_jis_113( \
    K00, K01, K02, K03, K04, K05, K06, K07, K08, K09, K0A, K0B, K0C, K0D, K0E, \
    K10, K11, K12, K13, K14, K15, K16, K17, K18, K19, K1A, K1B, K1C, K1D, K1E, \
    K20, K21, K22, K23, K24, K25, K26, K27, K28, K29, K2A, K2B, K2C, K2D, K2E, \
    K30, K31, K32, K33, K34, K35, K36, K37, K38, K39, K3A, K3B, K3C, K3D, \
    K40, K41, K42, K43, K44, K45, K46, K47, K48, K49, K4A, K4B, K4C, K4D, \
    K50, K51, K52, K53, K54, K55, K56, K57, K58, K59, K5A, K5B, K5C, K5D, \
    K60, K61, K62, K63, K64, K65, K66, K67, K68, K69, K6A, K6B, K6C, \
    K70, K71, K72, K73, K74, K75, K76, K77, K78, K79, K7A, K7B, K7C  \
) { \
    { K00, K01, K02, K03, K04, K05, K06, K07, K08, K09, K0A, K0B, K0C, K0D, K0E }, \
    { K10, K11, K12, K13, K14, K15, K16, K17, K18, K19, K1A, K1B, K1C, K1D, K1E }, \
    { K20, K21, K22, K23, K24, K25, K26, K27, K28, K29, K2A, K2B, K2C, K2D, K2E }, \
    { K30, K31, K32, K33, K34, K35, K36, K37, K38, K39, K3A, K3B, K3C, K3D, KC_NO   }, \
    { K40, K41, K42, K43, K44, K45, K46, K47, K48, K49, K4A, K4B, K4C, K4D, KC_NO   }, \
    { K50, K51, K52, K53, K54, K55, K56, K57, K58, K59, K5A, K5B, K5C, K5D, KC_NO   }, \
    { K60, K61, K62, K63, K64, K65, K66, K67, K68, K69, K6A, K6B, K6C, KC_NO,   KC_NO   }, \
    { K70, K71, K72, K73, K74, K75, K76, K77, K78, K79, K7A, K7B, K7C, KC_NO,   KC_NO   }  \
}

// 标准JIS 113键布局键位映射
#define LAYOUT_jis_113_default \
LAYOUT_jis_113( \
    ESC,  F1,   F2,   F3,   F4,   F5,   F6,   F7,   F8,   F9,   F10,  F11,  F12,        PSCR,SLCK,PAUS, \
    GRV,  1,    2,    3,    4,    5,    6,    7,    8,    9,    0,    MINS, EQL,  BSPC,        INS, HOME,PGUP, \
    TAB,  Q,    W,    E,    R,    T,    Y,    U,    I,    O,    P,    LBRC, RBRC,                DEL, END, PGDN, \
    CAPS, A,    S,    D,    F,    G,    H,    J,    K,    L,    SCLN, QUOT, NUBS, ENT,                         \
    LSFT, NUHS, Z,    X,    C,    V,    B,    N,    M,    COMM, DOT,  SLSH, RSFT,                              \
    LCtrl,LGUI,LALT,          SPC,                RALT, RGUI, APP,  RCTL,                              \
                                  LEFT,DOWN,UP,RGHT                                            \
)
