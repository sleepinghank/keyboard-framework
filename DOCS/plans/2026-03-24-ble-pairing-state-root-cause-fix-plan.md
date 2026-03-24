# BLE 首次配对失败根因与修复计划

日期: 2026-03-24

## 1. 问题现象

当前固件在以下场景下连接失败：

- 设备未绑定
- 上电后直接进入蓝牙可发现广播
- iPad 首次点击连接

日志特征：

- 已进入广播
- 已建立 ACL 连接
- 在进入 `PAIR STARTED` 之前断开
- 断开原因为 `reason=0x08`

## 2. 根因结论

根因不是 BLE 栈初始化失败，也不是当前 SMP/Bond 参数本身一定与 iPad 不兼容。

根因是：

`pairing_state` 在当前实现中被错误地当成了“进入配对广播”的总开关使用，但在 demo/原始设计里，它实际表示的是“已绑定设备重新配对时，是否切换广播身份地址”。

因此：

- 首次未绑定配对，本不该置 `pairing_state = TRUE`
- 当前代码却无条件置位
- 导致首次未绑定配对错误地走进“重配换地址”路径
- 广播身份地址被切换
- iPad 连到错误身份后，在进入 SMP 前即超时断开

## 3. 证据链

### 3.1 当前实现无条件置位 `pairing_state`

当前 `bt_driver_pairing_ex()` 中：

- 设置 `access_state.ble_idx = host_idx`
- 无条件设置 `access_state.pairing_state = TRUE`

位置：

- `drivers/communication/bluetooth/ch584/_bt_driver.c`

### 3.2 广播地址切换依赖 `pairing_state`

当前 `hidEmu_adv_enable()` 中，广播地址构造明确依赖：

- `access_state.pairing_state`
- `ble_mac_flag`

当 `pairing_state = TRUE` 时，会进入“重配地址”路径。

位置：

- `drivers/communication/bluetooth/ch584/hidkbd.c`

### 3.3 demo 并不是首次配对就置位 `pairing_state`

demo 的 `access_pairing_mode()` 逻辑是：

- 若当前已连接，准备断开后重配，则置 `pairing_state = TRUE`
- 若当前槽位已绑定，准备重配，则置 `pairing_state = TRUE`
- 若当前槽位未绑定，首次新配对，不置位

也就是说，demo 把：

- “首次未绑定配对”
- “已绑定后重配”

明确区分成了两条路径。

参考：

- `docs/code_demo/KB04122-13A-WCH_code.xml`

### 3.4 当前日志与该结论完全一致

本次失败日志中：

- `bond_flag=0`
- `pairing=1`

说明当前槽位明确是“未绑定”，却仍然被拉进了 `pairing_state=TRUE` 路径。

同时广播地址已经发生切换，随后：

- `link established`
- 未进入 `PAIR STARTED`
- `GAP_LINK_TERMINATED_EVENT reason=8`

这与“首次未绑定配对误走重配地址路径”完全吻合。

## 4. 断开原因解释

这次日志中的 `reason=0x08` 是链路断开原因，不是 SMP 的 `PAIRING_FAILED_UNSPECIFIED`。

按 Bluetooth Core Controller Error Codes：

- `0x08 = Connection Timeout`

也就是链路监督超时。

这说明：

- 链路已经建立
- 但没有顺利进入后续正常的安全/业务阶段
- 最终连接被超时释放

这更符合“连接到了错误身份地址或错误连接语义”的症状，而不是“已经进入配对流程但口令失败”。

参考：

- [Bluetooth Core Controller Error Codes](https://www.bluetooth.com/wp-content/uploads/Files/Specification/HTML/Core-54/out/en/architecture%2C-mixing%2C-and-conventions/controller-error-codes.html)

## 5. 修复目标

把以下三种蓝牙场景彻底分离：

1. 未绑定首次新配对
2. 已绑定上电回连
3. 已绑定后主动重配

确保每种场景使用各自正确的：

- 广播状态
- 地址策略
- 断开后续动作

## 6. 完整修复方案

### 6.1 修正 `pairing_state` 语义

在 `bt_driver_pairing_ex()` 中，不再无条件把 `pairing_state` 置为 `TRUE`。

新规则：

- 未绑定首次配对：
  - `pairing_state = FALSE`
- 已绑定后主动重配：
  - `pairing_state = TRUE`

含义统一为：

`pairing_state` 仅表示“是否进入已绑定设备的重配身份切换路径”，而不是“是否正在可发现广播”。

### 6.2 首次未绑定配对仍然进入 discoverable，但不换地址

未绑定首次配对需要：

- 进入可发现广播
- 允许主机连接和后续配对
- 但不得切到“重配换地址”路径

因此应保留：

- `wireless_pairing_ex()`
- discoverable 广播入口

但同时保证：

- `pairing_state = FALSE`
- 地址保持首次身份

### 6.3 已绑定后主动重配才允许切换地址

只有在以下场景中才允许：

- 当前槽位已有 bond
- 用户主动触发配对/重配

这时：

- `pairing_state = TRUE`
- 允许走现有地址切换逻辑

该逻辑继续用于避免旧主机缓存影响新配对。

### 6.4 修正 advertising 状态事件的判断条件

当前 `access_ble_notify_advertising()` 仅根据 `pairing_state` 决定：

- `WL_DISCOVERABLE_EVT`
- `WL_RECONNECTING_EVT`

这在修正语义后会失真。

应改为根据真实场景判断：

- 若当前是未绑定首次配对，或已绑定主动重配：
  - 投递 `WL_DISCOVERABLE_EVT`
- 若当前是已绑定普通回连：
  - 投递 `WL_RECONNECTING_EVT`

不能继续简单等同于：

- `pairing_state == TRUE` 才是 discoverable

### 6.5 修正断开后的自动动作

当前 `WL_DISCONNECTED_EVT` 后统一自动投递 `WL_RECONNECT_EVT`。

这对首次未绑定配对场景是错误的。

应改为按场景分流：

- 已绑定普通回连断开：
  - 自动 `WL_RECONNECT_EVT`
- 未绑定首次配对断开：
  - 继续或重新进入 discoverable
- 已绑定主动重配断开：
  - 继续 discoverable

### 6.6 保留当前默认启动分流

当前已完成的启动分流方向是正确的，应保留：

- 已绑定：
  - 默认 `connect_ex`
- 未绑定：
  - 默认 `pairing_ex`

问题不在“是否进入配对广播”，而在“首次未绑定配对时误置 `pairing_state`”。

## 7. 预计修改文件

核心修改：

- `drivers/communication/bluetooth/ch584/_bt_driver.c`
- `drivers/communication/bluetooth/ch584/hidkbd.c`
- `middleware/communication/wireless_callbacks.c`
- `application/service/communication_service.c`

不计划修改：

- `project/ch584m/Profile/hiddev.c`
- BondMgr 参数
- HID GATT 权限配置
- LPM 主状态机

## 8. 实施步骤

### 步骤 1

在 `_bt_driver.c` 中补充“当前槽位是否已绑定”的判断。

### 步骤 2

重写 `bt_driver_pairing_ex()` 的状态赋值逻辑：

- 未绑定首次配对：`pairing_state = FALSE`
- 已绑定重配：`pairing_state = TRUE`

### 步骤 3

在 `wireless_callbacks.c` 中，把 advertising 状态事件改为按“真实连接场景”判断，而不是只看 `pairing_state`。

### 步骤 4

在 `communication_service.c` 中调整 `WL_DISCONNECTED_EVT` 的后续策略：

- 未绑定首次配对失败后不再自动 reconnect
- 保持 discoverable

### 步骤 5

补充日志，至少打印：

- 当前配对请求类型：首次配对 / 已绑定重配 / 普通回连
- advertising 状态类型：discoverable / reconnecting
- 地址策略：是否切换地址
- disconnect 后续动作：reconnect / rediscoverable

## 9. 回归验证

### 9.1 未绑定首次新配对

预期：

- 进入 discoverable
- `pairing_state = FALSE`
- 地址不走重配切换逻辑
- 第一次点击连接即可进入 `PAIR STARTED -> COMPLETE -> BOND_SAVED`

### 9.2 已绑定上电回连

预期：

- 启动后走 `connect_ex`
- advertising 状态为 reconnecting
- 不进入 discoverable 指示

### 9.3 已绑定主动重配

预期：

- 进入 discoverable
- `pairing_state = TRUE`
- 地址切换逻辑生效

### 9.4 配对失败后的行为

预期：

- 未绑定首次配对失败：继续 discoverable
- 已绑定回连失败：允许 reconnect
- 已绑定重配失败：继续 discoverable

### 9.5 三模切换

预期：

- USB -> BT
- 2.4G -> BT
- BT -> USB / 2.4G

均不引入新的断链或错误状态。

## 10. 预期修复结果

修复完成后：

- 首次未绑定配对不再失败
- 已绑定回连逻辑保持正常
- 已绑定重配逻辑继续可用
- `pairing_state` 语义恢复与 demo 一致
