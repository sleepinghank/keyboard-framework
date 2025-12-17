/**
 * @file uart_hal.h
 * @brief Pixart2860 UART硬件抽象层实现
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * Pixart2860平台的UART驱动实现
 */

#ifndef UART_HAL_PIXART2860_H
#define UART_HAL_PIXART2860_H

#include "uart_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Pixart2860 UART端口定义 */
#define PIXART2860_UART_PORT_0        0
#define PIXART2860_UART_PORT_1        1
#define PIXART2860_UART_PORT_2        2

/* Pixart2860 UART端口数量 */
#define PIXART2860_UART_PORT_COUNT     3

/* Pixart2860 UART寄存器基址 */
#define PIXART2860_UART_BASE            0x40004000UL

/* Pixart2860 UART端口寄存器基址 */
#define PIXART2860_UART_PORT_BASE(port) (PIXART2860_UART_BASE + ((port) << 8))

/* Pixart2860 UART寄存器偏移 */
#define PIXART2860_UART_SR_REG            0x00  /* 状态寄存器 */
#define PIXART2860_UART_DR_REG            0x04  /* 数据寄存器 */
#define PIXART2860_UART_BRR_REG           0x08  /* 波特率寄存器 */
#define PIXART2860_UART_CR1_REG           0x0C  /* 控制寄存器1 */
#define PIXART2860_UART_CR2_REG           0x10  /* 控制寄存器2 */
#define PIXART2860_UART_CR3_REG           0x14  /* 控制寄存器3 */
#define PIXART2860_UART_GTPR_REG          0x18  /* 保护时间和预分频寄存器 */
#define PIXART2860_UART_RTOR_REG          0x1C  /* 接收超时寄存器 */
#define PIXART2860_UART_RQR_REG           0x20  /* 请求寄存器 */
#define PIXART2860_UART_ISR_REG           0x24  /* 中断和状态寄存器 */
#define PIXART2860_UART_ICR_REG           0x28  /* 中断清除寄存器 */
#define PIXART2860_UART_RDR_REG           0x2C  /* 接收数据寄存器 */
#define PIXART2860_UART_TDR_REG           0x30  /* 发送数据寄存器 */
#define PIXART2860_UART_PRESC_REG         0x34  /* 预分频寄存器 */

/* Pixart2860 UART控制寄存器1位定义 */
#define PIXART2860_UART_CR1_SBK          (1 << 0)    /* 发送断开字符 */
#define PIXART2860_UART_CR1_RWU          (1 << 1)    /* 接收唤醒 */
#define PIXART2860_UART_CR1_RE           (1 << 2)    /* 接收使能 */
#define PIXART2860_UART_CR1_TE           (1 << 3)    /* 发送使能 */
#define PIXART2860_UART_CR1_IDLEIE       (1 << 4)    /* IDLE中断使能 */
#define PIXART2860_UART_CR1_RXNEIE       (1 << 5)    /* RXNE中断使能 */
#define PIXART2860_UART_CR1_TCIE         (1 << 6)    /* TC中断使能 */
#define PIXART2860_UART_CR1_TXEIE        (1 << 7)    /* TXE中断使能 */
#define PIXART2860_UART_CR1_PEIE         (1 << 8)    /* PE中断使能 */
#define PIXART2860_UART_CR1_PS           (1 << 9)    /* 奇偶性选择 */
#define PIXART2860_UART_CR1_PCE          (1 << 10)   /* 奇偶性控制使能 */
#define PIXART2860_UART_CR1_WAKE         (1 << 11)   /* 唤醒方法 */
#define PIXART2860_UART_CR1_M0           (1 << 12)   /* 字符长度位0 */
#define PIXART2860_UART_CR1_MME          (1 << 13)   /* 静音模式使能 */
#define PIXART2860_UART_CR1_CMIE         (1 << 14)   /* 字符匹配中断使能 */
#define PIXART2860_UART_CR1_OVER8        (1 << 15)   /* 过采样模式 */
#define PIXART2860_UART_CR1_DEDT_SHIFT   16         /* 数据检测延时 */
#define PIXART2860_UART_CR1_DEAT_SHIFT   21         /* 数据检测超时 */
#define PIXART2860_UART_CR1_RTOIE        (1 << 26)   /* 接收超时中断使能 */
#define PIXART2860_UART_CR1_EOBIE        (1 << 27)   /* 块结束中断使能 */

/* Pixart2860 UART中断定义 */
#define PIXART2860_UART_IRQn(port)       (10 + (port))
#define PIXART2860_UART_IRQ_PRIORITY     4

/* Pixart2860 UART GPIO引脚定义 */
#define PIXART2860_UART0_TX_PIN         2   /* UART0 TX引脚 */
#define PIXART2860_UART0_RX_PIN         3   /* UART0 RX引脚 */
#define PIXART2860_UART1_TX_PIN         4   /* UART1 TX引脚 */
#define PIXART2860_UART1_RX_PIN         5   /* UART1 RX引脚 */
#define PIXART2860_UART2_TX_PIN         6   /* UART2 TX引脚 */
#define PIXART2860_UART2_RX_PIN         7   /* UART2 RX引脚 */

/* Pixart2860 UART最大波特率 */
#define PIXART2860_UART_MAX_BAUDRATE    9216000    /* 最大波特率9.2Mbps */

/* Pixart2860 UART最小波特率 */
#define PIXART2860_UART_MIN_BAUDRATE    9600       /* 最小波特率9.6kbps */

/* Pixart2860 UART输入时钟频率 */
#define PIXART2860_UART_INPUT_CLOCK_HZ  48000000   /* 输入时钟48MHz */

/* Pixart2860 UART函数声明 */

/**
 * @brief 获取Pixart2860 UART接口
 * @return UART接口指针
 */
const uart_interface_t *pixart2860_uart_get_interface(void);

/**
 * @brief 配置UART端口
 * @param port UART端口
 * @param config UART配置参数
 * @return 配置结果
 */
uart_result_t pixart2860_uart_config_port(uint8_t port, const uart_config_t *config);

/**
 * @brief 计算UART波特率寄存器值
 * @param baudrate 目标波特率
 * @param input_clock 输入时钟频率
 * @param over8 过采样模式
 * @return 波特率寄存器值
 */
uint32_t pixart2860_uart_calculate_baudrate(uint32_t baudrate, uint32_t input_clock, bool over8);

/**
 * @brief UART中断处理函数
 * @param port UART端口
 * @return 处理结果
 */
void pixart2860_uart_irq_handler(uint8_t port);

#ifdef __cplusplus
}
#endif

#endif /* UART_HAL_PIXART2860_H */