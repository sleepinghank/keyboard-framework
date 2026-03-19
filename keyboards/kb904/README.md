# KB904 键盘配置指南

本档说明 KB904 产品的配置结构和定制方法。

## 目录结构

```
keyboards/kb904/
├── config.h          # 主配置入口（产品级覆盖值）
├── config_hw.h       # GPIO 硬件映射 + LED 类型定义
├── indicator.c       # LED 硬件表实现
├── keymaps/
│   └── default.c     # 默认键位表（4 层）
└── README.md         # 本文档
```

## 配置层级

```
┌─────────────────────────────────────────┐
│  #include "keyboards/kb904/config.h"    │  ← 应用代码入口
└──────────────────┬──────────────────────┘
                   │
┌──────────────────▼──────────────────────┐
│  config.h                                │
│  ├── 定义覆盖值（覆盖 defaults/）        │
│  ├── #include "defaults/*.h"            │
│  ├── #include "config_hw.h"             │
│  └── #include "config_check.h"          │
└──────────────────────────────────────────┘
```

## 快速定制

### 修改键位表
编辑 `keymaps/default.c`：
```c
const uint16_t keymap_layers[LAYER_MAX][MATRIX_ROWS][MATRIX_COLS] = {
    [LAYER_BASE] = {
        /* Row 0 */  KC_Q, KC_W, KC_E, KC_R, ...
    },
    // ...
};
```

### 修改 LED 配置
1. **LED 数量**：在 `config.h` 中修改 `IND_LED_COUNT`
2. **LED 引脚**：在 `config_hw.h` 中修改 `LED_*_PIN` 定义
3. **LED 硬件表**：在 `indicator.c` 中修改 `ind_led_table[]`

### 修改矩阵配置
在 `config_hw.h` 中修改：
```c
#define MATRIX_ROWS           8
#define MATRIX_COLS           16
#define MATRIX_ROW_PINS { B9, B8, B7, ... }
#define MATRIX_COL_PINS { B17, B16, B15, ... }
```

## 产品配置清单

| 配置项 | 文件 | 当前值 |
|--------|------|--------|
| `PRODUCT_ID` | config.h | 0x0904 |
| `IND_LED_COUNT` | config.h | 4 |
| `MATRIX_ROWS` | config_hw.h | 8 |
| `MATRIX_COLS` | config_hw.h | 16 |
| `BLE_HOST_COUNT` | config.h | 1 |

## 兼容层
以下旧文件仍可用，但会产生编译警告：
- `#include "product_config.h"` → 自动转发到 `kb904/config.h`
- `#include "indicator_config.h"` → 自动转发到 `kb904/config.h`

建议直接使用新的 include 路径。

## 迁移指南
从旧的 `product_config.h` 迁移到新架构：
1. 将 `#include "product_config.h"` 改为 `#include "keyboards/kb904/config.h"`
2. 将 `#include "indicator_config.h"` 改为 `#include "keyboards/kb904/config_hw.h"`
3. 编译验证
详见：`docs/plans/2026-03-19-keyboards-config-refactor-design.md`

## 硬件对照表
| LED | GPIO | 网络名 | 功能 |
|-----|------|--------|------|
| CAPS | PA15 | LED1 | 大写锁定 |
| BT | PB22 | LED2 | 蓝牙状态 |
| POWER | PB23 | LED3 | 电源/低电量 |
| CHARGE | PA14 | LED4 | 充满电 |
