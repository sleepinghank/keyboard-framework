/**
 * @file _system_hal.c
 * @brief CH584 平台 system_hal 接口实现
 * @version 1.0
 *
 * 基于 WCH CH584 SDK 实现系统级 HAL 接口
 */

#include "system_hal.h"
#include "CH58x_common.h"
#include "_pin_defs.h"

/* 模块初始化标志 */
static bool g_system_initialized = false;


/**
 * @brief 系统HAL层初始化
 */
system_result_t system_hal_init(void)
{
    if (g_system_initialized) {
        return SYSTEM_ERROR_NOT_SUPPORTED;
    }

    //电源相关
#if(defined(DCDC_ENABLE)) && (DCDC_ENABLE == TRUE)
    PWR_DCDCCfg(ENABLE);
#endif
    //时钟相关
    HSECFG_Capacitance(HSECap_18p);//负载电容配置
    HSECFG_Current(HSE_RCur_100);//晶振的驱动电流
    SetSysClock( CLK_SOURCE_HSE_PLL_78MHz );//设置系统时钟源和频率
    SysTick_Config(0xFFFFFFFF);//系统滴答定时器，tmos所需基准时间片
    PFIC_DisableIRQ(SysTick_IRQn);//禁用系统滴答定时器中断
    mDelaymS(2);
    //GPIO配置 0输入1输出
    R32_PA_DIR &= 0;
    R32_PB_DIR &= 0;//io方向
    R32_PA_PU |= 0xFF7FFFFF;//去掉复位脚PB23配置
    R32_PB_PU |= 0xFFF7F3FF;//上拉使能，除了usb的引脚PB11,PB10,去掉PB19

    g_system_initialized = true;
    return SYSTEM_OK;
}

/**
 * @brief 初始化所有GPIO为默认安全状态
 * @note 将所有未使用的GPIO设置为输入上拉模式，防止悬空漏电
 *       保留引脚(USB、复位等)不会被初始化
 * @return SYSTEM_OK 成功，其他值表示错误
 */
system_result_t system_hal_gpio_init_all(void)
{
    /*
     * CH584 GPIO 配置说明:
     * - GPIOA: 24个引脚 (PA0-PA23)
     * - GPIOB: 24个引脚 (PB0-PB23)
     *
     * 保留引脚 (不进行初始化):
     * - PB10, PB11: USB D+/D-
     * - PB22, PB23: 复位引脚
     *
     * 默认配置: 输入模式 + 上拉使能
     * 这样可以防止GPIO悬空导致的漏电
     */

    /* 计算需要初始化的引脚掩码 (排除保留引脚) */
    uint32_t porta_init_mask = GPIO_ALL_PINS_MASK & (~GPIO_PORTA_RESERVED_MASK);
    uint32_t portb_init_mask = GPIO_ALL_PINS_MASK & (~GPIO_PORTB_RESERVED_MASK);

    /* GPIOA 配置: 全部设为输入模式 */
    R32_PA_DIR &= ~porta_init_mask;

    /* GPIOA 配置: 使能上拉 */
    R32_PA_PU |= porta_init_mask;

    /* GPIOB 配置: 全部设为输入模式 */
    R32_PB_DIR &= ~portb_init_mask;

    /* GPIOB 配置: 使能上拉 */
    R32_PB_PU |= portb_init_mask;

    return SYSTEM_OK;
}

/**
 * @brief 系统关机
 * @note 进入最低功耗 Shutdown 模式，RAM 内容丢失
 */
system_result_t system_hal_shutdown(void)
{
    /* 配置唤醒源为 GPIO */
    PWR_PeriphWakeUpCfg(ENABLE, RB_SLP_GPIO_WAKE, Long_Delay);

    /* 进入 Shutdown 模式，不保留 RAM */
    LowPower_Shutdown(0);

    /* 正常不会执行到这里，唤醒后会复位 */
    return SYSTEM_OK;
}

/**
 * @brief 进入睡眠模式
 * @param mode 目标睡眠模式
 * @param wakeup_sources 唤醒源掩码
 */
system_result_t system_hal_enter_sleep(system_power_mode_t mode, uint32_t wakeup_sources)
{
    /* 配置唤醒源 */
    uint8_t wake_ctrl = 0;

    if (wakeup_sources & SYSTEM_WAKEUP_GPIO) {
        wake_ctrl |= RB_SLP_GPIO_WAKE;
    }
    if (wakeup_sources & SYSTEM_WAKEUP_RTC) {
        wake_ctrl |= RB_SLP_RTC_WAKE;
    }
    if (wakeup_sources & SYSTEM_WAKEUP_USB) {
        wake_ctrl |= RB_SLP_USB_WAKE;
    }

    if (wake_ctrl) {
        PWR_PeriphWakeUpCfg(ENABLE, wake_ctrl, Short_Delay);
    }

    /* 根据模式选择不同的低功耗函数 */
    switch (mode) {
        case SYSTEM_POWER_MODE_IDLE:
            LowPower_Idle();
            break;

        case SYSTEM_POWER_MODE_SLEEP:
            /* Sleep 模式，保留 96K RAM 和扩展区域(BLE) */
            LowPower_Sleep(RB_PWR_RAM96K | RB_PWR_EXTEND);
            break;

        case SYSTEM_POWER_MODE_DEEP_SLEEP:
            /* 深度睡眠，只保留 32K RAM */
            LowPower_Sleep(RB_PWR_RAM32K);
            break;

        case SYSTEM_POWER_MODE_SHUTDOWN:
            /* Shutdown 模式，不保留 RAM */
            LowPower_Shutdown(0);
            break;

        default:
            return SYSTEM_ERROR_INVALID_PARAM;
    }

    return SYSTEM_OK;
}

/**
 * @brief 系统软件复位
 */
void system_hal_reset(void)
{
    SYS_ResetExecute();
    /* 不会执行到这里 */
}

/**
 * @brief 禁用全局中断
 * @return 返回当前中断状态，用于后续恢复
 */
uint32_t system_hal_disable_irq(void)
{
    uint32_t irq_status;
    SYS_DisableAllIrq(&irq_status);
    return irq_status;
}

/**
 * @brief 恢复全局中断状态
 * @param state 之前 system_hal_disable_irq 返回的状态值
 */
void system_hal_restore_irq(uint32_t state)
{
    SYS_RecoverIrq(state);
}
