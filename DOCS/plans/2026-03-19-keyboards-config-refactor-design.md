# keyboards/ 配置整合设计文档

> 创建日期: 2026-03-19
> 状态: Phase 2 完成，待实施

---

## 1. 需求摘要

将 `./keyboards/` 目录下三个独立文件整合到多产品分层配置架构：

| 文件 | 当前状态 | 目标状态 |
|------|---------|---------|
| `product_config.h` | 16 个引用 | 废弃 → `kb904/config.h` |
| `indicator_config.h` | 11 个引用 | 废弃 → `kb904/config_hw.h` + `indicator.c` |
| `default_keymap.c` | 根目录 | 迁移 → `kb904/keymaps/default.c` |

### 需求决策

| 问题 | 决策 |
|------|------|
| **产品范围** | 多产品架构，kb904 作为第一个实例 |
| **迁移策略** | 一次性迁移，更新所有引用 |
| **键位归属** | 产品级，移动到 `kb904/keymaps/` |
| **指示灯配置** | 分层设计（通用 `defaults/` + 产品 `kb904/`） |
| **验证要求** | 编译验证 |

---

## 2. 现状分析

### 2.1 当前目录结构

```
keyboards/
├── defaults/                    # 通用默认配置层（6个文件）
├── kb904/                       # KB904 产品专用配置
│   ├── config.h
│   ├── config_hw.h
│   └── README.md
├── keymaps/                     # 通用键位表头文件
├── config_check.h
├── product_config.h             # ⏳ 待废弃
├── indicator_config.h           # ⏳ 待废弃
└── default_keymap.c             # ⏳ 待迁移
```

### 2.2 依赖关系

```
product_config.h (16 引用)
    ↓
├── indicator_config.h (11 引用)
│       └── 混合配置覆盖 + 硬件表
│
└── default_keymap.c
        └── 间接依赖 MATRIX_ROWS/COLS
```

### 2.3 关键问题

1. **indicator_config.h 职责混合**：配置覆盖 + 硬件表定义
2. **产品配置分散**：product_config.h 在根目录，与 kb904/ 并存
3. **键位文件孤立**：default_keymap.c 未纳入产品目录

---

## 3. 方案设计

### 3.1 目标目录结构

```
keyboards/
├── defaults/                    # 通用默认配置层（不变）
│   ├── backlight_defaults.h
│   ├── ble_defaults.h
│   ├── indicator_defaults.h
│   ├── keyboard_defaults.h
│   ├── power_defaults.h
│   └── matrix_defaults.h
│
├── kb904/                       # KB904 产品专用配置
│   ├── config.h                 # 主配置入口（整合所有覆盖值）
│   ├── config_hw.h              # GPIO 硬件映射 + LED 别名
│   ├── indicator.c              # LED 硬件表实现（static 数组）
│   ├── keymaps/
│   │   └── default.c            # 键位表实现
│   └── README.md                # 产品配置说明
│
├── keymaps/                     # 通用键位表头文件（不变）
│   ├── default_keymap.h
│   ├── layout_8x16.h
│   └── layout_jis_113.h
│
├── config_check.h               # 静态断言检查（增强）
│
├── product_config.h             # ⚠️ 兼容层（#warning + 转发）
└── indicator_config.h           # ⚠️ 兼容层（#warning + 转发）
```

### 3.2 配置层级关系

```
┌─────────────────────────────────────────┐
│  application code                        │
│  #include "keyboards/kb904/config.h"    │
└──────────────────┬──────────────────────┘
                   │
┌──────────────────▼──────────────────────┐
│  kb904/config.h                          │
│  ├── 定义产品级覆盖值                    │
│  ├── #include "defaults/*.h"            │
│  ├── #include "config_hw.h"             │
│  └── #include "config_check.h"          │
└──────────────────────────────────────────┘
                   │
     ┌─────────────┼─────────────┐
     ▼             ▼             ▼
┌─────────┐ ┌───────────┐ ┌──────────────┐
│defaults/│ │config_hw.h│ │ indicator.c  │
│ 默认值  │ │ GPIO 映射 │ │ LED 硬件表   │
└─────────┘ └───────────┘ └──────────────┘
```

### 3.3 配置覆盖机制

```c
// kb904/config.h
#pragma once

// 1. 先定义覆盖值（在 include defaults 之前）
#define MATRIX_ROWS          8
#define MATRIX_COLS          16
#define DEBOUNCE             5
#define IND_LED_COUNT        4

// 2. 再 include defaults（#ifndef 保护使覆盖生效）
#include "defaults/matrix_defaults.h"
#include "defaults/indicator_defaults.h"
// ...

// 3. 最后 include 硬件映射
#include "config_hw.h"

// 4. 配置验证
#include "config_check.h"
```

### 3.4 LED 硬件表分离

**问题**：`static const` 数组不能放在 `.h` 文件中（多编译单元重复定义）

**解决方案**：
- 类型定义 + extern 声明 → `config_hw.h`
- 数组实现 → `kb904/indicator.c`

```c
// kb904/config_hw.h
typedef struct {
    uint8_t pin;
    uint8_t active_level;
} ind_led_hw_t;

extern const ind_led_hw_t ind_led_table[IND_LED_COUNT];

// LED 业务别名
#define LED_CAPS          0
#define LED_BT            1
#define LED_POWER_GREEN   2
#define LED_POWER_RED     3
```

```c
// kb904/indicator.c
#include "config.h"
#include "config_hw.h"

static const ind_led_hw_t ind_led_table[IND_LED_COUNT] = {
    { 12, 1 },  // LED_CAPS
    { 13, 0 },  // LED_BT (active low)
    { 14, 1 },  // LED_POWER_GREEN
    { 15, 1 },  // LED_POWER_RED
};
```

### 3.5 兼容层设计

```c
// keyboards/product_config.h
#pragma once
#warning "product_config.h is deprecated. Include keyboards/kb904/config.h instead."
#include "kb904/config.h"

// keyboards/indicator_config.h
#pragma once
#warning "indicator_config.h is deprecated. Include keyboards/kb904/config_hw.h instead."
#include "kb904/config_hw.h"
```

### 3.6 静态断言增强

```c
// config_check.h（新增部分）
#pragma once

_Static_assert(MATRIX_ROWS > 0 && MATRIX_ROWS <= 16,
    "MATRIX_ROWS must be 1-16");
_Static_assert(MATRIX_COLS > 0 && MATRIX_COLS <= 16,
    "MATRIX_COLS must be 1-16");
_Static_assert(DEBOUNCE >= 0 && DEBOUNCE <= 50,
    "DEBOUNCE must be 0-50ms");
_Static_assert(IND_LED_COUNT >= 0 && IND_LED_COUNT <= 8,
    "IND_LED_COUNT must be 0-8");
```

---

## 4. 配置迁移清单

### 4.1 product_config.h → kb904/config.h

| 配置项 | 类型 |
|--------|------|
| `PRODUCT_ID` | 产品标识 |
| `CHIP_TYPE_CH584` | 芯片类型 |
| `MATRIX_ROWS/COLS` | 矩阵尺寸 |
| `DEBOUNCE` | 防抖时间 |
| `TAPPING_TERM` | 按键行为 |
| `EEPROM_SIZE` | 存储配置 |
| `FN_KEY_ROW/COL` | FN 键位置 |

### 4.2 indicator_config.h → kb904/config_hw.h + indicator.c

| 内容 | 目标文件 |
|------|---------|
| `LED_*` 别名 | config_hw.h |
| `ind_led_hw_t` 类型 | config_hw.h |
| `ind_led_table[]` | indicator.c |

### 4.3 default_keymap.c → kb904/keymaps/default.c

| 内容 | 说明 |
|------|------|
| `keymap_layers[]` | 4 层键位表 |

---

## 5. 实施计划

### Phase 1：创建新文件（无依赖）

| 步骤 | 任务 | 验证 |
|------|------|------|
| 1.1 | 创建 `kb904/indicator.c` | 编译通过 |
| 1.2 | 创建 `kb904/keymaps/default.c` | 编译通过 |
| 1.3 | 增强 `config_check.h` 静态断言 | 编译通过 |

### Phase 2：更新引用（依赖 Phase 1）

#### 2.1 product_config.h 引用文件清单

```
# 查询命令：grep -r "#include.*product_config.h" --include="*.c" --include="*.h"
drivers/input/keyboard/matrix.c
drivers/input/keyboard/debounce.c
drivers/output/indicators/indicator.c
drivers/output/backlight/backlight.c
drivers/communication/ble/ble_driver.c
drivers/system/event_manager.c
middleware/keyboard/keyboard.c
middleware/keyboard/action.c
middleware/keyboard/report.c
middleware/keyboard/combo.c
middleware/communication/transport.c
middleware/communication/wireless.c
middleware/communication/report_buffer.c
application/system/system_init.c
application/main.c
keyboards/keymaps/default_keymap.h
```

**替换**：`#include "product_config.h"` → `#include "keyboards/kb904/config.h"`

#### 2.2 indicator_config.h 引用文件清单

```
# 查询命令：grep -r "#include.*indicator_config.h" --include="*.c" --include="*.h"
drivers/output/indicators/indicator.c
drivers/output/indicators/indicator_effect.c
drivers/output/backlight/backlight.c
middleware/keyboard/keyboard.c
middleware/keyboard/action.c
application/system/system_init.c
application/services/indicator_service.c
application/services/power_service.c
application/services/ble_service.c
keyboards/kb904/config_hw.h
keyboards/defaults/indicator_defaults.h
```

**替换**：`#include "indicator_config.h"` → `#include "keyboards/kb904/config_hw.h"`

#### 2.3 CMakeLists.txt 更新

```cmake
# 旧配置（删除）
set(KEYBOARD_SOURCES
    keyboards/default_keymap.c
)

# 新配置（替换）
set(KEYBOARD_SOURCES
    keyboards/kb904/indicator.c
    keyboards/kb904/keymaps/default.c
)
```

#### 2.4 CH584M makefile 更新

```makefile
# project/ch584m/makefile
# 添加源文件
SUBDIRS += keyboards/kb904
C_SOURCES += keyboards/kb904/indicator.c
C_SOURCES += keyboards/kb904/keymaps/default.c

# 删除旧源文件
# C_SOURCES -= keyboards/default_keymap.c  # 如果存在
```

### Phase 3：配置整合（依赖 Phase 2）

| 步骤 | 任务 | 验证 |
|------|------|------|
| 3.1 | 迁移 product_config.h 配置项 | 编译通过 |
| 3.2 | 迁移 indicator_config.h 内容 | 编译通过 |

### Phase 4：兼容层与清理（依赖 Phase 3）

| 步骤 | 任务 | 验证 |
|------|------|------|
| 4.1 | 添加兼容层 #warning | 编译警告出现 |
| 4.2 | 验证 Windows 编译 | cmake build |
| 4.3 | 验证 CH584M 编译 | wch-riscv-build |
| 4.4 | 删除废弃文件 | 编译通过 |
| 4.5 | 创建 kb904/README.md | 文档完整 |

---

## 6. 风险与缓解

| 风险 | 等级 | 缓解措施 |
|------|------|---------|
| CH584M makefile 手动同步 | 中 | 迁移后立即测试固件编译 |
| 兼容层警告堆积 | 低 | 设置 1-2 个版本后移除计划 |
| 静态断言编译阻塞 | 低 | 提供明确错误消息 |

### 6.5 回滚方案

#### Git 分支策略

```
master (稳定)
  │
  └── feature/keyboards-config-refactor  ← 本次迁移分支
        │
        ├── checkpoint-1: 新文件创建完成
        ├── checkpoint-2: 引用更新完成
        ├── checkpoint-3: 配置整合完成
        └── checkpoint-4: 验证通过，准备合并
```

#### 回滚触发条件

| 条件 | 回滚级别 |
|------|---------|
| Windows 编译失败 | 返回上一 checkpoint |
| CH584M 编译失败 | 返回上一 checkpoint |
| 键位/LED 功能异常 | 全部回滚，保留兼容层 |

#### 回滚命令

```bash
# 回滚到上一个 checkpoint
git reset --hard checkpoint-2

# 或使用 revert（保留历史）
git revert HEAD~3..HEAD

# 最坏情况：返回 master
git checkout master
git branch -D feature/keyboards-config-refactor
```

#### 兼容层保留周期

- **v1.0**（本版本）：兼容层 #warning 生效
- **v1.1**：兼容层保留，移除 #warning
- **v1.2**：废弃文件标记 `#error`
- **v2.0**：删除废弃文件

---

## 7. 检查点

- [x] 步骤 1-3: 需求确认（Phase 1）
- [x] 步骤 4-6: 方案设计（Phase 2）
- [x] 步骤 7-8: 计划输出（Phase 3）
- [ ] 步骤 9: 转入实施
