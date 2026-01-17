# CH584 ADC HAL 使用说明

## 概述

本文档描述了如何在CH584平台上使用键盘框架的ADC HAL接口。所有接口都基于CH584标准外设库实现，支持单端、差分、温度传感器、TouchKey等多种ADC采样模式，并提供轮询、中断、DMA等多种工作模式。

## 头文件包含

```c
#include "adc.h"
#include "pin_defs.h"
```

## ADC通道映射

CH584芯片支持14个外部ADC通道：

```c
typedef enum {
    ADC_CHANNEL_0 = 0,   /* PA4,AIN0 */
    ADC_CHANNEL_1 = 1,   /* PA5,AIN1 */
    ADC_CHANNEL_2 = 2,   /* PA12,AIN2 */
    ADC_CHANNEL_3 = 3,   /* PA13,AIN3 */
    ADC_CHANNEL_4 = 4,   /* PA14,AIN4 */
    ADC_CHANNEL_5 = 5,   /* PA15,AIN5 */
    ADC_CHANNEL_6 = 6,   /* PA3,AIN6 */
    ADC_CHANNEL_7 = 7,   /* PA2,AIN7 */
    ADC_CHANNEL_8 = 8,   /* PA1,AIN8 */
    ADC_CHANNEL_9 = 9,   /* PA0,AIN9 */
    ADC_CHANNEL_10 = 10, /* PA6,AIN10 */
    ADC_CHANNEL_11 = 11, /* PA7,AIN11 */
    ADC_CHANNEL_12 = 12, /* PA8,AIN12 */
    ADC_CHANNEL_13 = 13, /* PA9,AIN13 */
    ADC_CHANNEL_MAX = 14
} adc_channel_t;
```

## 采样频率和增益

### 采样频率
```c
typedef enum {
    ADC_SAMPLE_FREQ_8MHZ = 0,  /* 8MHz采样频率 (或4MHz) */
    ADC_SAMPLE_FREQ_4MHZ = 1,  /* 4MHz采样频率 (或2MHz) */
} adc_sample_freq_t;
```

### 增益设置
```c
typedef enum {
    ADC_PGA_0 = 0,   /* 0dB增益 */
    ADC_PGA_6 = 1,   /* 6dB增益 */
    ADC_PGA_12 = 2,  /* 12dB增益 */
    ADC_PGA_18 = 3,  /* 18dB增益 */
} adc_pga_t;
```

## 核心API

### 1. 基础采样模式

#### 单端模式采样

```c
/**
 * @brief   初始化ADC模块（单端模式）
 * @param   channel ADC通道号
 * @param   freq 采样频率
 * @param   gain 增益设置
 * @return  none
 */
void adc_init_single(adc_channel_t channel, adc_sample_freq_t freq, adc_pga_t gain);
```

**示例用法**：
```c
// 初始化通道0，8MHz采样频率，0dB增益
adc_init_single(ADC_CHANNEL_0, ADC_SAMPLE_FREQ_8MHZ, ADC_PGA_0);

// 启动单次转换
uint16_t value = adc_start_single_conversion();

// 读取平均值（20次采样）
uint16_t avg = adc_read_average(ADC_CHANNEL_0, 20);

// 读取电压值（毫伏）
uint32_t voltage = adc_read_voltage(ADC_CHANNEL_0);
```

#### 差分模式采样

```c
/**
 * @brief   初始化ADC模块（差分模式）
 * @param   channel ADC通道号（正输入通道）
 * @param   freq 采样频率
 * @param   gain 增益设置
 * @return  none
 */
void adc_init_diff(adc_channel_t channel, adc_sample_freq_t freq, adc_pga_t gain);
```

**示例用法**：
```c
// 初始化差分模式（通道0对应PA4(AIN0)、PA12(AIN2)）
adc_init_diff(ADC_CHANNEL_0, ADC_SAMPLE_FREQ_8MHZ, ADC_PGA_0);

// 执行转换
uint16_t value = adc_start_single_conversion();
```

#### 温度传感器采样

```c
/**
 * @brief   初始化温度传感器采样
 * @return  none
 */
void adc_init_temperature(void);
```

**示例用法**：
```c
// 初始化温度传感器
adc_init_temperature();

// 执行转换（丢弃首次数据）
adc_start_single_conversion();

// 读取温度值
for (int i = 0; i < 20; i++) {
    uint16_t value = adc_start_single_conversion();
    int temperature = adc_to_temperature_celsius(value);
    printf("Temperature: %d°C, ADC: %d\n", temperature, value);
}
```

#### TouchKey采样

```c
/**
 * @brief   初始化TouchKey采样
 * @return  none
 */
void adc_init_touchkey(void);

/**
 * @brief   启动TouchKey单次转换
 * @param   charge_time 充电时间
 * @param   discharge_time 放电时间
 * @return  TouchKey采样值
 */
uint16_t adc_start_touchkey_conversion(uint8_t charge_time, uint8_t discharge_time);
```

**示例用法**：
```c
// 初始化TouchKey
adc_init_touchkey();

// 配置通道
adc_configure_gpio_pin(ADC_CHANNEL_2);  // 使用PA12

// 执行TouchKey转换
uint16_t touch_value = adc_start_touchkey_conversion(0x10, 0);
```

### 2. 校准功能

```c
/**
 * @brief   ADC数据校准
 * @return  校准值（用于修正ADC偏差）
 */
int16_t adc_calibrate(void);
```

**示例用法**：
```c
// 初始化ADC
adc_init_single(ADC_CHANNEL_0, ADC_SAMPLE_FREQ_8MHZ, ADC_PGA_0);

// 执行校准
int16_t calib_value = adc_calibrate();
printf("Calibration value: %d\n", calib_value);

// 校准后的采样值会自动应用校准值
uint16_t value = adc_start_single_conversion();
```

### 3. 中断模式

```c
/**
 * @brief   配置ADC中断模式
 * @param   channel ADC通道号
 * @param   callback 转换完成回调函数
 * @return  none
 */
void adc_config_interrupt(adc_channel_t channel, adc_callback_t callback);

/**
 * @brief   启动ADC中断转换
 * @return  none
 */
void adc_start_interrupt(void);

/**
 * @brief   停止ADC中断转换
 * @return  none
 */
void adc_stop_interrupt(void);
```

**示例用法**：
```c
// 中断回调函数
void adc_callback(uint16_t value) {
    printf("ADC value: %d\n", value);
}

void interrupt_example(void) {
    // 配置中断模式
    adc_config_interrupt(ADC_CHANNEL_1, adc_callback);

    // 启动中断转换
    adc_start_interrupt();

    // 等待采样完成（或者在主循环中检查）
    while (g_adc_sample_count < 20) {
        // 等待...
    }

    // 停止中断
    adc_stop_interrupt();
}
```

### 4. DMA模式

```c
/**
 * @brief   配置ADC DMA模式
 * @param   buffer 接收缓冲区
 * @param   size 缓冲区大小
 * @return  none
 */
void adc_config_dma(uint16_t* buffer, uint16_t size);

/**
 * @brief   启动ADC DMA传输
 * @return  none
 */
void adc_start_dma(void);

/**
 * @brief   停止ADC DMA传输
 * @return  none
 */
void adc_stop_dma(void);

/**
 * @brief   检查DMA状态
 * @return  true表示DMA传输完成
 */
bool adc_get_dma_status(void);
```

**示例用法**：
```c
#define DMA_BUFFER_SIZE 40
uint16_t adc_buffer[DMA_BUFFER_SIZE];

void dma_example(void) {
    // 配置DMA
    adc_init_single(ADC_CHANNEL_0, ADC_SAMPLE_FREQ_8MHZ, ADC_PGA_0);
    ADC_ExcutSingleConver();  // 丢弃首次转换
    ADC_AutoConverCycle(192); // 设置采样周期

    adc_config_dma(adc_buffer, DMA_BUFFER_SIZE);

    // 启动DMA传输
    adc_start_dma();

    // 等待DMA完成
    while (!adc_get_dma_status()) {
        // 等待...
    }

    // 输出结果
    for (int i = 0; i < DMA_BUFFER_SIZE; i++) {
        printf("%d\n", adc_buffer[i]);
    }

    // 停止DMA
    adc_stop_dma();
}
```

### 5. 扫描模式（多通道自动采样）

```c
/**
 * @brief   配置ADC自动扫描模式
 * @param   channels 通道数组
 * @param   num_channels 通道数量
 * @param   buffer 接收缓冲区
 * @param   buffer_size 缓冲区大小
 * @return  none
 */
void adc_config_scan(const adc_channel_t* channels, uint8_t num_channels, uint16_t* buffer, uint16_t buffer_size);

/**
 * @brief   启动ADC扫描
 * @return  none
 */
void adc_start_scan(void);

/**
 * @brief   停止ADC扫描
 * @return  none
 */
void adc_stop_scan(void);
```

**示例用法**：
```c
#define SCAN_CHANNELS 14
adc_channel_t scan_channels[SCAN_CHANNELS] = {
    ADC_CHANNEL_0, ADC_CHANNEL_1, ADC_CHANNEL_2, ADC_CHANNEL_3,
    ADC_CHANNEL_4, ADC_CHANNEL_5, ADC_CHANNEL_6, ADC_CHANNEL_7,
    ADC_CHANNEL_8, ADC_CHANNEL_9, ADC_CHANNEL_10, ADC_CHANNEL_11,
    ADC_CHANNEL_12, ADC_CHANNEL_13
};
uint16_t scan_buffer[SCAN_CHANNELS];

void scan_example(void) {
    // 配置扫描模式
    adc_config_scan(scan_channels, SCAN_CHANNELS, scan_buffer, SCAN_CHANNELS);

    // 启动扫描
    adc_start_scan();

    // 等待扫描完成
    while (!adc_get_dma_status()) {
        // 等待...
    }

    // 输出结果
    for (int i = 0; i < SCAN_CHANNELS; i++) {
        printf("Channel %d: %d\n", i, scan_buffer[i]);
    }
}
```

### 6. GPIO引脚绑定

```c
/**
 * @brief   绑定GPIO引脚到ADC通道
 * @param   pin GPIO引脚号
 * @param   channel ADC通道号
 * @return  bool 绑定是否成功
 */
bool adc_bind_pin(pin_t pin, adc_channel_t channel);

/**
 * @brief   获取ADC通道绑定的GPIO引脚
 * @param   channel ADC通道号
 * @return  GPIO引脚号（如果未绑定则返回NO_PIN）
 */
pin_t adc_get_bound_pin(adc_channel_t channel);

/**
 * @brief   检查ADC通道是否已绑定GPIO引脚
 * @param   channel ADC通道号
 * @return  true表示已绑定，false表示未绑定
 */
bool adc_is_bound(adc_channel_t channel);
```

**示例用法**：
```c
// 绑定引脚
adc_bind_pin(A4, ADC_CHANNEL_0);  // PA4绑定到通道0

// 检查绑定状态
if (adc_is_bound(ADC_CHANNEL_0)) {
    pin_t pin = adc_get_bound_pin(ADC_CHANNEL_0);
    printf("Channel 0 is bound to pin %d\n", pin);
}
```

## 便捷宏和快捷函数

### 便捷宏

```c
/* 便捷宏 - 默认使用通道0 */
#define adc_init()              adc_init_single(ADC_CHANNEL_0, ADC_SAMPLE_FREQ_8MHZ, ADC_PGA_0)
#define adc_read()              adc_start_single_conversion()
#define adc_read_average(samples) adc_read_average(ADC_CHANNEL_0, (samples))
#define adc_read_voltage()      adc_read_voltage(ADC_CHANNEL_0)

/* 快速读取宏 */
#define ADC_READ(channel)               adc_start_single_conversion()
#define ADC_READ_AVG(channel, samples)  adc_read_average(channel, samples)
#define ADC_READ_VOLT(channel)          adc_read_voltage(channel)
```

### 转换宏

```c
/* 温度转换宏 */
#define adc_to_temperature_celsius(value) ((int)((int32_t)(value) - *((PUINT32)ROM_CFG_TMP_25C)) / 118)

/* 电压转换宏 - 假设3.3V参考电压，12位ADC */
#define ADC_TO_VOLTAGE(value) ((uint32_t)(value) * 3300 / 4096)
```

## 完整使用示例

### 示例1：基础单端采样

```c
#include "adc.h"
#include "pin_defs.h"

void basic_adc_example(void) {
    // 1. 初始化ADC（通道0，8MHz，0dB增益）
    adc_init_single(ADC_CHANNEL_0, ADC_SAMPLE_FREQ_8MHZ, ADC_PGA_0);

    // 2. 执行校准
    int16_t calib = adc_calibrate();
    printf("Calibration: %d\n", calib);

    // 3. 执行采样
    uint16_t value = adc_start_single_conversion();
    uint32_t voltage = adc_read_voltage(ADC_CHANNEL_0);

    printf("ADC Value: %d, Voltage: %d mV\n", value, voltage);

    // 4. 多次采样取平均
    uint16_t avg = adc_read_average(ADC_CHANNEL_0, 10);
    printf("Average: %d\n", avg);
}
```

### 示例2：多通道轮询采样

```c
void multi_channel_polling(void) {
    // 初始化多个通道
    adc_init_single(ADC_CHANNEL_0, ADC_SAMPLE_FREQ_8MHZ, ADC_PGA_0);
    adc_init_single(ADC_CHANNEL_1, ADC_SAMPLE_FREQ_8MHZ, ADC_PGA_0);
    adc_init_single(ADC_CHANNEL_2, ADC_SAMPLE_FREQ_8MHZ, ADC_PGA_0);

    // 校准
    adc_calibrate();

    // 轮询采样
    while (1) {
        uint16_t ch0 = adc_read_average(ADC_CHANNEL_0, 5);
        uint16_t ch1 = adc_read_average(ADC_CHANNEL_1, 5);
        uint16_t ch2 = adc_read_average(ADC_CHANNEL_2, 5);

        printf("CH0: %d, CH1: %d, CH2: %d\n", ch0, ch1, ch2);

        // 延时
        for (volatile int i = 0; i < 100000; i++);
    }
}
```

### 示例3：差分放大器采样

```c
void differential_mode_example(void) {
    // 初始化差分模式
    adc_init_diff(ADC_CHANNEL_0, ADC_SAMPLE_FREQ_8MHZ, ADC_PGA_6);

    // 校准
    adc_calibrate();

    // 执行差分采样
    for (int i = 0; i < 20; i++) {
        uint16_t value = adc_start_single_conversion();
        printf("Differential Value: %d\n", value);
    }
}
```

### 示例4：温度监控系统

```c
void temperature_monitoring(void) {
    // 初始化温度传感器
    adc_init_temperature();

    printf("Temperature Monitoring Started\n");

    while (1) {
        // 丢弃首次转换
        adc_start_single_conver();

        // 读取温度（20次平均）
        int total_temp = 0;
        for (int i = 0; i < 20; i++) {
            uint16_t value = adc_start_single_conversion();
            total_temp += adc_to_temperature_celsius(value);
        }

        int avg_temp = total_temp / 20;
        printf("Average Temperature: %d°C\n", avg_temp);

        // 检查温度阈值
        if (avg_temp > 50) {
            printf("Warning: High Temperature!\n");
        }

        // 延时
        for (volatile int i = 0; i < 200000; i++);
    }
}
```

### 示例5：TouchKey触摸检测

```c
void touchkey_detection(void) {
    // 初始化TouchKey
    adc_init_touchkey();

    // 配置TouchKey通道
    GPIOA_ModeCfg(GPIO_Pin_12, GPIO_ModeIN_Floating);
    ADC_ChannelCfg(2);

    printf("TouchKey Detection Started\n");

    uint16_t baseline = 0;
    uint16_t samples[10];

    // 获取基线值
    for (int i = 0; i < 10; i++) {
        samples[i] = adc_start_touchkey_conversion(0x10, 0);
        baseline += samples[i];
    }
    baseline /= 10;

    printf("Baseline: %d\n", baseline);

    while (1) {
        uint16_t value = adc_start_touchkey_conversion(0x10, 0);

        // 检测触摸（阈值可调整）
        if (value < baseline - 100) {
            printf("Touch Detected! Value: %d, Baseline: %d\n", value, baseline);
        }

        // 延时
        for (volatile int i = 0; i < 50000; i++);
    }
}
```

### 示例6：DMA连续采样

```c
#define DMA_SAMPLES 100
uint16_t dma_buffer[DMA_SAMPLES];
volatile bool dma_complete = FALSE;

void adc_dma_callback(uint16_t value) {
    static int count = 0;
    if (count < DMA_SAMPLES) {
        dma_buffer[count++] = value;
    }
    if (count >= DMA_SAMPLES) {
        dma_complete = TRUE;
    }
}

void dma_continuous_sampling(void) {
    // 初始化ADC
    adc_init_single(ADC_CHANNEL_0, ADC_SAMPLE_FREQ_8MHZ, ADC_PGA_0);

    // 丢弃首次转换
    ADC_ExcutSingleConver();

    // 配置DMA
    adc_config_dma(dma_buffer, DMA_SAMPLES);

    // 启动DMA
    adc_start_dma();

    // 等待完成
    while (!dma_complete) {
        // 等待...
    }

    // 输出结果
    printf("DMA Sampling Complete\n");
    for (int i = 0; i < DMA_SAMPLES; i++) {
        printf("%d\n", dma_buffer[i]);
    }

    // 停止DMA
    adc_stop_dma();
}
```

## 注意事项

### 1. GPIO配置

- ADC引脚必须配置为浮空输入模式（GPIO_ModeIN_Floating）
- 不要在ADC引脚上连接强驱动信号，以免损坏芯片
- 差分模式需要配置两个引脚

### 2. 采样频率选择

- 8MHz：适用于高速采样，但功耗较高
- 4MHz：适用于一般应用，功耗较低
- 选择合适的采样频率取决于应用需求和功耗要求

### 3. 增益设置

- 0dB：适用于一般电压测量
- 6dB/12dB/18dB：适用于小信号放大，但会降低输入范围
- 注意：增益越高，输入范围越小

### 4. 校准

- 建议每次初始化后都执行校准
- 校准值会自动应用到后续的采样中
- 温度变化可能影响校准精度

### 5. 中断和DMA

- 使用中断或DMA模式时，需要使能ADC中断（PFIC_EnableIRQ）
- 在中断处理函数中不要执行耗时操作
- DMA模式可以大大减少CPU占用

### 6. 扫描模式

- 扫描模式最多支持14个通道
- 扫描模式使用DMA传输数据
- 扫描期间不能切换通道

### 7. 电气特性

- 参考电压：通常为3.3V
- 输入范围：0V到参考电压
- 分辨率：12位（4096个刻度）
- 建议在输入端添加RC滤波电路

### 8. 功耗考虑

- 轮询模式：功耗最低
- 中断模式：中等功耗
- DMA模式：功耗较低
- 扫描模式：功耗较高（多通道采样）

## 错误处理

### 检查通道有效性

```c
if (channel >= ADC_CHANNEL_MAX) {
    // 错误：通道号无效
    return;
}

if (!adc_is_bound(channel)) {
    // 错误：通道未初始化
    return;
}
```

### 检查DMA状态

```c
if (adc_get_dma_status()) {
    // DMA传输完成
    adc_clear_dma_flag();
}
```

### 检查中断状态

```c
if (adc_get_interrupt_status()) {
    // 有中断请求
    adc_clear_interrupt_flag();
}
```

## API参考

| 函数名 | 功能描述 |
|--------|----------|
| `adc_init_single()` | 初始化单端模式ADC |
| `adc_init_diff()` | 初始化差分模式ADC |
| `adc_init_temperature()` | 初始化温度传感器 |
| `adc_init_touchkey()` | 初始化TouchKey |
| `adc_start_single_conversion()` | 启动单次转换 |
| `adc_start_touchkey_conversion()` | 启动TouchKey转换 |
| `adc_read_value()` | 读取转换值 |
| `adc_read_average()` | 读取平均值 |
| `adc_read_voltage()` | 读取电压值 |
| `adc_calibrate()` | 执行校准 |
| `adc_config_interrupt()` | 配置中断模式 |
| `adc_start_interrupt()` | 启动中断转换 |
| `adc_stop_interrupt()` | 停止中断转换 |
| `adc_config_dma()` | 配置DMA模式 |
| `adc_start_dma()` | 启动DMA传输 |
| `adc_stop_dma()` | 停止DMA传输 |
| `adc_config_scan()` | 配置扫描模式 |
| `adc_start_scan()` | 启动扫描 |
| `adc_stop_scan()` | 停止扫描 |
| `adc_bind_pin()` | 绑定GPIO引脚 |
| `adc_get_bound_pin()` | 获取绑定引脚 |
| `adc_is_bound()` | 检查绑定状态 |

## 底层API

本实现基于CH58x标准外设库提供的底层API：

- `ADC_ExtSingleChSampInit()` - 外部单通道采样初始化
- `ADC_ExtDiffChSampInit()` - 外部差分通道采样初始化
- `ADC_InterTSSampInit()` - 内部温度传感器采样初始化
- `TouchKey_ChSampInit()` - TouchKey通道采样初始化
- `ADC_ChannelCfg()` - 配置ADC通道
- `ADC_ExcutSingleConver()` - 执行单次转换
- `TouchKey_ExcutSingleConver()` - 执行TouchKey转换
- `ADC_ReadConverValue()` - 读取转换值
- `ADC_DataCalib_Rough()` - 粗调校准
- `ADC_ClearITFlag()` - 清除中断标志
- `ADC_StartUp()` - 启动ADC
- `ADC_GetITStatus()` - 获取中断状态
- `ADC_GetDMAStatus()` - 获取DMA状态
- `ADC_StopAutoDMA()` - 停止自动DMA
- `ADC_ClearDMAFlag()` - 清除DMA标志
- `ADC_AutoConverCycle()` - 设置自动转换周期
- `ADC_DMACfg()` - 配置DMA
- `ADC_StartAutoDMA()` - 启动自动DMA

## 常见应用场景

1. **电压监测**：使用单端模式测量电池电压、电源电压等
2. **温度监控**：使用内置温度传感器监测环境温度
3. **TouchKey**：使用TouchKey功能实现触摸按键
4. **传感器读取**：读取各种模拟传感器输出
5. **差分放大**：测量差分信号，提高抗干扰能力
6. **多通道采集**：使用扫描模式同时采集多个通道
7. **低功耗采样**：使用DMA模式减少CPU占用
8. **高精度测量**：结合校准和平均值提高精度

## 故障排除

### 1. 采样值异常

- 检查GPIO配置是否正确（浮空输入）
- 检查参考电压是否稳定
- 执行校准操作
- 检查输入信号是否在有效范围内

### 2. 温度值不准确

- 确保芯片充分预热
- 检查温度校准值（ROM_CFG_TMP_25C）
- 多次采样取平均

### 3. TouchKey无响应

- 检查TouchKey引脚配置
- 调整充放电时间参数
- 检查触摸阈值设置
- 确保PCB布局符合要求

### 4. DMA传输失败

- 检查缓冲区地址和大小
- 确保ADC中断已使能
- 检查DMA配置参数
- 确保内存空间足够

### 5. 扫描模式问题

- 检查通道数组和数量
- 确保缓冲区大小足够
- 检查扫描配置寄存器
- 确保GPIO配置正确
