// middleware/keyboard/combo/FN_Combo.h
#ifndef __FN_COMBO_H__
#define __FN_COMBO_H__

#include <stdint.h>

// 亮度控制
uint8_t Bringhness_Down(uint16_t* add_keys);
uint8_t Bringhness_Up(uint16_t* add_keys);

// 音量控制
uint8_t Volume_Down(uint16_t* add_keys);
uint8_t Volume_Up(uint16_t* add_keys);
uint8_t Volume_Mute(uint16_t* add_keys);

// 媒体控制
uint8_t Media_Play_Pause(uint16_t* add_keys);
uint8_t Media_Next(uint16_t* add_keys);
uint8_t Media_Previous(uint16_t* add_keys);

// 系统功能
uint8_t Multi_Task(uint16_t* add_keys);
uint8_t Search(uint16_t* add_keys);
uint8_t Print_Screen(uint16_t* add_keys);
uint8_t Lock_Screen(uint16_t* add_keys);
uint8_t Earth(uint16_t* add_keys);

// Fn 键处理
uint8_t FN_DOWN_KEY(uint16_t* add_keys);
uint8_t FN_UP_KEY(uint16_t* add_keys);
uint8_t FN_ESC_button(uint16_t* add_keys);

#endif
