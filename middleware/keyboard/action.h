#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "keyboard.h"
#include "action_layer.h"
#include "action_code.h"
#include "keycode.h"
#include "progmem.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Utilities for actions.  */
#if !defined(NO_ACTION_LAYER) && !defined(STRICT_LAYER_RELEASE)
extern bool disable_action_cache;
#endif

/* tapping count and state */
typedef struct {
    bool    interrupted : 1;
    bool    reserved2 : 1;
    bool    reserved1 : 1;
    bool    reserved0 : 1;
    uint8_t count : 4;
} tap_t;

/* Key event container for recording */
typedef struct {
    keyevent_t event;
#ifndef NO_ACTION_TAPPING
    tap_t tap;
#endif
#if defined(COMBO_ENABLE) || defined(REPEAT_KEY_ENABLE)
    uint16_t keycode;
#endif
} keyrecord_t;

// Action execution
void action_exec(keyevent_t event);

void process_action(keyrecord_t* record, action_t action);
void process_action_tapping(keyrecord_t* record, action_t action);

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

#ifdef ACTION_DEBUG
#    include "..\..\utils\logging\debug.h"
#    include "..\..\utils\logging\print.h"
#    define ac_dprintf(...) dprintf(__VA_ARGS__)
#else
#    define ac_dprintf(...) \
        do {                \
        } while (0)
#endif

void debug_event(keyevent_t event);
void debug_record(keyrecord_t record);
void debug_action(action_t action);

uint16_t get_record_keycode(keyrecord_t *record, bool update_layer_cache);
uint16_t get_event_keycode(keyevent_t event, bool update_layer_cache);

#ifdef __cplusplus
}
#endif
