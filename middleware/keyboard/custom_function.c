/*
 * Copyright (C) 2024 Keyboard Framework
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "custom_function.h"
#include "timer.h"
#include "print.h"
#include <string.h>
#include "wait.h"
// Tap Dance implementation
#if TAP_DANCE_ENABLE
static tap_dance_state_t tap_dance_state[MATRIX_ROWS][MATRIX_COLS];

void tap_dance_init(void) {
    memset(tap_dance_state, 0, sizeof(tap_dance_state));
    dprintf("Tap Dance: Initialized\n");
}

void tap_dance_action(keyevent_t event, uint8_t tap_count, uint8_t action) {
    dprintf("Tap Dance: Position (%d,%d), taps: %d, action: %d\n",
            event.key.row, event.key.col, tap_count, action);
}

void tap_dance_task(void) {
    uint16_t now = timer_read();

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            tap_dance_state_t* state = &tap_dance_state[row][col];

            if (state->count > 0) {
                uint16_t elapsed = now - state->time;
                if (elapsed > TAPPING_TERM) {
                    // Timeout - execute action with current tap count
                    if (event.pressed) {
                        tap_dance_action(event, state->count, state->count);
                    }
                    state->count = 0;
                }
            }
        }
    }
}

uint8_t tap_dance_get_taps(keypos_t key) {
    if (key.row < MATRIX_ROWS && key.col < MATRIX_COLS) {
        return tap_dance_state[key.row][key.col].count;
    }
    return 0;
}

void tap_dance_reset(keypos_t key) {
    if (key.row < MATRIX_ROWS && key.col < MATRIX_COLS) {
        tap_dance_state[key.row][key.col].count = 0;
    }
}
#endif

// Leader Key implementation
#if LEADER_KEY_ENABLE
static leader_state_t leader_state = {0};

void leader_init(void) {
    memset(&leader_state, 0, sizeof(leader_state));
    dprintf("Leader Key: Initialized\n");
}

void leader_event(keyevent_t event) {
    if (event.pressed) {
        leader_state.active = true;
        leader_state.length = 0;
        leader_state.start_time = timer_read();

        dprintf("Leader Key: Started at %dms\n", leader_state.start_time);
    } else {
        leader_state.active = false;
        dprintf("Leader Key: Ended sequence with %d keys\n", leader_state.length);
    }
}

void leader_task(void) {
    if (!leader_state.active) {
        return;
    }

    uint16_t elapsed = timer_elapsed(leader_state.start_time);
    if (elapsed > LEADER_TIMEOUT) {
        // Timeout - execute the leader sequence
        leader_state.active = false;
        dprintf("Leader Key: Timeout after %dms\n", elapsed);
    }
}

bool leader_is_active(void) {
    return leader_state.active;
}
#endif

// Macro implementation
#if MACRO_ENABLE
#define MAX_MACROS 8
static macro_state_t macro_states[MAX_MACROS];

void macro_init(void) {
    memset(macro_states, 0, sizeof(macro_states));
    dprintf("Macro: Initialized with %d slots\n", MAX_MACROS);
}

void macro_play(uint8_t macro_id) {
    if (macro_id >= MAX_MACROS) {
        dprintf("Macro: ERROR - Invalid macro ID %d\n", macro_id);
        return;
    }

    // This would load macro from storage and start playing
    macro_states[macro_id].playing = true;
    macro_states[macro_id].current_step = 0;
    macro_states[macro_id].start_time = timer_read();

    dprintf("Macro: Playing macro %d\n", macro_id);
}

void macro_stop(uint8_t macro_id) {
    if (macro_id >= MAX_MACROS) {
        return;
    }

    macro_states[macro_id].playing = false;
    dprintf("Macro: Stopped macro %d\n", macro_id);
}

void macro_task(void) {
    uint16_t now = timer_read();

    for (uint8_t i = 0; i < MAX_MACROS; i++) {
        if (macro_states[i].playing && macro_states[i].steps) {
            macro_state_t* macro = &macro_states[i];

            if (macro->current_step < macro->length) {
                const macro_step_t* step = &macro->steps[macro->current_step];
                uint16_t elapsed = now - macro->start_time;

                if (elapsed >= step->delay) {
                    // Execute this step
                    if (step->keycode != 0) {
                        register_code(step->keycode & 0xFF);
                        wait_ms(10);
                        unregister_code(step->keycode & 0xFF);
                    }

                    macro->current_step++;
                    macro->start_time = now;
                }
            } else {
                // Macro complete
                macro->playing = false;
                dprintf("Macro: Completed macro %d\n", i);
            }
        }
    }
}

bool macro_is_playing(uint8_t macro_id) {
    if (macro_id >= MAX_MACROS) {
        return false;
    }

    return macro_states[macro_id].playing;
}
#endif

// User callback hooks (weak implementations)
bool process_record_user(keyrecord_t* record, uint16_t keycode) {
    // User can override this to handle custom keycodes
    return true; // Continue processing
}

bool process_record_quantum(keyrecord_t* record, uint16_t keycode) {
    // User can override this to handle quantum keycodes
    return true; // Continue processing
}
