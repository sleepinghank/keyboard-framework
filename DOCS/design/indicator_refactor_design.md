# 指示灯模块重构设计文档

## 一、设计目标

| 目标 | 说明 |
|-----|------|
| 简单设计 | 驱动层只关心"灯"和"灯效"，不区分业务含义 |
| 高内聚低耦合 | 各层职责单一，通过接口交互 |
| 后者覆盖 | 灯共用时，后设置的状态自动覆盖前者 |
| 易扩展 | 新增灯效或硬件控制方式时改动最小 |
| 易使用 | 通过别名快速访问，减少出错 |

---

## 二、整体架构

```
┌─────────────────────────────────────────────────────────────┐
│                    Application Layer                         │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  业务状态机 (wireless_indicator, battery_indicator)     │ │
│  │  - 监听业务事件                                         │ │
│  │  - 通过别名设置灯效: indicator_set(LED_BT1, &effect)    │ │
│  └────────────────────────────────────────────────────────┘ │
└─────────────────────────────┬───────────────────────────────┘
                              │ indicator_set / indicator_off
┌─────────────────────────────▼───────────────────────────────┐
│                     Driver Layer                             │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  indicator_driver.c                                     │ │
│  │  - LED 实例数组管理                                     │ │
│  │  - 灯效状态机（ON/OFF/BLINK）                           │ │
│  │  - OSAL 事件驱动定时                                    │ │
│  └────────────────────────────────────────────────────────┘ │
└─────────────────────────────┬───────────────────────────────┘
                              │ ind_hal_set(index, level)
┌─────────────────────────────▼───────────────────────────────┐
│                       HAL Layer                              │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  indicator_hal.c                                        │ │
│  │  - GPIO 控制（当前实现）                                 │ │
│  │  - 预留：CH595 / 串口命令                               │ │
│  └────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

---

## 三、模块职责划分

### 3.1 HAL 层 (indicator_hal)

**职责**：封装硬件操作细节

```
输入: LED索引 + 电平状态
输出: 硬件动作（GPIO/CH595/串口）
```

**接口**：
```c
void ind_hal_init(void);                    // 初始化所有LED硬件
void ind_hal_set(uint8_t index, bool on);   // 设置LED状态
void ind_hal_deinit(void);                  // 反初始化
```

**特点**：
- 内部处理 active_high/active_low 极性转换
- 通过配置表获取引脚信息
- 预留硬件扩展点

### 3.2 Driver 层 (indicator_driver)

**职责**：灯效状态机 + 定时管理

```
输入: LED索引 + 灯效配置
输出: 调用 HAL 层控制硬件
```

**接口**：
```c
void indicator_init(void);                                    // 初始化
void indicator_set(uint8_t led_id, const ind_effect_t* eff);  // 设置灯效
void indicator_off(uint8_t led_id);                           // 关闭
```

**特点**：
- 使用 OSAL 事件驱动，无需主循环轮询
- 每个 LED 独立状态机
- 后设置自动覆盖前状态

### 3.3 Application 层

**职责**：业务逻辑 + 状态映射

```
输入: 业务事件（蓝牙连接、电量变化等）
输出: 调用 Driver 层设置灯效
```

**特点**：
- 使用别名访问 LED
- 根据业务状态选择灯效
- 不关心硬件细节

---

## 四、数据结构设计

### 4.1 灯效配置

```c
// 灯效模式
typedef enum {
    IND_MODE_OFF = 0,      // 熄灭
    IND_MODE_ON,           // 常亮
    IND_MODE_BLINK,        // 闪烁
} ind_mode_t;

// 灯效参数
typedef struct {
    ind_mode_t mode;       // 模式
    uint16_t   on_ms;      // 亮持续时间 (ms)
    uint16_t   off_ms;     // 灭持续时间 (ms)
    uint16_t   delay_ms;   // 延迟启动时间 (ms)
    uint16_t   duration_ms;// 总持续时间 (ms), 0=无限
    uint8_t    repeat;     // 重复次数, 0=无限
} ind_effect_t;
```

### 4.2 LED 硬件配置（产品定义）

```c
// LED 硬件定义
typedef struct {
    pin_t   pin;           // GPIO 引脚
    bool    active_high;   // true=高电平亮
} ind_led_def_t;

// 产品配置示例 (keyboards/xxx/indicator_config.h)
#define IND_LED_COUNT  5

static const ind_led_def_t ind_led_table[IND_LED_COUNT] = {
    [0] = {PIN_A0, true },   // LED0
    [1] = {PIN_A1, true },   // LED1
    [2] = {PIN_A2, true },   // LED2
    [3] = {PIN_B0, false},   // LED3 (低电平亮)
    [4] = {PIN_B1, true },   // LED4
};

// 业务别名
#define LED_BT1      0
#define LED_BT2      1
#define LED_BT3      2
#define LED_CAPS     0       // 与 BT1 共用
#define LED_BAT      3
#define LED_CHARGE   4
```

### 4.3 LED 运行时状态（Driver 内部）

```c
// LED 运行时状态
typedef struct {
    ind_effect_t effect;     // 当前灯效配置
    uint8_t      phase;      // 当前阶段: 0=delay, 1=on, 2=off
    uint8_t      count;      // 已闪烁次数
    uint32_t     start_time; // 阶段开始时间
    bool         is_on;      // 当前是否点亮
    bool         active;     // 是否激活
} ind_led_state_t;
```

---

## 五、状态机设计

### 5.1 灯效状态机

```
                    indicator_set()
                         │
                         ▼
              ┌──────────────────┐
              │   检查 delay_ms   │
              └────────┬─────────┘
                       │
           ┌───────────┴───────────┐
           │                       │
     delay > 0                delay = 0
           │                       │
           ▼                       ▼
    ┌─────────────┐        ┌─────────────┐
    │ PHASE_DELAY │───────▶│ PHASE_ON    │
    │ 等待延迟    │  超时   │ LED 点亮    │
    └─────────────┘        └──────┬──────┘
                                  │
                           on_ms 超时
                                  │
                                  ▼
                           ┌─────────────┐
                           │ PHASE_OFF   │
                           │ LED 熄灭    │
                           └──────┬──────┘
                                  │
                           off_ms 超时
                                  │
                    ┌─────────────┴─────────────┐
                    │                           │
              count < repeat             count >= repeat
              或 repeat = 0                     │
                    │                           ▼
                    │                    ┌─────────────┐
                    └───────────────────▶│   IDLE      │
                         循环到 PHASE_ON  │   停止      │
                                         └─────────────┘
```

### 5.2 主循环轮询驱动流程

```
┌─────────────────────────────────────────────────────────────┐
│                     indicator_init()                         │
│  1. 调用 ind_hal_init() 初始化硬件                           │
│  2. 初始化所有 LED 状态为 IDLE                               │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                  indicator_set(led_id, effect)               │
│  1. 保存新灯效配置                                           │
│  2. 记录当前时间戳 timer = timer_read32()                    │
│  3. 设置 next_period 为下一阶段的超时时间                    │
│  4. 根据 delay_ms 设置初始阶段                               │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│              主循环调用 indicator_task()                     │
│  1. 遍历所有 LED                                             │
│  2. 检查 timer_elapsed32(timer) >= next_period               │
│  3. 超时则执行 process_led_timeout()                         │
│     - 根据当前阶段执行操作（点亮/熄灭）                       │
│     - 更新 timer 和 next_period                              │
│     - 如已完成，进入 IDLE 状态                               │
└─────────────────────────────────────────────────────────────┘
```

### 5.3 定时机制对比

| 特性 | OSAL 事件驱动 | 主循环轮询 |
|-----|--------------|-----------|
| 耦合度 | 高（依赖 OSAL） | 低（仅依赖 timer.h） |
| 移植性 | 需要 OSAL 支持 | 任意平台 |
| 精度 | 依赖 OSAL 调度 | 依赖主循环频率 |
| 功耗 | 可休眠等待事件 | 需定期唤醒轮询 |
| 复杂度 | 较高 | 简单直观 |

---

## 六、接口设计

### 6.1 对外接口 (indicator.h)

```c
#pragma once

#include <stdint.h>
#include <stdbool.h>

// 灯效模式
typedef enum {
    IND_MODE_OFF = 0,
    IND_MODE_ON,
    IND_MODE_BLINK,
} ind_mode_t;

// 灯效配置
typedef struct {
    ind_mode_t mode;
    uint16_t   on_ms;
    uint16_t   off_ms;
    uint16_t   delay_ms;
    uint16_t   duration_ms;
    uint8_t    repeat;
} ind_effect_t;

// ============ 核心接口 ============

// 初始化指示灯模块
void indicator_init(void);

// 设置灯效（后者覆盖）
void indicator_set(uint8_t led_id, const ind_effect_t* effect);

// 关闭指定灯
void indicator_off(uint8_t led_id);

// 关闭所有灯
void indicator_off_all(void);

// ============ 预定义灯效 ============

extern const ind_effect_t IND_OFF;           // 熄灭
extern const ind_effect_t IND_ON;            // 常亮（无限）
extern const ind_effect_t IND_ON_1S;         // 亮1秒后灭
extern const ind_effect_t IND_ON_2S;         // 亮2秒后灭
extern const ind_effect_t IND_BLINK_SLOW;    // 慢闪 (500ms)
extern const ind_effect_t IND_BLINK_FAST;    // 快闪 (100ms)
extern const ind_effect_t IND_BLINK_1;       // 闪1次
extern const ind_effect_t IND_BLINK_2;       // 闪2次
extern const ind_effect_t IND_BLINK_3;       // 闪3次
```

### 6.2 HAL 接口 (indicator_hal.h)

```c
#pragma once

#include <stdint.h>
#include <stdbool.h>

// 初始化所有 LED 硬件
void ind_hal_init(void);

// 设置 LED 状态（HAL 内部处理极性）
void ind_hal_set(uint8_t index, bool on);

// 反初始化
void ind_hal_deinit(void);
```

### 6.3 产品配置接口 (indicator_config.h)

```c
#pragma once

#include "gpio.h"

// LED 定义结构
typedef struct {
    pin_t pin;
    bool  active_high;
} ind_led_def_t;

// LED 数量
#define IND_LED_COUNT  5

// LED 硬件配置表
extern const ind_led_def_t ind_led_table[IND_LED_COUNT];

// ============ 业务别名 ============
#define LED_BT1      0
#define LED_BT2      1
#define LED_BT3      2
#define LED_CAPS     0   // 与 BT1 共用
#define LED_BAT      3
#define LED_CHARGE   4
```

---

## 七、使用示例

### 7.1 基础使用

```c
#include "indicator.h"
#include "indicator_config.h"

void app_init(void) {
    indicator_init();
}

// 蓝牙连接成功
void on_bt_connected(uint8_t channel) {
    indicator_set(LED_BT1 + channel, &IND_ON_2S);
}

// 蓝牙配对中
void on_bt_pairing(uint8_t channel) {
    indicator_set(LED_BT1 + channel, &IND_BLINK_SLOW);
}

// 大小写切换（会覆盖蓝牙灯状态）
void on_caps_lock(bool on) {
    indicator_set(LED_CAPS, on ? &IND_ON : &IND_OFF);
}

// 低电量警告
void on_low_battery(void) {
    indicator_set(LED_BAT, &IND_BLINK_FAST);
}
```

### 7.2 自定义灯效

```c
// 自定义：延迟500ms后亮，亮200ms灭300ms，闪烁5次
ind_effect_t custom_effect = {
    .mode        = IND_MODE_BLINK,
    .on_ms       = 200,
    .off_ms      = 300,
    .delay_ms    = 500,
    .duration_ms = 0,
    .repeat      = 5,
};

indicator_set(LED_BT1, &custom_effect);
```

---

## 八、文件结构

```
drivers/output/indicators/
├── indicator.h              # 对外接口（应用层使用）
├── indicator.c              # 驱动层实现
├── indicator_hal.h          # HAL 接口
├── indicator_hal.c          # HAL 实现（GPIO 版本）
└── indicator_hal_ch595.c    # HAL 实现（CH595 版本，预留）

keyboards/xxx/
└── indicator_config.h       # 产品 LED 配置 + 别名
```

---

## 九、扩展性设计

### 9.1 新增灯效模式

只需修改 `indicator.c`：
1. 在 `ind_mode_t` 添加新模式
2. 在状态机处理逻辑中添加对应分支

### 9.2 新增硬件控制方式

只需新增 HAL 实现文件：
1. 创建 `indicator_hal_xxx.c`
2. 实现 `ind_hal_init/set/deinit` 接口
3. 在 CMakeLists.txt 中选择编译

### 9.3 新增 LED

只需修改产品配置：
1. 在 `indicator_config.h` 中增加 `IND_LED_COUNT`
2. 在 `ind_led_table` 中添加引脚配置
3. 添加业务别名

---

## 十、与现有代码对比

| 对比项 | 现有实现 | 重构后 |
|-------|---------|--------|
| 驱动方式 | 主循环轮询 task() | OSAL 事件驱动 |
| LED 实例 | 动态创建，需手动管理 | 静态配置表，自动管理 |
| 灯效配置 | 结构复杂，多个参数 | 精简，统一 ind_effect_t |
| 时间处理 | timer 字段语义混乱 | OSAL 定时器，清晰可靠 |
| HAL 极性 | 上下层都处理，不一致 | 统一在 HAL 层处理 |
| 使用方式 | 需创建实例、调用多个函数 | 一个函数搞定 |
| 模式扩展 | 函数指针表 | 简化状态机，易理解 |

---

## 十一、实现计划

1. **Phase 1**: 实现 HAL 层（GPIO 版本）
2. **Phase 2**: 实现 Driver 层（状态机 + OSAL 事件）
3. **Phase 3**: 创建产品配置模板
4. **Phase 4**: 编写单元测试
5. **Phase 5**: 迁移现有业务代码

---

## 十二、待讨论

1. **duration_ms 与 repeat 冲突时**：哪个优先？建议 repeat 优先，duration 作为最大时间限制。
2. **是否需要亮度控制**：当前设计只支持开/关，如需亮度需引入 PWM。
3. **低功耗考虑**：LED 全灭时是否需要通知系统进入低功耗？
