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

/**
 * @file    wireless_driver_example.c
 * @brief   无线驱动使用示例
 * @details 展示如何在主程序中注册和使用蓝牙、2.4G驱动
 */

#include <stdio.h>
#include <stdbool.h>
#include "bt_driver.h"
#include "p24g_driver.h"
#include "wireless.h"
#include "transport.h"
#include "indicator.h"
#include "battery.h"
#include "communication/lpm.h"
#include "transport.h"
/**
 * @brief 无线驱动初始化示例
 * @param 无
 * @return 无
 *
 * 此函数演示如何初始化无线模块并注册驱动
 */
void wireless_driver_init_example(void) {
    printf("=== 无线驱动初始化示例 ===\n");

    // 1. 初始化无线核心模块
    printf("1. 初始化无线核心模块...\n");
    wireless_init();

    // 2. 注册蓝牙驱动
    printf("2. 注册蓝牙驱动...\n");
    wireless_register_bt_driver(&g_bt_driver);

    // 3. 注册2.4G驱动
    printf("3. 注册2.4G驱动...\n");
    wireless_register_p24g_driver(&g_p24g_driver);

    // 4. 初始化其他相关模块
    printf("4. 初始化指示灯模块...\n");
    indicator_init();

    printf("5. 初始化电池管理模块...\n");
    battery_init();

    printf("6. 初始化低功耗管理模块...\n");
    lpm_init();

    printf("无线驱动初始化完成!\n");
}

/**
 * @brief 传输模式切换示例
 * @param 无
 * @return 无
 *
 * 此函数演示如何在蓝牙、2.4G、USB模式之间切换
 */
void transport_mode_switch_example(void) {
    printf("\n=== 传输模式切换示例 ===\n");

    // 切换到蓝牙模式
    printf("1. 切换到蓝牙模式...\n");
    set_transport(TRANSPORT_BLUETOOTH);

    // 模拟一些操作
    printf("   当前模式: %d\n", get_transport());
    printf("   当前驱动: %p\n", wireless_get_current_driver());

    // 等待一段时间
    // wait_ms(2000);

    // 切换到2.4G模式
    printf("2. 切换到2.4G模式...\n");
    set_transport(TRANSPORT_P2P4);

    printf("   当前模式: %d\n", get_transport());
    printf("   当前驱动: %p\n", wireless_get_current_driver());

    // 等待一段时间
    // wait_ms(2000);

    // 切换到USB模式
    printf("3. 切换到USB模式...\n");
    set_transport(TRANSPORT_USB);

    printf("   当前模式: %d\n", get_transport());
    printf("   当前驱动: %p\n", wireless_get_current_driver());
}

/**
 * @brief 驱动任务循环示例
 * @param 无
 * @return 无
 *
 * 此函数演示如何在主循环中调用驱动任务
 */
void wireless_task_loop_example(void) {
    printf("\n=== 驱动任务循环示例 ===\n");

    while (1) {
        // 1. 矩阵扫描（假设有矩阵扫描函数）
        // matrix_scan();

        // 2. 无线任务处理
        wireless_task();

        // 3. 其他任务处理
        // process_keyboard_task();
        // update_indicators();

        // 4. 短暂延时
        // wait_ms(1);
        break; // 示例中只运行一次
    }
}

/**
 * @brief 蓝牙连接示例
 * @param 无
 * @return 无
 *
 * 此函数演示如何进行蓝牙连接和配对
 */
void bluetooth_connection_example(void) {
    printf("\n=== 蓝牙连接示例 ===\n");

    // 1. 切换到蓝牙模式
    set_transport(TRANSPORT_BLUETOOTH);

    // 2. 开始配对
    printf("1. 开始蓝牙配对...\n");
    wireless_pairing();

    // 3. 等待配对完成
    printf("2. 等待配对完成...\n");
    // 这里应该等待用户完成配对过程

    // 4. 连接到指定主机
    printf("3. 连接到主机 1...\n");
    wireless_connect_ex(1, 0); // 连接到主机1，超时0表示使用默认超时

    // 5. 发送测试按键
    printf("4. 发送测试按键...\n");
    // keyboard_report_t test_report = {0};
    // test_report.mods = MOD_SHIFT;
    // test_report.keys[0] = KC_H; // 发送 'h' 键
    // wireless_send_keyboard(&test_report);
}

/**
 * @brief 2.4G连接示例
 * @param 无
 * @return 无
 *
 * 此函数演示如何进行2.4G连接
 */
void p24g_connection_example(void) {
    printf("\n=== 2.4G连接示例 ===\n");

    // 1. 切换到2.4G模式
    set_transport(TRANSPORT_P2P4);

    // 2. 开始配对
    printf("1. 开始2.4G配对...\n");
    wireless_pairing();

    // 3. 等待配对完成
    printf("2. 等待配对完成...\n");

    // 4. 自动连接（2.4G通常是点对点连接）
    printf("3. 连接到2.4G设备...\n");
    wireless_connect_ex(P24G_INDEX, 0);

    // 5. 发送测试数据
    printf("4. 发送测试数据...\n");
    // keyboard_report_t test_report = {0};
    // test_report.keys[0] = KC_W; // 发送 'w' 键
    // wireless_send_keyboard(&test_report);
}

/**
 * @brief 错误处理示例
 * @param 无
 * @return 无
 *
 * 此函数演示如何处理无线连接错误
 */
void wireless_error_handling_example(void) {
    printf("\n=== 错误处理示例 ===\n");

    // 1. 检查当前状态
    wt_state_t state = wireless_get_state();
    printf("1. 当前无线状态: %d\n", state);

    // 2. 根据状态处理错误
    switch (state) {
        case WT_DISCONNECTED:
            printf("2. 无线已断开，尝试重新连接...\n");
            wireless_connect();
            break;

        case WT_CONNECTED:
            printf("2. 无线已连接\n");
            break;

        case WT_PARING:
            printf("2. 正在配对中...\n");
            break;

        case WT_RECONNECTING:
            printf("2. 正在重连...\n");
            break;

        default:
            printf("2. 未知状态: %d\n", state);
            break;
    }
}

/**
 * @brief 主函数示例
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 0表示成功
 *
 * 完整的无线驱动使用示例
 */
int main(int argc, char *argv[]) {
    printf("无线驱动使用示例程序启动\n");
    printf("================================\n");

    // 1. 初始化无线驱动
    wireless_driver_init_example();

    // 2. 演示传输模式切换
    transport_mode_switch_example();

    // 3. 演示蓝牙连接
    bluetooth_connection_example();

    // 4. 演示2.4G连接
    p24g_connection_example();

    // 5. 演示错误处理
    wireless_error_handling_example();

    // 6. 主任务循环
    wireless_task_loop_example();

    printf("\n================================\n");
    printf("无线驱动使用示例程序结束\n");

    return 0;
}

/**
 * @brief 编译说明
 *
 * 编译此示例需要以下文件：
 * - drivers/communication/bluetooth/bt_driver.c
 * - drivers/communication/p2p4g/p24g_driver.c
 * - middleware/communication/wireless.c
 * - middleware/communication/transport.c
 * - middleware/communication/report_buffer.c
 * - drivers/output/indicators/indicator.c
 * - drivers/power/battery.c
 * - drivers/system/lpm.c
 *
 * 编译命令示例：
 * gcc -o wireless_example wireless_driver_example.c \
 *     bt_driver.c p24g_driver.c wireless.c transport.c report_buffer.c \
 *     indicator.c battery.c lpm.c \
 *     -I../include -I../../drivers/include -I../../middleware/include
 */
