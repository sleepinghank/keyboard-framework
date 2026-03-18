## ADDED Requirements

### Requirement: KB904 Backlight Expansion And Idle Sleep
The system SHALL provide 13 preset backlight colors, 4 preset brightness levels, and SHALL turn off backlight after 5 seconds of no activity.

#### Scenario: Backlight color cycle
- **WHEN** user triggers KB904 color combo repeatedly
- **THEN** color cycles through 13 presets and wraps to red

#### Scenario: Backlight idle sleep
- **WHEN** no keyboard or touch activity happens for 5 seconds
- **THEN** backlight turns off and wakes on next activity if not in low-battery mode

### Requirement: KB904 Indicator Mapping
The system SHALL support a 4-LED indicator mapping for KB904 while preserving compatibility with 2-LED products.

#### Scenario: Product-specific LED layout
- **WHEN** `PRODUCT_ID` equals `0x0904`
- **THEN** CAPS, BT, POWER_RED, POWER_GRN aliases and hardware mapping are enabled

### Requirement: KB904 Combo Actions
The system SHALL provide combo actions for brightness, color, battery check, and Siri invoke.

#### Scenario: Battery check blink count
- **WHEN** user triggers battery check combo
- **THEN** power LED blinks 1 to 4 times according to battery percentage bucket

### Requirement: Low Battery Linkage And Factory Reset
The system SHALL link low-battery state to indicator/backlight behavior and SHALL support factory reset by long-press combo.

#### Scenario: Low battery linkage
- **WHEN** power state changes to low (`<=20%`)
- **THEN** low-battery indicator request is sent and backlight is forced off

#### Scenario: Battery recovered
- **WHEN** power state changes from low to normal
- **THEN** normal-battery indicator request is sent and backlight restriction is removed

#### Scenario: Factory reset flow
- **WHEN** factory reset combo is triggered
- **THEN** system performs indicator feedback, clears BLE bonding, resets storage and backlight defaults, then resets system
