// middleware/keyboard/combo/FN_Combo.c
#include "kb_fn_action.h"
#include "keycode.h"
#include "kb_combo_engine.h"

// 外部变量
extern uint8_t FN_st;
extern uint8_t combinations_flag;

// Fn 键按下
uint8_t FN_DOWN_KEY(uint16_t* add_keys) {
    FN_st = 1;
    combinations_flag = 1;
    return 0;
}

// Fn 键释放
uint8_t FN_UP_KEY(uint16_t* add_keys) {
    FN_st = 0;
    combinations_flag = 0;
    return 0;
}

// 亮度降低
uint8_t Bringhness_Down(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F1;
    } else {
        add_keys[idx++] = M_BACKLIGHT_DOWN;
    }
    return idx;
}

// 亮度增加
uint8_t Bringhness_Up(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F2;
    } else {
        add_keys[idx++] = M_BACKLIGHT_UP;
    }
    return idx;
}

// 音量降低
uint8_t Volume_Down(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F10;
    } else {
        add_keys[idx++] = M_VOLUME_DOWN;
    }
    return idx;
}

// 音量增加
uint8_t Volume_Up(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F11;
    } else {
        add_keys[idx++] = M_VOLUME_UP;
    }
    return idx;
}

// 静音
uint8_t Volume_Mute(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F9;
    } else {
        add_keys[idx++] = M_MUTE;
    }
    return idx;
}

// 上一首
uint8_t Media_Previous(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F6;
    } else {
        add_keys[idx++] = M_SCAN_PREVIOUS_TRACK;
    }
    return idx;
}

// 播放/暂停
uint8_t Media_Play_Pause(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F7;
    } else {
        add_keys[idx++] = M_PLAY_N_PAUSE;
    }
    return idx;
}

// 下一首
uint8_t Media_Next(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F8;
    } else {
        add_keys[idx++] = M_SCAN_NEXT_TRACK;
    }
    return idx;
}

// 截图
uint8_t Print_Screen(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F4;
    } else {
        // Windows: Win+Shift+S
        add_keys[idx++] = KC_LEFT_GUI;
        add_keys[idx++] = KC_LEFT_SHIFT;
        add_keys[idx++] = KC_S;
    }
    return idx;
}

// 锁屏
uint8_t Lock_Screen(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F12;
    } else {
        // Windows: Win+L
        add_keys[idx++] = KC_LEFT_GUI;
        add_keys[idx++] = KC_L;
    }
    return idx;
}

// 搜索
uint8_t Search(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F3;
    } else {
        add_keys[idx++] = M_WWW_SEARCH;
    }
    return idx;
}

// 多任务
uint8_t Multi_Task(uint16_t* add_keys) {
    uint8_t idx = 0;
    // Windows: Win+Tab
    add_keys[idx++] = KC_LEFT_GUI;
    add_keys[idx++] = KC_TAB;
    return idx;
}

// 小地球
uint8_t Earth(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F5;
    } else {
        add_keys[idx++] = M_EARTH;
    }
    return idx;
}

// Fn+Esc
uint8_t FN_ESC_button(uint16_t* add_keys) {
    uint8_t idx = 0;
    add_keys[idx++] = KC_ESCAPE;
    return idx;
}
