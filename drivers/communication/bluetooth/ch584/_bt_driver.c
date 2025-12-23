
#include "hidkbd.h"
#include "CONFIG.h"
#include "_bt_driver.h"
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
    CH58x_BLEInit(BLE_SNV_ADDR+0x100);//低功耗蓝牙库初始化，蓝牙协议栈部分
    DelayUs(1);
    HAL_Init();//硬件参数，如RTC，sleep
    GAPRole_PeripheralInit( );
    HidDev_Init();//设备层，hid设备初始化基于蓝牙协议栈
    HidEmu_Init();//实际应用层
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
    // Set advertising enabled to start reconnecting
    hidEmu_adv_enable(TRUE);
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
    // Clear any previous bonding
    hidEmu_delete_ble_bonded();

    // Enable advertising in pairing mode
    hidEmu_adv_enable(TRUE);
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
 * @return  none
 */
void bt_driver_send_keyboard(uint8_t *report)
{
    uint8_t buffer[21];
    buffer[0] = CMD_CLASS_KEYBOARD;
    memcpy(&buffer[1], report, 20);
    hidEmu_receive(buffer, 21);
}

/*********************************************************************
 * @fn      bt_driver_send_nkro
 *
 * @brief   Send NKRO keyboard report
 *
 * @param   report - NKRO keyboard report data
 *
 * @return  none
 */
void bt_driver_send_nkro(uint8_t *report)
{
    uint8_t buffer[21];
    buffer[0] = CMD_ALL_KEYBOARD;
    memcpy(&buffer[1], report, 20);
    hidEmu_receive(buffer, 21);
}

/*********************************************************************
 * @fn      bt_driver_send_consumer
 *
 * @brief   Send consumer (media) report
 *
 * @param   report - consumer report data
 *
 * @return  none
 */
void bt_driver_send_consumer(uint16_t report)
{
    uint8_t buffer[3];
    buffer[0] = CMD_CONSUMER;
    buffer[1] = LO_UINT16(report);
    buffer[2] = HI_UINT16(report);
    hidEmu_receive(buffer, 3);
}

/*********************************************************************
 * @fn      bt_driver_send_system
 *
 * @brief   Send system control report
 *
 * @param   report - system control report data
 *
 * @return  none
 */
void bt_driver_send_system(uint16_t report)
{
    uint8_t buffer[3];
    buffer[0] = CMD_SYS_CTL;
    buffer[1] = LO_UINT16(report);
    buffer[2] = HI_UINT16(report);
    hidEmu_receive(buffer, 3);
}

/*********************************************************************
 * @fn      bt_driver_send_mouse
 *
 * @brief   Send mouse report
 *
 * @param   report - mouse report data
 *
 * @return  none
 */
void bt_driver_send_mouse(uint8_t *report)
{
    uint8_t buffer[21];
    buffer[0] = CMD_MOUSE;
    memcpy(&buffer[1], report, 20);
    hidEmu_receive(buffer, 21);
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
