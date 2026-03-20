# Keyboard Middleware 按键流程架构分析

## 概述

本文档分析 `middleware/keyboard` 目录下的键盘按键处理流程实现，包括架构设计、数据流向、核心模块职责以及优缺点分析。该实现主要参考 QMK 固件的按键处理管线。

## 目录结构

```
middleware/keyboard/
├── keyboard.c/h           # 键盘主模块，协调整体流程
├── action.c/h             # 动作处理核心
├── action_layer.c/h       # 层级管理
├── action_util.c/h        # 动作工具函数、修饰键管理
├── action_code.h          # 动作码定义
├── keymap_common.c/h      # 键映射转换
├── keymap_introspection.c/h # 键映射内省
├── report.c/h             # HID 报告生成
├── combo.c/h              # 组合键处理
├── custom_function.c/h    # 自定义功能（Tap Dance、Leader Key、Macro）
├── keycode.h              # 键码定义
├── keycodes.h             # 扩展键码
├── quantum_keycodes.h     # QMK 量子键码
├── keycode_config.c/h     # 键码配置
├── modifiers.h            # 修饰键定义
└── keymap_extras/         # 国际键盘布局支持
```

## 按键处理流程

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           按键处理管线                                        │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────────────────┐  │
│  │ 矩阵扫描  │───▶│ 硬件防抖  │───▶│ 状态变化  │───▶│ 层级映射/Action获取  │  │
│  │ matrix   │    │ debounce │    │ 检测     │    │ layer_switch_get_   │  │
│  │ _scan()  │    │          │    │          │    │ action()            │  │
│  └──────────┘    └──────────┘    └──────────┘    └──────────────────────┘  │
│       │                               │                    │               │
│       ▼                               ▼                    ▼               │
│  raw_matrix[]               matrix_previous[]         action_t            │
│                                   比较                                     │
│                                                                             │
│  ┌──────────────────────┐    ┌──────────────────┐    ┌─────────────────┐   │
│  │ 组合键处理(可选)       │───▶│ Tap/Hold 处理    │───▶│ 动作执行         │   │
│  │ process_combo()      │    │ process_action_  │    │ register_code() │   │
│  │                      │    │ tapping()        │    │ layer_on/off()  │   │
│  └──────────────────────┘    └──────────────────┘    └─────────────────┘   │
│                                                              │              │
│                                                              ▼              │
│                                                     ┌─────────────────┐    │
│                                                     │ HID 报告生成     │    │
│                                                     │ send_keyboard_  │    │
│                                                     │ report()        │    │
│                                                     └─────────────────┘    │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 核心模块分析

### 1. 键盘主模块 (`keyboard.c/h`)

**职责**：
- 系统初始化协调
- 主循环任务调度
- 矩阵状态变化检测
- 幽灵键过滤

**关键函数**：

```c
// 初始化
void keyboard_init(void) {
    debounce_init(MATRIX_ROWS);
    combo_init();           // 可选
    tap_dance_init();       // 可选
    leader_init();          // 可选
    macro_init();           // 可选
    matrix_init();
}

// 主任务循环
void keyboard_task(void) {
    matrix_task();          // 扫描并处理
    combo_task();           // 组合键定时处理
    tap_dance_task();       // Tap Dance 定时处理
    leader_task();          // Leader Key 定时处理
    macro_task();           // 宏定时处理
    housekeeping_task();    // 后台任务
}
```

**幽灵键检测** (`has_ghost_in_row()`):
- 检测同一行是否有多个按键与其他行共享列线
- 防止矩阵电气串扰导致的误触发

### 2. 动作处理模块 (`action.c/h`)

**职责**：
- 键事件到动作的转换
- Tap/Hold 逻辑处理
- 键码注册/注销
- 修饰键组合处理

**核心数据结构**：

```c
// 键事件
typedef struct {
    keypos_t        key;      // 行列位置
    uint16_t        time;     // 时间戳
    keyevent_type_t type;     // 事件类型
    bool            pressed;  // 按下/释放
} keyevent_t;

// 键记录
typedef struct {
    keyevent_t event;
    tap_t      tap;           // tap 计数和状态
    uint16_t   keycode;       // 组合键用
} keyrecord_t;

// 动作码（16位）
typedef union {
    uint16_t code;
    struct action_kind { ... } kind;
    struct action_key { ... } key;
    struct action_layer_tap { ... } layer_tap;
    // ...
} action_t;
```

**动作类型** (`action_kind_id`):
| 动作类型 | 编码 | 描述 |
|---------|------|------|
| ACT_LMODS | 0b0000 | 左修饰键 |
| ACT_RMODS | 0b0001 | 右修饰键 |
| ACT_LMODS_TAP | 0b0010 | 左修饰键+Tap |
| ACT_RMODS_TAP | 0b0011 | 右修饰键+Tap |
| ACT_USAGE | 0b0100 | 系统/消费者控制 |
| ACT_MOUSEKEY | 0b0101 | 鼠标键 |
| ACT_SWAP_HANDS | 0b0110 | 左右手交换 |
| ACT_LAYER | 0b1000 | 层级位操作 |
| ACT_LAYER_MODS | 0b1001 | 层级+修饰键 |
| ACT_LAYER_TAP | 0b1010 | 层级 Tap (0-15) |
| ACT_LAYER_TAP_EXT | 0b1011 | 层级 Tap (16-31) |

**Tap/Hold 处理流程**：
```c
void process_action_tapping(keyrecord_t *record, action_t action) {
    // 1. 记录按下时间
    // 2. 释放时计算持续时间
    // 3. 短于 TAPPING_TERM -> Tap
    // 4. 长于 TAPPING_TERM -> Hold
    // 5. 根据 action 类型执行相应操作
}
```

### 3. 层级管理模块 (`action_layer.c/h`)

**职责**：
- 默认层和动态层管理
- 层级状态位图操作
- 层级缓存（防止释放时层级变化导致卡键）

**层级状态**：
```c
layer_state_t default_layer_state = 0;  // 默认层（持久）
layer_state_t layer_state = 0;          // 动态层（临时）

// 根据配置选择 8/16/32 位
#if defined(LAYER_STATE_8BIT)
    typedef uint8_t layer_state_t;   // 最多 8 层
#elif defined(LAYER_STATE_16BIT)
    typedef uint16_t layer_state_t;  // 最多 16 层（默认）
#elif defined(LAYER_STATE_32BIT)
    typedef uint32_t layer_state_t;  // 最多 32 层
#endif
```

**层级查找策略**：
```c
uint8_t layer_switch_get_layer(keypos_t key) {
    layer_state_t layers = layer_state | default_layer_state;
    // 从最高层向下查找
    for (int8_t i = MAX_LAYER - 1; i >= 0; i--) {
        if (layers & ((layer_state_t)1 << i)) {
            action_t action = action_for_key(i, key);
            if (action.code != ACTION_TRANSPARENT) {
                return i;  // 找到非透明动作
            }
        }
    }
    return 0;  // 回退到层 0
}
```

**层级缓存机制**：
- `source_layers_cache[][]` 记录每个键按下时所在层级
- 释放时使用缓存的层级查找动作
- 防止按住键时切换层级导致释放动作不匹配

### 4. 键映射模块 (`keymap_common.c/h`)

**职责**：
- 键位到键码的转换
- 键码到动作的转换
- 支持各种 QMK 键码类型

**键码范围**：
```c
switch (keycode) {
    case BASIC_KEYCODE_RANGE:      // 基础键码 (A-Z, 0-9, etc.)
    case MODIFIER_KEYCODE_RANGE:   // 修饰键
    case SYSTEM_KEYCODE_RANGE:     // 系统控制
    case CONSUMER_KEYCODE_RANGE:   // 媒体控制
    case MOUSE_KEYCODE_RANGE:      // 鼠标键
    case QK_MODS ... QK_MODS_MAX:  // 修饰键组合
    case QK_LAYER_TAP ... :        // 层级 Tap
    case QK_MOMENTARY ... :        // 瞬时层
    case QK_MOD_TAP ... :          // 修饰键 Tap
    // ...
}
```

### 5. 报告生成模块 (`report.c/h`, `action_util.c/h`)

**职责**：
- HID 键盘报告构建
- 6KRO/NKRO 模式支持
- 修饰键状态管理
- OneShot 功能实现

**报告结构**：
```c
// 6KRO 报告 (8 字节)
typedef struct {
    uint8_t mods;
    uint8_t reserved;
    uint8_t keys[6];
} report_keyboard_t;

// NKRO 报告 (位图)
typedef struct {
    uint8_t report_id;
    uint8_t mods;
    uint8_t bits[30];  // 240 位，支持所有键码
} report_nkro_t;
```

**修饰键分类**：
- `real_mods`: 物理按下的修饰键
- `weak_mods`: 弱修饰键（发送后自动清除）
- `oneshot_mods`: 单次修饰键（下一个键后清除）
- `oneshot_locked_mods`: 锁定的单次修饰键

### 6. 组合键模块 (`combo.c/h`)

**职责**：
- 多键同时按下触发组合
- 时间窗口控制
- 重叠组合处理

**数据结构**：
```c
typedef struct combo_t {
    const uint16_t *keys;   // 组合键列表
    uint16_t keycode;       // 触发的键码
    bool disabled;
    bool active;
    uint8_t state;          // 已按下的键位图
} combo_t;
```

**处理流程**：
1. 按键按下 -> 检查是否为组合键成员
2. 缓存按键事件
3. 所有组合键按下 -> 触发组合
4. 超时或非组合键 -> 释放缓存的按键

## 架构优点

### 1. 模块化设计
- **清晰的职责分离**：每个模块有明确的单一职责
- **低耦合**：模块间通过定义良好的接口通信
- **易于扩展**：新功能可以作为独立模块添加

### 2. 功能完备
- **完整的 QMK 兼容性**：支持大部分 QMK 键码和功能
- **丰富的层级操作**：MO、TO、TG、OSL、LT 等
- **高级按键功能**：Tap Dance、Leader Key、Combo、Macro
- **修饰键处理**：OneShot、Weak Mods、Locked Mods

### 3. 可配置性强
- **编译时配置**：通过宏定义启用/禁用功能
- **运行时配置**：`keymap_config_t` 支持动态配置
- **多算法选择**：防抖算法、NKRO 模式等可选

### 4. 资源优化
- **按需编译**：未使用的功能不占用空间
- **层级缓存**：位压缩存储减少内存占用
- **报告优化**：只在变化时发送报告

### 5. 回调钩子丰富
- `layer_state_set_user/kb()`：层级变化回调
- `oneshot_*_changed_user/kb()`：OneShot 状态回调
- `process_combo_event()`：组合键事件回调
- `action_keypress/keyrelease()`：按键事件钩子

## 架构缺点

### 1. 代码复杂度高
- **大量条件编译**：代码中充斥 `#ifdef`，难以阅读
- **嵌套深度**：`process_action()` 函数 switch-case 嵌套过深
- **函数过长**：部分核心函数超过 200 行

**示例问题**：
```c
// action.c:process_action() 中的嵌套
switch (action.kind.id) {
    case ACT_LMODS_TAP:
        switch (action.layer_tap.code) {
            case MODS_ONESHOT:
                if (!keymap_config.oneshot_enable) {
                    // ... 100+ 行
                } else {
                    // ... 100+ 行
                }
                break;
            // ... 更多 case
        }
        break;
    // ... 更多 case
}
```

### 2. 重复代码
- **process_action() 与 process_action_tapping()**：功能重叠
- **多处 tap 计时逻辑**：分散在不同位置
- **层级操作**：`layer_on/off/invert` 与位操作重复

### 3. 状态管理分散
- **全局变量过多**：`layer_state`、`real_mods`、`weak_mods` 等
- **状态分散存储**：键盘状态、层级状态、修饰键状态各自独立
- **缺乏统一的状态管理器**

### 4. 内存效率问题
- **层级缓存开销**：`source_layers_cache[MATRIX_ROWS * MATRIX_COLS][MAX_LAYER_BITS]`
- **组合键缓冲区**：固定大小的静态数组
- **小型 MCU 资源紧张**

**内存占用估算**：
```
假设 8x13 矩阵，16 层：
层级缓存: 104 * 4 = 416 bytes
组合键缓冲: ~200 bytes
报告缓冲: ~50 bytes
```

### 5. Tap/Hold 检测不精确
- **简化的实现**：当前实现在按键按下时就判断
- **标准 QMK**：使用状态机跟踪，在释放或超时时决定
- **可能导致**：快速打字时误判 Hold

**当前问题代码**：
```c
void process_action_tapping(keyrecord_t *record, action_t action) {
    if (event.pressed) {
        // 问题：按下时 is_tap 总是 false，因为刚记录时间
        key_press_time[row][col] = timer_read32();
        // ...
        if (is_tap) {  // 永远不会在按下时为 true
            register_code(action.layer_tap.code);
        }
    }
}
```

### 6. 文档和注释不足
- **缺少设计文档**：模块间关系不清晰
- **注释过时**：部分 FIXME 注释未处理
- **API 文档缺失**：公共接口缺少详细说明

### 7. 测试覆盖不足
- **单元测试有限**：主要在 main.c 中的简单测试
- **缺少边界测试**：层级边界、组合键重叠等
- **缺少集成测试**：完整流程测试

### 8. 平台抽象不完整
- **register_code() 未实现**：只有日志输出
- **send_keyboard_report() 未连接**：报告未发送到主机
- **host_* 函数缺失**：与传输层未对接

## 改进建议

### 短期改进

1. **完善 Tap/Hold 状态机**
```c
typedef enum {
    TAPPING_IDLE,
    TAPPING_PRESSED,
    TAPPING_HOLDING,
    TAPPING_RELEASED
} tapping_state_t;
```

2. **实现 register_code 实际功能**
```c
void register_code(uint8_t code) {
    add_key_to_report(code);
    send_keyboard_report();
}
```

3. **添加状态管理器**
```c
typedef struct {
    layer_state_t layers;
    uint8_t mods;
    uint8_t weak_mods;
    uint8_t oneshot_mods;
} keyboard_state_t;
```

### 中期改进

1. **重构 process_action()**：拆分为小函数
2. **统一 tap 处理逻辑**：合并重复代码
3. **添加完整的单元测试**

### 长期改进

1. **引入事件驱动架构**
2. **支持动态键映射**
3. **优化内存使用**

## 数据流图

```
                    ┌─────────────────┐
                    │   Hardware      │
                    │   GPIO Matrix   │
                    └────────┬────────┘
                             │ matrix_scan()
                             ▼
                    ┌─────────────────┐
                    │   raw_matrix[]  │
                    └────────┬────────┘
                             │ debounce()
                             ▼
                    ┌─────────────────┐
                    │   matrix[]      │
                    │   (debounced)   │
                    └────────┬────────┘
                             │ matrix_task()
                             ▼
                    ┌─────────────────┐
                    │   keyevent_t    │
                    │   {row,col,     │
                    │    pressed,time}│
                    └────────┬────────┘
                             │ action_exec()
                             ▼
              ┌──────────────┴──────────────┐
              │                             │
              ▼                             ▼
    ┌─────────────────┐           ┌─────────────────┐
    │ process_combo() │           │ layer_switch_   │
    │  (if enabled)   │           │ get_action()    │
    └────────┬────────┘           └────────┬────────┘
             │                              │
             └──────────────┬───────────────┘
                            ▼
                   ┌─────────────────┐
                   │   action_t      │
                   │   {kind,params} │
                   └────────┬────────┘
                            │ process_action_tapping()
                            ▼
                   ┌─────────────────┐
                   │ register_code() │
                   │ layer_on/off()  │
                   │ add/del_mods()  │
                   └────────┬────────┘
                            │
                            ▼
                   ┌─────────────────┐
                   │ keyboard_report │
                   │ {mods, keys[6]} │
                   └────────┬────────┘
                            │ send_keyboard_report()
                            ▼
                   ┌─────────────────┐
                   │   USB/BLE/2.4G  │
                   │   Transport     │
                   └─────────────────┘
```

## 总结

当前 `middleware/keyboard` 的实现是一个功能完备、高度可配置的键盘处理管线，成功移植了 QMK 的核心功能。其模块化设计和丰富的功能支持是主要优点，但代码复杂度高、Tap/Hold 实现不完整、以及与传输层未对接是需要优先解决的问题。

**优先级排序**：
1. 🔴 高：完善 Tap/Hold 状态机
2. 🔴 高：实现 register_code 与传输层对接
3. 🟡 中：重构 process_action() 降低复杂度
4. 🟡 中：添加完整单元测试
5. 🟢 低：优化内存占用
6. 🟢 低：完善文档

---

*分析日期: 2026-01-23*
*分析版本: keyboard-framework master branch*d
