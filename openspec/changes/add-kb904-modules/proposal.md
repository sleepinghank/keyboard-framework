# Change: Add KB904 output/keyboard modules

## Why

KB904 需求包含背光扩展、指示灯扩展、组合键新增、低电量联动和恢复出厂流程，当前代码仅覆盖部分能力，且模块间联动缺失。

## What Changes

- 扩展背光颜色到 13 色并增加 5 秒无活动休眠机制。
- 新增 KB904 指示灯 4 灯配置与请求映射。
- 增加 KB904 专用组合键动作（亮度、颜色、电量检查、Siri）。
- 完成低电量阈值联动（20%）与恢复后反联动。
- 完成恢复出厂链路（闪灯反馈、清配对、重置存储/背光、复位）。
- 统一使用 OSAL `OSAL_SetDelayedEvent` 实现一次性定时事件。

## Impact

- Affected specs: `kb904-modules`
- Affected code:
  - `drivers/output/backlight/*`
  - `keyboards/indicator_config.h`
  - `middleware/keyboard/combo/*`
  - `middleware/keyboard/keyboard.*`
  - `application/service/input_service.*`
  - `application/service/output_service.*`
  - `application/service/system_service.c`
  - `drivers/communication/bluetooth/*`
