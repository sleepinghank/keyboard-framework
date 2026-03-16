# Matrix Scan Main Loop Flag Migration Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 将矩阵扫描触发从 OSAL 事件迁移为 main loop 标志位执行，降低调度压力并缩短触发路径。

**Architecture:** 保留现有分层与服务初始化链路，仅在 input service、main loop、event manager 边界做最小修改。通过新增 OSAL 单步处理接口打破 `OSAL_SystemProcess()` 的内部死循环阻塞，使 main loop 能够按轮询顺序执行“扫描标志消费 + OSAL 一轮处理”。

**Tech Stack:** C11, CH584 TMOS/OSAL 适配层, CMake (MinGW), 现有 keyboard middleware。

---

### Task 1: 清理矩阵扫描事件定义并新增标志位接口

**Files:**
- Modify: `application/service/input_service.h`

**Step 1: 写最小“失败检查”基线（静态检查）**

Run: `grep -n "INPUT_MATRIX_SCAN_EVT" "application/service/input_service.h"`
Expected: 至少 1 处命中（迁移前基线）。

**Step 2: 修改头文件接口**

- 从 `input_task_event_t` 删除 `INPUT_MATRIX_SCAN_EVT`。
- 新增接口声明：
  - `bool input_matrix_scan_pending(void);`
  - `void input_matrix_scan_clear_pending(void);`

**Step 3: 运行静态检查确认更新生效**

Run: `grep -n "INPUT_MATRIX_SCAN_EVT" "application/service/input_service.h"`
Expected: 无命中。

Run: `grep -n "input_matrix_scan_pending\|input_matrix_scan_clear_pending" "application/service/input_service.h"`
Expected: 新接口声明出现。

---

### Task 2: input_service 改为“中断置位 + main loop 消费”模型

**Files:**
- Modify: `application/service/input_service.c`

**Step 1: 写最小“失败检查”基线**

Run: `grep -n "OSAL_SetEvent(input_taskID, INPUT_MATRIX_SCAN_EVT)\|INPUT_MATRIX_SCAN_EVT" "application/service/input_service.c"`
Expected: 命中回调与事件处理分支。

**Step 2: 实现最小改动代码**

- 增加 `static volatile bool g_matrix_scan_pending = false;`
- 在 `matrix_scan_timer_callback()` 中置位标志，不再 `OSAL_SetEvent(...INPUT_MATRIX_SCAN_EVT)`。
- 实现：
  - `bool input_matrix_scan_pending(void)`
  - `void input_matrix_scan_clear_pending(void)`
- 删除 `input_process_event()` 中 `INPUT_MATRIX_SCAN_EVT` 分支。

**Step 3: 运行静态检查验证**

Run: `grep -n "INPUT_MATRIX_SCAN_EVT" "application/service/input_service.c"`
Expected: 无命中。

Run: `grep -n "g_matrix_scan_pending\|input_matrix_scan_pending\|input_matrix_scan_clear_pending" "application/service/input_service.c"`
Expected: 标志位与接口实现存在。

---

### Task 3: 增加 OSAL 单步处理接口

**Files:**
- Modify: `drivers/system/event_manager.h`
- Modify: `drivers/system/platforms/ch584/event_manager.c`

**Step 1: 写最小“失败检查”基线**

Run: `grep -n "OSAL_SystemProcessOnce" "drivers/system/event_manager.h" "drivers/system/platforms/ch584/event_manager.c"`
Expected: 无命中。

**Step 2: 增加接口与实现**

- 在 `event_manager.h` 增加声明：`void OSAL_SystemProcessOnce(void);`
- 在 CH584 平台 `event_manager.c` 中实现：
  - `OSAL_SystemProcessOnce()` 仅调用一次 `TMOS_SystemProcess()`。
- 保持 `OSAL_SystemProcess()` 现有行为不变（兼容原调用点）。

**Step 3: 运行静态检查验证**

Run: `grep -n "OSAL_SystemProcessOnce" "drivers/system/event_manager.h" "drivers/system/platforms/ch584/event_manager.c"`
Expected: 声明和定义均存在。

---

### Task 4: main loop 接入扫描标志消费逻辑

**Files:**
- Modify: `application/main.c`

**Step 1: 写最小“失败检查”基线**

Run: `grep -n "OSAL_SystemProcess();" "application/main.c"`
Expected: 存在单次调用。

**Step 2: 最小实现**

- 引入 `input_service.h`、`keyboard.h`。
- 将主流程改为显式 `while (1)`：
  1. `if (input_matrix_scan_pending()) { input_matrix_scan_clear_pending(); keyboard_task(); }`
  2. `OSAL_SystemProcessOnce();`

**Step 3: 运行静态检查验证**

Run: `grep -n "input_matrix_scan_pending\|keyboard_task\|OSAL_SystemProcessOnce\|while (1)" "application/main.c"`
Expected: 新循环逻辑与调用出现。

---

### Task 5: 交叉验证与构建验证

**Files:**
- Verify only

**Step 1: 全局残留检查**

Run: `grep -R -n "INPUT_MATRIX_SCAN_EVT" application drivers middleware`
Expected: 无残留（若仅保留注释需清理）。

**Step 2: 依赖一致性检查**

Run: `grep -R -n "input_matrix_scan_pending\|input_matrix_scan_clear_pending\|OSAL_SystemProcessOnce" application drivers`
Expected: 声明/定义/调用链完整。

**Step 3: 编译验证**

Run:

```bash
cmake -S . -B build-agent -G "MinGW Makefiles"
cmake --build build-agent -j4
```

Expected: 构建成功（若存在仓库已有缺失文件问题，明确记录为 pre-existing）。

**Step 4: 行为回归检查（最小）**

- 确认触控看门狗与电量相关事件分支未被误改。
- 确认通信服务事件处理链路未受影响。

---

### Task 6: 交付与变更说明

**Files:**
- Modify: `docs/plans/2026-03-12-matrix-scan-mainloop-design.md`（如需补充验证结论）

**Step 1: 记录验证结果**

- 记录静态检查、构建结果、已知限制。

**Step 2: 输出最终变更摘要**

- 列出实际改动文件和每个文件的职责变化。
- 说明为什么该方案满足“最小改动 + 架构一致 + 线程安全”。

**Step 3: 提交策略说明**

- 默认不提交 commit。
- 如用户明确要求再执行 `git add` / `git commit`。
