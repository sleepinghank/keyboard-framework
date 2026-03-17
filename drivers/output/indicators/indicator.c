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
 * @file indicator.c
 * @brief 指示灯驱动实现
 *
 * 使用主循环轮询 + 时间戳方式驱动状态机
 * 参考 Keychron indicator.c 实现
 */

#include "indicator.h"
#include "indicator_hal.h"
#include "indicator_config.h"
#include "timer.h"
#include <string.h>

/* ============ 内部定义 ============ */

// 状态机阶段
typedef enum {
    PHASE_IDLE = 0,     // 空闲
    PHASE_DELAY,        // 延迟等待
    PHASE_ON,           // LED 点亮
    PHASE_OFF,          // LED 熄灭
} ind_phase_t;

// LED 运行时状态
typedef struct {
    ind_effect_t effect;        // 当前灯效配置
    ind_phase_t  phase;         // 当前阶段
    uint8_t      count;         // 已闪烁次数
    bool         is_on;         // 当前是否点亮
    uint32_t     timer;         // 阶段开始时间戳
    uint16_t     next_period;   // 下一次切换的间隔时间
} ind_led_state_t;

/* ============ 模块状态 ============ */

static ind_led_state_t ind_states[IND_LED_COUNT];   // LED 状态数组
static ind_lpm_callback_t ind_lpm_cb = NULL;        // 低功耗回调
static bool ind_initialized = false;                // 初始化标志

/* ============ 预定义灯效 ============ */

// 熄灭：立即关闭 LED，并保持关闭状态
const ind_effect_t IND_OFF = {
    .mode = IND_MODE_OFF,
    .on_ms = 0, .off_ms = 0, .delay_ms = 0, .duration_ms = 0, .repeat = 0
};

// 常亮：立即点亮 LED，并持续保持点亮
const ind_effect_t IND_ON = {
    .mode = IND_MODE_ON,
    .on_ms = 0, .off_ms = 0, .delay_ms = 0, .duration_ms = 0, .repeat = 0
};

// 常亮 1 秒：立即点亮，1 秒后自动熄灭
const ind_effect_t IND_ON_1S = {
    .mode = IND_MODE_ON,
    .on_ms = 0, .off_ms = 0, .delay_ms = 0, .duration_ms = 1000, .repeat = 0
};

// 常亮 2 秒：立即点亮，2 秒后自动熄灭
const ind_effect_t IND_ON_2S = {
    .mode = IND_MODE_ON,
    .on_ms = 0, .off_ms = 0, .delay_ms = 0, .duration_ms = 2000, .repeat = 0
};

// 常亮 3 秒：立即点亮，3 秒后自动熄灭
const ind_effect_t IND_ON_3S = {
    .mode = IND_MODE_ON,
    .on_ms = 0, .off_ms = 0, .delay_ms = 0, .duration_ms = 3000, .repeat = 0
};

// 慢闪：亮 1 秒、灭 1 秒，循环闪烁
const ind_effect_t IND_BLINK_SLOW = {
    .mode = IND_MODE_BLINK,
    .on_ms = 1000, .off_ms = 1000, .delay_ms = 0, .duration_ms = 0, .repeat = 0
};

// 快闪：亮 500ms、灭 500ms，循环闪烁
const ind_effect_t IND_BLINK_FAST = {
    .mode = IND_MODE_BLINK,
    .on_ms = 250, .off_ms = 250, .delay_ms = 0, .duration_ms = 0, .repeat = 0
};

// 闪 1 次：亮 200ms、灭 200ms，完成 1 次后停止
const ind_effect_t IND_BLINK_1 = {
    .mode = IND_MODE_BLINK,
    .on_ms = 200, .off_ms = 200, .delay_ms = 0, .duration_ms = 0, .repeat = 1
};

// 闪 2 次：亮 200ms、灭 200ms，完成 2 次后停止
const ind_effect_t IND_BLINK_2 = {
    .mode = IND_MODE_BLINK,
    .on_ms = 200, .off_ms = 200, .delay_ms = 0, .duration_ms = 0, .repeat = 2
};

// 闪 3 次：亮 200ms、灭 200ms，完成 3 次后停止
const ind_effect_t IND_BLINK_3 = {
    .mode = IND_MODE_BLINK,
    .on_ms = 200, .off_ms = 200, .delay_ms = 0, .duration_ms = 0, .repeat = 3
};

// 延迟常亮：先等待 500ms，再点亮并保持常亮
const ind_effect_t IND_DELAY_ON = {
    .mode = IND_MODE_ON,
    .on_ms = 0, .off_ms = 0, .delay_ms = 500, .duration_ms = 0, .repeat = 0
};

// 延迟闪烁：先等待 500ms，再按 250ms 亮 / 250ms 灭循环闪烁
const ind_effect_t IND_DELAY_BLINK = {
    .mode = IND_MODE_BLINK,
    .on_ms = 250, .off_ms = 250, .delay_ms = 500, .duration_ms = 0, .repeat = 0
};

/* ============ 内部函数 ============ */

/**
 * @brief 检查并触发低功耗回调
 */
static void check_lpm_callback(void) {
    if (ind_lpm_cb == NULL) {
        return;
    }

    bool all_off = !indicator_any_active();
    ind_lpm_cb(all_off);
}

/**
 * @brief 处理单个 LED 的状态机（超时回调）
 */
static void process_led_timeout(uint8_t led_id) {
    if (led_id >= IND_LED_COUNT) {
        return;
    }

    ind_led_state_t* state = &ind_states[led_id];
    const ind_effect_t* eff = &state->effect;

    switch (state->phase) {
        case PHASE_IDLE:
            // 空闲状态，不处理
            state->next_period = 0;
            break;

        case PHASE_DELAY:
            // 延迟结束，进入实际模式
            if (eff->mode == IND_MODE_OFF) {
                // OFF 模式
                ind_hal_set(led_id, false);
                state->is_on = false;
                state->phase = PHASE_IDLE;
                state->next_period = 0;
            } else if (eff->mode == IND_MODE_ON) {
                // ON 模式：点亮
                ind_hal_set(led_id, true);
                state->is_on = true;
                state->phase = PHASE_ON;

                if (eff->duration_ms > 0) {
                    // 有持续时间限制
                    state->next_period = eff->duration_ms;
                } else {
                    // 无限常亮，进入空闲（但保持亮）
                    state->phase = PHASE_IDLE;
                    state->next_period = 0;
                }
            } else if (eff->mode == IND_MODE_BLINK) {
                // BLINK 模式：开始闪烁，先亮
                ind_hal_set(led_id, true);
                state->is_on = true;
                state->count = 0;
                state->phase = PHASE_ON;
                state->next_period = eff->on_ms;
            }
            // 重置定时器
            state->timer = timer_read32();
            break;

        case PHASE_ON:
            // 当前亮，需要切换
            if (eff->mode == IND_MODE_ON) {
                // 常亮模式：duration 到期，熄灭
                ind_hal_set(led_id, false);
                state->is_on = false;
                state->phase = PHASE_IDLE;
                state->next_period = 0;
                check_lpm_callback();
            } else if (eff->mode == IND_MODE_BLINK) {
                // 闪烁模式：从亮切换到灭
                ind_hal_set(led_id, false);
                state->is_on = false;
                state->count++;

                // 检查是否完成所有闪烁
                if (eff->repeat > 0 && state->count >= eff->repeat) {
                    state->phase = PHASE_IDLE;
                    state->next_period = 0;
                    check_lpm_callback();
                } else {
                    state->phase = PHASE_OFF;
                    state->next_period = eff->off_ms;
                    state->timer = timer_read32();
                }
            }
            break;

        case PHASE_OFF:
            // 当前灭，切换到亮
            if (eff->mode == IND_MODE_BLINK) {
                ind_hal_set(led_id, true);
                state->is_on = true;
                state->phase = PHASE_ON;
                state->next_period = eff->on_ms;
                state->timer = timer_read32();
            }
            break;
    }
}

/**
 * @brief 启动 LED 灯效
 */
static void start_led_effect(uint8_t led_id, const ind_effect_t* effect) {
    if (led_id >= IND_LED_COUNT || effect == NULL) {
        return;
    }

    ind_led_state_t* state = &ind_states[led_id];

    // 保存新配置
    state->effect = *effect;
    state->count = 0;
    state->is_on = false;
    state->timer = timer_read32();

    // 根据模式处理
    if (effect->mode == IND_MODE_OFF) {
        // OFF 模式：立即熄灭
        ind_hal_set(led_id, false);
        state->phase = PHASE_IDLE;
        state->next_period = 0;
        check_lpm_callback();
    } else if (effect->delay_ms > 0) {
        // 有延迟：进入延迟阶段
        ind_hal_set(led_id, false);
        state->phase = PHASE_DELAY;
        state->next_period = effect->delay_ms;
    } else {
        // 无延迟：立即进入延迟处理（会转到实际模式）
        state->phase = PHASE_DELAY;
        state->next_period = 0;
        process_led_timeout(led_id);
    }
}

/* ============ 公共接口 ============ */

void indicator_init(void) {
    if (ind_initialized) {
        return;
    }

    // 初始化 HAL
    ind_hal_init();

    // 初始化状态
    for (uint8_t i = 0; i < IND_LED_COUNT; i++) {
        memset(&ind_states[i], 0, sizeof(ind_led_state_t));
        ind_states[i].phase = PHASE_IDLE;
    }

    ind_initialized = true;
}

void indicator_deinit(void) {
    if (!ind_initialized) {
        return;
    }

    // 停止所有灯效
    indicator_off_all();

    // 反初始化 HAL
    ind_hal_deinit();

    ind_initialized = false;
}

void indicator_set(uint8_t led_id, const ind_effect_t* effect) {
    if (!ind_initialized || led_id >= IND_LED_COUNT || effect == NULL) {
        return;
    }

    start_led_effect(led_id, effect);
}

void indicator_off(uint8_t led_id) {
    indicator_set(led_id, &IND_OFF);
}

void indicator_off_all(void) {
    for (uint8_t i = 0; i < IND_LED_COUNT; i++) {
        indicator_off(i);
    }
}

bool indicator_is_active(uint8_t led_id) {
    if (!ind_initialized || led_id >= IND_LED_COUNT) {
        return false;
    }

    ind_led_state_t* state = &ind_states[led_id];

    // IDLE 且灯灭才算不活跃
    return (state->phase != PHASE_IDLE) || state->is_on;
}

bool indicator_any_active(void) {
    for (uint8_t i = 0; i < IND_LED_COUNT; i++) {
        if (indicator_is_active(i)) {
            return true;
        }
    }
    return false;
}

void indicator_set_lpm_callback(ind_lpm_callback_t callback) {
    ind_lpm_cb = callback;
}

void indicator_task(void) {
    if (!ind_initialized) {
        return;
    }

    // 遍历所有 LED，检查是否超时
    for (uint8_t i = 0; i < IND_LED_COUNT; i++) {
        ind_led_state_t* state = &ind_states[i];

        // 只处理非空闲状态
        if (state->phase != PHASE_IDLE && state->next_period > 0) {
            // 检查是否超时
            if (timer_elapsed32(state->timer) >= state->next_period) {
                process_led_timeout(i);
            }
        }
    }
}
