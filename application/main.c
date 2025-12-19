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
#include "CH585SFR.h"
#include "timer.h"

/**
 * @brief 主函数 - 系统启动入口
 * @return int (通常不会返回)
 */
int main(void) {
    PRINT("starting main.c...\r\n");
    return 0;
}