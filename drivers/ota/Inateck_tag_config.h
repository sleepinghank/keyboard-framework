#ifndef _INATECK_TAG_CONFIG_H_
#define _INATECK_TAG_CONFIG_H_

#include "kb904/config_product.h"
#include "CH58x_common.h"
#include "Inateck_tag.h"


// TAG列表大小
#define TAG_LIST_SIZE   18

// 系统相关（单字节）2个
#define TAG_ID_MAX_MTU                 0x10  // 最大支持的MTU
#define TAG_ID_CURRENT_MTU             0x11  // 当前的MTU

// 设备配置（单字节）5个
#define TAG_ID_BATTERY_LEVEL           0x20  // 设备电量（0~100）
// #define TAG_ID_TOUCHPAD_SWITCH         0x22  // 触摸板开关（0：关闭 1：开启）
// #define TAG_ID_LED_SWITCH              0x23  // 背光灯开关（0：关闭 1：开启）
// #define TAG_ID_LED_BRIGHTNESS          0x24  // 背光灯亮度（0~100）
// #define TAG_ID_LED_COLOR               0x25  // 背光灯颜色（0：白色 1：红色 2：绿色 3：蓝色 4：黄色 5：紫色 6：青色 7：橙色）

#define TAG_ID_FIND_SWITCH             0x27  // 查找开关（0：关闭 1：开启）
#define TAG_ID_FIND_ENABLE             0x28  // 查找（0：开始查找 1：停止查找）
#define TAG_ID_GAIN_LEVEL              0x29  // 增益调节（0~127）

// 系统相关（双字节）4个
#define TAG_ID_RESET_DEVICE            0xC100  // 设备复位（只写，写1就开始复位）
#define TAG_ID_FACTORY_RESET           0xC101  // 设备重置（只写，写1就开始设备重置）
#define TAG_ID_RESET_SETTINGS          0xC102  // 重置系统设置（只写，写1就开始重置）
#define TAG_ID_DISCONNECT              0xC103  // 断开连接（只写，写1就断开连接）
#define TAG_ID_DEVICE_NAME             0xC10B  // 蓝牙名称
// #define TAG_ID_PRODUCT_MODEL_NAME      0xC10C  // 设备型号名称

// 设备配置（双字节）10个
#define TAG_ID_SHUTDOWN_TIME              0xC200  // 关机时间（最高位为0表示秒，最高位为1表示分钟，范围为0-32767，0为不关机）
#define TAG_ID_SLEEP_TIME                 0xC201  // 待机时间（最高位为0表示秒，最高位为1表示分钟，范围为0-32767，0为不待机）
#define TAG_ID_LED_OFF_TIME               0xC202  // 背光灯关闭时间（最高位为0表示秒，最高位为1表示分钟，范围为0-32767，0为不关闭）
#define TAG_ID_CALIBRATE_BATTERY          0xC203  // 校准电池电量（只写，写1就开始校准）
#define TAG_ID_BATTERY_CALIBRATION_STATUS 0xC204  // 电池电量校准完成（0为无效值，1为正在校准，2为校准完成）
#define TAG_ID_TOUCH_REPORT_RATE          0xC205  // 触摸板上报率（0：80Hz 1：90Hz 2：100Hz 3：125Hz 4:150Hz）
#define TAG_ID_TOUCH_GESTURE_SUPPORT      0xC206  // 触摸板支持的自定义手势
#define TAG_ID_TOUCH_GESTURE_SHORTCUT     0xC207  // 触摸板自定义手势快捷键
/** !!!!! 注意这里的两个TAG采用精简版本的实现，TAG_ID_KEYBOARD_SHORTCUT_BITMAP的BITMAP长度为2个字节，TAG_ID_KEYBOARD_SHORTCUT最多支持16个按键**/
#define TAG_ID_KEYBOARD_SHORTCUT_BITMAP   0xC208  // 键盘预设快捷键启用BITMAP
#define TAG_ID_KEYBOARD_SHORTCUT          0xC209  // 键盘预设快捷键



// 快捷键枚举定义
typedef enum {
    SHORTCUT_NONE = 0,          // 无快捷键,默认状态
    SHORTCUT_SIRI = 1,          // 唤起Siri
    SHORTCUT_HOME = 2,          // 前往主屏幕
    SHORTCUT_APP_SWITCHER = 3,  // App切换器
    SHORTCUT_LOCK = 4,          // 锁屏
    SHORTCUT_SEARCH = 5,        // 搜索
    SHORTCUT_SCREENSHOT = 6,    // 截屏
    SHORTCUT_COPY = 7,          // 复制
    SHORTCUT_PASTE = 8,         // 粘贴
    SHORTCUT_CUT = 9,           // 剪切
    SHORTCUT_PLAY_PAUSE = 10,   // 播放/暂停
    SHORTCUT_MUTE = 11,         // 静音
    SHORTCUT_BRIGHTNESS_UP = 12,// 亮度+
    SHORTCUT_BRIGHTNESS_DN = 13,// 亮度-
    SHORTCUT_PAGE_UP = 14,      // 上一页
    SHORTCUT_PAGE_DN = 15,      // 下一页
    SHORTCUT_VOLUME_UP = 16,    // 音量+
    SHORTCUT_VOLUME_DN = 17,    // 音量-
    SHORTCUT_PREV_TRACK = 18,   // 上一首
    SHORTCUT_NEXT_TRACK = 19    // 下一首
} shortcut_type_t;


extern const tag_descriptor_t tag_list[TAG_LIST_SIZE];

//设置所有TAG默认值
extern void set_all_tags_to_default(void);
// 全局TAG写入成功回调函数
extern void tag_write_success_cb(const uint8_t* tags, uint16_t tag_count);
// 如果背光变化则主动推送
extern void push_backlight_change(void);
// 如果触摸板开关变化则主动推送
extern void push_touchpad_switch_change(void);
// 如果电池电量校准完成则主动推送
extern void push_battery_calibration_status_change(void);
extern void set_all_system_value(void);
#endif // _INATECK_TAG_CONFIG_H_ 
