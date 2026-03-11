// middleware/keyboard/keyboard.c
#include "keyboard.h"
#include "keymap.h"
#include "matrix.h"
#include "debounce.h"
#include "report.h"
#include "report_buffer.h"
#include "keycode.h"
#include <string.h>

// 按键列表（供 process_combo.c 使用）
list_t* _key_code_list = NULL;
list_t* _key_code_list_extend = NULL;

// 上一次矩阵状态（防抖后）
static matrix_row_t matrix_previous[MATRIX_ROWS];

// 当前防抖后的矩阵状态
static matrix_row_t matrix_debounced[MATRIX_ROWS];

// 上一次更新状态
static key_update_st_t last_update_state = NO_KEY_UPDATE;

// 内部函数声明
static key_update_st_t scan_and_debounce(void);
static void process_layer_switch_key(void);
static void update_key_code_list(void);

void keyboard_init(void) {
    // 初始化矩阵
    matrix_init();

    // 初始化防抖
    debounce_init(MATRIX_ROWS);

    // 初始化层级
    keymap_init();

    // 初始化链表
    _key_code_list = make_list_proc();
    _key_code_list_extend = make_list_proc();

    // 初始化组合键
    combo_init();

    // 初始化报告
    report_init();

    // 清空矩阵状态
    memset(matrix_previous, 0, sizeof(matrix_previous));
    memset(matrix_debounced, 0, sizeof(matrix_debounced));
}

void keyboard_task(void) {
    // 1. 矩阵扫描 + 驱动层防抖
    key_update_st_t key_st = scan_and_debounce();
    last_update_state = key_st;

    // 2. 幽灵键直接返回
    if (key_st == GHOST_KEY) {
        return;
    }

    // 3. 优先处理层切换键（解决同时按下时序问题）
    if (key_st == KEY_UPDATE) {
        dprintf("Key update detected, processing layer switch keys first\r\n");
        process_layer_switch_key();
    }

    // 4. 更新按键列表（基于防抖后的矩阵变化）
    if (key_st == KEY_UPDATE) {
        update_key_code_list();
    }

    // 5. 组合键处理
    combo_task(key_st);

    // 6. 生成并发送 HID 报告
    report_update_proc(key_st);

    // 7. 清空扩展键列表
    del_all_child(_key_code_list_extend);
}

key_update_st_t keyboard_get_last_update_state(void) {
    return last_update_state;
}

// 矩阵扫描 + 驱动层防抖处理
static key_update_st_t scan_and_debounce(void) {
    // 调用驱动层矩阵扫描
    bool raw_changed = matrix_scan();

    // 获取原始矩阵数据
    static matrix_row_t raw_matrix[MATRIX_ROWS];
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        raw_matrix[row] = matrix_get_row(row);
    }

    // 调用驱动层防抖处理
    bool debounced_changed = debounce(raw_matrix, matrix_debounced, MATRIX_ROWS, raw_changed);

    if (!debounced_changed) {
        return NO_KEY_UPDATE;
    }

    // TODO: 可选的幽灵键检测
    // if (detect_ghost_key(matrix_debounced)) {
    //     return GHOST_KEY;
    // }

    return KEY_UPDATE;
}

// 优先处理层切换键（Fn 键）
// 在其他键码解析前执行，确保层状态正确
static void process_layer_switch_key(void) {
#if defined(FN_KEY_ROW) && defined(FN_KEY_COL)
    matrix_row_t current = matrix_debounced[FN_KEY_ROW];
    matrix_row_t previous = matrix_previous[FN_KEY_ROW];
    matrix_row_t fn_mask = (matrix_row_t)1 << FN_KEY_COL;

    // 检查 Fn 键位置是否有变化
    if (!((current ^ previous) & fn_mask)) {
        return;
    }

    // 获取当前层在该位置的键码
    uint16_t keycode = keymap_get_keycode(FN_KEY_ROW, FN_KEY_COL);

    // 检查是否为 MO() 层切换键
    if (IS_QK_MOMENTARY(keycode)) {
        uint8_t target_layer = keycode & 0x1F;
        bool pressed = current & fn_mask;

        if (pressed) {
            layer_on(target_layer);
        } else {
            layer_off(target_layer);
        }
    }
#endif
}

// 更新按键列表（基于防抖后的矩阵变化）
static void update_key_code_list(void) {
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        matrix_row_t current = matrix_debounced[row];
        matrix_row_t changes = current ^ matrix_previous[row];

        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            if (!(changes & ((matrix_row_t)1 << col))) continue;

            uint16_t keycode = keymap_get_keycode(row, col);
            if (keycode == KC_NO) continue;

            bool pressed = current & ((matrix_row_t)1 << col);

            if (pressed) {
                // 按键按下：添加到列表
                if (!find_key(_key_code_list, keycode)) {
                    add(keycode, _key_code_list);
                }
            } else {
                // 按键释放：从列表移除
                del(keycode, _key_code_list);
            }
        }
        matrix_previous[row] = current;
    }
}
