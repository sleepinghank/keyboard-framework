# BLE iPad 连接失败调试设计

## 1. 背景

当前固件上电后会直接切到 BLE 并进入 `pairing_ex` 路径。实测在 iPad 上点击连接失败，现阶段需要先补充最小观测点，确认失败发生在：

1. 连接建立前后的 GAP 断链阶段
2. SMP 配对/绑定阶段
3. Passkey / IO capability 协商阶段

本次不修改任何连接逻辑，只添加日志。

## 2. 调试目标

本轮测试需要直接从串口日志判断以下问题：

1. `hidDevPairStateCB()` 是否被调用
2. 配对状态 `state` 和结果 `status` 是什么
3. 是否进入 `hidDevPasscodeCB()`，以及主机请求的 UI 能力是什么
4. 连接失败时链路终止原因 `reason` 是什么

## 3. 打点方案

### 3.1 配对状态回调

文件：`project/ch584m/Profile/hiddev.c`

在 `hidDevPairStateCB()` 中新增日志，打印：

- `connHandle`
- `state`
- `status`
- 常见状态名与失败码名

目的：

- 若出现 `status = 0x03`，优先怀疑认证要求 / IO capability 不匹配
- 若出现 `status = 0x09`，优先怀疑 repeated attempts
- 若 `PAIRING_STATE_COMPLETE/BONDED/BOND_SAVED` 全未出现，说明失败可能发生在更早阶段

### 3.2 Passcode 请求回调

文件：`project/ch584m/Profile/hiddev.c`

在 `hidDevPasscodeCB()` 中新增日志，打印：

- `connectionHandle`
- `uiInputs`
- `uiOutputs`

目的：

- 判断 iPad 是否要求进入 passkey 流程
- 判断当前配置的 IO capability 是否与主机协商路径冲突

### 3.3 链路终止日志增强

文件：`drivers/communication/bluetooth/ch584/hidkbd.c`

在 `GAP_LINK_TERMINATED_EVENT` 分支补充更明确的调试日志，打印：

- `reason`
- 当前 `pairing_state`
- 当前 `ble_idx`

目的：

- 与 `hidDevPairStateCB()` 的日志进行前后对照
- 判断是“刚连上就断”还是“配对失败后断”

## 4. 非目标

本次不做以下修改：

1. 不调整 `MITM / IO_CAP / BONDING` 参数
2. 不调整上电自动进入 pairing 的启动路径
3. 不修改 bond 同步逻辑
4. 不改 HID / GATT 属性权限

## 5. 预期输出

测试完成后，重点关注以下日志模式：

1. `[PAIR] state=... status=...`
2. `[PAIR] passcode needed ...`
3. `[PAIR] terminate reason=...`

若拿到上述三类日志，即可进入下一轮根因收敛和定向修复。
