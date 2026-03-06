# N0046 分支五：背光系统需求

> 分支名: `feature/n0046-backlight-system`
> 依赖: `feature/n0046-product-config`
> 优先级: P1 - 重要功能

---

## 1. 目标

实现 N0046 键盘的 RGB 七彩背光功能，包括颜色切换、亮度调节、自动休眠和低电量保护。

---

## 2. 硬件配置

| 功能 | GPIO | 说明 |
|------|------|------|
| 背光电源使能 | PB22 | 高电平有效 |
| 红色 R | PB0 | PWM 输出 |
| 绿色 G | PA12 | PWM 输出 |
| 蓝色 B | PA13 | PWM 输出 |

**电平逻辑:** 高电平点亮，PWM 调节亮度

---

## 3. 功能需求

### 3.1 背光颜色切换

**触发:** Fn + 右Enter

**颜色循环:**
```
红 → 黄 → 青 → 蓝 → 绿 → 白 → 洋红 → 红...
```

**颜色定义:**

| 颜色 | R | G | B |
|------|---|---|---|
| 红 | 100 | 0 | 0 |
| 黄 | 100 | 100 | 0 |
| 青 | 0 | 100 | 100 |
| 蓝 | 0 | 0 | 100 |
| 绿 | 0 | 100 | 0 |
| 白 | 100 | 100 | 100 |
| 洋红 | 100 | 0 | 100 |

### 3.2 背光亮度调节

**触发:** Fn + 右Shift

**亮度循环:**
```
关 → 1档 → 2档 → 3档 → 关...
```

**亮度档位:**

| 档位 | 亮度值 | 说明 |
|------|--------|------|
| 关 | 0% | 背光关闭 |
| 1档 | 33% | 低亮度 |
| 2档 | 66% | 中亮度（默认） |
| 3档 | 100% | 高亮度 |

### 3.3 默认值

**开机默认:**
- 颜色: 白光
- 亮度: 2档 (66%)

### 3.4 背光休眠

**触发条件:** 5 秒无按键操作

**行为:**
- 背光自动熄灭
- 任意按键唤醒背光
- 唤醒后恢复之前的颜色和亮度

**休眠计时重置:**
- 按键按下时重置计时器
- 背光操作（颜色/亮度切换）时重置计时器

### 3.5 背光记忆

**要求:**
- 关机/休眠后保持上次设置
- 下次开机恢复上次的颜色和亮度
- 默认值仅用于首次使用或恢复出厂

### 3.6 低电量保护

**触发条件:** 电量 < 20%

**行为:**
- 强制关闭背光
- 按键不唤醒背光
- 充电后电量恢复，背光功能恢复

**优先级:** 低电量保护 > 用户设置
- 低电量时忽略所有背光操作
- 电量恢复后恢复用户设置

---

## 4. 接口设计

### 4.1 枚举定义

```c
// 预设颜色
typedef enum {
    BL_COLOR_RED,
    BL_COLOR_YELLOW,
    BL_COLOR_CYAN,
    BL_COLOR_BLUE,
    BL_COLOR_GREEN,
    BL_COLOR_WHITE,
    BL_COLOR_MAGENTA,
    BL_COLOR_COUNT
} bl_preset_color_t;

// 预设亮度档位
typedef enum {
    BL_LEVEL_OFF,
    BL_LEVEL_LOW,
    BL_LEVEL_MEDIUM,
    BL_LEVEL_HIGH,
    BL_LEVEL_COUNT
} bl_preset_level_t;
```

### 4.2 业务接口

```c
// 初始化（恢复默认值）
void n0046_backlight_init(void);

// 应用当前设置
void n0046_backlight_apply(void);

// 颜色切换（循环到下一个颜色）
void n0046_backlight_color_step(void);

// 亮度切换（循环到下一个档位）
void n0046_backlight_level_step(void);

// 唤醒背光（按键时调用）
void n0046_backlight_wake(void);

// 低电量处理
void n0046_backlight_low_battery(bool low);

// 主循环任务（检查休眠、低电量）
void n0046_backlight_task(void);

// 按键回调（重置休眠计时器）
void n0046_backlight_on_key(void);
```

---

## 5. 状态机

### 5.1 背光状态

```
┌─────────────┐
│  背光开启   │
└──────┬──────┘
       │ 5秒无操作
       ▼
┌─────────────┐  按任意键（电量>20%）
│  背光关闭   │─────────────────────────►┌─────────────┐
└─────────────┘                          │  背光开启   │
       ▲                                 └─────────────┘
       │ 电量<20%（强制关闭）
       │
```

### 5.2 状态变量

```c
static bl_preset_color_t current_color = BL_COLOR_WHITE;
static bl_preset_level_t current_level = BL_LEVEL_MEDIUM;
static bool backlight_enabled = true;
static bool low_battery_disabled = false;
static uint32_t sleep_timer = 0;
```

---

## 6. 复用模块

以下模块无需修改，直接复用：

| 模块 | 文件 | 功能 |
|------|------|------|
| 背光驱动 | `drivers/output/backlight/backlight.c` | RGB 控制 |
| HAL 抽象 | `drivers/output/backlight/backlight_hal.c` | GPIO/PWM 操作 |

---

## 7. 需要创建的文件

| 文件 | 说明 |
|------|------|
| `keyboards/n0046/n0046_backlight.c` | 背光业务逻辑 |
| `keyboards/n0046/n0046_backlight.h` | 接口定义 |

---

## 8. 与其他模块的集成

### 8.1 与键盘模块集成

```c
bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    // 按键唤醒背光
    if (record->event.pressed) {
        n0046_backlight_on_key();
    }

    // Fn+右Enter 切换颜色
    if (keycode == BL_STEP && record->event.pressed) {
        n0046_backlight_color_step();
        return false;
    }

    // Fn+右Shift 切换亮度
    if (keycode == BL_UP && record->event.pressed) {
        n0046_backlight_level_step();
        return false;
    }

    return true;
}
```

### 8.2 与电池模块集成

```c
void n0046_backlight_task(void) {
    // 检查低电量
    if (battery_get_percentage() < 20) {
        n0046_backlight_low_battery(true);
    } else if (low_battery_disabled) {
        n0046_backlight_low_battery(false);
    }

    // 检查休眠超时
    ...
}
```

---

## 9. 验收标准

- [ ] Fn+右Enter 循环切换 7 种颜色
- [ ] Fn+右Shift 循环切换 4 档亮度
- [ ] 开机默认白光 2 档
- [ ] 5 秒无操作背光熄灭
- [ ] 按键唤醒背光
- [ ] 颜色/亮度记忆功能
- [ ] 低电量 (<20%) 强制关闭背光
- [ ] 低电量时按键不唤醒背光
- [ ] 充电后背光功能恢复

---

## 10. 测试方法

1. 开机观察背光（应为白光 2 档）
2. 按 Fn+右Enter，观察颜色循环
3. 按 Fn+右Shift，观察亮度变化
4. 不操作 5 秒，观察背光熄灭
5. 按任意键，观察背光唤醒
6. 模拟低电量，观察背光强制关闭
7. 按键验证低电量时不唤醒

---

## 11. 参考文件

- `drivers/output/backlight/backlight.c` - 背光驱动
- `drivers/output/backlight/backlight.h` - 接口定义
- `docs/knowledge/N0046_功能清单.md` - 第4章背光模块
