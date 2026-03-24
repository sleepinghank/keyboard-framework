# BLE 广播名称改为从 Storage 动态读取 — 设计文档

## 需求摘要

将 CH584 蓝牙广播中的设备名称从 `hidkbd.c` 内的硬编码改为运行时从 `storage` 读取，
保证广播名称、GATT Device Name 与 `BLE_PAIRING_NAME` 同源。

## 现状分析

- 当前扫描响应包名称字段在 `drivers/communication/bluetooth/ch584/hidkbd.c` 中硬编码为 `inateckKB904`
- `system_init_drivers()` 已在 `storage_init()` 后将 `BLE_PAIRING_NAME` 写入 `storage`
- `GGS_DEVICE_NAME_ATT` 当前已从 `storage_get_config_ptr()->ble_name_*` 设置
- `hidEmu_update_device_name()` 也会从 `storage` 读取名称，但目前没有统一在初始化和广播开启路径调用
- `hidEmu_update_device_name()` 中 `GAP_UpdateAdvertisingData()` 传入的长度若仅覆盖「名称 AD 结构」而非整包 `scanRspData`，易与栈语义不一致；合并 helper 后应以**重建后的完整扫描响应长度**为准

## 关键约束

- BLE Legacy Advertising / Scan Response 单包最大 31 字节
- **当前布局下** `scanRspData` 除名称外还包含：
  - 连接间隔范围
  - HID / Battery Service UUID（16-bit）
  - Tx Power
- 上述固定尾部合计 **15 字节**（不含名称 AD 结构的长度字节与类型字节）
- 名称 AD 结构占用：`1（长度） + 1（类型） + N（名称）`，Legacy 下总约束为  
  `2 + N + 15 ≤ 31` → **`N ≤ 14`**
- `storage` 中 `ble_name_data` 最大 **22** 字节，因此在**不改动 AD 布局**时：「Legacy 完整展示 22 字节名称」与「保留当前全部附加字段」不能同时成立

## AD 布局优化（优先实施，推荐）

在引入 Extended 广播之前，优先通过 **AD 重排** 提高 Legacy 扫描响应中可容纳的名称长度，减少兼容性与实现复杂度。

### 做法

- 将 **HID / Battery 的 16-bit Service UUID** 从 `scanRspData` 挪至 **`advertData`**（`hidkbd.c` 中广播数据段仍有空余，历史上曾注释掉同类字段）
- 扫描响应中保留：**完整本地名称**、**Slave Connection Interval Range**、**Tx Power**（与产品需求一致时可再评估是否必须全部保留）

### 效果（Legacy）

- 从扫描响应中移除 UUID 块约 **6 字节** 后，名称最大长度由 **14** 提升至约 **20** 字节（在仍保留连接间隔 + Tx Power 的前提下，`2 + N + 9 ≤ 31`）
- 与 `ble_name_data[22]` 对齐：多数产品名可在 **纯 Legacy** 下完整展示；仅当名称仍超过约 20 字节（或选择保留更多 AD 导致预算更紧）时，再考虑 Extended 后备策略

### 风险与说明

- 服务 UUID 出现在 **广播包（Advertising）** 而非扫描响应，为常见做法；若需与历史抓包行为逐字节一致，需在变更说明中注明
- 实施时 **`advertData` 与 `scanRspData` 需同步维护**，避免重复或遗漏 UUID

## 候选方案

### 方案 A（基线）

保留**当前**扫描响应包结构（名称 + 连接间隔 + UUID + Tx Power），仅把名称改为从 `storage` 读取。

- Legacy 名称上限：**14** 字节（超长截断）
- 优点：改动面最小
- 缺点：广播展示名称偏短；与「AD 重排」相比未利用 `advertData` 空余

### 方案 A+（首选基线：A + AD 重排）

在方案 A 基础上增加 **AD 重排**（见上一节）：UUID 进 `advertData`，名称仍从 `storage` 读取。

- Legacy 名称上限：约 **20** 字节（在保留连接间隔 + Tx Power 的典型布局下）
- GATT Device Name：继续使用完整 `storage` 名称
- 优点：多数场景**无需 Extended**；主机兼容性优于 Extended
- 缺点：广播/扫描响应字节布局与旧固件抓包不完全一致

### 方案 B

扫描响应包仅发送动态名称等字段，不再附带连接间隔 / UUID / Tx Power（或大幅删减）。

- 优点：Legacy 下可更长名称
- 缺点：行为变化大，需单独评估连接与发现行为

### 方案 C（后备：Extended）

当在 **已采用方案 A+ 布局** 的前提下，名称**仍**超过 Legacy 可容纳长度时，再切换为 **Extended 可扫描广播**（`GAP_ADTYPE_EXT_SCAN_UNDIRECT`），以在单包预算内携带完整名称与既定附加字段。

- 优点：可展示完整长名称且保留附加字段
- 缺点：
  - 需在广播参数变更时按栈要求处理（如先关闭广播再改参数等，**以沁恒例程与实测为准**）
  - 扫描端对 BLE 5 Extended 的支持程度不一（见「风险提示」）

## 推荐方案

采用 **方案 A+ 作为默认实现路径**，**方案 C 作为可选后备**（仅当名称在 A+ 布局下仍超出 Legacy 预算时启用）。

原因简述：

- A+ 在多数产品名长度下可避免 Extended，降低「旧手机/工具只认 Legacy」的风险
- SDK 已暴露 `GAPROLE_ADV_EVENT_TYPE`、`GAP_ADTYPE_EXT_SCAN_UNDIRECT`、`GAPROLE_*_DATA` 与 `B_MAX_ADV_EXT_LEN`，后备路径可继续复用 GAPRole，无需自研整套广播层
- 若产品强制要求「任何长度名称在广播中始终完整且不可截断」，再依赖 C；否则可仅实施 A+ 并文档化截断规则

## 实现设计

### 1. 抽取统一刷新函数

在 `drivers/communication/bluetooth/ch584/hidkbd.c` 中新增内部 helper，用于：

- 从 `storage_get_config_ptr()->ble_name_*` 读取名称
- 按 **A+ 布局** 重建 `advertData` 与 `scanRspData`（含 UUID 位置）
- 计算在 **仅 Legacy** 下是否仍需截断名称；若项目启用后备 **C**，再判断是否需要 Extended
- 处理 `$` 占位符替换逻辑
- 同步 `GGS_DEVICE_NAME_ATT`

建议拆成两个 helper：

- `hidEmu_refresh_device_name_cache()`
  - 生成动态名称缓冲区（含 `$` 替换）
  - 更新 `GGS_DEVICE_NAME_ATT`
- `hidEmu_apply_advertising_payload()`
  - 生成最终 `advertData` / `scanRspData`
  - 若启用方案 C：设置 `GAPROLE_ADV_EVENT_TYPE` 并在 `Legacy ↔ Extended` 间切换
  - 调用 `GAPRole_SetParameter()`；必要时 `GAP_UpdateAdvertisingData()`（**传入完整扫描响应长度**，与整包缓冲区一致）

### 2. `GAPROLE_ADV_EVENT_TYPE` 取值（重要）

当前工程**未显式设置** `GAPROLE_ADV_EVENT_TYPE`，栈行为以 **SDK 默认值** 为准。`CH58xBLE_LIB.h` 写明默认类型为 **`GAP_ADTYPE_ADV_IND`（可连接非定向）**，与 HID 外设常见需求一致。

- **恢复 Legacy、可连接广播时，应使用 `GAP_ADTYPE_ADV_IND`**，与「当前默认行为」对齐，**不要**误用 `GAP_ADTYPE_ADV_SCAN_IND`（可扫描不可连接类事件，易导致无法被正常连接）。
- 实施前建议在现固件上通过 `GAPRole_GetParameter(GAPROLE_ADV_EVENT_TYPE, ...)` 读一次基线，与文档、例程三方核对。

启用方案 C 时，长名称场景示例：

```c
uint8_t adv_event_type = GAP_ADTYPE_EXT_SCAN_UNDIRECT;
GAPRole_SetParameter(GAPROLE_ADV_EVENT_TYPE, sizeof(uint8_t), &adv_event_type);
```

恢复 Legacy（A+）时示例：

```c
uint8_t adv_event_type = GAP_ADTYPE_ADV_IND;
GAPRole_SetParameter(GAPROLE_ADV_EVENT_TYPE, sizeof(uint8_t), &adv_event_type);
```

### 3. 广播参数切换与栈行为

- 切换 `GAPROLE_ADV_EVENT_TYPE` 或更新整包数据时，若出现参数不生效或状态异常，应按沁恒文档/示例尝试：**先关闭广播** → 设置参数 → **再开启广播**；具体顺序以官方 HID 示例与实测为准。
- `GAP_UpdateAdvertisingData(taskId, adType, dataLen, pData)` 的 `dataLen` 必须与本次更新的有效数据长度一致；合并 helper 后，对扫描响应的更新应对应**完整重建后的 `scanRspData` 长度**（或栈要求的格式），避免仅覆盖名称前缀导致后续 AD 结构错乱。

### 4. 初始化路径统一使用 helper

在 `HidEmu_Init()` 设置 GAP 参数前，先调用 helper 刷新 `advertData` / `scanRspData`，再执行：

- `GAPRole_SetParameter(GAPROLE_ADVERT_DATA, ...)`
- `GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, ...)`
- `GGS_SetParameter(GGS_DEVICE_NAME_ATT, ...)`

### 5. 广播开启路径统一刷新

在 `hidEmu_adv_enable()` 中，当准备开启广播时先调用同一 helper，保证：

- 开机首次广播使用 `storage` 中的名称
- 通道切换、配对状态变化后，广播与 GATT 名称与 `storage` 一致
- 若启用 C：在名称长短变化时同步切换 `GAPROLE_ADV_EVENT_TYPE`

### 6. 对外更新接口

调整 `hidEmu_update_device_name()`，改为复用同一 helper，避免多处逻辑重复。

## 影响范围

- `application/system/system_init.c`  
  - 已写入 `BLE_PAIRING_NAME` 到 `storage`，通常无需改动
- `drivers/communication/bluetooth/ch584/hidkbd.c`  
  - `advertData` 与 `scanRspData` 的构造与同步  
  - 初始化路径、`hidEmu_adv_enable()`、`hidEmu_update_device_name()`  
  - 可选：`GAPROLE_ADV_EVENT_TYPE` 的 Legacy / Extended 切换

## 验证计划

1. 编译通过，`hidkbd.c` 无语法错误  
2. 开机首次广播名称非硬编码，来自 `storage` 的 `ble_name_data`  
3. GATT Device Name 与 `storage` 一致  
4. **A+**：抓包或日志确认 UUID 已出现在 `advertData`，`scanRspData` 含名称 + 连接间隔 + Tx Power（或实际选定字段）  
5. **A+**：典型产品名（如 ≤20 字节）在 **未启用 Extended** 时扫描列表中完整显示  
6. 若启用 **C**：名称超过 A+ Legacy 预算时切换 Extended，完整名称与附加字段仍正确；名称缩短回预算内可恢复 Legacy + `GAP_ADTYPE_ADV_IND`  
7. 设备可被主机正常连接、配对（重点回归 **未误用 `ADV_SCAN_IND` 作为默认可连接恢复类型**）

## 待确认项

- 是否必须实施 **方案 C**（后备），或仅 **A+** 即可满足产品名长度与兼容性  
- AD 重排后，是否接受与旧固件抓包在「UUID 所在 PDU」上的差异  
- 切换 Extended 时是否需额外配置 PHY（参考 `GAPROLE_PHY_*` 与 SDK 说明）  
- 目标市场最低 Android / iOS 版本（影响 Extended 可见性预期）

## 风险提示

- Extended 广播的可见性取决于扫描端是否支持 BLE 5 Extended Advertising  
- 仅解析 Legacy 的主机或工具，在 **C** 场景下可能仍只看到截断名或行为与 BLE 5 主机不一致  
- **`GAPROLE_ADV_EVENT_TYPE` 设错**（例如将可连接外设恢复为 `GAP_ADTYPE_ADV_SCAN_IND`）可能导致无法连接，实施时需以 **`GAP_ADTYPE_ADV_IND` + 实测** 为准
