/* Copyright 2025 @ keyboard-framework
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
 * @file demo_indicator.c
 * @brief 指示灯使用示例
 *
 * 演示如何使用指示灯驱动
 */

#include "indicator.h"
#include <stdio.h>

/* ========== 模拟GPIO引脚定义（测试平台） ========== */

// 测试平台使用虚拟引脚
#define PIN_TEST_LED1   1
#define PIN_TEST_LED2   2
#define PIN_TEST_LED3   3

/* ========== LED实例定义 ========== */

// 蓝牙LED
static indicator_t bt_led;

// 2.4G LED
static indicator_t p24g_led;

// 电池LED
static indicator_t battery_led;

/* ========== 初始化函数 ========== */

/**
 * @brief 初始化所有LED
 */
void demo_indicator_setup(void) {
    // 初始化蓝牙LED: PIN_TEST_LED1, 高电平亮
    indicator_init(&bt_led, PIN_TEST_LED1, true);

    // 初始化2.4G LED: PIN_TEST_LED2, 高电平亮
    indicator_init(&p24g_led, PIN_TEST_LED2, true);

    // 初始化电池LED: PIN_TEST_LED3, 低电平亮
    indicator_init(&battery_led, PIN_TEST_LED3, false);

    printf("Indicator Demo: All LEDs initialized\n");
}

/* ========== 示例1：基础使用 ========== */

/**
 * @brief 示例1：基础LED控制
 */
void demo_basic_usage(void) {
    printf("\n=== Demo 1: Basic Usage ===\n");

    // 点亮2秒
    ind_config_t cfg = IND_ON_2S;
    indicator_start(&bt_led, &cfg);
    printf("BT LED: ON for 2 seconds\n");

    // 模拟运行2秒
    for (int i = 0; i < 20; i++) {
        indicator_task(&bt_led);
        // delay 100ms
    }
}

/* ========== 示例2：闪烁效果 ========== */

/**
 * @brief 示例2：各种闪烁效果
 */
void demo_blink_effects(void) {
    printf("\n=== Demo 2: Blink Effects ===\n");

    // 慢闪（配对模式）
    ind_config_t slow_blink = IND_BLINK_SLOW;
    indicator_start(&bt_led, &slow_blink);
    printf("BT LED: Slow blink (pairing mode)\n");

    // 快闪（重连模式）
    ind_config_t fast_blink = IND_BLINK_FAST;
    indicator_start(&p24g_led, &fast_blink);
    printf("2.4G LED: Fast blink (reconnecting mode)\n");
}

/* ========== 示例3：使用配置宏 ========== */

/**
 * @brief 示例3：使用预定义配置宏
 */
void demo_preset_macros(void) {
    printf("\n=== Demo 3: Preset Macros ===\n");

    // 使用预定义宏
    indicator_start(&bt_led, &(ind_config_t)IND_HEARTBEAT);
    printf("BT LED: Heartbeat effect (using preset macro)\n");

    indicator_start(&p24g_led, &(ind_config_t)IND_BLINK_3_TIMES);
    printf("2.4G LED: Blink 3 times (using preset macro)\n");
}

/* ========== 示例4：自定义配置 ========== */

/**
 * @brief 示例4：自定义配置参数
 */
void demo_custom_config(void) {
    printf("\n=== Demo 4: Custom Config ===\n");

    // 自定义配置：亮200ms，灭800ms，循环5次
    ind_config_t custom_cfg = {
        .mode     = IND_MODE_BLINK,
        .on_time  = 200,
        .off_time = 800,
        .duration = 0,     // 无限
        .repeat   = 5,     // 5次后停止
    };
    indicator_start(&bt_led, &custom_cfg);
    printf("BT LED: Custom blink (200ms on, 800ms off, 5 times)\n");
}

/* ========== 示例5：业务场景模拟 ========== */

/**
 * @brief 模拟无线状态变化
 */
void demo_wireless_state_change(int state) {
    ind_config_t cfg;

    switch (state) {
        case 0: // 已连接
            printf("Wireless state: CONNECTED\n");
            cfg = IND_ON_2S;
            indicator_start(&bt_led, &cfg);
            break;

        case 1: // 已断开
            printf("Wireless state: DISCONNECTED\n");
            cfg = IND_OFF;
            indicator_start(&bt_led, &cfg);
            break;

        case 2: // 配对中
            printf("Wireless state: PAIRING\n");
            cfg.mode     = IND_MODE_BLINK;
            cfg.on_time  = 500;
            cfg.off_time = 500;
            cfg.duration = 0;
            cfg.repeat   = 0;
            indicator_start(&bt_led, &cfg);
            break;

        case 3: // 重连中
            printf("Wireless state: RECONNECTING\n");
            cfg.mode     = IND_MODE_BLINK;
            cfg.on_time  = 100;
            cfg.off_time = 100;
            cfg.duration = 0;
            cfg.repeat   = 0;
            indicator_start(&bt_led, &cfg);
            break;
    }
}

/**
 * @brief 模拟电量状态变化
 */
void demo_battery_state_change(int percentage) {
    ind_config_t cfg;

    if (percentage < 10) {
        printf("Battery: CRITICAL LOW (%d%%)\n", percentage);
        // 严重低电：心跳效果
        cfg = IND_HEARTBEAT;
        indicator_start(&battery_led, &cfg);
    } else if (percentage < 20) {
        printf("Battery: LOW (%d%%)\n", percentage);
        // 低电：慢闪
        cfg.mode     = IND_MODE_BLINK;
        cfg.on_time  = 500;
        cfg.off_time = 500;
        cfg.duration = 0;
        cfg.repeat   = 0;
        indicator_start(&battery_led, &cfg);
    } else {
        printf("Battery: NORMAL (%d%%)\n", percentage);
        // 正常：熄灭
        cfg = IND_OFF;
        indicator_start(&battery_led, &cfg);
    }
}

void demo_business_scenario(void) {
    printf("\n=== Demo 5: Business Scenario ===\n");

    // 模拟连接过程
    demo_wireless_state_change(2);  // 配对中
    // 模拟运行一段时间...

    demo_wireless_state_change(3);  // 重连中
    // 模拟运行一段时间...

    demo_wireless_state_change(0);  // 已连接
    // 模拟运行2秒...

    demo_wireless_state_change(1);  // 断开

    // 模拟电量状态
    demo_battery_state_change(50);  // 正常
    demo_battery_state_change(15);  // 低电
    demo_battery_state_change(5);   // 严重低电
}

/* ========== 示例6：状态查询 ========== */

/**
 * @brief 示例6：查询LED状态
 */
void demo_state_query(void) {
    printf("\n=== Demo 6: State Query ===\n");

    indicator_start(&bt_led, &(ind_config_t)IND_ON_2S);

    if (indicator_is_running(&bt_led)) {
        printf("BT LED is running\n");
    }

    indicator_stop(&bt_led);

    if (!indicator_is_running(&bt_led)) {
        printf("BT LED is stopped\n");
    }
}

/* ========== 示例7：更新配置 ========== */

/**
 * @brief 示例7：运行时更新配置
 */
void demo_update_config(void) {
    printf("\n=== Demo 7: Update Config ===\n");

    // 先启动慢闪
    ind_config_t cfg = IND_BLINK_SLOW;
    indicator_start(&bt_led, &cfg);
    printf("BT LED: Slow blink started\n");

    // 运行一段时间后更新为快闪
    // indicator_update(&bt_led, &(ind_config_t)IND_BLINK_FAST);
    // printf("BT LED: Updated to fast blink\n");
}

/* ========== 主函数 ========== */

/**
 * @brief 主函数
 */
int main(void) {
    printf("========================================\n");
    printf("  Indicator Driver Demo\n");
    printf("========================================\n");

    // 初始化
    demo_indicator_setup();

    // 运行各个示例
    demo_basic_usage();
    demo_blink_effects();
    demo_preset_macros();
    demo_custom_config();
    demo_business_scenario();
    demo_state_query();
    demo_update_config();

    printf("\n========================================\n");
    printf("  Demo Complete\n");
    printf("========================================\n");

    return 0;
}

/* ========== 任务函数示例 ========== */

/**
 * @brief 主循环中的任务函数
 *
 * 实际使用时，在主循环中调用此函数
 */
void app_main_loop(void) {
    // 处理所有LED任务
    indicator_task(&bt_led);
    indicator_task(&p24g_led);
    indicator_task(&battery_led);

    // ... 其他任务
    // matrix_scan();
    // battery_task();
    // wireless_task();
}

/**
 * @brief 模拟主循环
 *
 * 演示如何在主循环中使用指示灯
 */
void demo_main_loop(void) {
    // 主循环
    while (1) {
        app_main_loop();

        // 延时（或等待中断）
        // wait_ms(10);
    }
}
