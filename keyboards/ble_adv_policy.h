#pragma once

#ifndef BLE_PAIRING_NAME
#error "BLE_PAIRING_NAME must be defined before including ble_adv_policy.h"
#endif

#ifndef BLE_ADV_MODE_AUTO
#define BLE_ADV_MODE_AUTO      0u
#endif

#ifndef BLE_ADV_MODE_LEGACY
#define BLE_ADV_MODE_LEGACY    1u
#endif

#ifndef BLE_ADV_MODE_EXTENDED
#define BLE_ADV_MODE_EXTENDED  2u
#endif

#ifndef BLE_ADV_MODE
#define BLE_ADV_MODE           BLE_ADV_MODE_AUTO
#endif

enum
{
    BLE_PAIRING_NAME_LEN = (int)(sizeof(BLE_PAIRING_NAME) - 1u),
    BLE_STORAGE_NAME_MAX_LEN = 22,
    BLE_LEGACY_COMPLETE_NAME_MAX_LEN = 20,
    BLE_EFFECTIVE_ADV_MODE =
        (BLE_ADV_MODE == BLE_ADV_MODE_EXTENDED) ? BLE_ADV_MODE_EXTENDED :
        (BLE_ADV_MODE == BLE_ADV_MODE_LEGACY) ? BLE_ADV_MODE_LEGACY :
        (BLE_PAIRING_NAME_LEN > BLE_LEGACY_COMPLETE_NAME_MAX_LEN) ? BLE_ADV_MODE_EXTENDED :
        BLE_ADV_MODE_LEGACY
};

_Static_assert(
    (BLE_ADV_MODE == BLE_ADV_MODE_AUTO) ||
        (BLE_ADV_MODE == BLE_ADV_MODE_LEGACY) ||
        (BLE_ADV_MODE == BLE_ADV_MODE_EXTENDED),
    "BLE_ADV_MODE must be AUTO, LEGACY, or EXTENDED");

_Static_assert(
    !((BLE_ADV_MODE == BLE_ADV_MODE_LEGACY) &&
      (BLE_PAIRING_NAME_LEN > BLE_LEGACY_COMPLETE_NAME_MAX_LEN)),
    "BLE_PAIRING_NAME exceeds the Legacy advertising complete-name budget");

_Static_assert(
    BLE_PAIRING_NAME_LEN <= BLE_STORAGE_NAME_MAX_LEN,
    "BLE_PAIRING_NAME exceeds the storage-backed name capacity");
