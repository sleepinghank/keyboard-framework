#include <stdio.h>
#include <stdbool.h>

// 定义简单的类型和常量
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;

#define NO_PIN 0xFF
#define TRUE 1
#define FALSE 0

// 错误码定义
typedef enum {
    NO_ERROR = 0,
    ERROR_UART_INVALID_PORT = 0xC0B5,
    ERROR_INVALID_PARAM = 0xE0,
    ERROR_COMM_STATE = 0xC5,
} error_code_t;

// 平台UART定义
typedef enum {
    PLATFORM_UART_0 = 0,
    PLATFORM_UART_1,
    PLATFORM_UART_2,
    PLATFORM_UART_3
} platform_uart_t;

// 回调函数类型
typedef void (*uart_rx_callback_t)(uint8_t *data, uint16_t len);

// 简化的 UART 通道结构
typedef struct {
    uint8_t rx_pin;
    uint8_t tx_pin;
    bool rx_enabled;
    bool tx_enabled;
    bool initialized;
    uint32_t baudrate;
    uart_rx_callback_t rx_callback;
} uart_channel_t;

// 静态数组
static uart_channel_t uart_channels[4];

// UART 函数实现
error_code_t platform_uart_init(platform_uart_t uart, uint32_t baudrate, uint8_t enable_interrupt) {
    printf("[UART] platform_uart_init: uart=%d, baudrate=%u, enable_interrupt=%d\n",
           uart, baudrate, enable_interrupt);

    if (uart >= 4) {
        printf("[UART] ERROR: Invalid UART port %d\n", uart);
        return ERROR_UART_INVALID_PORT;
    }

    uart_channels[uart].initialized = true;
    uart_channels[uart].baudrate = baudrate;

    printf("[UART] UART%d initialized successfully (baudrate=%u)\n", uart, baudrate);

    return NO_ERROR;
}

error_code_t platform_uart_close(platform_uart_t uart) {
    printf("[UART] platform_uart_close: uart=%d\n", uart);

    if (uart >= 4) {
        printf("[UART] ERROR: Invalid UART port %d\n", uart);
        return ERROR_UART_INVALID_PORT;
    }

    uart_channels[uart].initialized = false;
    uart_channels[uart].rx_callback = NULL;

    printf("[UART] UART%d closed successfully\n", uart);

    return NO_ERROR;
}

error_code_t platform_uart_write_byte(platform_uart_t uart, uint8_t data) {
    printf("[UART] platform_uart_write_byte: uart=%d, data=0x%02X ('%c')\n",
           uart, data, (data >= 32 && data < 127) ? data : '.');

    if (uart >= 4) {
        printf("[UART] ERROR: Invalid UART port %d\n", uart);
        return ERROR_UART_INVALID_PORT;
    }

    if (!uart_channels[uart].initialized) {
        printf("[UART] ERROR: UART%d not initialized\n", uart);
        return ERROR_COMM_STATE;
    }

    if (!uart_channels[uart].tx_enabled) {
        printf("[UART] WARNING: UART%d TX not enabled\n", uart);
    }

    printf("[UART] UART%d: Sent byte 0x%02X\n", uart, data);

    return NO_ERROR;
}

error_code_t platform_uart_write(platform_uart_t uart, uint8_t *data, uint16_t len) {
    printf("[UART] platform_uart_write: uart=%d, data=%p, len=%d\n", uart, data, len);

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

    if (!uart_channels[uart].initialized) {
        printf("[UART] ERROR: UART%d not initialized\n", uart);
        return ERROR_COMM_STATE;
    }

    if (!uart_channels[uart].tx_enabled) {
        printf("[UART] WARNING: UART%d TX not enabled\n", uart);
    }

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

error_code_t platform_uart_register_rx_callback(platform_uart_t uart, uart_rx_callback_t callback) {
    printf("[UART] platform_uart_register_rx_callback: uart=%d, callback=%p\n",
           uart, callback);

    if (uart >= 4) {
        printf("[UART] ERROR: Invalid UART port %d\n", uart);
        return ERROR_UART_INVALID_PORT;
    }

    uart_channels[uart].rx_callback = callback;

    if (callback != NULL) {
        printf("[UART] UART%d: RX callback registered\n", uart);
    } else {
        printf("[UART] UART%d: RX callback unregistered\n", uart);
    }

    return NO_ERROR;
}

error_code_t platform_uart_bind_pins(uint8_t rx_pin, uint8_t tx_pin, platform_uart_t uart) {
    printf("[UART] platform_uart_bind_pins: uart=%d, rx_pin=0x%02X, tx_pin=0x%02X\n",
           uart, rx_pin, tx_pin);

    if (uart >= 4) {
        printf("[UART] ERROR: Invalid UART port %d\n", uart);
        return ERROR_UART_INVALID_PORT;
    }

    uart_channels[uart].rx_pin = rx_pin;
    uart_channels[uart].tx_pin = tx_pin;

    uart_channels[uart].rx_enabled = (rx_pin != NO_PIN);
    uart_channels[uart].tx_enabled = (tx_pin != NO_PIN);

    printf("[UART] UART%d: Pins bound successfully\n", uart);
    printf("[UART]   RX pin: 0x%02X (%s)\n", rx_pin, (rx_pin != NO_PIN) ? "enabled" : "disabled");
    printf("[UART]   TX pin: 0x%02X (%s)\n", tx_pin, (tx_pin != NO_PIN) ? "enabled" : "disabled");

    return NO_ERROR;
}

uint8_t platform_uart_get_rx_pin(platform_uart_t uart) {
    if (uart >= 4) {
        printf("[UART] ERROR: Invalid UART port %d\n", uart);
        return NO_PIN;
    }

    uint8_t pin = uart_channels[uart].rx_pin;
    printf("[UART] platform_uart_get_rx_pin: uart=%d, pin=0x%02X\n", uart, pin);

    return pin;
}

uint8_t platform_uart_get_tx_pin(platform_uart_t uart) {
    if (uart >= 4) {
        printf("[UART] ERROR: Invalid UART port %d\n", uart);
        return NO_PIN;
    }

    uint8_t pin = uart_channels[uart].tx_pin;
    printf("[UART] platform_uart_get_tx_pin: uart=%d, pin=0x%02X\n", uart, pin);

    return pin;
}

bool platform_uart_is_bound(platform_uart_t uart) {
    if (uart >= 4) {
        printf("[UART] ERROR: Invalid UART port %d\n", uart);
        return false;
    }

    bool bound = (uart_channels[uart].rx_pin != NO_PIN) || (uart_channels[uart].tx_pin != NO_PIN);
    printf("[UART] platform_uart_is_bound: uart=%d, bound=%s\n", uart, bound ? "true" : "false");

    return bound;
}

bool platform_uart_is_rx_enabled(platform_uart_t uart) {
    if (uart >= 4) {
        printf("[UART] ERROR: Invalid UART port %d\n", uart);
        return false;
    }

    bool enabled = uart_channels[uart].rx_enabled;
    printf("[UART] platform_uart_is_rx_enabled: uart=%d, enabled=%s\n", uart, enabled ? "true" : "false");

    return enabled;
}

bool platform_uart_is_tx_enabled(platform_uart_t uart) {
    if (uart >= 4) {
        printf("[UART] ERROR: Invalid UART port %d\n", uart);
        return false;
    }

    bool enabled = uart_channels[uart].tx_enabled;
    printf("[UART] platform_uart_is_tx_enabled: uart=%d, enabled=%s\n", uart, enabled ? "true" : "false");

    return enabled;
}

// 测试函数
int main(void) {
    printf("=== Simple UART Test ===\n\n");

    // Test 1: Bind pins
    printf("Test 1: Bind pins\n");
    platform_uart_bind_pins(0x01, 0x02, PLATFORM_UART_0);
    printf("\n");

    // Test 2: Initialize
    printf("Test 2: Initialize UART\n");
    platform_uart_init(PLATFORM_UART_0, 115200, 1);
    printf("\n");

    // Test 3: Write byte
    printf("Test 3: Send byte\n");
    platform_uart_write_byte(PLATFORM_UART_0, 'A');
    printf("\n");

    // Test 4: Write data
    printf("Test 4: Send data\n");
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    platform_uart_write(PLATFORM_UART_0, data, 5);
    printf("\n");

    // Test 5: Query status
    printf("Test 5: Query status\n");
    platform_uart_is_bound(PLATFORM_UART_0);
    platform_uart_is_rx_enabled(PLATFORM_UART_0);
    platform_uart_is_tx_enabled(PLATFORM_UART_0);
    platform_uart_get_rx_pin(PLATFORM_UART_0);
    platform_uart_get_tx_pin(PLATFORM_UART_0);
    printf("\n");

    // Test 6: Close
    printf("Test 6: Close UART\n");
    platform_uart_close(PLATFORM_UART_0);
    printf("\n");

    printf("=== Test Complete ===\n");

    return 0;
}
