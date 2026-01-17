# CH584 GPIO HAL 使用说明

## 概述

本文档描述了如何在CH584平台上使用键盘框架的GPIO HAL接口。所有接口都基于CH584标准外设库实现，提供统一的GPIO操作抽象。

## 头文件包含

```c
#include "gpio.h"
#include "pin_defs.h"
```

## 引脚定义

CH584平台的引脚使用以下格式定义：

```c
// PORTA 引脚
#define A0  ((PORTA << 6) | 0)   // PA0
#define A1  ((PORTA << 6) | 1)   // PA1
#define A2  ((PORTA << 6) | 2)   // PA2
// ... 一直到 A23

// PORTB 引脚
#define B0  ((PORTB << 6) | 0)   // PB0
#define B1  ((PORTB << 6) | 1)   // PB1
#define B2  ((PORTB << 6) | 2)   // PB2
// ... 一直到 B23
```

## GPIO模式配置

### 1. 输入模式

```c
// 浮空输入（默认输入模式）
gpio_set_pin_input(A0);

// 上拉输入（内部上拉电阻使能）
gpio_set_pin_input_high(A0);

// 下拉输入（内部下拉电阻使能）
gpio_set_pin_input_low(A0);
```

### 2. 输出模式

```c
// 推挽输出（默认输出模式，5mA驱动能力）
gpio_set_pin_output_push_pull(A0);

// 通用输出模式设置（默认推挽输出）
gpio_set_pin_output(A0);

// 开漏输出（CH584不支持，模拟实现）
// 注意：使用开漏模式需要在外部连接上拉电阻
gpio_set_pin_output_open_drain(A0);
```

## GPIO电平控制

```c
// 设置为高电平
gpio_write_pin_high(A0);

// 设置为低电平
gpio_write_pin_low(A0);

// 根据电平值设置引脚
gpio_write_pin(A0, 1);  // 设置为高电平
gpio_write_pin(A0, 0);  // 设置为低电平

// 翻转引脚电平
gpio_toggle_pin(A0);
```

## GPIO电平读取

```c
// 读取引脚电平状态
uint8_t level = gpio_read_pin(A0);
// 返回值：0（低电平）或 1（高电平）
```

## 使用示例

### 示例1：LED控制

```c
#include "gpio.h"
#include "pin_defs.h"

void led_init(void) {
    // 设置LED引脚为推挽输出模式
    gpio_set_pin_output_push_pull(B0);
    // 初始状态为熄灭
    gpio_write_pin_low(B0);
}

void led_toggle(void) {
    gpio_toggle_pin(B0);
}

void led_on(void) {
    gpio_write_pin_high(B0);
}

void led_off(void) {
    gpio_write_pin_low(B0);
}
```

### 示例2：按键输入

```c
#include "gpio.h"
#include "pin_defs.h"

void key_init(void) {
    // 设置按键引脚为上拉输入模式
    gpio_set_pin_input_high(A0);
}

bool key_pressed(void) {
    // 按键按下时为低电平
    return (gpio_read_pin(A0) == 0);
}
```

### 示例3：矩阵键盘行扫描

```c
#include "gpio.h"
#include "pin_defs.h"

void matrix_row_scan(uint8_t row) {
    // 设置当前行为输出低电平
    gpio_write_pin_low(row_pins[row]);

    // 读取所有列
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        uint8_t state = gpio_read_pin(col_pins[col]);
        // 处理按键状态
    }

    // 设置当前行为输入（释放）
    gpio_set_pin_input(row_pins[row]);
}
```

## 注意事项

### 1. CH584 GPIO特性

- **推挽输出**：支持5mA和20mA两种驱动能力，默认使用5mA
- **输入模式**：支持浮空、上拉、下拉三种输入模式
- **开漏输出**：CH584不支持真正的开漏输出，`gpio_set_pin_output_open_drain()` 会使用浮空输入模拟，实际使用需要在外部连接上拉电阻

### 2. 引脚编号范围

- **PORTA**：PA0 ~ PA23（实际可用引脚取决于封装）
- **PORTB**：PB0 ~ PB23（实际可用引脚取决于封装）

### 3. 端口判断

内部使用以下宏判断引脚所属端口：
```c
#define GET_GPIO_PORT(gpio) (((gpio) & GPIO_PORT_MASK) >> 6)
#define GET_GPIO_PIN(gpio) ((gpio) & GPIO_PIN_MASK)
```

### 4. 编译配置

确保在CMakeLists.txt中已添加：
- CH584头文件路径：`project/ch584m/StdPeriphDriver/inc`
- CH584源文件路径：`hal/platforms/ch584/_gpio.c`

## API参考

### 函数列表

| 函数名 | 功能描述 |
|--------|----------|
| `gpio_set_pin_input(pin_t pin)` | 设置引脚为浮空输入模式 |
| `gpio_set_pin_input_high(pin_t pin)` | 设置引脚为上拉输入模式 |
| `gpio_set_pin_input_low(pin_t pin)` | 设置引脚为下拉输入模式 |
| `gpio_set_pin_output_push_pull(pin_t pin)` | 设置引脚为推挽输出模式（5mA） |
| `gpio_set_pin_output_open_drain(pin_t pin)` | 设置引脚为开漏输出模式（模拟） |
| `gpio_set_pin_output(pin_t pin)` | 设置引脚为输出模式（默认推挽） |
| `gpio_write_pin_high(pin_t pin)` | 设置引脚为高电平 |
| `gpio_write_pin_low(pin_t pin)` | 设置引脚为低电平 |
| `gpio_write_pin(pin_t pin, uint8_t level)` | 根据电平值设置引脚 |
| `gpio_read_pin(pin_t pin)` | 读取引脚电平状态 |
| `gpio_toggle_pin(pin_t pin)` | 翻转引脚电平状态 |

### 参数说明

- `pin_t pin`：引脚编号，格式为 `(port << 6) | pin_number`
- `uint8_t level`：电平值，0表示低电平，1表示高电平
- `uint8_t`：返回值，0表示低电平，1表示高电平