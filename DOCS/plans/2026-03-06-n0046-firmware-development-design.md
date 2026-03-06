# N0046 键盘固件开发设计文档

> 版本: V1.0
> 日期: 2026-03-06
> 状态: 设计评审

---

## 1. 概述

### 1.1 项目目标

在现有 keyboard-framework 基础上，实现 N0046 iPad 键盘的完整固件功能。

### 1.2 产品规格

| 项目 | 规格 |
|------|------|
| 产品型号 | N0046 |
| 主控芯片 | CH584M (沁恒) |
| 蓝牙版本 | BLE 5.3 |
| 连接方式 | 蓝牙单通道 |
| 目标平台 | iPadOS 16/17/18 |
| 键盘布局 | US (本次优先) |
| 蓝牙名称 | "inateck AceTouch" |
| 电池 | 3.7V 500mAh 锂电池 |

### 1.3 开发范围

**本次实现：**
- 键盘矩阵扫描与防抖
- 蓝牙配对/回连/连接
- 指示灯业务逻辑（蓝牙状态、大写锁定、低电量、充电状态）
- 背光业务逻辑（七彩切换、亮度调节、5秒休眠、低电量关闭）
- Fn 组合键功能（配对、颜色、亮度、电量查询）
- 恢复出厂设置（ESC+= 3秒）
- 多媒体/系统快捷键（F1-F12）
- F6 默认 Siri 功能

**本次不实现：**
- 触控板模块（后续专项）
- OTA 升级功能（后续专项）
- F6 自定义功能（后续随 APP）
- 软件开关机功能（后续专项实现）
- 电源键功能（后续专项实现）

---

## 2. 硬件接口定义

### 2.1 主控芯片

| 项目 | 规格 |
|------|------|
| 型号 | CH584M (沁恒) |
| 封装 | QFN-48 (5.0x5.0mm, 0.35mm pitch) |
| 内核 | 32-bit RISC-V |
| 蓝牙 | BLE 5.3 |
| 工作电压 | 3.3V |
| 晶振 | 32MHz |

### 2.2 GPIO 配置

#### 按键矩阵 GPIO

**行线 (ROW) - 输出：**

| 行号 | GPIO | 网络名 |
|------|------|--------|
| R0 | PA4 | PA4 |
| R1 | PA5 | PA5 |
| R2 | PA6 | PA6 |
| R3 | PA0 | PA0 |
| R4 | PA1 | PA1 |
| R5 | PA8 | PA8_RXD1* |
| R6 | PA9 | PA9_TXD1* |
| R7 | PB9 | PB9 |

> *PA8/PA9 复用为 UART1 调试接口

**列线 (COL) - 输入 (带上拉)：**

| 列号 | GPIO | 列号 | GPIO |
|------|------|------|------|
| C0 | PB5 | C8 | PB3 |
| C1 | PB8 | C9 | PB4 |
| C2 | PB17 | C10 | PB2 |
| C3 | PB16 | C11 | PA7 |
| C4 | PB15 | C12 | PB7 |
| C5 | PB14 | C13 | PB18 |
| C6 | PB13 | C14 | PB1 |
| C7 | PB12 | C15 | PB6 |

#### 指示灯 GPIO

| 功能 | GPIO | 电平有效 |
|------|------|----------|
| 白灯 (CAPS/蓝牙) | PA15 | 高电平 |
| 红灯 (电源/充电) | PB23 | 高电平 |

#### 背光 GPIO

| 功能 | GPIO | 说明 |
|------|------|------|
| 背光电源使能 | PB22 | 高电平有效 |
| 红色 (R) | PB0 | PWM |
| 绿色 (G) | PA12 | PWM |
| 蓝色 (B) | PA13 | PWM |

#### 电源管理 GPIO

| 功能 | GPIO | 方向 |
|------|------|------|
| 电池 ADC 检测 | PA14 | 输入 |

### 2.3 矩阵配置

```c
#define MATRIX_ROWS 8
#define MATRIX_COLS 16
#define DIODE_DIRECTION ROW2COL
#define DEBOUNCE 5

#define MATRIX_ROW_PINS { PA4, PA5, PA6, PA0, PA1, PA8, PA9, PB9 }
#define MATRIX_COL_PINS { PB5, PB8, PB17, PB16, PB15, PB14, PB13, PB12, \
                          PB3, PB4, PB2, PA7, PB7, PB18, PB1, PB6 }
```

---

## 3. 分支规划

### 3.1 分支列表

| 分支名 | 功能范围 | 主要工作内容 |
|--------|----------|--------------|
| `feature/n0046-product-config` | 产品基础配置 | 创建 n0046 目录、GPIO 定义、产品配置 |
| `feature/n0046-keyboard-input` | 键盘输入 | 键位映射、矩阵扫描调试、基本打字 |
| `feature/n0046-bluetooth-connection` | 蓝牙连接 | 配对、回连、状态机、HID 报告 |
| `feature/n0046-indicator-system` | 指示灯系统 | 灯效逻辑：蓝牙状态、大写、低电量、充电 |
| `feature/n0046-backlight-system` | 背光系统 | 七彩切换、亮度调节、5秒休眠、低电量关闭 |
| `feature/n0046-special-functions` | 特殊功能 | Fn 组合键、恢复出厂、电量查询、多媒体键 |

### 3.2 开发顺序

```
1. feature/n0046-product-config
   ↓ (基础配置完成)
2. feature/n0046-keyboard-input
   ↓ (基本打字功能验证)
3. feature/n0046-bluetooth-connection
   ↓ (蓝牙连接验证)
4. feature/n0046-indicator-system
   ↓ (状态指示验证)
5. feature/n0046-backlight-system
   ↓ (背光功能验证)
6. feature/n0046-special-functions
   (组合键和特殊功能验证)
```

---

## 4. 模块详细设计

### 4.1 分支一：feature/n0046-product-config

**目标：** 创建 N0046 产品配置目录和基础文件

**新增文件：**

```
keyboards/n0046/
├── config.h              # 产品定义
├── config_gpio.h         # GPIO 映射
├── config_matrix.h       # 矩阵定义
├── config_led.h          # LED 配置
├── config_battery.h      # 电池配置
└── rules.mk              # 编译规则
```

**关键配置项：**

| 配置项 | 值 |
|--------|-----|
| PRODUCT_ID | 0x0046 |
| VENDOR_ID | 0x3554 (inateck) |
| PRODUCT | "inateck AceTouch" |
| MANUFACTURER | "inateck" |
| MATRIX_ROWS | 8 |
| MATRIX_COLS | 16 |
| DIODE_DIRECTION | ROW2COL |
| DEBOUNCE | 5 |
| BACKLIGHT_TYPE | RGB |
| IND_LED_COUNT | 2 |

**验收标准：**
- [ ] 编译通过，无警告
- [ ] GPIO 配置与原理图一致
- [ ] 矩阵尺寸正确

### 4.2 分支二：feature/n0046-keyboard-input

**目标：** 实现键盘输入功能

**新增文件：**

```
keyboards/n0046/
└── keymaps/
    └── default.c          # US 布局键位映射
```

**键位映射要点：**
- 78 键矩阵位置定义
- F1-F12 多媒体键映射
- Fn 组合键层定义

**复用现有模块：**
- `drivers/input/keyboard/matrix.c` - 无需修改
- `drivers/input/keyboard/debounce.c` - 无需修改
- `middleware/keyboard/keyboard.c` - 无需修改

**验收标准：**
- [ ] 所有按键位置正确映射
- [ ] 防抖功能正常
- [ ] 基本打字功能验证

### 4.3 分支三：feature/n0046-bluetooth-connection

**目标：** 实现蓝牙连接功能

**修改文件：**
- `keyboards/n0046/n0046.c` - 蓝牙状态机回调

**状态机配置：**

| 状态 | 指示灯行为 | 超时 |
|------|----------|------|
| WT_PARING | 白灯快闪 (3次/秒) | 60秒 → 休眠 |
| WT_RECONNECTING | 白灯慢闪 (1次/秒) | 60秒 → 休眠 |
| WT_CONNECTED | 白灯熄灭 | - |
| WT_DISCONNECTED | 白灯闪烁 | 10分钟 → 深度休眠 |

**配对触发：**
- 首次开机 → 自动配对
- Fn+Del 长按 3 秒 → 手动配对

**验收标准：**
- [ ] 蓝牙名称正确显示 "inateck AceTouch"
- [ ] 首次开机自动进入配对模式
- [ ] Fn+Del 长按3秒进入配对模式
- [ ] 配对成功后自动连接
- [ ] 回连功能正常
- [ ] HID 报告正确发送

### 4.4 分支四：feature/n0046-indicator-system

**目标：** 实现指示灯业务逻辑

**新增文件：**
- `keyboards/n0046/n0046_indicators.c`

**指示灯定义：**

| LED | GPIO | 功能 |
|-----|------|------|
| 白灯 (CAPS) | PA15 | 大写锁定 / 蓝牙状态 |
| 红灯 (PWR) | PB23 | 充电中 / 低电量 |

**灯效逻辑：**

| 事件 | 白灯 | 红灯 |
|------|------|------|
| 大写锁定 | 常亮 | - |
| 大写未锁定 | 熄灭 | - |
| 蓝牙配对 | 快闪 (3次/秒) | - |
| 蓝牙回连 | 慢闪 (1次/秒) | - |
| 蓝牙已连接 | 熄灭 | - |
| 充电中 | - | 常亮 |
| 充满 | - | 熄灭 (绿灯由硬件控制) |
| 低电量 (<20%) | - | 闪烁 |
| 电量查询 | - | 闪烁 N 次 (25% 档位) |

**复用现有模块：**
- `drivers/output/indicators/indicator.c` - 灯效驱动
- `drivers/output/indicators/indicator_hal.c` - HAL 抽象

**验收标准：**
- [ ] 大写锁定指示正确
- [ ] 蓝牙状态指示正确
- [ ] 充电状态指示正确
- [ ] 低电量警告指示正确
- [ ] 电量查询灯效正确

### 4.5 分支五：feature/n0046-backlight-system

**目标：** 实现背光业务逻辑

**新增文件：**
- `keyboards/n0046/n0046_backlight.c`

**功能实现：**

| 功能 | 实现方式 |
|------|----------|
| 颜色切换 | 循环：红→黄→青→蓝→绿→白→洋红 |
| 亮度调节 | 循环：关→1档→2档→3档 |
| 默认值 | 白光、2档 |
| 5秒休眠 | 定时器 + 按键唤醒 |
| 记忆功能 | 关机/休眠后保持设置 |
| 低电量关闭 | 电量 < 20% 时强制关闭，按键不唤醒 |

**复用现有模块：**
- `drivers/output/backlight/backlight.c` - RGB 驱动
- `drivers/output/backlight/backlight_hal.c` - HAL 抽象

**验收标准：**
- [ ] Fn+右Enter 切换颜色
- [ ] Fn+右Shift 切换亮度
- [ ] 开机默认白光2档
- [ ] 5秒无操作熄灭
- [ ] 按键唤醒背光
- [ ] 低电量强制关闭
- [ ] 颜色/亮度记忆功能

### 4.6 分支六：feature/n0046-special-functions

**目标：** 实现特殊组合键功能

**修改文件：**
- `keyboards/n0046/n0046.c` - 组合键处理

**组合键定义：**

| 组合键 | 功能 | 优先级 |
|--------|------|--------|
| Fn+Del 长按3s | 蓝牙配对 | P0 |
| Fn+右Enter | 背光颜色切换 | P1 |
| Fn+右Shift | 背光亮度切换 | P1 |
| Fn+右Cmd | 电量查询 | P1 |
| ESC+= 长按3s | 恢复出厂设置 | P0 |

**多媒体键映射：**

| 按键 | 功能 | HID Usage |
|------|------|-----------|
| F1 | ESC | Escape |
| F2 | 亮度减 | Brightness Down |
| F3 | 亮度加 | Brightness Up |
| F4 | 搜索 | Search |
| F5 | 截屏 | Screenshot |
| F6 | Siri | Voice Command |
| F7 | 上一首 | Scan Previous |
| F8 | 播放/暂停 | Play/Pause |
| F9 | 下一首 | Scan Next |
| F10 | 静音 | Mute |
| F11 | 音量减 | Volume Down |
| F12 | 音量加 | Volume Up |

**恢复出厂设置逻辑：**
1. 检测 ESC+= 同时按下
2. 长按 3 秒触发
3. 清除蓝牙配对记录
4. 重置背光为默认值（白光2档）
5. 所有指示灯闪烁 3 次
6. 进入配对模式

**验收标准：**
- [ ] Fn+Del 长按3秒进入配对
- [ ] Fn+右Enter 切换背光颜色
- [ ] Fn+右Shift 切换背光亮度
- [ ] Fn+右Cmd 电量查询灯效
- [ ] ESC+= 长按3秒恢复出厂
- [ ] F1-F12 多媒体键正确输出
- [ ] F6 触发 Siri

---

## 5. 文件结构

### 5.1 新增文件

```
keyboards/n0046/
├── config.h              # 产品配置
├── config_gpio.h         # GPIO 映射
├── config_matrix.h       # 矩阵定义
├── config_led.h          # LED 配置
├── config_battery.h      # 电池配置
├── keymaps/
│   └── default.c          # US 布局键位映射
├── n0046.c                # 产品入口
├── n0046_indicators.c     # 指示灯业务逻辑
├── n0046_backlight.c      # 背光业务逻辑
└── rules.mk               # 编译规则
```

### 5.2 修改文件

| 文件路径 | 修改内容 |
|----------|----------|
| `keyboards/product_config.h` | 添加 N0046 条件编译 |
| `CMakeLists.txt` | 添加 N0046 源文件 |

### 5.3 复用文件

| 模块 | 文件路径 | 说明 |
|------|----------|------|
| 矩阵扫描 | `drivers/input/keyboard/matrix.c` | 无需修改 |
| 防抖处理 | `drivers/input/keyboard/debounce.c` | 无需修改 |
| 背光驱动 | `drivers/output/backlight/backlight.c` | 无需修改 |
| 指示灯驱动 | `drivers/output/indicators/indicator.c` | 无需修改 |
| 电池管理 | `drivers/power/battery.c` | 无需修改 |
| 蓝牙驱动 | `drivers/communication/bluetooth/ch584/` | 无需修改 |
| 无线状态机 | `middleware/communication/wireless.c` | 无需修改 |
| 低功耗管理 | `middleware/communication/lpm.c` | 无需修改 |
| 键盘处理 | `middleware/keyboard/` | 无需修改 |
| HAL 层 | `hal/platforms/ch584/` | 无需修改 |

---

## 6. 模块依赖关系

```
┌─────────────────────────────────────────────────────────────┐
│                  Application (N0046)                        │
│  n0046.c - 组合键处理、恢复出厂、电量查询                      │
├─────────────────────────────────────────────────────────────┤
│  Service Layer                                              │
│  ├── n0046_indicators.c - 指示灯业务逻辑                     │
│  └── n0046_backlight.c - 背光业务逻辑                        │
├─────────────────────────────────────────────────────────────┤
│  Middleware (复用)                                          │
│  ├── keyboard/ - 按键处理、action、combo、report             │
│  └── communication/ - wireless、lpm、transport              │
├─────────────────────────────────────────────────────────────┤
│  Drivers (复用)                                             │
│  ├── input/keyboard/ - matrix、debounce                     │
│  ├── output/ - backlight、indicators                        │
│  ├── power/ - battery                                       │
│  └── communication/bluetooth/ch584/ - BLE 驱动              │
├─────────────────────────────────────────────────────────────┤
│  HAL (复用)                                                 │
│  └── platforms/ch584/ - gpio、pwm、adc、uart、i2c            │
└─────────────────────────────────────────────────────────────┘
```

---

## 7. 验收标准

### 7.1 功能验收

| 模块 | 验收项 | 状态 |
|------|--------|------|
| 键盘输入 | 所有按键正确映射 | [ ] |
| 键盘输入 | 防抖功能正常 | [ ] |
| 蓝牙连接 | 蓝牙名称正确 | [ ] |
| 蓝牙连接 | 配对功能正常 | [ ] |
| 蓝牙连接 | 回连功能正常 | [ ] |
| 指示灯 | 大写锁定指示 | [ ] |
| 指示灯 | 蓝牙状态指示 | [ ] |
| 指示灯 | 充电状态指示 | [ ] |
| 指示灯 | 低电量警告 | [ ] |
| 指示灯 | 电量查询 | [ ] |
| 背光 | 颜色切换 | [ ] |
| 背光 | 亮度调节 | [ ] |
| 背光 | 5秒休眠 | [ ] |
| 背光 | 低电量关闭 | [ ] |
| 组合键 | Fn+Del 配对 | [ ] |
| 组合键 | Fn+Enter 颜色 | [ ] |
| 组合键 | Fn+Shift 亮度 | [ ] |
| 组合键 | Fn+Cmd 电量 | [ ] |
| 恢复出厂 | ESC+= 触发 | [ ] |
| 恢复出厂 | 配对清除 | [ ] |
| 恢复出厂 | 背光重置 | [ ] |
| 多媒体键 | F1-F12 功能 | [ ] |

### 7.2 代码质量

- [ ] 编译通过，无警告
- [ ] GPIO 配置与原理图一致
- [ ] 遵循项目代码规范
- [ ] 有对应的测试用例或验证方法

---

## 8. 附录

### 8.1 参考文档

- `docs/knowledge/N0046_功能清单.md` - 功能需求
- `docs/N0046_原理图.md` - GPIO 配置
- `CLAUDE.md` - 项目架构和编码规范

### 8.2 修订历史

| 版本 | 日期 | 作者 | 说明 |
|------|------|------|------|
| V1.0 | 2026-03-06 | Claude | 初始版本 |

---

*本文档由 Claude 生成，基于 brainstorming skill 流程*
