# CH584 UART HAL 使用说明

## 概述

本文档描述了如何在CH584平台上使用键盘框架的UART HAL接口。所有接口都基于CH584标准外设库实现，支持4个UART通道（UART0-UART3），并提供灵活的GPIO引脚绑定机制。

## 头文件包含

```c
#include "uart.h"
#include "pin_defs.h"
```

## 支持的UART通道

```c
typedef enum {
    PLATFORM_UART_0 = 0,  // UART0
    PLATFORM_UART_1,      // UART1
    PLATFORM_UART_2,      // UART2
    PLATFORM_UART_3       // UART3
} platform_uart_t;
```

## UART引脚映射

### UART0
- **默认引脚**：RXD0/PB4, TXD0/PB7
- **备选引脚**：RXD0_/PA15, TXD0_/PA14

### UART1
- **默认引脚**：RXD1/PA8, TXD1/PA9
- **备选引脚**：RXD1_/PB12, TXD1_/PB13

### UART2
- **默认引脚**：RXD2/PA6, TXD2/PA7
- **备选引脚**：RXD2_/PB22, TXD2_/PB23

### UART3
- **默认引脚**：RXD3/PA4, TXD3/PA5
- **备选引脚**：RXD3_/PB20, TXD3_/PB21

## 核心API

### 1. 引脚绑定

```c
/**
 * @brief 绑定GPIO引脚到UART信道
 *
 * @param rx_pin RX引脚编号（如果为NO_PIN则禁用接收）
 * @param tx_pin TX引脚编号（如果为NO_PIN则禁用发送）
 * @param uart UART通道号
 * @return error_code_t 错误码
 */
error_code_t platform_uart_bind_pins(pin_t rx_pin, pin_t tx_pin, platform_uart_t uart);
```

**示例用法**：
```c
// 启用完整RX/TX功能
platform_uart_bind_pins(A0, A1, PLATFORM_UART_0);  // UART0使用PA0(RX)和PA1(TX)

// 仅启用TX功能（禁用RX）
platform_uart_bind_pins(NO_PIN, A1, PLATFORM_UART_0);  // UART0仅使用PA1(TX)

// 仅启用RX功能（禁用TX）
platform_uart_bind_pins(A0, NO_PIN, PLATFORM_UART_0);  // UART0仅使用PA0(RX)
```

### 2. UART初始化

```c
/**
 * @brief 平台UART初始化
 *
 * @param uart UART通道号
 * @param baudrate 波特率
 * @param enable_interrupt 是否启用中断
 * @return error_code_t 错误码
 */
error_code_t platform_uart_init(platform_uart_t uart, uint32_t baudrate, uint8_t enable_interrupt);
```

**示例用法**：
```c
// 初始化UART0，波特率115200，启用中断
platform_uart_init(PLATFORM_UART_0, 115200, 1);

// 初始化UART1，波特率9600，不启用中断
platform_uart_init(PLATFORM_UART_1, 9600, 0);
```

### 3. 数据发送

```c
// 发送单个字节
error_code_t platform_uart_write_byte(platform_uart_t uart, uint8_t data);

// 发送多个字节
error_code_t platform_uart_write(platform_uart_t uart, uint8_t *data, uint16_t len);
```

**示例用法**：
```c
// 发送单个字节
platform_uart_write_byte(PLATFORM_UART_0, 'A');

// 发送字符串
uint8_t msg[] = "Hello UART!";
platform_uart_write(PLATFORM_UART_0, msg, strlen((char*)msg));

// 发送二进制数据
uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
platform_uart_write(PLATFORM_UART_0, data, 4);
```

### 4. 接收回调

```c
/**
 * @brief 注册UART接收回调函数
 *
 * @param uart UART通道号
 * @param callback 回调函数指针
 * @return error_code_t 错误码
 *
 * @note 回调函数中不能做耗时操作，否则会影响串口接收
 */
error_code_t platform_uart_register_rx_callback(platform_uart_t uart, uart_rx_callback_t callback);
```

**示例用法**：
```c
// 定义接收回调函数
void uart0_rx_callback(uint8_t *data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        // 处理接收到的数据
        putchar(data[i]);
    }
}

// 注册回调函数
platform_uart_register_rx_callback(PLATFORM_UART_0, uart0_rx_callback);
```

### 5. 关闭UART

```c
error_code_t platform_uart_close(platform_uart_t uart);
```

**示例用法**：
```c
platform_uart_close(PLATFORM_UART_0);
```

### 6. 查询函数

```c
// 获取绑定的RX引脚
pin_t platform_uart_get_rx_pin(platform_uart_t uart);

// 获取绑定的TX引脚
pin_t platform_uart_get_tx_pin(platform_uart_t uart);

// 检查是否已绑定
bool platform_uart_is_bound(platform_uart_t uart);

// 检查RX是否启用
bool platform_uart_is_rx_enabled(platform_uart_t uart);

// 检查TX是否启用
bool platform_uart_is_tx_enabled(platform_uart_t uart);
```

## 使用示例

### 示例1：基本UART通信（轮询模式）

```c
#include "uart.h"
#include "pin_defs.h"

void uart_basic_example(void) {
    // 1. 绑定引脚
    platform_uart_bind_pins(B4, B7, PLATFORM_UART_0);  // UART0使用默认引脚

    // 2. 初始化UART（不启用中断）
    platform_uart_init(PLATFORM_UART_0, 115200, 0);

    // 3. 发送数据
    uint8_t tx_data[] = "Hello World!\r\n";
    platform_uart_write(PLATFORM_UART_0, tx_data, strlen((char*)tx_data));

    // 4. 轮询接收数据
    uint8_t rx_data[100];
    uint16_t rx_len = 0;

    // 这里需要实现轮询接收逻辑
    // 注意：CH58x_uart.h提供了UARTx_RecvByte()宏来读取单字节
    // 但需要自行实现超时和缓冲管理
}
```

### 示例2：中断驱动UART通信

```c
#include "uart.h"
#include "pin_defs.h"

#define RX_BUFFER_SIZE 256

// 接收缓冲区
static uint8_t rx_buffer[RX_BUFFER_SIZE];
static uint16_t rx_write_index = 0;

// UART0接收回调
void uart0_rx_callback(uint8_t *data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        // 将数据写入缓冲区
        if (rx_write_index < RX_BUFFER_SIZE) {
            rx_buffer[rx_write_index++] = data[i];
        }
    }
}

void uart_interrupt_example(void) {
    // 1. 绑定引脚
    platform_uart_bind_pins(B4, B7, PLATFORM_UART_0);

    // 2. 注册接收回调
    platform_uart_register_rx_callback(PLATFORM_UART_0, uart0_rx_callback);

    // 3. 初始化UART（启用中断）
    platform_uart_init(PLATFORM_UART_0, 115200, 1);

    // 4. 发送数据
    platform_uart_write_byte(PLATFORM_UART_0, 'A');

    // 主循环中处理接收数据
    while (1) {
        if (rx_write_index > 0) {
            // 处理接收到的数据
            // ...

            // 清空缓冲区（根据实际需要）
            rx_write_index = 0;
        }
    }
}
```

### 示例3：仅TX模式（调试打印）

```c
#include "uart.h"
#include "pin_defs.h"

void uart_debug_example(void) {
    // 仅绑定TX引脚（调试用）
    platform_uart_bind_pins(NO_PIN, A1, PLATFORM_UART_0);

    // 初始化UART
    platform_uart_init(PLATFORM_UART_0, 115200, 0);

    // 发送调试信息
    platform_uart_write_byte(PLATFORM_UART_0, 'D');
    platform_uart_write_byte(PLATFORM_UART_0, 'B');
    platform_uart_write_byte(PLATFORM_UART_0, 'G');
}
```

### 示例4：多UART通道

```c
#include "uart.h"
#include "pin_defs.h"

void uart_multi_example(void) {
    // UART0: 调试串口（仅TX）
    platform_uart_bind_pins(NO_PIN, A1, PLATFORM_UART_0);
    platform_uart_init(PLATFORM_UART_0, 115200, 0);

    // UART1: 主通信串口（RX+TX）
    platform_uart_bind_pins(B12, B13, PLATFORM_UART_1);
    platform_uart_register_rx_callback(PLATFORM_UART_1, uart1_rx_callback);
    platform_uart_init(PLATFORM_UART_1, 9600, 1);

    // UART2: 辅助串口（仅RX）
    platform_uart_bind_pins(PA6, NO_PIN, PLATFORM_UART_2);
    platform_uart_register_rx_callback(PLATFORM_UART_2, uart2_rx_callback);
    platform_uart_init(PLATFORM_UART_2, 38400, 1);

    // 发送数据
    platform_uart_write_byte(PLATFORM_UART_0, 'X');  // 调试
    platform_uart_write_byte(PLATFORM_UART_1, 'Y');  // 主通信
}
```

## 错误处理

所有函数返回`error_code_t`类型错误码：

```c
typedef enum {
    NO_ERROR = 0,                    // 无错误
    ERROR_UART_INVALID_PORT = 0xC0B5, // UART端口号无效
    // ... 其他错误码
} error_code_t;
```

**示例错误处理**：
```c
error_code_t ret = platform_uart_init(PLATFORM_UART_0, 115200, 1);
if (ret != NO_ERROR) {
    switch (ret) {
        case ERROR_UART_INVALID_PORT:
            // 处理无效端口错误
            break;
        default:
            // 处理其他错误
            break;
    }
}
```

## 注意事项

### 1. NO_PIN处理

- 当`rx_pin`为`NO_PIN`时，RX功能将被禁用
- 当`tx_pin`为`NO_PIN`时，TX功能将被禁用
- 至少需要一个有效引脚才能完成绑定

### 2. 中断使用

- 中断处理函数中避免耗时操作
- 回调函数中不要使用阻塞操作
- 如果仅使用TX功能，可以不启用中断

### 3. 引脚映射

- CH584的UART支持多组引脚，自动检测并配置
- 使用默认引脚无需额外配置
- 使用备选引脚时会自动启用引脚重映射

### 4. 波特率配置

- 支持标准波特率：9600, 19200, 38400, 57600, 115200等
- 自定义波特率需要确保FREQ_SYS时钟正确配置

### 5. FIFO配置

- 默认启用16字节FIFO
- 触发点设置为4字节
- 根据RX/TX使能情况自动配置FIFO

### 6. 线程安全

- 回调函数可能在中断上下文中执行
- 多线程环境下需要自行实现同步机制

## API参考

| 函数名 | 功能描述 |
|--------|----------|
| `platform_uart_bind_pins()` | 绑定GPIO引脚到UART |
| `platform_uart_init()` | 初始化UART |
| `platform_uart_close()` | 关闭UART |
| `platform_uart_write_byte()` | 发送单字节 |
| `platform_uart_write()` | 发送多字节 |
| `platform_uart_register_rx_callback()` | 注册接收回调 |
| `platform_uart_get_rx_pin()` | 获取RX引脚 |
| `platform_uart_get_tx_pin()` | 获取TX引脚 |
| `platform_uart_is_bound()` | 检查绑定状态 |
| `platform_uart_is_rx_enabled()` | 检查RX使能 |
| `platform_uart_is_tx_enabled()` | 检查TX使能 |

## 中断向量

| UART通道 | 中断向量 | 处理函数 |
|----------|----------|----------|
| UART0 | UART0_IRQn | UART0_IRQHandler() |
| UART1 | UART1_IRQn | UART1_IRQHandler() |
| UART2 | UART2_IRQn | UART2_IRQHandler() |
| UART3 | UART3_IRQn | UART3_IRQHandler() |

## 寄存器配置

- **LCR**：线路控制寄存器（8N1模式）
- **FCR**：FIFO控制寄存器（16字节FIFO，4字节触发）
- **IER**：中断使能寄存器
- **DIV**：分频器（默认1）

## 性能优化

1. **批量发送**：使用`platform_uart_write()`而非多次调用`platform_uart_write_byte()`
2. **中断触发**：根据数据量调整触发点（1/2/4/7字节）
3. **缓冲区管理**：自定义接收缓冲区避免数据丢失
4. **DMA传输**：对于大数据量，可考虑使用DMA（需要自行实现）
