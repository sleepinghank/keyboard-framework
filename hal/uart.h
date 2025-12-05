#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "sys_config.h"


//定义串口号
typedef enum {
    PLATFORM_UART_0 = 0,
    PLATFORM_UART_1,
    PLATFORM_UART_2,
    PLATFORM_UART_3
} platform_uart_t;

//定义串口模式
typedef enum {
    PLATFORM_UART_MODE_TX = 0,//只发送
    PLATFORM_UART_MODE_RX,//只接收
    PLATFORM_UART_MODE_TX_RX//发送和接收
} platform_uart_mode_t;

//定义是否启用映射IO
typedef enum {
    PLATFORM_UART_IO_MAP_ENABLE = 0,//启用映射IO
    PLATFORM_UART_IO_MAP_DISABLE//不启用映射IO
} platform_uart_io_map_t;

//定义回调函数类型
typedef void (*uart_rx_callback_t)(u8_t *data, u16_t len);

/**
 * @brief   平台串口初始化
 * @param   uart - 串口号
 * @param   baudrate - 波特率
 * @param   mode - 串口工作模式
 * @param   io_map - IO映射选择
 * @param   enable_interrupt - 是否启用中断
 * @return  error_code_t 错误码
 */
extern error_code_t platform_uart_init(platform_uart_t uart, u32_t baudrate, platform_uart_mode_t mode, platform_uart_io_map_t io_map, u8_t enable_interrupt);

/**
 * @brief   平台串口关闭
 * @param   uart - 串口号
 * @return  error_code_t 错误码
 */
extern error_code_t platform_uart_close(platform_uart_t uart);

/**
 * @brief   平台串口发送一个字节
 * @param   uart - 串口号
 * @param   data - 要发送的数据
 * @return  error_code_t 错误码
 */
extern error_code_t platform_uart_write_byte(platform_uart_t uart, u8_t data);

/**
 * @brief   平台串口发送数据
 * @param   uart - 串口号
 * @param   data - 要发送的数据缓冲区
 * @param   len - 要发送的数据长度
 * @return  error_code_t 错误码
 */
extern error_code_t platform_uart_write(platform_uart_t uart, u8_t *data, u16_t len);

/**
 * @brief   平台串口注册接收回调函数
 * @param   uart - 串口号
 * @param   callback - 回调函数指针
 * @return  error_code_t 错误码
 * @note    回调函数中不能做耗时操作，否则会影响串口接收
 */
extern error_code_t platform_uart_register_rx_callback(platform_uart_t uart, uart_rx_callback_t callback);

#ifdef __cplusplus
}
#endif
