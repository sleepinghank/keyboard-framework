/********************************** (C) COPYRIGHT *******************************
 * 文件名称          : scanparamservice.h
 * 作者             : WCH
 * 版本             : V1.0
 * 日期             : 2018/12/11
 * 描述             : 扫描参数服务头文件,包含扫描参数服务相关的常量、类型定义和API函数声明
 *********************************************************************************
 * 版权所有 (c) 2021 南京沁恒微电子股份有限公司
 * 注意: 本软件(修改或未修改)及其二进制文件仅用于
 * 南京沁恒微电子制造的微控制器。
 *******************************************************************************/

#ifndef SCANPARAMSERVICE_H
#define SCANPARAMSERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * 包含头文件
 */

/*********************************************************************
 * 常量定义
 */

// 扫描特征值长度
#define SCAN_INTERVAL_WINDOW_CHAR_LEN    4    // 扫描间隔和窗口特征值长度(字节)
#define SCAN_PARAM_REFRESH_LEN           1    // 扫描参数刷新特征值长度(字节)

// 扫描参数刷新值
#define SCAN_PARAM_REFRESH_REQ           0x00 // 扫描参数刷新请求值

// 回调事件
#define SCAN_INTERVAL_WINDOW_SET         1    // 扫描间隔和窗口设置事件

// 获取/设置参数ID
#define SCAN_PARAM_PARAM_INTERVAL        0    // 扫描间隔参数ID
#define SCAN_PARAM_PARAM_WINDOW          1    // 扫描窗口参数ID

/*********************************************************************
 * 类型定义
 */

/*********************************************************************
 * 宏定义
 */

/*********************************************************************
 * 配置文件回调函数
 */

// 扫描参数服务回调函数类型定义
typedef void (*scanParamServiceCB_t)(uint8_t event);

/*********************************************************************
 * API函数声明
 */

/*********************************************************************
 * @fn      ScanParam_AddService
 *
 * @brief   初始化扫描参数服务,通过向GATT服务器注册GATT属性来实现
 *
 * @return  成功返回SUCCESS，失败返回FAILURE
 */
extern bStatus_t ScanParam_AddService(void);

/*********************************************************************
 * @fn      ScanParam_Register
 *
 * @brief   注册扫描参数服务的回调函数
 *
 * @param   pfnServiceCB - 回调函数指针
 *
 * @return  无
 */
extern void ScanParam_Register(scanParamServiceCB_t pfnServiceCB);

/*********************************************************************
 * @fn      ScanParam_SetParameter
 *
 * @brief   设置扫描参数服务的参数
 *
 * @param   param - 配置文件参数ID
 * @param   len - 要写入数据的长度
 * @param   value - 指向要写入数据的指针。这取决于参数ID,
 *          并将被转换为适当的数据类型(例如:uint16_t类型的数据
 *          将被转换为uint16_t指针)。
 *
 * @return  bStatus_t
 */
extern bStatus_t ScanParam_SetParameter(uint8_t param, uint8_t len, void *value);

/*********************************************************************
 * @fn      ScanParam_GetParameter
 *
 * @brief   获取扫描参数服务的参数
 *
 * @param   param - 配置文件参数ID
 * @param   value - 指向获取数据的指针。这取决于参数ID,
 *          并将被转换为适当的数据类型(例如:uint16_t类型的数据
 *          将被转换为uint16_t指针)。
 *
 * @return  bStatus_t
 */
extern bStatus_t ScanParam_GetParameter(uint8_t param, void *value);

/*********************************************************************
 * @fn      ScanParam_RefreshNotify
 *
 * @brief   通知对端刷新扫描参数
 *
 * @param   connHandle - 连接句柄
 *
 * @return  无
 */
extern void ScanParam_RefreshNotify(uint16_t connHandle);

/*********************************************************************
 * @fn      ScanParam_HandleConnStatusCB
 *
 * @brief   处理连接状态变化的回调函数
 *
 * @param   connHandle - 连接句柄
 * @param   changeType - 变化类型
 *
 * @return  无
 */
extern void ScanParam_HandleConnStatusCB(uint16_t connHandle, uint8_t changeType);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SCANPARAMSERVICE_H */
