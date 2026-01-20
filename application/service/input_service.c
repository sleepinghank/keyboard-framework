#include "input_service.h"
#include "keyboard.h"
#include <stdint.h>
#include "event_manager.h"
#include "print.h"
#include "battery.h"
#include "indicator.h"
#include "system_service.h"

/* 低电量阈值 */
#ifndef LOW_BATTERY_THRESHOLD
#define LOW_BATTERY_THRESHOLD  10  /* 10% 触发低电量警告 */
#endif

#ifndef CRITICAL_BATTERY_THRESHOLD
#define CRITICAL_BATTERY_THRESHOLD  5   /* 5% 触发关机 */
#endif

/* 电量检测周期 (ms) */
#ifndef BATTERY_DETECT_INTERVAL
#define BATTERY_DETECT_INTERVAL  30000  /* 30秒检测一次 */
#endif

#ifdef __cplusplus
extern "C" {
#endif

uint8_t input_taskID = 0;

/**
 * @brief 输入服务事件处理器
 * @param task_id 任务ID
 * @param events 事件标志
 * @return 未处理的事件标志
 */
uint16_t input_process_event(uint8_t task_id, uint16_t events) {
    uint16_t unhandled = 0;

    // 处理矩阵扫描事件
    if (events & INPUT_MATRIX_SCAN_EVT) {
        keyboard_task();
        events ^= INPUT_MATRIX_SCAN_EVT;
    }

    // // 处理矩阵数据变化事件
    // if (events & INPUT_MATRIX_DATA_CHANGED_EVT) {
    //     PRINT("Input: Matrix data changed\r\n");
    //     // TODO: 处理矩阵数据变化
    //     // - 检测按键按下/释放
    //     // - 处理组合键
    //     // - 发送事件到其他服务
    //     events ^= INPUT_MATRIX_DATA_CHANGED_EVT;
    // }

    // 处理触控中断事件
    if (events & INPUT_TOUCH_INT_EVT) {
        println("Input: Touch interrupt");
        // TODO: 触摸板驱动实现后取消注释
        // touch_process_interrupt();
        events ^= INPUT_TOUCH_INT_EVT;
    }

    // 处理电量检测事件
    if (events & INPUT_BATTERY_DETE_EVT) {
        // 读取电池电量
        uint8_t battery_level = battery_get_percentage();
        xprintf("Input: Battery level = %d%%\r\n", battery_level);

        // 更新蓝牙电池服务上报
        bt_driver_update_bat_level(battery_level);

        // 检查低电量状态
        if (battery_level <= CRITICAL_BATTERY_THRESHOLD) {
            // 电量极低，触发关机事件
            println("Input: Critical battery, triggering shutdown");
            extern uint8_t system_taskID;
            OSAL_MsgSend(system_taskID, SYSTEM_LOW_BATTERY_SHUTDOWN_EVT);
        } else if (battery_level <= LOW_BATTERY_THRESHOLD) {
            // 低电量警告，闪烁指示灯
            println("Input: Low battery warning");
            indicator_set(0, &IND_BLINK_FAST);
        }

        events ^= INPUT_BATTERY_DETE_EVT;
    }

    // 返回未处理的事件
    return events;
}

/**
 * @brief 输入服务初始化
 */
void input_service_init(void) {
    // 注册任务并获取任务ID
    input_taskID = OSAL_ProcessEventRegister(input_process_event);

    // 启动矩阵扫描循环任务
    // OSAL_StartReloadTask(input_taskID, INPUT_MATRIX_SCAN_EVT, MATRIX_SCAN_TIMER);

    // // 启动电量检测定时任务
    // OSAL_StartReloadTask(input_taskID, INPUT_BATTERY_DETE_EVT, BATTERY_DETECT_INTERVAL);
}

#ifdef __cplusplus
}
#endif

