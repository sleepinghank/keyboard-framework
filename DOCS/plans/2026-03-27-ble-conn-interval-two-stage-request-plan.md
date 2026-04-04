# BLE 连接参数两阶段申请实现计划

> **给执行型智能体：** 必须使用子技能 `superpowers:subagent-driven-development`（推荐）或 `superpowers:executing-plans` 逐任务执行本计划。步骤使用复选框（`- [ ]`）跟踪。

**目标：** 以最小改动实现 `Default -> iOS` 的连接参数申请顺序，并提供清晰日志，区分本机主动申请流程与后续主机侧更新。

**架构：** 在 `hidkbd.c` 中保留编排职责（申请时序、连接内标志位、请求日志），在 `hiddev.c` 中保留协议栈回调职责（更新日志 + 回调桥接）。不引入独立状态机，仅使用现有 GAP 连接状态和少量标志位。

**技术栈：** C11、TMOS 事件循环、CH58x BLE GAPRole API、WCH RISC-V 工具链（`wch-riscv-build`）。

---

## 范围说明

- 本计划基于已确认设计文档：
  - `docs/plans/2026-03-27-ble-conn-interval-two-stage-request-design.md`
- 当前仓库不存在 `openspec/AGENTS.md`，本次按现有 `docs/plans` 流程执行。
- 遵循仓库规则：不做无关重构，改动尽量小且局部。

## 文件映射

- 修改：`drivers/communication/bluetooth/ch584/hidkbd.c`
  - 将单组参数改为两组 profile（`Default`、`iOS`）。
  - 增加连接内最小时序上下文（`conn_param_seq`、`ios_param_requested`、可选重申预算）。
  - 统一连接参数请求发送与日志输出 helper。
  - 在连接/断开时重置上下文，并在断开后清理遗留定时事件。
- 修改：`drivers/communication/bluetooth/ch584/hidkbd.h`
  - 仅增加 `hiddev.c` 需要调用的最小声明（若确有需要）。
- 修改：`project/ch584m/Profile/hiddev.c`
  - 优化参数更新日志格式。
  - 在更新回调中桥接调用 `hidkbd.c` 的时序控制入口。
  - 可选：在 GAP 消息处理中补充 `GAP_LINK_PARAM_UPDATE_EVENT` 观测日志。
- 可选修改：`docs/plans/2026-03-27-ble-conn-interval-two-stage-request-design.md`
  - 仅在实现细节与设计文档不一致时同步更新。

## 目标参数

- Default profile：`min=9, max=12, latency=6, timeout=300`
- iOS profile：`min=12, max=12, latency=0, timeout=500`

## 任务 1：在 `hidkbd.c` 引入参数 profile 与最小上下文

**文件：**
- 修改：`drivers/communication/bluetooth/ch584/hidkbd.c`（宏/常量区、局部静态变量、helper 函数）
- 修改：`drivers/communication/bluetooth/ch584/hidkbd.h`（仅当需要新增 extern）

- [ ] **步骤 1：定义两组明确的参数 profile（Default/iOS）**
  - 增加两组编译期常量。
  - 命名尽量贴近现有 `DEFAULT_DESIRED_*` 风格，降低引入风险。

- [ ] **步骤 2：增加最小连接内时序上下文（不新增状态机 enum）**
  - 增加 `conn_param_seq`（请求序号）。
  - 增加 `ios_param_requested`（连接内 0/1）。
  - 增加可选 `ios_reapply_budget`（按评审结论设置保守默认 `0` 或 `1`）。

- [ ] **步骤 3：新增统一请求 helper，并统一日志格式**
  - 新建小型内部 helper，负责：
    - 接收 profile 参数与 profile 标签；
    - 调用 `GAPRole_PeripheralConnParamUpdateReq`；
    - 输出 `[BT_CONN_REQ] ...`（含 seq/profile/params/ret）。

- [ ] **步骤 4：现有请求入口改为走 helper**
  - 将 `START_PARAM_UPDATE_EVT` 分支与 `hidEmu_SetConnParams()` 改为调用 helper（默认走 Default）。
  - 除目标参数与日志格式外，保持行为不变。

- [ ] **步骤 5：自检声明与可编译性影响**
  - 确认无缺失声明、无 static/extern 冲突。

- [ ] **步骤 6：提交**
```bash
git add drivers/communication/bluetooth/ch584/hidkbd.c drivers/communication/bluetooth/ch584/hidkbd.h
git commit -m "feat(ble): add default/ios conn param profiles and request helper"
```

## 任务 2：在现有事件流中接入两阶段顺序（`hidkbd.c`）

**文件：**
- 修改：`drivers/communication/bluetooth/ch584/hidkbd.c`
- 修改：`drivers/communication/bluetooth/ch584/hidkbd.h`（仅当桥接函数需要声明）

- [ ] **步骤 1：在连接建立时重置连接内时序上下文**
  - 在 `GAP_LINK_ESTABLISHED_EVENT` 分支中：
    - 重置 `ios_param_requested`；
    - 初始化/推进请求序号基线；
    - 保持现有 `START_PARAM_UPDATE_EVT` 调度用于第一阶段 Default 请求。

- [ ] **步骤 2：在断链路径清理遗留定时与标志**
  - 在断链分支中：
    - 停止 `START_PARAM_UPDATE_EVT` 定时/事件；
    - 清理 `ios_param_requested` 与可选重申预算。
  - 其余断链行为保持不变。

- [ ] **步骤 3：新增“参数更新回调桥接入口”**
  - 在 `hidkbd.c` 中实现一个可被 `hiddev.c` 调用的小函数：
    - 入参为当前协商结果；
    - 若仍连接且 `ios_param_requested == 0`，发送一次 iOS 请求并标记；
    - 否则仅记录（或在预算开启时做一次重申）。

- [ ] **步骤 4：补充简洁且可 grep 的触发原因日志**
  - 增加触发原因字段：
    - `trigger=default_timer`
    - `trigger=first_update_callback`
    - `trigger=optional_reapply`
  - 日志保持短小，便于线上排障。

- [ ] **步骤 5：提交**
```bash
git add drivers/communication/bluetooth/ch584/hidkbd.c drivers/communication/bluetooth/ch584/hidkbd.h
git commit -m "feat(ble): wire minimal default->ios conn param sequencing"
```

## 任务 3：增强 `hiddev.c` 的参数更新可观测性

**文件：**
- 修改：`project/ch584m/Profile/hiddev.c`

- [ ] **步骤 1：优化 `hidDevParamUpdateCB` 日志格式**
  - 将 `update handle=...` 改为结构化 `[BT_CONN_UPD] ...`。
  - 日志带上 interval/latency/timeout 以及可关联的序号信息（若可从桥接侧获取）。

- [ ] **步骤 2：在更新回调中调用桥接入口**
  - 日志之后调用 `hidkbd.c` 桥接函数，驱动一次性 iOS 请求。
  - `hiddev.c` 保持轻逻辑，避免承担复杂流程控制。

- [ ] **步骤 3：可选增加 GAP opcode 观测日志**
  - 在 `hidDevProcessGAPMsg` 中按可用结构尝试增加 `GAP_LINK_PARAM_UPDATE_EVENT` 日志。
  - 若当前回调路径拿不到完整字段，代码注释说明限制，并以 `hidDevParamUpdateCB` 为主观测点。

- [ ] **步骤 4：提交**
```bash
git add project/ch584m/Profile/hiddev.c
git commit -m "chore(ble): add structured conn param update logs and callback bridge"
```

## 任务 4：编译验证（通过后再做实机验证）

**文件：**
- 无代码改动（除非构建失败后需修复）。

- [ ] **步骤 1：所有代码修改完成后，执行 `wch-riscv-build`**
```bash
/wch-riscv-build
```
预期：CH584 工程编译通过，不引入新的编译/链接错误。

- [ ] **步骤 2：若编译失败，先修复代码问题后再次执行 `wch-riscv-build`**
```bash
/wch-riscv-build
```
预期：二次验证通过；若仍失败，输出首个真实报错并回到对应任务修正。

## 任务 5：实机日志验收（核心验收路径）

**文件：**
- 无源代码改动（除非验收失败需要回修）。

- [ ] **步骤 1：抓取一段完整连接日志**
  - 从 `Advertising..` 到连接完成并进入服务发现阶段。

- [ ] **步骤 2：核对关键时序**
  - `BT_CONN established ...`（建链初值）
  - `[BT_CONN_REQ] ... profile=default min=9 max=12 lat=6 to=300`
  - `[BT_CONN_UPD] ...`（首次更新结果）
  - `[BT_CONN_REQ] ... profile=ios min=12 max=12 lat=0 to=500`
  - `[BT_CONN_UPD] ...`（第二次结果或后续主机侧更新）

- [ ] **步骤 3：核对“一次连接只触发一次 iOS 主动申请”**
  - 未启用重申预算时，`profile=ios` 每连接仅出现一次。

- [ ] **步骤 4：核对断链后清理**
  - 断链后不再出现对失效 handle 的参数请求日志。

- [ ] **步骤 5：整理对比证据**
  - 记录简短的“修改前/修改后”日志片段，用于提交说明或评审材料。

## 任务 6：收口质量门禁

**文件：**
- 可选：设计文档（若实现与设计存在偏差）

- [ ] **步骤 1：复查 diff 的范围纯度**
  - 确认改动仅限连接参数时序与日志可观测性。

- [ ] **步骤 2：执行 `@verification-before-completion` 检查**
  - 构建结果已确认；
  - 实机日志验收通过；
  - 无事件风暴与无断链后遗留请求。

- [ ] **步骤 3：如实现与设计有偏差，同步更新设计文档**
```bash
git add docs/plans/2026-03-27-ble-conn-interval-two-stage-request-design.md
git commit -m "docs(ble): align conn param design with implemented behavior"
```

- [ ] **步骤 4：输出交付总结**
  - 改了什么；
  - 如何验证；
  - 已知限制与后续建议。

## 执行注意事项

- 若出现异常重试、参数拉扯或两次以上失败尝试，立即回到 `@systematic-debugging`，不要继续叠加修补。
- 每个提交保持聚焦、可回滚。
- 未经明确要求，不新增额外状态抽象层。
