#include "communication_service.h"
#include <stdint.h>
#include "event_manager.h"
#include "print.h"
#include "debug.h"
#include "wireless.h"
#include "wireless_event_type.h"
#include "report_buffer.h"
#include "wireless.h"
#include "keyboard.h"
#include "transport.h"
#include "system_service.h"
#include "storage.h"
#include "output_service.h"
#include "PMU.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t commu_taskID = 0;

wireless_event_t event    = {0};

/**
 * @brief 选择默认传输通道
 *
 * 根据编译配置选择默认的传输通道，优先使用蓝牙通道。
 * set_transport() 内部会调用相应的驱动切换函数来初始化无线传输函数表。
 */
void system_select_default_transport(void) {
    transport_t default_transport = TRANSPORT_NONE;

    // 默认使用蓝牙通道
#ifdef BLUETOOTH_ENABLE_FLAG
    default_transport = TRANSPORT_BLUETOOTH;
#elif defined(P2P4G_ENABLE_FLAG)
    default_transport = TRANSPORT_P2P4;
#endif

    if (default_transport != TRANSPORT_NONE) {
        set_transport(default_transport);
    }
}
/**
 * @brief 通信服务事件处理器（移植自 wireless_event_task）
 * @param task_id 任务ID
 * @param events 事件标志
 * @return 未处理的事件标志
 */
uint16_t commu_process_event(uint8_t task_id, uint16_t events) {
    (void)task_id;

    /* ---- 无线状态通知 ---- */

    // 无线模块初始化完成
    if (events & WL_STS_INIT) {
        dprintf("[COMMU_STS] WL_STS_INIT state_before=%d evt_type=%d task=%d\r\n",
                wireless_get_state(), event.evt_type, commu_taskID);
        // wireless_init();  // 已在 system_init_middleware() 调用，此处无需重复
        // 选择默认传输通道
        system_select_default_transport();
        dprintf("Communication: Wireless module initialization\r\n");
        return (events ^ WL_STS_INIT);
    }

    // 无线模块重置
    if (events & WL_STS_RESET) {
        dprintf("Communication: Wireless reset\r\n");
        return (events ^ WL_STS_RESET);
    }

    // 已进入可发现/配对广播
    if (events & WL_STS_DISCOVERABLE) {
        dprintf("[COMMU_STS] WL_STS_DISCOVERABLE host=%d state_before=%d evt_type=%d\r\n",
                event.params.hostIndex, wireless_get_state(), event.evt_type);
        wireless_state_set_pairing(event.params.hostIndex);
        output_service_request_indicator(IND_REQ_BT_PAIRING, 0);
        PMU_Update();
        dprintf("Communication: Wireless discoverable entered\r\n");
        return (events ^ WL_STS_DISCOVERABLE);
    }

    // 已进入回连广播
    if (events & WL_STS_RECONNECTING) {
        dprintf("[COMMU_STS] WL_STS_RECONNECTING host=%d state_before=%d evt_type=%d\r\n",
                event.params.hostIndex, wireless_get_state(), event.evt_type);
        wireless_state_set_reconnecting(event.params.hostIndex);
        output_service_request_indicator(IND_REQ_BT_RECONNECTING, 0);
        PMU_Update();
        dprintf("Communication: Wireless reconnecting\r\n");
        return (events ^ WL_STS_RECONNECTING);
    }

    // 无线连接成功
    if (events & WL_STS_CONNECTED) {
        dprintf("[COMMU_STS] WL_STS_CONNECTED host=%d state_before=%d evt_type=%d\r\n",
                event.params.hostIndex, wireless_get_state(), event.evt_type);
        wireless_state_set_connected(event.params.hostIndex);
        output_service_request_indicator(IND_REQ_BT_CONNECTED, 0);
        /* 连接建立后立即刷新 PMU 计时策略，确保无按键场景也能进入 Idle/Deep 计时 */
        PMU_Update();
        dprintf("Communication: Wireless connected\r\n");
        return (events ^ WL_STS_CONNECTED);
    }

    // 无线断开连接
    if (events & WL_STS_DISCONNECTED) {
        uint8_t host_idx = BLE_INDEX_1;
        event.params.hostIndex = host_idx;
        dprintf("[COMMU_STS] WL_STS_DISCONNECTED host=%d reason=%d state_before=%d evt_type=%d\r\n",
                host_idx, event.data, wireless_get_state(), event.evt_type);
        wireless_state_set_disconnected(host_idx, event.data);
        dprintf("Communication: Wireless disconnected\r\n");

        // if (event.evt_type == EVT_DISCOVERABLE) {
        //     dprintf("Communication: Resume discoverable host=%d\r\n", host_idx);
        //     wireless_pairing_ex(host_idx, NULL);
        // } else {
            OSAL_SetEvent(commu_taskID, WL_REQ_RECONNECT); // 已绑定普通回连失败后自动重连
        // }
        return (events ^ WL_STS_DISCONNECTED);
    }

    // 广播结束（进入挂起）
    if (events & WL_STS_ADV_END) {
        wireless_state_set_sleep();
        dprintf("Communication: Wireless suspended\r\n");
        return (events ^ WL_STS_ADV_END);
    }

    /* ---- 报告缓冲发送 ---- */
    if (events & REPORT_SEND_EVT) {
        if (report_buffer_task()){
            // 已发送但未完成（等待ACK或重试中）的报文，继续调度删除事件以完成生命周期
            OSAL_SetDelayedEvent(commu_taskID, REPORT_DELETE_PAK_EVT,4);
        }
        /* 有在等待，可以重新发送的报告就继续调度发送事件 */
        if (report_buffer_waiting_send()) {
            OSAL_SetDelayedEvent(commu_taskID, REPORT_SEND_EVT, 4);
        }
        return (events ^ REPORT_SEND_EVT);
    }
    /* ---- 删除已发送报告 ---- */
    if (events & REPORT_DELETE_PAK_EVT) {
        if (report_buffer_delete_pending() > 0){
            // 仍有待删除的报文，继续调度删除事件以完成生命周期
            OSAL_SetDelayedEvent(commu_taskID, REPORT_DELETE_PAK_EVT,4);
        }
        return (events ^ REPORT_DELETE_PAK_EVT);
    }
    /* ---- 清空报告缓冲 ---- */
    if (events & REPORT_CLEAR_EVT) {
        report_buffer_init();
        return (events ^ REPORT_CLEAR_EVT);
    }

    /* ---- 无线触发请求 ---- */
    // 发起配对
    if (events & WL_REQ_PAIR) {
        uint8_t host_idx = BLE_INDEX_1;
        event.params.hostIndex = BLE_INDEX_1;
        dprintf("[COMMU_REQ] WL_REQ_PAIR host=%d\r\n", host_idx);
        wireless_pairing_ex(host_idx, NULL);
        return (events ^ WL_REQ_PAIR);
    }

    // 发起回连
    if (events & WL_REQ_RECONNECT) {
        uint8_t host_idx = BLE_INDEX_1;
        event.params.hostIndex = host_idx;
        dprintf("[COMMU_REQ] WL_REQ_RECONNECT host=%d state_before=%d\r\n",
                host_idx, wireless_get_state());
        wireless_connect_ex(host_idx, 0);
        dprintf("Communication: Wireless reconnect\r\n");
        return (events ^ WL_REQ_RECONNECT);
    }

    // 发起断开
    if (events & WL_REQ_DISCONNECT) {
        wireless_disconnect();
        dprintf("Communication: Wireless disconnect\r\n");
        return (events ^ WL_REQ_DISCONNECT);
    }
    /* ---- USB 状态通知 ---- */

#ifdef USB_ENABLE_FLAG
    // USB 连接
    if (events & USB_STS_CONNECT) {

        wireless_switch_to_usb_mode();

        dprintf("Communication: USB connected\r\n");
        return (events ^ USB_STS_CONNECT);
    }

    // USB 断开
    if (events & USB_STS_DISCONNECT) {
        // #if (BLUETOOTH_ENABLE_FLAG == TRUE)
        // wireless_switch_to_bt_driver();
        // #endif
        dprintf("Communication: USB disconnected\r\n");
        return (events ^ USB_STS_DISCONNECT);
    }
#endif
    return 0;
}
/**
 * @brief 通信服务初始化
 */
void commu_service_init(void) {
    // 注册任务并获取任务ID
    commu_taskID = OSAL_ProcessEventRegister(commu_process_event);
    dprintf("Communication: Service initialized with task ID %d\r\n", commu_taskID);
}

#ifdef __cplusplus
}
#endif
