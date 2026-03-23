# combo 引擎计时机制改造 — 需求文档

## 需求摘要

将 combo 组合键引擎中的按键计时机制，从任务调用次数计数（ticks）改为基于
`timer_elapsed32()` 的毫秒时间戳计时，消除对调度频率的强耦合。

## 背景与问题

当前 `button_ticks()` 中 `combo->ticks` 每次 `combo_task()` 调用时 +1，而判断阈值
`long_press_ticks = LONG_TICKS = 2500 / TICKS_INTERVAL = 500`（假设 5ms 间隔）。

若调度频率变化（低功耗、BLE 竞争、任务堆叠），长按触发时机会漂移，用户感知为
"长按行为不稳定"。

## 需求 Q&A

| 问题 | 决定 |
|------|------|
| SHORT_TICKS 是否一起改为毫秒 | 是，SHORT_TICKS / LONG_TICKS 全部改为毫秒 |
| combo_t.ticks 字段是否改名 | 改为 press_time（存绝对时间戳） |
| long_press_ticks / short_ticks 配置字段是否改名 | 改为 long_press_ms / short_ms |

## 变更范围

1. `combo_t.ticks`（uint16_t）→ `press_time`（uint32_t）
2. `combo_t.long_press_ticks`（uint16_t）→ `long_press_ms`（uint16_t，单位毫秒）
3. `combo_t.short_ticks`（如有）→ `short_ms`（uint16_t，单位毫秒）
4. 宏 `SHORT_TICKS` → `SHORT_MS`，`LONG_TICKS` → `LONG_MS`（值改为毫秒）
5. `button_ticks()` 逻辑：
   - 删除首行累加 `combo->ticks++`
   - 按下时：`combo->press_time = timer_read32()`
   - 判断时：`timer_elapsed32(combo->press_time) >= combo->long_press_ms`
   - 状态重置时：`combo->ticks = 0` → `combo->press_time = timer_read32()`
6. `kb_combo_map.c`：初始化宏中的 ticks 数值换算为毫秒

## 已有接口

```c
// drivers/system/timer.h
uint32_t timer_read32(void);
uint32_t timer_elapsed32(uint32_t last);
```

已在 `drivers/output/indicators/indicator.c` 中成功使用，行为经过验证。

## 评审意见汇总

### 用户/产品视角
- 体验提升明确，长按触发稳定性改善
- 风险：休眠唤醒后时间戳跳变（深度睡眠期间 timer 是否继续计时）
- 建议：唤醒时清空 combo 状态，避免误触发

### 架构/编码视角
- `ticks` 必须从 uint16_t 升为 uint32_t（存绝对时间戳，防截断）
- `long_press_ms` / `short_ms` 可保留 uint16_t（最大 65535ms 足够）
- `button_ticks()` 首行累加逻辑需删除
- `kb_combo_map.c` 初始化宏的数值需从 ticks 换算为毫秒
- 架构方向符合（middleware 依赖 drivers，无跨层问题）

### 测试/质量视角
- Windows 仿真层 `timer_read32()` 实现需与硬件行为一致
- 建议 mock `timer_read32()` 补充单元测试
- 可接受误差：±20ms（受调度间隔影响）
- 本次暂不要求补充自动化测试，人工回归验证即可

## 范围边界

**本次包含：**
- `kb_combo_engine.h/c` 内的结构体、逻辑、宏
- `kb_combo_map.c` 的数值换算
- 涉及 combo_t 字段的所有引用

**本次不包含：**
- 补充自动化单元测试
- 修改休眠唤醒后的 combo 状态清理（另立任务）

## 代码现状分析

### 需修改的完整清单

| 文件 | 行号 | 改动 |
|------|------|------|
| `kb_combo_engine.h` | 14-17 | 删除 `TICKS_INTERVAL/DEBOUNCE_TICKS/SHORT_TICKS/LONG_TICKS`，改为 `SHORT_MS`/`LONG_MS` 毫秒常量 |
| `kb_combo_engine.h` | 48 | `combo_t.ticks` uint16_t → `press_time` uint32_t（存时间戳，必须 uint32_t） |
| `kb_combo_engine.h` | 57 | `combo_t.long_press_ticks` → `long_press_ms`（uint16_t 可保留，单位改为毫秒） |
| `kb_combo_engine.c` | 87 | 删除 `combo->ticks++`，改为 `uint32_t elapsed = timer_elapsed32(combo->press_time)` |
| `kb_combo_engine.c` | 99, 110, 124, 140 | `combo->ticks = 0` → `combo->press_time = timer_read32()` |
| `kb_combo_engine.c` | 112 | `combo->ticks > combo->long_press_ticks` → `elapsed > combo->long_press_ms` |
| `kb_combo_engine.c` | 126, 139, 145 | `combo->ticks > SHORT_TICKS` → `elapsed > SHORT_MS` |
| `kb_combo_map.c` | 65-76 | `COMBO_LONG_TICKS(xx, 5, ...)` → `COMBO_LONG_TICKS(xx, 25, ...)`（5×5ms=25ms） |
| `kb_combo_map.c` | 85-86 | `COMBO_LONG_TICKS(xx, 3000, ...)` — 保持（已是毫秒值） |

### timer 接口（已可用）

```c
// drivers/system/timer.h
uint32_t timer_read32(void);             // 当前毫秒时间戳
uint32_t timer_elapsed32(uint32_t last); // 距 last 的毫秒差值，自动处理溢出
```

`timer_elapsed32` 内部使用 `TIMER_DIFF_32` 宏处理 uint32_t 溢出（约 49.7 天周期）。
