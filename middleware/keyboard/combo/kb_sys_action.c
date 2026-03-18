// middleware/keyboard/combo/Special_Combo.c
#include "kb_sys_action.h"
#include "event_manager.h"
#include "system_service.h"

// 组合键触发标志位
uint8_t combinations_flag = 1;

// OSAL 事件定义（需要在 sys_config.h 或相关头文件中定义）
// 示例事件 ID，根据实际项目调整
#ifndef SYS_EVT_STORAGE_WRITE
#define SYS_EVT_STORAGE_WRITE    0x0001
#endif
#ifndef SYS_EVT_FACTORY_RESET
#define SYS_EVT_FACTORY_RESET    0x0002
#endif
#ifndef SYS_EVT_BLE_PAIRING
#define SYS_EVT_BLE_PAIRING      0x0004
#endif
#ifndef SYS_EVT_BACKLIGHT_COLOR
#define SYS_EVT_BACKLIGHT_COLOR  0x0008
#endif
#ifndef SYS_EVT_BACKLIGHT_BRIGHT
#define SYS_EVT_BACKLIGHT_BRIGHT 0x0010
#endif
#ifndef SYS_EVT_TOUCH_TOGGLE
#define SYS_EVT_TOUCH_TOGGLE     0x0020
#endif
#ifndef SYS_EVT_BATTERY_CHECK
#define SYS_EVT_BATTERY_CHECK    0x0040
#endif

// 系统服务任务 ID（需要根据实际项目定义）
#ifndef SYSTEM_SERVICE_TASK_ID
#define SYSTEM_SERVICE_TASK_ID   0
#endif

// Fn 锁定
uint8_t Set_FN_Lock(uint16_t* add_keys) {
    // TODO: 实现 Fn 锁定逻辑
    OSAL_SetEvent(SYSTEM_SERVICE_TASK_ID, SYS_EVT_STORAGE_WRITE);
    return 0;
}

// 切换背光颜色
uint8_t Backlight_Color(uint16_t* add_keys) {
    OSAL_SetEvent(SYSTEM_SERVICE_TASK_ID, SYS_EVT_BACKLIGHT_COLOR);
    OSAL_SetEvent(SYSTEM_SERVICE_TASK_ID, SYS_EVT_STORAGE_WRITE);
    return 0;
}

// 切换背光亮度
uint8_t Backlight_Light(uint16_t* add_keys) {
    OSAL_SetEvent(SYSTEM_SERVICE_TASK_ID, SYS_EVT_BACKLIGHT_BRIGHT);
    OSAL_SetEvent(SYSTEM_SERVICE_TASK_ID, SYS_EVT_STORAGE_WRITE);
    return 0;
}

// 触摸板锁定
uint8_t Touch_Locked(uint16_t* add_keys) {
    OSAL_SetEvent(SYSTEM_SERVICE_TASK_ID, SYS_EVT_TOUCH_TOGGLE);
    OSAL_SetEvent(SYSTEM_SERVICE_TASK_ID, SYS_EVT_STORAGE_WRITE);
    return 0;
}

// 恢复出厂设置
uint8_t Factory_Reset(uint16_t* add_keys) {
    OSAL_SetEvent(system_taskID, SYSTEM_FACTORY_RESET_EVT);
    return 0;
}

// 电池检测
uint8_t Bat_Check(uint16_t* add_keys) {
    OSAL_SetEvent(SYSTEM_SERVICE_TASK_ID, SYS_EVT_BATTERY_CHECK);
    return 0;
}

// 配对按钮
uint8_t Pair_button(uint16_t* add_keys) {
    OSAL_SetEvent(SYSTEM_SERVICE_TASK_ID, SYS_EVT_BLE_PAIRING);
    return 0;
}

#ifdef DE
uint8_t  X_Special_Combo(uint16_t* add_keys)
{
    uint8_t idx = 0;
    if(keycode_type == IOS || keycode_type == MAC){        
        add_keys[idx++] = KB_L_SHIFT;
        add_keys[idx++] = KB_GRAVE_ACCENT_N_TILDE;
    }else{
        add_keys[idx++] = KB_L_SHIFT;
        add_keys[idx++] = KB_EUROPE_2;
    }
    return idx;
}
uint8_t  Z_Special_Combo(uint16_t* add_keys)
{
    uint8_t idx = 0;
    if(keycode_type == IOS|| keycode_type == MAC){
        add_keys[idx++] = KB_GRAVE_ACCENT_N_TILDE;
    }else {
        add_keys[idx++] = KB_EUROPE_2;
    }
    return idx;
}
uint8_t C_Special_Combo(uint16_t* add_keys)
{
    uint8_t idx = 0;
    if(keycode_type == IOS|| keycode_type == MAC){
        add_keys[idx++] = KB_L_ALT;
        add_keys[idx++] = KB_7;
    }else {
        add_keys[idx++] = KB_R_ALT;
        add_keys[idx++] = KB_EUROPE_2;
    }
    return idx;
}
#endif


#ifdef UK
uint8_t X_Special_Combo(uint16_t* add_keys)
{
    uint8_t idx = 0;
    if(keycode_type == IOS){
        add_keys[idx++] = KB_L_SHIFT;
        add_keys[idx++] = KB_BACK_SPLASH_N_VERTICAL_BAR;
    }else if(keycode_type == WINDOWS){
    add_keys[idx++] = KB_EUROPE_2;
    }
    return idx;
}
uint8_t Z_Special_Combo(uint16_t* add_keys)
{
    uint8_t idx = 0;
    if(keycode_type == IOS){
        add_keys[idx++] = KB_BACK_SPLASH_N_VERTICAL_BAR;
    }else if(keycode_type == WINDOWS){
        add_keys[idx++] = KB_L_SHIFT;
        add_keys[idx++] = KB_EUROPE_2;
    }
    return idx;
}
#endif


// uint8_t Earth_PRESS_UP_Handler(uint16_t* add_keys)
// {
//     uint8_t idx = 0;   
//     if (combinations_flag == 1){
//         if(keycode_type == IOS || keycode_type == MAC){
//             add_keys[idx++] = M_EARTH;
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
//             add_keys[idx++] = M_EARTH;
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
