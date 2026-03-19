# keyboards/ 配置整合需求文档

> 创建日期: 2026-03-19
> 状态: Phase 1 完成，待进入 Phase 2

---

## 1. 需求背景

### 1.1 问题描述

当前 `./keyboards/` 目录下配置混乱，三个独立文件未整合到分层配置架构中：

```
keyboards/
├── defaults/           # ✅ 已有分层架构
├── kb904/              # ✅ 已有产品配置
├── default_keymap.c    # ⏳ 待整合
├── indicator_config.h  # ⏳ 待整合
└── product_config.h    # ⏳ 待整合
```

### 1.2 目标

将三个独立文件整合到多产品配置架构中，实现：
- 配置集中管理
- 方便维护和扩展
- 层级清晰，职责明确

---

## 2. 需求决策

| 问题 | 决策 |
|------|------|
| **产品范围** | 多产品架构，kb904 作为第一个实例 |
| **迁移策略** | 一次性迁移，更新所有引用 |
| **键位归属** | 产品级，移动到 `kb904/keymaps/` |
| **指示灯配置** | 分层设计（通用 `defaults/` + 产品 `kb904/`） |
| **验证要求** | 编译验证 |

---

## 3. 代码现状分析

### 3.1 现有架构

```
keyboards/
├── defaults/                    # 通用默认配置层
│   ├── backlight_defaults.h    # 背光默认值
│   ├── ble_defaults.h          # BLE 默认值
│   ├── indicator_defaults.h    # 指示灯默认值
│   ├── keyboard_defaults.h     # 键盘行为默认值
│   ├── power_defaults.h        # 电源管理默认值
│   └── matrix_defaults.h       # 矩阵扫描默认值
├── kb904/                       # KB904 产品专用配置
│   ├── config.h                 # 产品级配置（覆盖默认值）
│   ├── config_hw.h              # GPIO 硬件映射
│   └── README.md
├── keymaps/                     # 通用键位表头文件
│   ├── default_keymap.h
│   ├── layout_8x16.h
│   └── layout_jis_113.h
└── config_check.h               # 编译时配置验证
```

### 3.2 待整合文件分析

#### 3.2.1 product_config.h

| 属性 | 值 |
|------|-----|
| **职责** | 硬件/功能总配置入口 |
| **行数** | 166 行 |
| **引用数** | 16 个文件 |
| **关键定义** | `PRODUCT_ID`, `MATRIX_ROWS/COLS`, `MATRIX_ROW_PINS`, `DEBOUNCE`, `CHIP_TYPE_CH584` |
| **迁移目标** | 废弃，内容分散到 kb904/config.h 和 defaults/ |

#### 3.2.2 indicator_config.h

| 属性 | 值 |
|------|-----|
| **职责** | LED 指示灯配置（混合配置覆盖 + 硬件表） |
| **引用数** | 11 个文件 |
| **关键定义** | `ind_led_def_t`, `ind_led_table[]`, `LED_CAPS/BT/POWER_*` 别名 |
| **问题** | 混合了两层职责 |
| **迁移目标** | 配置覆盖 → kb904/config.h，硬件表 → kb904/indicator_hw.h |

#### 3.2.3 default_keymap.c

| 属性 | 值 |
|------|-----|
| **职责** | 键盘映射表实现 |
| **关键定义** | `keymap_layers[LAYER_MAX][8][16]`，4 层键码 |
| **依赖** | keymap.h → product_config.h |
| **迁移目标** | 移动到 kb904/keymaps/ |

### 3.3 依赖关系

```
product_config.h (16 引用)
    ↓
├── indicator_config.h (11 引用)
│       └── 依赖: product_config.h, gpio.h, kb904/config_hw.h
│
└── default_keymap.c
        └── 间接依赖: MATRIX_ROWS/COLS, LAYER_MAX
```

### 3.4 覆盖机制

```
产品 config.h 先定义覆盖值 → #include defaults/*.h → #ifndef 保护使覆盖值生效
```

**关键**: 覆盖值必须在 include defaults 之前定义。

---

## 4. 需求评审意见

### 4.1 用户/产品视角

| 风险 | 建议 |
|------|------|
| 引用统计需核实 | 确认实际引用数量 |
| 键位路径处理未明确 | 明确 include 路径变更方案 |
| 多产品编译切换缺失 | CMake 添加 PRODUCT 变量支持 |

**评价**: 需修订 - 执行细节存在遗漏

### 4.2 架构/编码视角

| 风险 | 建议 |
|------|------|
| product_config.h 与 kb904/config.h 重复定义 | 迁移前先清理重复配置 |
| 覆盖机制不完整 | 确保 config.h 包含顺序正确 |

**评价**: 需修订 - 存在冗余配置

### 4.3 测试/质量视角

| 风险 | 建议 |
|------|------|
| 仅编译验证不足 | 增加静态断言检查 |
| 引用语义一致性 | 迁移前后二进制对比 |

**评价**: 需修订 - 建议增加配置完整性检查

---

## 5. 范围边界

### 5.1 在范围内

- [x] 整合 3 个独立文件到现有架构
- [x] 更新所有 include 引用
- [x] 确保编译通过
- [x] 废弃 product_config.h

### 5.2 范围外（后续处理）

- [ ] CMake 多产品编译切换（PRODUCT 变量）
- [ ] 新增静态断言检查
- [ ] 运行时烟雾测试
- [ ] 创建其他产品目录模板

---

## 6. 迁移顺序建议

基于依赖关系，建议迁移顺序：

1. **product_config.h** → 无外部依赖，优先处理
2. **indicator_config.h** → 依赖 product_config.h
3. **default_keymap.c** → 间接依赖 product_config.h

---

## 7. 检查点

- [x] 步骤 1: 探索项目上下文
- [x] 步骤 2: 需求澄清与 Agent 评审
- [x] 步骤 3: 代码现状分析
- [ ] 步骤 4-6: 方案设计（Phase 2）
- [ ] 步骤 7-9: 计划输出（Phase 3）
