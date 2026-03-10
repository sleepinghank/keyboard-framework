# CH584 蓝牙 WT 状态机对齐修复文档

日期：2026-03-09
状态：待评审（评审通过前不进入编码）

## 1. 背景与问题定义

### 1.1 目标流程（需求基线）
- `WT_RESET -> WT_INITIALIZED`
- 无配对记录：`WT_PARING`（60s）-> 配对成功 -> `WT_CONNECTED`
- 有配对记录：`WT_RECONNECTING`（60s）-> 回连成功 -> `WT_CONNECTED`
- `WT_CONNECTED` 10 分钟无操作 -> `WT_SUSPEND`

### 1.2 当前现象
- 断连后可进入回连广播，但未必回连成功。
- 设备会在广告相关超时后进入深度休眠，主机再次开蓝牙时出现“不可发现”。

### 1.3 根因结论
- 当前运行路径主要由 `drivers/communication/bluetooth/ch584/hidkbd.c` 的 GAP 回调和 OSAL 事件驱动，使用 `pairing_state/deep_sleep_flag/con_work_mode` 组合表达状态。
- 需求定义的 WT 状态机在 `middleware/communication/wireless.c` 中存在，但当前主路径未将其作为唯一业务真源。
- 结果是“重连窗口语义”和“休眠语义”被混用，出现状态迁移偏差。

## 2. 现状与目标对照

### 2.1 现状实现（关键路径）
- 驱动侧状态入口：`drivers/communication/bluetooth/ch584/hidkbd.c` `hidEmuStateCB()`。
- 驱动侧超时处理：`ADV_TIMEOUT_SLEEP_EVT` 在 `HidEmu_ProcessEvent()` 中处理。
- 中间件 WT 状态定义：`middleware/communication/wireless.h`。
- 中间件 WT 行为：`middleware/communication/wireless.c`。

### 2.2 偏差点
1. **状态真源不统一**
   - 需求期望：WT 状态机统一决策。
   - 现状：驱动侧事件分支直接决定广播/休眠。

2. **重连超时与休眠触发耦合**
   - 需求期望：`WT_RECONNECTING` 的 60s 只决定“是否继续重连策略切换”。
   - 现状：广告超时可直接导向深睡，导致不可发现。

3. **`WT_SUSPEND` 语义未严格按需求约束**
   - 需求期望：仅 `WT_CONNECTED` 下 10 分钟无操作才进入 `WT_SUSPEND`。
   - 现状：存在非 `WT_CONNECTED` 路径进入休眠的可能。

## 3. 修复目标

1. 将蓝牙业务策略收敛到 WT 状态机语义：
   - 驱动层只负责能力与事件上报。
   - 中间件负责状态迁移和策略决策。

2. 严格执行需求状态转换：
   - 有配对记录上电/断连后进入 `WT_RECONNECTING`（60s窗口）。
   - 无配对记录进入 `WT_PARING`（60s窗口）。
   - 仅 `WT_CONNECTED` 可触发“10分钟无操作 -> WT_SUSPEND”。

3. 消除“断连后不可发现”问题：
   - 在 `WT_RECONNECTING` 生命周期内，不允许误入深睡。
   - 重连窗口结束后的行为由状态机统一决策，而非分散在驱动超时分支。

## 4. 修复内容标注（拟实施）

### 4.1 状态机对齐改造
- [Fix-01] 建立 WT 业务状态到驱动动作的单向映射表。
- [Fix-02] 将断连后的入口统一收敛为 `WT_RECONNECTING`（有配对）或 `WT_PARING`（无配对）。
- [Fix-03] 将连接成功入口统一收敛为 `WT_CONNECTED`，并重置重连窗口相关计时。

### 4.2 事件与计时器治理
- [Fix-04] 区分“重连窗口计时”与“连接后无操作计时”，禁止复用同一业务语义计时器。
- [Fix-05] 在 `WT_RECONNECTING` 期间，屏蔽任何直接进入深睡的驱动分支。
- [Fix-06] 在进入 `WT_CONNECTED` 时，显式取消/清理重连相关超时事件。

### 4.3 分层职责修正
- [Fix-07] 驱动层保留：广播开关、连接/断连动作、事件回调上报。
- [Fix-08] 中间件保留：状态迁移、超时策略、休眠策略、可发现策略。
- [Fix-09] 明确禁止：在驱动回调中直接执行业务级深睡决策。

### 4.4 可观测性与回归保障
- [Fix-10] 保留最小必要状态日志（`WT_*` 迁移日志 + 关键事件来源）。
- [Fix-11] 新增状态机一致性检查点：
  - `WT_RECONNECTING` 内不可进入 `WT_SUSPEND`。
  - 非 `WT_CONNECTED` 不启动 10 分钟无操作休眠计时。

## 5. 影响范围（拟）

### 5.1 主要文件
- `middleware/communication/wireless.c`
- `middleware/communication/wireless.h`
- `drivers/communication/bluetooth/ch584/hidkbd.c`
- `application/service/wireless_callbacks.c`

### 5.2 可能关联
- `drivers/communication/bluetooth/bt_driver.h`（若需补充事件接口语义）
- `middleware/communication/wireless_event_type.h`（若需补充事件类型）

## 6. 验证标准（评审后执行）

### 6.1 功能验证
1. 有配对记录：上电进入 `WT_RECONNECTING`，60s 窗口内可回连。
2. 无配对记录：进入 `WT_PARING`，60s 内可配对。
3. 断连后：进入 `WT_RECONNECTING`，主机蓝牙重开后可再次发现并连接。
4. `WT_CONNECTED` 下 10 分钟无操作才进入 `WT_SUSPEND`。

### 6.2 日志验收
- 必须可见 `WT_*` 状态迁移链路，不接受仅驱动局部日志。
- `WT_RECONNECTING` 生命周期内不得出现“直接深睡”日志。

### 6.3 编译验收
- 每波代码编辑后执行 `wch-riscv-build` 编译通过。

## 7. 风险与回退

### 7.1 风险
- 状态迁移收敛时可能引入短期竞态（连接成功与超时同时触发）。
- 旧驱动分支与新状态机并存期间可能出现双路径触发。

### 7.2 回退策略
- 保留关键变更点开关（编译期宏或可控分支），支持快速回退到旧路径。
- 每阶段小步提交与验证，确保可定位回退。

## 8. 本文档评审项

请重点确认以下内容后再进入编码：
1. 目标状态机是否完全按你给定流程落地。
2. `WT_SUSPEND` 触发条件是否严格限定为“`WT_CONNECTED` + 10 分钟无操作”。
3. 修复内容标注（Fix-01 ~ Fix-11）是否完整。
