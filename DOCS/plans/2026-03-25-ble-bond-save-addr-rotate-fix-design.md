# BLE Bond Save Addr Rotate Fix Design

## Background

Current BLE address generation is stable for reconnect as long as `ble_addr_ver[]` does not change.
The observed bug is caused by reconnect sessions re-entering `hidEmu_save_ble_bonded(..., 1)`,
which incorrectly increments the persisted address version and shifts the derived static address
after reboot.

## Root Cause

`GAPBOND_PAIRING_STATE_BOND_SAVED` currently hardcodes `hidEmu_save_ble_bonded(BLE_INDEX_1, 1)`.
That means every successful bond-save callback is treated as an address-rotating pairing success,
including reconnect sessions.

## Chosen Fix

Use a one-shot `addr_rotate_pending` policy:

1. `bt_driver_pairing_ex()` arms address rotation for the active BLE slot.
2. `bt_driver_connect_ex()` clears address rotation for the active BLE slot.
3. `GAPBOND_PAIRING_STATE_BOND_SAVED` consumes the pending flag once and passes the consumed value
   into `hidEmu_save_ble_bonded(...)`.
4. Repeated `BOND_SAVED` callbacks during reconnect must not advance `ble_addr_ver[]`.

## Touch Points

- `drivers/communication/bluetooth/ch584/_bt_driver.c`
- `drivers/communication/bluetooth/ch584/hidkbd.c`
- `drivers/communication/bluetooth/ch584/hidkbd.h`
- `project/ch584m/Profile/hiddev.c`
- `test/hidkbd_bond_policy_test.c`

## Verification

1. Pairing path: pending flag is consumed exactly once, first consume returns 1, second returns 0.
2. Reconnect path: consume always returns 0.
3. Existing address derivation logic remains unchanged.
