# 设计文档：keyboard_task / touchpad_task 采集-处理两步分离

## 1. 背景与问题

项目主循环结构为：

```c
while (1) {
    OSAL_SystemProcessOnce();   // BLE 协议栈 + OSAL 事件
    peripheral_process();        // 键盘 + 触控 + 指示灯
}
```

经测试发现，`peripheral_process()` 中的 `keyboard_task()` 和 `touchpad_task()` 耗时过长，导致 `OSAL_SystemProcessOnce()` 无法及时执行，蓝牙发送出现阻塞。

根本原因：
- `keyboard_scan()` 将 GPIO 矩阵读取、防抖、action 处理、HID 报告生成全部串行执行
- `touch_evt_task()` 将 I2C 读取（软件模拟，~500μs~1ms）、贝叶斯过滤、手势识别、HID 报告全部串行执行
- 两者均在 `peripheral_process()` 中同步阻塞，OSAL/BLE 必须等待

## 2. 设计目标

1. 每次 `peripheral_process()` 的执行时间显著缩短
2. 触控和键盘的数据采集及时性保持
3. 改动范围最小，结构清晰，后续可扩展
4. 为未来切换为中断/DMA 驱动的采集方式预留接口

## 3. 核心方案：采集-处理两步分离

### 3.1 设计原则

将每个耗时任务拆分为两个明确步骤：

| 步骤 | 执行位置 | 职责 | 时效要求 |
|------|---------|------|---------|
| Step 1：采集（acquire） | `peripheral_process()` | 原始数据获取（GPIO/I2C），写入共享 buffer，触发 OSAL 事件 | 高，保持原有响应频率 |
| Step 2：处理（process） | OSAL 事件处理器 | 防抖/手势/HID 报告生成，从 buffer 读取 | 低，允许延迟 1 个循环 |

**共享 buffer** 是两步之间的解耦界面：未来 Step 1 改为 ISR/DMA 驱动，只需替换 acquire 实现，Step 2 完全不变。

### 3.2 keyboard 拆分

**拆前**：
```
keyboard_task() → keyboard_scan()
                    ├── matrix_scan()          [GPIO 读，快]
                    ├── debounce()             [计算，快]
                    └── action_exec()          [处理+HID，可能慢]
```

**拆后**：
```
peripheral_process()
  └── keyboard_acquire()
        ├── matrix_scan()                      [GPIO 读，< 50μs]
        └── OSAL_SetEvent(INPUT_KEYBOARD_PROCESS_EVT)

OSAL INPUT_KEYBOARD_PROCESS_EVT
  └── keyboard_process()
        ├── debounce()
        └── action_exec()                      [防抖+HID 报告]
```

`matrix[]` 数组本身即为共享 buffer（`matrix_scan()` 已写入其中），无需新增数据结构。

### 3.3 touchpad 拆分

**拆前**：
```
touchpad_task() → touch_evt_task()
                    ├── I2C read               [阻塞，~500μs~1ms]
                    ├── bayes_filtering()      [计算]
                    └── HID 报告生成
```

**拆后**：
```
peripheral_process()
  └── touchpad_acquire()
        ├── touch_read_raw(&g_touch_raw_buf)   [I2C 读，~500μs]
        └── OSAL_SetEvent(TOUCHPAD_PROCESS_EVT)

OSAL TOUCHPAD_PROCESS_EVT
  └── touchpad_process()
        ├── bayes_filtering(g_touch_raw_buf)
        └── HID 报告生成
```

新增 `g_touch_raw_buf`（模块内静态变量）作为 I2C 原始数据的共享 buffer。

### 3.4 peripheral_process 改造后

```c
__HIGH_CODE
void peripheral_process(void) {
    // keyboard Step 1：采集
    if (input_get_matrix_scan_flag()) {
        input_clear_matrix_scan_flag();
        keyboard_acquire();
    } else if (g_last_wakeup_source == LPM_WAKEUP_MATRIX) {
        g_last_wakeup_source = LPM_WAKEUP_NONE;
        PMU_Update();
    }

#ifdef TOUCHPAD_ENABLE
    // touchpad Step 1：采集
    if (input_get_touchpad_int_flag()) {
        input_clear_touchpad_int_flag();
        touchpad_acquire();
    }
#endif

    indicator_task();
}
```

## 4. 执行时序对比

```
改造前：
  OSAL_SystemProcessOnce()   [BLE]
  peripheral_process()
    ├─ keyboard_task: GPIO + 防抖 + action + HID  ← 全部串行，可能 >1ms
    └─ touchpad_task: I2C读 + 手势 + HID          ← 全部串行，可能 >1ms
  （BLE 下次处理须等待整个 peripheral_process 完成）

改造后：
  OSAL_SystemProcessOnce()
    ├─ BLE 事件（高优先级）
    ├─ INPUT_KEYBOARD_PROCESS_EVT → 防抖 + action + HID
    └─ TOUCHPAD_DATA_EVT          → 手势 + HID
  peripheral_process()
    ├─ matrix flag → GPIO 读 + SetEvent   [< 50μs]
    ├─ touchpad flag → I2C 读 + SetEvent  [~500μs，阻塞 peripheral_process]
    └─ indicator_task()                   [轻量]
  （BLE 下次处理须等待 peripheral_process 完成，现在只剩采集时间）
```

**说明**：`touchpad_acquire()` 的 I2C 读取（~500μs）仍在 `peripheral_process()` 中同步阻塞，这是当前软件 I2C 的固有限制。改造收益在于将处理阶段（手势识别、HID 报告等）从 `peripheral_process()` 中移除，减少了约 50% 的阻塞时间。彻底消除 I2C 阻塞需升级为硬件 I2C + DMA（见第5节扩展路径）。

## 5. 延伸扩展性

当前 Step 1 仍在 `peripheral_process()` 中同步执行。未来可选择性升级：

- **keyboard acquire**：将 `matrix_scan()` 移入硬件定时器 ISR，直接写 `matrix[]`，ISR 末尾 `OSAL_SetEvent`，`peripheral_process()` 中的 acquire 调用即可移除。
- **touchpad acquire**：将 I2C 改为硬件 I2C + DMA，DMA 完成中断写 `g_touch_raw_buf`，中断末尾 `OSAL_SetEvent`，同样无需改动 Step 2。

两步分离的接口使这些升级路径相互独立，不影响处理逻辑。

## 6. 涉及改动文件

| 文件 | 改动内容 |
|------|---------|
| `middleware/keyboard/keyboard.c` | 拆 `keyboard_scan()` 为 `keyboard_acquire()`（仅 `matrix_scan()`）+ `keyboard_process()`（防抖+action）；保留 `keyboard_scan()` 作为两者的顺序调用，供非 OSAL 路径复用 |
| `middleware/keyboard/keyboard.h` | 新增 `keyboard_acquire()` / `keyboard_process()` 声明 |
| `middleware/touchpad/touchpad.c` | 拆 `touch_evt_task()` 为 `touchpad_acquire()`（仅 I2C 读，写 `g_touch_raw_buf`）+ `touchpad_process()`（从 buffer 做手势+HID）；新增模块内静态 `g_touch_raw_buf` |
| `middleware/touchpad/touchpad.h` | 新增 `touchpad_acquire()` / `touchpad_process()` 声明（`TOUCHPAD_DATA_EVT` 复用，见下） |
| `application/service/input_service.h` | 新增 `INPUT_KEYBOARD_PROCESS_EVT` 事件定义 |
| `application/service/input_service.c` | 新增 `INPUT_KEYBOARD_PROCESS_EVT` 事件分支，调用 `keyboard_process()` |
| `application/system/system_init.c` | `peripheral_process()` 改为调用 `keyboard_acquire()` / `touchpad_acquire()`；移除直接调用 `keyboard_task()` / `touchpad_task()` |

## 7. 约束与注意事项

**事件定义**：
- `INPUT_KEYBOARD_PROCESS_EVT`：新增，定义在 `input_service.h`，由 `input_taskID` 的 OSAL 处理器响应
- `TOUCHPAD_DATA_EVT`：已存在，定义在 `touchpad.h`，直接复用；`touchpad_acquire()` 触发此事件，处理器改为调用 `touchpad_process()`

**PMU_Update() 说明**：
`PMU_Update()` 是电源管理单元的状态刷新调用，在矩阵唤醒路径（`LPM_WAKEUP_MATRIX`）下负责恢复系统功耗状态。该调用属于 LPM 唤醒逻辑的一部分，不属于键盘扫描逻辑，保留在 `peripheral_process()` 原位即可。

**数据竞争**：
`g_touch_raw_buf` 为模块内静态变量，`acquire`（`peripheral_process()`）写、`process`（OSAL 事件）读。CH584M 单核 + OSAL 协作调度，`peripheral_process()` 和 OSAL 事件处理器不会并发执行，无竞争风险。

**HIGH_CODE 属性**：
`keyboard_acquire()` 和 `touchpad_acquire()` 须标记 `__HIGH_CODE`（在 RAM 中运行），与原 `keyboard_task()` / `touchpad_task()` 保持一致，避免 Flash 读取等待影响响应时序。
