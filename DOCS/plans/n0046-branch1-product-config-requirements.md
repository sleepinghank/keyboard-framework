# N0046 分支一：产品基础配置需求

> 分支名: `feature/n0046-product-config`
> 依赖: 无（第一个分支）
> 优先级: P0 - 必须首先完成

---

## 1. 目标

创建 N0046 产品的配置目录和基础文件，为后续功能开发提供配置基础。

---

## 2. 硬件上下文

### 2.1 主控芯片

| 项目 | 规格 |
|------|------|
| 型号 | CH584M (沁恒) |
| 封装 | QFN-48 |
| 内核 | 32-bit RISC-V |
| 蓝牙 | BLE 5.3 |
| 工作电压 | 3.3V |

### 2.2 GPIO 配置（来自原理图）

**矩阵行线 (ROW) - 输出：**

| 行号 | GPIO |
|------|------|
| R0 | PA4 |
| R1 | PA5 |
| R2 | PA6 |
| R3 | PA0 |
| R4 | PA1 |
| R5 | PA8 |
| R6 | PA9 |
| R7 | PB9 |

**矩阵列线 (COL) - 输入带上拉：**

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

**指示灯：**

| 功能 | GPIO | 电平 |
|------|------|------|
| 白灯 (CAPS/蓝牙) | PA15 | 高电平亮 |
| 红灯 (电源/充电) | PB23 | 高电平亮 |

**背光：**

| 功能 | GPIO | 说明 |
|------|------|------|
| 背光电源使能 | PB22 | 高电平有效 |
| 红色 R | PB0 | PWM |
| 绿色 G | PA12 | PWM |
| 蓝色 B | PA13 | PWM |

**电源管理：**

| 功能 | GPIO |
|------|------|
| 电池 ADC | PA14 |

---

## 3. 产品配置

### 3.1 产品信息

```c
#define PRODUCT_ID           0x0046
#define VENDOR_ID            0x3554    // inateck
#define MANUFACTURER         "inateck"
#define PRODUCT              "inateck AceTouch"
#define DEVICE_VER           0x0100
```

### 3.2 矩阵配置

```c
#define MATRIX_ROWS          8
#define MATRIX_COLS          16
#define DIODE_DIRECTION      ROW2COL
#define DEBOUNCE             5
```

### 3.3 蓝牙配置

```c
#define BLE_DEVICE_NAME      "inateck AceTouch"
#define BLE_HOST_COUNT       1          // 单通道
```

### 3.4 功能开关

```c
#define USB_ENABLE_FLAG      FALSE
#define BLUETOOTH_ENABLE     TRUE
#define BACKLIGHT_ENABLE     TRUE
#define INDICATOR_ENABLE     TRUE
```

---

## 4. 需要创建的文件

```
keyboards/n0046/
├── config.h              # 产品主配置
├── config_gpio.h         # GPIO 映射
├── config_matrix.h       # 矩阵配置
├── config_led.h          # LED 配置
└── rules.mk              # 编译规则
```

### 4.1 config.h 内容要求

- 产品 ID、厂商信息
- 包含子配置文件 (config_gpio.h, config_matrix.h, config_led.h)
- 功能开关定义
- 蓝牙名称配置

### 4.2 config_gpio.h 内容要求

- 矩阵行线引脚定义
- 矩阵列线引脚定义
- 指示灯引脚定义
- 背光引脚定义
- 电池 ADC 引脚定义

### 4.3 config_matrix.h 内容要求

- 矩阵尺寸 (8x16)
- 二极管方向 (ROW2COL)
- 防抖配置
- 扫描配置

### 4.4 config_led.h 内容要求

- 指示灯数量和 ID 定义
- 背光类型 (RGB)
- 背光默认值
- 亮度档位定义
- 颜色枚举定义

### 4.5 rules.mk 内容要求

- 源文件列表
- 包含路径
- 功能开关

---

## 5. 代码规范

- 遵循项目现有代码风格
- 使用 `#pragma once` 头文件保护
- 中文注释
- 4 空格缩进

---

## 6. 验收标准

- [ ] 编译通过，无警告
- [ ] GPIO 配置与原理图一致
- [ ] 矩阵尺寸正确 (8x16)
- [ ] 文件结构符合要求
- [ ] 配置项完整

---

## 7. 参考文件

- `docs/N0046_原理图.md` - GPIO 配置来源
- `keyboards/product_config.h` - 现有配置示例
- `CLAUDE.md` - 项目架构和编码规范
