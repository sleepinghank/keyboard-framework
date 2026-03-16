#include "event_manager.h"
#include "sys_error.h"
#include "CH58xBLE_LIB.h"
/**
 * @brief   注册事件处理回调函数
 * @param   eventCb - 事件回调函数
 * @return  0xFF - 错误, 其他值 - 任务ID
 */
uint8_t OSAL_ProcessEventRegister(osal_task_event_handler_t eventCb)
{
    return TMOS_ProcessEventRegister(eventCb); 
}

/**
 * @brief   立即触发一个事件
 * @param   taskID - 任务ID
 * @param   event - 事件值
 * @return  NO_ERROR - 成功，其他值 - 错误
 */
error_code_t OSAL_SetEvent(uint8_t taskID, uint16_t event)
{ 
   uint8_t err = tmos_set_event(taskID, event);
   if(err == 0) return NO_ERROR;
   else         return ERROR_EVENT_TRIGGER_FAILED;//事件触发失败
}

/**
 * @brief   清除一个已超时的事件,不能在事件自身的处理函数中使用
 * @param   taskID - 任务ID
 * @param   event - 事件值
 * @return  NO_ERROR - 成功，其他值 - 错误
 */
error_code_t OSAL_ClearEvent(uint8_t taskID, uint8_t event)
{
    uint8_t err = tmos_clear_event(taskID, event);
    if(err == 0) return NO_ERROR;
    else         return ERROR_EVENT_CLEAR_FAILED;//事件清除失败
}


/**
 * @brief   启动一个定时事件
 * @param   taskID - 任务ID
 * @param   event - 事件值
 * @param   time - 超时时间
 * @return  NO_ERROR - 成功，其他值 - 错误
 */
error_code_t OSAL_SetDelayedEvent(uint8_t taskID, uint16_t event, uint32_t time)
{
    BOOL err = tmos_start_task(taskID, event, time);
    if(err == TRUE) return NO_ERROR;
    else         return ERROR_EVENT_START_DELAYED_FAILED;//启动定时事件失败
}


/**
 * @brief   启动一个自动重载的定时事件
 * @param   taskID - 任务ID
 * @param   event - 事件值
 * @param   time - 超时时间
 * @return  NO_ERROR - 成功，其他值 - 错误
 */
error_code_t OSAL_StartReloadTask(uint8_t taskID, uint16_t event, uint32_t time)
{
    uint8_t err = tmos_start_reload_task(taskID, event, time);
    if(err == 0) return NO_ERROR;
    else if(err == 0x08)         return ERROR_NO_TIMER_AVAILABLE;//无可用定时器
    else                         return ERROR_EVENT_START_RELOAD_FAILED;//启动自动重载定时事件失败
}

/**
 * @brief   停止一个事件
 * @param   taskID - 任务ID
 * @param   event - 事件值
 * @return  NO_ERROR - 成功，其他值 - 错误
 */
error_code_t OSAL_StopTask(uint8_t taskID, uint16_t event)
{
    uint8_t err = tmos_stop_task(taskID, event);
    if(err == 0) return NO_ERROR;
    else         return ERROR_EVENT_STOP_FAILED;//事件停止失败
}

/**
 * @brief   获取事件的剩余时间
 * @param   taskID - 任务ID
 * @param   event - 事件值
 * @return  定时器的剩余时间,如果未找到返回0
 */
uint32_t OSAL_GetRemainingTime(uint8_t taskID, uint16_t event)
{
    return tmos_get_task_timer(taskID, event);
}



/**
 * @brief   发送消息到指定任务,将触发events&SYS_EVENT_MSG回调
 * @param   taskID - 目标任务ID
 * @param   msg_ptr - 消息指针
 * @return  NO_ERROR - 成功，其他值 - 错误
 */
error_code_t OSAL_MsgSendToTask(uint8_t taskID, uint8_t *msg_ptr)
{
    uint8_t err = tmos_msg_send(taskID, msg_ptr);
    if(err == 0) return NO_ERROR;
    else if(err == 0x03)         return ERROR_INVALID_TASK_ID;//无效任务ID
    else if(err == 0x05)         return ERROR_INVALID_MSG_POINTER;//无效消息指针
    else                         return ERROR_EVENT_SEND_TO_TASK_FAILED;//事件发送失败
}

/**
 * @brief   删除一个消息
 * @param   msg_ptr - 消息指针
 * @return  NO_ERROR - 成功，其他值 - 错误
 */
error_code_t OSAL_MsgDeallocate(uint8_t *msg_ptr)
{
   uint8_t err = tmos_msg_deallocate(msg_ptr);
   if(err == 0) return NO_ERROR;
   else         return ERROR_EVENT_DEALLOCATE_FAILED;//事件删除失败
}


/**
 * @brief   接收消息
 * @param   taskID - 任务ID
 * @return  消息指针,如果没有消息则返回NULL
 */
uint8_t *OSAL_MsgReceive(uint8_t taskID)
{
    return tmos_msg_receive(taskID);
}

/**
 * @brief   分配消息缓冲区
 * @param   len - 消息长度
 * @return  分配的缓冲区指针,分配失败返回NULL
 */
uint8_t *OSAL_MsgAllocate(uint16_t len)
{
    return tmos_msg_allocate(len);
}

/**
 * @brief   OSAL 系统单次处理（非阻塞）
 *          处理一轮事件后返回，用于自定义主循环
 */
__attribute__((used))
__HIGH_CODE
void OSAL_SystemProcessOnce(void) {
    TMOS_SystemProcess();
}

__attribute__((used))
__HIGH_CODE
void OSAL_SystemProcess(void){
    while(1)
    {
        TMOS_SystemProcess();
    }
}
uint32_t OSAL_GetTaskTimer( uint8_t taskID, uint16_t event )
{
    return tmos_get_task_timer( taskID, event );
}