# CH584 PWM HAL 使用说明

## 概述

本文档描述了如何在CH584平台上使用键盘框架的PWM HAL接口。所有接口都基于CH584标准外设库实现，支持8个PWM通道（PWM4-PWM11），并提供灵活的GPIO引脚绑定机制。

## 头文件包含

```c
#include "pwm.h"
#include "pin_defs.h"
```

## 支持的PWM通道

```c
typedef enum {
    PWM_CHANNEL_0  = (1 << 0),  /* 通道0 -> CH584 PWM4 */
    PWM_CHANNEL_1  = (1 << 1),  /* 通道1 -> CH584 PWM5 */
    PWM_CHANNEL_2  = (1 << 2),  /* 通道2 -> CH584 PWM6 */
    PWM_CHANNEL_3  = (1 << 3),  /* 通道3 -> CH584 PWM7 */
    PWM_CHANNEL_4  = (1 << 4),  /* 通道4 -> CH584 PWM8 */
    PWM_CHANNEL_5  = (1 << 5),  /* 通道5 -> CH584 PWM9 */
    PWM_CHANNEL_6  = (1 << 6),  /* 通道6 -> CH584 PWM10 */
    PWM_CHANNEL_7  = (1 << 7),  /* 通道7 -> CH584 PWM11 */
    PWM_CHANNEL_ALL = 0xFF       /* 所有通道 */
} pwm_channel_t;
```

**注意**：`pwm_channel_t` 使用按位设置，可以同时操作多个通道：
```c
PWM_CHANNEL_0 | PWM_CHANNEL_1  // 同时操作通道0和1
PWM_CHANNEL_ALL                // 操作所有通道
```

## PWM引脚映射

| HAL PWM通道 | CH584 PWM通道 | 默认GPIO引脚 |
|-------------|---------------|--------------|
| PWM_CHANNEL_0 | PWM4 | 根据具体芯片配置 |
| PWM_CHANNEL_1 | PWM5 | 根据具体芯片配置 |
| PWM_CHANNEL_2 | PWM6 | 根据具体芯片配置 |
| PWM_CHANNEL_3 | PWM7 | 根据具体芯片配置 |
| PWM_CHANNEL_4 | PWM8 | 根据具体芯片配置 |
| PWM_CHANNEL_5 | PWM9 | 根据具体芯片配置 |
| PWM_CHANNEL_6 | PWM10 | 根据具体芯片配置 |
| PWM_CHANNEL_7 | PWM11 | 根据具体芯片配置 |

**注意**：实际的GPIO引脚映射需要根据具体硬件设计确定，CH584的PWM4-PWM11可以映射到多个GPIO引脚。

## 核心API

### 1. PWM初始化

```c
/**
 * @brief 初始化PWM模块
 *
 * @param none
 * @return none
 */
void pwm_init(void);
```

**示例用法**：
```c
pwm_init();  // 初始化PWM模块，默认1KHz频率
```

### 2. 启动/停止PWM

```c
/**
 * @brief 启动PWM输出
 *
 * @param channel PWM通道号（可使用按位OR组合多个通道）
 * @return none
 */
void pwm_start(pwm_channel_t channel);

/**
 * @brief 停止PWM输出
 *
 * @param channel PWM通道号（可使用按位OR组合多个通道）
 * @return none
 */
void pwm_stop(pwm_channel_t channel);
```

**示例用法**：
```c
// 启动单个通道
pwm_start(PWM_CHANNEL_0);

// 同时启动多个通道
pwm_start(PWM_CHANNEL_0 | PWM_CHANNEL_1 | PWM_CHANNEL_2);

// 停止所有通道
pwm_stop(PWM_CHANNEL_ALL);
```

### 3. 配置PWM频率和占空比

```c
/**
 * @brief 设置PWM频率
 *
 * @param channel PWM通道号（可使用按位OR组合多个通道）
 * @param frequency PWM频率(Hz)
 * @return none
 */
void pwm_set_frequency(pwm_channel_t channel, uint32_t frequency);

/**
 * @brief 设置PWM占空比
 *
 * @param channel PWM通道号（可使用按位OR组合多个通道）
 * @param duty_cycle 占空比(0-100)
 * @return none
 */
void pwm_set_duty_cycle(pwm_channel_t channel, uint8_t duty_cycle);

/**
 * @brief 设置PWM占空比百分比
 *
 * @param channel PWM通道号（可使用按位OR组合多个通道）
 * @param percentage 占空比百分比(0.0-100.0)
 * @return none
 */
void pwm_set_duty_percentage(pwm_channel_t channel, float percentage);
```

**示例用法**：
```c
// 设置频率为1KHz
pwm_set_frequency(PWM_CHANNEL_0, 1000);

// 设置占空比为50%
pwm_set_duty_cycle(PWM_CHANNEL_0, 50);

// 设置占空比为75.5%
pwm_set_duty_percentage(PWM_CHANNEL_0, 75.5);

// 一次性配置频率和占空比
pwm_configure(PWM_CHANNEL_0, 2000, 30);  // 2KHz，30%占空比
```

### 4. 读取PWM参数

```c
/**
 * @brief 获取PWM占空比
 *
 * @param channel PWM通道号
 * @return 占空比(0-100)
 */
uint8_t pwm_get_duty_cycle(pwm_channel_t channel);

/**
 * @brief 获取PWM占空比百分比
 *
 * @param channel PWM通道号
 * @return 占空比百分比(0.0-100.0)
 */
float pwm_get_duty_percentage(pwm_channel_t channel);
```

**示例用法**：
```c
uint8_t duty = pwm_get_duty_cycle(PWM_CHANNEL_0);
float percentage = pwm_get_duty_percentage(PWM_CHANNEL_0);
```

### 5. 设置PWM极性

```c
/**
 * @brief 设置PWM极性
 *
 * @param channel PWM通道号（可使用按位OR组合多个通道）
 * @param polarity 极性(0:正常, 1:反向)
 * @return none
 */
void pwm_set_polarity(pwm_channel_t channel, uint8_t polarity);
```

**示例用法**：
```c
// 设置正常极性（高电平有效）
pwm_set_polarity(PWM_CHANNEL_0, 0);

// 设置反向极性（低电平有效）
pwm_set_polarity(PWM_CHANNEL_0, 1);
```

### 6. 使能/禁用PWM

```c
/**
 * @brief 使能PWM
 *
 * @param channel PWM通道号（可使用按位OR组合多个通道）
 * @return none
 */
void pwm_enable(pwm_channel_t channel);

/**
 * @brief 禁用PWM
 *
 * @param channel PWM通道号（可使用按位OR组合多个通道）
 * @return none
 */
void pwm_disable(pwm_channel_t channel);
```

**示例用法**：
```c
// 使能PWM通道
pwm_enable(PWM_CHANNEL_0);

// 禁用PWM通道
pwm_disable(PWM_CHANNEL_0);
```

### 7. GPIO引脚绑定

```c
/**
 * @brief 绑定GPIO引脚到PWM通道
 *
 * @param pin GPIO引脚号
 * @param channel PWM通道号（可使用按位OR组合多个通道）
 * @return bool 绑定是否成功
 */
bool pwm_bind_pin(pin_t pin, pwm_channel_t channel);

/**
 * @brief 取消GPIO引脚与PWM通道的绑定
 *
 * @param pin GPIO引脚号
 * @return bool 取消绑定是否成功
 */
bool pwm_unbind_pin(pin_t pin);

/**
 * @brief 获取PWM通道绑定的GPIO引脚
 *
 * @param channel PWM通道号
 * @return GPIO引脚号（如果未绑定则返回NO_PIN）
 */
pin_t pwm_get_bound_pin(pwm_channel_t channel);

/**
 * @brief 检查PWM通道是否已绑定GPIO引脚
 *
 * @param channel PWM通道号
 * @return true表示已绑定，false表示未绑定
 */
bool pwm_is_bound(pwm_channel_t channel);
```

**示例用法**：
```c
// 绑定引脚到PWM通道
pwm_bind_pin(A0, PWM_CHANNEL_0);  // 将PA0绑定到PWM_CHANNEL_0

// 检查绑定状态
if (pwm_is_bound(PWM_CHANNEL_0)) {
    pin_t pin = pwm_get_bound_pin(PWM_CHANNEL_0);
    // 使用绑定的引脚
}

// 取消绑定
pwm_unbind_pin(A0);
```

### 8. 使能引脚绑定（可选）

```c
/**
 * @brief 绑定PWM使能引脚到通道
 *
 * @param enable_pin 使能引脚号
 * @param channel PWM通道号
 * @return bool 绑定是否成功
 */
bool pwm_bind_enable_pin(pin_t enable_pin, pwm_channel_t channel);

/**
 * @brief 获取PWM通道绑定的使能引脚
 *
 * @param channel PWM通道号
 * @return 使能引脚号（如果未绑定则返回NO_PIN）
 */
pin_t pwm_get_bound_enable_pin(pwm_channel_t channel);
```

**示例用法**：
```c
// 绑定使能引脚
pwm_bind_enable_pin(A1, PWM_CHANNEL_0);  // 将PA1作为PWM_CHANNEL_0的使能引脚
```

### 9. 便捷宏

```c
/* 设置PWM电平 */
#define pwm_set_level(channel, level)        pwm_set_duty_cycle((channel), (level) ? 100 : 0)

/* 设置LED亮度 */
#define pwm_set_brightness(channel, brightness) pwm_set_duty_cycle((channel), (brightness))
#define pwm_get_brightness(channel)          pwm_get_duty_cycle(channel)

/* LED控制宏 */
#define pwm_led_on(channel)                  pwm_set_level((channel), 1)
#define pwm_led_off(channel)                 pwm_set_level((channel), 0)
#define pwm_led_toggle(channel)              pwm_set_level((channel), !pwm_get_duty_cycle(channel))
#define pwm_led_set(channel, val)            pwm_set_brightness((channel), (val))
#define pwm_led_get(channel)                 pwm_get_brightness(channel)
```

**示例用法**：
```c
// 控制LED
pwm_led_on(PWM_CHANNEL_0);      // LED亮
pwm_led_off(PWM_CHANNEL_0);     // LED灭
pwm_led_toggle(PWM_CHANNEL_0);  // LED翻转
pwm_led_set(PWM_CHANNEL_0, 50); // 设置亮度为50
uint8_t brightness = pwm_led_get(PWM_CHANNEL_0);  // 获取亮度
```

## 使用示例

### 示例1：LED亮度控制

```c
#include "pwm.h"
#include "pin_defs.h"

void led_pwm_example(void) {
    // 1. 初始化PWM
    pwm_init();

    // 2. 绑定引脚到PWM通道
    pwm_bind_pin(B0, PWM_CHANNEL_0);

    // 3. 配置PWM（1KHz，50%占空比）
    pwm_configure(PWM_CHANNEL_0, 1000, 50);

    // 4. 启动PWM
    pwm_start(PWM_CHANNEL_0);

    // 5. 渐变亮度
    for (int i = 0; i <= 100; i++) {
        pwm_set_brightness(PWM_CHANNEL_0, i);
        // 延时
    }
    for (int i = 100; i >= 0; i--) {
        pwm_set_brightness(PWM_CHANNEL_0, i);
        // 延时
    }
}
```

### 示例2：多通道PWM控制

```c
#include "pwm.h"
#include "pin_defs.h"

void multi_channel_pwm_example(void) {
    // 1. 初始化PWM
    pwm_init();

    // 2. 绑定多个引脚
    pwm_bind_pin(B0, PWM_CHANNEL_0);  // 红色LED
    pwm_bind_pin(B1, PWM_CHANNEL_1);  // 绿色LED
    pwm_bind_pin(B2, PWM_CHANNEL_2);  // 蓝色LED

    // 3. 配置不同通道
    pwm_configure(PWM_CHANNEL_0, 1000, 100);  // 红LED全亮
    pwm_configure(PWM_CHANNEL_1, 1000, 50);   // 绿LED半亮
    pwm_configure(PWM_CHANNEL_2, 1000, 0);    // 蓝LED熄灭

    // 4. 同时启动所有通道
    pwm_start(PWM_CHANNEL_0 | PWM_CHANNEL_1 | PWM_CHANNEL_2);
}
```

### 示例3：呼吸灯效果

```c
#include "pwm.h"
#include "pin_defs.h"

void breathing_led_example(void) {
    // 1. 初始化PWM
    pwm_init();

    // 2. 绑定引脚
    pwm_bind_pin(B0, PWM_CHANNEL_0);

    // 3. 配置PWM（高频用于平滑调光）
    pwm_set_frequency(PWM_CHANNEL_0, 20000);  // 20KHz，超声波避免闪烁

    // 4. 呼吸灯循环
    while (1) {
        // 逐渐变亮
        for (int i = 0; i <= 100; i++) {
            pwm_set_brightness(PWM_CHANNEL_0, i);
            // 延时10ms
        }
        // 逐渐变暗
        for (int i = 100; i >= 0; i--) {
            pwm_set_brightness(PWM_CHANNEL_0, i);
            // 延时10ms
        }
    }
}
```

### 示例4：舵机控制

```c
#include "pwm.h"
#include "pin_defs.h"

void servo_control_example(void) {
    // 1. 初始化PWM
    pwm_init();

    // 2. 绑定引脚
    pwm_bind_pin(B0, PWM_CHANNEL_0);

    // 3. 配置PWM（50Hz，舵机标准频率）
    pwm_set_frequency(PWM_CHANNEL_0, 50);

    // 4. 舵机控制
    // 0.5ms脉冲 -> 0度
    pwm_set_pulse_width(PWM_CHANNEL_0, 500000);  // 0.5ms = 500000ns

    // 1.5ms脉冲 -> 90度
    pwm_set_pulse_width(PWM_CHANNEL_0, 1500000);  // 1.5ms = 1500000ns

    // 2.5ms脉冲 -> 180度
    pwm_set_pulse_width(PWM_CHANNEL_0, 2500000);  // 2.5ms = 2500000ns
}
```

### 示例5：蜂鸣器控制

```c
#include "pwm.h"
#include "pin_defs.h"

void buzzer_example(void) {
    // 1. 初始化PWM
    pwm_init();

    // 2. 绑定引脚
    pwm_bind_pin(B0, PWM_CHANNEL_0);

    // 3. 配置PWM（2KHz）
    pwm_configure(PWM_CHANNEL_0, 2000, 50);

    // 4. 启动蜂鸣器
    pwm_start(PWM_CHANNEL_0);

    // 5. 播放旋律
    int melody[] = {262, 294, 330, 349, 392, 440, 494, 523};  // C大调音阶
    for (int i = 0; i < 8; i++) {
        pwm_set_frequency(PWM_CHANNEL_0, melody[i]);
        // 延时500ms
    }

    // 6. 停止蜂鸣器
    pwm_stop(PWM_CHANNEL_0);
}
```

### 示例6：背光控制

```c
#include "pwm.h"
#include "pin_defs.h"

void backlight_control_example(void) {
    // 1. 初始化PWM
    pwm_init();

    // 2. 绑定背光引脚（多个通道控制不同区域）
    pwm_bind_pin(B0, PWM_CHANNEL_0);
    pwm_bind_pin(B1, PWM_CHANNEL_1);
    pwm_bind_pin(B2, PWM_CHANNEL_2);
    pwm_bind_pin(B3, PWM_CHANNEL_3);

    // 3. 配置所有通道
    pwm_configure(PWM_CHANNEL_ALL, 1000, 80);  // 1KHz，80%亮度

    // 4. 启动背光
    pwm_start(PWM_CHANNEL_ALL);

    // 5. 根据环境光自动调节亮度（模拟）
    // 实际应用中需要读取光敏电阻或环境光传感器
}
```

## 注意事项

### 1. 通道映射

- CH584芯片的PWM4-PWM11对应HAL的PWM_CHANNEL_0-7
- 映射关系：`PWM_CHANNEL_0 -> CH_PWM4`, `PWM_CHANNEL_1 -> CH_PWM5`, ...
- 多通道操作使用按位OR：`PWM_CHANNEL_0 | PWM_CHANNEL_1`

### 2. 频率范围

- **低频（≤10KHz）**：使用16位精度，提供更精细的占空比控制
- **高频（>10KHz）**：使用8位精度，适合LED调光等应用
- **建议频率范围**：50Hz - 100KHz

### 3. 占空比范围

- 整数占空比：0-100
- 浮点占空比：0.0-100.0
- 占空比为0时，输出低电平
- 占空比为100时，输出高电平

### 4. 极性设置

- **正常极性（0）**：高电平有效，默认配置
- **反向极性（1）**：低电平有效，用于需要反向控制的场景

### 5. 精度说明

- **8位模式**：占空比精度约为1%
- **16位模式**：占空比精度约为0.1%
- 低频应用推荐使用16位模式以获得更好的控制精度

### 6. 硬件配置

- PWM使用CH584的PWM4-PWM11通道
- 需要在硬件设计中确保这些PWM通道已正确映射到GPIO引脚
- 某些GPIO引脚可能支持PWM复用，具体引脚映射请参考芯片手册

### 7. 性能优化

- 多通道操作会自动批量更新硬件寄存器
- 频繁调用`pwm_set_duty_cycle()`会影响性能，建议批量更新
- 对于LED调光应用，建议使用20KHz以上频率避免闪烁

### 8. 线程安全

- PWM操作不是线程安全的
- 多线程环境下需要自行实现同步机制

## API参考

| 函数名 | 功能描述 |
|--------|----------|
| `pwm_init()` | 初始化PWM模块 |
| `pwm_start()` | 启动PWM输出 |
| `pwm_stop()` | 停止PWM输出 |
| `pwm_set_frequency()` | 设置PWM频率 |
| `pwm_set_duty_cycle()` | 设置占空比（整数） |
| `pwm_set_duty_percentage()` | 设置占空比（浮点） |
| `pwm_get_duty_cycle()` | 获取占空比（整数） |
| `pwm_get_duty_percentage()` | 获取占空比（浮点） |
| `pwm_set_pulse_width()` | 设置脉冲宽度 |
| `pwm_set_period()` | 设置PWM周期 |
| `pwm_set_polarity()` | 设置PWM极性 |
| `pwm_enable()` | 使能PWM |
| `pwm_disable()` | 禁用PWM |
| `pwm_configure()` | 一次性配置频率和占空比 |
| `pwm_bind_pin()` | 绑定GPIO引脚到PWM通道 |
| `pwm_unbind_pin()` | 取消GPIO引脚绑定 |
| `pwm_get_bound_pin()` | 获取绑定的GPIO引脚 |
| `pwm_is_bound()` | 检查是否已绑定 |
| `pwm_bind_enable_pin()` | 绑定使能引脚 |
| `pwm_get_bound_enable_pin()` | 获取绑定的使能引脚 |

## 底层API

本实现基于CH58x_pwm.h提供的底层API：

- `PWMX_CLKCfg(d)` - 配置PWM基准时钟分频
- `PWMX_CycleCfg(cyc)` - 配置PWM周期
- `PWMX_ACTOUT(ch, da, pr, s)` - 8位PWM输出配置
- `PWMX_16bit_ACTOUT(ch, da, pr, s)` - 16位PWM输出配置
- `PWMX_AlterOutCfg(ch, s)` - PWM交替输出模式配置

## 常见应用场景

1. **LED调光**：使用20KHz以上频率，PWM_CHANNEL_ALL控制多个LED
2. **舵机控制**：使用50Hz频率，通过脉冲宽度控制角度
3. **蜂鸣器音调**：使用不同频率产生不同音调
4. **背光控制**：多通道控制键盘背光的不同区域
5. **电机调速**：使用适当频率和占空比控制直流电机转速
