#include "touchpad.h"
#include "kb904/config_product.h"
#include "gpio.h"
#include "PMU.h"
#if (TOUCHPAD_ENABLE == TRUE)
#include "bayes_filtering.h"
#include "touchpad_service.h"
#include "pct1336_driver.h"
#include "event_manager.h"
#include "debug.h"
#include "input_service.h"

#include "i2c_master.h"

#define TOUCHPAD_INVALID_TASK_ID      ((uint8_t)0xFF)
#define TOUCHPAD_REG_INIT_DELAY_MS    16*15U // 触控寄存器初始化重试间隔，单位毫秒
#define TOUCHPAD_REG_INIT_MAX_COUNT    50U // 触控寄存器初始化最大重试次数，50次约等于750ms的重试窗口


uint8_t touchpad_taskID = TOUCHPAD_INVALID_TASK_ID;

static pct1336_params_t params[] = {
    {.params.bank = 0, .params.address = 0x12, .params.data = 0x43},
    {.params.bank = 0, .params.address = 0x13, .params.data = 0x00},
    {.params.bank = 0, .params.address = 0x14, .params.data = 0x43},
    // {.params.bank = 2, .params.address = 0x00, .params.data = TOUCHPAD_MAX_X & 0xff},
    // {.params.bank = 2, .params.address = 0x01, .params.data = (TOUCHPAD_MAX_X >> 8) & 0xff},
    // {.params.bank = 2, .params.address = 0x02, .params.data = TOUCHPAD_MAX_Y & 0xff},
    // {.params.bank = 2, .params.address = 0x03, .params.data = (TOUCHPAD_MAX_Y >> 8) & 0xff},
};

// bayes_model_t KB04122_model = {
//     .priors = {0.3161764705882353, 0.6838235294117647},
//     .sigmas = {{26359.022994870033, 80.25122874664841}, {102082.63132022231, 27.03040063050838}},
//     .thetas = {{1312.4051878354203, 17.615384615384617}, {471.33664185277087, 10.91852770885029}}
// };

bayes_model_t KB04122_13A_model = {
    .priors = {0.3161764705882353, 0.6838235294117647},
    .sigmas = {{30550.249809593406, 80.25126568202222}, {118388.56510415889, 27.030437565882206}},
    .thetas = {{1413.4525939177101, 17.615384615384617}, {507.6232423490488, 10.91852770885029}}
};



static uint16_t touchpad_process_event(uint8_t task_id, uint16_t events);



/* 注册独立 taskID，供 input_service 只做事件转发。 */
void touchpad_setup(void)
{
    if (touchpad_taskID != TOUCHPAD_INVALID_TASK_ID) {
        return;
    }

    touchpad_taskID = OSAL_ProcessEventRegister(touchpad_process_event);
    if (touchpad_taskID == TOUCHPAD_INVALID_TASK_ID) {
        LOG_E("[TP] task registration failed");
    }
}

/* 上电路径允许阻塞 I2C 操作，符合触控初始化的时序约束。 */
void touchpad_power_on(void)
{
    if (touchpad_taskID == TOUCHPAD_INVALID_TASK_ID) {
        touchpad_setup();
    }

    if (touchpad_taskID == TOUCHPAD_INVALID_TASK_ID) {
        return;
    }
    // 触控板上电 ,下拉输入
    setPinInputLow(TOUCHPAD_POWER_PIN);
    // 触控板按键,高有效
    setPinOutput(TOUCHPAD_BUTTON_PIN);
    writePinLow(TOUCHPAD_BUTTON_PIN);

    bayes_mode_init(&KB04122_13A_model);
    
    if ((sizeof(params) / sizeof(params[0])) > 0) {

        if (touch_power_on_with_params(params,sizeof(params) / sizeof(params[0])) != 1) {
            touch_en = 0;
            LOG_E("[TP] power on failed");
            return;
        }
        (void)OSAL_StartReloadTask(touchpad_taskID, TOUCHPAD_REG_INIT_EVT,TOUCHPAD_REG_INIT_DELAY_MS);
    } else {
        if (touch_power_on() != 1) {
            touch_en = 0;
            LOG_E("[TP] power on failed");
            return;
        }
        touch_en = 1;
    }
}

/* 关机时同步清理 middleware 状态，避免旧定时事件残留。 */
void touchpad_power_off(void)
{
    (void)touch_power_off();
    setPinInputHigh(TOUCHPAD_POWER_PIN);
    setPinInputLow(TOUCHPAD_INT);
    setPinInputLow(TOUCHPAD_SDA);
    setPinInputLow(TOUCHPAD_SCL);
    setPinInputLow(TOUCHPAD_BUTTON_PIN);
}

/* 中断回调不直接读 I2C，设置标志位由主循环轮询处理。 */
void touchpad_notify_int(void)
{
    input_set_touchpad_int_flag(true);
}

/* 主循环轮询处理触控板数据，由 peripheral_process 调用。 */
void touchpad_task(void)
{
    if (input_get_touchpad_int_flag()) {
        input_clear_touchpad_int_flag();
        PMU_Update();
        touch_evt_task();
    }
}

void touchpad_set_kb_break(uint16_t ms)
{
    if (touchpad_taskID == TOUCHPAD_INVALID_TASK_ID) {
        return;
    }

    set_kb_break_cnt(1);
    (void)OSAL_SetDelayedEvent(touchpad_taskID, TOUCHPAD_KB_BREAK_EVT, ms);
}

void touchpad_stop_all_events(void)
{
    if (touchpad_taskID == TOUCHPAD_INVALID_TASK_ID) {
        return;
    }

    (void)OSAL_StopTask(touchpad_taskID, TOUCHPAD_DATA_EVT);
    (void)OSAL_StopTask(touchpad_taskID, TOUCHPAD_KB_BREAK_EVT);
}

/* watchdog 只负责探测异常，恢复流程统一回到 middleware 生命周期。 */
void touchpad_watchdog_check(void)
{
    if (touch_en != 1) {
        return;
    }

    if (touch_watchdog_check()) {
        return;
    }

    LOG_W("[TP] watchdog abnormal, reinit");
    touchpad_power_off();
    touchpad_power_on();
}

/* 对外保持无状态封装，模式枚举仍复用 touch_component 定义。 */
void touchpad_set_mode(uint8_t mode)
{
    set_touch_mode((touch_mode_t)mode);
}



/* touchpad middleware 的事件状态机，只负责自身生命周期推进。 */
static uint16_t touchpad_process_event(uint8_t task_id, uint16_t events)
{
    (void)task_id;

    if (events & TOUCHPAD_INIT_EVT) {
        /* 阻塞上电已在 touchpad_power_on() 完成，这里只启动异步轮询。 */
        touchpad_power_on();
        return (events ^ TOUCHPAD_INIT_EVT);
    }

    if (events & TOUCHPAD_DATA_EVT) {
        /* HID 上报路径暂时保持 touch_component 原实现。 */
        /* TODO: align touchpad HID reports with wireless_transport. */
        touch_evt_task();
        return (events ^ TOUCHPAD_DATA_EVT);
    }

    if (events & TOUCHPAD_KB_BREAK_EVT) {
        set_kb_break_cnt(0);
        return (events ^ TOUCHPAD_KB_BREAK_EVT);
    }

    if (events & TOUCHPAD_REG_INIT_EVT) {
        static uint8_t retry_cnt = 0;
        if (pct1336_register_cb()) {
            retry_cnt = 0;
            touch_en = 1;
            touchpad_set_mode(3);
            OSAL_StopTask(touchpad_taskID, TOUCHPAD_REG_INIT_EVT);
        } else {
            if (++retry_cnt > TOUCHPAD_REG_INIT_MAX_COUNT) {
                LOG_E("[TP] register failed after max retries");
                retry_cnt = 0;
                touch_en = 0;
                OSAL_StopTask(touchpad_taskID, TOUCHPAD_REG_INIT_EVT);
            }
        }

        return (events ^ TOUCHPAD_REG_INIT_EVT);
    }
    if (events & TOUCHPAD_OFF_EVT) {
        touchpad_power_off();
        return (events ^ TOUCHPAD_OFF_EVT);
    }
    return 0;
}


#else

void touchpad_setup(void)
{
}

void touchpad_power_on(void)
{
}

void touchpad_power_off(void)
{
    setPinInputHigh(TOUCHPAD_POWER_PIN);
    setPinInputLow(TOUCHPAD_INT);
    setPinInputLow(TOUCHPAD_SDA);
    setPinInputLow(TOUCHPAD_SCL);
    setPinInputLow(TOUCHPAD_BUTTON_PIN);
}

void touchpad_notify_int(void)
{
}

void touchpad_task(void)
{
}

void touchpad_set_kb_break(uint16_t ms)
{
    (void)ms;
}

void touchpad_stop_all_events(void)
{
}

void touchpad_watchdog_check(void)
{
}

void touchpad_set_mode(uint8_t mode)
{
    (void)mode;
}

#endif
