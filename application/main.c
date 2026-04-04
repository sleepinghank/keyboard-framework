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

#include "event_manager.h"
#include "system_init.h"
#include "kb904/config_product.h"
#include "debug.h"

/**计划
 * 1.回连系统识别策略，当前回连未进行系统识别键芯错误，方案1首次连接记忆系统，保存当前键芯（需要flash）。方案2回连也执行系统识别（可能影响回连速度）
 */

/**
 * @brief 主函数 - 系统启动入口
 * @return int (通常不会返回)
 */
int main(void)
{
    // 调用系统初始化协调器
    uint32_t init_result = system_init_coordinator();
   if (init_result != 0) {
#if (PRINTF_ENABLE == TRUE )
        dprintf("ERROR: System initialization failed! Error code: %d\r\n", init_result);
#endif
        return init_result;
    }
    dprint("System initialized successfully\r\n");

    // 主循环：矩阵扫描 + OSAL 事件处理
    while (1) {
        /* OSAL 单次处理（BLE 协议栈等） */
        OSAL_SystemProcessOnce();
         // 外设处理（矩阵扫描、指示灯等） - 频率较高，保持响应性
        peripheral_process();
    }

    return 0;
}
