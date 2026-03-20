# Keyboard Domain Knowledge

This document contains common knowledge for Bluetooth keyboard product specifications.
Use this as a reference when interpreting ambiguous or incomplete specifications.

## Bluetooth Terminology

### Channel Types

- **Single Channel (单通道)**: Keyboard can only connect to one device at a time. Switching
  to a new device requires re-pairing or forgetting the previous connection.
- **Multi-Channel (多通道)**: Keyboard can store multiple device pairings (typically 3-5)
  and switch between them using channel keys (e.g., Fn+1, Fn+2, Fn+3).

When a document mentions "channel switching keys" but also states "single channel", this is
typically a documentation error from template reuse. Clarify with the user.

### Connection Modes

- **Pairing Mode (配对模式/广播模式)**: Keyboard broadcasts and is discoverable by new devices.
  Indicated by fast LED flashing (typically 3 times/second).
- **Reconnection Mode (回连模式)**: Keyboard attempts to reconnect to the last paired device.
  Indicated by slow LED flashing (typically 1 time/second).
- **Polling Mode (轮询模式)**: A hybrid mode where the keyboard advertises while also accepting
  connections from previously paired devices. This replaces standard reconnection when the
  keyboard needs to accept new connections during reconnection attempts.

### Reconnection vs Polling

**Standard Reconnection:**
- Only attempts to connect to the specific previously-paired device
- Will not accept connections from other devices
- Times out after a set period (typically 60s) and enters sleep

**Polling Mode (replaces reconnection):**
- Advertises in a way that allows new connections
- Can connect to any device that initiates connection
- More flexible but may connect to unintended devices
- Common when single-channel keyboards need to work with multiple devices interchangeably

### Typical Timing Parameters

| Parameter | Typical Value | Range |
|-----------|---------------|-------|
| Pairing timeout | 60s | 30-120s |
| Reconnection timeout | 60s | 30-120s |
| Connection completion | 3s | 1-5s |
| Deep sleep entry | 10min | 5-30min |
| Backlight timeout | 5s | 3-30s |
| Long press threshold | 3s | 2-5s |

## HID Usage Codes

### Consumer Control (Media Keys)

| Function | HID Usage |
|----------|-----------|
| Volume Up | 0xE9 |
| Volume Down | 0xEA |
| Mute | 0xE2 |
| Play/Pause | 0xCD |
| Next Track | 0xB5 |
| Previous Track | 0xB6 |
| Brightness Up | 0x6F |
| Brightness Down | 0x70 |
| Screenshot | System-specific |
| Search/Spotlight | 0x221 |
| Siri/Voice Assistant | 0xCF |
| App Switch/Mission Control | System-specific |

### Special Keys (Apple/iPad)

| Key | Function |
|-----|----------|
| Globe Key (地球键) | Input source switch (single press), special functions (long press + character) |
| Power Key | Lock screen (single press), power on/off (long press 3s) |
| Fn Key | Modifier for function layer |

## Custom Key Functions

Common functions that can be assigned to customizable keys:

| Function | Description | HID Implementation |
|----------|-------------|-------------------|
| Siri | Invoke voice assistant | Consumer Control 0xCF |
| Previous Track | Previous media | Consumer Control 0xB6 |
| Next Track | Next media | Consumer Control 0xB5 |
| Play/Pause | Toggle playback | Consumer Control 0xCD |
| Brightness Up | Increase screen brightness | Consumer Control 0x6F |
| Brightness Down | Decrease screen brightness | Consumer Control 0x70 |
| Volume Up | Increase volume | Consumer Control 0xE9 |
| Volume Down | Decrease volume | Consumer Control 0xEA |
| Mute | Toggle mute | Consumer Control 0xE2 |
| Screenshot | Capture screen | System-specific shortcut |
| App Switch | Show running apps | System-specific |

## Touchpad Specifications

### Registration Types

- **Touchpad Protocol (触控板报文)**: Registers as a native trackpad device. Supports all
  system gestures natively. Required for full iPad/macOS gesture support.
- **Mouse Emulation (模拟鼠标)**: Emulates a mouse device. Limited gesture support, typically
  only basic pointer movement and clicks.

### Gesture Systems

- **ZWSN Gestures**: A proprietary gesture customization system. Allows mapping finger
  count gestures to custom actions.
- **System Native Gestures**: OS-provided gestures (pinch, swipe, scroll, etc.) that work
  when registered as a touchpad device.

### Common Touchpad Features

| Feature | Description |
|---------|-------------|
| Multi-touch points | Number of simultaneous touch points (typically 5) |
| Press-to-click | Physical button press registers as click |
| Drag without drop | Files don't drop during single-finger drag |
| Palm rejection | Prevents accidental touch during typing |
| Wake on touch | Touch can wake keyboard from sleep |
| Simultaneous use | Touchpad works while keys are held |

## Power Management

### States

| State | Description | Typical Current |
|-------|-------------|-----------------|
| Active (工作) | Full operation, backlight on | 5-15mA |
| Idle (空闲) | Connected, waiting for input | 0.5-2mA |
| Sleep (休眠) | Disconnected, can wake on keypress | 20-100μA |
| Power Off (关机) | Completely off | 0μA |

### Battery Protection

Standard lithium battery protections:
- Over-voltage protection (过压保护)
- Over-current protection (过流保护)
- Short-circuit protection (短路保护)
- Over-discharge protection (过放保护)

### Low Battery Behavior

Common low battery behaviors (typically < 20%):
- LED indicator flashing (red)
- Backlight forced off
- Reduced functionality
- More aggressive sleep

## Backlight Specifications

### Common Color Options

| Color | Chinese | RGB Approximate |
|-------|---------|-----------------|
| Red | 红 | (255, 0, 0) |
| Yellow | 黄 | (255, 255, 0) |
| Cyan | 青 | (0, 255, 255) |
| Blue | 蓝 | (0, 0, 255) |
| Green | 绿 | (0, 255, 0) |
| White | 白 | (255, 255, 255) |
| Magenta/Purple | 洋红/紫 | (255, 0, 255) |

### Brightness Levels

Typical brightness levels: Off → Level 1 → Level 2 → Level 3 (cycling)

### Memory Behavior

- Color setting typically persists across power cycles
- Brightness setting may or may not persist (clarify with user)
- Factory reset restores default (typically white, level 2)

## Firmware Upgrade

### OTA (Over-The-Air)

- Upgrade via Bluetooth connection
- Requires companion APP
- Device enters DFU mode during upgrade
- Typical for modern Bluetooth keyboards

### Wired Upgrade

- Upgrade via USB connection
- May require special software or bootloader mode
- Less common for Bluetooth-only keyboards

## Indicator LED Patterns

### Standard Flash Patterns

| Pattern | Chinese | Rate | Meaning |
|---------|---------|------|---------|
| Fast flash | 快闪 | 3 times/sec | Pairing mode |
| Slow flash | 慢闪 | 1 time/sec | Reconnection mode |
| Solid on | 常亮 | Continuous | Status confirmation |
| Off | 熄灭 | - | Normal/idle state |

### Common LED Colors and Meanings

| LED | Color | Typical Meaning |
|-----|-------|-----------------|
| Caps Lock LED | White | Caps lock on/Bluetooth status |
| Power LED | Green | Fully charged/Normal battery |
| Power LED | Red | Charging/Low battery |

## Language Variant Considerations

### Layout Differences

| Variant | Key Differences |
|---------|-----------------|
| US | Standard QWERTY, Del key |
| DE | QWERTZ (Y/Z swapped), Entf instead of Del, ÄÖÜ keys |
| UK | Different Enter shape, £ key, different @ position |
| JP | Additional keys (¥, ろ, む, け), different Enter shape, 半角/全角 key |
| FR | AZERTY layout, Suppr instead of Del |

### Special Character Combinations

Each language variant may require Fn or Opt combinations to produce characters that are
directly available on the native physical keyboard but not present on the compact layout.

Common examples:
- DE: Fn+Y for <, Fn+X for >, Opt+E for €, Opt+L for @
- UK: Fn+Z for \, Fn+X for |, Opt+2 for €, Opt+3 for #
- FR: Fn+W for <, Fn+X for >
