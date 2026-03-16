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

#include <stdio.h>
#include "system_init.h"
#include "sys_config.h"
#include "event_manager.h"
#include "lpm.h"
/**
 * @brief 主函数 - 系统启动入口
 * @return int (通常不会返回)
 */
int main(void) {
    // 打印启动信息
#if (PRINTF_ENABLE == TRUE && PRINTF_LEVEL >= PRINTF_LEVEL_INFO)
    printf("\r\n");
    printf("=======================================\r\n");
    printf("   Keyboard Framework v1.0.0\r\n");
    printf("   Chip Type: ");
#if (CHIP_TYPE == CHIP_CH584M)
    printf("CH584M\r\n");
#elif (CHIP_TYPE == CHIP_PAR2860)
    printf("PAR2860\r\n");
#else
    printf("Unknown\r\n");
#endif
    printf("   Build Date: 2025-12-16\r\n");
    printf("=======================================\r\n");
#endif

    // 调用系统初始化协调器
    uint32_t init_result = system_init_coordinator();
    if (init_result != 0) {
#if (PRINTF_ENABLE == TRUE && PRINTF_LEVEL >= PRINTF_LEVEL_ERROR)
        printf("ERROR: System initialization failed! Error code: %d\r\n", init_result);
#endif
        return init_result;
    }

#if (PRINTF_ENABLE == TRUE && PRINTF_LEVEL >= PRINTF_LEVEL_INFO)
    printf("System initialization completed successfully!\r\n");
    printf("Entering main loop...\r\n");
#endif

    // 主循环 - 持续调用各层级的生命周期函数
    while (1) {
        /* LPM 状态机推进：检查空闲超时并投递睡眠请求事件 */
        lpm_task();

        OSAL_SystemProcess();
    }

    // 理论上不会执行到这里
    return 0;
}