#pragma once
#include "sys_config.h"

#pragma once

#include "quantum.h"

// 硬件选项
#define VENDOR_ID 0x1234          // USB厂商ID，DIY项目可以任意设置
#define PRODUCT_ID 0x5678         // USB产品ID，DIY项目可以任意设置
#define DEVICE_VER 0x0100         // 设备版本号（常用于硬件修订）
#define MANUFACTURER "Keychron"   // 制造商/品牌名称
#define PRODUCT "KB04122-13A"     // 键盘产品名称

// 矩阵配置
#define MATRIX_ROWS 6             // 键盘矩阵的行数
#define MATRIX_COLS 13            // 键盘矩阵的列数

// 矩阵引脚配置（根据实际硬件取消注释并配置）
#define MATRIX_ROW_PINS { A0, A1, A2, A3,A4,A5,A6,A7 }  // 行引脚，从上到下
#define MATRIX_COL_PINS { B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10, B11, B12 }  // 列引脚，从左到右
#define MATRIX_IO_DELAY 30        // 改变矩阵引脚状态和读取值之间的延迟（微秒）
#define MATRIX_HAS_GHOST        // 定义矩阵是否存在重影问题（不太可能）
// #define MATRIX_UNSELECT_DRIVE_HIGH  // 取消选择时将引脚设置为输出高电平而非输入高电平
#define DIODE_DIRECTION ROW2COL   // 二极管方向：COL2ROW 或 ROW2COL
// #define DIRECT_PINS { { F极1, F0, B0, C7 }, { F4, F5, F6, F7 } }  // 开关直接引脚映射

// 音频选项（取消注释以启用）
// #define AUDIO_VOICES            // 启用备用音频声音（循环播放）
// #define AUDIO极_PIN C4           // 在特定引脚上启用音频
// #define AUDIO_PIN_ALT B5       // 双音功能的备用音频引脚

// 背光选项（取消注释以启用）
// #define BACKLIGHT_PIN B7       // 背光控制引脚
// #define BACKLIGHT_LEVELS 3     // 背光级别数量（最大31，不包括关闭）
// #define BACKLIGHT_BREATHING    // 启用背光呼吸效果
// #define BREATHING_PERIOD 6     // 一次背光"呼吸"的时长（秒）

// 消抖配置
#define DEBOUNCE 5                // 读取引脚值时的消抖延迟（默认5ms）

// 锁定支持（取消注释以启用）
// #define LOCKING_SUPPORT_ENABLE // 机械锁定支持。使用 KC_LCAP、KC_LNUM 或 KC_LSCR
// #define LOCKING_RESYNC_ENABLE  // 保持开关状态与键盘LED状态一致

// 命令配置
// #define IS_COMMAND() (get_mods() == MOD_MASK_SHIFT)  // 用于魔法命令的键组合（调试用）

// USB配置
#define USB_MAX_POWER_CONSUMPTION 500  // USB最大功耗（毫安，默认：500）
#define USB_POLLING_INTERVAL_MS 10     // USB轮询速率（毫秒），用于键盘/鼠标接口
#define USB_SUSPEND_WAKEUP_DELAY 0     // 发送唤醒数据包后的暂停时间（毫秒，0=禁用）

// I2C配置（用于使用I2C的键盘）
// #define F_SCL 100000L          // I2C时钟速率（默认：400000L，split_common为100000L）

// 可禁用的功能（取消注释以禁用）
// #define NO_DEBUG               // 禁用调试
// #define NO_PRINT               // 禁用使用hid_listen的打印/调试
// #define NO_ACTION_LAYER       // 禁用层功能
// #define NO_ACTION_TAPPING      // 禁用点击舞蹈和其他点击功能
// #define NO_ACTION_ONESHOT      // 禁用单次修饰键

// 可启用的功能（取消注释以启用）极
// #define ENABLE_COMPILE_KEYCODE // 启用 QK_MAKE 键码
// #define STRICT_LAYER_RELEASE   // 强制使用当前层栈评估键释放

// 可配置的行为
#define TAPPING_TERM 200          // 按键按下变为保持前的时长（毫秒）
// #define TAPPING_TERM_PER_KEY   // 启用按键的TAPPING_TERM设置
// #define RETRO_TAPPING          // 即使在TAPPING_TERM后，如果没有其他键中断，仍然点击
// #define RETRO_TAPPING_PER_KEY  // 启用按键的RETRO_TAPPING设置
#define TAPPING_TOGGLE 2          // 触发切换前的点击次数
// #define PERMISSIVE_HOLD        // 如果在释放前按下其他键，使点击保持键触发保持
// #define PERMISSIVE_HOLD_PER_KEY // 启用按键的PERMISSIVE_HOLD设置
#define QUICK_TAP_TERM 100        // 双角色键重复键码的点击然后保持时序
// #define QUICK_TAP_TERM_PER_KEY // 启用按键的QUICK_TAP_TERM设置
// #define HOLD_ON_OTHER_KEY_PRESS // 当点击被其他键按下中断时立即选择保持动作
// #define HOLD_ON_OTHER_KEY_PRESS_PER_KEY // 启用按键的HOLD_ON_OTHER_KEY_PRESS设置

// Leader键配置
#define LEADER_TIMEOUT 300        // Leader键超时前的时长（毫秒）
// #define LEADER_PER_KEY_TIMING  // 在Leader序列中每次按键后重置超时
// #define LEADER_KEY_STRICT_KEY_PROCESSING // 禁用Mod-Tap/Layer-Tap键码的过滤

// 鼠标选项
// #define MOUSE_EXTENDED_REPORT  // 启用扩展鼠标报告（-32767到32767）

// 单次触发配置
#define ONESHOT_TIMEOUT 300       // 单次触发超时前的时长（毫秒）
#define ONESHOT_TAP_TOGGLE 2      // 触发单次切换前的点击次数

// 组合键配置
#define COMBO_TERM 200            // 检测组合键的时长（毫秒）
// #define COMBO_MUST_HOLD_MODS   // 为包含修饰键的组合键启用超时扩展
#define COMBO_MOD_TERM 200        // 允许在组合过程中为修饰键扩展COMBO_TERM
// #define COMBO_MUST_HOLD_PER_COMBO // 启用按组合键的超时扩展
// #define COMBO_TERM_PER_COMBO   // 启用按组合键的COMBO_TERM扩展
// #define COMBO_STRICT_TIMER     // 仅在第一次按键时启动组合定时器
// #define COMBO_NO_TIMER         // 完全禁用组合定时器（宽松组合）

// 点击代码延迟配置
#define TAP_CODE_DELAY 100        // register_code和unregister_code之间的延迟（毫秒）
#define TAP_HOLD_CAPS_DELAY 80    // 使用KC_CAPS_LOCK键码时Tap Hold键的延迟（毫秒）

// 键覆盖配置
#define KEY_OVERRIDE_REPEAT_DELAY 500  // 键覆盖的键重复间隔（毫秒）

// 传统魔法处理
// #define LEGACY_MAGIC_HANDLING  // 为高级键码启用魔法配置处理

