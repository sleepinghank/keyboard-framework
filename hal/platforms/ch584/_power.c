/**
 * @file _power.c
 * @brief CH584平台电源管理实现
 * @version 1.0
 *
 * 实现低功耗模式进入函数 enter_power_mode()
 *
 * CH584低功耗模式说明:
 * - Idle: CPU暂停，外设继续运行
 * - Halt: 切换到HSI/5时钟运行
 * - Sleep: 低功耗睡眠，可配置保留RAM
 * - Shutdown: 最低功耗，可配置保留RAM
 */

#include "lpm.h"
#include "CH58x_common.h"

/**
 * @brief 进入指定的低功耗模式
 * @param mode 电源模式
 */
void enter_power_mode(pm_t mode)
{
    switch (mode) {
        case PM_RUN:
            /* 正常运行模式，无需处理 */
            break;

        case PM_LOW_POWER_RUN:
            /* 低功耗运行模式 */
            /* 可选：启用DCDC节省功耗 */
            PWR_DCDCCfg(ENABLE);
            break;

        case PM_SLEEP:
            /* 轻度睡眠模式 - 使用Idle */
            LowPower_Idle();
            break;

        case PM_LOW_POWER_SLEEP:
            /* 低功耗睡眠模式 - 使用Halt */
            LowPower_Halt();
            /* 唤醒后需要重新配置系统时钟 */
            // SetSysClock(CLK_SOURCE_PLL_80MHz);
            break;

        case PM_STOP0:
        case PM_STOP1:
        case PM_STOP2:
            /* Stop模式 - 使用Sleep，保留所有RAM */
            /* 配置GPIO唤醒源 */
            PWR_PeriphWakeUpCfg(ENABLE, RB_SLP_GPIO_WAKE, Long_Delay);
            /* 保留16K主SRAM + 2K retention SRAM */
            // LowPower_Sleep(RB_PWR_RAM16K | RB_PWR_RAM2K);
            // /* 唤醒后重新配置系统时钟 */
            // SetSysClock(CLK_SOURCE_PLL_80MHz);
            break;

        case PM_STANDBY_WITH_RAM:
            /* 带RAM保持的待机模式 - 使用Shutdown保留部分RAM */
            PWR_PeriphWakeUpCfg(ENABLE, RB_SLP_GPIO_WAKE, Long_Delay);
            /* 仅保留2K retention SRAM */
            // LowPower_Shutdown(RB_PWR_RAM2K);
            // /* 唤醒后重新配置系统时钟 */
            // SetSysClock(CLK_SOURCE_PLL_80MHz);
            break;

        case PM_STANDBY:
            /* 待机模式 - 使用Shutdown，不保留RAM */
            PWR_PeriphWakeUpCfg(ENABLE, RB_SLP_GPIO_WAKE, Long_Delay);
            LowPower_Shutdown(0);
            /* 唤醒后系统复位，不会执行到这里 */
            break;

        case PM_SHUTDOWN:
            /* 关机模式 - 执行软件复位或完全断电 */
            SYS_ResetExecute();
            /* 不会执行到这里 */
            break;

        default:
            break;
    }
}

/**
 * @brief 检查USB电源是否连接
 * @return true USB电源已连接
 * @note 需要根据实际硬件配置USB电源检测引脚
 */
bool usb_power_connected(void)
{
#ifdef USB_POWER_SENSE_PIN
    return gpio_read_pin(USB_POWER_SENSE_PIN) == USB_POWER_CONNECTED_LEVEL;
#else
    /* 默认返回true，认为有USB供电 */
    return true;
#endif
}
