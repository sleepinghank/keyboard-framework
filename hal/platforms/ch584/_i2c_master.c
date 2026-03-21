/********************************** (C) COPYRIGHT *******************************
 * File Name          : _i2c_master.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2022/11/04
 * Description        : CH584 I2C Master HAL Implementation
 *                    : 完全参照官方例程app_i2c.c实现
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "i2c_master.h"
#include "CH58x_common.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* CH584 I2C标准外设库头文件 */
#include "CH58x_i2c.h"
#include "CH58x_gpio.h"

/**
 * Note: 主机与从机的DEBUG接口需要同时打开或关闭，
 * 否则会产生时序问题。
 */
// #define CONFIG_I2C_DEBUG

#ifdef CONFIG_I2C_DEBUG
#define I2C_DBG(...)    PRINT(__VA_ARGS__)
#else
#define I2C_DBG(...)
#endif

/* I2C缓冲区长度（完全参照官方例程app_i2c.c） */
#define I2C_BUFFER_LENGTH   32

/* I2C状态定义（完全参照官方例程app_i2c.h） */
typedef enum {
    I2C_READY = 0,
    I2C_MRX,     /* Master Receive */
    I2C_MTX,     /* Master Transmit */
    I2C_SRX,     /* Slave Receive */
    I2C_STX      /* Slave Transmit */
} i2c_state_t;

/* I2C错误码定义（完全参照官方例程app_i2c.h） */
typedef enum {
    I2C_NO_MEM = 1,
    I2C_STATE,
    I2C_MT_NACK,
    I2C_ARB_LOST,
    I2C_BUS_ERROR,
    I2C_OVR,
    I2C_PECERR,
    I2C_TIMEOUT,
    I2C_SMBALERT,
} i2c_error_t;

/* I2C通道状态结构体（完全参照官方例程） */
typedef struct {
    pin_t sda_pin;           /* SDA引脚 */
    pin_t scl_pin;           /* SCL引脚 */
    bool initialized;        /* 初始化标志 */
    bool bound;              /* 引脚绑定标志 */
    uint8_t own_address;     /* 自身地址 */
    uint32_t clock_speed;    /* 时钟速度 */

    /* 官方例程中的变量（完全一致） */
    volatile uint8_t state;          /* I2C状态 */
    volatile uint8_t slave_addr_rw;  /* 从机地址+读写位 */
    volatile uint8_t send_stop;      /* 是否发送STOP */
    volatile uint8_t in_repstart;    /* 重复START标志 */
    volatile uint8_t error;          /* 错误码 */
    uint8_t master_buffer[I2C_BUFFER_LENGTH];  /* 主设备缓冲区 */
    volatile uint8_t master_buffer_index;      /* 缓冲区索引 */
    uint8_t master_buffer_length;    /* 缓冲区长度 */

    /* 接收模式专用变量（参照官方例程） */
    uint8_t is_nack_sent;     /* NACK发送标志 */
} i2c_channel_state_t;

/* I2C通道状态数组 - CH584有1个I2C控制器 */
static i2c_channel_state_t i2c_channels[I2C_CHANNEL_MAX] = {0};

/* ---------- I2C Slave：与官方完整示例一致的缓冲区与回调 ---------- */
static uint8_t i2c_slave_txbuffer[I2C_BUFFER_LENGTH];
static uint8_t i2c_slave_txbuffer_index = 0;
static uint8_t i2c_slave_txbuffer_length = 0;

static uint8_t i2c_slave_rxbuffer[I2C_BUFFER_LENGTH];
static uint8_t i2c_slave_rxbuffer_index = 0;

static const i2c_slave_callbacks_t *i2c_slave_cb = NULL;

void i2c_slave_set_callbacks(const i2c_slave_callbacks_t *cb)
{
    i2c_slave_cb = cb;
}

/*==========================================
 * 内部辅助函数（完全参照官方例程）
 *=========================================*/

/**
 * @brief 配置I2C引脚（完全参照app_i2c.c）将绑定到通道的引脚（i2c_bind_pins）配置gpio模式
 * @param channel I2C通道号
 */
static void i2c_configure_pins(i2c_channel_t channel) {
    i2c_channel_state_t *ch = &i2c_channels[channel];

    if (ch->sda_pin != NO_PIN && ch->scl_pin != NO_PIN) {
        uint8_t sda_port = GET_GPIO_PORT(ch->sda_pin);
        uint8_t sda_pin_num = GET_GPIO_PIN(ch->sda_pin);
        uint8_t scl_port = GET_GPIO_PORT(ch->scl_pin);
        uint8_t scl_pin_num = GET_GPIO_PIN(ch->scl_pin);

        uint32_t sda_mask = (sda_pin_num < 32) ? (1 << sda_pin_num) : 0;
        uint32_t scl_mask = (scl_pin_num < 32) ? (1 << scl_pin_num) : 0;

        if (sda_mask != 0 && scl_mask != 0) {
            uint32_t combined_mask = sda_mask | scl_mask;

            if (sda_port == 0 && scl_port == 0) {
                /* 都在PORTA */
                GPIOA_ModeCfg(combined_mask, GPIO_ModeIN_PU);
            } else if (sda_port == 1 && scl_port == 1) {
                /* 都在PORTB */
                GPIOB_ModeCfg(combined_mask, GPIO_ModeIN_PU);
            } else {
                /* 分别配置 */
                if (sda_port == 0) GPIOA_ModeCfg(sda_mask, GPIO_ModeIN_PU);
                if (sda_port == 1) GPIOB_ModeCfg(sda_mask, GPIO_ModeIN_PU);
                if (scl_port == 0) GPIOA_ModeCfg(scl_mask, GPIO_ModeIN_PU);
                if (scl_port == 1) GPIOB_ModeCfg(scl_mask, GPIO_ModeIN_PU);
            }
        }
    }
}

#ifdef CONFIG_I2C_DEBUG
static void print_i2c_irq_sta(uint32_t state)
{
    I2C_DBG("i2c irq: ( ");

    if(state & RB_I2C_SB)
        I2C_DBG("SB ");
    if(state & RB_I2C_ADDR)
        I2C_DBG("ADDR ");
    if(state & RB_I2C_BTF)
        I2C_DBG("BTF ");
    if(state & RB_I2C_ADD10)
        I2C_DBG("ADD10 ");
    if(state & RB_I2C_STOPF)
        I2C_DBG("STOP ");
    if(state & RB_I2C_RxNE)
        I2C_DBG("RxNE ");
    if(state & RB_I2C_TxE)
        I2C_DBG("TxE ");
    if(state & RB_I2C_BERR)
        I2C_DBG("BERR ");
    if(state & RB_I2C_ARLO)
        I2C_DBG("ARLO ");
    if(state & RB_I2C_AF)
        I2C_DBG("AF ");
    if(state & RB_I2C_OVR)
        I2C_DBG("OVR ");
    if(state & RB_I2C_PECERR)
        I2C_DBG("PECERR ");
    if(state & RB_I2C_TIMEOUT)
        I2C_DBG("TIMEOUT ");
    if(state & RB_I2C_SMBALERT)
        I2C_DBG("SMBALERT ");
    if(state & (RB_I2C_MSL << 16))
        I2C_DBG("MSL ");
    if(state & (RB_I2C_BUSY << 16))
        I2C_DBG("BUSY ");
    if(state & (RB_I2C_TRA << 16))
        I2C_DBG("TRA ");
    if(state & (RB_I2C_GENCALL << 16))
        I2C_DBG("GENCALL ");
    if(state & (RB_I2C_SMBDEFAULT << 16))
        I2C_DBG("SMBDEFAULT ");
    if(state & (RB_I2C_SMBHOST << 16))
        I2C_DBG("SMBHOST ");
    if(state & (RB_I2C_DUALF << 16))
        I2C_DBG("DUALF ");

    I2C_DBG(")\n");
}
#endif

/**
 * @brief I2C中断处理函数（完全参照官方app_i2c.c I2C_IRQHandler）
 * @note 这是官方例程中的标准中断处理函数
 */
__INTERRUPT
__HIGH_CODE
void I2C_IRQHandler(void) {
    uint32_t event = I2C_GetLastEvent();
    #ifdef CONFIG_I2C_DEBUG
    print_i2c_irq_sta(event);
    #endif
    /* I2C Master */
    if (event & (RB_I2C_MSL << 16)) {
        if (event & RB_I2C_SB) {
            /* Start condition sent, send address */
            /* 注意：在中断模式下，我们需要从通道0获取状态 */
            i2c_channel_state_t *ch = &i2c_channels[0];
            I2C_SendData(ch->slave_addr_rw);
            I2C_DBG("Master selected, send address\n");
        }

        /* I2C Master transmitter */
        if (event & (RB_I2C_TRA << 16)) {
            i2c_channel_state_t *ch = &i2c_channels[0];
            I2C_DBG("Master transmitter:\n");
            /* Slave receiver acked address or sent bit */
            if (event & (RB_I2C_ADDR | RB_I2C_BTF | RB_I2C_TxE | (RB_I2C_TRA << 16))) {
                /* if there is data to send, send it, otherwise stop */
                if (ch->master_buffer_index < ch->master_buffer_length) {
                    I2C_SendData(ch->master_buffer[ch->master_buffer_index++]);
                    I2C_DBG("  send (%#x)\n",
                            ch->master_buffer[ch->master_buffer_index - 1]);
                } else {
                    if (ch->send_stop) {
                        ch->state = I2C_READY;
                        I2C_GenerateSTOP(ENABLE);
                        I2C_DBG("  send STOP\n");
                    } else {
                        ch->in_repstart = 1;
                        /* we're gonna send the START, don't enable the interrupt. */
                        I2C_ITConfig(I2C_IT_BUF, DISABLE);
                        I2C_ITConfig(I2C_IT_EVT, DISABLE);
                        I2C_ITConfig(I2C_IT_ERR, DISABLE);
                        I2C_GenerateSTART(ENABLE);
                        ch->state = I2C_READY;
                        I2C_DBG("  restart\n");
                    }
                }
            }

            /* Address or data sent, nack received */
            if (event & RB_I2C_AF) {
                i2c_channel_state_t *ch = &i2c_channels[0];
                I2C_ClearFlag(I2C_FLAG_AF);

                ch->error = I2C_MT_NACK;
                ch->state = I2C_READY;
                I2C_GenerateSTOP(ENABLE);
                I2C_DBG("  NACK received, sent stop\n");
            }
        } else {
        /* I2C Master reveiver */
            i2c_channel_state_t *ch = &i2c_channels[0];
            I2C_DBG("Master receiver:\n");

            /* address sent, ack received */
            if(event & RB_I2C_ADDR) {
                /* ack if more bytes are expected, otherwise nack */
                if (ch->master_buffer_length) {
                    I2C_AcknowledgeConfig(ENABLE);
                    I2C_DBG("  address sent\n");
                    I2C_DBG("  ACK next\n");
                } else {
                    //XXX: Should not delay too match before NACK
                    I2C_AcknowledgeConfig(DISABLE);
                    ch->is_nack_sent = true;
                    I2C_DBG("  address sent\n");
                    I2C_DBG("  NACK next\n");
                }
            }

            /* data reveived */
            if (event & (RB_I2C_RxNE)) {
                /* put byte into buffer */
                ch->master_buffer[ch->master_buffer_index++] = I2C_ReceiveData();

                if (ch->master_buffer_index < ch->master_buffer_length) {
                    I2C_AcknowledgeConfig(ENABLE);
                    I2C_DBG("  ACK next\n");
                } else {
                    //XXX: Should not delay too match before NACK
                    I2C_AcknowledgeConfig(DISABLE);
                    I2C_DBG("  NACK next\n");

                    if (ch->is_nack_sent) {
                        ch->is_nack_sent = false;
                        if (ch->send_stop) {
                            I2C_GenerateSTOP(ENABLE);
                            ch->state = I2C_READY;
                            I2C_DBG("  send STOP\n");
                        } else {
                            ch->in_repstart = 1;
                            /* we're gonna send the START, don't enable the interrupt. */
                            I2C_ITConfig(I2C_IT_BUF, DISABLE);
                            I2C_ITConfig(I2C_IT_EVT, DISABLE);
                            I2C_ITConfig(I2C_IT_ERR, DISABLE);
                            I2C_GenerateSTART(ENABLE);
                            ch->state = I2C_READY;
                            I2C_DBG("  restart\n");

                        }
                    } else {
                        ch->is_nack_sent = true;
                    }
                }

                I2C_DBG("  received data (%#x)\n",
                        ch->master_buffer[ch->master_buffer_index - 1]);

            }

            /* nack received */
            if (event & RB_I2C_AF) {
                i2c_channel_state_t *ch = &i2c_channels[0];
                I2C_ClearFlag(I2C_FLAG_AF);
                /* put final byte into buffer */
                ch->master_buffer[ch->master_buffer_index++] = I2C_ReceiveData();

                if (ch->send_stop) {
                    ch->state = I2C_READY;
                    I2C_GenerateSTOP(ENABLE);
                    I2C_DBG("  NACK received, send STOP\n");
                } else {
                    ch->in_repstart = 1;
                    /* we're gonna send the START, don't enable the interrupt. */
                    I2C_ITConfig(I2C_IT_BUF, DISABLE);
                    I2C_ITConfig(I2C_IT_EVT, DISABLE);
                    I2C_ITConfig(I2C_IT_ERR, DISABLE);
                    I2C_GenerateSTART(ENABLE);
                    ch->state = I2C_READY;
                    I2C_DBG("  restart\n");
                }
            }
        }

    } else {
    /* I2C slave — 与官方完整示例一致 */
        i2c_channel_state_t *ch0 = &i2c_channels[0];

        /* addressed, returned ack */
        if (event & RB_I2C_ADDR) {

            if (event & ((RB_I2C_TRA << 16) | RB_I2C_TxE)) {
                I2C_DBG("Slave transmitter address matched\n");

                ch0->state = I2C_STX;
                i2c_slave_txbuffer_index = 0;
                i2c_slave_txbuffer_length = 0;

                if (i2c_slave_cb && i2c_slave_cb->on_transmit) {
                    i2c_slave_cb->on_transmit(i2c_slave_txbuffer, &i2c_slave_txbuffer_length);
                }
            } else {
                I2C_DBG("Slave reveiver address matched\n");

                ch0->state = I2C_SRX;
                i2c_slave_rxbuffer_index = 0;
            }
        }

        if (event & (RB_I2C_TRA << 16)) { /* TODO: STOP? */
            /* Slave transmintter */
            I2C_AcknowledgeConfig(ENABLE);
            I2C_DBG("Slave transmitter:\n");

            if (event & RB_I2C_AF) {
                /* Nack received */
                I2C_ClearFlag(I2C_FLAG_AF);
                I2C_AcknowledgeConfig(ENABLE);
                I2C_DBG("  Nack received\n");

                /* leave slave receiver state */
                ch0->state = I2C_READY;
                /* clear status */
                event = 0;
            }

            if (event & (RB_I2C_BTF | RB_I2C_TxE)) {
                /* if there is more to send, ack, otherwise send 0xff */
                if (i2c_slave_txbuffer_index < i2c_slave_txbuffer_length) {
                    /* copy data to output register */
                    I2C_SendData(i2c_slave_txbuffer[i2c_slave_txbuffer_index++]);
                    I2C_DBG("  send (%#x)\n",
                            i2c_slave_txbuffer[i2c_slave_txbuffer_index - 1]);
                } else {
                    I2C_SendData(0xff);
                    I2C_DBG("  no more data, send 0xff\n");
                }
            }
        } else {
            /* Slave receiver */
            I2C_DBG("Slave receiver:\n");

            if (event & RB_I2C_RxNE) {
                /* if there is still room in the rx buffer */
                if (i2c_slave_rxbuffer_index < I2C_BUFFER_LENGTH) {
                    /* put byte in buffer and ack */
                    i2c_slave_rxbuffer[i2c_slave_rxbuffer_index++] = I2C_ReceiveData();
                    I2C_AcknowledgeConfig(ENABLE);
                    I2C_DBG("  received (%#x)\n",
                            i2c_slave_rxbuffer[i2c_slave_rxbuffer_index - 1]);
                } else {
                    /* otherwise nack */
                    I2C_AcknowledgeConfig(DISABLE);
                }
            }

            if (event & RB_I2C_STOPF) {
                /* ack future responses and leave slave receiver state */
                R16_I2C_CTRL1 |= RB_I2C_PE; /* clear flag */

                I2C_DBG("  reveive stop\n");

                /* callback to user defined callback */
                if (i2c_slave_cb && i2c_slave_cb->on_receive) {
                    i2c_slave_cb->on_receive(i2c_slave_rxbuffer, i2c_slave_rxbuffer_index);
                }
                /* since we submit rx buffer , we can reset it */
                i2c_slave_rxbuffer_index = 0;
            }

            if (event & RB_I2C_AF) {
                I2C_ClearFlag(I2C_FLAG_AF);

                /* ack future responses */
                I2C_AcknowledgeConfig(ENABLE);
            }
        }
    }

    if (event & RB_I2C_BERR) {
        I2C_ClearFlag(RB_I2C_BERR);
        I2C_GenerateSTOP(ENABLE);

        i2c_channels[0].error = I2C_BUS_ERROR;
        I2C_DBG("RB_I2C_BERR\n");
    }

    if (event & RB_I2C_ARLO) {
        I2C_ClearFlag(RB_I2C_ARLO);

        i2c_channels[0].error = I2C_ARB_LOST;
        I2C_DBG("RB_I2C_ARLO\n");
    }

    if (event & RB_I2C_OVR) {
        I2C_ClearFlag(RB_I2C_OVR);

        i2c_channels[0].error = I2C_OVR;
        I2C_DBG("RB_I2C_OVR\n");
    }

    if (event & RB_I2C_PECERR) {
        I2C_ClearFlag(RB_I2C_PECERR);

        i2c_channels[0].error = I2C_PECERR;
        I2C_DBG("RB_I2C_PECERR\n");
    }

    if (event & RB_I2C_TIMEOUT) {
        I2C_ClearFlag(RB_I2C_TIMEOUT);

        i2c_channels[0].error = I2C_TIMEOUT;
        I2C_DBG("RB_I2C_TIMEOUT\n");
    }

    if (event & RB_I2C_SMBALERT) {
        I2C_ClearFlag(RB_I2C_SMBALERT);

        i2c_channels[0].error = I2C_SMBALERT;
        I2C_DBG("RB_I2C_SMBALERT\n");
    }

    I2C_DBG("\n");
}

/*==========================================
 * I2C通道基础函数（完全参照官方例程）
 * 软实现，软件状态结构体初始化/清零
 *=========================================*/

void soft_i2c_init_channel(i2c_channel_t channel) {
    if (channel >= I2C_CHANNEL_MAX) {
        return;
    }

    i2c_channel_state_t *ch = &i2c_channels[channel];

    /* 初始化通道状态（完全参照app_i2c.c i2c_app_init） */
    ch->state = I2C_READY;
    ch->send_stop = true;
    ch->in_repstart = false;
    ch->error = 0;
    ch->initialized = false;
    ch->bound = false;
    ch->sda_pin = NO_PIN;
    ch->scl_pin = NO_PIN;
    ch->own_address = 0x00;
    ch->clock_speed = 400000;
    ch->master_buffer_index = 0;
    ch->master_buffer_length = 0;
    ch->slave_addr_rw = 0;
    ch->is_nack_sent = false;
}

/**
 * @brief I2C写入操作（完全参照app_i2c.c i2c_write_to）
 * @param channel I2C通道号
 * @param addr_7bit 从设备7位地址
 * @param data 写入数据
 * @param length 数据长度
 * @param wait 是否等待完成
 * @param send_stop 是否发送STOP
 * @return 0=成功, 负值=错误码
 */
static int i2c_write_to_channel(i2c_channel_t channel, uint8_t addr_7bit, const uint8_t *data, uint8_t length,
        uint8_t wait, uint8_t send_stop) {
    i2c_channel_state_t *ch = &i2c_channels[channel];

    if (length > I2C_BUFFER_LENGTH) {
        return -I2C_NO_MEM;
    }

    if (ch->state != I2C_READY) {
        return -I2C_STATE;
    }

    if (!length) {
        return 0;
    }

    ch->state = I2C_MTX;
    ch->send_stop = send_stop;

    ch->error = 0;

    // initialize buffer iteration vars
    ch->master_buffer_index = 0;
    ch->master_buffer_length = length;

    memcpy(ch->master_buffer, data, length);

    ch->slave_addr_rw = I2C_WRITE;
    ch->slave_addr_rw |= addr_7bit << 1;

    I2C_GenerateSTOP(DISABLE);

    if (ch->in_repstart == true) {
        ch->in_repstart = false;

        do {
            I2C_SendData(ch->slave_addr_rw);
        } while(R16_I2C_STAR1 & RB_I2C_BTF);

        /* Disabled in IRS */
        I2C_ITConfig(I2C_IT_BUF, ENABLE);
        I2C_ITConfig(I2C_IT_EVT, ENABLE);
        I2C_ITConfig(I2C_IT_ERR, ENABLE);
    } else {
        I2C_GenerateSTART(ENABLE);
    }

    while(wait && (ch->state == I2C_MTX)) {
        continue;
    }

    if (ch->error) {
        return -ch->error;
    }

    return 0;
}

/**
 * @brief I2C读取操作（完全参照app_i2c.c i2c_read_from）
 * @param channel I2C通道号
 * @param addr_7bit 从设备7位地址
 * @param data 读取数据缓冲区
 * @param length 数据长度
 * @param send_stop 是否发送STOP
 * @param timeout 超时时间
 * @return 读取的字节数，负值=错误码
 */
static int i2c_read_from_channel(i2c_channel_t channel, uint8_t addr_7bit, uint8_t *data, uint8_t length,
        uint8_t send_stop, int timeout) {
    int to = 0;
    uint8_t forever = (timeout == -1);
    i2c_channel_state_t *ch = &i2c_channels[channel];

    if (length > I2C_BUFFER_LENGTH) {
        return -I2C_NO_MEM;
    }

    if (ch->state != I2C_READY) {
        return -I2C_STATE;
    }

    if (!length) {
        return 0;
    }

    ch->state = I2C_MRX;
    ch->send_stop = send_stop;

    ch->error = 0;

    // initialize buffer iteration vars
    ch->master_buffer_index = 0;
    ch->master_buffer_length = length - 1;

    ch->slave_addr_rw = I2C_READ;
    ch->slave_addr_rw |= addr_7bit << 1;

    I2C_GenerateSTOP(DISABLE);

    if (ch->in_repstart == true) {
        ch->in_repstart = false;

        do {
            I2C_SendData(ch->slave_addr_rw);
        } while(R16_I2C_STAR1 & RB_I2C_BTF);

        /* Disabled in IRS */
        I2C_ITConfig(I2C_IT_BUF, ENABLE);
        I2C_ITConfig(I2C_IT_EVT, ENABLE);
        I2C_ITConfig(I2C_IT_ERR, ENABLE);
    } else {
        I2C_GenerateSTART(ENABLE);
    }

    // wait for read operation to complete
    while (ch->state == I2C_MRX) {
        mDelaymS(1);
        to++;
        if (!forever && (to >= timeout)) {
            break;
        }
    }

    if (ch->master_buffer_index < length)
        length = ch->master_buffer_index;

    // copy i2c buffer to data
    memcpy(data, ch->master_buffer, length);

    return length;
}

/*==========================================
 * I2C通道基础函数（适配HAL接口）
 *=========================================*/

i2c_status_t i2c_start_channel(i2c_channel_t channel, uint8_t address, uint16_t timeout) {
    /* 使用官方例程的方式启动I2C传输 */
    int result = i2c_write_to_channel(channel, address >> 1, NULL, 0, 1, 0);

    if (result == 0) {
        return I2C_STATUS_SUCCESS;
    }

    return I2C_STATUS_ERROR;
}

i2c_status_t i2c_write_channel(i2c_channel_t channel, uint8_t data, uint16_t timeout) {
    i2c_channel_state_t *ch = &i2c_channels[channel];

    if (ch->state != I2C_MTX) {
        return I2C_STATUS_ERROR;
    }

    if (ch->master_buffer_index < ch->master_buffer_length) {
        ch->master_buffer[ch->master_buffer_index++] = data;
        I2C_SendData(data);
        return I2C_STATUS_SUCCESS;
    }

    return I2C_STATUS_ERROR;
}

int16_t i2c_read_ack_channel(i2c_channel_t channel, uint16_t timeout) {
    i2c_channel_state_t *ch = &i2c_channels[channel];

    if (ch->state != I2C_MRX) {
        return -1;
    }

    I2C_AcknowledgeConfig(ENABLE);

    if (ch->master_buffer_index < ch->master_buffer_length) {
        return I2C_ReceiveData();
    }

    return -1;
}

int16_t i2c_read_nack_channel(i2c_channel_t channel, uint16_t timeout) {
    i2c_channel_state_t *ch = &i2c_channels[channel];

    if (ch->state != I2C_MRX) {
        return -1;
    }

    I2C_AcknowledgeConfig(DISABLE);

    if (ch->master_buffer_index < ch->master_buffer_length + 1) {
        return I2C_ReceiveData();
    }

    return -1;
}

void i2c_stop_channel(i2c_channel_t channel) {
    i2c_channel_state_t *ch = &i2c_channels[channel];

    ch->send_stop = true;

    if (ch->state == I2C_MTX || ch->state == I2C_MRX) {
        I2C_GenerateSTOP(ENABLE);
        ch->state = I2C_READY;
    }
}

/*==========================================
 * I2C复合操作函数
 *=========================================*/

i2c_status_t i2c_transmit_channel(i2c_channel_t channel, uint8_t address, const uint8_t* data, uint16_t length, uint16_t timeout) {
    int result = i2c_write_to_channel(channel, address, data, (uint8_t)length, 1, 1);

    if (result == 0) {
        return I2C_STATUS_SUCCESS;
    }

    return I2C_STATUS_ERROR;
}

i2c_status_t i2c_receive_channel(i2c_channel_t channel, uint8_t address, uint8_t* data, uint16_t length, uint16_t timeout) {
    int result = i2c_read_from_channel(channel, address, data, (uint8_t)length, 1, (int)timeout);

    if (result >= 0) {
        return I2C_STATUS_SUCCESS;
    }

    return I2C_STATUS_ERROR;
}

i2c_status_t i2c_writeReg_channel(i2c_channel_t channel, uint8_t devaddr, uint8_t regaddr, const uint8_t* data, uint16_t length, uint16_t timeout) {
    int result;
    uint8_t buffer[I2C_BUFFER_LENGTH];

    if (length + 1 > I2C_BUFFER_LENGTH) {
        return I2C_STATUS_ERROR;
    }

    /* 先发送寄存器地址 */
    buffer[0] = regaddr;
    memcpy(&buffer[1], data, length);

    /* 使用官方例程的i2c_write_to实现 */
    result = i2c_write_to_channel(channel, devaddr, buffer, length + 1, 1, 1);

    if (result == 0) {
        return I2C_STATUS_SUCCESS;
    }

    return I2C_STATUS_ERROR;
}

i2c_status_t i2c_writeReg16_channel(i2c_channel_t channel, uint8_t devaddr, uint16_t regaddr, const uint8_t* data, uint16_t length, uint16_t timeout) {
    int result;
    uint8_t buffer[I2C_BUFFER_LENGTH];

    if (length + 2 > I2C_BUFFER_LENGTH) {
        return I2C_STATUS_ERROR;
    }

    /* 先发送16位寄存器地址（高字节在前） */
    buffer[0] = (regaddr >> 8) & 0xFF;
    buffer[1] = regaddr & 0xFF;
    memcpy(&buffer[2], data, length);

    /* 使用官方例程的i2c_write_to实现 */
    result = i2c_write_to_channel(channel, devaddr, buffer, length + 2, 1, 1);

    if (result == 0) {
        return I2C_STATUS_SUCCESS;
    }

    return I2C_STATUS_ERROR;
}

i2c_status_t i2c_readReg_channel(i2c_channel_t channel, uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length, uint16_t timeout) {
    int result;
    uint8_t reg = regaddr;

    /* 首先发送寄存器地址（写操作，不发送STOP） */
    result = i2c_write_to_channel(channel, devaddr, &reg, 1, 1, 0);
    if (result != 0) {
        return I2C_STATUS_ERROR;
    }

    /* 然后读取数据（重复START） */
    result = i2c_read_from_channel(channel, devaddr, data, (uint8_t)length, 1, (int)timeout);

    if (result >= 0) {
        return I2C_STATUS_SUCCESS;
    }

    return I2C_STATUS_ERROR;
}

i2c_status_t i2c_readReg16_channel(i2c_channel_t channel, uint8_t devaddr, uint16_t regaddr, uint8_t* data, uint16_t length, uint16_t timeout) {
    int result;
    uint8_t reg[2];

    /* 首先发送16位寄存器地址（写操作，不发送STOP） */
    reg[0] = (regaddr >> 8) & 0xFF;
    reg[1] = regaddr & 0xFF;

    result = i2c_write_to_channel(channel, devaddr, reg, 2, 1, 0);
    if (result != 0) {
        return I2C_STATUS_ERROR;
    }

    /* 然后读取数据（重复START） */
    result = i2c_read_from_channel(channel, devaddr, data, (uint8_t)length, 1, (int)timeout);

    if (result >= 0) {
        return I2C_STATUS_SUCCESS;
    }

    return I2C_STATUS_ERROR;
}

/*==========================================
 * GPIO引脚绑定函数
 * 将引脚绑定通道，外部操作以通道作为参数
 *=========================================*/

i2c_status_t i2c_bind_pins(pin_t sda_pin, pin_t scl_pin, i2c_channel_t channel) {
    if (channel >= I2C_CHANNEL_MAX) {
        return I2C_STATUS_ERROR;
    }

    /* 检查引脚是否已绑定到其他通道 */
    for (int i = 0; i < I2C_CHANNEL_MAX; i++) {
        if (i != channel) {
            if (i2c_channels[i].bound) {
                if ((i2c_channels[i].sda_pin == sda_pin) || (i2c_channels[i].scl_pin == scl_pin) ||
                    (i2c_channels[i].sda_pin == scl_pin) || (i2c_channels[i].scl_pin == sda_pin)) {
                    return I2C_STATUS_ERROR;  /* 引脚冲突 */
                }
            }
        }
    }

    /* 绑定引脚到通道 */
    i2c_channel_state_t *ch = &i2c_channels[channel];
    ch->sda_pin = sda_pin;
    ch->scl_pin = scl_pin;
    ch->bound = true;

    /* 配置GPIO引脚（完全参照app_i2c.c） */
    i2c_configure_pins(channel);

    return I2C_STATUS_SUCCESS;
}

pin_t i2c_get_sda_pin(i2c_channel_t channel) {
    if (channel >= I2C_CHANNEL_MAX) {
        return NO_PIN;
    }

    return i2c_channels[channel].sda_pin;
}

pin_t i2c_get_scl_pin(i2c_channel_t channel) {
    if (channel >= I2C_CHANNEL_MAX) {
        return NO_PIN;
    }

    return i2c_channels[channel].scl_pin;
}

bool i2c_is_bound(i2c_channel_t channel) {
    if (channel >= I2C_CHANNEL_MAX) {
        return false;
    }

    return i2c_channels[channel].bound;
}

/*==========================================
 * I2C初始化（包含引脚通道保存）真正的结合硬件的初始化
 *=========================================*/

i2c_status_t i2c_init_channel_with_pins(i2c_channel_t channel, pin_t sda_pin, pin_t scl_pin, uint32_t clock_speed) {
    if (channel >= I2C_CHANNEL_MAX) {
        return I2C_STATUS_ERROR;
    }

    /* 绑定引脚 */
    i2c_status_t status = i2c_bind_pins(sda_pin, scl_pin, channel);
    if (status != I2C_STATUS_SUCCESS) {
        return status;
    }

    /* 保存时钟速度 */
    i2c_channels[channel].clock_speed = clock_speed;
    i2c_channels[channel].own_address = 0x00;  /* 不使用双地址模式 */

    /* 若使用 PB20/PB21 作 SDA/SCL，须使能 I2C 引脚重映射（默认在 PB12/PB13） */
    if (sda_pin == B20 && scl_pin == B21) {
        GPIOPinRemap(ENABLE, RB_PIN_I2C);
    }
    else if(sda_pin == B12 && scl_pin == B13) {
        GPIOPinRemap(DISABLE, RB_PIN_I2C);
    }
    else {
        return I2C_STATUS_ERROR;
    }

    /* 初始化I2C控制器（完全参照app_i2c.c i2c_app_init） */
    I2C_Init(I2C_Mode_I2C, clock_speed, I2C_DutyCycle_16_9, I2C_Ack_Enable,
             I2C_AckAddr_7bit, i2c_channels[channel].own_address);

    /* 使能I2C（完全参照官方例程） */
    I2C_Cmd(ENABLE);

    /* 使能I2C中断（完全参照官方例程） */
    I2C_ITConfig(I2C_IT_BUF, ENABLE);
    I2C_ITConfig(I2C_IT_EVT, ENABLE);
    I2C_ITConfig(I2C_IT_ERR, ENABLE);
    PFIC_EnableIRQ(I2C_IRQn);

    /* 标记为已初始化 */
    i2c_channels[channel].initialized = true;

    return I2C_STATUS_SUCCESS;
}

/*==========================================
 * PCT1336 I2C通信测试
 *=========================================*/
void PCT1336_Communication_Test(void)
{
    uint8_t test_val = 0;
    uint8_t retry_count = 0;
    uint8_t test_success = 0;
    const i2c_channel_t ch = I2C_CHANNEL_0;
    const uint8_t dev_addr = 0x33; /* PCT1336 7-bit I2C 地址 */

    PRINT("=== PCT1336 I2C test ===\r\n");

    /* 测试1: 读取状态寄存器 0x70 */
    for (retry_count = 0; retry_count < 3; retry_count++) {
        if (i2c_readReg_channel(ch, dev_addr, 0x70, &test_val, 1, I2C_TIMEOUT_INFINITE) == I2C_STATUS_SUCCESS) {
            PRINT("reg0x70 success: 0x%02X\r\n", test_val);
            test_success = 1;
            break;
        } else {
            PRINT("reg0x70 error %d/3\r\n", retry_count + 1);
            mDelaymS(10);
        }
    }

    /* 测试2: 读取设备ID寄存器 0x71 */
    if (test_success) {
        if (i2c_readReg_channel(ch, dev_addr, 0x71, &test_val, 1, I2C_TIMEOUT_INFINITE) == I2C_STATUS_SUCCESS) {
            PRINT("reg0x71 success: 0x%02X\r\n", test_val);
        } else {
            PRINT("reg0x71 error\r\n");
        }
    }

    /* 测试3: 写入测试寄存器 0x72 并回读 */
    if (test_success) {
        uint8_t write_val = 0xBB;
        if (i2c_writeReg_channel(ch, dev_addr, 0x72, &write_val, 1, I2C_TIMEOUT_INFINITE) == I2C_STATUS_SUCCESS) {
            PRINT("reg0x72 write success\r\n");
            if (i2c_readReg_channel(ch, dev_addr, 0x72, &test_val, 1, I2C_TIMEOUT_INFINITE) == I2C_STATUS_SUCCESS) {
                PRINT("reg0x72 readback success: 0x%02X\r\n", test_val);
            } else {
                PRINT("reg0x72 readback error\r\n");
            }
        } else {
            PRINT("reg0x72 write error\r\n");
        }
    }

    if (test_success) {
        PRINT("=== PCT1336 I2C test success ===\r\n");
    } else {
        PRINT("=== PCT1336 I2C test error ===\r\n");
    }
}
