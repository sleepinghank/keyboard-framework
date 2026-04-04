# BLE 连接参数两阶段申请设计（Default -> iOS）

日期: 2026-03-27

## 1. 背景

当前蓝牙连接过程中的连接参数表现与预期不一致。期望行为为：

1. 连接建立后先申请 `Default` 参数；
2. `Default` 申请完成后，再申请 `iOS` 参数；
3. 全流程可从日志中明确区分“本机主动申请”与“主机（手机）侧更新”。

你给出的目标参数如下：

```c
#define DEFAULT_CONN_INTERVAL_MIN 9      /* 11.25 ms */
#define DEFAULT_CONN_INTERVAL_MAX 12     /* 15 ms */
#define DEFAULT_CONN_LATENCY 6
#define DEFAULT_CONN_SVTO 300            /* 3000 ms */

#define IOS_CONN_INTERVAL_MIN 12         /* 15 ms */
#define IOS_CONN_INTERVAL_MAX 12         /* 15 ms */
#define IOS_CONN_LATENCY 0
#define IOS_CONN_SVTO 500                /* 5000 ms */
```

## 2. 现象与证据

### 2.1 日志现象（本次问题）

日志时间线（简化）：

1. 连接建立：`interval=24 latency=0 timeout=72`
2. 约 2.3 秒后更新：`int=12 latency=0 timeout=600`
3. 约 8.6 秒后再次更新：`int=12 latency=4 timeout=100`

这说明链路参数在连接后被至少两次更新。

### 2.2 当前代码中的实际参数

当前实现中，连接参数宏为：

- `DEFAULT_DESIRED_MIN_CONN_INTERVAL = 12`
- `DEFAULT_DESIRED_MAX_CONN_INTERVAL = 12`
- `DEFAULT_DESIRED_SLAVE_LATENCY = 0`
- `DEFAULT_DESIRED_CONN_TIMEOUT = 600`

因此当前代码并未实现“Default(9-12/6/300) -> iOS(12/12/0/500)”两阶段策略。

### 2.3 当前代码中的更新入口

仓库内有效的 `GAPRole_PeripheralConnParamUpdateReq(...)` 仅在 `hidkbd.c` 中触发：

1. `START_PARAM_UPDATE_EVT` 事件处理分支；
2. `hidEmu_SetConnParams()`（同样走上述默认宏）。

未发现第二个活跃模块在本工程源码层面主动发起“12/4/100”请求。

### 2.4 协议栈语义证据

协议头文件与参考手册明确：

1. `GAPRole_PeripheralConnParamUpdateReq` 是“从机请求主机协商”；
2. 主机可拒绝或按自身策略协商；
3. 参数更新回调仅报告“更新结果”，不直接包含“更新发起方”。

因此，`int=12 latency=4 timeout=100` 更可能是主机侧（iOS）在后续流程中触发的协商结果，而不是本机第二次主动申请。

## 3. 根因归纳

本问题的核心根因是三点：

1. 参数目标未落地：当前编译期参数与目标参数不一致；
2. 流程缺失：不存在“Default -> iOS”明确时序申请流程；
3. 可观测性不足：日志无法区分“本机请求”与“对端协商结果”，导致调试时看起来像“无原因被改写”。

## 4. 设计目标

### 4.1 功能目标

1. 连接后按顺序执行两次申请：
   - 阶段 A：`Default`（9-12/6/300）
   - 阶段 B：`iOS`（12/12/0/500）
2. 两次申请有明确触发条件，避免乱序与重复；
3. 对端若后续改写参数，本机可按策略决定是否重申 iOS 参数（有限次数）。

### 4.2 调试目标

日志必须能回答三个问题：

1. 本机何时申请、申请了哪组参数；
2. 更新回调是哪次申请对应的结果；
3. 最终参数是否被主机再次改写。

### 4.3 约束目标

1. 最小侵入，不打破现有分层；
2. 保持事件驱动模型（TMOS task/event）；
3. 不引入无限参数拉扯（request storm）。

## 5. 方案概述（最小改动版）

不新增独立状态机，仅用现有连接状态 + 少量标志位完成两阶段申请；在 `hiddev.c` 增强参数更新事件日志。

### 5.1 最小上下文

只新增必要上下文：

1. `conn_param_seq`：请求序号（用于串日志）；
2. `ios_param_requested`：本连接是否已发过 iOS 参数请求（0/1）；
3. `ios_reapply_budget`：可选，是否允许一次重申（默认可设为 0，先关闭）。

### 5.2 触发规则

1. `GAP_LINK_ESTABLISHED_EVENT` 到来后：
   - 记录初始链路参数；
   - 复用现有 `START_PARAM_UPDATE_EVT` 发送 `Default` 请求。
2. 收到第一次参数更新回调后：
   - 若 `ios_param_requested == 0` 且当前仍连接，发送 `iOS` 请求；
   - 发送后置 `ios_param_requested = 1`。
3. 后续更新回调：
   - 只记录结果；
   - 如启用 `ios_reapply_budget` 且检测到明显偏离目标参数，可重申一次 iOS 参数。
4. 断链时：
   - 清理 `ios_param_requested` 和相关计数；
   - 停止参数更新定时事件，避免旧连接事件污染。

### 5.3 日志设计

新增统一日志格式：

1. 请求日志：
   - `[BT_CONN_REQ] seq=%u profile=%s handle=%x min=%u max=%u lat=%u to=%u ret=%u`
2. 更新结果日志：
   - `[BT_CONN_UPD] seq=%u handle=%x int=%u lat=%u to=%u source=%s`

`source` 判定规则（工程内推断）：

1. 若更新发生在“最近一次本机请求窗口”内，记为 `local_or_negotiated`；
2. 否则记为 `peer_or_stack`；
3. 明确标注“推断值”，避免误导为协议栈真实字段。

## 6. 参数配置设计

将参数分为两组固定 profile，建议放在同一模块集中管理（避免散落宏）：

1. `conn_param_default = {9, 12, 6, 300}`
2. `conn_param_ios = {12, 12, 0, 500}`

可保留宏定义，也可定义两个 `const conn_param_t` 常量，避免多个调用点硬编码。

## 7. 代码改动范围（设计阶段）

本设计预计改动文件：

1. `drivers/communication/bluetooth/ch584/hidkbd.c`
   - 增加最小两阶段时序控制（非独立状态机）；
   - 增加请求入口与重申策略；
   - 增强日志。
2. `drivers/communication/bluetooth/ch584/hidkbd.h`
   - 仅在确有新增接口时修改。
3. `project/ch584m/Profile/hiddev.c`
   - 增强参数更新回调日志；
   - 在 GAP 事件处理中增加 `GAP_LINK_PARAM_UPDATE_EVENT` 辅助打印（若当前路径可拿到该事件）。

不在本次范围：

1. 改动应用层无线模式切换逻辑；
2. 调整配对策略（BondMgr 配置）；
3. 改动 HID 功能层协议。

## 8. 风险与应对

### 风险 1：主机持续改写导致参数拉扯

应对：

1. 限制 iOS 重申次数（默认 1）；
2. 超预算后仅记录，不再重试。

### 风险 2：首次更新回调未按预期到来，导致 iOS 请求未发出

应对：

1. 打印“Default 已发但未收到回调”的超时日志；
2. 保持单次补发上限，避免循环重发。

### 风险 3：日志“来源判定”被误当绝对真相

应对：

1. 所有来源字段标记为“推断”；
2. 文档中注明协议栈不直接提供发起方字段。

## 9. 验证标准

### 9.1 功能验证

一次完整连接应看到：

1. 建链初值日志（例如 24/0/72）；
2. `REQ_DEFAULT` 请求日志（9-12/6/300）；
3. `REQ_IOS` 请求日志（12/12/0/500）；
4. 每次更新结果日志带 `seq/profile/source` 关联信息。

### 9.2 稳定性验证

1. 连续重连 20 次，不出现参数更新事件风暴；
2. 断链后不再对失效句柄发起参数请求；
3. 不影响已有配对、CCC 写入、HID 上报流程。

### 9.3 与本问题对应的验收口径

1. 代码中可明确看到“先 Default 再 iOS”的阶段逻辑；
2. 若出现后续 `12/4/100`，日志可明确显示其不属于本机当前阶段请求；
3. 调试者可从单次日志中还原全链路参数协商过程。

## 10. 评审清单

评审时重点确认：

1. 两阶段参数值是否最终采用本设计给出的目标值；
2. 阶段推进时机是否接受（建链后时延、回调触发条件）；
3. iOS 重申预算是否为 1（或你指定其他值）；
4. 日志粒度是否满足现场排障需要；
5. 是否同意先以“最小改动”落地，不额外重构其他蓝牙状态机逻辑。

## 11. 备注

仓库根 `AGENTS.md` 提示在涉及 proposal/spec/plan 时应参考 `openspec/AGENTS.md`。当前仓库未找到该文件，因此本设计文档按现有 `docs/plans` 规范编写，后续如补充 OpenSpec 规范，可再对文档模板做对齐更新。
