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

#include "uart.h"
#include "CH58x_common.h"
#include "pin_mapper.h"
#include <stdbool.h>
#include <stdint.h>
#include "debug.h"
/* CH584 UART标准外设库头文件 */
#include "CH58x_uart.h"

/* UART通道状态结构体 */
typedef struct {
    pin_t rx_pin;       // RX引脚
    pin_t tx_pin;       // TX引脚
    bool rx_enabled;    // RX使能标志
    bool tx_enabled;    // TX使能标志
    bool initialized;   // 初始化标志
} uart_channel_state_t;

/* UART通道状态数组（支持4个UART通道） */
static uart_channel_state_t uart_channels[4] = {0};

/* UART接收回调函数数组 */
static uart_rx_callback_t uart_rx_callbacks[4] = {NULL};

/**
 * @brief 获取UART通道对应的CH584 UART编号
 *
 * @param uart UART通道号
 * @return CH584 UART编号 (0-3)
 */
static uint8_t get_ch584_uart_num(platform_uart_t uart) {
    return (uint8_t)uart;
}

/**
 * @brief 配置UART引脚
 *
 * @param uart UART通道号
 * @param rx_pin RX引脚编号
 * @param tx_pin TX引脚编号
 */
static void configure_uart_pins(platform_uart_t uart, pin_t rx_pin, pin_t tx_pin) {
    uint8_t uart_num = get_ch584_uart_num(uart);

    /* 根据UART编号配置默认引脚映射 */
    switch (uart_num) {
        case 0: /* UART0 */
            /* UART0支持两组引脚:
             * 默认: RXD0/TXD0 on PB[4]/PB[7] (需要关闭映射)
             * 备选: RXD0_/TXD0_ on PA[15]/PA[14] (需要开启映射)
             */
            if (tx_pin != NO_PIN) {
                /* 检查是否为PA14 (TXD0_) */
                if (GET_GPIO_PORT(tx_pin) == PORTA && GET_GPIO_PIN(tx_pin) == 14) {
                    GPIOPinRemap(ENABLE, RB_PIN_UART0);
                    gpio_write_pin_high(tx_pin);
                    /* 配置PA14为推挽输出 */
                    gpio_set_pin_output_push_pull(tx_pin);
                } else if (GET_GPIO_PORT(tx_pin) == PORTB && GET_GPIO_PIN(tx_pin) == 7) {
                    /* PB7是默认TXD0，无需映射 */
                    GPIOPinRemap(DISABLE, RB_PIN_UART0);
                    gpio_write_pin_high(tx_pin);
                    gpio_set_pin_output_push_pull(tx_pin);
                }
            }

            if (rx_pin != NO_PIN) {
                /* 检查是否为PA15 (RXD0_) */
                if (GET_GPIO_PORT(rx_pin) == PORTA && GET_GPIO_PIN(rx_pin) == 15) {
                    /* RXD0_已在上面开启映射时配置 */
                    gpio_set_pin_input_high(rx_pin);
                } else if (GET_GPIO_PORT(rx_pin) == PORTB && GET_GPIO_PIN(rx_pin) == 4) {
                    /* PB4是默认RXD0，无需映射 */
                    gpio_set_pin_input_high(rx_pin);
                }
            }
            break;

        case 1: /* UART1 */
            /* UART1支持两组引脚:
             * 默认: RXD1/TXD1 on PA[8]/PA[9] (需要关闭映射)
             * 备选: RXD1_/TXD1_ on PB[12]/PB[13] (需要开启映射)
             */
            if (tx_pin != NO_PIN) {
                /* 检查是否为PB13 (TXD1_) */
                if (GET_GPIO_PORT(tx_pin) == PORTB && GET_GPIO_PIN(tx_pin) == 13) {
                    GPIOPinRemap(ENABLE, RB_PIN_UART1);
                    gpio_write_pin_high(tx_pin);
                    gpio_set_pin_output_push_pull(tx_pin);
                } else if (GET_GPIO_PORT(tx_pin) == PORTA && GET_GPIO_PIN(tx_pin) == 9) {
                    /* PA9是默认TXD1，无需映射 */
                    GPIOPinRemap(DISABLE, RB_PIN_UART1);
                    gpio_write_pin_high(tx_pin);
                    gpio_set_pin_output_push_pull(tx_pin);
                }
            }

            if (rx_pin != NO_PIN) {
                /* 检查是否为PB12 (RXD1_) */
                if (GET_GPIO_PORT(rx_pin) == PORTB && GET_GPIO_PIN(rx_pin) == 12) {
                    /* RXD1_已在上面开启映射时配置 */
                    gpio_set_pin_input_high(rx_pin);
                } else if (GET_GPIO_PORT(rx_pin) == PORTA && GET_GPIO_PIN(rx_pin) == 8) {
                    /* PA8是默认RXD1，无需映射 */
                    gpio_set_pin_input_high(rx_pin);
                }
            }
            break;

        case 2: /* UART2 */
            /* UART2支持两组引脚:
             * 默认: RXD2/TXD2 on PA[6]/PA[7] (需要关闭映射)
             * 备选: RXD2_/TXD2_ on PB[22]/PB[23] (需要开启映射)
             */
            if (tx_pin != NO_PIN) {
                /* 检查是否为PB23 (TXD2_) */
                if (GET_GPIO_PORT(tx_pin) == PORTB && GET_GPIO_PIN(tx_pin) == 23) {
                    GPIOPinRemap(ENABLE, RB_PIN_UART2);
                    gpio_write_pin_high(tx_pin);
                    gpio_set_pin_output_push_pull(tx_pin);
                } else if (GET_GPIO_PORT(tx_pin) == PORTA && GET_GPIO_PIN(tx_pin) == 7) {
                    /* PA7是默认TXD2，无需映射 */
                    GPIOPinRemap(DISABLE, RB_PIN_UART2);
                    gpio_write_pin_high(tx_pin);
                    gpio_set_pin_output_push_pull(tx_pin);
                }
            }

            if (rx_pin != NO_PIN) {
                /* 检查是否为PB22 (RXD2_) */
                if (GET_GPIO_PORT(rx_pin) == PORTB && GET_GPIO_PIN(rx_pin) == 22) {
                    /* RXD2_已在上面开启映射时配置 */
                    gpio_set_pin_input_high(rx_pin);
                } else if (GET_GPIO_PORT(rx_pin) == PORTA && GET_GPIO_PIN(rx_pin) == 6) {
                    /* PA6是默认RXD2，无需映射 */
                    gpio_set_pin_input_high(rx_pin);
                }
            }
            break;

        case 3: /* UART3 */
            /* UART3支持两组引脚:
             * 默认: RXD3/TXD3 on PA[4]/PA[5] (需要关闭映射)
             * 备选: RXD3_/TXD3_ on PB[20]/PB[21] (需要开启映射)
             */
            if (tx_pin != NO_PIN) {
                /* 检查是否为PB21 (TXD3_) */
                if (GET_GPIO_PORT(tx_pin) == PORTB && GET_GPIO_PIN(tx_pin) == 21) {
                    GPIOPinRemap(ENABLE, RB_PIN_UART3);
                    gpio_write_pin_high(tx_pin);
                    gpio_set_pin_output_push_pull(tx_pin);
                } else if (GET_GPIO_PORT(tx_pin) == PORTA && GET_GPIO_PIN(tx_pin) == 5) {
                    /* PA5是默认TXD3，无需映射 */
                    GPIOPinRemap(DISABLE, RB_PIN_UART3);
                    gpio_write_pin_high(tx_pin);
                    gpio_set_pin_output_push_pull(tx_pin);
                }
            }

            if (rx_pin != NO_PIN) {
                /* 检查是否为PB20 (RXD3_) */
                if (GET_GPIO_PORT(rx_pin) == PORTB && GET_GPIO_PIN(rx_pin) == 20) {
                    /* RXD3_已在上面开启映射时配置 */
                    gpio_set_pin_input_high(rx_pin);
                } else if (GET_GPIO_PORT(rx_pin) == PORTA && GET_GPIO_PIN(rx_pin) == 4) {
                    /* PA4是默认RXD3，无需映射 */
                    gpio_set_pin_input_high(rx_pin);
                }
            }
            break;

        default:
            break;
    }
}

/**
 * @brief 平台UART初始化
 *
 * @param uart UART通道号
 * @param baudrate 波特率
 * @param enable_interrupt 是否启用中断
 * @return error_code_t 错误码
 */
error_code_t platform_uart_init(platform_uart_t uart, uint32_t baudrate, uint8_t enable_interrupt) {
    /* 检查UART通道是否有效 */
    if (uart >= 4) {
        return ERROR_UART_INVALID_PORT;
    }

    /* 检查是否已绑定引脚 */
    if (!uart_channels[uart].initialized) {
        return ERROR_UART_INVALID_PORT;
    }

    uint8_t uart_num = get_ch584_uart_num(uart);

    /* 配置波特率 */
    switch (uart_num) {
        case 0:
            UART0_DefInit( );
            UART0_BaudRateCfg(baudrate);
            break;
        case 1:
            UART1_DefInit();
            UART1_BaudRateCfg(baudrate);
            break;
        case 2:
            UART2_DefInit( );
            UART2_BaudRateCfg(baudrate);
            break;
        case 3:
            UART3_DefInit( );
            UART3_BaudRateCfg(baudrate);
            break;
        default:
            return ERROR_UART_INVALID_PORT;
    }

    /* 配置FIFO和中断 */
    // uint8_t fcr_value = (2 << 6) | RB_FCR_FIFO_EN;  /* 触发点4字节 */

    // /* 根据RX/TX使能情况设置FIFO */
    // if (uart_channels[uart].rx_enabled && uart_channels[uart].tx_enabled) {
    //     fcr_value |= RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR;
    // } else if (uart_channels[uart].tx_enabled) {
    //     fcr_value |= RB_FCR_TX_FIFO_CLR;
    // } else if (uart_channels[uart].rx_enabled) {
    //     fcr_value |= RB_FCR_RX_FIFO_CLR;
    // }

    // /* 配置UART寄存器（使用默认的8N1模式） */
    // switch (uart_num) {
    //     case 0:
    //         R8_UART0_FCR = fcr_value;
    //         R8_UART0_LCR = RB_LCR_WORD_SZ;
    //         R8_UART0_DIV = 1;

    //         /* 配置中断 */
    //         if (enable_interrupt && uart_channels[uart].rx_enabled) {
    //             UART0_ByteTrigCfg(UART_1BYTE_TRIG);
    //             UART0_INTCfg(ENABLE, RB_IER_RECV_RDY | RB_IER_LINE_STAT);
    //             PFIC_EnableIRQ(UART0_IRQn);
    //         }
    //         break;

    //     case 1:
    //         R8_UART1_FCR = fcr_value;
    //         R8_UART1_LCR = RB_LCR_WORD_SZ;
    //         R8_UART1_DIV = 1;

    //         /* 配置中断 */
    //         if (enable_interrupt && uart_channels[uart].rx_enabled) {
    //             UART1_ByteTrigCfg(UART_1BYTE_TRIG);
    //             UART1_INTCfg(ENABLE, RB_IER_RECV_RDY | RB_IER_LINE_STAT);
    //             PFIC_EnableIRQ(UART1_IRQn);
    //         }
    //         break;

    //     case 2:
    //         R8_UART2_FCR = fcr_value;
    //         R8_UART2_LCR = RB_LCR_WORD_SZ;
    //         R8_UART2_DIV = 1;

    //         /* 配置中断 */
    //         if (enable_interrupt && uart_channels[uart].rx_enabled) {
    //             UART2_ByteTrigCfg(UART_1BYTE_TRIG);
    //             UART2_INTCfg(ENABLE, RB_IER_RECV_RDY | RB_IER_LINE_STAT);
    //             PFIC_EnableIRQ(UART2_IRQn);
    //         }
    //         break;

    //     case 3:
    //         R8_UART3_FCR = fcr_value;
    //         R8_UART3_LCR = RB_LCR_WORD_SZ;
    //         R8_UART3_DIV = 1;

    //         /* 配置中断 */
    //         if (enable_interrupt && uart_channels[uart].rx_enabled) {
    //             UART3_ByteTrigCfg(UART_1BYTE_TRIG);
    //             UART3_INTCfg(ENABLE, RB_IER_RECV_RDY | RB_IER_LINE_STAT);
    //             PFIC_EnableIRQ(UART3_IRQn);
    //         }
    //         break;

    //     default:
    //         return ERROR_UART_INVALID_PORT;
    // }

    return NO_ERROR;
}

/**
 * @brief 关闭UART
 *
 * @param uart UART通道号
 * @return error_code_t 错误码
 */
error_code_t platform_uart_close(platform_uart_t uart) {
    /* 检查UART通道是否有效 */
    if (uart >= PLATFORM_UART_END) {
        return ERROR_UART_INVALID_PORT;
    }

    uint8_t uart_num = get_ch584_uart_num(uart);

    /* 复位UART */
    switch (uart_num) {
        case 0:
            UART0_Reset();
            break;
        case 1:
            UART1_Reset();
            break;
        case 2:
            UART2_Reset();
            break;
        case 3:
            UART3_Reset();
            break;
        default:
            return ERROR_UART_INVALID_PORT;
    }

    return NO_ERROR;
}

/**
 * @brief UART发送一个字节
 *
 * @param uart UART通道号
 * @param data 要发送的数据
 * @return error_code_t 错误码
 */
error_code_t platform_uart_write_byte(platform_uart_t uart, uint8_t data) {
    /* 检查UART通道是否有效 */
    if (uart >= PLATFORM_UART_END) {
        return ERROR_UART_INVALID_PORT;
    }

    /* 检查TX是否使能 */
    if (!uart_channels[uart].tx_enabled) {
        return ERROR_UART_INVALID_PORT;
    }

    uint8_t uart_num = get_ch584_uart_num(uart);

    /* 发送数据 */
    switch (uart_num) {
        case 0:
            UART0_SendByte(data);
            break;
        case 1:
            UART1_SendByte(data);
            break;
        case 2:
            UART2_SendByte(data);
            break;
        case 3:
            UART3_SendByte(data);
            break;
        default:
            return ERROR_UART_INVALID_PORT;
    }

    return NO_ERROR;
}

/**
 * @brief UART发送数据
 *
 * @param uart UART通道号
 * @param data 要发送的数据缓冲区
 * @param len 要发送的数据长度
 * @return error_code_t 错误码
 */
error_code_t platform_uart_write(platform_uart_t uart, uint8_t *data, uint16_t len) {
    /* 检查UART通道是否有效 */
    if (uart >= PLATFORM_UART_END) {
        return ERROR_UART_INVALID_PORT;
    }

    /* 检查TX是否使能 */
    if (!uart_channels[uart].tx_enabled) {
        return ERROR_UART_INVALID_PORT;
    }

    uint8_t uart_num = get_ch584_uart_num(uart);

    /* 发送数据 */
    switch (uart_num) {
        case 0:
            UART0_SendString(data, len);
            break;
        case 1:
            UART1_SendString(data, len);
            break;
        case 2:
            UART2_SendString(data, len);
            break;
        case 3:
            UART3_SendString(data, len);
            break;
        default:
            return ERROR_UART_INVALID_PORT;
    }

    return NO_ERROR;
}

/**
 * @brief 注册UART接收回调函数
 *
 * @param uart UART通道号
 * @param callback 回调函数指针
 * @return error_code_t 错误码
 */
error_code_t platform_uart_register_rx_callback(platform_uart_t uart, uart_rx_callback_t callback) {
    /* 检查UART通道是否有效 */
    if (uart >= PLATFORM_UART_END) {
        return ERROR_UART_INVALID_PORT;
    }

    /* 检查RX是否使能 */
    if (!uart_channels[uart].rx_enabled) {
        return ERROR_UART_INVALID_PORT;
    }

    /* 注册回调函数 */
    uart_rx_callbacks[uart] = callback;

    return NO_ERROR;
}

/**
 * @brief 绑定GPIO引脚到UART信道
 *
 * @param rx_pin RX引脚编号（如果为NO_PIN则禁用接收）
 * @param tx_pin TX引脚编号（如果为NO_PIN则禁用发送）
 * @param uart UART通道号
 * @return error_code_t 错误码
 */
error_code_t platform_uart_bind_pins(pin_t rx_pin, pin_t tx_pin, platform_uart_t uart) {
    /* 检查UART通道是否有效 */
    if (uart >= PLATFORM_UART_END) {
        return ERROR_UART_INVALID_PORT;
    }

    /* 保存引脚信息 */
    uart_channels[uart].rx_pin = rx_pin;
    uart_channels[uart].tx_pin = tx_pin;

    /* 检查引脚是否为NO_PIN */
    uart_channels[uart].rx_enabled = (rx_pin != NO_PIN);
    uart_channels[uart].tx_enabled = (tx_pin != NO_PIN);

    /* 如果至少有一个有效引脚，则认为已绑定 */
    uart_channels[uart].initialized = (uart_channels[uart].rx_enabled || uart_channels[uart].tx_enabled);

    /* 配置GPIO引脚 */
    if (uart_channels[uart].initialized) {
        configure_uart_pins(uart, rx_pin, tx_pin);
    }

    return NO_ERROR;
}

/**
 * @brief 获取UART信道绑定的RX引脚
 *
 * @param uart UART通道号
 * @return RX引脚号（如果未绑定则返回NO_PIN）
 */
pin_t platform_uart_get_rx_pin(platform_uart_t uart) {
    if (uart >= PLATFORM_UART_END) {
        return NO_PIN;
    }

    return uart_channels[uart].rx_pin;
}

/**
 * @brief 获取UART信道绑定的TX引脚
 *
 * @param uart UART通道号
 * @return TX引脚号（如果未绑定则返回NO_PIN）
 */
pin_t platform_uart_get_tx_pin(platform_uart_t uart) {
    if (uart >= PLATFORM_UART_END) {
        return NO_PIN;
    }

    return uart_channels[uart].tx_pin;
}

/**
 * @brief 检查UART信道是否已绑定GPIO引脚
 *
 * @param uart UART通道号
 * @return true表示已绑定（至少有一个有效引脚），false表示未绑定
 */
bool platform_uart_is_bound(platform_uart_t uart) {
    if (uart >= PLATFORM_UART_END) {
        return false;
    }

    return uart_channels[uart].initialized;
}

/**
 * @brief 检查UART信道是否启用了RX功能
 *
 * @param uart UART通道号
 * @return true表示已启用RX，false表示未启用
 */
bool platform_uart_is_rx_enabled(platform_uart_t uart) {
    if (uart >= PLATFORM_UART_END) {
        return false;
    }

    return uart_channels[uart].rx_enabled;
}

/**
 * @brief 检查UART信道是否启用了TX功能
 *
 * @param uart UART通道号
 * @return true表示已启用TX，false表示未启用
 */
bool platform_uart_is_tx_enabled(platform_uart_t uart) {
    if (uart >= PLATFORM_UART_END) {
        return false;
    }

    return uart_channels[uart].tx_enabled;
}

/*==========================================
 * 中断处理函数
 *=========================================*/
//
///**
// * @brief UART0中断处理函数
// */
//__INTERRUPT __HIGH_CODE void UART0_IRQHandler(void) {
//    uint8_t tmp;
//
//    /* 获取中断标志 */
//    switch (UART0_GetITFlag()) {
//        case UART_II_LINE_STAT:  /* 线路状态错误 */
//            UART0_GetLinSTA();
//            break;
//
//        case UART_II_RECV_RDY:   /* 数据达到触发点 */
//            if (uart_rx_callbacks[0] != NULL && uart_channels[0].rx_enabled) {
//                tmp = UART0_RecvByte();
//                uart_rx_callbacks[0](&tmp, 1);
//            }
//            break;
//
//        case UART_II_RECV_TOUT:  /* 接收超时（暂时未使用） */
//            break;
//
//        case UART_II_THR_EMPTY:  /* 发送缓冲区空（暂时未使用） */
//            break;
//
//        case UART_II_MODEM_CHG:  /* 调制解调器状态变化（仅UART0支持，暂时未使用） */
//            break;
//
//        default:
//            break;
//    }
//}
//
///**
// * @brief UART1中断处理函数
// */
//__INTERRUPT __HIGH_CODE void UART1_IRQHandler(void) {
//    uint8_t tmp;
//
//    /* 获取中断标志 */
//    switch (UART1_GetITFlag()) {
//        case UART_II_LINE_STAT:  /* 线路状态错误 */
//            UART1_GetLinSTA();
//            break;
//
//        case UART_II_RECV_RDY:   /* 数据达到触发点 */
//            if (uart_rx_callbacks[1] != NULL && uart_channels[1].rx_enabled) {
//                tmp = UART1_RecvByte();
//                uart_rx_callbacks[1](&tmp, 1);
//            }
//            break;
//
//        case UART_II_RECV_TOUT:  /* 接收超时（暂时未使用） */
//            break;
//
//        case UART_II_THR_EMPTY:  /* 发送缓冲区空（暂时未使用） */
//            break;
//
//        default:
//            break;
//    }
//}
//
///**
// * @brief UART2中断处理函数
// */
//__INTERRUPT __HIGH_CODE void UART2_IRQHandler(void) {
//    uint8_t tmp;
//
//    /* 获取中断标志 */
//    switch (UART2_GetITFlag()) {
//        case UART_II_LINE_STAT:  /* 线路状态错误 */
//            UART2_GetLinSTA();
//            break;
//
//        case UART_II_RECV_RDY:   /* 数据达到触发点 */
//            if (uart_rx_callbacks[2] != NULL && uart_channels[2].rx_enabled) {
//                tmp = UART2_RecvByte();
//                uart_rx_callbacks[2](&tmp, 1);
//            }
//            break;
//
//        case UART_II_RECV_TOUT:  /* 接收超时（暂时未使用） */
//            break;
//
//        case UART_II_THR_EMPTY:  /* 发送缓冲区空（暂时未使用） */
//            break;
//
//        default:
//            break;
//    }
//}
//
///**
// * @brief UART3中断处理函数
// */
//__INTERRUPT __HIGH_CODE void UART3_IRQHandler(void) {
//    uint8_t tmp;
//
//    /* 获取中断标志 */
//    switch (UART3_GetITFlag()) {
//        case UART_II_LINE_STAT:  /* 线路状态错误 */
//            UART3_GetLinSTA();
//            break;
//
//        case UART_II_RECV_RDY:   /* 数据达到触发点 */
//            if (uart_rx_callbacks[3] != NULL && uart_channels[3].rx_enabled) {
//                tmp = UART3_RecvByte();
//                uart_rx_callbacks[3](&tmp, 1);
//            }
//            break;
//
//        case UART_II_RECV_TOUT:  /* 接收超时（暂时未使用） */
//            break;
//
//        case UART_II_THR_EMPTY:  /* 发送缓冲区空（暂时未使用） */
//            break;
//
//        default:
//            break;
//    }
//}
