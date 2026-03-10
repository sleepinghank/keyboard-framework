#pragma once

#include <stdint.h>

typedef struct {
    uint8_t     hostIndex;
    uint16_t    timeout;     /* Pairing timeout, valid value range from 30 to 3600 seconds, 0 for default */
    uint8_t     pairingMode; /* 0: default, 1: Just Works, 2: Passkey Entry */
    uint8_t     BRorLE;      /* Only available for dual mode module. Keep 0 for single mode module */
    uint8_t     txPower;     /* Only available for BLE module */
    // const char* leName;      /* Only available for BLE module */
} pairing_param_t;

/*********************************************************************
 * @fn      bt_driver_dump_state
 *
 * @brief   打印 BLE 底层状态快照，用于调试状态机转换
 *          输出：GAP 状态、当前工作主机、绑定信息
 *
 * @return  none
 */
void bt_driver_dump_state(void);



