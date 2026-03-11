/* Copyright 2023 Keychron
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY of any kind, either express or implied.
 * See the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "sys_config.h"
#include "sys_error.h"

/*********************************************************************
 * 蓝牙驱动接口方法 - CH584 蓝牙驱动实现
 *********************************************************************/

/*********************************************************************
 * @fn      bt_driver_setup
 *
 * @brief   系统初始化时调用的设置函数
 *          在主初始化之前的早期阶段执行
 *
 * @return  none
 */
void bt_driver_setup(void);

/*********************************************************************
 * @fn      bt_driver_init
 *
 * @brief   蓝牙驱动初始化函数
 *          初始化BLE协议栈、GAP角色、HID设备和HID仿真层
 *
 * @param   wakeup_from_low_power - 如果从低功耗模式唤醒则为TRUE
 *
 * @return  none
 */
void bt_driver_init(bool wakeup_from_low_power);

/*********************************************************************
 * @fn      bt_driver_connect_ex
 *
 * @brief   向已配对主机发起连接请求
 *          尝试连接到之前配对的蓝牙主机
 *
 * @param   host_idx - 主机索引 (0-4 对应 BLE_INDEX_1 到 BLE_INDEX_5)
 * @param   timeout - 超时值（秒）
 *
 * @return  none
 */
void bt_driver_connect_ex(uint8_t host_idx, uint16_t timeout);

/*********************************************************************
 * @fn      bt_driver_pairing_ex
 *
 * @brief   进入配对模式以配对新主机
 *          启用广播以允许新设备配对
 *
 * @param   host_idx - 主机索引
 * @param   param - 配对参数（pairing_param_t结构体）
 *
 * @return  none
 */
void bt_driver_pairing_ex(uint8_t host_idx, void *param);

/*********************************************************************
 * @fn      bt_driver_disconnect
 *
 * @brief   强制断开与当前已连接主机的连接
 *          终止活动的蓝牙连接
 *
 * @return  none
 */
void bt_driver_disconnect(void);

/*********************************************************************
 * @fn      bt_driver_send_keyboard
 *
 * @brief   发送标准键盘报告（6键滚动）
 *          通过蓝牙传输键盘HID报告
 *
 * @param   report - 键盘报告数据（20字节）
 *
 * @return  uint8_t - 发送状态（0:成功, 非0:失败）
 */
uint8_t bt_driver_send_keyboard(uint8_t *report);

/*********************************************************************
 * @fn      bt_driver_send_nkro
 *
 * @brief   发送NKRO（N键滚动）键盘报告
 *          通过蓝牙传输全键滚动键盘HID报告
 *
 * @param   report - NKRO键盘报告数据（20字节）
 *
 * @return  uint8_t - 发送状态（0:成功, 非0:失败）
 */
uint8_t bt_driver_send_nkro(uint8_t *report);

/*********************************************************************
 * @fn      bt_driver_send_consumer
 *
 * @brief   发送消费者（媒体）控制报告
 *          传输媒体控制HID报告（播放、暂停、音量等）
 *
 * @param   report - 消费者控制码（16位值）
 *
 * @return  uint8_t - 发送状态（0:成功, 非0:失败）
 */
uint8_t bt_driver_send_consumer(uint16_t report);

/*********************************************************************
 * @fn      bt_driver_send_system
 *
 * @brief   发送系统控制报告
 *          传输系统控制HID报告（电源、休眠、唤醒等）
 *
 * @param   report - 系统控制码（16位值）
 *
 * @return  uint8_t - 发送状态（0:成功, 非0:失败）
 */
uint8_t bt_driver_send_system(uint16_t report);

/*********************************************************************
 * @fn      bt_driver_send_mouse
 *
 * @brief   发送鼠标报告
 *          传输鼠标HID报告（按钮、移动、滚轮）
 *
 * @param   report - 鼠标报告数据（20字节）
 *
 * @return  uint8_t - 发送状态（0:成功, 非0:失败）
 */
uint8_t bt_driver_send_mouse(uint8_t *report);

/*********************************************************************
 * @fn      bt_driver_update_bat_level
 *
 * @brief   更新电池电量信息
 *          向主机通知当前电池电量百分比
 *
 * @param   bat_lvl - 电池电量百分比（0-100）
 *
 * @return  none
 */
void bt_driver_update_bat_level(uint8_t bat_lvl);

/**
 * @brief 控制蓝牙广播开关
 * @param enable true开启广播 false关闭广播
 */
void bt_driver_set_advertising(bool enable);

/*********************************************************************
 * @fn      bt_driver_send_ptp
 *
 * @brief   发送 PTP (Precision Touchpad) 报告
 *          传输精确触控板 HID 报告
 *
 * @param   report - PTP 报告数据
 * @param   len - 报告数据长度
 *
 * @return  uint8_t - 发送状态（0:成功, 非0:失败）
 */
uint8_t bt_driver_send_ptp(uint8_t *report, uint8_t len);
