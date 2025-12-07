#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "matrix.h"
#include "keyboard.h"

#ifdef __cplusplus
extern "C" {
#endif

// Combo key structure
typedef struct {
    keypos_t key[4];      /**< Keys that make up the combo (max 4 keys) */
    uint16_t keycode;     /**< Keycode to send when combo is triggered */
    uint8_t count;        /**< Number of keys in this combo */
} combo_t;

// Combo state
typedef struct {
    uint8_t index;         /**< Index of this combo definition */
    uint16_t start_time;   /**< When combo detection started */
    bool active;           /**< Whether combo is being tracked */
    bool triggered;        /**< Whether combo has been triggered */
} combo_state_t;

// Maximum number of combo keys
#ifndef MAX_COMBO_KEYS
#define MAX_COMBO_KEYS 4
#endif

// Maximum number of combos
#ifndef MAX_COMBOS
#define MAX_COMBOS 32
#endif

// Combo term timeout (default from product_config.h)
#ifndef COMBO_TERM
#define COMBO_TERM 200
#endif

// Extended combo term for modifier combos
#ifndef COMBO_MOD_TERM
#define COMBO_MOD_TERM 200
#endif

/**
 * @brief Initialize combo module
 */
void combo_init(void);

/**
 * @brief Process combo key event
 *
 * @param event Key event to process
 * @return true if combo was triggered
 */
bool combo_event(keyevent_t event);

/**
 * @brief Process combo state on each cycle
 *
 * Checks for combo timeout and triggers combos when appropriate
 */
void combo_task(void);

/**
 * @brief Add a combo key to the tracking state
 *
 * @param index Combo index
 * @param key Key position to add
 */
void combo_add_key(uint8_t index, keypos_t key);

/**
 * @brief Remove a combo key from tracking state
 *
 * @param index Combo index
 * @param key Key position to remove
 */
void combo_remove_key(uint8_t index, keypos_t key);

/**
 * @brief Clear all combo tracking state
 */
void combo_clear(void);

/**
 * @brief Check if a combo is currently active
 *
 * @param index Combo index
 * @return true if combo is active
 */
bool combo_is_active(uint8_t index);

/**
 * @brief Get combo state
 *
 * @param index Combo index
 * @return Pointer to combo state
 */
combo_state_t* combo_get_state(uint8_t index);

/**
 * @brief Get the current modifier state during combo processing
 *
 * @return Modifier state
 */
uint8_t get_combo_modifiers(void);

#ifdef __cplusplus
}
#endif
