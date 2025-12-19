/********************************** (C) COPYRIGHT *******************************
 * 文件名称          : scanparamservice.c
 * 作者             : WCH
 * 版本             : V1.0
 * 日期             : 2018/12/10
 * 描述             : 扫描参数服务,用于主机向从机通知扫描参数变化
 *********************************************************************************
 * 版权所有 (c) 2021 南京沁恒微电子股份有限公司
 * 注意: 本软件(修改或未修改)及其二进制文件仅用于
 * 南京沁恒微电子制造的微控制器。
 *******************************************************************************/

/*********************************************************************
 * 包含头文件
 */
#include "CONFIG.h"
#include "scanparamservice.h"

/*********************************************************************
 * 宏定义
 */

/*********************************************************************
 * 常量定义
 */

/*********************************************************************
 * 类型定义
 */

/*********************************************************************
 * 全局变量
 */
// 扫描参数服务UUID
const uint8_t scanParamServUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(SCAN_PARAM_SERV_UUID), HI_UINT16(SCAN_PARAM_SERV_UUID)};

// 扫描间隔窗口特征UUID
const uint8_t scanIntervalWindowUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(SCAN_INTERVAL_WINDOW_UUID), HI_UINT16(SCAN_INTERVAL_WINDOW_UUID)};

// 扫描参数刷新特征UUID
const uint8_t scanParamRefreshUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(SCAN_REFRESH_UUID), HI_UINT16(SCAN_REFRESH_UUID)};

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
static scanParamServiceCB_t scanParamServiceCB;

/*********************************************************************
 * Profile属性 - 变量
 */

// 扫描参数服务属性
static const gattAttrType_t scanParamService = {ATT_BT_UUID_SIZE, scanParamServUUID};

// 扫描间隔窗口特征属性
static uint8_t scanIntervalWindowProps = GATT_PROP_WRITE_NO_RSP;  // 无响应写属性
static uint8_t scanIntervalWindow[SCAN_INTERVAL_WINDOW_CHAR_LEN];

// 扫描参数刷新特征属性
static uint8_t       scanParamRefreshProps = GATT_PROP_NOTIFY;    // 通知属性
static uint8_t       scanParamRefresh[SCAN_PARAM_REFRESH_LEN];
static gattCharCfg_t scanParamRefreshClientCharCfg[GATT_MAX_NUM_CONN];  // 客户端特征配置

/*********************************************************************
 * Profile属性 - 表
 */

// 扫描参数服务属性表
static gattAttribute_t scanParamAttrTbl[] = {
    // 扫描参数服务声明
    {
        {ATT_BT_UUID_SIZE, primaryServiceUUID}, /* 类型 */
        GATT_PERMIT_READ,                       /* 权限 */
        0,                                      /* 句柄 */
        (uint8_t *)&scanParamService            /* 值指针 */
    },

    // 扫描间隔窗口特征声明
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &scanIntervalWindowProps},

    // 扫描间隔窗口特征值
    {
        {ATT_BT_UUID_SIZE, scanIntervalWindowUUID},
        GATT_PERMIT_ENCRYPT_WRITE,
        0,
        scanIntervalWindow},

    // 扫描参数刷新特征声明
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &scanParamRefreshProps},

    // 扫描参数刷新特征值
    {
        {ATT_BT_UUID_SIZE, scanParamRefreshUUID},
        0,
        0,
        scanParamRefresh},

    // 扫描参数刷新特征客户端特征配置
    {
        {ATT_BT_UUID_SIZE, clientCharCfgUUID},
        GATT_PERMIT_READ | GATT_PERMIT_ENCRYPT_WRITE,
        0,
        (uint8_t *)&scanParamRefreshClientCharCfg}
};

// 属性表索引枚举 - 这些索引与上面数组元素对应
enum
{
    SCAN_PARAM_SERVICE_IDX,       // 扫描参数服务
    SCAN_PARAM_INTERVAL_DECL_IDX, // 扫描间隔窗口声明
    SCAN_PARAM_INTERVAL_IDX,      // 扫描间隔窗口特征
    SCAN_PARAM_REFRESH_DECL_IDX,  // 扫描参数刷新声明
    SCAN_PARAM_REFRESH_IDX,       // 扫描参数刷新特征
    SCAN_PARAM_REFRESH_CCCD_IDX   // 扫描参数刷新特征客户端特征配置
};

/*********************************************************************
 * 本地函数
 */
// 写属性回调函数
static bStatus_t scanParamWriteAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                      uint8_t *pValue, uint16_t len, uint16_t offset, uint8_t method);
// 读属性回调函数                                      
static bStatus_t scanParamReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                     uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint16_t maxLen, uint8_t method);

/*********************************************************************
 * PROFILE回调
 */

// 服务回调函数
gattServiceCBs_t scanParamCBs = {
    scanParamReadAttrCB,  // 读回调函数指针
    scanParamWriteAttrCB, // 写回调函数指针
    NULL                  // 授权回调函数指针
};

/*********************************************************************
 * 公共函数
 */

/*********************************************************************
 * @fn      ScanParam_AddService
 *
 * @brief   初始化扫描参数服务,通过向GATT服务器注册GATT属性
 *
 * @return  成功或失败
 */
bStatus_t ScanParam_AddService(void)
{
    uint8_t status = SUCCESS;

    // 初始化客户端特征配置属性
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, scanParamRefreshClientCharCfg);

    // 向GATT服务器注册GATT属性列表和回调函数
    status = GATTServApp_RegisterService(scanParamAttrTbl, GATT_NUM_ATTRS(scanParamAttrTbl), GATT_MAX_ENCRYPT_KEY_SIZE,
                                         &scanParamCBs);

    return (status);
}

/*********************************************************************
 * @fn      ScanParam_Register
 *
 * @brief   注册扫描参数服务的回调函数
 *
 * @param   pfnServiceCB - 回调函数
 *
 * @return  无
 */
extern void ScanParam_Register(scanParamServiceCB_t pfnServiceCB)
{
    scanParamServiceCB = pfnServiceCB;
}

/*********************************************************************
 * @fn      ScanParam_SetParameter
 *
 * @brief   设置扫描参数服务的参数
 *
 * @param   param - 参数ID
 * @param   len - 数据长度
 * @param   value - 数据指针,根据参数ID会被转换为相应的数据类型
 *
 * @return  bStatus_t
 */
bStatus_t ScanParam_SetParameter(uint8_t param, uint8_t len, void *value)
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
 * @fn      ScanParam_GetParameter
 *
 * @brief   获取扫描参数服务的参数
 *
 * @param   param - 参数ID
 * @param   value - 数据指针,根据参数ID会被转换为相应的数据类型
 *
 * @return  bStatus_t
 */
bStatus_t ScanParam_GetParameter(uint8_t param, void *value)
{
    bStatus_t ret = SUCCESS;
    switch(param)
    {
        case SCAN_PARAM_PARAM_INTERVAL:  // 获取扫描间隔
            *((uint16_t *)value) = BUILD_UINT16(scanIntervalWindow[0],
                                                scanIntervalWindow[1]);
            break;

        case SCAN_PARAM_PARAM_WINDOW:    // 获取扫描窗口
            *((uint16_t *)value) = BUILD_UINT16(scanIntervalWindow[2],
                                                scanIntervalWindow[3]);
            break;

        default:
            ret = INVALIDPARAMETER;
            break;
    }

    return (ret);
}

/*********************************************************************
 * @fn      ScanParam_RefreshNotify
 *
 * @brief   通知对端刷新扫描参数
 *
 * @param   connHandle - 连接句柄
 *
 * @return  无
 */
void ScanParam_RefreshNotify(uint16_t connHandle)
{
    uint16_t value;

    // 读取客户端特征配置
    value = GATTServApp_ReadCharCfg(connHandle, scanParamRefreshClientCharCfg);
    if(value & GATT_CLIENT_CFG_NOTIFY)
    {
        attHandleValueNoti_t noti;

        // 分配通知内存
        noti.pValue = GATT_bm_alloc(connHandle, ATT_HANDLE_VALUE_NOTI,
                                    SCAN_PARAM_REFRESH_LEN, NULL, 0);
        if(noti.pValue != NULL)
        {
            // 发送通知
            noti.handle = scanParamAttrTbl[SCAN_PARAM_REFRESH_CCCD_IDX].handle;
            noti.len = SCAN_PARAM_REFRESH_LEN;
            noti.pValue[0] = SCAN_PARAM_REFRESH_REQ;

            if(GATT_Notification(connHandle, &noti, FALSE) != SUCCESS)
            {
                GATT_bm_free((gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI);
            }
        }
    }
}

/*********************************************************************
 * @fn          scanParamReadAttrCB
 *
 * @brief       GATT读回调函数
 *
 * @param       connHandle - 连接句柄
 * @param       pAttr - 属性指针
 * @param       pValue - 数据指针
 * @param       pLen - 数据长度指针
 * @param       offset - 第一个字节的偏移
 * @param       maxLen - 最大数据长度
 *
 * @return      成功或失败
 */
static bStatus_t scanParamReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                     uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint16_t maxLen, uint8_t method)
{
    bStatus_t status = SUCCESS;

    return (status);
}

/*********************************************************************
 * @fn      scanParamWriteAttrCB
 *
 * @brief   在写操作前验证属性数据
 *
 * @param   connHandle - 连接句柄
 * @param   pAttr - 属性指针
 * @param   pValue - 数据指针
 * @param   len - 数据长度
 * @param   offset - 第一个字节的偏移
 *
 * @return  成功或失败
 */
static bStatus_t scanParamWriteAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                      uint8_t *pValue, uint16_t len, uint16_t offset, uint8_t method)
{
    uint16_t  uuid;
    bStatus_t status = SUCCESS;

    // 确保不是块操作(Profile中没有长属性)
    if(offset > 0)
    {
        return (ATT_ERR_ATTR_NOT_LONG);
    }

    uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);

    // 只有一个可写属性
    if(uuid == SCAN_INTERVAL_WINDOW_UUID)
    {
        if(len == SCAN_INTERVAL_WINDOW_CHAR_LEN)
        {
            uint16_t interval = BUILD_UINT16(pValue[0], pValue[1]);
            uint16_t window = BUILD_UINT16(pValue[0], pValue[1]);

            // 验证值
            if(window <= interval)
            {
                tmos_memcpy(pAttr->pValue, pValue, len);

                (*scanParamServiceCB)(SCAN_INTERVAL_WINDOW_SET);
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
    }
    else
    {
        status = ATT_ERR_ATTR_NOT_FOUND;
    }

    return (status);
}

/*********************************************************************
 * @fn          ScanParam_HandleConnStatusCB
 *
 * @brief       服务链路状态变化处理函数
 *
 * @param       connHandle - 连接句柄
 * @param       changeType - 变化类型
 *
 * @return      无
 */
void ScanParam_HandleConnStatusCB(uint16_t connHandle, uint8_t changeType)
{
    // 确保不是回环连接
    if(connHandle != LOOPBACK_CONNHANDLE)
    {
        // 如果连接断开则重置客户端特征配置
        if((changeType == LINKDB_STATUS_UPDATE_REMOVED) ||
           ((changeType == LINKDB_STATUS_UPDATE_STATEFLAGS) &&
            (!linkDB_Up(connHandle))))
        {
            GATTServApp_InitCharCfg(connHandle, scanParamRefreshClientCharCfg);
        }
    }
}
