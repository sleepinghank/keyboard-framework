# indicator OSAL 事件驱动改造 — 设计文档

**日期：** 2026-04-04
**状态：** 待实施
**需求文档：** `2026-04-04-indicator-osal-refactor-requirements.md`

---

## 1. 需求摘要

将 `indicator_task()` 从 `while` 循环轮询改造为 OSAL 纯延迟事件驱动，消除 BLE 发送阻塞。

- 调用方接口完全不变
- 每个 LED 独立事件位，精确按各状态实际时长触发
- 重复触发语义：重置（StopTask + 重新 SetDelayedEvent）
- LED 数量：3（`IND_LED_COUNT=3`）

---

## 2. 核心流程图

```
indicator_start(led, effect)
        │
        ▼
  start_led_effect()
        ├─ OSAL_StopTask(ind_task_id, LED_N_EVT)    // 重置已挂起事件
        ├─ 更新 state->effect, phase=DELAY/ON, count=0
        └─ OSAL_SetDelayedEvent(ind_task_id, LED_N_EVT, delay_ms)
           （delay=0 时：直接调 process_led_timeout(i)，后续帧走 OSAL）

OSAL 调度（BLE 栈空闲窗口）
        │
        ▼
indicator_process_event(task_id, events)
        ├─ LED_0_EVT → 校验 phase!=IDLE → process_led_timeout(0)
        ├─ LED_1_EVT → 校验 phase!=IDLE → process_led_timeout(1)
        └─ LED_2_EVT → 校验 phase!=IDLE → process_led_timeout(2)
        
process_led_timeout(i)
        ├─ PHASE_DELAY → ON/BLINK: ind_hal_set(true)
        │   OSAL_SetDelayedEvent(ind_task_id, LED_N_EVT, state->effect.on_ms)
        ├─ PHASE_ON → OFF/IDLE:
        │   ind_hal_set(false)
        │   BLINK: OSAL_SetDelayedEvent(..., state->effect.off_ms)
        │   ON模式/repeat到期: phase=IDLE，不设事件（链条终止）
        └─ PHASE_OFF → ON: count++
            ind_hal_set(true)
            OSAL_SetDelayedEvent(..., state->effect.on_ms)

indicator_stop(led)
        ├─ OSAL_StopTask(ind_task_id, LED_N_EVT)
        ├─ state.phase = IDLE
        └─ ind_hal_set(led, false)
```

---

## 3. 架构设计

### 3.1 模块结构

```
indicator.c（改造后）
├── 新增变量
│   └── static uint8_t ind_task_id = 0xFF
│
├── 新增事件位宏（文件顶部）
│   ├── #define IND_EVT_LED_0  0x0001
│   ├── #define IND_EVT_LED_1  0x0002
│   └── #define IND_EVT_LED_2  0x0004
│
├── indicator_init() [修改]
│   └── ind_task_id = OSAL_ProcessEventRegister(indicator_process_event)
│       // 必须在 BLE 栈初始化之后调用（当前在 _init 阶段，顺序正确）
│
├── indicator_process_event() [新增]
│   └── 各事件位分发 → 校验 phase != IDLE → process_led_timeout(i)
│       return events ^ handled_bits
│
├── start_led_effect() [修改]
│   ├── OSAL_StopTask(ind_task_id, led_evt)         // 取消旧事件
│   ├── 更新状态机字段（phase, count, effect）
│   ├── 若 delay > 0: OSAL_SetDelayedEvent(ind_task_id, led_evt, delay_ms)
│   └── 若 delay = 0: 直接调 process_led_timeout(i)（同步首帧）
│
├── process_led_timeout() [修改]
│   └── 末尾状态切换后改为 OSAL_SetDelayedEvent(ind_task_id, led_evt, next_ms)
│       或 phase=IDLE 时不设事件（链条终止）
│
├── indicator_stop() [修改]
│   ├── OSAL_StopTask(ind_task_id, led_evt)
│   ├── state.phase = IDLE
│   └── ind_hal_set(false)
│
├── indicator_stop_all() [修改]
│   ├── 循环对每个 LED 调用 OSAL_StopTask          // 支持睡眠前清理
│   └── 原有逻辑保留
│
└── indicator_task() [修改]
    └── 函数体清空，保留空函数体（保持对外接口声明兼容）

system_init.c
└── 第 184 行：删除 indicator_task() 调用
```

### 3.2 层级边界

indicator.c 直接使用 `OSAL_*` API（经 `event_manager.h` 抽象），不跨层调用 service 层。与现有 output_service、input_service 的 task 注册模式完全一致。

---

## 4. 接口设计

### 4.1 对外接口（完全不变）

```c
// indicator.h — 签名不变，调用方无感知
void indicator_init(void);
void indicator_start(ind_led_t led, ind_effect_t effect);
void indicator_stop(ind_led_t led);
void indicator_stop_all(void);
void indicator_task(void);  // 保留声明，实现改为空函数
```

### 4.2 新增内部接口

```c
// indicator.c 内部（static，不对外暴露）
static uint16_t indicator_process_event(uint8_t task_id, uint16_t events);
static uint16_t led_to_event_bit(uint8_t led_index);  // index → 事件位
```

### 4.3 ind_led_state_t 结构体（调整）

删除：
- `uint32_t timer`（OSAL 接管计时，不再需要）
- `uint16_t next_period`（OSAL 接管计时，不再需要）

保留：
- `ind_effect_t effect`
- `ind_phase_t phase`
- `uint8_t count`
- `bool is_on`

---

## 5. 关键设计决策

### 5.1 delay=0 处理

- delay=0 时**同步执行首帧**（直接调用 `process_led_timeout(i)`），不产生 OSAL 事件
- 后续帧（ON/OFF 切换）正常走 OSAL 延迟事件
- 原因：避免 1ms 强制延迟在高频场景下累积误差

### 5.2 幽灵触发防御

- `indicator_process_event()` 入口：对每个 LED 事件位，先检查 `state.phase != PHASE_IDLE`，再调用 `process_led_timeout(i)`
- 若 phase 已为 IDLE（说明效果已被 stop），直接清除事件位，不执行状态机
- 防止 `StopTask` 与正在 pending 的事件之间的竞态

### 5.3 睡眠前清理

- `indicator_stop_all()` 内增加对所有 3 个 LED 事件位的 `OSAL_StopTask` 调用
- LPM 进入 Deep Sleep 前调用 `indicator_stop_all()`（调用方已有此逻辑）

### 5.4 事件位选择

- 使用 0x0001、0x0002、0x0004（LED 0/1/2）
- 安全：远离 TMOS 系统保留位 `SYS_EVENT_MSG = 0x8000`
- indicator 拥有独立 task_id，事件位空间不与其他模块共享

### 5.5 OSAL_StopTask 语义确认

`OSAL_StopTask(taskID, event)` 仅清除指定 task 的指定事件位，不影响其他 LED 的事件位。3 个 LED 各自独立，stop 一个不影响其他两个。

---

## 6. 数据流示例（BLINK 模式）

```
indicator_start(LED_1, {mode=BLINK, delay=state->effect.delay_ms,
                        on=state->effect.on_ms, off=state->effect.off_ms, repeat=3})
  OSAL_StopTask(ind_task_id, IND_EVT_LED_1)
  state.phase = PHASE_DELAY, state.count = 0
  OSAL_SetDelayedEvent(ind_task_id, IND_EVT_LED_1, effect.delay_ms)

[delay_ms 后] → indicator_process_event
  process_led_timeout(1): DELAY → ON
  ind_hal_set(true)
  OSAL_SetDelayedEvent(..., IND_EVT_LED_1, effect.on_ms)

[on_ms 后] → indicator_process_event
  process_led_timeout(1): ON → OFF
  ind_hal_set(false)
  OSAL_SetDelayedEvent(..., IND_EVT_LED_1, effect.off_ms)

[off_ms 后] → indicator_process_event
  process_led_timeout(1): OFF → ON, count++=1
  ind_hal_set(true)
  OSAL_SetDelayedEvent(..., IND_EVT_LED_1, effect.on_ms)

[count == repeat 时]
  process_led_timeout(1): ON → IDLE
  ind_hal_set(false)
  // 不设置事件，链条自然终止
```

---

## 7. 实施计划

### 步骤 1 — indicator.c：注册 OSAL task 和事件位

**文件：** `drivers/output/indicators/indicator.c`

- 文件顶部新增：
  ```c
  static uint8_t ind_task_id = 0xFF;
  #define IND_EVT_LED_0  0x0001
  #define IND_EVT_LED_1  0x0002
  #define IND_EVT_LED_2  0x0004
  static const uint16_t led_evt_map[IND_LED_COUNT] = {
      IND_EVT_LED_0, IND_EVT_LED_1, IND_EVT_LED_2
  };
  ```
- `indicator_init()` 末尾新增：
  ```c
  ind_task_id = OSAL_ProcessEventRegister(indicator_process_event);
  ```
- 新增 `led_to_event_bit(i)` → `return led_evt_map[i]`

**验证：** 编译通过，运行时 `ind_task_id != 0xFF`（日志确认）

---

### 步骤 2 — indicator.c：新增事件处理回调

**文件：** `drivers/output/indicators/indicator.c`

新增函数 `indicator_process_event()`：
```
对每个 LED_N_EVT:
  若 events & LED_N_EVT:
    若 ind_states[n].phase != PHASE_IDLE:
      process_led_timeout(n)
    events ^= LED_N_EVT
return events
```

**验证：** 函数签名符合 `osal_task_event_handler_t`，编译通过

---

### 步骤 3 — indicator.c：删除 ind_led_state_t 中的 timer 和 next_period

**文件：** `drivers/output/indicators/indicator.c`（结构体定义处）

- 删除 `uint32_t timer`
- 删除 `uint16_t next_period`
- 删除所有对 `state->timer` 和 `state->next_period` 的赋值/读取

**验证：** 编译无警告

---

### 步骤 4 — indicator.c：改造 start_led_effect()

**文件：** `drivers/output/indicators/indicator.c`

改造要点：
- 函数开头调用 `OSAL_StopTask(ind_task_id, led_evt_map[i])` 取消旧事件
- delay > 0：`OSAL_SetDelayedEvent(ind_task_id, led_evt_map[i], effect.delay_ms)`
- delay = 0：直接调用 `process_led_timeout(i)`（同步首帧）

**验证：** indicator_start 后 OSAL 事件被正确调度（日志确认）

---

### 步骤 5 — indicator.c：改造 process_led_timeout()

**文件：** `drivers/output/indicators/indicator.c`

- 删除所有 `state->timer = timer_read32()` 赋值
- 删除所有 `state->next_period = ...` 赋值
- 每次状态切换后（原 timer 重置处）改为：
  - 若 phase != IDLE：`OSAL_SetDelayedEvent(ind_task_id, led_evt_map[i], next_ms)`
  - 若 phase == IDLE：不调用（链条终止）

**验证：** BLINK 效果运行，LED 正确按 on_ms/off_ms 交替，repeat 到期后停止

---

### 步骤 6 — indicator.c：修改 indicator_stop/stop_all 和清空 indicator_task

**文件：** `drivers/output/indicators/indicator.c`

- `indicator_stop(led)`：增加 `OSAL_StopTask(ind_task_id, led_evt_map[led])` 调用
- `indicator_stop_all()`：循环对所有 LED 调用 `OSAL_StopTask`
- `indicator_task()`：函数体清空（保留空函数定义）

**验证：** indicator_stop 立即熄灯且无后续事件触发

---

### 步骤 7 — system_init.c：移除 indicator_task() 调用

**文件：** `application/system/system_init.c`

- 删除第 184 行的 `indicator_task()` 调用

**验证：** 编译通过；运行时 BLE 连接态下 LED 效果正常，蓝牙发送不再阻塞

---

## 8. 测试清单

| 测试场景 | 预期结果 | 验证方法 |
|---------|---------|---------|
| 单 LED ON 模式 | 亮灯，duration 到期后熄灭 | 观察 + 日志时间戳 |
| 单 LED BLINK 模式 | 按 on_ms/off_ms 闪烁，repeat 次后停止 | 观察 + 计数 |
| delay > 0 效果 | delay 后才亮 | 日志时间戳对比 |
| delay = 0 效果 | 立即亮，无额外延迟 | 观察 |
| 重复触发 | 旧效果立即中止，新效果从头开始 | 观察状态切换 |
| indicator_stop | 立即熄灭，无后续事件 | 观察 |
| 3 灯并发 | 各自独立运行不干扰 | 同时启动三灯观察 |
| BLE 连接态 | 蓝牙发送不阻塞 | 实测发送速率/延迟 |

---

## 8. 代码定位参考

### ind_led_state_t 实际定义（indicator.c 第 42-49 行）

```c
typedef struct {
    ind_effect_t effect;        // 当前灯效配置
    ind_phase_t  phase;         // 当前阶段
    uint8_t      count;         // 已闪烁次数
    bool         is_on;         // 当前是否点亮
    uint32_t     timer;         // 阶段开始时间戳       ← 步骤3删除
    uint16_t     next_period;   // 下一次切换的间隔时间  ← 步骤3删除
} ind_led_state_t;
```

### indicator.c 关键函数位置

| 函数 | 行号 | 改造步骤 |
|------|------|---------|
| `process_led_timeout()` | 148 | 步骤 5 |
| `start_led_effect()` | 240 | 步骤 4 |
| `indicator_init()` | 275 | 步骤 1 |
| `indicator_set()` | 306 | 不改（调用 start_led_effect）|
| `indicator_off()` | 314 | 步骤 6（增加 StopTask） |
| `indicator_off_all()` | 318 | 步骤 6（增加 StopTask 循环） |
| `indicator_task()` | 348 | 步骤 6（清空函数体） |

### delay=0 的初始化顺序（重要）

当前代码（start_led_effect 第 265-270 行）delay=0 路径：
```c
state->phase = PHASE_DELAY;      // 先设 phase
state->next_period = 0;          // 先设 next_period
process_led_timeout(led_id);     // 再调用 timeout（此时 phase 已正确）
```

改造后 delay=0 路径（保持相同顺序）：
```c
state->phase = PHASE_DELAY;      // 先设 phase（process_led_timeout 依赖此值）
state->effect = *effect;         // 确保 effect 已更新（on_ms/off_ms 可用）
state->count = 0;
// 不设置 OSAL 事件，直接同步执行首帧
process_led_timeout(led_id);     // 此函数内会设置后续 OSAL 事件
```

---

## 9. 参考文件

| 文件 | 用途 |
|------|------|
| `application/indicator_test.c` | OSAL 链式延迟事件完整示例 |
| `drivers/system/event_manager.h` | OSAL API 定义 |
| `drivers/output/indicators/indicator.c` | 核心改造文件（约 365 行） |
| `application/system/system_init.c:184` | 需删除 indicator_task() 调用 |
