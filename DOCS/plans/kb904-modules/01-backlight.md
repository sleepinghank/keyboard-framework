# KB904 背光模块设计

> 模块负责人: Agent A
> 创建日期: 2026-03-18
> 预估工时: 2h
> 开发顺序: 第一阶段（并行）

---

## 1. 模块职责

- 扩展背光颜色从 7 色到 13 色
- 实现背光 5 秒无操作自动熄灭（OSAL 定时事件）
- 实现按键/触控时自动唤醒背光
- 低电量时强制关闭背光

---

## 2. 依赖关系

```
┌─────────────────────────────────────────┐
│            依赖的外部模块                │
├─────────────────────────────────────────┤
│  driver/output/backlight/backlight.c    │  ← 现有
│  driver/output/backlight/backlight.h    │  ← 修改
│  application/service/output_service.c   │  ← 修改
│  application/service/output_service.h   │  ← 修改
│  OSAL (OSAL_StartTimer, OSAL_SetEvent)  │  ← 系统层
└─────────────────────────────────────────┘
```

---

## 3. 文件变更清单

### 3.1 修改 `drivers/output/backlight/backlight.h`

**任务**: 扩展颜色枚举从 7 色到 13 色

```c
/**
 * @brief 预设颜色枚举（13色）
 *
 * 顺序：红-暗红-粉红-紫-靛青-深蓝-蓝-青蓝-青绿-绿-淡黄-橙-白
 */
typedef enum {
    BL_COLOR_RED = 0,       // 红
    BL_COLOR_DARK_RED,      // 暗红 - 新增
    BL_COLOR_PINK,          // 粉红 - 新增
    BL_COLOR_PURPLE,        // 紫 - 新增
    BL_COLOR_INDIGO,        // 靛青 - 新增
    BL_COLOR_DARK_BLUE,     // 深蓝 - 新增
    BL_COLOR_BLUE,          // 蓝
    BL_COLOR_CYAN_BLUE,     // 青蓝 - 新增
    BL_COLOR_CYAN_GREEN,    // 青绿 - 新增
    BL_COLOR_GREEN,         // 绿
    BL_COLOR_LIGHT_YELLOW,  // 淡黄 - 新增
    BL_COLOR_ORANGE,        // 橙 - 新增
    BL_COLOR_WHITE,         // 白
    BL_COLOR_COUNT          // = 13
} bl_preset_color_t;

/* 兼容性别名 - 确保现有代码兼容 */
#define BL_COLOR_YELLOW         BL_COLOR_LIGHT_YELLOW
#define BL_COLOR_CYAN           BL_COLOR_CYAN_GREEN
#define BL_COLOR_MAGENTA        BL_COLOR_PURPLE
```

### 3.2 修改 `drivers/output/backlight/backlight.c`

**任务**: 扩展颜色预设表

```c
static const bl_rgb_t preset_colors[BL_COLOR_COUNT] = {
    [BL_COLOR_RED]        = {100,   0,   0},  // 红
    [BL_COLOR_DARK_RED]   = { 50,   0,   0},  // 暗红
    [BL_COLOR_PINK]       = {100,  50,  80},  // 粉红
    [BL_COLOR_PURPLE]     = { 60,   0, 100},  // 紫
    [BL_COLOR_INDIGO]     = { 30,   0, 100},  // 靛青
    [BL_COLOR_DARK_BLUE]  = {  0,   0,  80},  // 深蓝
    [BL_COLOR_BLUE]       = {  0,   0, 100},  // 蓝
    [BL_COLOR_CYAN_BLUE]  = {  0,  50, 100},  // 青蓝
    [BL_COLOR_CYAN_GREEN] = {  0, 100, 100},  // 青绿
    [BL_COLOR_GREEN]      = {  0, 100,   0},  // 绿
    [BL_COLOR_LIGHT_YELLOW]= { 80,  80,   0},  // 淡黄
    [BL_COLOR_ORANGE]     = {100,  50,   0},  // 橙
    [BL_COLOR_WHITE]      = {100, 100, 100},  // 白
};
```

### 3.3 修改 `application/service/output_service.h`

**任务**: 添加背光休眠 OSAL 事件和接口

```c
/* ========== 背光休眠配置 ========== */
#ifndef BACKLIGHT_IDLE_TIMEOUT_MS
#define BACKLIGHT_IDLE_TIMEOUT_MS   5000   // 5秒无操作熄灭
#endif

/* ========== OSAL 事件定义（新增） ========== */

#define OUTPUT_BACKLIGHT_SLEEP_EVT   (1 << 10)  /**< 背光休眠定时事件 */

/* ========== 背光休眠接口 ========== */

/**
 * @brief 通知背光有活动（按键/触控时调用）
 * @note 内部会重启 5 秒 OSAL 定时器
 */
void output_service_note_backlight_activity(void);

/**
 * @brief 设置低电量模式（控制背光）
 * @param low true=低电量，强制关背光
 */
void output_service_set_backlight_low_battery(bool low);
```

### 3.4 修改 `application/service/output_service.c`

**任务**: 使用 OSAL 定时事件实现背光休眠

```c
/* ========== 外部变量 ========== */
extern uint8_t output_taskID;

/* ========== 背光休眠状态 ========== */
static bool bl_low_battery_mode = false;
static bool bl_sleep_active = false;

/**
 * @brief 通知背光有活动（按键/触控时调用）
 *
 * 实现方式：
 * 1. 取消当前的休眠定时器
 * 2. 重新启动 5 秒休眠定时器
 * 3. 如果背光关闭且不是低电量模式，重新点亮
 */
void output_service_note_backlight_activity(void) {
    // 取消当前定时器（如果存在）
    OSAL_ClearEvent(output_taskID, OUTPUT_BACKLIGHT_SLEEP_EVT);

    // 重新启动 5 秒休眠定时器
    (output_taskID, OUTPUT_BACKLIGHT_SLEEP_EVT,
                    BACKLIGHT_IDLE_TIMEOUT_MS);

    // 如果背光关闭且不是低电量模式，重新点亮
    if (bl_sleep_active && !bl_low_battery_mode) {
        backlight_enable();
        bl_sleep_active = false;
    }
}

/**
 * @brief 设置低电量模式
 */
void output_service_set_backlight_low_battery(bool low) {
    bl_low_battery_mode = low;

    if (low) {
        // 低电量：取消定时器，关闭背光
        OSAL_ClearEvent(output_taskID, OUTPUT_BACKLIGHT_SLEEP_EVT);
        backlight_disable();
        bl_sleep_active = true;
    } else {
        // 电量恢复：如果有背光活动，重新启动定时器
        if (backlight_is_enabled()) {
            OSAL_StartTimer(output_taskID, OUTPUT_BACKLIGHT_SLEEP_EVT,
                            BACKLIGHT_IDLE_TIMEOUT_MS);
        }
        bl_sleep_active = false;
    }
}

/**
 * @brief 初始化背光休眠服务
 * @note 在 output_service_init() 中调用
 */
static void backlight_sleep_init(void) {
    bl_low_battery_mode = false;
    bl_sleep_active = false;

    // 启动初始 5 秒休眠定时器
    OSAL_StartTimer(output_taskID, OUTPUT_BACKLIGHT_SLEEP_EVT,
                    BACKLIGHT_IDLE_TIMEOUT_MS);
}

// 在 output_process_event 中处理休眠事件
uint16_t output_process_event(uint8_t task_id, uint16_t events) {
    // ... 现有事件处理 ...

    // ========== 背光休眠定时事件 ==========
    if (events & OUTPUT_BACKLIGHT_SLEEP_EVT) {
        // 5 秒超时，关闭背光
        if (!bl_low_battery_mode && backlight_is_enabled()) {
            backlight_disable();
            bl_sleep_active = true;
        }
        return (events ^ OUTPUT_BACKLIGHT_SLEEP_EVT);
    }

    // ... 其他事件 ...
}
```

---

## 4. OSAL 事件调度流程

```
┌──────────────────────────────────────────────────────────────┐
│               OSAL 背光休眠事件调度流程                       │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│  系统初始化                                                   │
│       │                                                      │
│       ▼                                                      │
│  OSAL_StartTimer(OUTPUT_BACKLIGHT_SLEEP_EVT, 5000ms)         │
│       │                                                      │
│       │  ┌─────────────────────────────────────────┐        │
│       │  │          等待期间用户按键                │        │
│       │  └─────────────────────────────────────────┘        │
│       │                         │                           │
│       │                         ▼                           │
│       │    output_service_note_backlight_activity()         │
│       │                         │                           │
│       │            ┌────────────┴────────────┐              │
│       │            ▼                         ▼              │
│       │   OSAL_ClearEvent()          如休眠中则唤醒          │
│       │            │                                          │
│       │            ▼                                          │
│       │   OSAL_StartTimer(5000ms)  ← 重置定时器             │
│       │                                                        │
│       │  ┌─────────────────────────────────────────┐        │
│       │  │        5 秒内无活动                       │        │
│       │  └─────────────────────────────────────────┘        │
│       │                         │                           │
│       ▼                         ▼                           │
│  OSAL 触发 OUTPUT_BACKLIGHT_SLEEP_EVT                       │
│       │                                                      │
│       ▼                                                      │
│  backlight_disable()                                         │
│  bl_sleep_active = true                                      │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

---

## 5. 接口说明

### 5.1 对外接口

| 函数 | 说明 | 调用者 |
|------|------|--------|
| `output_service_note_backlight_activity()` | 通知背光活动，重启 OSAL 定时器 | keyboard.c |
| `output_service_set_backlight_low_battery(bool)` | 设置低电量模式 | output_service 内部 |

### 5.2 OSAL API 使用

| 函数 | 说明 |
|------|------|
| `OSAL_StartTimer(taskID, event, timeout_ms)` | 启动一次性定时器 |
| `OSAL_ClearEvent(taskID, event)` | 取消定时器事件 |

### 5.3 调用时机

```
按键事件 ──▶ keyboard.c ──▶ keyboard_note_backlight_activity()
                                       │
                                       ▼
                    output_service_note_backlight_activity()
                                       │
                       ┌───────────────┴───────────────┐
                       ▼                               ▼
              OSAL_ClearEvent()               如休眠中则唤醒
                       │
                       ▼
              OSAL_StartTimer(5000ms)
```

---

## 6. 测试要点

| 测试项 | 测试方法 | 预期结果 |
|--------|----------|----------|
| 13 色循环 | 按 Fn+右Enter 13 次 | 颜色顺序正确：红→暗红→...→白→红 |
| 5 秒休眠 | 点亮背光后等待 | 恰好 5 秒后背光熄灭（OSAL 精确） |
| 按键唤醒 | 背光休眠后按任意键 | 背光立即点亮，定时器重置 |
| 连续按键 | 每 3 秒按一次键 | 背光持续不灭 |
| 低电量关背光 | 模拟电量≤20% | 背光强制关闭，定时器取消 |
| 颜色记忆 | 切换颜色后休眠再唤醒 | 颜色保持不变 |

---

## 7. 完成标准

- [ ] `backlight.h` 13 色枚举编译通过
- [ ] `backlight.c` 13 色循环正确
- [ ] OSAL 定时事件 `OUTPUT_BACKLIGHT_SLEEP_EVT` 正常触发
- [ ] 5 秒背光休眠精确（误差 <100ms）
- [ ] 按键唤醒背光延迟 <50ms
- [ ] 低电量时背光强制关闭，定时器取消

---

*文档结束*
