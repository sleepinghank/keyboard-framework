#include "output_service.h"
#include <stdint.h>
#include "event_manager.h"
#include "debug.h"
#include "CH585SFR.h"
#include "report_buffer.h"
#include "transport.h"
#include "indicator.h"

#ifdef P2P4G_ENABLE_FLAG
#include "p24g_driver.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

uint8_t output_taskID = 0;

uint16_t output_process_event(uint8_t task_id, uint16_t events) {
    (void)task_id;

    if (events & OUTPUT_BACKLIGHT_BRIGHTNESS_EVT) {
        // Placeholder: backlight control will be integrated with backlight driver.
        return (events ^ OUTPUT_BACKLIGHT_BRIGHTNESS_EVT);
    }

    return 0;
}

void output_service_init(void) {
    PRINT("Output service init start\r\n");

    output_taskID = OSAL_ProcessEventRegister(output_process_event);

    PRINT("Task registered, ID=%d\r\n", output_taskID);
    PRINT("Output service init done1\r\n");
}

#ifdef __cplusplus
}
#endif
