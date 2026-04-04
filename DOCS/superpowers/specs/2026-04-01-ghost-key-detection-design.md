# 鬼键检测设计

> 日期: 2026-04-01
> 状态: 已批准
> 范围: keyboard.c 中 detect_ghost_key 函数实现

## 背景

键盘矩阵在没有二极管的按键交叉点会产生"幽灵键"现象：当多个按键同时按下时，矩阵会在未实际按下的交叉位置产生虚假信号。需要在按键处理流水线中增加检测环节，丢弃包含幽灵键的扫描周期。

## 当前流水线

```
matrix_scan()  →  raw_matrix[]  →  debounce()  →  matrix[]（已防抖）
                                                          ↓
keyboard.c: scan_and_debounce() 获取 matrix_debounced[row]
  → 现有 TODO 注释位置（第136行）预留了 detect_ghost_key() 调用
```

## 设计决策

### 1. 检测位置：防抖后

| 方案 | 优点 | 缺点 |
|------|------|------|
| 防抖前 | 更早拦截 | 机械抖动会产生瞬态假信号，误报率高 |
| **防抖后** | 消除抖动噪声，检测更准确 | 延迟约 DEBOUNCE_DELAY（5ms） |

选择防抖后的原因：
- 防抖消除了机械触点抖动产生的瞬态信号，避免将短暂的抖动误判为幽灵键
- 实际幽灵键现象是持续性的电气短路，防抖不会掩盖真正的幽灵键
- 延迟极小（5ms），用户无感知

### 2. 实现位置：keyboard.c 内 static 函数

在 `scan_and_debounce()` 中、防抖后、返回 `KEY_UPDATE` 之前调用。不新增文件，不修改驱动层接口。

### 3. 检测算法：2860 SDK 风格（行维度适配）

原始 2860 SDK 以列为主维度（`matrix[col]`，bit 代表行），当前项目以行为主维度（`matrix[row]`，bit 代表列）。适配后逻辑：

```
ghost_detect_position = 0
遍历每行 row：
  rowdata = matrix_debounced[row]
  如果 popcount_more_than_one(rowdata) 为真（≥2个bit置位）：
    如果 (ghost_detect_position & rowdata) != 0：
      检测到幽灵键 → 返回 true
    ghost_detect_position |= rowdata
返回 false
```

**原理**：幽灵键的本质是多行共享相同列时产生虚假交叉。算法累积已按下列的 bit mask，检测新行是否与已按下列存在交叉。

### 4. 检测到幽灵键的行为

- `scan_and_debounce()` 返回 `GHOST_KEY`
- `keyboard_scan()` 中已有的 `if (key_st == GHOST_KEY) return;` 丢弃本次变化
- 不更新 `matrix_previous`，不影响已按下的键状态
- `combo_task()` 和 `report_update_proc()` 已有 `GHOST_KEY` 处理分支

### 5. 辅助函数

`popcount_more_than_one(rowdata)`：使用 bit trick `rowdata &= rowdata - 1`，结果非零表示至少 2 个 bit 置位。

### 6. 配置

- 可通过 `#ifdef MATRIX_HAS_GHOST` 条件编译控制启用/禁用
- 检测函数始终编译，通过宏控制是否在 `scan_and_debounce()` 中调用
- `matrix_defaults.h` 中已定义 `MATRIX_GHOST_THRESHOLD`（保留供未来扩展）

## 不做的事

- 不修改 `matrix.c` 或 `debounce.c`
- 不新增文件
- 不修改 `key_update_st_t` 枚举
- 不修改 `combo_task()` 或 `report_update_proc()` 接口

## 受影响文件

| 文件 | 变更 |
|------|------|
| `middleware/keyboard/keyboard.c` | 添加 `detect_ghost_key()` static 函数，取消注释第136-138行 |

## 代码概要

```c
// 检查是否有超过1个bit被置位
static inline bool popcount_more_than_one(matrix_row_t rowdata) {
    rowdata &= rowdata - 1;
    return rowdata != 0;
}

// 鬼键检测（防抖后矩阵数据）
static bool detect_ghost_key(const matrix_row_t matrix[]) {
    matrix_row_t ghost_detect_position = 0;

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        matrix_row_t rowdata = matrix[row];
        if (popcount_more_than_one(rowdata)) {
            if ((ghost_detect_position & rowdata) != 0) {
                return true;  // 检测到幽灵键
            }
            ghost_detect_position |= rowdata;
        }
    }
    return false;
}
```

`scan_and_debounce()` 中的调用：

```c
static key_update_st_t scan_and_debounce(void) {
    bool debounced_changed = matrix_scan();
    // ...获取 matrix_debounced...

    if (!debounced_changed) {
        return NO_KEY_UPDATE;
    }

#ifdef MATRIX_HAS_GHOST
    if (detect_ghost_key(matrix_debounced)) {
        return GHOST_KEY;
    }
#endif

    return KEY_UPDATE;
}
```
