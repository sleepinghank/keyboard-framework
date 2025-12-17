#include <stdio.h>
#include "hal/uart.h"
#include "application/sys_config.h"
#include "keyboards/pin_defs.h"
#include "hal/pin_defs.h"
#include "application/sys_error.h"

// 简单的接收回调函数
void uart_rx_callback_demo(uint8_t *data, uint16_t len) {
    printf("[RX Callback] Received %d bytes: ", len);
    for (int i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

int main(void) {
    printf("=== UART Test Demo ===\n\n");

    // 测试1: 绑定引脚
    printf("Test 1: Bind pins\n");
    platform_uart_bind_pins(PIN_A0, PIN_A1, PLATFORM_UART_0);
    printf("\n");

    // 测试2: 初始化UART
    printf("Test 2: Initialize UART\n");
    platform_uart_init(PLATFORM_UART_0, 115200, 1);
    printf("\n");

    // 测试3: 注册接收回调
    printf("Test 3: Register RX callback\n");
    platform_uart_register_rx_callback(PLATFORM_UART_0, uart_rx_callback_demo);
    printf("\n");

    // 测试4: 发送单个字节
    printf("Test 4: Send single byte\n");
    platform_uart_write_byte(PLATFORM_UART_0, 'A');
    printf("\n");

    // 测试5: 发送字符串
    printf("Test 5: Send string\n");
    uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    platform_uart_write(PLATFORM_UART_0, test_data, sizeof(test_data));
    printf("\n");

    // 测试6: 查询状态
    printf("Test 6: Query status\n");
    platform_uart_is_bound(PLATFORM_UART_0);
    platform_uart_is_rx_enabled(PLATFORM_UART_0);
    platform_uart_is_tx_enabled(PLATFORM_UART_0);
    platform_uart_get_rx_pin(PLATFORM_UART_0);
    platform_uart_get_tx_pin(PLATFORM_UART_0);
    printf("\n");

    // 测试7: 测试多个UART端口
    printf("Test 7: Test multiple UART ports\n");
    platform_uart_bind_pins(PIN_B0, PIN_B1, PLATFORM_UART_1);
    platform_uart_init(PLATFORM_UART_1, 9600, 0);
    platform_uart_write_byte(PLATFORM_UART_1, 'B');
    printf("\n");

    // 测试8: 关闭UART
    printf("Test 8: Close UART\n");
    platform_uart_close(PLATFORM_UART_0);
    printf("\n");

    // 测试9: 错误处理测试
    printf("Test 9: Error handling\n");
    platform_uart_init(PLATFORM_UART_5, 115200, 0);  // 无效端口
    platform_uart_write_byte(PLATFORM_UART_5, 'X');  // 未初始化端口
    printf("\n");

    printf("=== UART Test Complete ===\n");

    return 0;
}