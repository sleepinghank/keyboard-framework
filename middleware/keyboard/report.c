/* Copyright 2017 Fred Sundvik
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "report.h"
#include "keycode_config.h"
#include "debug.h"
#include "util.h"
#include <string.h>
#include "wireless.h"

// 外部变量
extern uint8_t keyboard_protocol;

// 全局键盘报告缓冲区
static report_keyboard_t _keyboard_report_data;
static report_nkro_t _nkro_report_data;
report_keyboard_t* keyboard_report = &_keyboard_report_data;
report_nkro_t* nkro_report = &_nkro_report_data;

// NKRO/6KRO 自适应模式计数器
#ifdef APDAPTIVE_NKRO_ENABLE
static uint8_t kb_keys_count = 0;
static uint8_t nkro_bit_count = 0;
uint8_t kb_report_changed = 0;
#endif

#ifdef RING_BUFFERED_6KRO_REPORT_ENABLE
#    define RO_ADD(a, b) ((a + b) % KEYBOARD_REPORT_KEYS)
#    define RO_SUB(a, b) ((a - b + KEYBOARD_REPORT_KEYS) % KEYBOARD_REPORT_KEYS)
#    define RO_INC(a) RO_ADD(a, 1)
#    define RO_DEC(a) RO_SUB(a, 1)
static int8_t cb_head  = 0;
static int8_t cb_tail  = 0;
static int8_t cb_count = 0;
#endif

/** \brief has_anykey
 *
 * FIXME: Needs doc
 */
uint8_t has_anykey(void) {
    uint8_t  cnt = 0;
    uint8_t* p   = keyboard_report->keys;
    uint8_t  lp  = sizeof(keyboard_report->keys);
#ifdef NKRO_ENABLE
#    ifdef APDAPTIVE_NKRO_ENABLE
    return kb_keys_count + nkro_bit_count;
#    endif
    if (keyboard_protocol && keymap_config.nkro) {
        p  = nkro_report->bits;
        lp = sizeof(nkro_report->bits);
    }
#endif
    while (lp--) {
        if (*p++) cnt++;
    }
    return cnt;
}

/** \brief get_first_key
 *
 * FIXME: Needs doc
 */
uint8_t get_first_key(void) {
#ifdef NKRO_ENABLE
#    ifdef APDAPTIVE_NKRO_ENABLE
    return keyboard_report->keys[0];
#    else
    if (keyboard_protocol && keymap_config.nkro) {
        uint8_t i = 0;
        for (; i < NKRO_REPORT_BITS && !nkro_report->bits[i]; i++)
            ;
        return i << 3 | biton(nkro_report->bits[i]);
    }
#    endif
#endif
#ifdef RING_BUFFERED_6KRO_REPORT_ENABLE
    uint8_t i = cb_head;
    do {
        if (keyboard_report->keys[i] != 0) {
            break;
        }
        i = RO_INC(i);
    } while (i != cb_tail);
    return keyboard_report->keys[i];
#else
    return keyboard_report->keys[0];
#endif
}

/** \brief Checks if a key is pressed in the report
 *
 * Returns true if the keyboard_report reports that the key is pressed, otherwise false
 * Note: The function doesn't support modifers currently, and it returns false for KC_NO
 */
bool is_key_pressed(uint8_t key) {
    if (key == KC_NO) {
        return false;
    }
#ifdef NKRO_ENABLE
#    ifdef APDAPTIVE_NKRO_ENABLE
    if (keyboard_protocol && nkro_bit_count) {
        if ((key >> 3) < NKRO_REPORT_BITS && (nkro_report->bits[key >> 3] & 1 << (key & 7))) return true;
    }
#    else
    if (keyboard_protocol && keymap_config.nkro) {
        if ((key >> 3) < NKRO_REPORT_BITS) {
            return nkro_report->bits[key >> 3] & 1 << (key & 7);
        } else {
            return false;
        }
    }
#endif
#endif
    for (int i = 0; i < KEYBOARD_REPORT_KEYS; i++) {
        if (keyboard_report->keys[i] == key) {
            return true;
        }
    }
    return false;
}

/** \brief add key byte
 *
 * FIXME: Needs doc
 */
void add_key_byte(report_keyboard_t* keyboard_report, uint8_t code) {
#ifdef RING_BUFFERED_6KRO_REPORT_ENABLE
    int8_t i     = cb_head;
    int8_t empty = -1;
    if (cb_count) {
        do {
            if (keyboard_report->keys[i] == code) {
                return;
            }
            if (empty == -1 && keyboard_report->keys[i] == 0) {
                empty = i;
            }
            i = RO_INC(i);
        } while (i != cb_tail);
        if (i == cb_tail) {
            if (cb_tail == cb_head) {
                // buffer is full
                if (empty == -1) {
                    // pop head when has no empty space
                    cb_head = RO_INC(cb_head);
                    cb_count--;
                } else {
                    // left shift when has empty space
                    uint8_t offset = 1;
                    i              = RO_INC(empty);
                    do {
                        if (keyboard_report->keys[i] != 0) {
                            keyboard_report->keys[empty] = keyboard_report->keys[i];
                            keyboard_report->keys[i]     = 0;
                            empty                        = RO_INC(empty);
                        } else {
                            offset++;
                        }
                        i = RO_INC(i);
                    } while (i != cb_tail);
                    cb_tail = RO_SUB(cb_tail, offset);
                }
            }
        }
    }
    // add to tail
    keyboard_report->keys[cb_tail] = code;
    cb_tail                        = RO_INC(cb_tail);
    cb_count++;
#else
    int8_t i     = 0;
    int8_t empty = -1;
    for (; i < KEYBOARD_REPORT_KEYS; i++) {
        if (keyboard_report->keys[i] == code) {
            break;
        }
        if (empty == -1 && keyboard_report->keys[i] == 0) {
            empty = i;
        }
    }
    if (i == KEYBOARD_REPORT_KEYS) {
        if (empty != -1) {
            keyboard_report->keys[empty] = code;
#    ifdef APDAPTIVE_NKRO_ENABLE
            kb_report_changed |= KB_RPT_STD;
            ++kb_keys_count;
#    endif
        }
    }
#endif
}

/** \brief del key byte
 *
 * FIXME: Needs doc
 */
void del_key_byte(report_keyboard_t* keyboard_report, uint8_t code) {
#ifdef RING_BUFFERED_6KRO_REPORT_ENABLE
    uint8_t i = cb_head;
    if (cb_count) {
        do {
            if (keyboard_report->keys[i] == code) {
                keyboard_report->keys[i] = 0;
                cb_count--;
                if (cb_count == 0) {
                    // reset head and tail
                    cb_tail = cb_head = 0;
                }
                if (i == RO_DEC(cb_tail)) {
                    // left shift when next to tail
                    do {
                        cb_tail = RO_DEC(cb_tail);
                        if (keyboard_report->keys[RO_DEC(cb_tail)] != 0) {
                            break;
                        }
                    } while (cb_tail != cb_head);
                }
                break;
            }
            i = RO_INC(i);
        } while (i != cb_tail);
    }
#else
    for (uint8_t i = 0; i < KEYBOARD_REPORT_KEYS; i++) {
        if (keyboard_report->keys[i] == code) {
            keyboard_report->keys[i] = 0;
#    ifdef APDAPTIVE_NKRO_ENABLE
            kb_report_changed |= KB_RPT_STD;
            --kb_keys_count;
#    endif
        }
    }
#endif
}

#ifdef NKRO_ENABLE
/** \brief add key bit
 *
 * FIXME: Needs doc
 */
void add_key_bit(report_nkro_t* nkro_report, uint8_t code) {
    if ((code >> 3) < NKRO_REPORT_BITS) {
        nkro_report->bits[code >> 3] |= 1 << (code & 7);
#    ifdef APDAPTIVE_NKRO_ENABLE
        kb_report_changed |= KB_RPT_NKRO;
        ++nkro_bit_count;
#    endif
    } else {
        dprintf("add_key_bit: can't add: %02X\n", code);
    }
}

/** \brief del key bit
 *
 * FIXME: Needs doc
 */
bool del_key_bit(report_nkro_t* nkro_report, uint8_t code) {
    if ((code >> 3) < NKRO_REPORT_BITS) {
#    ifdef APDAPTIVE_NKRO_ENABLE
        if (nkro_report->bits[code >> 3] & (1 << (code & 7))) {
            nkro_report->bits[code >> 3] &= ~(1 << (code & 7));
            kb_report_changed |= KB_RPT_NKRO;
            --nkro_bit_count;
            return true;
        }
#    else
        nkro_report->bits[code >> 3] &= ~(1 << (code & 7));
        return true;
#    endif
    } else {
        dprintf("del_key_bit: can't del: %02X\n", code);
    }

    return false;
}
#endif

/** \brief add key to report
 *
 * FIXME: Needs doc
 */
void add_key_to_report(uint8_t key) {
#ifdef NKRO_ENABLE
#    ifdef APDAPTIVE_NKRO_ENABLE
    if (keyboard_protocol && kb_keys_count == KEYBOARD_REPORT_KEYS) {
#    else
    if (keyboard_protocol && keymap_config.nkro) {
#    endif
        add_key_bit(nkro_report, key);
        return;
    }
#endif
    add_key_byte(keyboard_report, key);
}

/** \brief del key from report
 *
 * FIXME: Needs doc
 */
void del_key_from_report(uint8_t key) {
#ifdef NKRO_ENABLE
#    ifdef APDAPTIVE_NKRO_ENABLE
    if (keyboard_protocol && nkro_bit_count && del_key_bit(nkro_report, key)) return;
#    else
    if (keyboard_protocol && keymap_config.nkro) {
        del_key_bit(nkro_report, key);
        return;
    }
#    endif
#endif
    del_key_byte(keyboard_report, key);
}

/** \brief clear key from report
 *
 * FIXME: Needs doc
 */
void clear_keys_from_report(void) {
    // not clear mods
#ifdef NKRO_ENABLE
#    ifdef APDAPTIVE_NKRO_ENABLE
    memset(nkro_report->bits, 0, sizeof(nkro_report->bits));
    if (nkro_bit_count) {
        kb_report_changed |= KB_RPT_NKRO;
        nkro_bit_count = 0;
    }

    memset(keyboard_report->keys, 0, sizeof(keyboard_report->keys));
    if (kb_keys_count) {
        kb_report_changed |= KB_RPT_STD;
        kb_keys_count = 0;
    }
    return;
#    else
    if (keyboard_protocol && keymap_config.nkro) {
        memset(nkro_report->bits, 0, sizeof(nkro_report->bits));
        return;
    }
#    endif
#endif
    memset(keyboard_report->keys, 0, sizeof(keyboard_report->keys));
}

/** \brief clear all keys from keyboard report
 *
 * 清除键盘报告中所有按键，包括修饰键
 */
void clear_keyboard(void) {
    clear_keys_from_report();
    // 清除修饰键
    keyboard_report->mods = 0;
}

#ifdef MOUSE_ENABLE
/**
 * @brief Compares 2 mouse reports for difference and returns result. Empty
 * reports always evaluate as unchanged.
 *
 * @param[in] new_report report_mouse_t
 * @param[in] old_report report_mouse_t
 * @return bool result
 */
__attribute__((weak)) bool has_mouse_report_changed(report_mouse_t* new_report, report_mouse_t* old_report) {
    // memcmp doesn't work here because of the `report_id` field when using
    // shared mouse endpoint
    bool changed = ((new_report->buttons != old_report->buttons) ||
#    ifdef MOUSE_EXTENDED_REPORT
                    (new_report->boot_x != 0 && new_report->boot_x != old_report->boot_x) || (new_report->boot_y != 0 && new_report->boot_y != old_report->boot_y) ||
#    endif
                    (new_report->x != 0 && new_report->x != old_report->x) || (new_report->y != 0 && new_report->y != old_report->y) || (new_report->h != 0 && new_report->h != old_report->h) || (new_report->v != 0 && new_report->v != old_report->v));
    return changed;
}
#endif

// ============================================================================
// 新增：简化键盘报告接口实现
// ============================================================================

#include "linkedlist.h"
#include "report_buffer.h"
#include "keymap.h"

// 外部链表
extern list_t* _key_code_list;
extern list_t* _key_code_list_extend;

// 上一次报告缓存
static report_keyboard_t last_kb_report;
static uint16_t last_consumer_report;

void report_init(void) {
    memset(&last_kb_report, 0, sizeof(last_kb_report));
    last_consumer_report = 0;
}

// 键码分类并添加到报告
static void classify_and_add_keycode(uint16_t keycode,
                                      report_keyboard_t* kb_report,
                                      uint16_t* consumer_report,
                                      uint8_t* key_idx) {
    if (IS_QK_MOMENTARY(keycode) || IS_QK_TOGGLE_LAYER(keycode)) {
        return;
    }

    if (IS_MODIFIER_KEYCODE(keycode)) {
        // dprintf("  Modifier keycode: 0x%04X, adding to mods\n", keycode);
        kb_report->mods |= MOD_BIT(keycode);
        return;
    }

    if (IS_BASIC_KEYCODE(keycode)) {
        if (*key_idx < KEYBOARD_REPORT_KEYS) {
            // dprintf("  Regular keycode: 0x%04X, adding to keys[%d]\n", keycode, *key_idx);
            kb_report->keys[(*key_idx)++] = (uint8_t)keycode;
        }
        return;
    }

    if (IS_CONSUMER_KEYCODE(keycode)) {
        // dprintf("  Consumer keycode: 0x%04X, adding to consumer report\n", keycode);
        *consumer_report = KEYCODE2CONSUMER(keycode);
        return;
    }
}
extern wt_func_t wireless_transport;
void report_update_proc(key_update_st_t key_st) {
    if (key_st == GHOST_KEY) {
        return;
    }

    report_keyboard_t kb_report = {0};
    uint16_t consumer_report = 0;
    uint8_t key_idx = 0;

    // 遍历 _key_code_list
    if (_key_code_list != NULL) {
        node_t* current = _key_code_list->head;
        while (current != NULL) {
            if (current->data.is_report != 0) {
                uint16_t keycode = current->data.key_code;
                dprintf("Processing keycode from _key_code_list: 0x%04X\r\n", keycode);
                classify_and_add_keycode(keycode, &kb_report, &consumer_report, &key_idx);
            }
            current = current->next;
        }
    }

    // 遍历 _key_code_list_extend（组合键扩展）
    if (_key_code_list_extend != NULL) {
        node_t* current = _key_code_list_extend->head;
        while (current != NULL) {
            if (current->data.is_report != 0) {
                // 跳过已在主列表中的键
                if (!find_activate_key(_key_code_list, current->data.key_code)) {
                    uint16_t keycode = current->data.key_code;
                    dprintf("Processing keycode from _key_code_list_extend: 0x%04X\r\n", keycode);
                    classify_and_add_keycode(keycode, &kb_report, &consumer_report, &key_idx);
                }
            }
            current = current->next;
        }
    }

    // 检查并发送键盘报告
    if (memcmp(&kb_report, &last_kb_report, sizeof(kb_report)) != 0) {
        memcpy(&last_kb_report, &kb_report, sizeof(kb_report));
        dprintf("sending keycode:%d,%d,%d,%d,%d,%d,%d,%d\r\n", kb_report.mods,kb_report.reserved,
                kb_report.keys[0], kb_report.keys[1], kb_report.keys[2], kb_report.keys[3], kb_report.keys[4], kb_report.keys[5]);
        // report_buffer_t report;
        // report.type = REPORT_TYPE_KB;
        // memcpy(&report.keyboard, &kb_report, sizeof(kb_report));
        // report_buffer_enqueue(&report);
        if (wireless_transport.send_keyboard) {
            wireless_transport.send_keyboard((uint8_t *)&kb_report);
        }
    }

    // 检查并发送消费者报告
    if (consumer_report != last_consumer_report) {
        last_consumer_report = consumer_report;
        dprintf("Report: Consumer report changed, sending update:%04X\r\n", consumer_report);
        if (wireless_transport.send_consumer) {
            wireless_transport.send_consumer(consumer_report);
        }
        // report_buffer_t report;
        // report.type = REPORT_TYPE_CONSUMER;
        // report.consumer = consumer_report;
        // report_buffer_enqueue(&report);
    }
}
