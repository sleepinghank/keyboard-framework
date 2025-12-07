#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "keyboard.h"
#include "layer.h"

#ifdef __cplusplus
extern "C" {
#endif

// Action types
typedef enum {
    ACTION_NO = 0,
    ACTION_KEY,             /**< Regular key press/release */
    ACTION_MODS,            /**< Modifier keys */
    ACTION_LAYER_TAP,       /**< Layer tap (tap = keycode, hold = layer) */
    ACTION_LAYER_MOD,       /**< Layer mod (layer active when held with mod) */
    ACTION_MODS_TAP,        /**< Mod tap (tap = keycode, hold = modifier) */
    ACTION_FUNCTION,        /**< Custom function */
    ACTION_MACRO,           /**< Macro execution */
} action_type_t;

// Action structure
typedef struct {
    action_type_t type;     /**< Action type */
    uint8_t id;             /**< Action ID */
    uint8_t parameters;     /**< Additional parameters */
} action_t;

// Action state for tapping
typedef struct {
    uint8_t count;          /**< Tap count */
    bool interrupted;       /**< Whether tapping was interrupted */
    bool reserved[2];       /**< Reserved for future use */
    uint16_t keycode;       /**< Associated keycode */
} tap_state_t;

// Action record for tracking
typedef struct {
    keyevent_t event;       /**< Key event that triggered this action */
    tap_state_t tap;        /**< Tap state (for tapping actions) */
} action_record_t;

// Key processing state
extern uint8_t current_mods;        /**< Current modifier state */
extern uint8_t real_mods;           /**< Real modifier state */
extern uint8_t weak_mods;           /**< Weak modifier state */

// Action execution
void action_exec(keyevent_t event);
action_t action_for_key(layer_state_t layer, keypos_t key);
action_t action_for_keycode(uint16_t keycode);
void process_action(action_record_t* record, action_t action);

// Key registration
void register_code(uint8_t code);
void unregister_code(uint8_t code);
void tap_code(uint8_t code);
void tap_code_delay(uint8_t code, uint16_t delay);

// Modifier management
void register_mods(uint8_t mods);
void unregister_mods(uint8_t mods);
void register_weak_mods(uint8_t mods);
void unregister_weak_mods(uint8_t mods);
void clear_mods(void);
uint8_t get_weak_mods(void);
uint8_t get_real_mods(void);
uint8_t get_mods(void);

// Layer actions
void action_layer_tap(uint8_t layer, keypos_t key);
void action_layer_mod(uint8_t layer, uint8_t mods, keypos_t key);
void action_mods_tap(uint8_t mods, keypos_t key);

// One-shot layer support
void oneshot_layer_toggle(uint8_t layer);
void oneshot_layer_set(uint8_t layer);

// Keyboard state management
void clear_keyboard(void);
void clear_keyboard_but_mods(void);
void clear_keyboard_but_mods_and_keys(void);

// Layer switching actions
void layer_switch(uint8_t new_layer);

// Utility functions
bool is_tap_action(action_t action);
bool is_mod_action(action_t action);

// Weak callback hooks
void action_keypress(keyevent_t event);
void action_keyrelease(keyevent_t event);

#ifdef __cplusplus
}
#endif
