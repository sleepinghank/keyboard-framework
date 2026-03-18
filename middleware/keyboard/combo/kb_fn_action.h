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
uint8_t FN_UP_KEY(uint16_t* add_keys);
uint8_t FN_ESC_button(uint16_t* add_keys);

/*============================================================================
 * 背光控制 (Backlight Control)
 *============================================================================*/

/**
 * @brief 切换背光亮度档位
 *
 * Fn+右Shift: 在 OFF→LOW→MEDIUM→HIGH→OFF 之间循环
 * 同时通知背光服务有活动发生，重置 5 秒休眠定时器
 *
 * @param add_keys 未使用
 * @return 0（不发送额外键码）
 */
uint8_t Backlight_Level_Up(uint16_t* add_keys);

/**
 * @brief 切换背光颜色
 *
 * Fn+右Enter: 在 13 色之间循环（红→暗红→...→白→红）
 * 仅当背光开启时才切换颜色
 * 同时通知背光服务有活动发生，重置 5 秒休眠定时器
 *
 * @param add_keys 未使用
 * @return 0（不发送额外键码）
 */
uint8_t Backlight_Color_Next(uint16_t* add_keys);

/*============================================================================
 * 电量检测 (Battery Check)
 *============================================================================*/

/**
 * @brief 电量检查（Fn+右Cmd）
 *
 * 根据当前电量百分比控制红灯闪烁次数：
 * - 75-100%: 闪 4 次
 * - 50-74%: 闪 3 次
 * - 25-49%: 闪 2 次
 * - 0-24%: 闪 1 次
 *
 * @param add_keys 未使用
 * @return 0（不发送额外键码）
 */
uint8_t Battery_Check(uint16_t* add_keys);

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
 * 决定是否发送 M_EARTH 或转换为 EARTH_FN_TAKEN 状态
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
