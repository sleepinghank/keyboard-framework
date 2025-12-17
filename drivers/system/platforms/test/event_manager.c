/* Copyright 2021 QMK
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "event_manager.h"
#include "sys_config.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

// 调试输出宏
#define EVENT_MANAGER_DEBUG_PRINT(fmt, ...) printf("[EVENT_MANAGER] " fmt "\n", ##__VA_ARGS__)

/**
 * @brief   注册事件处理回调函数
 * @param   eventCb - 事件回调函数
 * @return  0xFF - 错误, 其他值 - 任务ID
 */
uint8_t OSAL_ProcessEventRegister(osal_task_event_handler_t eventCb) {
    EVENT_MANAGER_DEBUG_PRINT("OSAL_ProcessEventRegister called with eventCb = %p", (void*)eventCb);
    // 测试实现：返回固定的任务ID
    return 1;
}

/**
 * @brief   立即触发一个事件
 * @param   taskID - 任务ID
 * @param   msg_ptr - 消息指针
 * @return  NO_ERROR - 成功, 其他值 - 错误
 */
error_code_t OSAL_MsgSend(uint8_t taskID, uint16_t event) {
    EVENT_MANAGER_DEBUG_PRINT("OSAL_MsgSend called with taskID = %d, event = 0x%04X", taskID, event);
    // 测试实现：返回成功
    return 0;
}

/**
 * @brief   清除事件
 * @param   taskID - 任务ID
 * @param   event - 事件
 * @return  NO_ERROR - 成功, 其他值 - 错误
 */
error_code_t OSAL_ClearEvent(uint8_t taskID, uint8_t event) {
    EVENT_MANAGER_DEBUG_PRINT("OSAL_ClearEvent called with taskID = %d, event = 0x%02X", taskID, event);
    // 测试实现：返回成功
    return 0;
}

/**
 * @brief   设置延迟事件
 * @param   taskID - 任务ID
 * @param   event - 事件
 * @param   time - 时间
 * @return  NO_ERROR - 成功, 其他值 - 错误
 */
error_code_t OSAL_SetDelayedEvent(uint8_t taskID, uint16_t event, uint32_t time) {
    EVENT_MANAGER_DEBUG_PRINT("OSAL_SetDelayedEvent called with taskID = %d, event = 0x%04X, time = %lu", taskID, event, time);
    // 测试实现：返回成功
    return 0;
}

/**
 * @brief   启动自动重载定时事件
 * @param   taskID - 任务ID
 * @param   event - 事件
 * @param   time - 时间
 * @return  NO_ERROR - 成功, 其他值 - 错误
 */
error_code_t OSAL_StartReloadTask(uint8_t taskID, uint16_t event, uint32_t time) {
    EVENT_MANAGER_DEBUG_PRINT("OSAL_StartReloadTask called with taskID = %d, event = 0x%04X, time = %lu", taskID, event, time);
    // 测试实现：返回成功
    return 0;
}

/**
 * @brief   停止事件
 * @param   taskID - 任务ID
 * @param   event - 事件
 * @return  NO_ERROR - 成功, 其他值 - 错误
 */
error_code_t OSAL_StopTask(uint8_t taskID, uint16_t event) {
    EVENT_MANAGER_DEBUG_PRINT("OSAL_StopTask called with taskID = %d, event = 0x%04X", taskID, event);
    // 测试实现：返回成功
    return 0;
}

/**
 * @brief   获取剩余时间
 * @param   taskID - 任务ID
 * @param   event - 事件
 * @return  剩余时间
 */
uint32_t OSAL_GetRemainingTime(uint8_t taskID, uint16_t event) {
    EVENT_MANAGER_DEBUG_PRINT("OSAL_GetRemainingTime called with taskID = %d, event = 0x%04X", taskID, event);
    // 测试实现：返回0
    return 0;
}

/**
 * @brief   发送消息到任务
 * @param   taskID - 任务ID
 * @param   msg_ptr - 消息指针
 * @return  NO_ERROR - 成功, 其他值 - 错误
 */
error_code_t OSAL_MsgSendToTask(uint8_t taskID, uint8_t *msg_ptr) {
    EVENT_MANAGER_DEBUG_PRINT("OSAL_MsgSendToTask called with taskID = %d, msg_ptr = %p", taskID, (void*)msg_ptr);
    // 测试实现：返回成功
    return 0;
}

/**
 * @brief   释放消息
 * @param   msg_ptr - 消息指针
 * @return  NO_ERROR - 成功, 其他值 - 错误
 */
error_code_t OSAL_MsgDeallocate(uint8_t *msg_ptr) {
    EVENT_MANAGER_DEBUG_PRINT("OSAL_MsgDeallocate called with msg_ptr = %p", (void*)msg_ptr);
    // 测试实现：返回成功
    return 0;
}

/**
 * @brief   接收消息
 * @param   taskID - 任务ID
 * @return  消息指针
 */
uint8_t *OSAL_MsgReceive(uint8_t taskID) {
    EVENT_MANAGER_DEBUG_PRINT("OSAL_MsgReceive called with taskID = %d", taskID);
    // 测试实现：返回NULL
    return NULL;
}

/**
 * @brief   分配消息
 * @param   len - 消息长度
 * @return  消息指针
 */
uint8_t *OSAL_MsgAllocate(uint16_t len) {
    EVENT_MANAGER_DEBUG_PRINT("OSAL_MsgAllocate called with len = %d", len);
    // 测试实现：返回NULL
    return NULL;
}

/**
 * @brief   Process system
 *
 * @param   None.
 *
 * @return  None.
 */
void OSAL_SystemProcess(void) {
    EVENT_MANAGER_DEBUG_PRINT("OSAL_SystemProcess called");
    // 测试实现：空函数
}