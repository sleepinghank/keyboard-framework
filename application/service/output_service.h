
#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
 * @file output_service.h
 * @brief 输出服务模块
 *
 * 管理所有输出相关功能：
 * - 指示灯（蓝牙状态、大写锁定、电量状态）
 * - 背光灯（亮度、颜色、5 秒休眠、低电量联动）
 * - HID 报文发送
 * - LPM 低功耗管理
 */

/**
 * @brief 输出服务事件枚举
 */
typedef enum {
    OUTPUT_INDICATOR_EVT            = 1 << 0,  /**< 指示灯事件 */
    OUTPUT_BACKLIGHT_BRIGHTNESS_EVT = 1 << 1,  /**< 背光灯亮度事件 */
    OUTPUT_BACKLIGHT_COLOR_EVT      = 1 << 2,  /**< 背光灯颜色事件 */
    OUTPUT_SEND_HID_KEYBOARD_EVT    = 1 << 3,  /**< 发送HID键盘事件 */
    OUTPUT_SEND_MOUSE_REPORT_EVT    = 1 << 4,  /**< 发送鼠标报文事件 */
    OUTPUT_SEND_TOUCH_REPORT_EVT    = 1 << 5,  /**< 发送触控板报文事件 */
    OUTPUT_LPM_PREPARE_EVT          = 1 << 6,  /**< LPM prepare（Deep 时关灯，Idle 无操作） */
    OUTPUT_LPM_RESUME_EVT           = 1 << 7,  /**< LPM resume（Deep 唤醒后恢复灯效） */
    OUTPUT_BACKLIGHT_SLEEP_EVT      = 1 << 8,  /**< 背光 5 秒无操作自动休眠事件 */
    OUTPUT_BATTERY_CHECK_EVT        = 1 << 9,  /**< 电池电量检查事件 */
} output_task_event_t;

/**
 * @brief 指示灯请求类型
 *
 * 定义各种需要指示灯反馈的场景
 */
typedef enum {
    IND_REQ_NONE = 0,              /**< 无请求 */
    IND_REQ_BT_PAIRING,            /**< 蓝牙配对中 */
    IND_REQ_BT_RECONNECTING,       /**< 蓝牙回连中 */
    IND_REQ_BT_CONNECTED,          /**< 蓝牙已连接 */
    IND_REQ_BT_DISCONNECTED,       /**< 蓝牙已断连 */
    IND_REQ_CAPS_ON,               /**< 大写锁定开启 */
    IND_REQ_CAPS_OFF,              /**< 大写锁定关闭 */
    IND_REQ_CAPS_DISCONNECTED,     /**< 大写锁定断连状态 */
    IND_REQ_LOW_BATTERY,           /**< 低电量警告（≤20%） */
    IND_REQ_BATTERY_NORMAL,        /**< 电量恢复正常 */
} ind_req_type_t;

/*============================================================================
 * 事件处理与初始化
 *============================================================================*/

/**
 * @brief 输出服务事件处理器
 * @param task_id 任务ID
 * @param events 事件标志
 * @return 未处理的事件标志
 */
uint16_t output_process_event(uint8_t task_id, uint16_t events);

/**
 * @brief 初始化输出服务
 *
 * 初始化指示灯、背光灯、注册 OSAL 任务
 */
void output_service_init(void);

/*============================================================================
 * 指示灯控制
 *============================================================================*/

/**
 * @brief 请求指示灯效果
 *
 * 根据请求类型设置对应 LED 的灯效。
 * 对于低电量相关请求，会联动控制背光。
 *
 * @param type 指示灯请求类型
 * @param param 预留参数，当前未使用
 */
void output_service_request_indicator(ind_req_type_t type, uint8_t param);

/*============================================================================
 * 背光控制
 *============================================================================*/

/**
 * @brief 通知背光有活动发生
 *
 * 当有按键或触控活动时调用：
 * 1. 如果背光处于休眠状态，则唤醒
 * 2. 重置 5 秒休眠定时器
 *
 * @note 在低电量模式或 LPM 禁用状态下不执行任何操作
 */
void output_service_note_backlight_activity(void);

/**
 * @brief 设置低电量模式
 *
 * 控制背光的低电量联动行为：
 * - enable=true: 记忆当前背光状态，关闭背光，取消休眠定时器
 * - enable=false: 恢复之前的背光状态，重启活动检测
 *
 * @param enable true=进入低电量模式，false=退出
 */
void output_service_set_backlight_low_battery(bool enable);

/*============================================================================
 * 外部变量
 *============================================================================*/

/** 输出服务任务 ID */
extern uint8_t output_taskID;
