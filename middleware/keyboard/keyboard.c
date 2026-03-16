// middleware/keyboard/keyboard.c
#include "keyboard.h"
#include "keymap.h"
#include "matrix.h"
#include "debounce.h"
#include "report.h"
#include "report_buffer.h"
#include "keycode.h"
#include <string.h>
#include "sys_config.h"
#include "debug.h"
#include "wireless.h"

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

__HIGH_CODE
void keyboard_task(void) {

    // 1. 矩阵扫描 + 驱动层防抖
    key_update_st_t key_st = scan_and_debounce();
    last_update_state = key_st;

    // 2. 幽灵键直接返回
    if (key_st == GHOST_KEY) {
        return;
    }

    if (key_st == KEY_UPDATE) {
        dprintf("Key update detected, updating key code list\r\n");
        // 4. 更新按键列表（基于防抖后的矩阵变化）
        update_key_code_list();
        // 5. 组合键处理
        combo_task(key_st);
        // 6. 生成并发送 HID 报告
        report_update_proc(key_st);
    }

    // 7. 清空扩展键列表
    del_all_child(_key_code_list_extend);
}

key_update_st_t keyboard_get_last_update_state(void) {
    return last_update_state;
}

// 矩阵扫描 + 驱动层防抖处理
static key_update_st_t scan_and_debounce(void) {

    // 调用驱动层矩阵扫描（内部已完成防抖，返回防抖后是否有变化）
    bool debounced_changed = matrix_scan();

    if (debounced_changed) {
        dprintf("Matrix scan detected changes\r\n");
    }

    // 获取已防抖的矩阵数据（matrix_scan 内部已调用 debounce）
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        matrix_debounced[row] = matrix_get_row(row);
    }

    if (!debounced_changed) {
        return NO_KEY_UPDATE;
    }

    // TODO: 可选的幽灵键检测
    // if (detect_ghost_key(matrix_debounced)) {
    //     return GHOST_KEY;
    // }

    return KEY_UPDATE;
}

// 更新按键列表（基于防抖后的矩阵变化）
static void update_key_code_list(void) {
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        matrix_row_t current = matrix_debounced[row];
        matrix_row_t changes = current ^ matrix_previous[row];

        // 快速路径：无变化直接跳过整行
        if (changes == 0) {
            continue;
        }
        dprintf("Row %d changed: 0x%04X\r\n", row, changes);
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            matrix_row_t col_mask = (matrix_row_t)1 << col;
            if (!(changes & col_mask)) continue;

            uint16_t keycode = keymap_get_keycode(row, col-1);
            dprintf("  Column %d changed, keycode: 0x%04X\r\n", col, keycode);
            if (keycode == KC_NO) continue;

            if (current & col_mask) {
                
                // 按键按下：添加到列表
                if (!find_key(_key_code_list, keycode)) {
                    dprintf("Key pressed: row %d, col %d, keycode 0x%04X\r\n", row, col, keycode);
                    add(keycode, _key_code_list);
                }
            } else {
                dprintf("Key released: row %d, col %d, keycode 0x%04X\r\n", row, col, keycode);
                // 按键释放：从列表移除
                del(keycode, _key_code_list);
            }
        }
        matrix_previous[row] = current;
    }
}

/**
 * @brief 根据主机系统类型更新基础层
 * @note 供 wireless 层在连接状态变化时调用
 */
void keyboard_update_base_layer_by_system(void) {
    extern uint8_t host_system_type;
    layer_id_t target_layer;

    switch (host_system_type) {
        case MAC:
        case IOS:
            target_layer = LAYER_MACOS;
            break;
        case WIN:
        case ANDROID:
        default:
            target_layer = LAYER_WINDOWS;
            break;
    }

    dprintf("Layer switch: system_type=%d, target_layer=%d\r\n", host_system_type, target_layer);
    layer_set(target_layer);
}
