// middleware/keyboard/combo/kb_fn_action.h
#ifndef __FN_COMBO_H__
#define __FN_COMBO_H__

#include <stdint.h>

// 前向声明 - 必须与 linkedlist.h 中的 typedef 匹配
typedef struct list list_t;

/*============================================================================
 * 亮度控制 (Backlight Control)
 *============================================================================*/

/**
 * @brief 降低背光亮度
 * @param add_keys 输出键码数组（Fn 模式下发送 F1）
 * @return 添加的键码数量
 */
uint8_t Bringhness_Down(uint16_t* add_keys);

/**
 * @brief 增加背光亮度
 * @param add_keys 输出键码数组（Fn 模式下发送 F2）
 * @return 添加的键码数量
 */
uint8_t Bringhness_Up(uint16_t* add_keys);

/*============================================================================
 * 音量控制 (Volume Control)
 *============================================================================*/

uint8_t Volume_Down(uint16_t* add_keys);
uint8_t Volume_Up(uint16_t* add_keys);
uint8_t Volume_Mute(uint16_t* add_keys);

/*============================================================================
 * 媒体控制 (Media Control)
 *============================================================================*/

uint8_t Media_Play_Pause(uint16_t* add_keys);
uint8_t Media_Next(uint16_t* add_keys);
uint8_t Media_Previous(uint16_t* add_keys);

/*============================================================================
 * 系统功能 (System Functions)
 *============================================================================*/

uint8_t Multi_Task(uint16_t* add_keys);
uint8_t Search(uint16_t* add_keys);
uint8_t Print_Screen(uint16_t* add_keys);
uint8_t Lock_Screen(uint16_t* add_keys);
uint8_t Earth(uint16_t* add_keys);

/*============================================================================
 * Fn 键处理 (Fn Key Handlers)
 *============================================================================*/


uint8_t FN_DOWN_KEY(uint16_t* add_keys);
uint8_t FN_LONG_PRESS_KEY(uint16_t* add_keys);
uint8_t FN_UP_KEY(uint16_t* add_keys);
uint8_t FN_ESC_button(uint16_t* add_keys);

/*============================================================================
 * Siri 调用 (Siri Invoke)
 *============================================================================*/

/**
 * @brief 唤起 Siri 语音助手
 *
 * 发送 Consumer 键码 M_SIRI (0x00CF)，触发设备的语音命令功能
 *
 * @param add_keys 输出键码数组
 * @return 添加的键码数量（1）
 */
uint8_t Siri_Invoke(uint16_t* add_keys);

/*============================================================================
 * Earth 状态机接口 (Earth State Machine Interface)
 *============================================================================*/

/**
 * @brief Earth 状态机后处理决策
 *
 * 在 combo_task 主循环结束后调用，根据 fn_function_fired 和按键列表
 * 决定是否维持 Earth 输出或转换为 EARTH_FN_TAKEN 状态
 *
 * @param fn_fired 是否有 Fn 功能键被触发
 * @param key_list 当前按键列表
 * @param key_list_extend 组合键扩展列表
 */
void earth_post_loop_decision(uint8_t fn_fired, list_t* key_list, list_t* key_list_extend);

/**
 * @brief 检查 Earth 键是否处于按下状态
 * @return 1 表示按下，0 表示未按下
 */
uint8_t earth_is_down(void);

/**
 * @brief 重置 Earth 状态机
 */
void earth_reset(void);

/**
 * @brief 设置 Earth 状态为已被 Fn 功能占用
 */
void earth_set_fn_taken(void);

#endif
