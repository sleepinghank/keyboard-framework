
#pragma once
/**
 * 所有输出相关服务事件
 * 指示灯
 * 背光灯亮度
 * 背光灯颜色
 * 发送hid 键盘
 * 发送 鼠标报文
 * 发送触控板报文
 * LPM prepare/resume
 */
typedef enum {
    OUTPUT_INDICATOR_EVT            = 1 << 0,  /**< 指示灯事件 */
    OUTPUT_BACKLIGHT_BRIGHTNESS_EVT = 1 << 1,  /**< 背光灯亮度事件 */
    OUTPUT_BACKLIGHT_COLOR_EVT      = 1 << 2,  /**< 背光灯颜色事件 */
    OUTPUT_SEND_HID_KEYBOARD_EVT    = 1 << 3,  /**< 发送HID键盘事件 */
    OUTPUT_SEND_MOUSE_REPORT_EVT    = 1 << 4,  /**< 发送鼠标报文事件 */
    OUTPUT_SEND_TOUCH_REPORT_EVT    = 1 << 5,  /**< 发送触控板报文事件 */
    /* LPM 调度事件（bits 6-7） */
    OUTPUT_LPM_PREPARE_EVT          = 1 << 6,  /**< LPM prepare（仅 Deep 时关灯，Idle 为 no-op） */
    OUTPUT_LPM_RESUME_EVT           = 1 << 7,  /**< LPM resume（Deep 唤醒后按系统状态恢复灯效） */
} output_task_event_t;

#include <stdint.h>

uint16_t output_process_event(uint8_t task_id, uint16_t events);
void output_service_init(void);

/* task ID 外部声明 */
extern uint8_t output_taskID;
