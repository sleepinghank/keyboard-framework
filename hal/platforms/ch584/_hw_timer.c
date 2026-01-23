/**
 * @file _hw_timer.c
 * @brief CH584 平台硬件定时器 HAL 实现 (简化版)
 * @version 2.0.0
 *
 * 实现说明:
 * - 映射: HW_TIMER_0 → TMR0, HW_TIMER_1 → TMR1, HW_TIMER_2 → TMR2
 * - CH584 系统时钟 78MHz，最大定时周期 67108864 ticks (~860ms)
 * - 仅支持周期模式，长时间定时请使用 OSAL 软件定时器
 */

#include "hw_timer.h"
#include "CH58x_common.h"
#include "debug.h"
#include "gpio.h"
/*==========================================
 * 常量定义
 *=========================================*/

// 系统时钟频率 78MHz
#define FREQ_SYS_HZ         78000000UL

// 毫秒转时钟周期
#define MS_TO_TICKS(ms)     ((uint32_t)(ms) * (FREQ_SYS_HZ / 1000))

// 最大硬件定时周期 (ticks)
#define MAX_HW_TIMER_TICKS  67108864UL

// 最大可直接使用硬件定时的毫秒数
#define MAX_DIRECT_MS       (MAX_HW_TIMER_TICKS / (FREQ_SYS_HZ / 1000))

/*==========================================
 * 定时器状态管理
 *=========================================*/

// 定时器上下文结构 (简化版)
typedef struct {
    hw_timer_state_t    state;          // IDLE / RUNNING
    uint32_t            interval_ms;    // 定时间隔 (毫秒)
    uint32_t            hw_ticks;       // 硬件定时周期 (ticks)
    hw_timer_callback_t callback;       // 回调函数
} hw_timer_context_t;

static hw_timer_context_t g_timer_ctx[HW_TIMER_COUNT];
static bool g_hw_timer_initialized = false;

/*==========================================
 * 内部辅助函数
 *=========================================*/

/**
 * @brief 配置并启动指定定时器硬件
 */
static void timer_hw_start(hw_timer_id_t id, uint32_t ticks)
{
    /* 初始化顺序参考官方例程 kb04122:
     * ITCfg → EnableIRQ → SetPriority → TimerInit
     * 注意: TimerInit 内部已经启用定时器，无需再调用 Enable()
     */
    switch (id) {
        case HW_TIMER_0:
            dprintf("HW Timer: Starting TMR0 with %u ticks\r\n", ticks);
            TMR0_TimerInit(ticks);
            TMR0_ITCfg(ENABLE, TMR0_3_IT_CYC_END);
            PFIC_EnableIRQ(TMR0_IRQn);
            PFIC_SetPriority(TMR0_IRQn, 0xFF);
            break;
        case HW_TIMER_1:
            dprintf("HW Timer: Starting TMR1 with %u ticks\r\n", ticks);
            TMR1_TimerInit(ticks);
            TMR1_ITCfg(ENABLE, TMR0_3_IT_CYC_END);
            PFIC_EnableIRQ(TMR1_IRQn);
            PFIC_SetPriority(TMR1_IRQn, 0xFF);
            break;
        case HW_TIMER_2:
            dprintf("HW Timer: Starting TMR2 with %u ticks\r\n", ticks);
            TMR2_TimerInit(ticks);
            TMR2_ITCfg(ENABLE, TMR0_3_IT_CYC_END);
            PFIC_EnableIRQ(TMR2_IRQn);
            PFIC_SetPriority(TMR2_IRQn, 0xFF);
            break;
        default:
            break;
    }
}

/**
 * @brief 停止指定定时器硬件
 */
static void timer_hw_stop(hw_timer_id_t id)
{
    switch (id) {
        case HW_TIMER_0:
            TMR0_Disable();
            break;
        case HW_TIMER_1:
            TMR1_Disable();
            break;
        case HW_TIMER_2:
            TMR2_Disable();
            break;
        default:
            break;
    }
}

/**
 * @brief 通用定时器中断处理 (简化版)
 */
__HIGH_CODE
static void timer_irq_handler(hw_timer_id_t id)
{
    hw_timer_context_t *ctx = &g_timer_ctx[id];

    if (ctx->state == HW_TIMER_STATE_RUNNING && ctx->callback) {
        ctx->callback();
    }
}

/*==========================================
 * 中断处理函数
 *=========================================*/

__INTERRUPT
__HIGH_CODE
void TMR0_IRQHandler(void)
{
    if (TMR0_GetITFlag(TMR0_3_IT_CYC_END)) {
        TMR0_ClearITFlag(TMR0_3_IT_CYC_END);
        timer_irq_handler(HW_TIMER_0);
    }
    togglePin(B14);  /* 调试: 验证 TMR1 中断触发 */
}

__INTERRUPT
__HIGH_CODE
void TMR1_IRQHandler(void)
{
    if (TMR1_GetITFlag(TMR0_3_IT_CYC_END)) {
        TMR1_ClearITFlag(TMR0_3_IT_CYC_END);
        timer_irq_handler(HW_TIMER_1);
    }
    togglePin(B14);  /* 调试: 验证 TMR1 中断触发 */
}

__INTERRUPT
__HIGH_CODE
void TMR2_IRQHandler(void)
{
    if (TMR2_GetITFlag(TMR0_3_IT_CYC_END)) {
        TMR2_ClearITFlag(TMR0_3_IT_CYC_END);
        timer_irq_handler(HW_TIMER_2);
    }
    togglePin(B14);  /* 调试: 验证 TMR1 中断触发 */
}

/*==========================================
 * 公共接口实现
 *=========================================*/

error_code_t hw_timer_init(void)
{
    if (g_hw_timer_initialized) {
        dprintf("HW Timer: Already initialized\r\n");
        return ERROR_COMM_ALREADY_INIT;
    }

    // 初始化所有定时器上下文
    for (int i = 0; i < HW_TIMER_COUNT; i++) {
        g_timer_ctx[i].state = HW_TIMER_STATE_IDLE;
        g_timer_ctx[i].interval_ms = 0;
        g_timer_ctx[i].hw_ticks = 0;
        g_timer_ctx[i].callback = NULL;
    }

    g_hw_timer_initialized = true;
    dprintf("HW Timer: Initialized successfully\r\n");
    return NO_ERROR;
}

error_code_t hw_timer_deinit(void)
{
    if (!g_hw_timer_initialized) {
        return NO_ERROR;
    }

    // 停止所有定时器
    for (int i = 0; i < HW_TIMER_COUNT; i++) {
        if (g_timer_ctx[i].state != HW_TIMER_STATE_IDLE) {
            timer_hw_stop((hw_timer_id_t)i);
            g_timer_ctx[i].state = HW_TIMER_STATE_IDLE;
        }
    }

    g_hw_timer_initialized = false;
    return NO_ERROR;
}

error_code_t hw_timer_start(hw_timer_id_t timer_id, uint32_t interval_ms, hw_timer_callback_t callback)
{
    if (!g_hw_timer_initialized) {
        dprintf("HW Timer: ERROR - not initialized!\r\n");
        return ERROR_COMM_STATE;
    }

    if (timer_id >= HW_TIMER_MAX) {
        dprintf("HW Timer: ERROR - invalid timer_id %d\r\n", timer_id);
        return ERROR_INVALID_TIMER_ID;
    }

    if (interval_ms == 0 || interval_ms > MAX_DIRECT_MS) {
        dprintf("HW Timer: ERROR - invalid interval %u (max=%u)\r\n", interval_ms, MAX_DIRECT_MS);
        return ERROR_INVALID_PARAM;
    }

    hw_timer_context_t *ctx = &g_timer_ctx[timer_id];

    // 如果正在运行，先停止
    if (ctx->state == HW_TIMER_STATE_RUNNING) {
        timer_hw_stop(timer_id);
    }

    // 配置上下文
    ctx->interval_ms = interval_ms;
    ctx->hw_ticks = MS_TO_TICKS(interval_ms);
    ctx->callback = callback;
    ctx->state = HW_TIMER_STATE_RUNNING;

    dprintf("HW Timer: Starting timer %d, interval=%ums, ticks=%u\r\n",
            timer_id, interval_ms, ctx->hw_ticks);

    // 启动硬件定时器
    timer_hw_start(timer_id, ctx->hw_ticks);

    return NO_ERROR;
}

error_code_t hw_timer_stop(hw_timer_id_t timer_id)
{
    if (!g_hw_timer_initialized) {
        return ERROR_COMM_STATE;
    }

    if (timer_id >= HW_TIMER_MAX) {
        return ERROR_INVALID_TIMER_ID;
    }

    hw_timer_context_t *ctx = &g_timer_ctx[timer_id];

    if (ctx->state != HW_TIMER_STATE_IDLE) {
        timer_hw_stop(timer_id);
        ctx->state = HW_TIMER_STATE_IDLE;
    }

    return NO_ERROR;
}

error_code_t hw_timer_set_interval(hw_timer_id_t timer_id, uint32_t interval_ms)
{
    if (!g_hw_timer_initialized) {
        return ERROR_COMM_STATE;
    }

    if (timer_id >= HW_TIMER_MAX) {
        return ERROR_INVALID_TIMER_ID;
    }

    if (interval_ms == 0 || interval_ms > MAX_DIRECT_MS) {
        return ERROR_INVALID_PARAM;
    }

    hw_timer_context_t *ctx = &g_timer_ctx[timer_id];

    // 更新间隔配置
    ctx->interval_ms = interval_ms;
    ctx->hw_ticks = MS_TO_TICKS(interval_ms);

    // 如果正在运行，重新启动定时器
    if (ctx->state == HW_TIMER_STATE_RUNNING) {
        timer_hw_stop(timer_id);
        timer_hw_start(timer_id, ctx->hw_ticks);
    }

    return NO_ERROR;
}

error_code_t hw_timer_set_callback(hw_timer_id_t timer_id, hw_timer_callback_t callback)
{
    if (!g_hw_timer_initialized) {
        return ERROR_COMM_STATE;
    }

    if (timer_id >= HW_TIMER_MAX) {
        return ERROR_INVALID_TIMER_ID;
    }

    g_timer_ctx[timer_id].callback = callback;
    return NO_ERROR;
}

hw_timer_state_t hw_timer_get_state(hw_timer_id_t timer_id)
{
    if (timer_id >= HW_TIMER_MAX) {
        return HW_TIMER_STATE_IDLE;
    }

    return g_timer_ctx[timer_id].state;
}

bool hw_timer_is_running(hw_timer_id_t timer_id)
{
    if (timer_id >= HW_TIMER_MAX) {
        return false;
    }

    return g_timer_ctx[timer_id].state == HW_TIMER_STATE_RUNNING;
}
