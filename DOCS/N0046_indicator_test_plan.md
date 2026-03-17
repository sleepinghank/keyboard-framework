# N0046 指示灯功能测试方案

## 一、概述

本文档描述 N0046 键盘固件中指示灯驱动（`indicator.c`）的硬件验证测试方案。
测试目标：验证熄灭、常亮、常亮定时、慢闪、快闪、有限次闪烁、延迟启动、批量关闭等全部灯效模式在实际硬件上工作正常。

**测试硬件：**

| LED | GPIO | 极性 | 颜色 |
|-----|------|------|------|
| LED_WHITE（索引 0） | A15 | 高电平亮 | 白灯（大小写/蓝牙指示） |
| LED_RED（索引 1） | B23 | 高电平亮 | 红灯（电源指示） |

> **注意：** B23 在 `_pin_defs.h` 中被标记为某些 CH584 封装的复位引脚。
> N0046 原理图已确认该引脚在本产品中用作红灯，可正常配置为推挽输出。

---

## 二、前置条件

烧录固件前，必须完成以下 4 项代码修改，否则测试无法执行。

### 2.1 修改 indicator_config.h（引脚映射）

文件：`keyboards/indicator_config.h`

将占位符引脚改为实际硬件引脚，LED 数量改为 2：

```c
#define IND_LED_COUNT   2

static const ind_led_def_t ind_led_table[IND_LED_COUNT] = {
    /* 索引 0 */ {A15, true},   // 白灯：大小写/蓝牙指示灯
    /* 索引 1 */ {B23, true},   // 红灯：电源指示灯
};

#define LED_WHITE   0
#define LED_RED     1
```

### 2.2 取消注释 indicator_init()

文件：`application/system/system_init.c`，`system_init_drivers()` 函数

```c
// 修改前（被注释）
// indicator_init();

// 修改后（取消注释）
indicator_init();
```

### 2.3 在主循环添加 indicator_task()

文件：`application/main.c`，`while(1)` 主循环中

```c
while (1) {
    if (input_get_matrix_scan_flag()) {
        keyboard_task();
        input_clear_matrix_scan_flag();
    }

    indicator_task();   // 新增：驱动指示灯状态机

    OSAL_SystemProcessOnce();
}
```

### 2.4 开启测试宏

文件：`application/sys_config.h`（或编译参数）

```c
#define INDICATOR_TEST_ENABLE   // 开启指示灯测试序列
```

**核查清单（烧录前逐项确认）：**

```
□ indicator_config.h：IND_LED_COUNT=2，A15 白灯，B23 红灯，别名已定义
□ system_init_drivers()：indicator_init() 已取消注释
□ main.c 主循环：indicator_task() 已添加
□ sys_config.h：INDICATOR_TEST_ENABLE 已定义
```

---

## 三、测试代码设计

### 3.1 文件结构

```
application/system/
├── indicator_test.h    // 新增：宏开关 + 函数声明
└── indicator_test.c    // 新增：8 阶段测试状态机
```

### 3.2 indicator_test.h

```c
#pragma once

#ifdef INDICATOR_TEST_ENABLE
void indicator_test(void);
#else
#define indicator_test()
#endif
```

### 3.3 indicator_test.c

```c
#include "indicator_test.h"

#ifdef INDICATOR_TEST_ENABLE

#include "indicator.h"
#include "indicator_config.h"
#include "event_manager.h"

/* 测试阶段定义 */
typedef enum {
    TEST_PHASE_WHITE_ON = 0,        // 白灯常亮（无限）
    TEST_PHASE_WHITE_OFF,           // 白灯熄灭
    TEST_PHASE_RED_ON_2S,           // 红灯常亮 2s 后自动灭
    TEST_PHASE_WHITE_BLINK_SLOW,    // 白灯慢闪（250ms 亮 + 250ms 灭 = 500ms 周期）
    TEST_PHASE_RED_BLINK_FAST,      // 红灯快闪 100ms 周期
    TEST_PHASE_WHITE_BLINK_3,       // 白灯闪 3 次后停
    TEST_PHASE_RED_DELAY_ON,        // 红灯延迟 500ms 后常亮
    TEST_PHASE_ALL_OFF,             // 全灭，测试结束
    TEST_PHASE_COUNT
} ind_test_phase_t;

/* 每个阶段的 OSAL 推进间隔（ms） */
static const uint16_t TEST_PHASE_DURATION_MS = 3000;

static uint8_t ind_test_task_id = 0xFF;
static uint8_t ind_test_phase   = 0;

#define IND_TEST_EVT    0x0001

/* 设置当前阶段的灯效 */
static void apply_test_phase(uint8_t phase) {
    switch (phase) {
        case TEST_PHASE_WHITE_ON:
            indicator_set(LED_WHITE, &IND_ON);
            break;
        case TEST_PHASE_WHITE_OFF:
            indicator_off(LED_WHITE);
            break;
        case TEST_PHASE_RED_ON_2S:
            indicator_set(LED_RED, &IND_ON_2S);
            break;
        case TEST_PHASE_WHITE_BLINK_SLOW:
            indicator_set(LED_WHITE, &IND_BLINK_SLOW);
            break;
        case TEST_PHASE_RED_BLINK_FAST:
            indicator_set(LED_RED, &IND_BLINK_FAST);
            break;
        case TEST_PHASE_WHITE_BLINK_3:
            indicator_set(LED_WHITE, &IND_BLINK_3);
            break;
        case TEST_PHASE_RED_DELAY_ON:
            indicator_set(LED_RED, &IND_DELAY_ON);
            break;
        case TEST_PHASE_ALL_OFF:
            indicator_off_all();
            break;
        default:
            break;
    }
}

/* OSAL 事件处理函数：每 3s 推进一个阶段 */
static uint16_t ind_test_event_handler(uint8_t task_id, uint16_t events) {
    if (events & IND_TEST_EVT) {
        ind_test_phase++;

        if (ind_test_phase < TEST_PHASE_COUNT) {
            apply_test_phase(ind_test_phase);
            /* 继续注册下一次事件 */
            OSAL_SetEvent(ind_test_task_id, IND_TEST_EVT);
            OSAL_StartTimerEx(ind_test_task_id, IND_TEST_EVT, TEST_PHASE_DURATION_MS);
        }
        /* 阶段 ALL_OFF 之后不再注册，测试自然结束 */
        return (events ^ IND_TEST_EVT);
    }
    return 0;
}

void indicator_test(void) {
    /* 注册 OSAL 任务 */
    ind_test_task_id = OSAL_ProcessEventRegister(ind_test_event_handler);

    /* 设置阶段 0 的初始灯效 */
    ind_test_phase = TEST_PHASE_WHITE_ON;
    apply_test_phase(ind_test_phase);

    /* 3s 后推进到阶段 1 */
    OSAL_StartTimerEx(ind_test_task_id, IND_TEST_EVT, TEST_PHASE_DURATION_MS);
}

#endif /* INDICATOR_TEST_ENABLE */
```

### 3.4 在 system_init_application() 末尾调用

文件：`application/system/system_init.c`

```c
#include "indicator_test.h"

void system_init_application(void) {
    system_service_init();
    input_service_init();
    output_service_init();
    commu_service_init();

    indicator_test();   // 新增：开机后自动执行指示灯测试

    g_system_init_status = SYSTEM_INIT_STATUS_APPLICATION_INIT;
    g_system_init_status = SYSTEM_INIT_STATUS_COMPLETED;
    g_system_initialized = true;
}
```

---

## 四、测试序列与预期行为

开机后，测试序列自动执行，每 3 秒推进一个阶段，共 8 个阶段，约 24 秒完成。

| 阶段 | 开机后时间 | 操作 LED | 灯效 | 预期现象 | 通过判定 |
|------|-----------|---------|------|---------|---------|
| 0 | T=0s | 白灯 A15 | 常亮（无限） | 白灯持续亮，不闪烁 | 白灯亮且稳定 |
| 1 | T=3s | 白灯 A15 | 熄灭 | 白灯立即熄灭 | 白灯灭 |
| 2 | T=6s | 红灯 B23 | 常亮 2s 后自动灭 | 红灯亮起，约 2s 后自动熄灭，之后保持灭 | 红灯亮后自动灭，不需人工干预 |
| 3 | T=9s | 白灯 A15 | 慢闪（250ms亮+250ms灭） | 白灯以约 0.5s 周期规律闪烁，亮灭时间相等 | 肉眼可见均匀慢速闪烁 |
| 4 | T=12s | 红灯 B23 | 快闪（100ms 周期） | 红灯高频闪烁，明显快于阶段 3 | 闪烁频率远高于阶段 3，接近持续亮感 |
| 5 | T=15s | 白灯 A15 | 闪 3 次后停 | 白灯闪烁 3 下后停止，之后保持灭 | 恰好闪 3 下，不多不少 |
| 6 | T=18s | 红灯 B23 | 延迟 500ms 后常亮 | 约 0.5s 延迟后红灯亮起，之后持续亮 | 有明显停顿延迟才亮 |
| 7 | T=21s | 全部 | 批量熄灭 | 白灯、红灯同时熄灭，测试结束 | 两灯均灭 |

> **关于阶段 2 的时序说明：**
> 阶段 2（红灯亮 2s 自动灭）持续 3s 观察窗口内，2s 时红灯已自动熄灭，之后还有 1s 的空窗期。
> 若主循环负载过高导致 `indicator_task()` 调用延迟，定时精度会有轻微偏差，属已知限制，不影响功能判定。

---

## 五、失效场景排查

| 现象 | 可能原因 | 排查方法 |
|------|---------|---------|
| 灯完全不亮（所有阶段） | `indicator_init()` 未调用，或引脚配置错误 | 检查 `system_init_drivers()` 中 `indicator_init()` 是否取消注释；检查 `indicator_config.h` 引脚是否为 A15/B23 |
| 常亮（阶段 0/1）正常，闪烁不工作 | `indicator_task()` 未加入主循环 | 检查 `main.c` while(1) 中是否有 `indicator_task()` |
| 阶段不自动推进（卡在阶段 0） | OSAL 任务注册失败，或 `OSAL_SystemProcessOnce()` 未在主循环执行 | 检查 `main.c` 中 `OSAL_SystemProcessOnce()` 调用；检查 OSAL 任务数量是否超限 |
| 延迟阶段（阶段 6）灯立即亮，无延迟 | `delay_ms` 参数未生效 | 检查 `indicator.c` 中 `start_led_effect()` 对 `delay_ms > 0` 的分支逻辑 |
| 阶段 2 红灯不自动熄灭 | `duration_ms` 参数未生效，或 `indicator_task()` 未被调用 | 同"闪烁不工作"排查方向 |
| 阶段 5 闪烁次数不对 | `repeat` 计数逻辑错误 | 检查 `indicator.c` 中 `process_led_timeout()` 的 `count >= repeat` 判断 |
| 阶段 7 全灭后某灯仍亮 | `indicator_off_all()` 未覆盖所有 LED | 检查 `IND_LED_COUNT` 是否与 `ind_led_table` 条目数一致 |

---

## 六、测试完成后清理

测试验证通过后，执行以下清理步骤，避免测试代码影响生产固件：

```c
// 1. 注释或删除 sys_config.h 中的测试宏
// #define INDICATOR_TEST_ENABLE

// 2. 确认 indicator_test() 展开为空宏，不产生任何代码
```

测试文件 `indicator_test.h` / `indicator_test.c` 可保留，供后续回归验证使用。
