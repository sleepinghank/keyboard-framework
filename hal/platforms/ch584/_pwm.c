/* Copyright 2021 QMK
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#include "pwm.h"
#include "gpio.h"
#include "CH58x_common.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/* CH584 标准外设库头文件 */
#include "CH58x_pwm.h"
#include "CH58x_gpio.h"

/* PWM通道状态结构体 */
typedef struct {
    pin_t pin;              // GPIO pin bound to this channel
    pin_t enable_pin;       // Enable pin (optional, can be NO_PIN)
    bool initialized;       // Channel initialization flag
    bool enabled;           // Channel enabled flag
    uint8_t duty_cycle;     // Current duty cycle (0-100)
    uint32_t frequency;     // Current frequency in Hz
    uint8_t polarity;       // Polarity: 0=Normal, 1=Inverted
} pwm_channel_state_t;

/* PWM channel state array - CH584 has 8 PWM channels (PWM4-PWM11) */
static pwm_channel_state_t pwm_channels[8] = {0};

/* 当前PWM配置状态 */
static uint32_t g_pwm_frequency = 1000;  /* 默认1KHz */
static uint8_t g_pwm_divider = 4;        /* 默认时钟分频 */
static PWMX_CycleTypeDef g_pwm_cycle = PWMX_Cycle_64;  /* 默认64周期 */

/* 内部辅助函数 */

/**
 * @brief 将HAL PWM通道号转换为CH584 PWM通道掩码
 *
 * @param channel HAL PWM通道 (PWM_CHANNEL_0-7)
 * @return CH584 PWM通道掩码 (CH_PWM4-CH_PWM11)
 */
static uint8_t hal_to_ch584_channel(pwm_channel_t channel) {
    return (uint8_t)(channel << 4);  /* PWM_CHANNEL_0->CH_PWM4, PWM_CHANNEL_1->CH_PWM5, ... */
}

/**
 * @brief 将CH584 PWM通道掩码转换为HAL PWM通道号
 *
 * @param ch584_channel CH584 PWM通道掩码 (CH_PWM4-CH_PWM11)
 * @return HAL PWM通道号 (PWM_CHANNEL_0-7)
 */
static pwm_channel_t ch584_to_hal_channel(uint8_t ch584_channel) {
    return (pwm_channel_t)(ch584_channel >> 4);
}

/**
 * @brief 计算PWM周期配置
 *
 * @param frequency PWM频率(Hz)
 * @return PWMX_CycleTypeDef 周期配置
 */
static PWMX_CycleTypeDef calculate_pwm_cycle(uint32_t frequency) {
    uint32_t sysclk = FREQ_SYS;  /* 系统时钟频率 */

    /* 计算所需的周期数 */
    uint32_t cycles_needed = sysclk / (frequency * g_pwm_divider);

    /* 根据需要的周期数选择最接近的配置 */
    if (cycles_needed >= 256) {
        return PWMX_Cycle_256;
    } else if (cycles_needed >= 128) {
        return PWMX_Cycle_128;
    } else if (cycles_needed >= 64) {
        return PWMX_Cycle_64;
    } else {
        return PWMX_Cycle_63;
    }
}

/**
 * @brief 更新PWM硬件配置
 *
 * @param channel PWM通道
 */
static void update_pwm_hardware(pwm_channel_t channel) {
    uint8_t ch584_ch = hal_to_ch584_channel(channel);

    /* 遍历所有设置的通道 */
    for (int i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            /* 获取CH584通道对应的掩码 */
            uint8_t ch584_bit = (1 << (i + 4));  /* PWM_CHANNEL_0对应CH_PWM4 (bit 4) */

            /* 计算占空比数据 */
            uint16_t duty_data;
            if (g_pwm_frequency <= 10000) {  /* 对于低频，使用16位精度 */
                /* 16位模式下的占空比计算 */
                uint32_t period_cycles = 1;
                switch (g_pwm_cycle) {
                    case PWMX_Cycle_256: period_cycles = 256; break;
                    case PWMX_Cycle_255: period_cycles = 255; break;
                    case PWMX_Cycle_128: period_cycles = 128; break;
                    case PWMX_Cycle_127: period_cycles = 127; break;
                    case PWMX_Cycle_64:  period_cycles = 64; break;
                    case PWMX_Cycle_63:  period_cycles = 63; break;
                    default: period_cycles = 64; break;
                }
                duty_data = (pwm_channels[i].duty_cycle * period_cycles) / 100;

                /* 使用16位输出配置 */
                PWMX_16bit_ACTOUT(ch584_bit, duty_data,
                                  pwm_channels[i].polarity ? Low_Level : High_Level,
                                  pwm_channels[i].enabled ? ENABLE : DISABLE);
            } else {  /* 对于高频，使用8位精度 */
                /* 8位模式下的占空比计算 */
                uint32_t period_cycles = 1;
                switch (g_pwm_cycle) {
                    case PWMX_Cycle_256: period_cycles = 256; break;
                    case PWMX_Cycle_255: period_cycles = 255; break;
                    case PWMX_Cycle_128: period_cycles = 128; break;
                    case PWMX_Cycle_127: period_cycles = 127; break;
                    case PWMX_Cycle_64:  period_cycles = 64; break;
                    case PWMX_Cycle_63:  period_cycles = 63; break;
                    default: period_cycles = 64; break;
                }
                duty_data = (pwm_channels[i].duty_cycle * period_cycles) / 100;

                /* 使用8位输出配置 */
                PWMX_ACTOUT(ch584_bit, (uint8_t)duty_data,
                           pwm_channels[i].polarity ? Low_Level : High_Level,
                           pwm_channels[i].enabled ? ENABLE : DISABLE);
            }
        }
    }
}

/**
 * @brief 配置所有已启用通道的频率
 *
 * @param frequency PWM频率(Hz)
 */
static void configure_frequency_for_all(uint32_t frequency) {
    /* 计算新的周期配置 */
    PWMX_CycleTypeDef new_cycle = calculate_pwm_cycle(frequency);

    /* 更新全局频率和周期设置 */
    g_pwm_frequency = frequency;
    g_pwm_cycle = new_cycle;

    /* 配置PWM硬件 */
    PWMX_CLKCfg(g_pwm_divider);
    PWMX_CycleCfg(new_cycle);

    /* 更新所有已初始化的通道 */
    for (int i = 0; i < 8; i++) {
        if (pwm_channels[i].initialized) {
            pwm_channels[i].frequency = frequency;
            update_pwm_hardware(1 << i);
        }
    }
}

/*==========================================
 * PWM初始化和基础功能
 *=========================================*/

void pwm_init(void) {
    int i;

    /* 初始化所有通道状态 */
    for (i = 0; i < 8; i++) {
        pwm_channels[i].initialized = false;
        pwm_channels[i].enabled = false;
        pwm_channels[i].pin = NO_PIN;
        pwm_channels[i].enable_pin = NO_PIN;
        pwm_channels[i].duty_cycle = 0;
        pwm_channels[i].frequency = 1000;  /* 默认1KHz */
        pwm_channels[i].polarity = 0;      /* 默认正常极性 */
    }

    /* 配置默认PWM参数 */
    g_pwm_frequency = 1000;
    g_pwm_divider = 4;
    g_pwm_cycle = PWMX_Cycle_64;

    /* 配置PWM硬件 */
    PWMX_CLKCfg(g_pwm_divider);
    PWMX_CycleCfg(g_pwm_cycle);
}

void pwm_start(pwm_channel_t channel) {
    int i;

    /* 遍历所有可能的通道 */
    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            if (!pwm_channels[i].initialized) {
                /* 如果通道未初始化，先进行初始化 */
                pwm_channels[i].initialized = true;
            }

            /* 启用通道 */
            pwm_channels[i].enabled = true;

            /* 更新硬件配置 */
            update_pwm_hardware(1 << i);
        }
    }
}

void pwm_stop(pwm_channel_t channel) {
    int i;

    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            /* 禁用通道 */
            pwm_channels[i].enabled = false;

            /* 更新硬件配置 */
            uint8_t ch584_ch = hal_to_ch584_channel(1 << i);
            PWMX_ACTOUT(ch584_ch, 0, High_Level, DISABLE);
        }
    }
}

void pwm_set_frequency(pwm_channel_t channel, uint32_t frequency) {
    int i;
    bool configure_all = false;

    /* 检查是否需要更新所有通道的频率 */
    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            if (pwm_channels[i].initialized) {
                configure_all = true;
                break;
            }
        }
    }

    if (configure_all) {
        /* 更新所有已初始化通道的频率 */
        configure_frequency_for_all(frequency);
    } else {
        /* 仅保存频率设置，等待初始化时应用 */
        for (i = 0; i < 8; i++) {
            if (channel & (1 << i)) {
                pwm_channels[i].frequency = frequency;
            }
        }
    }
}

void pwm_set_duty_cycle(pwm_channel_t channel, uint8_t duty_cycle) {
    int i;

    /* 限制占空比范围 */
    if (duty_cycle > 100) duty_cycle = 100;

    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            pwm_channels[i].duty_cycle = duty_cycle;

            /* 如果通道已启用，立即更新硬件 */
            if (pwm_channels[i].enabled) {
                update_pwm_hardware(1 << i);
            }
        }
    }
}

void pwm_set_duty_percentage(pwm_channel_t channel, float percentage) {
    /* 将浮点数占空比转换为整数 */
    uint8_t duty_cycle = (uint8_t)percentage;
    pwm_set_duty_cycle(channel, duty_cycle);
}

uint8_t pwm_get_duty_cycle(pwm_channel_t channel) {
    int i;

    /* 对于多通道请求，返回第一个设置位的值 */
    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            return pwm_channels[i].duty_cycle;
        }
    }
    return 0;
}

float pwm_get_duty_percentage(pwm_channel_t channel) {
    int i;

    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            return (float)pwm_channels[i].duty_cycle;
        }
    }
    return 0.0f;
}

void pwm_set_pulse_width(pwm_channel_t channel, uint32_t pulse_width_ns) {
    int i;

    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            /* 根据当前频率计算占空比 */
            if (pwm_channels[i].frequency > 0) {
                uint32_t period_ns = 1000000000UL / pwm_channels[i].frequency;
                uint32_t duty_ns = pulse_width_ns;
                uint8_t duty_cycle = (uint8_t)((duty_ns * 100) / period_ns);

                pwm_channels[i].duty_cycle = duty_cycle;

                /* 如果通道已启用，立即更新硬件 */
                if (pwm_channels[i].enabled) {
                    update_pwm_hardware(1 << i);
                }
            }
        }
    }
}

void pwm_set_period(pwm_channel_t channel, uint32_t period_ns) {
    int i;

    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            /* 根据周期计算频率 */
            if (period_ns > 0) {
                uint32_t frequency = 1000000000UL / period_ns;
                pwm_set_frequency(1 << i, frequency);
            }
        }
    }
}

void pwm_set_polarity(pwm_channel_t channel, uint8_t polarity) {
    int i;

    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            pwm_channels[i].polarity = polarity;

            /* 如果通道已启用，立即更新硬件 */
            if (pwm_channels[i].enabled) {
                update_pwm_hardware(1 << i);
            }
        }
    }
}

void pwm_enable(pwm_channel_t channel) {
    int i;

    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            /* 如果有使能引脚，设置为高电平 */
            if (pwm_channels[i].enable_pin != NO_PIN) {
                gpio_write_pin_high(pwm_channels[i].enable_pin);
            }

            /* 标记通道为已初始化 */
            pwm_channels[i].initialized = true;
        }
    }
}

void pwm_disable(pwm_channel_t channel) {
    int i;

    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            /* 如果有使能引脚，设置为低电平 */
            if (pwm_channels[i].enable_pin != NO_PIN) {
                gpio_write_pin_low(pwm_channels[i].enable_pin);
            }

            /* 标记通道为未初始化 */
            pwm_channels[i].initialized = false;
            pwm_channels[i].enabled = false;
        }
    }
}

void pwm_configure(pwm_channel_t channel, uint32_t frequency, uint8_t duty_cycle) {
    int i;

    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            pwm_channels[i].frequency = frequency;
            pwm_channels[i].duty_cycle = duty_cycle;
            pwm_channels[i].initialized = true;
        }
    }

    /* 配置频率（这会更新所有已初始化的通道） */
    configure_frequency_for_all(frequency);
}

/*==========================================
 * GPIO引脚绑定功能
 *=========================================*/

bool pwm_bind_pin(pin_t pin, pwm_channel_t channel) {
    int i;

    /* 查找第一个设置的通道位 */
    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            /* 检查此引脚是否已绑定到其他通道 */
            int j;
            for (j = 0; j < 8; j++) {
                if (pwm_channels[j].pin == pin) {
                    return false;  /* 引脚已被绑定 */
                }
            }

            /* 将引脚绑定到此通道 */
            pwm_channels[i].pin = pin;

            /* 配置GPIO引脚为PWM输出（推挽输出，最大5mA） */
            if (pin != NO_PIN) {
                uint8_t port = GET_GPIO_PORT(pin);  /* 获取端口号：0=PORTA, 1=PORTB */
                uint8_t pin_num = GET_GPIO_PIN(pin);  /* 获取引脚号：0-31 */

                /* 转换为CH584 GPIO引脚掩码 */
                uint32_t gpio_pin_mask = (pin_num < 32) ? (1 << pin_num) : 0;

                if (gpio_pin_mask != 0) {
                    /* 根据端口号配置GPIO */
                    if (port == 0) {  /* PORTA */
                        GPIOA_ModeCfg(gpio_pin_mask, GPIO_ModeOut_PP_5mA);
                    } else if (port == 1) {  /* PORTB */
                        GPIOB_ModeCfg(gpio_pin_mask, GPIO_ModeOut_PP_5mA);
                    }
                }
            }

            return true;
        }
    }

    return false;  /* 无效通道 */
}

bool pwm_unbind_pin(pin_t pin) {
    int i;

    for (i = 0; i < 8; i++) {
        if (pwm_channels[i].pin == pin) {
            pwm_channels[i].pin = NO_PIN;
            pwm_channels[i].initialized = false;
            pwm_channels[i].enabled = false;

            /* 禁用对应的PWM通道 */
            uint8_t ch584_ch = hal_to_ch584_channel(1 << i);
            PWMX_ACTOUT(ch584_ch, 0, High_Level, DISABLE);

            return true;
        }
    }

    return false;  /* 引脚未绑定 */
}

pin_t pwm_get_bound_pin(pwm_channel_t channel) {
    int i;

    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            return pwm_channels[i].pin;
        }
    }

    return NO_PIN;
}

bool pwm_is_bound(pwm_channel_t channel) {
    int i;

    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            return (pwm_channels[i].pin != NO_PIN);
        }
    }

    return false;
}

bool pwm_bind_enable_pin(pin_t enable_pin, pwm_channel_t channel) {
    int i;

    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            /* 检查此使能引脚是否已绑定到其他通道 */
            int j;
            for (j = 0; j < 8; j++) {
                if (pwm_channels[j].enable_pin == enable_pin) {
                    return false;  /* 使能引脚已被绑定 */
                }
            }

            /* 绑定使能引脚到此通道 */
            pwm_channels[i].enable_pin = enable_pin;

            /* TODO: 在实际实现中，您需要配置使能引脚为GPIO输出 */

            return true;
        }
    }

    return false;  /* 无效通道 */
}

pin_t pwm_get_bound_enable_pin(pwm_channel_t channel) {
    int i;

    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            return pwm_channels[i].enable_pin;
        }
    }

    return NO_PIN;
}
