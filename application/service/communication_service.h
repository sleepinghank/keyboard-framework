
#pragma once
#include <stdint.h>

/**
 * @file communication_service.h
 * @brief 通信服务事件定义
 *
 * 所有通讯相关服务事件：
 * - reset
 * - INIT
 * - disconnected
 * - Paring（带信道号）
 * - connected
 * - suspend
 * - reconnect（带信道号，会自动判断）
 * - LPM prepare/resume（bits 12-13）
 */

/* 无线事件定义 */
#define WL_INIT_EVT               (1 << 0)
#define WL_RESET_EVT              (1 << 1)
#define WL_DISCOVERABLE_EVT       (1 << 2)   /* 已进入可发现/配对广播 */
#define WL_RECONNECTING_EVT       (1 << 3)   /* 已进入回连广播 */
#define WL_CONNECTED_EVT          (1 << 4)
#define WL_DISCONNECTED_EVT       (1 << 5)
#define WL_ADVEND_EVT             (1 << 6)
#define WL_HID_SET_PROTOCOL_EVT   (1 << 7)
#define WL_HID_INDICATOR_EVT      (1 << 8)
#define WL_CONECTION_INTERVAL_EVT (1 << 9)
#define WL_PAIR_EVT               (1 << 10)  /* 上层发起配对请求 */
#define WL_RECONNECT_EVT          (1 << 11)
// #define USB_CONNECT_EVT           (1 << 12)
// #define USB_DISCONNECT_EVT        (1 << 13)

/* LPM 调度事件（bits 14-15） */
#define COMMU_LPM_PREPARE_EVT     (1 << 12)
#define COMMU_LPM_RESUME_EVT      (1 << 13)

/* 配对参数结构体 */
typedef struct {
    uint8_t     hostIndex;
    uint16_t    timeout;     /* Pairing timeout, valid value range from 30 to 3600 seconds, 0 for default */
    uint8_t     pairingMode; /* 0: default, 1: Just Works, 2: Passkey Entry */
    uint8_t     BRorLE;      /* Only available for dual mode module. Keep 0 for single mode module */
    uint8_t     txPower;     /* Only available for BLE module */
    const char* leName;      /* Only available for BLE module */
} pairing_param_t;

/* task ID 外部声明 */
extern uint8_t commu_taskID;

/* 函数声明 */
void commu_service_init(void);
