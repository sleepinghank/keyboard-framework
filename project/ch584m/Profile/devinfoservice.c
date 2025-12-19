/********************************** (C) COPYRIGHT *******************************
 * File Name          : devinfoservice.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/12/10
 * Description        : 设备信息服务
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*********************************************************************
 * 包含头文件
 */
#include "CONFIG.h"
#include "devinfoservice.h"

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
// 设备信息服务UUID
const uint8_t devInfoServUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(DEVINFO_SERV_UUID), HI_UINT16(DEVINFO_SERV_UUID)};

// 系统ID UUID
const uint8_t devInfoSystemIdUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(SYSTEM_ID_UUID), HI_UINT16(SYSTEM_ID_UUID)};

// 型号字符串UUID
const uint8_t devInfoModelNumberUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(MODEL_NUMBER_UUID), HI_UINT16(MODEL_NUMBER_UUID)};

// 序列号字符串UUID
const uint8_t devInfoSerialNumberUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(SERIAL_NUMBER_UUID), HI_UINT16(SERIAL_NUMBER_UUID)};

// 固件版本字符串UUID
const uint8_t devInfoFirmwareRevUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(FIRMWARE_REV_UUID), HI_UINT16(FIRMWARE_REV_UUID)};

// 硬件版本字符串UUID
const uint8_t devInfoHardwareRevUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(HARDWARE_REV_UUID), HI_UINT16(HARDWARE_REV_UUID)};

// 软件版本字符串UUID
const uint8_t devInfoSoftwareRevUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(SOFTWARE_REV_UUID), HI_UINT16(SOFTWARE_REV_UUID)};

// 制造商名称字符串UUID
const uint8_t devInfoMfrNameUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(MANUFACTURER_NAME_UUID), HI_UINT16(MANUFACTURER_NAME_UUID)};

// IEEE 11073-20601认证数据列表UUID
const uint8_t devInfo11073CertUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(IEEE_11073_CERT_DATA_UUID), HI_UINT16(IEEE_11073_CERT_DATA_UUID)};

// PnP ID UUID
const uint8_t devInfoPnpIdUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(PNP_ID_UUID), HI_UINT16(PNP_ID_UUID)};

/*********************************************************************
 * 外部变量
 */

/*********************************************************************
 * 外部函数
 */

/*********************************************************************
 * 本地变量
 */

/*********************************************************************
 * Profile属性 - 变量
 */

// 设备信息服务属性
static const gattAttrType_t devInfoService = {ATT_BT_UUID_SIZE, devInfoServUUID};

// 系统ID特征
static uint8_t devInfoSystemIdProps = GATT_PROP_READ;  // 只读属性
static uint8_t devInfoSystemId[DEVINFO_SYSTEM_ID_LEN] = {0, 0, 0, 0, 0, 0, 0, 0};

// 型号字符串特征
static uint8_t       devInfoModelNumberProps = GATT_PROP_READ;  // 只读属性
static const uint8_t devInfoModelNumber[] = "Model Number";

// 序列号字符串特征
static uint8_t       devInfoSerialNumberProps = GATT_PROP_READ;  // 只读属性
static const uint8_t devInfoSerialNumber[] = "Serial Number";

// 固件版本字符串特征
static uint8_t       devInfoFirmwareRevProps = GATT_PROP_READ;  // 只读属性
static const uint8_t devInfoFirmwareRev[] = "Firmware Revision";

// 硬件版本字符串特征
static uint8_t       devInfoHardwareRevProps = GATT_PROP_READ;  // 只读属性
static const uint8_t devInfoHardwareRev[] = "Hardware Revision";

// 软件版本字符串特征
static uint8_t       devInfoSoftwareRevProps = GATT_PROP_READ;  // 只读属性
static const uint8_t devInfoSoftwareRev[] = "Software Revision";

// 制造商名称字符串特征
static uint8_t       devInfoMfrNameProps = GATT_PROP_READ;  // 只读属性
static const uint8_t devInfoMfrName[] = "Manufacturer Name";

// IEEE 11073-20601认证数据列表特征
static uint8_t       devInfo11073CertProps = GATT_PROP_READ;  // 只读属性
static const uint8_t devInfo11073Cert[] = {
    DEVINFO_11073_BODY_EXP,  // 认证机构类型
    0x00,                    // 认证机构结构类型
                            // 认证机构数据:
    'e', 'x', 'p', 'e', 'r', 'i', 'm', 'e', 'n', 't', 'a', 'l'};

// PnP ID特征
static uint8_t devInfoPnpIdProps = GATT_PROP_READ;  // 只读属性
static uint8_t devInfoPnpId[DEVINFO_PNP_ID_LEN] = {
    1,                                    // 厂商ID来源(1=蓝牙SIG)
    LO_UINT16(0x07D7), HI_UINT16(0x07D7), // 厂商ID (WCH)
    LO_UINT16(0x0000), HI_UINT16(0x0000), // 产品ID (厂商定义)
    LO_UINT16(0x0110), HI_UINT16(0x0110)  // 产品版本 (JJ.M.N)
};

/*********************************************************************
 * Profile属性 - 表
 */

// 设备信息服务属性表
static gattAttribute_t devInfoAttrTbl[] = {
    // 设备信息服务声明
    {
        {ATT_BT_UUID_SIZE, primaryServiceUUID}, /* 类型 */
        GATT_PERMIT_READ,                       /* 权限 */
        0,                                      /* 句柄 */
        (uint8_t *)&devInfoService              /* 值指针 */
    },

    // 系统ID声明
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &devInfoSystemIdProps},

    // 系统ID值
    {
        {ATT_BT_UUID_SIZE, devInfoSystemIdUUID},
        GATT_PERMIT_READ,
        0,
        (uint8_t *)devInfoSystemId},

    // 型号字符串声明
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &devInfoModelNumberProps},

    // 型号字符串值
    {
        {ATT_BT_UUID_SIZE, devInfoModelNumberUUID},
        GATT_PERMIT_READ,
        0,
        (uint8_t *)devInfoModelNumber},

    // 序列号字符串声明
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &devInfoSerialNumberProps},

    // 序列号字符串值
    {
        {ATT_BT_UUID_SIZE, devInfoSerialNumberUUID},
        GATT_PERMIT_READ,
        0,
        (uint8_t *)devInfoSerialNumber},

    // 固件版本字符串声明
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &devInfoFirmwareRevProps},

    // 固件版本字符串值
    {
        {ATT_BT_UUID_SIZE, devInfoFirmwareRevUUID},
        GATT_PERMIT_READ,
        0,
        (uint8_t *)devInfoFirmwareRev},

    // 硬件版本字符串声明
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &devInfoHardwareRevProps},

    // 硬件版本字符串值
    {
        {ATT_BT_UUID_SIZE, devInfoHardwareRevUUID},
        GATT_PERMIT_READ,
        0,
        (uint8_t *)devInfoHardwareRev},

    // 软件版本字符串声明
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &devInfoSoftwareRevProps},

    // 软件版本字符串值
    {
        {ATT_BT_UUID_SIZE, devInfoSoftwareRevUUID},
        GATT_PERMIT_READ,
        0,
        (uint8_t *)devInfoSoftwareRev},

    // 制造商名称字符串声明
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &devInfoMfrNameProps},

    // 制造商名称字符串值
    {
        {ATT_BT_UUID_SIZE, devInfoMfrNameUUID},
        GATT_PERMIT_READ,
        0,
        (uint8_t *)devInfoMfrName},

    // IEEE 11073-20601认证数据列表声明
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &devInfo11073CertProps},

    // IEEE 11073-20601认证数据列表值
    {
        {ATT_BT_UUID_SIZE, devInfo11073CertUUID},
        GATT_PERMIT_READ,
        0,
        (uint8_t *)devInfo11073Cert},

    // PnP ID声明
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &devInfoPnpIdProps},

    // PnP ID值
    {
        {ATT_BT_UUID_SIZE, devInfoPnpIdUUID},
        GATT_PERMIT_READ,
        0,
        (uint8_t *)devInfoPnpId}};

/*********************************************************************
 * 本地函数
 */
// 读取属性回调函数
static bStatus_t devInfo_ReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                    uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint16_t maxLen, uint8_t method);

/*********************************************************************
 * Profile回调函数
 */
// 设备信息服务回调函数
gattServiceCBs_t devInfoCBs = {
    devInfo_ReadAttrCB, // 读取回调函数指针
    NULL,               // 写入回调函数指针
    NULL                // 授权回调函数指针
};

/*********************************************************************
 * 网络层回调函数
 */

/*********************************************************************
 * 公共函数
 */

/*********************************************************************
 * @fn      DevInfo_AddService
 *
 * @brief   初始化设备信息服务,通过向GATT服务器注册GATT属性
 *
 * @return  成功或失败
 */
bStatus_t DevInfo_AddService(void)
{
    // 向GATT服务器应用注册GATT属性列表和回调函数
    return GATTServApp_RegisterService(devInfoAttrTbl,
                                       GATT_NUM_ATTRS(devInfoAttrTbl),
                                       GATT_MAX_ENCRYPT_KEY_SIZE,
                                       &devInfoCBs);
}

/*********************************************************************
 * @fn      DevInfo_SetParameter
 *
 * @brief   设置设备信息参数
 *
 * @param   param - Profile参数ID
 * @param   len - 要写入数据的长度
 * @param   value - 要写入数据的指针。这取决于参数ID,并且会被转换为适当的数据类型
 *          (例如:uint16_t的数据类型会被转换为uint16_t指针)
 *
 * @return  bStatus_t
 */
bStatus_t DevInfo_SetParameter(uint8_t param, uint8_t len, void *value)
{
    bStatus_t ret = SUCCESS;

    switch(param)
    {
        case DEVINFO_SYSTEM_ID:  // 设置系统ID
            tmos_memcpy(devInfoSystemId, value, len);
            break;

        default:
            ret = INVALIDPARAMETER;
            break;
    }

    return (ret);
}

/*********************************************************************
 * @fn      DevInfo_GetParameter
 *
 * @brief   获取设备信息参数
 *
 * @param   param - Profile参数ID
 * @param   value - 获取数据的指针。这取决于参数ID,并且会被转换为适当的数据类型
 *          (例如:uint16_t的数据类型会被转换为uint16_t指针)
 *
 * @return  bStatus_t
 */
bStatus_t DevInfo_GetParameter(uint8_t param, void *value)
{
    bStatus_t ret = SUCCESS;

    switch(param)
    {
        case DEVINFO_SYSTEM_ID:  // 获取系统ID
            tmos_memcpy(value, devInfoSystemId, sizeof(devInfoSystemId));
            break;

        case DEVINFO_MODEL_NUMBER:  // 获取型号
            tmos_memcpy(value, devInfoModelNumber, sizeof(devInfoModelNumber));
            break;
            
        case DEVINFO_SERIAL_NUMBER:  // 获取序列号
            tmos_memcpy(value, devInfoSerialNumber, sizeof(devInfoSerialNumber));
            break;

        case DEVINFO_FIRMWARE_REV:  // 获取固件版本
            tmos_memcpy(value, devInfoFirmwareRev, sizeof(devInfoFirmwareRev));
            break;

        case DEVINFO_HARDWARE_REV:  // 获取硬件版本
            tmos_memcpy(value, devInfoHardwareRev, sizeof(devInfoHardwareRev));
            break;

        case DEVINFO_SOFTWARE_REV:  // 获取软件版本
            tmos_memcpy(value, devInfoSoftwareRev, sizeof(devInfoSoftwareRev));
            break;

        case DEVINFO_MANUFACTURER_NAME:  // 获取制造商名称
            tmos_memcpy(value, devInfoMfrName, sizeof(devInfoMfrName));
            break;

        case DEVINFO_11073_CERT_DATA:  // 获取11073认证数据
            tmos_memcpy(value, devInfo11073Cert, sizeof(devInfo11073Cert));
            break;

        case DEVINFO_PNP_ID:  // 获取PnP ID
            tmos_memcpy(value, devInfoPnpId, sizeof(devInfoPnpId));
            break;

        default:
            ret = INVALIDPARAMETER;
            break;
    }

    return (ret);
}

/*********************************************************************
 * @fn          devInfo_ReadAttrCB
 *
 * @brief       读取属性的回调函数
 *
 * @param       connHandle - 接收到消息的连接句柄
 * @param       pAttr - 指向属性的指针
 * @param       pValue - 指向要读取数据的指针
 * @param       pLen - 要读取数据的长度
 * @param       offset - 要读取的第一个字节的偏移量
 * @param       maxLen - 可以读取的最大数据长度
 *
 * @return      成功或失败
 */
static bStatus_t devInfo_ReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                    uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint16_t maxLen, uint8_t method)
{
    bStatus_t status = SUCCESS;
    uint16_t  uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);

    switch(uuid)
    {
        case SYSTEM_ID_UUID:  // 读取系统ID
            // 验证偏移量
            if(offset >= sizeof(devInfoSystemId))
            {
                status = ATT_ERR_INVALID_OFFSET;
            }
            else
            {
                // 确定读取长度
                *pLen = MIN(maxLen, (sizeof(devInfoSystemId) - offset));

                // 复制数据
                tmos_memcpy(pValue, &devInfoSystemId[offset], *pLen);
            }
            break;

        case MODEL_NUMBER_UUID:  // 读取型号
            // 验证偏移量
            if(offset >= (sizeof(devInfoModelNumber) - 1))
            {
                status = ATT_ERR_INVALID_OFFSET;
            }
            else
            {
                // 确定读取长度(不包括空终止符)
                *pLen = MIN(maxLen, ((sizeof(devInfoModelNumber) - 1) - offset));

                // 复制数据
                tmos_memcpy(pValue, &devInfoModelNumber[offset], *pLen);
            }
            break;

        case SERIAL_NUMBER_UUID:  // 读取序列号
            // 验证偏移量
            if(offset >= (sizeof(devInfoSerialNumber) - 1))
            {
                status = ATT_ERR_INVALID_OFFSET;
            }
            else
            {
                // 确定读取长度(不包括空终止符)
                *pLen = MIN(maxLen, ((sizeof(devInfoSerialNumber) - 1) - offset));

                // 复制数据
                tmos_memcpy(pValue, &devInfoSerialNumber[offset], *pLen);
            }
            break;

        case FIRMWARE_REV_UUID:  // 读取固件版本
            // 验证偏移量
            if(offset >= (sizeof(devInfoFirmwareRev) - 1))
            {
                status = ATT_ERR_INVALID_OFFSET;
            }
            else
            {
                // 确定读取长度(不包括空终止符)
                *pLen = MIN(maxLen, ((sizeof(devInfoFirmwareRev) - 1) - offset));

                // 复制数据
                tmos_memcpy(pValue, &devInfoFirmwareRev[offset], *pLen);
            }
            break;

        case HARDWARE_REV_UUID:  // 读取硬件版本
            // 验证偏移量
            if(offset >= (sizeof(devInfoHardwareRev) - 1))
            {
                status = ATT_ERR_INVALID_OFFSET;
            }
            else
            {
                // 确定读取长度(不包括空终止符)
                *pLen = MIN(maxLen, ((sizeof(devInfoHardwareRev) - 1) - offset));

                // 复制数据
                tmos_memcpy(pValue, &devInfoHardwareRev[offset], *pLen);
            }
            break;

        case SOFTWARE_REV_UUID:  // 读取软件版本
            // 验证偏移量
            if(offset >= (sizeof(devInfoSoftwareRev) - 1))
            {
                status = ATT_ERR_INVALID_OFFSET;
            }
            else
            {
                // 确定读取长度(不包括空终止符)
                *pLen = MIN(maxLen, ((sizeof(devInfoSoftwareRev) - 1) - offset));

                // 复制数据
                tmos_memcpy(pValue, &devInfoSoftwareRev[offset], *pLen);
            }
            break;

        case MANUFACTURER_NAME_UUID:  // 读取制造商名称
            // 验证偏移量
            if(offset >= (sizeof(devInfoMfrName) - 1))
            {
                status = ATT_ERR_INVALID_OFFSET;
            }
            else
            {
                // 确定读取长度(不包括空终止符)
                *pLen = MIN(maxLen, ((sizeof(devInfoMfrName) - 1) - offset));

                // 复制数据
                tmos_memcpy(pValue, &devInfoMfrName[offset], *pLen);
            }
            break;

        case IEEE_11073_CERT_DATA_UUID:  // 读取11073认证数据
            // 验证偏移量
            if(offset >= sizeof(devInfo11073Cert))
            {
                status = ATT_ERR_INVALID_OFFSET;
            }
            else
            {
                // 确定读取长度
                *pLen = MIN(maxLen, (sizeof(devInfo11073Cert) - offset));

                // 复制数据
                tmos_memcpy(pValue, &devInfo11073Cert[offset], *pLen);
            }
            break;

        case PNP_ID_UUID:  // 读取PnP ID
            // 验证偏移量
            if(offset >= sizeof(devInfoPnpId))
            {
                status = ATT_ERR_INVALID_OFFSET;
            }
            else
            {
                // 确定读取长度
                *pLen = MIN(maxLen, (sizeof(devInfoPnpId) - offset));

                // 复制数据
                tmos_memcpy(pValue, &devInfoPnpId[offset], *pLen);
            }
            break;

        default:
            *pLen = 0;
            status = ATT_ERR_ATTR_NOT_FOUND;
            break;
    }

    return (status);
}

/*********************************************************************
*********************************************************************/
