
#include "hidkbd.h"
#include "CONFIG.h"
#include "_bt_driver.h"
#include "hiddev.h"
#include "hidkbdservice.h"





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
    uint8_t ble_state = 0;
    access_state.ble_idx = host_idx;
    // 检查当前工作模式是否为有效的蓝牙通道（BLE_INDEX_1到BLE_INDEX_5）

    // 获取当前蓝牙状态
    GAPRole_GetParameter( GAPROLE_STATE, &ble_state );
    // 如果当前通道与目标通道不同，则需要切换通道
    if (con_work_mode != host_idx){
        // 清除配对状态，准备新的连接
        access_state.pairing_state = FALSE;
        // 根据当前蓝牙状态执行相应操作
        if( ble_state == GAPROLE_CONNECTED )
        {
            // 如果当前已连接，需要断开连接
            hidEmu_disconnect();
        }
        else if( ble_state == GAPROLE_ADVERTISING )
        {
            // 如果正在广播，需要停止广播
            hidEmu_adv_enable( DISABLE );
        }
        else
        {
            // 其他状态：检查目标通道是否已经绑定过设备
            if( hidEmu_is_ble_bonded( access_state.ble_idx ) ) // 如果已绑定
            {
                // 启用广播，允许设备连接
                hidEmu_adv_enable( ENABLE );
            }
            else
            {
                // 如果未绑定设备，上报重连失败状态
                con_work_mode = access_state.ble_idx;
            }
        }

        // 检查是否有正在进行的断开连接任务
        if( OSAL_GetTaskTimer( hidEmuTaskId, SEND_DISCONNECT_EVT ) /* || (tmos_get_event(hidEmuTaskId)&SEND_DISCONNECT_EVT)*/ )
        {
            // 停止断开连接任务
            OSAL_StopTask(hidEmuTaskId, SEND_DISCONNECT_EVT);
        }
    } else {
        // 上次ble也是这个通道，重复切换到同一通道
        // 清除配对状态
        access_state.pairing_state = FALSE;
        // 如果当前已连接或正在断开连接
        if( (ble_state == GAPROLE_CONNECTED) || OSAL_GetTaskTimer( hidEmuTaskId, SEND_DISCONNECT_EVT ))
        {
        } else if (ble_state == GAPROLE_ADVERTISING){
            // 如果是正在广播，则保持现状不做处理

        } else {
            // 其他状态：检查目标通道是否已经绑定过设备
            if( hidEmu_is_ble_bonded( access_state.ble_idx ) ) // 如果已绑定
            {
                // 启用广播，允许设备连接
                hidEmu_adv_enable( ENABLE );
            }
            else
            {
                // 如果未绑定设备，上报重连失败状态
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
    uint8_t ble_state;                       // 当前蓝牙状态变量
    pairing_param_t default_pairing_param = {0, 0, 0, 0, 0};

    if (param == NULL) {
        param = &default_pairing_param;
    }
    pairing_param_t* p = (pairing_param_t*)param;
    // 获取当前蓝牙状态
    GAPRole_GetParameter( GAPROLE_STATE, &ble_state );

    // 如果当前处于连接状态
    if( ble_state == GAPROLE_CONNECTED )
    {
        // 打印断开连接的调试信息
        // 当前还在连接中，需要断开连接，然后换地址进行新配对
        hidEmu_disconnect();
        // 设置配对状态为TRUE，表示进入配对流程
        access_state.pairing_state = TRUE;
        // 开启广播60秒后进入睡眠，睡眠函数中如果还未连接，则停止广播直接睡眠
        // access_update_idel_sleep_timeout(ADV_IDEL_SLEEP_EVT_TIMEOUT);
        // 立即返回，等待断开连接完成
        return;
    }

    // 检查当前通道是否已经绑定过设备
    if( hidEmu_is_ble_bonded( access_state.ble_idx ) )
    {
        // 当前已经绑定过设备，需要换地址进行新配对
        // 设置配对状态为TRUE
        access_state.pairing_state = TRUE;
    }

    // 检查当前是否正在广播
    if( ble_state == GAPROLE_ADVERTISING )
    {
        // 如果当前正在广播，检查是否在相同通道
        if( con_work_mode == access_state.ble_idx )
        {
            // 同通道多次长按每次都要发送STATE_PAIRING状态，但不关闭广播
        }
        // 关闭当前广播，准备重新开始配对广播
        hidEmu_adv_enable( DISABLE );
    }
    else
    {
        // 上报配对状态给主控
        // 启用广播，开始等待配对
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
 * @return  uint8_t - 发送状态（0:成功, 非0:失败）
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
 * @return  uint8_t - 发送状态（0:成功, 非0:失败）
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
 * @return  uint8_t - 发送状态（0:成功, 非0:失败）
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
 * @return  uint8_t - 发送状态（0:成功, 非0:失败）
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
 * @return  uint8_t - 发送状态（0:成功, 非0:失败）
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
