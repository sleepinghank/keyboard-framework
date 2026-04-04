// middleware/keyboard/keyboard.h
#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <stdint.h>
#include <stdbool.h>
#include "linkedlist.h"
#include "combo/kb_combo_engine.h"

// 按键列表（供 combo 模块使用）
extern list_t* _key_code_list;
extern list_t* _key_code_list_extend;

/**
 * @brief 初始化键盘模块
 *
 * 初始化矩阵扫描、防抖、键位映射、组合键引擎和 HID 报告模块
 */
void keyboard_init(void);

/**
 * @brief 键盘主任务
 *
 * 执行矩阵扫描、防抖处理、组合键检测和 HID 报告生成
 * 应在主循环中周期性调用
 */
void keyboard_scan(void);

/**
 * @brief 获取上一次按键更新状态
 * @return 按键更新状态枚举值
 */
key_update_st_t keyboard_get_last_update_state(void);

/**
 * @brief 根据主机系统类型更新基础键位层
 *
 * 当主机系统类型改变时调用，切换到对应的键位映射
 */
void keyboard_update_base_layer_by_system(void);

/**
 * @brief 通知背光服务有按键活动发生
 *
 * 供 Middleware 层（如 kb_fn_action.c）调用，通知 Application 层
 * 有按键或触控活动，以便重置背光休眠定时器。
 * 此函数封装了 output_service_note_backlight_activity()，
 * 避免 Middleware 层直接依赖 Application 层头文件。
 */
void keyboard_note_backlight_activity(void);

#endif
