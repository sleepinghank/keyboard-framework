
#pragma once
/** 所有输入相关服务事件
 * 矩阵扫描
 * 矩阵数据变化
 * 触控中断
 * 电量变化
 *
 */
typedef enum {
    INPUT_KEYBOARD_SCAN_EVENT = 1 << 0,
} input_task_event_t;