
#pragma once
/**
 * 所有输出相关服务事件
 * 指示灯
 * 背光灯亮度
 * 背光灯颜色
 * 发送hid 键盘
 * 发送 鼠标报文
 * 发送触控板报文
 */
typedef enum {
    OUTPUT_INDICATOR_EVT            = 1 << 0,  /**< 指示灯事件 */
    OUTPUT_BACKLIGHT_BRIGHTNESS_EVT = 1 << 1,  /**< 背光灯亮度事件 */
    OUTPUT_BACKLIGHT_COLOR_EVT      = 1 << 2,  /**< 背光灯颜色事件 */
    OUTPUT_SEND_HID_KEYBOARD_EVT    = 1 << 3,  /**< 发送HID键盘事件 */
    OUTPUT_SEND_MOUSE_REPORT_EVT    = 1 << 4,  /**< 发送鼠标报文事件 */
    OUTPUT_SEND_TOUCH_REPORT_EVT    = 1 << 5,  /**< 发送触控板报文事件 */
} output_task_event_t;