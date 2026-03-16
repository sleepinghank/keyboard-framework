# 矩阵扫描由 OSAL 事件改为 Main Loop 标志位设计

## 1. 需求目标

- 将矩阵扫描触发从 `INPUT_MATRIX_SCAN_EVT` 迁移为 main loop 标志位执行。
- 在定时器回调中仅设置扫描请求标志位。
- 在 `application/main.c` 主循环中消费标志位，执行 `keyboard_task()` 后清除标志位。
- 保持现有分层和最小改动，不扩散到无关模块。

## 2. 已确认现状

- `INPUT_MATRIX_SCAN_EVT` 定义在 `application/service/input_service.h`，由 `matrix_scan_timer_callback()` 在 `application/service/input_service.c` 中触发。
- `application/main.c` 当前调用 `OSAL_SystemProcess()`，而 CH584 平台实现中该函数内部是无限循环，不会返回。
- `input_process_event()` 的矩阵扫描分支当前未执行 `keyboard_task()`（仅保留调试 `togglePin(B14)`）。

## 3. 不明确项与默认假设

由于未收到进一步澄清反馈，采用以下默认假设继续：

1. **扫描必须真实执行**：迁移后在 main loop 分支内执行 `keyboard_task()`。
2. **调试分支不再作为主路径**：移除 `INPUT_MATRIX_SCAN_EVT` 处理分支，`togglePin(B14)` 不再作为扫描执行依据。
3. **允许最小范围扩展 OSAL 接口**：新增单步 OSAL 处理函数，避免 `OSAL_SystemProcess()` 内部死循环阻塞 main loop。

## 4. 候选方案对比

### 方案 A（推荐）: 新增 OSAL 单步处理 + main 显式 while 循环

做法：

- 新增 `OSAL_SystemProcessOnce()`，内部只调用一次底层 `TMOS_SystemProcess()`。
- `main()` 改为显式 `while (1)`：
  1. 检查扫描标志位
  2. 若置位则执行 `keyboard_task()` 并清零标志位
  3. 调用 `OSAL_SystemProcessOnce()` 处理一轮 OSAL

优点：

- 满足“main loop 标志位执行”的核心需求。
- 保留 OSAL 生态，不破坏通信/系统服务事件机制。
- 改动点集中，架构边界清晰。

缺点：

- 需要在 event_manager 接口增加一个 API。

### 方案 B: 保持 OSAL 主循环不变，仅在 input_service 中轮询标志

做法：

- 不改 `main.c`。
- 在 OSAL 任务中轮询标志位并执行 `keyboard_task()`。

缺点：

- 不满足“在 main loop（application/main.c）执行”的明确要求。
- 仍受 OSAL 调度节拍影响，收益有限。

### 方案 C: 在定时器中断中直接执行 `keyboard_task()`

缺点：

- 违背 ISR 轻量原则。
- 风险高，可能引发中断占用过长与时序问题。
- 明显不符合现有事件驱动架构习惯。

## 5. 推荐设计（采用方案 A）

### 5.1 模块改动范围

- `application/main.c`
- `application/service/input_service.h`
- `application/service/input_service.c`
- `drivers/system/event_manager.h`
- `drivers/system/platforms/ch584/event_manager.c`

仅以上文件，不改动其他业务模块。

### 5.2 标志位设计

- 在 `input_service.c` 增加 `static volatile bool g_matrix_scan_pending`。
- `matrix_scan_timer_callback()` 中仅置位：`g_matrix_scan_pending = true;`
- 提供接口：
  - `bool input_matrix_scan_pending(void);`
  - `void input_matrix_scan_clear_pending(void);`

说明：

- CH58x 单核场景下，单字节读写可视为原子；`volatile` 保证 ISR 与主循环可见性。
- 当前仅需“有无请求”，允许事件合并，不追求每个 tick 精确计数。

### 5.3 main loop 执行序

`main()` 显式循环顺序如下：

1. 如果 `input_matrix_scan_pending()` 为真：
   - 清除标志
   - 执行 `keyboard_task()`
2. 调用 `OSAL_SystemProcessOnce()` 处理一轮 OSAL 事件。

该顺序优先保证扫描低延迟，同时保留协议栈和系统事件处理。

### 5.4 input_service 事件精简

- 从 `input_task_event_t` 移除 `INPUT_MATRIX_SCAN_EVT`。
- 删除 `input_process_event()` 中对应分支。
- 保留 `INPUT_TOUCH_INT_EVT`、`INPUT_TOUCH_WATCHDOG_EVT`、`INPUT_BATTERY_DETE_EVT` 逻辑不变。

### 5.5 OSAL 单步接口

- 在 `event_manager.h` 声明 `void OSAL_SystemProcessOnce(void);`
- 在 `drivers/system/platforms/ch584/event_manager.c` 实现该函数，单次调用 `TMOS_SystemProcess()`。
- `OSAL_SystemProcess()` 保持兼容（原行为不变），以免影响其他入口。

## 6. 风险与规避

1. **风险：扫描标志被覆盖导致 tick 合并**
   - 规避：当前需求只关心扫描触发，不关心每个 tick 数量，合并可接受。

2. **风险：main loop 次序影响 OSAL 处理频率**
   - 规避：每轮固定调用一次 `OSAL_SystemProcessOnce()`，避免 OSAL 饥饿。

3. **风险：接口改动影响其他平台**
   - 规避：本次仅在 CH584 平台实现，若有 test 平台需要编译通过可同步加空实现。

## 7. 验证计划

1. 静态检查：确认删除 `INPUT_MATRIX_SCAN_EVT` 后无残留引用。
2. 编译验证：确保 `main.c`、`input_service.*`、`event_manager.*` 编译通过。
3. 行为验证：
   - 定时器回调可持续置位。
   - main loop 可触发 `keyboard_task()`。
   - 触控/电量/通信相关 OSAL 事件仍正常。

## 8. 结论

采用方案 A 可以在最小改动前提下满足需求：

- 矩阵扫描迁移到 `application/main.c` 标志位路径
- 降低 OSAL 对高频扫描的调度压力
- 保持原有架构层次和模块边界
