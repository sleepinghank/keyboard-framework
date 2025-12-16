# TMOS任务事件调度系统分析报告

## 目录
1. [TMOS概述](#tmos概述)
2. [代码流程分析](#代码流程分析)
3. [实现方式详解](#实现方式详解)
4. [使用方法](#使用方法)
5. [项目应用实例](#项目应用实例)
6. [优缺点分析](#优缺点分析)
7. [优化建议](#优化建议)

---

## TMOS概述

TMOS（Task Manager Operating System）是沁恒微电子为其蓝牙协议栈CH58xBLE设计的一个**轻量级任务事件调度系统**。它采用事件驱动模型，为嵌入式应用提供高效的任务管理和时间调度功能。

### 设计目标
- 为蓝牙协议栈提供任务调度支持
- 最小化系统资源占用
- 提供简单易用的任务管理API
- 支持事件驱动编程模型

---

## 代码流程分析

### 1. 系统初始化流程

```c
// main.c:113-114
SysTick_Config(0xFFFFFFFF);        // 配置系统滴答定时器，TMOS所需时间基准
PFIC_DisableIRQ(SysTick_IRQn);     // 禁用系统滴答中断，由TMOS_SystemProcess()手动驱动

// main.c:49-51 - 主循环
void process_main(void) {
    while(1) {
        TMOS_SystemProcess();      // TMOS主调度器
        peripheral_process();      // 外设处理
    }
}
```

**关键点**：
- 使用系统滴答定时器作为时间基准
- 通过手动轮询方式驱动TMOS调度
- 无中断嵌套，保持简单可控

### 2. 任务注册流程

```c
// 标准任务注册模式（以keyboard.c为例）
uint8_t keyboard_taskID = 0;

// 1. 定义任务事件处理器
uint16_t keyboard_process_event(uint8_t task_id, uint16_t events) {
    if(events & KEYBOARD_LED_BLINK_EVENT) {
        // 处理LED闪烁
        return events ^ KEYBOARD_LED_BLINK_EVENT;
    }
    // 处理其他事件...
    return 0;
}

// 2. 在模块初始化时注册任务
void keyboard_init() {
    keyboard_taskID = TMOS_ProcessEventRegister(keyboard_process_event);
    // 启动定时任务
    tmos_start_reload_task(keyboard_taskID, KEYBOARD_VBAT_INFO_EVENT, GET_VBAT_INFO_INTERVAL);
}
```

**关键点**：
- 每个模块独立注册自己的任务
- 返回未处理的事件标志
- 事件处理后需要清除对应标志位

### 3. 事件调度流程

```c
// 事件触发的三种方式

// 方式1：立即事件
tmos_set_event(keyboard_taskID, KEYBOARD_LED_BLINK_EVENT);

// 方式2：单次延时事件
tmos_start_task(keyboard_taskID, KEYBOARD_VBAT_LED_TIMEOUT_EVENT,
                MS1_TO_SYSTEM_TIME(2000));

// 方式3：循环定时事件
tmos_start_reload_task(keyboard_taskID, KEYBOARD_VBAT_INFO_EVENT,
                       GET_VBAT_INFO_INTERVAL);
```

---

## 使用方法

### 1. 基础使用模式

#### 步骤1：定义事件类型

```c
// 在模块头文件中定义
#define KEYBOARD_LED_BLINK_EVENT     (1<<0)   // LED闪烁事件
#define KEYBOARD_VBAT_INFO_EVENT     (1<<2)   // 电量检测事件
#define KEYBOARD_BACKLIGHT_TIMEOUT_EVENT  (1<<4)   // 背光超时事件
```

#### 步骤2：实现事件处理器

```c
uint16_t keyboard_process_event(uint8_t task_id, uint16_t events) {
    // 必须返回未处理的事件
    uint16_t unhandled = 0;

    // 检查并处理每个事件
    if(events & KEYBOARD_LED_BLINK_EVENT) {
        // LED闪烁逻辑
        GPIOA_InverseBits(GPIO_Pin_15);

        // 清除已处理的事件位
        events ^= KEYBOARD_LED_BLINK_EVENT;
    }

    if(events & KEYBOARD_VBAT_INFO_EVENT) {
        // 电量检测逻辑
        vbat_info = keyboard_get_batt_info();

        // 低电压告警
        if(vbat_led_num == 6) {
            tmos_start_task(keyboard_taskID, KEYBOARD_VBAT_LED_TIMEOUT_EVENT,
                          MS1_TO_SYSTEM_TIME(2000));
        }

        events ^= KEYBOARD_VBAT_INFO_EVENT;
    }

    // 处理完成后返回未处理的事件
    return events;
}
```

#### 步骤3：注册任务并启动事件

```c
void keyboard_init() {
    // 1. 注册任务，获取taskID
    keyboard_taskID = TMOS_ProcessEventRegister(keyboard_process_event);

    // 2. 启动循环定时事件（周期性执行）
    tmos_start_reload_task(keyboard_taskID, KEYBOARD_VBAT_INFO_EVENT,
                          GET_VBAT_INFO_INTERVAL);

    // 3. 可以随时触发立即事件
    tmos_set_event(keyboard_taskID, KEYBOARD_LED_BLINK_EVENT);
}
```

### 2. 高级使用技巧

#### 2.1 事件组合处理

```c
// 同时等待多个事件
uint16_t my_process_event(uint8_t task_id, uint16_t events) {
    if(events & (EVENT_A | EVENT_B)) {  // 处理A或B事件
        // 共享处理逻辑
        return events ^ (EVENT_A | EVENT_B);
    }
    return 0;
}
```

#### 2.2 事件转发

```c
// 将事件转发给其他任务
if(events & FORWARD_TO_OTHER_TASK_EVENT) {
    tmos_set_event(other_taskID, OTHER_TASK_EVENT);
    return events ^ FORWARD_TO_OTHER_TASK_EVENT;
}
```

#### 2.3 动态启动/停止事件

```c
// 根据条件启动事件
if(condition) {
    tmos_start_reload_task(keyboard_taskID, PERIODIC_EVENT, INTERVAL);
} else {
    tmos_stop_task(keyboard_taskID, PERIODIC_EVENT);
}
```

---

## 项目应用实例

### 实例1：键盘LED闪烁控制 (keyboard.c)

**场景**：键盘需要控制LED闪烁来指示不同状态（配对、断开连接、低电量）

```c
// 1. 定义事件和变量
#define KEYBOARD_LED_BLINK_EVENT            1<<0
#define KEYBOARD_VBAT_LED_BLINK_EVENT       1<<6
#define KEYBOARD_VBAT_LED_TIMEOUT_EVENT     1<<7

#define PAIRING_LED_BLINK_INTERVAL          1600*1/4  // 快闪：400ms
#define DISCONNECT_LED_BLINK_INTERVAL       1600*1/2  // 慢闪：800ms

uint8_t keyboard_taskID = 0;
uint8_t vbat_led_blink_count = 0;      // 闪烁计数
uint8_t vbat_led_blink_state = 0;      // 闪烁状态

// 2. 事件处理函数
uint16_t keyboard_process_event(uint8_t task_id, uint16_t events) {
    // LED闪烁事件
    if(events & KEYBOARD_LED_BLINK_EVENT) {
        GPIOA_InverseBits(GPIO_Pin_15);           // 翻转LED引脚
        return events ^ KEYBOARD_LED_BLINK_EVENT;
    }

    // 低电量闪烁事件
    if(events & KEYBOARD_VBAT_LED_BLINK_EVENT) {
        vbat_led_blink_count++;

        if(vbat_led_blink_count >= 10) {  // 闪烁10次后停止
            vbat_led_blink_count = 0;
            tmos_stop_task(keyboard_taskID, KEYBOARD_VBAT_LED_BLINK_EVENT);
            tmos_stop_task(keyboard_taskID, KEYBOARD_VBAT_LED_TIMEOUT_EVENT);
            GPIOB_ModeCfg(GPIO_Pin_23, GPIO_ModeIN_PD);  // 关闭LED
        } else {
            // 翻转LED状态
            vbat_led_blink_state = !vbat_led_blink_state;
            if(vbat_led_blink_state) {
                GPIOB_SetBits(GPIO_Pin_23);
            } else {
                GPIOB_ResetBits(GPIO_Pin_23);
            }
        }
        return events ^ KEYBOARD_VBAT_LED_BLINK_EVENT;
    }

    // 超时停止事件
    if(events & KEYBOARD_VBAT_LED_TIMEOUT_EVENT) {
        tmos_stop_task(keyboard_taskID, KEYBOARD_VBAT_LED_BLINK_EVENT);
        GPIOB_ModeCfg(GPIO_Pin_23, GPIO_ModeIN_PD);
        return events ^ KEYBOARD_VBAT_LED_TIMEOUT_EVENT;
    }

    return 0;
}

// 3. 启动闪烁的公共接口
void keyboard_start_pairing_indicator() {
    // 启动快闪
    tmos_start_reload_task(keyboard_taskID, KEYBOARD_LED_BLINK_EVENT,
                          PAIRING_LED_BLINK_INTERVAL);
}

void keyboard_start_low_voltage_indicator() {
    // 启动慢闪10次，然后超时停止
    vbat_led_blink_count = 0;
    tmos_start_reload_task(keyboard_taskID, KEYBOARD_VBAT_LED_BLINK_EVENT,
                          VBAT_LED_BLINK_INTERVAL);
    tmos_start_task(keyboard_taskID, KEYBOARD_VBAT_LED_TIMEOUT_EVENT,
                   VBAT_LED_BLINK_TIMEOUT);
}
```

**应用场景**：
- 配对模式：LED快闪指示可配对状态
- 断开连接：LED慢闪指示连接断开
- 低电量：LED闪烁10次后自动停止

### 实例2：电量检测与低功耗管理 (access.c)

**场景**：蓝牙模块需要周期性检测电量，并在低电量时进入低功耗模式

```c
// 1. 定义事件
#define ACCESS_SLEEP_EVT           1<<0
#define ACCESS_IDEL_SLEEP_EVT      1<<1
#define ACCESS_WAKE_UP_EVT         1<<2

uint8_t access_taskId = INVALID_TASK_ID;
access_state_t access_state;

// 2. 事件处理函数
uint16_t access_ProcessEvent(uint8_t task_id, uint16_t events) {
    // 深度睡眠事件
    if(events & ACCESS_SLEEP_EVT) {
        access_state.sleep_en = TRUE;
        access_state.deep_sleep_flag = TRUE;

        // 如果正在连接，先断开连接
        uint8_t ble_state;
        GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
        if(ble_state == GAPROLE_CONNECTED) {
            hidEmu_disconnect();  // 断开蓝牙连接
            // 延迟等待断开完成
            tmos_start_task(access_taskId, ACCESS_SLEEP_EVT,
                           DISCONNECT_WAIT_REPORT_END_TIMEOUT);
            return (events ^ ACCESS_SLEEP_EVT);
        }

        // 停止广播
        hidEmu_adv_enable(DISABLE);
        access_state.ble_idx = BLE_INDEX_IDEL;

        // 关闭外设
        hidDevBattCB(BATT_LEVEL_NOTI_DISABLED);
        red_led_off();

        // 进入睡眠
        peripheral_enter_sleep();
        LowPower_Sleep(NULL);  // 只保留96K SRAM供电

        return (events ^ ACCESS_SLEEP_EVT);
    }

    // 空闲睡眠事件（保持蓝牙连接）
    if(events & ACCESS_IDEL_SLEEP_EVT) {
        access_state.sleep_en = TRUE;
        access_state.idel_sleep_flag = TRUE;

        // 检查蓝牙状态
        uint8_t ble_state;
        GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
        if(ble_state == GAPROLE_ADVERTISING) {
            // 停止广播
            access_state.ble_idx = BLE_INDEX_IDEL;
            hidEmu_adv_enable(DISABLE);
        }

        // 进入空闲睡眠（保持连接）
        hidDevBattCB(BATT_LEVEL_NOTI_DISABLED);
        peripheral_enter_sleep();

        return (events ^ ACCESS_IDEL_SLEEP_EVT);
    }

    // 唤醒事件
    if(events & ACCESS_WAKE_UP_EVT) {
        if(access_state.ble_idx == BLE_INDEX_IDEL) {
            // 重新开始广播
            hidEmu_adv_enable(ENABLE);
        }

        access_state.sleep_en = FALSE;
        access_state.deep_sleep_flag = FALSE;
        access_state.idel_sleep_flag = FALSE;

        return (events ^ ACCESS_WAKE_UP_EVT);
    }

    return 0;
}

// 3. 公共接口
void access_enter_sleep() {
    tmos_set_event(access_taskId, ACCESS_SLEEP_EVT);
}

void access_enter_idel_sleep() {
    tmos_set_event(access_taskId, ACCESS_IDEL_SLEEP_EVT);
}

void access_wakeup() {
    tmos_set_event(access_taskId, ACCESS_WAKE_UP_EVT);
}
```

**应用场景**：
- 无操作5分钟后自动进入空闲睡眠
- 断开连接10分钟后进入深度睡眠
- 按键或触摸事件唤醒系统

### 实例3：触摸板管理 (touch_manager.c)

**场景**：触摸板需要周期性扫描，并处理触摸中断

```c
// 1. 定义事件
#define TOUCH_SCAN_EVENT            1<<0
#define TOUCH_INTERRUPT_EVENT       1<<1
#define TOUCH_REG_INIT_EVT          1<<2

uint8_t touch_taskID = 0;

// 2. 事件处理函数
uint16_t touch_process_event(uint8_t task_id, uint16_t events) {
    // 周期性扫描事件
    if(events & TOUCH_SCAN_EVENT) {
        // 读取触摸数据
        uint8_t touch_data = touch_read_data();

        // 处理触摸数据
        if(touch_data_changed(touch_data)) {
            // 通知键盘处理触摸事件
            tmos_set_event(keyboard_taskID, KEYBOARD_TOUCH_EVENT);

            // 唤醒系统（如果正在睡眠）
            if(access_state.sleep_en) {
                access_wakeup();
            }
        }

        // 继续下一次扫描
        tmos_start_reload_task(touch_taskID, TOUCH_SCAN_EVENT, TOUCH_SCAN_INTERVAL);
        return events ^ TOUCH_SCAN_EVENT;
    }

    // 中断事件
    if(events & TOUCH_INTERRUPT_EVENT) {
        // 处理触摸中断
        touch_handle_interrupt();
        return events ^ TOUCH_INTERRUPT_EVENT;
    }

    // 寄存器初始化事件
    if(events & TOUCH_REG_INIT_EVT) {
        touch_init_registers();
        return events ^ TOUCH_REG_INIT_EVT;
    }

    return 0;
}

// 3. 触摸中断处理函数
void touch_interrupt_handler() {
    // 在GPIO中断中调用
    tmos_set_event(touch_taskID, TOUCH_INTERRUPT_EVENT);
}

// 4. 初始化
void touch_manager_init() {
    touch_taskID = TMOS_ProcessEventRegister(touch_process_event);

    // 启动周期性扫描
    tmos_start_reload_task(touch_taskID, TOUCH_SCAN_EVENT, TOUCH_SCAN_INTERVAL);
}
```

**应用场景**：
- 每10ms扫描一次触摸板
- 触摸中断立即响应
- 触摸时自动唤醒系统

---

## 优缺点分析

### 优点

#### 1. 轻量级设计
- **代码量小**：整个TMOS核心实现仅几百行代码
- **内存占用低**：无需复杂的任务控制块，最小化RAM使用
- **无外部依赖**：不依赖RTOS，可独立运行
- **适用资源受限系统**：适合CH58x这类资源有限的MCU

#### 2. 易用性强
- **API设计简洁**：仅需3-4个核心API即可完成所有操作
- **学习成本低**：事件驱动模型易于理解和掌握
- **快速上手**：5分钟即可理解基本使用方法
- **示例丰富**：项目中有大量实际应用示例

#### 3. 实时性好
- **高精度定时**：基于32位定时器，时间精度高
- **立即事件响应**：`tmos_set_event()`可立即触发事件处理
- **无延迟**：手动轮询方式，避免中断嵌套开销
- **确定性**：事件处理时间可预测

#### 4. 灵活性高
- **多种触发模式**：支持立即、单次、循环三种事件触发
- **事件组合**：支持一个事件处理器处理多个事件类型
- **任务间通信**：通过`tmos_msg_send()`实现任务间消息传递
- **动态管理**：可随时启动、停止、修改定时事件

#### 5. 模块化友好
- **任务独立**：各模块可独立管理自己的任务
- **松耦合**：任务间通过事件通信，降低耦合度
- **可复用**：一个任务可在多个项目中使用
- **便于测试**：可独立测试单个任务逻辑

### 缺点

#### 1. 功能局限

##### 无优先级管理
```c
// 问题：所有事件同等优先级
// 无法处理：高优先级事件可能被低优先级事件阻塞
// 示例：按键中断事件和LED闪烁事件优先级相同
tmos_set_event(keyboard_taskID, KEY_INTERRUPT_EVENT);    // 高优先级
tmos_set_event(keyboard_taskID, LED_BLINK_EVENT);        // 低优先级
// 如果LED闪烁正在处理，按键事件会延迟响应
```

##### 无任务同步原语
```c
// 问题：无法处理任务间同步问题
// 缺少：信号量、互斥量、事件标志组等
// 示例：两个任务同时访问共享资源
void task1_process() {
    // 没有互斥保护，可能导致数据竞争
    shared_buffer = get_data();
}
void task2_process() {
    // 可能与task1同时修改shared_buffer
    process_data(shared_buffer);
}
```

##### 无内存管理
```c
// 问题：缺乏动态内存分配管理
// 缺少：内存池、内存保护、泄漏检测
// 示例：消息传递可能导致内存泄漏
uint8_t *msg = tmos_msg_alloc(100);
tmos_msg_send(taskID, msg);
// 如果接收方忘记调用tmos_msg_deallocate()，会导致内存泄漏
```

#### 2. 可扩展性限制

##### 事件数量限制
```c
// 限制：每个任务最多16种事件（16位标志）
typedef uint16_t tmosEvents;  // 无法扩展

// 问题：复杂模块事件数量可能超过16种
// 示例：键盘模块已有8个事件，未来扩展可能超过16个
#define EVENT_1  (1<<0)
#define EVENT_2  (1<<1)
// ...
#define EVENT_16 (1<<15)
// EVENT_17 无法定义！
```

##### 任务ID限制
```c
// 限制：最多支持256个任务（8位ID）
typedef uint8_t tmosTaskID;

// 问题：大型项目可能超过256个任务
// 示例：复杂嵌入式系统可能有数百个模块
```

##### 不支持嵌套事件
```c
// 问题：事件处理函数中触发的事件不会立即处理
uint16_t task_process_event(uint8_t task_id, uint16_t events) {
    if(events & EVENT_A) {
        // 处理事件A
        tmos_set_event(task_id, EVENT_B);  // 触发事件B

        // 事件B不会立即处理，要等到下次TMOS_SystemProcess()
        // 这可能导致逻辑复杂化

        return events ^ EVENT_A;
    }
    return 0;
}
```

#### 4. 死锁风险

##### 无死锁检测
```c
// 风险：任务间相互等待可能导致死锁
// 缺少：死锁检测机制、超时机制

// 示例死锁场景：
// 任务1等待任务2触发事件A
// 任务2等待任务1触发事件B
// 两个任务相互等待，无法继续

void task1_process() {
    if(wait_for_event_from_task2) {
        tmos_set_event(task2_id, EVENT_B);  // 可能导致死锁
    }
}
void task2_process() {
    if(wait_for_event_from_task1) {
        tmos_set_event(task1_id, EVENT_A);  // 可能导致死锁
    }
}
```

##### 无超时机制
```c
// 问题：事件可能永远不会被处理
// 缺少：事件超时检测、自动清理机制

// 示例：定时器事件设置时间过长，可能永远不会被触发
tmos_start_task(taskID, EVENT_THAT_NEVER_HAPPENS, 0xFFFFFFFF);
// 这个事件可能永远不会触发，系统永远等待
```

#### 5. 功耗管理不足

##### 手动功耗管理
```c
// 问题：需要开发者手动管理睡眠/唤醒
// 示例：access.c中的复杂逻辑
if(events & ACCESS_SLEEP_EVT) {
    // 手动检查蓝牙状态
    GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
    if(ble_state == GAPROLE_CONNECTED) {
        // 手动处理断开连接
        hidEmu_disconnect();
        // 手动设置延迟事件
        tmos_start_task(access_taskId, ACCESS_SLEEP_EVT,
                       DISCONNECT_WAIT_REPORT_END_TIMEOUT);
    }
    // 手动进入睡眠
    LowPower_Sleep(NULL);
}
```

##### 无空闲度感知
```c
// 问题：TMOS无法感知系统空闲程度
// 缺少：根据系统负载自动调整睡眠策略

// 当前：无论系统多空闲，都要轮询TMOS_SystemProcess()
// 理想：系统空闲时进入深度睡眠，有事件时自动唤醒
```

##### 定时器精度问题
```c
// 问题：睡眠模式下定时器可能不准确
// 示例：低功耗睡眠时，SYSCLK可能关闭，定时器精度下降

// 当前实现：main.c:114 禁用SysTick中断
PFIC_DisableIRQ(SysTick_IRQn);
// 这意味着TMOS依赖外部唤醒，无法精确控制睡眠时间
```

#### 6. 代码可维护性问题

##### 事件处理函数过长
```c
// 问题：复杂模块的事件处理函数可能超过200行
// 示例：keyboard.c:66 keyboard_process_event()处理8种不同事件

uint16_t keyboard_process_event(uint8_t task_id, uint16_t events) {
    if(events & KEYBOARD_LED_BLINK_EVENT) { /* 50行代码 */ }
    if(events & KEYBOARD_VBAT_INFO_EVENT) { /* 50行代码 */ }
    if(events & KEYBOARD_BACKLIGHT_TIMEOUT_EVENT) { /* 40行代码 */ }
    // 总共超过200行，难以维护
}
```

##### 事件定义分散
```c
// 问题：事件定义分散在多个文件中
// 示例：
// keyboard.h: 定义KEYBOARD_LED_BLINK_EVENT
// access.h: 定义ACCESS_SLEEP_EVT
// touch_manager.h: 定义TOUCH_SCAN_EVENT

// 难以全局掌握所有事件类型，容易造成命名冲突
```

##### 逻辑耦合严重
```c
// 问题：事件处理函数中包含多层嵌套逻辑
// 示例：access.c:77-145 处理睡眠事件时包含多种状态判断

if(events & ACCESS_SLEEP_EVT) {
    access_state.sleep_en = TRUE;
    if((access_state.ble_idx > BLE_INDEX_IDEL) && (access_state.ble_idx < BLE_INDEX_MAX)) {
        uint8_t ble_state;
        GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
        if(ble_state == GAPROLE_CONNECTED) {
            // 嵌套3层if，逻辑复杂
        }
    }
}
```

---

## 优化建议

### 1. 模块化重构

#### 1.1 子任务拆分法

将复杂事件拆分为独立子任务处理：

```c
// 修改前：单一事件处理函数（8个事件）
uint16_t keyboard_process_event(uint8_t task_id, uint16_t events) {
    if(events & KEYBOARD_LED_BLINK_EVENT) { /* 50行 */ }
    if(events & KEYBOARD_VBAT_INFO_EVENT) { /* 60行 */ }
    if(events & KEYBOARD_BACKLIGHT_TIMEOUT_EVENT) { /* 40行 */ }
    // 总共超过200行
}

// 修改后：拆分子任务
uint8_t keyboard_led_taskID = INVALID_TASK_ID;
uint8_t keyboard_vbat_taskID = INVALID_TASK_ID;
uint8_t keyboard_backlight_taskID = INVALID_TASK_ID;

// LED闪烁子任务（只处理LED相关逻辑）
uint16_t keyboard_led_process(uint8_t task_id, uint16_t events) {
    if(events & LED_BLINK_EVENT) {
        GPIOA_InverseBits(GPIO_Pin_15);
        return events ^ LED_BLINK_EVENT;
    }
    return 0;
}

// 电量检测子任务（只处理电量相关逻辑）
uint16_t keyboard_vbat_process(uint8_t task_id, uint16_t events) {
    if(events & VBAT_INFO_EVENT) {
        vbat_info = keyboard_get_batt_info();
        if(vbat_led_num == 5 || vbat_led_num == 6) {
            tmos_set_event(keyboard_vbat_taskID, LOW_VOLTAGE_ALERT_EVENT);
        }
        return events ^ VBAT_INFO_EVENT;
    }
    if(events & LOW_VOLTAGE_ALERT_EVENT) {
        // 低电压告警逻辑
        return events ^ LOW_VOLTAGE_ALERT_EVENT;
    }
    return 0;
}

// 主事件处理函数（只做分发）
uint16_t keyboard_process_event(uint8_t task_id, uint16_t events) {
    if(keyboard_led_taskID != INVALID_TASK_ID) {
        events = keyboard_led_process(keyboard_led_taskID, events);
    }
    if(keyboard_vbat_taskID != INVALID_TASK_ID) {
        events = keyboard_vbat_process(keyboard_vbat_taskID, events);
    }
    if(keyboard_backlight_taskID != INVALID_TASK_ID) {
        events = keyboard_backlight_process(keyboard_backlight_taskID, events);
    }
    return events;
}

// 初始化时注册所有子任务
void keyboard_init() {
    keyboard_taskID = TMOS_ProcessEventRegister(keyboard_process_event);
    keyboard_led_taskID = TMOS_ProcessEventRegister(keyboard_led_process);
    keyboard_vbat_taskID = TMOS_ProcessEventRegister(keyboard_vbat_process);
    keyboard_backlight_taskID = TMOS_ProcessEventRegister(keyboard_backlight_process);

    tmos_start_reload_task(keyboard_vbat_taskID, VBAT_INFO_EVENT, GET_VBAT_INFO_INTERVAL);
}
```

**优点**：
- 每个子任务独立，代码可读性高
- 便于单元测试和调试
- 可独立修改某个功能而不影响其他
- 减少单个函数的复杂度

#### 1.2 事件表驱动法

使用函数指针表消除大量if-else：

```c
// 事件处理函数指针类型
typedef uint16_t (*EventHandler)(uint8_t task_id, uint16_t events);

// 事件映射表
typedef struct {
    tmosEvents event_mask;
    EventHandler handler;
} EventMap_t;

static const EventMap_t keyboard_event_map[] = {
    {KEYBOARD_LED_BLINK_EVENT, keyboard_led_handler},
    {KEYBOARD_VBAT_INFO_EVENT, keyboard_vbat_handler},
    {KEYBOARD_BACKLIGHT_UPDATE_EVENT, keyboard_backlight_handler},
    {KEYBOARD_RESET_EVENT, keyboard_reset_handler},
};

// 主事件处理函数（简化）
uint16_t keyboard_process_event(uint8_t task_id, uint16_t events) {
    uint16_t unhandled = events;

    for(uint8_t i = 0; i < sizeof(keyboard_event_map)/sizeof(EventMap_t); i++) {
        if(events & keyboard_event_map[i].event_mask) {
            unhandled = keyboard_event_map[i].handler(task_id, unhandled);
        }
    }

    return unhandled;
}
```

**优点**：
- 新增事件只需修改映射表，无需修改主函数
- 便于事件枚举和文档化
- 支持事件分组和优先级排序

#### 1.3 模块化拆分法（最推荐）

按功能模块彻底拆分：

```
BLE/3mode/KEYBOARD/modules/
├── led/
│   ├── keyboard_led.c
│   ├── keyboard_led.h
│   └── led_manager.h
├── vbat/
│   ├── keyboard_vbat.c
│   ├── keyboard_vbat.h
│   └── vbat_manager.h
├── backlight/
│   ├── keyboard_backlight.c
│   ├── keyboard_backlight.h
│   └── backlight_manager.h
└── keyboard.c (主模块)
```

**示例实现**：

```c
// modules/led/keyboard_led.h
#ifndef KEYBOARD_LED_H
#define KEYBOARD_LED_H

#include "CONFIG.h"

void keyboard_led_init(void);
void keyboard_led_start_blink(uint32_t interval);
void keyboard_led_stop_blink(void);
void keyboard_led_set_state(uint8_t state);

#endif

// modules/led/keyboard_led.c
#include "keyboard_led.h"

typedef struct {
    uint8_t taskID;
    uint32_t blink_interval;
    uint8_t blink_count;
} LED_Control_t;

static LED_Control_t led_ctrl;

void keyboard_led_init() {
    led_ctrl.taskID = TMOS_ProcessEventRegister(keyboard_led_process);
    led_ctrl.blink_interval = 0;
    led_ctrl.blink_count = 0;
}

uint16_t keyboard_led_process(uint8_t task_id, uint16_t events) {
    if(events & LED_BLINK_EVENT) {
        GPIOA_InverseBits(GPIO_Pin_15);
        led_ctrl.blink_count++;

        // 闪烁10次后自动停止
        if(led_ctrl.blink_count >= 10) {
            keyboard_led_stop_blink();
        }

        return events ^ LED_BLINK_EVENT;
    }
    return 0;
}

void keyboard_led_start_blink(uint32_t interval) {
    led_ctrl.blink_interval = interval;
    led_ctrl.blink_count = 0;
    tmos_start_reload_task(led_ctrl.taskID, LED_BLINK_EVENT, interval);
}

void keyboard_led_stop_blink() {
    tmos_stop_task(led_ctrl.taskID, LED_BLINK_EVENT);
    led_ctrl.blink_count = 0;
}

// modules/keyboard.c
#include "keyboard_led.h"
#include "keyboard_vbat.h"
#include "keyboard_backlight.h"

uint8_t keyboard_taskID = 0;

void keyboard_init() {
    keyboard_led_init();
    keyboard_vbat_init();
    keyboard_backlight_init();

    // 主任务只负责模块间协调
    keyboard_taskID = TMOS_ProcessEventRegister(keyboard_coordinate_process);
}
```

**优点**：
- 彻底解耦，各模块独立
- 可独立编译和测试
- 便于代码复用
- 隐藏实现细节

### 3. 优先级机制

#### 3.1 事件优先级定义

```c
// 扩展事件类型，支持优先级
typedef struct {
    tmosEvents event_mask;
    uint8_t priority;      // 0-255，0为最高优先级
} tmos_event_priority_t;

// 优先级队列
typedef struct {
    tmosTaskID task_id;
    tmosEvents events;
    uint8_t priority;
} tmos_priority_queue_t;

#define PRIORITY_QUEUE_SIZE 32
static tmos_priority_queue_t priority_queue[PRIORITY_QUEUE_SIZE];
static uint8_t queue_size = 0;

// 添加事件到优先级队列
bStatus_t tmos_set_event_with_priority(tmosTaskID taskID, tmosEvents event, uint8_t priority) {
    if(queue_size >= PRIORITY_QUEUE_SIZE) return NO_TIMER_AVAIL;

    // 按优先级插入队列
    uint8_t insert_pos = 0;
    while(insert_pos < queue_size &&
          priority_queue[insert_pos].priority <= priority) {
        insert_pos++;
    }

    // 移动元素腾出位置
    for(uint8_t i = queue_size; i > insert_pos; i--) {
        priority_queue[i] = priority_queue[i-1];
    }

    // 插入新元素
    priority_queue[insert_pos].task_id = taskID;
    priority_queue[insert_pos].events = event;
    priority_queue[insert_pos].priority = priority;
    queue_size++;

    return SUCCESS;
}
```

#### 3.2 任务优先级

```c
// 任务优先级配置
typedef struct {
    tmosTaskID task_id;
    uint8_t priority;          // 任务优先级
    uint8_t timeslice;         // 时间片（用于公平调度）
    uint32_t run_count;        // 运行次数统计
} tmos_task_priority_t;

#define MAX_PRIORITY_LEVELS 8
static tmos_task_priority_t priority_levels[MAX_PRIORITY_LEVELS];

// 设置任务优先级
void TMOS_SetTaskPriority(tmosTaskID task_id, uint8_t priority) {
    if(priority >= MAX_PRIORITY_LEVELS) return;

    priority_levels[priority].task_id = task_id;
    priority_levels[priority].priority = priority;
}

// 按优先级调度任务
void TMOS_SystemProcess() {
    // 按优先级从高到低处理
    for(uint8_t p = 0; p < MAX_PRIORITY_LEVELS; p++) {
        tmosTaskID task_id = priority_levels[p].task_id;
        if(task_id != INVALID_TASK_ID) {
            // 处理该优先级的所有任务
            process_task_events(task_id);

            // 更新时间片
            priority_levels[p].timeslice--;
            if(priority_levels[p].timeslice == 0) {
                // 时间片用完，切换到下一个任务
                priority_levels[p].timeslice = get_default_timeslice();
                break;  // 避免低优先级任务饿死
            }
        }
    }
}
```

### 4. 功耗优化

#### 4.1 空闲度感知

```c
// 空闲度检测
typedef struct {
    uint32_t idle_counter;        // 空闲计数器
    uint32_t total_counter;       // 总计数器
    uint8_t idle_percentage;      // 空闲百分比
} tmos_idle_monitor_t;

static tmos_idle_monitor_t idle_monitor;

// 更新空闲度统计
void update_idle_stats() {
    idle_monitor.total_counter++;
    // 如果没有任务需要处理
    if(get_pending_events_count() == 0) {
        idle_monitor.idle_counter++;
    }
    idle_monitor.idle_percentage = (idle_monitor.idle_counter * 100) /
                                   idle_monitor.total_counter;
}

// 根据空闲度自动调整睡眠策略
void adjust_sleep_strategy() {
    if(idle_monitor.idle_percentage > 80) {
        // 系统80%以上时间空闲，进入深度睡眠
        set_sleep_mode(DEEP_SLEEP);
        set_auto_sleep_timeout(SHORT_TIMEOUT);
    } else if(idle_monitor.idle_percentage > 50) {
        // 系统50%以上时间空闲，进入空闲睡眠
        set_sleep_mode(IDLE_SLEEP);
        set_auto_sleep_timeout(MEDIUM_TIMEOUT);
    } else {
        // 系统忙碌，保持唤醒
        set_sleep_mode(NO_SLEEP);
    }
}
```

#### 4.2 智能唤醒管理

```c
// 智能唤醒管理
typedef struct {
    tmosTimer next_wakeup_time;   // 下次唤醒时间
    uint8_t wakeup_enabled;       // 是否启用智能唤醒
    uint8_t min_sleep_time;       // 最小睡眠时间阈值
} tmos_smart_wakeup_t;

static tmos_smart_wakeup_t smart_wakeup;

// 计算下次唤醒时间
void calculate_next_wakeup() {
    tmosTimer earliest_time = 0xFFFFFFFF;

    // 扫描所有任务的定时器，找出最早到期的事件
    for(uint8_t i = 0; i < MAX_TASKS; i++) {
        if(tmos_task[i].active && tmos_task[i].timer_count > 0) {
            for(uint8_t j = 0; j < tmos_task[i].timer_count; j++) {
                if(tmos_task[i].timer_queue[j].timeout < earliest_time) {
                    earliest_time = tmos_task[i].timer_queue[j].timeout;
                }
            }
        }
    }

    smart_wakeup.next_wakeup_time = earliest_time;
}

// 进入智能睡眠
void enter_smart_sleep() {
    calculate_next_wakeup();

    tmosTimer current_time = get_system_time();
    tmosTimer sleep_duration = smart_wakeup.next_wakeup_time - current_time;

    // 如果睡眠时间足够长，进入低功耗模式
    if(sleep_duration >= smart_wakeup.min_sleep_time) {
        // 设置定时唤醒
        set_rtc_wakeup(sleep_duration);

        // 进入低功耗睡眠
        LowPower_Sleep(NULL);

        // 唤醒后重新初始化系统时钟
        sync_system_time();
    }
    // 如果睡眠时间太短，保持活跃状态
}
```

### 5. 错误处理增强

#### 5.1 详细错误码

```c
// 扩展错误码
#define TMOS_SUCCESS                 0x00
#define TMOS_ERROR_INVALID_TASK      0x01
#define TMOS_ERROR_INVALID_EVENT     0x02
#define TMOS_ERROR_NO_TIMER_AVAIL    0x03
#define TMOS_ERROR_QUEUE_FULL        0x04
#define TMOS_ERROR_TIMEOUT           0x05
#define TMOS_ERROR_PRIORITY_INVALID  0x06
#define TMOS_ERROR_MEMORY_FULL       0x07
#define TMOS_ERROR_DEADLOCK          0x08

// 带详细信息的错误结构
typedef struct {
    uint8_t error_code;
    tmosTaskID task_id;
    tmosEvents event;
    tmosTimer timestamp;
    char error_msg[32];
} tmos_error_info_t;

#define ERROR_LOG_SIZE 16
static tmos_error_info_t error_log[ERROR_LOG_SIZE];
static uint8_t error_index = 0;

// 记录错误
void tmos_log_error(uint8_t code, tmosTaskID task_id, tmosEvents event, char *msg) {
    error_log[error_index].error_code = code;
    error_log[error_index].task_id = task_id;
    error_log[error_index].event = event;
    error_log[error_index].timestamp = get_system_time();
    strncpy(error_log[error_index].error_msg, msg, 31);
    error_log[error_index].error_msg[31] = '\0';

    error_index = (error_index + 1) % ERROR_LOG_SIZE;
}

// 启动定时器时记录详细信息
bStatus_t tmos_start_task_debug(tmosTaskID taskID, tmosEvents event, tmosTimer time,
                                char *caller_func) {
    if(taskID >= MAX_TASKS) {
        tmos_log_error(TMOS_ERROR_INVALID_TASK, taskID, event,
                      "Invalid task ID in tmos_start_task()");
        return INVALID_TASK;
    }

    if(event == 0) {
        tmos_log_error(TMOS_ERROR_INVALID_EVENT, taskID, event,
                      "Event mask is zero");
        return INVALID_EVENT;
    }

    uint8_t slot = find_free_timer_slot(taskID);
    if(slot == 0xFF) {
        tmos_log_error(TMOS_ERROR_NO_TIMER_AVAIL, taskID, event,
                      "No timer slots available");
        return NO_TIMER_AVAIL;
    }

    // 设置定时器
    tmos_task[taskID].timer_queue[slot].event = event;
    tmos_task[taskID].timer_queue[slot].timeout = get_current_time() + time;
    tmos_task[taskID].timer_queue[slot].caller = caller_func;

    return SUCCESS;
}
```

#### 5.2 死锁检测

```c
// 死锁检测机制
#define WAIT_GRAPH_SIZE MAX_TASKS
static uint8_t wait_graph[WAIT_GRAPH_SIZE][WAIT_GRAPH_SIZE];  // 等待图
static tmosTaskID waiting_task[MAX_TASKS];                    // 任务等待关系
static tmosEvents waiting_event[MAX_TASKS];                   // 等待的事件

// 记录任务等待关系
void tmos_record_wait(tmosTaskID waiter, tmosTaskID target, tmosEvents event) {
    if(waiter < MAX_TASKS && target < MAX_TASKS) {
        wait_graph[waiter][target] = 1;  // waiter等待target
        waiting_task[waiter] = target;
        waiting_event[waiter] = event;

        // 检查是否形成环
        if(detect_deadlock(waiter)) {
            // 发现死锁，记录错误并尝试恢复
            tmos_log_error(TMOS_ERROR_DEADLOCK, waiter, event,
                          "Deadlock detected");
            resolve_deadlock();
        }
    }
}

// 检测环（深度优先搜索）
uint8_t detect_deadlock(tmosTaskID start_task) {
    uint8_t visited[MAX_TASKS] = {0};
    return dfs_check_cycle(start_task, start_task, visited);
}

uint8_t dfs_check_cycle(tmosTaskID current, tmosTaskID start, uint8_t *visited) {
    if(current == start && visited[start]) {
        return 1;  // 发现环
    }

    visited[current] = 1;

    for(uint8_t i = 0; i < MAX_TASKS; i++) {
        if(wait_graph[current][i]) {
            if(dfs_check_cycle(i, start, visited)) {
                return 1;
            }
        }
    }

    return 0;
}

// 死锁恢复（强制唤醒等待时间最长的任务）
void resolve_deadlock() {
    tmosTaskID victim = find_longest_waiting_task();

    // 强制触发等待的事件
    tmos_set_event(victim, waiting_event[victim]);

    // 清除等待关系
    clear_wait_relationships(victim);
}
```

### 8. 总结

TMOS作为一个轻量级任务调度系统，在沁恒芯片584三模键盘项目中发挥了重要作用。其简单易用、资源占用低的特点非常适合资源受限的嵌入式系统。

**核心优势**：
- 轻量级设计，适合资源受限系统
- API简洁，易于学习和使用
- 事件驱动模型，符合嵌入式编程习惯
- 实时性好，响应速度快

**主要不足**：
- 缺乏优先级管理和任务同步原语
- 事件数量和任务数量有限制
- 调试和监控能力不足
- 功耗管理需要手动干预

**重构建议**：
1. **模块化拆分**：将复杂任务拆分为多个子任务
2. **引入优先级机制**：为关键事件设置高优先级
3. **增强监控能力**：添加任务状态查询和性能分析
4. **优化功耗管理**：实现空闲度感知和智能唤醒
5. **完善错误处理**：添加详细错误码和死锁检测
6. **提升时间精度**：支持高精度定时和睡眠补偿

通过这些优化措施，可以在保持TMOS轻量级特性的同时，显著提升系统的可维护性、可靠性和开发效率。

---

**文档版本**：v1.0
**创建日期**：2025-12-16
**作者**：Claude Code
**适用范围**：沁恒微电子CH58xBLE系列芯片SDK
