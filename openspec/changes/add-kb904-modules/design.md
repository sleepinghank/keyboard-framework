## Context

KB904 功能以事件驱动方式接入现有分层架构，需避免 middleware 直接控制 driver，同时保持 N0046 等既有产品兼容。

## Goals / Non-Goals

- Goals:
  - 通过最小改动引入 KB904 背光/指示灯/组合键/低电量/恢复出厂能力
  - 保持现有层级边界和可移植性
  - 保持旧产品配置兼容
- Non-Goals:
  - OTA、APP 配置、手势自定义
  - 构建系统重构

## Decisions

- 定时策略：一次性定时事件使用 `OSAL_SetDelayedEvent`，取消使用 `OSAL_StopTask`。
- 跨层调用：通过 `keyboard_note_backlight_activity()` 包装函数转发到 output_service。
- 低电量判断：以 `battery_get_power_state()` 为主，按状态变化触发请求，避免重复投递。
- 恢复出厂：由 combo 触发 `SYSTEM_FACTORY_RESET_EVT`，在 system_service 执行完整流程。

## Risks / Trade-offs

- 指示灯硬件差异（2 灯/4 灯）需严格条件编译，避免宏缺失。
- 背光驱动初始化时机不当可能导致动作调用无效，需在 output_service 初始化时兜底初始化。
- 恢复出厂流程涉及蓝牙/存储/复位，调试窗口短，需提前打印关键日志。

## Migration Plan

1. 先落地配置与接口，再接入组合键动作。
2. 联通低电量状态迁移与输出联动。
3. 最后接入恢复出厂完整动作。
4. 使用 CH584 工具链构建验证并修复编译问题。

## Open Questions

- KB904 固件是否需要切换独立 keymap 文件（当前先复用现有 keymap 并接入功能逻辑）。
