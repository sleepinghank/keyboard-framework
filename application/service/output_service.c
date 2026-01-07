//
// Created by 18494 on 2025/12/18.
//
#include "output_service.h"
#include <stdint.h>
#include "event_manager.h"
#include "print.h"
#include "debug.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t output_taskID = 0;

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
        // TODO: 更新指示灯状态
        // - 蓝牙连接状态指示
        // - 配对模式指示
        // - 低电量指示
        // indicator_update_led();
        events ^= OUTPUT_INDICATOR_EVT;
    }

    // 处理背光灯亮度事件
    if (events & OUTPUT_BACKLIGHT_BRIGHTNESS_EVT) {
        dprintf("Output: Backlight brightness changed\r\n");
        // TODO: 调整背光亮度
        // - 更新PWM占空比
        // - 保存亮度设置到EEPROM
        // backlight_set_brightness();
        events ^= OUTPUT_BACKLIGHT_BRIGHTNESS_EVT;
    }

    // 处理背光灯颜色事件
    if (events & OUTPUT_BACKLIGHT_COLOR_EVT) {
        dprintf("Output: Backlight color changed\r\n");
        // TODO: 调整背光颜色
        // - 更新RGB LED颜色
        // - 应用颜色配置文件
        // backlight_set_color();
        events ^= OUTPUT_BACKLIGHT_COLOR_EVT;
    }

    // 处理发送HID键盘事件
    if (events & OUTPUT_SEND_HID_KEYBOARD_EVT) {
        dprintf("Output: Send HID keyboard report\r\n");
        // TODO: 发送键盘HID报告
        // - 构建键盘报告数据
        // - 通过USB或蓝牙发送
        // - 发送修饰键和普通键状态
        // hid_keyboard_send();
        events ^= OUTPUT_SEND_HID_KEYBOARD_EVT;
    }

    // 处理发送鼠标报文事件
    if (events & OUTPUT_SEND_MOUSE_REPORT_EVT) {
        dprintf("Output: Send mouse report\r\n");
        // TODO: 发送鼠标HID报告
        // - 构建鼠标报告数据
        // - 发送移动、点击、滚轮信息
        // hid_mouse_send();
        events ^= OUTPUT_SEND_MOUSE_REPORT_EVT;
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
    // OSAL_StartReloadTask(output_taskID, OUTPUT_BACKLIGHT_TIMEOUT_EVT, 60000);
}

#ifdef __cplusplus
}
#endif

