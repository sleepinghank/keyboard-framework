/********************************** (C) COPYRIGHT *******************************
 * File Name          : pct1336_driver.h
 * Author             : Hank
 * Version            : V1.0
 * Date               : 2025/10/15
 * Description        : 
 *******************************************************************************/

#ifndef PCT1336_DRIVER_H
#define PCT1336_DRIVER_H

/*********************************************************************
 * INCLUDES 头文件
 */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/*********************************************************************
 * CONSTANTS 常量
 */
#pragma pack(1)
/// @brief 触控板状态位枚举
typedef enum{
    TOUCH_STATUS_ERROR          = 0x01,  ///< [0] 错误状态位。当此位被置位时，表示发生了某些错误。主机必须重置并重新初始化设备。
    TOUCH_STATUS_TOUCH          = 0x02,  ///< [1] 触摸状态位。当此位被置位时，本帧的触摸数据有效。主机可以从0x01~0x1f读取数据。
    TOUCH_STATUS_GESTURE        = 0x08,  ///< [3] 手势状态位。当此位被置位时，本帧的手势数据有效。主机可以从0x60~0x6A读取数据。
    TOUCH_STATUS_PALM_DETECTION = 0x20,  ///< [5] 手掌检测状态位。当此位被置位时，检测到手掌。
    TOUCH_STATUS_BUTTON         = 0x40,  ///< [6] 按钮状态位。当此位被置位时，本帧的按钮状态有效。主机可以从BUTTON_STAT读取数据。
    TOUCH_STATUS_WATCHDOG_RESET = 0x80   ///< [7] 看门狗复位位。当此位被置位时，设备被看门狗重置到调试模式。主机必须重置并重新初始化设备。
}touch_status_t;


/// @brief 触控板手势类型枚举
typedef enum {
    GESTURE_NONE = 0,                   ///< 无手势
    GESTURE_ONE_FINGER_MOVE = 1,        ///< 单指相对移动，需要读取Reg0x62~65获取相对光标移动
    GESTURE_ONE_FINGER_DOWN = 11,       ///< 单指按下，需要读取Reg0x62~65获取相对移动
    GESTURE_ONE_FINGER_UP = 12,         ///< 单指抬起
    GESTURE_TWO_FINGER_DOWN = 3,        ///< 双指按下
    GESTURE_TWO_FINGER_UP = 60,         ///< 双指抬起
    GESTURE_TWO_FINGER_SCROLL_V = 7,    ///< 双指垂直滚动，需要读取Reg0x62~63获取滚动量
    GESTURE_TWO_FINGER_SCROLL_H = 8,    ///< 双指水平滚动，需要读取Reg0x62~63获取滚动量
    GESTURE_TWO_FINGER_PINCH = 9,       ///< 双指捏合(缩放)，需要读取Reg0x62~63获取捏合量
    GESTURE_TWO_FINGER_PINCH_END = 10,  ///< 双指捏合结束
    GESTURE_THREE_FINGER_DOWN = 30,     ///< 三指按下
    GESTURE_THREE_FINGER_UP = 60,       ///< 三指抬起
    GESTURE_THREE_FINGER_SWIPE_UP = 17, ///< 三指向上滑动
    GESTURE_THREE_FINGER_SWIPE_DOWN = 18, ///< 三指向下滑动
    GESTURE_THREE_FINGER_SWIPE_LEFT = 19, ///< 三指向左滑动，详情参考文档第58页图39
    GESTURE_THREE_FINGER_SWIPE_RIGHT = 20, ///< 三指向右滑动，详情参考文档第58页图39
    GESTURE_THREE_FINGER_SWIPE_MOVE = 65, ///< 三指水平滑动的移动信息，详情参考文档第58页图39
    GESTURE_THREE_FINGER_SWIPE_END = 66,  ///< 三指水平滑动结束
    GESTURE_FOUR_FINGER_DOWN = 63,      ///< 四指按下
    GESTURE_FOUR_FINGER_UP = 64         ///< 四指抬起
} gesture_type_t;

/// @brief 触控板按钮状态枚举
typedef enum {
    BUTTON_NONE = 0x00,              ///< 无按钮按下
    BUTTON_LEFT_DOWN = 0x01,         ///< 左键按下
    BUTTON_LEFT_UP = 0x02,           ///< 左键抬起
    BUTTON_RIGHT_DOWN = 0x08,        ///< 右键按下
    BUTTON_RIGHT_UP = 0x10           ///< 右键抬起
} button_state_t;
#pragma pack()
/*********************************************************************
 * MACROS 宏定义
 */
#define	TP_MAX_CONTACT_COUNT							4 // 最大总触摸点数
#define	TP_MAX_CONTACT_ID						6 // 报告的最大ID

/*********************************************************************
 * TYPEDEFS 数据结构
 */

#pragma pack(push)
#pragma pack(1) 
/* 单手指数据 */
typedef struct contact_data {
	uint8_t contact_id;
    uint8_t tip: 1;
    uint8_t confidence: 1;
    uint8_t : 6;
    uint16_t x;
    uint16_t y;
	uint16_t size;
} contact_data_t;

// typedef struct contact_attr {
//     uint16_t size; // 大小
//     // uint16_t soi; // 物体强度
//     // uint8_t width; //  宽度
//     // uint8_t height; //  高度
//     // uint16_t force; // 物体的最⼤接触⼒
// } contact_attr_t;

/* 完整触控数据 */
typedef struct touchpad_data {
    uint8_t contact_count;// 手指总数    1
    contact_data_t contacts[TP_MAX_CONTACT_COUNT]; // 手指 x y 信息 6*4 = 24
    // CONTACT_ATTR contact_attrs[TP_MAX_FINGER_COUNT]; // 手指强度信息  8*4 = 32
    // uint8_t :3;
    // uint8_t button_status:1; // DRV0/BTN 状态指⽰位 0
    // uint8_t :4;
    // uint8_t gesture_type;//GESTURE_0_TYPE
} touchpad_data_t;

/* pct1336 固件参数 */
typedef union  {
    struct params_struct  // 为结构体添加名称
    {
        uint8_t bank;
        uint8_t address;
        uint8_t data;
    } params;  // 为结构体成员添加名称
    uint8_t normal_keys[3];
} pct1336_params_t;

#pragma pack(pop)


/*********************************************************************
 * Profile Callbacks 结构体
 */


/*********************************************************************
 * API FUNCTIONS 公开的函数
 */
/*********************************************************************
 * @fn      pct1336_init
 * @brief   触控板初始化，如果需要携带参数，请先调用pct1336_set_init_params设置参数
 *
 * @note    请严格遵循启动时序，VDDIO上电后150ms再调用该方法
 *
 * @return  Success(1) or Failure(0)
 */
int8_t pct1336_init(void);

/*********************************************************************
 * @fn      pct1336_resume
 * @brief   触控板复位方法
 *
 * @param   none
 *
 * @return  Success(1) or Failure(0)
 */
int8_t pct1336_resume(void);


/*********************************************************************
 * @fn      pct1336_sleep
 * @brief   触控板深度睡眠方法。
 *
 * @param   none
 *
 * @return  Success(1) or Failure(0)
 */
int8_t pct1336_sleep(void);

/*********************************************************************
 * @fn      pct1336_read_ptp_report
 * @brief   获取PTP 触摸数据，用于实现PTP 触控板
 *
 * @param   *ptp 原始数据指针 
 *
 * @return  1：有数据 0：无数据
 */
int8_t pct1336_read_ptp_report(touchpad_data_t *ptp);


/*********************************************************************
 * @fn      pct1336_read_mouse
 * @brief   获取鼠标数据
 *
 * @note    当gesture_type=1 时（是否为手指相对移动）。x和y 数据才有效。
 * 
 * @param   *gesture_type 手势类型 具体请参考数据手册
 * @param   *x  鼠标X轴偏移量
 * @param   *y  鼠标Y轴偏移量
 *
 * @return  1：有数据 0：无数据
 */
int8_t pct1336_read_mouse(uint16_t *x, uint16_t *y);
/*********************************************************************
 * @fn      pct1336_clear_int
 * @brief   触控板中断清除
 *
 * @note    请在每次读取完数据后调用该方法
 * 
 * @param   none
 *
 * @return  Success(1) or Failure(0)
 */
int8_t pct1336_clear_int(void);


/*********************************************************************
 * @fn      pct1336_read_status
 * @brief   获取触控板状态
 * 
 * @param   *st 触摸状态 不能为NULL
 * @param   *button_st 按钮状态  可以为NULL
 * @param   *gesture_st 手势状态  可以为NULL
 *
 * @return  Success(1) or Failure(0)
 */
int8_t pct1336_read_status(uint8_t *st, uint8_t *button_st , uint8_t *gesture_st);

/**
 * @brief 查询固件是否就绪
 * @return true 已就绪, false 未就绪或读取失败
 */
bool pct1336_fw_ready(void);

/**
 * @brief 设置触控板初始化参数
 * @param params 具体参数
 * @param len 参数长度，以3个字节为一个单位
 */
void pct1336_set_init_params(pct1336_params_t* params, uint8_t len);


/*********************************************************************
 * @fn      pct1336_watchdog_check
 * @brief   定时检查触控板状态，检测到错误时记录日志
 *          建议每2秒调用一次
 *
 * @param   none
 *
 * @return  none
 */
bool pct1336_watchdog_check(void);


int8_t pct1336_register_cb(void);
/*********************************************************************
*********************************************************************/

#endif //PCT1336_DRIVER_H
