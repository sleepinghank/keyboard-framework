# Change: Refactor CH584 BLE init and lifecycle wiring

## Why

当前 CH584 蓝牙协议栈初始化仍放在 `system_hal_init`，破坏了分层边界，也导致通信服务链路未完整接通，广播 连接 回连 断联 重广播流程无法稳定验证。

## What Changes

- 将 CH584 BLE 协议栈初始化职责从 HAL 迁移到蓝牙驱动层入口，保留 HAL 仅做平台通用初始化。
- 打通 `system_init_coordinator -> communication_service -> wireless -> bt_driver` 启动链路。
- 在现有事件模型内明确广播 连接 回连 断联 重广播状态迁移，采用最小改动方式实现。
- 增加串口日志验证点，支持逐功能编译烧录验证。

## Impact

- Affected specs: `bluetooth-connection-lifecycle`
- Affected code:
  - `application/system/system_init.c`
  - `application/service/communication_service.c`
  - `middleware/communication/wireless.c`
  - `hal/platforms/ch584/_system_hal.c`
  - `drivers/communication/bluetooth/ch584/_bt_driver.c`
