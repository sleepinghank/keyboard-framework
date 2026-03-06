# N0046 分支三：蓝牙连接需求

> 分支名: `feature/n0046-bluetooth-connection`
> 依赖: `feature/n0046-product-config`, `feature/n0046-keyboard-input`
> 优先级: P0 - 核心功能

---

## 1. 目标

实现 N0046 键盘的蓝牙连接功能，包括配对、回连、连接状态管理。

---

## 2. 蓝牙规格

| 项目 | 规格 |
|------|------|
| 蓝牙版本 | BLE 5.3 |
| 设备名称 | "inateck AceTouch" |
| 连接方式 | 单通道（仅连接 1 台设备） |
| 有效距离 | 10 米 |
| HID 模式 | HID over GATT |

---

## 3. 状态机定义

### 3.1 状态转换

```
┌─────────────┐
│   WT_RESET  │ 开机初始化
└──────┬──────┘
       │
       ▼
┌─────────────┐
│ WT_INITIALIZED │
└──────┬──────┘
       │
       ├─── 首次开机/无配对记录 ───► ┌─────────────┐
       │                            │ WT_PARING   │ 配对模式 (60s超时)
       │                            └──────┬──────┘
       │                                   │ 配对成功
       │                                   ▼
       ├─── 有配对记录 ───► ┌─────────────┐ ┌─────────────┐
       │                    │WT_RECONNECTING│◄──│ WT_CONNECTED │
       │                    │ 回连模式 (60s)│   │ 已连接       │
       │                    └──────┬──────┘   └──────┬──────┘
       │                           │ 回连成功         │ 断开
       │                           ▼                  │
       │                    ┌─────────────┐          │
       │                    │ WT_CONNECTED│◄─────────┘
       │                    └──────┬──────┘
       │                           │ 10分钟无操作
       │                           ▼
       │                    ┌─────────────┐
       └───────────────────►│ WT_SUSPEND  │ 深度休眠
                            └─────────────┘
```

### 3.2 状态指示灯

| 状态 | 白灯行为 | 超时 |
|------|----------|------|
| WT_PARING | 快闪 (3次/秒) | 60秒 → 休眠 |
| WT_RECONNECTING | 慢闪 (1次/秒) | 60秒 → 休眠 |
| WT_CONNECTED | 熄灭 | - |
| WT_DISCONNECTED | 闪烁 | 10分钟 → 深度休眠 |
| WT_SUSPEND | 熄灭 | - |

---

## 4. 配对功能

### 4.1 首次配对

- 首次开机或恢复出厂后自动进入配对模式
- 广播 60 秒，超时进入休眠
- 设备端点击配对后 3 秒内完成连接

### 4.2 手动配对

- **触发方式**: Fn + Del 长按 3 秒
- 进入配对模式后白灯快闪
- 配对完成后自动连接

### 4.3 配对取消处理

- 用户取消配对弹窗后重新进入配对状态
- 继续等待配对直到超时

---

## 5. 回连功能

### 5.1 开机回连

- 开机后自动回连上次连接的设备
- 无需按键触发
- 3 秒内完成回连

### 5.2 回连超时

- 回连失败后进入轮询模式（可接受新连接）
- 轮询 60 秒，超时进入休眠

### 5.3 休眠唤醒回连

- 按任意键唤醒后自动回连
- 触控板按压可唤醒（本次不实现触控板）

### 5.4 断连后轮询

- 设备关机/关蓝牙/超距后进入轮询模式
- 轮询 60 秒，超时进入休眠

---

## 6. 休眠与功耗

### 6.1 休眠触发

| 条件 | 休眠时间 |
|------|----------|
| 配对超时 | 60 秒 |
| 回连超时 | 60 秒 |
| 轮询超时 | 60 秒 |
| 连接状态断开 | 10 分钟 |

### 6.2 功耗要求

| 状态 | 电流 |
|------|------|
| 休眠 | < 40μA |
| 空闲 | ~ 1mA |
| 工作 | ≤ 10mA |

---

## 7. 需要实现的接口

### 7.1 状态回调函数

```c
// 蓝牙状态变化回调（weak 函数，在 n0046.c 中实现）
void wireless_enter_connected_kb(uint8_t host_idx);
void wireless_enter_disconnected_kb(uint8_t host_idx, uint8_t reason);
void wireless_enter_discoverable_kb(uint8_t host_idx);
void wireless_enter_reconnecting_kb(uint8_t host_idx);
```

### 7.2 配对触发

```c
// Fn+Del 长按检测
#define PAIRING_HOLD_TIME 3000  // 3秒

// 在 process_record_kb 中检测
if (keycode == KC_DEL && (get_mods() & MOD_BIT(KC_FN))) {
    if (record->event.pressed) {
        pairing_timer = timer_read32();
    } else if (timer_elapsed32(pairing_timer) >= PAIRING_HOLD_TIME) {
        wireless_pairing();  // 进入配对模式
    }
}
```

---

## 8. 复用模块

以下模块无需修改，直接复用：

| 模块 | 文件 | 功能 |
|------|------|------|
| BLE 驱动 | `drivers/communication/bluetooth/ch584/` | 底层蓝牙驱动 |
| 无线状态机 | `middleware/communication/wireless.c` | 状态管理 |
| 低功耗管理 | `middleware/communication/lpm.c` | 休眠管理 |
| HID 报告 | `middleware/communication/report_buffer.c` | 报告缓冲 |

---

## 9. 需要修改的文件

| 文件 | 修改内容 |
|------|----------|
| `keyboards/n0046/n0046.c` | 实现状态回调、配对触发 |

---

## 10. 验收标准

- [ ] 蓝牙名称正确显示 "inateck AceTouch"
- [ ] 首次开机自动进入配对模式
- [ ] Fn+Del 长按3秒进入配对模式
- [ ] 配对成功后自动连接
- [ ] 开机自动回连上次设备
- [ ] 回连失败后进入轮询模式
- [ ] 按键唤醒后自动回连
- [ ] 断开后进入轮询模式
- [ ] 60秒超时进入休眠
- [ ] HID 报告正确发送（键盘/多媒体）

---

## 11. 测试方法

1. 首次开机，验证自动配对模式
2. iPad 蓝牙设置中查找 "inateck AceTouch"
3. 配对成功后测试打字
4. 关闭 iPad 蓝牙，验证回连行为
5. 长时间不操作，验证休眠
6. 按键唤醒，验证回连

---

## 12. 参考文件

- `middleware/communication/wireless.c` - 无线状态机
- `middleware/communication/wireless.h` - 状态定义
- `drivers/communication/bluetooth/ch584/hidkbd.c` - HID 实现
