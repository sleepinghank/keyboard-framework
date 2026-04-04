# BLE Compile-Time Advertising Path Fix Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 修复 `BLE_PAIRING_NAME` 导致的广播越界问题，并在编译期固定选择 Legacy 或 Extended Connectable Advertising，运行时不再切换广播类型。

**Architecture:** 新增一个纯编译期策略头文件，根据 `BLE_PAIRING_NAME` 长度和 `BLE_ADV_MODE` 计算固件最终广播路径。`hidkbd.c` 收敛为统一的载荷构建/下发入口：Legacy 路径继续使用 `ADV_IND + scan response`，Extended 路径固定使用 `EXT_CONN_UNDIRECT`，并把名称与附加字段都装入 `advertData`，`scanRspData` 清空。这里把 “connectable extended advertising 不依赖 scan response” 视为基于 SDK 事件类型拆分的实现假设，后续用抓包验证。

**Tech Stack:** C11、CH58x BLE SDK（GAPRole）、MSYS2 MinGW64 `gcc` 主机测试、CMake/MinGW 固件构建

**Spec:** `docs/superpowers/specs/2026-03-26-ble-compile-time-extended-advertising-design.md`

---

## File Structure

| 操作 | 文件路径 | 责任 |
|------|----------|------|
| 新建 | `keyboards/ble_adv_policy.h` | 纯编译期广播模式决策、长度预算、编译期保护 |
| 新建 | `test/hidkbd_adv_mode_test.c` | 主机侧验证 `AUTO / LEGACY / EXTENDED` 选择逻辑 |
| 修改 | `keyboards/defaults/ble_defaults.h` | 暴露 `BLE_ADV_MODE` 默认值和相关注释 |
| 修改 | `keyboards/config_check.h` | 统一引入广播模式编译期校验 |
| 修改 | `drivers/storage/storage.c` | 收回 BLE 名称初始化时多减掉的 1 字节余量 |
| 修改 | `drivers/communication/bluetooth/ch584/hidkbd.c` | 重建 Legacy / Extended 载荷、统一下发、复用现有状态机 |

---

## Task 1: 抽取编译期广播策略并建立主机测试

**Files:**
- Create: `keyboards/ble_adv_policy.h`
- Create: `test/hidkbd_adv_mode_test.c`

- [ ] **Step 1: 写主机侧失败测试，先锁定当前需求**

```c
#include <stdio.h>

#ifndef BLE_PAIRING_NAME
#define BLE_PAIRING_NAME "inateck NeoMagic Pro"
#endif

#ifndef EXPECTED_ADV_MODE
#error "EXPECTED_ADV_MODE must be defined for hidkbd_adv_mode_test"
#endif

#include "ble_adv_policy.h"

static int failures = 0;

static void expect_true(int condition, const char *message)
{
    if(condition)
    {
        printf("[PASS] %s\n", message);
    }
    else
    {
        failures++;
        printf("[FAIL] %s\n", message);
    }
}

int main(void)
{
    expect_true(BLE_EFFECTIVE_ADV_MODE == EXPECTED_ADV_MODE,
                "effective advertising mode matches expectation");
    expect_true(BLE_LEGACY_COMPLETE_NAME_MAX_LEN == 20u,
                "legacy complete-name budget stays at 20 bytes");
    expect_true(BLE_STORAGE_NAME_MAX_LEN == 22u,
                "storage name capacity stays aligned at 22 bytes");
    printf("failures=%d\n", failures);
    return failures == 0 ? 0 : 1;
}
```

- [ ] **Step 3: 创建纯编译期策略头文件**

```c
#pragma once

#ifndef BLE_PAIRING_NAME
#error "BLE_PAIRING_NAME must be defined before including ble_adv_policy.h"
#endif

#define BLE_ADV_MODE_AUTO      0u
#define BLE_ADV_MODE_LEGACY    1u
#define BLE_ADV_MODE_EXTENDED  2u

#ifndef BLE_ADV_MODE
#define BLE_ADV_MODE BLE_ADV_MODE_AUTO
#endif

enum
{
    BLE_PAIRING_NAME_LEN = (unsigned int)(sizeof(BLE_PAIRING_NAME) - 1u),
    BLE_STORAGE_NAME_MAX_LEN = 22u,
    BLE_LEGACY_COMPLETE_NAME_MAX_LEN = 20u,
    BLE_EFFECTIVE_ADV_MODE =
        (BLE_ADV_MODE == BLE_ADV_MODE_EXTENDED) ? BLE_ADV_MODE_EXTENDED :
        (BLE_ADV_MODE == BLE_ADV_MODE_LEGACY) ? BLE_ADV_MODE_LEGACY :
        (BLE_PAIRING_NAME_LEN > BLE_LEGACY_COMPLETE_NAME_MAX_LEN) ? BLE_ADV_MODE_EXTENDED :
        BLE_ADV_MODE_LEGACY
};

#if (BLE_ADV_MODE != BLE_ADV_MODE_AUTO) && \
    (BLE_ADV_MODE != BLE_ADV_MODE_LEGACY) && \
    (BLE_ADV_MODE != BLE_ADV_MODE_EXTENDED)
#error "BLE_ADV_MODE must be AUTO, LEGACY, or EXTENDED"
#endif

#if (BLE_ADV_MODE == BLE_ADV_MODE_LEGACY) && \
    (BLE_PAIRING_NAME_LEN > BLE_LEGACY_COMPLETE_NAME_MAX_LEN)
#error "BLE_PAIRING_NAME exceeds the Legacy advertising complete-name budget"
#endif

#if (BLE_PAIRING_NAME_LEN > BLE_STORAGE_NAME_MAX_LEN)
#error "BLE_PAIRING_NAME exceeds the storage-backed name capacity"
#endif
```

- [ ] **Step 5: 提交本任务**

```bash
git add keyboards/ble_adv_policy.h test/hidkbd_adv_mode_test.c
git commit -m "test: add compile-time BLE advertising mode policy checks"
```

---

## Task 2: 接入默认配置与全局编译期校验

**Files:**
- Modify: `keyboards/defaults/ble_defaults.h`
- Modify: `keyboards/config_check.h`
- Modify: `drivers/storage/storage.c`

- [ ] **Step 1: 在默认蓝牙配置中暴露广播模式宏**

```c
/* ========== 广播模式配置 ========== */
#ifndef BLE_ADV_MODE
#define BLE_ADV_MODE BLE_ADV_MODE_AUTO
#endif
```

同时在注释中说明：

- `AUTO`：名字 `<= 20` 走 Legacy
- `LEGACY`：强制走 Legacy，超预算直接编译失败
- `EXTENDED`：强制走 Extended Connectable

- [ ] **Step 2: 在 `config_check.h` 中统一引入策略校验**

```c
/* ========== BLE 广播模式编译期校验 ========== */
#include "ble_adv_policy.h"
```

要求放在 `BLE_PAIRING_NAME` 和 `BLE_ADV_MODE` 已可见的位置之后，
让所有包含 `config_product.h` 的编译单元都能尽早失败。

- [ ] **Step 3: 修正 storage 默认初始化的 1 字节保守裁剪**

将 `drivers/storage/storage.c` 中：

```c
if (default_config.ble_name_len > sizeof(default_config.ble_name_data) - 1) {
    default_config.ble_name_len = sizeof(default_config.ble_name_data) - 1;
}
memcpy(default_config.ble_name_data, BLE_PAIRING_NAME, default_config.ble_name_len);
```

改为：

```c
memset(default_config.ble_name_data, 0, sizeof(default_config.ble_name_data));
if (default_config.ble_name_len > sizeof(default_config.ble_name_data)) {
    default_config.ble_name_len = sizeof(default_config.ble_name_data);
}
memcpy(default_config.ble_name_data, BLE_PAIRING_NAME, default_config.ble_name_len);
```

目的：

- 利用完整 22 字节存储容量
- 保持剩余字节清零，避免旧数据残留


- [ ] **Step 5: 提交本任务**

```bash
git add keyboards/defaults/ble_defaults.h keyboards/config_check.h drivers/storage/storage.c
git commit -m "feat: add compile-time BLE advertising mode configuration"
```

---

## Task 3: 在 `hidkbd.c` 中重建固定广播载荷

**Files:**
- Modify: `drivers/communication/bluetooth/ch584/hidkbd.c`

- [ ] **Step 1: 引入策略头并替换固定 31 字节静态布局**

在 `hidkbd.c` 顶部新增：

```c
#include "ble_adv_policy.h"

#if (BLE_EFFECTIVE_ADV_MODE == BLE_ADV_MODE_EXTENDED)
#define HIDEMU_ADV_BUFFER_CAPACITY B_MAX_ADV_EXT_LEN
#else
#define HIDEMU_ADV_BUFFER_CAPACITY 31u
#endif

static uint8_t advertData[HIDEMU_ADV_BUFFER_CAPACITY];
static uint8_t scanRspData[HIDEMU_ADV_BUFFER_CAPACITY];
static uint16_t advertDataLen = 0;
static uint16_t scanRspLen = 0;
static uint8_t advEventType = GAP_ADTYPE_ADV_IND;
```

并删除当前写死名字的 `scanRspData[31]` / `advertData[]` 初始化字面量。

- [ ] **Step 2: 写最小载荷拼接 helper**

在 `hidkbd.c` 内新增两个静态 helper：

```c
static uint16_t hidEmu_append_ad_structure(uint8_t *buffer,
                                           uint16_t offset,
                                           uint8_t ad_type,
                                           const uint8_t *payload,
                                           uint8_t payload_len);

static void hidEmu_build_adv_payload(const uint8_t *ble_name,
                                     uint8_t ble_name_len);
```

`hidEmu_append_ad_structure()` 负责按 `[len][type][payload...]` 形式追加 AD 结构，
`hidEmu_build_adv_payload()` 负责根据编译期模式构建两条固定路径：

Legacy 路径：

```c
advertData  = Flags + Appearance + Manufacturer + HID/BATT UUID
scanRspData = Conn Interval + Tx Power + Complete Local Name
advEventType = GAP_ADTYPE_ADV_IND
```

Extended 路径：

```c
advertData  = Flags + Appearance + Manufacturer + HID/BATT UUID +
              Conn Interval + Tx Power + Complete Local Name
scanRspData = empty
advEventType = GAP_ADTYPE_EXT_CONN_UNDIRECT
```

- [ ] **Step 3: 写统一下发 helper，取代散落的 `SetParameter()`**

```c
static void hidEmu_apply_adv_payload(const uint8_t *ble_name,
                                     uint8_t ble_name_len)
{
    hidEmu_build_adv_payload(ble_name, ble_name_len);
    GAPRole_SetParameter(GAPROLE_ADV_EVENT_TYPE, sizeof(advEventType), &advEventType);
    GAPRole_SetParameter(GAPROLE_ADVERT_DATA, advertDataLen, advertData);
    GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, scanRspLen, scanRspData);
}
```

这一步的目标是把广播类型和广播内容变成“一次性提交”，避免不同路径互相覆盖。

- [ ] **Step 4: 用日志把当前模式打出来，便于板级验证**

加入一条固定调试日志：

```c
PRINT("GAP: adv_mode=%u adv_event_type=0x%02X adv_len=%u scan_len=%u name_len=%u\n",
      (unsigned)BLE_EFFECTIVE_ADV_MODE,
      (unsigned)advEventType,
      (unsigned)advertDataLen,
      (unsigned)scanRspLen,
      (unsigned)ble_name_len);
```

Expected:

- 当前产品名 20 字节时，日志显示 `adv_mode=1`（Legacy）
- 将名称临时改成 21/22 字节时，日志显示 `adv_mode=2`（Extended）

- [ ] **Step 5: 提交本任务**

```bash
git add drivers/communication/bluetooth/ch584/hidkbd.c
git commit -m "feat: build fixed BLE payloads for legacy and extended advertising"
```

---

## Task 4: 让所有开播路径复用同一个固定模式载荷

**Files:**
- Modify: `drivers/communication/bluetooth/ch584/hidkbd.c`

- [ ] **Step 1: 让 `hidEmu_apply_advert_discoverability_flags()` 只改状态，不直接下发广告包**

把：

```c
GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(advertData), advertData);
```

从该函数中删除，只保留：

- 根据 `mode` 更新 Flags 值
- 更新 `discoverability_mode`

这样避免它在统一 helper 之外单独覆盖 `advertData`。

- [ ] **Step 2: 在 `HidEmu_Init()` 中改为统一 helper 下发**

把当前这段手写名字拷贝逻辑：

```c
scanRspData[15] = ble_name_len + 1;
scanRspData[16] = GAP_ADTYPE_LOCAL_NAME_COMPLETE;
tmos_memcpy(&scanRspData[17], ble_pairing_name, ble_name_len);
scanRspLen = 17 + ble_name_len;
GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(advertData), advertData);
GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, scanRspLen, scanRspData);
```

替换为：

```c
hidEmu_apply_adv_payload((const uint8_t *)ble_pairing_name, (uint8_t)ble_name_len);
GGS_SetParameter(GGS_DEVICE_NAME_ATT, ble_name_len, (void *)ble_pairing_name);
```

- [ ] **Step 3: 在 `hidEmu_start_advertising_for_purpose()` 中统一重建载荷**

在地址策略和 discoverability 设置之后，开播之前插入：

```c
hidEmu_apply_adv_payload((const uint8_t *)BLE_PAIRING_NAME,
                         (uint8_t)(sizeof(BLE_PAIRING_NAME) - 1u));
```

要求顺序固定为：

1. `hidDev_adv_enable(DISABLE)`
2. 地址/绑定策略
3. discoverability 标记
4. `hidEmu_apply_adv_payload(...)`
5. `hidDevInitialAdvertising()`

- [ ] **Step 4: 修正 `GAPROLE_WAITING` 里的自动续播**

当前 `GAP_END_DISCOVERABLE_DONE_EVENT` 分支里有：

```c
hidEmu_apply_advert_discoverability_flags(2);
hidDevInitialAdvertising();
```

改为：

```c
hidEmu_apply_advert_discoverability_flags(2);
hidEmu_apply_adv_payload((const uint8_t *)BLE_PAIRING_NAME,
                         (uint8_t)(sizeof(BLE_PAIRING_NAME) - 1u));
hidDevInitialAdvertising();
```

目标是保证：

- 断链重播
- Limited 超时后的续播
- 手动停播再重开

都只会重启“当前编译产物固定的广播模式”。

- [ ] **Step 5: 提交本任务**

```bash
git add drivers/communication/bluetooth/ch584/hidkbd.c
git commit -m "refactor: reuse fixed BLE advertising payload across restart paths"
```

---

## Task 5: 主机测试、构建验证与板级回归

**Files:**
- Test: `test/hidkbd_adv_mode_test.c`
- Verify: `drivers/communication/bluetooth/ch584/hidkbd.c`
- Verify: `keyboards/ble_adv_policy.h`

- [ ] **Step 1: 跑主机侧三条核心检查**

Run: `gcc -I keyboards -DEXPECTED_ADV_MODE=BLE_ADV_MODE_LEGACY -o test/hidkbd_adv_mode_test.exe test/hidkbd_adv_mode_test.c && ./test/hidkbd_adv_mode_test.exe`

Expected: PASS

Run: `gcc -I keyboards -DBLE_PAIRING_NAME='"ABCDEFGHIJKLMNOPQRSTU"' -DEXPECTED_ADV_MODE=BLE_ADV_MODE_EXTENDED -o test/hidkbd_adv_mode_test.exe test/hidkbd_adv_mode_test.c && ./test/hidkbd_adv_mode_test.exe`

Expected: PASS

Run: `gcc -I keyboards -DBLE_PAIRING_NAME='"ABCDEFGHIJKLMNOPQRSTU"' -DBLE_ADV_MODE=BLE_ADV_MODE_LEGACY -DEXPECTED_ADV_MODE=BLE_ADV_MODE_LEGACY -o test/hidkbd_adv_mode_test.exe test/hidkbd_adv_mode_test.c`

Expected: FAIL（预期编译失败）

- [ ] **Step 2: 固件编译验证**

- 使用wch-riscv-build 进行固件编译

- [ ] **Step 3: 板级验证当前产品名（20 字节）**

手工动作：

1. 烧录当前产品配置固件
2. 打开串口日志
3. 用支持 BLE 的手机或 PC 扫描并连接

Expected:

- 日志显示 `adv_mode=1`
- 广播类型对应 `ADV_IND`
- 设备名称完整显示为 `inateck NeoMagic Pro`
- 可正常连接

- [ ] **Step 4: 板级验证 21/22 字节名称的 Extended 路径**

临时把 `BLE_PAIRING_NAME` 改成 21 或 22 字节字符串后重新编译烧录，重复上一步。

Expected:

- 日志显示 `adv_mode=2`
- 广播类型对应 `EXT_CONN_UNDIRECT`
- 名称可被支持扩展广播的扫描端看到
- 设备仍可连接
- `scanRspLen=0`

- [ ] **Step 5: 清理仅用于验证的冗余日志并提交收尾**

```bash
git add keyboards/ble_adv_policy.h keyboards/defaults/ble_defaults.h keyboards/config_check.h drivers/storage/storage.c drivers/communication/bluetooth/ch584/hidkbd.c test/hidkbd_adv_mode_test.c
git commit -m "fix: select BLE legacy or extended advertising at compile time"
```

---

## Acceptance Checklist

- [ ] `BLE_ADV_MODE` 已在 `ble_defaults.h` 暴露默认值
- [ ] `keyboards/ble_adv_policy.h` 能在编译期选择 `AUTO / LEGACY / EXTENDED`
- [ ] `BLE_PAIRING_NAME` 超过 Legacy 预算时不会再生成非法 Legacy 固件
- [ ] `BLE_PAIRING_NAME` 超过 storage 可承载上限时会在编译期失败
- [ ] `storage.c` 默认名称初始化已使用完整 22 字节容量
- [ ] `hidkbd.c` 不再手写拼接固定 31 字节名称布局
- [ ] Legacy 模式下 UUID 已移到 `advertData`
- [ ] Extended Connectable 模式下名称与附加字段都位于 `advertData`，`scanRspLen=0`
- [ ] 所有重播/续播路径都复用同一个固定广播模式
- [ ] 主机测试通过
- [ ] 固件构建未新增编译错误
- [ ] 实机扫描与连接验证通过

---

## Notes

- `openspec/AGENTS.md` 在当前仓库中不存在，本计划按仓库现有 `docs/superpowers/plans` 约定编写。
- `drivers/ota/Inateck_tag_config.c` 中的 BLE 改名能力仍为注释状态，暂不纳入本次实现范围。
- 如果后续要支持超过 22 字节的产品名，需要单独扩展 storage 容量与 OTA 名称校验，这不属于本计划。
