// middleware/keyboard/combo/Special_Combo.c
#include "Special_Combo.h"
#include "osal.h"

// 组合键触发标志位
uint8_t combinations_flag = 1;

// OSAL 事件定义（需要在 sys_config.h 或相关头文件中定义）
// 示例事件 ID，根据实际项目调整
#ifndef SYS_EVT_STORAGE_WRITE
#define SYS_EVT_STORAGE_WRITE    0x0001
#endif
#ifndef SYS_EVT_FACTORY_RESET
#define SYS_EVT_FACTORY_RESET    0x0002
#endif
#ifndef SYS_EVT_BLE_PAIRING
#define SYS_EVT_BLE_PAIRING      0x0004
#endif
#ifndef SYS_EVT_BACKLIGHT_COLOR
#define SYS_EVT_BACKLIGHT_COLOR  0x0008
#endif
#ifndef SYS_EVT_BACKLIGHT_BRIGHT
#define SYS_EVT_BACKLIGHT_BRIGHT 0x0010
#endif
#ifndef SYS_EVT_TOUCH_TOGGLE
#define SYS_EVT_TOUCH_TOGGLE     0x0020
#endif
#ifndef SYS_EVT_BATTERY_CHECK
#define SYS_EVT_BATTERY_CHECK    0x0040
#endif

// 系统服务任务 ID（需要根据实际项目定义）
#ifndef SYSTEM_SERVICE_TASK_ID
#define SYSTEM_SERVICE_TASK_ID   0
#endif

// Fn 锁定
uint8_t Set_FN_Lock(uint16_t* add_keys) {
    // TODO: 实现 Fn 锁定逻辑
    osal_set_event(SYSTEM_SERVICE_TASK_ID, SYS_EVT_STORAGE_WRITE);
    return 0;
}

// 切换背光颜色
uint8_t Backlight_Color(uint16_t* add_keys) {
    osal_set_event(SYSTEM_SERVICE_TASK_ID, SYS_EVT_BACKLIGHT_COLOR);
    osal_set_event(SYSTEM_SERVICE_TASK_ID, SYS_EVT_STORAGE_WRITE);
    return 0;
}

// 切换背光亮度
uint8_t Backlight_Light(uint16_t* add_keys) {
    osal_set_event(SYSTEM_SERVICE_TASK_ID, SYS_EVT_BACKLIGHT_BRIGHT);
    osal_set_event(SYSTEM_SERVICE_TASK_ID, SYS_EVT_STORAGE_WRITE);
    return 0;
}

// 触摸板锁定
uint8_t Touch_Locked(uint16_t* add_keys) {
    osal_set_event(SYSTEM_SERVICE_TASK_ID, SYS_EVT_TOUCH_TOGGLE);
    osal_set_event(SYSTEM_SERVICE_TASK_ID, SYS_EVT_STORAGE_WRITE);
    return 0;
}

// 恢复出厂设置
uint8_t Factory_Reset(uint16_t* add_keys) {
    osal_set_event(SYSTEM_SERVICE_TASK_ID, SYS_EVT_FACTORY_RESET);
    return 0;
}

// 电池检测
uint8_t Bat_Check(uint16_t* add_keys) {
    osal_set_event(SYSTEM_SERVICE_TASK_ID, SYS_EVT_BATTERY_CHECK);
    return 0;
}

// 配对按钮
uint8_t Pair_button(uint16_t* add_keys) {
    osal_set_event(SYSTEM_SERVICE_TASK_ID, SYS_EVT_BLE_PAIRING);
    return 0;
}
