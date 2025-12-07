/*
 * Keyboard Framework - Example Usage
 *
 * This file demonstrates how to use the keyboard framework
 * for cross-platform keyboard development
 */

#include "keyboard_framework.h"
#include "timer.h"

// Define your keymap
const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // Layer 0 - Base layer
    [0] = {
        {KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL},
        {KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC},
        {KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_ENT},
        {KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT, KC_NO},
        {KC_LCTL, KC_LGUI, KC_LALT, KC_SPACE,KC_SPACE,KC_SPACE,KC_RALT, KC_RGUI, KC_APP,  KC_RCTL, KC_NO,   KC_NO,   KC_NO}
    },
    // Layer 1 - Function layer
    [1] = {
        {KC_GRV,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12},
        {KC_TRNS, KC_TRNS, KC_UP,   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_DEL},
        {KC_TRNS, KC_LEFT, KC_DOWN, KC_RIGHT,KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS},
        {KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS},
        {KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS}
    }
};

const uint8_t num_keymaps = 2;

// Example combo definitions
#ifdef COMBO_ENABLE
const combo_t COMBO[] = {
    // Ctrl+C
    {
        .key = {{0, 0}, {1, 0}},  // Position of Ctrl and C
        .keycode = KC_C,
        .count = 2
    },
    // Ctrl+V
    {
        .key = {{0, 0}, {1, 2}},  // Position of Ctrl and V
        .keycode = KC_V,
        .count = 2
    }
};

const uint16_t COMBO_COUNT = sizeof(COMBO) / sizeof(COMBO[0]);
#endif

// Example custom keycode handling
bool process_record_user(action_record_t* record, uint16_t keycode) {
    switch (keycode) {
        case KC_CAPS:
            // Custom Caps Lock handling
            if (record->event.pressed) {
                // Toggle caps lock LED
                // led_toggle_caps_lock();
            }
            break;

        case KC_LGUI:
            // Custom GUI key handling
            break;

        default:
            break;
    }
    return true; // Continue processing
}

// Example quantum keycode handling
bool process_record_quantum(action_record_t* record, uint16_t keycode) {
    switch (keycode) {
        case QK_TOGGLE_LAYER(1):
            // Custom layer toggle handling
            if (record->event.pressed) {
                dprintf("Toggling to layer 1\n");
            }
            break;

        case QK_FUNCTION(0):
            // Custom function 0
            if (record->event.pressed) {
                dprintf("Executing custom function 0\n");
                // Perform custom action
            }
            break;

        default:
            break;
    }
    return true; // Continue processing
}

// Example tap dance
#if TAP_DANCE_ENABLE
void tap_dance_fn(uint8_t row, uint8_t col, uint8_t tap_count, uint8_t action) {
    switch (tap_count) {
        case 1:
            tap_code(KC_A);
            break;
        case 2:
            tap_code(KC_B);
            break;
        case 3:
            tap_code(KC_C);
            break;
        default:
            break;
    }
}
#endif

// Example leader key sequence
#if LEADER_KEY_ENABLE
void leader_end(void) {
    if (leader_sequence_one_key(KC_S)) {
        tap_code(KC_LCTL);
        tap_code(KC_S);
    } else if (leader_sequence_two_keys(KC_D, KC_D)) {
        tap_code(KC_ESC);
    }
}
#endif

// Example macro
#if MACRO_ENABLE
const macro_step_t macro_save[] = {
    {KC_LCTRL, 0},
    {KC_S, 10},
    {KC_ENTER, 0}
};

const macro_step_t macro_copy[] = {
    {KC_LCTRL, 0},
    {KC_C, 0},
    {KC_LCTRL, 0}
};
#endif

// Main application example
int main(void) {
    // Initialize the keyboard framework
    KEYBOARD_FRAMEWORK_INIT();

    // Set default layer
    default_layer_set(LAYER_BIT(0));

    // Main loop
    while (1) {
        // Run keyboard framework task
        KEYBOARD_FRAMEWORK_TASK();

        // Additional application-specific tasks can go here
        // For example:
        // - LED updates
        // - Display updates
        // - USB handling
        // - Power management

        // Small delay to prevent excessive CPU usage
        wait_ms(1);
    }

    return 0;
}

// Platform-specific initialization examples

#ifdef PLATFORM_CHIPSTM32
void matrix_init(void) {
    // STM32-specific matrix initialization
    platform_gpio_init_stm32();
    platform_timer_init_stm32();
}

void matrix_scan(void) {
    // STM32-specific matrix scanning
    platform_matrix_scan_stm32(matrix);
}
#endif

#ifdef PLATFORM_CHIPWCH
void matrix_init(void) {
    // WCH CH58x-specific matrix initialization
    platform_gpio_init_wch();
    platform_timer_init_wch();
}

void matrix_scan(void) {
    // WCH CH58x-specific matrix scanning
    platform_matrix_scan_wch(matrix);
}
#endif

#ifdef PLATFORM_NORDIC
void matrix_init(void) {
    // Nordic nRF5x-specific matrix initialization
    platform_gpio_init_nordic();
    platform_timer_init_nordic();
}

void matrix_scan(void) {
    // Nordic nRF5x-specific matrix scanning
    platform_matrix_scan_nordic(matrix);
}
#endif
