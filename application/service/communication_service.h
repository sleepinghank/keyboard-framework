
#pragma once
#include <stdint.h>

/** 所有通讯相关服务事件
 * reset
 * INIT
 * disconnected 
 * Paring*（带信道号）
 * connected
 * suspend
 * reconnect(带信道号，会自动判断)
 * USB 连接
 * USB 断开
 */
typedef enum {
    WL_INIT_EVT             = 1 << 0,  /**< 无线模块初始化事件 */
    WL_RESET_EVT            = 1 << 1,  /**< 无线模块初始化事件 */
    WL_DISCOVERABLE_EVT          = 1 << 2,  /**< 无线配对事件（带信道号） */
    WL_RECONNECTING_EVT        = 1 << 3,  /**< 无线回连事件（带信道号，自动判断） */
    WL_CONNECTED_EVT        = 1 << 4,  /**< 无线连接成功事件 */
    WL_DISCONNECTED_EVT     = 1 << 5,  /**< 无线断开连接事件 */
    WL_ADVEND_EVT          = 1 << 6,  /**< 广播结束，进入无线挂起事件 */
    WL_HID_SET_PROTOCOL_EVT          = 1 << 7,  /**< 设置协议 */
    WL_HID_INDICATOR_EVT          = 1 << 8,  /**< hid 指示灯 */
    WL_CONECTION_INTERVAL_EVT          = 1 << 9,  /**< 连接间隔 */
    WL_PAIR_EVT          = 1 << 10,  /**< 配对事件（带信道号） */
    WL_RECONNECT_EVT= 1 << 11,  /**< 回连事件（带信道号，自动判断） */
    USB_CONNECT_EVT               = 1 << 12,  /**< USB 连接事件 */
    USB_DISCONNECT_EVT            = 1 << 13,  /**< USB 断开事件 */
} commu_task_EVT_t;

typedef struct {
    uint8_t     hostIndex;
    uint16_t    timeout;     /* Pairing timeout, valid value range from 30 to 3600 seconds, 0 for default */
    uint8_t     pairingMode; /* 0: default, 1: Just Works, 2: Passkey Entry */
    uint8_t     BRorLE;      /* Only available for dual mode module. Keep 0 for single mode module */
    uint8_t     txPower;     /* Only available for BLE module */
    const char* leName;      /* Only available for BLE module */
} pairing_param_t;
