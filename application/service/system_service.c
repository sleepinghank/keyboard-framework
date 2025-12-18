//
// Created by 18494 on 2025/12/18.
//
#include "output_service.h"
#include <stdint.h>
#include "event_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t system_taskID=0;
uint16_t system_process_event(uint8_t task_id, uint16_t events){

    return 0;
}

void system_service_init(void) {
    system_taskID = OSAL_ProcessEventRegister(system_process_event);
}
#ifdef __cplusplus
}
#endif

