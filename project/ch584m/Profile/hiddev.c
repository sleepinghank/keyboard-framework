/********************************** (C) COPYRIGHT *******************************
 * 文件名称          : hiddev.c
 * 作者             : WCH
 * 版本             : V1.0
 * 日期             : 2018/12/10
 * 描述             : HID设备任务处理程序,包含HID设备初始化、事件处理、报告发送等功能
 *********************************************************************************
 * 版权所有 (c) 2021 南京沁恒微电子股份有限公司
 * 注意: 本软件(修改或未修改)及其二进制文件仅用于
 * 南京沁恒微电子制造的微控制器。
 *******************************************************************************/

/*********************************************************************
 * 包含头文件
 */
// 包含系统配置头文件
#include "CONFIG.h"
// 包含电池服务头文件
#include "battservice.h"
// 包含扫描参数服务头文件
#include "scanparamservice.h"
// 包含设备信息服务头文件
#include "devinfoservice.h"
// 包含HID键盘应用头文件
#include "hidkbd.h"
// 包含HID设备头文件
#include "hiddev.h"

#include "CONFIG.h"
#include "HAL.h"

#include "platform_ble.h"
#include "log_printf.h"

#define TAG "hiddev"
/*********************************************************************
 * 宏定义
 */

// 电池测量周期(单位:625us)
#define DEFAULT_BATT_PERIOD               15000

// 是否运行扫描参数刷新通知测试
#define DEFAULT_SCAN_PARAM_NOTIFY_TEST    TRUE

// 广播间隔(单位:625us)
// 初始广播最小间隔(48*0.625=30ms)
#define HID_INITIAL_ADV_INT_MIN           48
// 初始广播最大间隔(80*0.625=50ms)  
#define HID_INITIAL_ADV_INT_MAX           80
// 高频广播最小间隔(32*0.625=20ms)
#define HID_HIGH_ADV_INT_MIN              32
// 高频广播最大间隔(48*0.625=30ms)
#define HID_HIGH_ADV_INT_MAX              48
// 低频广播最小间隔(160*0.625=100ms)
#define HID_LOW_ADV_INT_MIN               160
// 低频广播最大间隔(160*0.625=100ms)
#define HID_LOW_ADV_INT_MAX               160

// 广播超时时间(单位:秒)
#define HID_INITIAL_ADV_TIMEOUT           60  // 初始广播超时时间
#define HID_HIGH_ADV_TIMEOUT              5   // 高频广播超时时间  
#define HID_LOW_ADV_TIMEOUT               0   // 低频广播超时时间(0表示不超时)

// HID设备任务事件
#define START_DEVICE_EVT                  0x0001  // 启动设备事件
#define BATT_PERIODIC_EVT                 0x0002  // 电池周期测量事件

/*********************************************************************
 * 常量定义
 */

/*********************************************************************
 * 类型定义
 */

/*********************************************************************
 * 全局变量
 */

// HID设备任务ID
uint8_t hidDevTaskId;

/*********************************************************************
 * 外部变量
 */

/*********************************************************************
 * 外部函数
 */

/*********************************************************************
 * 本地变量
 */

// GAP角色状态
static gapRole_States_t hidDevGapState = GAPROLE_INIT;

// 连接是否安全标志
static uint8_t hidDevConnSecure = FALSE;

// GAP连接句柄
static uint16_t gapConnHandle;

// 最近一次配对状态
static uint8_t pairingStatus = SUCCESS;

// HID报告映射表指针
static hidRptMap_t *pHidDevRptTbl;

// HID报告映射表长度
static uint8_t hidDevRptTblLen;

// HID设备回调函数指针
static hidDevCB_t *pHidDevCB;

// HID设备配置参数指针
static hidDevCfg_t *pHidDevCfg;

/*********************************************************************
 * 本地函数声明
 */

// TMOS消息处理函数
static void hidDev_ProcessTMOSMsg(tmos_event_hdr_t *pMsg);

// GATT消息处理函数 - 处理来自GATT层的消息
static void hidDevProcessGattMsg(gattMsgEvent_t *pMsg);

// GAP消息处理函数 - 处理来自GAP层的消息
static void hidDevProcessGAPMsg(gapRoleEvent_t *pEvent);

// 断开连接处理函数 - 处理设备断开连接时的清理工作
static void hidDevDisconnected(void);

// GAP状态变化回调函数 - 处理GAP角色状态变化
static void hidDevGapStateCB(gapRole_States_t newState, gapRoleEvent_t *pEvent);

// 连接参数更新回调函数 - 处理连接参数更新完成事件
static void hidDevParamUpdateCB(uint16_t connHandle, uint16_t connInterval,
                                uint16_t connSlaveLatency, uint16_t connTimeout);

// 配对状态回调函数 - 处理配对/绑定状态变化
static void hidDevPairStateCB(uint16_t connHandle, uint8_t state, uint8_t status);

// 配对密码回调函数 - 处理配对过程中的密码请求
static void hidDevPasscodeCB(uint8_t *deviceAddr, uint16_t connectionHandle,
                             uint8_t uiInputs, uint8_t uiOutputs);

// 电池服务回调函数 - 处理电池服务相关事件
static void hidDevBattCB(uint8_t event);

// 扫描参数服务回调函数 - 处理扫描参数服务相关事件
static void hidDevScanParamCB(uint8_t event);

// 电池周期性任务函数 - 执行电池电量定期测量
static void hidDevBattPeriodicTask(void);

// 通过句柄查找HID报告映射 - 根据ATT句柄查找对应的HID报告
static hidRptMap_t *hidDevRptByHandle(uint16_t handle);

// 通过ID和类型查找HID报告映射 - 根据报告ID和类型查找对应的HID报告
static hidRptMap_t *hidDevRptById(uint8_t id, uint8_t type);

// 通过CCCD句柄查找HID报告映射 - 根据客户端特征配置描述符句柄查找对应的HID报告
static hidRptMap_t *hidDevRptByCccdHandle(uint16_t handle);

// 发送HID报告函数 - 发送指定ID和类型的HID报告数据
static uint8_t hidDevSendReport(uint8_t id, uint8_t type, uint8_t len, uint8_t *pData);

// 高速广播函数 - 配置并启动高速广播
static void    hidDevHighAdvertising(void);

// 低速广播函数 - 配置并启动低速广播
static void    hidDevLowAdvertising(void);

// 初始广播函数 - 配置并启动初始连接的广播
static void    hidDevInitialAdvertising(void);

// 获取绑定设备数量函数 - 返回已绑定的设备总数
static uint8_t hidDevBondCount(void);

// 发送通知函数 - 通过指定句柄发送通知数据
static uint8_t HidDev_sendNoti(uint16_t handle, uint8_t len, uint8_t *pData);
/*********************************************************************
 * 配置文件回调
 */

// GAP外围设备角色回调
static gapRolesCBs_t hidDev_PeripheralCBs = {
    hidDevGapStateCB,     // GAP状态变化回调
    NULL,                 // 当从控制器读取到有效RSSI时的回调
    hidDevParamUpdateCB   // 连接参数更新回调
};

// GAP绑定管理器回调
static gapBondCBs_t hidDevBondCB = {
    hidDevPasscodeCB,     // 配对密码回调
    hidDevPairStateCB     // 配对状态回调
};

/*********************************************************************
 * 公共函数
 */

/*********************************************************************
 * @fn      HidDev_Init
 *
 * @brief   HID设备任务初始化函数。在初始化期间调用,用于初始化任务相关的
 *          参数,如硬件初始化/设置、表初始化、上电通知等。
 *
 * @param   task_id - TMOS分配的任务ID,用于发送消息和设置定时器
 *
 * @return  无
 */
void HidDev_Init()
{
    // 注册任务处理函数,获取任务ID
    hidDevTaskId = TMOS_ProcessEventRegister(HidDev_ProcessEvent);

    // 设置GAP绑定管理器参数
    {
        uint8_t syncWL = TRUE;

        // 如果创建了绑定,HID设备应该将HID主机的地址写入HID设备控制器的白名单,
        // 并将HID设备控制器的广播过滤策略设置为"仅处理来自白名单中设备的扫描和连接请求"
        GAPBondMgr_SetParameter(GAPBOND_AUTO_SYNC_WL, sizeof(uint8_t), &syncWL);
    }

    // 添加服务
    GGS_AddService(GATT_ALL_SERVICES);         // GAP服务
    GATTServApp_AddService(GATT_ALL_SERVICES); // GATT属性
    DevInfo_AddService();                      // 设备信息服务
    Batt_AddService();                         // 电池服务
    ScanParam_AddService();                    // 扫描参数服务

    // 注册电池服务回调
    Batt_Register(hidDevBattCB);

    // 注册扫描参数服务回调
    ScanParam_Register(hidDevScanParamCB);

    // 设置延迟启动配置文件事件
    tmos_set_event(hidDevTaskId, START_DEVICE_EVT);
}

/*********************************************************************
 * @fn      HidDev_ProcessEvent
 *
 * @brief   HID设备任务事件处理函数。此函数用于处理所有任务事件,包括定时器、
 *          消息和任何其他用户定义的事件。
 *
 * @param   task_id - TMOS分配的任务ID
 * @param   events - 要处理的事件,可以包含多个事件
 *
 * @return  未处理的事件
 */
uint16_t HidDev_ProcessEvent(uint8_t task_id, uint16_t events)
{
    //VOID task_id; // TMOS要求的参数,在此函数中未使用

    // 处理系统消息事件
    if(events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;

        if((pMsg = tmos_msg_receive(hidDevTaskId)) != NULL)
        {
            // 处理TMOS消息
            hidDev_ProcessTMOSMsg((tmos_event_hdr_t *)pMsg);

            // 释放TMOS消息
            tmos_msg_deallocate(pMsg);
        }

        // 返回未处理的事件
        return (events ^ SYS_EVENT_MSG);
    }

    // 处理启动设备事件
    if(events & START_DEVICE_EVT)
    {
        // 启动设备
        GAPRole_PeripheralStartDevice(hidDevTaskId, &hidDevBondCB, &hidDev_PeripheralCBs);

        return (events ^ START_DEVICE_EVT);
    }

    // 处理电池周期测量事件
    if(events & BATT_PERIODIC_EVT)
    {
        // 执行周期性电池测量任务
        hidDevBattPeriodicTask();

        return (events ^ BATT_PERIODIC_EVT);
    }

    // 返回0表示所有事件都已处理
    return 0;
}

/*********************************************************************
 * @fn      HidDev_Register
 *
 * @brief   注册HID设备回调函数
 *
 * @param   pCfg - 配置参数
 * @param   pfnServiceCB - 回调函数
 *
 * @return  无
 */
void HidDev_Register(hidDevCfg_t *pCfg, hidDevCB_t *pCBs)
{
    pHidDevCB = pCBs;
    pHidDevCfg = pCfg;
}

/*********************************************************************
 * @fn      HidDev_RegisterReports
 *
 * @brief   注册HID报告表
 *
 * @param   numReports - 报告表长度
 * @param   pRpt - 报告表
 *
 * @return  无
 */
void HidDev_RegisterReports(uint8_t numReports, hidRptMap_t *pRpt)
{
    pHidDevRptTbl = pRpt;
    hidDevRptTblLen = numReports;
}

/*********************************************************************
 * @fn      HidDev_Report
 *
 * @brief   发送HID报告
 *
 * @param   id - HID报告ID
 * @param   type - HID报告类型
 * @param   len - 报告长度
 * @param   pData - 报告数据
 *
 * @return  无
 */
uint8_t HidDev_Report(uint8_t id, uint8_t type, uint8_t len, uint8_t *pData)
{
    // 如果已连接
    if(hidDevGapState == GAPROLE_CONNECTED)
    {
        // 如果连接是安全的
        if(hidDevConnSecure)
        {
            // 发送报告
            return hidDevSendReport(id, type, len, pData);
        }
    }
    // 如果未在广播
    else if(hidDevGapState != GAPROLE_ADVERTISING)
    {
        // 如果已绑定
        if(hidDevBondCount() > 0)
        {
            // 开始高频广播
            hidDevHighAdvertising();
        }
        // 如果未绑定
        else
        {
            // 开始初始广播
            hidDevInitialAdvertising();
        }
    }
    return bleNotReady;
}

/*********************************************************************
 * @fn      HidDev_Close
 *
 * @brief   断开连接或停止广播
 *
 * @return  无
 */
void HidDev_Close(void)
{
    uint8_t param;

    // 如果已连接则断开连接
    if(hidDevGapState == GAPROLE_CONNECTED)
    {
        GAPRole_TerminateLink(gapConnHandle);
    }
    // 否则停止广播
    else
    {
        param = FALSE;
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &param);
    }
}

/*********************************************************************
 * @fn      HidDev_SetParameter
 *
 * @brief   设置HID设备参数
 *
 * @param   param - 配置文件参数ID
 * @param   len - 要写入数据的长度
 * @param   pValue - 指向要写入数据的指针。这取决于参数ID,
 *          并将被转换为适当的数据类型(例如:uint16_t类型的数据
 *          将被转换为uint16_t指针)。
 *
 * @return  bStatus_t
 */
bStatus_t HidDev_SetParameter(uint8_t param, uint8_t len, void *pValue)
{
    bStatus_t ret = SUCCESS;

    switch(param)
    {
        case HIDDEV_ERASE_ALLBONDS:
            if(len == 0)
            {
                // 断开连接
                if(hidDevGapState == GAPROLE_CONNECTED)
                {
                    GAPRole_TerminateLink(gapConnHandle);
                }

                // 擦除所有绑定信息
                GAPBondMgr_SetParameter(GAPBOND_ERASE_ALLBONDS, 0, NULL);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;

        default:
            ret = INVALIDPARAMETER;
            break;
    }

    return (ret);
}

/*********************************************************************
 * @fn      HidDev_GetParameter
 *
 * @brief   获取HID设备参数
 *
 * @param   param - 配置文件参数ID
 * @param   pValue - 指向获取数据的指针。这取决于参数ID,
 *          并将被转换为适当的数据类型(例如:uint16_t类型的数据
 *          将被转换为uint16_t指针)。
 *
 * @return  bStatus_t
 */
bStatus_t HidDev_GetParameter(uint8_t param, void *pValue)
{
    bStatus_t ret = SUCCESS;

    switch(param)
    {
        default:
            ret = INVALIDPARAMETER;
            break;
    }

    return (ret);
}

/*********************************************************************
 * @fn      HidDev_PasscodeRsp
 *
 * @brief   响应配对密码请求
 *
 * @param   status - 如果密码可用则为SUCCESS,否则参见@ref SMP_PAIRING_FAILED_DEFINES
 * @param   passcode - 包含密码的整数值
 *
 * @return  无
 */
void HidDev_PasscodeRsp(uint8_t status, uint32_t passcode)
{
    // 发送密码响应
    GAPBondMgr_PasscodeRsp(gapConnHandle, status, passcode);
}

/*********************************************************************
 * @fn          HidDev_ReadAttrCB
 *
 * @brief       HID设备属性读取回调函数
 *
 * @param       connHandle - 接收消息的连接句柄
 * @param       pAttr - 指向属性的指针
 * @param       pValue - 指向要读取数据的指针
 * @param       pLen - 要读取数据的长度
 * @param       offset - 要读取的第一个字节的偏移量
 * @param       maxLen - 可以读取的最大数据长度
 *
 * @return      成功或失败
 */
bStatus_t HidDev_ReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                            uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint16_t maxLen, uint8_t method)
{
    bStatus_t    status = SUCCESS;
    hidRptMap_t *pRpt;

    uint16_t uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);

    // 只有报告映射是长属性
    if(offset > 0 && uuid != REPORT_MAP_UUID)
    {
        return (ATT_ERR_ATTR_NOT_LONG);
    }

    if(uuid == REPORT_UUID ||
       uuid == BOOT_KEY_INPUT_UUID ||
       uuid == BOOT_KEY_OUTPUT_UUID ||
       uuid == BOOT_MOUSE_INPUT_UUID)
    {
        // 在表中查找报告ID
        if((pRpt = hidDevRptByHandle(pAttr->handle)) != NULL)
        {
            // 执行报告回调
            status = (*pHidDevCB->reportCB)(pRpt->id, pRpt->type, uuid,
                                            HID_DEV_OPER_READ, pLen, pValue);
        }
        else
        {
            *pLen = 0;
        }
    }
    else if(uuid == REPORT_MAP_UUID)
    {
        // 验证偏移量
        if(offset >= hidReportMapLen)
        {
            status = ATT_ERR_INVALID_OFFSET;
        }
        else
        {
            // 确定读取长度
            *pLen = MIN(maxLen, (hidReportMapLen - offset));

            // 复制数据
            tmos_memcpy(pValue, pAttr->pValue + offset, *pLen);
        }
    }
    else if(uuid == HID_INFORMATION_UUID)
    {
        *pLen = HID_INFORMATION_LEN;
        tmos_memcpy(pValue, pAttr->pValue, HID_INFORMATION_LEN);
    }
    else if(uuid == GATT_REPORT_REF_UUID)
    {
        *pLen = HID_REPORT_REF_LEN;
        tmos_memcpy(pValue, pAttr->pValue, HID_REPORT_REF_LEN);
    }
    else if(uuid == PROTOCOL_MODE_UUID)
    {
        *pLen = HID_PROTOCOL_MODE_LEN;
        pValue[0] = pAttr->pValue[0];
    }
    else if(uuid == GATT_EXT_REPORT_REF_UUID)
    {
        *pLen = HID_EXT_REPORT_REF_LEN;
        tmos_memcpy(pValue, pAttr->pValue, HID_EXT_REPORT_REF_LEN);
    }

    return (status);
}

/*********************************************************************
 * @fn      HidDev_WriteAttrCB
 *
 * @brief   HID设备属性写入回调函数
 *
 * @param   connHandle - 接收消息的连接句柄
 * @param   pAttr - 指向属性的指针
 * @param   pValue - 指向要写入数据的指针
 * @param   len - 数据长度
 * @param   offset - 要写入的第一个字节的偏移量
 *
 * @return  成功或失败
 */
bStatus_t HidDev_WriteAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                             uint8_t *pValue, uint16_t len, uint16_t offset, uint8_t method)
{
    uint16_t     uuid;
    bStatus_t    status = SUCCESS;
    hidRptMap_t *pRpt;

    // 确保不是块操作(配置文件中没有长属性)
    if(offset > 0)
    {
        return (ATT_ERR_ATTR_NOT_LONG);
    }

    uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);

    if(uuid == REPORT_UUID ||
       uuid == BOOT_KEY_OUTPUT_UUID)
    {
        // 在表中查找报告ID
        if((pRpt = hidDevRptByHandle(pAttr->handle)) != NULL)
        {
            // 执行报告回调
            status = (*pHidDevCB->reportCB)(pRpt->id, pRpt->type, uuid,
                                            HID_DEV_OPER_WRITE, &len, pValue);
        }
    }
    else if(uuid == HID_CTRL_PT_UUID)
    {
        // 验证长度和值范围
        if(len == 1)
        {
            if(pValue[0] == HID_CMD_SUSPEND || pValue[0] == HID_CMD_EXIT_SUSPEND)
            {
                // 执行HID应用事件回调
                (*pHidDevCB->evtCB)((pValue[0] == HID_CMD_SUSPEND) ? HID_DEV_SUSPEND_EVT : HID_DEV_EXIT_SUSPEND_EVT);
            }
            else
            {
                status = ATT_ERR_INVALID_VALUE;
            }
        }
        else
        {
            status = ATT_ERR_INVALID_VALUE_SIZE;
        }
    }
    else if(uuid == GATT_CLIENT_CHAR_CFG_UUID)
    {
        status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len,
                                                offset, GATT_CLIENT_CFG_NOTIFY);
        if(status == SUCCESS)
        {
            uint16_t charCfg = BUILD_UINT16(pValue[0], pValue[1]);

            // 在表中查找报告ID
            if((pRpt = hidDevRptByCccdHandle(pAttr->handle)) != NULL)
            {
                // 执行报告回调
                (*pHidDevCB->reportCB)(pRpt->id, pRpt->type, uuid,
                                       (charCfg == GATT_CLIENT_CFG_NOTIFY) ? HID_DEV_OPER_ENABLE : HID_DEV_OPER_DISABLE,
                                       &len, pValue);
            }
        }
    }
    else if(uuid == PROTOCOL_MODE_UUID)
    {
        if(len == HID_PROTOCOL_MODE_LEN)
        {
            if(pValue[0] == HID_PROTOCOL_MODE_BOOT ||
               pValue[0] == HID_PROTOCOL_MODE_REPORT)
            {
                pAttr->pValue[0] = pValue[0];

                // 执行HID应用事件回调
                (*pHidDevCB->evtCB)((pValue[0] == HID_PROTOCOL_MODE_BOOT) ? HID_DEV_SET_BOOT_EVT : HID_DEV_SET_REPORT_EVT);
            }
            else
            {
                status = ATT_ERR_INVALID_VALUE;
            }
        }
        else
        {
            status = ATT_ERR_INVALID_VALUE_SIZE;
        }
    }

    return (status);
}

/*********************************************************************
 * @fn      hidDev_ProcessTMOSMsg
 *
 * @brief   处理传入的任务消息
 *
 * @param   pMsg - 要处理的消息
 *
 * @return  无
 */
static void hidDev_ProcessTMOSMsg(tmos_event_hdr_t *pMsg)
{
    switch(pMsg->event)
    {
        case GATT_MSG_EVENT:
        {
            // 处理GATT消息
            hidDevProcessGattMsg((gattMsgEvent_t *)pMsg);
            break;
        }

        case GAP_MSG_EVENT:
        {
            // 处理GAP消息
            hidDevProcessGAPMsg((gapRoleEvent_t *)pMsg);
            break;
        }

        default:
            break;
    }
}

/*********************************************************************
 * @fn      hidDevProcessGattMsg
 *
 * @brief   处理GATT消息
 *
 * @return  无
 */
static void hidDevProcessGattMsg(gattMsgEvent_t *pMsg)
{
}

/*********************************************************************
 * @fn      hidDevProcessGAPMsg
 *
 * @brief   处理传入的任务消息
 *
 * @param   pMsg - 要处理的消息
 *
 * @return  无
 */
static void hidDevProcessGAPMsg(gapRoleEvent_t *pEvent)
{
    switch(pEvent->gap.opcode)
    {
        case GAP_SCAN_REQUEST_EVENT:
        {
            LOG_INFO(TAG , "收到扫描请求,地址:");//
            for(int i = 0; i < B_ADDR_LEN; i++)
                LOG_DEBUG_EX("%02x ", pEvent->scanReqEvt.scannerAddr[i]);
            LOG_INFO(TAG , "\n");
            break;
        }

        case GAP_PHY_UPDATE_EVENT:
        {
            LOG_INFO(TAG , "PHY更新 Rx:%x Tx:%x ..\n", pEvent->linkPhyUpdate.connRxPHYS, pEvent->linkPhyUpdate.connTxPHYS);
            break;
        }
        default:
            break;
    }
}

/*********************************************************************
 * @fn          hidDevHandleConnStatusCB
 *
 * @brief       重置客户端特征配置
 *
 * @param       connHandle - 连接句柄
 * @param       changeType - 变化类型
 *
 * @return      无
 */
static void hidDevHandleConnStatusCB(uint16_t connHandle, uint8_t changeType)
{
    uint8_t          i;
    hidRptMap_t     *p = pHidDevRptTbl;
    uint16_t         retHandle;
    gattAttribute_t *pAttr;

    // 确保不是回环连接
    if(connHandle != LOOPBACK_CONNHANDLE)
    {
        if((changeType == LINKDB_STATUS_UPDATE_REMOVED) ||
           ((changeType == LINKDB_STATUS_UPDATE_STATEFLAGS) &&
            (!linkDB_Up(connHandle))))
        {
            for(i = hidDevRptTblLen; i > 0; i--, p++)
            {
                if(p->cccdHandle != 0)
                {
                    if((pAttr = GATT_FindHandle(p->cccdHandle, &retHandle)) != NULL)
                    {
                        GATTServApp_InitCharCfg(connHandle, (gattCharCfg_t *)pAttr->pValue);
                    }
                }
            }
        }
    }
}

/*********************************************************************
 * @fn      hidDevDisconnected
 *
 * @brief   处理断开连接
 *
 * @return  无
 */
static void hidDevDisconnected(void)
{
    // 重置客户端特征配置描述符
    Batt_HandleConnStatusCB(gapConnHandle, LINKDB_STATUS_UPDATE_REMOVED);
    ScanParam_HandleConnStatusCB(gapConnHandle, LINKDB_STATUS_UPDATE_REMOVED);
    hidDevHandleConnStatusCB(gapConnHandle, LINKDB_STATUS_UPDATE_REMOVED);

    // 重置状态变量
    hidDevConnSecure = FALSE;
    hidProtocolMode = HID_PROTOCOL_MODE_REPORT;

    // 如果已绑定且设置为正常可连接,则开始广播
    if((hidDevBondCount() > 0) &&
       (pHidDevCfg->hidFlags & HID_FLAGS_NORMALLY_CONNECTABLE))
    {
        hidDevLowAdvertising();
    }
}

/*********************************************************************
 * @fn      hidDevGapStateCB
 *
 * @brief   GAP状态变化的通知回调函数
 *
 * @param   newState - 新的状态
 * @param   pEvent - 事件参数
 *
 * @return  无
 */
static void hidDevGapStateCB(gapRole_States_t newState, gapRoleEvent_t *pEvent)
{
    // 如果已连接
    if(newState == GAPROLE_CONNECTED)
    {
        gapEstLinkReqEvent_t *event = (gapEstLinkReqEvent_t *)pEvent;

        // 获取连接句柄
        gapConnHandle = event->connectionHandle;

        // 连接还未安全
        hidDevConnSecure = FALSE;
    }
    // 如果断开连接
    else if(hidDevGapState == GAPROLE_CONNECTED &&
            newState != GAPROLE_CONNECTED)
    {
        hidDevDisconnected();

        if(pairingStatus == SMP_PAIRING_FAILED_CONFIRM_VALUE)
        {
            // 由于确认值不匹配导致绑定失败
            hidDevInitialAdvertising();

            pairingStatus = SUCCESS;
        }
    }
    // 如果已启动
    else if(newState == GAPROLE_STARTED)
    {
        // 暂时无需处理
    }

    if(pHidDevCB && pHidDevCB->pfnStateChange)
    {
        // 执行HID应用状态变化回调
        (*pHidDevCB->pfnStateChange)(newState, pEvent);
    }

    hidDevGapState = newState;
}

/*********************************************************************
 * @fn      hidDevParamUpdateCB
 *
 * @brief   连接参数更新完成的回调函数
 *
 * @param   connHandle - 连接句柄
 * @param   connInterval - 连接间隔
 * @param   connSlaveLatency - 从机延迟
 * @param   connTimeout - 连接超时
 *
 * @return  无
 */
static void hidDevParamUpdateCB(uint16_t connHandle, uint16_t connInterval,
                                uint16_t connSlaveLatency, uint16_t connTimeout)
{
    LOG_INFO(TAG , "连接参数更新 %d - Int 0x%x - Latency %d\n", connHandle, connInterval, connSlaveLatency);
}

/*********************************************************************
 * @fn      hidDevPairStateCB
 *
 * @brief   配对状态回调函数
 *
 * @return  无
 */
static void hidDevPairStateCB(uint16_t connHandle, uint8_t state, uint8_t status)
{
    if(state == GAPBOND_PAIRING_STATE_COMPLETE)
    {
        if(status == SUCCESS)
        {
            hidDevConnSecure = TRUE;
        }

        pairingStatus = status;
    }
    else if(state == GAPBOND_PAIRING_STATE_BONDED)
    {
        if(status == SUCCESS)
        {
            hidDevConnSecure = TRUE;

#if DEFAULT_SCAN_PARAM_NOTIFY_TEST == TRUE
            ScanParam_RefreshNotify(gapConnHandle);
#endif
        }
    }
    else if(state == GAPBOND_PAIRING_STATE_BOND_SAVED)
    {
    }
}

/*********************************************************************
 * @fn      hidDevPasscodeCB
 *
 * @brief   配对密码回调函数
 *
 * @param   deviceAddr - 要配对设备的地址,可以是公共地址或随机地址
 * @param   connectionHandle - 连接句柄
 * @param   uiInputs - 配对用户界面输入 - 要求用户输入密码
 * @param   uiOutputs - 配对用户界面输出 - 显示密码
 *
 * @return  无
 */
static void hidDevPasscodeCB(uint8_t *deviceAddr, uint16_t connectionHandle,
                             uint8_t uiInputs, uint8_t uiOutputs)
{
    if(pHidDevCB && pHidDevCB->passcodeCB)
    {
        // 执行HID应用密码回调
        (*pHidDevCB->passcodeCB)(deviceAddr, connectionHandle, uiInputs, uiOutputs);
    }
    else
    {
        uint32_t passkey;
        GAPBondMgr_GetParameter(GAPBOND_PERI_DEFAULT_PASSCODE, &passkey);

        // 发送密码响应
        GAPBondMgr_PasscodeRsp(connectionHandle, SUCCESS, passkey);
    }
}

/*********************************************************************
 * @fn      hidDevBattCB
 *
 * @brief   电池服务的回调函数
 *
 * @param   event - 服务事件
 *
 * @return  无
 */
static void hidDevBattCB(uint8_t event)
{
    if(event == BATT_LEVEL_NOTI_ENABLED)
    {
        tmos_start_task(hidDevTaskId, BATT_PERIODIC_EVT, DEFAULT_BATT_PERIOD);
    }
    else if(event == BATT_LEVEL_NOTI_DISABLED)
    {
        // 停止周期性测量
        tmos_stop_task(hidDevTaskId, BATT_PERIODIC_EVT);
    }
}

/*********************************************************************
 * @fn      hidDevScanParamCB
 *
 * @brief   扫描参数服务的回调函数
 *
 * @param   event - 服务事件
 *
 * @return  无
 */
static void hidDevScanParamCB(uint8_t event)
{
}

/*********************************************************************
 * @fn      hidDevBattPeriodicTask
 *
 * @brief   执行电池测量的周期性任务
 *
 * @param   无
 *
 * @return  无
 */
static void hidDevBattPeriodicTask(void)
{
    // 执行电池电量检查
    Batt_MeasLevel();

    // 重启定时器
    tmos_start_task(hidDevTaskId, BATT_PERIODIC_EVT, DEFAULT_BATT_PERIOD);
}

/*********************************************************************
 * @fn      hidDevRptByHandle
 *
 * @brief   根据给定句柄查找HID报告结构
 *
 * @param   handle - ATT句柄
 *
 * @return  指向HID报告结构的指针
 */
static hidRptMap_t *hidDevRptByHandle(uint16_t handle)
{
    uint8_t      i;
    hidRptMap_t *p = pHidDevRptTbl;

    for(i = hidDevRptTblLen; i > 0; i--, p++)
    {
        if(p->handle == handle && p->mode == hidProtocolMode)
        {
            return p;
        }
    }

    return NULL;
}

/*********************************************************************
 * @fn      hidDevRptByCccdHandle
 *
 * @brief   根据给定的CCC句柄查找HID报告结构
 *
 * @param   handle - ATT句柄
 *
 * @return  指向HID报告结构的指针
 */
static hidRptMap_t *hidDevRptByCccdHandle(uint16_t handle)
{
    uint8_t      i;
    hidRptMap_t *p = pHidDevRptTbl;

    for(i = hidDevRptTblLen; i > 0; i--, p++)
    {
        if(p->cccdHandle == handle)
        {
            return p;
        }
    }

    return NULL;
}

/*********************************************************************
 * @fn      hidDevRptById
 *
 * @brief   根据报告ID和类型查找HID报告结构
 *
 * @param   id - HID报告ID
 * @param   type - HID报告类型
 *
 * @return  指向HID报告结构的指针
 */
static hidRptMap_t *hidDevRptById(uint8_t id, uint8_t type)
{
    uint8_t      i;
    hidRptMap_t *p = pHidDevRptTbl;

    for(i = hidDevRptTblLen; i > 0; i--, p++)
    {
        if(p->id == id && p->type == type && p->mode == hidProtocolMode)
        {
            return p;
        }
    }

    return NULL;
}

/*********************************************************************
 * @fn      hidDevSendReport
 *
 * @brief   发送HID报告
 *
 * @param   id - HID报告ID
 * @param   type - HID报告类型
 * @param   len - 报告长度
 * @param   pData - 报告数据
 *
 * @return  无
 */
static uint8_t hidDevSendReport(uint8_t id, uint8_t type, uint8_t len, uint8_t *pData)
{
    hidRptMap_t     *pRpt;
    gattAttribute_t *pAttr;
    uint16_t         retHandle;
    uint8_t          state = bleNoResources;

    // 获取报告的att句柄
    if((pRpt = hidDevRptById(id, type)) != NULL)
    {
        // 如果通知已使能
        if((pAttr = GATT_FindHandle(pRpt->cccdHandle, &retHandle)) != NULL)
        {
            uint16_t value;

            value = GATTServApp_ReadCharCfg(gapConnHandle, (gattCharCfg_t *)pAttr->pValue);
            if(value & GATT_CLIENT_CFG_NOTIFY)
            {
                // 发送报告通知
                state = HidDev_sendNoti(pRpt->handle, len, pData);
            }
        }
    }
    return state;
}

/*********************************************************************
 * @fn      hidDevSendNoti
 *
 * @brief   发送HID通知
 *
 * @param   handle - 属性句柄
 * @param   len - 报告长度
 * @param   pData - 报告数据
 *
 * @return  成功或失败
 */
static uint8_t HidDev_sendNoti(uint16_t handle, uint8_t len, uint8_t *pData)
{
    uint8_t              status;
    attHandleValueNoti_t noti;

    noti.pValue = GATT_bm_alloc(gapConnHandle, ATT_HANDLE_VALUE_NOTI, len, NULL, 0);
    if(noti.pValue != NULL)
    {
        noti.handle = handle;
        noti.len = len;
        tmos_memcpy(noti.pValue, pData, len);

        // 发送通知
        status = GATT_Notification(gapConnHandle, &noti, FALSE);
        if(status != SUCCESS)
        {
            GATT_bm_free((gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI);
        }
    }
    else
    {
        status = bleMemAllocError;
    }

    return status;
}

/*********************************************************************
 * @fn      hidDevHighAdvertising
 *
 * @brief   以高占空比开始广播
 *
 * @param   无
 *
 * @return  无
 */
static void hidDevHighAdvertising(void)
{
    uint8_t param;

    GAP_SetParamValue(TGAP_DISC_ADV_INT_MIN, HID_HIGH_ADV_INT_MIN);
    GAP_SetParamValue(TGAP_DISC_ADV_INT_MAX, HID_HIGH_ADV_INT_MAX);
    GAP_SetParamValue(TGAP_LIM_ADV_TIMEOUT, HID_HIGH_ADV_TIMEOUT);

    param = TRUE;
    GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &param);
}

/*********************************************************************
 * @fn      hidDevLowAdvertising
 *
 * @brief   以低占空比开始广播
 *
 * @param   无
 *
 * @return  无
 */
static void hidDevLowAdvertising(void)
{
    uint8_t param;

    GAP_SetParamValue(TGAP_DISC_ADV_INT_MIN, HID_LOW_ADV_INT_MIN);
    GAP_SetParamValue(TGAP_DISC_ADV_INT_MAX, HID_LOW_ADV_INT_MAX);
    GAP_SetParamValue(TGAP_LIM_ADV_TIMEOUT, HID_LOW_ADV_TIMEOUT);

    param = TRUE;
    GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &param);
}

/*********************************************************************
 * @fn      hidDevInitialAdvertising
 *
 * @brief   开始初始连接的广播
 *
 * @return  无
 */
static void hidDevInitialAdvertising(void)
{
    uint8_t param;

    GAP_SetParamValue(TGAP_DISC_ADV_INT_MIN, HID_INITIAL_ADV_INT_MIN);//设置广播间隔最小值
    GAP_SetParamValue(TGAP_DISC_ADV_INT_MAX, HID_INITIAL_ADV_INT_MAX);//设置广播间隔最大值
    GAP_SetParamValue(TGAP_LIM_ADV_TIMEOUT, HID_INITIAL_ADV_TIMEOUT);//设置广播超时时间

    param = TRUE;
    GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &param);//设置广播使能
}

/*********************************************************************
 * @fn      hidDevBondCount
 *
 * @brief   获取已绑定设备的总数
 *
 * @param   无
 *
 * @return  已绑定设备的数量
 */
static uint8_t hidDevBondCount(void)
{
    uint8_t bondCnt = 0;

    GAPBondMgr_GetParameter(GAPBOND_BOND_COUNT, &bondCnt);

    return (bondCnt);
}

/*********************************************************************
*********************************************************************/
//设置广播内容
void hidDevSetAdvData(void)
{
    uint8_t advertData[32] = {0};
    uint8_t advertDataLen = 0;

    advertData[advertDataLen++] = 0x02;//数据长度
    advertData[advertDataLen++] = GAP_ADTYPE_FLAGS;//AD类型


    advertData[advertDataLen++] = GAP_ADTYPE_FLAGS_LIMITED | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED;//发现模式：LE有限可发现模式 | BR/EDR不支持


    advertData[advertDataLen++] = 0x03;//数据长度
    advertData[advertDataLen++] = GAP_ADTYPE_APPEARANCE;//AD类型
    advertData[advertDataLen++] = LO_UINT16(GAP_APPEARE_HID_KEYBOARD);//HID键盘
    advertData[advertDataLen++] = HI_UINT16(GAP_APPEARE_HID_KEYBOARD);

    
    // 设备名称
    advertData[advertDataLen++] = sizeof(BLE_NAME);
    advertData[advertDataLen++] = GAP_ADTYPE_LOCAL_NAME_COMPLETE;
    memcpy(advertData + advertDataLen, BLE_NAME, sizeof(BLE_NAME));
    advertDataLen += sizeof(BLE_NAME);


}





void port_ble_init(void)
{
    LOG_INFO(TAG, "port_ble_init");
    CH58x_BLEInit();
    HAL_Init();
    GAPRole_PeripheralInit();
    HidDev_Init();
    HidEmu_Init();
}

//开始广播
void port_ble_start_advertising(ble_adv_type_t type)
{   uint8_t param;

    LOG_INFO(TAG, "port_ble_start_advertising");
    switch (type)
    {
    case BLE_ADV_TYPE_DISCOVERABLE:
        LOG_INFO(TAG, "发现广播");
        break;
    case BLE_ADV_TYPE_CONNECTABLE_UNDIRECTED:
        LOG_INFO(TAG, "可连接的广播");
        break;
    case BLE_ADV_TYPE_CONNECTABLE_DIRECTED:
        LOG_INFO(TAG, "可连接的定向广播");
        break;
    default:
        LOG_INFO(TAG, "未知广播类型");
        break;
    }

    hidDevSetAdvData();

    GAP_SetParamValue(TGAP_DISC_ADV_INT_MIN, HID_INITIAL_ADV_INT_MIN);//设置广播间隔最小值
    GAP_SetParamValue(TGAP_DISC_ADV_INT_MAX, HID_INITIAL_ADV_INT_MAX);//设置广播间隔最大值
    GAP_SetParamValue(TGAP_LIM_ADV_TIMEOUT, HID_INITIAL_ADV_TIMEOUT);//设置广播超时时间
    
    param = TRUE;
    GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &param);//设置广播使能
}


//停止广播
void port_ble_stop_advertising(void)
{
    LOG_INFO(TAG, "port_ble_stop_advertising");
    uint8_t param;
    param = FALSE;
    GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &param);//设置广播使能
}

