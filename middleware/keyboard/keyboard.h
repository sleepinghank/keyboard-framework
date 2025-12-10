#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "timer.h"


#define ACTION_DEBUG
//#define COMBO_ENABLE

#ifdef __cplusplus
extern "C" {
#endif

// Matrix configuration (from product_config.h)
#ifndef MATRIX_ROWS
#define MATRIX_ROWS 8
#endif

#ifndef MATRIX_COLS
#define MATRIX_COLS 13
#endif

#ifndef MATRIX_HAS_GHOST
#define MATRIX_HAS_GHOST 0
#endif


/* key matrix position */
typedef struct {
    uint8_t col;
    uint8_t row;
} keypos_t;

typedef enum keyevent_type_t { TICK_EVENT = 0, KEY_EVENT = 1, ENCODER_CW_EVENT = 2, ENCODER_CCW_EVENT = 3, COMBO_EVENT = 4 } keyevent_type_t;

/* key event */
typedef struct {
    keypos_t        key;
    uint16_t        time;
    keyevent_type_t type;
    bool            pressed;
} keyevent_t;

/* equivalent test of keypos_t */
#define KEYEQ(keya, keyb) ((keya).row == (keyb).row && (keya).col == (keyb).col)

/* special keypos_t entries */
#define KEYLOC_ENCODER_CW 253
#define KEYLOC_ENCODER_CCW 252

static inline bool IS_NOEVENT(const keyevent_t event) {
    return event.type == TICK_EVENT;
}
static inline bool IS_EVENT(const keyevent_t event) {
    return event.type != TICK_EVENT;
}
static inline bool IS_KEYEVENT(const keyevent_t event) {
    return event.type == KEY_EVENT;
}
static inline bool IS_COMBOEVENT(const keyevent_t event) {
    return event.type == COMBO_EVENT;
}
static inline bool IS_ENCODEREVENT(const keyevent_t event) {
    return event.type == ENCODER_CW_EVENT || event.type == ENCODER_CCW_EVENT;
}

/* Common keypos_t object factory */
#define MAKE_KEYPOS(row_num, col_num) ((keypos_t){.row = (row_num), .col = (col_num)})

/* Common keyevent_t object factory */
#define MAKE_EVENT(row_num, col_num, press, event_type) ((keyevent_t){.key = MAKE_KEYPOS((row_num), (col_num)), .pressed = (press), .time = timer_read(), .type = (event_type)})

/**
 * @brief Constructs a key event for a pressed or released key.
 */
#define MAKE_KEYEVENT(row_num, col_num, press) MAKE_EVENT((row_num), (col_num), (press), KEY_EVENT)

/**
 * @brief Constructs a combo event.
 */
#define MAKE_COMBOEVENT(press) MAKE_EVENT(0, 0, (press), COMBO_EVENT)

/**
 * @brief Constructs a internal tick event that is used to drive the internal QMK state machine.
 */
#define MAKE_TICK_EVENT MAKE_EVENT(0, 0, false, TICK_EVENT)


/**
 * @brief Initialize keyboard system
 */
void keyboard_init(void);

/**
 * @brief Main keyboard task loop
 *
 * This function should be called repeatedly in the main loop
 * It handles matrix scanning, debouncing, key processing, and action execution
 */
void keyboard_task(void);

/**
 * @brief Process key event from matrix
 *
 * @param row Row of the key
 * @param col Column of the key
 * @param pressed true if key is pressed, false if released
 */
void keyboard_process_key(uint8_t row, uint8_t col, bool pressed);

/**
 * @brief Housekeeping task
 *
 * Called on each iteration of the keyboard task loop
 * Can be used for background tasks and maintenance
 */
void housekeeping_task(void);

#ifdef __cplusplus
}
#endif
