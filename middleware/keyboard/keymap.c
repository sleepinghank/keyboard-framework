// middleware/keyboard/keymap.c
#include "keymap.h"
#include "keycode.h"
#include "debug.h"
layer_state_t g_layer_state;

void keymap_init(void) {
    g_layer_state.current_layer = LAYER_MACOS;
    g_layer_state.base_layer = LAYER_BASE;
    g_layer_state.mo_stack_top = 0;
}

uint16_t keymap_get_keycode(uint8_t row, uint8_t col) {
    if (row >= MATRIX_ROWS || col >= MATRIX_COLS) {
        return KC_NO;
    }

    layer_id_t layer = g_layer_state.current_layer;
    uint16_t keycode = keymap_layers[layer][row][col];
    // 透明键回退到基础层
    if (keycode == KC_TRANSPARENT && layer != g_layer_state.base_layer) {
        keycode = keymap_layers[g_layer_state.base_layer][row][col];
    }

    return keycode;
}

// TG: 切换基础层
void layer_set(layer_id_t layer) {
    if (layer >= LAYER_MAX) return;

    g_layer_state.base_layer = layer;
    if (g_layer_state.mo_stack_top == 0) {
        g_layer_state.current_layer = layer;
    }
}

// MO: 临时激活层（压栈）
void layer_on(layer_id_t layer) {
    if (layer >= LAYER_MAX) return;

    if (g_layer_state.mo_stack_top < 4) {
        g_layer_state.mo_stack[g_layer_state.mo_stack_top++] = layer;
        g_layer_state.current_layer = layer;
    }
}

// MO: 取消临时层（出栈）
void layer_off(layer_id_t layer) {
    // 从栈中移除指定层
    for (int i = g_layer_state.mo_stack_top - 1; i >= 0; i--) {
        if (g_layer_state.mo_stack[i] == layer) {
            // 移除该元素
            for (int j = i; j < g_layer_state.mo_stack_top - 1; j++) {
                g_layer_state.mo_stack[j] = g_layer_state.mo_stack[j + 1];
            }
            g_layer_state.mo_stack_top--;
            break;
        }
    }

    // 更新当前层
    if (g_layer_state.mo_stack_top > 0) {
        g_layer_state.current_layer = g_layer_state.mo_stack[g_layer_state.mo_stack_top - 1];
    } else {
        g_layer_state.current_layer = g_layer_state.base_layer;
    }
}

layer_id_t layer_get_current(void) {
    return g_layer_state.current_layer;
}
