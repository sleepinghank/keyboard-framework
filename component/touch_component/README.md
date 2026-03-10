# Touch Component 触控组件库

## 项目简介

Touch Component 是一个完整的触控板控制组件库，支持 PCT1336 触控芯片，提供鼠标模式和 PTP（Precision Touchpad）模式两种操作模式。该组件集成了手势识别、防误触算法、驱动层控制等功能，适用于嵌入式系统中的触控板应用开发。

## 目录结构

```
touch_component/
├── include/                    # 头文件目录
│   ├── bayes_filtering.h      # 贝叶斯防误触算法
│   ├── mcs_gesture.h          # 手势识别算法
│   ├── pct1336_driver.h       # PCT1336驱动接口
│   └── touchpad_service.h     # 触控服务层接口
├── bayes_filtering.c          # 贝叶斯防误触算法实现
├── mcs_gesture.c              # 手势识别算法实现
├── pct1336_driver.c           # PCT1336驱动实现
├── touchpad_service.c         # 触控服务层实现
└── README.md                  # 项目说明文档
```

## 功能特性

### 1. 触控模式支持
- **鼠标模式（Mouse Mode）**：传统鼠标操作模式，支持相对移动、滚轮操作
- **PTP模式（Precision Touchpad Mode）**：精确触控板模式，支持多点触控、精确坐标报告

### 2. 手势识别
支持以下基本手势识别：
- S型手势
- Z型手势
- N型手势
- W型手势

### 3. 防误触算法
采用贝叶斯算法进行误触检测，提高触控准确性。

### 4. 驱动支持
完整支持PCT1336触控芯片，包括初始化、数据读取、状态管理等功能。

## API 接口说明

### 触控服务层接口 (touchpad_service.h)

#### 初始化相关函数

```c
/// @brief 触控板开机，会延迟初始化，是否启动成功请参考touch_en值
/// @return Success(1) or Failure(0)
int8_t touch_power_on(void);

/// @brief 带参数的开机，会延迟初始化，是否启动成功请参考touch_en值
/// @param params 参数指针
/// @param len 参数长度，以3个字节为一个单位
/// @return Success(1) or Failure(0)
int8_t touch_power_on_with_params(pct1336_params_t* params, uint8_t len);

/// @brief 触控板关机，默认GPIO 都为上拉输入
/// @return Success(1) or Failure(0)
int8_t touch_power_off(void);
```

#### 任务处理函数

```c
/// @brief 触控板定时器任务处理，放到系统定时器中调用
/// @return Success(1) or Failure(0)
int8_t touch_timer_task(void);

/// @brief 触控板事件处理，放到main主循环中调用
void touch_evt_task(void);
```

#### 配置函数

```c
/// @brief 设置触摸板模式
/// @param mode 触摸板模式 (TOUCH_MODE_MOUSE 或 TOUCH_MODE_PTP)
void set_touch_mode(touch_mode_t mode);

/// @brief 设置键盘打字隔离触摸板时间
/// @param cnt 打字隔离时间，单位为扫描定时器时间，默认应该设置为400ms
void set_kb_break_cnt(uint16_t cnt);

/// @brief 获取触摸板扫描时间
/// @return scan_time 触摸板扫描时间
uint16_t get_touch_scan_time(void);
```

### PCT1336驱动接口 (pct1336_driver.h)

#### 核心驱动函数

```c
/// @brief 触控板初始化
/// @return Success(1) or Failure(0)
int8_t pct1336_init();

/// @brief 触控板复位方法
/// @return Success(1) or Failure(0)
int8_t pct1336_resume(void);

/// @brief 触控板深度睡眠方法
/// @return Success(1) or Failure(0)
int8_t pct1336_sleep(void);

/// @brief 获取PTP 触摸数据，用于实现PTP 触控板
/// @param *ptp 原始数据指针 
/// @return 1：有数据 0：无数据
int8_t pct1336_read_ptp_report(touchpad_data_t *ptp);

/// @brief 获取鼠标数据
/// @param *gesture_type 手势类型
/// @param *x  鼠标X轴偏移量
/// @param *y  鼠标Y轴偏移量
/// @return 1：有数据 0：无数据
int8_t pct1336_read_mouse(uint16_t *x, uint16_t *y);

/// @brief 触控板中断清除
/// @return Success(1) or Failure(0)
int8_t pct1336_clear_int(void);

/// @brief 获取触控板状态
/// @param *st 触摸状态 不能为NULL
/// @param *button_st 按钮状态  可以为NULL
/// @param *gesture_st 手势状态  可以为NULL
/// @return Success(1) or Failure(0)
int8_t pct1336_read_status(uint8_t *st, uint8_t *button_st , uint8_t *gesture_st);

/// @brief 设置触控板初始化参数
/// @param params 具体参数
/// @param len 参数长度，以3个字节为一个单位
void pct1336_set_init_params(pct1336_params_t* params, uint8_t len);
```

### 手势识别接口 (mcs_gesture.h)

```c
/// @brief 识别手势类型
/// @param input_points 输入的手势坐标点数组
/// @param point_count 坐标点数量
/// @return 识别出的手势类型
GestureType recognize_gesture(Point* input_points, uint8_t point_count);

/// @brief 设置余弦相似度阈值，默认值0.75
/// @param threshold 阈值，范围0-1
void set_similarity_threshold(double threshold);

/// @brief 设置距离阈值 有默认值
/// @param thresholds 针对每个手势可以设置单独的距离阈值
void set_distance_thresholds(const distance_thresholds_t* thresholds);
```

### 贝叶斯防误触接口 (bayes_filtering.h)

```c
/// @brief 模型初始化
/// @param model 具体的参数
void bayes_mode_init(bayes_model_t* model);

/// @brief 计算 posterior probabilities
double* compute(double features[N_FEATURES]);

/// @brief 预测结果
/// @param features 特征向量
/// @return 预测结果
int predict(double features[]);

double* predictProba(double features[]);
```

## 数据结构说明

### 触控数据结构

```c
// 单手指数据
typedef struct contact_data {
    uint8_t contact_id;     // 接触ID
    uint8_t tip: 1;         // 是否接触
    uint8_t confidence: 1;  // 置信度
    uint8_t : 6;            // 保留位
    uint16_t x;             // X坐标
    uint16_t y;             // Y坐标
    uint16_t size;          // 接触大小
} contact_data_t;

// 完整触控数据
typedef struct touchpad_data {
    uint8_t contact_count;                    // 接触点数量
    contact_data_t contacts[TP_MAX_CONTACT_COUNT]; // 接触点数据
} touchpad_data_t;
```

### 手势识别数据结构

```c
// 点坐标结构
typedef struct {
    double x;
    double y;
} Point;

// 手势数据结构
typedef struct {
    Point points[MAX_FRAMES];      // 原始坐标序列
    int length;                    // 原始长度
    Point inv_points[MAX_FRAMES];  // 平移不变处理后的坐标
    Point resampled[FIXED_LENGTH]; // 固定长度坐标
    double feature[2*FIXED_LENGTH]; // 最终特征向量
} GestureData;
```

## 使用方法

### 1. 基本初始化

```c
// 初始化触控板
if (touch_power_on() == 1) {
    printf("Touchpad initialized successfully\n");
} else {
    printf("Failed to initialize touchpad\n");
}

// 设置触控模式为PTP模式
set_touch_mode(TOUCH_MODE_PTP);
```

### 2. 主循环处理

```c
// 在定时器中调用
void timer_handler(void) {
    if (touch_timer_task()) {
        // 有触控事件需要处理
    }
}

// 在主循环中调用
void main_loop(void) {
    touch_evt_task();
}
```

### 3. 配置参数

```c
// 设置键盘防误触时间
set_kb_break_cnt(400); // 400ms

// 设置手势识别阈值
set_similarity_threshold(0.8);
```

## 配置选项

### 编译选项

在 `config.h` 中定义以下宏来启用相关功能：

- `MCS_GESTURE`：启用手势识别功能
- `BAYES_MISTOUCH`：启用贝叶斯防误触算法
- `DOUBLE_CLICK_TO_RIGHT_BUTTON`：启用双击转右键功能
- `HOLD_BUTTON`：启用按压保持功能

### 参数配置

1. **触控板尺寸**：在 `touchpad_service.h` 中配置 `TOUCHPAD_MAX_X` 和 `TOUCHPAD_MAX_Y`
2. **防误触参数**：在 `bayes_filtering.c` 中配置 `default_model`
3. **手势识别参数**：在 `mcs_gesture.c` 中配置模板特征向量和阈值

## 注意事项

1. **初始化时序**：触控板上电后需要等待至少150ms再进行初始化
2. **中断处理**：需要正确配置GPIO中断来处理触控事件
3. **I2C配置**：确保I2C总线速度和引脚配置正确
5. **防误触**：根据实际使用场景调整防误触算法参数

## 故障排除

### 常见问题

1. **触控板无法初始化**
   - 检查电源是否正常
   - 确认I2C通信是否正常
   - 检查初始化时序是否符合要求

2. **触控数据异常**
   - 检查中断配置是否正确
   - 确认数据读取时序是否正确
   - 验证防误触算法参数设置

3. **手势识别不准确**
   - 调整相似度阈值
   - 重新校准手势模板
   - 检查触控数据质量

### 调试建议

1. 启用调试日志输出以跟踪触控事件
2. 使用示波器检查I2C通信波形
3. 验证GPIO中断触发是否正常

## 版本历史

- v1.0.0 (2025/10/31)：初始版本，支持PCT1336触控芯片，提供鼠标和PTP两种模式，集成手势识别和防误触算法。
