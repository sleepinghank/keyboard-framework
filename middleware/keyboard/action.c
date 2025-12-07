/*
 * Copyright (C) 2024 Keyboard Framework
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "action.h"
#include "keycode.h"
#include "timer.h"
#include "print.h"
#include "debug.h"
#include <string.h>

// Modifier state
uint8_t current_mods = 0;
uint8_t real_mods = 0;
uint8_t weak_mods = 0;

// Tapping configuration (from product_config.h)
#ifndef TAPPING_TERM
#define TAPPING_TERM 200
#endif

#ifndef TAPPING_TOGGLE
#define TAPPING_TOGGLE 2
#endif

#ifndef QUICK_TAP_TERM
#define QUICK_TAP_TERM 100
#endif

/**
 * @brief Execute an action based on a key event
 *
 * @param event Key event to process
 */
void action_exec(keyevent_t event) {
    if (IS_EVENT(event)) {
        dprintf("\nAction: Executing action for event\n");
        debug_event(event);
    }

    // Clear weak mods on key press
    if (event.pressed) {
        clear_weak_mods();
    }

    // Get the current layer state
    layer_state_t layer_state = layer_state_get();

    // Get the action for this key position
    action_t action = action_for_key(layer_state, event.key);

    // Create action record
    action_record_t record = {
        .event = event,
        .tap.count = 0,
        .tap.interrupted = false,
        .tap.keycode = 0
    };

    // Process the action
    process_action(&record, action);
}

/**
 * @brief Get the action for a key at a given layer
 *
 * @param layer Current layer state
 * @param key Key position
 * @return Action to execute
 */
action_t action_for_key(layer_state_t layer, keypos_t key) {
    action_t action = {
        .type = ACTION_NO,
        .id = 0,
        .parameters = 0
    };

    // Get the keycode from the keymap
    uint16_t keycode = keymap_key_to_keycode(layer, key.row, key.col);

    // Convert keycode to action
    action = action_for_keycode(keycode);

    return action;
}

/**
 * @brief Get the action for a specific keycode
 *
 * @param keycode Keycode to convert
 * @return Action structure
 */
action_t action_for_keycode(uint16_t keycode) {
    action_t action = {
        .type = ACTION_NO,
        .id = 0,
        .parameters = 0
    };

    if (IS_KEYCODE(keycode)) {
        // Regular key
        action.type = ACTION_KEY;
        action.id = keycode;
    } else if (IS_QK_LAYER_TAP(keycode)) {
        // Layer tap (tap = keycode, hold = layer)
        action.type = ACTION_LAYER_TAP;
        action.id = GET_LAYER(keycode);
        action.parameters = keycode & 0xFF;
    } else if (IS_QK_LAYER_MOD(keycode)) {
        // Layer mod (activate layer with modifier)
        action.type = ACTION_LAYER_MOD;
        action.id = GET_LAYER(keycode);
        action.parameters = (keycode >> 8) & 0x1F;
    } else if (IS_QK_MOD_TAP(keycode)) {
        // Mod tap (tap = keycode, hold = modifier)
        action.type = ACTION_MODS_TAP;
        action.id = (keycode >> 8) & 0x1F;
        action.parameters = keycode & 0xFF;
    } else if (IS_FUNCTION_KEY(keycode)) {
        // Custom function
        action.type = ACTION_FUNCTION;
        action.id = keycode & 0xFF;
    }

    return action;
}

/**
 * @brief Process an action
 *
 * @param record Action record
 * @param action Action to execute
 */
void process_action(action_record_t* record, action_t action) {
    switch (action.type) {
        case ACTION_NO:
            // No action
            break;

        case ACTION_KEY:
            if (record->event.pressed) {
                register_code(action.id);
                dprintf("Action: Registered key 0x%02X\n", action.id);
            } else {
                unregister_code(action.id);
                dprintf("Action: Unregistered key 0x%02X\n", action.id);
            }
            break;

        case ACTION_MODS:
            if (record->event.pressed) {
                register_mods(action.id);
            } else {
                unregister_mods(action.id);
            }
            break;

        case ACTION_LAYER_TAP:
            action_layer_tap(action.id, record->event.key);
            break;

        case ACTION_LAYER_MOD:
            action_layer_mod(action.id, action.parameters, record->event.key);
            break;

        case ACTION_MODS_TAP:
            action_mods_tap(action.id, record->event.key);
            break;

        case ACTION_FUNCTION:
            // Call custom function handler
            if (record->event.pressed) {
                process_record_function(record, action.id);
            }
            break;

        case ACTION_MACRO:
            // Process macro
            process_record_macro(record, action.id);
            break;

        default:
            dprintf("Action: ERROR - Unknown action type %d\n", action.type);
            break;
    }
}

/**
 * @brief Register a keycode
 *
 * @param code Keycode to register
 */
void register_code(uint8_t code) {
    if (code == KC_NO) {
        return;
    }

    // Add to current mods if it's a modifier
    if (code >= KC_LCTRL && code <= KC_RGUI) {
        current_mods |= MOD_BIT(code);
        real_mods |= MOD_BIT(code);
    }

    dprintf("Action: Registered code 0x%02X\n", code);
}

/**
 * @brief Unregister a keycode
 *
 * @param code Keycode to unregister
 */
void unregister_code(uint8_t code) {
    if (code == KC_NO) {
        return;
    }

    // Remove from current mods if it's a modifier
    if (code >= KC_LCTRL && code <= KC_RGUI) {
        current_mods &= ~MOD_BIT(code);
        real_mods &= ~MOD_BIT(code);
    }

    dprintf("Action: Unregistered code 0x%02X\n", code);
}

/**
 * @brief Tap (press and release) a keycode
 *
 * @param code Keycode to tap
 */
void tap_code(uint8_t code) {
    register_code(code);
    wait_ms(TAP_CODE_DELAY);
    unregister_code(code);
}

/**
 * @brief Tap a keycode with delay
 *
 * @param code Keycode to tap
 * @param delay Delay in milliseconds
 */
void tap_code_delay(uint8_t code, uint16_t delay) {
    register_code(code);
    wait_ms(delay);
    unregister_code(code);
}

/**
 * @brief Register modifier keys
 *
 * @param mods Modifier bitmask
 */
void register_mods(uint8_t mods) {
    real_mods |= mods;
    current_mods |= mods;
    dprintf("Action: Registered mods 0x%02X\n", mods);
}

/**
 * @brief Unregister modifier keys
 *
 * @param mods Modifier bitmask
 */
void unregister_mods(uint8_t mods) {
    real_mods &= ~mods;
    current_mods &= ~mods;
    dprintf("Action: Unregistered mods 0x%02X\n", mods);
}

/**
 * @brief Register weak modifier keys
 *
 * Weak mods are temporary and cleared on key press
 *
 * @param mods Modifier bitmask
 */
void register_weak_mods(uint8_t mods) {
    weak_mods |= mods;
    current_mods |= mods;
    dprintf("Action: Registered weak mods 0x%02X\n", mods);
}

/**
 * @brief Unregister weak modifier keys
 *
 * @param mods Modifier bitmask
 */
void unregister_weak_mods(uint8_t mods) {
    weak_mods &= ~mods;
    current_mods &= ~mods;
    dprintf("Action: Unregistered weak mods 0x%02X\n", mods);
}

/**
 * @brief Clear all modifiers
 */
void clear_mods(void) {
    real_mods = 0;
    current_mods = 0;
    dprintf("Action: Cleared all mods\n");
}

/**
 * @brief Clear all weak modifiers
 */
void clear_weak_mods(void) {
    current_mods &= ~weak_mods;
    weak_mods = 0;
}

/**
 * @brief Get current weak modifiers
 *
 * @return Weak modifier state
 */
uint8_t get_weak_mods(void) {
    return weak_mods;
}

/**
 * @brief Get current real modifiers
 *
 * @return Real modifier state
 */
uint8_t get_real_mods(void) {
    return real_mods;
}

/**
 * @brief Get current modifiers
 *
 * @return Current modifier state
 */
uint8_t get_mods(void) {
    return current_mods;
}

/**
 * @brief Handle layer tap action
 *
 * Tap = execute keycode, Hold = activate layer
 *
 * @param layer Layer to activate on hold
 * @param key Key position
 */
void action_layer_tap(uint8_t layer, keypos_t key) {
    uint16_t keycode = keymap_key_to_keycode(default_layer_state, key.row, key.col);
    keycode &= 0xFF; // Get the tap keycode

    if (keycode == KC_TRANSPARENT) {
        return;
    }

    // This is a simplified implementation
    // In a full implementation, we would track tap timing
    dprintf("Action: Layer tap - layer %d, keycode 0x%02X\n", layer, keycode);
}

/**
 * @brief Handle layer mod action
 *
 * Activate layer when modifier is held
 *
 * @param layer Layer to activate
 * @param mods Modifiers to activate with
 * @param key Key position
 */
void action_layer_mod(uint8_t layer, uint8_t mods, keypos_t key) {
    dprintf("Action: Layer mod - layer %d, mods 0x%02X\n", layer, mods);

    // When mods are pressed, activate layer
    if (real_mods & mods) {
        layer_on(layer);
    } else {
        layer_off(layer);
    }
}

/**
 * @brief Handle mod tap action
 *
 * Tap = execute keycode, Hold = activate modifier
 *
 * @param mods Modifiers to activate on hold
 * @param key Key position
 */
void action_mods_tap(uint8_t mods, keypos_t key) {
    uint16_t keycode = keymap_key_to_keycode(default_layer_state, key.row, key.col);
    keycode &= 0xFF;

    if (keycode == KC_TRANSPARENT) {
        return;
    }

    dprintf("Action: Mod tap - mods 0x%02X, keycode 0x%02X\n", mods, keycode);
}

/**
 * @brief Switch to a layer
 *
 * @param new_layer Layer to switch to
 */
void layer_switch(uint8_t new_layer) {
    if (new_layer >= MAX_LAYER_COUNT) {
        dprintf("Action: ERROR - Invalid layer %d\n", new_layer);
        return;
    }

    layer_clear();
    layer_on(new_layer);
    dprintf("Action: Switched to layer %d\n", new_layer);
}

/**
 * @brief Clear the entire keyboard state
 */
void clear_keyboard(void) {
    clear_mods();
    clear_keyboard_but_mods();
    dprintf("Action: Cleared keyboard\n");
}

/**
 * @brief Clear keyboard but keep modifiers
 */
void clear_keyboard_but_mods(void) {
    // This would send key release for all currently pressed keys
    // Implementation depends on the driver layer
    dprintf("Action: Cleared keyboard (keeping mods)\n");
}

/**
 * @brief Clear keyboard but keep modifiers and keys
 */
void clear_keyboard_but_mods_and_keys(void) {
    // Only clear LED states and other keyboard state
    dprintf("Action: Cleared keyboard state (keeping mods and keys)\n");
}

/**
 * @brief Check if an action is a tap action
 *
 * @param action Action to check
 * @return true if it's a tap action
 */
bool is_tap_action(action_t action) {
    return (action.type == ACTION_LAYER_TAP ||
            action.type == ACTION_MODS_TAP);
}

/**
 * @brief Check if an action is a modifier action
 *
 * @param action Action to check
 * @return true if it's a modifier action
 */
bool is_mod_action(action_t action) {
    return (action.type == ACTION_MODS ||
            action.type == ACTION_LAYER_MOD ||
            action.type == ACTION_MODS_TAP);
}

// Weak callback hooks (can be overridden)
__attribute__((weak)) void action_keypress(keyevent_t event) {
    // Hook for custom key press handling
}

__attribute__((weak)) void action_keyrelease(keyevent_t event) {
    // Hook for custom key release handling
}

// Function and macro processing (weak implementations)
__attribute__((weak)) void process_record_function(action_record_t* record, uint8_t function_id) {
    dprintf("Action: Processing function %d\n", function_id);
}

__attribute__((weak)) void process_record_macro(action_record_t* record, uint8_t macro_id) {
    dprintf("Action: Processing macro %d\n", macro_id);
}
