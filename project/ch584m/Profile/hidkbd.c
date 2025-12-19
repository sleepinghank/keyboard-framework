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
 * 包含的头文件
 */
#include "CONFIG.h"           // 配置头文件
#include "log_printf.h"       // 添加日志打印头文件
#include "devinfoservice.h"   // 设备信息服务头文件
#include "battservice.h"      // 电池服务头文件
#include "hidkbdservice.h"    // HID键盘服务头文件
#include "hiddev.h"          // HID设备头文件
#include "hidkbd.h"          // HID键盘头文件

#define TAG "hidkbd"

/*********************************************************************
 * 宏定义
 */
// HID键盘输入报告长度
#define HID_KEYBOARD_IN_RPT_LEN              8

// HID LED输出报告长度
#define HID_LED_OUT_RPT_LEN                  1

/*********************************************************************
 * 常量定义
 */
// 参数更新延迟时间
#define START_PARAM_UPDATE_EVT_DELAY         12800

// PHY层更新延迟时间
#define START_PHY_UPDATE_DELAY               1600

// HID空闲超时时间(毫秒)，设为0则禁用超时
#define DEFAULT_HID_IDLE_TIMEOUT             60000

// 最小连接间隔(单位1.25ms)
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL    8

// 最大连接间隔(单位1.25ms)
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL    8

// 从设备延迟参数
#define DEFAULT_DESIRED_SLAVE_LATENCY        0

// 监督超时时间(单位10ms)
#define DEFAULT_DESIRED_CONN_TIMEOUT         500

// 默认配对密码
#define DEFAULT_PASSCODE                     0

// 默认GAP配对模式
#define DEFAULT_PAIRING_MODE                 GAPBOND_PAIRING_MODE_WAIT_FOR_REQ

// 默认MITM模式(TRUE表示配对时需要密码或OOB)
#define DEFAULT_MITM_MODE                    FALSE

// 默认绑定模式，TRUE表示允许绑定
#define DEFAULT_BONDING_MODE                 TRUE

// 默认GAP绑定IO能力
#define DEFAULT_IO_CAPABILITIES              GAPBOND_IO_CAP_NO_INPUT_NO_OUTPUT

// 电池电量低于此百分比时视为电量严重不足
#define DEFAULT_BATT_CRITICAL_LEVEL          6

/*********************************************************************
 * 类型定义
 */

/*********************************************************************
 * 全局变量
 */

// 任务ID
static uint8_t hidEmuTaskId = INVALID_TASK_ID;

/*********************************************************************
 * 外部变量
 */

/*********************************************************************
 * 外部函数
 */

/*********************************************************************
 * 本地变量
 */

// GAP配置文件 - 用于扫描响应数据的名称属性
static uint8_t scanRspData[] = {
    0x0D,                           // 数据长度
    GAP_ADTYPE_LOCAL_NAME_COMPLETE, // AD类型 = 完整本地名称
    'H',
    'I',
    'D',
    ' ',
    'K',
    'e',
    'y',
    'b',
    'r',
    'o',
    'a',
    'd',  // 连接间隔范围
    0x05, // 数据长度
    GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
    LO_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL), // 100ms
    HI_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
    LO_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL), // 1s
    HI_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),

    // 服务UUID
    0x05, // 数据长度
    GAP_ADTYPE_16BIT_MORE,
    LO_UINT16(HID_SERV_UUID),
    HI_UINT16(HID_SERV_UUID),
    LO_UINT16(BATT_SERV_UUID),
    HI_UINT16(BATT_SERV_UUID),

    // 发射功率等级
    0x02, // 数据长度
    GAP_ADTYPE_POWER_LEVEL,
    0 // 0dBm
};

// 广播数据
static uint8_t advertData[] = {
    // 标志
    0x02, // 数据长度
    GAP_ADTYPE_FLAGS,
    GAP_ADTYPE_FLAGS_LIMITED | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

    // 外观特征
    0x03, // 数据长度
    GAP_ADTYPE_APPEARANCE,
    LO_UINT16(GAP_APPEARE_HID_KEYBOARD),
    HI_UINT16(GAP_APPEARE_HID_KEYBOARD)};

// 设备名称属性值
static CONST uint8_t attDeviceName[GAP_DEVICE_NAME_LEN] = "HID Keyboard";

// HID设备配置
static hidDevCfg_t hidEmuCfg = {
    DEFAULT_HID_IDLE_TIMEOUT, // 空闲超时
    HID_FEATURE_FLAGS         // HID特性标志
};

// 连接句柄
static uint16_t hidEmuConnHandle = GAP_CONNHANDLE_INIT;

/*********************************************************************
 * 本地函数声明
 */

// 处理TMOS消息
static void    hidEmu_ProcessTMOSMsg(tmos_event_hdr_t *pMsg);
// 发送键盘报告
static void    hidEmuSendKbdReport(uint8_t keycode);
// 接收报告回调
static uint8_t hidEmuRcvReport(uint8_t len, uint8_t *pData);
// HID报告回调
static uint8_t hidEmuRptCB(uint8_t id, uint8_t type, uint16_t uuid,
                           uint8_t oper, uint16_t *pLen, uint8_t *pData);
// HID事件回调                           
static void    hidEmuEvtCB(uint8_t evt);
// 状态改变回调
static void    hidEmuStateCB(gapRole_States_t newState, gapRoleEvent_t *pEvent);

/*********************************************************************
 * 配置文件回调
 */

// HID设备回调结构体
static hidDevCB_t hidEmuHidCBs = {
    hidEmuRptCB,    // 报告回调
    hidEmuEvtCB,    // 事件回调
    NULL,           // 保留
    hidEmuStateCB   // 状态回调
};

/*********************************************************************
 * 公共函数
 */

/*********************************************************************
 * @fn      HidEmu_Init
 *
 * @brief   HID键盘应用任务的初始化函数。在初始化期间调用，应包含任何
 *          特定于应用程序的初始化(如硬件初始化/设置、表初始化、
 *          上电通知等)。
 *
 * @param   task_id - TMOS分配的任务ID。此ID应用于发送消息和设置定时器。
 *
 * @return  无
 */
void HidEmu_Init()
{
    // 注册任务处理函数,获取任务ID
    hidEmuTaskId = TMOS_ProcessEventRegister(HidEmu_ProcessEvent);

    // 设置GAP外围设备角色配置文件
    {
        // 初始广播使能标志
        uint8_t initial_advertising_enable = TRUE;

        // 设置GAP角色参数
        // 使能广播
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &initial_advertising_enable);//启用/禁用广告。读/写。大小为uint8_t。默认值为TRUE=启用。

        // 设置广播数据和扫描响应数据
        GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(advertData), advertData);
        GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, sizeof(scanRspData), scanRspData);
    }

    // 设置GAP特征值 - 设备名称
    GGS_SetParameter(GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, (void *)attDeviceName);

    // 设置GAP绑定管理器参数
    {
        uint32_t passkey = DEFAULT_PASSCODE;      // 默认配对密码
        uint8_t  pairMode = DEFAULT_PAIRING_MODE; // 默认配对模式
        uint8_t  mitm = DEFAULT_MITM_MODE;        // 默认MITM保护
        uint8_t  ioCap = DEFAULT_IO_CAPABILITIES; // 默认IO能力
        uint8_t  bonding = DEFAULT_BONDING_MODE;  // 默认绑定模式
        
        // 设置各项绑定参数
        GAPBondMgr_SetParameter(GAPBOND_PERI_DEFAULT_PASSCODE, sizeof(uint32_t), &passkey);
        GAPBondMgr_SetParameter(GAPBOND_PERI_PAIRING_MODE, sizeof(uint8_t), &pairMode);
        GAPBondMgr_SetParameter(GAPBOND_PERI_MITM_PROTECTION, sizeof(uint8_t), &mitm);
        GAPBondMgr_SetParameter(GAPBOND_PERI_IO_CAPABILITIES, sizeof(uint8_t), &ioCap);
        GAPBondMgr_SetParameter(GAPBOND_PERI_BONDING_ENABLED, sizeof(uint8_t), &bonding);
    }

    // 设置电池服务特征值
    {
        uint8_t critical = DEFAULT_BATT_CRITICAL_LEVEL; // 默认电池临界值
        Batt_SetParameter(BATT_PARAM_CRITICAL_LEVEL, sizeof(uint8_t), &critical);
    }

    // 添加HID键盘服务
    Hid_AddService();

    // 注册HID设备回调
    HidDev_Register(&hidEmuCfg, &hidEmuHidCBs);

    // 设置延迟启动配置文件事件
    tmos_set_event(hidEmuTaskId, START_DEVICE_EVT);
}

/*********************************************************************
 * @fn      HidEmu_ProcessEvent
 *
 * @brief   HID键盘应用任务事件处理函数。此函数用于处理所有任务事件。
 *          事件包括定时器、消息和任何其他用户定义的事件。
 *
 * @param   task_id  - TMOS分配的任务ID
 * @param   events - 要处理的事件。这是一个位图,可以包含多个事件。
 *
 * @return  未处理的事件
 */
uint16_t HidEmu_ProcessEvent(uint8_t task_id, uint16_t events)
{
    // 用于发送按键的字符计数器
    static uint8_t send_char = 4;

    // 处理系统消息事件
    if(events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;

        // 接收消息
        if((pMsg = tmos_msg_receive(hidEmuTaskId)) != NULL)
        {
            // 处理TMOS消息
            hidEmu_ProcessTMOSMsg((tmos_event_hdr_t *)pMsg);

            // 释放TMOS消息
            tmos_msg_deallocate(pMsg);
        }

        // 返回未处理的事件
        return (events ^ SYS_EVENT_MSG);
    }

    // 处理启动设备事件
    if(events & START_DEVICE_EVT)
    {
        return (events ^ START_DEVICE_EVT);
    }

    // 处理连接参数更新事件
    if(events & START_PARAM_UPDATE_EVT)
    {
        // 发送连接参数更新请求
        GAPRole_PeripheralConnParamUpdateReq(hidEmuConnHandle,
                                             DEFAULT_DESIRED_MIN_CONN_INTERVAL,
                                             DEFAULT_DESIRED_MAX_CONN_INTERVAL,
                                             DEFAULT_DESIRED_SLAVE_LATENCY,
                                             DEFAULT_DESIRED_CONN_TIMEOUT,
                                             hidEmuTaskId);

        return (events ^ START_PARAM_UPDATE_EVT);
    }

    // 处理PHY更新事件
    if(events & START_PHY_UPDATE_EVT)
    {
        // 启动PHY更新
        LOG_INFO(TAG , "发送Phy更新 %x...\n", GAPRole_UpdatePHY(hidEmuConnHandle, 0, 
                    GAP_PHY_BIT_LE_2M, GAP_PHY_BIT_LE_2M, 0));

        return (events ^ START_PHY_UPDATE_EVT);
    }

    // 处理报告发送事件
    if(events & START_REPORT_EVT)
    {
        // 发送键盘按键报告
        hidEmuSendKbdReport(send_char);
        send_char++;
        if(send_char >= 29)
            send_char = 4;
        // 发送按键释放报告
        hidEmuSendKbdReport(0x00);
        // 2秒后再次触发报告事件
        tmos_start_task(hidEmuTaskId, START_REPORT_EVT, 2000);
        return (events ^ START_REPORT_EVT);
    }
    return 0;
}

/*********************************************************************
 * @fn      hidEmu_ProcessTMOSMsg
 *
 * @brief   处理接收到的任务消息
 *
 * @param   pMsg - 要处理的消息
 *
 * @return  无
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
 * @fn      hidEmuSendKbdReport
 *
 * @brief   构建并发送HID键盘报告
 *
 * @param   keycode - HID按键码
 *
 * @return  无
 */
static void hidEmuSendKbdReport(uint8_t keycode)
{
    uint8_t buf[HID_KEYBOARD_IN_RPT_LEN];

    buf[0] = 0;       // 修饰键(Ctrl,Alt等)
    buf[1] = 0;       // 保留字节
    buf[2] = keycode; // 按键码1
    buf[3] = 0;       // 按键码2
    buf[4] = 0;       // 按键码3
    buf[5] = 0;       // 按键码4
    buf[6] = 0;       // 按键码5
    buf[7] = 0;       // 按键码6

    // 发送输入报告
    HidDev_Report(HID_RPT_ID_KEY_IN, HID_REPORT_TYPE_INPUT,
                  HID_KEYBOARD_IN_RPT_LEN, buf);
}

/*********************************************************************
 * @fn      hidEmuStateCB
 *
 * @brief   GAP状态改变回调函数
 *
 * @param   newState - 新状态
 * @param   pEvent - 状态事件
 *
 * @return  无
 */
static void hidEmuStateCB(gapRole_States_t newState, gapRoleEvent_t *pEvent)
{
    switch(newState & GAPROLE_STATE_ADV_MASK)
    {
        case GAPROLE_STARTED:
        {
            uint8_t ownAddr[6];
            // 获取本机蓝牙地址
            GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddr);
            // 配置设备地址
            GAP_ConfigDeviceAddr(ADDRTYPE_STATIC, ownAddr);
            LOG_INFO(TAG , "已初始化...\n");
        }
        break;

        case GAPROLE_ADVERTISING:
            if(pEvent->gap.opcode == GAP_MAKE_DISCOVERABLE_DONE_EVENT)
            {
                LOG_INFO(TAG , "广播...\n");
            }
            break;

        case GAPROLE_CONNECTED:
            if(pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT)
            {
                gapEstLinkReqEvent_t *event = (gapEstLinkReqEvent_t *)pEvent;

                // 获取连接句柄
                hidEmuConnHandle = event->connectionHandle;
                // 启动参数更新定时器
                tmos_start_task(hidEmuTaskId, START_PARAM_UPDATE_EVT, START_PARAM_UPDATE_EVT_DELAY);
                LOG_INFO(TAG , "连接..\n");
            }
            break;

        case GAPROLE_CONNECTED_ADV:
            if(pEvent->gap.opcode == GAP_MAKE_DISCOVERABLE_DONE_EVENT)
            {
                LOG_INFO(TAG , "回连广播...\n");
            }
            break;
        case GAPROLE_WAITING:
            if(pEvent->gap.opcode == GAP_END_DISCOVERABLE_DONE_EVENT)
            {
                LOG_INFO(TAG , "等待广播...\n");
            }
            else if(pEvent->gap.opcode == GAP_LINK_TERMINATED_EVENT)
            {
                const char* reason_str;
                switch(pEvent->linkTerminate.reason)
                {
                    case 0x08:
                        reason_str = "连接超时";
                        break;
                    case 0x13:
                        reason_str = "远端设备主动断开";
                        break;
                    case 0x16:
                        reason_str = "连接终止由本地主机发起";
                        break;
                    case 0x22:
                        reason_str = "连接参数不被接受"; 
                        break;
                    case 0x3B:
                        reason_str = "连接参数超出范围";
                        break;
                    default:
                        reason_str = "未知原因";
                        break;
                }
                LOG_INFO(TAG , "断开连接.. 原因:%s(0x%02x)\n", reason_str, pEvent->linkTerminate.reason);
            }
            else if(pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT)
            {
                LOG_INFO(TAG , "广播超时...\n");
            }
            // 重新使能广播
            {
                uint8_t initial_advertising_enable = TRUE;
                GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &initial_advertising_enable);
            }
            break;

        case GAPROLE_ERROR:
            LOG_ERROR(TAG, "错误 %x ..\n", (int)pEvent->gap.opcode);
            break;

        default:
            break;
    }
}

/*********************************************************************
 * @fn      hidEmuRcvReport
 *
 * @brief   处理接收到的HID键盘报告
 *
 * @param   len - 报告长度
 * @param   pData - 报告数据
 *
 * @return  处理状态
 */
static uint8_t hidEmuRcvReport(uint8_t len, uint8_t *pData)
{
    // 验证数据长度
    if(len == HID_LED_OUT_RPT_LEN)
    {
        // 设置LED状态
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
 * @brief   HID设备报告回调函数
 *
 * @param   id - HID报告ID
 * @param   type - HID报告类型
 * @param   uuid - 属性UUID
 * @param   oper - 操作类型:读、写等
 * @param   len - 报告长度
 * @param   pData - 报告数据
 *
 * @return  GATT状态码
 */
static uint8_t hidEmuRptCB(uint8_t id, uint8_t type, uint16_t uuid,
                           uint8_t oper, uint16_t *pLen, uint8_t *pData)
{
    uint8_t status = SUCCESS;

    // 写操作
    if(oper == HID_DEV_OPER_WRITE)
    {
        if(uuid == REPORT_UUID)
        {
            // 处理LED输出报告的写入,忽略其他
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
    // 读操作
    else if(oper == HID_DEV_OPER_READ)
    {
        status = Hid_GetParameter(id, type, uuid, pLen, pData);
    }
    // 通知使能
    else if(oper == HID_DEV_OPER_ENABLE)
    {
        tmos_start_task(hidEmuTaskId, START_REPORT_EVT, 500);
    }
    return status;
}

/*********************************************************************
 * @fn      hidEmuEvtCB
 *
 * @brief   HID设备事件回调函数
 *
 * @param   evt - 事件ID
 *
 * @return  HID响应码
 */
static void hidEmuEvtCB(uint8_t evt)
{
    // 处理进入/退出挂起或进入/退出引导模式
    return;
}

/*********************************************************************
*********************************************************************/
