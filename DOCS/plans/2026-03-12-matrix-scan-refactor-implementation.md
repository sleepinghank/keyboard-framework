# 矩阵扫描重构实现计划

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 将矩阵扫描从 OSAL 事件调度改为 main loop 标志位执行，减少调度压力和延迟。

**Architecture:** 硬件定时器中断设置 volatile 标志位，main loop 直接检查并执行 keyboard_task()，OSAL_SystemProcess 改为单次执行函数。

**Tech Stack:** CH584 RISC-V, TMOS BLE 协议栈, C 语言

---

## Task 1: 添加 OSAL 单次执行函数

**Files:**
- Modify: `drivers/system/event_manager.h:96` (在 OSAL_SystemProcess 声明后添加)
- Modify: `drivers/system/platforms/ch584/event_manager.c:147` (在 OSAL_SystemProcess 前添加)

**Step 1: 在 event_manager.h 添加函数声明**

在 `void OSAL_SystemProcess(void);` 声明后添加：

```c
/**
 * @brief   OSAL 系统单次处理（非阻塞）
 *          处理一轮事件后返回，用于自定义主循环
 */
void OSAL_SystemProcessOnce(void);
```

**Step 2: 在 event_manager.c 添加函数实现**

在 `OSAL_SystemProcess` 函数前添加：

```c
/**
 * @brief   OSAL 系统单次处理（非阻塞）
 *          处理一轮事件后返回，用于自定义主循环
 */
void OSAL_SystemProcessOnce(void) {
    TMOS_SystemProcess();
}
```

**Step 3: 编译验证**

```bash
cmd.exe /c "cd /d D:\Code\C_Project\keyboard-framework\project\ch584m\obj && make.exe main-build"
```

Expected: 编译成功，无错误

**Step 4: Commit**

```bash
git add drivers/system/event_manager.h drivers/system/platforms/ch584/event_manager.c
git commit -m "feat(event_manager): add OSAL_SystemProcessOnce for custom main loop"
```

---

## Task 2: 添加矩阵扫描标志位接口

**Files:**
- Modify: `application/service/input_service.h:34` (移除 INPUT_MATRIX_SCAN_EVT，添加新接口)
- Modify: `application/service/input_service.c:41` (添加标志位变量和接口实现)

**Step 1: 在 input_service.h 移除事件定义并添加接口**

修改 `input_task_event_t` 枚举：

```c
typedef enum {
    // INPUT_MATRIX_SCAN_EVT     = 1 << 0,  // 移除：改用 main loop 标志位
    INPUT_MATRIX_CHANGED_EVT  = 1 << 1,  /**< 矩阵数据变化事件 */
    INPUT_TOUCH_INT_EVT       = 1 << 2,  /**< 触控中断事件 */
    INPUT_BATTERY_DETE_EVT    = 1 << 3,  /**< 电量变化事件 */
    INPUT_TOUCH_WATCHDOG_EVT  = 1 << 4,  /**< 触控板看门狗检查事件 */
} input_task_event_t;
```

在 `input_service_init()` 声明后添加：

```c
/*==========================================
 * 矩阵扫描标志位接口
 *=========================================*/

/**
 * @brief 检查是否需要执行矩阵扫描
 * @return true 需要执行, false 不需要
 */
bool input_get_matrix_scan_flag(void);

/**
 * @brief 清除矩阵扫描标志位
 */
void input_clear_matrix_scan_flag(void);
```

**Step 2: 在 input_service.c 添加标志位变量**

在 `uint8_t input_taskID = 0;` 后添加：

```c
/* 矩阵扫描标志位 - volatile 保证中断可见性 */
static volatile bool g_matrix_scan_flag = false;
```

**Step 3: 在 input_service.c 添加接口实现**

在 `matrix_scan_timer_stop()` 函数后添加：

```c
/*==========================================
 * 矩阵扫描标志位接口实现
 *=========================================*/

bool input_get_matrix_scan_flag(void) {
    return g_matrix_scan_flag;
}

void input_clear_matrix_scan_flag(void) {
    g_matrix_scan_flag = false;
}
```

**Step 4: 修改 matrix_scan_timer_callback**

将回调函数从 `OSAL_SetEvent` 改为设置标志位：

```c
__HIGH_CODE
static void matrix_scan_timer_callback(void)
{
    g_matrix_scan_flag = true;
}
```

**Step 5: 移除 input_process_event 中的 INPUT_MATRIX_SCAN_EVT 处理**

删除以下代码块：

```c
    // 处理矩阵扫描事件
    if (events & INPUT_MATRIX_SCAN_EVT) {
        // keyboard_task();
        // dprintln("1*");
        togglePin(B14);  /* 调试: 验证 TMR1 中断触发 */
#ifdef TOUCH_EN
        // 检查触摸板状态，如有触摸中断则设置事件
        if (touch_timer_task() > 0) {
            OSAL_SetEvent(task_id, INPUT_TOUCH_INT_EVT);
        }
#endif
        return (events ^ INPUT_MATRIX_SCAN_EVT);
    }
```

**Step 6: 编译验证**

```bash
cmd.exe /c "cd /d D:\Code\C_Project\keyboard-framework\project\ch584m\obj && make.exe main-build"
```

Expected: 编译成功，无错误

**Step 7: Commit**

```bash
git add application/service/input_service.h application/service/input_service.c
git commit -m "refactor(input_service): replace INPUT_MATRIX_SCAN_EVT with flag-based execution"
```

---

## Task 3: 重构 main.c 主循环

**Files:**
- Modify: `application/main.c:1-39`

**Step 1: 添加 input_service.h 头文件包含**

在 `#include "debug.h"` 后添加：

```c
#include "input_service.h"
```

**Step 2: 重构主循环**

将 `OSAL_SystemProcess();` 替换为新的主循环结构：

```c
    // 主循环：矩阵扫描 + OSAL 事件处理
    while (1) {
        /* 检查矩阵扫描标志位 */
        if (input_get_matrix_scan_flag()) {
            keyboard_task();
            input_clear_matrix_scan_flag();
        }

        /* OSAL 单次处理（BLE 协议栈等） */
        OSAL_SystemProcessOnce();
    }
```

完整的 main 函数应为：

```c
int main(void)
{
    // 调用系统初始化协调器
    uint32_t init_result = system_init_coordinator();
    if (init_result != 0) {
#if (PRINTF_ENABLE == TRUE && PRINTF_LEVEL >= PRINTF_LEVEL_ERROR)
        dprintf("ERROR: System initialization failed! Error code: %d\r\n", init_result);
#endif
        return init_result;
    }
    dprint("System initialized successfully");

    // 主循环：矩阵扫描 + OSAL 事件处理
    while (1) {
        /* 检查矩阵扫描标志位 */
        if (input_get_matrix_scan_flag()) {
            keyboard_task();
            input_clear_matrix_scan_flag();
        }

        /* OSAL 单次处理（BLE 协议栈等） */
        OSAL_SystemProcessOnce();
    }

    return 0;
}
```

**Step 3: 编译验证**

```bash
cmd.exe /c "cd /d D:\Code\C_Project\keyboard-framework\project\ch584m\obj && make.exe main-build"
```

Expected: 编译成功，无错误

**Step 4: Commit**

```bash
git add application/main.c
git commit -m "refactor(main): use flag-based matrix scan in custom main loop"
```

---

## Task 4: 更新 spec 文档

**Files:**
- Modify: `openspec/specs/matrix-scan-timer/spec.md`

**Step 1: 更新 spec 文档**

将原内容替换为：

```markdown
# matrix-scan-timer Specification

## Purpose
使用硬件定时器触发矩阵扫描，通过 main loop 标志位执行，减少 OSAL 调度压力和延迟。

## Requirements

### Requirement: 硬件定时器触发矩阵扫描标志位

系统 SHALL 使用硬件定时器以固定周期设置矩阵扫描标志位。

#### Scenario: 正常扫描周期

- **WHEN** 系统初始化完成且输入服务启动
- **THEN** 硬件定时器以 5ms 周期运行
- **AND** 每次定时器中断设置 `g_matrix_scan_flag` 标志位
- **AND** main loop 检测到标志位后执行 `keyboard_task()`

#### Scenario: 标志位访问

- **WHEN** 定时器中断设置标志位
- **THEN** main loop 通过 `input_get_matrix_scan_flag()` 读取标志位
- **AND** 执行完成后通过 `input_clear_matrix_scan_flag()` 清除标志位
- **AND** 标志位使用 `volatile` 保证中断可见性

### Requirement: 扫描周期可配置

系统 SHALL 支持通过宏定义配置矩阵扫描周期。

#### Scenario: 默认扫描周期

- **WHEN** 未定义 `MATRIX_SCAN_INTERVAL_MS` 宏
- **THEN** 使用默认值 5ms 作为扫描周期

#### Scenario: 自定义扫描周期

- **WHEN** 在 `product_config.h` 中定义 `MATRIX_SCAN_INTERVAL_MS` 为 10
- **THEN** 系统以 10ms 周期执行矩阵扫描

### Requirement: 低功耗模式支持

系统 SHALL 支持在低功耗模式下停止矩阵扫描定时器。

#### Scenario: 进入低功耗模式

- **WHEN** 系统进入低功耗模式
- **THEN** 可调用 `matrix_scan_timer_stop()` 停止定时器
- **AND** 减少功耗

#### Scenario: 唤醒恢复扫描

- **WHEN** 系统从低功耗模式唤醒
- **THEN** 可调用 `matrix_scan_timer_start()` 恢复定时器
- **AND** 矩阵扫描恢复正常周期运行
```

**Step 2: Commit**

```bash
git add openspec/specs/matrix-scan-timer/spec.md
git commit -m "docs(spec): update matrix-scan-timer spec for flag-based execution"
```

---

## Task 5: 最终验证

**Step 1: 完整编译**

```bash
cmd.exe /c "cd /d D:\Code\C_Project\keyboard-framework\project\ch584m\obj && make.exe clean && make.exe main-build"
```

Expected: 编译成功，生成 keyboard-framework.hex

**Step 2: 检查改动文件**

```bash
git status
git diff --stat
```

Expected: 5 个文件被修改

**Step 3: 查看提交历史**

```bash
git log --oneline -5
```

Expected: 4 个新提交

---

## Summary

| Task | Description | Files |
|------|-------------|-------|
| 1 | 添加 OSAL_SystemProcessOnce | event_manager.h, event_manager.c |
| 2 | 添加矩阵扫描标志位接口 | input_service.h, input_service.c |
| 3 | 重构 main.c 主循环 | main.c |
| 4 | 更新 spec 文档 | spec.md |
| 5 | 最终验证 | - |
