/********************************** (C) COPYRIGHT *******************************
 * File Name          : touchpad_service.h
 * Author             : Hank
 * Version            : V1.0
 * Date               : 2025/10/15
 * Description        : 
 *******************************************************************************/

#ifndef TOUCHPAD_SERVICE_H
#define TOUCHPAD_SERVICE_H

/*********************************************************************
 * MACROS 宏定义
 */


/*********************************************************************
 * INCLUDES 头文件
 */
#include "pct1336_driver.h"
#include "kb904/config_product.h"
#include "report.h"
#ifdef MCS_GESTURE
#include "mcs_gesture.h"
#endif
#ifdef BAYES_MISTOUCH
#include "bayes_filtering.h"
#endif
#include "report.h"
/*********************************************************************
 * CONSTANTS 常量
 */
typedef enum
{
    TOUCH_MODE_MOUSE = 0,
	TOUCH_MODE_PTP = 3,
} touch_mode_t;

// 触控板防误触防抖周期
#ifndef BAYES_DEBOUNCE_CYCLE
#define BAYES_DEBOUNCE_CYCLE  3
#endif
/*********************************************************************
 * TYPEDEFS 数据结构
 */


// typedef struct HidPTPReport
// {	
// 	uint8_t scantime_l8;
// 	uint8_t scantime_m8;
//     uint8_t button:1;
//     uint8_t button1:1;
//     uint8_t button2:1;
//     uint8_t Reserved:5;
// 	contact_report_t contact_rpt[TP_MAX_CONTACT_COUNT];
// 	uint8_t contactCnt;
// }report_ptp_t;
 

// typedef struct HidMouseReport
// {	
// 	uint8_t button;
// 	uint8_t x_l8:8;
// 	uint8_t x_m4:4; 
// 	uint8_t y_l4:4;	
// 	uint8_t y_m8:8;	
// 	uint8_t wheel;
// 	uint8_t twheel;
// }report_mouse_t;

typedef struct AntiMistouchData {
    uint8_t is_new; // 是否新数据
    // int small_count; // 小触点连续保持次数 需要超过22，size为4，
    int valid; // 当前是否有效 0:默认   1：有效  -1：无效
    int confidence;  // 0:默认   1：有效  -1：无效
    // int valid_key_count; // 记录防误触增强后有效按键次数，（生命周期为按键后 一段时间内）
    // uint16_t total_count; // 连续触摸次数
    // uint16_t distance; // 两手指的距离
    // uint16_t size; // 触摸大小
    // uint8_t index;// 在数组的索引位置
    // int edge_valid;// 是否在边缘  0:默认   1：有效  -1：无效
    int distance_valid;// 默认距离是否有效 0:默认   1：有效  -1：无效
    uint8_t distance_valid_count;// 距离 连续有效次数
} anti_mistouch_data_t;

/*********************************************************************
 * Global Variables
 */
extern uint8_t touch_en;
#ifdef MCS_GESTURE
extern GestureType act_gesture;
#endif
/*********************************************************************
 * Profile Callbacks 回调
 */

/*********************************************************************
 * API FUNCTIONS 公开的函数
 */

/*********************************************************************
 * @fn      touch_power_on
 *
 * @brief   触控板开机，直接完成基础硬件初始化，是否启动成功请参考touch_en值
 *
 * @param   none
 *
 * @return  Success or Failure
 */
int8_t touch_power_on(void);


/*********************************************************************
 * @fn      touch_power_on_with_params
 * 
 * @brief   带参数的开机，直接完成基础硬件初始化，是否启动成功请参考touch_en值
 * 
 * @param   params 参数指针
 * 
 * @param   len 参数长度，以3个字节为一个单位
 * 
 * @return  Success or Failure
 */
int8_t touch_power_on_with_params(pct1336_params_t* params, uint8_t len);


/*********************************************************************
 * @fn      touch_power_off
 *
 * @brief   触控板关机，默认GPIO 都为上拉输入
 *
 * @param   none
 *
 * @return  Success or Failure
 */
int8_t touch_power_off(void);

/*********************************************************************
 * @fn      touch_evt_task
 *
 * @brief   触控板事件处理，放到main主循环中调用
 *
 * @param   none
 *
 * @return  Success or Failure
 */
void touch_evt_task(void);


/*********************************************************************
 * @fn      set_touch_mode
 * @brief   设置触摸板模式
 *
 * @note    如果不进行设置，默认是鼠标模式
 * 
 * @param   mode 触摸板模式
 *
 * @return  none
 */
void set_touch_mode(touch_mode_t mode);

/*********************************************************************
 * @fn      set_kb_break_cnt
 *
 * @brief   设置键盘打字隔离触摸板时间
 *
 * @param   cnt 打字隔离时间，单位为扫描定时器时间，默认应该设置为400ms
 *
 * @return  none
 */
void set_kb_break_cnt(uint16_t cnt);

/*********************************************************************
 * @fn      get_touch_scan_time
 *
 * @brief   获取触摸板扫描时间
 *
 * @param   none
 *
 * @return  scan_time 触摸板扫描时间
 */
uint16_t get_touch_scan_time(void);

/*********************************************************************
 * @fn      end_touchpad_input
 *
 * @brief   结束触摸板输入处理，用于释放或重置触摸板输入相关资源
 *
 * @param   none
 *
 * @return  uint8_t 1：有数据 0：无数据
 */
uint8_t end_touchpad_input(void);


/*********************************************************************
 * @fn      touch_watchdog_check
 * @brief   触控板看门狗检查函数，用于检测触控板是否出现错误状态或看门狗复位
 *          生命周期恢复由上层 middleware 负责
 *
 * @note    该函数应定期调用（建议每2秒调用一次）以监控触控板状态
 *          只有在触控板已启用但未处于触摸处理状态时才执行检查
 * 
 * @param   none
 *
 * @return  none
 */
bool touch_watchdog_check(void);
/*********************************************************************
*********************************************************************/

#endif //TOUCHPAD_SERVICE_H
