# Feature List Template

Use this template structure when generating feature lists from product specification documents.

---

## Template Structure

```markdown
# <ProductModel> 键盘产品功能清单

## 1. 项目概述

| 项目 | 规格 |
|------|------|
| **产品型号** | <model> |
| **连接方式** | <connection_method> |
| **支持系统** | <supported_os> |
| **键盘布局** | <layout> |
| **丝印语种** | <language_variants> |
| **触控板** | <touchpad_type> |
| **背光** | <backlight_type> |
| **主控芯片** | <controller_chips> |
| **电池** | <battery_spec> |
| **蓝牙配对名称** | <bluetooth_name> |

---

## 2. 系统兼容性模块

### 2.1 <OS_Name> 系统支持

| 功能名称 | 简要描述 | 优先级 | 涉及模块 |
|----------|----------|--------|----------|
| <feature> | <description> | P0/P1/P2 | <modules> |

---

## 3. 键芯矩阵模块

### 3.1 丝印语种列表

| 语种 | 代码 | 特殊键差异 |
|------|------|------------|
| <language> | <code> | <differences> |

### 3.2 通用组合键功能

| 功能名称 | 组合键 | 简要描述 | 优先级 | 涉及模块 |
|----------|--------|----------|--------|----------|
| <feature> | <combo> | <description> | P0/P1/P2 | <modules> |

### 3.3 各语种特殊组合键

#### <Language_Code>（<Language_Name>）

| 功能名称 | 组合键 | 输出 | 优先级 | 涉及模块 |
|----------|--------|------|--------|----------|
| <feature> | <combo> | <output> | P0/P1/P2 | <modules> |

### 3.4 多媒体/系统快捷键

| 功能名称 | 按键 | 简要描述 | 优先级 | 涉及模块 |
|----------|------|----------|--------|----------|
| <feature> | <key_position> | <description> | P0/P1/P2 | <modules> |

---

## 4. 背光模块

### 4.1 背光基础功能

| 功能名称 | 简要描述 | 优先级 | 涉及模块 |
|----------|----------|--------|----------|
| 背光颜色切换 | 支持 N 种颜色循环：<colors> | P1 | 固件/硬件 |
| 背光亮度调节 | N 档亮度：<levels> | P1 | 固件 |
| 背光默认值 | 开机默认<default_color><default_level> | P1 | 固件 |
| 背光休眠 | <timeout>秒无操作背光熄灭 | P1 | 固件 |
| 背光颜色记忆 | 关机/休眠后保持上次颜色设置 | P1 | 固件 |
| 低电量背光关闭 | 电量低于<threshold>%时强制关闭背光 | P1 | 固件 |

---

## 5. 蓝牙模块

### 5.1 蓝牙基础参数

| 功能名称 | 简要描述 | 优先级 | 涉及模块 |
|----------|----------|--------|----------|
| 蓝牙版本 | 蓝牙 <version> 及以上 | P0 | 硬件 |
| 配对名称 | "<device_name>" | P0 | 固件 |
| 有效距离 | <range> 米范围内有效操作 | P0 | 硬件 |
| 通道数量 | 仅支持连接 1 台设备 / 支持 N 通道 | P0 | 固件 |

### 5.2 蓝牙配对功能

| 功能名称 | 简要描述 | 优先级 | 涉及模块 |
|----------|----------|--------|----------|
| 首次配对 | 首次开机或恢复出厂后自动进入配对模式 | P0 | 固件 |
| 手动配对 | <trigger_method> 启动配对模式 | P0 | 固件 |
| 配对超时 | 广播 <timeout> 秒，超时进入休眠 | P0 | 固件 |
| 配对完成时间 | 设备端点击配对后 <time> 秒内完成连接 | P0 | 固件 |

### 5.3 蓝牙回连/轮询功能

| 功能名称 | 简要描述 | 优先级 | 涉及模块 |
|----------|----------|--------|----------|
| 开机自动回连 | 开机后直接回连上次连接的设备 | P0 | 固件 |
| 回连完成时间 | <time> 秒内完成回连 | P0 | 固件 |
| 回连超时处理 | 回连失败后进入轮询模式（可接受新连接） | P0 | 固件 |
| 轮询模式时长 | 轮询 <timeout> 秒，超时进入休眠 | P0 | 固件 |

### 5.4 蓝牙休眠与功耗

| 功能名称 | 简要描述 | 优先级 | 涉及模块 |
|----------|----------|--------|----------|
| 深度休眠进入 | 蓝牙断开后 <timeout> 分钟进入深度休眠 | P0 | 固件 |
| 休眠电流 | 休眠电流 < <current>μA | P0 | 硬件/固件 |

---

## 6. 指示灯模块

### 6.1 指示灯硬件定义

| 位置 | 颜色 | 用途 |
|------|------|------|
| <position> | <color> | <purpose> |

### 6.2 灯效定义

| 灯效名称 | 描述 | 用途 |
|----------|------|------|
| 慢闪 | 每秒闪 1 次 | 回连状态 |
| 快闪 | 每秒闪 3 次 | 配对状态 |
| 常亮 | 持续点亮 | 状态确认 |

### 6.3 <Indicator_Name>指示

| 功能名称 | 简要描述 | 优先级 | 涉及模块 |
|----------|----------|--------|----------|
| <state> | <behavior> | P0/P1 | 固件 |

---

## 7. 触控板模块

### 7.1 触控板基础功能

| 功能名称 | 简要描述 | 优先级 | 涉及模块 |
|----------|----------|--------|----------|
| N点触控 | 支持最多 N 点同时触控 | P0 | 硬件/固件 |
| 触控板报文注册 | 作为原生触控板设备注册（非模拟鼠标） | P0 | 固件 |
| 系统原生手势 | 支持 <OS> 全部原生触控板手势 | P0 | 固件 |

### 7.2 触控板高级功能

| 功能名称 | 简要描述 | 优先级 | 涉及模块 |
|----------|----------|--------|----------|
| <feature> | <description> | P0/P1/P2 | <modules> |

---

## 8. 电源管理模块

### 8.1 开关机功能

| 功能名称 | 简要描述 | 优先级 | 涉及模块 |
|----------|----------|--------|----------|
| 开机 | <trigger_method> 开机 | P0 | 固件 |
| 关机 | <trigger_method> 关机 | P0 | 固件 |

### 8.2 电池管理

| 功能名称 | 简要描述 | 优先级 | 涉及模块 |
|----------|----------|--------|----------|
| 电池容量 | <capacity>mAh 锂电池 | P0 | 硬件 |
| 电池保护 | 过压/过流/短路/过放保护 | P0 | 硬件 |

### 8.3 功耗规格

| 功能名称 | 简要描述 | 优先级 | 涉及模块 |
|----------|----------|--------|----------|
| 休眠功耗 | 休眠电流 < <current>μA | P0 | 固件/硬件 |
| 关机功耗 | 关机电流 <current>μA | P0 | 硬件 |
| 工作功耗 | 正常工作 ≤ <current>mA | P0 | 固件/硬件 |

---

## 9. 应用端（APP）模块

### 9.1 APP 基础功能

| 功能名称 | 简要描述 | 优先级 | 涉及模块 |
|----------|----------|--------|----------|
| 实时电量显示 | APP 显示键盘当前电量 | P1 | 固件/APP |
| OTA 固件升级 | 通过蓝牙无线升级固件 | P0 | 固件/APP |

### 9.2 APP 自定义功能

| 功能名称 | 简要描述 | 优先级 | 涉及模块 |
|----------|----------|--------|----------|
| 自定义键功能设置 | 设置自定义键功能 | P1 | 固件/APP |
| <feature> | <description> | P1/P2 | 固件/APP |

### 9.3 自定义键可选功能列表

| 功能选项 | 对应 HID 码/行为 |
|----------|------------------|
| <function> | <hid_usage> |

---

## 10. 恢复出厂设置

| 功能名称 | 简要描述 | 优先级 | 涉及模块 |
|----------|----------|--------|----------|
| 触发方式 | <trigger_method> | P0 | 固件 |
| 完成指示 | <indicator_behavior> | P0 | 固件 |
| 清除配对 | 清除所有蓝牙配对记录 | P0 | 固件 |
| 重置设置 | 恢复默认背光/亮度等设置 | P1 | 固件 |

---

## 11. 功能优先级汇总

| 优先级 | 数量 | 说明 |
|--------|------|------|
| **P0** | N 项 | 核心功能，必须实现，影响产品基本可用性 |
| **P1** | N 项 | 重要功能，显著提升用户体验 |
| **P2** | N 项 | 增值功能，主要为 APP 端高级设置 |

---

## 12. 模块依赖关系

```
硬件层
  └── 固件层
        ├── 矩阵扫描 ← 按键输入
        ├── 触控板驱动 ← <touchpad_chip>
        ├── 蓝牙协议栈 ← <bluetooth_chip>
        ├── 背光驱动 ← PWM/RGB LED
        ├── 电源管理 ← 电池监测/休眠
        └── OTA 升级 ← 蓝牙通道
              └── APP 层
                    ├── 电量显示
                    ├── 固件升级
                    └── 自定义配置
```

---

## 附录 A: 各语种蓝牙配对键映射

| 语种 | 配对组合键 |
|------|------------|
| <variant> | <combo> |

---

## 附录 B: 状态机概览

### 蓝牙状态机

[Include state machine diagram if applicable]

### 背光状态机

[Include state machine diagram if applicable]

---

*文档版本: V1.0*
*生成日期: <date>*
*基于: <source_document>*
```

---

## Priority Assignment Guidelines

### P0 (Critical)
- Basic connectivity (Bluetooth pairing, reconnection)
- Core input functionality (key scanning, typing)
- Power on/off
- Battery protection
- Safety-critical features

### P1 (Important)
- Enhanced user experience (backlight, indicators)
- Status feedback (LED patterns)
- State memory (backlight color, caps lock)
- APP basic features
- Custom key functionality

### P2 (Nice-to-have)
- APP advanced settings
- Fine-tuning options (timeout adjustments)
- Secondary customization features

---

## Module Assignment Guidelines

| Module | Scope |
|--------|-------|
| **硬件** | Physical components, electrical specifications, chip selection |
| **固件** | Embedded software, drivers, protocols, state machines |
| **APP** | Mobile/desktop application features, OTA, configuration UI |
| **固件/硬件** | Features requiring both firmware logic and hardware support |
| **固件/APP** | Features requiring firmware support and APP interface |
