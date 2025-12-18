//
// Created by 18494 on 2025/12/18.
//
#include "output_service.h"
#include <stdint.h>
#include "event_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t output_taskID=0;
uint16_t output_process_event(uint8_t task_id, uint16_t events){

    return 0;
}

void output_service_init(void) {
    output_taskID = OSAL_ProcessEventRegister(output_process_event);

}
#ifdef __cplusplus
}
#endif

