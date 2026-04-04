# Shared Fn/Earth 系统组合键抑制误发实现计划

> **适用于执行代理：** 按步骤顺序执行，使用 checkbox 跟踪状态；先测试失败，再做最小修复，再做编译验证。

**目标：** 修复共享 `S_FN_KEY` 在系统组合键 active 期间仍会误发 Earth 的问题，确保 `BLE_PARING_combo` 等长按型系统组合键在回调触发前也能抑制 Earth。

**架构：** 不修改蓝牙和 report 层，只在 Earth 状态机入口补充“系统组合键 active 即抢占”的判定。新增一个 host 侧最小测试文件，直接验证 `kb_fn_action.c` 的状态机行为。

**技术栈：** C11、PowerShell、GCC（host 侧最小测试）、WCH 固件编译脚本

---

### 任务 1：建立失败测试

**文件：**
- 新建：`test/shared_fn_earth_combo_suppression_test.c`

- [ ] **步骤 1：编写最小失败测试**

覆盖以下行为：

- `active_fn_combo == 1` 时，`earth_post_loop_decision()` 必须把 `EARTH_PENDING` 切到 `EARTH_FN_TAKEN`
- 在上述状态下调用 `FN_LONG_PRESS_KEY()`，不能进入 `EARTH_HOLD`
- 在上述状态下调用 `FN_UP_KEY()`，不能输出 Earth 键码

- [ ] **步骤 2：运行测试并确认失败**

运行：

```powershell
gcc -std=c11 -Wall -Wextra -o test\shared_fn_earth_combo_suppression_test.exe test\shared_fn_earth_combo_suppression_test.c middleware\keyboard\combo\kb_fn_action.c utils\linkedlist.c -I. -Imiddleware\keyboard -Imiddleware\keyboard\combo -Imiddleware\communication -Iutils -Ikeyboards -Idrivers\output\logging
.\test\shared_fn_earth_combo_suppression_test.exe
```

预期：

- 测试失败，暴露“系统组合键 active 期间仍可进入 Earth 长按/释放路径”的现状

### 任务 2：做最小修复

**文件：**
- 修改：`middleware/keyboard/combo/kb_fn_action.c`

- [ ] **步骤 1：在 Earth 状态机中加入 `active_fn_combo` 抢占判断**

修改 `earth_post_loop_decision()`，让 `fn_fired == 1` 或 `active_fn_combo == 1` 任一成立时都进入 `EARTH_FN_TAKEN`。

- [ ] **步骤 2：补充必要注释，保持逻辑聚焦**

只解释系统组合键为何要提前抢占 Earth，不做额外重构。

- [ ] **步骤 3：重新运行测试并确认转绿**

重复任务 1 的命令，预期测试通过。

### 任务 3：整仓验证

**文件：**
- 修改：`middleware/keyboard/combo/kb_fn_action.c`
- 新建：`test/shared_fn_earth_combo_suppression_test.c`

- [ ] **步骤 1：执行固件编译验证**

运行：

```bash
/wch-riscv-build
```

预期：

- 编译通过

- [ ] **步骤 2：整理结果**

记录：

- 新增测试命令
- 修复点
- 是否存在未覆盖的手动验证项
