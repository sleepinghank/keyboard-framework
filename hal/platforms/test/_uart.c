#include <stdio.h>
#include <string.h>
#include "uart.h"
#include "kb904/config_product.h"

// UART通道状态结构体
typedef struct {
    pin_t rx_pin;           // RX引脚
    pin_t tx_pin;           // TX引脚
    bool rx_enabled;        // RX是否启用
    bool tx_enabled;        // TX是否启用
    bool initialized;       // 是否已初始化
    uint32_t baudrate;      // 波特率
    uart_rx_callback_t rx_callback;  // 接收回调函数
} uart_channel_t;

// 全局UART通道数组，支持4个UART端口
static uart_channel_t uart_channels[4] = {
    [0] = {.rx_pin = NO_PIN, .tx_pin = NO_PIN, .rx_enabled = false, .tx_enabled = false, .initialized = false, .baudrate = 0, .rx_callback = NULL},
    [1] = {.rx_pin = NO_PIN, .tx_pin = NO_PIN, .rx_enabled = false, .tx_enabled = false, .initialized = false, .baudrate = 0, .rx_callback = NULL},
    [2] = {.rx_pin = NO_PIN, .tx_pin = NO_PIN, .rx_enabled = false, .tx_enabled = false, .initialized = false, .baudrate = 0, .rx_callback = NULL},
    [3] = {.rx_pin = NO_PIN, .tx_pin = NO_PIN, .rx_enabled = false, .tx_enabled = false, .initialized = false, .baudrate = 0, .rx_callback = NULL},
};

/**
 * @brief   平台串口初始化
 * @param   uart - 串口号
 * @param   baudrate - 波特率
 * @param   mode - 串口工作模式
 * @param   io_map - IO映射选择
 * @param   enable_interrupt - 是否启用中断
 * @return  error_code_t 错误码
 */
error_code_t platform_uart_init(platform_uart_t uart, uint32_t baudrate, uint8_t enable_interrupt) {
    printf("[UART] platform_uart_init: uart=%d, baudrate=%u, enable_interrupt=%d\n",
           uart, baudrate, enable_interrupt);

    // 参数检查
    if (uart >= 4) {
        printf("[UART] ERROR: Invalid UART port %d\n", uart);
        return ERROR_UART_INVALID_PORT;
    }

    // 初始化UART通道
    uart_channels[uart].initialized = true;
    uart_channels[uart].baudrate = baudrate;

    printf("[UART] UART%d initialized successfully (baudrate=%u)\n", uart, baudrate);

    return NO_ERROR;
}

/**
 * @brief   平台串口关闭
 * @param   uart - 串口号
 * @return  error_code_t 错误码
 */
error_code_t platform_uart_close(platform_uart_t uart) {
    printf("[UART] platform_uart_close: uart=%d\n", uart);

    // 参数检查
    if (uart >= 4) {
        printf("[UART] ERROR: Invalid UART port %d\n", uart);
        return ERROR_UART_INVALID_PORT;
    }

    // 关闭UART通道
    uart_channels[uart].initialized = false;
    uart_channels[uart].rx_callback = NULL;

    printf("[UART] UART%d closed successfully\n", uart);

    return NO_ERROR;
}

/**
 * @brief   平台串口发送一个字节
 * @param   uart - 串口号
 * @param   data - 要发送的数据
 * @return  error_code_t 错误码
 */
error_code_t platform_uart_write_byte(platform_uart_t uart, uint8_t data) {
    printf("[UART] platform_uart_write_byte: uart=%d, data=0x%02X ('%c')\n",
           uart, data, (data >= 32 && data < 127) ? data : '.');

    // 参数检查
    if (uart >= 4) {
        printf("[UART] ERROR: Invalid UART port %d\n", uart);
        return ERROR_UART_INVALID_PORT;
    }

    // 检查是否已初始化
    if (!uart_channels[uart].initialized) {
        printf("[UART] ERROR: UART%d not initialized\n", uart);
        return ERROR_COMM_STATE;
    }

    // 检查TX是否启用
    if (!uart_channels[uart].tx_enabled) {
        printf("[UART] WARNING: UART%d TX not enabled\n", uart);
    }

    printf("[UART] UART%d: Sent byte 0x%02X\n", uart, data);

    return NO_ERROR;
}

/**
 * @brief   平台串口发送数据
 * @param   uart - 串口号
 * @param   data - 要发送的数据缓冲区
 * @param   len - 要发送的数据长度
 * @return  error_code_t 错误码
 */
error_code_t platform_uart_write(platform_uart_t uart, uint8_t *data, uint16_t len) {
    printf("[UART] platform_uart_write: uart=%d, data=%p, len=%d\n", uart, data, len);

    // 参数检查
    if (uart >= 4) {
        printf("[UART] ERROR: Invalid UART port %d\n", uart);
        return ERROR_UART_INVALID_PORT;
    }

    if (data == NULL) {
        printf("[UART] ERROR: NULL data pointer\n");
        return ERROR_INVALID_PARAM;
    }

    if (len == 0) {
        printf("[UART] WARNING: Zero length data\n");
        return NO_ERROR;
    }

    // 检查是否已初始化
    if (!uart_channels[uart].initialized) {
        printf("[UART] ERROR: UART%d not initialized\n", uart);
        return ERROR_COMM_STATE;
    }

    // 检查TX是否启用
    if (!uart_channels[uart].tx_enabled) {
        printf("[UART] WARNING: UART%d TX not enabled\n", uart);
    }

    // 打印数据内容（最多打印16字节）
    printf("[UART] UART%d: Sending %d bytes: ", uart, len);
    for (int i = 0; i < len && i < 16; i++) {
        printf("%02X ", data[i]);
    }
    if (len > 16) {
        printf("... (%d more bytes)", len - 16);
    }
    printf("\n");

    return NO_ERROR;
}

/**
 * @brief   平台串口注册接收回调函数
 * @param   uart - 串口号
 * @param   callback - 回调函数指针
 * @return  error_code_t 错误码
 * @note    回调函数中不能做耗时操作，否则会影响串口接收
 */
error_code_t platform_uart_register_rx_callback(platform_uart_t uart, uart_rx_callback_t callback) {
    printf("[UART] platform_uart_register_rx_callback: uart=%d, callback=%p\n",
           uart, callback);

    // 参数检查
    if (uart >= 4) {
        printf("[UART] ERROR: Invalid UART port %d\n", uart);
        return ERROR_UART_INVALID_PORT;
    }

    // 注册回调函数
    uart_channels[uart].rx_callback = callback;

    if (callback != NULL) {
        printf("[UART] UART%d: RX callback registered\n", uart);
    } else {
        printf("[UART] UART%d: RX callback unregistered\n", uart);
    }

    return NO_ERROR;
}

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
error_code_t platform_uart_bind_pins(pin_t rx_pin, pin_t tx_pin, platform_uart_t uart) {
    printf("[UART] platform_uart_bind_pins: uart=%d, rx_pin=0x%02X, tx_pin=0x%02X\n",
           uart, rx_pin, tx_pin);

    // 参数检查
    if (uart >= 4) {
        printf("[UART] ERROR: Invalid UART port %d\n", uart);
        return ERROR_UART_INVALID_PORT;
    }

    // 绑定引脚
    uart_channels[uart].rx_pin = rx_pin;
    uart_channels[uart].tx_pin = tx_pin;

    // Enable RX/TX based on whether pins are valid (not NO_PIN)
    uart_channels[uart].rx_enabled = (rx_pin != NO_PIN);
    uart_channels[uart].tx_enabled = (tx_pin != NO_PIN);

    printf("[UART] UART%d: Pins bound successfully\n", uart);
    printf("[UART]   RX pin: 0x%02X (%s)\n", rx_pin, (rx_pin != NO_PIN) ? "enabled" : "disabled");
    printf("[UART]   TX pin: 0x%02X (%s)\n", tx_pin, (tx_pin != NO_PIN) ? "enabled" : "disabled");

    return NO_ERROR;
}

/**
 * @brief   获取UART信道绑定的RX引脚
 * @param   uart UART通道号
 * @return  RX引脚号（如果未绑定则返回NO_PIN）
 */
pin_t platform_uart_get_rx_pin(platform_uart_t uart) {
    if (uart >= 4) {
        printf("[UART] ERROR: Invalid UART port %d\n", uart);
        return NO_PIN;
    }

    pin_t pin = uart_channels[uart].rx_pin;
    printf("[UART] platform_uart_get_rx_pin: uart=%d, pin=0x%02X\n", uart, pin);

    return pin;
}

/**
 * @brief   获取UART信道绑定的TX引脚
 * @param   uart UART通道号
 * @return  TX引脚号（如果未绑定则返回NO_PIN）
 */
pin_t platform_uart_get_tx_pin(platform_uart_t uart) {
    if (uart >= 4) {
        printf("[UART] ERROR: Invalid UART port %d\n", uart);
        return NO_PIN;
    }

    pin_t pin = uart_channels[uart].tx_pin;
    printf("[UART] platform_uart_get_tx_pin: uart=%d, pin=0x%02X\n", uart, pin);

    return pin;
}

/**
 * @brief   检查UART信道是否已绑定GPIO引脚
 * @param   uart UART通道号
 * @return  true表示已绑定（至少有一个有效引脚），false表示未绑定
 */
bool platform_uart_is_bound(platform_uart_t uart) {
    if (uart >= 4) {
        printf("[UART] ERROR: Invalid UART port %d\n", uart);
        return false;
    }

    bool bound = (uart_channels[uart].rx_pin != NO_PIN) || (uart_channels[uart].tx_pin != NO_PIN);
    printf("[UART] platform_uart_is_bound: uart=%d, bound=%s\n", uart, bound ? "true" : "false");

    return bound;
}

/**
 * @brief   检查UART信道是否启用了RX功能
 * @param   uart UART通道号
 * @return  true表示已启用RX，false表示未启用
 */
bool platform_uart_is_rx_enabled(platform_uart_t uart) {
    if (uart >= 4) {
        printf("[UART] ERROR: Invalid UART port %d\n", uart);
        return false;
    }

    bool enabled = uart_channels[uart].rx_enabled;
    printf("[UART] platform_uart_is_rx_enabled: uart=%d, enabled=%s\n", uart, enabled ? "true" : "false");

    return enabled;
}

/**
 * @brief   检查UART信道是否启用了TX功能
 * @param   uart UART通道号
 * @return  true表示已启用TX，false表示未启用
 */
bool platform_uart_is_tx_enabled(platform_uart_t uart) {
    if (uart >= 4) {
        printf("[UART] ERROR: Invalid UART port %d\n", uart);
        return false;
    }

    bool enabled = uart_channels[uart].tx_enabled;
    printf("[UART] platform_uart_is_tx_enabled: uart=%d, enabled=%s\n", uart, enabled ? "true" : "false");

    return enabled;
}
