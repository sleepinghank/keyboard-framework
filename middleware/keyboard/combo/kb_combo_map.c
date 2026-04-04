// middleware/keyboard/combo/media_combo.c
#include "kb_combo_engine.h"
#include "kb_fn_action.h"
#include "kb_sys_action.h"
#include "keycode.h"

// 组合键枚举
enum combos {
    FN_KEY_ID = 0,
    FN_LONG_PRESS_ID,
    SCREEN_CAPTURE_ID,
    LOCK_SCREEN_ID,
    DIY_KEY_ID,

    FN_RIGHT_SHIFT_ID,
    FN_RIGHT_ENTER_ID,
    FN_RIGHT_CMD_ID,

    TOUCH_BUTTON_ID,
    BLE_PARING_ID,
    Reset_factory_ID,
    COMBOS_END,
};

uint8_t number_of_combos = COMBOS_END;

// 组合键触发键定义
const uint16_t PROGMEM FN_KEY_combo[] = { S_FN_KEY, COMBO_END };

const uint16_t PROGMEM SCREEN_CAPTURE_combo[]  = { KC_SCRE,  COMBO_END };
const uint16_t PROGMEM LOCK_SCREEN_combo[]  = { KC_LOCK,  COMBO_END };

const uint16_t PROGMEM FN_RIGHT_SHIFT_combo[] = { S_FN_KEY,KC_RSFT, COMBO_END };
const uint16_t PROGMEM FN_RIGHT_ENTER_combo[] = { S_FN_KEY,KC_ENTER, COMBO_END };
const uint16_t PROGMEM FN_RIGHT_CMD_combo[]   = { S_FN_KEY, KC_RCMD, COMBO_END };
const uint16_t PROGMEM DIY_KEY_combo[]     = { KC_DIY, COMBO_END };

const uint16_t PROGMEM TOUCH_BUTTON_combo[] = { KC_TUBT, COMBO_END };
const uint16_t PROGMEM BLE_PARING_combo[]       = { S_FN_KEY, KC_DELETE, COMBO_END };
const uint16_t PROGMEM Reset_factory_combo[] = { KC_ESCAPE, KC_EQUAL, COMBO_END };

// 组合键数组
combo_t key_combos[] = {
    // Fn 键按下/释放
    [FN_KEY_ID] = COMBO2(FN_KEY_combo, PRESS_DOWN, FN_DOWN_KEY,PRESS_UP,   FN_UP_KEY),
    [FN_LONG_PRESS_ID] = COMBO_LONG_MS(FN_KEY_combo,
                                       1500,
                                       LONG_PRESS_START,
                                       FN_LONG_PRESS_KEY),

    // F1~F12 特殊媒体键
    [SCREEN_CAPTURE_ID]  = COMBO(SCREEN_CAPTURE_combo, PRESS_DOWN, Print_Screen),
    [LOCK_SCREEN_ID]  = COMBO(LOCK_SCREEN_combo,PRESS_DOWN, Lock_Screen),
    [DIY_KEY_ID]         = COMBO(DIY_KEY_combo,     PRESS_DOWN, Siri_Invoke),

    // 新增 Fn 系统组合键，右侧 Shift/Enter/Cmd + Fn
    [FN_RIGHT_SHIFT_ID] = COMBO(FN_RIGHT_SHIFT_combo, PRESS_DOWN, Backlight_Light),
    [FN_RIGHT_ENTER_ID] = COMBO(FN_RIGHT_ENTER_combo, PRESS_DOWN, Backlight_Color),
    [FN_RIGHT_CMD_ID]   = COMBO(FN_RIGHT_CMD_combo,   PRESS_DOWN, Bat_Check),

    // 特殊组合键
    [BLE_PARING_ID]        = COMBO_LONG_MS(BLE_PARING_combo,        3000, LONG_PRESS_START, Pair_button),
    [Reset_factory_ID] = COMBO_LONG_MS(Reset_factory_combo, 3000, LONG_PRESS_START, Factory_Reset),
    [TOUCH_BUTTON_ID] = COMBO2(TOUCH_BUTTON_combo, PRESS_DOWN, Touch_Button_down, PRESS_UP, Touch_Button_up),
};
