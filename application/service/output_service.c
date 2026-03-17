#include "output_service.h"
#include <stdint.h>
#include "event_manager.h"
#include "debug.h"
#include "CH585SFR.h"
#include "report_buffer.h"
#include "transport.h"
#include "indicator.h"
#include "indicator_config.h"
#include "lpm.h"
#include "system_service.h"

#ifdef P2P4G_ENABLE_FLAG
#include "p24g_driver.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

uint8_t output_taskID = 0;

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

static const ind_req_map_t ind_req_map[] = {
    { IND_REQ_BT_PAIRING,        LED_WHITE, &IND_BLINK_SLOW },
    { IND_REQ_BT_RECONNECTING,   LED_WHITE, &IND_BLINK_FAST },
    { IND_REQ_BT_CONNECTED,      LED_WHITE, &IND_ON         },
    { IND_REQ_BT_DISCONNECTED,   LED_WHITE, &IND_OFF        },
    { IND_REQ_CAPS_ON,           LED_WHITE, &IND_ON         },
    { IND_REQ_CAPS_OFF,          LED_WHITE, &IND_OFF        },
    { IND_REQ_CAPS_DISCONNECTED, LED_WHITE, &IND_BLINK_SLOW },
    { IND_REQ_LOW_BATTERY,       LED_RED,   &IND_BLINK_FAST },
    { IND_REQ_BATTERY_NORMAL,    LED_RED,   &IND_OFF        },
};

void output_service_request_indicator(ind_req_type_t type, uint8_t param) {
    (void)param;

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

    /*========================================
     * LPM prepare/resume 事件处理
     *========================================*/

    // 处理 LPM prepare 事件（仅 Deep 模式需要关灯）
    if (events & OUTPUT_LPM_PREPARE_EVT) {
        lpm_mode_t mode = lpm_get_mode();
        dprintf("Output: LPM prepare start (mode=%d)\r\n", mode);

        if (mode == LPM_MODE_DEEP) {
            /* Deep：关闭背光和指示灯 */
            indicator_off_all();
#ifdef RGB_MATRIX_ENABLE
            // rgb_matrix_disable_noeeprom();
#endif
#ifdef LED_MATRIX_ENABLE
            // led_matrix_disable_noeeprom();
#endif
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
        for (uint8_t i = 0; i < IND_LED_COUNT; i++) {
            if (ind_pending[i].effect != NULL) {
                indicator_set(i, ind_pending[i].effect);
            }
        }

#ifdef RGB_MATRIX_ENABLE
        // rgb_matrix_enable_noeeprom();
#endif
#ifdef LED_MATRIX_ENABLE
        // led_matrix_enable_noeeprom();
#endif

        dprintf("Output: Resume LEDs after deep wake\r\n");
        return (events ^ OUTPUT_LPM_RESUME_EVT);
    }

    return 0;
}

void output_service_init(void) {
    PRINT("Output service init start\r\n");

    output_taskID = OSAL_ProcessEventRegister(output_process_event);

    PRINT("Task registered, ID=%d\r\n", output_taskID);
    PRINT("Output service init done\r\n");
}

#ifdef __cplusplus
}
#endif
