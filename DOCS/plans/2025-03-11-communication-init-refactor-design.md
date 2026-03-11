# 通信模块初始化流程重构设计

## 概述

重构 `middleware/communication` 和 `application/system` 的初始化流程，明确 `transport` 和 `wireless` 的职责边界，遵循 Keychron 架构设计原则。

## 问题分析

### 当前问题

1. **bt_driver_init() 从未被调用** - BLE 协议栈未初始化
2. **职责混乱** - `wireless_switch_to_bt_driver()` 在中间件层被调用，但只注册函数指针
3. **初始化顺序错误** - 缺少"具体驱动 init → wireless_init → set_transport"的清晰流程

## 架构设计

### 职责划分

| 模块 | 职责 | 核心接口 |
|------|------|----------|
| **transport** | 通道选择（USB/BLE/2.4G），切换时协调 host_driver | `set_transport()`, `get_transport()` |
| **wireless** | 无线状态机、事件队列、HID 收发抽象 | `wireless_init()`, `wt_func_t` |
| **bt_driver** | CH584 BLE 协议栈具体实现 | `bt_driver_init()`, `bt_driver_connect_ex()` |

### 初始化顺序

```
1. Driver 层: bt_driver_init(false)
   └── CH58x_BLEInit() → HAL_Init() → GAPRole_PeripheralInit()

2. Middleware 层: wireless_init()
   └── 设置 WT_INITIALIZED, 加载配置

3. Application 层: set_transport(TRANSPORT_BLUETOOTH)
   └── bt_transport_enable(true)
       ├── host_set_driver(&wireless_driver)
       ├── wireless_switch_to_bt_driver()
       └── wireless_connect_ex(host_idx, 0)
```

### 依赖关系

```
transport ──依赖──> wireless (调用 wireless_disconnect/connect_ex)
wireless ──查询──> transport (通过 get_transport() 判断当前通道)
```

## 修改清单

### 1. system_init.c

**修改 `system_init_drivers()`：**
- 添加 `bt_driver_init(false)` 调用（在 `#ifdef BLUETOOTH_ENABLE_FLAG` 条件下）

**修改 `system_init_middleware()`：**
- 移除 `wireless_switch_to_bt_driver()` 调用
- 保留 `wireless_init()` 调用

### 2. 新增 system_select_default_transport()

**位置：** `system_init_application()` 末尾或单独函数

**逻辑：**
1. 检测 USB 是否连接 → 使用 USB 通道
2. 否则使用蓝牙或 2.4G 通道
3. 调用 `set_transport(default_transport)`

### 3. 无需修改的文件

- `wireless.c` - `wireless_init()` 保持当前实现
- `transport.c` - `set_transport()` 保持当前实现
- `_bt_driver.c` - `bt_driver_init()` 保持当前实现

## 验证点

1. 上电后 `bt_driver_init()` 被调用一次
2. `wireless_init()` 将状态设为 `WT_INITIALIZED`
3. `set_transport(TRANSPORT_BLUETOOTH)` 触发连接流程
4. 蓝牙设备可被发现并配对

## 风险评估

- **低风险** - 仅调整初始化调用顺序，不修改核心逻辑
- **回滚方案** - 恢复原有初始化代码即可

## 参考文档

- Keychron V6 Max 通信架构设计
- CLAUDE.md 模块生命周期说明
