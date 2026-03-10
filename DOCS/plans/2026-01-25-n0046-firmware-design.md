# N0046 键盘固件设计文档

**产品型号**: N0046
**日期**: 2026-01-25
**状态**: 已确认

---

## 1. 项目概述

| 项目 | 规格 |
|------|------|
| 主控芯片 | 沁恒 CH584M (蓝牙) |
| 触控芯片 | Pixart PCT1336QN (本次不实现) |
| 连接方式 | 蓝牙 5.3 单通道 |
| 目标平台 | iPadOS 16/17/18 |
| 键盘布局 | 8x16 矩阵 |
| 支持语种 | US、DE、UK、JP、FR (编译时选择) |

---

## 2. 项目结构

```
keyboards/
└── n0046/
    ├── n0046_config.h          # 产品硬件配置
    ├── n0046_indicator.h       # 指示灯配置 (2灯)
    ├── n0046_backlight.h       # 背光配置 (7色RGB)
    └── keymaps/
        ├── keymap_us.h         # US 布局
        ├── keymap_de.h         # DE 布局
        ├── keymap_uk.h         # UK 布局
        ├── keymap_jp.h         # JP 布局
        └── keymap_fr.h         # FR 布局

drivers/power/
└── power_switch.c/.h           # 电源开关机管理

application/service/
└── factory_reset.c/.h          # 恢复出厂设置
```

**复用模块**:
- `drivers/input/keyboard/matrix.c` - 矩阵扫描
- `drivers/input/keyboard/debounce.c` - 防抖
- `drivers/output/backlight/backlight.c` - 背光驱动
- `drivers/output/indicators/indicator.c` - 指示灯驱动
- `hal/platforms/ch584/` - CH584 HAL 层

---

## 3. GPIO 引脚配置

### 3.1 矩阵配置 (8行 x 16列)

```c
#define MATRIX_ROWS 8
#define MATRIX_COLS 16
#define MATRIX_ROW_PINS { A4, A5, A6, A0, A1, A8, A9, B9 }
#define MATRIX_COL_PINS { B5, B8, B17, B16, B15, B14, B13, B12, B3, B4, B2, A7, B7, B18, B1, B6 }
```

### 3.2 指示灯

```c
#define LED_CAPS_BT_PIN     A15     // 白灯：大写/蓝牙状态
#define LED_BATTERY_PIN     B23     // 红灯：电量/充电
```

### 3.3 背光 RGB

```c
#define BACKLIGHT_POWER_PIN B22     // 背光电源控制
#define BACKLIGHT_R_PIN     B0      // 红色通道
#define BACKLIGHT_G_PIN     A12     // 绿色通道
#define BACKLIGHT_B_PIN     A13     // 蓝色通道
```

### 3.4 触控板 (本次不实现)

```c
#define TOUCH_POWER_PIN     A11     // 电源控制，低电平有效
#define TOUCH_INT_PIN       A3      // 中断，下降沿
#define TOUCH_SDA_PIN       B20
#define TOUCH_SCL_PIN       B21
```

### 3.5 电量检测

```c
#define BATTERY_ADC_PIN     A14     // ADC 输入
```

### 3.6 电源控制

```c
#define POWER_BUTTON_PIN    A2      // 电源按钮检测，低电平按下
#define POWER_HOLD_PIN      B19     // 供电保持，高电平保持
```

---

## 4. 电源管理模块

### 4.1 状态定义

- **关机**: 完全断电 (B19 低电平)，电流 0μA
- **休眠**: 低功耗模式，电流 < 40μA
- **运行**: 正常工作，空闲约 1mA

### 4.2 状态机

```
[断电] --电源键按下--> [上电检测]
[上电检测] --长按3秒--> [开机运行] (拉高B19)
[上电检测] --短按松开--> [断电] (不拉高B19)
[开机运行] --长按电源键3秒--> [关机] (拉低B19)
[开机运行] --单按电源键--> [发送锁屏键码]
[开机运行] --10分钟无操作--> [深度休眠]
[深度休眠] --任意键按下--> [开机运行]
```

### 4.3 接口函数

```c
void power_switch_init(void);
void power_switch_task(void);
void power_shutdown(void);
void power_enter_sleep(void);
void power_exit_sleep(void);
bool power_button_pressed(void);
```

---

## 5. 蓝牙状态机

### 5.1 配置参数

```c
#define BT_HOST_DEVICES_COUNT       1       // 单通道
#define BT_PAIRING_TIMEOUT_SEC      60
#define BT_RECONNECT_TIMEOUT_SEC    60
#define IDLE_SLEEP_TIMEOUT_SEC      600     // 10分钟
```

### 5.2 状态与指示灯映射 (白灯 A15)

| 状态 | 白灯行为 |
|------|----------|
| 配对中 | 快闪 (3次/秒) |
| 回连中 | 慢闪 (1次/秒) |
| 已连接 | 熄灭 |
| 大写锁定 | 常亮 (优先级最高) |

### 5.3 超时处理

- 配对超时 60 秒 → 进入休眠
- 回连超时 60 秒 → 进入休眠
- 已连接无操作 10 分钟 → 进入深度休眠

---

## 6. 背光模块

### 6.1 颜色循环顺序

```c
typedef enum {
    BL_COLOR_RED = 0,    // 红
    BL_COLOR_YELLOW,     // 黄
    BL_COLOR_CYAN,       // 青
    BL_COLOR_BLUE,       // 蓝
    BL_COLOR_GREEN,      // 绿
    BL_COLOR_WHITE,      // 白
    BL_COLOR_MAGENTA,    // 洋红
    BL_COLOR_COUNT
} bl_preset_color_t;
```

### 6.2 配置参数

```c
#define BACKLIGHT_DEFAULT_COLOR     BL_COLOR_WHITE
#define BACKLIGHT_DEFAULT_LEVEL     BL_LEVEL_MEDIUM  // 二档
#define BACKLIGHT_IDLE_TIMEOUT_SEC  5                // 5秒无操作熄灭
#define BACKLIGHT_LOW_BATTERY_PCT   20               // 低于20%强制关闭
```

### 6.3 组合键

- `Fn + 右Enter`: 循环切换颜色
- `Fn + 右Shift` (JP为右_): 循环切换亮度档位 (关→一档→二档→三档)

---

## 7. 电量检测

### 7.1 ADC 配置

```c
#define BATTERY_ADC_PIN         A14
#define BATTERY_DIVIDER_RATIO   0.781f  // 2M / (560k + 2M)
#define BATTERY_FULL_VOLTAGE    4200    // mV
#define BATTERY_EMPTY_VOLTAGE   3000    // mV
```

### 7.2 电量阈值

```c
#define BATTERY_LOW_THRESHOLD       20   // 低电警告阈值
#define BATTERY_LEVEL_1             25   // 查询闪1次
#define BATTERY_LEVEL_2             50   // 查询闪2次
#define BATTERY_LEVEL_3             75   // 查询闪3次
#define BATTERY_LEVEL_4             100  // 查询闪4次
```

### 7.3 指示灯行为 (红灯 B23)

| 状态 | 红灯行为 |
|------|----------|
| 正常电量 (>20%) | 熄灭 |
| 低电量 (≤20%) | 闪烁 |
| 充电中 | 常亮 |
| 充满 | 熄灭 (绿灯由充电IC控制) |

### 7.4 电量查询

`Fn + 右Cmd`: 红灯闪烁显示电量档位 (1-4次)

---

## 8. 按键映射

### 8.1 多媒体键 (F1-F12 位置)

| 位置 | 功能 | 键码 |
|------|------|------|
| F1 | ESC | KC_ESC |
| F2 | 亮度减 | KC_BRIGHTNESS_DOWN |
| F3 | 亮度加 | KC_BRIGHTNESS_UP |
| F4 | 搜索 | KC_SPOTLIGHT |
| F5 | 截屏 | KC_SCREENSHOT |
| F6 | Siri | KC_SIRI |
| F7 | 上一首 | KC_MEDIA_PREV |
| F8 | 暂停/播放 | KC_MEDIA_PLAY_PAUSE |
| F9 | 下一首 | KC_MEDIA_NEXT |
| F10 | 静音 | KC_MUTE |
| F11 | 音量减 | KC_VOLUME_DOWN |
| F12 | 音量加 | KC_VOLUME_UP |

### 8.2 通用组合键

| 组合键 | 功能 |
|--------|------|
| Fn + Del | 长按3秒进入配对 |
| Fn + 右Enter | 背光颜色切换 |
| Fn + 右Shift | 背光亮度切换 |
| Fn + 右Cmd | 电量查询 |
| ESC + = | 长按3秒恢复出厂 |

### 8.3 语种特殊键

**DE (德语)**:
- `Fn + Y` = <
- `Fn + X` = >
- `Opt + E` = €
- `Opt + L` = @

**UK (英式)**:
- `Fn + Z` = \
- `Fn + X` = |
- `Opt + 2` = €
- `Opt + 3` = #

**FR (法语)**:
- `Fn + W` = <
- `Fn + X` = >

---

## 9. 恢复出厂设置

### 9.1 触发条件

ESC + = 同时按住 3 秒

### 9.2 执行流程

1. 清除蓝牙配对记录
2. 重置背光为默认值 (白光二档)
3. 清除其他用户设置
4. 指示灯闪烁 3 次确认
5. 进入蓝牙配对模式

---

## 10. 开发顺序

| 阶段 | 模块 | 关键任务 |
|------|------|----------|
| 1 | 电源管理 | 开关机、供电保持、电源键检测 |
| 2 | 矩阵+指示灯 | GPIO 配置验证、基本按键输入 |
| 3 | 蓝牙连接 | 配对、回连、状态机、HID 报告 |
| 4 | 多媒体键 | F1-F12 映射、地球键 |
| 5 | 组合键 | Fn 层、长按检测、恢复出厂 |
| 6 | 电量管理 | ADC 采集、低电警告、休眠 |
| 7 | 背光 | RGB 控制、颜色/亮度切换 |

---

## 11. 文件修改清单

### 11.1 新增文件

```
keyboards/n0046/
├── n0046_config.h
├── n0046_indicator.h
├── n0046_backlight.h
└── keymaps/
    ├── keymap_us.h
    ├── keymap_de.h
    ├── keymap_uk.h
    ├── keymap_jp.h
    └── keymap_fr.h

drivers/power/
└── power_switch.c/.h

application/service/
└── factory_reset.c/.h
```

### 11.2 修改文件

```
drivers/output/backlight/backlight.c    # 颜色顺序调整
drivers/power/battery.c                 # 电量阈值、查询功能
middleware/communication/wireless.c     # 状态机、超时参数
keyboards/indicator_config.h            # N0046 指示灯映射
CMakeLists.txt                          # 添加新文件、语种编译选项
```

### 11.3 编译配置

```c
#define PRODUCT_N0046           // 产品型号
#define KEYBOARD_LAYOUT_US      // 语种选择 (US/DE/UK/JP/FR)
```

---

## 12. 本次不实现

- 触控板功能 (PCT1336QN 驱动)
- OTA 固件升级
- APP 自定义功能 (F6 自定义、背光时间调整、休眠时间调整)

---

*文档版本: V1.0*
*基于: N0046_功能清单.md, N0046_硬件配置.md*
