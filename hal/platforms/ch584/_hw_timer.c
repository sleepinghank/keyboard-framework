/**
 * @file _hw_timer.c
 * @brief CH584 平台硬件定时器 HAL 实现
 * @version 1.0.0
 *
 * 实现说明:
 * - 映射: HW_TIMER_0 → TMR0, HW_TIMER_1 → TMR1, HW_TIMER_2 → TMR2
 * - CH584 系统时钟 78MHz，最大定时周期 67108864 ticks (~860ms)
 * - 对于超过 860ms 的定时，使用软件计数器扩展
 */

#include "hw_timer.h"
#include "CH58x_common.h"

/*==========================================
 * 常量定义
 *=========================================*/

// 系统时钟频率 78MHz
#define FREQ_SYS_HZ         78000000UL

// 毫秒转时钟周期
#define MS_TO_TICKS(ms)     ((uint32_t)(ms) * (FREQ_SYS_HZ / 1000))

// 最大硬件定时周期 (ticks)
#define MAX_HW_TIMER_TICKS  67108864UL

// 基准定时周期 (1ms)
#define BASE_TIMER_TICKS    (FREQ_SYS_HZ / 1000)

// 最大可直接使用硬件定时的毫秒数
#define MAX_DIRECT_MS       (MAX_HW_TIMER_TICKS / (FREQ_SYS_HZ / 1000))

/*==========================================
 * 定时器状态管理
 *=========================================*/

// 定时器上下文结构
typedef struct {
    hw_timer_state_t    state;          // 定时器状态
    hw_timer_mode_t     mode;           // 定时模式
    uint32_t            interval_ms;    // 目标间隔 (毫秒)
    uint32_t            hw_ticks;       // 硬件定时周期 (ticks)
    uint32_t            sw_counter;     // 软件计数器 (用于长定时)
    uint32_t            sw_target;      // 软件计数目标
    uint32_t            remaining_ms;   // 剩余毫秒数 (用于暂停恢复)
    hw_timer_callback_t callback;       // 回调函数
} hw_timer_context_t;

static hw_timer_context_t g_timer_ctx[HW_TIMER_COUNT];
static bool g_hw_timer_initialized = false;

/*==========================================
 * 内部辅助函数
 *=========================================*/

/**
 * @brief 计算定时器配置参数
 * @param interval_ms 目标间隔 (毫秒)
 * @param hw_ticks 输出: 硬件定时周期
 * @param sw_target 输出: 软件计数目标
 */
static void calculate_timer_params(uint32_t interval_ms, uint32_t *hw_ticks, uint32_t *sw_target)
{
    if (interval_ms <= MAX_DIRECT_MS) {
        // 短定时: 直接使用硬件定时器
        *hw_ticks = MS_TO_TICKS(interval_ms);
        *sw_target = 1;
    } else {
        // 长定时: 使用 1ms 基准 + 软件计数
        *hw_ticks = BASE_TIMER_TICKS;
        *sw_target = interval_ms;
    }
}

/**
 * @brief 配置并启动指定定时器硬件
 */
static void timer_hw_start(hw_timer_id_t id, uint32_t ticks)
{
    switch (id) {
        case HW_TIMER_0:
            TMR0_TimerInit(ticks);
            TMR0_ITCfg(ENABLE, TMR0_3_IT_CYC_END);
            PFIC_EnableIRQ(TMR0_IRQn);
            TMR0_Enable();
            break;
        case HW_TIMER_1:
            TMR1_TimerInit(ticks);
            TMR1_ITCfg(ENABLE, TMR0_3_IT_CYC_END);
            PFIC_EnableIRQ(TMR1_IRQn);
            TMR1_Enable();
            break;
        case HW_TIMER_2:
            TMR2_TimerInit(ticks);
            TMR2_ITCfg(ENABLE, TMR0_3_IT_CYC_END);
            PFIC_EnableIRQ(TMR2_IRQn);
            TMR2_Enable();
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
            TMR0_ITCfg(DISABLE, TMR0_3_IT_CYC_END);
            PFIC_DisableIRQ(TMR0_IRQn);
            break;
        case HW_TIMER_1:
            TMR1_Disable();
            TMR1_ITCfg(DISABLE, TMR0_3_IT_CYC_END);
            PFIC_DisableIRQ(TMR1_IRQn);
            break;
        case HW_TIMER_2:
            TMR2_Disable();
            TMR2_ITCfg(DISABLE, TMR0_3_IT_CYC_END);
            PFIC_DisableIRQ(TMR2_IRQn);
            break;
        default:
            break;
    }
}

/**
 * @brief 通用定时器中断处理
 */
static void timer_irq_handler(hw_timer_id_t id)
{
    hw_timer_context_t *ctx = &g_timer_ctx[id];

    if (ctx->state != HW_TIMER_STATE_RUNNING) {
        return;
    }

    // 软件计数器递增
    ctx->sw_counter++;

    if (ctx->sw_counter >= ctx->sw_target) {
        // 达到目标计数，触发回调
        ctx->sw_counter = 0;

        if (ctx->callback) {
            ctx->callback(id);
        }

        // 单次模式自动停止
        if (ctx->mode == HW_TIMER_MODE_ONESHOT) {
            timer_hw_stop(id);
            ctx->state = HW_TIMER_STATE_IDLE;
        }
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
}

__INTERRUPT
__HIGH_CODE
void TMR1_IRQHandler(void)
{
    if (TMR1_GetITFlag(TMR0_3_IT_CYC_END)) {
        TMR1_ClearITFlag(TMR0_3_IT_CYC_END);
        timer_irq_handler(HW_TIMER_1);
    }
}

__INTERRUPT
__HIGH_CODE
void TMR2_IRQHandler(void)
{
    if (TMR2_GetITFlag(TMR0_3_IT_CYC_END)) {
        TMR2_ClearITFlag(TMR0_3_IT_CYC_END);
        timer_irq_handler(HW_TIMER_2);
    }
}

/*==========================================
 * 公共接口实现
 *=========================================*/

error_code_t hw_timer_init(void)
{
    if (g_hw_timer_initialized) {
        return ERROR_COMM_ALREADY_INIT;
    }

    // 初始化所有定时器上下文
    for (int i = 0; i < HW_TIMER_COUNT; i++) {
        g_timer_ctx[i].state = HW_TIMER_STATE_IDLE;
        g_timer_ctx[i].mode = HW_TIMER_MODE_PERIODIC;
        g_timer_ctx[i].interval_ms = 0;
        g_timer_ctx[i].hw_ticks = 0;
        g_timer_ctx[i].sw_counter = 0;
        g_timer_ctx[i].sw_target = 0;
        g_timer_ctx[i].remaining_ms = 0;
        g_timer_ctx[i].callback = NULL;
    }

    g_hw_timer_initialized = true;
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
        return ERROR_COMM_STATE;
    }

    if (timer_id >= HW_TIMER_MAX) {
        return ERROR_INVALID_TIMER_ID;
    }

    if (interval_ms == 0) {
        return ERROR_INVALID_PARAM;
    }

    hw_timer_context_t *ctx = &g_timer_ctx[timer_id];

    // 如果正在运行，先停止
    if (ctx->state == HW_TIMER_STATE_RUNNING) {
        timer_hw_stop(timer_id);
    }

    // 计算定时参数
    calculate_timer_params(interval_ms, &ctx->hw_ticks, &ctx->sw_target);

    // 配置上下文
    ctx->interval_ms = interval_ms;
    ctx->mode = HW_TIMER_MODE_PERIODIC;
    ctx->callback = callback;
    ctx->sw_counter = 0;
    ctx->state = HW_TIMER_STATE_RUNNING;

    // 启动硬件定时器
    timer_hw_start(timer_id, ctx->hw_ticks);

    return NO_ERROR;
}

error_code_t hw_timer_start_oneshot(hw_timer_id_t timer_id, uint32_t delay_ms, hw_timer_callback_t callback)
{
    if (!g_hw_timer_initialized) {
        return ERROR_COMM_STATE;
    }

    if (timer_id >= HW_TIMER_MAX) {
        return ERROR_INVALID_TIMER_ID;
    }

    if (delay_ms == 0) {
        return ERROR_INVALID_PARAM;
    }

    hw_timer_context_t *ctx = &g_timer_ctx[timer_id];

    // 如果正在运行，先停止
    if (ctx->state == HW_TIMER_STATE_RUNNING) {
        timer_hw_stop(timer_id);
    }

    // 计算定时参数
    calculate_timer_params(delay_ms, &ctx->hw_ticks, &ctx->sw_target);

    // 配置上下文
    ctx->interval_ms = delay_ms;
    ctx->mode = HW_TIMER_MODE_ONESHOT;
    ctx->callback = callback;
    ctx->sw_counter = 0;
    ctx->state = HW_TIMER_STATE_RUNNING;

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
        ctx->sw_counter = 0;
    }

    return NO_ERROR;
}

error_code_t hw_timer_pause(hw_timer_id_t timer_id)
{
    if (!g_hw_timer_initialized) {
        return ERROR_COMM_STATE;
    }

    if (timer_id >= HW_TIMER_MAX) {
        return ERROR_INVALID_TIMER_ID;
    }

    hw_timer_context_t *ctx = &g_timer_ctx[timer_id];

    if (ctx->state != HW_TIMER_STATE_RUNNING) {
        return ERROR_COMM_STATE;
    }

    // 保存剩余时间信息
    if (ctx->sw_target > 1) {
        // 长定时模式: 剩余毫秒数
        ctx->remaining_ms = ctx->sw_target - ctx->sw_counter;
    } else {
        // 短定时模式: 读取当前计数器值计算剩余时间
        uint32_t current_count = 0;
        switch (timer_id) {
            case HW_TIMER_0: current_count = TMR0_GetCurrentTimer(); break;
            case HW_TIMER_1: current_count = TMR1_GetCurrentTimer(); break;
            case HW_TIMER_2: current_count = TMR2_GetCurrentTimer(); break;
            default: break;
        }
        ctx->remaining_ms = (ctx->hw_ticks - current_count) / (FREQ_SYS_HZ / 1000);
        if (ctx->remaining_ms == 0) ctx->remaining_ms = 1;
    }

    timer_hw_stop(timer_id);
    ctx->state = HW_TIMER_STATE_PAUSED;

    return NO_ERROR;
}

error_code_t hw_timer_resume(hw_timer_id_t timer_id)
{
    if (!g_hw_timer_initialized) {
        return ERROR_COMM_STATE;
    }

    if (timer_id >= HW_TIMER_MAX) {
        return ERROR_INVALID_TIMER_ID;
    }

    hw_timer_context_t *ctx = &g_timer_ctx[timer_id];

    if (ctx->state != HW_TIMER_STATE_PAUSED) {
        return ERROR_COMM_STATE;
    }

    // 使用剩余时间重新配置定时器
    if (ctx->sw_target > 1) {
        // 长定时模式: 继续使用1ms基准，更新软件计数目标
        ctx->sw_target = ctx->remaining_ms;
        ctx->sw_counter = 0;
    } else {
        // 短定时模式: 使用剩余时间重新配置
        ctx->hw_ticks = MS_TO_TICKS(ctx->remaining_ms);
    }

    ctx->state = HW_TIMER_STATE_RUNNING;
    timer_hw_start(timer_id, ctx->hw_ticks);

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

    if (interval_ms == 0) {
        return ERROR_INVALID_PARAM;
    }

    hw_timer_context_t *ctx = &g_timer_ctx[timer_id];

    // 更新间隔配置
    ctx->interval_ms = interval_ms;
    calculate_timer_params(interval_ms, &ctx->hw_ticks, &ctx->sw_target);

    // 如果正在运行，重新启动定时器
    if (ctx->state == HW_TIMER_STATE_RUNNING) {
        timer_hw_stop(timer_id);
        ctx->sw_counter = 0;
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

uint32_t hw_timer_get_remaining(hw_timer_id_t timer_id)
{
    if (timer_id >= HW_TIMER_MAX) {
        return 0;
    }

    hw_timer_context_t *ctx = &g_timer_ctx[timer_id];

    if (ctx->state == HW_TIMER_STATE_PAUSED) {
        return ctx->remaining_ms;
    }

    if (ctx->state != HW_TIMER_STATE_RUNNING) {
        return 0;
    }

    if (ctx->sw_target > 1) {
        // 长定时模式: 返回剩余毫秒数
        return ctx->sw_target - ctx->sw_counter;
    } else {
        // 短定时模式: 根据当前计数器计算
        uint32_t current_count = 0;
        switch (timer_id) {
            case HW_TIMER_0: current_count = TMR0_GetCurrentTimer(); break;
            case HW_TIMER_1: current_count = TMR1_GetCurrentTimer(); break;
            case HW_TIMER_2: current_count = TMR2_GetCurrentTimer(); break;
            default: break;
        }
        return (ctx->hw_ticks - current_count) / (FREQ_SYS_HZ / 1000);
    }
}
