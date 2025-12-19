/********************************** (C) COPYRIGHT *******************************
 * 文件名称          : devinfoservice.h
 * 作者             : WCH
 * 版本             : V1.0
 * 日期             : 2018/12/11
 * 描述             : 设备信息服务头文件,包含设备信息服务相关的常量、类型定义和API函数声明
 *********************************************************************************
 * 版权所有 (c) 2021 南京沁恒微电子股份有限公司
 * 注意: 本软件(修改或未修改)及其二进制文件仅用于
 * 南京沁恒微电子制造的微控制器。
 *******************************************************************************/

#ifndef DEVINFOSERVICE_H
#define DEVINFOSERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * 包含头文件
 */

/*********************************************************************
 * 常量定义
 */

// 设备信息服务参数定义
#define DEVINFO_SYSTEM_ID              0  // 系统ID
#define DEVINFO_MODEL_NUMBER           1  // 型号
#define DEVINFO_SERIAL_NUMBER          2  // 序列号
#define DEVINFO_FIRMWARE_REV           3  // 固件版本
#define DEVINFO_HARDWARE_REV           4  // 硬件版本
#define DEVINFO_SOFTWARE_REV           5  // 软件版本
#define DEVINFO_MANUFACTURER_NAME      6  // 制造商名称
#define DEVINFO_11073_CERT_DATA        7  // IEEE 11073认证数据
#define DEVINFO_PNP_ID                 8  // PnP ID

// IEEE 11073授权机构值定义
#define DEVINFO_11073_BODY_EMPTY       0  // 空
#define DEVINFO_11073_BODY_IEEE        1  // IEEE
#define DEVINFO_11073_BODY_CONTINUA    2  // Continua
#define DEVINFO_11073_BODY_EXP         254 // 实验性

// 系统ID长度
#define DEVINFO_SYSTEM_ID_LEN          8  // 系统ID字段长度(字节)

// PnP ID长度
#define DEVINFO_PNP_ID_LEN             7  // PnP ID字段长度(字节)

/*********************************************************************
 * 类型定义
 */

/*********************************************************************
 * 宏定义
 */

/*********************************************************************
 * 配置文件回调
 */

/*********************************************************************
 * API函数
 */

/*
 * DevInfo_AddService - 初始化设备信息服务,通过向GATT服务器注册GATT属性来实现
 *
 */

extern bStatus_t DevInfo_AddService(void);

/*********************************************************************
 * @fn      DevInfo_SetParameter
 *
 * @brief   设置设备信息参数
 *
 * @param   param - 配置文件参数ID
 * @param   len - 要写入数据的长度
 * @param   value - 指向要写入数据的指针。这取决于参数ID,
 *          并将被转换为适当的数据类型(例如:uint16_t类型的数据
 *          将被转换为uint16_t指针)。
 *
 * @return  bStatus_t
 */
bStatus_t DevInfo_SetParameter(uint8_t param, uint8_t len, void *value);

/*
 * DevInfo_GetParameter - 获取设备信息参数
 *
 *    param - 配置文件参数ID
 *    value - 指向获取数据的指针。这取决于参数ID,
 *          并将被转换为适当的数据类型(例如:uint16_t类型的数据
 *          将被转换为uint16_t指针)。
 */
extern bStatus_t DevInfo_GetParameter(uint8_t param, void *value);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* DEVINFOSERVICE_H */
