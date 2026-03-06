## ADDED Requirements

### Requirement: CH584 Bluetooth driver owns BLE stack initialization

The system SHALL initialize CH584 BLE stack from the Bluetooth driver path instead of HAL common initialization.

#### Scenario: System startup enters communication init path

- **WHEN** system startup reaches application init stage
- **THEN** communication service is initialized and triggers wireless init event
- **AND** wireless init invokes Bluetooth driver init entry
- **AND** HAL common init does not execute BLE stack app init calls

### Requirement: Startup automatically enables advertising in Bluetooth mode

The system SHALL start advertising automatically after Bluetooth initialization succeeds in single channel mode.

#### Scenario: Boot in Bluetooth transport mode

- **WHEN** Bluetooth driver initialization completes successfully
- **THEN** device enters advertising state
- **AND** advertising can be observed by host scan

### Requirement: Disconnect triggers automatic re advertising

The system SHALL restart advertising after link termination when Bluetooth mode remains active.

#### Scenario: Link terminated by host or local side

- **WHEN** connection termination event is received
- **THEN** wireless state transitions to disconnected
- **AND** advertising restart is requested once
- **AND** duplicate advertising start requests are suppressed while already advertising

### Requirement: Reconnect follows event driven flow

The system SHALL process reconnect through existing event driven communication flow.

#### Scenario: Paired host reconnects after disconnect

- **WHEN** device is advertising after disconnect
- **THEN** reconnect establishes a new connection
- **AND** wireless state transitions to connected
- **AND** serial logs include reconnect success marker
