# kb_sys_action 组合键事件实现 — 设计文档

**日期**：2026-03-21
**分支**：feature/touch
**状态**：已评审
**需求文档**：`docs/plans/2026-03-21-kb-sys-action-events-requirements.md`

## 1. 需求摘要

将 `kb_sys_action.c` 中 7 个空引用 SYS_EVT_* 事件全部实现，合并 `kb_fn_action.c` 重叠函数，统一 OSAL 事件调度，combo 层不直接依赖底层模块（storage 宏除外）。

## 2. 现状分析

### 2.1 核心问题

- `kb_sys_action.c` 定义 7 个 SYS_EVT_*（0x0001~0x0040），与 system_service.h enum 位冲突
- `SYSTEM_SERVICE_TASK_ID` 硬编码为 0，实际 system_taskID 动态分配，事件投递到错误任务
- `kb_fn_action.c` 中 3 个函数直接调用 backlight/battery/indicator，跨越层级

### 2.2 可复用资源

- `OUTPUT_BACKLIGHT_BRIGHTNESS_EVT` (bit 1) — 已有空占位
- `OUTPUT_BACKLIGHT_COLOR_EVT` (bit 2) — 已有空占位
- `WL_PAIR_EVT` (bit 10) — 已有完整配对流程
- `SYSTEM_STORAGE_EVT` (bit 4) — 已有 storage_save()
- `SYSTEM_FACTORY_RESET_EVT` (bit 6) — 已有完整流程（Factory_Reset 已正确使用）

## 3. 方案设计

### 3.1 事件映射总览

```
组合键触发
    |
    v
kb_sys_action.c（combo 回调）
    |
    +-- Backlight_Light()    --> OSAL_SetEvent(output_taskID, OUTPUT_BACKLIGHT_BRIGHTNESS_EVT)
    |                            OSAL_SetEvent(system_taskID, SYSTEM_STORAGE_EVT)
    |
    +-- Backlight_Color()    --> OSAL_SetEvent(output_taskID, OUTPUT_BACKLIGHT_COLOR_EVT)
    |                            OSAL_SetEvent(system_taskID, SYSTEM_STORAGE_EVT)
    |
    +-- Bat_Check()          --> OSAL_SetEvent(output_taskID, OUTPUT_BATTERY_CHECK_EVT)
    |
    +-- Touch_Locked()       --> OSAL_SetEvent(input_taskID, INPUT_TOUCH_TOGGLE_EVT)
    |
    +-- Factory_Reset()      --> OSAL_SetEvent(system_taskID, SYSTEM_FACTORY_RESET_EVT)  [不变]
    |
    +-- Pair_button()        --> OSAL_SetEvent(commu_taskID, WL_PAIR_EVT)
    |
    +-- Set_FN_Lock()        --> STORAGE_SET_FN_LOCK(toggle)
                                 OSAL_SetEvent(system_taskID, SYSTEM_STORAGE_EVT)
```

### 3.2 kb_sys_action.c 重写

**删除**：
- 全部 `#ifndef SYS_EVT_*` ... `#define` ... `#endif` 块（行 9~31）
- `#ifndef SYSTEM_SERVICE_TASK_ID` ... `#define` ... `#endif` 块（行 33~36）

**新增 include**：
```c
#include "output_service.h"
#include "input_service.h"
#include "communication_service.h"
#include "storage.h"
```

**函数实现**：

```c
uint8_t Set_FN_Lock(uint16_t* add_keys) {
    uint8_t current = STORAGE_GET_FN_LOCK();
    STORAGE_SET_FN_LOCK(current ? 0 : 1);
    OSAL_SetEvent(system_taskID, SYSTEM_STORAGE_EVT);
    return 0;
}

uint8_t Backlight_Color(uint16_t* add_keys) {
    OSAL_SetEvent(output_taskID, OUTPUT_BACKLIGHT_COLOR_EVT);
    OSAL_SetEvent(system_taskID, SYSTEM_STORAGE_EVT);
    return 0;
}

uint8_t Backlight_Light(uint16_t* add_keys) {
    OSAL_SetEvent(output_taskID, OUTPUT_BACKLIGHT_BRIGHTNESS_EVT);
    OSAL_SetEvent(system_taskID, SYSTEM_STORAGE_EVT);
    return 0;
}

uint8_t Touch_Locked(uint16_t* add_keys) {
    OSAL_SetEvent(input_taskID, INPUT_TOUCH_TOGGLE_EVT);
    OSAL_SetEvent(system_taskID, SYSTEM_STORAGE_EVT);
    return 0;
}

uint8_t Factory_Reset(uint16_t* add_keys) {
    OSAL_SetEvent(system_taskID, SYSTEM_FACTORY_RESET_EVT);
    return 0;
}

uint8_t Bat_Check(uint16_t* add_keys) {
    OSAL_SetEvent(output_taskID, OUTPUT_BATTERY_CHECK_EVT);
    return 0;
}

uint8_t Pair_button(uint16_t* add_keys) {
    OSAL_SetEvent(commu_taskID, WL_PAIR_EVT);
    return 0;
}
```

DE/UK 特殊 combo 函数及底部注释代码保持不变。

### 3.3 kb_fn_action.c 瘦身

**删除函数**：
- `Backlight_Level_Up()`（行 360~365）
- `Backlight_Color_Next()`（行 374~382）
- `Battery_Check()`（行 393~402）
- `static uint8_t battery_blink_count()`（行 91~102）

**删除 include**：
- `#include "backlight.h"`
- `#include "battery.h"`
- `#include "indicator.h"`

**kb_fn_action.h 删除声明**：
- `uint8_t Backlight_Level_Up(uint16_t* add_keys);`
- `uint8_t Backlight_Color_Next(uint16_t* add_keys);`
- `uint8_t Battery_Check(uint16_t* add_keys);`

### 3.4 kb_combo_map.c 引用替换

```c
// 行 79~81 替换
[FN_RIGHT_SHIFT_ID] = COMBO(FN_RIGHT_SHIFT_combo, PRESS_DOWN, Backlight_Light),
[FN_RIGHT_ENTER_ID] = COMBO(FN_RIGHT_ENTER_combo, PRESS_DOWN, Backlight_Color),
[FN_RIGHT_CMD_ID]   = COMBO(FN_RIGHT_CMD_combo,   PRESS_DOWN, Bat_Check),
```

确认 `kb_combo_map.c` 已 include `kb_sys_action.h`。

### 3.5 output_service 事件处理

**output_service.h**：新增 `OUTPUT_BATTERY_CHECK_EVT = 1 << 9`

**output_service.c**：

填充 `OUTPUT_BACKLIGHT_BRIGHTNESS_EVT`（替换 Placeholder）：
```c
if (events & OUTPUT_BACKLIGHT_BRIGHTNESS_EVT) {
    backlight_level_step();
    output_service_note_backlight_activity();
    return (events ^ OUTPUT_BACKLIGHT_BRIGHTNESS_EVT);
}
```

填充 `OUTPUT_BACKLIGHT_COLOR_EVT`（替换 Placeholder）：
```c
if (events & OUTPUT_BACKLIGHT_COLOR_EVT) {
    if (backlight_is_enabled()) {
        backlight_color_step();
    }
    output_service_note_backlight_activity();
    return (events ^ OUTPUT_BACKLIGHT_COLOR_EVT);
}
```

新增 `OUTPUT_BATTERY_CHECK_EVT`：
```c
if (events & OUTPUT_BATTERY_CHECK_EVT) {
    uint8_t percentage = battery_get_percentage();
    uint8_t blink_count;
    if (percentage >= 75)      blink_count = 4;
    else if (percentage >= 50) blink_count = 3;
    else if (percentage >= 25) blink_count = 2;
    else                       blink_count = 1;

    ind_effect_t effect = IND_BLINK_CUSTOM(200, 200, blink_count);
    indicator_set(LED_POWER_RED, &effect);
    return (events ^ OUTPUT_BATTERY_CHECK_EVT);
}
```

新增 `#include "battery.h"`。

### 3.6 input_service 触摸板事件

**input_service.h**：新增 `INPUT_TOUCH_TOGGLE_EVT = 1 << 2`

**input_service.c**：
```c
if (events & INPUT_TOUCH_TOGGLE_EVT) {
    // TODO: 触摸板锁定/解锁逻辑，待 touchpad 模块接口稳定后实现
    dprintf("Input: Touch toggle event received (not implemented)\r\n");
    return (events ^ INPUT_TOUCH_TOGGLE_EVT);
}
```

## 4. 评审意见与决策

| 维度 | 结论 | 关键发现 | 决策 |
|------|------|---------|------|
| 功能完整性 | 通过 | 7 个事件全覆盖 | — |
| 技术可行性 | 通过 | output_service 调 battery_get_percentage 引入反向依赖 | 维持，仅读 RAM 缓存无副作用 |
| 可维护性 | 通过 | 改动局部化良好 | — |
| 可测试性 | 通过 | 状态交叉场景为盲区 | 不在本次范围 |
| 风险识别 | 2 项关注 | EEPROM 磨损 + FN Lock 并发 | EEPROM 防护交给 storage_save 内部 |

## 5. 实施计划

| # | 任务 | 文件 | 完成标准 | 前置依赖 |
|---|------|------|---------|---------|
| 1 | output_service.h 新增事件 | `application/service/output_service.h` | 新增 OUTPUT_BATTERY_CHECK_EVT = 1 << 9 | 无 |
| 2 | input_service.h 新增事件 | `application/service/input_service.h` | 新增 INPUT_TOUCH_TOGGLE_EVT = 1 << 2 | 无 |
| 3 | output_service.c 填充处理 | `application/service/output_service.c` | 填充背光空占位 + 新增电池检查 + include battery.h | Step 1 |
| 4 | input_service.c 新增处理 | `application/service/input_service.c` | 新增 TOUCH_TOGGLE handler（dprintf + TODO） | Step 2 |
| 5 | kb_sys_action.c 重写 | `middleware/keyboard/combo/kb_sys_action.c` | 删除 SYS_EVT_*，新增 service include，重写 7 个函数 | Step 1, 2 |
| 6 | kb_sys_action.h 更新 | `middleware/keyboard/combo/kb_sys_action.h` | 确认声明与实现一致 | Step 5 |
| 7 | kb_fn_action.c 删除重叠 | `middleware/keyboard/combo/kb_fn_action.c` | 删除 3 个函数 + battery_blink_count + 清理 include | Step 5 |
| 8 | kb_fn_action.h 删除声明 | `middleware/keyboard/combo/kb_fn_action.h` | 删除 3 个函数声明 | Step 7 |
| 9 | kb_combo_map.c 替换引用 | `middleware/keyboard/combo/kb_combo_map.c` | 3 行替换 + 确认 include kb_sys_action.h | Step 5, 8 |
| 10 | 编译验证 | — | /wch-riscv-build 编译通过，无未定义符号 | Step 1-9 |

## 6. 验证策略

- **编译验证**：使用 `/wch-riscv-build` skill 执行 CH584M 工具链编译
- **日志验证**：各事件处理入口 dprintf/println，串口日志确认事件到达
- **功能回归**：
  - Fn+右Shift -> 背光亮度步进
  - Fn+右Enter -> 背光颜色步进
  - Fn+右Cmd -> 红灯闪烁（电量对应次数）
  - 组合键配对 -> 进入配对模式
  - 组合键 Factory Reset -> 灯闪 + 重启
  - 触摸板锁定 -> 串口打印 "not implemented"
