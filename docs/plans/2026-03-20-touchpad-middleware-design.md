# 触控板模块化接入 — 设计文档

**日期：** 2026-03-20
**状态：** 已确认
**分支：** feature/touch
**需求文档：** docs/plans/2026-03-20-touchpad-middleware-requirements.md

---

## 1. 需求摘要

将 component/touch_component 模块化接入键盘框架：
- middleware 层新建 touchpad 模块，自注册 OSAL 任务，管理完整生命周期
- middleware 只管自身生命周期，跨模块交互上抛 service 层
- HID 报告在 touch_component 内生成并发送（快速实现，TODO 对齐 wireless_transport）
- 本次不含 LPM 集成
- touch_component 最小化改动

## 2. 架构设计

### 2.1 分层结构

```
Application Layer
  └── input_service
       ├── GPIO 中断 → touchpad_notify_int()
       ├── 看门狗定时器 → touchpad_watchdog_check()
       └── LPM prepare/resume（本次不实现）

Middleware Layer
  └── touchpad/touchpad.c（新建）
       ├── touchpad_taskID（独立 OSAL 任务）
       ├── 生命周期：init / power_on / power_off
       ├── 内部事件：INIT / REG_INIT / FW_POLL / DATA
       └── HID 报告由 touch_component 生成并发送（TODO）

Component Layer（最小改动）
  └── touch_component/
       ├── touchpad_service.c  — 删除 OSAL 逻辑，保留业务函数
       ├── pct1336_driver.c    — 新增 pct1336_fw_ready() 查询接口
       ├── mcs_gesture.c       — 不改动
       └── bayes_filtering.c   — 不改动
```

### 2.2 调用方向

```
input_service → middleware/touchpad → touch_component
```

单向依赖，不可反转。

### 2.3 事件边界

| 归属 | 事件 | 用途 |
|------|------|------|
| middleware/touchpad taskID | TOUCHPAD_INIT_EVT | 触发初始化序列 |
| middleware/touchpad taskID | TOUCHPAD_FW_POLL_EVT | 10ms 轮询固件就绪 |
| middleware/touchpad taskID | TOUCHPAD_REG_INIT_EVT | 200ms 延迟后寄存器初始化 |
| middleware/touchpad taskID | TOUCHPAD_DATA_EVT | 处理触控数据 |
| input_service taskID | INPUT_TOUCH_INT_EVT | GPIO 中断转发 |
| input_service taskID | INPUT_TOUCH_WATCHDOG_EVT | 2000ms 看门狗检查 |

## 3. 组件/接口设计

### 3.1 新建文件：middleware/touchpad/touchpad.h

```c
#pragma once

#include <stdint.h>
#include <stdbool.h>

// 事件定义
#define TOUCHPAD_INIT_EVT        (1 << 0)
#define TOUCHPAD_REG_INIT_EVT    (1 << 1)
#define TOUCHPAD_FW_POLL_EVT     (1 << 2)
#define TOUCHPAD_DATA_EVT        (1 << 3)

// 生命周期接口（供 input_service 调用）
void touchpad_init(void);
void touchpad_power_on(void);
void touchpad_power_off(void);

// 事件触发接口（供 input_service 转发）
void touchpad_notify_int(void);
void touchpad_watchdog_check(void);

// 模式配置
void touchpad_set_mode(uint8_t mode);
```

### 3.2 新建文件：middleware/touchpad/touchpad.c

| 函数 | 职责 |
|------|------|
| touchpad_init() | OSAL_ProcessEventRegister() 注册任务，获取 touchpad_taskID |
| touchpad_power_on() | 调用 touch_power_on()（含阻塞硬件初始化），触发 TOUCHPAD_INIT_EVT |
| touchpad_power_off() | 调用 touch_power_off() |
| touchpad_process_event() | OSAL 事件分发（见下方状态机） |
| touchpad_notify_int() | OSAL_SetEvent(touchpad_taskID, TOUCHPAD_DATA_EVT) |
| touchpad_watchdog_check() | 调用 touch_watchdog_check()，异常时重走初始化 |
| touchpad_set_mode() | 调用 set_touch_mode() |

### 3.3 touch_component 改动

**touchpad_service.c：**
- 删除 `touch_taskID` 全局变量
- 删除 `touch_process_event()` 函数
- 删除 4 行 OSAL 调用（SetEvent/StartReloadTask/StopTask/ProcessEventRegister）

**pct1336_driver.c：**
- 新增 `bool pct1336_fw_ready(void)` 查询接口（只读寄存器判断固件是否就绪）
- 原有 OSAL 逻辑（pct1336_taskID/pct1336_process_event）删除

**不改动：** mcs_gesture.c、bayes_filtering.c

### 3.4 input_service 改动

| 位置 | 原代码 | 改为 |
|------|--------|------|
| input_service_init() | touch_power_on() | touchpad_init() + touchpad_power_on() |
| INPUT_TOUCH_INT_EVT | touch_evt_task() | touchpad_notify_int() |
| INPUT_TOUCH_WATCHDOG_EVT | touch_watchdog_check() | touchpad_watchdog_check() |

## 4. 数据流

### 4.1 初始化序列

```
input_service_init()
  → touchpad_init()              // 注册 OSAL 任务
  → touchpad_power_on()          // 直接调用路径（非事件回调）
      → touch_power_on()         // 硬件上电（含阻塞等待）
      → OSAL_SetEvent(TOUCHPAD_INIT_EVT)
          ↓
TOUCHPAD_INIT_EVT:
  → 软件初始化
  → 启动 TOUCHPAD_FW_POLL_EVT（10ms 周期）
      ↓
TOUCHPAD_FW_POLL_EVT（循环，非阻塞）:
  → pct1336_fw_ready()           // 只查询，不阻塞
  → 未就绪：继续轮询
  → 就绪：停止轮询，启动 TOUCHPAD_REG_INIT_EVT（200ms 延迟）
      ↓
TOUCHPAD_REG_INIT_EVT:
  → 寄存器初始化完成，触控板就绪
```

### 4.2 运行时数据流

```
GPIO 中断 → input_service INPUT_TOUCH_INT_EVT
  → touchpad_notify_int()
    → OSAL_SetEvent(TOUCHPAD_DATA_EVT)
        ↓
TOUCHPAD_DATA_EVT:
  → touch_evt_task()
    → ProcessPTPData() / ProcessMouseData()
    → gesture_judgment()
    → send_ptp_data() → 发送 HID 报告（TODO：对齐 wireless_transport）
```

### 4.3 看门狗流程

```
input_service 2000ms 定时器 → INPUT_TOUCH_WATCHDOG_EVT
  → touchpad_watchdog_check()
    → touch_watchdog_check()
    → 异常：touchpad_power_off() + touchpad_power_on() 重新初始化
```

## 5. 错误处理

| 场景 | 处理 |
|------|------|
| FW 轮询超时（10ms x 50 次 = 500ms） | 停止轮询，记录错误日志，不阻塞系统 |
| 看门狗检测到 I2C 异常 | 重新走初始化序列 |
| 初始化失败 | 触控板功能不可用，不影响键盘功能 |

## 6. 已知风险与 TODO

| # | 风险 | 影响 | 处理方式 |
|---|------|------|---------|
| 1 | HID 报告未走 wireless_transport | 多模切换可能异常 | TODO 标注，后续对齐 |
| 2 | 无 LPM 集成 | 初始化阶段 10ms 轮询阻止睡眠（轮询结束后无影响） | 后续迭代，已知缺陷 |
| 3 | TMOS taskID 数量 | 删除 pct1336_taskID + touch_taskID，新增 touchpad_taskID，净减 1 | 需确认上限 |

## 7. 构建配置

### CMakeLists.txt 改动

- 取消注释 touch_component 4 个源文件
- 新增 middleware/touchpad/touchpad.c
- 新增头文件路径 middleware/touchpad/
- 确保 TOUCH_EN 宏定义

## 8. 实施计划

### Step 1：新建 middleware/touchpad 模块

**文件：** `middleware/touchpad/touchpad.h`、`middleware/touchpad/touchpad.c`

**具体内容：**
- `touchpad.h`：事件定义（4 个 EVT）、6 个接口函数声明
- `touchpad.c`：
  - `static uint8_t touchpad_taskID` — 模块私有
  - `static uint8_t touchpad_fw_poll_count` — FW 轮询计数器（上限 50）
  - `touchpad_init()` — 调用 `OSAL_ProcessEventRegister(touchpad_process_event)`
  - `touchpad_power_on()` — 调用 `touch_power_on()` + `OSAL_SetEvent(TOUCHPAD_INIT_EVT)`
  - `touchpad_power_off()` — 调用 `touch_power_off()`
  - `touchpad_process_event()` — 四个事件分支：
    - INIT：软件初始化 + 启动 FW_POLL（10ms 周期）
    - FW_POLL：调用 `pct1336_fw_ready()`，就绪则停止轮询并启动 REG_INIT（200ms），超时则记录日志
    - REG_INIT：寄存器初始化完成
    - DATA：调用 `touch_evt_task()` 处理触控数据
  - `touchpad_notify_int()` — `OSAL_SetEvent(touchpad_taskID, TOUCHPAD_DATA_EVT)`
  - `touchpad_watchdog_check()` — 调用 `touch_watchdog_check()`，异常重走初始化
  - `touchpad_set_mode()` — 调用 `set_touch_mode()`

**前置条件：** 无
**验证：** grep 确认接口签名与设计一致

---

### Step 2：改动 touch_component

**文件：** `component/touch_component/touchpad_service.c`、`component/touch_component/pct1336_driver.c`

**touchpad_service.c 删除：**
- `touch_taskID` 全局变量
- `touch_process_event()` 函数体
- 所有 `OSAL_SetEvent`、`OSAL_StartReloadTask`、`OSAL_StopTask`、`OSAL_ProcessEventRegister` 调用

**pct1336_driver.c：**
- 新增 `bool pct1336_fw_ready(void)` — 读取寄存器判断固件状态，返回 bool
- 删除 `pct1336_taskID`、`pct1336_process_event()` 函数
- 删除所有 `OSAL_StartReloadTask`、`OSAL_StopTask`、`OSAL_ProcessEventRegister` 调用

**前置条件：** Step 1 完成
**验证：** `grep -r "OSAL_ProcessEventRegister\|pct1336_taskID\|touch_taskID" component/touch_component/` 返回空

---

### Step 3：改动 input_service

**文件：** `application/service/input_service.c`

**改动点：**
- `input_service_init()` 中 `touch_power_on()` → `touchpad_init()` + `touchpad_power_on()`
- `INPUT_TOUCH_INT_EVT` 处理中 `touch_evt_task()` → `touchpad_notify_int()`
- `INPUT_TOUCH_WATCHDOG_EVT` 处理中 `touch_watchdog_check()` → `touchpad_watchdog_check()`
- 头文件 include：增加 `#include "touchpad.h"`

**前置条件：** Step 1、Step 2 完成
**验证：** grep 确认 input_service 中无直接调用 touch_component 函数

---

### Step 4：更新构建配置

**改动：**
- 取消注释 touch_component 4 个源文件
- 新增 `middleware/touchpad/touchpad.c`
- 新增头文件路径 `middleware/touchpad/`
- 确认 `TOUCH_EN` 宏定义存在

**前置条件：** Step 1-3 完成
**验证：** 使用skills wch-riscv-build 进行编译

---

### Step 5：集成验证

**内容：**
- CH584M 烧录，串口日志确认：INIT → FW_POLL → REG_INIT 按序完成
- 触控板触摸测试，确认 HID 报告正常
- 键盘打字回归，确认无功能异常
- `TOUCH_EN` 宏注释后重新编译，确认编译通过

**前置条件：** Step 4 编译通过
**验证：** 所有测试项通过
