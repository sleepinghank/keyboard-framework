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

/**
 * @file _bt_driver.c
 * @brief 蓝牙驱动测试实现（用于本地模拟构建）
 *
 * 本文件提供 bt_driver.h 接口的模拟实现，仅用于测试和本地编译验证。
 * 所有方法只打印日志信息，不执行实际的蓝牙功能。
 */

#include "bt_driver.h"
#include <stdio.h>

/*********************************************************************
 * 本地函数声明
 *********************************************************************/

static void print_buffer(const char *prefix, const uint8_t *data, uint8_t len);

/*********************************************************************
 * 公共接口实现
 *********************************************************************/

/**
 * @brief 蓝牙驱动早期设置（测试模拟）
 */
void bt_driver_setup(void) {
    printf("[BT_TEST] bt_driver_setup() called\n");
    printf("[BT_TEST] Early stage setup completed (mock)\n");
}

/**
 * @brief 蓝牙驱动初始化（测试模拟）
 *
 * @param wakeup_from_low_power 是否从低功耗模式唤醒
 */
void bt_driver_init(bool wakeup_from_low_power) {
    printf("[BT_TEST] bt_driver_init() called\n");
    printf("[BT_TEST]   wakeup_from_low_power: %s\n",
           wakeup_from_low_power ? "true" : "false");
    printf("[BT_TEST] Bluetooth driver initialized (mock)\n");
}

/**
 * @brief 向已配对主机发起连接（测试模拟）
 *
 * @param host_idx 主机索引 (0-4)
 * @param timeout 超时值（秒）
 */
void bt_driver_connect_ex(uint8_t host_idx, uint16_t timeout) {
    printf("[BT_TEST] bt_driver_connect_ex() called\n");
    printf("[BT_TEST]   host_idx: %u\n", host_idx);
    printf("[BT_TEST]   timeout: %u seconds\n", timeout);
    printf("[BT_TEST] Attempting to connect to paired host (mock)\n");
}

/**
 * @brief 进入配对模式（测试模拟）
 *
 * @param host_idx 主机索引
 * @param param 配对参数
 */
void bt_driver_pairing_ex(uint8_t host_idx, void *param) {
    printf("[BT_TEST] bt_driver_pairing_ex() called\n");
    printf("[BT_TEST]   host_idx: %u\n", host_idx);
    printf("[BT_TEST]   param: %p\n", param);
    printf("[BT_TEST] Entering pairing mode (mock)\n");
}

/**
 * @brief 断开当前连接（测试模拟）
 */
void bt_driver_disconnect(void) {
    printf("[BT_TEST] bt_driver_disconnect() called\n");
    printf("[BT_TEST] Disconnecting from current host (mock)\n");
}

/**
 * @brief 发送标准键盘报告（测试模拟）
 *
 * @param report 键盘报告数据（20字节）
 * @return uint8_t 发送状态（0:成功）
 */
uint8_t bt_driver_send_keyboard(uint8_t *report) {
    printf("[BT_TEST] bt_driver_send_keyboard() called\n");
    if (report != NULL) {
        print_buffer("[BT_TEST]   Keyboard Report", report, 20);
    } else {
        printf("[BT_TEST]   report: NULL\n");
    }
    printf("[BT_TEST] Keyboard report sent (mock)\n");
    return 0; // 成功
}

/**
 * @brief 发送NKRO键盘报告（测试模拟）
 *
 * @param report NKRO报告数据（20字节）
 * @return uint8_t 发送状态（0:成功）
 */
uint8_t bt_driver_send_nkro(uint8_t *report) {
    printf("[BT_TEST] bt_driver_send_nkro() called\n");
    if (report != NULL) {
        print_buffer("[BT_TEST]   NKRO Report", report, 20);
    } else {
        printf("[BT_TEST]   report: NULL\n");
    }
    printf("[BT_TEST] NKRO report sent (mock)\n");
    return 0; // 成功
}

/**
 * @brief 发送消费者控制报告（测试模拟）
 *
 * @param report 消费者控制码（16位）
 * @return uint8_t 发送状态（0:成功）
 */
uint8_t bt_driver_send_consumer(uint16_t report) {
    printf("[BT_TEST] bt_driver_send_consumer() called\n");
    printf("[BT_TEST]   Consumer Code: 0x%04X\n", report);
    printf("[BT_TEST] Consumer report sent (mock)\n");
    return 0; // 成功
}

/**
 * @brief 发送系统控制报告（测试模拟）
 *
 * @param report 系统控制码（16位）
 * @return uint8_t 发送状态（0:成功）
 */
uint8_t bt_driver_send_system(uint16_t report) {
    printf("[BT_TEST] bt_driver_send_system() called\n");
    printf("[BT_TEST]   System Code: 0x%04X\n", report);
    printf("[BT_TEST] System report sent (mock)\n");
    return 0; // 成功
}

/**
 * @brief 发送鼠标报告（测试模拟）
 *
 * @param report 鼠标报告数据（20字节）
 * @return uint8_t 发送状态（0:成功）
 */
uint8_t bt_driver_send_mouse(uint8_t *report) {
    printf("[BT_TEST] bt_driver_send_mouse() called\n");
    if (report != NULL) {
        print_buffer("[BT_TEST]   Mouse Report", report, 20);
    } else {
        printf("[BT_TEST]   report: NULL\n");
    }
    printf("[BT_TEST] Mouse report sent (mock)\n");
    return 0; // 成功
}

/**
 * @brief 更新电池电量信息（测试模拟）
 *
 * @param bat_lvl 电池电量百分比（0-100）
 */
void bt_driver_update_bat_level(uint8_t bat_lvl) {
    printf("[BT_TEST] bt_driver_update_bat_level() called\n");
    printf("[BT_TEST]   Battery Level: %u%%\n", bat_lvl);
    printf("[BT_TEST] Battery level updated (mock)\n");
}

/*********************************************************************
 * 辅助函数实现
 *********************************************************************/

/**
 * @brief 打印缓冲区内容（用于调试）
 *
 * @param prefix 前缀字符串
 * @param data 数据缓冲区
 * @param len 数据长度
 */
static void print_buffer(const char *prefix, const uint8_t *data, uint8_t len) {
    if (data == NULL || len == 0) {
        printf("%s: (empty)\n", prefix);
        return;
    }

    printf("%s: [", prefix);
    for (uint8_t i = 0; i < len; i++) {
        printf("%02X", data[i]);
        if (i < len - 1) {
            printf(" ");
        }
    }
    printf("]\n");
}
