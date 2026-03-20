# 触控板模块化接入 — 需求确认文档

**日期：** 2026-03-20
**状态：** 已确认
**分支：** feature/touch

---

## 1. 需求概述

将已调通的触控板组件（component/touch_component）模块化接入键盘框架，在 middleware 层新建 touchpad 模块，统一管理触控板生命周期，通过 input_service 分发事件。

## 2. 需求决策

| 决策项 | 结论 | 说明 |
|--------|------|------|
| OSAL 事件调度 | middleware/touchpad 自注册 OSAL 任务，独立 taskID | 模块自治 |
| 隔离原则 | middleware 只管自身生命周期，跨模块交互上抛 service 层 | 层级隔离 |
| 数据上报 | HID 报告在 touch_component 内生成并发送，后续对齐 wireless_transport | 快速实现，标注 TODO |
| LPM 集成 | 本次不包含 | 后续迭代 |
| 原 OSAL 逻辑 | 从 touch_component 中删除，迁入 middleware | 直接删除 |
| touch_component 改动 | 最小化 | 仅删除 OSAL 相关代码 |

## 3. 代码现状分析

### 3.1 touch_component OSAL 依赖

**touchpad_service.c：**
- `touch_taskID = OSAL_ProcessEventRegister(touch_process_event)` — 任务注册
- `OSAL_SetEvent(touch_taskID, TOUCH_INIT_EVENT)` — 触发初始化
- `OSAL_StartReloadTask(touch_taskID, TOUCH_REG_INIT_EVT, 200)` — 200ms 延迟定时器
- `OSAL_StopTask(touch_taskID, TOUCH_REG_INIT_EVT)` — 停止定时器
- `touch_process_event()` — 事件处理函数

**pct1336_driver.c：**
- `pct1336_taskID = OSAL_ProcessEventRegister(pct1336_process_event)` — 驱动层任务注册
- `OSAL_StartReloadTask(pct1336_taskID, PCT1336_REG_POLL_EVT, 10)` — 10ms 轮询（等固件就绪）
- `OSAL_StopTask(pct1336_taskID, PCT1336_REG_POLL_EVT)` — 停止轮询
- `pct1336_process_event()` — 驱动层事件处理

### 3.2 HID 报告发送路径

**键盘（参考）：**
```
report_update_proc() → wireless_transport.send_keyboard()
```

**触控板（当前）：**
```
send_ptp_data() → bt_driver_send_ptp()  // 直接调用 BT 驱动，未走 wireless_transport
```

**目标：** 对齐到 wireless_transport 路径。

### 3.3 input_service 现有触控代码

- 事件：`INPUT_TOUCH_INT_EVT`、`INPUT_TOUCH_WATCHDOG_EVT`
- 初始化：调用 `touch_power_on()`，启动看门狗 2000ms 定时器
- LPM：prepare/resume 中触控代码已注释
- 条件编译：`TOUCH_EN` 宏控制

### 3.4 数据流

```
GPIO 中断 → _touch_cb() 置标志
  → input_service INPUT_TOUCH_INT_EVT
    → touch_evt_task()
      → ProcessPTPData() / ProcessMouseData()
        → gesture_judgment()
          → send_ptp_data() → wireless_transport 发送
```

## 4. touch_component 最小改动清单

| 文件 | 改动 | 数量 |
|------|------|------|
| touchpad_service.c | 删除 touch_taskID、touch_process_event()、4 行 OSAL 调用 | ~1 函数 + 4 行 |
| pct1336_driver.c | 删除 pct1336_taskID、pct1336_process_event()、10ms 轮询改同步 | ~1 函数 + 3 行 |

## 5. 范围边界

**包含：**
- middleware/touchpad 模块创建（生命周期管理、OSAL 任务注册）
- touch_component OSAL 逻辑删除
- input_service 事件转发集成
- HID 报告路径对齐到 wireless_transport
- CMakeLists.txt 构建配置

**不包含：**
- 低功耗（LPM）集成
- 手势到快捷键映射
- 触控板启用/禁用快捷键
- 防误触与键盘联动
