# BLE 可追溯 Static Address 简化设计

日期: 2026-03-25

## 1. 目标

- 广播地址不再使用纯随机方式生成。
- 广播地址统一由 `base_mac + BLE 通道 + 通道版本号` 推导。
- 配对和回连都调用同一个地址更新方法。
- 切换到不同 BLE 通道回连时，能够计算出该通道上次使用的地址。

## 2. 设计规则

1. `base_mac` 在 `GAPROLE_STARTED` 后缓存。
2. 为每个 BLE 通道新增一个持久化版本号 `ble_addr_ver[3]`。
3. 回连时不修改版本号，只按当前版本号计算地址。
4. 配对时：
   - 若该通道历史上未连接过，则使用当前版本号地址。
   - 若该通道历史上已连接过，则先将该通道版本号加 1，再计算新地址。
5. 地址计算公式：
   - `slot = ble_idx - BLE_INDEX_1`
   - `offset = slot + ble_addr_ver[slot] * 3`
   - 将 `offset` 加到 `base_mac` 的 16-bit 可变区（字节 3/4）
6. 输出地址强制整理为 static random address 格式。

## 3. 影响范围

- `drivers/communication/bluetooth/ch584/hidkbd.c`
- `drivers/storage/storage.h`

## 4. 本轮不做

- 不改 `on_ble_connected`。
- 不新增 PC 侧测试代码验证；后续由用户烧录实机验证。
