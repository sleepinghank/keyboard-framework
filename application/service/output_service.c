//
// Created by 18494 on 2025/12/18.
//
#include "output_service.h"
#include <stdint.h>
#include "event_manager.h"
#include "debug.h"
#include "CH585SFR.h"
#include "report_buffer.h"
#include "transport.h"
#include "indicator.h"

#ifdef P2P4G_ENABLE_FLAG
#include "p24g_driver.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

uint8_t output_taskID = 0;

uint8_t temp = 0;
/**
 * @brief 输出服务事件处理器
 * @param task_id 任务ID
 * @param events 事件标志
 * @return 未处理的事件标志
 */
uint16_t output_process_event(uint8_t task_id, uint16_t events) {
    uint16_t unhandled = 0;

    // 处理背光灯亮度事件
    if (events & OUTPUT_BACKLIGHT_BRIGHTNESS_EVT) {
        temp ++;
        if ((temp % 50) < 25)
            PRINT("ON");
        else
            PRINT("OFF");
        // TODO: 调整背光亮度 (依赖RGB/LED Matrix驱动)
        return (events ^ OUTPUT_BACKLIGHT_BRIGHTNESS_EVT);
    }

    // 返回未处理的事件
    return 0;
}

/**
 * @brief 输出服务初始化
 */
void output_service_init(void) {
    PRINT("Output service init start\n");
    // 注册任务并获取任务ID
    output_taskID = OSAL_ProcessEventRegister(output_process_event);

    PRINT("Task registered, ID=%d\n", output_taskID);

    // TODO: 根据配置启动相应的定时任务
    // 例如：背光超时检测、指示灯状态更新等
    OSAL_StartReloadTask(output_taskID, OUTPUT_BACKLIGHT_BRIGHTNESS_EVT, 200);

    PRINT("Output service init done\n");
}

#ifdef __cplusplus
}
#endif

