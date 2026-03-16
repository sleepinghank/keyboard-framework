// middleware/keyboard/combo/media_combo.c
#include "kb_combo_engine.h"
#include "kb_fn_action.h"
#include "kb_sys_action.h"
#include "keycode.h"

// 组合键枚举
enum combos {
    FN_DOWN_ID = 0,
    FN_UP_ID,
    FN_F1_ID,
    FN_F2_ID,
    FN_F3_ID,
    FN_F4_ID,
    FN_F5_ID,
    FN_F6_ID,
    FN_F7_ID,
    FN_F8_ID,
    FN_F9_ID,
    FN_F10_ID,
    FN_F11_ID,
    FN_F12_ID,
    FN_DEL_ID,
    Reset_factory_ID,
    COMBOS_END,
};

uint8_t number_of_combos = COMBOS_END;

// 组合键触发键定义
const uint16_t PROGMEM FN_DOWN_combo[] = { S_FN_KEY, COMBO_END };
const uint16_t PROGMEM FN_UP_combo[]   = { S_FN_KEY, COMBO_END };

const uint16_t PROGMEM FN_F1_combo[]  = { KC_F1,  COMBO_END };
const uint16_t PROGMEM FN_F2_combo[]  = { KC_F2,  COMBO_END };
const uint16_t PROGMEM FN_F3_combo[]  = { KC_F3,  COMBO_END };
const uint16_t PROGMEM FN_F4_combo[]  = { KC_F4,  COMBO_END };
const uint16_t PROGMEM FN_F5_combo[]  = { KC_F5,  COMBO_END };
const uint16_t PROGMEM FN_F6_combo[]  = { KC_F6,  COMBO_END };
const uint16_t PROGMEM FN_F7_combo[]  = { KC_F7,  COMBO_END };
const uint16_t PROGMEM FN_F8_combo[]  = { KC_F8,  COMBO_END };
const uint16_t PROGMEM FN_F9_combo[]  = { KC_F9,  COMBO_END };
const uint16_t PROGMEM FN_F10_combo[] = { KC_F10, COMBO_END };
const uint16_t PROGMEM FN_F11_combo[] = { KC_F11, COMBO_END };
const uint16_t PROGMEM FN_F12_combo[] = { KC_F12, COMBO_END };

const uint16_t PROGMEM FN_DEL_combo[]       = { S_FN_KEY, KC_DELETE, COMBO_END };
const uint16_t PROGMEM Reset_factory_combo[] = { KC_ESCAPE, KC_EQUAL, COMBO_END };

// 组合键数组
combo_t key_combos[] = {
    // Fn 键按下/释放
    [FN_DOWN_ID] = COMBO(FN_DOWN_combo, PRESS_DOWN, FN_DOWN_KEY),
    [FN_UP_ID]   = COMBO(FN_UP_combo,   PRESS_UP,   FN_UP_KEY),

    // F1~F12 媒体键（长按保持触发）
    [FN_F1_ID]  = COMBO_LONG_TICKS(FN_F1_combo,  5, LONG_PRESS_HOLD, Bringhness_Down),
    [FN_F2_ID]  = COMBO_LONG_TICKS(FN_F2_combo,  5, LONG_PRESS_HOLD, Bringhness_Up),
    [FN_F3_ID]  = COMBO_LONG_TICKS(FN_F3_combo,  5, LONG_PRESS_HOLD, Search),
    [FN_F4_ID]  = COMBO_LONG_TICKS(FN_F4_combo,  5, PRESS_DOWN,      Print_Screen),
    [FN_F5_ID]  = COMBO_LONG_TICKS(FN_F5_combo,  5, LONG_PRESS_HOLD, Earth),
    [FN_F6_ID]  = COMBO_LONG_TICKS(FN_F6_combo,  5, LONG_PRESS_HOLD, Media_Previous),
    [FN_F7_ID]  = COMBO_LONG_TICKS(FN_F7_combo,  5, LONG_PRESS_HOLD, Media_Play_Pause),
    [FN_F8_ID]  = COMBO_LONG_TICKS(FN_F8_combo,  5, LONG_PRESS_HOLD, Media_Next),
    [FN_F9_ID]  = COMBO_LONG_TICKS(FN_F9_combo,  5, LONG_PRESS_HOLD, Volume_Mute),
    [FN_F10_ID] = COMBO_LONG_TICKS(FN_F10_combo, 5, LONG_PRESS_HOLD, Volume_Down),
    [FN_F11_ID] = COMBO_LONG_TICKS(FN_F11_combo, 5, LONG_PRESS_HOLD, Volume_Up),
    [FN_F12_ID] = COMBO_LONG_TICKS(FN_F12_combo, 5, PRESS_DOWN,      Lock_Screen),

    // 特殊组合键
    [FN_DEL_ID]        = COMBO_LONG_TICKS(FN_DEL_combo,        3000, LONG_PRESS_START, Pair_button),
    [Reset_factory_ID] = COMBO_LONG_TICKS(Reset_factory_combo, 3000, LONG_PRESS_START, Factory_Reset),
};
