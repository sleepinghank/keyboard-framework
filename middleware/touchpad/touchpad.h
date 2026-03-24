#pragma once

#include <stdbool.h>
#include <stdint.h>

extern uint8_t touchpad_taskID;

/* middleware/touchpad 内部事件 */
#define TOUCHPAD_INIT_EVT        (1 << 0)  // 触控板初始化事件
#define TOUCHPAD_DATA_EVT        (1 << 1)  // 触控数据处理事件
#define TOUCHPAD_REG_INIT_EVT    (1 << 2)  // 触控寄存器事件
#define TOUCHPAD_KB_BREAK_EVT    (1 << 3)  // 键盘打字隔离到期事件

/* 注册 touchpad middleware 的独立 OSAL 任务。 */
void touchpad_setup(void);

/* 直接执行上电路径，并启动 middleware 初始化状态机。 */
void touchpad_power_on(void);

/* 关闭触控板，并清理 middleware 侧的初始化定时事件。 */
void touchpad_power_off(void);

/* 将 input_service 的 GPIO 中断转发为 touchpad 数据事件。 */
void touchpad_notify_int(void);

/* 设置键盘打字后触控板抑制时间，单位毫秒。 */
void touchpad_set_kb_break(uint16_t ms);

/* 停止 touchpad middleware 已投递的待处理事件。 */
void touchpad_stop_all_events(void);

/* 执行看门狗检查，必要时重走触控板初始化流程。 */
void touchpad_watchdog_check(void);

/* 对外暴露触控板模式切换，底层仍由 touch_component 实现。 */
void touchpad_set_mode(uint8_t mode);
