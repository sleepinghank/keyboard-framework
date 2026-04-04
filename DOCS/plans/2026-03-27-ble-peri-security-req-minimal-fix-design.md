# BLE PeriSecurityReq Minimal Fix Design

## Background

Current first-connect failures on iPad show this pattern:

- the peripheral starts advertising successfully
- the iPad establishes a BLE link
- the link drops quickly before any visible pairing-state callback
- unbonded disconnects are routed into an unconditional reconnect loop

The current implementation configures BondMgr with `GAPBOND_PAIRING_MODE_WAIT_FOR_REQ`, but no longer schedules the reference implementation's delayed peripheral security request after `GAP_LINK_ESTABLISHED_EVENT`.

## Goal

Use the smallest possible change set to restore the reference flow needed for first-connect investigation:

1. actively request security from the peripheral side after link establishment
2. stop the security-request retry path once pairing completes, bonding completes, bond-save completes, or the link drops
3. prevent unbonded/discoverable disconnects from being forced into `WL_RECONNECT_EVT`

## Non-Goals

- no global change from `WAIT_FOR_REQ` to `INITIATE`
- no transport-state-machine rewrite
- no advertising-intent redesign
- no bond-save semantic refactor beyond what is required for this investigation

## Design

### 1. Restore delayed `PERI_SECURITY_REQ_EVT`

Match the reference behavior by scheduling `PERI_SECURITY_REQ_EVT` shortly after `GAP_LINK_ESTABLISHED_EVENT`.

At event handling time:

- read the current GAP state
- if still connected, call `GAPBondMgr_PeriSecurityReq(hidEmuConnHandle)`
- log the request result
- if the stack reports the request was not accepted yet, re-arm the event with the same delay for a bounded retry-style behavior consistent with the reference path

This keeps pairing mode as `WAIT_FOR_REQ`, but uses the SDK's documented "slave security request" path to proactively drive security.

### 2. Stop security-request retries on terminal transitions

To avoid stale retries and noisy post-disconnect behavior:

- stop `PERI_SECURITY_REQ_EVT` when leaving `GAPROLE_CONNECTED`
- stop it again in `hidDevPairStateCB()` when pairing reaches `COMPLETE`, `BONDED`, or `BOND_SAVED`
- stop it on disconnect handling paths as a defensive cleanup

### 3. Split disconnect follow-up by session semantics

Current `WL_DISCONNECTED_EVT` handling always posts `WL_RECONNECT_EVT`.
For first-connect debugging this is harmful, because an unbonded pairing/discoverable failure is immediately converted into reconnect behavior.

Use a tiny pure helper for policy:

- `EVT_RECONNECTING` + bonded slot => auto reconnect allowed
- `EVT_DISCOVERABLE` => do not auto reconnect
- unbonded slot => do not auto reconnect

This keeps reconnect behavior for bonded reconnect sessions while letting first pairing failures remain in discoverable mode.

## Files

- Modify: `drivers/communication/bluetooth/ch584/hidkbd.c`
- Modify: `project/ch584m/Profile/hiddev.c`
- Modify: `application/service/communication_service.c`

## Verification

### Firmware-side expected logs

After flashing, first-connect investigation should show:

- `GAP_LINK_ESTABLISHED_EVENT`
- scheduled security request log
- `GAPBondMgr_PeriSecurityReq()` result log
- either pairing-state callbacks begin to appear, or the failure point becomes narrower and easier to identify

### Safety checks

- bonded reconnect behavior remains unchanged
- disconnect cleanup stops stale security-request retries
