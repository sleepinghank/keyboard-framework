#pragma once

#include <stdint.h>
#include "storage.h"

typedef enum {
    WORK_IDEL = 0,
    WORK_PARING,
    WORK_RECONNECT,
    WORK_DISCONNECTED,
    WORK_MAX,
}access_work_mode_t;

typedef struct {
    uint8_t     hostIndex;
    uint16_t    timeout;     /* Pairing timeout, valid value range from 30 to 3600 seconds, 0 for default */
    uint8_t     pairingMode; /* 0: default, 1: Just Works, 2: Passkey Entry */
    uint8_t     BRorLE;      /* Only available for dual mode module. Keep 0 for single mode module */
    uint8_t     txPower;     /* Only available for BLE module */
    // const char* leName;      /* Only available for BLE module */
} pairing_param_t;

typedef struct
{
    uint8_t pairing_state;
    volatile uint8_t sleep_en;
    uint8_t deep_sleep_flag;
    uint8_t idel_sleep_flag;
    access_ble_idx_t  ble_idx;
    access_work_mode_t work_mode;
}access_state_t;

extern access_state_t access_state;
extern access_ble_idx_t con_work_mode;
/*********************************************************************
 * @fn      bt_driver_dump_state
 *
 * @brief   打印 BLE 底层状态快照，用于调试状态机转换
 *          输出：GAP 状态、当前工作主机、绑定信息
 *
 * @return  none
 */
void bt_driver_dump_state(void);

/*********************************************************************
 * @brief   HID LED 状态回调函数类型
 *
 * @param   led_state - LED 状态位图 (bit0:NumLock, bit1:CapsLock, bit2:ScrollLock)
 */
typedef void (*bt_led_cb_t)(uint8_t led_state);

/*********************************************************************
 * @fn      bt_driver_register_led_cb
 *
 * @brief   注册 LED 状态回调函数，由 wireless 层调用
 *
 * @param   cb - 回调函数指针，NULL 表示注销
 */
void bt_driver_register_led_cb(bt_led_cb_t cb);

/*********************************************************************
 * @fn      bt_driver_notify_led_state
 *
 * @brief   通知 LED 状态变更，由 Profile 层收到 HID Out Report 时调用
 *
 * @param   led_state - LED 状态位图
 */
void bt_driver_notify_led_state(uint8_t led_state);

