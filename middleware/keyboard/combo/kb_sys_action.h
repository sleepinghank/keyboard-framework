// middleware/keyboard/combo/kb_sys_action.h
#ifndef __SPECIAL_COMBO_H__
#define __SPECIAL_COMBO_H__

#include <stdint.h>

extern uint8_t combinations_flag;

// 特殊功能（通过 OSAL 事件触发）
uint8_t Set_FN_Lock(uint16_t* add_keys);
uint8_t Backlight_Color(uint16_t* add_keys);
uint8_t Backlight_Light(uint16_t* add_keys);
uint8_t Touch_Locked(uint16_t* add_keys);
uint8_t Factory_Reset(uint16_t* add_keys);
uint8_t Bat_Check(uint16_t* add_keys);

// 配对功能
uint8_t Pair_button(uint16_t* add_keys);


//小地球共用部分
uint8_t Earth_LONG_PRESS_START_UP_Handler(uint16_t* add_keys);
uint8_t Earth_PRESS_UP_Handler(uint16_t* add_keys);

//x键
uint8_t X_Special_Combo(uint16_t* add_keys);
//z键
uint8_t Z_Special_Combo(uint16_t* add_keys);
//c键
uint8_t C_Special_Combo(uint16_t* add_keys);
//page up
uint8_t Page_Up(uint16_t* add_keys);
//page down
uint8_t Page_Down(uint16_t* add_keys);
#endif
