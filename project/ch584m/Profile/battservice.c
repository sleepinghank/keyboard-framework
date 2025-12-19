/********************************** (C) COPYRIGHT *******************************
 * File Name          : battservice.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/12/10
 * Description        : 电池服务
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*********************************************************************
 * 包含头文件
 */
#include "CONFIG.h"
#include "hiddev.h"
#include "battservice.h"

/*********************************************************************
 * 宏定义
 */

/*********************************************************************
 * 常量定义
 */

// ADC电压等级
#define BATT_ADC_LEVEL_3V            409  // 3V对应的ADC值
#define BATT_ADC_LEVEL_2V            273  // 2V对应的ADC值

#define BATT_LEVEL_VALUE_IDX         2    // 电池电量值在属性数组中的位置
#define BATT_LEVEL_VALUE_CCCD_IDX    3    // 电池电量CCCD在属性数组中的位置

#define BATT_LEVEL_VALUE_LEN         1    // 电池电量值长度

/*********************************************************************
 * 类型定义
 */

/*********************************************************************
 * 全局变量
 */
// 电池服务UUID
const uint8_t battServUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(BATT_SERV_UUID), HI_UINT16(BATT_SERV_UUID)};

// 电池电量特征UUID
const uint8_t battLevelUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(BATT_LEVEL_UUID), HI_UINT16(BATT_LEVEL_UUID)};

/*********************************************************************
 * 外部变量
 */

/*********************************************************************
 * 外部函数
 */

/*********************************************************************
 * 本地变量
 */

// 应用回调函数
static battServiceCB_t battServiceCB;

// 测量设置回调函数
static battServiceSetupCB_t battServiceSetupCB = NULL;

// 测量拆卸回调函数
static battServiceTeardownCB_t battServiceTeardownCB = NULL;

// 测量计算回调函数
static battServiceCalcCB_t battServiceCalcCB = NULL;

static uint16_t battMinLevel = BATT_ADC_LEVEL_2V; // VDD/3测量的最小电压值
static uint16_t battMaxLevel = BATT_ADC_LEVEL_3V; // VDD/3测量的最大电压值

// 临界电池电量设置
static uint8_t battCriticalLevel;

// 用于读取的ADC通道
//static uint8_t battServiceAdcCh = HAL_ADC_CHANNEL_VDD;

/*********************************************************************
 * Profile属性 - 变量
 */

// 电池服务属性
static const gattAttrType_t battService = {ATT_BT_UUID_SIZE, battServUUID};

// 电池电量特征
static uint8_t       battLevelProps = GATT_PROP_READ | GATT_PROP_NOTIFY;  // 支持读取和通知
static uint8_t       battLevel = 100;  // 初始电量为100%
static gattCharCfg_t battLevelClientCharCfg[GATT_MAX_NUM_CONN];  // 客户端配置

// HID报告参考特征描述符,电池电量
static uint8_t hidReportRefBattLevel[HID_REPORT_REF_LEN] = {
    HID_RPT_ID_BATT_LEVEL_IN, HID_REPORT_TYPE_INPUT};

/*********************************************************************
 * Profile属性 - 表
 */

// 电池服务属性表
static gattAttribute_t battAttrTbl[] = {
    // 电池服务声明
    {
        {ATT_BT_UUID_SIZE, primaryServiceUUID}, /* 类型 */
        GATT_PERMIT_READ,                       /* 权限 */
        0,                                      /* 句柄 */
        (uint8_t *)&battService                 /* 值指针 */
    },

    // 电池电量特征声明
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &battLevelProps},

    // 电池电量特征值
    {
        {ATT_BT_UUID_SIZE, battLevelUUID},
        GATT_PERMIT_READ,
        0,
        &battLevel},

    // 电池电量客户端特征配置
    {
        {ATT_BT_UUID_SIZE, clientCharCfgUUID},
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8_t *)&battLevelClientCharCfg},

    // HID报告参考特征描述符,电池电量输入
    {
        {ATT_BT_UUID_SIZE, reportRefUUID},
        GATT_PERMIT_READ,
        0,
        hidReportRefBattLevel}
};

/*********************************************************************
 * 本地函数
 */
// 读取属性回调函数
static bStatus_t battReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint16_t maxLen, uint8_t method);
// 写入属性回调函数                                
static bStatus_t battWriteAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                 uint8_t *pValue, uint16_t len, uint16_t offset, uint8_t method);
// 通知回调函数                                 
static void      battNotifyCB(linkDBItem_t *pLinkItem);
// 电池电量测量函数
static uint8_t   battMeasure(void);
// 电池电量通知函数
static void      battNotifyLevel(void);

/*********************************************************************
 * Profile回调
 */
// 电池服务回调函数
gattServiceCBs_t battCBs = {
    battReadAttrCB,  // 读取回调函数指针
    battWriteAttrCB, // 写入回调函数指针
    NULL             // 授权回调函数指针
};

/*********************************************************************
 * 公共函数
 */

/*********************************************************************
 * @fn      Batt_AddService
 *
 * @brief   初始化电池服务,通过向GATT服务器注册GATT属性
 *
 * @return  成功或失败
 */
bStatus_t Batt_AddService(void)
{
    uint8_t status = SUCCESS;

    // 初始化客户端特征配置属性
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, battLevelClientCharCfg);

    // 向GATT服务器注册GATT属性列表和回调函数
    status = GATTServApp_RegisterService(battAttrTbl,
                                         GATT_NUM_ATTRS(battAttrTbl),
                                         GATT_MAX_ENCRYPT_KEY_SIZE,
                                         &battCBs);

    return (status);
}

/*********************************************************************
 * @fn      Batt_Register
 *
 * @brief   注册电池服务的回调函数
 *
 * @param   pfnServiceCB - 回调函数
 *
 * @return  无
 */
extern void Batt_Register(battServiceCB_t pfnServiceCB)
{
    battServiceCB = pfnServiceCB;
}

/*********************************************************************
 * @fn      Batt_SetParameter
 *
 * @brief   设置电池服务参数
 *
 * @param   param - 参数ID
 * @param   len - 数据长度
 * @param   value - 数据指针
 *
 * @return  bStatus_t
 */
bStatus_t Batt_SetParameter(uint8_t param, uint8_t len, void *value)
{
    bStatus_t ret = SUCCESS;

    switch(param)
    {
        case BATT_PARAM_CRITICAL_LEVEL:  // 设置临界电量
            battCriticalLevel = *((uint8_t *)value);

            // 如果当前电量低于临界值且未设置临界状态,则发送通知
            if(battLevel < battCriticalLevel)
            {
                battNotifyLevel();
            }
            break;

        default:
            ret = INVALIDPARAMETER;
            break;
    }

    return (ret);
}

/*********************************************************************
 * @fn      Batt_GetParameter
 *
 * @brief   获取电池服务参数
 *
 * @param   param - 参数ID
 * @param   value - 数据指针
 *
 * @return  bStatus_t
 */
bStatus_t Batt_GetParameter(uint8_t param, void *value)
{
    bStatus_t ret = SUCCESS;
    switch(param)
    {
        case BATT_PARAM_LEVEL:  // 获取电池电量
            *((uint8_t *)value) = battLevel;
            break;

        case BATT_PARAM_CRITICAL_LEVEL:  // 获取临界电量
            *((uint8_t *)value) = battCriticalLevel;
            break;

        case BATT_PARAM_SERVICE_HANDLE:  // 获取服务句柄
            *((uint16_t *)value) = GATT_SERVICE_HANDLE(battAttrTbl);
            break;

        case BATT_PARAM_BATT_LEVEL_IN_REPORT:  // 获取电池电量输入报告
        {
            hidRptMap_t *pRpt = (hidRptMap_t *)value;

            pRpt->id = hidReportRefBattLevel[0];
            pRpt->type = hidReportRefBattLevel[1];
            pRpt->handle = battAttrTbl[BATT_LEVEL_VALUE_IDX].handle;
            pRpt->cccdHandle = battAttrTbl[BATT_LEVEL_VALUE_CCCD_IDX].handle;
            pRpt->mode = HID_PROTOCOL_MODE_REPORT;
        }
        break;

        default:
            ret = INVALIDPARAMETER;
            break;
    }

    return (ret);
}

/*********************************************************************
 * @fn          Batt_MeasLevel
 *
 * @brief       测量电池电量并更新服务特征中的电池电量值。
 *              如果电池电量特征配置为通知且电池电量自上次测量以来发生变化,
 *              则发送通知。
 *
 * @return      Success
 */
bStatus_t Batt_MeasLevel(void)
{
    uint8_t level;

    level = battMeasure();

    // 如果电量降低
    if(level < battLevel)
    {
        // 更新电量
        battLevel = level;

        // 发送通知
        battNotifyLevel();
    }

    return SUCCESS;
}

/*********************************************************************
 * @fn      Batt_Setup
 *
 * @brief   设置要使用的ADC源。默认为VDD/3。
 *
 * @param   adc_ch - ADC通道,如HAL_ADC_CHN_AIN6
 * @param   minVal - 最小电池电量
 * @param   maxVal - 最大电池电量
 * @param   sCB - 硬件设置回调
 * @param   tCB - 硬件拆卸回调
 * @param   cCB - 百分比计算回调
 *
 * @return  无
 */
void Batt_Setup(uint8_t adc_ch, uint16_t minVal, uint16_t maxVal,
                battServiceSetupCB_t sCB, battServiceTeardownCB_t tCB,
                battServiceCalcCB_t cCB)
{
    //battServiceAdcCh = adc_ch;
    battMinLevel = minVal;
    battMaxLevel = maxVal;

    battServiceSetupCB = sCB;
    battServiceTeardownCB = tCB;
    battServiceCalcCB = cCB;
}

/*********************************************************************
 * @fn          battReadAttrCB
 *
 * @brief       读取属性回调函数
 *
 * @param       connHandle - 接收消息的连接句柄
 * @param       pAttr - 属性指针
 * @param       pValue - 要读取的数据指针
 * @param       pLen - 要读取的数据长度
 * @param       offset - 要读取的第一个字节的偏移量
 * @param       maxLen - 要读取的最大数据长度
 *
 * @return      Success或Failure
 */
static bStatus_t battReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint16_t maxLen, uint8_t method)
{
    uint16_t  uuid;
    bStatus_t status = SUCCESS;

    // 确保不是blob操作(profile中没有长属性)
    if(offset > 0)
    {
        return (ATT_ERR_ATTR_NOT_LONG);
    }

    uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);

    // 如果读取电量则测量电池电量
    if(uuid == BATT_LEVEL_UUID)
    {
        uint8_t level;

        level = battMeasure();

        // 如果电量降低
        if(level < battLevel)
        {
            // 更新电量
            battLevel = level;
        }

        *pLen = 1;
        pValue[0] = battLevel;
    }
    else if(uuid == GATT_REPORT_REF_UUID)
    {
        *pLen = HID_REPORT_REF_LEN;
        tmos_memcpy(pValue, pAttr->pValue, HID_REPORT_REF_LEN);
    }
    else
    {
        status = ATT_ERR_ATTR_NOT_FOUND;
    }

    return (status);
}

/*********************************************************************
 * @fn      battWriteAttrCB
 *
 * @brief   在写操作之前验证属性数据
 *
 * @param   connHandle - 接收消息的连接句柄
 * @param   pAttr - 属性指针
 * @param   pValue - 要写入的数据指针
 * @param   len - 数据长度
 * @param   offset - 要写入的第一个字节的偏移量
 *
 * @return  Success或Failure
 */
static bStatus_t battWriteAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                 uint8_t *pValue, uint16_t len, uint16_t offset, uint8_t method)
{
    bStatus_t status = SUCCESS;

    uint16_t uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);
    switch(uuid)
    {
        case GATT_CLIENT_CHAR_CFG_UUID:
            status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len,
                                                    offset, GATT_CLIENT_CFG_NOTIFY);
            if(status == SUCCESS)
            {
                uint16_t charCfg = BUILD_UINT16(pValue[0], pValue[1]);

                if(battServiceCB)
                {
                    (*battServiceCB)((charCfg == GATT_CFG_NO_OPERATION) ? BATT_LEVEL_NOTI_DISABLED : BATT_LEVEL_NOTI_ENABLED);
                }
            }
            break;

        default:
            status = ATT_ERR_ATTR_NOT_FOUND;
            break;
    }

    return (status);
}

/*********************************************************************
 * @fn          battNotifyCB
 *
 * @brief       发送电量状态特征的通知
 *
 * @param       connHandle - linkDB项
 *
 * @return      无
 */
static void battNotifyCB(linkDBItem_t *pLinkItem)
{
    if(pLinkItem->stateFlags & LINK_CONNECTED)
    {
        uint16_t value = GATTServApp_ReadCharCfg(pLinkItem->connectionHandle,
                                                 battLevelClientCharCfg);
        if(value & GATT_CLIENT_CFG_NOTIFY)
        {
            attHandleValueNoti_t noti;

            noti.pValue = GATT_bm_alloc(pLinkItem->connectionHandle, ATT_HANDLE_VALUE_NOTI,
                                        BATT_LEVEL_VALUE_LEN, NULL, 0);
            if(noti.pValue != NULL)
            {
                noti.handle = battAttrTbl[BATT_LEVEL_VALUE_IDX].handle;
                noti.len = BATT_LEVEL_VALUE_LEN;
                noti.pValue[0] = battLevel;

                if(GATT_Notification(pLinkItem->connectionHandle, &noti, FALSE) != SUCCESS)
                {
                    GATT_bm_free((gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI);
                }
            }
        }
    }
}

/*********************************************************************
 * @fn      battMeasure
 *
 * @brief   使用ADC测量电池电量并以0-100%的百分比返回
 *
 * @return  电池电量
 */
static uint8_t battMeasure(void)
{
    uint16_t adc;
    uint8_t  percent;

    // 调用测量设置回调
    if(battServiceSetupCB != NULL)
    {
        battServiceSetupCB();
    }

    // 配置ADC并执行读取
    adc = 300;
    // 调用测量拆卸回调
    if(battServiceTeardownCB != NULL)
    {
        battServiceTeardownCB();
    }

    if(adc >= battMaxLevel)
    {
        percent = 100;
    }
    else if(adc <= battMinLevel)
    {
        percent = 0;
    }
    else
    {
        if(battServiceCalcCB != NULL)
        {
            percent = battServiceCalcCB(adc);
        }
        else
        {
            uint16_t range = battMaxLevel - battMinLevel + 1;

            // 可选,如果要保持偶数,否则直接取除法的下限
            // range += (range & 1);
            range >>= 2; // 除以4

            percent = (uint8_t)((((adc - battMinLevel) * 25) + (range - 1)) / range);
        }
    }

    return percent;
}

/*********************************************************************
 * @fn      battNotifyLevel
 *
 * @brief   如果建立了连接,发送电池电量状态特征的通知
 *
 * @return  无
 */
static void battNotifyLevel(void)
{
    // 执行linkDB回调以发送通知
    linkDB_PerformFunc(battNotifyCB);
}

/*********************************************************************
 * @fn          Batt_HandleConnStatusCB
 *
 * @brief       电池服务链接状态变化处理函数
 *
 * @param       connHandle - 连接句柄
 * @param       changeType - 变化类型
 *
 * @return      无
 */
void Batt_HandleConnStatusCB(uint16_t connHandle, uint8_t changeType)
{
    // 确保不是回环连接
    if(connHandle != LOOPBACK_CONNHANDLE)
    {
        // 如果连接断开则重置客户端特征配置
        if((changeType == LINKDB_STATUS_UPDATE_REMOVED) ||
           ((changeType == LINKDB_STATUS_UPDATE_STATEFLAGS) &&
            (!linkDB_Up(connHandle))))
        {
            GATTServApp_InitCharCfg(connHandle, battLevelClientCharCfg);
        }
    }
}

/*********************************************************************
*********************************************************************/
