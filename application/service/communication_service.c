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
#include "lpm.h"
#include "system_service.h"
#include "storage.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t commu_taskID = 0;

wireless_event_t event    = {0};

static uint8_t commu_resolve_ble_host_index(uint8_t host_idx) {
    storage_config_t* cfg = storage_get_config_ptr();

    if ((host_idx > BLE_INDEX_IDEL) && (host_idx < BLE_INDEX_MAX)) {
        return host_idx;
    }

    if ((cfg != NULL) && (cfg->ble_idx > BLE_INDEX_IDEL) && (cfg->ble_idx < BLE_INDEX_MAX)) {
        return cfg->ble_idx;
    }

    return BLE_INDEX_1;
}


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

    // 处理无线模块初始化事件
    if (events & WL_INIT_EVT) {
        // wireless_init();  // 已在 system_init_middleware() 调用，此处无需重复
        // 选择默认传输通道
        system_select_default_transport();
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
        uint8_t host_idx = commu_resolve_ble_host_index(event.params.hostIndex);
        event.params.hostIndex = host_idx;
        wireless_state_set_pairing(host_idx);
        wireless_enter_discoverable_kb(host_idx);
        dprintf("Communication: Wireless discoverable entered\r\n");
        return (events ^ WL_DISCOVERABLE_EVT);
    }

    // 处理无线重连事件
    if (events & WL_RECONNECTING_EVT) {
        uint8_t host_idx = commu_resolve_ble_host_index(event.params.hostIndex);
        event.params.hostIndex = host_idx;
        wireless_state_set_reconnecting(host_idx);
        wireless_enter_reconnecting_kb(host_idx);
        dprintf("Communication: Wireless reconnecting\r\n");
        return (events ^ WL_RECONNECTING_EVT);
    }

    // 处理无线连接成功事件
    if (events & WL_CONNECTED_EVT) {
        wireless_state_set_connected(event.params.hostIndex);
        dprintf("Communication: Wireless connected\r\n");
        return (events ^ WL_CONNECTED_EVT);
    }

    // 处理无线断开连接事件
    if (events & WL_DISCONNECTED_EVT) {
        uint8_t host_idx = commu_resolve_ble_host_index(event.params.hostIndex);
        event.params.hostIndex = host_idx;
        wireless_state_set_disconnected(host_idx, event.data);
        dprintf("Communication: Wireless disconnected\r\n");

        if (event.evt_type == EVT_DISCOVERABLE) {
            dprintf("Communication: Resume discoverable host=%d\r\n", host_idx);
            wireless_pairing_ex(host_idx, NULL);
        } else {
            OSAL_SetEvent(commu_taskID, WL_RECONNECT_EVT); // 已绑定普通回连失败后自动重连
        }
        return (events ^ WL_DISCONNECTED_EVT);
    }

    // 处理广播结束事件（进入挂起）
    if (events & WL_ADVEND_EVT) {
        wireless_state_set_sleep();
        dprintf("Communication: Wireless suspended\r\n");
        return (events ^ WL_ADVEND_EVT);
    }

    // 处理蓝牙配对事件
    if (events & WL_PAIR_EVT) {
        uint8_t host_idx = commu_resolve_ble_host_index(0);
        event.params.hostIndex = host_idx;
        dprintf("Communication: Wireless pair request host=%d\r\n", host_idx);
        wireless_pairing_ex(host_idx, NULL);
        return (events ^ WL_PAIR_EVT);
    }

    // 处理蓝牙回连事件
    if (events & WL_RECONNECT_EVT) {
        uint8_t host_idx = commu_resolve_ble_host_index(event.params.hostIndex);
        event.params.hostIndex = host_idx;
        wireless_connect_ex(host_idx, 0);
        dprintf("Communication: Wireless reconnect\r\n");
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
#ifdef USB_ENABLE_FLAG
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
#endif
    /*========================================
     * LPM prepare/resume 事件处理
     *========================================*/

    // 处理 LPM prepare 事件
    if (events & COMMU_LPM_PREPARE_EVT) {
        lpm_mode_t mode = lpm_get_mode();
        dprintf("Commu: LPM prepare start (mode=%d)\r\n", mode);

        if (mode == LPM_MODE_IDLE) {
            /* Idle：请求 BLE 低功耗，不断连 */
            /* WCH BLE 低功耗依赖 HAL_SLEEP=1 的 idleCB，此处主要是减少业务活动 */
            /* 停止主动发送报文（report_buffer 停发） */
            /* 注意：不调用 wireless_disconnect()，保持连接 */
            dprintf("Commu: BLE idle low-power prepare done\r\n");
        } else {
            /* Deep：断开 BLE 连接，停止广播 */
            wireless_disconnect();
            dprintf("Commu: BLE disconnected for deep sleep\r\n");
        }

        /* 标记 commu prepare 完成，通知 system_service 汇聚 */
        lpm_mark_prepare_done(LPM_PREPARE_COMMU);
        OSAL_SetEvent(system_taskID, SYSTEM_LPM_STEP_DONE_EVT);

        dprintf("Commu: LPM prepare done\r\n");
        return (events ^ COMMU_LPM_PREPARE_EVT);
    }

    // 处理 LPM resume 事件
    if (events & COMMU_LPM_RESUME_EVT) {
        dprintf("Commu: LPM resume start\r\n");
        /* Deep 唤醒后：仅恢复本地无线上下文，不自动回连 */
        /* 业务层回连由上层（如 SYSTEM_LPM_WAKE_EVT 处理或用户按键触发）决定 */
        /* 此处清理睡眠标志，恢复 report_buffer 状态 */
        dprintf("Commu: LPM resume done (reconnect decision deferred to app)\r\n");
        return (events ^ COMMU_LPM_RESUME_EVT);
    }

    return 0;
}

/**
 * @brief 通信服务初始化
 */
void commu_service_init(void) {
    // 注册任务并获取任务ID
    commu_taskID = OSAL_ProcessEventRegister(commu_process_event);

    // TODO: 根据配置启动相应的定时任务
    // 例如：启动广播检测、连接状态监控等
    // OSAL_StartReloadTask(commu_taskID, WL_CONNECTION_CHECK_EVT, 1000);
}

#ifdef __cplusplus
}
#endif
