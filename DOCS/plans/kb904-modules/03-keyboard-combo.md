# KB904 按键与组合键设计

> 模块负责人: Agent C
> 创建日期: 2026-03-18
> 预估工时: 2.5h
> 开发顺序: 第一阶段（并行）

---

## 1. 模块职责

- 新增 Fn 组合键：Fn+右Shift/右Enter/右Cmd
- 实现背光亮度切换（4 档循环）
- 实现背光颜色切换（13 色循环）
- 实现电量检查（闪烁 1-4 次）
- 实现自定义键（唤起 Siri）
- 提供背光活动通知包装函数

---

## 2. 依赖关系

```
┌─────────────────────────────────────────┐
│            依赖的外部模块                │
├─────────────────────────────────────────┤
│  middleware/keyboard/combo/kb_combo_map.c    │  ← 修改
│  middleware/keyboard/combo/kb_fn_action.c    │  ← 修改
│  middleware/keyboard/keyboard.c              │  ← 修改
│  drivers/output/backlight/backlight.h        │  ← 现有
│  drivers/power/battery.h                     │  ← 现有
│  drivers/output/indicators/indicator.h       │  ← 现有
└─────────────────────────────────────────┘
```

---

## 3. 文件变更清单

### 3.1 修改 `middleware/keyboard/keyboard.c`

**任务**: 添加背光活动通知包装函数

```c
// 声明外部函数（避免包含 output_service.h）
extern void output_service_note_backlight_activity(void);

/**
 * @brief 通知背光服务有活动发生
 *
 * 此函数供 Middleware 层（如 kb_fn_action.c）调用，
 * 避免直接依赖 Application 层。
 */
void keyboard_note_backlight_activity(void) {
    output_service_note_backlight_activity();
}
```

### 3.2 修改 `middleware/keyboard/combo/kb_fn_action.c`

**任务**: 新增 Action 函数

```c
#include "backlight.h"
#include "keyboard.h"              // 获取包装函数
#include "battery.h"
#include "indicator.h"
#include "indicator_config.h"

/* ========== 背光控制 Action ========== */

/**
 * @brief Fn+右Shift: 切换背光亮度（4档循环）
 */
uint8_t Backlight_Level_Up(uint16_t* add_keys) {
    backlight_level_step();                   // OFF→LOW→MEDIUM→HIGH→OFF
    keyboard_note_backlight_activity();       // 重置休眠计时器
    return 0;
}

/**
 * @brief Fn+右Enter: 切换背光颜色（13色循环）
 */
uint8_t Backlight_Color_Next(uint16_t* add_keys) {
    if (backlight_is_enabled()) {
        backlight_color_step();               // 只有背光亮时才切换
    }
    keyboard_note_backlight_activity();       // 重置休眠计时器
    return 0;
}

/* ========== 电量检查 Action ========== */

/**
 * @brief Fn+右Cmd: 电量检查（闪烁1-4次）
 */
uint8_t Battery_Check(uint16_t* add_keys) {
    uint8_t percentage = battery_get_percentage();
    uint8_t blinks;

    // 计算闪烁次数
    if (percentage >= 75)      blinks = 4;
    else if (percentage >= 50) blinks = 3;
    else if (percentage >= 25) blinks = 2;
    else                       blinks = 1;

    // 触发指示灯闪烁
    ind_effect_t effect = IND_BLINK_CUSTOM(200, 200, blinks);
    indicator_set(LED_BAT, &effect);

    return 0;
}

/* ========== Siri 唤起 Action ========== */

/**
 * @brief 自定义键: 唤起 Siri (Consumer 0xCF)
 */
uint8_t Siri_Invoke(uint16_t* add_keys) {
    add_keys[0] = M_SIRI;  // HCI_CONSUMER_CODE = 0xCF
    return 1;
}
```

### 3.3 修改 `middleware/keyboard/combo/kb_combo_map.c`

**任务**: 新增组合键映射

```c
// 新增组合键枚举
enum combos {
    // ... 现有项 ...

    // KB904 新增
    FN_RIGHT_SHIFT_ID,    // Fn+右Shift: 亮度切换
    FN_RIGHT_ENTER_ID,    // Fn+右Enter: 颜色切换
    FN_RIGHT_CMD_ID,      // Fn+右Cmd: 电量检查
    CUSTOM_KEY_ID,        // 自定义键: Siri

    COMBOS_END,
};

// 组合键定义
const uint16_t PROGMEM FN_RIGHT_SHIFT_combo[] = { S_FN_KEY, KC_RIGHT_SHIFT, COMBO_END };
const uint16_t PROGMEM FN_RIGHT_ENTER_combo[] = { S_FN_KEY, KC_RIGHT_ENTER, COMBO_END };
const uint16_t PROGMEM FN_RIGHT_CMD_combo[]   = { S_FN_KEY, KC_RIGHT_GUI, COMBO_END };
const uint16_t PROGMEM CUSTOM_KEY_combo[]     = { KC_APPLICATION, COMBO_END };

// 组合键数组
combo_t key_combos[] = {
    // ... 现有项 ...

    [FN_RIGHT_SHIFT_ID] = COMBO(FN_RIGHT_SHIFT_combo, PRESS_DOWN, Backlight_Level_Up),
    [FN_RIGHT_ENTER_ID] = COMBO(FN_RIGHT_ENTER_combo, PRESS_DOWN, Backlight_Color_Next),
    [FN_RIGHT_CMD_ID]   = COMBO(FN_RIGHT_CMD_combo,   PRESS_DOWN, Battery_Check),
    [CUSTOM_KEY_ID]     = COMBO(CUSTOM_KEY_combo,     PRESS_DOWN, Siri_Invoke),
};
```

---

## 4. 组合键功能表

| 组合键 | 功能 | Action 函数 |
|--------|------|-------------|
| Fn+右Shift | 切换亮度（4档循环） | `Backlight_Level_Up()` |
| Fn+右Enter | 切换颜色（13色循环） | `Backlight_Color_Next()` |
| Fn+右Cmd | 电量检查（闪烁 1-4 次） | `Battery_Check()` |
| 自定义键 | 唤起 Siri | `Siri_Invoke()` |

---

## 5. 键码定义

### 5.1 需要确认的键码

```c
/* KB904 专用键码（需参照现有 keycode.h 确认） */
#define KC_RIGHT_SHIFT      0xE5    // 右 Shift 键位
#define KC_RIGHT_ENTER      0xE6    // 右 Enter 键位
#define KC_RIGHT_GUI        0xE7    // 右 Cmd/Win 键位
#define KC_APPLICATION      0x65    // 应用程序键（自定义键位置）

/* Consumer 键码 - Siri 唤起 */
#define M_SIRI              0x00CF  // HCI Consumer Code for Voice Command

/* Fn 功能键 */
#define S_FN_KEY            0x00F0  // Fn 键内部标识
```

---

## 6. 接口说明

### 6.1 新增对外接口

| 函数 | 说明 | 调用者 |
|------|------|--------|
| `keyboard_note_backlight_activity()` | 通知背光活动 | kb_fn_action.c |

### 6.2 依赖的现有接口

| 函数 | 模块 |
|------|------|
| `backlight_level_step()` | backlight.c |
| `backlight_color_step()` | backlight.c |
| `backlight_is_enabled()` | backlight.c |
| `battery_get_percentage()` | battery.c |
| `indicator_set()` | indicator.c |

---

## 7. 测试要点

| 测试项 | 测试方法 | 预期结果 |
|--------|----------|----------|
| 亮度切换 | 按 Fn+右Shift 4 次 | OFF→LOW→MEDIUM→HIGH→OFF |
| 颜色切换 | 按 Fn+右Enter 13 次 | 颜色顺序正确循环 |
| 颜色切换限制 | 背光关闭时按 Fn+右Enter | 颜色不变 |
| 电量检查 100% | 按 Fn+右Cmd | 红灯闪 4 次 |
| 电量检查 50% | 按 Fn+右Cmd | 红灯闪 3 次 |
| Siri 唤起 | 按自定义键 | iPad 弹出 Siri |

---

## 8. 完成标准

- [ ] `keyboard.c` 包装函数编译通过
- [ ] `kb_fn_action.c` 新增 Action 编译通过
- [ ] `kb_combo_map.c` 组合键映射正确
- [ ] Fn+右Shift 亮度切换正常
- [ ] Fn+右Enter 颜色切换正常
- [ ] Fn+右Cmd 电量检查正常
- [ ] 自定义键唤起 Siri 正常

---

*文档结束*
