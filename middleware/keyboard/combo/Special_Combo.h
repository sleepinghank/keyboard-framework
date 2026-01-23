// middleware/keyboard/combo/Special_Combo.h
#ifndef __SPECIAL_COMBO_H__
#define __SPECIAL_COMBO_H__

#include <stdint.h>

// 特殊功能（通过 OSAL 事件触发）
uint8_t Set_FN_Lock(uint16_t* add_keys);
uint8_t Backlight_Color(uint16_t* add_keys);
uint8_t Backlight_Light(uint16_t* add_keys);
uint8_t Touch_Locked(uint16_t* add_keys);
uint8_t Factory_Reset(uint16_t* add_keys);
uint8_t Bat_Check(uint16_t* add_keys);

// 配对功能
uint8_t Pair_button(uint16_t* add_keys);

#endif
