/**
 * @file n0046.c
 * @brief N0046 产品入口
 */

#include "config.h"
#include "config_led.h"
#include "keyboard.h"
#include "wireless.h"
#include "output_service.h"
#include "backlight.h"
#include "battery.h"
#include "host.h"
#include "action.h"

/* 自定义键码定义 - 必须与 keymaps/default.c 保持一致 */
enum n0046_keycodes {
    KC_SIRI = QK_KB_0,     /* Siri 唤起 (Fn+F6) */
    KC_BAT,                 /* 电量查询 (Fn+右Cmd) */
    BL_COLOR,               /* 背光颜色切换 (Fn+右Enter) */
    BL_LEVEL,               /* 背光亮度切换 (Fn+右Shift) */
    BT_PAIR,                /* 蓝牙配对 (Fn+Del) */
    FACTORY_RST,            /* 恢复出厂 (ESC+= 长按) */
};

/* Voice Command HID Consumer Usage (0x0C, 0x00CF) */
#define HID_VOICE_COMMAND 0x00CF

void n0046_setup(void) {}

void n0046_init(void) {
    backlight_init(NULL);
    battery_init();
}

void n0046_task(void) {
    backlight_task();
    battery_task();
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case KC_SIRI:
            if (record->event.pressed) {
                /* 发送 Voice Command 消费者用法 */
                host_consumer_send(HID_VOICE_COMMAND);
            } else {
                host_consumer_send(0);
            }
            return false;

        case KC_BAT:
            if (record->event.pressed) {
                /* 电量查询 - 触发电量指示灯效 */
                /* TODO: 实现电量显示功能 (分支四) */
            }
            return false;

        case BL_COLOR:
            if (record->event.pressed) {
                /* 切换背光颜色 */
                backlight_color_step();
            }
            return false;

        case BL_LEVEL:
            if (record->event.pressed) {
                /* 切换背光亮度 */
                backlight_level_step();
            }
            return false;

        case BT_PAIR:
            /* 蓝牙配对 - 需要长按 3 秒 */
            /* TODO: 由组合键模块处理 (分支三) */
            return true;

        case FACTORY_RST:
            /* 恢复出厂设置 - 需要长按 ESC+= 3 秒 */
            /* TODO: 由组合键模块处理 (分支三) */
            return true;
    }

    return true;
}

void wireless_enter_connected_kb(uint8_t host_idx) {
    (void)host_idx;
    output_service_request_indicator(IND_REQ_CAPS_OFF, 0);
}

void wireless_enter_disconnected_kb(uint8_t host_idx, uint8_t reason) {
    (void)host_idx;
    (void)reason;
    output_service_request_indicator(IND_REQ_CAPS_DISCONNECTED, 0);
}

void wireless_enter_discoverable_kb(uint8_t host_idx) {
    output_service_request_indicator(IND_REQ_BT_PAIRING, host_idx);
}

void wireless_enter_reconnecting_kb(uint8_t host_idx) {
    output_service_request_indicator(IND_REQ_BT_RECONNECTING, host_idx);
}

