/**
 * @file i2c_hal.h
 * @brief Pixart2860 I2C硬件抽象层实现
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * Pixart2860平台的I2C驱动实现
 */

#ifndef I2C_HAL_PIXART2860_H
#define I2C_HAL_PIXART2860_H

#include "../interface/i2c_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Pixart2860 I2C总线定义 */
#define PIXART2860_I2C_BUS_0            0
#define PIXART2860_I2C_BUS_1            1

/* Pixart2860 I2C总线数量 */
#define PIXART2860_I2C_BUS_COUNT         2

/* Pixart2860 I2C寄存器基址 */
#define PIXART2860_I2C_BASE              0x40003000UL

/* Pixart2860 I2C总线寄存器基址 */
#define PIXART2860_I2C_BUS_BASE(bus)     (PIXART2860_I2C_BASE + ((bus) << 8))

/* Pixart2860 I2C寄存器偏移 */
#define PIXART2860_I2C_CR1_REG           0x00  /* 控制寄存器1 */
#define PIXART2860_I2C_CR2_REG           0x04  /* 控制寄存器2 */
#define PIXART2860_I2C_OAR1_REG          0x08  /* 自身地址寄存器1 */
#define PIXART2860_I2C_OAR2_REG          0x0C  /* 自身地址寄存器2 */
#define PIXART2860_I2C_DR_REG            0x10  /* 数据寄存器 */
#define PIXART2860_I2C_SR1_REG           0x14  /* 状态寄存器1 */
#define PIXART2860_I2C_SR2_REG           0x18  /* 状态寄存器2 */
#define PIXART2860_I2B_CCR_REG           0x1C  /* 时钟控制寄存器 */
#define PIXART2860_I2C_TRISE_REG         0x20  /* 上升时间寄存器 */
#define PIXART2860_I2C_FLTR_REG          0x24  /* 滤波器寄存器 */

/* Pixart2860 I2C控制寄存器1位定义 */
#define PIXART2860_I2C_CR1_PE            (1 << 0)    /* 外设使能 */
#define PIXART2860_I2C_CR1_SMBUS         (1 << 1)    /* SMBUS模式 */
#define PIXART2860_I2C_CR1_SMBTYPE       (1 << 3)    /* SMBUS类型 */
#define PIXART2860_I2C_CR1_ENARP         (1 << 4)    /* ARP使能 */
#define PIXART2860_I2C_CR1_ENPEC         (1 << 5)    /* PEC使能 */
#define PIXART2860_I2C_CR1_ENGC          (1 << 6)    /* 通用呼叫使能 */
#define PIXART2860_I2C_CR1_NOSTRETCH    (1 << 7)    /* 时钟拉伸禁止 */
#define PIXART2860_I2C_CR1_START        (1 << 8)    /* 起始位 */
#define PIXART2860_I2C_CR1_STOP         (1 << 9)    /* 停止位 */
#define PIXART2860_I2C_CR1_ACK          (1 << 10)   /* 应答使能 */
#define PIXART2860_I2C_CR1_POS          (1 << 11)   /* 字符位置 */
#define PIXART2860_I2C_CR1_PEC          (1 << 12)   /* 数据包错误检查 */
#define PIXART2860_I2C_CR1_ALERT        (1 << 13)   /* SMALERT位 */
#define PIXART2860_I2C_CR1_SWRST        (1 << 15)   /* 软件复位 */

/* Pixart2860 I2C状态寄存器1位定义 */
#define PIXART2860_I2C_SR1_SB           (1 << 0)    /* 起始位 */
#define PIXART2860_I2C_SR1_ADDR         (1 << 1)    /* 地址发送 */
#define PIXART2860_I2C_SR1_BTF          (1 << 2)    /* 字节传输完成 */
#define PIXART2860_I2C_SR1_ADD10        (1 << 3)    /* 10位地址头发送 */
#define PIXART2860_I2C_SR1_STOPF        (1 << 4)    /* 停止位检测 */
#define PIXART2860_I2C_SR1_RXNE         (1 << 6)    /* 数据寄存器非空 */
#define PIXART2860_I2C_SR1_TXE          (1 << 7)    /* 数据寄存器为空 */
#define PIXART2860_I2C_SR1_BERR         (1 << 8)    /* 总线错误 */
#define PIXART2860_I2C_SR1_ARLO         (1 << 9)    /* 仲裁丢失 */
#define PIXART2860_I2C_SR1_AF           (1 << 10)   /* 应答失败 */
#define PIXART2860_I2C_SR1_OVR          (1 << 11)   /* 过载/下溢 */
#define PIXART2860_I2C_SR1_PECERR       (1 << 12)   /* PEC错误 */
#define PIXART2860_I2C_SR1_TIMEOUT      (1 << 14)   /* 超时 */
#define PIXART2860_I2C_SR1_SMBALERT     (1 << 15)   /* SMBUS警报 */

/* Pixart2860 I2C中断定义 */
#define PIXART2860_I2C_IRQn(bus)        (6 + (bus))
#define PIXART2860_I2C_IRQ_PRIORITY     4

/* Pixart2860 I2C时钟使能 */
#define PIXART2860_I2C_CLK_ENABLE(bus)   do { \
    /* 启用I2C时钟 */ \
} while(0)

#define PIXART2860_I2C_CLK_DISABLE(bus)  do { \
    /* 禁用I2C时钟 */ \
} while(0)

/* Pixart2860 I2C GPIO引脚定义 */
#define PIXART2860_I2C0_SCL_PIN          14  /* I2C0 SCL引脚 */
#define PIXART2860_I2C0_SDA_PIN          15  /* I2C0 SDA引脚 */
#define PIXART2860_I2C1_SCL_PIN          0   /* I2C1 SCL引脚 */
#define PIXART2860_I2C1_SDA_PIN          1   /* I2C1 SDA引脚 */

/* Pixart2860 I2C最大时钟频率 */
#define PIXART2860_I2C_MAX_FREQ_KHZ      400 /* 最大时钟频率400kHz */
#define PIXART2860_I2C_STD_FREQ_KHZ      100 /* 标准时钟频率100kHz */

/* Pixart2860 I2C超时时间 */
#define PIXART2860_I2C_TIMEOUT_MS        100 /* 默认超时时间100ms */

/* Pixart2860 I2C配置参数 */
typedef struct {
    uint8_t                 bus_num;            /* I2C总线号 */
    i2c_speed_t            speed;              /* I2C速度 */
    i2c_address_size_t      address_size;       /* 地址位数 */
    i2c_mode_t              mode;               /* 操作模式 */
    i2c_clock_stretching_t  clock_stretching;   /* 时钟拉伸设置 */
    i2c_dual_address_t      dual_address;       /* 双地址模式 */
    i2c_general_call_t      general_call;       /* 通用呼叫设置 */
    uint16_t                timeout_ms;         /* 操作超时时间(毫秒) */
    uint16_t                own_address;        /* 自身地址 */
    uint16_t                dual_address_val;   /* 双地址值 */
    bool                    auto_retries;       /* 自动重试 */
    uint8_t                 retry_count;        /* 重试次数 */
    bool                    dma_enabled;        /* DMA使能 */
    uint8_t                 priority;           /* 中断优先级 */
    uint32_t                input_clock_hz;     /* 输入时钟频率 */
    uint32_t                ccr_value;          /* 时钟控制寄存器值 */
    uint32_t                trise_value;        /* 上升时间寄存器值 */
    bool                    filter_enable;      /* 滤波器使能 */
    uint8_t                 filter_value;       /* 滤波器值 */
} pixart2860_i2c_config_t;

/* Pixart2860 I2C实例结构体 */
typedef struct {
    volatile uint32_t      *cr1_reg;            /* 控制寄存器1 */
    volatile uint32_t      *cr2_reg;            /* 控制寄存器2 */
    volatile uint32_t      *oar1_reg;           /* 自身地址寄存器1 */
    volatile uint32_t      *oar2_reg;           /* 自身地址寄存器2 */
    volatile uint32_t      *dr_reg;             /* 数据寄存器 */
    volatile uint32_t      *sr1_reg;            /* 状态寄存器1 */
    volatile uint32_t      *sr2_reg;            /* 状态寄存器2 */
    volatile uint32_t      *ccr_reg;            /* 时钟控制寄存器 */
    volatile uint32_t      *trise_reg;          /* 上升时间寄存器 */
    volatile uint32_t      *fltr_reg;           /* 滤波器寄存器 */
    pixart2860_i2c_config_t *config;            /* 配置参数 */
    i2c_callback_t         callback;           /* 中断回调函数 */
    void                  *user_data;          /* 用户数据 */
    bool                   initialized;        /* 初始化标志 */
    bool                   transfer_active;    /* 传输活跃标志 */
    uint8_t                *write_buffer;       /* 写入缓冲区 */
    uint8_t                *read_buffer;        /* 读取缓冲区 */
    uint16_t               write_size;          /* 写入数据大小 */
    uint16_t               read_size;           /* 读取数据大小 */
    uint16_t               bytes_written;       /* 已写入字节数 */
    uint16_t               bytes_read;          /* 已读取字节数 */
    uint16_t                device_address;     /* 设备地址 */
    uint16_t                register_address;   /* 寄存器地址 */
    bool                    combined_transfer;  /* 组合传输标志 */
    bool                    restart_condition;  /* 重启条件标志 */
    uint32_t               timeout_counter;    /* 超时计数器 */
    uint8_t                 retry_count;        /* 当前重试次数 */
    uint32_t               error_count;        /* 错误计数 */
} pixart2860_i2c_instance_t;

/* Pixart2860 I2C函数声明 */

/**
 * @brief 获取Pixart2860 I2C接口
 * @return I2C接口指针
 */
const i2c_interface_t *pixart2860_i2c_get_interface(void);

/**
 * @brief 配置I2C总线
 * @param bus_num I2C总线号
 * @param config I2C配置参数
 * @return 配置结果
 */
i2c_result_t pixart2860_i2c_config_bus(uint8_t bus_num, const pixart2860_i2c_config_t *config);

/**
 * @brief 配置I2C时钟
 * @param bus_num I2C总线号
 * @param speed I2C速度
 * @param input_clock_hz 输入时钟频率
 * @return 配置结果
 */
i2c_result_t pixart2860_i2c_config_clock(uint8_t bus_num, i2c_speed_t speed, uint32_t input_clock_hz);

/**
 * @brief 配置I2C滤波器
 * @param bus_num I2C总线号
 * @param enable 滤波器使能
 * @param filter_value 滤波器值
 * @return 配置结果
 */
i2c_result_t pixart2860_i2c_config_filter(uint8_t bus_num, bool enable, uint8_t filter_value);

/**
 * @brief 配置I2C地址
 * @param bus_num I2C总线号
 * @param own_address 自身地址
 * @return 配置结果
 */
i2c_result_t pixart2860_i2c_config_address(uint8_t bus_num, uint16_t own_address);

/**
 * @brief 配置I2C双地址
 * @param bus_num I2C总线号
 * @param dual_address 双地址值
 * @return 配置结果
 */
i2c_result_t pixart2860_i2c_config_dual_address(uint8_t bus_num, uint16_t dual_address);

/**
 * @brief 配置I2C通用呼叫
 * @param bus_num I2C总线号
 * @param enable 通用呼叫使能
 * @return 配置结果
 */
i2c_result_t pixart2860_i2c_config_general_call(uint8_t bus_num, bool enable);

/**
 * @brief 启动I2C传输
 * @param bus_num I2C总线号
 * @param device_address 设备地址
 * @param operation 操作类型
 * @return 启动结果
 */
i2c_result_t pixart2860_i2c_start_transfer(uint8_t bus_num, uint16_t device_address, uint8_t operation);

/**
 * @brief 停止I2C传输
 * @param bus_num I2C总线号
 * @return 停止结果
 */
i2c_result_t pixart2860_i2c_stop_transfer(uint8_t bus_num);

/**
 * @brief 发送I2C数据
 * @param bus_num I2C总线号
 * @param data 发送数据
 * @return 发送结果
 */
i2c_result_t pixart2860_i2c_send_byte(uint8_t bus_num, uint8_t data);

/**
 * @brief 接收I2C数据
 * @param bus_num I2C总线号
 * @param data 输出接收数据
 * @param ack 是否发送应答
 * @return 接收结果
 */
i2c_result_t pixart2860_i2c_receive_byte(uint8_t bus_num, uint8_t *data, bool ack);

/**
 * @brief 等待I2C事件
 * @param bus_num I2C总线号
 * @param event_mask 事件掩码
 * @param timeout_ms 超时时间(毫秒)
 * @return 等待结果
 */
i2c_result_t pixart2860_i2c_wait_event(uint8_t bus_num, uint32_t event_mask, uint32_t timeout_ms);

/**
 * @brief 检查I2C事件
 * @param bus_num I2C总线号
 * @param event_mask 事件掩码
 * @return 检查结果
 */
bool pixart2860_i2c_check_event(uint8_t bus_num, uint32_t event_mask);

/**
 * @brief 清除I2C事件
 * @param bus_num I2C总线号
 * @param event_mask 事件掩码
 * @return 清除结果
 */
i2c_result_t pixart2860_i2c_clear_event(uint8_t bus_num, uint32_t event_mask);

/**
 * @brief I2C中断处理函数
 * @param bus_num I2C总线号
 * @return 处理结果
 */
void pixart2860_i2c_irq_handler(uint8_t bus_num);

/**
 * @brief 计算I2C时钟控制值
 * @param speed I2C速度
 * @param input_clock_hz 输入时钟频率
 * @return 时钟控制值
 */
uint32_t pixart2860_i2c_calculate_ccr(i2c_speed_t speed, uint32_t input_clock_hz);

/**
 * @brief 计算I2C上升时间值
 * @param speed I2C速度
 * @param input_clock_hz 输入时钟频率
 * @return 上升时间值
 */
uint32_t pixart2860_i2c_calculate_trise(i2c_speed_t speed, uint32_t input_clock_hz);

/**
 * @brief 扫描I2C设备
 * @param bus_num I2C总线号
 * @param found_devices 输出找到的设备地址数组
 * @param max_devices 最大设备数量
 * @param device_count 输出找到的设备数量
 * @return 扫描结果
 */
i2c_result_t pixart2860_i2c_scan_devices(uint8_t bus_num, uint16_t *found_devices, uint8_t max_devices, uint8_t *device_count);

/* 便捷宏定义 */
#define PIXART2860_I2C_CONFIG_DEFAULT(bus) \
    { (bus), I2C_SPEED_STANDARD, I2C_ADDRESS_7BIT, I2C_MODE_POLLING, I2C_CLOCK_STRETCHING_ENABLE, I2C_DUAL_ADDRESS_DISABLE, I2C_GENERAL_CALL_DISABLE, 1000, 0, 0, false, 0, false, 0, 48000000, 0, 0, true, 0 }

#define PIXART2860_I2C_CONFIG_FAST(bus) \
    { (bus), I2C_SPEED_FAST, I2C_ADDRESS_7BIT, I2C_MODE_INTERRUPT, I2C_CLOCK_STRETCHING_ENABLE, I2C_DUAL_ADDRESS_DISABLE, I2C_GENERAL_CALL_DISABLE, 500, 0, 0, false, 0, false, 0, 48000000, 0, 0, true, 0 }

#define PIXART2860_I2C_CONFIG_HIGH_SPEED(bus) \
    { (bus), I2C_SPEED_FAST_PLUS, I2C_ADDRESS_7BIT, I2C_MODE_DMA, I2C_CLOCK_STRETCHING_DISABLE, I2C_DUAL_ADDRESS_DISABLE, I2C_GENERAL_CALL_DISABLE, 100, 0, 0, false, 0, true, 0, 48000000, 0, 0, false, 1 }

/* 寄存器操作宏 */
#define PIXART2860_I2C_GET_REG(bus, offset) \
    (*((volatile uint32_t *)(PIXART2860_I2C_BUS_BASE(bus) + (offset))))

#define PIXART2860_I2C_SET_REG(bus, offset, value) \
    (*((volatile uint32_t *)(PIXART2860_I2C_BUS_BASE(bus) + (offset))) = (value))

#define PIXART2860_I2C_MODIFY_REG(bus, offset, mask, value) \
    (*((volatile uint32_t *)(PIXART2860_I2C_BUS_BASE(bus) + (offset))) = \
     ((*((volatile uint32_t *)(PIXART2860_I2C_BUS_BASE(bus) + (offset))) & ~(mask)) | ((value) & (mask))))

/* 状态标志操作宏 */
#define PIXART2860_I2C_GET_FLAG(bus, flag) \
    (PIXART2860_I2C_GET_REG(bus, PIXART2860_I2C_SR1_REG) & (flag))

#define PIXART2860_I2C_SET_FLAG(bus, flag) \
    (PIXART2860_I2C_GET_REG(bus, PIXART2860_I2C_SR1_REG) | (flag))

#define PIXART2860_I2C_CLR_FLAG(bus, flag) \
    (PIXART2860_I2C_GET_REG(bus, PIXART2860_I2C_SR1_REG) & ~(flag))

#define PIXART2860_I2C_IS_BUSY(bus) \
    (PIXART2860_I2C_GET_REG(bus, PIXART2860_I2C_SR2_REG) & 0x0001)

#define PIXART2860_I2C_DATA_REGISTER_EMPTY(bus) \
    (PIXART2860_I2C_GET_REG(bus, PIXART2860_I2C_SR1_REG) & PIXART2860_I2C_SR1_TXE)

#define PIXART2860_I2C_DATA_REGISTER_NOT_EMPTY(bus) \
    (PIXART2860_I2C_GET_REG(bus, PIXART2860_I2C_SR1_REG) & PIXART2860_I2C_SR1_RXNE)

/* 常用I2C设备地址定义 */
#define PIXART2860_I2C_TOUCHPAD_ADDR    0x38    /* 触控板地址 */
#define PIXART2860_I2C_EEPROM_ADDR      0x50    /* EEPROM地址 */
#define PIXART2860_I2C_ACCEL_ADDR       0x1C    /* 加速度传感器地址 */
#define PIXART2860_I2C_GYRO_ADDR        0x6B    /* 陀螺仪地址 */
#define PIXART2860_I2C_COMPASS_ADDR     0x1E    /* 磁力计地址 */
#define PIXART2860_I2C_RTC_ADDR         0x68    /* 实时时钟地址 */
#define PIXART2860_I2C_OLED_ADDR        0x3C    /* OLED显示地址 */

/* 错误处理宏 */
#define PIXART2860_I2C_WAIT_EVENT(bus, flag, timeout) \
    do { \
        uint32_t _start = timer_hal->get_tick(); \
        while (!pixart2860_i2c_check_event((bus), (flag))) { \
            if (timer_hal->get_elapsed_ms(_start, timer_hal->get_tick()) > (timeout)) { \
                return I2C_ERROR_TIMEOUT; \
            } \
        } \
    } while(0)

#define PIXART2860_I2C_WAIT_FLAG_SET(bus, flag, timeout) \
    PIXART2860_I2C_WAIT_EVENT((bus), (flag), (timeout))

#define PIXART2860_I2C_WAIT_FLAG_CLR(bus, flag, timeout) \
    do { \
        uint32_t _start = timer_hal->get_tick(); \
        while (pixart2860_i2c_check_event((bus), (flag))) { \
            if (timer_hal->get_elapsed_ms(_start, timer_hal->get_tick()) > (timeout)) { \
                return I2C_ERROR_TIMEOUT; \
            } \
        } \
    } while(0)

/* 温度补偿宏 */
#define PIXART2860_I2C_TEMPERATURE_COMPENSATION(speed, temp_c) \
    ((speed) == I2C_SPEED_HIGH_SPEED ? \
     ((temp_c) > 60.0f ? 1 : 0) : 0)

#ifdef __cplusplus
}
#endif

#endif /* I2C_HAL_PIXART2860_H */