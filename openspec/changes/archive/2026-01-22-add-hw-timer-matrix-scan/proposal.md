# Change: 使用硬件定时器触发矩阵扫描

## Why

当前矩阵扫描依赖 OSAL 软件定时器（`OSAL_StartReloadTask`），精度受限于 OSAL 任务调度。使用硬件定时器可以实现更精确的 5ms 扫描周期，提升按键响应的一致性和实时性。

## What Changes

- 新增矩阵扫描定时器模块，使用硬件定时器（HW_TIMER_0）实现 5ms 周期触发
- 硬件定时器回调中通过 `OSAL_SetEvent()` 设置 `INPUT_MATRIX_SCAN_EVT` 事件
- OSAL 主循环检测到事件后执行 `keyboard_task()` 进行实际扫描
- 提供启动/停止接口，支持低功耗模式下关闭定时器

## Impact

- Affected specs: 新增 `matrix-scan-timer` 规格
- Affected code:
  - `application/service/input_service.c` - 集成硬件定时器启动逻辑
  - `application/service/input_service.h` - 新增扫描周期配置宏
  - 可能新增 `drivers/input/keyboard/matrix_timer.c` 模块（可选，取决于实现复杂度）
