
#pragma once
/** 所有输入相关服务事件
 * 矩阵扫描
 * 矩阵数据变化
 * 触控中断
 * 电量变化

 */
typedef enum {
    INPUT_MATRIX_SCAN_EVT          = 1 << 0,  /**< 矩阵扫描事件 */
    INPUT_MATRIX_CHANGED_EVT  = 1 << 1,  /**< 矩阵数据变化事件 */
    INPUT_TOUCH_INT_EVT      = 1 << 2,  /**< 触控中断事件 */
    INPUT_BATTERY_DETE_EVT       = 1 << 3,  /**< 电量变化事件 */
} input_task_event_t;