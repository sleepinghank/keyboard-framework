# 触控板中断标志重构 — 设计文档

> 文档类型：design
> 日期：2026-03-21
> 状态：待实施
> 需求文档：2026-03-21-touchpad-int-flag-refactor-requirements.md

## 1. 设计概述

将触控板 ISR 中断通知从 `_touch_int_flag` 标志位轮询模式切换为 OSAL 事件直接投递模式。改动集中在 `touchpad_service.c`，middleware 层已有的 `touchpad_notify_int()` 无需修改。

### 1.1 设计原则

- 最小改动：仅替换通知机制，不改变触控数据处理逻辑
- 层级合规：component 层 ISR 通过 middleware 公开接口向上通知
- 接口收敛：废弃旧接口，不保留双轨并行

## 2. 方案设计

### 2.1 ISR 切换

**文件：** `component/touch_component/touchpad_service.c`

新增头文件引用：
```c
#include "touchpad/touchpad.h"
```

`_touch_cb()` 改动：
```c
// 改动前
__HIGH_CODE
void _touch_cb(pin_t pin)
{
    (void)pin;
    if (touch_en == 1) {
        _touch_int_flag++;
    }
}

// 改动后
__HIGH_CODE
void _touch_cb(pin_t pin)
{
    (void)pin;
    if (touch_en == 1) {
        touchpad_notify_int();
    }
}
```

**技术确认：**
- `OSAL_SetEvent` 在 CH584 ISR 中是安全的（`tmos_set_event` 为位或操作，幂等且中断安全）
- `__HIGH_CODE`（RAM）调用非 `__HIGH_CODE`（flash）函数是正常操作，无地址冲突
- `touchpad_notify_int()` 内有 `touchpad_taskID == INVALID` 守卫，初始化前调用安全

### 2.2 touch_evt_task 简化

**文件：** `component/touch_component/touchpad_service.c`

```c
// 改动前
void touch_evt_task(void){
    if (touch_en != 1 && start_touch_flag != 1){
        return ;
    }
    if(_touch_int_flag > 0)
    {
        ATOMIC_BLOCK_FORCEON {
            if (_touch_int_flag > 0) {
                _touch_int_flag--;
            }
        }
        _touch_button_repeat_count = 0;
        touch_task();
    }
}

// 改动后
void touch_evt_task(void){
    if (touch_en != 1){
        return;
    }
    touch_task();
}
```

**连带删除：**

| 删除项 | 位置 | 原因 |
|--------|------|------|
| `start_touch_flag` 变量 | :98 | 由 `_touch_int_flag` 驱动，事件模式下不再需要 |
| `set_touch_data_event()` 函数 | :752-754 | 设置 `start_touch_flag`，已无用 |
| `clear_touch_data_event()` 函数 | :748-750 | 清除 `start_touch_flag`，已无用 |
| `_touch_button_repeat_count` 变量 | :96 | 按钮重复逻辑废弃 |
| `_touch_button_repeat_flag` 变量 | :97 | 按钮重复逻辑废弃 |
| `_touch_button_repeat_flag` 赋值 | ProcessGetPTPData :608, :614, :642 | 变量已删除 |
| `#include "atomic_util.h"` | 头部 | ATOMIC_BLOCK 已移除 |

### 2.3 touch_timer_task 废弃

**文件：** `component/touch_component/touchpad_service.c`、`touchpad_service.h`

删除整个 `touch_timer_task()` 函数（:945-998）及其在 `touchpad_service.h` 中的声明。

**连带删除：**

| 删除项 | 位置 | 原因 |
|--------|------|------|
| `int_low_cnt` 变量 | :120 | INT 异常检测已由 watchdog 覆盖 |
| `touch_timer_task` 声明 | touchpad_service.h | 函数已废弃 |

**废弃的逻辑段及其处置：**

| 原逻辑 | 行号 | 处置 |
|--------|------|------|
| `kb_break_cnt--` 递减 | :947-949 | 改为 OSAL 一次性定时事件（见 2.4 节） |
| `_touch_int_flag` 检测 + `start_touch_flag` 设置 | :953-957 | 废弃，由 OSAL 事件替代 |
| INT 引脚异常恢复 | :960-968 | 废弃，由 `touchpad_watchdog_check()` 每 2 秒兜底 |
| 按钮重复发送 | :970-985 | 废弃，PCT1336 持续按下时持续产生中断 |
| `scan_time > 0` 状态返回 | :996 | 废弃，调用者不存在 |

### 2.4 kb_break_cnt 改为 OSAL 一次性定时事件

**涉及 3 层改动：**

#### middleware/touchpad/touchpad.h — 新增定义和接口

```c
#define TOUCHPAD_KB_BREAK_EVT    (1 << 3)  // 键盘打字隔离到期事件

// 设置键盘打字隔离触摸板时间
void touchpad_set_kb_break(uint16_t ms);
```

#### middleware/touchpad/touchpad.c — 新增事件处理和包装函数

`touchpad_process_event()` 新增分支：
```c
if (events & TOUCHPAD_KB_BREAK_EVT) {
    set_kb_break_cnt(0);
    return (events ^ TOUCHPAD_KB_BREAK_EVT);
}
```

新增包装函数：
```c
void touchpad_set_kb_break(uint16_t ms)
{
    if (touchpad_taskID == TOUCHPAD_INVALID_TASK_ID) {
        return;
    }
    set_kb_break_cnt(1);
    OSAL_SetDelayedEvent(touchpad_taskID, TOUCHPAD_KB_BREAK_EVT, ms);
}
```

**注意：** API 为 `OSAL_SetDelayedEvent`（非 `OSAL_StartTask`），覆盖语义——重复调用会重置倒计时。

#### component/touch_component/touchpad_service.c — 接口收敛

`set_kb_break_cnt()` 改为 `static`，不再对外暴露：
```c
static void set_kb_break_cnt(uint16_t cnt)
{
    kb_break_cnt = cnt;
}
```

移除 `touchpad_service.h` 中 `set_kb_break_cnt` 的声明。

**语义变更：**
- 旧：`set_kb_break_cnt(40)` → 每 10ms 递减，约 400ms 后归零
- 新：`touchpad_set_kb_break(400)` → 400ms 后 OSAL 事件到期，一次性清零

### 2.5 清理收尾

#### touch_power_off() 修改

```c
// 改动前
int8_t touch_power_off(void){
    TOUCHPAD_log("Touch power off");
    pct1336_sleep();
    touch_en = 0;
    wait_us(200);
    touch_gpio_uninit();
    if(_touch_int_flag > 0){
        _touch_int_flag = 0;
    }
    return 1;
}

// 改动后
int8_t touch_power_off(void){
    TOUCHPAD_log("Touch power off");
    pct1336_sleep();
    touch_en = 0;
    wait_us(200);
    touch_gpio_uninit();
    touchpad_stop_all_events();
    return 1;
}
```

**保护机制分析：**
- `touch_en = 0`：`touch_evt_task()` 第一行守卫拦截
- `touch_gpio_uninit()`：禁用 GPIO 中断，不再产生新事件
- `touchpad_stop_all_events()`：清除 OSAL 队列中的 pending 事件

#### touchpad_stop_all_events() — middleware 新增

**文件：** `middleware/touchpad/touchpad.c` 和 `touchpad.h`

```c
// touchpad.h
void touchpad_stop_all_events(void);

// touchpad.c
void touchpad_stop_all_events(void)
{
    if (touchpad_taskID == TOUCHPAD_INVALID_TASK_ID) {
        return;
    }
    OSAL_StopTask(touchpad_taskID, TOUCHPAD_DATA_EVT);
    OSAL_StopTask(touchpad_taskID, TOUCHPAD_KB_BREAK_EVT);
}
```

#### end_touchpad_input() 修改

```c
// 改动前
    if(_touch_int_flag > 0){
        _touch_int_flag = 0;
        _touch_button_repeat_count = 0;
        pct1336_clear_int();
    }

// 改动后
    pct1336_clear_int();
```

移除 flag 清零和 repeat_count 清零，`pct1336_clear_int()` 改为无条件执行。

#### 删除 _touch_int_flag 定义和声明

| 文件 | 操作 |
|------|------|
| `touchpad_service.c:72` | 删除 `uint8_t _touch_int_flag = 0;` |
| `touchpad_service.h:99` | 删除 `extern uint8_t _touch_int_flag;` |

## 3. 实施计划

### 3.1 步骤总览

| 步骤 | 任务 | 涉及文件 | 前置 | 验证 |
|------|------|----------|------|------|
| S1 | ISR 切换 | touchpad_service.c | 无 | 编译通过 |
| S2 | touch_evt_task 简化 | touchpad_service.c | S1 | 编译通过 |
| S3 | touch_timer_task 废弃 + kb_break_cnt 改造 | touchpad_service.c/h, touchpad.c/h | S2 | 编译通过 |
| S4 | 清理收尾 + 评审修正 | touchpad_service.c/h, touchpad.c/h | S3 | 编译通过 + grep 零引用 |

### 3.2 详细步骤

**S1 — ISR 切换**
1. `touchpad_service.c` 新增 `#include "touchpad/touchpad.h"`
2. `_touch_cb()` 中 `_touch_int_flag++` 改为 `touchpad_notify_int()`
- 验证：编译通过

**S2 — touch_evt_task 简化**
1. `touch_evt_task()` 简化为 `touch_en` 守卫 + `touch_task()` 调用
2. 删除变量：`start_touch_flag`、`_touch_button_repeat_count`、`_touch_button_repeat_flag`
3. 删除函数：`set_touch_data_event()`、`clear_touch_data_event()`
4. 删除 `ProcessGetPTPData()` 中 `_touch_button_repeat_flag` 的 3 处赋值（:608、:614、:642）
5. 移除 `#include "atomic_util.h"`
- 验证：编译通过

**S3 — touch_timer_task 废弃 + kb_break_cnt 改造**
1. 删除 `touch_timer_task()` 函数和 `touchpad_service.h` 中声明
2. 删除 `int_low_cnt` 变量
3. `touchpad.h` 新增 `TOUCHPAD_KB_BREAK_EVT` 定义和 `touchpad_set_kb_break()` 声明
4. `touchpad.c` 新增 `TOUCHPAD_KB_BREAK_EVT` 事件处理分支和 `touchpad_set_kb_break()` 实现
5. `set_kb_break_cnt()` 改为 `static`，移除 `touchpad_service.h` 中声明
- 验证：编译通过

**S4 — 清理收尾 + 评审修正**
1. 删除 `_touch_int_flag` 定义（:72）和 extern 声明（:99）
2. `touch_power_off()` 移除 flag 清零，新增 `touchpad_stop_all_events()` 调用
3. `touchpad.c` 新增 `touchpad_stop_all_events()` 实现
4. `touchpad.h` 新增 `touchpad_stop_all_events()` 声明
5. `end_touchpad_input()` 移除 flag 清零，`pct1336_clear_int()` 改为无条件执行
- 验证：
  - 编译通过，零新增 warning
  - `grep -r "_touch_int_flag" --include="*.c" --include="*.h"` 零结果（排除 DOCS/）
  - `grep -r "touch_timer_task" --include="*.c" --include="*.h"` 零结果

## 4. 评审修正记录

以下为 Phase 2 多角度评审中发现的问题及修正：

| # | 来源 | 问题 | 修正 |
|---|------|------|------|
| R1 | 技术可行性 | `OSAL_StartTask()` API 不存在 | 改为 `OSAL_SetDelayedEvent()`，覆盖语义 |
| R2 | 功能完整性 | `touch_power_off()` 未补充事件停止 | 新增 `touchpad_stop_all_events()` 调用 |
| R3 | 功能完整性 | `_touch_button_repeat_flag` 在 ProcessGetPTPData 中有残留赋值 | S2 中一并删除 3 处赋值 |
| R4 | 可维护性 | `set_kb_break_cnt` 与 `touchpad_set_kb_break` 命名歧义 | `set_kb_break_cnt` 改为 static 内部函数 |

## 5. 验证策略

### 5.1 编译验证

- 编译通过，零新增 warning
- grep 确认 `_touch_int_flag` 和 `touch_timer_task` 零引用

### 5.2 实机验证清单

| 场景 | 预期结果 | 优先级 |
|------|----------|--------|
| 单指触摸滑动 30 秒 | PTP 报告正常上报，无卡顿 | P0 |
| 多指触摸 | 多点触控正常 | P0 |
| 长按物理按钮 3 秒+ | 按钮状态持续保持，拖拽不断裂 | P0 |
| 触控板关机后 | 无 I2C 误访问、无异常日志 | P0 |
| 键盘打字后立即触摸 | confidence 被抑制（若 kb_break 已接入） | P1 |
| 触控板 Mouse 模式 | 鼠标报告正常 | P1 |
| 30 分钟压力测试 | 无功能退化 | P2 |

### 5.3 回滚方案

改动集中在 4 个文件，git revert 可完整回滚，无跨模块 API 破坏。
