// middleware/keyboard/keyboard.h
#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <stdint.h>
#include <stdbool.h>
#include "linkedlist.h"
#include "combo/process_combo.h"

// 按键列表（供 combo 模块使用）
extern list_t* _key_code_list;
extern list_t* _key_code_list_extend;

// 主函数
void keyboard_init(void);
void keyboard_task(void);

// 获取上一次的更新状态
key_update_st_t keyboard_get_last_update_state(void);

// 根据主机系统类型更新基础层
void keyboard_update_base_layer_by_system(void);

#endif
