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

#include "p24g_driver.h"
#include "wireless.h"
#include "wireless_event_type.h"
#include "spi_master.h"
#include "gpio.h"
#include "timer.h"
#include "wait.h"
#include <string.h>

/* 2.4G模块引脚定义 - 需要在product_config.h中定义 */
#ifndef P24G_CS_PIN
#define P24G_CS_PIN NO_PIN
#endif

#ifndef P24G_INT_PIN
#define P24G_INT_PIN NO_PIN
#endif

#ifndef P24G_RST_PIN
#define P24G_RST_PIN NO_PIN
#endif

/* 2.4G通信命令定义 */
#define P24G_CMD_NOP            0x00
#define P24G_CMD_READ_REG       0x01
#define P24G_CMD_WRITE_REG      0x02
#define P24G_CMD_SEND_KEYBOARD  0x10
#define P24G_CMD_SEND_NKRO      0x11
#define P24G_CMD_SEND_CONSUMER  0x12
#define P24G_CMD_SEND_SYSTEM    0x13
#define P24G_CMD_SEND_MOUSE     0x14
#define P24G_CMD_CONNECT        0x20
#define P24G_CMD_DISCONNECT     0x21
#define P24G_CMD_PAIRING        0x22
#define P24G_CMD_UPDATE_BAT     0x30

/* 2.4G模块状态 */
typedef enum {
    P24G_STATE_IDLE = 0,
    P24G_STATE_INITIALIZED,
    P24G_STATE_CONNECTING,
    P24G_STATE_CONNECTED,
    P24G_STATE_PAIRING,
    P24G_STATE_ERROR
} p24g_state_t;

static p24g_state_t g_p24g_state = P24G_STATE_IDLE;
static bool g_p24g_initialized = false;

/* SPI传输辅助函数 */
static uint8_t p24g_spi_transfer(uint8_t data) {
    spi_write(data);
    return (uint8_t)spi_read();
}

static void p24g_send_cmd(uint8_t cmd, const uint8_t* data, uint8_t len) {
    if (P24G_CS_PIN == NO_PIN) return;

    spi_start(P24G_CS_PIN, false, 0, 8);
    p24g_spi_transfer(cmd);
    for (uint8_t i = 0; i < len; i++) {
        p24g_spi_transfer(data[i]);
    }
    spi_stop();
}

// 2.4G驱动实现 - 生命周期管理
void p24g_driver_init_impl(bool wakeup_from_low_power) {
    if (g_p24g_initialized && !wakeup_from_low_power) {
        return;
    }

    // 1. 初始化SPI
    spi_init();

    // 2. 配置控制引脚
    if (P24G_RST_PIN != NO_PIN) {
        gpio_set_pin_output_push_pull(P24G_RST_PIN);
        // 复位模块
        gpio_write_pin_low(P24G_RST_PIN);
        wait_ms(10);
        gpio_write_pin_high(P24G_RST_PIN);
        wait_ms(50);
    }

    if (P24G_INT_PIN != NO_PIN) {
        gpio_set_pin_input_high(P24G_INT_PIN);
    }

    // 3. 发送初始化命令
    uint8_t init_data[] = {0x01};  // 初始化命令数据
    p24g_send_cmd(P24G_CMD_NOP, init_data, sizeof(init_data));

    g_p24g_state = P24G_STATE_INITIALIZED;
    g_p24g_initialized = true;
}

void p24g_driver_task_impl(void) {
    if (!g_p24g_initialized) return;

    // 检查中断引脚，判断是否有数据需要处理
    if (P24G_INT_PIN != NO_PIN && gpio_read_pin(P24G_INT_PIN) == 0) {
        // 有中断信号，读取数据
        // 具体处理依赖2.4G芯片的协议
    }
}

// 2.4G驱动实现 - 连接管理
void p24g_driver_connect_ex_impl(uint8_t host_idx, uint16_t timeout) {
    if (!g_p24g_initialized) return;

    // 2.4G点对点连接
    uint8_t connect_data[3] = {host_idx, (uint8_t)(timeout & 0xFF), (uint8_t)(timeout >> 8)};
    p24g_send_cmd(P24G_CMD_CONNECT, connect_data, sizeof(connect_data));
    g_p24g_state = P24G_STATE_CONNECTING;
}

void p24g_driver_pairing_ex_impl(uint8_t host_idx, void *param) {
    if (!g_p24g_initialized) return;

    // 进入配对模式
    uint8_t pairing_data[2] = {host_idx, 0x01};
    if (param != NULL) {
        p24g_pairing_param_t* p = (p24g_pairing_param_t*)param;
        pairing_data[1] = p->pairingMode;
    }
    p24g_send_cmd(P24G_CMD_PAIRING, pairing_data, sizeof(pairing_data));
    g_p24g_state = P24G_STATE_PAIRING;
}

void p24g_driver_disconnect_impl(void) {
    if (!g_p24g_initialized) return;

    p24g_send_cmd(P24G_CMD_DISCONNECT, NULL, 0);
    g_p24g_state = P24G_STATE_INITIALIZED;
}

// 2.4G驱动实现 - 数据传输
void p24g_driver_send_keyboard_impl(uint8_t *report) {
    if (!g_p24g_initialized || g_p24g_state != P24G_STATE_CONNECTED) return;
    if (report == NULL) return;

    // 发送标准键盘报告 (8 bytes)
    p24g_send_cmd(P24G_CMD_SEND_KEYBOARD, report, 8);
}

void p24g_driver_send_nkro_impl(uint8_t *report) {
    if (!g_p24g_initialized || g_p24g_state != P24G_STATE_CONNECTED) return;
    if (report == NULL) return;

    // 发送NKRO报告 (通常16-32 bytes)
    p24g_send_cmd(P24G_CMD_SEND_NKRO, report, 16);
}

void p24g_driver_send_consumer_impl(uint16_t report) {
    if (!g_p24g_initialized || g_p24g_state != P24G_STATE_CONNECTED) return;

    uint8_t data[2] = {(uint8_t)(report & 0xFF), (uint8_t)(report >> 8)};
    p24g_send_cmd(P24G_CMD_SEND_CONSUMER, data, sizeof(data));
}

void p24g_driver_send_system_impl(uint16_t report) {
    if (!g_p24g_initialized || g_p24g_state != P24G_STATE_CONNECTED) return;

    uint8_t data[2] = {(uint8_t)(report & 0xFF), (uint8_t)(report >> 8)};
    p24g_send_cmd(P24G_CMD_SEND_SYSTEM, data, sizeof(data));
}

void p24g_driver_send_mouse_impl(uint8_t *report) {
    if (!g_p24g_initialized || g_p24g_state != P24G_STATE_CONNECTED) return;
    if (report == NULL) return;

    // 发送鼠标报告 (通常5-8 bytes)
    p24g_send_cmd(P24G_CMD_SEND_MOUSE, report, 5);
}

// 2.4G驱动实现 - 电池管理
void p24g_driver_update_bat_level_impl(uint8_t bat_lvl) {
    if (!g_p24g_initialized) return;

    uint8_t data[2] = {0x01, bat_lvl};  // 0x01=电量类型, bat_lvl=电量值
    p24g_send_cmd(P24G_CMD_UPDATE_BAT, data, sizeof(data));
}

void p24g_driver_update_bat_state_impl(uint8_t bat_state) {
    if (!g_p24g_initialized) return;

    uint8_t data[2] = {0x02, bat_state};  // 0x02=状态类型, bat_state=状态值
    p24g_send_cmd(P24G_CMD_UPDATE_BAT, data, sizeof(data));
}

// 2.4G驱动实现 - 模块管理
void p24g_driver_send_protocol_ver_impl(uint16_t ver) {
    // TODO: 实现协议版本发送
}

void p24g_driver_send_cmd_impl(uint8_t* payload, uint8_t len, bool ack_enable, bool retry) {
    // TODO: 实现命令发送
}

void p24g_driver_become_discoverable_impl(uint8_t host_idx, void* param) {
    // TODO: 实现进入可发现模式
}

void p24g_driver_read_state_reg_impl(uint8_t reg, uint8_t len) {
    // TODO: 实现读取状态寄存器
}

// 2.4G模块操作函数实现
void p24g_driver_get_info(p24g_module_info_t* info) {
    // TODO: 获取模块信息
}

void p24g_driver_set_param(p24g_module_param_t* param) {
    // TODO: 设置模块参数
}

void p24g_driver_get_param(p24g_module_param_t* param) {
    // TODO: 获取模块参数
}

void p24g_driver_set_local_name(const char* name) {
    // TODO: 设置本地名称
}

void p24g_driver_get_local_name(void) {
    // TODO: 获取本地名称
}

void p24g_driver_factory_reset(uint8_t clr_msk) {
    // TODO: 恢复出厂设置
}

void p24g_driver_int_pin_test(bool enable) {
    // TODO: 中断引脚测试
}

void p24g_driver_dfu_rx(uint8_t* data, uint8_t length) {
    // TODO: DFU接收
}

void p24g_driver_radio_test(uint8_t channel) {
    // TODO: 射频测试
}

void p24g_driver_write_customize_data(uint8_t* data, uint8_t len) {
    // TODO: 写入自定义数据
}

bool p24g_driver_read_customize_data(uint8_t* data, uint8_t len) {
    // TODO: 读取自定义数据
    return false;
}

// 2.4G特定功能实现
bool p24g_driver_get_pairing_status(void) {
    // TODO: 获取配对状态
    return false;
}

void p24g_driver_clear_pairing(void) {
    // TODO: 清除配对信息
}

uint8_t p24g_driver_get_rssi(void) {
    // TODO: 获取信号强度
    return 0;
}

// 2.4G驱动实例 - 适配wt_func_t接口
p24g_driver_t g_p24g_driver = {
    .init = p24g_driver_init_impl,
    .task = p24g_driver_task_impl,
    .connect_ex = p24g_driver_connect_ex_impl,
    .pairing_ex = p24g_driver_pairing_ex_impl,
    .disconnect = p24g_driver_disconnect_impl,
    .send_keyboard = p24g_driver_send_keyboard_impl,
    .send_nkro = p24g_driver_send_nkro_impl,
    .send_consumer = p24g_driver_send_consumer_impl,
    .send_system = p24g_driver_send_system_impl,
    .send_mouse = p24g_driver_send_mouse_impl,
    .update_bat_level = p24g_driver_update_bat_level_impl,
    .update_bat_state = p24g_driver_update_bat_state_impl,
    .send_protocol_ver = p24g_driver_send_protocol_ver_impl,
    .send_cmd = p24g_driver_send_cmd_impl,
    .become_discoverable = p24g_driver_become_discoverable_impl,
    .read_state_reg = p24g_driver_read_state_reg_impl,
};