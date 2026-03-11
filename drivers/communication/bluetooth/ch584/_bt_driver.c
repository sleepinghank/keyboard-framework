
#include "hidkbd.h"
#include "CONFIG.h"
#include "_bt_driver.h"
#include "hiddev.h"
#include "hidkbdservice.h"
#include "CH58x_common.h"
#include "HAL.h"
#include "print.h"
#include "debug.h"
static bool g_bt_driver_initialized = false;





/*********************************************************************
 * @fn      bt_driver_setup
 *
 * @brief   Setup function called during system initialization
 *
 * @return  none
 */
void bt_driver_setup(void)
{
    // Setup will be called in early initialization
    // Full init happens in bt_driver_init
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
    GAPRole_PeripheralInit( );
    HidDev_Init();
    HidEmu_Init();

    g_bt_driver_initialized = true;
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
    dprintf("bt_driver_connect_ex: host_idx=%d, timeout=%d\r\n", host_idx, timeout);
    if ((target_host_idx <= BLE_INDEX_IDEL) || (target_host_idx >= BLE_INDEX_MAX)) {
        storage_config_t *cfg = storage_get_config_ptr();
        if ((cfg != NULL) && (cfg->ble_idx > BLE_INDEX_IDEL) && (cfg->ble_idx < BLE_INDEX_MAX)) {
            target_host_idx = cfg->ble_idx;
        } else {
            target_host_idx = BLE_INDEX_1;
        }
    }

    access_state.ble_idx = target_host_idx;



    GAPRole_GetParameter( GAPROLE_STATE, &ble_state );

    if (con_work_mode != target_host_idx){

        access_state.pairing_state = FALSE;

        if( ble_state == GAPROLE_CONNECTED )
        {

            hidEmu_disconnect();
        }
        else if( ble_state == GAPROLE_ADVERTISING )
        {

            hidEmu_adv_enable( DISABLE );
        }
        else
        {

            if( hidEmu_is_ble_bonded( access_state.ble_idx ) )
            {

                hidEmu_adv_enable( ENABLE );
            }
            else
            {

                con_work_mode = access_state.ble_idx;
            }
        }


        if( OSAL_GetTaskTimer( hidEmuTaskId, SEND_DISCONNECT_EVT ) /* || (tmos_get_event(hidEmuTaskId)&SEND_DISCONNECT_EVT)*/ )
        {

            OSAL_StopTask(hidEmuTaskId, SEND_DISCONNECT_EVT);
        }
    } else {


        access_state.pairing_state = FALSE;

        if( (ble_state == GAPROLE_CONNECTED) || OSAL_GetTaskTimer( hidEmuTaskId, SEND_DISCONNECT_EVT ))
        {
        } else if (ble_state == GAPROLE_ADVERTISING){


        } else {

            if( hidEmu_is_ble_bonded( access_state.ble_idx ) )
            {

                hidEmu_adv_enable( ENABLE );
            }
            else
            {

            }

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
    pairing_param_t default_pairing_param = {0, 0, 0, 0, 0};
    dprintf("bt_driver_pairing_ex: host_idx=%d\n", host_idx);
    if (param == NULL) {
        param = &default_pairing_param;
    }
    pairing_param_t* p = (pairing_param_t*)param;

    GAPRole_GetParameter( GAPROLE_STATE, &ble_state );


    if( ble_state == GAPROLE_CONNECTED )
    {


        hidEmu_disconnect();

        access_state.pairing_state = TRUE;

        // access_update_idel_sleep_timeout(ADV_IDEL_SLEEP_EVT_TIMEOUT);

        return;
    }


    if( hidEmu_is_ble_bonded( access_state.ble_idx ) )
    {


        access_state.pairing_state = TRUE;
    }


    if( ble_state == GAPROLE_ADVERTISING )
    {

        if( con_work_mode == access_state.ble_idx )
        {

        }

        hidEmu_adv_enable( DISABLE );
    }
    else
    {


        hidEmu_adv_enable( ENABLE );
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
    hidEmu_disconnect();
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
    uint8_t buffer[21];
    // buffer[0] = CMD_CLASS_KEYBOARD;
    memcpy(&buffer[1], report, 20);
    return HidDev_Report(HID_RPT_ID_MOUSE_IN, HID_REPORT_TYPE_INPUT, 21, buffer);
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
    uint8_t buffer[21];
    // buffer[0] = CMD_ALL_KEYBOARD;
    memcpy(&buffer[1], report, 20);
    return HidDev_Report(HID_RPT_ID_MOUSE_IN, HID_REPORT_TYPE_INPUT, 21, buffer);
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
    uint8_t buffer[3];
    // buffer[0] = CMD_CONSUMER;
    buffer[1] = LO_UINT16(report);
    buffer[2] = HI_UINT16(report);
    return HidDev_Report(HID_RPT_ID_MOUSE_IN, HID_REPORT_TYPE_INPUT, 3, buffer);
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
    uint8_t buffer[3];
    // buffer[0] = CMD_SYS_CTL;
    buffer[1] = LO_UINT16(report);
    buffer[2] = HI_UINT16(report);
    return HidDev_Report(HID_RPT_ID_MOUSE_IN, HID_REPORT_TYPE_INPUT, 3, buffer);
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
    uint8_t buffer[21];
    // buffer[0] = CMD_MOUSE;
    memcpy(&buffer[1], report, 20);
    return HidDev_Report(HID_RPT_ID_MOUSE_IN, HID_REPORT_TYPE_INPUT, 21, buffer);
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
    // Batt_SetParameter(BATT_PARAM_LEVEL, sizeof(uint8_t), &bat_lvl);
}

void bt_driver_set_advertising(bool enable)
{
    hidEmu_adv_enable(enable ? ENABLE : DISABLE);
}
