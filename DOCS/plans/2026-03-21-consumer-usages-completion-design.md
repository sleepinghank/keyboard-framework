# Consumer Usages 补全设计文档

- 日期：2026-03-21
- 类型：design
- 状态：已确认
- 范围：对标参考项目，补齐 report.h / keycodes.h 中缺失的媒体键定义

## 1. 背景

当前项目的 `consumer_usages` 枚举和 keycode 定义不完整，部分媒体键在 iPad 等平台上缺少支持。需要对标参考项目的 `media_keycode_t` 定义，补齐缺失的 consumer usage 和对应 keycode 映射。

## 2. 差异分析

### 2.1 参考项目完整 usage 列表对比

| 参考项目名称 | HID Usage | 当前项目状态 |
|---|---|---|
| M_SCAN_NEXT_TRACK | 0x00B5 | 已有 TRANSPORT_NEXT_TRACK |
| M_SCAN_PREVIOUS_TRACK | 0x00B6 | 已有 TRANSPORT_PREV_TRACK |
| M_STOP | 0x00B7 | 已有 TRANSPORT_STOP |
| M_PLAY_N_PAUSE | 0x00CD | 已有 TRANSPORT_PLAY_PAUSE |
| M_MUTE | 0x00E2 | 已有 AUDIO_MUTE |
| M_BASS_BOOST | 0x00E5 | **缺失** |
| M_LOUDNESS | 0x00E7 | **缺失** |
| M_VOLUME_UP | 0x00E9 | 已有 AUDIO_VOL_UP |
| M_VOLUME_DOWN | 0x00EA | 已有 AUDIO_VOL_DOWN |
| M_BASS_UP | 0x0152 | **缺失** |
| M_BASS_DOWN | 0x0153 | **缺失** |
| M_TREBLE_UP | 0x0154 | **缺失** |
| M_TREBLE_DOWN | 0x0155 | **缺失** |
| M_MEDIA_SEL | 0x0183 | 已有 AL_CC_CONFIG |
| M_MAIL | 0x018A | 已有 AL_EMAIL |
| M_CALCULATOR | 0x0192 | 已有 AL_CALCULATOR |
| M_MY_COMPUTER | 0x0194 | 已有 AL_LOCAL_BROWSER |
| M_WWW_SEARCH | 0x0221 | 已有 AC_SEARCH |
| M_WWW_HOME | 0x0223 | 已有 AC_HOME |
| M_WWW_BACK | 0x0224 | 已有 AC_BACK |
| M_WWW_FORWARD | 0x0225 | 已有 AC_FORWARD |
| M_WWW_STOP | 0x0226 | 已有 AC_STOP |
| M_WWW_REFRESH | 0x0227 | 已有 AC_REFRESH |
| M_WWW_FAVOURITES | 0x022A | 已有 AC_BOOKMARKS |
| M_POWER | 0x0030 | 属于 Generic Desktop Page，不纳入 consumer 补全 |
| M_IOS_VIRTUAL_KEYBOARD | 0x00B8 | 已有 TRANSPORT_EJECT，保留不变 |
| M_BACKLIGHT_UP | 0x006F | 已有 BRIGHTNESS_UP |
| M_BACKLIGHT_DOWN | 0x0070 | 已有 BRIGHTNESS_DOWN |
| M_EARTH | 0x029D | 已有 AC_NEXT_KEYBOARD_LAYOUT_SELECT，**但缺少 keycode 映射** |

### 2.2 需要补全的内容

共 7 项：
- 6 个新增 consumer usage + keycode + 映射
- 1 个已有 consumer usage 补充 keycode + 映射（地球键）

## 3. 修改方案

### 3.1 report.h - consumer_usages 枚举

在 `AUDIO_VOL_DOWN = 0x0EA` 之后新增 6 个枚举值：

```c
// 15.9.1 Audio Controls - Volume
AUDIO_MUTE     = 0x0E2,
AUDIO_VOL_UP   = 0x0E9,
AUDIO_VOL_DOWN = 0x0EA,
// 15.9.2 Audio Controls - Tone
AUDIO_BASS_BOOST  = 0x0E5,
AUDIO_LOUDNESS    = 0x0E7,
AUDIO_BASS_UP     = 0x152,
AUDIO_BASS_DOWN   = 0x153,
AUDIO_TREBLE_UP   = 0x154,
AUDIO_TREBLE_DOWN = 0x155,
```

命名规则：遵循现有 `AUDIO_*` 前缀惯例。

注意：`AUDIO_BASS_BOOST` 和 `AUDIO_LOUDNESS` 的 HID 值（0x0E5, 0x0E7）位于 `AUDIO_MUTE`（0x0E2）和 `AUDIO_VOL_UP`（0x0E9）之间，但为保持代码可读性，新增项统一放在 Volume 小节之后，以注释区分子类。

### 3.2 keycodes.h - keycode 定义

在 `KC_LAUNCHPAD = 0x00C2` 之后扩展：

```c
KC_BASS_BOOST      = 0x00C3,
KC_LOUDNESS        = 0x00C4,
KC_BASS_UP         = 0x00C5,
KC_BASS_DOWN       = 0x00C6,
KC_TREBLE_UP       = 0x00C7,
KC_TREBLE_DOWN     = 0x00C8,
KC_KEYBOARD_LAYOUT = 0x00C9,  // 地球键
```

别名区域新增：

```c
KC_BSBT = KC_BASS_BOOST,
KC_LOUD = KC_LOUDNESS,
KC_BSUP = KC_BASS_UP,
KC_BSDN = KC_BASS_DOWN,
KC_TRUP = KC_TREBLE_UP,
KC_TRDN = KC_TREBLE_DOWN,
KC_KBLY = KC_KEYBOARD_LAYOUT,
```

宏更新：

```c
// 原：
#define IS_CONSUMER_KEYCODE(code) ((code) >= KC_AUDIO_MUTE && (code) <= KC_LAUNCHPAD)
#define CONSUMER_KEYCODE_RANGE              KC_AUDIO_MUTE ... KC_LAUNCHPAD

// 改为：
#define IS_CONSUMER_KEYCODE(code) ((code) >= KC_AUDIO_MUTE && (code) <= KC_KEYBOARD_LAYOUT)
#define CONSUMER_KEYCODE_RANGE              KC_AUDIO_MUTE ... KC_KEYBOARD_LAYOUT
```

### 3.3 report.h - KEYCODE2CONSUMER() 函数

在 switch 中新增 7 个 case：

```c
case KC_BASS_BOOST:
    return AUDIO_BASS_BOOST;
case KC_LOUDNESS:
    return AUDIO_LOUDNESS;
case KC_BASS_UP:
    return AUDIO_BASS_UP;
case KC_BASS_DOWN:
    return AUDIO_BASS_DOWN;
case KC_TREBLE_UP:
    return AUDIO_TREBLE_UP;
case KC_TREBLE_DOWN:
    return AUDIO_TREBLE_DOWN;
case KC_KEYBOARD_LAYOUT:
    return AC_NEXT_KEYBOARD_LAYOUT_SELECT;
```

### 3.4 不需要修改的部分

| 文件 | 原因 |
|---|---|
| hidkbdservice.c HID 报告描述符 | usage range 0x0000~0x033C（828），新增最大值 0x155（341）在范围内 |
| report_extra_t 结构体 | usage 字段为 uint16_t，可容纳所有新值 |
| BLE 发送逻辑 | consumer report 是通用 16-bit usage 发送，无需改动 |
| desktop_usages 枚举 | M_POWER 属于 Generic Desktop Page，不纳入本次修改 |

## 4. 文件修改清单

| 文件 | 修改内容 |
|---|---|
| middleware/keyboard/report.h | consumer_usages 枚举新增 6 项 |
| middleware/keyboard/report.h | KEYCODE2CONSUMER() 新增 7 个 case |
| middleware/keyboard/keycodes.h | keycode 定义新增 7 项 |
| middleware/keyboard/keycodes.h | 别名新增 7 项 |
| middleware/keyboard/keycodes.h | IS_CONSUMER_KEYCODE / CONSUMER_KEYCODE_RANGE 上界更新 |

## 5. 验证要点

- 编译通过，无 keycode 值冲突
- 新增 keycode 值（0x00C3~0x00C9）不与其他 range 重叠
- consumer_usages 枚举值与 USB HID Usage Tables 标准一致
- KEYCODE2CONSUMER() 所有新 case 返回正确 usage 值
