#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "action.h"

#ifdef __cplusplus
extern "C" {
#endif

// Tap Dance configuration
#ifndef TAP_DANCE_ENABLE
#define TAP_DANCE_ENABLE 0
#endif

#if TAP_DANCE_ENABLE
typedef struct {
    uint8_t count;              /**< Number of taps */
    uint16_t time;              /**< Time of last tap */
    keypos_t key;               /**< Key position */
    bool interrupted;           /**< Whether tap was interrupted */
} tap_dance_state_t;

// Tap Dance function pointers
typedef void (*tap_dance_fn_t)(uint8_t row, uint8_t col, uint8_t tap_count, uint8_t action);

/**
 * @brief Initialize tap dance
 */
void tap_dance_init(void);

/**
 * @brief Process tap dance for a key
 *
 * @param event Key event
 * @param tap_count Current tap count
 * @param action Action to take based on tap count
 */
void tap_dance_action(keyevent_t event, uint8_t tap_count, uint8_t action);

/**
 * @brief Task function for tap dance (called periodically)
 */
void tap_dance_task(void);

/**
 * @brief Get current tap count for a key
 *
 * @param key Key position
 * @return Current tap count
 */
uint8_t tap_dance_get_taps(keypos_t key);

/**
 * @brief Reset tap count for a key
 *
 * @param key Key position
 */
void tap_dance_reset(keypos_t key);
#endif

// Leader Key configuration
#ifndef LEADER_KEY_ENABLE
#define LEADER_KEY_ENABLE 0
#endif

#if LEADER_KEY_ENABLE
#define LEADER_TIMEOUT 300  /**< Leader key timeout in ms */

// Leader sequence storage
#define LEADER_KEY_MAX_SEQUENCE 8

typedef struct {
    uint16_t sequence[LEADER_KEY_MAX_SEQUENCE];  /**< Key sequence */
    uint8_t length;                               /**< Current sequence length */
    uint16_t start_time;                          /**< When leader key was pressed */
    bool active;                                  /**< Whether leader mode is active */
} leader_state_t;

/**
 * @brief Initialize leader key
 */
void leader_init(void);

/**
 * @brief Process leader key event
 *
 * @param event Key event
 */
void leader_event(keyevent_t event);

/**
 * @brief Task function for leader key
 */
void leader_task(void);

/**
 * @brief Check if leader key is active
 *
 * @return true if leader mode is active
 */
bool leader_is_active(void);
#endif

// Macro configuration
#ifndef MACRO_ENABLE
#define MACRO_ENABLE 0
#endif

#if MACRO_ENABLE
typedef struct {
    uint16_t keycode;  /**< Keycode to send */
    uint16_t delay;    /**< Delay after this keycode in ms */
} macro_step_t;

#define MACRO_MAX_STEPS 64

typedef struct {
    const macro_step_t* steps;  /**< Pointer to macro steps */
    uint16_t length;            /**< Number of steps */
    uint8_t current_step;       /**< Current step being executed */
    uint16_t start_time;        /**< When macro started */
    bool playing;               /**< Whether macro is playing */
} macro_state_t;

/**
 * @brief Initialize macro system
 */
void macro_init(void);

/**
 * @brief Play a macro
 *
 * @param macro_id Macro ID
 */
void macro_play(uint8_t macro_id);

/**
 * @brief Stop current macro
 *
 * @param macro_id Macro ID
 */
void macro_stop(uint8_t macro_id);

/**
 * @brief Task function for macro
 */
void macro_task(void);

/**
 * @brief Check if macro is playing
 *
 * @param macro_id Macro ID
 * @return true if macro is playing
 */
bool macro_is_playing(uint8_t macro_id);
#endif

// User customization hooks

/**
 * @brief Process custom keycode
 *
 * This function is called for custom keycodes in the PROCESS_RECORD_USER callback
 *
 * @param record Action record
 * @param keycode Keycode being processed
 * @return true to continue processing, false to skip
 */
bool process_record_user(action_record_t* record, uint16_t keycode);

/**
 * @brief Process quantum keycodes
 *
 * Called for QK keycodes that require custom handling
 *
 * @param record Action record
 * @param keycode Quantum keycode
 * @return true to continue processing, false to skip
 */
bool process_record_quantum(action_record_t* record, uint16_t keycode);

#ifdef __cplusplus
}
#endif
