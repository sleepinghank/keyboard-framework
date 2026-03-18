## ADDED Requirements
### Requirement: Activity-aware Fn combo shortcuts
The keyboard system SHALL expose four Fn combo shortcuts that invoke backlight, battery, and Siri-related actions.

#### Scenario: Fn plus right Shift increases backlight level
- **WHEN** the user presses `Fn + Right Shift`
- **THEN** the system SHALL run the backlight level-up action
- **AND** the action SHALL notify keyboard backlight activity

#### Scenario: Fn plus right Enter advances backlight color
- **WHEN** the user presses `Fn + Right Enter`
- **THEN** the system SHALL run the backlight color-next action
- **AND** the action SHALL only change color when backlight is enabled
- **AND** the action SHALL notify keyboard backlight activity

#### Scenario: Fn plus right Cmd invokes Siri
- **WHEN** the user presses `Fn + Right Cmd`
- **THEN** the system SHALL emit the Siri consumer usage code

#### Scenario: Fn plus application key checks battery
- **WHEN** the user presses `Fn + KC_APPLICATION`
- **THEN** the system SHALL read the current battery percentage
- **AND** the system SHALL flash the red indicator using a custom 200 ms on / 200 ms off pattern

