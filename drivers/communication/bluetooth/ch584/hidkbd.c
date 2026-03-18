/********************************** (C) COPYRIGHT *******************************
 * File Name          : hidkbd.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/12/10
 * Description        : 蓝牙键盘应用程序，初始化广播连接参数，然后广播，直至连接主机后，定时上传键值
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "CH58x_common.h"
#include "CH58xBLE_LIB.h"
#include "CONFIG.h"
#include "devinfoservice.h"
#include "battservice.h"
#include "hidkbdservice.h"
#include "hiddev.h"
#include "HAL.h"
#include "hidkbd.h"
#include "event_manager.h"
#include "debug.h"
#include "wireless.h"
#include "keyboard.h"
#include <string.h>

#ifdef dprint
#undef dprint
#endif
#define dprint(...) dprintf(__VA_ARGS__)

//应用状态
enum
{
    BLE_STATE_IDLE,
    BLE_STATE_CONNECTING,
    BLE_STATE_CONNECTED,
    BLE_STATE_DISCONNECTING
};
//发现状态
enum
{
    BLE_DISC_STATE_IDLE, // Idle
    BLE_DISC_STATE_SVC,  // Service discovery
    BLE_DISC_STATE_CHAR, // Characteristic discovery
    BLE_DISC_STATE_CCCD  // client characteristic configuration discovery
};
// static volatile uint8_t centralTaskId;
uint8_t centralTaskId = INVALID_TASK_ID;

uint16_t Central_ProcessEvent(uint8_t task_id, uint16_t events);
static void centralProcessGATTMsg(gattMsgEvent_t *pMsg);
//中心状态变量
static uint8_t centralState = BLE_STATE_IDLE;
static uint8_t centralDiscState = BLE_DISC_STATE_IDLE;
static uint8_t centralProcedureInProgress = FALSE;
static volatile uint16_t centralConnHandle = GAP_CONNHANDLE_INIT;

static void central_ProcessTMOSMsg(tmos_event_hdr_t *pMsg);
static void centralStartDiscovery(void);
static void centralGATTDiscoveryEvent(gattMsgEvent_t *pMsg);
//中心服务句柄
static uint16_t centralSvcStartHdl = 0;
static uint16_t centralSvcEndHdl = 0;
static uint16_t centralCharHdl = 0;
static uint16_t centralCCCDHdl = 0;

typedef struct att_read_by_type_pair_val
{
    uint16_t hdl;
    uint8_t val[10];
} att_read_by_type_pair_val_t;

typedef struct att_read_by_type_val_rsp
{
    uint8_t length;
    struct att_read_by_type_pair_val pair[1];
} att_read_by_type_val_rsp_t;

/*********************************************************************
 * MACROS
 */
// HID keyboard input report length
#define HID_KEYBOARD_IN_RPT_LEN              8

// HID LED output report length
#define HID_LED_OUT_RPT_LEN                  1

/*********************************************************************
 * CONSTANTS
 */
#define TERMINATE_EVT_TIMEOUT                160


// PHY update delay
#define START_PHY_UPDATE_DELAY               1600

// HID idle timeout in msec; set to zero to disable timeout
#define DEFAULT_HID_IDLE_TIMEOUT             60000
#define WL_RECONNECT_ADV_TIMEOUT             (1600 * 60)

// Default passcode
#define DEFAULT_PASSCODE                     0

// Default GAP pairing mode
#define DEFAULT_PAIRING_MODE                 GAPBOND_PAIRING_MODE_WAIT_FOR_REQ

// Default MITM mode (TRUE to require passcode or OOB when pairing)
#define DEFAULT_MITM_MODE                    FALSE

// Default bonding mode, TRUE to bond
#define DEFAULT_BONDING_MODE                 TRUE

// Default GAP bonding I/O capabilities
#define DEFAULT_IO_CAPABILITIES              GAPBOND_IO_CAP_NO_INPUT_NO_OUTPUT

// Battery level is critical when it is less than this %
#define DEFAULT_BATT_CRITICAL_LEVEL          100

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// Task ID
uint8_t hidEmuTaskId = INVALID_TASK_ID;

uint8_t start_device_over = FALSE;
uint8_t adv_enable_process_flag = FALSE;
/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */
extern void access_ble_notify_advertising(uint8_t pairing_state, uint8_t host_idx);
extern void access_ble_notify_connected(uint8_t host_idx);
extern void access_ble_notify_disconnected(uint8_t host_idx, uint8_t reason);
extern void access_ble_enter_idel_sleep(void);
extern void access_ble_init_done(void);
extern void access_ble_schedule_deep_sleep_evt(uint32_t delay_ticks);
extern void access_ble_cancel_deep_sleep_evt(void);

/*********************************************************************
 * LOCAL VARIABLES
 */
access_state_t access_state;            // Access模块的全局状态结构体
bleConfig_t ble_config;
static uint8_t reconnect_adv_fallback_stage = 1; // 0: whitelist reconnect, 1: general fallback

// GAP Profile - Name attribute for SCAN RSP data
static uint8_t scanRspData[] = {
    0x0D,                           // length of this data
    GAP_ADTYPE_LOCAL_NAME_COMPLETE, // AD Type = Complete local name
    'H',
    'I',
    'D',
    ' ',
    'K',
    'e',
    'y',
    'b',
    'T',
    'E',
    'S',
    'T',  // connection interval range
    0x05, // length of this data
    GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
    LO_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL), // 100ms
    HI_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
    LO_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL), // 1s
    HI_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),

    // service UUIDs
    0x05, // length of this data
    GAP_ADTYPE_16BIT_MORE,
    LO_UINT16(HID_SERV_UUID),
    HI_UINT16(HID_SERV_UUID),
    LO_UINT16(BATT_SERV_UUID),
    HI_UINT16(BATT_SERV_UUID),

    // Tx power level
    0x02, // length of this data
    GAP_ADTYPE_POWER_LEVEL,
    0 // 0dBm                        发射功率
};

// Advertising data
static uint8_t advertData[31] = {
    // flags
    0x02, // length of this data
    GAP_ADTYPE_FLAGS,
    GAP_ADTYPE_FLAGS_LIMITED | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

//    // service UUIDs
//    0x03, // length of this data
//    GAP_ADTYPE_16BIT_MORE,
//    LO_UINT16(HID_SERV_UUID),
//    HI_UINT16(HID_SERV_UUID),
//    LO_UINT16(BATT_SERV_UUID),
//    HI_UINT16(BATT_SERV_UUID),

    // appearance
    0x03, // length of this data
    GAP_ADTYPE_APPEARANCE,
    LO_UINT16(GAP_APPEARE_HID_KEYBOARD),
    HI_UINT16(GAP_APPEARE_HID_KEYBOARD)
};

// Advertising data
static uint8_t reconAdvertData[1] = {
    0
//    // flags
//    0x02, // length of this data
//    GAP_ADTYPE_FLAGS,
//    GAP_ADTYPE_FLAGS_LIMITED | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED

};

// HID Dev configuration
static hidDevCfg_t hidEmuCfg = {
    DEFAULT_HID_IDLE_TIMEOUT, // Idle timeout
    HID_FEATURE_FLAGS         // HID feature flags
};

uint16_t hidEmuConnHandle = GAP_CONNHANDLE_INIT;

access_ble_idx_t con_work_mode = BLE_INDEX_IDEL;

#define BLE_SEND_BUF_LEN      20   //  缓存5个包

typedef struct
{
    uint8_t len;
    uint8_t resend;
    uint8_t pData[20];
}BLE_send_buffer_t;

BLE_send_buffer_t BLE_send_buffer[BLE_SEND_BUF_LEN]={0};

uint8_t BLE_buf_out_idx=0;
uint8_t BLE_buf_data_num=0;
uint8_t BLE_buf_resend_num=0;
/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void    hidEmu_ProcessTMOSMsg(tmos_event_hdr_t *pMsg);
static uint8_t hidEmuRcvReport(uint8_t len, uint8_t *pData);
static uint8_t hidEmuRptCB(uint8_t id, uint8_t type, uint16_t uuid,
                           uint8_t oper, uint16_t *pLen, uint8_t *pData);
static void    hidEmuEvtCB(uint8_t evt);
static void    hidEmuStateCB(gapRole_States_t newState, gapRoleEvent_t *pEvent);

uint8_t HCI_MB_DisconnectCmd( uint16_t connHandle, uint8_t reason );
extern uint8_t LL_SetDataRelatedAddressChanges( uint8_t Advertising_Handle, uint8_t Change_Reasons ) ;
extern void access_ble_enter_idel_sleep(void);

__attribute__((weak)) void access_ctl_process(uint8_t ble_idx)
{
    (void)ble_idx;
}

__attribute__((weak)) void hidDevBattCB(uint8_t evt)
{
    (void)evt;
}
//void hidEmu_NEXT_BUF(void);

/*********************************************************************
 * PROFILE CALLBACKS
 */

static hidDevCB_t hidEmuHidCBs = {
    hidEmuRptCB,//报告回调
    hidEmuEvtCB,//事件回调
    NULL,
    hidEmuStateCB//状态回调
    };//回调函数

pfnHidEmuReceiveCB_t    hidEmu_receive_cb = 0;

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      HidEmu_Init
 *
 * @brief   Initialization function for the HidEmuKbd App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by TMOS.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void HidEmu_Init()
{
    hidEmuTaskId = OSAL_ProcessEventRegister(HidEmu_ProcessEvent);
    //注册中心设备任务
    centralTaskId = OSAL_ProcessEventRegister(Central_ProcessEvent);
    dprint("hidEmuTaskId:%x,centralTaskId:%x\n",hidEmuTaskId,centralTaskId);

    {
        storage_config_t *cfg = storage_get_config_ptr();
        access_state.pairing_state = FALSE;
        access_state.deep_sleep_flag = FALSE;
        access_state.idel_sleep_flag = FALSE;
        access_state.ble_idx = BLE_INDEX_1;
        if((cfg != NULL) && (cfg->ble_idx > BLE_INDEX_IDEL) && (cfg->ble_idx < BLE_INDEX_MAX))
        {
            access_state.ble_idx = cfg->ble_idx;
        }
        con_work_mode = access_state.ble_idx;
    }

    // Setup the GAP Peripheral Role Profile
    {
        uint8_t initial_advertising_enable = TRUE;//使能广播
        GAP_SetParamValue(TGAP_DISC_ADV_INT_MIN, 160);
        GAP_SetParamValue(TGAP_DISC_ADV_INT_MAX, 160);

        // Set the GAP Role Parameters 设置gap参数，广播参数，扫描参数等
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &initial_advertising_enable);

        // 从flash恢复设备名称
        // scanRspData[0] = storage_get_config_ptr()->ble_name_len+1;
        // scanRspData[1] = GAP_ADTYPE_LOCAL_NAME_COMPLETE;
        // memcpy(&scanRspData[2], storage_get_config_ptr()->ble_name_data, storage_get_config_ptr()->ble_name_len);
        // memcpy(&advertData[13-2-4], scanRspData, storage_get_config_ptr()->ble_name_len+2);
//        dprint("len %d %x\n",scanRspData[0],scanRspData[1]);
        GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(advertData), advertData);
        GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, sizeof(scanRspData), scanRspData);//实际的广播参数设置，之前的sdk截断了这个广播包
    }

    // Set the GAP Characteristics
    GGS_SetParameter(GGS_DEVICE_NAME_ATT, storage_get_config_ptr()->ble_name_len, (void *)storage_get_config_ptr()->ble_name_data);

    // Setup the GAP Bond Manager
    {
        uint32_t passkey = DEFAULT_PASSCODE;
        uint8_t  pairMode = DEFAULT_PAIRING_MODE;
        uint8_t  mitm = DEFAULT_MITM_MODE;
        uint8_t  ioCap = DEFAULT_IO_CAPABILITIES;
        uint8_t  bonding = DEFAULT_BONDING_MODE;
//        uint8_t  RL_enable = ENABLE;
        GAPBondMgr_SetParameter(GAPBOND_PERI_DEFAULT_PASSCODE, sizeof(uint32_t), &passkey);
        GAPBondMgr_SetParameter(GAPBOND_PERI_PAIRING_MODE, sizeof(uint8_t), &pairMode);
        GAPBondMgr_SetParameter(GAPBOND_PERI_MITM_PROTECTION, sizeof(uint8_t), &mitm);
        GAPBondMgr_SetParameter(GAPBOND_PERI_IO_CAPABILITIES, sizeof(uint8_t), &ioCap);
        GAPBondMgr_SetParameter(GAPBOND_PERI_BONDING_ENABLED, sizeof(uint8_t), &bonding);
//        GAPBondMgr_SetParameter(GAPBOND_AUTO_SYNC_RL, sizeof(uint8_t), &RL_enable);
    }

    // Setup Battery Characteristic Values
    {
        uint8_t critical = DEFAULT_BATT_CRITICAL_LEVEL;
        Batt_SetParameter(BATT_PARAM_CRITICAL_LEVEL, sizeof(uint8_t), &critical);
    }

    // Set up HID keyboard service
    Hid_AddService();

    // Register for HID Dev callback 注册回调函数到系统
    HidDev_Register(&hidEmuCfg, &hidEmuHidCBs);

    // Initialize GATT Client 初始化客户端
    GATT_InitClient();
    // Register to receive incoming ATT Indications/Notifications
    GATT_RegisterForInd(centralTaskId);

    // Setup a delayed profile startup
    OSAL_SetEvent(hidEmuTaskId, START_DEVICE_EVT);
}

/*********************************************************************
 * @fn      HidEmu_ProcessEvent
 *
 * @brief   HidEmuKbd Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
#if 0
uint16_t HidEmu_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & SYS_EVENT_MSG)////消息处理，连接断开，连接成功等  系统消息处理，事件处理，系统预定义的，不能被修改
    {
        uint8_t *pMsg;

        if((pMsg = OSAL_MsgReceive(hidEmuTaskId)) != NULL)
        {
            hidEmu_ProcessTMOSMsg((tmos_event_hdr_t *)pMsg);

            // Release the TMOS message
            OSAL_MsgDeallocate(pMsg);
        }

        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    if(events & START_DEVICE_EVT)//启动事件，实际启动设备
    {
        return (events ^ START_DEVICE_EVT);
    }

    if(events & START_PHY_UPDATE_EVT)//切换物理层，更新物理层
    {
        // start phy update
        dprint("Send Phy Update %x...\n", GAPRole_UpdatePHY(hidEmuConnHandle, 0, 
                    GAP_PHY_BIT_LE_2M, GAP_PHY_BIT_LE_2M, 0));

        return (events ^ START_PHY_UPDATE_EVT);
    }

    if(events & PERI_SECURITY_REQ_EVT)//外设安全请求
    {
        // start phy update
        uint8_t ble_state;
        GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
        if(ble_state == GAPROLE_CONNECTED)
        {
            dprint("Send Security Req ...\n");
            if(GAPBondMgr_PeriSecurityReq(hidEmuConnHandle))
            {
                OSAL_SetDelayedEvent(hidEmuTaskId, PERI_SECURITY_REQ_EVT, 4800);
            }
        }

        return (events ^ PERI_SECURITY_REQ_EVT);
    }

    if(events & WAIT_TERMINATE_EVT)//等待断开连接，等待终止
    {
        uint8_t ble_state;
        GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
        if(ble_state == GAPROLE_CONNECTED)
        {
            HCI_MB_DisconnectCmd(hidEmuConnHandle, 0x16);
        }
        return (events ^ WAIT_TERMINATE_EVT);
    }

    if(events & SEND_DISCONNECT_EVT)//发送断开连接，连接后发送数据
    {
    {
        dprint("SEND_DISCONNECT_EVT\n");
        uint8_t ble_state;
//        dprint("SD\n");
        GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
        if(ble_state != GAPROLE_CONNECTED)
        {
            //access_tran_report(REPORT_CMD_STATE, STATE_CON_TERMINATE);
            // last_led_data = 0xFF;
            // 清空buff
            BLE_buf_out_idx=0;
            BLE_buf_data_num=0;
            BLE_buf_resend_num=0;
        }
        return (events ^ SEND_DISCONNECT_EVT);
    }

    if(events & ADV_TIMEOUT_SLEEP_EVT)
    {
        uint8_t ble_state;
        GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
        if((ble_state != GAPROLE_CONNECTED) && (!access_state.pairing_state))
        {
            dprint("ADV timeout -> request deep sleep\n");
            access_state.deep_sleep_flag = TRUE;
            hidEmu_adv_enable(DISABLE);
            access_ble_enter_idel_sleep();
        }
        else
        {
            dprint("ADV timeout ignored, state:%x pairing:%x\n", ble_state, access_state.pairing_state);
        }
        return (events ^ ADV_TIMEOUT_SLEEP_EVT);
    }

    if(events & SEND_PACKET_EVT)//发送hid数据包
    {
    {
        uint8_t ble_state;
        GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
        if(ble_state == GAPROLE_CONNECTED)
        {
//             if(BLE_buf_data_num && (BLE_send_buffer[BLE_buf_out_idx].resend==0))
//             {
//                 uint8_t state=0;
// //                dprint("%d\n",BLE_send_buffer[BLE_buf_out_idx].pData[0]);
//                 switch(BLE_send_buffer[BLE_buf_out_idx].pData[0])
//                 {
//                     case CMD_CLASS_KEYBOARD:
//                         state = hidEmu_class_keyboard_report(&BLE_send_buffer[BLE_buf_out_idx].pData[1],BLE_send_buffer[BLE_buf_out_idx].len-1);
//                         break;

//                     case CMD_ALL_KEYBOARD:
//                         state = hidEmu_all_keyboard_report(&BLE_send_buffer[BLE_buf_out_idx].pData[1],BLE_send_buffer[BLE_buf_out_idx].len-1);
//                         break;

//                     case CMD_CONSUMER:
//                         state = hidEmu_consumer_report(&BLE_send_buffer[BLE_buf_out_idx].pData[1],BLE_send_buffer[BLE_buf_out_idx].len-1);
//                         break;

//                     case CMD_SYS_CTL:
//                         state = hidEmu_sys_ctl_report(BLE_send_buffer[BLE_buf_out_idx].pData[1]);
//                         break;

//                     case CMD_MOUSE:
//                         state = hidEmu_mouse_report(&BLE_send_buffer[BLE_buf_out_idx].pData[1],BLE_send_buffer[BLE_buf_out_idx].len-1);
//                         break;

//                     case CMD_FN_DATA:
//                         state = hidEmu_fn_report(&BLE_send_buffer[BLE_buf_out_idx].pData[1],BLE_send_buffer[BLE_buf_out_idx].len-1);
//                         break;
// //
// //                    case CMD_SMART_WHEEL:
// //                        state = hidEmu_smart_wheel_report(&BLE_send_buffer[BLE_buf_out_idx].pData[1],BLE_send_buffer[BLE_buf_out_idx].len-1);
// //                        break;
//                     //触摸板数据发送，待验证先注释
//                     // case CMD_TOUCHPAD:
//                     //     state = hidEmu_touchpad_report(&BLE_send_buffer[BLE_buf_out_idx].pData[1],BLE_send_buffer[BLE_buf_out_idx].len-1);
//                     //     break;

//                     default:
//                     {
// //                        uint8_t temp1;
// //                        uint8_t temp2;
//                         dprint("code %x\n",BLE_send_buffer[BLE_buf_out_idx].pData[0]);
// //                        dprint("buf err$\n");
// //                        temp1 = BLE_buf_data_num;
// //                        temp2 = BLE_buf_out_idx;
// //                        while(temp1)
// //                        {
// //                            dprint("buf %d %d: %x %x %x %x %x\n",temp2,BLE_send_buffer[temp2].len,BLE_send_buffer[temp2].pData[0],BLE_send_buffer[temp2].pData[1],BLE_send_buffer[temp2].pData[2]
// //                                                       ,BLE_send_buffer[temp2].pData[3],BLE_send_buffer[temp2].pData[4]);
// //                            (temp2==(BLE_SEND_BUF_LEN-1))?(temp2=0):(temp2++);
// //                            temp1--;
// //                        }
// //                        // 清空buff
// //                        BLE_buf_out_idx=0;
// //                        BLE_buf_data_num=0;
// //                        BLE_buf_resend_num=0;
//                         state = 0xF1;
//                         break;
//                     }
//                 }
//                 if( (!state) || (state == 0xF1))
//                 {
// //                    dprint("1\n");
//                     BLE_send_buffer[BLE_buf_out_idx].resend = 1;
//                     BLE_buf_resend_num++;
// //                    dprint("b %d %d: %x %x %x %x %x\n",BLE_buf_out_idx,BLE_send_buffer[BLE_buf_out_idx].len,BLE_send_buffer[BLE_buf_out_idx].pData[0],BLE_send_buffer[BLE_buf_out_idx].pData[1],BLE_send_buffer[BLE_buf_out_idx].pData[2]
// //                                               ,BLE_send_buffer[BLE_buf_out_idx].pData[3],BLE_send_buffer[BLE_buf_out_idx].pData[4]);
// //                    {
//                         (BLE_buf_out_idx==(BLE_SEND_BUF_LEN-1))?(BLE_buf_out_idx=0):(BLE_buf_out_idx++);
//                         if((BLE_buf_data_num - BLE_buf_resend_num) >0)
//                         {
//                             tmos_set_event(hidEmuTaskId, SEND_PACKET_EVT);
//                         }
// //                    }
// //                    else {
// //                        dprint("all resend\n");
// //                        (BLE_buf_out_idx==(BLE_SEND_BUF_LEN-1))?(BLE_buf_out_idx=0):(BLE_buf_out_idx++);
// //                        tmos_set_event(hidEmuTaskId, SEND_PACKET_EVT);
// //                    }
//                 }
//                 else {
//                     dprint("!\n");
//                     OSAL_SetDelayedEvent(hidEmuTaskId, SEND_PACKET_EVT, 4);
//                 }
//                 OSAL_SetDelayedEvent(hidEmuTaskId, DELETE_PACKET_EVT, 4);
//             }
        }
        return (events ^ SEND_PACKET_EVT);
    }

    if(events & DELETE_PACKET_EVT)//删除已确认的数据包
    {
        uint8_t ble_state;
        // GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
        // if(ble_state == GAPROLE_CONNECTED)
        // {
        //     uint8_t UnAckPacket = LL_GetNumberOfUnAckPacket(hidEmuConnHandle);
        //     if( UnAckPacket < BLE_buf_resend_num )
        //     {
        //         BLE_buf_data_num -= (BLE_buf_resend_num - UnAckPacket);
        //         BLE_buf_resend_num = UnAckPacket;
        //     }
        //     if(UnAckPacket)
        //     {
        //         OSAL_SetDelayedEvent(hidEmuTaskId, DELETE_PACKET_EVT, 4);
        //     }
        // }
        return (events ^ DELETE_PACKET_EVT);
    }

    if(events & BLE_CLEAR_BUF_EVT)//清空缓存，清空数据队列
    {
        uint8_t ble_state;
        GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
        if(ble_state != GAPROLE_CONNECTED)
        {
            // 清空buff
            BLE_buf_out_idx=0;
            BLE_buf_data_num=0;
            BLE_buf_resend_num=0;
        }
        return (events ^ BLE_CLEAR_BUF_EVT);
    }
    return 0;
}
#endif

uint16_t HidEmu_ProcessEvent(uint8_t task_id, uint16_t events)
{
    (void)task_id;

    if(events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;
        if((pMsg = OSAL_MsgReceive(hidEmuTaskId)) != NULL)
        {
            hidEmu_ProcessTMOSMsg((tmos_event_hdr_t *)pMsg);
            OSAL_MsgDeallocate(pMsg);
        }
        return (events ^ SYS_EVENT_MSG);
    }

    if(events & START_DEVICE_EVT)
    {
        return (events ^ START_DEVICE_EVT);
    }

    if(events & START_PHY_UPDATE_EVT)
    {
        dprint("Send Phy Update %x...\n", GAPRole_UpdatePHY(hidEmuConnHandle, 0,
                                                            GAP_PHY_BIT_LE_2M, GAP_PHY_BIT_LE_2M, 0));
        return (events ^ START_PHY_UPDATE_EVT);
    }

    if(events & PERI_SECURITY_REQ_EVT)
    {
        uint8_t ble_state;
        GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
        if(ble_state == GAPROLE_CONNECTED)
        {
            dprint("Send Security Req ...\n");
            if(GAPBondMgr_PeriSecurityReq(hidEmuConnHandle))
            {
                OSAL_SetDelayedEvent(hidEmuTaskId, PERI_SECURITY_REQ_EVT, 4800);
            }
        }
        return (events ^ PERI_SECURITY_REQ_EVT);
    }

    if(events & WAIT_TERMINATE_EVT)
    {
        uint8_t ble_state;
        GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
        if(ble_state == GAPROLE_CONNECTED)
        {
            HCI_MB_DisconnectCmd(hidEmuConnHandle, 0x16);
        }
        return (events ^ WAIT_TERMINATE_EVT);
    }

    if(events & SEND_DISCONNECT_EVT)
    {
        uint8_t ble_state;
        uint8_t advertising_state;
        dprint("SEND_DISCONNECT_EVT\n");
        GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
        GAPRole_GetParameter(GAPROLE_ADVERT_ENABLED, &advertising_state);
        dprint("[TRACE_EVT] SEND_DISCONNECT_EVT state=%x adv=%x pairing=%x deep=%x\n",
               ble_state, advertising_state, access_state.pairing_state, access_state.deep_sleep_flag);
        if(ble_state != GAPROLE_CONNECTED)
        {
            BLE_buf_out_idx = 0;
            BLE_buf_data_num = 0;
            BLE_buf_resend_num = 0;
        }
        return (events ^ SEND_DISCONNECT_EVT);
    }

    if(events & SEND_PACKET_EVT)
    {
        uint8_t ble_state;
        GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
        if(ble_state == GAPROLE_CONNECTED)
        {
            /* Reserved: send logic is currently disabled in this branch. */
        }
        return (events ^ SEND_PACKET_EVT);
    }

    if(events & DELETE_PACKET_EVT)
    {
        return (events ^ DELETE_PACKET_EVT);
    }

    if(events & BLE_CLEAR_BUF_EVT)
    {
        uint8_t ble_state;
        GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
        if(ble_state != GAPROLE_CONNECTED)
        {
            BLE_buf_out_idx = 0;
            BLE_buf_data_num = 0;
            BLE_buf_resend_num = 0;
        }
        return (events ^ BLE_CLEAR_BUF_EVT);
    }

    return 0;
}

/*********************************************************************
 * @fn      hidEmu_ProcessTMOSMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void hidEmu_ProcessTMOSMsg(tmos_event_hdr_t *pMsg)
{
    switch(pMsg->event)
    {
        default:
            break;
    }
}
/*********************************************************************
 * @fn      hidEmu_disconnect
 *
 * @brief   强制断开连接
 *
 * @return  none.
 */
void hidEmu_disconnect()
{
    GAPRole_TerminateLink(hidEmuConnHandle);
    if( OSAL_GetTaskTimer( hidEmuTaskId, WAIT_TERMINATE_EVT ) )
    {

    }
    else
    {
        OSAL_SetDelayedEvent( hidEmuTaskId , WAIT_TERMINATE_EVT, TERMINATE_EVT_TIMEOUT );
    }
}

/*********************************************************************
 * @fn      hidEmu_delete_ble_bonded
 *
 * @brief   清楚当前模式的设备的绑定标志，存储相关绑定标志信息到flash
 *
 * @return  none.
 */
void hidEmu_delete_ble_bonded()
{
    storage_config_t *cfg = storage_get_config_ptr();
    uint8_t bond_flag = 0;

    if(cfg == NULL)
    {
        return;
    }

    switch(con_work_mode)
    {
        case BLE_INDEX_1:
            bond_flag = BLE_BOND_FLAG_1;
            break;
        case BLE_INDEX_2:
            bond_flag = BLE_BOND_FLAG_2;
            break;
        case BLE_INDEX_3:
            bond_flag = BLE_BOND_FLAG_3;
            break;
        default:
            dprint("work mode err %x\n",con_work_mode);
            return;
    }

    cfg->ble_bond_flag &= (uint8_t)(~bond_flag);
    cfg->ble_mac_flag &= (uint8_t)(~bond_flag);
    storage_save();
}

/*********************************************************************
 * @fn      hidEmu_save_ble_bonded
 *
 * @brief   存储相关绑定标志信息到flash，自动调用此函数，调用前已经将绑定信息存到flash中
 *
 * @return  none.
 */
void hidEmu_save_ble_bonded(uint8_t is_pairing)
{
    storage_config_t *cfg = storage_get_config_ptr();
    uint8_t bond_flag = 0;

    if(cfg == NULL)
    {
        dprint("[BOND] save failed: cfg=NULL\n");
        return;
    }

    switch(con_work_mode)
    {
        case BLE_INDEX_1:
            bond_flag = BLE_BOND_FLAG_1;
            break;
        case BLE_INDEX_2:
            bond_flag = BLE_BOND_FLAG_2;
            break;
        case BLE_INDEX_3:
            bond_flag = BLE_BOND_FLAG_3;
            break;
        default:
            dprint("[BOND] save failed: con_work_mode=%x invalid\n", con_work_mode);
            return;
    }
    dprint("[BOND] save: mode=%x is_pairing=%x flag=%x before=%x\n",
           con_work_mode, is_pairing, bond_flag, cfg->ble_bond_flag);
    cfg->ble_bond_flag |= bond_flag;
    dprint("[BOND] save: after=%x\n", cfg->ble_bond_flag);
    if(is_pairing)
    {
        cfg->ble_mac_flag ^= bond_flag;
    }
    storage_save();
}

/*********************************************************************
 * @fn      hidEmu_is_ble_mac_change
 *
 * @brief   判断mac地址是否需要+1
 *
 * @return  none.
 */
uint8_t hidEmu_is_ble_mac_change( access_ble_idx_t ble_idx )
{
    storage_config_t *cfg = storage_get_config_ptr();
    uint8_t bond_flag = 0;

    if(cfg == NULL)
    {
        return 0;
    }

    switch(ble_idx)
    {
        case BLE_INDEX_1:
            bond_flag = BLE_BOND_FLAG_1;
            break;
        case BLE_INDEX_2:
            bond_flag = BLE_BOND_FLAG_2;
            break;
        case BLE_INDEX_3:
            bond_flag = BLE_BOND_FLAG_3;
            break;
        default:
            dprint("work mode err %x\n",ble_idx);
            return 0;
    }

    return ((cfg->ble_mac_flag & bond_flag) != 0) ? TRUE : FALSE;
}

/*********************************************************************
 * @fn      hidEmu_is_ble_bonded
 *
 * @brief   判断是否绑定
 *
 * @return  none.
 */
uint8_t hidEmu_is_ble_bonded( access_ble_idx_t ble_idx )
{
    storage_config_t *cfg = storage_get_config_ptr();
    uint8_t bond_flag = 0;

    if(cfg == NULL)
    {
        return 0;
    }

    switch(ble_idx)
    {
        case BLE_INDEX_1:
            bond_flag = BLE_BOND_FLAG_1;
            break;
        case BLE_INDEX_2:
            bond_flag = BLE_BOND_FLAG_2;
            break;
        case BLE_INDEX_3:
            bond_flag = BLE_BOND_FLAG_3;
            break;
        default:
            dprint("[BOND] check failed: ble_idx=%x invalid\n", ble_idx);
            return 0;
    }
    uint8_t result = ((cfg->ble_bond_flag & bond_flag) != 0) ? TRUE : FALSE;
    dprint("[BOND] check: idx=%x flag=%x bond_flag=%x result=%x\n",
           ble_idx, bond_flag, cfg->ble_bond_flag, result);
    return result;
}

/*********************************************************************
 * @fn      hidEmu_adv_enable
 *
 * @brief   打开广播，并根据是否绑定选择开始过滤的广播，更换广播名称.
 *
 * @return  none.
 */
void hidEmu_adv_enable(uint8_t enable)
{
    uint8_t i,need_update=0;
    uint8_t ownAddr[6];
    uint8_t initial_advertising_enable = enable;
    uint8_t IRK[KEYLEN]={0};

    if(initial_advertising_enable)
    {
        uint8_t advertising_state;
        uint8_t RL_enable = TRUE;
        GAPRole_GetParameter( GAPROLE_ADVERT_ENABLED, &advertising_state );
        dprint("adv state %x\n",advertising_state);
        dprint("[TRACE_ADV] req=%x idx=%x pairing=%x deep=%x adv_before=%x\n",
               initial_advertising_enable, access_state.ble_idx,
               access_state.pairing_state, access_state.deep_sleep_flag, advertising_state);
        if( !advertising_state )
        {
            adv_enable_process_flag = TRUE;
        }
        ble_config.SNVAddr = (access_state.ble_idx - BLE_INDEX_1) * 0x100 + BLE_SNV_ADDR;
        GAPBondMgr_SetParameter( GAPBOND_AUTO_SYNC_RL, sizeof(uint8_t), &RL_enable );
        RL_enable = FALSE;
        GAPBondMgr_SetParameter( GAPBOND_AUTO_SYNC_RL, sizeof(uint8_t), &RL_enable );

        GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddr);
        // 修改广播地址。
        tmos_snv_read(BLE_NVID_IRK,KEYLEN,IRK);
        dprint("IRK %x %x %x %x %x %x\n",IRK[5],IRK[4],IRK[3],IRK[2],IRK[1],IRK[0]);
        GAPRole_SetParameter(GAPROLE_IRK, KEYLEN, IRK);
        dprint("%x %x %x %x %x %x\n",ownAddr[5],ownAddr[4],ownAddr[3],ownAddr[2],ownAddr[1],ownAddr[0]);
        ownAddr[4] += access_state.ble_idx;
        if( access_state.pairing_state )//要为true
        {
            // 如果更换标志未置位，则更换
            if(!hidEmu_is_ble_mac_change(access_state.ble_idx))
            {
                ownAddr[3] += access_state.ble_idx;
            }
        }
        else
        {
            // 如果更换标志已置位，则更换
            if(hidEmu_is_ble_mac_change(access_state.ble_idx))
            {
                ownAddr[3] += access_state.ble_idx;
            }
        }
        dprint("%x %x %x %x %x %x\n",ownAddr[5],ownAddr[4],ownAddr[3],ownAddr[2],ownAddr[1],ownAddr[0]);
        GAP_ConfigDeviceAddr(ADDRTYPE_STATIC, ownAddr);

        // 有需求是不同通道蓝牙名字不一样比如通道1名称为“BT-1”，通道2名称为“BT-2”等，则MCU发送是“BT-$”,这个美元符号表示蓝牙不同通道显示不同的名称
        // for(i=0; i<storage_get_config_ptr()->ble_name_len; i++ )
        // {
        //     if(storage_get_config_ptr()->ble_name_data[i]=='$')
        //     {
        //         scanRspData[2+i] = access_state.ble_idx+0x30-BLE_INDEX_IDEL;
        //         need_update = 1;
        //     }
        // }
        // if( need_update )
        // {
        //     dprint( "need_update\n");
        //     GGS_SetParameter(GGS_DEVICE_NAME_ATT, storage_get_config_ptr()->ble_name_len, (void *)&scanRspData[2]);
        // }
        // GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, storage_get_config_ptr()->ble_name_len+2, scanRspData);
        // memcpy(&advertData[13-2-4], scanRspData, storage_get_config_ptr()->ble_name_len+2);
        // GAPRole_SetParameter(GAPROLE_ADVERT_DATA, storage_get_config_ptr()->ble_name_len+2+13-2-4, advertData);//唤醒之后恢复广播导致广播包截断
    }
    uint8_t bonded = hidEmu_is_ble_bonded(access_state.ble_idx);
    dprint("[ADV] enable=%x pairing=%x bonded=%x idx=%x\n",
           initial_advertising_enable, access_state.pairing_state, bonded, access_state.ble_idx);
    if((initial_advertising_enable != 0) && (bonded != 0) && (!access_state.pairing_state) && (reconnect_adv_fallback_stage == 0))
    {
        uint8_t filter_policy = GAP_FILTER_POLICY_WHITE;
        GAPRole_SetParameter(GAPROLE_ADV_FILTER_POLICY, sizeof(uint8_t), &filter_policy);
        dprint("[ADV] WHITE LIST mode (reconnect)\n");
        // GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(reconAdvertData), reconAdvertData);
        // GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, sizeof(reconAdvertData), reconAdvertData);锟睫凤拷锟斤拷锟斤拷锟斤拷注锟斤拷使锟斤拷锟斤拷通锟姐播
        GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(advertData), advertData);
        GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, sizeof(scanRspData), scanRspData);
//        GAPRole_SetParameter(GAPROLE_ADVERT_DATA, 0, reconAdvertData);
    }
    else
    {
        uint8_t filter_policy = GAP_FILTER_POLICY_ALL;
        GAPRole_SetParameter(GAPROLE_ADV_FILTER_POLICY, sizeof(uint8_t), &filter_policy);
        if((initial_advertising_enable != 0) && (reconnect_adv_fallback_stage == 1) && (!access_state.pairing_state))
        {
            dprint("[ADV] GENERAL mode (fallback after WL timeout)\n");
        }
        else
        {
            dprint("[ADV] GENERAL mode (pairing or not bonded)\n");
        }
    }

    access_ble_cancel_deep_sleep_evt();
    if(initial_advertising_enable && (!access_state.pairing_state))
    {
        access_state.deep_sleep_flag = FALSE;
        access_ble_schedule_deep_sleep_evt(SYSTEM_DEEP_SLEEP_EVT_TIMEOUT);
        dprint("[TRACE_ADV] set SYSTEM_DEEP_SLEEP_EVT timeout=%d req=%x idx=%x\n",
               SYSTEM_DEEP_SLEEP_EVT_TIMEOUT, initial_advertising_enable, access_state.ble_idx);
    }
    else if(initial_advertising_enable == 0)
    {
        reconnect_adv_fallback_stage = 1;
    }

    GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &initial_advertising_enable);
    if(initial_advertising_enable)
    {
        access_ble_notify_advertising(access_state.pairing_state, access_state.ble_idx);
    }
}

/*********************************************************************
 * @fn      hidEmu_update_adv_data
 *
 * @brief   HID Dev update_adv_data.
 *
 * @return  none.
 */
uint8_t hidEmu_update_device_name()
{
    uint8_t i;
    scanRspData[0] = storage_get_config_ptr()->ble_name_len+1;
    scanRspData[1] = GAP_ADTYPE_LOCAL_NAME_COMPLETE;
    memcpy(&scanRspData[2], storage_get_config_ptr()->ble_name_data, storage_get_config_ptr()->ble_name_len);
    for(i=0; i<storage_get_config_ptr()->ble_name_len; i++ )
    {
        if(storage_get_config_ptr()->ble_name_data[i]=='$')
        {
            scanRspData[2+i] = access_state.ble_idx+0x30-BLE_INDEX_IDEL;
//                need_update = 1;
        }
    }
    GGS_SetParameter(GGS_DEVICE_NAME_ATT, storage_get_config_ptr()->ble_name_len, (void *)&scanRspData[2]);
    return GAP_UpdateAdvertisingData( hidEmuTaskId, FALSE, storage_get_config_ptr()->ble_name_len+2, scanRspData );
}

/*********************************************************************
 * @fn      hidEmu_resend_BUF
 *
 * @brief   hidEmu_resend_BUF
 *
 * @return  none
 */
void hidEmu_resend_BUF(void)
{
    while(BLE_buf_resend_num)
    {
        (BLE_buf_out_idx==0)?(BLE_buf_out_idx=(BLE_SEND_BUF_LEN-1)):(BLE_buf_out_idx--);
        BLE_send_buffer[BLE_buf_out_idx].resend = 0;
        BLE_buf_resend_num--;
//        dprint("re %d %d: %x %x %x\n",BLE_buf_out_idx,BLE_send_buffer[BLE_buf_out_idx].len,BLE_send_buffer[BLE_buf_out_idx].pData[0]
//                   ,BLE_send_buffer[BLE_buf_out_idx].pData[1],BLE_send_buffer[BLE_buf_out_idx].pData[2]);
    }
}

/*********************************************************************
 * @fn      hidEmu_receive
 *
 * @brief   hidEmu_receive
 *
 * @return  none
 */
uint8_t hidEmu_receive( uint8_t *pData, uint8_t len )
{
    uint8_t input_idx;
    uint8_t ble_state;
    //锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟阶刺?
    GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
    if((ble_state != GAPROLE_CONNECTED)&&
        (OSAL_GetTaskTimer(hidEmuTaskId, BLE_CLEAR_BUF_EVT)==0)
        /*&&((tmos_get_event(hidEmuTaskId)&BLE_CLEAR_BUF_EVT)==0)*/)
    {
        OSAL_SetDelayedEvent(hidEmuTaskId, BLE_CLEAR_BUF_EVT, 1600*5);
    }
    //锟斤拷榛猴拷锟斤拷锟斤拷欠锟斤拷锟?
    if(BLE_buf_data_num==BLE_SEND_BUF_LEN)
    {
        dprint("BLE_BUF FF\n");
        tmos_set_event(hidEmuTaskId, SEND_PACKET_EVT);
        return 0xFF;
    }
    //锟斤拷锟斤拷锟捷凤拷锟斤拷锟斤拷锟斤拷锟斤拷锟酵伙拷锟斤拷锟斤拷
    input_idx = (BLE_buf_out_idx+BLE_buf_data_num-BLE_buf_resend_num)%BLE_SEND_BUF_LEN;
    BLE_send_buffer[input_idx].len = len;
    BLE_send_buffer[input_idx].resend = 0;
    memcpy((BLE_send_buffer[input_idx].pData), pData, len);
    BLE_buf_data_num++;
    //锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟铰硷拷
    tmos_set_event(hidEmuTaskId, SEND_PACKET_EVT);
    return 0;

    dprint("RF %x %d\n",pData[0], len);
}

// /*********************************************************************
//  * @fn      hidEmu_class_keyboard_report
//  *
//  * @brief   Build and send a HID report.
//  *
//  * @return  none
//  */
// uint8_t hidEmu_class_keyboard_report(uint8_t *pData, uint8_t len)
// {
//     return HidDev_Report(HID_RPT_ID_CLASS_KEY_IN, HID_REPORT_TYPE_INPUT, len, pData);
// }

// /*********************************************************************
//  * @fn      hidEmu_mouse_report
//  *
//  * @brief   Build and send a HID report.
//  *
//  * @return  none
//  */
// uint8_t hidEmu_mouse_report(uint8_t *pData, uint8_t len)
// {
//     return HidDev_Report(HID_RPT_ID_MOUSE_IN, HID_REPORT_TYPE_INPUT, len, pData);
// }

// /*********************************************************************
//  * @fn      hidEmu_consumer_report
//  *
//  * @brief   Build and send a HID report.
//  *
//  * @return  none
//  */
// uint8_t hidEmu_consumer_report(uint8_t *pData, uint8_t len)
// {
//     return HidDev_Report(HID_RPT_ID_CONSUMER_IN, HID_REPORT_TYPE_INPUT, len, pData);
// }

// /*********************************************************************
//  * @fn      hidEmu_sys_ctl_report
//  *
//  * @brief   Build and send a HID report.
//  *
//  * @return  none
//  */
// uint8_t hidEmu_sys_ctl_report(uint8_t data)
// {
//     return HidDev_Report(HID_RPT_ID_SYS_CTL_IN, HID_REPORT_TYPE_INPUT, 1, &data);
// }

// /*********************************************************************
//  * @fn      hidEmu_all_keyboard_report
//  *
//  * @brief   Build and send a HID report.
//  *
//  * @return  none
//  */
// uint8_t hidEmu_all_keyboard_report(uint8_t *pData, uint8_t len)
// {
//     return HidDev_Report(HID_RPT_ID_ALL_KEY_IN, HID_REPORT_TYPE_INPUT, len, pData);
// }

// /*********************************************************************
//  * @fn      hidEmu_fn_report
//  *
//  * @brief   Build and send a HID report.
//  *
//  * @return  none
//  */
// uint8_t hidEmu_fn_report(uint8_t *pData, uint8_t len)
// {
//     return HidDev_Report(HID_RPT_ID_FN_IN, HID_REPORT_TYPE_INPUT, len, pData);
// }

// /*********************************************************************
//  * @fn      hidEmu_smart_wheel_report
//  *
//  * @brief   Build and send a HID report.
//  *
//  * @return  none
//  */
// uint8_t hidEmu_smart_wheel_report(uint8_t *pData, uint8_t len)
// {
//     return HidDev_Report(HID_RPT_ID_SMART_WHEEL_IN, HID_REPORT_TYPE_INPUT, len, pData);
// }
// Task ID
uint8_t access_taskId = INVALID_TASK_ID;
/*********************************************************************
 * @fn      hidEmuStateCB
 *
 * @brief   GAP state change callback.GAP角色状态回调函数（启动，广播，连接）
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void hidEmuStateCB(gapRole_States_t newState, gapRoleEvent_t *pEvent)
{
    switch(newState & GAPROLE_STATE_ADV_MASK)
    {
        case GAPROLE_STARTED://GAP角色已启动（外设，中心，广播）
        {
            start_device_over = TRUE;
            LL_SetDataRelatedAddressChanges( 1, 1 ) ;
//             if(access_taskId != INVALID_TASK_ID)//等待access任务注册成功
//             {
// // 初始化完成后，检查是否需要直接切换到对应连接  记录的  当前的  恢复成flash记录的通道
//                 if(storage_get_config_ptr()->ble_idx != access_state.ble_idx)
//                 {
//                     access_ctl_process( storage_get_config_ptr()->ble_idx);
//                 }
//             }
            dprint("1\n");
            access_ble_init_done();
            dprint("Initialized..\n");
        }
        break;

        case GAPROLE_ADVERTISING:
            if(pEvent->gap.opcode == GAP_MAKE_DISCOVERABLE_DONE_EVENT)
            {
                adv_enable_process_flag = FALSE;
                // 记下当前模式
                if((access_state.ble_idx<BLE_INDEX_1) || (access_state.ble_idx>=BLE_INDEX_MAX))
                {
                    dprint("ADV mode err.. %x\n",access_state.ble_idx);
                    // 状态错误，关闭广播
                    hidEmu_adv_enable(DISABLE);
                }
                else
                {
                    con_work_mode = access_state.ble_idx;
                }
                if( OSAL_GetTaskTimer( hidEmuTaskId, SEND_DISCONNECT_EVT )/* || (tmos_get_event(hidEmuTaskId)&SEND_DISCONNECT_EVT) */)
                {
                }
                else {
                    // last_led_data = 0xFF;
                }
                dprint("Advertising..\n");
            }
            break;

        case GAPROLE_CONNECTED:
            if(pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT)
            {
                gapEstLinkReqEvent_t *event = (gapEstLinkReqEvent_t *)pEvent;

                // get connection handle
                hidEmuConnHandle = event->connectionHandle;
                access_state.deep_sleep_flag = FALSE;
                reconnect_adv_fallback_stage = 1;
                access_ble_notify_connected(access_state.ble_idx);
                access_ble_cancel_deep_sleep_evt();
                centralConnHandle = event->connectionHandle;//锟斤拷锟斤拷锟斤拷锟接撅拷锟? 系统识锟斤拷使锟斤拷

                OSAL_SetDelayedEvent(hidEmuTaskId, PERI_SECURITY_REQ_EVT, 4800);//锟斤拷锟杰帮拷全锟斤拷锟斤拷
                OSAL_StopTask(hidEmuTaskId, BLE_CLEAR_BUF_EVT);
                if( OSAL_GetTaskTimer( hidEmuTaskId, SEND_DISCONNECT_EVT ) /*|| (tmos_get_event(hidEmuTaskId)&SEND_DISCONNECT_EVT)*/ )
                {
                  dprint("buf_num %d\n",BLE_buf_data_num);
                }
                else {
                  // 清空buff
                  BLE_buf_out_idx=0;
                  BLE_buf_data_num=0;
                  BLE_buf_resend_num=0;
                }
                dprint("Connected..int %x\n",event->connInterval);//锟斤拷印锟斤拷锟接硷拷锟?
//                {
//                    uint32_t time;
//                    time = RTC_GetCycle32k();
//                    time += WAKE_UP_RTC_MAX_TIME*10;
//                    if(time > 0xA8C00000)
//                    {
//                        time -= 0xA8C00000;
//                    }
//                    RTC_SetTignTime(time);
//                    // LOW POWER-sleep模式
//                    if((!RTCTigFlag)&&(GPIOB_ReadPortPin(bRXD1_)))
//                    {
//                        LowPower_Sleep(RB_PWR_RAM2K | RB_PWR_RAM30K | RB_PWR_EXTEND);
//                //        LowPower_Idle();
//                        GPIOA_ResetBits(bTXD0_);
//                        mDelaymS(1);
//                        BLE_RegInit();
//                        GPIOA_SetBits(bTXD0_);
//                        HSECFG_Current(HSE_RCur_100); // 降为额定电流(低功耗函数中提升了HSE偏置电流
//                    }
//                }
//                access_update_idel_sleep_timeout(0);
            }
            break;

        case GAPROLE_CONNECTED_ADV:
            if(pEvent->gap.opcode == GAP_MAKE_DISCOVERABLE_DONE_EVENT)
            {
                dprint("Connected Advertising..\n");
            }
            break;

        case GAPROLE_WAITING:
            if(pEvent->gap.opcode == GAP_END_DISCOVERABLE_DONE_EVENT)
            {
                // 1、切换到其他模式，命令停止的广播，则判断模式，不开启新的广播
                // 2、还是当前模式没变，只是limit广播自动停止，则继续广播(注意是否是OTA模式)
                // 3、切换到另外的蓝牙模式，命令停止的广播，则修改mac地址，判断是否已经绑定过，是则开启广播，并开启过滤，否则不开启广播，等待配对命令。
                dprint("con_mode %x\n",con_work_mode);
                if(access_state.deep_sleep_flag)
                {
                    dprint("Waiting for deep sleep, stop restart advertising\n");
                    break;
                }
                if((con_work_mode == access_state.ble_idx))
                {
                    if( access_state.ble_idx == BLE_INDEX_MAX)
                    {
                        uint8_t initial_advertising_enable = ENABLE;
                        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &initial_advertising_enable);
                    }
                    else
                    {
                        // limit广播自动停止，重新打开即可
                        hidEmu_adv_enable(ENABLE);
                    }
                }
                else if((access_state.ble_idx>BLE_INDEX_IDEL) && (access_state.ble_idx<BLE_INDEX_MAX) )
                {
                    if(hidEmu_is_ble_bonded(access_state.ble_idx))
                    {
                        if( access_state.pairing_state )
                        {
                            //access_tran_report(REPORT_CMD_STATE, STATE_PAIRING);
                        }
                        else
                        {
                            // 进入新蓝牙模式的回连状态
                            //access_tran_report(REPORT_CMD_STATE, STATE_RE_CONNECTING);
                        }
                        hidEmu_adv_enable(ENABLE);
                    }
                    else {
                        // 没绑定过，无法回连
                        con_work_mode = access_state.ble_idx;
                        //access_tran_report(REPORT_CMD_STATE, STATE_RE_CONNECT_FAIL);
                    }
                }
//                // 记下当前模式 睡眠后模式改为idel，所以这里不能同步模式
//                con_work_mode = access_state.ble_idx;
                dprint("Waiting for advertising..\n");
            }
            else if(pEvent->gap.opcode == GAP_LINK_TERMINATED_EVENT)
            {
                dprint("GAP_LINK_TERMINATED_EVENT..\n");
                dprint("[TRACE_DISC] terminate reason=%x con_work_mode=%x idx=%x pairing=%x deep=%x\n",
                       pEvent->linkTerminate.reason, con_work_mode, access_state.ble_idx,
                       access_state.pairing_state, access_state.deep_sleep_flag);
                reconnect_adv_fallback_stage = 1;
                access_ble_notify_disconnected(access_state.ble_idx, pEvent->linkTerminate.reason);
                if(OSAL_GetTaskTimer(hidEmuTaskId, WAIT_TERMINATE_EVT))
                {
                    OSAL_StopTask(hidEmuTaskId, WAIT_TERMINATE_EVT);
                }
            }
            else if(pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT)
            {
                dprint("Advertising timeout..\n");
            }
            // Enable advertising

            break;

        case GAPROLE_ERROR:
            dprint("Error %x %x..\n", pEvent->gap.opcode,pEvent->dataUpdate.adType);
            break;

        default:
            break;
    }
}

/*********************************************************************
 * @fn      hidEmu_led_cb_register
 *
 * @brief   hidEmu_led_cb_register
 *
 * @return  none
 */
void hidEmu_receive_cb_register(pfnHidEmuReceiveCB_t cback)
{
    hidEmu_receive_cb = cback;
}

/*********************************************************************
 * @fn      hidEmuRcvReport
 *
 * @brief   Process an incoming HID keyboard report.
 *
 * @param   len - Length of report.
 * @param   pData - Report data.
 *
 * @return  status
 */
static uint8_t hidEmuRcvReport(uint8_t len, uint8_t *pData)
{
    // verify data length
    if(len == HID_LED_OUT_RPT_LEN)
    {
        // 上报键盘状态灯
        if(hidEmu_receive_cb)
        {
            uint8_t buf[2];
            buf[0] = RF_DATA_LED;
            buf[1] = pData[0];
            hidEmu_receive_cb(buf, 2);
        }
        // set LEDs
        return SUCCESS;
    }
    else
    {
        return ATT_ERR_INVALID_VALUE_SIZE;
    }
}

/*********************************************************************
 * @fn      hidEmuRptCB
 *
 * @brief   HID Dev report callback.
 *
 * @param   id - HID report ID.
 * @param   type - HID report type.
 * @param   uuid - attribute uuid.
 * @param   oper - operation:  read, write, etc.
 * @param   len - Length of report.
 * @param   pData - Report data.
 *
 * @return  GATT status code.
 */
static uint8_t hidEmuRptCB(uint8_t id, uint8_t type, uint16_t uuid,
                           uint8_t oper, uint16_t *pLen, uint8_t *pData)
{
    uint8_t status = SUCCESS;
    // write
    if(oper == HID_DEV_OPER_WRITE)
    {
        if(uuid == REPORT_UUID)
        {
            // process write to LED output report; ignore others
            if(type == HID_REPORT_TYPE_OUTPUT)
            {
                status = hidEmuRcvReport(*pLen, pData);
            }
        }

        if(status == SUCCESS)
        {
            status = Hid_SetParameter(id, type, uuid, *pLen, pData);
        }
    }
    // read
    else if(oper == HID_DEV_OPER_READ)
    {
        status = Hid_GetParameter(id, type, uuid, pLen, pData);
    }
    // notifications enabled
    else if(oper == HID_DEV_OPER_ENABLE)
    {

    }
    return status;
}

/*********************************************************************
 * @fn      hidEmuEvtCB
 *
 * @brief   HID Dev event callback.
 *
 * @param   evt - event ID.
 *
 * @return  HID response code.
 */
static void hidEmuEvtCB(uint8_t evt)
{
    // process enter/exit suspend or enter/exit boot mode
    if(evt == HID_DEV_SUSPEND_EVT )
    {
#if(DEBUG_3MODE)
        dprint("HID_SUSPEND\n");
#endif
        dprint("HID_SUSPEND\n");
        hidDevBattCB(BATT_LEVEL_NOTI_DISABLED);
        access_ble_enter_idel_sleep();
    }
    else if(evt == HID_DEV_EXIT_SUSPEND_EVT )
    {
#if(DEBUG_3MODE)
        dprint("HID_EXIT_SUSPEND\n");
#endif
        dprint("HID_EXIT_SUSPEND\n");
        hidDevBattCB(BATT_LEVEL_NOTI_ENABLED);
    }
    return;
    return;
}
//系统识别相关变量
uint8_t sys_buf[3] = {0x0e,0X14,0X0C};

/*********************************************************************
 * @fn      Central_ProcessEvent
 * @brief   中心设备事件处理
 */
uint16_t Central_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;
        
        if((pMsg = OSAL_MsgReceive(centralTaskId)) != NULL)
        {
            central_ProcessTMOSMsg((tmos_event_hdr_t *)pMsg);
            OSAL_MsgDeallocate(pMsg);
        }
        return (events ^ SYS_EVENT_MSG);
    }
    
    if(events & START_SVC_DISCOVERY_EVT)
    {
        centralStartDiscovery();
        return (events ^ START_SVC_DISCOVERY_EVT);
    }
    
    if(events & START_READ_OR_WRITE_EVT)
    {
        attReadReq_t req;
        static uint8_t i = 0;
        
        req.handle = sys_buf[i];
        uint8_t ret = GATT_ReadCharValue(centralConnHandle, &req, centralTaskId);
        dprint("ReadCharValue RET:%x, handle:%02x\n", ret, req.handle);
        
        if(i >= 2){
            i = 0;
            OSAL_StopTask(centralTaskId, START_READ_OR_WRITE_EVT);
        }else{
            OSAL_SetDelayedEvent(centralTaskId, START_READ_OR_WRITE_EVT, 1600);
        }
        i++;
        
        return (events ^ START_READ_OR_WRITE_EVT);
    }
    
    return 0;
}

/*********************************************************************
 * @fn      central_ProcessTMOSMsg
 * @brief   处理TMOS消息
 */
static void central_ProcessTMOSMsg(tmos_event_hdr_t *pMsg)
{
    switch(pMsg->event)
    {
        case GATT_MSG_EVENT:
            centralProcessGATTMsg((gattMsgEvent_t *)pMsg);
            break;
    }
}

/*********************************************************************
 * @fn      centralProcessGATTMsg
 * @brief   处理GATT消息 - 用于系统识别逻辑
 */
static void centralProcessGATTMsg(gattMsgEvent_t *pMsg)
{
    // 处理MTU交换
    if((pMsg->method == ATT_EXCHANGE_MTU_RSP) ||
       ((pMsg->method == ATT_ERROR_RSP) &&
        (pMsg->msg.errorRsp.reqOpcode == ATT_EXCHANGE_MTU_REQ)))
    {
        if(pMsg->method == ATT_ERROR_RSP)
        {
            dprint("Exchange MTU Error: %x\n", pMsg->msg.errorRsp.errCode);
        }
        centralProcedureInProgress = FALSE;
    }
    
    if(pMsg->method == ATT_MTU_UPDATED_EVENT)
    {
        dprint("MTU: %x\n", pMsg->msg.mtuEvt.MTU);
    }
    
    //处理特征值读取响应
    if((pMsg->method == ATT_READ_RSP) ||
       ((pMsg->method == ATT_ERROR_RSP) &&
        (pMsg->msg.errorRsp.reqOpcode == ATT_READ_REQ)))
    {
        if(pMsg->method == ATT_ERROR_RSP)
        {
            dprint("Read Error: %x\n", pMsg->msg.errorRsp.errCode);
        }
        else
        {
            //系统识别判断逻辑
            if (pMsg->msg.readRsp.pValue[0] == 0x01 && 
                pMsg->msg.readRsp.pValue[1] == 0x06)
            {
                host_system_type = WIN;
                dprint("========== OS_WINDOWS ==========\r\n");
                // 根据系统类型切换层
                keyboard_update_base_layer_by_system();
            }
            else if (pMsg->msg.readRsp.pValue[0] == 0x69 && 
                     pMsg->msg.readRsp.pValue[1] == 0x50)
            {
                host_system_type = IOS;
                dprint("========== OS_IOS ==========\r\n");
                dprint("Device: %s\n", pMsg->msg.readRsp.pValue);
                // 根据系统类型切换层
                keyboard_update_base_layer_by_system();
            }
            else if (pMsg->msg.readRsp.pValue[0] == 0x4D && 
                     pMsg->msg.readRsp.pValue[1] == 0x61)
            {
                host_system_type = MAC;
                dprint("========== OS_MAC ==========\r\n");
                // 根据系统类型切换层
                keyboard_update_base_layer_by_system();
            }
            
        }
        centralProcedureInProgress = FALSE;
    }
    else if((pMsg->method == ATT_WRITE_RSP) ||
            ((pMsg->method == ATT_ERROR_RSP) &&
             (pMsg->msg.errorRsp.reqOpcode == ATT_WRITE_REQ)))
    {
        if(pMsg->method == ATT_ERROR_RSP)
        {
            dprint("Write Error: %x\n", pMsg->msg.errorRsp.errCode);
        }
        else
        {
            dprint("Write success \n");
        }
        centralProcedureInProgress = FALSE;
    }
    else if(pMsg->method == ATT_HANDLE_VALUE_NOTI)
    {
        dprint("Receive noti: %x\n", *pMsg->msg.handleValueNoti.pValue);
    }
    else if(centralDiscState != BLE_DISC_STATE_IDLE)
    {
        centralGATTDiscoveryEvent(pMsg);
    }
    
    GATT_bm_free(&pMsg->msg, pMsg->method);
}

#define PNP_ID_UUID 0x2A50                      // Windows
#define Device_Information_Service_UUID 0x180A  // 
#define MODEL_NUMBER_STRING_UUID 0x2A24         // iOS
#define MFG_NAME_UUID 0x2a29                    // Mac
/*********************************************************************
 * @fn      centralStartDiscovery
 * @brief   开始设备信息发现
 */
static void centralStartDiscovery(void)
{
    uint8_t uuid[ATT_BT_UUID_SIZE] = {
        LO_UINT16(Device_Information_Service_UUID),
        HI_UINT16(Device_Information_Service_UUID)
    };
    
    // Initialize cached handles
    centralSvcStartHdl = centralSvcEndHdl = centralCharHdl = 0;
    centralDiscState = BLE_DISC_STATE_SVC;
    
    // Discovery device information service
    uint8_t ret = GATT_DiscPrimaryServiceByUUID(centralConnHandle,
                                                 uuid,
                                                 ATT_BT_UUID_SIZE,
                                                 centralTaskId);
    dprint("GATT_DiscPrimaryServiceByUUID:%x\n", ret);
}

/*********************************************************************
 * @fn      centralGATTDiscoveryEvent
 * @brief   处理GATT发现事件
 */
static void centralGATTDiscoveryEvent(gattMsgEvent_t *pMsg)
{
    // Android识别：发现失败时判断为Android
    if(pMsg->method == ATT_ERROR_RSP)
    {
        if(pMsg->msg.errorRsp.reqOpcode == ATT_FIND_BY_TYPE_VALUE_REQ)
        {
            dprint("========== OS_ANDROID ==========\n");
            host_system_type = ANDROID;
            // 根据系统类型切换层
            keyboard_update_base_layer_by_system();
        }
    }
    
    // 继续读取特征值
    OSAL_SetDelayedEvent(centralTaskId, START_READ_OR_WRITE_EVT, 1600);
    return;
}
/*********************************************************************
*********************************************************************/
