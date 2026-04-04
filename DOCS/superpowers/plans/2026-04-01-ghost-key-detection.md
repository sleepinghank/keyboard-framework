# 鬼键检测实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在 keyboard.c 中实现 detect_ghost_key 函数，检测防抖后矩阵中的幽灵键并丢弃对应扫描周期。

**Architecture:** 在 middleware/keyboard/keyboard.c 的 scan_and_debounce() 内添加 static 辅助函数 popcount_more_than_one() 和 detect_ghost_key()，防抖后、返回 KEY_UPDATE 之前调用。通过 MATRIX_HAS_GHOST 宏控制启用。

**Tech Stack:** 嵌入式 C，CH584M 平台，MounRiver Studio 工具链

---

## File Structure

| File | Action | Description |
|------|--------|-------------|
| `keyboards/defaults/matrix_defaults.h` | Modify | 添加 `MATRIX_HAS_GHOST` 默认宏定义 |
| `middleware/keyboard/keyboard.c` | Modify | 添加辅助函数 + 取消注释 ghost key 调用 |

---

### Task 1: 添加 MATRIX_HAS_GHOST 宏定义

**Files:**
- Modify: `keyboards/defaults/matrix_defaults.h:14-16`（在 `MATRIX_GHOST_THRESHOLD` 之后）

- [ ] **Step 1: 在 matrix_defaults.h 中添加 MATRIX_HAS_GHOST 默认定义**

在 `MATRIX_GHOST_THRESHOLD` 定义之后添加：

```c
/* ========== 鬼键检测 ========== */
#ifndef MATRIX_HAS_GHOST
#define MATRIX_HAS_GHOST
#endif
```

完整的修改区域变为：

```c
#ifndef MATRIX_GHOST_THRESHOLD
#define MATRIX_GHOST_THRESHOLD  3
#endif

/* ========== 鬼键检测 ========== */
#ifndef MATRIX_HAS_GHOST
#define MATRIX_HAS_GHOST
#endif

/* ========== 二极管方向 ========== */
```

- [ ] **Step 2: 编译验证**

Run: `cmd.exe /c "cd /d D:\\Code\\C_Project\\keyboard-framework\\project\\ch584m\\obj && set PATH=D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\components\\WCH\\Toolchain\\RISC-V Embedded GCC12\\bin;D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\others\\Build_Tools\\Make\\bin;%PATH% && make.exe main-build"`

Expected: 编译成功（添加了一个空宏定义，不影响现有代码）

- [ ] **Step 3: Commit**

```bash
git add keyboards/defaults/matrix_defaults.h
git commit -m "feat(matrix): add MATRIX_HAS_GHOST config macro"
```

---

### Task 2: 实现 detect_ghost_key 函数

**Files:**
- Modify: `middleware/keyboard/keyboard.c:118-142`（scan_and_debounce 函数区域）

- [ ] **Step 1: 在 scan_and_debounce() 上方添加辅助函数**

在 `// 矩阵扫描 + 驱动层防抖处理` 注释之前（第117行之后、第119行之前），插入：

```c
// 检查是否有超过1个bit被置位（bit trick：清除最低位后非零 = 至少2个bit）
static inline bool popcount_more_than_one(matrix_row_t rowdata) {
    rowdata &= rowdata - 1;
    return rowdata != 0;
}

// 鬼键检测（防抖后矩阵数据）
// 累积已按下列的 bit mask，检测多行是否共享相同列（幽灵键特征）
#ifdef MATRIX_HAS_GHOST
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
#endif
```

- [ ] **Step 2: 取消注释 scan_and_debounce() 中的 ghost key 调用**

将第136-139行的注释代码：

```c
    // TODO: 可选的幽灵键检测
    // if (detect_ghost_key(matrix_debounced)) {
    //     return GHOST_KEY;
    // }
```

替换为：

```c
    // 幽灵键检测
#ifdef MATRIX_HAS_GHOST
    if (detect_ghost_key(matrix_debounced)) {
        return GHOST_KEY;
    }
#endif
```

- [ ] **Step 3: 编译验证**

Run: `cmd.exe /c "cd /d D:\\Code\\C_Project\\keyboard-framework\\project\\ch584m\\obj && set PATH=D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\components\\WCH\\Toolchain\\RISC-V Embedded GCC12\\bin;D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\others\\Build_Tools\\Make\\bin;%PATH% && make.exe main-build"`

Expected: 编译成功，无新增 warning

- [ ] **Step 4: 功能验证**

烧录固件到 KB904 设备，测试以下场景：
1. 单键按下 → 正常响应
2. 两键同时按下（不同行不同列）→ 正常响应
3. 三键同时按下（构成幽灵键条件的对角组合）→ 应被丢弃，不产生键码
4. 所有按键释放后再次单键按下 → 正常恢复

- [ ] **Step 5: Commit**

```bash
git add middleware/keyboard/keyboard.c
git commit -m "feat(keyboard): implement ghost key detection after debounce"
```
