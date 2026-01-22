#pragma once

#ifdef __cplusplus
"C" {
#endif

#include "sys_config.h"

// 添加通用类型和函数指针定义
typedef uint16_t (*osal_task_event_handler_t)(uint8_t task_id, uint16_t events);

/**
 * @brief   注册事件处理回调函数
 * @param   eventCb - 事件回调函数
 * @return  0xFF - 错误, 其他值 - 任务ID
 */
uint8_t OSAL_ProcessEventRegister(osal_task_event_handler_t eventCb);
/**
 * @brief   立即触发一个事件
 * @param   taskID - 任务ID
 * @param   msg_ptr - 消息指针
 * @return  NO_ERROR - 成功, 其他值 - 错误
 */
error_code_t OSAL_SetEvent(uint8_t taskID, uint16_t event);
/**
 * @brief   清除事件
 * @param   taskID - 任务ID
 * @param   event - 事件
 * @return  NO_ERROR - 成功, 其他值 - 错误
 */
error_code_t OSAL_ClearEvent(uint8_t taskID, uint8_t event);
/**
 * @brief   设置延迟事件    
 * @param   taskID - 任务ID
 * @param   event - 事件
 * @param   time - 时间
 * @return  NO_ERROR - 成功, 其他值 - 错误
 */
error_code_t OSAL_SetDelayedEvent(uint8_t taskID, uint16_t event, uint32_t time);
/**
 * @brief   启动自动重载定时事件
 * @param   taskID - 任务ID
 * @param   event - 事件
 * @param   time - 时间
 * @return  NO_ERROR - 成功, 其他值 - 错误
 */
error_code_t OSAL_StartReloadTask(uint8_t taskID, uint16_t event, uint32_t time);
/**
 * @brief   停止事件    
 * @param   taskID - 任务ID
 * @param   event - 事件
 * @return  NO_ERROR - 成功, 其他值 - 错误
 */
error_code_t OSAL_StopTask(uint8_t taskID, uint16_t event);
/**
 * @brief   获取剩余时间    
 * @param   taskID - 任务ID
 * @param   event - 事件
 * @return  剩余时间
 */
uint32_t OSAL_GetRemainingTime(uint8_t taskID, uint16_t event);

/**
 * @brief   发送消息到任务
 * @param   taskID - 任务ID
 * @param   msg_ptr - 消息指针
 * @return  NO_ERROR - 成功, 其他值 - 错误
 */
error_code_t OSAL_MsgSendToTask(uint8_t taskID, uint8_t *msg_ptr);
/**
 * @brief   释放消息
 * @param   msg_ptr - 消息指针
 * @return  NO_ERROR - 成功, 其他值 - 错误
 */
error_code_t OSAL_MsgDeallocate(uint8_t *msg_ptr);
/**
 * @brief   接收消息
 * @param   taskID - 任务ID
 * @return  消息指针
 */
uint8_t *OSAL_MsgReceive(uint8_t taskID);
/**
 * @brief   分配消息
 * @param   len - 消息长度
 * @return  消息指针
 */
uint8_t *OSAL_MsgAllocate(uint16_t len);

/**
 * @brief   Process system
 *
 * @param   None.
 *
 * @return  None.
 */
void OSAL_SystemProcess(void);

/**
 * @brief   get last period of time for this event
 *
 * @param   taskID - task ID of event
 * @param   event - event value
 *
 * @return  the timer's tick count if found, zero otherwise.
 */
uint32_t OSAL_GetTaskTimer( uint8_t taskID, uint16_t event );




#ifdef __cplusplus
}
#endif



// OSAL 使用案例
// ### 2. 任务注册流程

// ```c
// // 标准任务注册模式（以keyboard.c为例）
// uint8_t keyboard_taskID = 0;

// // 1. 定义任务事件处理器
// uint16_t keyboard_process_event(uint8_t task_id, uint16_t events) {
//     if(events & KEYBOARD_LED_BLINK_EVENT) {
//         // 处理LED闪烁
//         return events ^ KEYBOARD_LED_BLINK_EVENT;
//     }
//     // 处理其他事件...
//     return 0;
// }

// // 2. 在模块初始化时注册任务
// void keyboard_init() {
//     keyboard_taskID = OSAL_ProcessEventRegister(keyboard_process_event);
//     // 启动定时任务
//     OSAL_start_reload_task(keyboard_taskID, KEYBOARD_VBAT_INFO_EVENT, GET_VBAT_INFO_INTERVAL);
// }
// ```

// **关键点**：
// - 每个模块独立注册自己的任务
// - 返回未处理的事件标志
// - 事件处理后需要清除对应标志位

// ### 3. 事件调度流程

// ```c
// // 事件触发的三种方式

// // 方式1：立即事件
// OSAL_set_event(keyboard_taskID, KEYBOARD_LED_BLINK_EVENT);

// // 方式2：单次延时事件
// OSAL_start_task(keyboard_taskID, KEYBOARD_VBAT_LED_TIMEOUT_EVENT,
//                 MS1_TO_SYSTEM_TIME(2000));

// // 方式3：循环定时事件
// OSAL_start_reload_task(keyboard_taskID, KEYBOARD_VBAT_INFO_EVENT,
//                        GET_VBAT_INFO_INTERVAL);
// ```

// ---

// ## 使用方法

// ### 1. 基础使用模式

// #### 步骤1：定义事件类型

// ```c
// // 在模块头文件中定义
// #define KEYBOARD_LED_BLINK_EVENT     (1<<0)   // LED闪烁事件
// #define KEYBOARD_VBAT_INFO_EVENT     (1<<2)   // 电量检测事件
// #define KEYBOARD_BACKLIGHT_TIMEOUT_EVENT  (1<<4)   // 背光超时事件
// ```

// #### 步骤2：实现事件处理器

// ```c
// uint16_t keyboard_process_event(uint8_t task_id, uint16_t events) {
//     // 必须返回未处理的事件
//     uint16_t unhandled = 0;

//     // 检查并处理每个事件
//     if(events & KEYBOARD_LED_BLINK_EVENT) {
//         // LED闪烁逻辑
//         GPIOA_InverseBits(GPIO_Pin_15);

//         // 清除已处理的事件位
//         events ^= KEYBOARD_LED_BLINK_EVENT;
//     }

//     if(events & KEYBOARD_VBAT_INFO_EVENT) {
//         // 电量检测逻辑
//         vbat_info = keyboard_get_batt_info();

//         // 低电压告警
//         if(vbat_led_num == 6) {
//             OSAL_start_task(keyboard_taskID, KEYBOARD_VBAT_LED_TIMEOUT_EVENT,
//                           MS1_TO_SYSTEM_TIME(2000));
//         }

//         events ^= KEYBOARD_VBAT_INFO_EVENT;
//     }

//     // 处理完成后返回未处理的事件
//     return events;
// }
// ```

// #### 步骤3：注册任务并启动事件

// ```c
// void keyboard_init() {
//     // 1. 注册任务，获取taskID
//     keyboard_taskID = OSAL_ProcessEventRegister(keyboard_process_event);

//     // 2. 启动循环定时事件（周期性执行）
//     OSAL_start_reload_task(keyboard_taskID, KEYBOARD_VBAT_INFO_EVENT,
//                           GET_VBAT_INFO_INTERVAL);

//     // 3. 可以随时触发立即事件
//     OSAL_set_event(keyboard_taskID, KEYBOARD_LED_BLINK_EVENT);
// }
// ```

// ### 2. 高级使用技巧

// #### 2.1 事件组合处理

// ```c
// // 同时等待多个事件
// uint16_t my_process_event(uint8_t task_id, uint16_t events) {
//     if(events & (EVENT_A | EVENT_B)) {  // 处理A或B事件
//         // 共享处理逻辑
//         return events ^ (EVENT_A | EVENT_B);
//     }
//     return 0;
// }
// ```

// #### 2.2 事件转发

// ```c
// // 将事件转发给其他任务
// if(events & FORWARD_TO_OTHER_TASK_EVENT) {
//     OSAL_set_event(other_taskID, OTHER_TASK_EVENT);
//     return events ^ FORWARD_TO_OTHER_TASK_EVENT;
// }
// ```

// #### 2.3 动态启动/停止事件

// ```c
// // 根据条件启动事件
// if(condition) {
//     OSAL_start_reload_task(keyboard_taskID, PERIODIC_EVENT, INTERVAL);
// } else {
//     OSAL_stop_task(keyboard_taskID, PERIODIC_EVENT);
// }
// ```

// ---

// ## 项目应用实例

// ### 实例1：键盘LED闪烁控制 (keyboard.c)

// **场景**：键盘需要控制LED闪烁来指示不同状态（配对、断开连接、低电量）

// ```c
// // 1. 定义事件和变量
// #define KEYBOARD_LED_BLINK_EVENT            1<<0
// #define KEYBOARD_VBAT_LED_BLINK_EVENT       1<<6
// #define KEYBOARD_VBAT_LED_TIMEOUT_EVENT     1<<7

// #define PAIRING_LED_BLINK_INTERVAL          1600*1/4  // 快闪：400ms
// #define DISCONNECT_LED_BLINK_INTERVAL       1600*1/2  // 慢闪：800ms

// uint8_t keyboard_taskID = 0;
// uint8_t vbat_led_blink_count = 0;      // 闪烁计数
// uint8_t vbat_led_blink_state = 0;      // 闪烁状态

// // 2. 事件处理函数
// uint16_t keyboard_process_event(uint8_t task_id, uint16_t events) {
//     // LED闪烁事件
//     if(events & KEYBOARD_LED_BLINK_EVENT) {
//         GPIOA_InverseBits(GPIO_Pin_15);           // 翻转LED引脚
//         return events ^ KEYBOARD_LED_BLINK_EVENT;
//     }

//     // 低电量闪烁事件
//     if(events & KEYBOARD_VBAT_LED_BLINK_EVENT) {
//         vbat_led_blink_count++;

//         if(vbat_led_blink_count >= 10) {  // 闪烁10次后停止
//             vbat_led_blink_count = 0;
//             OSAL_stop_task(keyboard_taskID, KEYBOARD_VBAT_LED_BLINK_EVENT);
//             OSAL_stop_task(keyboard_taskID, KEYBOARD_VBAT_LED_TIMEOUT_EVENT);
//             GPIOB_ModeCfg(GPIO_Pin_23, GPIO_ModeIN_PD);  // 关闭LED
//         } else {
//             // 翻转LED状态
//             vbat_led_blink_state = !vbat_led_blink_state;
//             if(vbat_led_blink_state) {
//                 GPIOB_SetBits(GPIO_Pin_23);
//             } else {
//                 GPIOB_ResetBits(GPIO_Pin_23);
//             }
//         }
//         return events ^ KEYBOARD_VBAT_LED_BLINK_EVENT;
//     }

//     // 超时停止事件
//     if(events & KEYBOARD_VBAT_LED_TIMEOUT_EVENT) {
//         OSAL_stop_task(keyboard_taskID, KEYBOARD_VBAT_LED_BLINK_EVENT);
//         GPIOB_ModeCfg(GPIO_Pin_23, GPIO_ModeIN_PD);
//         return events ^ KEYBOARD_VBAT_LED_TIMEOUT_EVENT;
//     }

//     return 0;
// }

// // 3. 启动闪烁的公共接口
// void keyboard_start_pairing_indicator() {
//     // 启动快闪
//     OSAL_start_reload_task(keyboard_taskID, KEYBOARD_LED_BLINK_EVENT,
//                           PAIRING_LED_BLINK_INTERVAL);
// }

// void keyboard_start_low_voltage_indicator() {
//     // 启动慢闪10次，然后超时停止
//     vbat_led_blink_count = 0;
//     OSAL_start_reload_task(keyboard_taskID, KEYBOARD_VBAT_LED_BLINK_EVENT,
//                           VBAT_LED_BLINK_INTERVAL);
//     OSAL_start_task(keyboard_taskID, KEYBOARD_VBAT_LED_TIMEOUT_EVENT,
//                    VBAT_LED_BLINK_TIMEOUT);
// }
// ```

// **应用场景**：
// - 配对模式：LED快闪指示可配对状态
// - 断开连接：LED慢闪指示连接断开
// - 低电量：LED闪烁10次后自动停止

// ### 实例2：电量检测与低功耗管理 (access.c)

// **场景**：蓝牙模块需要周期性检测电量，并在低电量时进入低功耗模式

// ```c
// // 1. 定义事件
// #define ACCESS_SLEEP_EVT           1<<0
// #define ACCESS_IDEL_SLEEP_EVT      1<<1
// #define ACCESS_WAKE_UP_EVT         1<<2

// uint8_t access_taskId = INVALID_TASK_ID;
// access_state_t access_state;

// // 2. 事件处理函数
// uint16_t access_ProcessEvent(uint8_t task_id, uint16_t events) {
//     // 深度睡眠事件
//     if(events & ACCESS_SLEEP_EVT) {
//         access_state.sleep_en = TRUE;
//         access_state.deep_sleep_flag = TRUE;

//         // 如果正在连接，先断开连接
//         uint8_t ble_state;
//         GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
//         if(ble_state == GAPROLE_CONNECTED) {
//             hidEmu_disconnect();  // 断开蓝牙连接
//             // 延迟等待断开完成
//             OSAL_start_task(access_taskId, ACCESS_SLEEP_EVT,
//                            DISCONNECT_WAIT_REPORT_END_TIMEOUT);
//             return (events ^ ACCESS_SLEEP_EVT);
//         }

//         // 停止广播
//         hidEmu_adv_enable(DISABLE);
//         access_state.ble_idx = BLE_INDEX_IDEL;

//         // 关闭外设
//         hidDevBattCB(BATT_LEVEL_NOTI_DISABLED);
//         red_led_off();

//         // 进入睡眠
//         peripheral_enter_sleep();
//         LowPower_Sleep(NULL);  // 只保留96K SRAM供电

//         return (events ^ ACCESS_SLEEP_EVT);
//     }

//     // 空闲睡眠事件（保持蓝牙连接）
//     if(events & ACCESS_IDEL_SLEEP_EVT) {
//         access_state.sleep_en = TRUE;
//         access_state.idel_sleep_flag = TRUE;

//         // 检查蓝牙状态
//         uint8_t ble_state;
//         GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
//         if(ble_state == GAPROLE_ADVERTISING) {
//             // 停止广播
//             access_state.ble_idx = BLE_INDEX_IDEL;
//             hidEmu_adv_enable(DISABLE);
//         }

//         // 进入空闲睡眠（保持连接）
//         hidDevBattCB(BATT_LEVEL_NOTI_DISABLED);
//         peripheral_enter_sleep();

//         return (events ^ ACCESS_IDEL_SLEEP_EVT);
//     }

//     // 唤醒事件
//     if(events & ACCESS_WAKE_UP_EVT) {
//         if(access_state.ble_idx == BLE_INDEX_IDEL) {
//             // 重新开始广播
//             hidEmu_adv_enable(ENABLE);
//         }

//         access_state.sleep_en = FALSE;
//         access_state.deep_sleep_flag = FALSE;
//         access_state.idel_sleep_flag = FALSE;

//         return (events ^ ACCESS_WAKE_UP_EVT);
//     }

//     return 0;
// }

// // 3. 公共接口
// void access_enter_sleep() {
//     OSAL_set_event(access_taskId, ACCESS_SLEEP_EVT);
// }

// void access_enter_idel_sleep() {
//     OSAL_set_event(access_taskId, ACCESS_IDEL_SLEEP_EVT);
// }

// void access_wakeup() {
//     OSAL_set_event(access_taskId, ACCESS_WAKE_UP_EVT);
// }
// ```

// **应用场景**：
// - 无操作5分钟后自动进入空闲睡眠
// - 断开连接10分钟后进入深度睡眠
// - 按键或触摸事件唤醒系统

// ### 实例3：触摸板管理 (touch_manager.c)

// **场景**：触摸板需要周期性扫描，并处理触摸中断

// ```c
// // 1. 定义事件
// #define TOUCH_SCAN_EVENT            1<<0
// #define TOUCH_INTERRUPT_EVENT       1<<1
// #define TOUCH_REG_INIT_EVT          1<<2

// uint8_t touch_taskID = 0;

// // 2. 事件处理函数
// uint16_t touch_process_event(uint8_t task_id, uint16_t events) {
//     // 周期性扫描事件
//     if(events & TOUCH_SCAN_EVENT) {
//         // 读取触摸数据
//         uint8_t touch_data = touch_read_data();

//         // 处理触摸数据
//         if(touch_data_changed(touch_data)) {
//             // 通知键盘处理触摸事件
//             OSAL_set_event(keyboard_taskID, KEYBOARD_TOUCH_EVENT);

//             // 唤醒系统（如果正在睡眠）
//             if(access_state.sleep_en) {
//                 access_wakeup();
//             }
//         }

//         // 继续下一次扫描
//         OSAL_start_reload_task(touch_taskID, TOUCH_SCAN_EVENT, TOUCH_SCAN_INTERVAL);
//         return events ^ TOUCH_SCAN_EVENT;
//     }

//     // 中断事件
//     if(events & TOUCH_INTERRUPT_EVENT) {
//         // 处理触摸中断
//         touch_handle_interrupt();
//         return events ^ TOUCH_INTERRUPT_EVENT;
//     }

//     // 寄存器初始化事件
//     if(events & TOUCH_REG_INIT_EVT) {
//         touch_init_registers();
//         return events ^ TOUCH_REG_INIT_EVT;
//     }

//     return 0;
// }

// // 3. 触摸中断处理函数
// void touch_interrupt_handler() {
//     // 在GPIO中断中调用
//     OSAL_set_event(touch_taskID, TOUCH_INTERRUPT_EVENT);
// }

// // 4. 初始化
// void touch_manager_init() {
//     touch_taskID = OSAL_ProcessEventRegister(touch_process_event);

//     // 启动周期性扫描
//     OSAL_start_reload_task(touch_taskID, TOUCH_SCAN_EVENT, TOUCH_SCAN_INTERVAL);
// }
// ```

// **应用场景**：
// - 每10ms扫描一次触摸板
// - 触摸中断立即响应
// - 触摸时自动唤醒系统


