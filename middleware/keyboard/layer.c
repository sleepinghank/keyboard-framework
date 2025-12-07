/*
 * Copyright (C) 2024 Keyboard Framework
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "layer.h"
#include "print.h"
#include <string.h>

// Layer state
static layer_state_t layer_state = 0;
static layer_state_t default_layer_state = 0;

// External keymap data
extern const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS];
extern const uint8_t  num_keymaps;

// External weak callback hooks
__attribute__((weak)) layer_state_t layer_state_set_user(layer_state_t state) {
    return state;
}

__attribute__((weak)) layer_state_t layer_state_set_kb(layer_state_t state) {
    return layer_state_set_user(state);
}

/**
 * @brief Initialize layer system
 */
void layer_init(void) {
    layer_state = 0;
    dprintf("Layer: Initialized\n");
}

/**
 * @brief Clear all layers
 */
void layer_clear(void) {
    layer_state = 0;
    dprintf("Layer: Cleared\n");
}

/**
 * @brief Activate a layer
 *
 * @param layer Layer number (0-indexed)
 */
void layer_on(uint8_t layer) {
    if (layer >= MAX_LAYER_COUNT) {
        dprintf("Layer: ERROR - Invalid layer %d\n", layer);
        return;
    }

    layer_state |= LAYER_BIT(layer);
    dprintf("Layer: Turned on layer %d\n", layer);
}

/**
 * @brief Deactivate a layer
 *
 * @param layer Layer number (0-indexed)
 */
void layer_off(uint8_t layer) {
    if (layer >= MAX_LAYER_COUNT) {
        dprintf("Layer: ERROR - Invalid layer %d\n", layer);
        return;
    }

    layer_state &= ~LAYER_BIT(layer);
    dprintf("Layer: Turned off layer %d\n", layer);
}

/**
 * @brief Toggle a layer
 *
 * @param layer Layer number (0-indexed)
 */
void layer_invert(uint8_t layer) {
    if (layer >= MAX_LAYER_COUNT) {
        dprintf("Layer: ERROR - Invalid layer %d\n", layer);
        return;
    }

    layer_state ^= LAYER_BIT(layer);
    dprintf("Layer: Toggled layer %d\n", layer);
}

/**
 * @brief Get current layer state
 *
 * @return Current layer state bitmask
 */
layer_state_t layer_state_get(void) {
    return layer_state;
}

/**
 * @brief Set layer state
 *
 * @param state New layer state
 */
void layer_state_set(layer_state_t state) {
    layer_state = state;
    dprintf("Layer: State set to 0x%04X\n", state);
}

/**
 * @brief OR operation with layer state
 *
 * @param state Layer mask to OR
 * @return New layer state
 */
layer_state_t layer_state_or(layer_state_t state) {
    layer_state |= state;
    return layer_state;
}

/**
 * @brief AND operation with layer state
 *
 * @param state Layer mask to AND
 * @return New layer state
 */
layer_state_t layer_state_and(layer_state_t state) {
    layer_state &= state;
    return layer_state;
}

/**
 * @brief XOR operation with layer state
 *
 * @param state Layer mask to XOR
 * @return New layer state
 */
layer_state_t layer_state_xor(layer_state_t state) {
    layer_state ^= state;
    return layer_state;
}

/**
 * @brief Compare layer state with specific layer
 *
 * @param state Layer state to check
 * @param layer Layer number
 * @return true if layer is active
 */
bool layer_state_cmp(layer_state_t state, uint8_t layer) {
    if (layer >= MAX_LAYER_COUNT) {
        return false;
    }
    return (state & LAYER_BIT(layer)) != 0;
}

/**
 * @brief Check if a specific layer is active in current state
 *
 * @param layer Layer number
 * @return true if layer is active
 */
bool layer_state_is(uint8_t layer) {
    return layer_state_cmp(layer_state, layer);
}

/**
 * @brief Get highest active layer
 *
 * @param state Layer state to check
 * @return Layer number of highest active layer
 */
uint8_t get_highest_layer(layer_state_t state) {
    // Find the highest set bit
    for (int i = MAX_LAYER_COUNT - 1; i >= 0; i--) {
        if (state & LAYER_BIT(i)) {
            return i;
        }
    }
    return 0;
}

/**
 * @brief Set default layer
 *
 * @param state Default layer state
 */
void default_layer_set(layer_state_t state) {
    default_layer_state = state;
    dprintf("Layer: Default layer set to 0x%04X\n", state);
}

/**
 * @brief Get default layer state
 *
 * @return Default layer state
 */
layer_state_t default_layer_state_get(void) {
    return default_layer_state;
}

/**
 * @brief Convert matrix position to keycode
 *
 * This function searches through layers to find the keycode for a given position.
 * It starts with the highest active layer and works down.
 *
 * @param layer Current layer state
 * @param row Matrix row
 * @param col Matrix column
 * @return Keycode at position
 */
uint16_t keymap_key_to_keycode(layer_state_t layer, uint8_t row, uint8_t col) {
    // Validate parameters
    if (row >= MATRIX_ROWS || col >= MATRIX_COLS) {
        dprintf("Keymap: ERROR - Invalid position (%d, %d)\n", row, col);
        return KC_NO;
    }

    // Combine layer state with default layer
    layer_state_t search_state = layer | default_layer_state;

    // Search from highest layer to lowest
    for (int8_t current_layer = MAX_LAYER_COUNT - 1; current_layer >= 0; current_layer--) {
        if (search_state & LAYER_BIT(current_layer)) {
            // Check if keymap exists for this layer
            if (current_layer < num_keymaps) {
                uint16_t keycode = keymaps[current_layer][row][col];

                // Skip transparent keys (they pass through to lower layers)
                if (keycode != KC_TRANSPARENT) {
                    return keycode;
                }
            }
        }
    }

    // Fallback - return KC_NO if nothing found
    return KC_NO;
}

/**
 * @brief Update tri-layer state
 *
 * Activates layer3 when both layer1 and layer2 are active
 *
 * @param layer1 First layer to check
 * @param layer2 Second layer to check
 * @param layer3 Layer to activate if both 1 and 2 are active
 */
void update_tri_layer(uint8_t layer1, uint8_t layer2, uint8_t layer3) {
    if (layer1 >= MAX_LAYER_COUNT || layer2 >= MAX_LAYER_COUNT || layer3 >= MAX_LAYER_COUNT) {
        dprintf("Tri-layer: ERROR - Invalid layer numbers\n");
        return;
    }

    bool layer1_active = layer_state_is(layer1);
    bool layer2_active = layer_state_is(layer2);

    if (layer1_active && layer2_active) {
        layer_on(layer3);
        dprintf("Tri-layer: Activated layer %d\n", layer3);
    } else {
        layer_off(layer3);
        dprintf("Tri-layer: Deactivated layer %d\n", layer3);
    }
}

/**
 * @brief Update tri-layer state from bitmask
 *
 * @param state Current layer state
 * @param layer1 First layer to check
 * @param layer2 Second layer to check
 * @param layer3 Layer to activate if both 1 and 2 are active
 * @return Modified layer state
 */
layer_state_t update_tri_layer_state(layer_state_t state, uint8_t layer1, uint8_t layer2, uint8_t layer3) {
    if (layer1 >= MAX_LAYER_COUNT || layer2 >= MAX_LAYER_COUNT || layer3 >= MAX_LAYER_COUNT) {
        dprintf("Tri-layer: ERROR - Invalid layer numbers\n");
        return state;
    }

    bool layer1_active = state & LAYER_BIT(layer1);
    bool layer2_active = state & LAYER_BIT(layer2);

    if (layer1_active && layer2_active) {
        state |= LAYER_BIT(layer3);
    } else {
        state &= ~LAYER_BIT(layer3);
    }

    return state;
}

/**
 * @brief Debug layer state
 */
void layer_debug(void) {
    dprintf("\nLayer Debug:\n");
    dprintf("  Current State: 0x%04X\n", layer_state);
    dprintf("  Default State: 0x%04X\n", default_layer_state);
    dprintf("  Active Layers:");

    for (int i = 0; i < MAX_LAYER_COUNT; i++) {
        if (layer_state & LAYER_BIT(i)) {
            dprintf(" %d", i);
        }
    }

    dprintf("\n  Default Layers:");
    for (int i = 0; i < MAX_LAYER_COUNT; i++) {
        if (default_layer_state & LAYER_BIT(i)) {
            dprintf(" %d", i);
        }
    }
    dprintf("\n");
}
