# CH584 I2C HAL 使用说明

## 概述

本文档描述了如何在CH584平台上使用键盘框架的I2C HAL接口。所有接口都基于CH584标准外设库实现，支持I2C主模式操作，并提供灵活的GPIO引脚绑定机制。

## 头文件包含

```c
#include "i2c_master.h"
#include "pin_defs.h"
```

## 支持的I2C通道

```c
typedef enum {
    I2C_CHANNEL_0 = 0,  /* I2C通道0 */
    I2C_CHANNEL_1 = 1,  /* I2C通道1 */
    I2C_CHANNEL_2 = 2,  /* I2C通道2 */
    I2C_CHANNEL_3 = 3,  /* I2C通道3 */
    I2C_CHANNEL_MAX = 4 /* 最大通道数 */
} i2c_channel_t;
```

**注意**：CH584芯片实际只有1个I2C控制器，多通道支持为未来扩展预留。

## I2C引脚映射

### CH584默认I2C引脚
- **SDA**: PA13 / PB14（可通过GPIO重映射）
- **SCL**: PA14 / PB15（可通过GPIO重映射）

**注意**：实际引脚映射需要根据具体硬件设计确定，I2C支持多组引脚映射。

## 核心API

### 1. 引脚绑定

```c
/**
 * @brief 绑定GPIO引脚到I2C信道
 * @param sda_pin SDA引脚号
 * @param scl_pin SCL引脚号
 * @param channel I2C通道号
 * @return i2c_status_t 状态码
 */
i2c_status_t i2c_bind_pins(pin_t sda_pin, pin_t scl_pin, i2c_channel_t channel);
```

**示例用法**：
```c
// 绑定引脚到I2C通道0
i2c_bind_pins(A13, A14, I2C_CHANNEL_0);

// 检查绑定状态
if (i2c_is_bound(I2C_CHANNEL_0)) {
    pin_t sda = i2c_get_sda_pin(I2C_CHANNEL_0);
    pin_t scl = i2c_get_scl_pin(I2C_CHANNEL_0);
}
```

### 2. I2C初始化

```c
/**
 * @brief 初始化I2C通道
 * @param channel I2C通道号
 */
void i2c_init_channel(i2c_channel_t channel);

/**
 * @brief 绑定引脚并初始化I2C通道
 * @param channel I2C通道号
 * @param sda_pin SDA引脚号
 * @param scl_pin SCL引脚号
 * @param clock_speed 时钟速度（Hz）
 * @return i2c_status_t 状态码
 */
i2c_status_t i2c_init_channel_with_pins(i2c_channel_t channel, pin_t sda_pin, pin_t scl_pin, uint32_t clock_speed);
```

**示例用法**：
```c
// 方法一：分步操作
i2c_init_channel(I2C_CHANNEL_0);
i2c_bind_pins(A13, A14, I2C_CHANNEL_0);

// 方法二：一步完成
i2c_init_channel_with_pins(I2C_CHANNEL_0, A13, A14, 400000);  // 400KHz
```

### 3. 基础I2C操作

```c
/**
 * @brief 启动I2C传输
 * @param channel I2C通道号
 * @param address 从设备地址
 * @param timeout 超时时间（ms）
 * @return I2C_STATUS_SUCCESS/ERROR/TIMEOUT
 */
i2c_status_t i2c_start_channel(i2c_channel_t channel, uint8_t address, uint16_t timeout);

/**
 * @brief 向I2C总线写入一个字节
 * @param channel I2C通道号
 * @param data 要写入的数据
 * @param timeout 超时时间（ms）
 * @return I2C_STATUS_SUCCESS/ERROR/TIMEOUT
 */
i2c_status_t i2c_write_channel(i2c_channel_t channel, uint8_t data, uint16_t timeout);

/**
 * @brief 从I2C总线读取一个字节并发送ACK
 * @param channel I2C通道号
 * @param timeout 超时时间（ms）
 * @return 读取的数据（0-255），-1表示错误
 */
int16_t i2c_read_ack_channel(i2c_channel_t channel, uint16_t timeout);

/**
 * @brief 从I2C总线读取一个字节并发送NACK
 * @param channel I2C通道号
 * @param timeout 超时时间（ms）
 * @return 读取的数据（0-255），-1表示错误
 */
int16_t i2c_read_nack_channel(i2c_channel_t channel, uint16_t timeout);

/**
 * @brief 停止I2C传输
 * @param channel I2C通道号
 */
void i2c_stop_channel(i2c_channel_t channel);
```

**示例用法**：
```c
// 读取从设备数据（基础操作）
i2c_start_channel(I2C_CHANNEL_0, 0x50, 1000);  // 启动，地址0x50
i2c_write_channel(I2C_CHANNEL_0, 0x00, 1000);  // 发送寄存器地址0x00
i2c_start_channel(I2C_CHANNEL_0, 0x50 | 0x01, 1000);  // 重新启动，读取模式

uint8_t data = i2c_read_ack_channel(I2C_CHANNEL_0, 1000);  // 读取第一个字节（ACK）
data = i2c_read_nack_channel(I2C_CHANNEL_0, 1000);  // 读取最后一个字节（NACK）

i2c_stop_channel(I2C_CHANNEL_0);  // 停止
```

### 4. 复合I2C操作

```c
/**
 * @brief 通过I2C传输多个字节
 * @param channel I2C通道号
 * @param address 从设备地址
 * @param data 要发送的数据缓冲区
 * @param length 数据长度
 * @param timeout 超时时间（ms）
 * @return I2C_STATUS_SUCCESS/ERROR/TIMEOUT
 */
i2c_status_t i2c_transmit_channel(i2c_channel_t channel, uint8_t address, const uint8_t* data, uint16_t length, uint16_t timeout);

/**
 * @brief 通过I2C接收多个字节
 * @param channel I2C通道号
 * @param address 从设备地址
 * @param data 接收数据缓冲区
 * @param length 要接收的数据长度
 * @param timeout 超时时间（ms）
 * @return I2C_STATUS_SUCCESS/ERROR/TIMEOUT
 */
i2c_status_t i2c_receive_channel(i2c_channel_t channel, uint8_t address, uint8_t* data, uint16_t length, uint16_t timeout);
```

### 5. 寄存器操作

```c
/**
 * @brief 向I2C设备的寄存器写入数据（8位寄存器地址）
 * @param channel I2C通道号
 * @param devaddr 从设备地址
 * @param regaddr 寄存器地址
 * @param data 要写入的数据
 * @param length 数据长度
 * @param timeout 超时时间（ms）
 * @return I2C_STATUS_SUCCESS/ERROR/TIMEOUT
 */
i2c_status_t i2c_writeReg_channel(i2c_channel_t channel, uint8_t devaddr, uint8_t regaddr, const uint8_t* data, uint16_t length, uint16_t timeout);

/**
 * @brief 向I2C设备的寄存器写入数据（16位寄存器地址）
 * @param channel I2C通道号
 * @param devaddr 从设备地址
 * @param regaddr 寄存器地址（16位）
 * @param data 要写入的数据
 * @param length 数据长度
 * @param timeout 超时时间（ms）
 * @return I2C_STATUS_SUCCESS/ERROR/TIMEOUT
 */
i2c_status_t i2c_writeReg16_channel(i2c_channel_t channel, uint8_t devaddr, uint16_t regaddr, const uint8_t* data, uint16_t length, uint16_t timeout);

/**
 * @brief 从I2C设备的寄存器读取数据（8位寄存器地址）
 * @param channel I2C通道号
 * @param devaddr 从设备地址
 * @param regaddr 寄存器地址
 * @param data 接收数据缓冲区
 * @param length 要读取的数据长度
 * @param timeout 超时时间（ms）
 * @return I2C_STATUS_SUCCESS/ERROR/TIMEOUT
 */
i2c_status_t i2c_readReg_channel(i2c_channel_t channel, uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length, uint16_t timeout);

/**
 * @brief 从I2C设备的寄存器读取数据（16位寄存器地址）
 * @param channel I2C通道号
 * @param devaddr 从设备地址
 * @param regaddr 寄存器地址（16位）
 * @param data 接收数据缓冲区
 * @param length 要读取的数据长度
 * @param timeout 超时时间（ms）
 * @return I2C_STATUS_SUCCESS/ERROR/TIMEOUT
 */
i2c_status_t i2c_readReg16_channel(i2c_channel_t channel, uint8_t devaddr, uint16_t regaddr, uint8_t* data, uint16_t length, uint16_t timeout);
```

### 6. 查询函数

```c
// 获取绑定的SDA引脚
pin_t i2c_get_sda_pin(i2c_channel_t channel);

// 获取绑定的SCL引脚
pin_t i2c_get_scl_pin(i2c_channel_t channel);

// 检查是否已绑定
bool i2c_is_bound(i2c_channel_t channel);
```

## 使用示例

### 示例1：I2C设备初始化

```c
#include "i2c_master.h"
#include "pin_defs.h"

void i2c_sensor_example(void) {
    // 1. 初始化I2C通道0
    i2c_init_channel_with_pins(I2C_CHANNEL_0, A13, A14, 400000);  // 400KHz

    // 2. 检查设备是否存在
    i2c_status_t status = i2c_start_channel(I2C_CHANNEL_0, 0x50, 1000);
    if (status == I2C_STATUS_SUCCESS) {
        // 设备存在
        i2c_stop_channel(I2C_CHANNEL_0);
    } else {
        // 设备不存在
        return;
    }
}
```

### 示例2：写入寄存器

```c
void i2c_write_register_example(void) {
    // 初始化I2C
    i2c_init_channel_with_pins(I2C_CHANNEL_0, A13, A14, 400000);

    // 向设备0x50的寄存器0x10写入数据
    uint8_t data[] = {0xAB, 0xCD};
    i2c_status_t status = i2c_writeReg_channel(I2C_CHANNEL_0, 0x50, 0x10, data, 2, 1000);

    if (status == I2C_STATUS_SUCCESS) {
        // 写入成功
    }
}
```

### 示例3：读取寄存器

```c
void i2c_read_register_example(void) {
    // 初始化I2C
    i2c_init_channel_with_pins(I2C_CHANNEL_0, A13, A14, 400000);

    // 从设备0x50的寄存器0x20读取2字节数据
    uint8_t buffer[2];
    i2c_status_t status = i2c_readReg_channel(I2C_CHANNEL_0, 0x50, 0x20, buffer, 2, 1000);

    if (status == I2C_STATUS_SUCCESS) {
        // 读取成功
        uint8_t value1 = buffer[0];
        uint8_t value2 = buffer[1];
    }
}
```

### 示例4：I2C设备扫描

```c
void i2c_scan_devices(void) {
    // 初始化I2C
    i2c_init_channel(I2C_CHANNEL_0);
    i2c_bind_pins(A13, A14, I2C_CHANNEL_0);

    // 扫描I2C总线上的设备
    printf("I2C Device Scan:\n");
    for (uint8_t addr = 1; addr < 127; addr++) {
        i2c_status_t status = i2c_start_channel(I2C_CHANNEL_0, addr, 100);
        if (status == I2C_STATUS_SUCCESS) {
            printf("  Found device at address 0x%02X\n", addr);
            i2c_stop_channel(I2C_CHANNEL_0);
        }
    }
}
```

### 示例5：EEPROM读写

```c
#define EEPROM_ADDR 0x50

void eeprom_write_example(void) {
    // 初始化I2C
    i2c_init_channel_with_pins(I2C_CHANNEL_0, A13, A14, 400000);

    // 向EEPROM写入数据（16位地址）
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint16_t eeprom_addr = 0x0010;  // EEPROM内部地址

    i2c_status_t status = i2c_writeReg16_channel(I2C_CHANNEL_0, EEPROM_ADDR, eeprom_addr, data, 5, 1000);

    if (status == I2C_STATUS_SUCCESS) {
        printf("EEPROM write successful\n");
    }
}

void eeprom_read_example(void) {
    // 初始化I2C
    i2c_init_channel_with_pins(I2C_CHANNEL_0, A13, A14, 400000);

    // 从EEPROM读取数据（16位地址）
    uint8_t buffer[5];
    uint16_t eeprom_addr = 0x0010;  // EEPROM内部地址

    i2c_status_t status = i2c_readReg16_channel(I2C_CHANNEL_0, EEPROM_ADDR, eeprom_addr, buffer, 5, 1000);

    if (status == I2C_STATUS_SUCCESS) {
        printf("EEPROM read successful: %02X %02X %02X %02X %02X\n",
               buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
    }
}
```

### 示例6：多字节传输

```c
void i2c_multi_byte_example(void) {
    // 初始化I2C
    i2c_init_channel_with_pins(I2C_CHANNEL_0, A13, A14, 400000);

    // 发送多字节数据
    uint8_t tx_data[] = {0xAA, 0xBB, 0xCC, 0xDD};
    i2c_status_t status = i2c_transmit_channel(I2C_CHANNEL_0, 0x60, tx_data, 4, 1000);

    if (status == I2C_STATUS_SUCCESS) {
        // 发送成功
    }

    // 接收多字节数据
    uint8_t rx_data[4];
    status = i2c_receive_channel(I2C_CHANNEL_0, 0x60, rx_data, 4, 1000);

    if (status == I2C_STATUS_SUCCESS) {
        // 接收成功
        for (int i = 0; i < 4; i++) {
            printf("Data[%d] = 0x%02X\n", i, rx_data[i]);
        }
    }
}
```

## 状态码

```c
typedef int16_t i2c_status_t;

#define I2C_STATUS_SUCCESS (0)    /* 成功 */
#define I2C_STATUS_ERROR (-1)     /* 错误 */
#define I2C_STATUS_TIMEOUT (-2)   /* 超时 */

#define I2C_TIMEOUT_IMMEDIATE (0)
#define I2C_TIMEOUT_INFINITE (0xFFFF)
```

## 注意事项

### 1. 引脚绑定

- I2C引脚必须配置为开漏输入（GPIO_ModeIN_PU）
- SDA和SCL引脚需要外部上拉电阻（通常为4.7KΩ或10KΩ）
- 避免在I2C总线上连接过多设备，以免造成信号衰减

### 2. 时钟速度

- 标准模式：100KHz
- 快速模式：400KHz
- CH584支持高达400KHz的I2C通信
- 选择合适的时钟速度取决于总线长度和设备数量

### 3. 超时设置

- 根据设备特性设置合适的超时时间
- 快速设备可以使用较短超时（如100ms）
- 慢速设备（如EEPROM）需要较长超时（如1000ms）

### 4. 错误处理

- 总是检查I2C操作返回值
- 出现错误时调用`i2c_stop_channel()`释放总线
- 总线忙时不要重复发起传输

### 5. 电气特性

- I2C总线电压：通常为3.3V或5V
- 上拉电阻大小影响上升时间，建议使用4.7KΩ
- 总线长度不超过30cm（标准模式）

### 6. 线程安全

- I2C操作不是线程安全的
- 多线程环境下需要自行实现同步机制
- 建议在中断上下文中避免I2C操作

## API参考

| 函数名 | 功能描述 |
|--------|----------|
| `i2c_init_channel()` | 初始化I2C通道 |
| `i2c_bind_pins()` | 绑定GPIO引脚到I2C通道 |
| `i2c_init_channel_with_pins()` | 绑定引脚并初始化I2C |
| `i2c_start_channel()` | 启动I2C传输 |
| `i2c_write_channel()` | 写入一个字节 |
| `i2c_read_ack_channel()` | 读取一个字节（ACK） |
| `i2c_read_nack_channel()` | 读取一个字节（NACK） |
| `i2c_stop_channel()` | 停止I2C传输 |
| `i2c_transmit_channel()` | 发送多个字节 |
| `i2c_receive_channel()` | 接收多个字节 |
| `i2c_writeReg_channel()` | 写入寄存器（8位地址） |
| `i2c_writeReg16_channel()` | 写入寄存器（16位地址） |
| `i2c_readReg_channel()` | 读取寄存器（8位地址） |
| `i2c_readReg16_channel()` | 读取寄存器（16位地址） |
| `i2c_get_sda_pin()` | 获取SDA引脚 |
| `i2c_get_scl_pin()` | 获取SCL引脚 |
| `i2c_is_bound()` | 检查绑定状态 |

## 底层API

本实现基于CH58x_i2c.h提供的底层API：

- `I2C_Init()` - 初始化I2C控制器
- `I2C_Cmd()` - 使能/禁用I2C
- `I2C_GenerateSTART()` - 生成START条件
- `I2C_GenerateSTOP()` - 生成STOP条件
- `I2C_Send7bitAddress()` - 发送7位地址
- `I2C_SendData()` - 发送数据
- `I2C_ReceiveData()` - 接收数据
- `I2C_AcknowledgeConfig()` - 配置ACK
- `I2C_CheckEvent()` - 检查I2C事件
- `I2C_GetFlagStatus()` - 获取标志状态

## 常见应用场景

1. **传感器读取**：使用i2c_readReg_channel读取温湿度、光强等传感器数据
2. **EEPROM存储**：使用i2c_readReg16_channel/i2c_writeReg16_channel读写配置数据
3. **I/O扩展**：控制I2C接口的GPIO扩展芯片
4. **显示驱动**：控制I2C接口的OLED、LCD显示屏
5. **电源管理**：控制I2C接口的电源管理芯片

## 故障排除

### 1. 设备无响应

- 检查I2C地址是否正确
- 检查SDA/SCL引脚连接
- 检查上拉电阻是否连接
- 检查设备电源是否正常

### 2. 数据错误

- 检查时钟速度是否过高
- 检查总线长度是否过长
- 检查是否存在信号干扰
- 检查设备是否支持当前时钟速度

### 3. 总线忙

- 检查是否有其他设备占用总线
- 调用i2c_stop_channel()释放总线
- 检查是否有多次START未匹配STOP
