
#pragma once

/** 所有通讯相关服务事件
 * 开始广播
 * 结束广播
 * 连接
 * 断开连接
 * 重新配对
 * 切换信道
 * USB 连接
 * USB 断开
 */
typedef enum {
    WIRELESS_ADV_EVENT = 1 << 0,
} commu_task_event_t;