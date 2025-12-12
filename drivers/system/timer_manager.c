#include "timer_manager.h"
#include "timer.h"

#define TAG "platform_tim"

uint32_t System_Time = 0;

// 前向声明MultiTimer结构体
typedef struct MultiTimerHandle MultiTimer;

// 定时器链表头指针
static MultiTimer* timerList = NULL;


// 定时器控制块结构体
struct MultiTimerHandle {
    MultiTimer* next;          // 指向下一个定时器的指针,用于链表
    uint8_t timerId;             // 定时器ID
    uint64_t deadline;         // 定时器到期时间点
    TimerCallback_t callback;  // 定时器回调函数
    void* userData;           // 用户数据指针
};

//定义一个标志位用于表示定时器是否占用
uint8_t timer_used[32] = {0};

MultiTimer software_timer_param[32 - 16];

//获取支持的硬件定时器数量
extern uint8_t port_timer_get_hardware_count(void);

//启动一个定时器
extern error_code_t port_timer_start(uint8_t timerId, TimerMode_e timerMode,uint32_t timing, uint32_t count, TimerCallback_t timerCallback, void* userData);

//停止一个定时器
extern error_code_t port_timer_stop(uint8_t timerId);



/**
 * @brief 从定时器链表中移除指定定时器
 * @param timer 要移除的定时器指针
 */
static void removeTimer(uint8_t timerId) {
    MultiTimer** current = &timerList;
    while (*current) {
        if ((*current)->timerId == timerId) {
            *current = (*current)->next; // 从链表中移除定时器
            break;
        }
        current = &(*current)->next;
    }

}

/**
 * @brief 启动一个定时器
 * @param timer 定时器指针
 * @param timing 定时时长
 * @param callback 定时器回调函数
 * @param userData 用户数据
 * @return 0:成功 -1:失败
 */
int multiTimerStart(uint8_t timerId, uint64_t timing, TimerCallback_t callback, void* userData) {
    if ( !callback ) {
        return -1; // 参数无效则返回错误
    }

    removeTimer(timerId); // 先移除定时器,避免重复添加
    
    uint8_t id = timerId - 16;

    // 计算超时时间点
    software_timer_param[id].deadline = System_Time + timing;
    software_timer_param[id].timerId = timerId;
    software_timer_param[id].callback = callback;
    software_timer_param[id].userData = userData;

    // 按照超时时间升序插入链表
    MultiTimer** current = &timerList;
    while (*current && ((*current)->deadline < software_timer_param[id].deadline)) {
        current = &(*current)->next;
    }
    software_timer_param[id].next = *current;
    *current = &software_timer_param[id];

    return 0;
}

/**
 * @brief 停止一个定时器
 * @param timer 要停止的定时器指针
 * @return 0:成功
 */
int multiTimerStop(uint8_t timerId) {
    MultiTimer** current = &timerList;
    while (*current) {
        if ((*current)->timerId == timerId) {
            *current = (*current)->next; // 从链表中移除定时器
            break;
        }
        current = &(*current)->next;
    }

    return 0;
}

/**
 * @brief 定时器系统主循环函数
 * @return -1:错误 0:无定时器 >0:最近定时器的剩余时间
 */
int multiTimerYield(void) {

    uint64_t currentTicks = System_Time;
    
    // 遍历链表,执行所有已超时的定时器
    while (timerList && (currentTicks >= timerList->deadline)) {
        MultiTimer* timer = timerList;
        timerList = timer->next; // 移除已超时的定时器

        if (timer->callback) {
            timer->callback(timer->timerId, timer->userData); // 执行回调函数
        }
    }
    
    // 返回最近的定时器剩余时间,如果没有定时器则返回0
    return timerList ? (int)(timerList->deadline - currentTicks) : 0;
}








//获取空闲的定时器ID
uint8_t platform_timer_get_free_id(TimerType_e timerType)
{
        /*
    定时器ID 0 ~ 15为硬件定时器，其中0固定分配给软件定时器作为时基
    定时器ID 16 ~ 31为软件定时器
    */
    uint8_t timerId = 0xff;

    if(timerType == TIMER_HARDWARE)//硬件定时器
    {
        uint8_t hardwareCount = 4;//port_timer_get_hardware_count();
        // LOG_INFO(TAG ,"硬件定时器数量:%d", hardwareCount);
        for (uint8_t i = 0; i < hardwareCount; i++)
        {
            // LOG_INFO(TAG ,"硬件定时器ID:%d-used:%d", i, timer_used[i]);
            if(timer_used[i] == 0)
            {
                timerId = i;
                break;
            }
        }
    }
    else if(timerType == TIMER_SOFTWARE)//软件定时器
    {
        for (uint8_t i = 16; i < 31; i++)
        {
            if(timer_used[i] == 0)
            {
                timerId = i;
                break;
            }
        }
    }

    return timerId;
}


//启动一个定时器
error_code_t platform_timer_start(uint8_t timerId,  TimerMode_e timerMode, uint32_t timing, uint32_t count, TimerCallback_t timerCallback, void* userData)
{

    if(timerId >= 16 && timerId <= 31)//软件定时器
    {
        // LOG_INFO(TAG ,"启动软件定时器ID:%d", timerId);
        timer_used[timerId] = 1;
//        port_timer_start(timerId, timerMode, timing, count, timerCallback, userData);
    }
    else if(timerId >= 0 && timerId <= 15)//硬件定时器
    {
        // LOG_INFO(TAG ,"启动硬件定时器ID:%d", timerId);
        timer_used[timerId] = 1;
//        port_timer_start(timerId, timerMode, timing, count, timerCallback, userData);
    }

    return NO_ERROR;
}


//停止一个定时器
error_code_t platform_timer_stop(uint8_t timerId)
{
    if(timerId >= 16 && timerId <= 31)//软件定时器
    {
        timer_used[timerId] = 0;
        multiTimerStop(timerId);
    }
    else if(timerId >= 0 && timerId <= 15)//硬件定时器
    {
        timer_used[timerId] = 0;
//        port_timer_stop(timerId);
    }
    return NO_ERROR;
}

//定时器主循环函数
error_code_t platform_timer_yield(void)
{
    multiTimerYield();
    return NO_ERROR;
}
