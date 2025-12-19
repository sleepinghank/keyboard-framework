/********************************** (C) COPYRIGHT *******************************
 * 文件名称          : hidkbdservice.h
 * 作者             : WCH
 * 版本             : V1.0
 * 日期             : 2018/12/10
 * 描述             : HID键盘服务头文件
 *********************************************************************************
 * 版权所有 (c) 2021 南京沁恒微电子有限公司
 * 注意: 本软件(修改或未修改)及二进制文件仅用于
 * 南京沁恒微电子制造的微控制器。
 *******************************************************************************/

#ifndef HIDKBDSERVICE_H
#define HIDKBDSERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * 包含头文件
 */

/*********************************************************************
 * 常量定义
 */

// 服务中定义的HID报告数量
#define HID_NUM_REPORTS        7

// 服务的HID报告ID
#define HID_RPT_ID_KEY_IN      0                      // 键盘输入报告ID
#define HID_RPT_ID_MOUSE_IN    1                      // 鼠标输入报告ID
#define HID_RPT_ID_LED_OUT     0                      // LED输出报告ID
#define HID_RPT_ID_FEATURE     0                      // 特性报告ID

// HID特性标志
#define HID_FEATURE_FLAGS      HID_FLAGS_REMOTE_WAKE  // 远程唤醒功能标志

/*********************************************************************
 * 类型定义
 */

/*********************************************************************
 * 宏定义
 */

/*********************************************************************
 * 配置文件回调函数
 */

/*********************************************************************
 * API函数声明
 */

/*********************************************************************
 * @fn      Hid_AddService
 *
 * @brief   通过向GATT服务器注册GATT属性来初始化键盘的HID服务
 *
 * @param   none - 无参数
 *
 * @return  成功返回SUCCESS，失败返回FAILURE
 */
extern bStatus_t Hid_AddService(void);

/*********************************************************************
 * @fn      Hid_SetParameter
 *
 * @brief   设置HID键盘参数
 *
 * @param   id     - HID报告ID
 * @param   type   - HID报告类型
 * @param   uuid   - 属性UUID
 * @param   len    - 要写入的数据长度
 * @param   pValue - 指向要写入数据的指针。这取决于输入参数，
 *                   并将被转换为适当的数据类型
 *                   (例如：uint16_t类型的数据将被转换为uint16_t指针)
 *
 * @return  GATT状态码
 */
extern uint8_t Hid_SetParameter(uint8_t id, uint8_t type, uint16_t uuid, uint8_t len,
                                void *pValue);

/*********************************************************************
 * @fn      Hid_GetParameter
 *
 * @brief   获取HID键盘参数
 *
 * @param   id     - HID报告ID
 * @param   type   - HID报告类型
 * @param   uuid   - 属性UUID
 * @param   pLen   - 要读取的数据长度指针
 * @param   pValue - 指向获取数据的指针。这取决于输入参数，
 *                   并将被转换为适当的数据类型
 *                   (例如：uint16_t类型的数据将被转换为uint16_t指针)
 *
 * @return  GATT状态码
 */
extern uint8_t Hid_GetParameter(uint8_t id, uint8_t type, uint16_t uuid, uint16_t *pLen, void *pValue);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* HIDKBDSERVICE_H */
