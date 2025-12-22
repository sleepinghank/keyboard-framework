#include "input_service.h"
#include "keyboard.h"
#include <stdint.h>
#include "event_manager.h"
#include "print.h"

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
        // PRINT("Input: Touch interrupt\r\n");
        // TODO: 处理触控中断
        // - 读取触摸坐标
        // - 处理手势
        // - 转换为按键事件
        // touch_process_interrupt();
        events ^= INPUT_TOUCH_INT_EVT;
    }

    // 处理电量变化事件
    if (events & INPUT_BATTERY_DETE_EVT) {
        // PRINT("Input: Battery level changed\r\n");
        // TODO: 处理电量变化
        // - 读取电池电压
        // - 计算电量百分比
        // - 通知系统服务低电量
        // battery_update_level();
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
    OSAL_StartReloadTask(input_taskID, INPUT_MATRIX_SCAN_EVT, MATRIX_SCAN_TIMER);

    // TODO: 根据配置启动其他定时任务
    // 例如：电量检测、触摸板扫描等
    // OSAL_StartReloadTask(input_taskID, INPUT_BATTERY_CHANGE_EVT, 5000);
}

#ifdef __cplusplus
}
#endif

