// middleware/keyboard/keymap.h
#ifndef __KEYMAP_H__
#define __KEYMAP_H__

#include <stdint.h>
#include "kb904/config_product.h"

// 层类型
typedef enum {
    LAYER_BASE = 0,
    LAYER_MACOS,
    LAYER_WINDOWS,
    LAYER_MAX
} layer_id_t;

// 层状态
typedef struct {
    layer_id_t current_layer;       // 当前激活层（唯一）
    layer_id_t base_layer;          // 基础层（TG 切换的目标）
    layer_id_t mo_stack[4];         // MO 层栈
    uint8_t    mo_stack_top;        // 栈顶索引
} layer_state_t;

// 层状态全局变量
extern layer_state_t g_layer_state;

// 层级映射表（需要在产品配置中定义）
extern const uint16_t keymap_layers[LAYER_MAX][MATRIX_ROWS][MATRIX_COLS];

// 初始化
void keymap_init(void);

// 获取当前层的键码
uint16_t keymap_get_keycode(uint8_t row, uint8_t col);

// 层切换 API
void layer_set(layer_id_t layer);       // TG: 切换基础层
void layer_on(layer_id_t layer);        // MO: 临时激活层
void layer_off(layer_id_t layer);       // MO: 取消临时层
layer_id_t layer_get_current(void);     // 获取当前层

#endif
