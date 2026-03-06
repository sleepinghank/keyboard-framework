## Context

当前工程中 CH584 BLE 协议栈初始化位于 HAL 层 `system_hal_init`，同时 Drivers 层已存在 `bt_driver_init` 实现，导致职责重叠和链路分散。应用层启动中 `commu_service_init` 未接入，也使通信事件驱动链路未形成闭环。

## Goals / Non-Goals

- Goals:
  - 恢复分层边界，BLE 初始化归位 Drivers 层
  - 以最小改动打通通信启动链路
  - 保证广播 连接 回连 断联 重广播可验证
- Non-Goals:
  - 不重写整套蓝牙状态机
  - 不做多通道 BLE 全量改造
  - 不改动无关输入 输出 存储模块

## Decisions

- Decision: HAL 仅保留平台公共初始化，不再承载 BLE 协议栈应用逻辑。
  - Why: BLE 业务属于通信驱动域，避免跨层耦合。
- Decision: 通过 `communication_service` 的 `WL_INIT_EVT` 驱动 `wireless_init` 进入蓝牙驱动初始化。
  - Why: 对齐现有 OSAL 任务事件模型，改动面小。
- Decision: 断联后自动重广播由 middleware 层统一做状态门控。
  - Why: 避免驱动回调中直接做复杂控制导致重入和风暴。

## Risks / Trade-offs

- 风险: 移除 HAL 中 BLE 初始化后可能暴露隐藏时序依赖。
  - Mitigation: 保留 HAL 时钟 电源 中断基础初始化，驱动 init 前后加串口日志和状态检查。
- 风险: 断联重广播可能重复触发。
  - Mitigation: 增加状态判定，已在广播时禁止重复 start。
- 风险: 参考工程是多通道，本次只做单通道可能遗留分支行为。
  - Mitigation: 在设计中明确单通道目标并保留后续扩展点。

## Migration Plan

1. 接通 `commu_service_init` 到系统启动流程。
2. 在 middleware 中完成 driver init 和初始广播触发。
3. 从 HAL 移除 BLE 协议栈初始化调用。
4. 执行编译 烧录 串口日志回归验证。

## Open Questions

- CH584 实际构建入口采用参考工程方式，需在实施阶段锁定具体命令与日志采集步骤。
