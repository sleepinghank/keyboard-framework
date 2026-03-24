# BLE 蓝牙稳定性修复需求文档

**日期：** 2026-03-24
**状态：** 需求确认完成，待方案设计

---

## 1. 问题陈述

当前项目蓝牙功能存在两个严重可用性问题：

1. **配对模式连不上**：首次配对或重新配对时，设备无法与主机建立连接
2. **重新开关机无回连**：已配对设备断电重启后，无法自动发起回连广播

两个问题同等优先级，均需修复。

---

## 2. 需求范围

### 目标范围

- `drivers/communication/bluetooth/ch584/_bt_driver.c`：核心驱动逻辑修复
- `drivers/communication/bluetooth/ch584/hidkbd.c`：广播控制、地址管理修复
- `middleware/communication/wireless_callbacks.c`：`access_ble_notify_disconnected` 信号转发时序修正（附带必要修改，约 10-20 行）

### 不在范围内

- 无线状态机（`wireless.c`、`transport.c`）重构
- `communication_service.c` 事件流重设计
- HAL 层或 OSAL 层修改

### 多主机支持

需支持最多 3 个主机（BLE Index 1/2/3），各主机独立配对/回连。

---

## 3. 参考基准

**Demo 工程：** `DOCS/code_demo/pcl004-ch59x.xml`

Demo 工程蓝牙功能稳定，关键设计特点：
- 广播分级：`hidDevHighAdvertising()`（配对用，短时限）→ `hidDevLowAdvertising()`（保活）
- 断开后停广播，等上层决策，**不自动回连**
- 无 `pairing_state` 中间标志，直接按场景调用对应广播接口
- 不使用 MAC 地址偏移

---

## 4. 根因分析（基于代码探索）

### 根因 A：地址漂移导致回连失败

**位置：** `hidkbd.c`，`hidEmu_adv_enable()` 中的地址偏移逻辑

**问题：**
```c
// 每次调用都对 ownAddr 做偏移
GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddr);
ownAddr[3] += ble_idx;  // 叠加偏移，非幂等操作
```

`ownAddr` 读取的是"当前已生效地址"而非 base 地址。若调用多次，地址持续漂移，主机端保存的 IRK/白名单与设备实际地址不匹配，导致白名单过滤广播永远收不到回连请求。

**预期修复：** 保存固定 base MAC（从 flash/OTP 读一次），每次用 `base_mac[3] = base + idx` 计算，确保地址幂等、可预测。

### 根因 B：`pairing_state` 语义二义性

**位置：** `_bt_driver.c`、`hidkbd.c`、`wireless_callbacks.c`

**问题：** `pairing_state` 单一标志同时控制两个独立职责：
1. 广播过滤策略（`GAP_FILTER_POLICY_WHITE` vs 全开）
2. MAC 地址是否偏移切换

两个职责的切换时机不同，共用一个字段导致：配对场景下地址偏移/不偏移的逻辑分支不可预测；`hidEmuStateCB` 在 `GAP_LINK_TERMINATED_EVENT` 后的广播路由依赖该字段，但字段含义上下文相关。

**预期修复：** 拆分为两个独立字段：
- `adv_filter_open`：TRUE = 全开广播（配对场景），FALSE = 白名单过滤（回连场景）
- `mac_need_change`：TRUE = 本次广播需切换 MAC（重配场景）

### 根因 C：配对广播间接触发链路过长

**位置：** `_bt_driver.c` `bt_driver_pairing_ex()` → `hidEmu_disconnect()` → `GAP_LINK_TERMINATED_EVENT` → `access_ble_notify_disconnected()`（`wireless_callbacks.c`）→ 广播启动

**问题：** `pairing_ex` 不直接启动配对广播，而是通过 disconnect→回调间接触发。若已处于未连接状态（GAPROLE_WAIT_FOR_NEWCONNECTION），中间链路不会触发，配对广播无法启动。

**预期修复：** `pairing_ex` 应直接判断当前状态并启动广播，不依赖中间回调链路。

### 根因 D：`wireless_callbacks.c` 对 disconnect 事件的处理

**问题：** 当前 `WL_DISCONNECTED_EVT` 统一自动投递 `WL_RECONNECT_EVT`，但配对失败场景下（未绑定）不应自动回连，应等待新的 `pairing_ex` 命令。此逻辑需结合 `adv_filter_open` 字段判断。

---

## 5. 验证场景

| 场景 | 描述 | 预期结果 |
|------|------|---------|
| A | 首次开机无绑定，调用 `pairing_ex(INDEX_1)` | 进入全开广播（无白名单），30s 超时后停播 |
| B | 已绑定主机1，切换主机2配对 | 切换 MAC 偏移，进入全开广播，旧主机无法用旧密钥连接 |
| C | 已绑定，关机重启，调用 `connect_ex(INDEX_1)` | 发起白名单过滤回连广播，主机可连接 |
| D | 连接中主机关蓝牙断开，设备停止广播 | 停播不自动重连，等上层决策 |
| E | 配对广播超时无连接 | 停播并通过回调上报超时 |
| F | 配对广播中调用 `pairing_ex(INDEX_2)` | 停当前广播，切换 idx，重新发起新配对广播 |
| G | 对已绑定主机重新配对（`pairing_ex`，bonded=TRUE） | 切换 MAC，全开广播，旧绑定记录清除 |
| H | `connect_ex` 调用时设备正在广播中 | 停当前广播，重新发起白名单广播 |
| I | 3 个 idx 循环切换（1→2→3→1） | 各主机地址独立，SNV 无冲突 |

---

## 6. 评审意见摘要

### 用户体验评审
- 主机切换速度感知（用户等待感知）需关注
- iOS/Android/Windows 跨平台配对行为差异需兼顾
- 3 个 idx 用满后尝试配对第 4 台的行为需定义（指示灯/无响应）

### 架构评审
- "仅驱动层"不能完全解决配对问题，必须联动修正 `wireless_callbacks.c`（约 10-20 行）
- 地址管理改为 base MAC 方案是高优先级修复
- `pairing_state` 拆分可在驱动层内完成

### 测试评审
- 场景 F/G/H/I 是补充的高风险边界场景
- `wireless_send_keyboard` 按键时意外触发 `wireless_connect()` 是高风险副作用，需在场景 D 中覆盖"断开后按键"子场景
- 3 个 idx 的 SNV 地址步长（`(ble_idx - BLE_INDEX_1) * 0x100`）需验证分区足够

---

## 7. 现状分析（步骤 3 完成）

### 7.1 根因 A：地址漂移（精确定位）

**文件：** `hidkbd.c` L1027-L1051，`hidEmu_adv_enable()` 中

```c
GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddr);  // 读取运行时地址（非出厂地址）
ownAddr[4] += access_state.ble_idx;              // 每次调用无条件叠加

if (access_state.pairing_state) {
    if (!hidEmu_is_ble_mac_change(access_state.ble_idx))
        ownAddr[3] += access_state.ble_idx;       // 配对态 + mac_flag 未置位时再叠加
} else {
    if (hidEmu_is_ble_mac_change(access_state.ble_idx))
        ownAddr[3] += access_state.ble_idx;       // 回连态 + mac_flag 已置位时再叠加
}
GAP_ConfigDeviceAddr(ADDRTYPE_STATIC, ownAddr);  // 写入后成为新基准
```

每次调用 `hidEmu_adv_enable(ENABLE)` 都在上次已修改的地址上继续叠加 `ble_idx`。开关机重连时再调一次，地址漂移，主机白名单中的地址与设备实际地址不匹配，回连失败。

`hidEmu_is_ble_mac_change` 通过 `cfg->ble_mac_flag ^= bond_flag`（XOR 翻转）标记配对/回连态，设计意图是让两种状态地址互斥，但由于基准地址不固定，多次调用后地址完全不可预测。

### 7.2 根因 B：`pairing_state` 语义二义性

`access_state.pairing_state` 在不同位置有不同语义：

| 写入点 | 值 | 语义 |
|--------|-----|------|
| `bt_driver_connect_ex` L97 | FALSE | "不需要换地址" |
| `bt_driver_pairing_ex` L170 | bonded ? TRUE : FALSE | "是否重配（已有绑定）" |
| `hidEmu_adv_enable` L1027+ | 读取 | 控制地址偏移方向 |
| `hidEmu_adv_enable` L1105 | 读取 | 控制是否调度深睡眠定时器 |
| `hidEmuStateCB` L1459/1466 | 读取 | 广播超时后是否保持唤醒 |

单一字段承担"重配标志"、"广播过滤策略"、"地址偏移方向"、"睡眠豁免"四个职责，任何一处的写入时序错误都会影响其他三处逻辑。

### 7.3 根因 C：`bt_driver_pairing_ex` 广播启动路径

**文件：** `_bt_driver.c` L185-L191

```c
if (ble_state == GAPROLE_ADVERTISING) {
    hidEmu_adv_enable(DISABLE);   // 停广播
    // 没有直接调用 ENABLE，等异步回调
}
else {
    hidEmu_adv_enable(ENABLE);    // 空闲态才直接开广播
}
```

停广播后，需等 `GAP_END_DISCOVERABLE_DONE_EVENT` 回调 → 判断 `con_work_mode == access_state.ble_idx` → 再调 `hidEmu_adv_enable(ENABLE)`，期间再次叠加地址偏移（触发根因 A）。

`bt_driver_connect_ex` 的 `GAPROLE_CONNECTED` 分支同样存在问题：
```c
// _bt_driver.c L127
hidEmu_disconnect();  // 断开后无续接广播逻辑
// GAP_LINK_TERMINATED_EVENT 回调里只调 notify_disconnected，不重开广播
```

### 7.4 根因 D：`wireless_callbacks.c` 断开后决策时序

**文件：** `wireless_callbacks.c` L182-L193

```c
// access_ble_notify_disconnected 在底层驱动回调中提前决策
bool discoverable = access_ble_is_discoverable_session();  // 读取 pairing_state
event.evt_type = discoverable ? EVT_DISCOVERABLE : EVT_RECONNECTING;
OSAL_SetEvent(commu_taskID, WL_DISCONNECTED_EVT);
```

问题：配对失败场景下，`access_state.pairing_state` 清零的时机不确定。若断开事件先于 `pairing_state` 清零到达，`access_ble_is_discoverable_session()` 返回 true，走进 `EVT_DISCOVERABLE` 分支（此时恰好正确）；但若时序反转，则误判为 `EVT_RECONNECTING`，触发不该有的回连广播。整个逻辑依赖时序，而非显式状态机保证。

### 7.5 Demo 工程核心差异

**文件：** `DOCS/code_demo/pcl004-ch59x.xml`

Demo 用独立 `pairingStatus` 变量（由 `HidDev_PairStateCB` 在 `GAPBOND_PAIRING_STATE_COMPLETE` 时赋值），配对失败与断开两条路径完全隔离：

```c
// GAP_LINK_TERMINATED_EVENT 处理
if (pairingStatus == SMP_PAIRING_FAILED_CONFIRM_VALUE) {
    hidDevInitialAdvertising();  // 配对失败路径：单独处理
    pairingStatus = SUCCESS;     // 立即清零，防止污染断开路径
}
// TERMINATED_EVENT 处理（仅处理正常断开）
if (sleepModeFlag == 3 && nvs_flash_info.find_flag == 1) {
    restartDevLowAdvertising();  // 低占空比回连
} else {
    hidDevInitialAdvertising();  // 高占空比重连
}
```

广播分级：High（配对，短时限）/ Low（回连，低频）/ Initial（正常断开）三档，调用时机明确，不依赖标志位时序。

### 7.6 模块修改需求对照表

| 模块 | 现状问题 | 需修改 |
|------|---------|--------|
| `hidkbd.c` `hidEmu_adv_enable` | ownAddr 从运行时读取并叠加偏移 | 从出厂 base MAC 计算地址 |
| `hidkbd.c` `hidEmuStateCB` | GAP_LINK_TERMINATED 无路径区分 | 配对失败与正常断开分路处理 |
| `_bt_driver.c` `bt_driver_pairing_ex` | 停广播后依赖异步回调续接 | 直接判断状态启动广播 |
| `_bt_driver.c` `bt_driver_connect_ex` | 断开/停广播后无续接广播 | 补充续接逻辑 |
| `_bt_driver.c` `access_state_t` | pairing_state 四职一身 | 拆分为 adv_filter_open + mac_need_change |
| `wireless_callbacks.c` | 断开后在回调里提前决策 | 改为接收驱动层明确的事件类型 |
