# KB904 恢复出厂设置设计

> 模块负责人: Agent E
> 创建日期: 2026-03-18
> 预估工时: 1.5h
> 开发顺序: 第三阶段

---

## 1. 模块职责

- 长按 ESC+= 3 秒触发恢复出厂设置
- 指示灯闪烁 3 次反馈
- 清除蓝牙配对记录
- 重置背光为默认值（白光 2 档）
- 重启系统

---

## 2. 依赖关系

```
┌─────────────────────────────────────────┐
│            依赖的外部模块                │
├─────────────────────────────────────────┤
│  middleware/keyboard/combo/kb_fn_action.c    │  ← 修改
│  middleware/keyboard/combo/kb_combo_map.c    │  ← 修改
│  drivers/communication/bt_driver.c           │  ← 现有
│  drivers/storage/storage.c                   │  ← 现有
│  drivers/output/backlight/backlight.c        │  ← 现有
│  drivers/output/indicators/indicator.c       │  ← 现有
│  hal/platforms/ch584/_system.c               │  ← 现有
└─────────────────────────────────────────┘
```

**依赖说明**:
- 依赖所有前置模块完成
- 需要确认 `bt_driver_clear_bonding()` 接口存在
- 需要确认 `system_restart()` 接口存在

---

## 3. 文件变更清单

### 3.1 修改 `middleware/keyboard/combo/kb_fn_action.c`

**任务**: 新增恢复出厂设置 Action

```c
#include "indicator.h"
#include "indicator_config.h"
#include "backlight.h"
#include "bt_driver.h"
#include "storage.h"
#include "CH585SFR.h"

extern void wait_ms(uint32_t ms);  // 系统延时

/**
 * @brief ESC+= 长按3秒: 恢复出厂设置
 *
 * 触发后依次执行：
 * 1. 指示灯闪烁 3 次
 * 2. 清除蓝牙配对记录
 * 3. 重置存储为默认值
 * 4. 重置背光（白光 2 档）
 * 5. 延迟后重启
 */
uint8_t Factory_Reset(uint16_t* add_keys) {
    (void)add_keys;

    // 1. 指示灯反馈（白灯和蓝灯同时闪烁 3 次）
    ind_effect_t blink_3 = IND_BLINK_CUSTOM(200, 200, 3);
    indicator_set(LED_CAPS, &blink_3);
    indicator_set(LED_BT, &blink_3);

    // 2. 清除蓝牙配对记录
    bt_driver_clear_bonding();

    // 3. 重置存储为默认值
    storage_factory_reset();

    // 4. 重置背光（白光 2 档）
    backlight_set_preset_level(BL_LEVEL_MEDIUM);
    backlight_set_preset_color(BL_COLOR_WHITE);

    // 5. 延迟后重启
    wait_ms(500);
    system_restart();

    return 0;
}
```

### 3.2 修改 `middleware/keyboard/combo/kb_combo_map.c`

**任务**: 新增长按组合键映射

```c
// 新增组合键枚举
enum combos {
    // ... 现有项 ...

    // KB904 恢复出厂
    ESC_EQUAL_ID,    // ESC+= 长按3秒

    COMBOS_END,
};

// 组合键定义（长按）
const uint16_t PROGMEM ESC_EQUAL_combo[] = { KC_ESCAPE, KC_EQUAL, COMBO_END };

// 组合键数组
combo_t key_combos[] = {
    // ... 现有项 ...

    // 长按 3 秒（300 ticks = 3000ms，假设 1 tick = 10ms）
    [ESC_EQUAL_ID] = COMBO_LONG_TICKS(ESC_EQUAL_combo, 300, LONG_PRESS_START, Factory_Reset),
};
```

---

## 4. 恢复出厂流程

```
┌──────────────────────────────────────────────────────────────┐
│                     恢复出厂设置流程                          │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│  用户长按 ESC+= 3 秒                                         │
│       │                                                      │
│       ▼                                                      │
│  combo_engine 检测到长按                                     │
│       │                                                      │
│       ▼                                                      │
│  Factory_Reset() 被调用                                      │
│       │                                                      │
│       ├─────────────────────────────────────────┐            │
│       │                                         │            │
│       ▼                                         ▼            │
│  指示灯闪烁 3 次                         bt_driver_clear_bonding()│
│  (LED_CAPS + LED_BT)                                │            │
│       │                                         │            │
│       ▼                                         ▼            │
│  storage_factory_reset()               背光重置为白光 2 档   │
│       │                                         │            │
│       └─────────────────────────────────────────┘            │
│                          │                                   │
│                          ▼                                   │
│                   wait_ms(500)                               │
│                          │                                   │
│                          ▼                                   │
│                   system_restart()                           │
│                          │                                   │
│                          ▼                                   │
│                   系统重启，重新连接 iPad                     │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

---

## 5. 接口说明

### 5.1 依赖的现有接口

| 函数 | 模块 | 说明 |
|------|------|------|
| `bt_driver_clear_bonding()` | bt_driver.c | 清除蓝牙配对记录 |
| `storage_factory_reset()` | storage.c | 重置存储为默认值 |
| `backlight_set_preset_level()` | backlight.c | 设置亮度档位 |
| `backlight_set_preset_color()` | backlight.c | 设置颜色 |
| `indicator_set()` | indicator.c | 设置灯效 |
| `wait_ms()` | system | 毫秒延时 |
| `system_restart()` | _system.c | 系统重启 |

### 5.2 需要确认的接口

> **注意**: 实现前需确认以下接口存在且可用

```c
// drivers/communication/bt_driver.h
void bt_driver_clear_bonding(void);

// hal/platforms/ch584/_system.h
void system_restart(void);
void wait_ms(uint32_t ms);
```

---

## 6. 测试要点

| 测试项 | 测试方法 | 预期结果 |
|--------|----------|----------|
| 长按识别 | 长按 ESC+= 2 秒 | 不触发（需 3 秒） |
| 长按触发 | 长按 ESC+= 3 秒 | 指示灯闪烁 3 次，系统重启 |
| 配对记录清除 | 恢复出厂后检查 | iPad 显示设备未配对 |
| 背光重置 | 恢复出厂后检查 | 白光 2 档亮度 |
| 重新配对 | 恢复出厂后配对 | 正常连接 |

---

## 7. 完成标准

- [ ] `kb_fn_action.c` Factory_Reset 函数编译通过
- [ ] `kb_combo_map.c` 长按组合键映射正确
- [ ] 长按 3 秒触发恢复出厂
- [ ] 指示灯闪烁 3 次反馈
- [ ] 蓝牙配对记录清除
- [ ] 背光重置为白光 2 档
- [ ] 系统重启正常

---

## 8. 注意事项

1. **Combo 长按时间单位**: 确认 `COMBO_LONG_TICKS` 的 tick 单位
   - 如果 1 tick = 10ms，则 300 ticks = 3 秒
   - 如果 1 tick = 1ms，则 3000 ticks = 3 秒

2. **接口存在性**: 实现前需确认 `bt_driver_clear_bonding()` 和 `system_restart()` 接口

3. **电量状态**: 恢复出厂后电量检测应正常工作

---

*文档结束*
