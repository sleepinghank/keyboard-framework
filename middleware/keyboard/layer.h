/*
 * Copyright (C) 2024 Keyboard Framework
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Layer state type definitions
#if defined(LAYER_STATE_8BIT)
typedef uint8_t layer_state_t;
#define MAX_LAYER_BITS 3
#define MAX_LAYER_COUNT 8
#elif defined(LAYER_STATE_16BIT)
typedef uint16_t layer_state_t;
#define MAX_LAYER_BITS 4
#define MAX_LAYER_COUNT 16
#elif defined(LAYER_STATE_32BIT)
typedef uint32_t layer_state_t;
#define MAX_LAYER_BITS 5
#define MAX_LAYER_COUNT 32
#else
#error "Layer state type not defined"
#endif

// Layer mask helpers
#define LAYER_BIT(layer) ((layer_state_t)1 << (layer))
#define LAYER_MASK_ALL ((layer_state_t)((1 << MAX_LAYER_COUNT) - 1))

// Layer management functions
void layer_init(void);
void layer_clear(void);
void layer_on(uint8_t layer);
void layer_off(uint8_t layer);
void layer_invert(uint8_t layer);
layer_state_t layer_state_get(void);
void layer_state_set(layer_state_t state);
layer_state_t layer_state_or(layer_state_t state);
layer_state_t layer_state_and(layer_state_t state);
layer_state_t layer_state_xor(layer_state_t state);
bool layer_state_cmp(layer_state_t state, uint8_t layer);
bool layer_state_is(uint8_t layer);
uint8_t get_highest_layer(layer_state_t state);

/**
 * @brief Get the current keycode for a given position
 *
 * This function looks up the keycode for a given matrix position.
 * It searches through active layers from highest to lowest priority.
 *
 * @param layer Current active layer state
 * @param row Matrix row (0-indexed)
 * @param col Matrix column (0-indexed)
 * @return The keycode at the given position
 */
uint16_t keymap_key_to_keycode(layer_state_t layer, uint8_t row, uint8_t col);

/**
 * @brief Default layer management
 *
 * Set the default layer (base layer that is always active)
 */
void default_layer_set(layer_state_t state);
layer_state_t default_layer_state_get(void);

// Callback hooks for customization
layer_state_t layer_state_set_user(layer_state_t state);
layer_state_t layer_state_set_kb(layer_state_t state);

/**
 * @brief Update tri-layer state
 *
 * Activate layer3 when both layer1 and layer2 are active
 *
 * @param layer1 First layer to check
 * @param layer2 Second layer to check
 * @param layer3 Layer to activate when both 1 and 2 are active
 */
void update_tri_layer(uint8_t layer1, uint8_t layer2, uint8_t layer3);

/**
 * @brief Update tri-layer state from bitmask
 *
 * @param state Current layer state
 * @param layer1 First layer to check
 * @param layer2 Second layer to check
 * @param layer3 Layer to activate when both 1 and 2 are active
 * @return Modified layer state with tri-layer applied
 */
layer_state_t update_tri_layer_state(layer_state_t state, uint8_t layer1, uint8_t layer2, uint8_t layer3);

// Debug functions
void layer_debug(void);

#ifdef __cplusplus
}
#endif
