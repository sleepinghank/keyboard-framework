/**
 * @file main.c
 * @brief 键盘框架主入口
 * @version 1.0.0
 * @date 2025-12-16
 *
 * 设计说明:
 * - 启动系统初始化协调器
 * - 进入主循环，按层级调用各模块生命周期函数
 * - 遵循_setup → _init → _pre_task → _task → _post_task的模式
 */

#include "system_hal.h"
#include "event_manager.h"
#include "system_init.h"
#include "sys_config.h"
#include "debug.h"
#include "input_service.h"
#include "keyboard.h"
#include "gpio.h"

/**
 * @brief 主函数 - 系统启动入口
 * @return int (通常不会返回)
 */
int main(void)
{
    // 调用系统初始化协调器
    uint32_t init_result = system_init_coordinator();
   if (init_result != 0) {
#if (PRINTF_ENABLE == TRUE && PRINTF_LEVEL >= PRINTF_LEVEL_ERROR)
        dprintf("ERROR: System initialization failed! Error code: %d\r\n", init_result);
#endif
        return init_result;
    }
    dprint("System initialized successfully");

    // 主循环：矩阵扫描 + OSAL 事件处理
    while (1) {
        if (input_get_matrix_scan_flag()) {
            togglePin(B14);  /* 调试: 验证 TMR1 中断触发 */
            keyboard_task();
            input_clear_matrix_scan_flag();
        }

        /* OSAL 单次处理（BLE 协议栈等） */
        OSAL_SystemProcessOnce();
    }

    return 0;
}