/********************************** (C) COPYRIGHT *******************************
 * 文件名称          : SimpleProfile.h
 * 作者             : WCH
 * 版本             : V1.0
 * 日期             : 2023/12/01
 * 描述             : 
 *********************************************************************************
 * 版权所有 (c) 2021 南京沁恒微电子股份有限公司
 * 注意: 本软件 (修改或未修改) 以及二进制文件仅用于
 * 南京沁恒微电子制造的微控制器。
 *******************************************************************************/

#ifndef OTASERVICE_H
#define OTASERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * 头文件包含
 */

/*********************************************************************
 * 常量定义
 */
// Profile特征值索引定义
#define SIMPLEPROFILE_CHAR1         0           // 特征值1索引,支持读写操作,uint8_t类型


// Simple Profile服务UUID定义
#define SIMPLEPROFILE_SERV_UUID     0xFFE0      // 服务UUID

// 各特征值的UUID定义
#define SIMPLEPROFILE_CHAR1_UUID    0xFFE1      // 特征值1的UUID

// Simple Profile服务位域定义,用于服务添加时的控制
#define SIMPLEPROFILE_SERVICE       0x00000001

// 特征值长度定义(字节)
#define SIMPLEPROFILE_CHAR1_LEN     255         // 特征值1的数据长度

/*********************************************************************
 * 类型定义
 */

/*********************************************************************
 * 宏定义
 */

/*********************************************************************
 * Profile回调函数定义
 */

// 特征值变化时的回调函数类型定义
typedef void (*simpleProfileChange_t)(uint8_t paramID, uint8_t *pValue, uint16_t len);

// Profile回调函数结构体定义
typedef struct
{
    simpleProfileChange_t pfnSimpleProfileChange;  // 特征值变化时的回调函数指针
} simpleProfileCBs_t;

/*********************************************************************
 * API函数声明
 */

/*
 * @brief   Simple Profile服务初始化函数
 *
 * @param   services - 服务控制字,用于指定要添加的服务,支持按位或操作添加多个服务
 *
 * @return  初始化状态,SUCCESS(0x00)表示成功,其他值表示失败
 */
extern bStatus_t SimpleProfile_AddService(void);

/*
 * @brief   注册应用程序回调函数
 *
 * @note    该函数只能调用一次
 *
 * @param   appCallbacks - 应用程序回调函数结构体指针
 *
 * @return  注册状态,SUCCESS(0x00)表示成功,其他值表示失败
 */
extern bStatus_t SimpleProfile_RegisterAppCBs(simpleProfileCBs_t *appCallbacks);

/*
 * @brief   设置Simple Profile的参数
 *
 * @param   param - 参数ID
 * @param   len - 数据长度
 * @param   value - 数据指针,根据参数ID会被转换为对应的数据类型
 *
 * @return  设置状态,SUCCESS(0x00)表示成功,其他值表示失败
 */
extern bStatus_t SimpleProfile_SetParameter(uint8_t param, uint8_t len, void *value);

/*
 * @brief   获取Simple Profile的参数
 *
 * @param   param - 参数ID
 * @param   value - 数据指针,根据参数ID会被转换为对应的数据类型
 *
 * @return  获取状态,SUCCESS(0x00)表示成功,其他值表示失败
 */
extern bStatus_t SimpleProfile_GetParameter(uint8_t param, void *value);

/*
 * @brief   发送通知
 *
 * @param   connHandle - 连接句柄
 * @param   pNoti - 通知数据结构指针
 *
 * @return  操作状态,SUCCESS(0x00)表示成功,其他值表示失败
 */
extern bStatus_t SimpleProfile_Notify(uint16_t connHandle, attHandleValueNoti_t *pNoti);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif 