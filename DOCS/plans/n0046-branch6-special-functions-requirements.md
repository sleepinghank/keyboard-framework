# N0046 分支六：特殊功能需求

> 分支名: `feature/n0046-special-functions`
> 依赖: `feature/n0046-product-config`, `feature/n0046-bluetooth-connection`, `feature/n0046-indicator-system`, `feature/n0046-backlight-system`
> 优先级: P0/P1 混合

---

## 1. 目标

实现 N0046 键盘的特殊功能，包括 Fn 组合键、恢复出厂设置、电量查询和多媒体键。

---

## 2. Fn 组合键功能

### 2.1 组合键列表

| 组合键 | 功能 | 优先级 |
|--------|------|--------|
| Fn + Del 长按3s | 蓝牙配对 | P0 |
| Fn + 右Enter | 背光颜色切换 | P1 |
| Fn + 右Shift | 背光亮度切换 | P1 |
| Fn + 右Cmd | 电量查询 | P1 |

### 2.2 实现要点

**Fn+Del 长按检测:**
- 需要检测同时按下 Fn 和 Del
- 长按 3 秒触发配对
- 3 秒内释放则发送正常的 Del 键

**右键识别:**
- 右Enter、右Shift、右Cmd 需要正确识别
- 通过矩阵位置区分左右修饰键

---

## 3. 恢复出厂设置

### 3.1 触发方式

**组合键:** ESC + = 同时长按 3 秒

### 3.2 执行流程

```
1. 检测 ESC 和 = 同时按下
2. 计时 3 秒
3. 触发恢复出厂
   ├── 清除蓝牙配对记录
   ├── 重置背光为默认值（白光2档）
   ├── 所有指示灯闪烁 3 次
   └── 进入配对模式
```

### 3.3 实现要点

```c
#define FACTORY_RESET_HOLD_TIME 3000    // 3秒
#define FACTORY_RESET_KEY_COUNT 2       // ESC + =

static const uint16_t factory_reset_keys[2] = { KC_ESC, KC_EQL };
static bool factory_reset_pressed[2] = { false, false };
static uint32_t factory_reset_timer = 0;

// 检测两个键同时按下
static bool check_factory_reset_keys(void) {
    return factory_reset_pressed[0] && factory_reset_pressed[1];
}

// 执行恢复出厂
static void do_factory_reset(void) {
    // 1. 清除蓝牙配对
    wireless_disconnect();

    // 2. 重置背光
    n0046_backlight_init();

    // 3. 灯效
    n0046_indicator_factory_reset();

    // 4. 进入配对模式
    wireless_pairing();
}
```

---

## 4. 电量查询

### 4.1 触发方式

**组合键:** Fn + 右Cmd

### 4.2 执行流程

```
1. 检测 Fn + 右Cmd 按下
2. 获取当前电量百分比
3. 计算闪烁次数（25%为一档）
4. 红灯闪烁 N 次
```

### 4.3 闪烁次数计算

| 电量范围 | 闪烁次数 |
|----------|----------|
| 0-25% | 1 次 |
| 26-50% | 2 次 |
| 51-75% | 3 次 |
| 76-100% | 4 次 |

```c
void n0046_indicator_battery_query(void) {
    uint8_t percentage = battery_get_percentage();
    uint8_t blinks = (percentage + 24) / 25;  // 向上取整
    if (blinks > 4) blinks = 4;
    if (blinks < 1) blinks = 1;

    // 设置闪烁效果
    ind_effect_t effect = {
        .mode = IND_MODE_BLINK,
        .on_ms = 200,
        .off_ms = 200,
        .repeat = blinks
    };
    indicator_set(IND_LED_POWER, &effect);
}
```

---

## 5. 多媒体键功能

### 5.1 F1-F12 映射

| 按键 | Fn 层功能 | HID Usage |
|------|-----------|-----------|
| F1 | ESC | Escape |
| F2 | 屏幕亮度减 | Brightness Down |
| F3 | 屏幕亮度加 | Brightness Up |
| F4 | 搜索 | Find |
| F5 | 截屏 | Print Screen |
| F6 | Siri | Voice Command |
| F7 | 上一首 | Scan Previous Track |
| F8 | 播放/暂停 | Play/Pause |
| F9 | 下一首 | Scan Next Track |
| F10 | 静音 | Mute |
| F11 | 音量减 | Volume Down |
| F12 | 音量加 | Volume Up |

### 5.2 键码定义

```c
// 标准 HID 键码
#define KC_BRID    0x6F    // Brightness Down
#define KC_BRIU    0x70    // Brightness Up
#define KC_MPRV    0x6B    // Scan Previous
#define KC_MNXT    0x6C    // Scan Next
#define KC_MPLY    0x6D    // Play/Pause
#define KC_MUTE    0x6A    // Mute
#define KC_VOLD    0x6E    // Volume Down
#define KC_VOLU    0x68    // Volume Up
#define KC_PSCR    0x46    // Print Screen
#define KC_FIND    0x7A    // Find

// 自定义键码
#define KC_SIRI    SAFE_RANGE + 1    // Voice Command
```

### 5.3 F6 Siri 功能

**本次实现:** 仅默认 Siri 功能
**后续实现:** 自定义功能（随 APP）

```c
// F6 触发 Siri
bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (keycode == KC_SIRI && record->event.pressed) {
        // 发送 Voice Command 消费键
        register_code16(HID_USAGE_VOICE_COMMAND);
        unregister_code16(HID_USAGE_VOICE_COMMAND);
        return false;
    }
    return true;
}
```

---

## 6. 语种特殊组合键

**说明:** 本次优先实现 US 布局，以下功能供参考

### 6.1 DE（德语）

| 组合键 | 输出 |
|--------|------|
| Fn + Y | < |
| Fn + X | > |
| Opt + E | € |
| Opt + L | @ |

### 6.2 UK（英式英语）

| 组合键 | 输出 |
|--------|------|
| Fn + Z | \ |
| Fn + X | \| |
| Opt + 2 | € |
| Opt + 3 | # |

### 6.3 FR（法语）

| 组合键 | 输出 |
|--------|------|
| Fn + W | < |
| Fn + X | > |

---

## 7. 需要修改的文件

| 文件 | 修改内容 |
|------|----------|
| `keyboards/n0046/n0046.c` | 组合键处理、恢复出厂、电量查询 |
| `keyboards/n0046/keymaps/default.c` | Fn 层多媒体键映射 |

---

## 8. 验收标准

### 8.1 P0 功能

- [ ] Fn+Del 长按3秒进入配对模式
- [ ] ESC+= 长按3秒恢复出厂设置
- [ ] 恢复出厂后清除配对记录
- [ ] 恢复出厂后背光重置为默认值
- [ ] 恢复出厂后指示灯闪烁3次
- [ ] F1-F12 多媒体键正确输出

### 8.2 P1 功能

- [ ] Fn+右Enter 切换背光颜色
- [ ] Fn+右Shift 切换背光亮度
- [ ] Fn+右Cmd 电量查询灯效
- [ ] F6 触发 Siri

---

## 9. 测试方法

### 9.1 组合键测试

1. 按 Fn+Del，松开 → 应发送 Del
2. 长按 Fn+Del 3秒 → 应进入配对模式
3. 同时按 ESC+=，长按3秒 → 应恢复出厂

### 9.2 电量查询测试

1. 按 Fn+右Cmd → 观察红灯闪烁次数
2. 对比实际电量验证闪烁次数正确

### 9.3 多媒体键测试

1. 连接 iPad
2. 按 Fn+F2/F3 → 屏幕亮度变化
3. 按 Fn+F7/F8/F9 → 媒体控制
4. 按 Fn+F10/F11/F12 → 音量控制
5. 按 Fn+F6 → Siri 唤起

---

## 10. 参考文件

- `middleware/keyboard/action.c` - 按键动作处理
- `middleware/keyboard/keycode.h` - 键码定义
- `docs/knowledge/N0046_功能清单.md` - 第3章组合键功能
