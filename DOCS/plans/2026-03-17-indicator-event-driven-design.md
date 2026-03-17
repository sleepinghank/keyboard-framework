# 指示灯事件驱动重构设计文档

- **日期**: 2026-03-17
- **状态**: 待实施

---

## 一、需求背景

### 1.1 问题描述

当前项目中所有指示灯控制均通过直接调用 `indicator_set()` 实现，共 11 处调用点分散在 4 个文件中，其中 5 处位于 middleware 层，违反了层级边界（middleware 不应直接操作 driver）。

### 1.2 目标

- 将 middleware 层对 `indicator_set()` 的直接调用，改为通过 OSAL 事件 + 中间变量的方式，由 `output_service` 统一管理
- 降低模块耦合度，指示灯控制逻辑集中在一处
- 支持 LPM 深度睡眠唤醒后自动恢复灯效

### 1.3 范围边界

**在范围内：**
- `middleware/communication/wireless_callbacks.c` 调用点改造
- `middleware/communication/wireless.c` 调用点改造
- `keyboards/n0046/n0046.c` 调用点改造
- `application/service/input_service.c` 调用点改造
- `application/service/output_service.c` 填充 `OUTPUT_INDICATOR_EVT` 和 `OUTPUT_LPM_RESUME_EVT` 处理逻辑

**不在范围内：**
- `drivers/output/indicators/indicator.c/h`（驱动层不改动）
- 多优先级冲突处理
- RGB/背光相关

---

## 二、现状分析

### 2.1 调用点全景

| 文件 | 层级 | 调用点数 | 语义 |
|------|------|----------|------|
| `middleware/communication/wireless.c:91` | middleware | 1 | `wireless_indicator_update()` 内部调用 |
| `middleware/communication/wireless_callbacks.c:77,92,119,136` | middleware | 4 | 配对/重连/连接/断开回调 |
| `keyboards/n0046/n0046.c:91,97,102,107` | application | 4 | CAPS 灯随无线状态切换 |
| `application/service/input_service.c:152` | application | 1 | 低电量快闪（硬编码 LED 索引 0） |

### 2.2 层级违规

```
当前（违规）：
middleware/wireless.c        ──直接调用──▶  drivers/indicator.c
middleware/wireless_callbacks.c ─────────▶  drivers/indicator.c

期望（合规）：
middleware/wireless.c        ──事件──▶  application/output_service.c
                                                  │
                                           indicator_set()
                                                  │
                                           drivers/indicator.c
```

### 2.3 已有基础

- `OUTPUT_INDICATOR_EVT` 已在 `output_service.h` 定义（bit 0），处理桩已存在
- `output_taskID` 已对外 extern，可直接用于 `OSAL_SetEvent`
- `wireless.c` 内已有 `wireless_indicator_effect()` 语义→灯效映射函数，可迁移至映射表
- `OUTPUT_LPM_RESUME_EVT` 中已有 `indicator_update_from_wireless_state()` 注释，预留了恢复入口

---

## 三、方案设计

### 3.1 架构

```
┌──────────────────────────────────────────────────────────┐
│  调用方（middleware / keyboards / input_service）         │
│                                                           │
│  output_service_request_indicator(type, param)           │
└─────────────────────────────┬────────────────────────────┘
                              │ 写 ind_pending + OSAL_SetEvent
┌─────────────────────────────▼────────────────────────────┐
│  application/service/output_service.c                    │
│                                                           │
│  static ind_pending_t ind_pending[IND_LED_COUNT]         │
│                                                           │
│  OUTPUT_INDICATOR_EVT：遍历 dirty 槽 → indicator_set()   │
│  OUTPUT_LPM_RESUME_EVT：遍历非空槽 → indicator_set()     │
└─────────────────────────────┬────────────────────────────┘
                              │ indicator_set(led_id, effect)
┌─────────────────────────────▼────────────────────────────┐
│  drivers/output/indicators/indicator.c（不改动）          │
└──────────────────────────────────────────────────────────┘
```

### 3.2 数据结构

#### 3.2.1 请求类型枚举（新增至 `output_service.h`）

```c
typedef enum {
    IND_REQ_NONE = 0,
    /* 无线状态类（param = host_idx） */
    IND_REQ_BT_PAIRING,           /* 配对中：白灯慢闪 */
    IND_REQ_BT_RECONNECTING,      /* 重连中：白灯快闪 */
    IND_REQ_BT_CONNECTED,         /* 已连接：白灯常亮 */
    IND_REQ_BT_DISCONNECTED,      /* 已断开：白灯熄灭 */
    /* CAPS 类（param 无效，填 0） */
    IND_REQ_CAPS_ON,              /* 大写开：白灯常亮 */
    IND_REQ_CAPS_OFF,             /* 大写关：白灯熄灭 */
    IND_REQ_CAPS_DISCONNECTED,    /* 断开时 CAPS 状态：白灯慢闪 */
    /* 电量类（param 无效，填 0） */
    IND_REQ_LOW_BATTERY,          /* 低电量：红灯快闪 */
    IND_REQ_BATTERY_NORMAL,       /* 电量正常：红灯熄灭 */
} ind_req_type_t;
```

#### 3.2.2 中间变量（`output_service.c` 内部私有）

```c
/* 每个物理 LED 一个 pending 槽，存解析后的灯效 */
typedef struct {
    const ind_effect_t *effect;  /* 已解析的灯效，NULL = 从未设置 */
    bool                dirty;   /* true = 待执行 */
} ind_pending_t;

static ind_pending_t ind_pending[IND_LED_COUNT];  /* 全零初始化 */
```

#### 3.2.3 映射表（`output_service.c` 内部私有）

```c
typedef struct {
    ind_req_type_t     type;
    uint8_t            led_id;
    const ind_effect_t *effect;
} ind_req_map_t;

static const ind_req_map_t ind_req_map[] = {
    { IND_REQ_BT_PAIRING,        LED_WHITE, &IND_BLINK_SLOW },
    { IND_REQ_BT_RECONNECTING,   LED_WHITE, &IND_BLINK_FAST },
    { IND_REQ_BT_CONNECTED,      LED_WHITE, &IND_ON         },
    { IND_REQ_BT_DISCONNECTED,   LED_WHITE, &IND_OFF        },
    { IND_REQ_CAPS_ON,           LED_WHITE, &IND_ON         },
    { IND_REQ_CAPS_OFF,          LED_WHITE, &IND_OFF        },
    { IND_REQ_CAPS_DISCONNECTED, LED_WHITE, &IND_BLINK_SLOW },
    { IND_REQ_LOW_BATTERY,       LED_RED,   &IND_BLINK_FAST },
    { IND_REQ_BATTERY_NORMAL,    LED_RED,   &IND_OFF        },
};
```

### 3.3 接口设计

#### `output_service.h` 新增

```c
/* 指示灯请求类型 */
typedef enum { ... } ind_req_type_t;

/* 写入接口：供 middleware 和 application 调用 */
void output_service_request_indicator(ind_req_type_t type, uint8_t param);
```

#### `output_service.c` 写入函数实现

```c
void output_service_request_indicator(ind_req_type_t type, uint8_t param) {
    /* 查映射表，找到 led_id 和 effect */
    for (uint8_t i = 0; i < sizeof(ind_req_map) / sizeof(ind_req_map[0]); i++) {
        if (ind_req_map[i].type == type) {
            uint8_t led_id = ind_req_map[i].led_id;
            ind_pending[led_id].effect = ind_req_map[i].effect;
            ind_pending[led_id].dirty  = true;
            OSAL_SetEvent(output_taskID, OUTPUT_INDICATOR_EVT);
            return;
        }
    }
    /* type 不在映射表中，忽略 */
}
```

#### `OUTPUT_INDICATOR_EVT` 处理

```c
if (events & OUTPUT_INDICATOR_EVT) {
    for (uint8_t i = 0; i < IND_LED_COUNT; i++) {
        if (ind_pending[i].dirty) {
            indicator_set(i, ind_pending[i].effect);
            ind_pending[i].dirty = false;
        }
    }
    return (events ^ OUTPUT_INDICATOR_EVT);
}
```

#### `OUTPUT_LPM_RESUME_EVT` 处理

```c
if (events & OUTPUT_LPM_RESUME_EVT) {
    for (uint8_t i = 0; i < IND_LED_COUNT; i++) {
        if (ind_pending[i].effect != NULL) {
            indicator_set(i, ind_pending[i].effect);
        }
    }
    return (events ^ OUTPUT_LPM_RESUME_EVT);
}
```

### 3.4 调用方改造对应关系

#### `wireless_callbacks.c`

| 原调用 | 新调用 |
|--------|--------|
| `indicator_set(led, &IND_BLINK_SLOW)` | `output_service_request_indicator(IND_REQ_BT_PAIRING, host_idx)` |
| `indicator_set(led, &IND_BLINK_FAST)` | `output_service_request_indicator(IND_REQ_BT_RECONNECTING, host_idx)` |
| `indicator_set(led, &IND_ON)` | `output_service_request_indicator(IND_REQ_BT_CONNECTED, host_idx)` |
| `indicator_set(led, &IND_OFF)` | `output_service_request_indicator(IND_REQ_BT_DISCONNECTED, host_idx)` |

删除：`#include "indicator.h"`、`wireless_cb_indicator_led()` 辅助函数

#### `wireless.c`

删除：`wireless_indicator_update()`、`wireless_indicator_effect()`、`wireless_indicator_led()`、`#include "indicator.h"`

> 说明：这三个函数的逻辑已由映射表和 `wireless_callbacks.c` 覆盖，不再需要。

#### `n0046.c`

| 原调用 | 新调用 |
|--------|--------|
| `indicator_off(IND_LED_CAPS)` | `output_service_request_indicator(IND_REQ_CAPS_OFF, 0)` |
| `indicator_set(IND_LED_CAPS, &IND_BLINK_SLOW)`（断开） | `output_service_request_indicator(IND_REQ_CAPS_DISCONNECTED, 0)` |
| `indicator_set(IND_LED_CAPS, &IND_BLINK_FAST)`（配对） | `output_service_request_indicator(IND_REQ_BT_PAIRING, host_idx)` |
| `indicator_set(IND_LED_CAPS, &IND_BLINK_SLOW)`（重连） | `output_service_request_indicator(IND_REQ_BT_RECONNECTING, host_idx)` |

删除：`#include "indicator.h"`

#### `input_service.c`

| 原调用 | 新调用 |
|--------|--------|
| `indicator_set(0, &IND_BLINK_FAST)` | `output_service_request_indicator(IND_REQ_LOW_BATTERY, 0)` |

---

## 四、完整数据流

```
【正常流程示例：BT 配对】

wireless_enter_discoverable_kb(host_idx=1)
    │
    ▼
output_service_request_indicator(IND_REQ_BT_PAIRING, 1)
    ├─ 查映射表 → led_id=LED_WHITE(0), effect=&IND_BLINK_SLOW
    ├─ ind_pending[0].effect = &IND_BLINK_SLOW
    ├─ ind_pending[0].dirty  = true
    └─ OSAL_SetEvent(output_taskID, OUTPUT_INDICATOR_EVT)
            │
            ▼  （OSAL 调度）
    output_process_event()
        └─ i=0: dirty=true → indicator_set(0, &IND_BLINK_SLOW)
                              ind_pending[0].dirty = false

【LPM Deep 唤醒恢复】

OUTPUT_LPM_RESUME_EVT
    └─ 遍历 ind_pending：effect 非 NULL 则重放 indicator_set()

【多源竞争同一 LED（CAPS + BT 共用白灯）】

T1: output_service_request_indicator(IND_REQ_BT_CONNECTED, 1)
    → ind_pending[LED_WHITE].effect = &IND_ON

T2: output_service_request_indicator(IND_REQ_CAPS_DISCONNECTED, 0)
    → ind_pending[LED_WHITE].effect = &IND_BLINK_SLOW  ← 覆盖

事件处理：indicator_set(LED_WHITE, &IND_BLINK_SLOW)
```

---

## 五、错误处理

| 场景 | 处理方式 |
|------|----------|
| `type` 不在映射表中 | 遍历完映射表无匹配，直接 return，不触发事件 |
| `output_service_init()` 未调用 | `ind_pending` 全零，dirty=false，事件处理跳过，无副作用 |
| LPM resume 时 `ind_pending` 全空 | 遍历时 effect 全为 NULL，跳过，灯保持熄灭 |

---

## 六、实施计划

### Step 1 — 扩展 `output_service.h`

**修改：** 新增 `ind_req_type_t` 枚举 + `output_service_request_indicator()` 声明

**验证：** 编译通过

### Step 2 — 实现中间变量、映射表、写入函数（`output_service.c`）

**修改：** 新增 `ind_pending_t`、`ind_pending[]`、`ind_req_map[]`、`output_service_request_indicator()`

**验证：** 调用写入函数后 `ind_pending[LED_WHITE].effect == &IND_BLINK_SLOW`

**前置：** Step 1

### Step 3 — 填充事件处理逻辑（`output_service.c`）

**修改：** 填充 `OUTPUT_INDICATOR_EVT`、`OUTPUT_LPM_RESUME_EVT` 处理块

**验证：** stub 测试 `indicator_set` 调用次数和参数正确

**前置：** Step 2

### Step 4 — 删除 `wireless.c` 冗余函数

**修改：** 删除 `wireless_indicator_update()`、`wireless_indicator_effect()`、`wireless_indicator_led()`、`#include "indicator.h"`

**验证：** 编译通过，无 indicator 符号残留

**前置：** Step 3

### Step 5 — 改造 `wireless_callbacks.c`

**修改：** 删除 `#include "indicator.h"`，4 处调用改为 `output_service_request_indicator()`

**验证：** 编译通过，无 `indicator.h` 依赖

**前置：** Step 3

### Step 6 — 改造 `keyboards/n0046/n0046.c`

**修改：** 删除 `#include "indicator.h"`，4 处调用改为 `output_service_request_indicator()`

**验证：** 编译通过，无 `indicator.h` 依赖

**前置：** Step 3

### Step 7 — 改造 `application/service/input_service.c`

**修改：** 低电量处改为 `output_service_request_indicator(IND_REQ_LOW_BATTERY, 0)`

**验证：** 编译通过，无硬编码 LED 索引

**前置：** Step 3
