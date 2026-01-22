# Tasks: 硬件定时器矩阵扫描

## 1. 配置定义

- [x] 1.1 在 `input_service.h` 中定义矩阵扫描周期宏 `MATRIX_SCAN_INTERVAL_MS` (默认 5ms)
- [x] 1.2 在 `input_service.h` 中定义使用的硬件定时器 ID `MATRIX_SCAN_TIMER_ID` (默认 HW_TIMER_0)

## 2. 定时器回调实现

- [x] 2.1 在 `input_service.c` 中实现硬件定时器回调函数 `matrix_scan_timer_callback()`
- [x] 2.2 回调函数中调用 `OSAL_SetEvent(input_taskID, INPUT_MATRIX_SCAN_EVT)` 触发扫描事件

## 3. 启动/停止接口

- [x] 3.1 实现 `matrix_scan_timer_start()` 函数，调用 `hw_timer_start()` 启动定时器
- [x] 3.2 实现 `matrix_scan_timer_stop()` 函数，调用 `hw_timer_stop()` 停止定时器
- [x] 3.3 在 `input_service.h` 中声明启动/停止接口

## 4. 服务集成

- [x] 4.1 修改 `input_service_init()` 调用 `matrix_scan_timer_start()` 启动定时器
- [x] 4.2 确保 `hw_timer_init()` 在系统初始化阶段被调用

## 5. 验证测试

> 硬件验证在实际设备上进行，此处跳过。
