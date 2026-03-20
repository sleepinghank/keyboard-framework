# Keyboard Framework - 完善实现计划

**创建日期**: 2026-01-18
**基于分析**: application-layer-analysis.md + 全框架现状分析
**目标平台**: CH584M (WCH 低功耗蓝牙芯片)

---

## 一、现状总览

### 各层完成度

| 层级 | 完成度 | 关键缺失 |
|------|--------|----------|
| HAL | 73% | SPI Master、I2C Slave、UART中断 |
| Drivers | 67% | 2.4G驱动、触摸板、USB HID |
| Middleware | 82% | Combo逻辑、LPM平台实现、自定义函数 |
| Application | 55% | System Service、Output Service、Service集成 |

### 核心依赖链

```
Application Service
       ↓
   Middleware (keyboard, wireless, lpm)
       ↓
   Drivers (matrix, battery, indicator, bluetooth)
       ↓
   HAL (GPIO, ADC, PWM, UART, SPI, I2C)
       ↓
   CH584 SDK
```

---

## 二、模块化实现计划

按优先级分为三个阶段：
- **P0 (关键路径)**: 系统无法正常运行的阻塞项
- **P1 (核心功能)**: 产品核心功能
- **P2 (增强功能)**: 提升用户体验的功能

---

## 阶段一：P0 关键路径 (系统基础)

### TODO-1: HAL层SPI Master实现

**位置**: `hal/platforms/ch584/_spi_master.c`

**当前状态**: 仅18行文件头，0%实现

**实现内容**:
```c
// 需要实现的6个接口函数
void spi_master_init(spi_master_t *spi);
void spi_master_start(spi_master_t *spi);
void spi_master_stop(spi_master_t *spi);
uint8_t spi_master_write(spi_master_t *spi, uint8_t data);
uint8_t spi_master_read(spi_master_t *spi);
void spi_master_transmit(spi_master_t *spi, uint8_t *tx, uint8_t *rx, uint16_t len);
```

**实现方式**:
1. 参考CH584 SDK的`SPI0_MasterDefInit()`初始化函数
2. 使用`SPI0_MasterSendByte()`和`SPI0_MasterRecvByte()`实现收发
3. 支持可配置的时钟频率、极性、相位
4. 实现CS引脚的软件控制

**依赖**: CH584 SDK SPI库

**阻塞**: 2.4G无线驱动、部分触摸芯片通信

---

### TODO-2: Service初始化集成到system_init

**位置**: `application/system/system_init.c`

**当前状态**: `system_init_application()`为空函数

**实现内容**:
```c
void system_init_application(void) {
    // 1. 按依赖顺序初始化服务
    system_service_init();          // 系统服务最先（电源、存储）
    input_service_init();           // 输入服务
    output_service_init();          // 输出服务
    communication_service_init();   // 通信服务最后

    // 2. 标记系统初始化完成
    g_system_init_status = SYSTEM_INIT_COMPLETE;

    // 3. 启动看门狗（如果启用）
    #if WATCHDOG_ENABLE
    watchdog_start();
    #endif
}
```

**实现方式**:
1. 在`system_init.c`中添加Service头文件包含
2. 按依赖顺序调用各Service的init函数
3. 添加初始化状态标记
4. 添加初始化失败处理

**依赖**: 各Service的init函数已定义

---

### TODO-3: 看门狗机制实现

**位置**:
- `hal/interface/watchdog.h` (新建)
- `hal/platforms/ch584/_watchdog.c` (新建)
- `application/system/system_init.c` (集成)

**当前状态**: 完全缺失

**实现内容**:
```c
// watchdog.h - 接口定义
void watchdog_init(uint32_t timeout_ms);
void watchdog_start(void);
void watchdog_stop(void);
void watchdog_feed(void);
bool watchdog_caused_reset(void);

// 主循环集成
while (1) {
    watchdog_feed();
    OSAL_SystemProcess();
}
```

**实现方式**:
1. CH584使用IWDG (Independent Watchdog)
2. 设置合理的超时时间（推荐5秒）
3. 在OSAL主循环中喂狗
4. 提供低功耗模式下的特殊处理

**依赖**: CH584 SDK看门狗库

---

### TODO-4: UART中断接收启用

**位置**: `hal/platforms/ch584/_uart.c`

**当前状态**: 中断处理函数被注释（550-672行）

**实现内容**:
```c
// 取消注释并完善中断处理
void UART0_IRQHandler(void) {
    if (UART0_GetITFlag(UART_II_RECV_RDY)) {
        uint8_t data = UART0_RecvByte();
        if (uart_rx_callback[0]) {
            uart_rx_callback[0](data);
        }
    }
    // 处理其他中断标志...
}

// 启用中断的初始化
void platform_uart_init(platform_uart_t uart, uint32_t baudrate) {
    // ... 现有初始化代码 ...
    UART_INTCfg(uart, ENABLE, RB_IER_RECV_RDY);  // 启用接收中断
    NVIC_EnableIRQ(uart_irq[uart]);              // 启用NVIC中断
}
```

**实现方式**:
1. 取消中断处理函数的注释
2. 实现环形缓冲区接收
3. 添加FIFO阈值配置
4. 测试115200波特率稳定性

**依赖**: CH584 SDK UART库

---

## 阶段二：P1 核心功能

### TODO-5: System Service电源管理实现

**位置**: `application/service/system_service.c`

**当前状态**: 所有事件处理为TODO（30%完成）

**实现内容**:
```c
// 需要实现的事件处理
if (events & SYSTEM_LOW_BATTERY_SHUTDOWN_EVT) {
    // 1. 保存当前状态到存储
    storage_save_config(storage_get_config_ptr());
    // 2. 发送断开连接命令
    wireless_enter_disconnected(0, DISCONNECT_REASON_LOW_BATTERY);
    // 3. 关闭所有外设
    indicator_all_off();
    backlight_disable();
    // 4. 进入关机模式
    enter_power_mode(PM_SHUTDOWN);
}

if (events & SYSTEM_DEEP_SLEEP_EVT) {
    // 1. 配置唤醒源（矩阵按键）
    lpm_configure_wakeup_source(WAKEUP_SOURCE_MATRIX);
    // 2. 保存状态
    lpm_save_state_before_sleep();
    // 3. 进入深度睡眠
    enter_power_mode(PM_DEEP_SLEEP);
}

if (events & SYSTEM_WAKEUP_EVT) {
    // 1. 恢复外设
    system_init_drivers();
    // 2. 恢复连接
    wireless_enter_reconnecting(wireless_get_last_host());
}
```

**实现方式**:
1. 实现各电源状态的进入/退出逻辑
2. 配置CH584的低功耗模式（Sleep/DeepSleep/Shutdown）
3. 实现唤醒源配置（GPIO中断、RTC定时器）
4. 与battery驱动集成，响应电量变化

**依赖**: LPM中间件、Battery驱动、Storage驱动

---

### TODO-6: LPM低功耗管理平台实现

**位置**:
- `middleware/communication/lpm.c` (完善)
- `hal/platforms/ch584/_power.c` (新建)

**当前状态**: 框架60%，`enter_power_mode()`为弱函数空实现

**实现内容**:
```c
// _power.c - CH584平台实现
void enter_power_mode(power_mode_t mode) {
    switch (mode) {
        case PM_IDLE:
            // 降低CPU频率，外设保持
            PWR_PeriphWakeUpCfg(ENABLE, RB_SLP_CLK_RAMRET);
            LowPower_Idle();
            break;

        case PM_SLEEP:
            // 关闭高频时钟，保持SRAM
            PWR_PeriphWakeUpCfg(ENABLE, RB_SLP_CLK_RAMRET | RB_SLP_GPIO_WAKE);
            LowPower_Sleep(RB_PWR_RAM30K | RB_PWR_RAM2K);
            break;

        case PM_DEEP_SLEEP:
            // 仅保留RTC，最低功耗
            LowPower_Shutdown(0);  // 0=不保留SRAM
            break;

        case PM_SHUTDOWN:
            // 完全关机
            SYS_ResetExecute();
            break;
    }
}

// lpm.c - 完善lpm_task()
void lpm_task(void) {
    // 当前被注释的代码需要启用
    if (lpm_timer_elapsed() && !lpm_any_matrix_action()) {
        if (!usb_power_connected()) {
            enter_power_mode(current_lpm_level);
        }
    }
}
```

**实现方式**:
1. 实现CH584的各低功耗模式切换
2. 配置GPIO唤醒（矩阵按键）
3. 配置RTC唤醒（定时重连）
4. 实现唤醒后的状态恢复

**依赖**: CH584 SDK电源管理库

---

### TODO-7: Output Service HID报告发送

**位置**: `application/service/output_service.c`

**当前状态**: 40%完成，发送逻辑为TODO

**实现内容**:
```c
if (events & OUTPUT_SEND_HID_KEYBOARD_EVT) {
    // 从报告缓冲区获取数据
    report_keyboard_t *report = report_buffer_dequeue(REPORT_TYPE_KEYBOARD);
    if (report) {
        // 根据当前传输模式发送
        switch (get_transport()) {
            case TRANSPORT_USB:
                usb_send_keyboard(report);
                break;
            case TRANSPORT_BLUETOOTH:
                bt_driver_send_keyboard((uint8_t*)report);
                break;
            case TRANSPORT_P24G:
                p24g_driver_send_keyboard((uint8_t*)report);
                break;
        }
    }
    events ^= OUTPUT_SEND_HID_KEYBOARD_EVT;
}

if (events & OUTPUT_SEND_MOUSE_REPORT_EVT) {
    // 类似处理鼠标报告
    report_mouse_t *report = report_buffer_dequeue(REPORT_TYPE_MOUSE);
    if (report) {
        host_mouse_send(report);
    }
    events ^= OUTPUT_SEND_MOUSE_REPORT_EVT;
}
```

**实现方式**:
1. 与report_buffer模块集成
2. 根据transport模式选择发送通道
3. 处理发送失败重试
4. 添加发送完成回调

**依赖**: report_buffer、transport、各驱动的send函数

---

### TODO-8: Input Service触摸和电量事件

**位置**: `application/service/input_service.c`

**当前状态**: 70%完成，触摸和电量事件为TODO

**实现内容**:
```c
if (events & INPUT_TOUCH_INT_EVT) {
    // 读取触摸数据
    touch_data_t touch;
    if (touchpad_read(&touch) == 0) {
        // 处理手势
        gesture_result_t gesture = gesture_process(&touch);
        if (gesture.type != GESTURE_NONE) {
            // 转换为HID报告
            process_gesture_to_hid(gesture);
        }
    }
    events ^= INPUT_TOUCH_INT_EVT;
}

if (events & INPUT_BATTERY_DETE_EVT) {
    // 获取电池状态
    uint8_t level = battery_get_percentage();
    uint8_t state = battery_get_power_state();

    // 更新无线模块的电量
    wireless_update_battery(level);

    // 检查是否需要低电处理
    if (state == BAT_POWER_SHUTDOWN) {
        OSAL_SetEvent(system_taskID, SYSTEM_LOW_BATTERY_SHUTDOWN_EVT);
    } else if (state == BAT_POWER_CRITICAL_LOW) {
        indicator_set(LED_BATTERY, &IND_BLINK_FAST);
    }
    events ^= INPUT_BATTERY_DETE_EVT;
}
```

**实现方式**:
1. 实现触摸中断响应和数据读取
2. 集成手势识别模块
3. 实现电量变化检测和通知
4. 添加低电警告指示

**依赖**: touchpad驱动、gesture模块、battery驱动

---

### TODO-9: Combo组合键逻辑完善

**位置**: `middleware/keyboard/combo.c`

**当前状态**: 70%完成，有TODO标记和逻辑不完整

**实现内容**:
```c
// 修复process_combo()函数签名和实现
bool process_combo(keyrecord_t *record) {
    // 1. 检查是否启用combo
    if (!combo_enabled) return true;

    // 2. 检查按键是否属于任何combo
    uint16_t keycode = record->event.key;
    for (uint8_t i = 0; i < combo_count; i++) {
        combo_t *combo = &combos[i];

        // 3. 检查按键匹配
        if (is_key_in_combo(combo, keycode)) {
            if (record->event.pressed) {
                // 4. 添加到活动combo
                add_to_active_combo(combo, keycode);

                // 5. 检查combo是否完成
                if (is_combo_complete(combo)) {
                    // 6. 触发combo动作
                    process_combo_event(combo, true);
                    return false;  // 消费此按键
                }
            } else {
                // 7. 处理释放
                remove_from_active_combo(combo, keycode);
            }
        }
    }
    return true;  // 继续正常处理
}

// 修复与action.c的集成
// action.c line 68 的调用需要匹配
```

**实现方式**:
1. 统一函数签名（与action.c调用匹配）
2. 实现完整的combo状态机
3. 支持combo超时（COMBO_TERM）
4. 支持combo顺序要求

**依赖**: action.c、keyboard.c

---

### TODO-10: 2.4G无线驱动核心实现

**位置**: `drivers/communication/p2p4g/p24g_driver.c`

**当前状态**: 0%实现，仅有接口定义

**实现内容**:
```c
// 1. SPI通信基础
static void p24g_spi_write_reg(uint8_t reg, uint8_t value);
static uint8_t p24g_spi_read_reg(uint8_t reg);
static void p24g_spi_write_buf(uint8_t reg, uint8_t *buf, uint8_t len);

// 2. 初始化
void p24g_driver_init_impl(bool wakeup) {
    // 初始化SPI
    spi_master_init(&p24g_spi);
    // 配置RF参数
    p24g_spi_write_reg(REG_CONFIG, CONFIG_PWR_UP | CONFIG_EN_CRC);
    p24g_spi_write_reg(REG_RF_CH, DEFAULT_CHANNEL);
    p24g_spi_write_reg(REG_RF_SETUP, RF_DR_2MBPS | RF_PWR_0DBM);
    // 设置地址
    p24g_set_address(DEFAULT_ADDRESS);
}

// 3. 数据发送
uint8_t p24g_send_keyboard_impl(uint8_t *report) {
    // 构建数据包
    uint8_t packet[32];
    packet[0] = PACKET_TYPE_KEYBOARD;
    memcpy(&packet[1], report, 8);
    // 发送
    return p24g_transmit(packet, 9);
}

// 4. 配对流程
void p24g_pairing_ex_impl(uint8_t host_idx, void *param) {
    // 进入配对模式
    p24g_set_mode(MODE_RX);
    // 等待接收器响应
    // 交换地址和密钥
}
```

**实现方式**:
1. 先实现SPI通信层
2. 参考nRF24L01/类似芯片的协议
3. 实现基本的发送/接收
4. 实现配对和重连逻辑

**依赖**: SPI Master HAL

---

### TODO-11: 蓝牙驱动电池上报启用

**位置**: `drivers/communication/bluetooth/ch584/_bt_driver.c`

**当前状态**: `bt_driver_update_bat_level()`被注释

**实现内容**:
```c
void bt_driver_update_bat_level(uint8_t bat_lvl) {
    // 1. 更新BAS (Battery Service) 特征值
    uint8_t battery_level = bat_lvl;
    Batt_SetParameter(BATT_PARAM_LEVEL, 1, &battery_level);

    // 2. 如果已连接，发送通知
    if (hidEmuState == HIDEMU_STATE_CONNECTED) {
        Batt_MeasLevel();  // 触发通知
    }
}
```

**实现方式**:
1. 取消注释现有代码
2. 确保BAS服务已初始化
3. 在battery_task中定期调用

**依赖**: CH584 BLE协议栈

---

## 阶段三：P2 增强功能

### TODO-12: 自定义函数完整实现

**位置**: `middleware/keyboard/custom_function.c`

**当前状态**: 50%完成，Tap Dance/Leader Key/Macro为框架

**实现内容**:

#### 12.1 Tap Dance
```c
void tap_dance_task(void) {
    for (uint8_t i = 0; i < tap_dance_count; i++) {
        tap_dance_state_t *state = &tap_dance_states[i];
        if (state->count > 0 && timer_elapsed(state->timer) > TAPPING_TERM) {
            // 超时，执行动作
            tap_dance_action_t *action = &tap_dance_actions[i];
            if (action->fn.on_dance_finished) {
                action->fn.on_dance_finished(state);
            }
            state->count = 0;
        }
    }
}
```

#### 12.2 Leader Key
```c
void leader_task(void) {
    if (leader_active && timer_elapsed(leader_timer) > LEADER_TIMEOUT) {
        // 匹配序列
        for (uint8_t i = 0; i < leader_sequence_count; i++) {
            if (match_leader_sequence(&leader_sequences[i])) {
                execute_leader_action(&leader_sequences[i]);
                break;
            }
        }
        leader_reset();
    }
}
```

#### 12.3 Macro
```c
void macro_task(void) {
    if (macro_playing && macro_current_step < macro_length) {
        macro_step_t *step = &macro_steps[macro_current_step];
        if (timer_elapsed(macro_timer) >= step->delay) {
            execute_macro_step(step);
            macro_current_step++;
            macro_timer = timer_read();
        }
    }
}
```

**实现方式**:
1. 参考QMK的tap_dance.c实现
2. 实现状态机和超时检测
3. 提供用户自定义回调接口

**依赖**: action.c、keyboard.c

---

### TODO-13: 触摸板驱动实现

**位置**:
- `drivers/input/touchpad/touchpad.h` (新建)
- `drivers/input/touchpad/pct1336_driver.c` (基于component)

**当前状态**: 空文件（1行）

**实现内容**:
```c
// touchpad.h - 通用接口
typedef struct {
    int16_t x, y;           // 坐标
    uint8_t finger_count;   // 手指数量
    bool    touched;        // 是否触摸
} touch_data_t;

void touchpad_init(void);
int touchpad_read(touch_data_t *data);
void touchpad_set_sensitivity(uint8_t level);
void touchpad_enable_gesture(bool enable);

// pct1336_driver.c - PCT1336芯片实现
void pct1336_init(void) {
    // I2C初始化
    i2c_master_init(&touchpad_i2c);
    // 芯片配置
    pct1336_write_reg(REG_CONFIG, DEFAULT_CONFIG);
    pct1336_write_reg(REG_SENSITIVITY, DEFAULT_SENSITIVITY);
}

int pct1336_read(touch_data_t *data) {
    uint8_t buf[6];
    i2c_master_read(PCT1336_ADDR, buf, 6);
    // 解析数据
    data->x = (buf[1] << 8) | buf[0];
    data->y = (buf[3] << 8) | buf[2];
    data->finger_count = buf[4];
    data->touched = (buf[5] & 0x01);
    return 0;
}
```

**实现方式**:
1. 参考component/pct1336_driver.c已有代码
2. 实现I2C通信
3. 集成手势识别（component/mcs_gesture.c）
4. 配置中断触发

**依赖**: I2C Master HAL、component模块

---

### TODO-14: USB HID驱动实现

**位置**:
- `drivers/communication/usb/usb_hid.h` (新建)
- `drivers/communication/usb/ch584/usb_hid.c` (新建)

**当前状态**: 完全缺失

**实现内容**:
```c
// usb_hid.h - 接口定义
void usb_hid_init(void);
void usb_hid_task(void);
uint8_t usb_send_keyboard(report_keyboard_t *report);
uint8_t usb_send_mouse(report_mouse_t *report);
uint8_t usb_send_consumer(uint16_t data);
bool usb_is_configured(void);

// ch584/usb_hid.c - CH584实现
void usb_hid_init(void) {
    // 初始化USB设备
    USB_DeviceInit();
    // 配置HID描述符
    USB_SetHIDReportDescriptor(hid_report_descriptor, sizeof(hid_report_descriptor));
    // 启用USB中断
    NVIC_EnableIRQ(USB_IRQn);
}

uint8_t usb_send_keyboard(report_keyboard_t *report) {
    if (!usb_is_configured()) return 1;
    return USB_SendData(EP_KEYBOARD, (uint8_t*)report, sizeof(report_keyboard_t));
}
```

**实现方式**:
1. 参考CH584 SDK的USB例程
2. 实现HID描述符配置
3. 实现各端点的数据发送
4. 处理USB挂起/恢复

**依赖**: CH584 SDK USB库

---

### TODO-15: 背光RGB效果实现

**位置**:
- `drivers/output/backlight/rgb_matrix.h` (新建)
- `drivers/output/backlight/rgb_matrix.c` (新建)

**当前状态**: backlight仅支持单色PWM

**实现内容**:
```c
// rgb_matrix.h
typedef struct {
    uint8_t r, g, b;
} rgb_t;

void rgb_matrix_init(void);
void rgb_matrix_set_color(uint8_t index, rgb_t color);
void rgb_matrix_set_all(rgb_t color);
void rgb_matrix_task(void);
void rgb_matrix_set_effect(uint8_t effect);

// 预定义效果
enum rgb_effect_t {
    RGB_EFFECT_SOLID,       // 纯色
    RGB_EFFECT_BREATHING,   // 呼吸
    RGB_EFFECT_RAINBOW,     // 彩虹
    RGB_EFFECT_WAVE,        // 波浪
    RGB_EFFECT_REACTIVE,    // 按键响应
};
```

**实现方式**:
1. 支持WS2812/SK6812等LED驱动
2. 实现基于定时器的效果刷新
3. 提供效果参数配置
4. 支持按键响应效果

**依赖**: PWM HAL或专用LED驱动

---

### TODO-16: 服务间消息传递机制

**位置**:
- `application/service/service_message.h` (新建)
- `application/service/service_message.c` (新建)

**当前状态**: 服务间直接函数调用，无标准化通信

**实现内容**:
```c
// service_message.h
typedef enum {
    MSG_KEY_PRESSED,
    MSG_KEY_RELEASED,
    MSG_BATTERY_CHANGED,
    MSG_WIRELESS_STATE_CHANGED,
    MSG_USB_STATE_CHANGED,
} service_msg_type_t;

typedef struct {
    service_msg_type_t type;
    uint8_t            source_service;
    union {
        struct { uint8_t row, col; } key;
        struct { uint8_t level; } battery;
        struct { uint8_t state, host; } wireless;
    } data;
} service_msg_t;

// 发布-订阅接口
void service_publish(service_msg_t *msg);
void service_subscribe(service_msg_type_t type, service_msg_handler_t handler);

// service_message.c
#define MAX_SUBSCRIBERS 8

static struct {
    service_msg_type_t type;
    service_msg_handler_t handlers[MAX_SUBSCRIBERS];
    uint8_t count;
} subscriptions[MSG_TYPE_COUNT];

void service_publish(service_msg_t *msg) {
    for (uint8_t i = 0; i < subscriptions[msg->type].count; i++) {
        subscriptions[msg->type].handlers[i](msg);
    }
}
```

**实现方式**:
1. 实现轻量级发布-订阅模式
2. 使用静态分配避免动态内存
3. 支持同步和异步消息

**依赖**: 无

---

### TODO-17: 状态机调试日志

**位置**: `middleware/communication/wireless.c` (增强)

**当前状态**: 状态转换无日志

**实现内容**:
```c
// 状态名称映射
static const char* state_names[] = {
    "RESET", "INITIALIZED", "DISCONNECTED",
    "CONNECTED", "PARING", "RECONNECTING", "SUSPEND"
};

// 增强的状态设置函数
void wireless_set_state(wt_state_t new_state) {
    // 1. 验证状态转换
    if (!is_valid_transition(wt_state, new_state)) {
        LOG_ERROR("[WL] Invalid transition: %s -> %s",
                  state_names[wt_state], state_names[new_state]);
        return;
    }

    // 2. 记录转换
    LOG_INFO("[WL] State: %s -> %s",
             state_names[wt_state], state_names[new_state]);

    // 3. 执行转换
    wt_state_t old_state = wt_state;
    wt_state = new_state;

    // 4. 触发回调
    wireless_state_changed_cb(old_state, new_state);
}

// 状态转换有效性表
static const bool valid_transitions[7][7] = {
    // RESET  INIT  DISC  CONN  PAIR  RECON SUSP
    {  0,     1,    0,    0,    0,    0,    0 },  // from RESET
    {  1,     0,    1,    0,    1,    1,    0 },  // from INIT
    {  1,     0,    0,    1,    1,    1,    1 },  // from DISC
    {  1,     0,    1,    0,    0,    0,    1 },  // from CONN
    // ...
};
```

**实现方式**:
1. 添加状态名称字符串表
2. 实现状态转换验证
3. 添加调试日志输出
4. 支持条件编译（DEBUG模式）

**依赖**: 日志系统

---

### TODO-18: 内存使用监控

**位置**:
- `utils/memory_monitor.h` (新建)
- `utils/memory_monitor.c` (新建)

**当前状态**: 完全缺失

**实现内容**:
```c
// memory_monitor.h
void memory_monitor_init(void);
uint32_t get_stack_free_size(void);
uint32_t get_heap_free_size(void);
void memory_health_check(void);

// memory_monitor.c
// 栈使用检测（使用栈填充模式）
#define STACK_FILL_PATTERN 0xDEADBEEF

void memory_monitor_init(void) {
    // 填充栈区域
    extern uint32_t _estack, _Min_Stack_Size;
    uint32_t *stack_start = &_estack - _Min_Stack_Size;
    for (uint32_t *p = stack_start; p < &_estack; p++) {
        *p = STACK_FILL_PATTERN;
    }
}

uint32_t get_stack_free_size(void) {
    extern uint32_t _estack, _Min_Stack_Size;
    uint32_t *stack_start = &_estack - _Min_Stack_Size;
    uint32_t free = 0;
    for (uint32_t *p = stack_start; p < &_estack; p++) {
        if (*p == STACK_FILL_PATTERN) free += 4;
        else break;
    }
    return free;
}
```

**实现方式**:
1. 使用栈填充模式检测栈使用
2. 定期检查并警告低内存
3. 调试模式下输出详细统计

**依赖**: 链接脚本符号

---

### TODO-19: 生命周期函数完整集成

**位置**: `application/main.c` 或 `test_main.c`

**当前状态**: `_pre_task()`和`_post_task()`未调用

**实现内容**:
```c
// 在各模块中添加生命周期函数
// keyboard.h
void keyboard_pre_task(void);
void keyboard_post_task(void);

// wireless.h
void wireless_pre_task(void);
void wireless_post_task(void);

// 主循环集成
int main(void) {
    system_init_coordinator();

    while (1) {
        // Pre-task阶段
        keyboard_pre_task();
        wireless_pre_task();
        indicator_pre_task();

        // 主事件处理
        OSAL_SystemProcess();

        // Post-task阶段
        keyboard_post_task();
        wireless_post_task();
        indicator_post_task();

        // 看门狗喂狗
        watchdog_feed();
    }
}
```

**实现方式**:
1. 为每个需要的模块添加pre/post_task函数
2. 在主循环中按顺序调用
3. pre_task处理准备工作，post_task处理清理工作

**依赖**: 各模块

---

### TODO-20: 存储模块异步写入

**位置**: `drivers/storage/storage.c`

**当前状态**: 同步写入，可能阻塞主循环

**实现内容**:
```c
// 异步写入队列
typedef struct {
    uint16_t offset;
    uint8_t data[32];
    uint8_t len;
} storage_write_task_t;

static storage_write_task_t write_queue[8];
static uint8_t write_queue_head, write_queue_tail;

// 异步写入请求
bool storage_save_async(const storage_config_t *config) {
    // 将写入任务加入队列
    // 分块写入，每块32字节
    uint16_t offset = 0;
    const uint8_t *data = (const uint8_t*)config;

    while (offset < sizeof(storage_config_t)) {
        storage_write_task_t *task = &write_queue[write_queue_tail];
        task->offset = offset;
        task->len = MIN(32, sizeof(storage_config_t) - offset);
        memcpy(task->data, data + offset, task->len);

        write_queue_tail = (write_queue_tail + 1) % 8;
        offset += 32;
    }
    return true;
}

// 存储任务（在主循环中调用）
void storage_task(void) {
    if (write_queue_head != write_queue_tail) {
        storage_write_task_t *task = &write_queue[write_queue_head];
        eeprom_write_block(STORAGE_BASE_ADDR + task->offset, task->data, task->len);
        write_queue_head = (write_queue_head + 1) % 8;
    }
}
```

**实现方式**:
1. 实现写入任务队列
2. 分块写入避免长时间阻塞
3. 在OSAL任务中处理写入
4. 添加写入完成回调

**依赖**: EEPROM驱动

---

## 三、实现顺序建议

### 第一周：基础设施
1. TODO-1: SPI Master HAL
2. TODO-2: Service初始化集成
3. TODO-3: 看门狗机制
4. TODO-4: UART中断启用

### 第二周：电源管理
5. TODO-5: System Service电源管理
6. TODO-6: LPM平台实现
7. TODO-11: 蓝牙电池上报

### 第三周：核心功能
8. TODO-7: Output Service HID发送
9. TODO-8: Input Service触摸/电量
10. TODO-9: Combo逻辑完善

### 第四周：无线通信
11. TODO-10: 2.4G驱动核心
12. TODO-14: USB HID驱动

### 第五周：增强功能
13. TODO-12: 自定义函数
14. TODO-13: 触摸板驱动
15. TODO-15: RGB背光

### 第六周：系统优化
16. TODO-16: 服务间消息
17. TODO-17: 状态机日志
18. TODO-18: 内存监控
19. TODO-19: 生命周期集成
20. TODO-20: 存储异步写入

---

## 四、验收标准

### 每个TODO完成标准

1. **代码实现**: 功能代码完整，无TODO注释
2. **编译通过**: 无warning，无error
3. **单元测试**: 关键函数有测试用例
4. **集成测试**: 与相关模块联调通过
5. **文档更新**: CLAUDE.md或相关文档更新

### 阶段验收

| 阶段 | 验收条件 |
|------|----------|
| P0完成 | 系统能正常启动，矩阵扫描正常，看门狗工作 |
| P1完成 | 蓝牙连接正常，按键能发送，低功耗正常 |
| P2完成 | 所有功能可用，性能达标，稳定运行24小时 |

---

## 五、风险与依赖

### 外部依赖

| 依赖项 | 状态 | 风险 |
|--------|------|------|
| CH584 SDK | 已有 | 低 |
| 2.4G芯片规格 | 待确认 | 高 |
| 触摸芯片型号 | PCT1336 | 中 |
| USB库 | SDK提供 | 低 |

### 技术风险

1. **2.4G协议**: 需确认使用的芯片型号和协议
2. **低功耗**: 需实际测量功耗，可能需要多次优化
3. **USB兼容性**: 需在多种主机上测试

---

**文档版本**: v1.0
**最后更新**: 2026-01-18
**维护者**: Claude Code
