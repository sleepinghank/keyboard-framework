/**
 * @file _watchdog.c
 * @brief CH584平台看门狗实现
 * @version 1.0
 *
 * CH584看门狗特点：
 * - 8位递增计数器，溢出时触发中断或复位
 * - 计数周期约为 256us（系统时钟相关）
 * - 最大超时时间约 65ms（256 * 256us）
 * - 需要通过多次喂狗实现更长的软件超时
 */

#include "watchdog.h"
#include "CH58x_common.h"

/* 看门狗配置参数 */
static uint32_t g_timeout_ms = 5000;    /* 软件超时时间(ms) */
static uint32_t g_feed_counter = 0;     /* 喂狗计数器 */
static uint32_t g_feed_threshold = 0;   /* 喂狗阈值 */
static bool g_watchdog_enabled = false; /* 看门狗使能标志 */

/* CH584看门狗硬件超时约65ms，使用0x00初值达到最大超时 */
#define WWDG_COUNT_VALUE    0x00
/* 假设每次硬件超时约65ms，计算软件超时需要的喂狗次数 */
#define WWDG_HW_TIMEOUT_MS  65

/**
 * @brief 初始化看门狗定时器
 * @param timeout_ms 超时时间（毫秒）
 */
void watchdog_init(uint32_t timeout_ms)
{
    g_timeout_ms = timeout_ms;
    g_feed_counter = 0;

    /* 计算软件超时需要的喂狗阈值 */
    /* 每次喂狗重置硬件计数器，实际超时 = 喂狗阈值 * 硬件超时 */
    g_feed_threshold = timeout_ms / WWDG_HW_TIMEOUT_MS;
    if (g_feed_threshold == 0) {
        g_feed_threshold = 1;
    }

    /* 配置看门狗：使能复位功能，禁用中断 */
    WWDG_ResetCfg(ENABLE);
    WWDG_ITCfg(DISABLE);

    /* 设置初始计数值 */
    WWDG_SetCounter(WWDG_COUNT_VALUE);
}

/**
 * @brief 启动看门狗定时器
 */
void watchdog_start(void)
{
    g_watchdog_enabled = true;
    g_feed_counter = 0;

    /* 重新加载计数值启动看门狗 */
    WWDG_SetCounter(WWDG_COUNT_VALUE);
}

/**
 * @brief 停止看门狗定时器
 * @note CH584看门狗启动后无法完全停止，只能通过禁用复位功能来"软停止"
 */
void watchdog_stop(void)
{
    g_watchdog_enabled = false;

    /* 禁用复位功能（看门狗仍在运行但不会导致复位） */
    WWDG_ResetCfg(DISABLE);
}

/**
 * @brief 喂狗（重置看门狗计数器）
 */
void watchdog_feed(void)
{
    if (!g_watchdog_enabled) {
        return;
    }

    /* 重新加载计数值 */
    WWDG_SetCounter(WWDG_COUNT_VALUE);

    /* 清除溢出标志 */
    if (WWDG_GetFlowFlag()) {
        WWDG_ClearFlag();
    }
}

/**
 * @brief 检查上次复位是否由看门狗引起
 * @return true 上次复位由看门狗超时引起
 */
bool watchdog_caused_reset(void)
{
    uint8_t reset_status = SYS_GetLastResetSta();

    /* RST_STATUS_WTR = 2, 看门狗超时复位 */
    /* RST_STATUS_LRM1 = 6, 唤醒复位-看门狗引起 */
    return (reset_status == RST_STATUS_WTR) || (reset_status == RST_STATUS_LRM1);
}
