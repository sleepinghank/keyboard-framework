/* Copyright 2022~2023 @ lokher (https://www.keychron.com)
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

/**
 * @file transport.h
 * @brief 传输通道选择层
 *
 * @details 职责：管理当前使用的传输通道（USB / 蓝牙 / 2.4G），以及切换通道时的动作。
 *
 * @section arch 架构设计
 * - transport 层：通道选择层，决定"走哪条路"
 * - wireless 层：无线抽象层，管理无线状态和收发
 *
 * @section init 初始化顺序
 * 1. Driver 层: bt_driver_init() - BLE 协议栈初始化
 * 2. Middleware 层: wireless_init() - 无线状态机初始化
 * 3. Application 层: set_transport() - 选择通道，触发驱动绑定
 */

#pragma once

/**
 * @brief 传输通道类型枚举
 * @details 使用位掩码设计，支持多通道组合检测
 */
typedef enum {
    TRANSPORT_NONE,       /**< 无通道 */
#ifdef USB_ENABLE_FLAG
    TRANSPORT_USB       = 0x01 << 0,  /**< USB 有线通道 */
#endif
#ifdef BLUETOOTH_ENABLE_FLAG
    TRANSPORT_BLUETOOTH = 0x01 << 1,  /**< 蓝牙无线通道 */
#endif
#ifdef P2P4G_ENABLE_FLAG
    TRANSPORT_P2P4      = 0x01 << 2,  /**< 2.4G 无线通道 */
#endif
    TRANSPORT_MAX,                        /**< 通道数量上限 */
} transport_t;

#define P24G_INDEX 24  /**< 2.4G 通道索引，用于 wireless_connect_ex() */

/**
 * @brief 通知驱动切换
 * @details 当传输通道切换时，通知相关模块进行驱动切换
 * @param new_transport 新的传输通道类型
 */
void transport_notify_driver_switch(transport_t new_transport);

#ifdef NKRO_ENABLE
/**
 * @brief NKRO 支持状态结构
 * @details 记录各通道是否支持 NKRO（N 键无冲）
 */
typedef struct {
#ifdef USB_ENABLE_FLAG
    bool usb : 1;       /**< USB 是否支持 NKRO */
#endif
#ifdef BLUETOOTH_ENABLE_FLAG
    bool bluetooth : 1; /**< 蓝牙是否支持 NKRO */
#endif
} nkro_t;
#endif

/* 无线通道组合定义 */
#if defined(BLUETOOTH_ENABLE_FLAG) && defined(P2P4G_ENABLE_FLAG)
#    define TRANSPORT_WIRELESS (TRANSPORT_BLUETOOTH | TRANSPORT_P2P4)
#elif defined(BLUETOOTH_ENABLE_FLAG)
#    define TRANSPORT_WIRELESS TRANSPORT_BLUETOOTH
#elif defined(P2P4G_ENABLE_FLAG)
#    define TRANSPORT_WIRELESS TRANSPORT_P2P4
#else
#    define TRANSPORT_WIRELESS 0
#endif

/**
 * @brief 设置当前传输通道
 *
 * @details 切换到指定的传输通道，并执行：
 * 1. 停用当前通道的驱动
 * 2. 激活新通道的驱动
 * 3. 设置对应的 host_driver
 * 4. 对于无线通道，调用 wireless_switch_to_xx_driver() 绑定驱动函数表
 * 5. 重置低功耗定时器
 *
 * @param new_transport 目标传输通道
 */
void set_transport(transport_t new_transport);

/**
 * @brief 获取当前传输通道
 * @return 当前活动的传输通道类型
 */
transport_t get_transport(void);

/* ==================== USB 通道接口 ==================== */
#ifdef USB_ENABLE_FLAG
/**
 * @brief USB 电源连接
 * @details 当 USB 线缆插入时调用，弱定义函数可在应用层重写
 */
void usb_power_connect(void);

/**
 * @brief USB 电源断开
 * @details 当 USB 线缆拔出时调用，弱定义函数可在应用层重写
 */
void usb_power_disconnect(void);

/**
 * @brief 启用/禁用 USB 传输
 * @param enable true=启用, false=禁用
 * @details 启用时设置 usb_driver 为当前 host_driver
 */
void usb_transport_enable(bool enable);

/**
 * @brief USB 远程唤醒
 * @details 当主机处于挂起状态时，通过远程唤醒将其唤醒
 */
void usb_remote_wakeup(void);
#endif

/* ==================== 蓝牙通道接口 ==================== */
#ifdef BLUETOOTH_ENABLE_FLAG
/**
 * @brief 启用/禁用蓝牙传输
 *
 * @param enable true=启用, false=禁用
 *
 * @details 启用时执行：
 * - 设置 wireless_driver 为当前 host_driver
 * - 断开并重新连接以同步无线状态
 * - 重置低功耗定时器
 *
 * @note 禁用时发送空报告清除按键状态
 */
void bt_transport_enable(bool enable);
#endif

/* ==================== 2.4G 通道接口 ==================== */
#ifdef P2P4G_ENABLE_FLAG
/**
 * @brief 启用/禁用 2.4G 传输
 * @param enable true=启用, false=禁用
 * @details 行为与 bt_transport_enable() 类似，但使用 2.4G 驱动
 */
void p24g_transport_enable(bool enable);
#endif
