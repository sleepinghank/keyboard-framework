#include "communication_service.h"
#include <stdint.h>
#include "event_manager.h"
#include "print.h"
#include "debug.h"
#include "wireless.h"
#include "wireless_event_type.h"
#include "report_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t commu_taskID = 0;

wireless_event_t event    = {0};
/**
 * @brief 通信服务事件处理器（移植自 wireless_event_task）
 * @param task_id 任务ID
 * @param events 事件标志
 * @return 未处理的事件标志
 */
uint16_t commu_process_event(uint8_t task_id, uint16_t events) {

    // 处理无线模块初始化事件
    if (events & WL_INIT_EVT) {
        wireless_init();
        dprintf("Communication: Wireless module initialization\r\n");
        return (events ^ WL_INIT_EVT);
    }

    // 处理无线重置事件
    if (events & WL_RESET_EVT) {
        wireless_enter_reset_kb(event.params.reason);
        dprintf("Communication: Wireless reset\r\n");
        return (events ^ WL_RESET_EVT);
    }

    // 处理无线可发现事件
    if (events & WL_DISCOVERABLE_EVT) {
        wireless_enter_discoverable_kb(event.params.hostIndex);
        dprintf("Communication: Wireless discoverable\r\n");
        return (events ^ WL_DISCOVERABLE_EVT);
    }

    // 处理无线重连事件
    if (events & WL_RECONNECTING_EVT) {
        wireless_enter_reconnecting_kb(event.params.hostIndex);
        dprintf("Communication: Wireless reconnecting\r\n");
        return (events ^ WL_RECONNECTING_EVT);
    }

    // 处理无线连接成功事件
    if (events & WL_CONNECTED_EVT) {
        wireless_enter_connected_kb(event.params.hostIndex);
        dprintf("Communication: Wireless connected\r\n");
        return (events ^ WL_CONNECTED_EVT);
    }

    // 处理无线断开连接事件
    if (events & WL_DISCONNECTED_EVT) {
        wireless_enter_disconnected_kb(event.params.hostIndex, event.data);
        dprintf("Communication: Wireless disconnected\r\n");
        return (events ^ WL_DISCONNECTED_EVT);
    }

    // 处理广播结束事件（进入挂起）
    if (events & WL_ADVEND_EVT) {
        wireless_enter_sleep_kb();
        dprintf("Communication: Wireless suspended\r\n");
        return (events ^ WL_ADVEND_EVT);
    }

    // 处理蓝牙配对事件
    if (events & WL_PAIR_EVT) {
        dprintf("Communication: Bluetooth PIN code entry\r\n");
        // pairing_param_t
        wireless_pairing_ex(event.params.hostIndex, NULL);
        return (events ^ WL_PAIR_EVT);
    }

    // 处理蓝牙回连事件
    if (events & WL_RECONNECT_EVT) {
        wireless_connect_ex(event.params.hostIndex, 0);
        dprintf("Communication: Exit Bluetooth PIN code entry\r\n");
        return (events ^ WL_RECONNECT_EVT);
    }

    // 处理HID协议设置事件
    if (events & WL_HID_SET_PROTOCOL_EVT) {
        // TODO: Implement wireless_hid_set_protocol
        // wireless_hid_set_protocol(event.params.protocol);
        dprintf("Communication: HID set protocol\r\n");
        return (events ^ WL_HID_SET_PROTOCOL_EVT);
    }

    // 处理HID指示器事件
    if (events & WL_HID_INDICATOR_EVT) {
        // 注意：led_state 是 static 变量，直接更新
        dprintf("Communication: HID indicator update,%d\r\n", event.params.led);
        return (events ^ WL_HID_INDICATOR_EVT);
    }

    // 处理连接间隔事件
    if (events & WL_CONECTION_INTERVAL_EVT) {
        report_buffer_set_inverval(event.params.interval);
        dprintf("Communication: Connection interval updated\r\n");
        return (events ^ WL_CONECTION_INTERVAL_EVT);
    }

    // 处理USB连接事件
    if (events & USB_CONNECT_EVT) {
        wireless_switch_to_usb_mode();
        dprintf("Communication: USB connected\r\n");
        return (events ^ USB_CONNECT_EVT);
    }

    // 处理USB断开事件
    if (events & USB_DISCONNECT_EVT) {
        // #if (BLUETOOTH_ENABLE_FLAG == TRUE)
        // wireless_switch_to_bt_driver();
        // #endif
        dprintf("Communication: USB disconnected\r\n");
        return (events ^ USB_DISCONNECT_EVT);
    }

    return 0;
}

/**
 * @brief 通信服务初始化
 */
void commu_service_init(void) {
    // 注册任务并获取任务ID
    commu_taskID = OSAL_ProcessEventRegister(commu_process_event);

    // 启动无线模块初始化事件
    OSAL_SetEvent(commu_taskID, WL_INIT_EVT);

    // TODO: 根据配置启动相应的定时任务
    // 例如：启动广播检测、连接状态监控等
    // OSAL_StartReloadTask(commu_taskID, WL_CONNECTION_CHECK_EVT, 1000);
}

#ifdef __cplusplus
}
#endif

