#include "indicator_test.h"
#include "gpio.h"
#ifdef INDICATOR_TEST_ENABLE

#include <stdint.h>

#include "event_manager.h"
#include "indicator.h"
#include "kb904/config.h"

typedef enum {
    TEST_PHASE_WHITE_ON = 0,
    TEST_PHASE_WHITE_OFF,
    TEST_PHASE_RED_ON_2S,
    TEST_PHASE_WHITE_BLINK_SLOW,
    TEST_PHASE_RED_BLINK_FAST,
    TEST_PHASE_WHITE_BLINK_3,
    TEST_PHASE_RED_DELAY_ON,
    TEST_PHASE_ALL_OFF,
    TEST_PHASE_COUNT
} ind_test_phase_t;

#define IND_TEST_EVT              ((uint16_t)0x0001)
#define IND_TEST_INVALID_TASK_ID  ((uint8_t)0xFF)
#define IND_TEST_PHASE_DURATION_MS 3000U

static uint8_t ind_test_task_id = IND_TEST_INVALID_TASK_ID;
static ind_test_phase_t ind_test_phase = TEST_PHASE_WHITE_ON;

static void indicator_test_schedule_next(void) {
    if (ind_test_task_id == IND_TEST_INVALID_TASK_ID) {
        return;
    }

    if (((uint8_t)ind_test_phase + 1U) < (uint8_t)TEST_PHASE_COUNT) {
        OSAL_SetDelayedEvent(ind_test_task_id, IND_TEST_EVT, IND_TEST_PHASE_DURATION_MS);
    }
}

static void indicator_test_apply_phase(ind_test_phase_t phase) {
    indicator_off_all();
    togglePin(B13);
    switch (phase) {
        case TEST_PHASE_WHITE_ON:
            indicator_set(LED_BT, &IND_ON);
            break;

        case TEST_PHASE_WHITE_OFF:
            indicator_off(LED_BT);
            break;

        case TEST_PHASE_RED_ON_2S:
            indicator_set(LED_POWER_RED, &IND_ON_2S);
            break;

        case TEST_PHASE_WHITE_BLINK_SLOW:
            indicator_set(LED_BT, &IND_BLINK_SLOW);
            break;

        case TEST_PHASE_RED_BLINK_FAST:
            indicator_set(LED_POWER_RED, &IND_BLINK_FAST);
            break;

        case TEST_PHASE_WHITE_BLINK_3:
            indicator_set(LED_BT, &IND_BLINK_3);
            break;

        case TEST_PHASE_RED_DELAY_ON:
            indicator_set(LED_POWER_RED, &IND_DELAY_ON);
            break;

        case TEST_PHASE_ALL_OFF:
            indicator_off_all();
            break;

        default:
            break;
    }
}

static uint16_t indicator_test_process_event(uint8_t task_id, uint16_t events) {
    (void)task_id;

    if (events & IND_TEST_EVT) {
        if (((uint8_t)ind_test_phase + 1U) < (uint8_t)TEST_PHASE_COUNT) {
            ind_test_phase = (ind_test_phase_t)((uint8_t)ind_test_phase + 1U);
            indicator_test_apply_phase(ind_test_phase);
            indicator_test_schedule_next();
        }

        return (events ^ IND_TEST_EVT);
    }

    return 0;
}

void indicator_test(void) {
    if (ind_test_task_id != IND_TEST_INVALID_TASK_ID) {
        return;
    }

    ind_test_task_id = OSAL_ProcessEventRegister(indicator_test_process_event);
    if (ind_test_task_id == IND_TEST_INVALID_TASK_ID) {
        return;
    }

    ind_test_phase = TEST_PHASE_WHITE_ON;
    indicator_test_apply_phase(ind_test_phase);
    indicator_test_schedule_next();
}

#endif
