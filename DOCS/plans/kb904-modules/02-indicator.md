# KB904 指示灯系统设计

> 模块负责人: Agent B
> 创建日期: 2026-03-18
> 预估工时: 1.5h
> 开发顺序: 第一阶段（并行）

---

## 1. 模块职责

- 扩展指示灯配置从 2 灯到 4 灯
- 实现蓝牙状态指示（配对快闪/回连慢闪）
- 实现大写锁定指示
- 实现低电量指示（红灯闪烁）

---

## 2. 依赖关系

```
┌─────────────────────────────────────────┐
│            依赖的外部模块                │
├─────────────────────────────────────────┤
│  drivers/output/indicators/indicator.c  │  ← 现有
│  keyboards/indicator_config.h           │  ← 修改
│  application/service/output_service.c   │  ← 修改
└─────────────────────────────────────────┘
```

---

## 3. 文件变更清单

### 3.1 修改 `keyboards/indicator_config.h`

**任务**: 通过条件编译支持 KB904 的 4 灯配置

```c
#pragma once

#include "gpio.h"
#include <stdbool.h>

/* ============ 产品区分 ============ */

#if PRODUCT_ID == 0x0904  // KB904

/* ========== KB904: 4 颗 LED ========== */
#define IND_LED_COUNT   4

/* LED 索引定义 */
#define LED_CAPS        0   // 白灯：大写锁定
#define LED_BT          1   // 蓝灯：蓝牙状态
#define LED_POWER_RED   2   // 红灯：电源/低电量
#define LED_POWER_GRN   3   // 绿灯：充满电（只读，TP4056控制）

/* 兼容性别名 */
#define LED_BAT         LED_POWER_RED
#define LED_CHARGE      LED_POWER_RED
#define LED_WHITE       LED_CAPS
#define LED_RED         LED_POWER_RED

/* LED 硬件配置表 */
static const ind_led_def_t ind_led_table[IND_LED_COUNT] = {
    /* LED_CAPS */       {A15, true},   // 白灯：大写锁定
    /* LED_BT */         {B22, true},   // 蓝灯：蓝牙状态
    /* LED_POWER_RED */  {B23, true},   // 红灯：电源/低电量
    /* LED_POWER_GRN */  {A14, true},   // 绿灯：充满（仅读取）
};

/* 硬件限制说明：
 * - LED_POWER_GRN (A14) 由 TP4056 硬件控制
 * - 软件只能读取状态，无法主动控制
 * - 充电中：红灯亮（TP4056_CHRG = 低）
 * - 充满：绿灯亮（TP4056_STDBY = 低）
 */

#else  // N0046 及其他产品

/* ========== N0046: 2 颗 LED ========== */
#define IND_LED_COUNT   2

#define LED_WHITE       0
#define LED_RED         1
#define LED_CAPS        LED_WHITE
#define LED_BAT         LED_RED
#define LED_CHARGE      LED_RED

static const ind_led_def_t ind_led_table[IND_LED_COUNT] = {
    {A15, true},   // 白灯
    {B23, true},   // 红灯
};

#endif
```

### 3.2 修改 `application/service/output_service.c`

**任务**: 扩展指示灯请求映射

```c
// 扩展现有 ind_req_map
static const ind_req_map_t ind_req_map[] = {
    // 蓝牙状态（KB904 使用独立蓝牙灯）
#if PRODUCT_ID == 0x0904
    { IND_REQ_BT_PAIRING,        LED_BT,   &IND_BLINK_FAST },  // 蓝灯快闪
    { IND_REQ_BT_RECONNECTING,   LED_BT,   &IND_BLINK_SLOW },  // 蓝灯慢闪
    { IND_REQ_BT_CONNECTED,      LED_BT,   &IND_OFF        },  // 蓝灯灭
    { IND_REQ_BT_DISCONNECTED,   LED_BT,   &IND_OFF        },  // 蓝灯灭
#else
    { IND_REQ_BT_PAIRING,        LED_WHITE, &IND_BLINK_SLOW },
    { IND_REQ_BT_RECONNECTING,   LED_WHITE, &IND_BLINK_FAST },
    { IND_REQ_BT_CONNECTED,      LED_WHITE, &IND_ON         },
    { IND_REQ_BT_DISCONNECTED,   LED_WHITE, &IND_OFF        },
#endif

    // 大写锁定（共用白灯）
    { IND_REQ_CAPS_ON,           LED_CAPS, &IND_ON         },
    { IND_REQ_CAPS_OFF,          LED_CAPS, &IND_OFF        },
    { IND_REQ_CAPS_DISCONNECTED, LED_CAPS, &IND_BLINK_SLOW },

    // 电源/电量状态（红灯）
    { IND_REQ_LOW_BATTERY,       LED_BAT,  &IND_BLINK_SLOW },
    { IND_REQ_BATTERY_NORMAL,    LED_BAT,  &IND_OFF        },
};
```

---

## 4. 指示灯状态表

### 4.1 蓝牙状态指示灯（蓝灯 B22）

| 蓝牙状态 | 灯效 |
|----------|------|
| 配对中 | 快闪（3Hz） |
| 回连中 | 慢闪（1Hz） |
| 已连接 | 熄灭 |
| 未连接 | 熄灭 |

### 4.2 大写锁定指示灯（白灯 A15）

| 状态 | 灯效 |
|------|------|
| 大写锁定 | 常亮 |
| 大写未锁定 | 熄灭 |

### 4.3 电源指示灯（红灯 B23）

| 场景 | 状态 | 灯效 |
|------|------|------|
| 电量 100%~20% | 正常 | 熄灭 |
| 电量 <20% | 低电量警告 | 慢闪 |
| 充电中 | 未充满 | 常亮（TP4056 控制） |
| 充电中 | 已充满 | 绿灯亮（TP4056 控制） |

### 4.4 电量检查（Fn+右Cmd）

| 电量范围 | 闪烁次数 |
|----------|----------|
| 0-25% | 1 次 |
| 25-50% | 2 次 |
| 50-75% | 3 次 |
| 75-100% | 4 次 |

---

## 5. 测试要点

| 测试项 | 测试方法 | 预期结果 |
|--------|----------|----------|
| 配对指示 | 启动配对模式 | 蓝灯快闪（3Hz） |
| 回连指示 | 开机/唤醒回连 | 蓝灯慢闪（1Hz） |
| 已连接指示 | 连接成功后 | 蓝灯熄灭 |
| 大写锁定 | 按下 Caps Lock | 白灯亮/灭切换 |
| 低电量警告 | 模拟电量≤20% | 红灯慢闪 |
| 电量检查 | 按 Fn+右Cmd | 红灯闪烁 1-4 次 |

---

## 6. 完成标准

- [ ] `indicator_config.h` 条件编译编译通过
- [ ] 4 颗 LED 初始化正确
- [ ] 蓝牙状态指示正确
- [ ] 大写锁定指示正确
- [ ] 低电量指示正确

---

*文档结束*
