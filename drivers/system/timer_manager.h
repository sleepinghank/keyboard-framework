#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "sys_config.h"
#include "sys_error.h"

//定义定时器类型枚举
typedef enum{
    TIMER_HARDWARE = 0,//硬件定时器
    TIMER_SOFTWARE = 1 //软件定时器
}TimerType_e;

//定义定时器模式枚举
typedef enum{
    //单次触发模式
    TIMER_MODE_ONESHOT = 0,
    //周期触发模式
    TIMER_MODE_PERIODIC = 1,
    //持续触发模式
    TIMER_MODE_CONTINUOUS = 2
}TimerMode_e;

//定义定时器回调函数类型
typedef void (*TimerCallback_t)(uint8_t timerId,void* userData);

//获取空闲的定时器ID
extern uint8_t platform_timer_get_free_id(TimerType_e timerType);

//启动一个定时器
extern error_code_t platform_timer_start(uint8_t timerId, TimerMode_e timerMode, uint32_t timing, uint32_t count, TimerCallback_t timerCallback, void* userData);

//停止一个定时器
extern error_code_t platform_timer_stop(uint8_t  timerId);

//定时器主循环函数
extern error_code_t platform_timer_yield(void);

#ifdef __cplusplus
}
#endif
