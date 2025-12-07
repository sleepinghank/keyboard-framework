/*
 * Copyright (C) 2024 Keyboard Framework
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "combo.h"
#include "timer.h"
#include "action.h"
#include "print.h"
#include <string.h>

// Combo definitions - to be provided by keyboard/keymap
extern const combo_t COMBO[];

// Combo count - to be provided by keyboard/keymap
extern const uint16_t COMBO_COUNT;

// Combo state tracking
static combo_state_t combo_state[MAX_COMBOS];
static uint8_t combo_modifiers = 0;

// Helper macro to check if two keys are equal
#define KEY_EQ(a, b) ((a).row == (b).row && (a).col == (b).col)

/**
 * @brief Initialize combo module
 */
void combo_init(void) {
    memset(combo_state, 0, sizeof(combo_state));
    combo_modifiers = 0;

    dprintf("Combo: Initialized with %d combos\n", COMBO_COUNT);
}

/**
 * @brief Check if a key position matches a combo key
 *
 * @param combo_index Index of the combo
 * @param key Key position to check
 * @return true if key is part of the combo
 */
static bool combo_key_match(uint8_t combo_index, keypos_t key) {
    if (combo_index >= COMBO_COUNT) {
        return false;
    }

    const combo_t* combo = &COMBO[combo_index];

    for (uint8_t i = 0; i < combo->count; i++) {
        if (KEY_EQ(combo->key[i], key)) {
            return true;
        }
    }

    return false;
}

/**
 * @brief Check if all combo keys are currently pressed
 *
 * @param combo_index Index of the combo
 * @return true if all keys are pressed
 */
static bool combo_all_keys_pressed(uint8_t combo_index) {
    if (combo_index >= COMBO_COUNT) {
        return false;
    }

    const combo_t* combo = &COMBO[combo_index];

    for (uint8_t i = 0; i < combo->count; i++) {
        // Check if this key is currently pressed in the matrix
        // This would require access to the current matrix state
        // For now, we'll track this in combo_add_key/remove_key
        if (!(combo_state[combo_index].active)) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Get combo term based on whether it contains modifiers
 *
 * @param combo_index Index of the combo
 * @return Timeout in milliseconds
 */
static uint16_t get_combo_term(uint8_t combo_index) {
    if (combo_index >= COMBO_COUNT) {
        return COMBO_TERM;
    }

    const combo_t* combo = &COMBO[combo_index];

    // Check if combo contains modifier keys
    for (uint8_t i = 0; i < combo->count; i++) {
        uint16_t keycode = action_get_keycode(combo->key[i]);
        if (IS_KEYCODE(keycode) && (keycode >= KC_LCTRL && keycode <= KC_RGUI)) {
            return COMBO_MOD_TERM;
        }
    }

    return COMBO_TERM;
}

/**
 * @brief Process combo key event
 *
 * @param event Key event to process
 * @return true if combo was triggered
 */
bool combo_event(keyevent_t event) {
    bool combo_triggered = false;

    // Track modifier state
    if (IS_KEYEVENT(event)) {
        uint16_t keycode = action_get_keycode(event.key);

        // Check if this is a modifier key
        if (IS_KEYCODE(keycode) && keycode >= KC_LCTRL && keycode <= KC_RGUI) {
            uint8_t modifier_bit = 0;
            switch (keycode) {
                case KC_LCTRL: modifier_bit = MOD_LCTRL; break;
                case KC_LSHIFT: modifier_bit = MOD_LSHIFT; break;
                case KC_LALT: modifier_bit = MOD_LALT; break;
                case KC_LGUI: modifier_bit = MOD_LGUI; break;
                case KC_RCTRL: modifier_bit = MOD_RCTRL; break;
                case KC_RSHIFT: modifier_bit = MOD_RSHIFT; break;
                case KC_RALT: modifier_bit = MOD_RALT; break;
                case KC_RGUI: modifier_bit = MOD_RGUI; break;
            }

            if (event.pressed) {
                combo_modifiers |= modifier_bit;
            } else {
                combo_modifiers &= ~modifier_bit;
            }
        }
    }

    // Check all combos for this key
    for (uint8_t i = 0; i < COMBO_COUNT; i++) {
        if (combo_key_match(i, event.key)) {
            if (event.pressed) {
                combo_add_key(i, event.key);
            } else {
                combo_remove_key(i, event.key);
            }
        }
    }

    // Check if any combo should be triggered
    for (uint8_t i = 0; i < COMBO_COUNT; i++) {
        if (combo_is_active(i) && !combo_state[i].triggered) {
            const combo_t* combo = &COMBO[i];
            uint16_t term = get_combo_term(i);

            // If all keys are pressed and we're within the timeout, trigger
            if (combo_all_keys_pressed(i)) {
                uint16_t elapsed = timer_elapsed(combo_state[i].start_time);
                if (elapsed < term) {
                    // Trigger the combo
                    keyevent_t combo_event = {
                        .key = MAKE_KEYPOS(0, 0), // Dummy position
                        .time = timer_read(),
                        .type = COMBO_EVENT,
                        .pressed = true
                    };

                    action_exec(combo_event);

                    // Send the combo keycode
                    tap_code(combo->keycode);

                    combo_state[i].triggered = true;
                    combo_triggered = true;

                    dprintf("Combo: Triggered combo %d (keycode 0x%04X)\n", i, combo->keycode);
                }
            }
        }
    }

    return combo_triggered;
}

/**
 * @brief Process combo state on each cycle
 *
 * Checks for combo timeout and triggers combos when appropriate
 */
void combo_task(void) {
    uint16_t now = timer_read();

    for (uint8_t i = 0; i < COMBO_COUNT; i++) {
        if (combo_is_active(i) && !combo_state[i].triggered) {
            const combo_t* combo = &COMBO[i];
            uint16_t term = get_combo_term(i);

            // Check timeout
            uint16_t elapsed = now - combo_state[i].start_time;
            if (elapsed > term) {
                // Timeout - trigger the combo key that should be pressed first
                // Actually, for defer mode, we let the keys pass through
                dprintf("Combo: Combo %d timed out after %dms\n", i, elapsed);

                // For now, just clear the active state
                combo_state[i].active = false;
            } else if (combo_all_keys_pressed(i)) {
                // All keys are pressed within timeout - trigger
                keyevent_t combo_event = {
                    .key = MAKE_KEYPOS(0, 0),
                    .time = now,
                    .type = COMBO_EVENT,
                    .pressed = true
                };

                action_exec(combo_event);
                tap_code(combo->keycode);

                combo_state[i].triggered = true;
                dprintf("Combo: Triggered combo %d (keycode 0x%04X) at %dms\n", i, combo->keycode, elapsed);
            }
        }
    }
}

/**
 * @brief Add a combo key to the tracking state
 *
 * @param index Combo index
 * @param key Key position to add
 */
void combo_add_key(uint8_t index, keypos_t key) {
    if (index >= COMBO_COUNT) {
        dprintf("Combo: ERROR - Invalid combo index %d\n", index);
        return;
    }

    if (!combo_state[index].active) {
        combo_state[index].index = index;
        combo_state[index].start_time = timer_read();
        combo_state[index].active = true;
        combo_state[index].triggered = false;

        dprintf("Combo: Started tracking combo %d at %dms\n", index, combo_state[index].start_time);
    }
}

/**
 * @brief Remove a combo key from tracking state
 *
 * @param index Combo index
 * @param key Key position to remove
 */
void combo_remove_key(uint8_t index, keypos_t key) {
    if (index >= COMBO_COUNT) {
        dprintf("Combo: ERROR - Invalid combo index %d\n", index);
        return;
    }

    // Clear active state when any key is released
    combo_state[index].active = false;
    combo_state[index].triggered = false;

    dprintf("Combo: Stopped tracking combo %d\n", index);
}

/**
 * @brief Clear all combo tracking state
 */
void combo_clear(void) {
    memset(combo_state, 0, sizeof(combo_state));
    combo_modifiers = 0;
    dprintf("Combo: Cleared all states\n");
}

/**
 * @brief Check if a combo is currently active
 *
 * @param index Combo index
 * @return true if combo is active
 */
bool combo_is_active(uint8_t index) {
    if (index >= COMBO_COUNT) {
        return false;
    }

    return combo_state[index].active;
}

/**
 * @brief Get combo state
 *
 * @param index Combo index
 * @return Pointer to combo state
 */
combo_state_t* combo_get_state(uint8_t index) {
    if (index >= COMBO_COUNT) {
        return NULL;
    }

    return &combo_state[index];
}

/**
 * @brief Get the current modifier state during combo processing
 *
 * @return Modifier state
 */
uint8_t get_combo_modifiers(void) {
    return combo_modifiers;
}

// Helper function to get keycode for a position (defined in action module)
__attribute__((weak)) uint16_t action_get_keycode(keypos_t key) {
    // This should be implemented in the action module
    // For now, return KC_NO
    return KC_NO;
}
