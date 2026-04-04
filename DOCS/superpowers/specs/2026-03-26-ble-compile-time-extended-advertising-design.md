# BLE 广播长名称问题修复设计

## 1. 背景

当前项目在 `drivers/communication/bluetooth/ch584/hidkbd.c` 中构造 BLE 广播数据时，
将 `BLE_PAIRING_NAME` 直接写入 `scanRspData[31]`。
当产品名较长时，扫描响应总长度会超过 Legacy Advertising 的 31 字节限制，导致广播异常。

当前产品配置中：

- `BLE_PAIRING_NAME = "inateck NeoMagic Pro"`
- 名称长度为 20 字节

现有扫描响应布局同时携带以下字段：

- Slave Connection Interval Range
- HID / Battery 16-bit UUID
- Tx Power
- Complete Local Name

在该布局下，完整名称超出普通广播长度预算，问题会在初始化阶段直接出现。

## 2. 现状分析

### 2.1 当前名字来源

当前广播名和 GATT Device Name 的实际来源仍是编译期宏 `BLE_PAIRING_NAME`：

- `drivers/communication/bluetooth/ch584/hidkbd.c`
- `keyboards/kb904/config_product.h`

虽然 `storage` 也会在初始化时镜像保存一份名称，但当前工程没有生效的运行时改名入口。
`drivers/ota/Inateck_tag_config.c` 中的 BLE 名称写入逻辑仍处于注释状态。

因此，当前项目适合在编译期固定广播路径，而不是在运行时根据名称长度动态切换。

### 2.2 当前问题的直接原因

当前 `scanRspData` 的固定字段占用如下：

- Connection Interval AD 结构：6 字节
- 16-bit UUID AD 结构：6 字节
- Tx Power AD 结构：3 字节
- Name AD 结构头：2 字节

固定开销合计为 17 字节，剩余名称预算仅为 14 字节。

当 `BLE_PAIRING_NAME` 长度为 20 时：

- 总长度 = 17 + 20 = 37
- 超出 Legacy Advertising / Scan Response 上限 31

### 2.3 当前协议栈能力

CH58x BLE SDK 已提供：

- `GAPROLE_ADV_EVENT_TYPE`
- `GAP_ADTYPE_ADV_IND`
- `GAP_ADTYPE_EXT_CONN_UNDIRECT`
- `GAPROLE_ADVERT_DATA`
- `GAPROLE_SCAN_RSP_DATA`
- `B_MAX_ADV_EXT_LEN`

说明协议栈支持扩展广播能力，且可通过 GAPRole 参数切换广播事件类型。

## 3. 需求目标

本次设计的目标如下：

1. 修复长名称导致的广播异常。
2. 保持键盘设备可连接，不允许只可扫描不可连接。
3. 不在运行时动态切换 Legacy / Extended 广播路径。
4. 在编译期就确定当前固件应使用的广播类型。
5. 尽量保持现有配对 / 回连状态机不变。

## 4. 方案结论

采用“编译期兼容优先”方案：

1. 优先保留 Legacy 广播路径。
2. 通过调整 AD 布局，提升 Legacy 下可容纳的名称长度。
3. 当 `BLE_PAIRING_NAME` 仍超过 Legacy 可容纳上限时，在编译期固定切换到 Extended Advertising。
4. Extended 路径固定使用 `GAP_ADTYPE_EXT_CONN_UNDIRECT`，保证设备仍可连接。
5. 运行时只负责重启同一类型的广播，不做 Legacy / Extended 的动态切换。

## 5. 详细设计

### 5.1 广播模式宏

在产品配置层新增广播模式配置，建议提供以下宏：

- `BLE_ADV_MODE_AUTO`
- `BLE_ADV_MODE_LEGACY`
- `BLE_ADV_MODE_EXTENDED`

默认使用 `BLE_ADV_MODE_AUTO`。

`AUTO` 模式下，编译期根据 `sizeof(BLE_PAIRING_NAME) - 1` 与 Legacy 名称预算比较，
自动决定当前固件使用 Legacy 或 Extended 路径。

### 5.2 Legacy 布局重排

为尽量减少进入 Extended 的概率，Legacy 布局先做一次重排。

建议布局如下：

`advertData`

- Flags
- Appearance
- Manufacturer Specific Data
- HID / Battery 16-bit UUID

`scanRspData`

- Slave Connection Interval Range
- Tx Power
- Complete Local Name

重排后：

- `advertData` 总长度约为 17 字节
- `scanRspData` 固定开销为 11 字节
- Legacy 下完整名称预算提升为 20 字节

即：

- 名称长度 `<= 20`：可继续使用 Legacy
- 名称长度 `> 20`：需改为 Extended

### 5.3 编译期决策规则

编译期决策规则如下：

1. 如果显式配置为 `BLE_ADV_MODE_LEGACY`
   - 固定使用 `GAP_ADTYPE_ADV_IND`
   - 若名称长度超过 20，直接编译失败

2. 如果显式配置为 `BLE_ADV_MODE_EXTENDED`
   - 固定使用 `GAP_ADTYPE_EXT_CONN_UNDIRECT`

3. 如果配置为 `BLE_ADV_MODE_AUTO`
   - 名称长度 `<= 20`：使用 `GAP_ADTYPE_ADV_IND`
   - 名称长度 `> 20`：使用 `GAP_ADTYPE_EXT_CONN_UNDIRECT`

### 5.4 运行时行为约束

本设计明确禁止在运行时切换广播路径。

运行时允许做的事情只有：

- 停止当前广播
- 更新随机静态地址
- 更新 discoverability 标记
- 重新下发当前固件对应的固定广播数据
- 重新开启广播

运行时不允许做的事情：

- 根据实际名称长度临时改用 Legacy 或 Extended
- 在续播、断链重播、超时重播时切换广播事件类型

这样可以避免广播路径在状态切换中出现不可预期异常。

### 5.5 关键实现边界

#### 5.5.1 广播事件类型

Legacy 路径：

- 使用 `GAP_ADTYPE_ADV_IND`

Extended 路径：

- 使用 `GAP_ADTYPE_EXT_CONN_UNDIRECT`

不得使用：

- `GAP_ADTYPE_EXT_SCAN_UNDIRECT`

原因是键盘设备必须保持可连接。
若误用仅可扫描类型，会导致“能发现但不能连接”的错误行为。

#### 5.5.2 统一载荷提交点

建议在 `hidkbd.c` 中引入统一的广播载荷提交逻辑，
但其职责仅限于“提交当前编译期确定的固定路径”。

推荐思路：

- `hidEmu_build_adv_payload()`
  - 按当前编译产物生成固定 `advertData` / `scanRspData`

- `hidEmu_apply_adv_payload()`
  - 下发 `GAPROLE_ADV_EVENT_TYPE`
  - 下发 `GAPROLE_ADVERT_DATA`
  - 下发 `GAPROLE_SCAN_RSP_DATA`

#### 5.5.3 与现有开播流程的关系

保留当前主流程入口不变：

- `HidEmu_Init()`
- `hidEmu_start_advertising_for_purpose()`
- `GAPROLE_WAITING` 中的续播路径

改动原则：

- 不改变配对 / 回连状态机语义
- 只在这些路径中复用同一套固定广播配置

## 6. 编译期保护

建议增加以下编译期保护：

1. Legacy 强制模式超限保护

   - 若 `BLE_ADV_MODE_LEGACY`
   - 且 `sizeof(BLE_PAIRING_NAME) - 1 > 20`
   - 则直接 `#error`

2. 名称镜像与存储一致性保护

   当前 `storage` 中：

   - `ble_name_data[22]`

   但 `storage.c` 目前按 `sizeof(ble_name_data) - 1` 做裁剪。
   这意味着如果未来产品名超过当前安全长度，可能出现：

   - 广播名完整
   - storage 镜像被截断
   - GATT 名与镜像名称不一致

   因此应增加编译期保护，或同步调整 storage 名称容量与裁剪策略。

## 7. 影响范围

预计主要影响以下位置：

- `keyboards/defaults/ble_defaults.h`
- `keyboards/kb904/config_product.h`
- `drivers/communication/bluetooth/ch584/hidkbd.c`
- `drivers/communication/bluetooth/ch584/hidkbd.h`
- `drivers/storage/storage.c`（如需统一名称容量边界）

## 8. 验证方案

### 8.1 编译期验证

验证以下构建场景：

1. 名称长度 `<= 20`，`AUTO`
   - 应生成 Legacy 固件

2. 名称长度 `> 20`，`AUTO`
   - 应生成 Extended 固件

3. 名称长度 `> 20`，强制 `LEGACY`
   - 应编译失败

### 8.2 抓包验证

Legacy 固件验证：

- 广播类型为 `ADV_IND`
- 设备可被扫描并连接
- 名称完整或符合 Legacy 预算

Extended 固件验证：

- 广播类型为 `EXT_CONN_UNDIRECT`
- 设备可被扫描并连接
- 扩展广播数据中名称完整

### 8.3 行为回归

验证以下场景：

- 开机首次广播
- 配对广播
- 回连广播
- 断链后续播
- 广播超时后自动续播

要求：

- 不出现广播异常
- 不出现“可发现但不可连接”
- 不出现续播时广播类型被意外切换

## 9. 风险与说明

1. Extended Advertising 的显示效果取决于扫描端是否支持 BLE 5 扩展广播。
   该风险影响“是否完整显示名称”，但不应影响本机正常发出广播。

2. 若后续项目重新引入运行时改名能力，则本设计不再成立。
   届时需要重新评估：
   - 广播路径是否允许动态切换
   - storage 与广播名的同步策略

3. 若后续产品名继续增长，storage 名称容量也需要同步评估。

## 10. 推荐实施顺序

1. 先完成 Legacy 布局重排
2. 加入编译期广播模式宏与保护
3. 固化 Legacy / Extended 事件类型选择
4. 统一所有开播路径的载荷下发逻辑
5. 完成抓包与连接回归验证

## 11. 结论

本次修复不采用运行时动态切换，而是采用编译期固定广播路径。

最终结论是：

- 名称较短时，固件固定使用 Legacy Advertising
- 名称较长时，固件固定使用 Extended Connectable Advertising
- 运行时只重启同一种广播，不在状态切换中改变广播类型

这样可以在修复长名称广播异常的同时，最大限度降低运行时切换带来的不确定性。
