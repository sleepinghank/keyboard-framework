/* Copyright 2023 Keychron
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY of any kind, either express or implied.
 * See the GNU General Public License for the more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

// 2.4G驱动实例 - 适配wt_func_t接口
typedef struct {
    // 生命周期管理
    void (*init)(bool wakeup_from_low_power);     // 初始化2.4G模块
    void (*task)(void);                           // 驱动任务循环

    // 连接管理
    void (*connect_ex)(uint8_t host_idx, uint16_t timeout);     // 连接指定主机（通常忽略host_idx）
    void (*pairing_ex)(uint8_t host_idx, void *param);          // 配对指定主机
    void (*disconnect)(void);                                     // 断开连接

    // 数据传输
    void (*send_keyboard)(uint8_t *report);       // 发送键盘报告
    void (*send_nkro)(uint8_t *report);           // 发送NKRO报告
    void (*send_consumer)(uint16_t report);       // 发送媒体键
    void (*send_system)(uint16_t report);         // 发送系统键
    void (*send_mouse)(uint8_t *report);          // 发送鼠标报告

    // 电池管理
    void (*update_bat_level)(uint8_t bat_lvl);    // 更新电池电量
    void (*update_bat_state)(uint8_t bat_state);  // 更新电池状态

    // 模块管理
    void (*send_protocol_ver)(uint16_t ver);      // 发送协议版本
    void (*send_cmd)(uint8_t* payload, uint8_t len, bool ack_enable, bool retry); // 发送命令
    void (*become_discoverable)(uint8_t host_idx, void* param); // 进入配对模式
    void (*read_state_reg)(uint8_t reg, uint8_t len);           // 读取状态寄存器
} p24g_driver_t;

// 2.4G模块信息结构
typedef struct {
    uint8_t  model_name[11];
    uint8_t  mode;
    uint8_t  version;
    uint8_t  firmware_version[11];
    uint8_t  hardware_version[11];
    uint16_t cmd_set_version;
} p24g_module_info_t;

// 2.4G模块参数结构
typedef struct {
    uint8_t  event_mode;
    uint16_t connected_idle_timeout;
    uint16_t pairing_timeout;
    uint8_t  pairing_mode;
    uint16_t reconnect_timeout;
    uint8_t  report_rate;
    uint8_t  vendor_id_source;
    uint16_t vendor_id;
    uint16_t product_id;
} p24g_module_param_t;

// 配对参数结构
typedef struct {
    uint8_t     hostIndex;
    uint16_t    timeout;
    uint8_t     pairingMode;
    uint8_t     reserved1;
    uint8_t     txPower;
    const char* name;
} p24g_pairing_param_t;

// 外部接口声明
extern p24g_driver_t g_p24g_driver;

void p24g_driver_init(void);
void p24g_driver_task(void);

// 2.4G模块操作函数
void p24g_driver_get_info(p24g_module_info_t* info);
void p24g_driver_set_param(p24g_module_param_t* param);
void p24g_driver_get_param(p24g_module_param_t* param);
void p24g_driver_set_local_name(const char* name);
void p24g_driver_get_local_name(void);
void p24g_driver_factory_reset(uint8_t clr_msk);
void p24g_driver_int_pin_test(bool enable);
void p24g_driver_dfu_rx(uint8_t* data, uint8_t length);
void p24g_driver_radio_test(uint8_t channel);
void p24g_driver_write_customize_data(uint8_t* data, uint8_t len);
bool p24g_driver_read_customize_data(uint8_t* data, uint8_t len);

// 2.4G特定功能
bool p24g_driver_get_pairing_status(void);
void p24g_driver_clear_pairing(void);
uint8_t p24g_driver_get_rssi(void);