/* Copyright 2021 QMK
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pwm.h"
#include "pin_defs.h"
#include "pin_mapper.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

// 调试输出宏
#define PWM_DEBUG_PRINT(fmt, ...) printf("[PWM] " fmt "\n", ##__VA_ARGS__)

// PWM channel state structure
typedef struct {
    pin_t pin;              // GPIO pin bound to this channel
    pin_t enable_pin;       // Enable pin (optional, can be NO_PIN)
    bool initialized;       // Channel initialization flag
    bool enabled;           // Channel enabled flag
    uint8_t duty_cycle;     // Current duty cycle (0-100)
    uint32_t frequency;     // Current frequency in Hz
} pwm_channel_state_t;

// PWM channel state array - use uint8_t for channel index (0-7)
static pwm_channel_state_t pwm_channels[8] = {0};

/*==========================================
 * PWM初始化和基础功能
 *=========================================*/

void pwm_init(void) {
    int i;
    for (i = 0; i < 8; i++) {
        pwm_channels[i].initialized = false;
        pwm_channels[i].enabled = false;
        pwm_channels[i].pin = NO_PIN;
        pwm_channels[i].enable_pin = NO_PIN;
        pwm_channels[i].duty_cycle = 0;
        pwm_channels[i].frequency = 1000; // Default 1KHz
    }
    PWM_DEBUG_PRINT("PWM initialized - %d channels available", 8);
}

void pwm_start(pwm_channel_t channel) {
    int i;
    // Iterate through all 8 possible channels
    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            if (!pwm_channels[i].initialized) {
                PWM_DEBUG_PRINT("pwm_start: Channel %d not initialized", i);
                continue;
            }
            if (pwm_channels[i].pin == NO_PIN) {
                PWM_DEBUG_PRINT("pwm_start: Channel %d not bound to any pin", i);
                continue;
            }
            pwm_channels[i].enabled = true;
            PWM_DEBUG_PRINT("pwm_start: Started channel %d on pin 0x%02X", i, pwm_channels[i].pin);
        }
    }
}

void pwm_stop(pwm_channel_t channel) {
    int i;
    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            pwm_channels[i].enabled = false;
            PWM_DEBUG_PRINT("pwm_stop: Stopped channel %d", i);
        }
    }
}

void pwm_set_frequency(pwm_channel_t channel, uint32_t frequency) {
    int i;
    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            pwm_channels[i].frequency = frequency;
            PWM_DEBUG_PRINT("pwm_set_frequency: Channel %d frequency set to %lu Hz", i, frequency);
        }
    }
}

void pwm_set_duty_cycle(pwm_channel_t channel, uint8_t duty_cycle) {
    int i;
    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            pwm_channels[i].duty_cycle = duty_cycle;
            PWM_DEBUG_PRINT("pwm_set_duty_cycle: Channel %d duty cycle set to %d%%", i, duty_cycle);
        }
    }
}

void pwm_set_duty_percentage(pwm_channel_t channel, float percentage) {
    int i;
    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            pwm_channels[i].duty_cycle = (uint8_t)percentage;
            PWM_DEBUG_PRINT("pwm_set_duty_percentage: Channel %d duty cycle set to %.1f%%", i, percentage);
        }
    }
}

uint8_t pwm_get_duty_cycle(pwm_channel_t channel) {
    int i;
    // For multi-channel requests, return the value of the first set bit
    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            return pwm_channels[i].duty_cycle;
        }
    }
    PWM_DEBUG_PRINT("pwm_get_duty_cycle: Invalid channel 0x%02X", channel);
    return 0;
}

float pwm_get_duty_percentage(pwm_channel_t channel) {
    int i;
    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            return (float)pwm_channels[i].duty_cycle;
        }
    }
    PWM_DEBUG_PRINT("pwm_get_duty_percentage: Invalid channel 0x%02X", channel);
    return 0.0f;
}

void pwm_set_pulse_width(pwm_channel_t channel, uint32_t pulse_width_ns) {
    int i;
    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            uint32_t period_ns = 1000000000UL / pwm_channels[i].frequency; // Calculate period from frequency
            uint32_t duty_ns = (period_ns * pwm_channels[i].duty_cycle) / 100;
            PWM_DEBUG_PRINT("pwm_set_pulse_width: Channel %d pulse width set to %lu ns (period: %lu ns, duty: %lu ns)",
                            i, pulse_width_ns, period_ns, duty_ns);
        }
    }
}

void pwm_set_period(pwm_channel_t channel, uint32_t period_ns) {
    int i;
    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            pwm_channels[i].frequency = 1000000000UL / period_ns; // Calculate frequency from period
            PWM_DEBUG_PRINT("pwm_set_period: Channel %d period set to %lu ns (frequency: %lu Hz)",
                            i, period_ns, pwm_channels[i].frequency);
        }
    }
}

void pwm_set_polarity(pwm_channel_t channel, uint8_t polarity) {
    int i;
    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            PWM_DEBUG_PRINT("pwm_set_polarity: Channel %d polarity set to %d (%s)",
                            i, polarity, polarity ? "Inverted" : "Normal");
        }
    }
}

void pwm_enable(pwm_channel_t channel) {
    int i;
    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            if (pwm_channels[i].enable_pin != NO_PIN) {
                // TODO: Set enable pin to high level if there's a dedicated enable pin
                PWM_DEBUG_PRINT("pwm_enable: Channel %d enable pin 0x%02X activated", i, pwm_channels[i].enable_pin);
            }
            pwm_channels[i].initialized = true;
            PWM_DEBUG_PRINT("pwm_enable: Enabled channel %d", i);
        }
    }
}

void pwm_disable(pwm_channel_t channel) {
    int i;
    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            if (pwm_channels[i].enable_pin != NO_PIN) {
                // TODO: Set enable pin to low level if there's a dedicated enable pin
                PWM_DEBUG_PRINT("pwm_disable: Channel %d enable pin 0x%02X deactivated", i, pwm_channels[i].enable_pin);
            }
            pwm_channels[i].initialized = false;
            PWM_DEBUG_PRINT("pwm_disable: Disabled channel %d", i);
        }
    }
}

void pwm_configure(pwm_channel_t channel, uint32_t frequency, uint8_t duty_cycle) {
    int i;
    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            pwm_channels[i].frequency = frequency;
            pwm_channels[i].duty_cycle = duty_cycle;
            pwm_channels[i].initialized = true;
            PWM_DEBUG_PRINT("pwm_configure: Channel %d configured - Frequency: %lu Hz, Duty: %d%%",
                            i, frequency, duty_cycle);
        }
    }
}

/*==========================================
 * GPIO引脚绑定功能
 *=========================================*/

bool pwm_bind_pin(pin_t pin, pwm_channel_t channel) {
    int i;
    // Find the first channel bit that's set
    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            // Check if this pin is already bound to another channel
            int j;
            for (j = 0; j < 8; j++) {
                if (pwm_channels[j].pin == pin) {
                    PWM_DEBUG_PRINT("pwm_bind_pin: Pin 0x%02X already bound to channel %d", pin, j);
                    return false;
                }
            }

            // Bind the pin to this channel
            pwm_channels[i].pin = pin;
            pwm_channels[i].initialized = true;

            PWM_DEBUG_PRINT("pwm_bind_pin: Bound pin 0x%02X to channel %d", pin, i);

            // TODO: In a real implementation, you would:
            // 1. Configure the GPIO pin as PWM output
            // 2. Configure the PWM peripheral
            // 3. Initialize the PWM channel if not already done

            return true;
        }
    }

    PWM_DEBUG_PRINT("pwm_bind_pin: Invalid channel 0x%02X", channel);
    return false;
}

bool pwm_unbind_pin(pin_t pin) {
    int i;
    for (i = 0; i < 8; i++) {
        if (pwm_channels[i].pin == pin) {
            pwm_channels[i].pin = NO_PIN;
            pwm_channels[i].initialized = false;
            pwm_channels[i].enabled = false;
            PWM_DEBUG_PRINT("pwm_unbind_pin: Unbound pin 0x%02X from channel %d", pin, i);
            return true;
        }
    }

    PWM_DEBUG_PRINT("pwm_unbind_pin: Pin 0x%02X not bound to any channel", pin);
    return false;
}

pin_t pwm_get_bound_pin(pwm_channel_t channel) {
    int i;
    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            return pwm_channels[i].pin;
        }
    }

    PWM_DEBUG_PRINT("pwm_get_bound_pin: Invalid channel 0x%02X", channel);
    return NO_PIN;
}

bool pwm_is_bound(pwm_channel_t channel) {
    int i;
    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            return (pwm_channels[i].pin != NO_PIN);
        }
    }

    PWM_DEBUG_PRINT("pwm_is_bound: Invalid channel 0x%02X", channel);
    return false;
}

bool pwm_bind_enable_pin(pin_t enable_pin, pwm_channel_t channel) {
    int i;
    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            // Check if this enable pin is already bound to another channel
            int j;
            for (j = 0; j < 8; j++) {
                if (pwm_channels[j].enable_pin == enable_pin) {
                    PWM_DEBUG_PRINT("pwm_bind_enable_pin: Enable pin 0x%02X already bound to channel %d", enable_pin, j);
                    return false;
                }
            }

            // Bind the enable pin to this channel
            pwm_channels[i].enable_pin = enable_pin;

            PWM_DEBUG_PRINT("pwm_bind_enable_pin: Bound enable pin 0x%02X to channel %d", enable_pin, i);

            // TODO: In a real implementation, you would configure the enable pin as GPIO output

            return true;
        }
    }

    PWM_DEBUG_PRINT("pwm_bind_enable_pin: Invalid channel 0x%02X", channel);
    return false;
}

pin_t pwm_get_bound_enable_pin(pwm_channel_t channel) {
    int i;
    for (i = 0; i < 8; i++) {
        if (channel & (1 << i)) {
            return pwm_channels[i].enable_pin;
        }
    }

    PWM_DEBUG_PRINT("pwm_get_bound_enable_pin: Invalid channel 0x%02X", channel);
    return NO_PIN;
}
