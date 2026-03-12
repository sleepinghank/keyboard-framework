# 矩阵扫描重构设计文档

## 概述

将矩阵扫描定时器由 OSAL 事件调度改为 main loop 标志位执行，减少调度压力和延迟。

## 动机

当前架构中，硬件定时器中断通过 `OSAL_SetEvent()` 触发 `INPUT_MATRIX_SCAN_EVT` 事件，由 OSAL 主循环调度执行 `keyboard_task()`。这种方式存在以下问题：

1. 事件队列调度增加延迟
2. OSAL 事件处理增加系统开销
3. 矩阵扫描是高频任务（5ms 周期），应该有更直接的执行路径

## 设计方案

### 架构变更

```
原架构：
硬件定时器中断 → OSAL_SetEvent() → 事件队列 → input_process_event() → keyboard_task()

新架构：
硬件定时器中断 → 设置 volatile 标志位 → main loop 直接检查 → keyboard_task()
```

### 改动文件清单

| 文件 | 改动类型 | 说明 |
|------|----------|------|
| `application/main.c` | 修改 | 重构主循环结构 |
| `application/service/input_service.c` | 修改 | 添加标志位，移除事件触发 |
| `application/service/input_service.h` | 修改 | 添加接口声明，移除事件定义 |
| `drivers/system/platforms/ch584/event_manager.c` | 修改 | 拆分单次执行函数 |
| `drivers/system/event_manager.h` | 修改 | 新增函数声明 |

### 详细设计

#### 1. input_service.h 接口变更

**新增接口：**

```c
/**
 * @brief 检查是否需要执行矩阵扫描
 * @return true 需要执行, false 不需要
 */
bool input_get_matrix_scan_flag(void);

/**
 * @brief 设置矩阵扫描标志位（中断中调用）
 */
void input_set_matrix_scan_flag(void);

/**
 * @brief 清除矩阵扫描标志位
 */
void input_clear_matrix_scan_flag(void);
```

**移除定义：**

```c
typedef enum {
    // INPUT_MATRIX_SCAN_EVT = 1 << 0,  // 删除
    INPUT_MATRIX_CHANGED_EVT  = 1 << 1,
    INPUT_TOUCH_INT_EVT       = 1 << 2,
    INPUT_BATTERY_DETE_EVT    = 1 << 3,
    INPUT_TOUCH_WATCHDOG_EVT  = 1 << 4,
} input_task_event_t;
```

#### 2. input_service.c 实现

```c
/* 矩阵扫描标志位 - volatile 保证中断可见性 */
static volatile bool g_matrix_scan_flag = false;

/* 回调函数：改为设置标志位 */
__HIGH_CODE
static void matrix_scan_timer_callback(void)
{
    g_matrix_scan_flag = true;
}

/* 标志位接口实现 */
bool input_get_matrix_scan_flag(void) {
    return g_matrix_scan_flag;
}

void input_set_matrix_scan_flag(void) {
    g_matrix_scan_flag = true;
}

void input_clear_matrix_scan_flag(void) {
    g_matrix_scan_flag = false;
}
```

**input_process_event() 移除 INPUT_MATRIX_SCAN_EVT 处理分支。**

#### 3. event_manager.c 新增单次执行函数

```c
/**
 * @brief   OSAL 系统单次处理（非阻塞）
 *          处理一轮事件后返回，用于自定义主循环
 */
void OSAL_SystemProcessOnce(void) {
    TMOS_SystemProcess();
}
```

#### 4. main.c 主循环重构

```c
#include "input_service.h"  // 新增

int main(void)
{
    uint32_t init_result = system_init_coordinator();
    if (init_result != 0) {
        dprintf("ERROR: System initialization failed! Error code: %d\r\n", init_result);
        return init_result;
    }
    dprint("System initialized successfully");

    /* 主循环：矩阵扫描 + OSAL 事件处理 */
    while (1) {
        /* 检查矩阵扫描标志位 */
        if (input_get_matrix_scan_flag()) {
            keyboard_task();
            input_clear_matrix_scan_flag();
        }

        /* OSAL 单次处理（BLE 协议栈等） */
        OSAL_SystemProcessOnce();
    }

    return 0;
}
```

## 线程安全

使用 `volatile` 关键字保证标志位的中断可见性。CH584 平台上单字节 `bool` 类型的读写是原子的，无需额外的关中断保护。

## 验证方案

### 编译验证

```bash
cmd.exe /c "cd /d D:\Code\C_Project\keyboard-framework\project\ch584m\obj && make.exe clean && make.exe main-build"
```

### 功能验证

1. 烧录固件后，按键应正常响应
2. 示波器/逻辑分析仪检查 `togglePin(B14)` 仍能验证定时器触发
3. BLE 连接正常，不影响协议栈运行

## 影响范围

- `INPUT_MATRIX_SCAN_EVT` 事件完全移除
- `openspec/specs/matrix-scan-timer/spec.md` 需同步更新

## 决策记录

| 决策点 | 选择 | 原因 |
|--------|------|------|
| 线程安全方案 | 仅 volatile | CH584 单字节原子写入，简单高效 |
| 标志位归属模块 | input_service | 职责清晰，回调函数在同一模块 |
| 是否移除事件定义 | 是 | 彻底清理，避免歧义 |
