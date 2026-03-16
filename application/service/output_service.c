#include "output_service.h"
#include <stdint.h>
#include "event_manager.h"
#include "debug.h"
#include "CH585SFR.h"
#include "report_buffer.h"
#include "transport.h"
#include "indicator.h"
#include "lpm.h"
#include "system_service.h"

#ifdef P2P4G_ENABLE_FLAG
#include "p24g_driver.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

uint8_t output_taskID = 0;

uint16_t output_process_event(uint8_t task_id, uint16_t events) {
    (void)task_id;

    if (events & OUTPUT_INDICATOR_EVT) {
        // 指示灯状态更新
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
        /* Deep 唤醒后：根据当前无线状态恢复指示灯显示 */
        /* 例如：当前未连接则显示"未连接"指示，已连接则显示连接指示 */
        /* 具体指示灯状态由 wireless_state 决定，此处触发一次状态同步 */
        // indicator_update_from_wireless_state();

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
