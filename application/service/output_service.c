//
// Created by 18494 on 2025/12/18.
//
#include "output_service.h"
#include <stdint.h>
#include "event_manager.h"
#include "print.h"
#include "debug.h"
#include "report_buffer.h"
#include "transport.h"
#include "indicator.h"
#include "gpio.h"

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

    // 处理指示灯事件
    if (events & OUTPUT_INDICATOR_EVT) {
        dprintf("Output: Indicator LED update\r\n");
        // 指示灯任务由indicator模块自身维护
        indicator_task();
        events ^= OUTPUT_INDICATOR_EVT;
    }

    // 处理背光灯亮度事件
    if (events & OUTPUT_BACKLIGHT_BRIGHTNESS_EVT) {
        temp ++;
        if (temp % 2 == 0)
            setPinInputHigh(B13);
        else
            setPinInputLow(B13);
        dprintf("Output: Backlight brightness changed\r\n");
        // TODO: 调整背光亮度 (依赖RGB/LED Matrix驱动)
        events ^= OUTPUT_BACKLIGHT_BRIGHTNESS_EVT;
    }

    // 处理背光灯颜色事件
    if (events & OUTPUT_BACKLIGHT_COLOR_EVT) {
        dprintf("Output: Backlight color changed\r\n");
        // TODO: 调整背光颜色 (依赖RGB Matrix驱动)
        events ^= OUTPUT_BACKLIGHT_COLOR_EVT;
    }

    // 处理发送HID键盘事件
    if (events & OUTPUT_SEND_HID_KEYBOARD_EVT) {
        dprintf("Output: Send HID keyboard report\r\n");
        report_buffer_t report;
        // 从报告缓冲区获取待发送的报告
        if (report_buffer_dequeue(&report)) {
            transport_t current = get_transport();
            uint8_t result = 0;

            switch (report.type) {
                case REPORT_TYPE_KB:
                    

                    break;

                case REPORT_TYPE_CONSUMER:
                    
                    break;

                default:
                    break;
            }

            // 如果发送失败且缓冲区非空，重新调度发送事件
            if (result != 0 && !report_buffer_is_empty()) {
                OSAL_MsgSend(output_taskID, OUTPUT_SEND_HID_KEYBOARD_EVT);
            }
        }
        events ^= OUTPUT_SEND_HID_KEYBOARD_EVT;
    }

    // 处理发送鼠标报文事件
    if (events & OUTPUT_SEND_MOUSE_REPORT_EVT) {
        dprintf("Output: Send mouse report\r\n");
        // TODO: 实现鼠标报告发送
        events ^= OUTPUT_SEND_MOUSE_REPORT_EVT;
    }

    // 处理发送触摸板报文事件
    if (events & OUTPUT_SEND_TOUCH_REPORT_EVT) {
        dprintf("Output: Send touch report\r\n");
        // TODO: 实现触摸板报告发送
        events ^= OUTPUT_SEND_TOUCH_REPORT_EVT;
    }

    // 返回未处理的事件
    return events;
}

/**
 * @brief 输出服务初始化
 */
void output_service_init(void) {
    // 注册任务并获取任务ID
    output_taskID = OSAL_ProcessEventRegister(output_process_event);

    // TODO: 根据配置启动相应的定时任务
    // 例如：背光超时检测、指示灯状态更新等
    OSAL_StartReloadTask(output_taskID, OUTPUT_BACKLIGHT_BRIGHTNESS_EVT, 300);
    setPinInputLow(B13);
}

#ifdef __cplusplus
}
#endif

