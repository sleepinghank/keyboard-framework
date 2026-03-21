# kb_sys_action 组合键事件实现 — 需求文档

**日期**：2026-03-21
**分支**：feature/touch
**状态**：已确认

## 1. 需求概述

将 `kb_sys_action.c` 中 7 个空引用的 `SYS_EVT_*` 事件全部实现，同时合并 `kb_fn_action.c` 中功能重叠的函数，统一采用 OSAL 事件调度方式，实现 combo 层与底层模块的隔离。

## 2. 用户决策记录

| # | 问题 | 决策 |
|---|------|------|
| 1 | 实现方式 | 合并 kb_fn_action.c 和 kb_sys_action.c 重叠函数到 kb_sys_action.c，统一走 OSAL 事件调度 |
| 2 | 事件分配 | 按职责分散：背光 -> output_service，触摸板 -> input_service，配对 -> communication_service，storage/battery/reset -> system_service |
| 3 | combo 层隔离 | kb_fn_action.c 中的直接调用也改为 OSAL 事件，combo 层不直接依赖底层模块 |
| 4 | 触摸板锁定 | 事件管道搭通，处理函数留 TODO |
| 5 | FN Lock | combo 回调中直接 toggle RAM 标志位（STORAGE_SET_FN_LOCK）+ 发 SYSTEM_STORAGE_EVT 延迟写入 |

## 3. 当前问题分析

### 3.1 kb_sys_action.c 空引用事件

7 个 `SYS_EVT_*` 事件（0x0001~0x0040）存在两个致命问题：
- **Task ID 错误**：`SYSTEM_SERVICE_TASK_ID` 硬编码为 0，实际 `system_taskID` 动态分配
- **事件位冲突**：与 `system_service.h` enum 值重叠，且无对应处理分支

### 3.2 kb_fn_action.c 功能重叠

以下函数直接调用底层模块，跨越层级边界：
- `Backlight_Level_Up()` -> 直接调用 `backlight_level_step()`
- `Backlight_Color_Next()` -> 直接调用 `backlight_color_step()`
- `Battery_Check()` -> 直接调用 `battery_get_percentage()` + `indicator_set()`

### 3.3 Factory_Reset 已正确实现

`Factory_Reset()` 使用 `system_taskID + SYSTEM_FACTORY_RESET_EVT`，是 7 个函数中唯一正确工作的。

## 4. 各 service 事件空间现状

| Service | 已用 bits | 剩余可用 | 需新增 |
|---------|----------|---------|--------|
| system_service | bit 0-14（15位） | bit 15（1位） | 无（复用现有） |
| output_service | bit 0-8（9位） | bit 9-15（7位） | OUTPUT_BATTERY_CHECK_EVT |
| input_service | bit 1,3,4,5（4位） | 11位 | INPUT_TOUCH_TOGGLE_EVT |
| communication_service | bit 0-13（12位） | bit 14-15（2位） | 无（复用 WL_PAIR_EVT） |

## 5. 事件映射方案

| 功能 | 目标 service | 事件 | 策略 |
|------|-------------|------|------|
| 背光亮度步进 | output_service | OUTPUT_BACKLIGHT_BRIGHTNESS_EVT (bit 1) | 复用，填充空占位 |
| 背光颜色步进 | output_service | OUTPUT_BACKLIGHT_COLOR_EVT (bit 2) | 复用，填充空占位 |
| 电池检查显示 | output_service | OUTPUT_BATTERY_CHECK_EVT (bit 9，新增) | 新增，读电量+红灯闪烁 |
| 触摸板锁定 | input_service | INPUT_TOUCH_TOGGLE_EVT (bit 2，新增) | 新增，handler 留 TODO |
| BLE 配对 | communication_service | WL_PAIR_EVT (bit 10) | 复用，已有配对流程 |
| 存储写入 | system_service | SYSTEM_STORAGE_EVT (bit 4) | 复用，已有 storage_save |
| 恢复出厂 | system_service | SYSTEM_FACTORY_RESET_EVT (bit 6) | 复用，已正确实现 |
| FN Lock | combo 层直接操作 | SYSTEM_STORAGE_EVT (bit 4) | toggle RAM + 发存储事件 |

## 6. 涉及文件清单

| 文件 | 操作 | 说明 |
|------|------|------|
| `middleware/keyboard/combo/kb_sys_action.c` | 重写 | 删除 SYS_EVT_* 定义，统一使用各 service 事件 |
| `middleware/keyboard/combo/kb_sys_action.h` | 修改 | 更新函数声明 |
| `middleware/keyboard/combo/kb_fn_action.c` | 修改 | 重叠函数改为调用 kb_sys_action 或发 OSAL 事件 |
| `middleware/keyboard/combo/kb_fn_action.h` | 修改 | 删除迁移到 sys_action 的函数声明 |
| `application/service/output_service.h` | 修改 | 新增 OUTPUT_BATTERY_CHECK_EVT |
| `application/service/output_service.c` | 修改 | 填充背光事件处理 + 新增电池检查处理 |
| `application/service/input_service.h` | 修改 | 新增 INPUT_TOUCH_TOGGLE_EVT |
| `application/service/input_service.c` | 修改 | 新增触摸板锁定处理（TODO） |

## 7. 评审意见摘要

- **用户/产品**：OSAL 延迟不可感知；触摸板 TODO 处加 dprintf 标识未实现
- **架构/编码**：各 service 事件空间够用；combo 层 extern 多个 taskID 符合项目惯例
- **测试/质量**：OSAL_SetEvent 位或操作，快速连按合并无害（背光步进幂等）；需回归验证双路径
