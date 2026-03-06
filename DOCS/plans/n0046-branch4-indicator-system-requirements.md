# N0046 分支四：指示灯系统需求

> 分支名: `feature/n0046-indicator-system`
> 依赖: `feature/n0046-product-config`, `feature/n0046-bluetooth-connection`
> 优先级: P0 - 核心功能

---

## 1. 目标

实现 N0046 键盘的指示灯业务逻辑，包括大写锁定、蓝牙状态、充电状态、低电量警告和电量查询。

---

## 2. 硬件配置

| LED | GPIO | 功能 |
|-----|------|------|
| 白灯 | PA15 | 大写锁定 / 蓝牙状态 |
| 红灯 | PB23 | 充电 / 低电量 / 电量查询 |
| 绿灯 | 充电 IC 控制 | 充满指示（硬件控制，无需软件） |

**电平逻辑:** 高电平点亮

---

## 3. 灯效定义

| 灯效名称 | 参数 | 用途 |
|----------|------|------|
| 常亮 | 持续点亮 | 状态确认 |
| 慢闪 | 250ms ON / 250ms OFF | 回连状态 |
| 快闪 | 100ms ON / 100ms OFF | 配对状态 |
| N次闪烁 | 200ms ON / 200ms OFF, 重复N次 | 电量查询 |

---

## 4. 功能需求

### 4.1 大写锁定指示（白灯）

| 条件 | 白灯行为 |
|------|----------|
| Caps Lock 开启 | 常亮 |
| Caps Lock 关闭 | 熄灭 |

**注意:** 白灯与大写键位置相同（Caps Lock 键上）

### 4.2 蓝牙状态指示（白灯）

| 蓝牙状态 | 白灯行为 | 说明 |
|----------|----------|------|
| WT_PARING | 快闪 (3次/秒) | 配对模式 |
| WT_RECONNECTING | 慢闪 (1次/秒) | 回连模式 |
| WT_CONNECTED | 熄灭 | 已连接 |
| WT_DISCONNECTED | 慢闪 | 等待连接 |

**优先级:** 大写锁定 > 蓝牙状态
- 当大写锁定开启时，白灯常亮（不显示蓝牙状态）
- 当大写锁定关闭时，白灯显示蓝牙状态

### 4.3 充电状态指示（红灯）

| 状态 | 红灯行为 |
|------|----------|
| 充电中 | 常亮 |
| 充满 | 熄灭（绿灯由硬件控制亮起） |
| 未充电 | 熄灭 |

**注意:** 需要检测充电 IC 的 CHRG 和 STDBY 信号

### 4.4 低电量警告（红灯）

| 条件 | 红灯行为 |
|------|----------|
| 电量 < 20% | 慢闪 |
| 电量 ≥ 20% | 熄灭 |

**优先级:** 充电状态 > 低电量警告
- 充电中时显示充电状态，不显示低电量警告

### 4.5 电量查询（红灯）

**触发:** Fn + 右Cmd

**灯效:**
- 红灯闪烁 N 次
- N = 电量档位（25%、50%、75%、100% → 1、2、3、4 次）

| 电量范围 | 闪烁次数 |
|----------|----------|
| 0-25% | 1 次 |
| 26-50% | 2 次 |
| 51-75% | 3 次 |
| 76-100% | 4 次 |

**参数:**
- ON 时间: 200ms
- OFF 时间: 200ms
- 闪烁完成后自动熄灭

### 4.6 恢复出厂指示（所有灯）

**触发:** ESC + = 长按 3 秒

**灯效:**
- 白灯和红灯同时闪烁 3 次
- 闪烁完成后进入配对模式

---

## 5. 接口设计

### 5.1 指示灯 ID 定义

```c
#define IND_LED_CAPS    0    // 白灯
#define IND_LED_POWER   1    // 红灯
#define IND_LED_COUNT   2
```

### 5.2 业务接口

```c
// 大写锁定指示
void n0046_indicator_caps_lock(bool enabled);

// 充电状态指示
void n0046_indicator_charging(bool charging);

// 低电量警告
void n0046_indicator_low_battery(bool low);

// 电量查询灯效
void n0046_indicator_battery_query(void);

// 恢复出厂灯效
void n0046_indicator_factory_reset(void);
```

---

## 6. 复用模块

以下模块无需修改，直接复用：

| 模块 | 文件 | 功能 |
|------|------|------|
| 指示灯驱动 | `drivers/output/indicators/indicator.c` | 灯效状态机 |
| HAL 抽象 | `drivers/output/indicators/indicator_hal.c` | GPIO 操作 |

---

## 7. 需要创建的文件

| 文件 | 说明 |
|------|------|
| `keyboards/n0046/n0046_indicators.c` | 指示灯业务逻辑 |
| `keyboards/n0046/n0046_indicators.h` | 接口定义 |

---

## 8. 与其他模块的集成

### 8.1 与蓝牙模块集成

在蓝牙状态回调中调用指示灯：

```c
void wireless_enter_discoverable_kb(uint8_t host_idx) {
    n0046_indicator_bt_pairing();  // 白灯快闪
}

void wireless_enter_reconnecting_kb(uint8_t host_idx) {
    n0046_indicator_bt_reconnecting();  // 白灯慢闪
}

void wireless_enter_connected_kb(uint8_t host_idx) {
    n0046_indicator_bt_connected();  // 白灯熄灭
}
```

### 8.2 与电池模块集成

在主循环中检测电量：

```c
void n0046_task(void) {
    // 检查低电量
    if (battery_get_percentage() < 20) {
        n0046_indicator_low_battery(true);
    }
}
```

### 8.3 与键盘模块集成

处理大写锁定：

```c
void led_update_kb(led_t led_state) {
    n0046_indicator_caps_lock(led_state.caps_lock);
}
```

---

## 9. 验收标准

- [ ] 大写锁定时白灯常亮
- [ ] 取消大写时白灯熄灭
- [ ] 配对模式白灯快闪 (3次/秒)
- [ ] 回连模式白灯慢闪 (1次/秒)
- [ ] 已连接白灯熄灭
- [ ] 充电中红灯常亮
- [ ] 充满红灯熄灭
- [ ] 低电量 (<20%) 红灯闪烁
- [ ] Fn+右Cmd 电量查询灯效正确
- [ ] 恢复出厂所有灯闪烁3次

---

## 10. 测试方法

1. 按 Caps Lock 键，观察白灯
2. 进入配对模式（Fn+Del 长按），观察白灯
3. 连接蓝牙后，观察白灯熄灭
4. 连接 USB 充电，观察红灯
5. 模拟低电量（可临时调低阈值），观察红灯
6. 按 Fn+右Cmd，观察红灯闪烁次数

---

## 11. 参考文件

- `drivers/output/indicators/indicator.c` - 灯效驱动
- `drivers/output/indicators/indicator.h` - 灯效定义
- `docs/knowledge/N0046_功能清单.md` - 第6章指示灯模块
