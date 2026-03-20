/* Copyright 2023~2025 @ lokher (https://www.keychron.com)
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
 * @file wireless.h
 * @brief 无线传输抽象层
 *
 * @details 职责：对无线连接状态和无线收发做统一抽象，与具体芯片解耦。
 *
 * @section arch 架构层次
 * - transport 层：通道选择（走 USB 还是蓝牙/2.4G）
 * - wireless 层：无线抽象（状态机、事件队列、HID 收发）
 * - driver 层：具体驱动实现（bt_driver, p24g_driver）
 *
 * @section state 状态机
 * 状态转换流程：
 * @code
 * WT_RESET → WT_INITIALIZED → WT_DISCONNECTED ⇄ WT_PARING
 *                                ↓↑              ↓
 *                          WT_RECONNECTING → WT_CONNECTED ⇄ WT_SUSPEND
 * @endcode
 */

#pragma once

#include "kb904/config.h"
#include "wireless_event_type.h"
#include "bt_driver.h"
#ifdef P2P4G_ENABLE_FLAG
#include "p24g_driver.h"
#endif

// #define KC_DEBUG

/* ==================== 调试输出配置 ==================== */
#ifdef KC_DEBUG
#    define kc_printf dprintf
#else
#    define kc_printf(format, ...)
#endif

/* ==================== 低功耗配置 ==================== */
#ifndef LOW_POWER_MODE
#    define LOW_POWER_MODE PM_STOP
#endif

/* 蓝牙模块中断唤醒引脚 */
#ifndef BLUETOOTH_INT_INPUT_PIN
#    define WAKE_PIN A5
#endif

/* ==================== 无线状态枚举 ==================== */
/**
 * @brief 无线传输状态枚举
 *
 * @details 描述无线连接的完整生命周期状态
 */
typedef enum {
    WT_RESET,         /**< 复位状态，上电初始状态 */
    WT_INITIALIZED,   /**< 已初始化，驱动已加载 */
    WT_DISCONNECTED,  /**< 已断开，未连接任何主机 */
    WT_CONNECTED,     /**< 已连接，与主机正常通信 */
    WT_PARING,        /**< 配对中，等待新主机配对 */
    WT_RECONNECTING,  /**< 重连中，尝试连接已配对主机 */
    WT_SUSPEND        /**< 挂起，低功耗模式 */
} wt_state_t;

//系统类型枚举
enum
{
    ANDROID,
    IOS,
    WIN,
    MAC
};
#define ALL_SYSTEM 0xff // for combo map, 0xff means trigger for all system
#define APPLE_SYSTEM (IOS | MAC) // for combo map, 0x0c means trigger for apple system (macOS and iOS), but not trigger for windows and android
#define OTHER_SYSTEM (ALL_SYSTEM & ~APPLE_SYSTEM) // for combo map, 0x03 means trigger for windows and android, but not trigger for macOS and iOS
extern uint8_t host_system_type;

/* ==================== 驱动函数表结构 ==================== */
/**
 * @brief 无线驱动函数表类型
 *
 * @details 定义无线驱动的统一接口，由具体驱动（bt_driver, p24g_driver）实现。
 *
 * 使用方式：
 * 1. 驱动层实现这些函数
 * 2. 调用 wireless_set_transport() 注册函数表
 * 3. wireless 层通过 wireless_transport.xxx() 调用驱动
 */
typedef struct {
    void (*init)(bool wakeup_from_low_power);           /**< 初始化驱动 */
    void (*connect_ex)(uint8_t host_idx, uint16_t timeout); /**< 连接到指定主机 */
    void (*pairing_ex)(uint8_t host_idx, void *param);  /**< 进入配对模式 */
    void (*disconnect)(void);                           /**< 断开连接 */
    uint8_t (*send_keyboard)(uint8_t *report);          /**< 发送键盘报告 */
    uint8_t (*send_nkro)(uint8_t *report);              /**< 发送 NKRO 报告 */
    uint8_t (*send_consumer)(uint16_t usage);           /**< 发送消费者键 */
    uint8_t (*send_system)(uint16_t usage);             /**< 发送系统键 */
    uint8_t (*send_mouse)(uint8_t *report);             /**< 发送鼠标报告 */
    void (*update_bat_level)(uint8_t level);            /**< 更新电池电量 */
} wt_func_t;

/* ==================== 全局变量声明 ==================== */
/** @brief 当前活动的无线驱动函数表 */
extern wt_func_t wireless_transport;

/* ==================== 初始化函数 ==================== */
/**
 * @brief 初始化无线管理层
 *
 * @details 执行以下操作：
 * - 设置无线状态为 WT_INITIALIZED
 * - 加载无线配置（如果启用 EECONFIG）
 * - 初始化报告缓冲区
 *
 * @note 应在 system_init_middleware() 中调用
 */
void wireless_init(void);

/**
 * @brief 重置无线配置为默认值
 */
void wireless_config_reset(void);

/* ==================== 驱动注册函数 ==================== */
/**
 * @brief 设置无线驱动函数表
 *
 * @param transport 指向驱动函数表的指针
 *
 * @details 使用 memcpy 复制函数表到 wireless_transport
 */
void wireless_set_transport(wt_func_t *transport);

/* ==================== 驱动切换函数 ==================== */
#ifdef BLUETOOTH_ENABLE_FLAG
/**
 * @brief 切换到蓝牙驱动
 *
 * @details 将 bt_driver 函数表绑定到 wireless_transport
 * @note 由 set_transport(TRANSPORT_BLUETOOTH) 内部调用
 */
void wireless_switch_to_bt_driver(void);
#endif

#ifdef P2P4G_ENABLE_FLAG
/**
 * @brief 注册 2.4G 驱动
 * @param driver 2.4G 驱动函数表指针
 */
void wireless_register_p24g_driver(p24g_driver_t *driver);

/**
 * @brief 切换到 2.4G 驱动
 * @note 由 set_transport(TRANSPORT_P2P4) 内部调用
 */
void wireless_switch_to_p24g_driver(void);
#endif

#ifdef USB_ENABLE_FLAG
/**
 * @brief 切换到 USB 模式
 * @note 仅做状态记录，USB 驱动独立于 wireless 层
 */
void wireless_switch_to_usb_mode(void);
#endif

/* ==================== 连接管理函数 ==================== */
/**
 * @brief 使用默认参数发起连接
 */
void wireless_connect(void);

/**
 * @brief 发起到指定主机的连接
 *
 * @param host_idx 主机索引 (1-3 对应 BLE_INDEX_1 到 BLE_INDEX_3)
 * @param timeout 连接超时时间（0 表示使用默认值）
 */
void wireless_connect_ex(uint8_t host_idx, uint16_t timeout);

/**
 * @brief 断开当前连接
 */
void wireless_disconnect(void);

/**
 * @brief 进入配对模式
 */
void wireless_pairing(void);

/**
 * @brief 到指定主机进入配对模式
 *
 * @param host_idx 主机索引
 * @param param 配对参数（可为 NULL）
 */
void wireless_pairing_ex(uint8_t host_idx, void *param);

/* ==================== 状态回调函数（弱定义） ==================== */
/** @brief 进入复位状态回调 */
void wireless_enter_reset_kb(uint8_t reason);

/** @brief 进入可发现状态回调 */
void wireless_enter_discoverable_kb(uint8_t host_idx);

/** @brief 进入重连状态回调 */
void wireless_enter_reconnecting_kb(uint8_t host_idx);

/** @brief 进入已连接状态回调 */
void wireless_enter_connected_kb(uint8_t host_idx);

/** @brief 进入断开状态回调 */
void wireless_enter_disconnected_kb(uint8_t host_idx, uint8_t reason);

/** @brief 进入 PIN 码输入状态回调 */
void wireless_enter_bluetooth_pin_code_entry_kb(void);

/** @brief 退出 PIN 码输入状态回调 */
void wireless_exit_bluetooth_pin_code_entry_kb(void);

/** @brief 进入睡眠状态回调 */
void wireless_enter_sleep_kb(void);

/* ==================== 任务函数 ==================== */
/** @brief 无线主任务，应在主循环中调用 */
void wireless_task(void);

/** @brief 无线预处理任务 */
void wireless_pre_task(void);

/** @brief 无线后处理任务 */
void wireless_post_task(void);

/** @brief 字符串发送任务 */
void send_string_task(void);

/* ==================== 状态查询函数 ==================== */
/**
 * @brief 获取当前无线状态
 * @return 当前 wt_state_t 状态值
 */
wt_state_t wireless_get_state(void);

/* ==================== 状态同步 API（供驱动层调用） ==================== */
/**
 * @brief 设置状态为配对中
 * @param host_idx 主机索引
 */
void wireless_state_set_pairing(uint8_t host_idx);

/**
 * @brief 设置状态为重连中
 * @param host_idx 主机索引
 */
void wireless_state_set_reconnecting(uint8_t host_idx);

/**
 * @brief 设置状态为已连接
 * @param host_idx 主机索引
 */
void wireless_state_set_connected(uint8_t host_idx);

/**
 * @brief 设置状态为已断开
 * @param host_idx 主机索引
 * @param reason 断开原因
 */
void wireless_state_set_disconnected(uint8_t host_idx, uint8_t reason);

/**
 * @brief 设置状态为挂起
 */
void wireless_state_set_sleep(void);

/**
 * @brief 获取当前驱动函数表
 * @return 指向当前 wireless_transport 的指针
 */
wt_func_t* wireless_get_current_driver(void);

/* ==================== 电源管理 ==================== */
/**
 * @brief 低电量关机
 * @details 发送空报告后断开连接
 */
void wireless_low_battery_shutdown(void);

/* ==================== 按键处理 ==================== */
/**
 * @brief 处理无线相关按键事件
 * @param keycode 键码
 * @return true 继续处理，false 停止处理
 */
bool process_record_wireless(uint16_t keycode);

/* ==================== RAW HID 接口 ==================== */
/**
 * @brief 处理 RAW HID 接收数据
 * @param data 接收到的数据
 * @param length 数据长度
 */
void wireless_raw_hid_rx(uint8_t *data, uint8_t length);
