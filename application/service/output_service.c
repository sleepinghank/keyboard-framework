#include "output_service.h"
#include <stdint.h>
#include "event_manager.h"
#include "debug.h"
#include "CH585SFR.h"
#include "report_buffer.h"
#include "transport.h"
#include "indicator.h"
#include "kb904/config.h"
#include "backlight.h"
#include "lpm.h"
#include "system_service.h"

#ifdef P2P4G_ENABLE_FLAG
#include "p24g_driver.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

uint8_t output_taskID = 0;

#define BACKLIGHT_SLEEP_TIMEOUT_MS 5000UL

typedef struct {
    const ind_effect_t* effect;
    bool                dirty;
} ind_pending_t;

typedef struct {
    ind_req_type_t      type;
    uint8_t             led_id;
    const ind_effect_t* effect;
} ind_req_map_t;

static ind_pending_t ind_pending[IND_LED_COUNT];
static bool          output_service_ready = false;
static bool          backlight_sleeping = false;
static bool          backlight_low_battery = false;
static bool          backlight_lpm_disabled = false;
static bool          backlight_enabled_before_low_battery = true;

static const ind_req_map_t ind_req_map[] = {
#if PRODUCT_ID == 0x0904
    { IND_REQ_BT_PAIRING,        LED_BT,   &IND_BLINK_FAST },
    { IND_REQ_BT_RECONNECTING,   LED_BT,   &IND_BLINK_SLOW },
    { IND_REQ_BT_CONNECTED,      LED_BT,   &IND_OFF        },
    { IND_REQ_BT_DISCONNECTED,   LED_BT,   &IND_OFF        },
#endif
    { IND_REQ_CAPS_ON,           LED_CAPS, &IND_ON         },
    { IND_REQ_CAPS_OFF,          LED_CAPS, &IND_OFF        },
    { IND_REQ_CAPS_DISCONNECTED, LED_CAPS, &IND_BLINK_SLOW },
    { IND_REQ_LOW_BATTERY,       LED_POWER_RED,  &IND_BLINK_SLOW },
    { IND_REQ_BATTERY_NORMAL,    LED_POWER_RED,  &IND_OFF        },
};

/**
 * @brief 启动/重置背光休眠定时器
 *
 * 内部函数，用于启动或重置 5 秒背光休眠定时器。
 * 在以下情况下不启动定时器：
 * - 服务未就绪
 * - 低电量模式下（背光应保持关闭）
 * - LPM 禁用状态下
 */
static void output_service_arm_backlight_sleep_timer(void) {
    if (!output_service_ready || backlight_low_battery || backlight_lpm_disabled) {
        return;
    }

    (void)OSAL_StopTask(output_taskID, OUTPUT_BACKLIGHT_SLEEP_EVT);
    (void)OSAL_SetDelayedEvent(output_taskID, OUTPUT_BACKLIGHT_SLEEP_EVT, BACKLIGHT_SLEEP_TIMEOUT_MS);
}

/**
 * @brief 请求指示灯效果
 *
 * 根据请求类型设置对应 LED 的灯效，并触发指示灯更新事件。
 * 对于低电量相关请求，还会联动控制背光。
 *
 * @param type 指示灯请求类型（蓝牙状态、大写锁定、电量状态等）
 * @param param 预留参数，当前未使用
 */
void output_service_request_indicator(ind_req_type_t type, uint8_t param) {
    (void)param;

    if (!output_service_ready) {
        return;
    }

    // 低电量请求：联动关闭背光
    if (type == IND_REQ_LOW_BATTERY) {
        output_service_set_backlight_low_battery(true);
    } else if (type == IND_REQ_BATTERY_NORMAL) {
        output_service_set_backlight_low_battery(false);
    }

    // 查找并设置对应的指示灯效果
    for (uint8_t i = 0; i < (sizeof(ind_req_map) / sizeof(ind_req_map[0])); i++) {
        if (ind_req_map[i].type == type) {
            uint8_t led_id = ind_req_map[i].led_id;

            ind_pending[led_id].effect = ind_req_map[i].effect;
            ind_pending[led_id].dirty = true;
            OSAL_SetEvent(output_taskID, OUTPUT_INDICATOR_EVT);
            return;
        }
    }
}

/**
 * @brief 通知背光有活动发生
 *
 * 当有按键或触控活动时调用此函数：
 * 1. 如果背光处于休眠状态，则唤醒背光
 * 2. 重置 5 秒休眠定时器
 *
 * 在低电量模式或 LPM 禁用状态下，此函数不执行任何操作。
 *
 * @note 供 keyboard.c 的 keyboard_note_backlight_activity() 包装函数调用
 */
void output_service_note_backlight_activity(void) {
    if (!output_service_ready || backlight_low_battery || backlight_lpm_disabled) {
        return;
    }

    // 如果背光处于休眠状态，则唤醒
    if (backlight_sleeping) {
        backlight_enable();
        backlight_sleeping = false;
    }

    // 背光开启时才启动休眠定时器
    if (backlight_is_enabled()) {
        output_service_arm_backlight_sleep_timer();
    }
}

/**
 * @brief 设置低电量模式
 *
 * 控制背光的低电量联动行为：
 * - 进入低电量时：记忆当前背光状态，关闭背光，取消休眠定时器
 * - 退出低电量时：恢复之前的背光状态，重启活动检测
 *
 * @param enable true=进入低电量模式，false=退出低电量模式
 *
 * @note 使用 backlight_enabled_before_low_battery 实现状态记忆，
 *       确保电量恢复后背光状态正确恢复
 */
void output_service_set_backlight_low_battery(bool enable) {
    if (!output_service_ready) {
        return;
    }

    if (enable) {
        // 幂等检查：已处于低电量模式则跳过
        if (backlight_low_battery) {
            return;
        }

        // 记忆当前背光状态
        backlight_enabled_before_low_battery = backlight_is_enabled();
        backlight_low_battery = true;
        backlight_sleeping = false;

        // 取消休眠定时器
        (void)OSAL_StopTask(output_taskID, OUTPUT_BACKLIGHT_SLEEP_EVT);

        // 如果背光是开启的，则关闭
        if (backlight_enabled_before_low_battery) {
            backlight_disable();
        }
        return;
    }

    // 幂等检查：未处于低电量模式则跳过
    if (!backlight_low_battery) {
        return;
    }

    // 退出低电量模式
    backlight_low_battery = false;

    // 恢复之前的背光状态
    if (backlight_enabled_before_low_battery) {
        backlight_enable();
        output_service_note_backlight_activity();
    }
}

uint16_t output_process_event(uint8_t task_id, uint16_t events) {
    (void)task_id;

    if (events & OUTPUT_INDICATOR_EVT) {
        for (uint8_t i = 0; i < IND_LED_COUNT; i++) {
            if (ind_pending[i].dirty && ind_pending[i].effect != NULL) {
                indicator_set(i, ind_pending[i].effect);
                ind_pending[i].dirty = false;
            }
        }
        return (events ^ OUTPUT_INDICATOR_EVT);
    }

    if (events & OUTPUT_BACKLIGHT_BRIGHTNESS_EVT) {
        // Placeholder: backlight control will be integrated with backlight driver.
        return (events ^ OUTPUT_BACKLIGHT_BRIGHTNESS_EVT);
    }

    if (events & OUTPUT_BACKLIGHT_COLOR_EVT) {
        // Placeholder: backlight color control
        return (events ^ OUTPUT_BACKLIGHT_COLOR_EVT);
    }

    if (events & OUTPUT_SEND_HID_KEYBOARD_EVT) {
        // HID 键盘报文发送
        return (events ^ OUTPUT_SEND_HID_KEYBOARD_EVT);
    }

    if (events & OUTPUT_SEND_MOUSE_REPORT_EVT) {
        // 鼠标报文发送
        return (events ^ OUTPUT_SEND_MOUSE_REPORT_EVT);
    }

    if (events & OUTPUT_SEND_TOUCH_REPORT_EVT) {
        // 触控板报文发送
        return (events ^ OUTPUT_SEND_TOUCH_REPORT_EVT);
    }

    if (events & OUTPUT_BACKLIGHT_SLEEP_EVT) {
        if (!backlight_low_battery && !backlight_lpm_disabled && backlight_is_enabled()) {
            backlight_sleeping = true;
            backlight_disable();
        }
        return (events ^ OUTPUT_BACKLIGHT_SLEEP_EVT);
    }

    /*========================================
     * LPM prepare/resume 事件处理
     *========================================*/

    // 处理 LPM prepare 事件（仅 Deep 模式需要关灯）
    if (events & OUTPUT_LPM_PREPARE_EVT) {
        lpm_mode_t mode = lpm_get_mode();
        dprintf("Output: LPM prepare start (mode=%d)\r\n", mode);

        if (mode == LPM_MODE_DEEP) {
            /* Deep：关闭背光和指示灯 */
            backlight_lpm_disabled = backlight_is_enabled();
            if (backlight_lpm_disabled) {
                backlight_disable();
            }
            (void)OSAL_StopTask(output_taskID, OUTPUT_BACKLIGHT_SLEEP_EVT);
            indicator_off_all();
            dprintf("Output: LEDs off for deep sleep\r\n");
        }
        /* Idle：不关灯，直接标记完成 */

        /* 标记 output prepare 完成，通知 system_service 汇聚 */
        lpm_mark_prepare_done(LPM_PREPARE_OUTPUT);
        OSAL_SetEvent(system_taskID, SYSTEM_LPM_STEP_DONE_EVT);

        dprintf("Output: LPM prepare done\r\n");
        return (events ^ OUTPUT_LPM_PREPARE_EVT);
    }

    // 处理 LPM resume 事件（Deep 唤醒后恢复灯效）
    if (events & OUTPUT_LPM_RESUME_EVT) {
        dprintf("Output: LPM resume start\r\n");

        if (backlight_lpm_disabled) {
            backlight_lpm_disabled = false;
            if (!backlight_low_battery) {
                backlight_enable();
                output_service_note_backlight_activity();
            }
        }

        for (uint8_t i = 0; i < IND_LED_COUNT; i++) {
            if (ind_pending[i].effect != NULL) {
                indicator_set(i, ind_pending[i].effect);
            }
        }

        dprintf("Output: Resume LEDs after deep wake\r\n");
        return (events ^ OUTPUT_LPM_RESUME_EVT);
    }

    return 0;
}

void output_service_init(void) {
    PRINT("Output service init start\r\n");

    output_taskID = OSAL_ProcessEventRegister(output_process_event);
    // backlight_init(NULL);  // 已移至 system_init_drivers() - 驱动层初始化应在驱动层完成
    backlight_set_preset_color(BL_COLOR_WHITE);
    backlight_set_preset_level(BL_LEVEL_MEDIUM);
    output_service_ready = (output_taskID != 0xFF);
    backlight_sleeping = false;
    backlight_low_battery = false;
    backlight_lpm_disabled = false;
    backlight_enabled_before_low_battery = backlight_is_enabled();
    if (output_service_ready) {
        output_service_note_backlight_activity();
    } else {
        PRINT("Output service task register failed\r\n");
    }

    PRINT("Task registered, ID=%d\r\n", output_taskID);
    PRINT("Output service init done\r\n");
}

#ifdef __cplusplus
}
#endif
