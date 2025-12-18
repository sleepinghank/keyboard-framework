#pragma once
/**
 * 所有系统相关服务事件
 * 低电关机
 * idle
 * 关机
 * 深度睡眠
 * 存储
 * 唤醒
 * 恢复出厂设置
 * ota
 */
typedef enum {
    INPUT_KEYBOARD_SCAN_EVENT = 1 << 0,
} system_task_event_t;