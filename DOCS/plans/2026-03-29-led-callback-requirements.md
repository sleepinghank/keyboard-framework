# 需求文档：统一三模 HID LED 回调架构

> 日期：2026-03-29
> 状态：需求确认中

## 1. 需求概述

**一句话描述：** 为蓝牙驱动 bt_driver 注册大小写灯回调，调用点在 hidEmuRcvReport，从整体系统架构层面统一三模（USB/BLE/2.4G）LED 回调机制。

## 2. 需求 Q&A

| 维度 | 答案 |
|------|------|
| 目的 | 统一三模（USB/BLE/2.4G）LED 回调架构 |
| 现有链路 | 不确定各模式现状，需代码分析确认 |
| 回调范围 | 所有 HID LED 状态（Caps Lock / Num Lock / Scroll Lock） |
| 实现约束 | 复用 host_driver 回调 / wireless.c 注册 / hidEmuRcvReport 调用 |

## 3. 功能范围

### IN（包含）
- BLE 模式完整 LED 回调链路：hidEmuRcvReport → bt_driver → wireless → indicator
- 复用现有 host_driver_t 的 keyboard_leds 回调机制
- 2.4G/USB 预留统一接口（仅声明，不实现）

### OUT（不包含）
- 2.4G/USB 模式的 LED 回调具体实现
- 连接状态指示灯、配对提示灯、低电量灯效（非 HID LED 范畴）
- 自定义灯效扩展

## 4. 评审意见

### 用户/产品视角
- USB 直连场景下回调注册路径需区分
- 模式切换时回调如何迁移需设计
- 多连接（USB + BLE 同时在线）时 LED 回调优先级需明确

### 架构/编码视角
- hidEmuRcvReport 在 Profile 层（低于 Driver），跨多层回调需收敛
- host_driver_t 的 keyboard_leds 当前语义需确认是否覆盖 BLE/2.4G
- 2.4G 模式的 HID Out Report 机制是关键遗漏，需代码分析确认

### 测试/质量视角
- 验收标准需量化（回调延迟、状态一致性）
- BLE/2.4G 需实机测试，USB 可桌面仿真
- 边界条件：模式切换、断连瞬间的 LED 状态竞态

## 5. 代码现状分析

### 各模块现状

| 模块 | 文件 | 现状 | 差距 |
|------|------|------|------|
| BLE HID | `drivers/communication/bluetooth/ch584/hidkbd.c` | hidEmuRcvReport 收到 LED 报告，仅打印日志 | 缺少回调通知上层 |
| BT Driver | `drivers/communication/bluetooth/ch584/_bt_driver.c` | 无 LED 相关接口 | 需添加 LED 回调注册/调用接口 |
| Wireless | `middleware/communication/wireless.c` | 驱动函数表中无 LED 回调字段 | 需添加 LED 回调注册 |
| Host Driver | `middleware/communication/host_driver.h` | keyboard_leds 回调签名 `uint8_t (*)(void)` | 需确认语义是否满足需求 |
| Host | `middleware/communication/host.c` | 已实现 host_keyboard_leds() | 与 indicator 未联动 |
| Indicator | `drivers/output/indicators/indicator.c` | LED 控制接口完善 | 缺少外部 LED 状态更新入口 |

### 数据流

- **当前**：hidkbd.hidEmuRcvReport → 打印日志 → 终止
- **目标**：hidkbd.hidEmuRcvReport → bt_driver 回调 → wireless → indicator 更新

### 关键差距

1. bt_driver 缺少 LED 回调注册接口（无线驱动层需要对外暴露注册函数）
2. wireless.c 需要注册 LED 回调到 bt_driver
3. hidEmuRcvReport 中需要调用已注册的回调
4. indicator 需要接收并处理外部 LED 状态更新
