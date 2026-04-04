// middleware/keyboard/combo/Special_Combo.c
#include "kb904/config_product.h"
#include "kb_sys_action.h"
#include "config_product.h"
#include "event_manager.h"
#include "output_service.h"
#include "input_service.h"
#include "communication_service.h"
#include "storage.h"
#include "system_service.h"
#include "debug.h"

// 组合键触发标志位
uint8_t combinations_flag = 1;

// Fn 锁定
uint8_t Set_FN_Lock(uint16_t* add_keys) {
    dprintf("Set_FN_Lock triggered\n");
    uint8_t current = STORAGE_GET_FN_LOCK();
    STORAGE_SET_FN_LOCK(current ? 0 : 1);
    // OSAL_SetEvent(system_taskID, SYSTEM_STORAGE_EVT);
    return 0;
}

// 切换背光颜色
uint8_t Backlight_Color(uint16_t* add_keys) {
    dprintf("Backlight_Color triggered\n");

    OSAL_SetEvent(output_taskID, OUTPUT_BACKLIGHT_COLOR_EVT);
    return 0;
}

// 切换背光亮度
uint8_t Backlight_Light(uint16_t* add_keys) {
    dprintf("Backlight_Light triggered\n");
    OSAL_SetEvent(output_taskID, OUTPUT_BACKLIGHT_BRIGHTNESS_EVT);
    return 0;
}

// 触摸板锁定
uint8_t Touch_Locked(uint16_t* add_keys) {
    dprintf("Touch_Locked triggered\n");
    // OSAL_SetEvent(input_taskID, INPUT_TOUCH_TOGGLE_EVT);
    // OSAL_SetEvent(system_taskID, SYSTEM_STORAGE_EVT);
    return 0;
}

// 恢复出厂设置
uint8_t Factory_Reset(uint16_t* add_keys) {
    dprintf("Factory_Reset triggered\n");
    OSAL_SetEvent(system_taskID, SYSTEM_FACTORY_RESET_EVT);
    return 0;
}

// 电池检测
uint8_t Bat_Check(uint16_t* add_keys) {
    dprintf("Bat_Check triggered\n");
    OSAL_SetEvent(output_taskID, OUTPUT_BATTERY_CHECK_EVT);
    return 0;
}

// 配对按钮
uint8_t Pair_button(uint16_t* add_keys) {
    dprintf("Pair_button triggered\n");
    OSAL_SetEvent(commu_taskID, WL_REQ_PAIR);
    return 0;
}


/* =====================================================
 * 特殊组合键（按国家差异）
 * X 键组合：输出特殊字符
 * ===================================================== */
#if IS_COUNTRY_DE
// 德国布局：Shift + ` (grave) 或 Shift + < (Europe 2)
uint8_t X_Special_Combo(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (keycode_type == IOS || keycode_type == MAC) {
        add_keys[idx++] = KB_L_SHIFT;
        add_keys[idx++] = KB_GRAVE_ACCENT_N_TILDE;
    } else {
        add_keys[idx++] = KB_L_SHIFT;
        add_keys[idx++] = KB_EUROPE_2;
    }
    return idx;
}

// Z 键组合：Y/Z 互换布局的特殊处理
uint8_t Z_Special_Combo(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (keycode_type == IOS || keycode_type == MAC) {
        add_keys[idx++] = KB_GRAVE_ACCENT_N_TILDE;
    } else {
        add_keys[idx++] = KB_EUROPE_2;
    }
    return idx;
}

// C 键组合：输出特殊字符
uint8_t C_Special_Combo(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (keycode_type == IOS || keycode_type == MAC) {
        add_keys[idx++] = KB_L_ALT;
        add_keys[idx++] = KB_7;
    } else {
        add_keys[idx++] = KB_R_ALT;
        add_keys[idx++] = KB_EUROPE_2;
    }
    return idx;
}

#elif IS_COUNTRY_UK
// 英国布局：` 和 # 位置差异
uint8_t X_Special_Combo(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (keycode_type == IOS) {
        add_keys[idx++] = KB_L_SHIFT;
        add_keys[idx++] = KB_BACK_SPLASH_N_VERTICAL_BAR;
    } else if (keycode_type == WINDOWS) {
        add_keys[idx++] = KB_EUROPE_2;
    }
    return idx;
}

uint8_t Z_Special_Combo(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (keycode_type == IOS) {
        add_keys[idx++] = KB_BACK_SPLASH_N_VERTICAL_BAR;
    } else if (keycode_type == WINDOWS) {
        add_keys[idx++] = KB_L_SHIFT;
        add_keys[idx++] = KB_EUROPE_2;
    }
    return idx;
}

#else
// 默认美国布局（无特殊组合键差异）
// 如需添加美国特定的组合键处理，在此处实现
#endif


// uint8_t Earth_PRESS_UP_Handler(uint16_t* add_keys)
// {
//     uint8_t idx = 0;   
//     if (combinations_flag == 1){
//         if(keycode_type == IOS || keycode_type == MAC){
//             add_keys[idx++] = KC_KEYBOARD_LAYOUT;
//             INFO_log("Earth_PRESS_UP_Handler```````````````` \n");
//         }else if(keycode_type == WINDOWS){
//             add_keys[idx++] = KB_L_GUI;
//             add_keys[idx++] = KB_SPACEBAR;
//             INFO_log("Windows_PRESS_UP_Handler```````````````` \n");
//         }
//         else if(keycode_type == ANDROID && Amazon_bit == 1){
//             add_keys[idx++] = KB_L_GUI;
//             add_keys[idx++] = KB_SPACEBAR;
//         }
//         else if(keycode_type == ANDROID){
//             add_keys[idx++] = KB_L_SHIFT;
//             add_keys[idx++] = KB_SPACEBAR;
//             INFO_log("sanxing_PRESS_UP_Handler```````````````` \n");
//         }
//         else{
//             add_keys[idx++] = KB_L_GUI;
//             add_keys[idx++] = KB_SPACEBAR;
//         }    
//     }
//     return idx;
// }

// uint8_t Earth_LONG_PRESS_START_UP_Handler(uint16_t* add_keys)
// {
//     uint8_t idx = 0;
//     if(keycode_type == IOS || keycode_type == MAC){
//         if (combinations_flag == 1 ){
//             add_keys[idx++] = KC_KEYBOARD_LAYOUT;
//             // INFO_log("Earth_LONG_PRESS_START_UP_Handler```````````````` \n");        
//         }
//     }
//     return idx;
// }

// uint8_t Page_Up(uint16_t* add_keys)
// {
//     uint8_t idx = 0;
//     add_keys[idx++] = KB_PAGEUP;
//     return idx;
// }
// //PAGE_DOWN
// uint8_t Page_Down(uint16_t* add_keys)
// {
//     uint8_t idx = 0;
//     add_keys[idx++] = KB_PAGEDOWN;
//     return idx;
// }

uint8_t Touch_Button_down(uint16_t* add_keys){
    writePinHigh(TOUCHPAD_BUTTON_PIN);
    return 0; 
}
uint8_t Touch_Button_up(uint16_t* add_keys){
    writePinLow(TOUCHPAD_BUTTON_PIN);
    return 0;
}
