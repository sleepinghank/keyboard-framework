/**
 * @file gpio_hal.h
 * @brief Pixart2860 GPIO硬件抽象层实现
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * Pixart2860平台的GPIO驱动实现
 */

#ifndef GPIO_HAL_PIXART2860_H
#define GPIO_HAL_PIXART2860_H

#include "gpio_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Pixart2860 GPIO端口定义 */
#define PIXART2860_GPIO_PORT_A        0
#define PIXART2860_GPIO_PORT_B        1
#define PIXART2860_GPIO_PORT_C        2
#define PIXART2860_GPIO_PORT_D        3

/* Pixart2860 GPIO引脚定义 */
#define PIXART2860_GPIO_PIN_0         0
#define PIXART2860_GPIO_PIN_1         1
#define PIXART2860_GPIO_PIN_2         2
#define PIXART2860_GPIO_PIN_3         3
#define PIXART2860_GPIO_PIN_4         4
#define PIXART2860_GPIO_PIN_5         5
#define PIXART2860_GPIO_PIN_6         6
#define PIXART2860_GPIO_PIN_7         7
#define PIXART2860_GPIO_PIN_8         8
#define PIXART2860_GPIO_PIN_9         9
#define PIXART2860_GPIO_PIN_10        10
#define PIXART2860_GPIO_PIN_11        11
#define PIXART2860_GPIO_PIN_12        12
#define PIXART2860_GPIO_PIN_13        13
#define PIXART2860_GPIO_PIN_14        14
#define PIXART2860_GPIO_PIN_15        15

/* Pixart2860 GPIO引脚数量 */
#define PIXART2860_GPIO_PIN_COUNT     16

/* Pixart2860 GPIO寄存器基址 */
#define PIXART2860_GPIO_BASE          0x40000000UL

/* Pixart2860 GPIO寄存器偏移 */
#define PIXART2860_GPIO_OUT_REG      0x00
#define PIXART2860_GPIO_IN_REG       0x04
#define PIXART2860_GPIO_DIR_REG      0x08
#define PIXART2860_GPIO_IE_REG       0x0C
#define PIXART2860_GPIO_IS_REG       0x10
#define PIXART2860_GPIO_IBE_REG      0x14
#define PIXART2860_GPIO_IEV_REG      0x18
#define PIXART2860_GPIO_IC_REG       0x1C
#define PIXART2860_GPIO_PUPDR_REG    0x20
#define PIXART2860_GPIO_ODR_REG      0x24

/* Pixart2860 GPIO功能复用定义 */
#define PIXART2860_GPIO_AF_0         0
#define PIXART2860_GPIO_AF_1         1
#define PIXART2860_GPIO_AF_2         2
#define PIXART2860_GPIO_AF_3         3

/* Pixart2860 GPIO中断定义 */
#define PIXART2860_GPIO_IRQn         0
#define PIXART2860_GPIO_IRQ_PRIORITY 3

/* Pixart2860 GPIO时钟使能 */
#define PIXART2860_GPIO_CLK_ENABLE()  do { \
    /* 启用GPIO时钟 */ \
} while(0)

#define PIXART2860_GPIO_CLK_DISABLE() do { \
    /* 禁用GPIO时钟 */ \
} while(0)

/* Pixart2860 GPIO配置参数 */
typedef struct {
    uint8_t                 pin;                /* 引脚号 */
    gpio_mode_t             mode;               /* GPIO模式 */
    gpio_pull_t             pull;               /* 上拉/下拉配置 */
    gpio_speed_t            speed;              /* 输出速度 */
    gpio_drive_strength_t   drive_strength;     /* 驱动强度 */
    bool                    alternate_function; /* 是否使用复用功能 */
    uint8_t                 af_number;          /* 复用功能编号 */
    gpio_int_trigger_t      int_trigger;        /* 中断触发方式 */
    gpio_int_priority_t     int_priority;       /* 中断优先级 */
    volatile uint32_t      *port_reg;           /* 端口寄存器基地址 */
    uint32_t                pin_mask;           /* 引脚掩码 */
} pixart2860_gpio_config_t;

/* Pixart2860 GPIO实例结构体 */
typedef struct {
    volatile uint32_t      *out_reg;            /* 输出寄存器 */
    volatile uint32_t      *in_reg;             /* 输入寄存器 */
    volatile uint32_t      *dir_reg;            /* 方向寄存器 */
    volatile uint32_t      *ie_reg;             /* 中断使能寄存器 */
    volatile uint32_t      *is_reg;             /* 中断状态寄存器 */
    volatile uint32_t      *ibe_reg;            /* 中断双边沿寄存器 */
    volatile uint32_t      *iev_reg;            /* 中断事件寄存器 */
    volatile uint32_t      *ic_reg;             /* 中断清除寄存器 */
    volatile uint32_t      *pupdr_reg;          /* 上拉下拉寄存器 */
    volatile uint32_t      *odr_reg;            /* 开漏输出寄存器 */
    gpio_callback_t        *callbacks;          /* 中断回调函数数组 */
    void                  *user_data;          /* 用户数据数组 */
    bool                   initialized;         /* 初始化标志 */
    uint32_t               interrupt_mask;      /* 中断掩码 */
} pixart2860_gpio_instance_t;

/* Pixart2860 GPIO函数声明 */

/**
 * @brief 获取Pixart2860 GPIO接口
 * @return GPIO接口指针
 */
const gpio_interface_t *pixart2860_gpio_get_interface(void);

/**
 * @brief 计算引脚掩码
 * @param pin 引脚号
 * @return 引脚掩码
 */
uint32_t pixart2860_gpio_get_pin_mask(uint8_t pin);

/**
 * @brief 计算寄存器偏移
 * @param port 端口号
 * @return 寄存器基址
 */
volatile uint32_t *pixart2860_gpio_get_port_base(uint8_t port);

/**
 * @brief 配置引脚复用功能
 * @param pin 引脚号
 * @param af_number 复用功能编号
 * @return 配置结果
 */
gpio_result_t pixart2860_gpio_config_alternate_function(uint8_t pin, uint8_t af_number);

/**
 * @brief 配置引脚上拉下拉
 * @param pin 引脚号
 * @param pull 上拉下拉配置
 * @return 配置结果
 */
gpio_result_t pixart2860_gpio_config_pull(uint8_t pin, gpio_pull_t pull);

/**
 * @brief 配置引脚驱动强度
 * @param pin 引脚号
 * @param drive_strength 驱动强度
 * @return 配置结果
 */
gpio_result_t pixart2860_gpio_config_drive_strength(uint8_t pin, gpio_drive_strength_t drive_strength);

/**
 * @brief 配置引脚输出速度
 * @param pin 引脚号
 * @param speed 输出速度
 * @return 配置结果
 */
gpio_result_t pixart2860_gpio_config_speed(uint8_t pin, gpio_speed_t speed);

/**
 * @brief 配置引脚模式
 * @param pin 引脚号
 * @param mode GPIO模式
 * @return 配置结果
 */
gpio_result_t pixart2860_gpio_config_mode(uint8_t pin, gpio_mode_t mode);

/**
 * @brief 配置引脚中断
 * @param pin 引脚号
 * @param trigger 触发方式
 * @param priority 优先级
 * @return 配置结果
 */
gpio_result_t pixart2860_gpio_config_interrupt(uint8_t pin, gpio_int_trigger_t trigger, gpio_int_priority_t priority);

/**
 * @brief GPIO中断处理函数
 * @param port 端口号
 * @param pin 引脚号
 * @return 处理结果
 */
void pixart2860_gpio_irq_handler(uint8_t port, uint8_t pin);

/* 便捷宏定义 */
#define PIXART2860_GPIO_CONFIG(pin, mode, pull, speed) \
    { (pin), (mode), (pull), (speed), GPIO_DRIVE_STRENGTH_MEDIUM, false, 0, GPIO_INT_TRIGGER_NONE, GPIO_INT_PRIORITY_LOW, NULL, 0 }

#define PIXART2860_GPIO_CONFIG_AF(pin, af) \
    { (pin), GPIO_MODE_AF_PP, GPIO_PULL_NONE, GPIO_SPEED_HIGH, GPIO_DRIVE_STRENGTH_MEDIUM, true, (af), GPIO_INT_TRIGGER_NONE, GPIO_INT_PRIORITY_LOW, NULL, 0 }

#define PIXART2860_GPIO_CONFIG_INT(pin, trigger) \
    { (pin), GPIO_MODE_INPUT, GPIO_PULL_NONE, GPIO_SPEED_LOW, GPIO_DRIVE_STRENGTH_LOW, false, 0, (trigger), GPIO_INT_PRIORITY_MEDIUM, NULL, 0 }

/* 引脚操作宏 */
#define PIXART2860_GPIO_SET(pin) \
    *pixart2860_gpio_get_port_base(PIXART2860_GPIO_PORT_A + ((pin) / 8)) |= (1 << ((pin) % 8))

#define PIXART2860_GPIO_CLR(pin) \
    *pixart2860_gpio_get_port_base(PIXART2860_GPIO_PORT_A + ((pin) / 8)) &= ~(1 << ((pin) % 8))

#define PIXART2860_GPIO_TOGGLE(pin) \
    *pixart2860_gpio_get_port_base(PIXART2860_GPIO_PORT_A + ((pin) / 8)) ^= (1 << ((pin) % 8))

#define PIXART2860_GPIO_GET(pin) \
    ((*pixart2860_gpio_get_port_base(PIXART2860_GPIO_PORT_A + ((pin) / 8)) >> ((pin) % 8)) & 1)

/* 寄存器操作宏 */
#define PIXART2860_GPIO_GET_REG(port, offset) \
    (*((volatile uint32_t *)(PIXART2860_GPIO_BASE + ((port) << 8) + (offset))))

#define PIXART2860_GPIO_SET_REG(port, offset, value) \
    (*((volatile uint32_t *)(PIXART2860_GPIO_BASE + ((port) << 8) + (offset))) = (value))

#define PIXART2860_GPIO_MODIFY_REG(port, offset, mask, value) \
    (*((volatile uint32_t *)(PIXART2860_GPIO_BASE + ((port) << 8) + (offset))) = \
     ((*((volatile uint32_t *)(PIXART2860_GPIO_BASE + ((port) << 8) + (offset))) & ~(mask)) | ((value) & (mask)))

/* 常用引脚配置宏 */
#define PIXART2860_GPIO_LED_PIN         0   /* LED指示灯引脚 */
#define PIXART2860_GPIO_BUTTON_PIN      1   /* 按键引脚 */
#define PIXART2860_GPIO_MATRIX_COL0     2   /* 矩阵键盘列0 */
#define PIXART2860_GPIO_MATRIX_COL1     3   /* 矩阵键盘列1 */
#define PIXART2860_GPIO_MATRIX_ROW0     4   /* 矩阵键盘行0 */
#define PIXART2860_GPIO_MATRIX_ROW1     5   /* 矩阵键盘行1 */
#define PIXART2860_GPIO_TOUCH_SCL       6   /* 触控板SCL */
#define PIXART2860_GPIO_TOUCH_SDA       7   /* 触控板SDA */
#define PIXART2860_GPIO_UART_TX         8   /* 串口TX */
#define PIXART2860_GPIO_UART_RX         9   /* 串口RX */
#define PIXART2860_GPIO_PWM_OUT         10  /* PWM输出 */
#define PIXART2860_GPIO_ADC_INPUT       11  /* ADC输入 */
#define PIXART2860_GPIO_SPI_CS          12  /* SPI片选 */
#define PIXART2860_GPIO_SPI_CLK         13  /* SPI时钟 */
#define PIXART2860_GPIO_SPI_MOSI        14  /* SPI主机输出 */
#define PIXART2860_GPIO_SPI_MISO        15  /* SPI主机输入 */

#ifdef __cplusplus
}
#endif

#endif /* GPIO_HAL_PIXART2860_H */