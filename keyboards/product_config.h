#pragma once

// Test product configuration - simplified version
#define PRODUCT_ID 0x0001
#define VENDOR_ID 0x0001
#define MANUFACTURER "Test Manufacturer"
#define PRODUCT "Test Keyboard"
#define DEVICE_VER 0x0100

// Matrix configuration
#define MATRIX_ROWS 6
#define MATRIX_COLS 13
#define MATRIX_IO_DELAY 30
#define DIODE_DIRECTION ROW2COL

// Matrix pins
#define MATRIX_ROW_PINS { 0, 1, 2, 3, 4, 5 }
#define MATRIX_COL_PINS { 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 }

// No direct pins for this test
// #define DIRECT_PINS {}

// Matrix input configuration
#define MATRIX_INPUT_PRESSED_STATE 0
// #define MATRIX_HAS_GHOST

// Debounce configuration
#define DEBOUNCE 5
#define DEBOUNCE_ALGORITHM DEBOUNCE_SYM_DEFER_PK
#define DEBOUNCE_DELAY 5
#define MAX_MATRIX_ROWS 16

// USB configuration
#define USB_MAX_POWER_CONSUMPTION 500
#define USB_POLLING_INTERVAL_MS 10
#define USB_SUSPEND_WAKEUP_DELAY 0

// Pin definitions
#ifdef NO_PIN
#define NO_PIN_DEFINED
#else
#define NO_PIN 0xFF
#endif

// Tap configuration
#define TAPPING_TERM 200
#define TAPPING_TOGGLE 2
#define QUICK_TAP_TERM 100

// Oneshot configuration
#define ONESHOT_TIMEOUT 300
#define ONESHOT_TAP_TOGGLE 2

// Combo configuration
#define COMBO_TERM 200
#define COMBO_MOD_TERM 200

// Code delay configuration
#define TAP_CODE_DELAY 100
#define TAP_HOLD_CAPS_DELAY 80

// Key override configuration
#define KEY_OVERRIDE_REPEAT_DELAY 500

// Leader configuration
#define LEADER_TIMEOUT 300

// Feature toggles
#define ENABLE_COMPILE_KEYCODE
