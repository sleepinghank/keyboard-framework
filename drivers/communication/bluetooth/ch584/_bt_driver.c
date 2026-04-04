
#include "hidkbd.h"
#include "CONFIG.h"
#include "_bt_driver.h"
#include "kb904/config_product.h"
#include "hiddev.h"
#include "hidkbdservice.h"
#include "battservice.h"
#include "CH58x_common.h"
#include "HAL.h"
#include "debug.h"
#include "event_manager.h"

static bool g_bt_driver_initialized = false;

/* LED 状态回调：由 wireless 层注册，Profile 层收到 HID Out Report 时触发 */
static bt_led_cb_t s_led_cb = NULL;

void bt_driver_register_led_cb(bt_led_cb_t cb) {
    s_led_cb = cb;
}

void bt_driver_notify_led_state(uint8_t led_state) {
    if (s_led_cb) {
        s_led_cb(led_state);
    }
}

/* 报告数据长度：与 HID 描述符保持一致 */
#define BT_DRV_KEYBOARD_RPT_LEN    8    /* modifier(1) + reserved(1) + keycodes(6) */
#define BT_DRV_MOUSE_RPT_LEN       5    /* buttons(1) + X(1) + Y(1) + wheel(1) + pan(1) */
#define BT_DRV_PTP_RPT_LEN       19    /* buttons(1) + X(1) + Y(1) + wheel(1) + pan(1) */

access_state_t access_state;
access_ble_idx_t con_work_mode = BLE_INDEX_1;
/*********************************************************************
 * @fn      bt_driver_setup
 *
 * @brief   Setup function called during system initialization
 *
 * @return  none
 */
void bt_driver_setup(void)
{
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
    (void)wakeup_from_low_power;

    if (g_bt_driver_initialized) {
        return;
    }
    CH58x_BLEInit();
    DelayUs(1);
    HAL_Init();
    GAPRole_PeripheralInit();
    HidDev_Init();
    HidEmu_Init();

    g_bt_driver_initialized = true;
    LOG_I("[BT] init done");
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
    (void)timeout;
    ble_state = hidEmu_GetGAP_State();
    access_state.work_mode  = WORK_IDEL;
    if (con_work_mode!= host_idx ) {
        access_state.pairing_state = FALSE;
        access_state.ble_idx = host_idx;
        if (ble_state == GAPROLE_CONNECTED)
        {
            access_state.work_mode  = WORK_RECONNECT;
            hidEmu_disconnect();
        } else if( ble_state == GAPROLE_ADVERTISING )
        {
            access_state.work_mode  = WORK_RECONNECT;
            hidDev_adv_enable( DISABLE );
        } else {
            hidEmu_connect_adv(host_idx);
        }
    } else {
        if(ble_state == GAPROLE_CONNECTED)
        {

        }
        else if( ble_state == GAPROLE_ADVERTISING )
        {
        }
        else{
            access_state.pairing_state = FALSE;
            access_state.ble_idx = host_idx;
            hidEmu_connect_adv(host_idx);
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

    uint8_t ble_state = 0;
    (void)param;
    ble_state = hidEmu_GetGAP_State();
    access_state.pairing_state = TRUE;
    access_state.ble_idx = host_idx;
    if (ble_state == GAPROLE_CONNECTED)
    {
        access_state.work_mode  = WORK_PARING;
        hidEmu_disconnect();
    } else if( ble_state == GAPROLE_ADVERTISING )
    {
        access_state.work_mode  = WORK_PARING;
        hidDev_adv_enable( DISABLE );
    } else {
        hidEmu_pairing_adv(host_idx);
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
    LOG_I("[BT] disconnect");
    access_state.pairing_state = FALSE;
    access_state.ble_idx = 0;
    access_state.work_mode  = WORK_DISCONNECTED;
    uint8_t ble_state;
    GAPRole_GetParameter( GAPROLE_STATE, &ble_state );
    if( ble_state == GAPROLE_CONNECTED )
    {
        hidEmu_disconnect();
    } else if( ble_state == GAPROLE_ADVERTISING )
    {
        hidDev_adv_enable( DISABLE );
    }
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
    LOG_I("[BT] clear_bonding");
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
    if (ret != 0 && ret != blePending) {
        LOG_E("[BT] send_keyboard fail, ret=%d", ret);
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
    if (ret != 0 && ret != blePending) {
        LOG_E("[BT] send_mouse fail, ret=%d", ret);
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
    Batt_SetParameter(BATT_PARAM_LEVEL, sizeof(uint8_t), &bat_lvl);
}

/*********************************************************************
 * @fn      bt_driver_send_ptp
 *
 * @brief   发送 PTP 报告
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
    uint8_t ret = HidDev_Report(HID_RPT_ID_TOUCHPAD_IN, HID_REPORT_TYPE_INPUT,
                                len, report);
    if (ret != 0) {
        LOG_E("[BT] send_ptp fail, ret=%d,len:%d", ret, len);
    }
    return ret;
}

void bt_driver_set_advertising(bool enable)
{
    hidDev_adv_enable(enable ? ENABLE : DISABLE);
}

/* 连接句柄：由 hidkbd.c 中 GAP 连接事件写入 */
extern uint16_t hidEmuConnHandle;

uint32_t bt_driver_get_unack_packets(void)
{
    if (hidEmuConnHandle == GAP_CONNHANDLE_INIT) {
        return 0;
    }
    return LL_GetNumberOfUnAckPacket(hidEmuConnHandle);
}
