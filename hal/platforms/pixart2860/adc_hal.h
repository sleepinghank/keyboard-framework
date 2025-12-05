/**
 * @file adc_hal.h
 * @brief Pixart2860 ADC硬件抽象层实现
 * @author Keyboard Framework Team
 * @version 1.0
 * @date 2024
 *
 * Pixart2860平台的ADC驱动实现
 */

#ifndef ADC_HAL_PIXART2860_H
#define ADC_HAL_PIXART2860_H

#include "../interface/adc_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Pixart2860 ADC通道定义 */
#define PIXART2860_ADC_CHANNEL_0        0
#define PIXART2860_ADC_CHANNEL_1        1
#define PIXART2860_ADC_CHANNEL_2        2
#define PIXART2860_ADC_CHANNEL_3        3
#define PIXART2860_ADC_CHANNEL_4        4
#define PIXART2860_ADC_CHANNEL_5        5
#define PIXART2860_ADC_CHANNEL_6        6
#define PIXART2860_ADC_CHANNEL_7        7
#define PIXART2860_ADC_CHANNEL_8        8
#define PIXART2860_ADC_CHANNEL_9        9
#define PIXART2860_ADC_CHANNEL_10       10
#define PIXART2860_ADC_CHANNEL_11       11
#define PIXART2860_ADC_CHANNEL_12       12
#define PIXART2860_ADC_CHANNEL_13       13
#define PIXART2860_ADC_CHANNEL_14       14
#define PIXART2860_ADC_CHANNEL_15       15

/* Pixart2860 ADC专用通道定义 */
#define PIXART2860_ADC_CHANNEL_VREF     16  /* 内部参考电压 */
#define PIXART2860_ADC_CHANNEL_TEMP     17  /* 温度传感器 */
#define PIXART2860_ADC_CHANNEL_VBAT     18  /* 电池电压 */

/* Pixart2860 ADC通道数量 */
#define PIXART2860_ADC_CHANNEL_COUNT    19

/* Pixart2860 ADC寄存器基址 */
#define PIXART2860_ADC_BASE             0x40001000UL

/* Pixart2860 ADC寄存器偏移 */
#define PIXART2860_ADC_CR_REG          0x00  /* 控制寄存器 */
#define PIXART2860_ADC_SR_REG          0x04  /* 状态寄存器 */
#define PIXART2860_ADC_DR_REG          0x08  /* 数据寄存器 */
#define PIXART2860_ADC_SMPR1_REG       0x0C  /* 采样时间寄存器1 */
#define PIXART2860_ADC_SMPR2_REG       0x10  /* 采样时间寄存器2 */
#define PIXART2860_ADC_JOFR1_REG       0x14  /* 注入通道偏移寄存器1 */
#define PIXART2860_ADC_JOFR2_REG       0x18  /* 注入通道偏移寄存器2 */
#define PIXART2860_ADC_JOFR3_REG       0x1C  /* 注入通道偏移寄存器3 */
#define PIXART2860_ADC_JOFR4_REG       0x20  /* 注入通道偏移寄存器4 */
#define PIXART2860_ADC_HTR_REG         0x24  /* 高阈值寄存器 */
#define PIXART2860_ADC_LTR_REG         0x28  /* 低阈值寄存器 */
#define PIXART2860_ADC_SQR1_REG        0x2C  /* 规则序列寄存器1 */
#define PIXART2860_ADC_SQR2_REG        0x30  /* 规则序列寄存器2 */
#define PIXART2860_ADC_SQR3_REG        0x34  /* 规则序列寄存器3 */
#define PIXART2860_ADC_JSQR_REG        0x38  /* 注入序列寄存器 */
#define PIXART2860_ADC_JDR1_REG        0x3C  /* 注入数据寄存器1 */
#define PIXART2860_ADC_JDR2_REG        0x40  /* 注入数据寄存器2 */
#define PIXART2860_ADC_JDR3_REG        0x44  /* 注入数据寄存器3 */
#define PIXART2860_ADC_JDR4_REG        0x48  /* 注入数据寄存器4 */

/* Pixart2860 ADC控制寄存器位定义 */
#define PIXART2860_ADC_CR_ADON          (1 << 0)    /* ADC使能位 */
#define PIXART2860_ADC_CR_CONT         (1 << 1)    /* 连续转换位 */
#define PIXART2860_ADC_CR_CAL          (1 << 2)    /* 校准位 */
#define PIXART2860_ADC_CR_RSTCAL       (1 << 3)    /* 复位校准位 */
#define PIXART2860_ADC_CR_ALIGN        (1 << 4)    /* 数据对齐位 */
#define PIXART2860_ADC_CR_JEOC_IE      (1 << 7)    /* 注入转换完成中断使能位 */
#define PIXART2860_ADC_CR_EOC_IE       (1 << 8)    /* 转换完成中断使能位 */
#define PIXART2860_ADC_CR_SCAN         (1 << 9)    /* 扫描模式位 */
#define PIXART2860_ADC_CR_JSWSTART     (1 << 21)   /* 注入组软件启动位 */
#define PIXART2860_ADC_CR_JEXTTRIG     (1 << 22)   /* 注入组外部触发使能位 */
#define PIXART2860_ADC_CR_JEXTTRIG_SHIFT  20      /* 注入组外部触发选择移位 */
#define PIXART2860_ADC_CR_SWSTART      (1 << 22)   /* 规则组软件启动位 */
#define PIXART2860_ADC_CR_EXTTRIG      (1 << 23)   /* 规则组外部触发使能位 */
#define PIXART2860_ADC_CR_EXTTRIG_SHIFT   17      /* 规则组外部触发选择移位 */

/* Pixart2860 ADC状态寄存器位定义 */
#define PIXART2860_ADC_SR_AWD          (1 << 0)    /* 模拟看门狗位 */
#define PIXART2860_ADC_SR_EOC          (1 << 1)    /* 转换结束位 */
#define PIXART2860_ADC_SR_JEOC         (1 << 2)    /* 注入组转换结束位 */
#define PIXART2860_ADC_SR_JSTRT        (1 << 3)    /* 注入组开始位 */
#define PIXART2860_ADC_SR_STRT         (1 << 4)    /* 规则组开始位 */
#define PIXART2860_ADC_SR_ADSTP        (1 << 5)    /* ADC停止位 */

/* Pixart2860 ADC中断定义 */
#define PIXART2860_ADC_IRQn            1
#define PIXART2860_ADC_IRQ_PRIORITY    4

/* Pixart2860 ADC时钟使能 */
#define PIXART2860_ADC_CLK_ENABLE()    do { \
    /* 启用ADC时钟 */ \
} while(0)

#define PIXART2860_ADC_CLK_DISABLE()   do { \
    /* 禁用ADC时钟 */ \
} while(0)

/* Pixart2860 ADC参考电压定义 */
#define PIXART2860_ADC_VREF_INTERNAL   1200    /* 内部参考电压(mV) */
#define PIXART2860_ADC_VREF_VDDA       3300    /* VDDA参考电压(mV) */

/* Pixart2860 ADC最大转换时间 */
#define PIXART2860_ADC_MAX_TIME_US     10      /* 最大转换时间(微秒) */

/* Pixart2860 ADC配置参数 */
typedef struct {
    uint8_t                 channel;        /* ADC通道号 */
    adc_channel_type_t      type;           /* 通道类型 */
    adc_resolution_t        resolution;     /* 分辨率 */
    adc_reference_t         reference;      /* 参考电压 */
    adc_sample_time_t       sample_time;    /* 采样时间 */
    bool                    differential;   /* 是否差分模式 */
    uint8_t                 differential_pair; /* 差分对编号 */
    float                   offset;          /* 偏移校准值 */
    float                   scale;           /* 比例校准值 */
    uint32_t                sample_time_cycles; /* 采样周期数 */
    uint32_t                channel_sequence; /* 通道序列号 */
    bool                    watchdog_enable; /* 看门狗使能 */
    uint16_t                watchdog_high;    /* 看门狗高阈值 */
    uint16_t                watchdog_low;     /* 看门狗低阈值 */
} pixart2860_adc_config_t;

/* Pixart2860 ADC实例结构体 */
typedef struct {
    volatile uint32_t      *cr_reg;          /* 控制寄存器 */
    volatile uint32_t      *sr_reg;          /* 状态寄存器 */
    volatile uint32_t      *dr_reg;          /* 数据寄存器 */
    volatile uint32_t      *smpr1_reg;       /* 采样时间寄存器1 */
    volatile uint32_t      *smpr2_reg;       /* 采样时间寄存器2 */
    volatile uint32_t      *sqr1_reg;        /* 规则序列寄存器1 */
    volatile uint32_t      *sqr2_reg;        /* 规则序列寄存器2 */
    volatile uint32_t      *sqr3_reg;        /* 规则序列寄存器3 */
    volatile uint32_t      *jsqr_reg;        /* 注入序列寄存器 */
    volatile uint32_t      *htr_reg;         /* 高阈值寄存器 */
    volatile uint32_t      *ltr_reg;         /* 低阈值寄存器 */
    pixart2860_adc_config_t *configs;        /* 通道配置数组 */
    adc_conversion_callback_t *callbacks;     /* 转换完成回调数组 */
    void                  **user_data;       /* 用户数据数组 */
    uint32_t              *conversion_results; /* 转换结果数组 */
    bool                   initialized;      /* 初始化标志 */
    bool                   conversion_active; /* 转换活跃标志 */
    uint8_t                current_channel;  /* 当前转换通道 */
    uint8_t                remaining_channels; /* 剩余通道数 */
    uint16_t               conversion_count; /* 转换次数 */
    uint32_t               timeout_counter;  /* 超时计数器 */
} pixart2860_adc_instance_t;

/* Pixart2860 ADC函数声明 */

/**
 * @brief 获取Pixart2860 ADC接口
 * @return ADC接口指针
 */
const adc_interface_t *pixart2860_adc_get_interface(void);

/**
 * @brief 配置ADC通道采样时间
 * @param channel ADC通道号
 * @param sample_time 采样时间
 * @return 配置结果
 */
adc_result_t pixart2860_adc_config_sample_time(uint8_t channel, adc_sample_time_t sample_time);

/**
 * @brief 配置ADC通道序列
 * @param channel ADC通道号
 * @param sequence 序列号
 * @return 配置结果
 */
adc_result_t pixart2860_adc_config_channel_sequence(uint8_t channel, uint8_t sequence);

/**
 * @brief 配置ADC看门狗
 * @param high_threshold 高阈值
 * @param low_threshold 低阈值
 * @return 配置结果
 */
adc_result_t pixart2860_adc_config_watchdog(uint16_t high_threshold, uint16_t low_threshold);

/**
 * @brief 启动ADC校准
 * @return 校准结果
 */
adc_result_t pixart2860_adc_start_calibration(void);

/**
 * @brief 等待ADC校准完成
 * @param timeout_ms 超时时间(毫秒)
 * @return 等待结果
 */
adc_result_t pixart2860_adc_wait_calibration(uint32_t timeout_ms);

/**
 * @brief 读取ADC原始数据
 * @param channel ADC通道号
 * @param raw_data 输出原始数据
 * @return 读取结果
 */
adc_result_t pixart2860_adc_read_raw(uint8_t channel, uint16_t *raw_data);

/**
 * @brief 转换原始数据为电压值
 * @param raw_data 原始数据
 * @param voltage 输出电压值(mV)
 * @param reference 参考电压(mV)
 * @param resolution 分辨率
 * @return 转换结果
 */
adc_result_t pixart2860_adc_convert_to_voltage(uint16_t raw_data, uint32_t *voltage, uint32_t reference, adc_resolution_t resolution);

/**
 * @brief 转换电压值为原始数据
 * @param voltage 电压值(mV)
 * @param raw_data 输出原始数据
 * @param reference 参考电压(mV)
 * @param resolution 分辨率
 * @return 转换结果
 */
adc_result_t pixart2860_adc_convert_from_voltage(uint32_t voltage, uint16_t *raw_data, uint32_t reference, adc_resolution_t resolution);

/**
 * @brief ADC中断处理函数
 * @return 处理结果
 */
void pixart2860_adc_irq_handler(void);

/**
 * @brief 获取ADC采样时间周期数
 * @param sample_time 采样时间
 * @return 采样周期数
 */
uint32_t pixart2860_adc_get_sample_time_cycles(adc_sample_time_t sample_time);

/**
 * @brief 计算ADC转换时间
 * @param sample_time 采样时间
 * @param resolution 分辨率
 * @return 转换时间(微秒)
 */
uint32_t pixart2860_adc_get_conversion_time(adc_sample_time_t sample_time, adc_resolution_t resolution);

/* 便捷宏定义 */
#define PIXART2860_ADC_CONFIG(channel, resolution, ref, sample_time) \
    { (channel), ADC_CHANNEL_SINGLE, (resolution), (ref), (sample_time), false, 0, 0.0f, 1.0f, 0, 0, false, 0, 0 }

#define PIXART2860_ADC_CONFIG_DIFF(channel, pair, resolution, ref, sample_time) \
    { (channel), ADC_CHANNEL_DIFFERENTIAL, (resolution), (ref), (sample_time), true, (pair), 0.0f, 1.0f, 0, 0, false, 0, 0 }

#define PIXART2860_ADC_CONFIG_BATTERY(channel) \
    { (channel), ADC_CHANNEL_VBAT, ADC_RESOLUTION_12BIT, ADC_REF_VDD, ADC_SAMPLE_TIME_128CYCLES, false, 0, 0.0f, 1.0f, 0, 0, true, 3300, 0 }

#define PIXART2860_ADC_CONFIG_TEMPERATURE(channel) \
    { (channel), ADC_CHANNEL_TEMPERATURE, ADC_RESOLUTION_12BIT, ADC_REF_INTERNAL, ADC_SAMPLE_TIME_256CYCLES, false, 0, 0.0f, 1.0f, 0, 0, true, 1200, 0 }

/* 寄存器操作宏 */
#define PIXART2860_ADC_GET_REG(offset) \
    (*((volatile uint32_t *)(PIXART2860_ADC_BASE + (offset))))

#define PIXART2860_ADC_SET_REG(offset, value) \
    (*((volatile uint32_t *)(PIXART2860_ADC_BASE + (offset))) = (value))

#define PIXART2860_ADC_MODIFY_REG(offset, mask, value) \
    (*((volatile uint32_t *)(PIXART2860_ADC_BASE + (offset))) = \
     ((*((volatile uint32_t *)(PIXART2860_ADC_BASE + (offset))) & ~(mask)) | ((value) & (mask)))

#define PIXART2860_ADC_GET_FLAG(flag) \
    (PIXART2860_ADC_GET_REG(PIXART2860_ADC_SR_REG) & (flag))

#define PIXART2860_ADC_SET_FLAG(flag) \
    (PIXART2860_ADC_SET_REG(PIXART2860_ADC_SR_REG, (flag)))

#define PIXART2860_ADC_CLR_FLAG(flag) \
    (PIXART2860_ADC_SET_REG(PIXART2860_ADC_SR_REG, ~(flag)))

/* 常用ADC通道配置宏 */
#define PIXART2860_ADC_BATTERY_CHANNEL    PIXART2860_ADC_CHANNEL_0   /* 电池电压检测通道 */
#define PIXART2860_ADC_TOUCH_CHANNEL      PIXART2860_ADC_CHANNEL_1   /* 触控板ADC通道 */
#define PIXART2860_ADC_TEMP_SENSOR_CHANNEL PIXART2860_ADC_CHANNEL_TEMP /* 温度传感器通道 */
#define PIXART2860_ADC_VREF_CHANNEL      PIXART2860_ADC_CHANNEL_VREF /* 参考电压通道 */
#define PIXART2860_ADC_LIGHT_CHANNEL     PIXART2860_ADC_CHANNEL_2   /* 光敏电阻通道 */
#define PIXART2860_ADC_POTENTIOMETER_CHANNEL PIXART2860_ADC_CHANNEL_3 /* 电位器通道 */

/* ADC电压转换宏 */
#define PIXART2860_ADC_VALUE_TO_MV(value, ref_volt, resolution) \
    (((uint32_t)(value) * (ref_volt)) / ((1U << (resolution)) - 1))

#define PIXART2860_ADC_MV_TO_VALUE(voltage, ref_volt, resolution) \
    (((uint16_t)((voltage) * ((1U << (resolution)) - 1)) + ((ref_volt) >> 1)) / (ref_volt))

#define PIXART2860_ADC_VALUE_TO_PERCENTAGE(value) \
    (((value) * 100) / ((1U << 12) - 1))

#define PIXART2860_ADC_BATTERY_VOLTAGE_TO_PERCENTAGE(voltage_mv, full_voltage_mv, empty_voltage_mv) \
    ((((voltage_ms) <= (empty_voltage_ms)) ? 0 : \
      (((voltage_ms) >= (full_voltage_ms)) ? 100 : \
       (((voltage_ms) - (empty_voltage_ms)) * 100 / ((full_voltage_ms) - (empty_voltage_ms)))))

#ifdef __cplusplus
}
#endif

#endif /* ADC_HAL_PIXART2860_H */