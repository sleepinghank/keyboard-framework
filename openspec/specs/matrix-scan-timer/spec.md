# matrix-scan-timer Specification

## Purpose
使用硬件定时器触发矩阵扫描，通过 main loop 标志位执行，减少 OSAL 调度压力和延迟。

## Requirements

### Requirement: 硬件定时器触发矩阵扫描标志位

系统 SHALL 使用硬件定时器以固定周期设置矩阵扫描标志位。

#### Scenario: 正常扫描周期

- **WHEN** 系统初始化完成且输入服务启动
- **THEN** 硬件定时器以 5ms 周期运行
- **AND** 每次定时器中断设置 `g_matrix_scan_flag` 标志位
- **AND** main loop 检测到标志位后执行 `keyboard_task()`

#### Scenario: 标志位访问

- **WHEN** 定时器中断设置标志位
- **THEN** main loop 通过 `input_get_matrix_scan_flag()` 读取标志位
- **AND** 执行完成后通过 `input_clear_matrix_scan_flag()` 清除标志位
- **AND** 标志位使用 `volatile` 保证中断可见性

### Requirement: 扫描周期可配置

系统 SHALL 支持通过宏定义配置矩阵扫描周期。

#### Scenario: 默认扫描周期

- **WHEN** 未定义 `MATRIX_SCAN_INTERVAL_MS` 宏
- **THEN** 使用默认值 5ms 作为扫描周期

#### Scenario: 自定义扫描周期

- **WHEN** 在 `product_config.h` 中定义 `MATRIX_SCAN_INTERVAL_MS` 为 10
- **THEN** 系统以 10ms 周期执行矩阵扫描

### Requirement: 低功耗模式支持

系统 SHALL 支持在低功耗模式下停止矩阵扫描定时器。

#### Scenario: 进入低功耗模式

- **WHEN** 系统进入低功耗模式
- **THEN** 可调用 `matrix_scan_timer_stop()` 停止定时器
- **AND** 减少功耗

#### Scenario: 唤醒恢复扫描

- **WHEN** 系统从低功耗模式唤醒
- **THEN** 可调用 `matrix_scan_timer_start()` 恢复定时器
- **AND** 矩阵扫描恢复正常周期运行
