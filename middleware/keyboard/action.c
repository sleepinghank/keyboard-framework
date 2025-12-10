/*
 * Copyright (C) 2024 Keyboard Framework
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "action.h"
#include "action_util.h"
#include "keycode.h"
#include "timer.h"
#include "print.h"
#include "debug.h"
#include <string.h>
#include "keycode_config.h"
#include "wait.h"
#include "action_layer.h"
#include "keymap_common.h"

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


void debug_event(keyevent_t event) {
    ac_dprintf("%04X%c(%u)", (event.key.row << 8 | event.key.col), (event.pressed ? 'd' : 'u'), event.time);
}

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

    /* Clear weak mods on key press */
    if (event.pressed) {
        clear_weak_mods();
    }

    /* Create action record */
    keyrecord_t record = {
        .event = event,
    };

    /* Get current layer state */
    layer_state_t layer_state = layer_state_get();

    /* Get the action for this key position */
    action_t action = action_for_key(layer_state, event.key);

#ifdef COMBO_ENABLE
    /* Process combo handling first */
    if (!combo_event(event)) {
        /* Combo not triggered, continue with tapping */
        process_action_tapping(&record, action);
    }
#else
    /* No combo support, process tapping directly */
    process_action_tapping(&record, action);
#endif
}

/**
 * @brief Process an action
 *
 * @param record Action record
 * @param action Action to execute
 */
void process_action(keyrecord_t *record, action_t action) {
    keyevent_t event = record->event;
#ifndef NO_ACTION_TAPPING
    uint8_t tap_count = record->tap.count;
#endif

#ifndef NO_ACTION_ONESHOT
    bool do_release_oneshot = false;
    // notice we only clear the one shot layer if the pressed key is not a modifier.
    if (is_oneshot_layer_active() && event.pressed &&
        (action.kind.id == ACT_USAGE || !(IS_MODIFIER_KEYCODE(action.key.code)
#    ifndef NO_ACTION_TAPPING
                                          || ((action.kind.id == ACT_LMODS_TAP || action.kind.id == ACT_RMODS_TAP) && (action.layer_tap.code <= MODS_TAP_TOGGLE || tap_count == 0))
#    endif
                                              ))
#    ifdef SWAP_HANDS_ENABLE
        && !(action.kind.id == ACT_SWAP_HANDS && action.swap.code == OP_SH_ONESHOT)
#    endif
        && keymap_config.oneshot_enable) {
        clear_oneshot_layer_state(ONESHOT_OTHER_KEY_PRESSED);
        do_release_oneshot = !is_oneshot_layer_active();
    }
#endif

    switch (action.kind.id) {
        /* Key and Mods */
        case ACT_LMODS:
        case ACT_RMODS: {
            uint8_t mods = (action.kind.id == ACT_LMODS) ? action.key.mods : action.key.mods << 4;
            if (event.pressed) {
                if (mods) {
                    if (IS_MODIFIER_KEYCODE(action.key.code) || action.key.code == KC_NO) {
                        // e.g. LSFT(KC_LEFT_GUI): we don't want the LSFT to be weak as it would make it useless.
                        // This also makes LSFT(KC_LEFT_GUI) behave exactly the same as LGUI(KC_LEFT_SHIFT).
                        // Same applies for some keys like KC_MEH which are declared as MEH(KC_NO).
                        add_mods(mods);
                    } else {
                        add_weak_mods(mods);
                    }
                    send_keyboard_report();
                }
                register_code(action.key.code);
            } else {
                unregister_code(action.key.code);
                if (mods) {
                    if (IS_MODIFIER_KEYCODE(action.key.code) || action.key.code == KC_NO) {
                        del_mods(mods);
                    } else {
                        del_weak_mods(mods);
                    }
                    send_keyboard_report();
                }
            }
        } break;
        case ACT_LMODS_TAP:
        case ACT_RMODS_TAP: {
#ifndef NO_ACTION_TAPPING
            uint8_t mods = (action.kind.id == ACT_LMODS_TAP) ? action.key.mods : action.key.mods << 4;
            switch (action.layer_tap.code) {
#    ifndef NO_ACTION_ONESHOT
                case MODS_ONESHOT:
                    // Oneshot modifier
                    if (!keymap_config.oneshot_enable) {
                        if (event.pressed) {
                            if (mods) {
                                if (IS_MODIFIER_KEYCODE(action.key.code) || action.key.code == KC_NO) {
                                    // e.g. LSFT(KC_LGUI): we don't want the LSFT to be weak as it would make it useless.
                                    // This also makes LSFT(KC_LGUI) behave exactly the same as LGUI(KC_LSFT).
                                    // Same applies for some keys like KC_MEH which are declared as MEH(KC_NO).
                                    add_mods(mods);
                                } else {
                                    add_weak_mods(mods);
                                }
                                send_keyboard_report();
                            }
                            register_code(action.key.code);
                        } else {
                            unregister_code(action.key.code);
                            if (mods) {
                                if (IS_MODIFIER_KEYCODE(action.key.code) || action.key.code == KC_NO) {
                                    del_mods(mods);
                                } else {
                                    del_weak_mods(mods);
                                }
                                send_keyboard_report();
                            }
                        }
                    } else {
                        if (event.pressed) {
                            if (tap_count == 0) {
                                // Not a tap, but a hold: register the held mod
                                ac_dprintf("MODS_TAP: Oneshot: 0\n");
                                register_mods(mods);
                            } else if (tap_count == 1) {
                                ac_dprintf("MODS_TAP: Oneshot: start\n");
                                add_oneshot_mods(mods);
#        if defined(ONESHOT_TAP_TOGGLE) && ONESHOT_TAP_TOGGLE > 1
                            } else if (tap_count == ONESHOT_TAP_TOGGLE) {
                                ac_dprintf("MODS_TAP: Toggling oneshot");
                                register_mods(mods);
                                del_oneshot_mods(mods);
                                add_oneshot_locked_mods(mods);
#        endif
                            }
                        } else {
                            if (tap_count == 0) {
                                // Release hold: unregister the held mod and its variants
                                unregister_mods(mods);
                                del_oneshot_mods(mods);
                                del_oneshot_locked_mods(mods);
#        if defined(ONESHOT_TAP_TOGGLE) && ONESHOT_TAP_TOGGLE > 1
                            } else if (tap_count == 1 && (mods & get_mods())) {
                                unregister_mods(mods);
                                del_oneshot_mods(mods);
                                del_oneshot_locked_mods(mods);
#        endif
                            }
                        }
                    }
                    break;
#    endif
                case MODS_TAP_TOGGLE:
                    if (event.pressed) {
                        if (tap_count <= TAPPING_TOGGLE) {
                            register_mods(mods);
                        }
                    } else {
                        if (tap_count < TAPPING_TOGGLE) {
                            unregister_mods(mods);
                        }
                    }
                    break;
                default:
                    if (event.pressed) {
                        if (tap_count > 0) {
#    ifdef HOLD_ON_OTHER_KEY_PRESS
                            if (
#        ifdef HOLD_ON_OTHER_KEY_PRESS_PER_KEY
                                get_hold_on_other_key_press(get_event_keycode(record->event, false), record) &&
#        endif
                                record->tap.interrupted) {
                                ac_dprintf("mods_tap: tap: cancel: add_mods\n");
                                // ad hoc: set 0 to cancel tap
                                record->tap.count = 0;
                                register_mods(mods);
                            } else
#    endif
                            {
                                ac_dprintf("MODS_TAP: Tap: register_code\n");
                                register_code(action.key.code);
                            }
                        } else {
                            ac_dprintf("MODS_TAP: No tap: add_mods\n");
                            register_mods(mods);
                        }
                    } else {
                        if (tap_count > 0) {
                            ac_dprintf("MODS_TAP: Tap: unregister_code\n");
                            if (action.layer_tap.code == KC_CAPS_LOCK) {
                                wait_ms(TAP_HOLD_CAPS_DELAY);
                            } else {
                                wait_ms(TAP_CODE_DELAY);
                            }
                            unregister_code(action.key.code);
                        } else {
                            ac_dprintf("MODS_TAP: No tap: add_mods\n");
#    if defined(RETRO_TAPPING) && defined(DUMMY_MOD_NEUTRALIZER_KEYCODE)
                            // Send a dummy keycode to neutralize flashing modifiers
                            // if the key was held and then released with no interruptions.
                            if (retro_tapping_counter == 2) {
                                neutralize_flashing_modifiers(get_mods());
                            }
#    endif
                            unregister_mods(mods);
                        }
                    }
                    break;
            }
#endif // NO_ACTION_TAPPING
        } break;
#ifdef EXTRAKEY_ENABLE
        /* other HID usage */
        case ACT_USAGE:
            switch (action.usage.page) {
                case PAGE_SYSTEM:
                    host_system_send(event.pressed ? action.usage.code : 0);
                    break;
                case PAGE_CONSUMER:
                    host_consumer_send(event.pressed ? action.usage.code : 0);
                    break;
            }
            break;
#endif // EXTRAKEY_ENABLE
        /* Mouse key */
        case ACT_MOUSEKEY:
//            register_mouse(action.key.code, event.pressed);
            break;
#ifndef NO_ACTION_LAYER
        case ACT_LAYER:
            if (action.layer_bitop.on == 0) {
                /* Default Layer Bitwise Operation */
                if (!event.pressed) {
                    uint8_t       shift = action.layer_bitop.part * 4;
                    layer_state_t bits  = ((layer_state_t)action.layer_bitop.bits) << shift;
                    layer_state_t mask  = (action.layer_bitop.xbit) ? ~(((layer_state_t)0xf) << shift) : 0;
                    switch (action.layer_bitop.op) {
                        case OP_BIT_AND:
                            default_layer_and(bits | mask);
                            break;
                        case OP_BIT_OR:
                            default_layer_or(bits | mask);
                            break;
                        case OP_BIT_XOR:
                            default_layer_xor(bits | mask);
                            break;
                        case OP_BIT_SET:
                            default_layer_set(bits | mask);
                            break;
                    }
                }
            } else {
                /* Layer Bitwise Operation */
                if (event.pressed ? (action.layer_bitop.on & ON_PRESS) : (action.layer_bitop.on & ON_RELEASE)) {
                    uint8_t       shift = action.layer_bitop.part * 4;
                    layer_state_t bits  = ((layer_state_t)action.layer_bitop.bits) << shift;
                    layer_state_t mask  = (action.layer_bitop.xbit) ? ~(((layer_state_t)0xf) << shift) : 0;
                    switch (action.layer_bitop.op) {
                        case OP_BIT_AND:
                            layer_and(bits | mask);
                            break;
                        case OP_BIT_OR:
                            layer_or(bits | mask);
                            break;
                        case OP_BIT_XOR:
                            layer_xor(bits | mask);
                            break;
                        case OP_BIT_SET:
                            layer_state_set(bits | mask);
                            break;
                    }
                }
            }
            break;
        case ACT_LAYER_MODS:
            if (event.pressed) {
                layer_on(action.layer_mods.layer);
                register_mods(action.layer_mods.mods);
            } else {
                unregister_mods(action.layer_mods.mods);
                layer_off(action.layer_mods.layer);
            }
            break;
        case ACT_LAYER_TAP:
        case ACT_LAYER_TAP_EXT:
            switch (action.layer_tap.code) {
#    ifndef NO_ACTION_TAPPING
                case OP_TAP_TOGGLE:
                    /* tap toggle */
                    if (event.pressed) {
                        if (tap_count < TAPPING_TOGGLE) {
                            layer_invert(action.layer_tap.val);
                        }
                    } else {
                        if (tap_count <= TAPPING_TOGGLE) {
                            layer_invert(action.layer_tap.val);
                        }
                    }
                    break;
#    endif
                case OP_ON_OFF:
                    event.pressed ? layer_on(action.layer_tap.val) : layer_off(action.layer_tap.val);
                    break;
                case OP_OFF_ON:
                    event.pressed ? layer_off(action.layer_tap.val) : layer_on(action.layer_tap.val);
                    break;
                case OP_SET_CLEAR:
                    event.pressed ? layer_move(action.layer_tap.val) : layer_clear();
                    break;
#    if !defined(NO_ACTION_ONESHOT) && !defined(NO_ACTION_TAPPING)
                case OP_ONESHOT:
                    // Oneshot modifier
                    if (!keymap_config.oneshot_enable) {
                        if (event.pressed) {
                            layer_on(action.layer_tap.val);
                        } else {
                            layer_off(action.layer_tap.val);
                        }
                    } else {
#        if defined(ONESHOT_TAP_TOGGLE) && ONESHOT_TAP_TOGGLE > 1
                        do_release_oneshot = false;
                        if (event.pressed) {
                            if (get_oneshot_layer_state() == ONESHOT_TOGGLED) {
                                reset_oneshot_layer();
                                layer_off(action.layer_tap.val);
                                break;
                            } else if (tap_count < ONESHOT_TAP_TOGGLE) {
                                layer_on(action.layer_tap.val);
                                set_oneshot_layer(action.layer_tap.val, ONESHOT_START);
                            }
                        } else {
                            if (tap_count >= ONESHOT_TAP_TOGGLE) {
                                reset_oneshot_layer();
                                set_oneshot_layer(action.layer_tap.val, ONESHOT_TOGGLED);
                            } else {
                                clear_oneshot_layer_state(ONESHOT_PRESSED);
                            }
                        }
#        else
                        if (event.pressed) {
                            layer_on(action.layer_tap.val);
                            set_oneshot_layer(action.layer_tap.val, ONESHOT_START);
                        } else {
                            clear_oneshot_layer_state(ONESHOT_PRESSED);
                            if (tap_count > 1) {
                                clear_oneshot_layer_state(ONESHOT_OTHER_KEY_PRESSED);
                            }
                        }
#        endif
                    }
#    else  // NO_ACTION_ONESHOT && NO_ACTION_TAPPING
                    if (event.pressed) {
                        layer_on(action.layer_tap.val);
                    } else {
                        layer_off(action.layer_tap.val);
                    }
#    endif // !defined(NO_ACTION_ONESHOT) && !defined(NO_ACTION_TAPPING)
                    break;
                default:
#    ifndef NO_ACTION_TAPPING /* tap key */
                    if (event.pressed) {
                        if (tap_count > 0) {
                            ac_dprintf("KEYMAP_TAP_KEY: Tap: register_code\n");
                            register_code(action.layer_tap.code);
                        } else {
                            ac_dprintf("KEYMAP_TAP_KEY: No tap: On on press\n");
                            layer_on(action.layer_tap.val);
                        }
                    } else {
                        if (tap_count > 0) {
                            ac_dprintf("KEYMAP_TAP_KEY: Tap: unregister_code\n");
                            if (action.layer_tap.code == KC_CAPS_LOCK) {
                                wait_ms(TAP_HOLD_CAPS_DELAY);
                            } else {
                                wait_ms(TAP_CODE_DELAY);
                            }
                            unregister_code(action.layer_tap.code);
                        } else {
                            ac_dprintf("KEYMAP_TAP_KEY: No tap: Off on release\n");
                            layer_off(action.layer_tap.val);
                        }
                    }
#    else
                    if (event.pressed) {
                        ac_dprintf("KEYMAP_TAP_KEY: Tap: register_code\n");
                        register_code(action.layer_tap.code);
                    } else {
                        ac_dprintf("KEYMAP_TAP_KEY: Tap: unregister_code\n");
                        if (action.layer_tap.code == KC_CAPS) {
                            wait_ms(TAP_HOLD_CAPS_DELAY);
                        } else {
                            wait_ms(TAP_CODE_DELAY);
                        }
                        unregister_code(action.layer_tap.code);
                    }
#    endif
                    break;
            }
            break;
#endif // NO_ACTION_LAYER

#ifdef SWAP_HANDS_ENABLE
        case ACT_SWAP_HANDS:
            switch (action.swap.code) {
                case OP_SH_TOGGLE:
                    if (event.pressed) {
                        swap_hands = !swap_hands;
                    }
                    break;
                case OP_SH_ON_OFF:
                    swap_hands = event.pressed;
                    break;
                case OP_SH_OFF_ON:
                    swap_hands = !event.pressed;
                    break;
                case OP_SH_ON:
                    if (!event.pressed) {
                        swap_hands = true;
                    }
                    break;
                case OP_SH_OFF:
                    if (!event.pressed) {
                        swap_hands = false;
                    }
                    break;
#    ifndef NO_ACTION_ONESHOT
                case OP_SH_ONESHOT:
                    if (event.pressed) {
                        set_oneshot_swaphands();
                    } else {
                        release_oneshot_swaphands();
                    }
                    break;
#    endif

#    ifndef NO_ACTION_TAPPING
                case OP_SH_TAP_TOGGLE:
                    /* tap toggle */

                    if (event.pressed) {
                        if (swap_held) {
                            swap_held = false;
                        } else {
                            swap_hands = !swap_hands;
                        }
                    } else {
                        if (tap_count < TAPPING_TOGGLE) {
                            swap_hands = !swap_hands;
                        }
                    }
                    break;
                default:
                    /* tap key */
                    if (tap_count > 0) {
                        if (swap_held) {
                            swap_hands = !swap_hands; // undo hold set up in _tap_hint
                            swap_held  = false;
                        }
                        if (event.pressed) {
                            register_code(action.swap.code);
                        } else {
                            wait_ms(TAP_CODE_DELAY);
                            unregister_code(action.swap.code);
                            *record = (keyrecord_t){}; // hack: reset tap mode
                        }
                    } else {
                        if (swap_held && !event.pressed) {
                            swap_hands = !swap_hands; // undo hold set up in _tap_hint
                            swap_held  = false;
                        }
                    }
#    endif
            }
#endif
        default:
            break;
    }

#ifndef NO_ACTION_LAYER
    // if this event is a layer action, update the leds
    switch (action.kind.id) {
        case ACT_LAYER:
        case ACT_LAYER_MODS:
#    ifndef NO_ACTION_TAPPING
        case ACT_LAYER_TAP:
        case ACT_LAYER_TAP_EXT:
#    endif
//            led_set(host_keyboard_leds());
            break;
        default:
            break;
    }
#endif

#ifndef NO_ACTION_TAPPING
#    if defined(RETRO_TAPPING) || defined(RETRO_TAPPING_PER_KEY) || (defined(AUTO_SHIFT_ENABLE) && defined(RETRO_SHIFT))
    if (!is_tap_action(action)) {
        retro_tapping_counter = 0;
    } else {
        if (event.pressed) {
            if (tap_count > 0) {
                retro_tapping_counter = 0;
            }
        } else {
            if (tap_count > 0) {
                retro_tapping_counter = 0;
            } else {
                if (
#        ifdef RETRO_TAPPING_PER_KEY
                    get_retro_tapping(get_event_keycode(record->event, false), record) &&
#        endif
                    retro_tapping_counter == 2) {
#        if defined(AUTO_SHIFT_ENABLE) && defined(RETRO_SHIFT)
                    process_auto_shift(action.layer_tap.code, record);
#        else
                    tap_code(action.layer_tap.code);
#        endif
                }
                retro_tapping_counter = 0;
            }
        }
    }
#    endif
#endif

#ifdef SWAP_HANDS_ENABLE
#    ifndef NO_ACTION_ONESHOT
    if (event.pressed && !(action.kind.id == ACT_SWAP_HANDS && action.swap.code == OP_SH_ONESHOT)) {
        use_oneshot_swaphands();
    }
#    endif
#endif

#ifndef NO_ACTION_ONESHOT
    /* Because we switch layers after a oneshot event, we need to release the
     * key before we leave the layer or no key up event will be generated.
     */
    if (do_release_oneshot && !(get_oneshot_layer_state() & ONESHOT_PRESSED)) {
        record->event.pressed = false;
        layer_on(get_oneshot_layer());
//        process_record(record);
        layer_off(get_oneshot_layer());
    }
#endif
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
//    if (code >= KC_LCTRL && code <= KC_RGUI) {
//        current_mods |= MOD_BIT(code);
//        real_mods |= MOD_BIT(code);
//    }

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

//    // Remove from current mods if it's a modifier
//    if (code >= KC_LCTRL && code <= KC_RGUI) {
//        current_mods &= ~MOD_BIT(code);
//        real_mods &= ~MOD_BIT(code);
//    }

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
    switch (action.kind.id) {
        case ACT_LMODS_TAP:
        case ACT_RMODS_TAP:
        case ACT_LAYER_TAP:
        case ACT_LAYER_TAP_EXT:
            switch (action.layer_tap.code) {
                case KC_NO ... KC_RIGHT_GUI:
                case OP_TAP_TOGGLE:
                case OP_ONESHOT:
                    return true;
            }
            return false;
        case ACT_SWAP_HANDS:
            switch (action.swap.code) {
                case KC_NO ... KC_RIGHT_GUI:
                case OP_SH_TAP_TOGGLE:
                    return true;
            }
            return false;
    }
    return false;
}

// Weak callback hooks (can be overridden)
__attribute__((weak)) void action_keypress(keyevent_t event) {
    // Hook for custom key press handling
}

__attribute__((weak)) void action_keyrelease(keyevent_t event) {
    // Hook for custom key release handling
}

// Function and macro processing (weak implementations)
__attribute__((weak)) void process_record_function(keyrecord_t* record, uint8_t function_id) {
    dprintf("Action: Processing function %d\n", function_id);
}

__attribute__((weak)) void process_record_macro(keyrecord_t* record, uint8_t macro_id) {
    dprintf("Action: Processing macro %d\n", macro_id);
}

/**
 * @brief Process simplified tapping actions
 *
 * Handles layer tap, key tap, and modifier tap without complex state tracking
 *
 * @param record Key record
 * @param action Action to process
 */
void process_action_tapping(keyrecord_t *record, action_t action) {
    keyevent_t event = record->event;

    // Simple tap detection - check if key was released within tapping term
    static uint32_t key_press_time[MATRIX_ROWS][MATRIX_COLS];
    static bool key_was_pressed[MATRIX_ROWS][MATRIX_COLS];
    bool is_tap = false;

    // Track key press time for tap detection
    uint8_t row = event.key.row;
    uint8_t col = event.key.col;

    if (event.pressed) {
        key_press_time[row][col] = timer_read32();
        key_was_pressed[row][col] = true;
    } else {
        // Check if key was held long enough to be a hold
        uint32_t press_duration = timer_read32() - key_press_time[row][col];
        is_tap = (press_duration < TAPPING_TERM);
        key_was_pressed[row][col] = false;
    }

    // Process action based on type
    switch (action.kind.id) {
        /* Layer Tap - MO(layer), TO(layer), TG(layer) */
        case ACT_LAYER_TAP:
        case ACT_LAYER_TAP_EXT:
            switch (action.layer_tap.code) {
                case OP_TAP_TOGGLE:
                    // TG(layer) - tap to toggle layer
                    if (event.pressed) {
                        layer_invert(action.layer_tap.val);
                    }
                    break;

                case OP_ON_OFF:
                    // MO(layer) - press to activate, release to deactivate
                    if (event.pressed) {
                        layer_on(action.layer_tap.val);
                    } else {
                        layer_off(action.layer_tap.val);
                    }
                    break;

                case OP_OFF_ON:
                    // Release to activate
                    if (!event.pressed) {
                        layer_on(action.layer_tap.val);
                    }
                    break;

                case OP_SET_CLEAR:
                    // TO(layer) - tap to switch to layer
                    if (event.pressed) {
                        layer_move(action.layer_tap.val);
                    }
                    break;

                default:
                    // Regular layer tap - tap to send keycode, hold to activate layer
                    if (event.pressed) {
                        if (is_tap) {
                            // Tap: send keycode
                            register_code(action.layer_tap.code);
                        } else {
                            // Hold: activate layer
                            layer_on(action.layer_tap.val);
                        }
                    } else {
                        if (is_tap) {
                            // Release tap: unregister keycode
                            unregister_code(action.layer_tap.code);
                        } else {
                            // Release hold: deactivate layer
                            layer_off(action.layer_tap.val);
                        }
                    }
                    break;
            }
            break;

        /* Modifier Tap - MT(mod, key), LT(layer, key) */
        case ACT_LMODS_TAP:
        case ACT_RMODS_TAP: {
            uint8_t mods = (action.kind.id == ACT_LMODS_TAP) ? action.key.mods : action.key.mods << 4;

            if (event.pressed) {
                if (is_tap) {
                    // Tap: send keycode with mods
                    add_mods(mods);
                    register_code(action.key.code);
                } else {
                    // Hold: activate mods only
                    register_mods(mods);
                }
            } else {
                if (is_tap) {
                    // Release tap: unregister keycode and mods
                    unregister_code(action.key.code);
                    del_mods(mods);
                } else {
                    // Release hold: unregister mods
                    unregister_mods(mods);
                }
            }
        } break;

        /* Regular Key */
        case ACT_LMODS:
        case ACT_RMODS: {
            uint8_t mods = (action.kind.id == ACT_LMODS) ? action.key.mods : action.key.mods << 4;

            if (event.pressed) {
                if (mods) {
                    if (IS_MODIFIER_KEYCODE(action.key.code) || action.key.code == KC_NO) {
                        add_mods(mods);
                    } else {
                        add_weak_mods(mods);
                    }
                }
                register_code(action.key.code);
            } else {
                unregister_code(action.key.code);
                if (mods) {
                    if (IS_MODIFIER_KEYCODE(action.key.code) || action.key.code == KC_NO) {
                        del_mods(mods);
                    } else {
                        del_weak_mods(mods);
                    }
                }
            }
        } break;

        /* Layer Bitwise Operations */
        case ACT_LAYER:
            if (event.pressed) {
                uint8_t shift = action.layer_bitop.part * 4;
                layer_state_t bits = ((layer_state_t)action.layer_bitop.bits) << shift;
                layer_state_t mask = (action.layer_bitop.xbit) ? ~(((layer_state_t)0xf) << shift) : 0;

                switch (action.layer_bitop.op) {
                    case OP_BIT_AND:
                        layer_and(bits | mask);
                        break;
                    case OP_BIT_OR:
                        layer_or(bits | mask);
                        break;
                    case OP_BIT_XOR:
                        layer_xor(bits | mask);
                        break;
                    case OP_BIT_SET:
                        layer_state_set(bits | mask);
                        break;
                }
            }
            break;

        /* Layer Mods - Layer + Mods combination */
        case ACT_LAYER_MODS:
            if (event.pressed) {
                layer_on(action.layer_mods.layer);
                register_mods(action.layer_mods.mods);
            } else {
                unregister_mods(action.layer_mods.mods);
                layer_off(action.layer_mods.layer);
            }
            break;

        default:
            // For other actions, just register/unregister the keycode
            if (event.pressed) {
                register_code(action.key.code);
            } else {
                unregister_code(action.key.code);
            }
            break;
    }
}
