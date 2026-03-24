
#include "hidkbd.h"
#include "CONFIG.h"
#include "_bt_driver.h"
#include "hiddev.h"
#include "hidkbdservice.h"
#include "battservice.h"
#include "CH58x_common.h"
#include "HAL.h"
#include "debug.h"
#include "event_manager.h"

static bool g_bt_driver_initialized = false;

/* 报告数据长度：与 HID 描述符保持一致 */
#define BT_DRV_KEYBOARD_RPT_LEN    8    /* modifier(1) + reserved(1) + keycodes(6) */
#define BT_DRV_MOUSE_RPT_LEN       5    /* buttons(1) + X(1) + Y(1) + wheel(1) + pan(1) */

static bool bt_driver_is_host_bonded(uint8_t host_idx)
{
    return (hidEmu_is_ble_bonded((access_ble_idx_t)host_idx) != 0u);
}





/*********************************************************************
 * @fn      bt_driver_setup
 *
 * @brief   Setup function called during system initialization
 *
 * @return  none
 */
void bt_driver_setup(void)
{
    dprintf("[BT] setup\n");
}

/*********************************************************************
 * @fn      bt_driver_init
 *
 * @brief   Initialization function for the Bluetooth driver
 *
 * @param   wakeup_from_low_power - TRUE if waking from low power mode
 *
 * @return  none
 */
void bt_driver_init(bool wakeup_from_low_power)
{
    dprintf("[BT] init wakeup=%d\n", wakeup_from_low_power);

    if (g_bt_driver_initialized) {
        dprintf("[BT] init skip, already initialized\n");
        return;
    }
    CH58x_BLEInit();
    DelayUs(1);
    HAL_Init();
    GAPRole_PeripheralInit();
    HidDev_Init();
    HidEmu_Init();

    g_bt_driver_initialized = true;
    dprintf("[BT] init done\n");
}

/*********************************************************************
 * @fn      bt_driver_connect_ex
 *
 * @brief   Initiate connection request to paired host
 *
 * @param   host_idx - host index
 * @param   timeout - timeout value
 *
 * @return  none
 */
void bt_driver_connect_ex(uint8_t host_idx, uint16_t timeout)
{
    uint8_t ble_state = 0;
    uint8_t target_host_idx = host_idx;
    (void)timeout;

    dprintf("bt_driver_connect_ex: host_idx=%d\n", host_idx);

    /* 验证并修正主机索引 */
    if ((target_host_idx <= BLE_INDEX_IDEL) || (target_host_idx >= BLE_INDEX_MAX)) {
        storage_config_t *cfg = storage_get_config_ptr();
        if ((cfg != NULL) && (cfg->ble_idx > BLE_INDEX_IDEL) && (cfg->ble_idx < BLE_INDEX_MAX)) {
            target_host_idx = cfg->ble_idx;
        } else {
            target_host_idx = BLE_INDEX_1;
        }
    }

    access_state.ble_idx = target_host_idx;
    access_state.pairing_state = FALSE;

    GAPRole_GetParameter(GAPROLE_STATE, &ble_state);

    dprintf("[BT] connect_ex: target=%d con_work_mode=%d ble_state=%d\n",
            target_host_idx, con_work_mode, ble_state);

    if (con_work_mode == target_host_idx) {
        /* 目标主机与当前一致：已连接或正在处理断开流程则不操作 */
        if ((ble_state == GAPROLE_CONNECTED) ||
            OSAL_GetTaskTimer(hidEmuTaskId, SEND_DISCONNECT_EVT)) {
            dprintf("[BT] connect_ex: already connected or disconnecting, skip\n");
            return;
        }
        /* 未连接且未在广播，已绑定则发起广播 */
        if ((ble_state != GAPROLE_ADVERTISING) && hidEmu_is_ble_bonded(target_host_idx)) {
            dprintf("[BT] connect_ex: same host, start adv (bonded)\n");
            hidEmu_prepare_reconnect_adv();
            hidEmu_adv_enable(ENABLE);
        } else {
            dprintf("[BT] connect_ex: same host, adv already active or not bonded, skip\n");
        }
    } else {
        /* 切换到不同主机：先取消可能挂起的断开定时器 */
        if (OSAL_GetTaskTimer(hidEmuTaskId, SEND_DISCONNECT_EVT)) {
            dprintf("[BT] connect_ex: cancel pending SEND_DISCONNECT_EVT\n");
            OSAL_StopTask(hidEmuTaskId, SEND_DISCONNECT_EVT);
        }

        if (ble_state == GAPROLE_CONNECTED) {
            dprintf("[BT] connect_ex: switch host, disconnect first\n");
            hidEmu_disconnect();
        } else if (ble_state == GAPROLE_ADVERTISING) {
            dprintf("[BT] connect_ex: switch host, stop adv first\n");
            hidEmu_adv_enable(DISABLE);
        } else if (hidEmu_is_ble_bonded(target_host_idx)) {
            dprintf("[BT] connect_ex: switch host, start adv (bonded)\n");
            hidEmu_prepare_reconnect_adv();
            hidEmu_adv_enable(ENABLE);
        } else {
            dprintf("[BT] connect_ex: switch host, not bonded, wait pairing cmd\n");
            con_work_mode = target_host_idx;
        }
    }
}

/*********************************************************************
 * @fn      bt_driver_pairing_ex
 *
 * @brief   Enter pairing mode
 *
 * @param   host_idx - host index
 * @param   param - pairing parameters
 *
 * @return  none
 */
void bt_driver_pairing_ex(uint8_t host_idx, void *param)
{
    uint8_t ble_state;
    bool    bonded;
    (void)param;

    dprintf("bt_driver_pairing_ex: host_idx=%d\n", host_idx);

    /* 验证并修正主机索引 */
    if ((host_idx <= BLE_INDEX_IDEL) || (host_idx >= BLE_INDEX_MAX)) {
        host_idx = BLE_INDEX_1;
    }

    bonded = bt_driver_is_host_bonded(host_idx);

    /* pairing_state 仅表示“已绑定设备重配时是否切换身份地址” */
    access_state.ble_idx = host_idx;
    access_state.pairing_state = bonded ? TRUE : FALSE;

    GAPRole_GetParameter(GAPROLE_STATE, &ble_state);

    dprintf("[BT] pairing_ex: ble_state=%d ble_idx=%d bonded=%d repair=%d\n",
            ble_state, access_state.ble_idx, bonded, access_state.pairing_state);

    if (ble_state == GAPROLE_CONNECTED) {
        /* 先断开，hidEmuStateCB 在 GAP_LINK_TERMINATED_EVENT 中会根据
         * access_state.ble_idx 和 access_state.pairing_state 发起配对广播 */
        dprintf("[BT] pairing_ex: connected, disconnect first then adv\n");
        hidEmu_disconnect();
        return;
    }

    if (ble_state == GAPROLE_ADVERTISING) {
        dprintf("[BT] pairing_ex: advertising, stop adv then restart\n");
        hidEmu_adv_enable(DISABLE);
    } else {
        dprintf("[BT] pairing_ex: idle, start pairing adv\n");
        hidEmu_adv_enable(ENABLE);
    }
}

/*********************************************************************
 * @fn      bt_driver_disconnect
 *
 * @brief   Force disconnection
 *
 * @return  none
 */
void bt_driver_disconnect(void)
{
    dprintf("[BT] disconnect\n");
    hidEmu_disconnect();
}

/*********************************************************************
 * @fn      bt_driver_clear_bonding
 *
 * @brief   Clear BLE bonding records
 *
 * @return  none
 */
void bt_driver_clear_bonding(void)
{
    dprintf("[BT] clear_bonding\n");
    hidEmu_delete_ble_bonded();
}

/*********************************************************************
 * @fn      bt_driver_send_keyboard
 *
 * @brief   Send keyboard report
 *
 * @param   report - keyboard report data
 *
 * @return  uint8_t - send status (0: success, non-zero: failure)
 */
uint8_t bt_driver_send_keyboard(uint8_t *report)
{
    uint8_t ret = HidDev_Report(HID_RPT_ID_CLASS_KEY_IN, HID_REPORT_TYPE_INPUT,
                                BT_DRV_KEYBOARD_RPT_LEN, report);
    if (ret != 0) {
        dprintf("[BT] send_keyboard fail ret=%d\n", ret);
    }
    return ret;
}

/*********************************************************************
 * @fn      bt_driver_send_nkro
 *
 * @brief   Send NKRO keyboard report
 *
 * @param   report - NKRO keyboard report data
 *
 * @return  uint8_t - send status (0: success, non-zero: failure)
 */
uint8_t bt_driver_send_nkro(uint8_t *report)
{
    /* BLE 不支持标准 NKRO，转发给标准键盘报告 */
    return bt_driver_send_keyboard(report);
}

/*********************************************************************
 * @fn      bt_driver_send_consumer
 *
 * @brief   Send consumer (media) report
 *
 * @param   report - consumer report data
 *
 * @return  uint8_t - send status (0: success, non-zero: failure)
 */
uint8_t bt_driver_send_consumer(uint16_t report)
{
    uint8_t buffer[2];
    buffer[0] = LO_UINT16(report);
    buffer[1] = HI_UINT16(report);
    uint8_t ret = HidDev_Report(HID_RPT_ID_CONSUMER_IN, HID_REPORT_TYPE_INPUT,
                                sizeof(buffer), buffer);
    dprintf("[BT] send_consumer val=0x%04x ret=%d\n", report, ret);
    return ret;
}

/*********************************************************************
 * @fn      bt_driver_send_system
 *
 * @brief   Send system control report
 *
 * @param   report - system control report data
 *
 * @return  uint8_t - send status (0: success, non-zero: failure)
 */
uint8_t bt_driver_send_system(uint16_t report)
{
    uint8_t buffer[2];
    buffer[0] = LO_UINT16(report);
    buffer[1] = HI_UINT16(report);
    uint8_t ret = HidDev_Report(HID_RPT_ID_SYS_CTL_IN, HID_REPORT_TYPE_INPUT,
                                sizeof(buffer), buffer);
    dprintf("[BT] send_system val=0x%04x ret=%d\n", report, ret);
    return ret;
}

/*********************************************************************
 * @fn      bt_driver_send_mouse
 *
 * @brief   Send mouse report
 *
 * @param   report - mouse report data
 *
 * @return  uint8_t - send status (0: success, non-zero: failure)
 */
uint8_t bt_driver_send_mouse(uint8_t *report)
{
    uint8_t ret = HidDev_Report(HID_RPT_ID_MOUSE_IN, HID_REPORT_TYPE_INPUT,
                                BT_DRV_MOUSE_RPT_LEN, report);
    if (ret != 0) {
        dprintf("[BT] send_mouse fail ret=%d\n", ret);
    }
    return ret;
}

/*********************************************************************
 * @fn      bt_driver_update_bat_level
 *
 * @brief   Update battery level
 *
 * @param   bat_lvl - battery level percentage
 *
 * @return  none
 */
void bt_driver_update_bat_level(uint8_t bat_lvl)
{
    dprintf("[BT] update_bat_level %d%%\n", bat_lvl);
    Batt_SetParameter(BATT_PARAM_LEVEL, sizeof(uint8_t), &bat_lvl);
}

/*********************************************************************
 * @fn      bt_driver_dump_state
 *
 * @brief   打印 BLE 底层状态快照，用于调试状态机转换
 *
 * @return  none
 */
void bt_driver_dump_state(void)
{
    uint8_t ble_state = 0;
    GAPRole_GetParameter(GAPROLE_STATE, &ble_state);

    const char *gap_name;
    switch (ble_state) {
        case GAPROLE_INIT:          gap_name = "INIT";          break;
        case GAPROLE_STARTED:       gap_name = "STARTED";       break;
        case GAPROLE_ADVERTISING:   gap_name = "ADVERTISING";   break;
        case GAPROLE_WAITING:       gap_name = "WAITING";       break;
        case GAPROLE_CONNECTED:     gap_name = "CONNECTED";     break;
        case GAPROLE_CONNECTED_ADV: gap_name = "CONNECTED_ADV"; break;
        case GAPROLE_ERROR:         gap_name = "ERROR";         break;
        default:                    gap_name = "UNKNOWN";       break;
    }

    dprintf("[BT_DUMP] gap=%s(%d) con_mode=%d ble_idx=%d pairing=%d\n",
            gap_name, ble_state,
            con_work_mode, access_state.ble_idx, access_state.pairing_state);
    dprintf("[BT_DUMP] bonded: [1]=%d [2]=%d [3]=%d\n",
            hidEmu_is_ble_bonded(BLE_INDEX_1),
            hidEmu_is_ble_bonded(BLE_INDEX_2),
            hidEmu_is_ble_bonded(BLE_INDEX_3));
}

/*********************************************************************
 * @fn      bt_driver_send_ptp
 *
 * @brief   发送 PTP 报告（当前阶段不实现）
 *
 * @param   report - PTP 报告数据
 * @param   len - 报告数据长度
 *
 * @return  uint8_t - 0（成功）
 *
 * @note    TODO: 待 Profile 工具生成 PTP 描述符后实现
 */
uint8_t bt_driver_send_ptp(uint8_t *report, uint8_t len)
{
    uint8_t ret = HidDev_Report(HID_RPT_ID_MOUSE_IN, HID_REPORT_TYPE_INPUT,
                                len, report);
    if (ret != 0) {
        dprintf("[BT] send_ptp fail ret=%d\n", ret);
    }
    return ret;
}

void bt_driver_set_advertising(bool enable)
{
    hidEmu_adv_enable(enable ? ENABLE : DISABLE);
}
