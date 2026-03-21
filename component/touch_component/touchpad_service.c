/********************************** (C) COPYRIGHT *******************************
 * File Name          : touchpad_service.c
 * Author             : Hank
 * Version            : V1.0
 * Date               : 2025/10/15
 * Description        : 触控板业务层，读取对应驱动层的数据，并通过BLE发送
 *******************************************************************************/

/*********************************************************************
 * INCLUDES 头文件
 */
#include "touchpad_service.h"
#include "pct1336_driver.h"
#include "gpio.h"
#include "wait.h"
#include "atomic_util.h"
#include "bt_driver.h"
#include "debug.h"
#include "kb904/config.h"
#include "util.h"
#include "i2c_master.h"
#include <math.h>
#include <stdlib.h>

/*********************************************************************
 * MACROS 宏定义
 */
// 调试打印宏
#if 1
#define TOUCHPAD_log(...) dprintf(__VA_ARGS__)
#else
#define TOUCHPAD_log(...)
#endif

// 触控板逻辑
#define SPECIAL_EDGE_X_MIN 5
#define SPECIAL_EDGE_X_MAX (TOUCHPAD_MAX_X - SPECIAL_EDGE_X_MIN)
#define SPECIAL_EDGE_Y_MIN 5
#define SPECIAL_EDGE_Y_MAX (TOUCHPAD_MAX_Y - SPECIAL_EDGE_Y_MIN)

// 取从右往左第一个bit为1的位置
#define LOW1BIT(X) (debruijn[((unsigned int)(((X) & -(X)) * 0x077CB531U)) >> 27])
/*********************************************************************
 * CONSTANTS 常量
 */
static const int debruijn[32] = {
        0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
        31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};
/*********************************************************************
 * TYPEDEFS 数据类型定义
 */
typedef struct _OTHER_TOUCH_DATA {
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
} extend_,*OOTHER_TOUCH_DATA;
/*********************************************************************
 * GLOBAL VARIABLES 全局变量
 */
uint8_t touch_en = 0; // 触摸板状态
uint8_t touch_mode = 1; // 触摸板模式
uint8_t _touch_int_flag = 0; // 触摸中断标志
/*********************************************************************
 * EXTERNAL VARIABLES   外部变量
 */
// 操作系统类型定义 (用于 Y 轴翻转判断)
#ifndef IOS
#define IOS 1
#endif

// 默认操作系统类型 (0: Windows/Android, 1: iOS/macOS)
#ifndef TOUCHPAD_DEFAULT_OS_TYPE
#define TOUCHPAD_DEFAULT_OS_TYPE 0
#endif
static uint8_t keycode_type = TOUCHPAD_DEFAULT_OS_TYPE;

// 触控板手势开关配置
static uint8_t touch_gesture_switch_bitmap = 0;
/*********************************************************************
 * EXTERNAL FUNCTIONS 外部函数
 */

/*********************************************************************
 * LOCAL VARIABLES 本地变量
 */
static uint8_t _touch_button_repeat_count=0; // 按键重复次数
static uint8_t _touch_button_repeat_flag=0; // 按键重复标志
static uint8_t start_touch_flag = 0; // 触摸开始标志
static uint16_t scan_time =0; // 扫描时间
static hid_ptp_report_t ptp_reports; // PTP报文
static hid_ptp_report_t ptp_reports_clone; // PTP报文
static touchpad_data_t original_reports;
uint8_t motion[4]= {0};
uint8_t motion_btn=0;
#ifdef DOUBLE_CLICK_TO_RIGHT_BUTTON
// 右边按钮向左边按钮过度标识
uint8_t left2right_button_flag = 0;
#endif
// 标记手指是否完全离开, 0:已离开，1：未离开
uint8_t touch_out_flag = 0;
uint8_t Pre_Tip[TP_MAX_CONTACT_ID]= {0,0,0,0,0,0};
uint8_t Pre_Confidence[TP_MAX_CONTACT_ID]= {0,0,0,0,0,0};
#ifdef HOLD_BUTTON
// 按压时 记录按压手指
contact_data_t pre_contact;
// 是否持续按压
static uint8_t is_contin_press = 0;
#endif
// 中断为低的异常次数
uint8_t int_low_cnt = 0;


/*****防误触相关*****/
#ifdef BAYES_MISTOUCH
// 防误触激活
uint8_t filter_active_flag = 0;

// 防误触原始数据
static anti_mistouch_data_t mistouch_datas[TP_MAX_CONTACT_ID];
// 原始数据备份
contact_data_t contacts[TP_MAX_CONTACT_ID];
// 记录上次原始数据
uint8_t Pre_Tip_Raw[TP_MAX_CONTACT_ID]= {0,0,0,0,0,0};
#endif
// 键盘按键被按下后，防误触计时
uint16_t kb_break_cnt = 0;


/******触控预设手势相关 */
#ifdef MCS_GESTURE
static Point touch_arr[100] = {0};
static uint8_t touch_idx = 0;
gesture_type_t act_gesture = GESTURE_NONE;
#endif 


/*****鼠标报文*******/
static uint8_t Touchkeybuf[KEYBOARD_REPORT_SIZE];
static hid_mouse_report_t mouse_reports; // 鼠标报文
static uint8_t mouse_button_flag = 0; // 鼠标按键状态
/*********************************************************************
 * LOCAL FUNCTIONS 本地函数
 */

#ifdef MCS_GESTURE
void gesture_judgment(){
    GestureType result;
    if (touch_idx < 20){
        return;
    }
    result = recognize_gesture(touch_arr,touch_idx);
    TOUCHPAD_log("Recognized gesture: %d\n",result);
    if (result != GESTURE_NONE){
        act_gesture = result;
    }
}
#endif

static void send_ptp_data(hid_ptp_report_t *ptp){
    uint8_t send_len = sizeof(hid_ptp_report_t);
    if (keycode_type == IOS)
    {
        send_len--;
    }
    bt_driver_send_ptp((uint8_t*)ptp, send_len);

    #ifdef CONFIG_UART_ENABLE
    uint8_t i;
    uint16_t t = (uint16_t)ptp ->scantime_l8+((uint16_t)ptp ->scantime_m8<<8);
    TOUCHPAD_log("cnt:%d,button:%d,button2:%d,time:%d",ptp ->contactCnt,ptp ->button,ptp ->button1,t);
    for(i=0;i<4;i++)
    {
        TOUCHPAD_log("%d %x %x %d %d",ptp ->contact_rpt[i].contact_id,ptp ->contact_rpt[i].tip,ptp ->contact_rpt[i].confidence,
        (uint16_t)(ptp ->contact_rpt[i].x_l8+(ptp ->contact_rpt[i].x_m4<<8)),(uint16_t)(ptp ->contact_rpt[i].y_l4+(ptp ->contact_rpt[i].y_m8<<4)));
    }
    TOUCHPAD_log("-----------------------------------------");
    #endif
}

static uint8_t send_touch_data(){
    TOUCHPAD_log("send touch data");
    if (touch_mode == TOUCH_MODE_MOUSE){
        bt_driver_send_mouse((uint8_t*)&mouse_reports);
        #ifdef CONFIG_UART_ENABLE
        TOUCHPAD_log("button:%d,x:%d,y:%d,wheel:%d,twheel:%d",mouse_reports.button,mouse_reports.x_l8,mouse_reports.y_l4,mouse_reports.wheel,mouse_reports.twheel);
        #endif
    }
    else if (touch_mode == TOUCH_MODE_PTP){
        send_ptp_data(&ptp_reports);
    }
    return 0;
}

/// @brief 发送鼠标报文
/// @return 1：有数据 0：无数据
uint8_t ProcessMouseData(void)
{
    uint8_t st = 0,button_st = 0,gesture_st = 0;
    uint8_t send_flag = 0;
    // 读取触摸板状态
    if(pct1336_read_status(&st,&button_st,&gesture_st)  == 0){
        return 0;  // 设备读取失败直接返回
    }
    // TOUCHPAD_log("st:%x,button_st:%x,gesture_st:%x\n",st,button_st,gesture_st);
    // 清空鼠标报文
    memset((uint8_t*)&mouse_reports,0x00, sizeof(hid_mouse_report_t));
    memset(Touchkeybuf, 0x00, KEYBOARD_REPORT_SIZE);
    // 触摸板错误
    if ((st & TOUCH_STATUS_ERROR) == TOUCH_STATUS_ERROR) {
        pct1336_resume();
        return 0;
    }
    if ((st & TOUCH_STATUS_GESTURE) == TOUCH_STATUS_GESTURE) {
        uint16_t x=0,y=0;
        if(pct1336_read_mouse(&x,&y) == 0){
            return 0;  // 鼠标数据读取失败直接返回
        }
        switch (gesture_st)
        {
        case GESTURE_ONE_FINGER_MOVE:
            if( (x | y) != 0 )	//Gating x=y=0, insurance
            {
                mouse_reports.x_l8 = (x&0x00FF);
                mouse_reports.x_m4 = (x&0x0F00) >> 8;
                mouse_reports.y_l4 = (y&0x000F);
                mouse_reports.y_m8 = (y&0x0FF0)>>4;
                if(mouse_button_flag&0x01)
                    mouse_reports.button |= 0x01;
                else if(mouse_button_flag&0x02)
                    mouse_reports.button |= 0x02;
                send_flag = 1;
            }
            break;
        case GESTURE_TWO_FINGER_SCROLL_V:		//7: z wheel
            if(mouse_button_flag&0x01)
                mouse_reports.button |= 0x01;
            else if(mouse_button_flag&0x02)
                mouse_reports.button |= 0x02;
            mouse_reports.wheel = (int16_t)(-x);
            send_flag = 1;
            break;
        case GESTURE_TWO_FINGER_SCROLL_H:		// 8 : horizontal scroll
            if(mouse_button_flag&0x01)
                mouse_reports.button |= 0x01;
            else if(mouse_button_flag&0x02)
                mouse_reports.button |= 0x02;
            mouse_reports.twheel = x;
            send_flag = 1;
            break;
        default:
            break;
        }
    }
    if ((st & TOUCH_STATUS_BUTTON) == TOUCH_STATUS_BUTTON) {
        if(button_st==1 ||button_st==8)
        {
            if(button_st==1)
            {
                if(!(mouse_button_flag&0x01))
                {
                    mouse_reports.button |= 0x01;
                    mouse_button_flag |= 0x01;
                    send_flag = 1;
                }
            }
            else if(button_st==8)
            {
                if(!(mouse_button_flag&0x02))
                {
                    mouse_reports.button |= 0x02;
                    mouse_button_flag |= 0x02;
                    send_flag = 1;
                }
            }
        }
        else if(button_st==2||button_st==16) {
            if((mouse_button_flag & 0x01) == 0x01 && button_st == 2)
            {
                // 将第一个bit置为0
                mouse_button_flag = mouse_button_flag & 0xfe;
                send_flag = 1;
            } else if((mouse_button_flag & 0x02) == 0x02 && button_st == 16)
            {
                // 将第二个bit置为0
                mouse_button_flag = mouse_button_flag & 0xfd; 
                send_flag = 1;
            }
        }
    }
    if(send_flag == 1)
    {
        send_touch_data();
    }
    return send_flag;
}
#ifdef BAYES_MISTOUCH
/// @brief 根据X 轴 从大到小排序 冒泡排序
static void bubble_sort(contact_data_t arr[], int len) {
    int i, j;
    contact_data_t temp;
    for (i = 0; i < len - 1; i++){
        for (j = 0; j < len - 1 - i; j++){
            if (arr[j].x < arr[j + 1].x) {
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/// @brief 防误触过滤，只检测一指之后接触的，选左右间距最大的。前面两帧做防抖验证，
/// @param *pp 原始数据指针
/// @return 1：有数据 0：无数据
uint8_t unite_filtration(touchpad_data_t *ptp){
    uint8_t is_activate = 0;
    uint8_t i;
    uint8_t temp = 0;
    uint8_t contact_count = ptp -> contact_count;
    // if (contact_count < 2){
    //     // 只有一个触点时，延迟BAYES_DEBOUNCE_CYCLE个周期再报告。
    //     Pre_Tip_Raw[ptp -> contacts[0].contact_id] = ptp -> contacts[0].tip;
    //     memset(mistouch_datas, 0, sizeof(mistouch_datas));
    //     return is_activate;
    // }
    // 只有在新增数据时计算
    for (i = 0; i < contact_count; i++){
        if (ptp -> contacts[i].contact_id >= TP_MAX_CONTACT_ID) {
            continue;
        }
		if (ptp -> contacts[i].confidence == 0 || ptp -> contacts[i].tip == 0){
            memset(&mistouch_datas[ptp -> contacts[i].contact_id], 0, sizeof(anti_mistouch_data_t));
			continue;
		}
        if (mistouch_datas[ptp -> contacts[i].contact_id].is_new > 0){
            temp = 1;
            continue;
        }
        // 第一次接触
        if (Pre_Tip_Raw[ptp -> contacts[i].contact_id] == 0 ){
            mistouch_datas[ptp -> contacts[i].contact_id].is_new = BAYES_DEBOUNCE_CYCLE;
            temp = 1;
        }
    }
    if (temp == 1 && contact_count > 1){
        double features[2];
        memset(&contacts, 0,sizeof(contact_data_t) * contact_count);
        memcpy(&contacts, ptp ->contacts, sizeof(contact_data_t) * (contact_count));

        bubble_sort(contacts,contact_count);

        for (i = 0; i < contact_count; i++){
            uint8_t contact_id = contacts[i].contact_id;
            if (contact_id >= TP_MAX_CONTACT_ID) {
                continue;
            }
            // DBG("contact_id:%d,is_new:%d,x:%d",contact_id,mistouch_datas[contact_id].is_new,contacts[i].x);
            if (mistouch_datas[contact_id].is_new > 0 && mistouch_datas[contact_id].distance_valid != -1 ) {
                uint16_t befor_distance = 0, after_distance = 0;
                uint8_t befor_id = (i == 0 ? i : (i-1)), after_id = (i == (contact_count -1) ? i : (i + 1));
                // DBG("i:%d,befor_id:%d,after_id:%d",i,befor_id,after_id);
                befor_distance = contacts[befor_id].x - contacts[i].x ;
                after_distance = contacts[i].x - contacts[after_id].x;
                features[0] = (befor_distance > after_distance ? befor_distance : after_distance);
                features[1] = contacts[i].size;
                // DBG("contact_id:%d,distance:%f,size:%f",contact_id,features[0],features[1]);
                if (!predict(features)){
                    // DBG("no confind contact_id:%d",contact_id);
                    mistouch_datas[contact_id].distance_valid_count = mistouch_datas[contact_id].distance_valid_count+1;
                    if (mistouch_datas[contact_id].distance_valid_count >= BAYES_DEBOUNCE_CYCLE){
                        mistouch_datas[contact_id].distance_valid = -1;
                    }
                } else {
                    mistouch_datas[contact_id].distance_valid_count = 0;
                }
            }
        }
    }
    memset(Pre_Tip_Raw, 0, sizeof(Pre_Tip_Raw));
    // 更改原始数据
    for (i = 0; i < contact_count; i++){
        // 检查contact_id是否有效，防止数组越界
        if (ptp -> contacts[i].contact_id >= TP_MAX_CONTACT_ID) {
            continue;
        }
        Pre_Tip_Raw[ptp -> contacts[i].contact_id] = ptp -> contacts[i].tip;
        if (ptp -> contacts[i].confidence == 0 || ptp -> contacts[i].tip == 0){
			continue;
		}
        // TOUCHPAD_log("distance_valid:%d,is_new:%d",mistouch_datas[ptp -> contacts[i].contact_id].distance_valid,mistouch_datas[ptp -> contacts[i].contact_id].is_new);
        if (mistouch_datas[ptp -> contacts[i].contact_id].distance_valid == 0 &&  mistouch_datas[ptp -> contacts[i].contact_id].is_new > 0){
            // TOUCHPAD_log("contact_id:%d,set tip:0",ptp -> contacts[i].contact_id);
            // 第一次接触 并且没验证完成
            ptp -> contacts[i].x = 0;
            ptp -> contacts[i].y = 0;
            ptp -> contacts[i].confidence = 0;
            ptp -> contacts[i].tip = 0;
            // 当处于中间时，会误删掉最后一位
            // ptp -> contact_count = ptp -> contact_count - 1;
            is_activate = 1;
        } else if (mistouch_datas[ptp -> contacts[i].contact_id].distance_valid == -1 ){
            // DBG("contact_id:%d,set confidence:0",ptp -> contacts[i].contact_id);
            ptp -> contacts[i].confidence = 0;
            is_activate = 1;
        }
        if (mistouch_datas[ptp -> contacts[i].contact_id].is_new > 0){
            mistouch_datas[ptp -> contacts[i].contact_id].is_new --;
            // TOUCHPAD_log("is_new:%d",mistouch_datas[ptp -> contacts[i].contact_id].is_new);
        }
    }
    return is_activate;
}

#endif

/// @brief 判断是否存在无效数据
/// @return 1 存在无效数据，0 不存在无效数据
uint8_t has_no_confidence(void){
    uint8_t temp = 0;
    for (temp = 0; temp < TP_MAX_CONTACT_COUNT; temp++)
    {
        if (ptp_reports.contact_rpt[temp].confidence == 0 && ptp_reports.contact_rpt[temp].tip == 1)
        {
            return 1;
        }
    }
    return 0;
}
#ifdef DOUBLE_CLICK_TO_RIGHT_BUTTON
void send_clone_ptp_report(void){ 
    TOUCHPAD_log("send_clone_ptp_report");  
    send_ptp_data(&ptp_reports_clone);
}

void auto_release_right_button(){
    uint16_t t = scan_time + 200;
    TOUCHPAD_log("Auto release right button at time:%d",t);
    memcpy(&ptp_reports_clone, &ptp_reports, sizeof(hid_ptp_report_t));
    ptp_reports_clone.button = 0;
    ptp_reports_clone.button1 = 0;
    ptp_reports_clone.button2 = 0;
    ptp_reports_clone.scantime_l8= (uint8_t)(t&0x00ff);
    ptp_reports_clone.scantime_m8= (uint8_t)((t&0xff00)>>8);
    send_clone_ptp_report();
}
#endif
uint8_t ProcessGetPTPData(void){
    uint8_t st = 0,button_st = 0;
    uint8_t u8tmp, cnt = 0,contact_id;
    uint8_t temp = 0xff;
    memset((uint8_t*)&motion,0x00, 4);
    motion_btn = 0;
    // 读取触摸板状态
    if(pct1336_read_status(&st,&button_st,NULL) == 0){
        return 0;  // 设备读取失败直接返回
    }
    // TOUCHPAD_log("st:%x,button_st:%x",st,button_st);
    if ((st & TOUCH_STATUS_ERROR) == TOUCH_STATUS_ERROR || (st &TOUCH_STATUS_WATCHDOG_RESET) == TOUCH_STATUS_WATCHDOG_RESET) {
        pct1336_resume();
        return 0;
    }
    if ((st & TOUCH_STATUS_TOUCH) == TOUCH_STATUS_TOUCH) {
        u8tmp = pct1336_read_ptp_report(&original_reports);
        if (u8tmp == 0){   
            if (touch_out_flag == 0x81){
                touch_out_flag = 0;
                if (ptp_reports.button == 1){
                    ptp_reports.button = 0;
                    ptp_reports.button1 = 0;
                    motion_btn = 1;
                }
            }
            return 0;
        }
        // 存在触摸点
        touch_out_flag = 1;
        // #ifdef CONFIG_UART_ENABLE
        // {
        //     uint8_t i;
        //     TOUCHPAD_log("Original cnt:%d",original_reports.contact_count);
        //     for(i=0;i<4;i++)
        //     {
        //         TOUCHPAD_log("%d %x %x %d %d %d ",original_reports.contacts[i].contact_id,original_reports.contacts[i].tip,original_reports.contacts[i].confidence,
        //         original_reports.contacts[i].x,original_reports.contacts[i].y,original_reports.contacts[i].size);
        //     }
        //     TOUCHPAD_log("**************************");
        // }
        // #endif
        #ifdef EDGE_MISTOUCH
        if (cnt == 1 && button_st == 0){
            if (Pre_Tip[original_reports.contacts[0].contact_id] == 1 || original_reports.contacts[0].tip == 0){

            } else if (((original_reports.contacts[0].x > SPECIAL_EDGE_X_MAX) || (original_reports.contacts[0].x < SPECIAL_EDGE_X_MIN)) && original_reports.contacts[0].size < 6) {
                original_reports.contacts[0].tip = 0;
                original_reports.contacts[0].confidence = 0;
                original_reports.contacts[0].contact_id = 0;
                original_reports.contacts[0].x = 0;
                original_reports.contacts[0].y = 0;
                original_reports.contact_count --;
                cnt --;
            }
        }
        #endif

        #ifdef BAYES_MISTOUCH
        if (button_st == 0 ){
            // if (cnt != 2 || (fabs(original_reports.contacts[0].x - original_reports.contacts[1].x) > 1250)){
            filter_active_flag = unite_filtration(&original_reports);
            // }
        }
        #endif
        cnt = original_reports.contact_count;
        ptp_reports.contactCnt = cnt;
        for(u8tmp=0; u8tmp<TP_MAX_CONTACT_COUNT; u8tmp++)
        {
            uint16_t y_offset = original_reports.contacts[u8tmp].y;

            contact_id = original_reports.contacts[u8tmp].contact_id;
            ptp_reports.contact_rpt[u8tmp].tip=original_reports.contacts[u8tmp].tip;
            ptp_reports.contact_rpt[u8tmp].confidence=original_reports.contacts[u8tmp].confidence;
            ptp_reports.contact_rpt[u8tmp].contact_id= contact_id;
            // windows下需要反转Y轴
            if (keycode_type == IOS){
                if (y_offset == 0 && ptp_reports.contact_rpt[u8tmp].confidence == 0 && ptp_reports.contact_rpt[u8tmp].tip == 0){
                    y_offset = 0;
                } else {
                    y_offset = TOUCHPAD_MAX_Y - y_offset;
                }
            }
            
            ptp_reports.contact_rpt[u8tmp].x_l8 = (original_reports.contacts[u8tmp].x) & 0xff;
            ptp_reports.contact_rpt[u8tmp].x_m4 = (original_reports.contacts[u8tmp].x >> 8) & 0x0f ;
            ptp_reports.contact_rpt[u8tmp].y_l4 = (y_offset)&0x0f;
            ptp_reports.contact_rpt[u8tmp].y_m8 = (y_offset)>>4;
            if (u8tmp < cnt){
                // 不能从无置信度到有置信度，否则会导致数据错误
                if (Pre_Tip[contact_id] == 1 && Pre_Confidence[contact_id] == 0){
                    ptp_reports.contact_rpt[u8tmp].confidence = 0;
                }

                // 判断数据是否有变更，用于判断是否需要发送数据
                if((((ptp_reports.contact_rpt[u8tmp].x_l8 != 0) ||
                        (ptp_reports.contact_rpt[u8tmp].x_m4!=0)||
                        (ptp_reports.contact_rpt[u8tmp].y_l4!=0)||
                        (ptp_reports.contact_rpt[u8tmp].y_m8!=0)) && ptp_reports.contact_rpt[u8tmp].tip == 1)||
                        ((ptp_reports.contact_rpt[u8tmp].tip != Pre_Tip[contact_id]) && (Pre_Tip[contact_id]==1))) {
                    motion[u8tmp] = 1;
                    // TOUCHPAD_log("motion[%d] id:%d,:%d",u8tmp,contact_id,motion[u8tmp]);
                    // 判断有上报数据即为有触摸
                }
            }
        }

        memset(&Pre_Tip,0,sizeof(Pre_Tip));
        memset(&Pre_Confidence,0,sizeof(Pre_Confidence));
        // 用于 contact_id 去重和排序
        temp = 0xff;
        for (u8tmp = 0; u8tmp < TP_MAX_CONTACT_COUNT; u8tmp++) {
            if (u8tmp < cnt){
                contact_id = ptp_reports.contact_rpt[u8tmp].contact_id;
                Pre_Tip[contact_id] = ptp_reports.contact_rpt[u8tmp].tip;
                Pre_Confidence[contact_id] = ptp_reports.contact_rpt[u8tmp].confidence;
            } else {
                // 从低往高取空闲id，清空位置
                contact_id = LOW1BIT(temp);
                ptp_reports.contact_rpt[u8tmp].contact_id= contact_id;                // ptp_reports.contact_rpt[u8tmp].tip = 0;
                // ptp_reports.contact_rpt[u8tmp].confidence= 0;
                // ptp_reports.contact_rpt[u8tmp].x_l8= 0;
                // ptp_reports.contact_rpt[u8tmp].x_m4 = 0;
                // ptp_reports.contact_rpt[u8tmp].y_l4= 0;
                // ptp_reports.contact_rpt[u8tmp].y_m8= 0;
            }
            temp &= ~(BIT(contact_id));
        }
    } 
    if ((st & TOUCH_STATUS_BUTTON) == TOUCH_STATUS_BUTTON){
        if (button_st > 0){
            motion_btn = 1;
            if(button_st==1 ||button_st==8) // 1: L-button press, 8: R-button press
            {
                // 第一次按下时，记录按下的位置
                #ifdef DOUBLE_CLICK_TO_RIGHT_BUTTON
                if (cnt == 2 && scan_time < 2000){
                    ptp_reports.button = 0;
                    ptp_reports.button1 = 1;
                    left2right_button_flag = 1;
                } else {
                    #endif
                    ptp_reports.button=1;
                    ptp_reports.button1 = 0;
                    #ifdef HOLD_BUTTON
                    memcpy(&pre_contact, &original_reports.contacts[0], sizeof(contact_data_t));
                    #endif
                    #ifdef DOUBLE_CLICK_TO_RIGHT_BUTTON
                    left2right_button_flag = 0;
                }
                #endif
                _touch_button_repeat_flag = 1;
            }
            else if(button_st==2||button_st==16) // 2: L-button release, 16: R-button release
            {
                ptp_reports.button=0;
                ptp_reports.button1=0;
                _touch_button_repeat_flag = 0;
                // 按钮松开时，清空按下的位置
                #ifdef HOLD_BUTTON
                memset(&pre_contact, 0, sizeof(contact_data_t));
                #endif
            }
            _touch_button_repeat_count =0;
        }
    }
    #ifdef HOLD_BUTTON
    if (ptp_reports.button==1 && cnt == 1 && pre_contact.tip == 1 && is_contin_press == 0){
        // 判断单指按压按钮后是否移动
        if (original_reports.contacts[0].tip == 1 && original_reports.contacts[0].contact_id == pre_contact.contact_id){
            if (abs(original_reports.contacts[0].x - pre_contact.x) > 50 || abs(original_reports.contacts[0].y - pre_contact.y) > 25){
                is_contin_press = 1;
            }
        }
    }
    if (is_contin_press == 1){
        // 持续保持按钮状态
        ptp_reports.button= 1;
    }
    #endif
    if (Pre_Tip[0] == 0 && Pre_Tip[1] == 0 &&Pre_Tip[2] == 0 &&Pre_Tip[3] == 0&&Pre_Tip[4] == 0&&Pre_Tip[5] == 0){
        TOUCHPAD_log("All fingers no touch");
        ptp_reports.button=0;
        ptp_reports.button1=0;
        motion_btn = 0;
        _touch_button_repeat_flag = 0;
        #ifdef HOLD_BUTTON
        // 所有手指离开时 释放按钮
        is_contin_press = 0;
        #endif
        touch_out_flag |= 0x80;
    }
    if (original_reports.contact_count == 0 && original_reports.contacts[0].tip == 0 && original_reports.contacts[0].confidence == 0){
        TOUCHPAD_log("All fingers leave");
        #ifdef HOLD_BUTTON
        memset(&pre_contact, 0, sizeof(contact_data_t));
        #endif
        start_touch_flag = 0;
    }
    // 用于判断是否存在不符合的值 如果是andriod 需要设置tip才生效。
    if (has_no_confidence() > 0){
        memset(&Pre_Confidence,0,sizeof(Pre_Confidence));
        ptp_reports.contact_rpt[0].confidence = 0;
        ptp_reports.contact_rpt[1].confidence = 0;
        ptp_reports.contact_rpt[2].confidence = 0;
        ptp_reports.contact_rpt[3].confidence = 0;
    }

    #ifdef DOUBLE_CLICK_TO_RIGHT_BUTTON
    // 当双指触发了右键时，需要发一次左键，防止卡死。
    if (left2right_button_flag == 1 && (ptp_reports.button == 0 && ptp_reports.button1 == 0))
    {
        ptp_reports.button = 1;
        motion_btn = 1;
        if (touch_out_flag == 0x81){
            // 手指已经离开，需要定时发送抬起
            auto_release_right_button();
            left2right_button_flag = 0;
            // 同时需要保持当前 tip
            for (u8tmp = 0; u8tmp < TP_MAX_CONTACT_COUNT; u8tmp++) {
                if (ptp_reports.contact_rpt[u8tmp].confidence == 1 || (ptp_reports.contact_rpt[u8tmp].x_l8 >0 || ptp_reports.contact_rpt[u8tmp].x_m4 >0)){
                    ptp_reports.contact_rpt[u8tmp].tip = 1;
                }
            }
        } else {
            left2right_button_flag = 2;
        }
    }
    else if (left2right_button_flag == 2)
    {
        left2right_button_flag = 0;
        ptp_reports.button = 0;
        motion_btn = 1;
    }
    #endif

    return 1;
}

uint8_t ProcessPTPData(void){
    // 获取触控数据
    ProcessGetPTPData();

    pct1336_clear_int();
    if(motion[0] ||motion[1] || motion[2] ||motion[3]||(ptp_reports.button==1)||(ptp_reports.button1==1)||(motion_btn ==1))
    {
        scan_time += 100;
        ptp_reports.scantime_l8= (uint8_t)(scan_time&0x00ff);
        ptp_reports.scantime_m8= (uint8_t)((scan_time&0xff00)>>8);
        if (kb_break_cnt > 0 && motion_btn == 0){
            ptp_reports.contact_rpt[0].confidence = 0;
            ptp_reports.contact_rpt[1].confidence = 0;
            ptp_reports.contact_rpt[2].confidence = 0;
            ptp_reports.contact_rpt[3].confidence = 0;
        }
        send_touch_data();
        #ifdef MCS_GESTURE
        if (ptp_reports.contactCnt == 1 && touch_gesture_switch_bitmap > 0){
            if (scan_time % 200 == 0){
                touch_arr[touch_idx].x = (double)(( (uint16_t)(ptp_reports.contact_rpt[0].x_m4 & 0x0F) << 8 ) | (ptp_reports.contact_rpt[0].x_l8 & 0xFF));
                touch_arr[touch_idx].y = (double)(
                    ( (uint16_t)(ptp_reports.contact_rpt[0].y_m8 & 0xFF) << 4 ) |
                    (ptp_reports.contact_rpt[0].y_l4 & 0x0F)
                );
                if (keycode_type == IOS){
                    touch_arr[touch_idx].y = TOUCHPAD_MAX_Y - touch_arr[touch_idx].y;
                }
                touch_idx++;
                if (touch_idx > MAX_FRAMES){
                    touch_idx = 0;
                }
            }
        } else {
            touch_idx = 0;
        }
        #endif
    }
    // 判断为接触后离开
    if (touch_out_flag == 0x81 && motion_btn == 0){
        TOUCHPAD_log("Touch out");
        scan_time = 0;
        #ifdef BAYES_MISTOUCH
        memset(mistouch_datas, 0, sizeof(mistouch_datas));
        #endif
        #ifdef MCS_GESTURE
        gesture_judgment();
        touch_idx = 0;
        #endif
    }
    return 1;
}
void clear_touch_data_event(void){
    start_touch_flag = 0;
}

void set_touch_data_event(void){
    start_touch_flag = 1;
}
int8_t touch_task(void){
    int8_t ret = 0;
    int_low_cnt = 0;
    if(touch_mode == TOUCH_MODE_PTP)
    {
        ret = ProcessPTPData();
    } else if(touch_mode == TOUCH_MODE_MOUSE )
    {
        ret = ProcessMouseData();
        pct1336_clear_int();
    }  else {
        ret = 0;
    }
    if (ret > 0){
        set_touch_data_event();
    }
    return ret;
}

/// @brief 触控板中断回调函数
__HIGH_CODE
void _touch_cb(pin_t pin)
{
    (void)pin;  /* 未使用，保留参数以匹配回调签名 */
    if (touch_en == 1) {
        _touch_int_flag++;
    }
}



/// @brief 触摸板gpio口初始化
void touch_gpio_init(void){
    // 配置中断引脚为上拉输入
    gpio_set_pin_input_high(TOUCHPAD_INT);

    // 配置 I2C 引脚
    gpio_set_pin_input_high(TOUCHPAD_SCL);
    gpio_set_pin_input_high(TOUCHPAD_SDA);
    // 初始化 I2C
    i2c_init();
    TOUCHPAD_log("Touch GPIO init \r\n");
    int16_t status = i2c_init_channel_with_pins(I2C_CHANNEL_0, TOUCHPAD_SDA, TOUCHPAD_SCL, 400000);
    TOUCHPAD_log("i2c init status: %d\r\n", status);
    // 启用中断
    gpio_enable_interrupt(TOUCHPAD_INT, GPIO_INT_LOW_LEVEL, _touch_cb);
}

/// @brief 触摸板 物理断电
/// 注意此处 如果有单独GPIO 控制触摸板供电，需要将供电切断
void touch_gpio_uninit(void){
    // 禁用中断
    gpio_disable_interrupt(TOUCHPAD_INT);

    // 配置引脚为上拉输入（低功耗状态）
    gpio_set_pin_input_high(TOUCHPAD_SDA);
    gpio_set_pin_input_high(TOUCHPAD_SCL);
    gpio_set_pin_input_high(TOUCHPAD_INT);
}



// @brief 触摸板初始化
void touch_Init(void)
{  
    int8_t result;

    result = pct1336_init();
    
    if (result == 1)
    {
        touch_en = 1;
        TOUCHPAD_log("touch_Init success\r\n");
    }
    else
    {
        touch_en = 0;
        TOUCHPAD_log("touch_Init fail\r\n");
    }
    
    // 可选：释放参数内存（如果参数是动态分配的）
    // init_params = NULL;
    // init_params_len = 0;
}

/*********************************************************************
 * PROFILE CALLBACKS    回调
 */


/*********************************************************************
 * PUBLIC FUNCTIONS 公开的函数
 */
int8_t touch_power_on_with_params(pct1336_params_t* params, uint8_t len)
{
    // 保存参数
    pct1336_set_init_params(params, len);

    touch_gpio_init();
    touch_Init();

    return (touch_en == 1) ? 1 : 0;
}
int8_t touch_power_on(void){
    return touch_power_on_with_params(NULL, 0);
}

int8_t touch_power_off(void){
    TOUCHPAD_log("Touch power off");
    pct1336_sleep();
    touch_en = 0;
    wait_us(200);
    touch_gpio_uninit();
     // 触摸板关机后处理 中断标志，防止关机后由于标志位继续访问IIC
    if(_touch_int_flag > 0){
        _touch_int_flag = 0;
    }
    return 1;
}

void set_touch_mode(touch_mode_t mode){
    // 参数有效性检查
    switch (mode) {
        case TOUCH_MODE_MOUSE:
        case TOUCH_MODE_PTP:
            touch_mode = mode;
            break;
        default:
            // 记录错误日志并设置默认模式
            TOUCHPAD_log("Invalid touch mode: %d, setting to default MOUSE mode", mode);
            touch_mode = TOUCH_MODE_MOUSE;
            break;
    }
}

void set_kb_break_cnt(uint16_t cnt)
{
    kb_break_cnt = cnt;
}

uint16_t get_touch_scan_time(void){
    return scan_time;
}

/// @brief 停止触摸板数据发送
/// @return 1：有数据 0：无数据
uint8_t end_touchpad_input(void){
    uint8_t u8tmp,status = 0;
    for (u8tmp = 0; u8tmp < TP_MAX_CONTACT_COUNT; u8tmp++) {
        if (ptp_reports.contact_rpt[u8tmp].tip > 0){
            status = 1;
            ptp_reports.contact_rpt[u8tmp].tip = 0;
        } 
        // else {
        //     ptp_reports.contact_rpt[u8tmp].tip = 0;
        //     ptp_reports.contact_rpt[u8tmp].confidence= 0;
        //     ptp_reports.contact_rpt[u8tmp].contact_id= 0;
        //     ptp_reports.contact_rpt[u8tmp].x_l8= 0;
        //     ptp_reports.contact_rpt[u8tmp].x_m4 = 0;
        //     ptp_reports.contact_rpt[u8tmp].y_l4= 0;
        //     ptp_reports.contact_rpt[u8tmp].y_m8= 0;
        // }
    }

    if (ptp_reports.button == 1 ){
        status = 1;
        ptp_reports.button = 0;
        ptp_reports.button1 = 0;
    }
    if (status == 1){
        scan_time+=100;
        ptp_reports.scantime_l8= (uint8_t)(scan_time&0x00ff);
        ptp_reports.scantime_m8= (uint8_t)((scan_time&0xff00)>>8);
        // DEBUG_log("Manual touch end\r\n");
        send_touch_data();
        memset(&Pre_Tip,0,sizeof(Pre_Tip));
        memset(&Pre_Confidence,0,sizeof(Pre_Confidence));
    } else {
        scan_time = 0;
    }

    // data_status = 0;
    if(_touch_int_flag > 0){
        _touch_int_flag = 0;
        _touch_button_repeat_count = 0;
        pct1336_clear_int();
    }
    return status;
}

int8_t touch_timer_task(void){
    int8_t touch_st = 0;
    if (kb_break_cnt > 0){
        kb_break_cnt --;
    }
    if (touch_en != 1){
        return 0;
    }
    touch_st = (_touch_int_flag > 0) ? 1 : 0;
    if(_touch_int_flag > 0)
    {
        start_touch_flag = 1;
    }
    else
    {
        // 开机状态下连续5次中断脚为低电平，认为中断脚异常
        if (gpio_read_pin(TOUCHPAD_INT)){
            int_low_cnt ++;
            if (int_low_cnt > 14 && _touch_int_flag == 0 )
            {
                TOUCHPAD_log("Touch INT error");
                int_low_cnt = 0;
                _touch_int_flag ++;
            }
        }
        if(_touch_button_repeat_flag == 1)
        {
            if(++_touch_button_repeat_count == 3)
            {
                _touch_button_repeat_count = 0;

                scan_time+=120;
                ptp_reports.scantime_l8= (uint8_t)(scan_time&0x00ff);
                ptp_reports.scantime_m8= (uint8_t)((scan_time&0xff00)>>8);
                ptp_reports.button = 1;
                memcpy(&ptp_reports, &ptp_reports, sizeof(hid_ptp_report_t));
                send_touch_data();
                TOUCHPAD_log("_touch_button_repeat \n");
            }
            touch_st = 1;
        }

        // if (left2right_button_flag == 2){
        //     if(++_touch_button_repeat_count == 6)
        //     {
        //         _touch_button_repeat_count = 0;
        //         _touch_int_flag ++;
        //         TOUCHPAD_log("left2right_button_flag ---------- \n");
        //     }
        // }
    }
    touch_st =  scan_time > 0 ? 1 : touch_st;
    return touch_st;
}

void touch_evt_task(void){
    if (touch_en != 1 && start_touch_flag != 1){
        return ;
    }
    // 当有键盘中断时，降低触摸板的处理频率
    // if (kb_break_cnt > 0){
    //     return ;
    // }
    if(_touch_int_flag > 0)
    {
        ATOMIC_BLOCK_FORCEON {
            if (_touch_int_flag > 0) {
                _touch_int_flag--;
            }
        }
        _touch_button_repeat_count = 0;
        touch_task();
    }
}


bool touch_watchdog_check(void){
    if (touch_en != 1 ){
        return false;
    }
    return pct1336_watchdog_check();
}



/*********************************************************************
*********************************************************************/
