#include "input_service.h"
#include "keyboard.h"
#include <stdint.h>
#include "event_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t input_taskID=0;
uint16_t input_process_event(uint8_t task_id, uint16_t events){
    if (events & INPUT_KEYBOARD_SCAN_EVENT) {
        keyboard_task();
        return events ^ INPUT_KEYBOARD_SCAN_EVENT;
    }
    return 0;
}

void input_service_init(void) {
    input_taskID = OSAL_ProcessEventRegister(input_process_event);

    OSAL_StartReloadTask(input_taskID, INPUT_KEYBOARD_SCAN_EVENT, MATRIX_SCAN_TIMER);
}
#ifdef __cplusplus
}
#endif

