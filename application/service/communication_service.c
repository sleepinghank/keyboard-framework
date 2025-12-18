#include "input_service.h"
#include "keyboard.h"
#include <stdint.h>
#include "event_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t commu_taskID=0;
uint16_t commu_process_event(uint8_t task_id, uint16_t events){
    
    return 0;
}

void commu_service_init(void) {
    commu_taskID = OSAL_ProcessEventRegister(commu_process_event);

    
}
#ifdef __cplusplus
}
#endif

