# KB904 低电量联动设计

> 模块负责人: Agent D
> 创建日期: 2026-03-18
> 预估工时: 1.5h
> 开发顺序: 第二阶段

---

## 1. 模块职责

- 检测电池电量变化
- 电量 ≤20% 时关闭背光 + 红灯慢闪
- 电量恢复 >20% 时恢复背光 + 熄灭红灯
- 复用现有 `output_service_request_indicator()` 机制

---

## 2. 依赖关系

```
┌─────────────────────────────────────────┐
│            依赖的外部模块                │
├─────────────────────────────────────────┤
│  drivers/power/battery.c                │  ← 现有（battery_get_power_state）
│  application/service/input_service.h    │  ← 修改（电池检测事件）
│  application/service/input_service.c    │  ← 修改（电池检测逻辑）
│  application/service/output_service.c   │  ← 修改（指示灯联动）
│  模块01-backlight (output_service_set_backlight_low_battery) │
│  模块02-indicator (IND_REQ_LOW_BATTERY) │
└─────────────────────────────────────────┘
```

**依赖说明**:
- 依赖 **drivers/power/battery.c** 的 `battery_get_power_state()` 现有接口
- 依赖 **input_service** 提供电池检测 OSAL 定时事件
- 依赖 **output_service** 提供指示灯和背光联动接口
- 依赖 **01-backlight** 模块的 `output_service_set_backlight_low_battery()` 接口
- 依赖 **02-indicator** 模块的 `IND_REQ_LOW_BATTERY` 请求类型

---

## 3. 文件变更清单

### 3.1 修改 `application/service/input_service.h`

**任务**: 添加电池检测 OSAL 事件定义

```c
/* ========== 电池检测配置 ========== */
#ifndef BATTERY_CHECK_INTERVAL_MS
#define BATTERY_CHECK_INTERVAL_MS   30000   // 30秒检测一次电量
#endif

/* ========== OSAL 事件定义（新增） ========== */

#define INPUT_BATTERY_CHECK_EVT    (1 << 5)  /**< 电池定时检测事件 */
```

### 3.2 修改 `application/service/input_service.c`

**任务**: 使用 OSAL 定时事件实现电量检测

```c
#include "battery.h"
#include "output_service.h"

/* ========== 外部变量 ========== */
extern uint8_t input_taskID;

/* ========== 电池状态 ========== */
static uint8_t last_battery_state = BAT_POWER_NORMAL;

/**
 * @brief 初始化电池检测服务
 * @note 在 input_service_init() 中调用
 */
static void battery_check_init(void) {
    last_battery_state = battery_get_power_state();

    // 启动初始 30 秒电池检测定时器
    OSAL_StartTimer(input_taskID, INPUT_BATTERY_CHECK_EVT,
                    BATTERY_CHECK_INTERVAL_MS);
}

/**
 * @brief 处理电池状态变化
 * @note 由 OSAL 事件 INPUT_BATTERY_CHECK_EVT 触发
 */
static void handle_battery_state_change(void) {
    uint8_t current_state = battery_get_power_state();

    // 检测状态变化
    if (current_state != last_battery_state) {
        last_battery_state = current_state;

        if (current_state == BAT_POWER_LOW) {
            // 复用现有机制：发送低电量指示请求
            output_service_request_indicator(IND_REQ_LOW_BATTERY, 0);
        } else if (current_state == BAT_POWER_NORMAL) {
            // 电量恢复：发送正常指示请求
            output_service_request_indicator(IND_REQ_BATTERY_NORMAL, 0);
        }
    }

    // 重启 30 秒定时器
    OSAL_StartTimer(input_taskID, INPUT_BATTERY_CHECK_EVT,
                    BATTERY_CHECK_INTERVAL_MS);
}

// 在 input_process_event 中处理电池检测事件
uint16_t input_process_event(uint8_t task_id, uint16_t events) {
    // ... 现有事件处理 ...

    // ========== 电池定时检测事件 ==========
    if (events & INPUT_BATTERY_CHECK_EVT) {
        handle_battery_state_change();
        return (events ^ INPUT_BATTERY_CHECK_EVT);
    }

    // ... 其他事件 ...
}
```

### 3.3 修改 `application/service/output_service.c`

**任务**: 扩展指示灯请求处理，添加背光联动

```c
void output_service_request_indicator(ind_req_type_t type, uint8_t param) {
    (void)param;

    for (uint8_t i = 0; i < (sizeof(ind_req_map) / sizeof(ind_req_map[0])); i++) {
        if (ind_req_map[i].type == type) {
            uint8_t led_id = ind_req_map[i].led_id;

            // 设置指示灯效果
            ind_pending[led_id].effect = ind_req_map[i].effect;
            ind_pending[led_id].dirty = true;
            OSAL_SetEvent(output_taskID, OUTPUT_INDICATOR_EVT);

            // ========== 新增：低电量联动背光 ==========
            if (type == IND_REQ_LOW_BATTERY) {
                output_service_set_backlight_low_battery(true);
            } else if (type == IND_REQ_BATTERY_NORMAL) {
                output_service_set_backlight_low_battery(false);
            }

            return;
        }
    }
}
```

---

## 4. OSAL 事件调度流程

```
┌──────────────────────────────────────────────────────────────┐
│               OSAL 电池检测事件调度流程                       │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│  系统初始化                                                   │
│       │                                                      │
│       ▼                                                      │
│  OSAL_StartTimer(INPUT_BATTERY_CHECK_EVT, 30000ms)           │
│       │                                                      │
│       │  ┌─────────────────────────────────────────┐        │
│       │  │          等待 30 秒                       │        │
│       │  └─────────────────────────────────────────┘        │
│       │                         │                           │
│       ▼                         ▼                           │
│  OSAL 触发 INPUT_BATTERY_CHECK_EVT                          │
│       │                                                      │
│       ▼                                                      │
│  handle_battery_state_change()                              │
│       │                                                      │
│       ├──────────────────────────────────────┐              │
│       ▼                                      ▼              │
│  battery_get_power_state()            状态是否变化?         │
│       │                                      │              │
│       │                          ┌───────────┴───────┐      │
│       │                          ▼                   ▼      │
│       │                    状态变化            状态未变化   │
│       │                          │                   │      │
│       │                          ▼                   │      │
│       │         output_service_request_indicator()   │      │
│       │                    (低电量/恢复)              │      │
│       │                          │                   │      │
│       │                          ▼                   │      │
│       │              背光联动 + 指示灯效果            │      │
│       │                          │                   │      │
│       │                          ▼                   ▼      │
│       └────────────────────► OSAL_StartTimer(30000ms)       │
│                                                              │
│  ┌────────────────────────────────────────────────────────┐ │
│  │              低电量联动子流程                           │ │
│  │  output_service_request_indicator(IND_REQ_LOW_BATTERY) │ │
│  │       │                                               │ │
│  │       ├────────────────┬────────────────┐             │ │
│  │       ▼                ▼                ▼             │ │
│  │  ind_pending[]    OSAL_SetEvent()   output_service_   │ │
│  │  设置效果                           set_backlight_    │ │
│  │                                    low_battery(true)  │ │
│  │       │                │                │             │ │
│  │       ▼                ▼                ▼             │ │
│  │  红灯慢闪         事件触发           背光关闭          │ │
│  └────────────────────────────────────────────────────────┘ │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

---

## 5. 接口说明

### 5.1 新增 OSAL 事件

| 事件 | 说明 | 所属服务 |
|------|------|----------|
| `INPUT_BATTERY_CHECK_EVT` | 电池定时检测事件（30秒周期） | input_service |

### 5.2 复用的现有接口

| 函数 | 说明 |
|------|------|
| `output_service_request_indicator(type, param)` | 发送指示灯请求，触发联动 |
| `battery_get_power_state()` | 获取当前电池状态 |

### 5.3 依赖的模块接口

| 函数 | 来源模块 |
|------|----------|
| `output_service_set_backlight_low_battery(bool)` | 01-backlight |

### 5.4 OSAL API 使用

| 函数 | 说明 |
|------|------|
| `OSAL_StartTimer(input_taskID, INPUT_BATTERY_CHECK_EVT, 30000)` | 启动 30 秒电池检测定时器 |

---

## 6. 测试要点

| 测试项 | 测试方法 | 预期结果 |
|--------|----------|----------|
| 低电量触发 | 模拟电量降到 ≤20% | 红灯开始慢闪，背光立即关闭 |
| 背光联动 | 低电量状态下按键 | 背光不亮，红灯持续闪烁 |
| 电量恢复 | 模拟电量恢复到 >20% | 红灯熄灭，背光可正常唤醒 |
| 状态去抖 | 电量在 20% 边界波动 | 只触发一次状态切换 |
| 定时检测 | 等待 30 秒 | 事件触发，电量检测执行 |

---

## 7. 完成标准

- [ ] `input_service.h` 电池检测事件定义正确
- [ ] `input_service.c` OSAL 定时事件编译通过
- [ ] `INPUT_BATTERY_CHECK_EVT` 事件正常触发（30秒周期）
- [ ] 低电量时红灯慢闪
- [ ] 低电量时背光关闭
- [ ] 低电量时按键不能唤醒背光
- [ ] 电量恢复后背光可正常唤醒
- [ ] 状态去抖正确（边界波动只触发一次）

---

*文档结束*
