#include "output_service.h"
#include <stdint.h>
#include "event_manager.h"
#include "debug.h"

#if (CHIP_TYPE == CHIP_CH584M)
#include "CH585SFR.h"
#endif

#include "report_buffer.h"
#include "transport.h"
#include "indicator.h"
#include "kb904/config_product.h"
#include "backlight.h"
#include "battery.h"
#include "system_service.h"
#include "storage.h"

#ifdef P2P4G_ENABLE_FLAG
#include "p24g_driver.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

uint8_t output_taskID = 0;

#define BACKLIGHT_SLEEP_TIMEOUT_MS 1600*5

typedef struct {
    const ind_effect_t* effect;
    bool                dirty;
} ind_pending_t;

typedef struct {
    ind_req_type_t      type;
    uint8_t             led_id;
    const ind_effect_t* effect;
} ind_req_map_t;

static ind_pending_t ind_pending[IND_LED_COUNT];
static bool          output_service_ready = false;
static bool          backlight_sleeping = false;
static bool          backlight_low_battery = false;
static bool          backlight_enabled_before_low_battery = true;
static bool          backlight_sleeping_before_low_battery = false;
static bool          backlight_restored_from_storage = false;

#define STORAGE_BACKLIGHT_ENABLE_MASK 0x80u//bit7 为开关
#define STORAGE_BACKLIGHT_COLOR_MASK  0x7Fu//bit0~6 为颜色索引

static const ind_req_map_t ind_req_map[] = {
#if PRODUCT_ID == 0x0904
    { IND_REQ_BT_PAIRING,        LED_BT,   &IND_BLINK_FAST },
    { IND_REQ_BT_RECONNECTING,   LED_BT,   &IND_BLINK_SLOW },
    { IND_REQ_BT_CONNECTED,      LED_BT,   &IND_OFF        },
    { IND_REQ_BT_DISCONNECTED,   LED_BT,   &IND_OFF        },
#endif
    { IND_REQ_CAPS_ON,           LED_CAPS, &IND_ON         },
    { IND_REQ_CAPS_OFF,          LED_CAPS, &IND_OFF        },
    { IND_REQ_CAPS_DISCONNECTED, LED_CAPS, &IND_BLINK_SLOW },
    { IND_REQ_LOW_BATTERY,       LED_POWER_RED,  &IND_BLINK_SLOW },
    { IND_REQ_BATTERY_NORMAL,    LED_POWER_RED,  &IND_OFF        },
};

/**
 * @brief 启动/重置背光休眠定时器
 *
 * 内部函数，用于启动或重置 5 秒背光休眠定时器。
 * 在以下情况下不启动定时器：
 * - 服务未就绪
 * - 低电量模式下（背光应保持关闭）
 */
static void output_service_arm_backlight_sleep_timer(void) {
    if (!output_service_ready || backlight_low_battery) {
        return;
    }

    (void)OSAL_StopTask(output_taskID, OUTPUT_BACKLIGHT_SLEEP_EVT);
    (void)OSAL_SetDelayedEvent(output_taskID, OUTPUT_BACKLIGHT_SLEEP_EVT, BACKLIGHT_SLEEP_TIMEOUT_MS);
}

static uint8_t output_service_clamp_backlight_level(uint8_t level) {
    if (level >= BL_LEVEL_COUNT) {
        return (uint8_t)BACKLIGHT_DEFAULT_LEVEL;
    }
    return level;
}

static uint8_t output_service_clamp_color_index(uint8_t color_index) {
    if (color_index >= BL_COLOR_COUNT) {
        return (uint8_t)BACKLIGHT_DEFAULT_COLOR;
    }
    return color_index;
}

static void output_service_decode_backlight_setting(const storage_config_t *cfg,
                                                    uint8_t *color_index,
                                                    bool *enabled) {
    uint8_t packed = cfg->backlight_color;
    *color_index = output_service_clamp_color_index((uint8_t)(packed & STORAGE_BACKLIGHT_COLOR_MASK));
    *enabled = (packed & STORAGE_BACKLIGHT_ENABLE_MASK) != 0u;
}

static void output_service_sync_backlight_to_storage_and_save(void) {
    storage_config_t *cfg = storage_get_config_ptr();
    uint8_t color_index = (uint8_t)backlight_get_preset_color();
    uint8_t level = (uint8_t)backlight_get_preset_level();
    bool enabled = backlight_is_enabled();

    if (cfg == NULL) {
        return;
    }

    cfg->backlight_brightness = output_service_clamp_backlight_level(level);
    cfg->backlight_color = (uint8_t)(output_service_clamp_color_index(color_index) & STORAGE_BACKLIGHT_COLOR_MASK);
    if (enabled) {
        cfg->backlight_color |= STORAGE_BACKLIGHT_ENABLE_MASK;
    }
    (void)storage_save();
}

/**
 * @brief 请求指示灯效果
 *
 * 根据请求类型设置对应 LED 的灯效，并触发指示灯更新事件。
 * 对于低电量相关请求，还会联动控制背光。
 *
 * @param type 指示灯请求类型（蓝牙状态、大写锁定、电量状态等）
 * @param param 预留参数，当前未使用
 */
void output_service_request_indicator(ind_req_type_t type, uint8_t param) {
    (void)param;

    if (!output_service_ready) {
        return;
    }

    // 低电量请求：联动关闭背光
    if (type == IND_REQ_LOW_BATTERY) {
        output_service_set_backlight_low_battery(true);
    } else if (type == IND_REQ_BATTERY_NORMAL) {
        output_service_set_backlight_low_battery(false);
    }

    // 查找并设置对应的指示灯效果
    for (uint8_t i = 0; i < (sizeof(ind_req_map) / sizeof(ind_req_map[0])); i++) {
        if (ind_req_map[i].type == type) {
            uint8_t led_id = ind_req_map[i].led_id;

            ind_pending[led_id].effect = ind_req_map[i].effect;
            ind_pending[led_id].dirty = true;
            OSAL_SetEvent(output_taskID, OUTPUT_INDICATOR_EVT);
            return;
        }
    }
}

/**
 * @brief 通知背光有活动发生
 *
 * 当有按键或触控活动时调用此函数：
 * 1. 如果背光处于休眠状态，则唤醒背光
 * 2. 重置 5 秒休眠定时器
 *
 * 在低电量模式或 LPM 禁用状态下，此函数不执行任何操作。
 *
 * @note 供 keyboard.c 的 keyboard_note_backlight_activity() 包装函数调用
 */
void output_service_note_backlight_activity(void) {
    if (!output_service_ready || backlight_low_battery) {
        return;
    }

    // 如果背光处于休眠状态，则唤醒
    if (backlight_sleeping) {
        backlight_enable();
        backlight_sleeping = false;
    }

    // 背光开启时才启动休眠定时器
    if (backlight_is_enabled()) {
        output_service_arm_backlight_sleep_timer();
    }
}

/**
 * @brief 设置低电量模式
 *
 * 控制背光的低电量联动行为：
 * - 进入低电量时：记忆当前背光状态，关闭背光，取消休眠定时器
 * - 退出低电量时：恢复之前的背光状态，重启活动检测
 *
 * @param enable true=进入低电量模式，false=退出低电量模式
 *
 * @note 使用 backlight_enabled_before_low_battery 实现状态记忆，
 *       确保电量恢复后背光状态正确恢复
 */
void output_service_set_backlight_low_battery(bool enable) {
    if (!output_service_ready) {
        return;
    }

    if (enable) {
        // 幂等检查：已处于低电量模式则跳过
        if (backlight_low_battery) {
            LOG_I("[OUTPUT] low battery enter ignored(idempotent): flag=%d",
                  (int)backlight_low_battery);
            return;
        }

        LOG_I("[OUTPUT] low battery enter: before_flag=%d, before_restore=%d, backlight_en=%d, sleeping=%d",
              (int)backlight_low_battery,
              (int)backlight_enabled_before_low_battery,
              (int)backlight_is_enabled(),
              (int)backlight_sleeping);

        // 记忆当前背光状态（低电恢复要区分：背光是“开着”还是“正处于休眠关灯”）
        backlight_enabled_before_low_battery = backlight_is_enabled();
        backlight_sleeping_before_low_battery = backlight_sleeping;
        backlight_low_battery = true;
        backlight_sleeping = false;

        // 取消休眠定时器
        (void)OSAL_StopTask(output_taskID, OUTPUT_BACKLIGHT_SLEEP_EVT);

        // 如果背光是开启的，则关闭
        if (backlight_enabled_before_low_battery) {
            backlight_disable();
        }

        LOG_I("[OUTPUT] low battery enter done: flag=%d, restore_en=%d, backlight_en=%d",
              (int)backlight_low_battery,
              (int)backlight_enabled_before_low_battery,
              (int)backlight_is_enabled());
        return;
    }

    // 幂等检查：未处于低电量模式则跳过
    if (!backlight_low_battery) {
        LOG_I("[OUTPUT] low battery exit ignored(idempotent): flag=%d",
              (int)backlight_low_battery);
        return;
    }

    LOG_I("[OUTPUT] low battery exit start: flag=%d, restore_en=%d, backlight_en=%d, sleeping=%d",
          (int)backlight_low_battery,
          (int)backlight_enabled_before_low_battery,
          (int)backlight_is_enabled(),
          (int)backlight_sleeping);

    // 退出低电量模式
    backlight_low_battery = false;

    // 恢复之前的背光状态
    if (backlight_enabled_before_low_battery) {
        backlight_enable();
        output_service_note_backlight_activity();
    } else if (backlight_sleeping_before_low_battery) {
        /*
         * 低电进入时背光处于休眠关灯：
         * 恢复时不强制点亮（避免立即耗电），
         * 但需要把 backlight_sleeping 置回 true，
         * 让下一次普通按键触发 note_backlight_activity() 时能唤醒点亮。
         */
        backlight_sleeping = true;
    }

    LOG_I("[OUTPUT] low battery exit done: flag=%d, restore_en=%d, backlight_en=%d, sleeping=%d",
          (int)backlight_low_battery,
          (int)backlight_enabled_before_low_battery,
          (int)backlight_is_enabled(),
          (int)backlight_sleeping);
}

uint16_t output_process_event(uint8_t task_id, uint16_t events) {
    (void)task_id;

    if (events & OUTPUT_INDICATOR_EVT) {
        LOG_I("[OUTPUT] indicator update event");
        for (uint8_t i = 0; i < IND_LED_COUNT; i++) {
            if (ind_pending[i].dirty && ind_pending[i].effect != NULL) {
                indicator_set(i, ind_pending[i].effect);
                ind_pending[i].dirty = false;
            }
        }
        return (events ^ OUTPUT_INDICATOR_EVT);
    }

    if (events & OUTPUT_BACKLIGHT_BRIGHTNESS_EVT) {
        LOG_I("[OUTPUT] backlight brightness event");
         /* 低电/LPM 禁用：禁止档位切换点亮背光（与 note_backlight_activity 策略一致） */
        if (!backlight_low_battery ) {
            backlight_level_step();
            output_service_sync_backlight_to_storage_and_save();
        }
        output_service_note_backlight_activity();
        return (events ^ OUTPUT_BACKLIGHT_BRIGHTNESS_EVT);
    }

    if (events & OUTPUT_BACKLIGHT_COLOR_EVT) {
        LOG_I("[OUTPUT] backlight color event");

        if (backlight_is_enabled() && !backlight_low_battery) {
            backlight_color_step();
            output_service_sync_backlight_to_storage_and_save();
        }
        output_service_note_backlight_activity();
        return (events ^ OUTPUT_BACKLIGHT_COLOR_EVT);
    }

    if (events & OUTPUT_BATTERY_CHECK_EVT) {
        LOG_I("[OUTPUT] battery check event");
        uint8_t percentage = battery_get_percentage();
        uint8_t blink_count;
        if (percentage >= 75)      blink_count = 4;
        else if (percentage >= 50) blink_count = 3;
        else if (percentage >= 25) blink_count = 2;
        else                       blink_count = 1;

        ind_effect_t effect = IND_BLINK_CUSTOM(200, 200, blink_count);
        indicator_set(LED_POWER_RED, &effect);
        return (events ^ OUTPUT_BATTERY_CHECK_EVT);
    }

    if (events & OUTPUT_SEND_HID_KEYBOARD_EVT) {
        // HID 键盘报文发送
        return (events ^ OUTPUT_SEND_HID_KEYBOARD_EVT);
    }

    if (events & OUTPUT_SEND_MOUSE_REPORT_EVT) {
        // 鼠标报文发送
        return (events ^ OUTPUT_SEND_MOUSE_REPORT_EVT);
    }

    if (events & OUTPUT_SEND_TOUCH_REPORT_EVT) {
        // 触控板报文发送
        return (events ^ OUTPUT_SEND_TOUCH_REPORT_EVT);
    }

    if (events & OUTPUT_BACKLIGHT_SLEEP_EVT) {
        LOG_I("[OUTPUT] backlight sleep event");
        if (!backlight_low_battery && backlight_is_enabled()) {
            backlight_sleeping = true;
            backlight_disable();
        }
        return (events ^ OUTPUT_BACKLIGHT_SLEEP_EVT);
    }

    return 0;
}

void output_service_init(void) {
    storage_config_t *cfg;
    uint8_t level;
    uint8_t color_index;
    bool enabled;

    output_taskID = OSAL_ProcessEventRegister(output_process_event);

    cfg = storage_get_config_ptr();
    level = (uint8_t)BACKLIGHT_DEFAULT_LEVEL;
    color_index = (uint8_t)BACKLIGHT_DEFAULT_COLOR;
    enabled = BACKLIGHT_DEFAULT_ON;

    if (cfg != NULL) {
        level = output_service_clamp_backlight_level(cfg->backlight_brightness);
        output_service_decode_backlight_setting(cfg, &color_index, &enabled);
        backlight_restored_from_storage = true;
    } else {
        backlight_restored_from_storage = false;
    }

    backlight_set_preset_color((bl_preset_color_t)color_index);
    backlight_set_preset_level((bl_preset_level_t)level);
    if (enabled) {
        backlight_enable();
    } else {
        backlight_disable();
    }

    output_service_ready = (output_taskID != 0xFF);
    backlight_sleeping = false;
    backlight_low_battery = false;//低电上电检测不及时会点亮背光再熄灭
    backlight_enabled_before_low_battery = backlight_is_enabled();
    LOG_I("[OUTPUT] backlight restore: %s level=%d color=%d en=%d",
          backlight_restored_from_storage ? "from_storage" : "default",
          level, color_index, enabled);
    if (output_service_ready) {
        output_service_note_backlight_activity();
    } else {
        LOG_E("[OUTPUT] task register failed");
    }
}

#ifdef __cplusplus
}
#endif
