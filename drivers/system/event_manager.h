#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "sys_config.h"

// 添加通用类型和函数指针定义
typedef uint16_t (*osal_task_event_handler_t)(uint8_t task_id, uint16_t events);

/**
 * @brief   注册事件处理回调函数
 * @param   eventCb - 事件回调函数
 * @return  0xFF - 错误, 其他值 - 任务ID
 */
extern uint8_t OSAL_ProcessEventRegister(osal_task_event_handler_t eventCb);
/**
 * @brief   立即触发一个事件
 * @param   taskID - 任务ID
 * @param   msg_ptr - 消息指针
 * @return  NO_ERROR - 成功, 其他值 - 错误
 */
extern error_code_t OSAL_MsgSend(uint8_t taskID, uint16_t event);
/**
 * @brief   清除事件
 * @param   taskID - 任务ID
 * @param   event - 事件
 * @return  NO_ERROR - 成功, 其他值 - 错误
 */
extern error_code_t OSAL_ClearEvent(uint8_t taskID, uint8_t event);
/**
 * @brief   设置延迟事件    
 * @param   taskID - 任务ID
 * @param   event - 事件
 * @param   time - 时间
 * @return  NO_ERROR - 成功, 其他值 - 错误
 */
extern error_code_t OSAL_SetDelayedEvent(uint8_t taskID, uint16_t event, uint32_t time);
/**
 * @brief   启动自动重载定时事件
 * @param   taskID - 任务ID
 * @param   event - 事件
 * @param   time - 时间
 * @return  NO_ERROR - 成功, 其他值 - 错误
 */
extern error_code_t OSAL_StartReloadTask(uint8_t taskID, uint16_t event, uint32_t time);
/**
 * @brief   停止事件    
 * @param   taskID - 任务ID
 * @param   event - 事件
 * @return  NO_ERROR - 成功, 其他值 - 错误
 */
extern error_code_t OSAL_StopTask(uint8_t taskID, uint16_t event);
/**
 * @brief   获取剩余时间    
 * @param   taskID - 任务ID
 * @param   event - 事件
 * @return  剩余时间
 */
extern uint32_t OSAL_GetRemainingTime(uint8_t taskID, uint16_t event);

/**
 * @brief   发送消息到任务
 * @param   taskID - 任务ID
 * @param   msg_ptr - 消息指针
 * @return  NO_ERROR - 成功, 其他值 - 错误
 */
extern error_code_t OSAL_MsgSendToTask(uint8_t taskID, uint8_t *msg_ptr);
/**
 * @brief   释放消息
 * @param   msg_ptr - 消息指针
 * @return  NO_ERROR - 成功, 其他值 - 错误
 */
extern error_code_t OSAL_MsgDeallocate(uint8_t *msg_ptr);
/**
 * @brief   接收消息
 * @param   taskID - 任务ID
 * @return  消息指针
 */
extern uint8_t *OSAL_MsgReceive(uint8_t taskID);
/**
 * @brief   分配消息
 * @param   len - 消息长度
 * @return  消息指针
 */
extern uint8_t *OSAL_MsgAllocate(uint16_t len);



#ifdef __cplusplus
}
#endif

