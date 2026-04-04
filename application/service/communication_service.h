
#pragma once
#include <stdint.h>

/**
 * @file communication_service.h
 * @brief 通信服务事件定义
 *
 * 事件分三类：
 * 1. WL_STS_* — 无线状态通知（驱动层上报）
 * 2. WL_REQ_* — 无线触发请求（上层发起）
 * 3. USB_STS_* — USB 状态通知
 */

/* 无线状态通知 - 驱动层上报 */
#define WL_STS_INIT               (1 << 0)   // 无线模块初始化完成
#define WL_STS_RESET              (1 << 1)   // 无线模块重置
#define WL_STS_DISCOVERABLE       (1 << 2)   // 已进入可发现/配对广播
#define WL_STS_RECONNECTING       (1 << 3)   // 已进入回连广播
#define WL_STS_CONNECTED          (1 << 4)   // 无线连接成功
#define WL_STS_DISCONNECTED       (1 << 5)   // 无线断开连接
#define WL_STS_ADV_END            (1 << 6)   // 广播结束（进入挂起）

/* 报告缓冲发送事件 */
#define REPORT_SEND_EVT            (1 << 7)   // 报告缓冲区有数据待发送
#define REPORT_DELETE_PAK_EVT      (1 << 8)   // 报告缓冲区有数据待删除
#define REPORT_CLEAR_EVT            (1 << 9)   // 报告缓冲区清空事件

/* 无线触发请求 - 上层发起 */
#define WL_REQ_PAIR               (1 << 10)  // 发起配对
#define WL_REQ_RECONNECT          (1 << 11)  // 发起回连
#define WL_REQ_DISCONNECT         (1 << 12)  // 发起断开

/* USB 状态通知 */
// #define USB_STS_CONNECT           (1 << 12)
// #define USB_STS_DISCONNECT        (1 << 13)

/* task ID 外部声明 */
extern uint8_t commu_taskID;

/* 函数声明 */
void commu_service_init(void);
