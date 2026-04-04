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

#include "gpio.h"
#include "pin_defs.h"
#include <stdint.h>
#include <stdbool.h>
#include "debug.h"
#include "input_service.h"
#include "kb904/config_product.h"
/* 引入CH584标准外设库头文件 */
#include "CH58x_gpio.h"
#include "CH58x_common.h"

/**
 * @brief 将标准引脚编号转换为CH584的GPIO位掩码
 *
 * @param pin 标准引脚编号
 * @return CH584 GPIO位掩码
 */
static uint32_t get_gpio_bit_mask(pin_t pin) {
    uint8_t pin_num = GET_GPIO_PIN(pin);
    return (1U << pin_num);
}

/**
 * @brief 设置引脚为输入模式（浮空输入）
 *
 * @param pin 引脚编号
 */
void gpio_set_pin_input(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint32_t pin_bit = get_gpio_bit_mask(pin);

    if (port == PORTA) {
        GPIOA_ModeCfg(pin_bit, GPIO_ModeIN_Floating);
    } else if (port == PORTB) {
        GPIOB_ModeCfg(pin_bit, GPIO_ModeIN_Floating);
    }
}

/**
 * @brief 设置引脚为高电平输入模式（上拉输入）
 *
 * @param pin 引脚编号
 */
void gpio_set_pin_input_high(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint32_t pin_bit = get_gpio_bit_mask(pin);

    if (port == PORTA) {
        GPIOA_ModeCfg(pin_bit, GPIO_ModeIN_PU);
    } else if (port == PORTB) {
        GPIOB_ModeCfg(pin_bit, GPIO_ModeIN_PU);
    }
}

/**
 * @brief 设置引脚为低电平输入模式（下拉输入）
 *
 * @param pin 引脚编号
 */
void gpio_set_pin_input_low(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint32_t pin_bit = get_gpio_bit_mask(pin);

    if (port == PORTA) {
        GPIOA_ModeCfg(pin_bit, GPIO_ModeIN_PD);
    } else if (port == PORTB) {
        GPIOB_ModeCfg(pin_bit, GPIO_ModeIN_PD);
    }
}

/**
 * @brief 设置引脚为推挽输出模式（默认5mA驱动能力）
 *
 * @param pin 引脚编号
 */
void gpio_set_pin_output_push_pull(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint32_t pin_bit = get_gpio_bit_mask(pin);

    if (port == PORTA) {
        /* 使用5mA推挽输出作为默认输出模式 */
        GPIOA_ModeCfg(pin_bit, GPIO_ModeOut_PP_5mA);
    } else if (port == PORTB) {
        /* 使用5mA推挽输出作为默认输出模式 */
        GPIOB_ModeCfg(pin_bit, GPIO_ModeOut_PP_5mA);
    }
}

/**
 * @brief 设置引脚为开漏输出模式
 *
 * @param pin 引脚编号
 * @note CH584芯片GPIO不支持真正的开漏输出模式，这里使用浮空输入模拟
 *       实际使用中需要在外部接上拉电阻
 */
void gpio_set_pin_output_open_drain(pin_t pin) {
    /* CH584不支持开漏输出，使用浮空输入作为替代 */
    /* 实际使用需要在外部连接上拉电阻 */
    gpio_set_pin_input(pin);
}

/**
 * @brief 设置引脚为输出模式（默认推挽输出，5mA）
 *
 * @param pin 引脚编号
 */
void gpio_set_pin_output(pin_t pin) {
    gpio_set_pin_output_push_pull(pin);
}

/**
 * @brief 设置引脚为高电平
 *
 * @param pin 引脚编号
 */
void gpio_write_pin_high(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint32_t pin_bit = get_gpio_bit_mask(pin);

    if (port == PORTA) {
        GPIOA_SetBits(pin_bit);
    } else if (port == PORTB) {
        GPIOB_SetBits(pin_bit);
    }
}

/**
 * @brief 设置引脚为低电平
 *
 * @param pin 引脚编号
 */
void gpio_write_pin_low(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint32_t pin_bit = get_gpio_bit_mask(pin);

    if (port == PORTA) {
        GPIOA_ResetBits(pin_bit);
    } else if (port == PORTB) {
        GPIOB_ResetBits(pin_bit);
    }
}

/**
 * @brief 根据电平值设置引脚状态
 *
 * @param pin 引脚编号
 * @param level 电平值，1为高电平，0为低电平
 */
void gpio_write_pin(pin_t pin, uint8_t level) {
    if (level) {
        gpio_write_pin_high(pin);
    } else {
        gpio_write_pin_low(pin);
    }
}

/**
 * @brief 读取引脚电平状态
 *
 * @param pin 引脚编号
 * @return 引脚当前电平状态（0或1）
 */
uint8_t gpio_read_pin(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint32_t pin_bit = get_gpio_bit_mask(pin);

    if (port == PORTA) {
        /* GPIOA_ReadPortPin 返回非零值表示高电平，0表示低电平 */
        return (GPIOA_ReadPortPin(pin_bit) ? 1 : 0);
    } else if (port == PORTB) {
        /* GPIOB_ReadPortPin 返回非零值表示高电平，0表示低电平 */
        return (GPIOB_ReadPortPin(pin_bit) ? 1 : 0);
    }
    return 0;
}

/**
 * @brief 切换引脚电平状态
 *
 * @param pin 引脚编号
 */
void gpio_toggle_pin(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint32_t pin_bit = get_gpio_bit_mask(pin);

    if (port == PORTA) {
        GPIOA_InverseBits(pin_bit);
    } else if (port == PORTB) {
        GPIOB_InverseBits(pin_bit);
    }
}

/*********************************************************************
 * GPIO 中断接口实现
 *********************************************************************/

/* 回调槽最大数量（可在平台头文件中覆盖） */
#ifndef GPIO_INT_MAX_SLOTS
#define GPIO_INT_MAX_SLOTS  4
#endif

/* 回调槽结构体 */
typedef struct {
    uint32_t            pin_mask;   /* 位掩码，0 表示空槽 */
    gpio_int_callback_t callback;   /* 共享回调 */
    gpio_int_trigger_t  trigger;    /* 触发类型 */
} gpio_int_slot_t;

/* 端口 A/B 回调槽表 */
static gpio_int_slot_t s_porta_slots[GPIO_INT_MAX_SLOTS];
static gpio_int_slot_t s_portb_slots[GPIO_INT_MAX_SLOTS];

/* CH584 触发类型映射表 */
static const GPIOITModeTpDef s_trigger_map[] = {
    [GPIO_INT_FALLING]    = GPIO_ITMode_FallEdge,
    [GPIO_INT_RISING]     = GPIO_ITMode_RiseEdge,
    [GPIO_INT_LOW_LEVEL]  = GPIO_ITMode_LowLevel,
    [GPIO_INT_HIGH_LEVEL] = GPIO_ITMode_HighLevel
};

/**
 * @brief 查找槽位
 *        优先查找已有相同回调的槽，否则返回空槽索引
 *
 * @param slots 槽表指针
 * @param callback 回调函数
 * @param[out] empty_idx 空槽索引（-1 表示无空槽）
 * @return 已有相同回调的槽索引，-1 表示未找到
 */
static int _find_slot(gpio_int_slot_t *slots, gpio_int_callback_t callback, int *empty_idx) {
    int found = -1;
    *empty_idx = -1;

    for (int i = 0; i < GPIO_INT_MAX_SLOTS; i++) {
        if (slots[i].pin_mask == 0 && *empty_idx < 0) {
            *empty_idx = i;  /* 记录第一个空槽 */
        }
        if (slots[i].callback == callback && slots[i].pin_mask != 0) {
            found = i;  /* 找到相同回调的槽 */
        }
    }
    return found;
}

/**
 * @brief 从所有槽中移除指定 pin 位
 *
 * @param slots 槽表指针
 * @param mask 要移除的 pin 位掩码
 */
static void _remove_pin_from_all_slots(gpio_int_slot_t *slots, uint32_t mask) {
    for (int i = 0; i < GPIO_INT_MAX_SLOTS; i++) {
        if (slots[i].pin_mask & mask) {
            slots[i].pin_mask &= ~mask;
            if (slots[i].pin_mask == 0) {
                slots[i].callback = NULL;
            }
        }
    }
}

/**
 * @brief 配置中断（内部函数）
 *        调用 CH58x 官方 API 配置中断模式 + 清标志 + PFIC 使能 + 唤醒源配置
 *
 * @param port 端口号 (PORTA/PORTB)
 * @param mask pin 位掩码
 * @param trigger 触发类型
 */
static void _config_interrupt(uint8_t port, uint32_t mask, gpio_int_trigger_t trigger) {
    GPIOITModeTpDef mode = s_trigger_map[trigger];
    dprintf("Configuring GPIO interrupt: port=%c mask=0x%08X trigger=%d\n",
            (port == PORTA) ? 'A' : 'B', mask, trigger);
    if (port == PORTA) {
        GPIOA_ModeCfg(mask, GPIO_ModeIN_PU);
        GPIOA_ITModeCfg(mask, mode);
        GPIOA_ClearITFlagBit(mask);
        PFIC_ClearPendingIRQ(GPIO_A_IRQn);
        PFIC_EnableIRQ(GPIO_A_IRQn);
    } else if (port == PORTB) {
        GPIOB_ModeCfg(mask, GPIO_ModeIN_PU);
        GPIOB_ITModeCfg(mask, mode);
        GPIOB_ClearITFlagBit(mask);
        PFIC_ClearPendingIRQ(GPIO_B_IRQn);
        PFIC_EnableIRQ(GPIO_B_IRQn);
    }
    /* 开启 GPIO 唤醒源 */
    PWR_PeriphWakeUpCfg(ENABLE, RB_SLP_GPIO_WAKE, Long_Delay);
}

/**
 * @brief 禁用中断（内部函数）
 *        直接操作 INT_EN 寄存器（无官方 API）+ 清标志 + PFIC 自动管理
 *
 * @param port 端口号 (PORTA/PORTB)
 * @param mask pin 位掩码
 * @param slots 对应端口的槽表
 */
static void _disable_interrupt(uint8_t port, uint32_t mask, gpio_int_slot_t *slots) {
    if (port == PORTA) {
        R16_PA_INT_EN &= ~mask;
        GPIOA_ClearITFlagBit(mask);
    } else if (port == PORTB) {
        /* PB22/PB23 特殊处理：INT_EN 寄存器需要 >>14 位映射 */
        uint32_t en_mask = mask | ((mask & (GPIO_Pin_22 | GPIO_Pin_23)) >> 14);
        R16_PB_INT_EN &= ~en_mask;
        GPIOB_ClearITFlagBit(mask);
    }

    /* 检查该端口所有槽是否为空，若空则禁用 PFIC */
    bool all_empty = true;
    for (int i = 0; i < GPIO_INT_MAX_SLOTS; i++) {
        if (slots[i].pin_mask != 0) {
            all_empty = false;
            break;
        }
    }
    if (all_empty) {
        if (port == PORTA) {
            PFIC_DisableIRQ(GPIO_A_IRQn);
        } else if (port == PORTB) {
            PFIC_DisableIRQ(GPIO_B_IRQn);
        }
    }
}

/**
 * @brief ISR 分发函数
 *        遍历槽 -> 读标志 -> 清标志 -> 逐位调用回调
 *
 * @param slots 槽表指针
 * @param port 端口号 (PORTA/PORTB)
 */
__HIGH_CODE
static void _gpio_irq_dispatch(gpio_int_slot_t *slots, uint8_t port) {
    for (int i = 0; i < GPIO_INT_MAX_SLOTS; i++) {
        if (slots[i].pin_mask == 0) {
            continue;
        }

        uint32_t hit = 0;
        if (port == PORTA) {
            hit = GPIOA_ReadITFlagBit(slots[i].pin_mask);
            if (hit) {
                GPIOA_ClearITFlagBit(hit);
            }
        } else if (port == PORTB) {
            hit = GPIOB_ReadITFlagBit(slots[i].pin_mask);
            if (hit) {
                GPIOB_ClearITFlagBit(hit);
            }
        }

        /* 逐位调用回调 */
        while (hit) {
            uint8_t bit_pos = __builtin_ctz(hit);  /* 获取最低位位置 */
            hit &= ~(1U << bit_pos);               /* 清除该位 */
            pin_t pin = PAL_LINE(port, bit_pos);
            if (slots[i].callback) {
                slots[i].callback(pin);
            }
        }
    }
}

/**
 * @brief 启用 GPIO 中断
 *
 * @param pin 引脚编号
 * @param trigger 触发类型
 * @param callback 中断回调函数
 */
void gpio_enable_interrupt(pin_t pin, gpio_int_trigger_t trigger, gpio_int_callback_t callback) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint32_t pin_bit = get_gpio_bit_mask(pin);
    uint32_t irq_state;

    gpio_int_slot_t *slots = (port == PORTA) ? s_porta_slots : s_portb_slots;

    SYS_DisableAllIrq(&irq_state);
    {
        /* 重叠检测：先从所有槽移除该 pin */
        _remove_pin_from_all_slots(slots, pin_bit);

        /* 查找可用槽 */
        int empty_idx;
        int slot_idx = _find_slot(slots, callback, &empty_idx);

        if (slot_idx < 0) {
            /* 无相同回调槽，使用空槽 */
            if (empty_idx < 0) {
                SYS_RecoverIrq(irq_state);
                return;  /* 无可用槽 */
            }
            slot_idx = empty_idx;
            slots[slot_idx].callback = callback;
            slots[slot_idx].trigger = trigger;
        }

        /* 添加 pin 到槽 */
        slots[slot_idx].pin_mask |= pin_bit;

        /* 配置硬件中断 */
        _config_interrupt(port, pin_bit, trigger);
    }
    SYS_RecoverIrq(irq_state);
}

/**
 * @brief 禁用 GPIO 中断
 *
 * @param pin 引脚编号
 */
void gpio_disable_interrupt(pin_t pin) {
    uint8_t port = GET_GPIO_PORT(pin);
    uint32_t pin_bit = get_gpio_bit_mask(pin);
    uint32_t irq_state;

    gpio_int_slot_t *slots = (port == PORTA) ? s_porta_slots : s_portb_slots;

    SYS_DisableAllIrq(&irq_state);
    {
        /* 从所有槽移除该 pin */
        _remove_pin_from_all_slots(slots, pin_bit);

        /* 禁用硬件中断 */
        _disable_interrupt(port, pin_bit, slots);
    }
    SYS_RecoverIrq(irq_state);
}

/**
 * @brief 批量启用 GPIO 中断（原子语义）
 *        全部成功或全部回滚
 *
 * @param pins 引脚编号数组
 * @param count 引脚数量
 * @param trigger 触发类型
 * @param callback 共享回调
 * @return true 全部成功，false 失败
 */
bool gpio_enable_interrupt_batch(const pin_t *pins, uint8_t count,
                                 gpio_int_trigger_t trigger,
                                 gpio_int_callback_t callback) {
    if (count == 0 || pins == NULL) {
        return false;
    }

    /* 按 port 分组，计算掩码 */
    uint32_t porta_mask = 0;
    uint32_t portb_mask = 0;

    for (uint8_t i = 0; i < count; i++) {
        pin_t pin = pins[i];
        uint8_t port = GET_GPIO_PORT(pin);
        uint8_t pin_num = GET_GPIO_PIN(pin);

        if (port == PORTA) {
            porta_mask |= (1U << pin_num);
        } else if (port == PORTB) {
            portb_mask |= (1U << pin_num);
        }
    }

    uint32_t irq_state;
    SYS_DisableAllIrq(&irq_state);
    {
        /* Port A 处理 */
        if (porta_mask) {
            int empty_idx;
            int slot_idx = _find_slot(s_porta_slots, callback, &empty_idx);

            if (slot_idx < 0) {
                if (empty_idx < 0) {
                    SYS_RecoverIrq(irq_state);
                    return false;  /* 无可用槽，回滚 */
                }
                slot_idx = empty_idx;
                s_porta_slots[slot_idx].callback = callback;
                s_porta_slots[slot_idx].trigger = trigger;
            }

            /* 重叠检测：先移除 */
            _remove_pin_from_all_slots(s_porta_slots, porta_mask);
            /* 添加 pin 到槽 */
            s_porta_slots[slot_idx].pin_mask |= porta_mask;
            /* 配置硬件 */
            _config_interrupt(PORTA, porta_mask, trigger);
        }

        /* Port B 处理 */
        if (portb_mask) {
            int empty_idx;
            int slot_idx = _find_slot(s_portb_slots, callback, &empty_idx);

            if (slot_idx < 0) {
                if (empty_idx < 0) {
                    /* 回滚 Port A */
                    if (porta_mask) {
                        _remove_pin_from_all_slots(s_porta_slots, porta_mask);
                        _disable_interrupt(PORTA, porta_mask, s_porta_slots);
                    }
                    SYS_RecoverIrq(irq_state);
                    return false;
                }
                slot_idx = empty_idx;
                s_portb_slots[slot_idx].callback = callback;
                s_portb_slots[slot_idx].trigger = trigger;
            }

            /* 重叠检测：先移除 */
            _remove_pin_from_all_slots(s_portb_slots, portb_mask);
            /* 添加 pin 到槽 */
            s_portb_slots[slot_idx].pin_mask |= portb_mask;
            /* 配置硬件 */
            _config_interrupt(PORTB, portb_mask, trigger);
        }
    }
    SYS_RecoverIrq(irq_state);

    return true;
}

/**
 * @brief 批量禁用 GPIO 中断
 *
 * @param pins 引脚编号数组
 * @param count 引脚数量
 */
void gpio_disable_interrupt_batch(const pin_t *pins, uint8_t count) {
    if (count == 0 || pins == NULL) {
        return;
    }

    /* 按 port 分组，计算掩码 */
    uint32_t porta_mask = 0;
    uint32_t portb_mask = 0;

    for (uint8_t i = 0; i < count; i++) {
        pin_t pin = pins[i];
        uint8_t port = GET_GPIO_PORT(pin);
        uint8_t pin_num = GET_GPIO_PIN(pin);

        if (port == PORTA) {
            porta_mask |= (1U << pin_num);
        } else if (port == PORTB) {
            portb_mask |= (1U << pin_num);
        }
    }

    uint32_t irq_state;
    SYS_DisableAllIrq(&irq_state);
    {
        if (porta_mask) {
            _remove_pin_from_all_slots(s_porta_slots, porta_mask);
            _disable_interrupt(PORTA, porta_mask, s_porta_slots);
        }
        if (portb_mask) {
            _remove_pin_from_all_slots(s_portb_slots, portb_mask);
            _disable_interrupt(PORTB, portb_mask, s_portb_slots);
        }
    }
    SYS_RecoverIrq(irq_state);
}

/*********************************************************************
 * GPIO 中断服务程序
 *********************************************************************/

__INTERRUPT
__HIGH_CODE
void GPIOA_IRQHandler(void) {
    _gpio_irq_dispatch(s_porta_slots, PORTA);
}

__INTERRUPT
__HIGH_CODE
void GPIOB_IRQHandler(void) {
    _gpio_irq_dispatch(s_portb_slots, PORTB);
}
