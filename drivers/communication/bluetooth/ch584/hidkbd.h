/********************************** (C) COPYRIGHT *******************************
 * File Name          : hidkbd.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/12/10
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef HIDKBD_H
#define HIDKBD_H
#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>

#include "storage.h"

/*********************************************************************
 * CONSTANTS
 */

// Task Events
#define START_DEVICE_EVT          0x0001
#define START_PARAM_UPDATE_EVT    0x0002
#define START_PHY_UPDATE_EVT      0x0004
#define WAIT_TERMINATE_EVT          0x0010
#define SEND_PACKET_EVT          0x0040
#define DELETE_PACKET_EVT          0x0080
#define SEND_DISCONNECT_EVT          0x0100
#define BLE_CLEAR_BUF_EVT         0x0200
#define PERI_SECURITY_REQ_EVT         0x0400


#define TERMINATE_EVT_TIMEOUT                160

// Minimum connection interval (units of 1.25ms)
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL    9

// Maximum connection interval (units of 1.25ms)
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL    12

// Slave latency to use if parameter update request
#define DEFAULT_DESIRED_SLAVE_LATENCY        6

// Supervision timeout value (units of 10ms)
#define DEFAULT_DESIRED_CONN_TIMEOUT         300

// iOS 连接参数申请目标
#define IOS_DESIRED_MIN_CONN_INTERVAL        12
#define IOS_DESIRED_MAX_CONN_INTERVAL        12
#define IOS_DESIRED_SLAVE_LATENCY            4
#define IOS_DESIRED_CONN_TIMEOUT             400

/**
 * 广播会话目的（与上层 BLE_INTENT_PAIRING / RECONNECT 对齐）。
 *
 * - PAIRING_NEW：重新配对窗口；Limited Discoverable、较快广播间隔；可选轮换 Random Static 地址。
 * - RECONNECT：已有绑定下的回连；General Discoverable、低占空比广播；保持与绑定一致的设备身份。
 */
typedef enum {
    BLE_ADV_PURPOSE_PAIRING_NEW = 0,
    BLE_ADV_PURPOSE_RECONNECT   = 1,
} ble_adv_purpose_t;
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * FUNCTIONS
 */
/*********************************************************************
 * GLOBAL VARIABLES
 */

/*
 * Task Initialization for the BLE Application
 */
extern void HidEmu_Init(void);

/*
 * Task Event Processor for the BLE Application
 */
extern uint16_t HidEmu_ProcessEvent(uint8_t task_id, uint16_t events);

extern void update_conn_pamm(void);

extern void hidEmu_disconnect(void);
extern uint8_t hidEmu_GetGAP_State(void);

/** 设置广播 Flags 为 Limited / General Discoverable，实际载荷由统一 helper 下发。 */
extern void hidEmu_apply_advert_discoverability_flags(uint8_t mode);

/**
 * 按会话目的启动广播（停播 → 地址/Flags/间隔策略 → 开播）。
 * @param purpose 见 ble_adv_purpose_t
 * @param idx     主机槽位（回连时用于校验绑定；配对时预留）
 */
extern void hidEmu_start_advertising_for_purpose(ble_adv_purpose_t purpose, uint8_t idx);

extern void hidEmu_pairing_adv(uint8_t idx);

extern void hidEmu_connect_adv(uint8_t idx);


extern uint8_t hidEmu_is_ble_bonded(uint8_t ble_idx);
extern void hidEmu_save_ble_bonded(access_ble_idx_t ble_idx,uint8_t is_pairing);
extern uint8_t hidEmu_is_ble_mac_change( access_ble_idx_t ble_idx );
extern void hidEmu_delete_ble_bonded_by_idx(access_ble_idx_t idx);
extern void hidEmu_delete_ble_bonded(void);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif
