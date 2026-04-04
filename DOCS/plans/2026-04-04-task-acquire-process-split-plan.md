# keyboard/touchpad 采集-处理分离 Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将 `keyboard_scan()` 和 `touch_evt_task()` 拆分为采集（peripheral_process 中执行）和处理（OSAL 事件中执行）两步，消除两者对 BLE 调度的阻塞。

**Architecture:** `peripheral_process()` 只做轻量 flag 检测 + 原始数据采集（GPIO/I2C），通过 `OSAL_SetEvent` 触发数据处理事件。处理逻辑在 OSAL 事件处理器中运行，与 BLE 任务在同一调度层竞争，BLE 优先级更高。共享 buffer（keyboard 用 `matrix_debounced[]` + `g_pending_key_st`，touchpad 用 `g_touch_raw_buf`）是采集和处理的解耦界面。

**Tech Stack:** C, OSAL（WCH CH584M TMOS），软件 I2C（bit-bang），`__HIGH_CODE` 属性

---

## Chunk 1: keyboard 拆分

### Task 1: 拆分 keyboard_scan() → keyboard_acquire() + keyboard_process()

**Files:**
- Modify: `middleware/keyboard/keyboard.c`
- Modify: `middleware/keyboard/keyboard.h`

**背景：** `keyboard_scan()` 当前依次执行：
1. `scan_and_debounce()` — GPIO 矩阵读取，结果写入 `matrix_debounced[]`（采集）
2. `update_key_code_list()` + `combo_task()` + `report_update_proc()` + PMU（处理）

需要在两者之间增加一个静态变量 `g_pending_key_st` 暂存状态，作为两步之间的传递。

- [ ] **Step 1: 在 keyboard.c 中增加共享状态变量**

在 `keyboard.c` 现有静态变量声明区（约第 21 行）后增加：

```c
// 采集阶段暂存的键盘状态，供处理阶段使用
static key_update_st_t g_pending_key_st = NO_KEY_UPDATE;
```

- [ ] **Step 2: 在 keyboard.c 中实现 keyboard_acquire()**

在 `keyboard_scan()` 函数之后添加：

```c
/**
 * @brief 键盘数据采集（Step 1）
 *
 * 执行矩阵 GPIO 扫描，将防抖后结果存入 matrix_debounced[]
 * 和 g_pending_key_st，然后触发 OSAL 处理事件。
 * 应在 peripheral_process() 中调用，替代原 keyboard_task()。
 *
 * @note last_update_state 在此处同步更新，保证
 *       keyboard_get_last_update_state() 在 process 之前
 *       已能返回最新扫描结果（与原 keyboard_scan() 行为一致）。
 */
__HIGH_CODE
void keyboard_acquire(void) {
    g_pending_key_st = scan_and_debounce();
    last_update_state = g_pending_key_st;
    OSAL_SetEvent(input_taskID, INPUT_KEYBOARD_PROCESS_EVT);
}
```

> 注意：`input_taskID` 已在 `input_service.h` 中 extern 声明，需要包含该头文件。

- [ ] **Step 3: 在 keyboard.c 中实现 keyboard_process()**

在 `keyboard_acquire()` 之后添加：

```c
/**
 * @brief 键盘数据处理（Step 2）
 *
 * 从 g_pending_key_st / matrix_debounced[] 读取采集结果，
 * 执行防抖后处理：键码更新、combo、HID 报告生成。
 * 应在 OSAL INPUT_KEYBOARD_PROCESS_EVT 事件处理器中调用。
 */
void keyboard_process(void) {
    key_update_st_t key_st = g_pending_key_st;
    g_pending_key_st = NO_KEY_UPDATE;

    if (key_st == KEY_UPDATE) {
        update_key_code_list();
    }
    combo_task(key_st);
    report_update_proc(key_st);
    del_all_child(_key_code_list_extend);

    if (is_empty(_key_code_list) == false || key_st == KEY_UPDATE) {
        PMU_Update();
        output_service_note_backlight_activity();
    }
}
```

- [ ] **Step 4: 在 keyboard.h 中声明新函数**

在 `keyboard_scan()` 声明之后添加：

```c
/**
 * @brief 键盘矩阵采集（Step 1，peripheral_process 调用）
 *
 * 执行 GPIO 矩阵扫描，结果存入内部 buffer，触发 OSAL 处理事件。
 * 不执行防抖后处理，不生成 HID 报告。
 */
void keyboard_acquire(void);

/**
 * @brief 键盘数据处理（Step 2，OSAL 事件处理器调用）
 *
 * 从内部 buffer 读取采集结果，执行 combo、HID 报告生成。
 */
void keyboard_process(void);
```

- [ ] **Step 5: 在 keyboard.c 中添加 input_service.h 依赖**

在 `keyboard.c` 的 `#include` 区末尾添加：

```c
#include "input_service.h"  // input_taskID, INPUT_KEYBOARD_PROCESS_EVT
```

- [ ] **Step 6: 确认编译通过**

```bash
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build -j4
```
预期：无错误、无警告。（Windows 仿真构建，不烧录）

- [ ] **Step 7: Commit**

```bash
git add middleware/keyboard/keyboard.c middleware/keyboard/keyboard.h
git commit -m "feat(keyboard): add keyboard_acquire/keyboard_process for acquire-process split"
```

---

### Task 2: 新增 INPUT_KEYBOARD_PROCESS_EVT 并接入 input_service

**Files:**
- Modify: `application/service/input_service.h`
- Modify: `application/service/input_service.c`

- [ ] **Step 1: 在 input_service.h 中修复重复定义并新增事件**

当前 `input_service.h` 第 43 行有两处 `INPUT_BATTERY_DETE_EVT` 均定义为 `(1 << 3)`，删除重复行，然后新增：

```c
#define INPUT_KEYBOARD_PROCESS_EVT  (1 << 6)  /* 键盘数据处理事件（采集完成后触发） */
```

最终事件定义区应为（无重复）：

```c
#define INPUT_MATRIX_CHANGED_EVT    (1 << 1)
#define INPUT_TOUCH_TOGGLE_EVT      (1 << 2)
#define INPUT_BATTERY_DETE_EVT      (1 << 3)
#define INPUT_TOUCH_ON_EVT          (1 << 4)
#define INPUT_TOUCH_OFF_EVT         (1 << 5)
#define INPUT_KEYBOARD_PROCESS_EVT  (1 << 6)
```

- [ ] **Step 2: 在 input_service.c 的事件处理器中添加处理分支**

在 `input_process_event()` 函数中，现有事件处理之前（靠近函数开头）添加：

```c
    if (events & INPUT_KEYBOARD_PROCESS_EVT) {
        keyboard_process();
        return (events ^ INPUT_KEYBOARD_PROCESS_EVT);
    }
```

在函数文件顶部确认已 `#include "keyboard.h"`（已有则无需重复）。

- [ ] **Step 3: 确认编译通过**

```bash
cmake --build build -j4
```
预期：无错误。

- [ ] **Step 4: Commit**

```bash
git add application/service/input_service.h application/service/input_service.c
git commit -m "feat(input): add INPUT_KEYBOARD_PROCESS_EVT and keyboard_process dispatch"
```

---

### Task 3: 更新 peripheral_process() 调用 keyboard_acquire()

**Files:**
- Modify: `application/system/system_init.c`

- [ ] **Step 1: 修改 peripheral_process()**

将当前 `keyboard_task()` 调用替换为 `keyboard_acquire()`，同时统一使用 accessor 访问唤醒原因：

```c
__HIGH_CODE
void peripheral_process(void) {
    // keyboard Step 1: 采集（GPIO 读取 + 触发 OSAL 事件）
    if (input_get_matrix_scan_flag()) {
        input_clear_matrix_scan_flag();
        keyboard_acquire();
    } else if (input_get_last_wakeup_source() == LPM_WAKEUP_MATRIX) {
        input_set_last_wakeup_source(LPM_WAKEUP_NONE);
        PMU_Update();
    }

#ifdef TOUCHPAD_ENABLE
    touchpad_task();   // touchpad 暂保持不变，Task 4-6 会更新
#endif

    indicator_task();
}
```

> `g_last_wakeup_source` 是 `input_service.c` 中的静态变量，必须通过 `input_get_last_wakeup_source()` / `input_set_last_wakeup_source()` 访问（已在 `input_service.h` 声明）。若现有代码直接引用变量，一并改为 accessor。

- [ ] **Step 2: 确认 keyboard_task() 可以安全移除**

搜索项目中所有 `keyboard_task` 引用：

```bash
grep -r "keyboard_task" --include="*.c" --include="*.h"
```

预期：只剩 `input_service.c` 中的 `keyboard_task()` 函数定义本身（供保留）。若 `keyboard_task()` 仍有其他调用，不要删除定义，仅将 `peripheral_process()` 中的调用换成 `keyboard_acquire()`。

- [ ] **Step 3: 编译并确认正常**

```bash
cmake --build build -j4
```
预期：无错误。

- [ ] **Step 4: Commit**

```bash
git add application/system/system_init.c
git commit -m "refactor(system): replace keyboard_task with keyboard_acquire in peripheral_process"
```

---

## Chunk 2: touchpad 拆分

### Task 4: 在 touchpad_service 中拆分 touch_task() 为读取和处理两步

**Files:**
- Modify: `component/touch_component/touchpad_service.c`
- Modify: `component/touch_component/include/touchpad_service.h`

**背景：** `touch_task()` 当前根据 `touch_mode` 调用 `ProcessMouseData()` 或 `ProcessPTPData()`，这两个函数内部交替执行 I2C 读取和数据处理。需要新增两个对外函数，将 I2C 读取和数据处理分开：

- `touch_read_data()` — 只做 I2C 读取，存入内部 static buffer
- `touch_process_data()` — 只做数据处理和 HID 发送，从 buffer 读

- [ ] **Step 1: 阅读 touchpad_service.c 中 ProcessMouseData() 和 ProcessPTPData() 的完整实现**

在修改前务必读取这两个函数，理解所有 I2C 调用的位置和局部变量依赖。

- [ ] **Step 2: 在 touchpad_service.c 中增加 raw buffer 结构体和静态变量**

在现有静态变量声明区（约第 80-128 行）后添加：

```c
/* 采集-处理分离用原始数据 buffer */
typedef struct {
    bool    has_data;
    uint8_t status;
    uint8_t button_st;
    uint8_t gesture_st;
    uint16_t x;           // 鼠标模式移动量 X
    uint16_t y;           // 鼠标模式移动量 Y
    touchpad_data_t ptp;  // PTP 模式原始报告
} touch_raw_buf_t;

static touch_raw_buf_t s_touch_raw = {0};
```

- [ ] **Step 3: 实现 touch_read_data() — 只做 I2C 读取**

```c
/**
 * @brief 触控板原始数据采集（Step 1）
 *
 * 从 pct1336 读取状态和运动数据，存入 s_touch_raw。
 * 不做任何数据处理或 HID 发送。
 * @return true 有数据待处理，false 无数据或读取失败
 */
bool touch_read_data(void) {
    s_touch_raw.has_data = false;
    s_touch_raw.status = 0;
    s_touch_raw.button_st = 0;
    s_touch_raw.gesture_st = 0;
    s_touch_raw.x = 0;
    s_touch_raw.y = 0;

    if (touch_mode == TOUCH_MODE_MOUSE) {
        if (pct1336_read_status(&s_touch_raw.status,
                                 &s_touch_raw.button_st,
                                 &s_touch_raw.gesture_st) == 0) {
            return false;
        }
        if ((s_touch_raw.status & TOUCH_STATUS_ERROR) == TOUCH_STATUS_ERROR) {
            pct1336_resume();
            return false;
        }
        if ((s_touch_raw.status & TOUCH_STATUS_GESTURE) == TOUCH_STATUS_GESTURE) {
            if (pct1336_read_mouse(&s_touch_raw.x, &s_touch_raw.y) == 0) {
                return false;
            }
        }
        s_touch_raw.has_data = true;
    } else if (touch_mode == TOUCH_MODE_PTP) {
        if (pct1336_read_status(&s_touch_raw.status,
                                 &s_touch_raw.button_st, NULL) == 0) {
            return false;
        }
        if ((s_touch_raw.status & TOUCH_STATUS_ERROR) == TOUCH_STATUS_ERROR) {
            pct1336_resume();
            return false;
        }
        if ((s_touch_raw.status & TOUCH_STATUS_TOUCH) == TOUCH_STATUS_TOUCH) {
            if (pct1336_read_ptp_report(&s_touch_raw.ptp) == 0) {
                return false;
            }
        }
        s_touch_raw.has_data = true;
    }
    return s_touch_raw.has_data;
}
```

> **注意：** 此函数仅覆盖 I2C 读取路径。原 `ProcessMouseData()` 和 `ProcessPTPData()` 中可能还有其他 I2C 读（如多次读取 status 等），实现时需参照原函数完整逻辑确保不遗漏。

- [ ] **Step 4: 实现 touch_process_data() — 只做数据处理和 HID 发送**

```c
/**
 * @brief 触控板数据处理（Step 2）
 *
 * 从 s_touch_raw 读取采集结果，执行数据格式转换、
 * 防误触过滤、HID 报告构建和无线发送。
 * 不做任何 I2C 读取。
 */
void touch_process_data(void) {
    if (!s_touch_raw.has_data) {
        return;
    }

    if (touch_mode == TOUCH_MODE_MOUSE) {
        // 从 s_touch_raw 中取出 gesture_st, button_st, x, y
        // 执行原 ProcessMouseData() 中 pct1336_read_mouse() 之后的所有逻辑
        // 包括：switch(gesture_st)、build mouse_reports、call send_touch_data()
        // *** 实现者：将 ProcessMouseData() 中 pct1336_read_mouse() 调用之后的
        //     代码段移至此处，将局部变量 gesture_st/x/y 替换为 s_touch_raw 字段 ***
        pct1336_clear_int();
    } else if (touch_mode == TOUCH_MODE_PTP) {
        // 从 s_touch_raw.ptp 读取，执行原 ProcessPTPData() 中 read 之后的所有逻辑
        // *** 实现者：将 ProcessPTPData() 中 pct1336_read_ptp_report() 之后的
        //     代码段移至此处，将 original_reports 替换为 s_touch_raw.ptp ***
    }
}
```

> **实现者注意：** `touch_process_data()` 的具体实现需要将 `ProcessMouseData()` 和 `ProcessPTPData()` 中各自 I2C 读取之后的代码段提取出来。这两个函数逻辑较长，移植时请逐行对照原函数，确保所有 button 处理、滚轮、手势判断、HID 发送路径均被覆盖。

- [ ] **Step 5: 在 touchpad_service.h 中声明新函数**

在 `touch_evt_task()` 声明之后添加：

```c
/**
 * @brief 触控板数据采集（Step 1，peripheral_process 调用）
 * @return true 有数据，false 无数据/读取失败
 */
bool touch_read_data(void);

/**
 * @brief 触控板数据处理（Step 2，OSAL 事件处理器调用）
 */
void touch_process_data(void);
```

- [ ] **Step 6: 确认编译通过**

```bash
cmake --build build -j4
```
预期：无错误。原有 `touch_evt_task()` 保留，不影响现有路径。

- [ ] **Step 7: Commit**

```bash
git add component/touch_component/touchpad_service.c \
        component/touch_component/include/touchpad_service.h
git commit -m "feat(touchpad-service): add touch_read_data/touch_process_data for acquire-process split"
```

---

### Task 5: 在 touchpad middleware 中实现 touchpad_acquire() 和 touchpad_process()

**Files:**
- Modify: `middleware/touchpad/touchpad.c`
- Modify: `middleware/touchpad/touchpad.h`

- [ ] **Step 1: 在 touchpad.c 中实现 touchpad_acquire()**

在 `touchpad_task()` 函数之后添加：

```c
/**
 * @brief 触控板数据采集（Step 1，peripheral_process 调用）
 *
 * 检查触控使能状态，调用 touch_read_data() 完成 I2C 读取，
 * 触发 OSAL TOUCHPAD_DATA_EVT 事件交由处理阶段执行。
 *
 * @note touch_en 通过 touchpad_service.h 的 extern 声明可见，
 *       touchpad.c 已 #include "touchpad_service.h"，无需额外 getter。
 */
__HIGH_CODE
void touchpad_acquire(void) {
    if (touch_en != 1) {
        return;
    }
    PMU_Update();
    if (touch_read_data()) {
        OSAL_SetEvent(touchpad_taskID, TOUCHPAD_DATA_EVT);
    }
}
```

- [ ] **Step 2: 在 touchpad.c 中实现 touchpad_process()**

```c
/**
 * @brief 触控板数据处理（Step 2，OSAL TOUCHPAD_DATA_EVT 处理器调用）
 *
 * 从内部 buffer 执行数据格式转换和 HID 报告发送，不做 I2C 读取。
 */
void touchpad_process(void) {
    if (touch_en != 1) {
        return;
    }
    touch_process_data();
}
```

- [ ] **Step 3: 阅读 touchpad_process_event() 当前完整实现（middleware/touchpad/touchpad.c 约第 171-215 行），再更新 TOUCHPAD_DATA_EVT 处理**

将现有：

```c
    if (events & TOUCHPAD_DATA_EVT) {
        touch_evt_task();
        return (events ^ TOUCHPAD_DATA_EVT);
    }
```

改为：

```c
    if (events & TOUCHPAD_DATA_EVT) {
        touchpad_process();
        return (events ^ TOUCHPAD_DATA_EVT);
    }
```

- [ ] **Step 4: 在 touchpad.h 中声明新函数**

在 `touchpad_task()` 声明之后添加：

```c
/* 触控板数据采集，peripheral_process 中替代 touchpad_task() 调用。 */
void touchpad_acquire(void);

/* 触控板数据处理，OSAL TOUCHPAD_DATA_EVT 处理器内部调用。 */
void touchpad_process(void);
```

- [ ] **Step 5: 确认编译通过**

```bash
cmake --build build -j4
```
预期：无错误。

- [ ] **Step 6: Commit**

```bash
git add middleware/touchpad/touchpad.c middleware/touchpad/touchpad.h
git commit -m "feat(touchpad): add touchpad_acquire/touchpad_process and wire TOUCHPAD_DATA_EVT"
```

---

### Task 6: 更新 peripheral_process() 调用 touchpad_acquire()

**Files:**
- Modify: `application/system/system_init.c`

- [ ] **Step 1: 将 touchpad_task() 调用替换为 touchpad_acquire()**

将 Task 3 修改后的 `peripheral_process()` 中的 `touchpad_task()` 替换：

```c
__HIGH_CODE
void peripheral_process(void) {
    // keyboard Step 1: 采集
    if (input_get_matrix_scan_flag()) {
        input_clear_matrix_scan_flag();
        keyboard_acquire();
    } else if (input_get_last_wakeup_source() == LPM_WAKEUP_MATRIX) {
        input_set_last_wakeup_source(LPM_WAKEUP_NONE);
        PMU_Update();
    }

#ifdef TOUCHPAD_ENABLE
    // touchpad Step 1: 采集
    if (input_get_touchpad_int_flag()) {
        input_clear_touchpad_int_flag();
        touchpad_acquire();
    }
#endif

    indicator_task();
}
```

> `g_last_wakeup_source` 的直接访问已被 `input_get/set_last_wakeup_source()` 接口替代，
> 若原代码直接引用变量，改为调用接口函数。

- [ ] **Step 2: 确认 touchpad_task() 调用已完全移除**

```bash
grep -r "touchpad_task\(\)" application/system/system_init.c
```
预期：无输出。

- [ ] **Step 3: 编译通过**

```bash
cmake --build build -j4
```
预期：无错误。

- [ ] **Step 4: Commit**

```bash
git add application/system/system_init.c
git commit -m "refactor(system): replace touchpad_task with touchpad_acquire in peripheral_process"
```

---

## Chunk 3: 验证

### Task 7: 编译固件并烧录验证

- [ ] **Step 1: 编译 CH584M 固件**

```bash
cmd.exe /c "cd /d D:\\Code\\C_Project\\keyboard-framework\\project\\ch584m\\obj && \
  set PATH=D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\components\\WCH\\Toolchain\\RISC-V Embedded GCC12\\bin;D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\others\\Build_Tools\\Make\\bin;%PATH% && \
  make.exe clean && make.exe main-build"
```

预期：生成 `keyboard-framework.hex`，无错误。

- [ ] **Step 2: 烧录验证**

使用 WCH-LinkUtility 或 WCHISPTool 烧录 `keyboard-framework.hex`。

- [ ] **Step 3: 验证 BLE 蓝牙稳定性**

连接蓝牙，进行持续按键输入和触控板操作，观察：
- 按键输入无延迟/卡顿
- 触控板响应流畅
- 蓝牙连接不出现发送阻塞（之前的问题现象）

- [ ] **Step 4: 验证 keyboard 时序**

快速连续按键，确认：
- 无按键丢失
- combo 键（组合键）正常触发

- [ ] **Step 5: 验证 touchpad 时序**

快速移动和点击，确认：
- 光标移动流畅（无明显延迟）
- 左右键点击正常
- PTP/鼠标模式均正常（根据实际配置）

- [ ] **Step 6: 最终 Commit（如有残余改动）**

```bash
git add -p
git commit -m "chore: finalize acquire-process split after hardware validation"
```
