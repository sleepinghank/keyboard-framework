# 硬件定时器简化实施计划

## 概述

简化 `_hw_timer.c` 和 `hw_timer.h`，移除软件长定时、单次模式、暂停/恢复功能，只保留最简洁的周期定时器功能。长时间定时器由 OSAL 软件调度实现。

## 修改范围

### 文件列表

| 文件 | 操作 |
|------|------|
| `hal/hw_timer.h` | 修改接口定义 |
| `hal/platforms/ch584/_hw_timer.c` | 修改实现 |

## 实施步骤

### Step 1: 修改 hw_timer.h 接口定义

**修改内容:**

1. 移除 `hw_timer_mode_t` 枚举
2. 移除 `HW_TIMER_STATE_PAUSED` 状态
3. 移除 `hw_timer_start_oneshot()` 声明
4. 移除 `hw_timer_pause()` 声明
5. 移除 `hw_timer_resume()` 声明
6. 移除 `hw_timer_get_remaining()` 声明
7. 添加 `HW_TIMER_MAX_MS` 常量说明硬件限制

**简化后的接口:**

```c
// 类型定义
typedef enum {
    HW_TIMER_0 = 0,
    HW_TIMER_1 = 1,
    HW_TIMER_2 = 2,
    HW_TIMER_MAX
} hw_timer_id_t;

typedef enum {
    HW_TIMER_STATE_IDLE    = 0,
    HW_TIMER_STATE_RUNNING = 1
} hw_timer_state_t;

typedef void (*hw_timer_callback_t)(hw_timer_id_t timer_id);

// 初始化
error_code_t hw_timer_init(void);
error_code_t hw_timer_deinit(void);

// 核心操作
error_code_t hw_timer_start(hw_timer_id_t id, uint32_t interval_ms, hw_timer_callback_t cb);
error_code_t hw_timer_stop(hw_timer_id_t id);

// 动态配置
error_code_t hw_timer_set_interval(hw_timer_id_t id, uint32_t interval_ms);
error_code_t hw_timer_set_callback(hw_timer_id_t id, hw_timer_callback_t cb);

// 状态查询
hw_timer_state_t hw_timer_get_state(hw_timer_id_t id);
bool hw_timer_is_running(hw_timer_id_t id);
```

### Step 2: 简化 _hw_timer.c 上下文结构

**修改内容:**

1. 简化 `hw_timer_context_t` 结构体，移除:
   - `mode` 字段
   - `sw_counter` 字段
   - `sw_target` 字段
   - `remaining_ms` 字段

**简化后的结构:**

```c
typedef struct {
    hw_timer_state_t    state;          // IDLE / RUNNING
    uint32_t            interval_ms;    // 定时间隔 (毫秒)
    uint32_t            hw_ticks;       // 硬件定时周期 (ticks)
    hw_timer_callback_t callback;       // 回调函数
} hw_timer_context_t;
```

### Step 3: 移除辅助函数

**移除:**

- `calculate_timer_params()` 函数

**修改:**

- `timer_hw_start()` 内部直接计算 ticks

### Step 4: 简化中断处理函数

**修改 `timer_irq_handler()`:**

- 移除软件计数逻辑
- 移除单次模式处理
- 直接调用回调

**简化后:**

```c
__HIGH_CODE
static void timer_irq_handler(hw_timer_id_t id)
{
    hw_timer_context_t *ctx = &g_timer_ctx[id];

    if (ctx->state == HW_TIMER_STATE_RUNNING && ctx->callback) {
        ctx->callback(id);
    }
}
```

### Step 5: 简化公共接口实现

**修改 `hw_timer_init()`:**
- 简化上下文初始化

**修改 `hw_timer_start()`:**
- 移除 `calculate_timer_params()` 调用
- 直接计算 `hw_ticks = MS_TO_TICKS(interval_ms)`
- 添加最大值检查 (interval_ms <= MAX_DIRECT_MS)
- 移除 mode 设置

**修改 `hw_timer_set_interval()`:**
- 直接计算 ticks，移除 `calculate_timer_params()` 调用

**移除函数:**
- `hw_timer_start_oneshot()`
- `hw_timer_pause()`
- `hw_timer_resume()`
- `hw_timer_get_remaining()`

### Step 6: 清理调试代码

**移除:**
- `togglePin(B14)` 调试代码
- 多余的 `dprint()` / `dprintf()` 调用
- `#include "gpio.h"` (如果只用于调试)

## 硬件限制说明

| 参数 | 值 |
|------|-----|
| 系统时钟 | 78 MHz |
| 计数器位宽 | 26 位 |
| 最大 ticks | 67,108,864 |
| 最大定时 | ~860 ms |

超过 860ms 的定时需求应使用 OSAL 软件定时器实现。
