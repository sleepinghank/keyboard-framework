#pragma once

#ifdef __cplusplus
"C" {
#endif

#include "kb904/config_product.h"
#include "pin_defs.h"
#include "pin_mapper.h"
#include "gpio.h"
//定义串口号
typedef enum {
    PLATFORM_UART_0 = 0,
    PLATFORM_UART_1,
    PLATFORM_UART_2,
    PLATFORM_UART_3,
    PLATFORM_UART_END
} platform_uart_t;


//定义回调函数类型
typedef void (*uart_rx_callback_t)(uint8_t *data, uint16_t len);

/**
 * @brief   平台串口初始化
 * @param   uart - 串口号
 * @param   baudrate - 波特率
 * @param   mode - 串口工作模式
 * @param   io_map - IO映射选择
 * @param   enable_interrupt - 是否启用中断
 * @return  error_code_t 错误码
 */
error_code_t platform_uart_init(platform_uart_t uart, uint32_t baudrate,  uint8_t enable_interrupt);

/**
 * @brief   平台串口关闭
 * @param   uart - 串口号
 * @return  error_code_t 错误码
 */
error_code_t platform_uart_close(platform_uart_t uart);

/**
 * @brief   平台串口发送一个字节
 * @param   uart - 串口号
 * @param   data - 要发送的数据
 * @return  error_code_t 错误码
 */
error_code_t platform_uart_write_byte(platform_uart_t uart, uint8_t data);

/**
 * @brief   平台串口发送数据
 * @param   uart - 串口号
 * @param   data - 要发送的数据缓冲区
 * @param   len - 要发送的数据长度
 * @return  error_code_t 错误码
 */
error_code_t platform_uart_write(platform_uart_t uart, uint8_t *data, uint16_t len);

/**
 * @brief   平台串口注册接收回调函数
 * @param   uart - 串口号
 * @param   callback - 回调函数指针
 * @return  error_code_t 错误码
 * @note    回调函数中不能做耗时操作，否则会影响串口接收
 */
error_code_t platform_uart_register_rx_callback(platform_uart_t uart, uart_rx_callback_t callback);

/*==========================================
 * GPIO引脚绑定到UART信道
 *=========================================*/

/**
 * @brief   绑定GPIO引脚到UART信道
 * @param   rx_pin RX引脚号（如果为NO_PIN则禁用接收）
 * @param   tx_pin TX引脚号（如果为NO_PIN则禁用发送）
 * @param   uart UART通道号
 * @return  error_code_t 错误码
 * @note    此函数将GPIO引脚绑定到指定UART信道，根据引脚是否有效决定启用RX/TX
 *          示例：
 *          platform_uart_bind_pins(PIN_A0, PIN_A1, PLATFORM_UART_0); // 启用RX和TX
 *          platform_uart_bind_pins(PIN_A0, NO_PIN, PLATFORM_UART_0); // 仅启用RX
 *          platform_uart_bind_pins(NO_PIN, PIN_A1, PLATFORM_UART_0); // 仅启用TX
 */
error_code_t platform_uart_bind_pins(pin_t rx_pin, pin_t tx_pin, platform_uart_t uart);

/**
 * @brief   获取UART信道绑定的RX引脚
 * @param   uart UART通道号
 * @return  RX引脚号（如果未绑定则返回NO_PIN）
 */
pin_t platform_uart_get_rx_pin(platform_uart_t uart);

/**
 * @brief   获取UART信道绑定的TX引脚
 * @param   uart UART通道号
 * @return  TX引脚号（如果未绑定则返回NO_PIN）
 */
pin_t platform_uart_get_tx_pin(platform_uart_t uart);

/**
 * @brief   检查UART信道是否已绑定GPIO引脚
 * @param   uart UART通道号
 * @return  true表示已绑定（至少有一个有效引脚），false表示未绑定
 */
bool platform_uart_is_bound(platform_uart_t uart);

/**
 * @brief   检查UART信道是否启用了RX功能
 * @param   uart UART通道号
 * @return  true表示已启用RX，false表示未启用
 */
bool platform_uart_is_rx_enabled(platform_uart_t uart);

/**
 * @brief   检查UART信道是否启用了TX功能
 * @param   uart UART通道号
 * @return  true表示已启用TX，false表示未启用
 */
bool platform_uart_is_tx_enabled(platform_uart_t uart);

#ifdef __cplusplus
}
#endif
