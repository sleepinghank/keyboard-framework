
#pragma once
/**
 * 所有输出相关服务事件
 * 指示灯
 * 背光灯亮度
 * 背光灯颜色
 * 发送hid 键盘
 * 发送 鼠标报文
 *
 */
typedef enum {
    INPUT_KEYBOARD_SCAN_EVENT = 1 << 0,
} output_task_event_t;