/********************************** (C) COPYRIGHT *******************************
 * 文件名称          : battservice.h
 * 作者             : WCH
 * 版本             : V1.0
 * 日期             : 2018/12/11
 * 描述             : 电池服务头文件,包含电池服务相关的常量、类型定义和API函数声明
 *********************************************************************************
 * 版权所有 (c) 2021 南京沁恒微电子股份有限公司
 * 注意: 本软件(修改或未修改)及其二进制文件仅用于
 * 南京沁恒微电子制造的微控制器。
 *******************************************************************************/

#ifndef BATTSERVICE_H
#define BATTSERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * 包含头文件
 */

/*********************************************************************
 * 常量定义
 */

// 电池服务参数获取/设置标识
#define BATT_PARAM_LEVEL                   0  // 电池电量值
#define BATT_PARAM_CRITICAL_LEVEL          1  // 电池临界电量值
#define BATT_PARAM_SERVICE_HANDLE          2  // 电池服务句柄
#define BATT_PARAM_BATT_LEVEL_IN_REPORT    3  // 电池电量输入报告

// 回调事件定义
#define BATT_LEVEL_NOTI_ENABLED            1  // 电池电量通知使能
#define BATT_LEVEL_NOTI_DISABLED           2  // 电池电量通知禁用

// 服务的HID报告ID
#define HID_RPT_ID_BATT_LEVEL_IN           4  // 电池电量输入报告ID

/*********************************************************************
 * 类型定义
 */

// 电池服务回调函数类型
typedef void (*battServiceCB_t)(uint8_t event);

// 电池测量硬件设置回调函数类型
typedef void (*battServiceSetupCB_t)(void);

// 电池电量百分比计算回调函数类型
typedef uint8_t (*battServiceCalcCB_t)(uint16_t adcVal);

// 电池测量硬件拆卸回调函数类型
typedef void (*battServiceTeardownCB_t)(void);

/*********************************************************************
 * 宏定义
 */

/*********************************************************************
 * 配置文件回调
 */

/*********************************************************************
 * API函数
 */

/*********************************************************************
 * @fn      Batt_AddService
 *
 * @brief   初始化电池服务,通过向GATT服务器注册GATT属性来实现
 *
 * @return  成功或失败
 */
extern bStatus_t Batt_AddService(void);

/*********************************************************************
 * @fn      Batt_Register
 *
 * @brief   注册电池服务的回调函数
 *
 * @param   pfnServiceCB - 回调函数指针
 *
 * @return  无
 */
extern void Batt_Register(battServiceCB_t pfnServiceCB);

/*********************************************************************
 * @fn      Batt_SetParameter
 *
 * @brief   设置电池服务参数
 *
 * @param   param - 配置文件参数ID
 * @param   len - 要写入数据的长度
 * @param   value - 指向要写入数据的指针。这取决于参数ID,
 *          并将被转换为适当的数据类型(例如:uint16_t类型的数据
 *          将被转换为uint16_t指针)。
 *
 * @return  bStatus_t
 */
extern bStatus_t Batt_SetParameter(uint8_t param, uint8_t len, void *value);

/*********************************************************************
 * @fn      Batt_GetParameter
 *
 * @brief   获取电池参数
 *
 * @param   param - 配置文件参数ID
 * @param   value - 指向获取数据的指针。这取决于参数ID,
 *          并将被转换为适当的数据类型(例如:uint16_t类型的数据
 *          将被转换为uint16_t指针)。
 *
 * @return  bStatus_t
 */
extern bStatus_t Batt_GetParameter(uint8_t param, void *value);

/*********************************************************************
 * @fn      Batt_MeasLevel
 *
 * @brief   测量电池电量并更新服务特征中的电池电量值。
 *          如果电池电量状态特征配置为通知,且电池电量自上次
 *          测量以来发生变化,则会发送通知。
 *
 * @return  成功或失败
 */
extern bStatus_t Batt_MeasLevel(void);

/*********************************************************************
 * @fn      Batt_Setup
 *
 * @brief   设置要使用的ADC源。默认为VDD/3。
 *
 * @param   adc_ch - ADC通道,例如HAL_ADC_CHN_AIN6
 * @param   minVal - 最小电池电量
 * @param   maxVal - 最大电池电量
 * @param   sCB - 硬件设置回调
 * @param   tCB - 硬件拆卸回调
 * @param   cCB - 百分比计算回调
 *
 * @return  无
 */
extern void Batt_Setup(uint8_t adc_ch, uint16_t minVal, uint16_t maxVal,
                       battServiceSetupCB_t sCB, battServiceTeardownCB_t tCB,
                       battServiceCalcCB_t cCB);

/*********************************************************************
 * @fn      Batt_HandleConnStatusCB
 *
 * @brief   电池服务连接状态变化处理函数
 *
 * @param   connHandle - 连接句柄
 * @param   changeType - 变化类型
 *
 * @return  无
 */
void Batt_HandleConnStatusCB(uint16_t connHandle, uint8_t changeType);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* BATTSERVICE_H */
