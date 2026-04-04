/********************************** (C) COPYRIGHT *******************************
 * 文件名称          : SimpleProfile.C
 * 作者             : WCH
 * 版本             : V1.0
 * 日期             : 2023/12/01
 * 描述             : 自定义包含五种不同属性的服务，包含可读、可写、通知、可读可写、安全可读
 *********************************************************************************
 * 版权所有 (c) 2021 南京沁恒微电子股份有限公司
 * 注意: 本软件 (修改或未修改) 以及二进制文件仅用于
 * 南京沁恒微电子制造的微控制器。
 *******************************************************************************/

/*********************************************************************
 * 包含的头文件
 */
#include "CONFIG.h"
#include "Inateck_ota.h"
#include "hiddev.h"
#include "otaservice.h"

/*********************************************************************
 * 宏定义
 */

// SimpleProfile特征值1在属性数组中的位置
#define SIMPLEPROFILE_CHAR1_VALUE_POS    2

/*********************************************************************
 * 类型定义
 */

/*********************************************************************
 * 全局变量
 */
// Simple GATT Profile Service UUID: 0xFFF0
const uint8_t simpleProfileServUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(SIMPLEPROFILE_SERV_UUID), HI_UINT16(SIMPLEPROFILE_SERV_UUID)};

// Characteristic 1 UUID: 0xFFF1
const uint8_t simpleProfilechar1UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(SIMPLEPROFILE_CHAR1_UUID), HI_UINT16(SIMPLEPROFILE_CHAR1_UUID)};

/*********************************************************************
 * 外部变量
 */

/*********************************************************************
 * 外部函数
 */

/*********************************************************************
 * 本地变量
 */

// 应用回调函数指针
static simpleProfileCBs_t *simpleProfile_AppCBs = NULL;

/*********************************************************************
 * Profile属性变量
 */

// Simple Profile Service属性
static const gattAttrType_t simpleProfileService = {ATT_BT_UUID_SIZE, simpleProfileServUUID};

// Simple Profile Characteristic 1 Properties (支持读写和通知)
static uint8_t simpleProfileChar1Props = GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP | GATT_PROP_NOTIFY;

// Characteristic 1 Value
static uint8_t simpleProfileChar1[SIMPLEPROFILE_CHAR1_LEN] = {0};

// Characteristic 1 配置,每个客户端都有自己的实例
static gattCharCfg_t simpleProfileChar1Cfg[PERIPHERAL_MAX_CONNECTION];

// Simple Profile Characteristic 1 User Description
static uint8_t simpleProfileChar1UserDesp[] = "Characteristic 1\0";

/*********************************************************************
 * Profile属性表
 */

static gattAttribute_t simpleProfileAttrTbl[] = {
    // Simple Profile Service
    {
        {ATT_BT_UUID_SIZE, primaryServiceUUID}, /* type */
        GATT_PERMIT_READ,                       /* permissions */
        0,                                      /* handle */
        (uint8_t *)&simpleProfileService        /* pValue */
    },

    // Characteristic 1 Declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &simpleProfileChar1Props},

    // Characteristic Value 1
    {
        {ATT_BT_UUID_SIZE, simpleProfilechar1UUID},
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        simpleProfileChar1},
        
    // Characteristic 1 configuration
    {
        {ATT_BT_UUID_SIZE, clientCharCfgUUID},
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8_t *)simpleProfileChar1Cfg},

    // Characteristic 1 User Description
    {
        {ATT_BT_UUID_SIZE, charUserDescUUID},
        GATT_PERMIT_READ,
        0,
        simpleProfileChar1UserDesp},
};

/*********************************************************************
 * 本地函数声明
 */
static bStatus_t simpleProfile_ReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                          uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint16_t maxLen, uint8_t method);
static bStatus_t simpleProfile_WriteAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t len, uint16_t offset, uint8_t method);

static void simpleProfile_HandleConnStatusCB(uint16_t connHandle, uint8_t changeType);

/*********************************************************************
 * Profile回调函数
 */
// Simple Profile Service回调函数
gattServiceCBs_t simpleProfileCBs = {
    simpleProfile_ReadAttrCB,  // 读回调函数
    simpleProfile_WriteAttrCB, // 写回调函数
    NULL                       // 授权回调函数
};

/*********************************************************************
 * 公共函数
 */

/*********************************************************************
 * @fn      SimpleProfile_AddService
 *
 * @brief   初始化Simple Profile服务,通过向GATT服务器注册属性表来完成。
 *
 * @param   services - 要添加的服务。这是一个位图,可以包含多个服务。
 *
 * @return  成功或失败
 */
bStatus_t SimpleProfile_AddService(void)
{
    uint8_t status = SUCCESS;

    // Initialize Client Characteristic Configuration attributes
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, simpleProfileChar1Cfg);
    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService(simpleProfileAttrTbl,
                                         GATT_NUM_ATTRS(simpleProfileAttrTbl),
                                         GATT_MAX_ENCRYPT_KEY_SIZE,
                                         &simpleProfileCBs);

    return (status);

    // uint8_t status = SUCCESS;

    // // 初始化特征值1的客户端配置特性
    // GATTServApp_InitCharCfg(INVALID_CONNHANDLE, simpleProfileChar1Cfg);

    // // 注册连接状态回调函数
    // linkDB_Register(simpleProfile_HandleConnStatusCB);

    // if(services & SIMPLEPROFILE_SERVICE)
    // {
    //     // 向GATT服务器注册属性表和回调函数
    //     status = GATTServApp_RegisterService(simpleProfileAttrTbl,
    //                                          GATT_NUM_ATTRS(simpleProfileAttrTbl),
    //                                          GATT_MAX_ENCRYPT_KEY_SIZE,
    //                                          &simpleProfileCBs);
    // }

    // return (status);
}

/*********************************************************************
 * @fn      SimpleProfile_RegisterAppCBs
 *
 * @brief   注册应用程序回调函数。此函数只能调用一次。
 *
 * @param   callbacks - 指向应用程序回调函数的指针。
 *
 * @return  SUCCESS或bleAlreadyInRequestedMode
 */
bStatus_t SimpleProfile_RegisterAppCBs(simpleProfileCBs_t *appCallbacks)
{
    if(appCallbacks)
    {
        simpleProfile_AppCBs = appCallbacks;

        return (SUCCESS);
    }
    else
    {
        return (bleAlreadyInRequestedMode);
    }
}

/*********************************************************************
 * @fn      SimpleProfile_SetParameter
 *
 * @brief   设置Simple Profile参数。
 *
 * @param   param - 参数ID
 * @param   len - 数据长度
 * @param   value - 指向要写入的数据的指针。这取决于
 *          参数ID,会被转换为适当的数据类型
 *          (例如:uint16_t类型的数据将被转换为
 *          uint16_t指针)。
 *
 * @return  bStatus_t
 */
bStatus_t SimpleProfile_SetParameter(uint8_t param, uint8_t len, void *value)
{
    bStatus_t ret = SUCCESS;
    switch(param)
    {
        case SIMPLEPROFILE_CHAR1:
            if(len <= SIMPLEPROFILE_CHAR1_LEN)
            {
                tmos_memcpy(simpleProfileChar1, value, len);
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
 * @fn      SimpleProfile_GetParameter
 *
 * @brief   获取Simple Profile参数。
 *
 * @param   param - 参数ID
 * @param   value - 指向要放置数据的指针。这取决于
 *          参数ID,会被转换为适当的数据类型
 *          (例如:uint16_t类型的数据将被转换为
 *          uint16_t指针)。
 *
 * @return  bStatus_t
 */
bStatus_t SimpleProfile_GetParameter(uint8_t param, void *value)
{
    bStatus_t ret = SUCCESS;
    switch(param)
    {
        case SIMPLEPROFILE_CHAR1:
            tmos_memcpy(value, simpleProfileChar1, SIMPLEPROFILE_CHAR1_LEN);
            break;
        default:
            ret = INVALIDPARAMETER;
            break;
    }

    return (ret);
}

/*********************************************************************
 * @fn      SimpleProfile_Notify
 *
 * @brief   发送通知包含数据。
 *
 * @param   connHandle - 连接句柄
 * @param   pNoti - 指向通知结构的指针
 *
 * @return  成功或失败
 */
bStatus_t SimpleProfile_Notify(uint16_t connHandle, attHandleValueNoti_t *pNoti)
{
    uint16_t value = GATTServApp_ReadCharCfg(connHandle, simpleProfileChar1Cfg);

  //  PRINT("SimpleProfile_Notify - connHandle=%d, value=0x%04X\r\n", connHandle, value);
    
    // 如果通知已启用
    if(value & GATT_CLIENT_CFG_NOTIFY)
    {
    //    PRINT("通知已启用，发送数据，长度: %d\r\n", pNoti->len);
        // 设置句柄
        pNoti->handle = simpleProfileAttrTbl[SIMPLEPROFILE_CHAR1_VALUE_POS].handle;

        // 发送通知
        return GATT_Notification(connHandle, pNoti, FALSE);
    }
  //  PRINT("通知未启用，无法发送数据\r\n");
    return bleIncorrectMode;
    return 0;
}

/*********************************************************************
 * @fn      simpleProfile_ReadAttrCB
 *
 * @brief   读取属性回调函数。
 *
 * @param   connHandle - 接收到消息的连接
 * @param   pAttr - 指向属性的指针
 * @param   pValue - 指向要读取的数据的指针
 * @param   pLen - 要读取的数据长度指针
 * @param   offset - 要读取的第一个字节的偏移量
 * @param   maxLen - 可以读取的最大数据长度
 *
 * @return  成功或失败
 */
static bStatus_t simpleProfile_ReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                          uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint16_t maxLen, uint8_t method)
{
    bStatus_t status = SUCCESS;

    // 确保它不是blob操作(Profile中没有长属性)
    if(offset > 0)
    {
        PRINT("读取属性失败：不支持blob操作\r\n");
        return (ATT_ERR_ATTR_NOT_LONG);
    }

    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        // 16位UUID
        uint16_t uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);
        PRINT("读取属性: UUID=0x%04X\r\n", uuid);
        
        switch(uuid)
        {
            // "GATT_SERVICE_UUID"或"GATT_CLIENT_CHAR_CFG_UUID"无需处理;
            // gattserverapp处理这些读取操作

            case SIMPLEPROFILE_CHAR1_UUID:
                if(maxLen > SIMPLEPROFILE_CHAR1_LEN)
                {
                    *pLen = SIMPLEPROFILE_CHAR1_LEN;
                }
                else
                {
                    *pLen = maxLen;
                }
                // tmos_memcpy(pValue, pAttr->pValue, *pLen);
                // uint8_t sz = 0;
                // uint8_t rsp[sizeof(origin_cmd_resp)] = {0};
                // if (_Origin_Command_Check())
                // {
                //     _Origin_Rsp(rsp, &sz);
                //     // copy data
                //     tmos_memcpy(pValue, rsp + offset, (sz -offset));
                // }
        
                PRINT("读取特征值1, 长度: %d\r\n", *pLen);
                break;

            default:
                // 不应该到达这里!
                *pLen = 0;
                status = ATT_ERR_ATTR_NOT_FOUND;
                PRINT("读取属性失败：未找到属性\r\n");
                break;
        }
    }
    else
    {
        // 128位UUID
        *pLen = 0;
        status = ATT_ERR_INVALID_HANDLE;
        PRINT("读取属性失败：无效句柄\r\n");
    }

    return (status);
}

/*********************************************************************
 * @fn      simpleProfile_WriteAttrCB
 *
 * @brief   验证属性数据,进行写操作前的检查
 *
 * @param   connHandle - 接收到消息的连接
 * @param   pAttr - 指向属性的指针
 * @param   pValue - 指向要写入的数据的指针
 * @param   len - 数据长度
 * @param   offset - 要写入的第一个字节的偏移量
 *
 * @return  成功或失败
 */
static bStatus_t simpleProfile_WriteAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t len, uint16_t offset, uint8_t method)
{
    bStatus_t status = SUCCESS;
    uint8_t   notifyApp = 0xFF;

    // 如果属性权限需要授权才能写入,返回错误
    if(gattPermitAuthorWrite(pAttr->permissions))
    {
        PRINT("写入属性失败：授权不足\r\n");
        // 授权不足
        return (ATT_ERR_INSUFFICIENT_AUTHOR);
    }

    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        // 16位UUID
        uint16_t uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);
       // PRINT("写入属性: UUID=0x%04X, 长度: %d\r\n", uuid, len);
        
        switch(uuid)
        {
            case SIMPLEPROFILE_CHAR1_UUID:
                // 验证值
                // 确保不是blob操作
                if(offset == 0)
                {
                    if(len > SIMPLEPROFILE_CHAR1_LEN)
                    {
                        status = ATT_ERR_INVALID_VALUE_SIZE;
                        PRINT("写入特征值1失败：值大小无效\r\n");
                    }
                }
                else
                {
                    status = ATT_ERR_ATTR_NOT_LONG;
                    PRINT("写入特征值1失败：不支持blob操作\r\n");
                }

                // 写入值
                if(status == SUCCESS)
                {
                    tmos_memcpy(pAttr->pValue, pValue, len);
                    notifyApp = SIMPLEPROFILE_CHAR1;
                    _OTA_Command_Processor(pValue, len);
             //       PRINT("写入特征值1成功\r\n");
                }
                break;

            case GATT_CLIENT_CHAR_CFG_UUID:
                PRINT("处理客户端特征配置写入请求\r\n");
                status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len,
                                                        offset, GATT_CLIENT_CFG_NOTIFY);
                if(status == SUCCESS)
                {
                    uint16_t charCfg = BUILD_UINT16(pValue[0], pValue[1]);
                    PRINT("客户端配置更新: %s, connHandle=%d, 值=0x%04X\r\n", 
                           (charCfg & GATT_CLIENT_CFG_NOTIFY) ? "通知已启用" : "通知已禁用",
                           connHandle, charCfg);
                    
                    // 查找这个配置是哪个特征的
                    if(pAttr->handle == simpleProfileAttrTbl[SIMPLEPROFILE_CHAR1_VALUE_POS + 1].handle)
                    {
                        PRINT("特征1的通知配置已更新\r\n");
                    }
                }
                else
                {
                    PRINT("客户端配置更新失败, 状态: %d\r\n", status);
                }
                break;

            default:
                // 不应该到达这里!
                status = ATT_ERR_ATTR_NOT_FOUND;
                PRINT("写入属性失败：未找到属性\r\n");
                break;
        }
    }
    else
    {
        // 128位UUID
        status = ATT_ERR_INVALID_HANDLE;
        PRINT("写入属性失败：无效句柄\r\n");
    }

    // 如果属性值已更改,则回调通知应用程序
    if((notifyApp != 0xFF) && simpleProfile_AppCBs && simpleProfile_AppCBs->pfnSimpleProfileChange)
    {
        PRINT("通知应用程序特征值变化: %d\r\n", notifyApp);
        simpleProfile_AppCBs->pfnSimpleProfileChange(notifyApp, pValue, len);
    }

    return (status);
}

/*********************************************************************
 * @fn      simpleProfile_HandleConnStatusCB
 *
 * @brief   Simple Profile连接状态变化处理函数。
 *
 * @param   connHandle - 连接句柄
 * @param   changeType - 变化类型
 *
 * @return  none
 */
static void simpleProfile_HandleConnStatusCB(uint16_t connHandle, uint8_t changeType)
{
    // 确保这不是环回连接
    if(connHandle != LOOPBACK_CONNHANDLE)
    {
        // 如果连接已断开,重置客户端特征配置
        if((changeType == LINKDB_STATUS_UPDATE_REMOVED) ||
           ((changeType == LINKDB_STATUS_UPDATE_STATEFLAGS) &&
            (!linkDB_Up(connHandle))))
        {
            GATTServApp_InitCharCfg(connHandle, simpleProfileChar1Cfg);
        }
    }
}

/*********************************************************************
*********************************************************************/ 