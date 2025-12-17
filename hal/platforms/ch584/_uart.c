#include "uart.h"
#include "CH58x_common.h"
#include "pin_mapper.h"
#include <stdbool.h>

#if CHIP_TYPE == CHIP_CH584M

//串口0接收回调
uart_rx_callback_t uart0_rx_callback = NULL;
//串口1接收回调
uart_rx_callback_t uart1_rx_callback = NULL;
//串口2接收回调
uart_rx_callback_t uart2_rx_callback = NULL;
//串口3接收回调
uart_rx_callback_t uart3_rx_callback = NULL;

// UART channel state structure
typedef struct {
    pin_t rx_pin;       // RX pin for this channel
    pin_t tx_pin;       // TX pin for this channel
    bool rx_enabled;    // RX enable flag
    bool tx_enabled;    // TX enable flag
    bool initialized;   // Initialization flag
} uart_channel_state_t;

// UART channel state array
static uart_channel_state_t uart_channels[4] = {0};


//串口初始化
error_code_t port_uart_init(platform_uart_t uart, u32_t baudrate, platform_uart_mode_t mode, platform_uart_io_map_t io_map, u8_t enable_interrupt)
{

    switch (uart)
    {
    case PLATFORM_UART_0:

        if(io_map == PLATFORM_UART_IO_MAP_ENABLE)
        {
            GPIOPinRemap(ENABLE,RB_PIN_UART0);//启用映射IO:RXD0_/TXD0_ on PA[15]/PA[14]
            if(mode == PLATFORM_UART_MODE_TX || mode == PLATFORM_UART_MODE_TX_RX)
            {
                GPIOA_SetBits(bTXD0_);
                GPIOA_ModeCfg(bTXD0_, GPIO_ModeOut_PP_5mA);
            }
            if(mode == PLATFORM_UART_MODE_RX || mode == PLATFORM_UART_MODE_TX_RX)
            {
                GPIOA_SetBits(bRXD0_);
                GPIOA_ModeCfg(bRXD0_, GPIO_ModeIN_PU);
            }
        }
        else
        {
            GPIOPinRemap(DISABLE,RB_PIN_UART0);//关闭映射IO:RXD0/TXD0 on PB[4]/PB[7]
            if(mode == PLATFORM_UART_MODE_TX || mode == PLATFORM_UART_MODE_TX_RX)
            {
                GPIOB_SetBits(bTXD0);
                GPIOB_ModeCfg(bTXD0, GPIO_ModeOut_PP_5mA);
            }
            if(mode == PLATFORM_UART_MODE_RX || mode == PLATFORM_UART_MODE_TX_RX)
            {
                GPIOB_SetBits(bRXD0);
                GPIOB_ModeCfg(bRXD0, GPIO_ModeIN_PU);
            }
        }

        UART0_BaudRateCfg(baudrate);
        if(mode == PLATFORM_UART_MODE_TX_RX)
        {
           R8_UART0_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO打开，触发点4字节
        }else if(mode == PLATFORM_UART_MODE_TX)
        {
            R8_UART0_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR  | RB_FCR_FIFO_EN; // FIFO打开，触发点4字节
        }else if(mode == PLATFORM_UART_MODE_RX)
        {
            R8_UART0_FCR = (2 << 6)  | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO打开，触发点4字节
        }
        
        R8_UART0_LCR = RB_LCR_WORD_SZ;
        R8_UART0_IER = RB_IER_TXD_EN;
        R8_UART0_DIV = 1;
        
        if(enable_interrupt)
        {
            UART0_ByteTrigCfg(UART_1BYTE_TRIG);
            UART0_INTCfg(ENABLE, RB_IER_RECV_RDY | RB_IER_LINE_STAT);
            PFIC_EnableIRQ(UART0_IRQn);
        }
        break;
    case PLATFORM_UART_1:

        if(io_map == PLATFORM_UART_IO_MAP_ENABLE)
        {
            GPIOPinRemap(ENABLE,RB_PIN_UART1);//启用映射IO:RXD1_/TXD1_ on PB[12]/PB[13]
            if(mode == PLATFORM_UART_MODE_TX || mode == PLATFORM_UART_MODE_TX_RX)
            {
                GPIOB_SetBits(bTXD1_);
                GPIOB_ModeCfg(bTXD1_, GPIO_ModeOut_PP_5mA);
            }
            if(mode == PLATFORM_UART_MODE_RX || mode == PLATFORM_UART_MODE_TX_RX)
            {
                GPIOB_SetBits(bRXD1_);
                GPIOB_ModeCfg(bRXD1_, GPIO_ModeIN_PU);
            }
        }
        else
        {
            GPIOPinRemap(DISABLE,RB_PIN_UART1);//关闭映射IO:RXD1/TXD1 on PA[8]/PA[9]
            if(mode == PLATFORM_UART_MODE_TX || mode == PLATFORM_UART_MODE_TX_RX)
            {
                GPIOA_SetBits(bTXD1);
                GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA);
            }
            if(mode == PLATFORM_UART_MODE_RX || mode == PLATFORM_UART_MODE_TX_RX)
            {
                GPIOA_SetBits(bRXD1);
                GPIOA_ModeCfg(bRXD1, GPIO_ModeIN_PU);
            }
        }
        UART1_BaudRateCfg(baudrate);    
        if(mode == PLATFORM_UART_MODE_TX_RX)
        {
           R8_UART1_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO打开，触发点4字节
        }else if(mode == PLATFORM_UART_MODE_TX)
        {
            R8_UART1_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR  | RB_FCR_FIFO_EN; // FIFO打开，触发点4字节
        }else if(mode == PLATFORM_UART_MODE_RX)
        {
            R8_UART1_FCR = (2 << 6)  | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO打开，触发点4字节
        }

        R8_UART1_LCR = RB_LCR_WORD_SZ;
        R8_UART1_IER = RB_IER_TXD_EN;
        R8_UART1_DIV = 1;

        if(enable_interrupt)
        {
            UART1_ByteTrigCfg(UART_1BYTE_TRIG);
            UART1_INTCfg(ENABLE, RB_IER_RECV_RDY | RB_IER_LINE_STAT);
            PFIC_EnableIRQ(UART1_IRQn);
        }
        break;
    case PLATFORM_UART_2:
        if(io_map == PLATFORM_UART_IO_MAP_ENABLE)
        {
            GPIOPinRemap(ENABLE,RB_PIN_UART2);//启用映射IO:RXD2_/TXD2_ on PB[22]/PB[23]
            if(mode == PLATFORM_UART_MODE_TX || mode == PLATFORM_UART_MODE_TX_RX)
            {
                GPIOB_SetBits(bTXD2_);
                GPIOB_ModeCfg(bTXD2_, GPIO_ModeOut_PP_5mA);
            }
            if(mode == PLATFORM_UART_MODE_RX || mode == PLATFORM_UART_MODE_TX_RX)
            {
                GPIOB_SetBits(bRXD2_);
                GPIOB_ModeCfg(bRXD2_, GPIO_ModeIN_PU);
            }
        }
        else
        {
            GPIOPinRemap(DISABLE,RB_PIN_UART2);//关闭映射IO:RXD2/TXD2 on PA[6]/PA[7]
            if(mode == PLATFORM_UART_MODE_TX || mode == PLATFORM_UART_MODE_TX_RX)
            {
                GPIOA_SetBits(bTXD2);
                GPIOA_ModeCfg(bTXD2, GPIO_ModeOut_PP_5mA);
            }
            if(mode == PLATFORM_UART_MODE_RX || mode == PLATFORM_UART_MODE_TX_RX)
            {
                GPIOA_SetBits(bRXD2);
                GPIOA_ModeCfg(bRXD2, GPIO_ModeIN_PU);
            }
        }
        UART2_BaudRateCfg(baudrate);
        if(mode == PLATFORM_UART_MODE_TX_RX)
        {
           R8_UART2_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO打开，触发点4字节
        }else if(mode == PLATFORM_UART_MODE_TX)
        {
            R8_UART2_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR  | RB_FCR_FIFO_EN; // FIFO打开，触发点4字节
        }else if(mode == PLATFORM_UART_MODE_RX)
        {
            R8_UART2_FCR = (2 << 6)  | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO打开，触发点4字节
        }
        R8_UART2_LCR = RB_LCR_WORD_SZ;
        R8_UART2_IER = RB_IER_TXD_EN;
        R8_UART2_DIV = 1;

        if(enable_interrupt)
        {
            UART2_ByteTrigCfg(UART_1BYTE_TRIG);
            UART2_INTCfg(ENABLE, RB_IER_RECV_RDY | RB_IER_LINE_STAT);
            PFIC_EnableIRQ(UART2_IRQn);
        }
        break;
    case PLATFORM_UART_3:
        if(io_map == PLATFORM_UART_IO_MAP_ENABLE)
        {
            GPIOPinRemap(ENABLE,RB_PIN_UART3);//启用映射IO:RXD3_/TXD3_ on PB[20]/PB[21]
            if(mode == PLATFORM_UART_MODE_TX || mode == PLATFORM_UART_MODE_TX_RX)
            {
                GPIOB_SetBits(bTXD3_);
                GPIOB_ModeCfg(bTXD3_, GPIO_ModeOut_PP_5mA);
            }
            if(mode == PLATFORM_UART_MODE_RX || mode == PLATFORM_UART_MODE_TX_RX)
            {
                GPIOB_SetBits(bRXD3_);
                GPIOB_ModeCfg(bRXD3_, GPIO_ModeIN_PU);
            }
        }
        else
        {
            GPIOPinRemap(DISABLE,RB_PIN_UART3);//关闭映射IO:RXD3/TXD3 on PA[4]/PA[5]
            if(mode == PLATFORM_UART_MODE_TX || mode == PLATFORM_UART_MODE_TX_RX)
            {
                GPIOA_SetBits(bTXD3);
                GPIOA_ModeCfg(bTXD3, GPIO_ModeOut_PP_5mA);
            }
            if(mode == PLATFORM_UART_MODE_RX || mode == PLATFORM_UART_MODE_TX_RX)
            {
                GPIOA_SetBits(bRXD3);
                GPIOA_ModeCfg(bRXD3, GPIO_ModeIN_PU);
            }
        }

        UART3_BaudRateCfg(baudrate);
        if(mode == PLATFORM_UART_MODE_TX_RX)
        {
           R8_UART3_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO打开，触发点4字节
        }else if(mode == PLATFORM_UART_MODE_TX)
        {
            R8_UART3_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR  | RB_FCR_FIFO_EN; // FIFO打开，触发点4字节
        }else if(mode == PLATFORM_UART_MODE_RX)
        {
            R8_UART3_FCR = (2 << 6)  | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO打开，触发点4字节
        }
        R8_UART3_LCR = RB_LCR_WORD_SZ;
        R8_UART3_IER = RB_IER_TXD_EN;
        R8_UART3_DIV = 1;

        if(enable_interrupt)
        {
            UART3_ByteTrigCfg(UART_1BYTE_TRIG);
            UART3_INTCfg(ENABLE, RB_IER_RECV_RDY | RB_IER_LINE_STAT);
            PFIC_EnableIRQ(UART3_IRQn);
        }
        break;

    default:
        return ERROR_UART_INVALID_PORT;//无效串口
        break;
    }


    return NO_ERROR;
}

//关闭串口
error_code_t port_uart_close(platform_uart_t uart)
{
    switch(uart)
    {
        case PLATFORM_UART_0:
            UART0_Reset();
            break;
        case PLATFORM_UART_1:
            UART1_Reset();
            break;
        case PLATFORM_UART_2:
            UART2_Reset();
            break;
        case PLATFORM_UART_3:
            UART3_Reset();
            break;
        default:
            return ERROR_UART_INVALID_PORT;//无效串口
            break;
    }
    return NO_ERROR;
}

//串口发送一个字节
error_code_t port_uart_write_byte(platform_uart_t uart, u8_t data)
{
    switch(uart)
    {
        case PLATFORM_UART_0:
            UART0_SendByte(data);
            break;
        case PLATFORM_UART_1:
            UART1_SendByte(data);
            break;
        case PLATFORM_UART_2:
            UART2_SendByte(data);
            break;
        case PLATFORM_UART_3:
            UART3_SendByte(data);
            break;
        default:
            return ERROR_UART_INVALID_PORT;//无效串口
            break;
    }
    return NO_ERROR;
}

//串口发送数据
error_code_t port_uart_write(platform_uart_t uart, u8_t *data, u16_t len)
{
    switch(uart)
    {
        case PLATFORM_UART_0:
            UART0_SendString(data,len);
            break;
        case PLATFORM_UART_1:
            UART1_SendString(data,len);
            break;
        case PLATFORM_UART_2:
            UART2_SendString(data,len);
            break;
        case PLATFORM_UART_3:
            UART3_SendString(data,len);
            break;
        default:
            return ERROR_UART_INVALID_PORT;//无效串口
            break;
    }
    return NO_ERROR;
}

//注册串口接收回调函数
error_code_t port_uart_register_rx_callback(platform_uart_t uart, uart_rx_callback_t callback)
{
    switch(uart)
    {   
        case PLATFORM_UART_0:
            uart0_rx_callback = callback;
            break;
        case PLATFORM_UART_1:
            uart1_rx_callback = callback;
            break;
        case PLATFORM_UART_2:
            uart2_rx_callback = callback;
            break;
        case PLATFORM_UART_3:
            uart3_rx_callback = callback;
            break;
        default:
            return ERROR_UART_INVALID_PORT;//无效串口
            break;
    }
    return NO_ERROR;
}


/*********************************************************************
 * @fn      UART0_IRQHandler
 *
 * @brief   UART0中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void UART0_IRQHandler(void)
{
    uint8_t tmp;

    switch(UART0_GetITFlag())
    {
        case UART_II_LINE_STAT: // 线路状态错误
        {
            UART0_GetLinSTA();
            break;
        }

        case UART_II_RECV_RDY: // 数据达到设置触发点
                tmp = UART0_RecvByte();
                if(uart0_rx_callback != NULL)
                {
                    uart0_rx_callback(&tmp, 1);
                }
            break;

        case UART_II_RECV_TOUT: // 接收超时，暂时一帧数据接收完成

            break;

        case UART_II_THR_EMPTY: // 发送缓存区空，可继续发送
            break;

        case UART_II_MODEM_CHG: // 只支持串口0
            break;

        default:
            break;
    }
}

/*********************************************************************
 * @fn      UART1_IRQHandler
 *
 * @brief   UART1中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void UART1_IRQHandler(void)
{
    uint8_t tmp;

    switch(UART1_GetITFlag())
    {
        case UART_II_LINE_STAT: // 线路状态错误
        {
            UART1_GetLinSTA();
            break;
        }

        case UART_II_RECV_RDY: // 数据达到设置触发点

                tmp = UART1_RecvByte();
                if(uart1_rx_callback != NULL)
                {
                    uart1_rx_callback(&tmp, 1);
                }
            break;

        case UART_II_RECV_TOUT: // 接收超时，暂时一帧数据接收完成

            break;

        case UART_II_THR_EMPTY: // 发送缓存区空，可继续发送
            break;

        case UART_II_MODEM_CHG: // 只支持串口0
            break;

        default:
            break;
    }
}



/*********************************************************************
 * @fn      UART2_IRQHandler
 *
 * @brief   UART2中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void UART2_IRQHandler(void)
{
    uint8_t tmp;

    switch(UART2_GetITFlag())
    {
        case UART_II_LINE_STAT: // 线路状态错误
        {
            UART2_GetLinSTA();
            break;
        }

        case UART_II_RECV_RDY: // 数据达到设置触发点

                tmp = UART2_RecvByte();
                if(uart2_rx_callback != NULL)
                {
                    uart2_rx_callback(&tmp, 1);
                }
            break;

        case UART_II_RECV_TOUT: // 接收超时，暂时一帧数据接收完成

            break;

        case UART_II_THR_EMPTY: // 发送缓存区空，可继续发送
            break;

        case UART_II_MODEM_CHG: // 只支持串口0
            break;

        default:
            break;
    }
}


/*********************************************************************
 * @fn      UART3_IRQHandler
 *
 * @brief   UART3中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void UART3_IRQHandler(void)
{
    uint8_t tmp;

    switch(UART3_GetITFlag())
    {
        case UART_II_LINE_STAT: // 线路状态错误
        {
            UART3_GetLinSTA();
            break;
        }

        case UART_II_RECV_RDY: // 数据达到设置触发点

                tmp = UART3_RecvByte();
                if(uart3_rx_callback != NULL)
                {
                    uart3_rx_callback(&tmp, 1);
                }
            break;

        case UART_II_RECV_TOUT: // 接收超时，暂时一帧数据接收完成

            break;

        case UART_II_THR_EMPTY: // 发送缓存区空，可继续发送
            break;

        case UART_II_MODEM_CHG: // 只支持串口0
            break;

        default:
            break;
    }
}

/*==========================================
 * GPIO pin binding functions
 *=========================================*/

error_code_t platform_uart_bind_pins(pin_t rx_pin, pin_t tx_pin, platform_uart_t uart) {
    if (uart >= PLATFORM_UART_3 + 1) {
        return ERROR_UART_INVALID_PORT;
    }

    // Store the pin assignments
    uart_channels[uart].rx_pin = rx_pin;
    uart_channels[uart].tx_pin = tx_pin;

    // Enable RX/TX based on whether pins are valid (not NO_PIN)
    uart_channels[uart].rx_enabled = (rx_pin != NO_PIN);
    uart_channels[uart].tx_enabled = (tx_pin != NO_PIN);
    uart_channels[uart].initialized = true;

    // TODO: In a real implementation, you would:
    // 1. Configure the GPIO pins for UART (RX/TX)
    // 2. Set appropriate pin modes (push-pull for TX, input with pull-up for RX)
    // 3. Configure UART alternate functions if needed

    return NO_ERROR;
}

pin_t platform_uart_get_rx_pin(platform_uart_t uart) {
    if (uart >= PLATFORM_UART_3 + 1) {
        return NO_PIN;
    }

    return uart_channels[uart].rx_pin;
}

pin_t platform_uart_get_tx_pin(platform_uart_t uart) {
    if (uart >= PLATFORM_UART_3 + 1) {
        return NO_PIN;
    }

    return uart_channels[uart].tx_pin;
}

bool platform_uart_is_bound(platform_uart_t uart) {
    if (uart >= PLATFORM_UART_3 + 1) {
        return false;
    }

    return uart_channels[uart].initialized;
}

bool platform_uart_is_rx_enabled(platform_uart_t uart) {
    if (uart >= PLATFORM_UART_3 + 1) {
        return false;
    }

    return uart_channels[uart].rx_enabled;
}

bool platform_uart_is_tx_enabled(platform_uart_t uart) {
    if (uart >= PLATFORM_UART_3 + 1) {
        return false;
    }

    return uart_channels[uart].tx_enabled;
}

#endif // CHIP_CH584M
