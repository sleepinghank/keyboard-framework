// middleware/keyboard/combo/kb_combo_engine.h
#ifndef C_PROCESS_COMBO_H
#define C_PROCESS_COMBO_H

#include <stdint.h>
#include <stdbool.h>
#include "keycode.h"

// 平台无关宏定义
#define PROGMEM
#define pgm_read_word(address_short) *((uint16_t*)(address_short))

// 时间配置（单位：ms）
#define SHORT_MS  300
#define LONG_MS   2500

// 组合键结束标志
#define COMBO_END 0

// 按键更新状态
typedef enum {
    NO_KEY_UPDATE = 0,
    KEY_UPDATE,
    GHOST_KEY
} key_update_st_t;

// 回调函数类型
typedef uint8_t (*BtnCallback)(uint16_t* add_keys);

// 按键事件种类
typedef enum {
    PRESS_DOWN = 0,         // 按下
    PRESS_UP,               // 释放
    PRESS_REPEAT,           // 重复
    SINGLE_CLICK,           // 单击
    DOUBLE_CLICK,           // 双击
    LONG_PRESS_START,       // 长按开始
    LONG_PRESS_HOLD,        // 长按保持
    number_of_event,        // 事件数量
    NONE_PRESS              // 无事件
} PressEvent;
// 组合键对象
typedef struct combo_t {
    const uint16_t *keys; // 记录组合键 按键
    bool  disabled; // 记录是否禁用
    uint32_t press_time; // 按下时刻绝对时间戳，单位 ms（timer_read32 返回值）
    uint8_t  system_type;   // 记录系统类型：每个bit代表一个系统，BIT(TYPE)。0xff为所有系统
    uint8_t  repeat : 4; // 记录重复次数
    uint8_t  event : 4; // 记录激活事件
    uint8_t  state : 4; // 记录触发的事件
    uint8_t  f1_flag:1; // 记录是否为F1
    uint8_t  fn_combo : 1; // 记录是否为FN组合键
    uint8_t  active_status : 1; // 判断组合键是否激活
    uint8_t  button_level : 1; // 记录当前电平
    uint16_t long_press_ms; // 长按时间阈值，单位 ms
    BtnCallback cb[number_of_event]; // 回调函数
} combo_t;

#define COMBO(ck, event_idx,callback) { .keys = &(ck)[0], .cb[event_idx] = (callback) ,\
    .long_press_ms = LONG_MS,.event=(uint8_t)NONE_PRESS,.state = 0,.f1_flag=0,.system_type = 0xff}

#define COMBO2(ck, event_idx1,callback1, event_idx2,callback2) \
    { .keys = &(ck)[0], .cb[event_idx1] = (callback1),.cb[event_idx2] = (callback2),\
    .long_press_ms = LONG_MS,.event=(uint8_t)NONE_PRESS,.state = 0,.f1_flag=0,.system_type = 0xff}

#define COMBO3(ck, event_idx1,callback1, event_idx2,callback2, event_idx3,callback3) \
    { .keys = &(ck)[0], .cb[event_idx1] = (callback1),.cb[event_idx2] = (callback2),.cb[event_idx3] = (callback3) ,\
    .event=(uint8_t)NONE_PRESS,.long_press_ms = LONG_MS,.state = 0,.f1_flag=0,.system_type = 0xff}

#define COMBO_LONG_MS(ck,long_ms, event_idx,callback) \
    { .keys = &(ck)[0], .cb[event_idx] = (callback) ,.event=(uint8_t)NONE_PRESS,\
    .long_press_ms = (uint16_t)(long_ms),.state = 0,.f1_flag=0,.system_type = 0xff}

#define COMBO2_LONG_MS(ck,long_ms, event_idx1,callback1, event_idx2,callback2) \
    { .keys = &(ck)[0], .cb[event_idx1] = (callback1),.cb[event_idx2] = (callback2),.event=(uint8_t)NONE_PRESS,\
    .long_press_ms = (uint16_t)(long_ms),.state = 0,.f1_flag=0,.system_type = 0xff}

#define COMBO3_LONG_MS(ck,long_ms, event_idx1,callback1, event_idx2,callback2, event_idx3,callback3) \
    { .keys = &(ck)[0], .cb[event_idx1] = (callback1),.cb[event_idx2] = (callback2),.cb[event_idx3] = (callback3) ,\
    .event=(uint8_t)NONE_PRESS,.long_press_ms = (uint16_t)(long_ms),.state = 0,.f1_flag=0,.system_type = 0xff}

#define COMBO_ALL(ck,long_ms, event_idx,callback,f1_key,sys_type) { .keys = &(ck)[0], .cb[event_idx] = (callback) ,\
    .long_press_ms = (uint16_t)(long_ms),.event=(uint8_t)NONE_PRESS,.state = 0,.f1_flag=f1_key,.system_type = sys_type}

#define COMBO2_ALL(ck,long_ms, event_idx1,callback1,event_idx2,callback2,f1_key,sys_type) { .keys = &(ck)[0], .cb[event_idx1] = (callback1),.cb[event_idx2] = (callback2),.event=(uint8_t)NONE_PRESS,\
    .long_press_ms = (uint16_t)(long_ms),.state = 0,.f1_flag=f1_key,.system_type = sys_type}

extern uint8_t active_fn_combo;
extern uint8_t active_event;
/**
  * @brief  处理组合键任务
  */
void combo_init(void);
void combo_task(key_update_st_t _keyUpdateSt);
#ifdef MCS_GESTURE
// 按键状态机处理
void skip_inactive_keys(BtnCallback cb);
#endif

extern uint8_t number_of_combos;
#endif // C_PROCESS_COMBO_H
