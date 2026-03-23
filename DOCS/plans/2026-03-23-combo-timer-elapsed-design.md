# combo 引擎计时机制改造 — 设计文档

> 需求文档：`DOCS/plans/2026-03-23-combo-timer-elapsed-requirements.md`

## 需求摘要

将 combo 组合键引擎中按键计时机制从调用次数计数（ticks）改为基于 `timer_elapsed32()`
的毫秒时间戳，消除对调度频率的强耦合，使长按/双击判定时间独立于任务调度间隔。

## 现状分析

- `combo_t.ticks`（uint16_t）：每次 `combo_task()` 调用 +1，不是毫秒
- `LONG_TICKS = 2500 / TICKS_INTERVAL = 500`（依赖 TICKS_INTERVAL=5ms 假设）
- 调度频率变化时长按触发时机漂移
- `timer_read32()` / `timer_elapsed32()` 已在 `drivers/system/timer.h` 可用

## 方案设计

### 核心流程

```
每次 combo_task() 调用
     │
     ▼
button_ticks()
     │
     ├─ state==0: elapsed=0（未激活，不参与比较）
     │
     └─ state>0 : elapsed = timer_elapsed32(combo->press_time)
                       │
                       ├─ state1: elapsed > long_press_ms → LONG_PRESS_START
                       ├─ state2: elapsed > SHORT_MS     → SINGLE/DOUBLE_CLICK
                       └─ state3: elapsed > SHORT_MS     → 状态转移判断

状态切换时（按下/释放）：
     combo->press_time = timer_read32()   // 记录切换时刻
```

### 结构体变更

```c
// kb_combo_engine.h — combo_t

// 改前
uint16_t ticks;            // 相对计数（调用次数）
uint16_t long_press_ticks; // 长按阈值（调用次数）

// 改后
uint32_t press_time;    /* 按下时刻绝对时间戳，单位 ms（timer_read32() 返回值） */
uint16_t long_press_ms; /* 长按阈值，单位 ms */
```

### 宏常量变更

```c
// kb_combo_engine.h — 改前
#define TICKS_INTERVAL  5
#define DEBOUNCE_TICKS  (20  / TICKS_INTERVAL)
#define SHORT_TICKS     (300 / TICKS_INTERVAL)
#define LONG_TICKS      (2500/ TICKS_INTERVAL)

// 改后（删除 TICKS_INTERVAL/DEBOUNCE_TICKS，直接使用毫秒）
#define SHORT_MS   300   /* ms */
#define LONG_MS   2500   /* ms */
```

### button_ticks() 逻辑变更

```c
// 改前首行
if ((combo->state) > 0) combo->ticks++;

// 改后首行
uint32_t elapsed = (combo->state > 0) ? timer_elapsed32(combo->press_time) : 0;
```

状态重置：

```c
// 改前
combo->ticks = 0;

// 改后
combo->press_time = timer_read32();
```

阈值比较：

```c
// 改前
combo->ticks > combo->long_press_ticks
combo->ticks > SHORT_TICKS
combo->ticks < SHORT_TICKS

// 改后
elapsed > combo->long_press_ms
elapsed > SHORT_MS
elapsed < SHORT_MS
```

### kb_combo_map.c 数值换算

```c
// 改前（5 = 5个tick，每tick 5ms = 25ms）
COMBO_LONG_TICKS(FN_F1_combo,  5,  ...)

// 改后（直接毫秒，宏同步改名）
COMBO_LONG_MS(FN_F1_combo, 25, ...)

// 3000 的条目：保持原值（原本已是毫秒）
COMBO_LONG_MS(xxx, 3000, ...)
```

### 宏 COMBO_LONG_TICKS 改名

```c
// 宏定义中 COMBO_LONG_TICKS → COMBO_LONG_MS（消除名称与单位不一致）
#define COMBO_LONG_MS(name, long_ms, ...) \
    combo_t name = { .long_press_ms = (uint16_t)(long_ms), ... }
```

## 边界情况处理

| 场景 | 处理方式 |
|------|---------|
| state==0（未激活） | `elapsed=0`，不参与比较，安全 |
| timer 溢出（约 49.7 天） | `TIMER_DIFF_32` 宏无符号减法自动处理 |
| 初始化 | state 显式置 0，press_time 不参与读取 |
| 休眠唤醒后时间戳跳变 | 本次不处理，另立任务；风险为误触发长按一次 |
| 长时间未调用恢复 | elapsed 超阈值 → 强制状态超时，符合预期 |

## 实施计划

### 步骤 1 — 修改 `kb_combo_engine.h`

**文件：** `middleware/keyboard/combo/kb_combo_engine.h`

1. 删除 `TICKS_INTERVAL`、`SHORT_TICKS`、`LONG_TICKS` 宏
2. 搜索 `DEBOUNCE_TICKS` 在整个 combo 目录是否有实际使用（`grep -r "DEBOUNCE_TICKS" middleware/keyboard/combo/`），若无引用则一并删除，若有则保留并评估是否需要替代逻辑
3. 新增 `SHORT_MS 300` 和 `LONG_MS 2500` 宏
3. `combo_t.ticks`（uint16_t）→ `press_time`（uint32_t），加注释
4. `combo_t.long_press_ticks`（uint16_t）→ `long_press_ms`（uint16_t），加注释
5. `COMBO_LONG_TICKS` 宏 → `COMBO_LONG_MS`，参数语义改为毫秒

**验证：** 头文件编译无报错，无遗留旧宏引用。

### 步骤 2 — 修改 `kb_combo_engine.c`

**文件：** `middleware/keyboard/combo/kb_combo_engine.c`

1. 文件顶部检查是否已引入 `#include "timer.h"`，若无则添加（按项目现有 include 习惯，不加完整路径前缀）
2. `button_ticks()` 首行：`if (...) combo->ticks++` → `uint32_t elapsed = (combo->state > 0) ? timer_elapsed32(combo->press_time) : 0`
3. 所有 `combo->ticks = 0` → `combo->press_time = timer_read32()`
4. 所有 `combo->ticks > combo->long_press_ticks` → `elapsed > combo->long_press_ms`
5. 所有 `combo->ticks > SHORT_TICKS` → `elapsed > SHORT_MS`
6. 所有 `combo->ticks < SHORT_TICKS` → `elapsed < SHORT_MS`

**验证：** 编译无报错，无遗留 `combo->ticks` 引用。

### 步骤 3 — 修改 `kb_combo_map.c`

**文件：** `middleware/keyboard/combo/kb_combo_map.c`

1. 宏调用 `COMBO_LONG_TICKS` → `COMBO_LONG_MS`（全局替换）
2. 数值 `5` → `25`（F1-F12 长按，5×5ms=25ms）
3. 数值 `3000` 保持不变（已是毫秒）

**验证：** 编译无报错，宏参数无遗留旧 ticks 值。

### 步骤 4 — 全局检查

执行以下命令，期望输出均为空（零命中）：

```bash
grep -r "long_press_ticks\|short_ticks\|SHORT_TICKS\|LONG_TICKS\|TICKS_INTERVAL\|COMBO_LONG_TICKS" middleware/keyboard/combo/
grep -r "combo->ticks" middleware/keyboard/combo/
```

**验证：** 编译整个项目无错误：
```bash
cmake -S . -B build -G "MinGW Makefiles" && cmake --build build -j4
```

### 步骤 5 — 人工验证

**基线（改造前行为）：**
- 长按触发：约 500 次任务调用后（5ms×500=2500ms，但调度抖动导致实际偏差可达 ±50ms）
- 双击窗口：约 60 次任务调用（5ms×60=300ms）

**改造后验证：**
1. 启动 Windows 仿真 (`./build/keyboard-framework.exe`)
2. 长按 Fn 键，观察 `LONG_PRESS_START` 日志时间戳，应接近 2500ms（偏差 ≤ 一个调度间隔，约 ±5ms）
3. 快速双击，观察 `DOUBLE_CLICK` 事件，300ms 内双击应触发，超过 300ms 应为两次 SINGLE_CLICK
4. F1-F12 Fn 组合键：长按 25ms 触发（极短，人工难以区分，主要确认无崩溃）

## 评审意见汇总

| 维度 | 结论 |
|------|------|
| 功能完整性 | 覆盖全部状态；`COMBO_LONG_TICKS` 宏已纳入改名 |
| 技术可行性 | 可行，uint32_t 无截断，state==0 路径安全 |
| 可维护性 | 可读性提升，字段加注释说明单位 |
| 可测试性 | 人工验证为主，单元测试暂缓 |
| 风险 | 中等；初始化 state=0 确保安全，休眠另立任务 |
