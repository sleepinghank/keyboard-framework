# 硬件定时器 HAL 实现计划

## 概述

封装通用硬件定时器 HAL 接口，支持 CH584、PAR2860 等平台，实现上下层解耦。

## 接口设计

### 文件结构

```
hal/
├── hw_timer.h                      # 通用接口定义 (已完成)
└── platforms/
    ├── ch584/
    │   └── _hw_timer.c             # CH584 平台实现
    ├── par2860/
    │   └── _hw_timer.c             # PAR2860 平台实现
    └── test/
        └── _hw_timer.c             # 测试平台实现
```

### 核心接口

| 接口 | 说明 |
|------|------|
| `hw_timer_init()` | 初始化定时器模块 |
| `hw_timer_start(id, interval_ms, callback)` | 启动周期定时器 |
| `hw_timer_start_oneshot(id, delay_ms, callback)` | 启动单次定时器 |
| `hw_timer_stop(id)` | 停止定时器 |
| `hw_timer_pause(id)` | 暂停定时器 |
| `hw_timer_resume(id)` | 恢复定时器 |
| `hw_timer_set_interval(id, interval_ms)` | 修改定时间隔 |

### 设计要点

- 时间单位：毫秒 (ms)
- 回调函数参数：仅定时器 ID
- 定时器数量：3 个 (HW_TIMER_0/1/2)
- 默认模式：周期模式 (PERIODIC)
- 直接参数传递，无需配置结构体

## 实现计划

### 阶段 1: CH584 平台实现

#### 1.1 定时器映射

| HAL 定时器 | CH584 定时器 | 说明 |
|------------|--------------|------|
| HW_TIMER_0 | TMR0 | 通用定时器 |
| HW_TIMER_1 | TMR1 | 通用定时器 |
| HW_TIMER_2 | TMR2 | 通用定时器 |

#### 1.2 实现步骤

1. 创建 `hal/platforms/ch584/_hw_timer.c`
2. 实现定时器状态管理结构
3. 实现毫秒到时钟周期转换
4. 实现中断处理函数
5. 实现各接口函数

#### 1.3 关键代码逻辑

```c
// 毫秒转时钟周期 (CH584 系统时钟 78MHz)
#define MS_TO_TICKS(ms)  ((uint32_t)(ms) * (FREQ_SYS / 1000))

// 定时器状态结构
typedef struct {
    hw_timer_state_t    state;
    hw_timer_mode_t     mode;
    uint32_t            interval_ms;
    hw_timer_callback_t callback;
    uint8_t             priority;
} hw_timer_context_t;

static hw_timer_context_t g_timer_ctx[HW_TIMER_COUNT];
```

#### 1.4 中断处理

```c
void TMR0_IRQHandler(void) {
    if (TMR0_GetITFlag(TMR0_3_IT_CYC_END)) {
        TMR0_ClearITFlag(TMR0_3_IT_CYC_END);
        if (g_timer_ctx[HW_TIMER_0].callback) {
            g_timer_ctx[HW_TIMER_0].callback(HW_TIMER_0);
        }
        // 单次模式自动停止
        if (g_timer_ctx[HW_TIMER_0].mode == HW_TIMER_MODE_ONESHOT) {
            TMR0_Disable();
            g_timer_ctx[HW_TIMER_0].state = HW_TIMER_STATE_IDLE;
        }
    }
}
```

### 阶段 2: PAR2860 平台实现

#### 2.1 定时器映射

| HAL 定时器 | PAR2860 定时器 | 说明 |
|------------|----------------|------|
| HW_TIMER_0 | Timer 0 | 32.768KHz |
| HW_TIMER_1 | Timer 1 | 32.768KHz |
| HW_TIMER_2 | Timer 2 | 32.768KHz |

#### 2.2 关键差异

- PAR2860 时钟频率 32.768KHz，最小分辨率约 30.5us
- 使用 `timer_enable(id, callback, interval, int_enable)` 原生接口
- 毫秒转换：`interval = ms * 32768 / 1000`

### 阶段 3: 测试平台实现

创建模拟实现用于单元测试。

## 使用示例

### 示例 1: 周期定时器 (按键扫描)

```c
#define KEY_SCAN_INTERVAL_MS  10

void key_scan_callback(hw_timer_id_t id) {
    key_scan_flag = 1;
}

void key_scan_init(void) {
    hw_timer_init();
    hw_timer_start_periodic(HW_TIMER_0, KEY_SCAN_INTERVAL_MS, key_scan_callback);
}
```

### 示例 2: 单次定时器 (超时检测)

```c
void timeout_callback(hw_timer_id_t id) {
    timeout_flag = 1;
}

void start_timeout(uint32_t timeout_ms) {
    hw_timer_start_oneshot(HW_TIMER_1, timeout_ms, timeout_callback);
}
```

### 示例 3: 完整配置

```c
void timer_example(void) {
    hw_timer_config_t config = {
        .interval_ms = 100,
        .mode = HW_TIMER_MODE_PERIODIC,
        .callback = my_callback,
        .priority = 128
    };

    hw_timer_init();
    hw_timer_config(HW_TIMER_2, &config);
    hw_timer_start(HW_TIMER_2);

    // 动态修改间隔
    hw_timer_set_interval(HW_TIMER_2, 200);

    // 暂停/恢复
    hw_timer_pause(HW_TIMER_2);
    hw_timer_resume(HW_TIMER_2);

    // 停止
    hw_timer_stop(HW_TIMER_2);
}
```

## 任务清单

- [x] 设计通用接口 (`hal/hw_timer.h`)
- [x] CH584 平台实现 (`hal/platforms/ch584/_hw_timer.c`)
- [ ] PAR2860 平台实现 (`hal/platforms/par2860/_hw_timer.c`)
- [ ] 测试平台实现 (`hal/platforms/test/_hw_timer.c`)
- [ ] 集成到 system_init 初始化流程
- [ ] 编写单元测试
- [ ] 更新 CMakeLists.txt

## 注意事项

1. **中断安全**: 回调函数在中断上下文执行，应尽量简短
2. **时钟精度**: 不同平台时钟精度不同，毫秒级精度可能有偏差
3. **资源冲突**: 避免与 TMOS/OSAL 软件定时器冲突
4. **低功耗**: 需考虑睡眠模式下定时器行为
