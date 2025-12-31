#pragma once
#include "gpio.h"

// Test product configuration - simplified version
#define PRODUCT_ID 0x0001
#define VENDOR_ID 0x0002
#define MANUFACTURER "Test Manufacturer"
#define PRODUCT "Test Keyboard"
#define DEVICE_VER 0x0100

// Matrix configuration
#define MATRIX_ROWS 8
#define MATRIX_COLS 16
#define MATRIX_IO_DELAY 30
#define DIODE_DIRECTION ROW2COL
#define MATRIX_SCAN_TIMER 5 

// Matrix pins
#define MATRIX_ROW_PINS { A4,A5,A6,A0,A1,A8,A9,B9 }
#define MATRIX_COL_PINS { B5,B8,B17,B16,B15,B14,B13,B12,B3,B4,B2,A7,B7,B18,B1,B6 }

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


// #define NKRO_ENABLE
#define EXTRAKEY_ENABLE

/* Backlit disable timeout when keyboard is disconnected(unit: second) */
#        define DISCONNECTED_BACKLIGHT_DISABLE_TIMEOUT 40

/* Backlit disable timeout when keyboard is connected(unit: second) */
#        define CONNECTED_BACKLIGHT_DISABLE_TIMEOUT 600

#ifndef BT_HOST_DEVICES_COUNT
#    define BT_HOST_DEVICES_COUNT 3
#endif

#define P2P4G_HOST_DEVICES_COUNT 1

#define BAT_LOW_LED_PIN 1
#define BAT_LOW_LED_PIN_ON_STATE 1

//  Uint: Second, the timer restarts on key activities.
#ifndef CONNECTED_IDLE_TIME
#    define CONNECTED_IDLE_TIME 7200
#endif
/* key code*/


#define COMBO_ENABLE

//****touch *****/
#define SDA_PIN A3
#define SCL_PIN A2
#define I2C_CHANNEL 0

/* URAT */
#define UART_TX_PIN A1
#define UART_RX_PIN NO_PIN
#define UART_CHANNEL 0

/**ADC */
#define ADC_PIN B1
#define ADC_CHANNEL 1


#define EEPROM_CUSTOM
#define EEPROM_SIZE 32

/*touch */

#define TOUCH_EN

#define LK_WIRELESS_ENABLE
//#define RGB_MATRIX_ENABLE

/* Communication transport enable flags */
#define USB_ENABLE_FLAG          FALSE
#define BLUETOOTH_ENABLE_FLAG    TRUE
// #define P2P4G_ENABLE_FLAG        TRUE
