# 无线驱动拆分设计方案

## 概述

本文档详细描述了将QMK Keychron无线驱动拆分为独立蓝牙和2.4G驱动模块的设计方案。拆分后的架构更加清晰，便于维护和扩展。

## 设计背景

### 原始问题
- QMK Keychron使用统一的`wireless_transport`全局变量管理所有无线驱动
- 蓝牙和2.4G驱动通过索引（如P24G_INDEX=24）区分
- 难以明确区分不同驱动的独立功能和代码职责

### 拆分目标
- 明确分离蓝牙和2.4G驱动代码
- 保持向后兼容性
- 便于独立开发和维护
- 支持透明驱动切换

## 架构设计

### 整体架构
```
┌─────────────────────────────────────────────────┐
│            应用层 (Application Layer)             │
│  - 键盘矩阵扫描  - 按键处理  - 模式切换           │
├─────────────────────────────────────────────────┤
│          传输管理层 (Transport Manager)           │
│  - set_transport()  - 模式切换  - 驱动通知       │
├─────────────────┬───────────────────────────────┤
│   蓝牙服务层     │        2.4G服务层              │
│ (Bluetooth Svc) │    (2.4G Wireless Svc)        │
│                 │                               │
│ - 状态机管理     │ - 状态机管理                   │
│ - 事件队列       │ - 事件队列                     │
│ - HID报告处理    │ - HID报告处理                   │
├─────────────────┼───────────────────────────────┤
│   蓝牙驱动层     │        2.4G驱动层              │
│ (BT Driver HAL) │    (P24G Driver HAL)          │
│                 │                               │
│ - bt_driver.c   │ - p24g_driver.c               │
│ - 蓝牙协议栈     │ - 2.4G协议栈                   │
│ - SPI通信        │ - SPI通信                      │
├─────────────────┴───────────────────────────────┤
│              硬件抽象层 (HAL)                      │
│  - SPI驱动  - GPIO控制  - 定时器  - 中断处理       │
└─────────────────────────────────────────────────┘
```

### 核心组件

#### 1. 驱动接口层
- **bt_driver.h/c**: 蓝牙驱动接口
- **p24g_driver.h/c**: 2.4G驱动接口
- 适配统一的`wt_func_t`接口结构

#### 2. 无线管理层
- **wireless.c/h**: 无线核心管理
  - 状态机管理
  - 事件队列
  - HID报告处理
  - 双驱动切换

#### 3. 传输管理层
- **transport.c/h**: 传输模式切换
  - USB/蓝牙/2.4G模式切换
  - 驱动切换通知

## 关键实现

### 1. 驱动注册机制

#### 蓝牙驱动注册
```c
// 注册蓝牙驱动
void wireless_register_bt_driver(bt_driver_t *driver) {
    bt_driver = driver;
    kc_printf("Wireless: BT driver registered\n");
}
```

#### 2.4G驱动注册
```c
// 注册2.4G驱动
void wireless_register_p24g_driver(p24g_driver_t *driver) {
    p24g_driver = driver;
    kc_printf("Wireless: P24G driver registered\n");
}
```

### 2. 驱动切换机制

#### 当前驱动管理
```c
// 双驱动管理
static wt_func_t  *current_driver = NULL;  // 当前使用的驱动
static bt_driver_t *bt_driver = NULL;      // 蓝牙驱动实例
static p24g_driver_t *p24g_driver = NULL;  // 2.4G驱动实例

// 切换到蓝牙驱动
void wireless_switch_to_bt_driver(void) {
    if (bt_driver) {
        current_driver = (wt_func_t*)bt_driver;
        wireless_transport = *current_driver;  // 保持向后兼容
        kc_printf("Wireless: Switched to BT driver\n");
    }
}
```

### 3. 传输模式切换

#### set_transport函数增强
```c
void set_transport(transport_t new_transport) {
    switch (transport) {
        case TRANSPORT_BLUETOOTH:
            // ... 原有逻辑 ...
            // 通知无线层切换到蓝牙驱动
            wireless_switch_to_bt_driver();
            break;

        case TRANSPORT_P2P4:
            // ... 原有逻辑 ...
            // 通知无线层切换到2.4G驱动
            wireless_switch_to_p24g_driver();
            break;

        case TRANSPORT_USB:
            // ... 原有逻辑 ...
            // 通知无线层切换到USB模式
            wireless_switch_to_usb_mode();
            break;
    }
}
```

### 4. 透明数据发送

#### 当前驱动调用
```c
void wireless_task(void) {
    // 调用当前驱动的任务函数
    if (current_driver && current_driver->task) {
        current_driver->task();
    }

    // ... 其他任务 ...
}

void wireless_send_keyboard(report_keyboard_t *report) {
    if (current_driver && current_driver->send_keyboard) {
        current_driver->send_keyboard(&report->mods);
    }
}
```

## 文件结构

### 新增文件
```
drivers/communication/
├── bluetooth/
│   ├── bt_driver.h          # 蓝牙驱动接口
│   └── bt_driver.c          # 蓝牙驱动实现
└── p2p4g/
    ├── p24g_driver.h         # 2.4G驱动接口
    └── p24g_driver.c         # 2.4G驱动实现

examples/
└── wireless_driver_example.c # 驱动使用示例
```

### 修改文件
```
middleware/communication/
├── wireless.h                # 添加双驱动管理函数声明
├── wireless.c                # 添加双驱动管理实现
├── transport.h               # 添加驱动切换通知声明
└── transport.c               # 添加驱动切换通知实现
```

## 向后兼容性

### 保持原有接口
- 保留`wireless_transport`全局变量
- 保留`wireless_set_transport()`函数
- 保留所有原有函数接口

### 渐进式迁移
1. **第一阶段**: 同时支持新旧接口
2. **第二阶段**: 逐步迁移到新接口
3. **第三阶段**: 移除旧接口（可选）

## 使用示例

### 基本初始化
```c
int main(void) {
    // 1. 初始化无线模块
    wireless_init();

    // 2. 注册驱动
    wireless_register_bt_driver(&g_bt_driver);
    wireless_register_p24g_driver(&g_p24g_driver);

    // 3. 设置默认模式
    set_transport(TRANSPORT_BLUETOOTH);

    // 4. 主循环
    while (1) {
        wireless_task();
        // ... 其他任务 ...
    }
}
```

### 模式切换
```c
// 切换到蓝牙模式
set_transport(TRANSPORT_BLUETOOTH);

// 切换到2.4G模式
set_transport(TRANSPORT_P2P4G);

// 切换到USB模式
set_transport(TRANSPORT_USB);
```

## 优势分析

### 1. 代码职责清晰
- 蓝牙和2.4G驱动完全独立
- 易于理解和维护
- 减少代码耦合

### 2. 独立开发
- 蓝牙和2.4G可以独立开发和调试
- 不同团队可以并行工作
- 减少开发冲突

### 3. 易于测试
- 可以独立测试蓝牙和2.4G驱动
- 便于单元测试和集成测试
- 快速定位问题

### 4. 灵活扩展
- 可以轻松添加新的无线协议
- 支持多驱动并存
- 便于功能定制

### 5. 向后兼容
- 保持原有API不变
- 平滑迁移过程
- 降低迁移风险

## 迁移建议

### 开发顺序
1. **第一阶段**: 创建驱动适配层
   - 实现bt_driver.h/c
   - 实现p24g_driver.h/c
   - 适配lkbt51.c功能

2. **第二阶段**: 修改无线管理层
   - 修改wireless.c/h
   - 添加双驱动管理
   - 修改传输管理层

3. **第三阶段**: 测试和验证
   - 单元测试
   - 集成测试
   - 性能测试

### 测试策略
1. **单元测试**: 测试各个驱动模块
2. **集成测试**: 测试双驱动协同工作
3. **回归测试**: 确保原有功能正常
4. **性能测试**: 确保性能无损失

### 风险控制
1. **灰度发布**: 逐步替换原有代码
2. **回滚机制**: 保留原有代码作为备份
3. **监控告警**: 监控运行状态和错误
4. **文档更新**: 及时更新相关文档

## 后续工作

### 1. 实现具体驱动
- 完善bt_driver.c中的具体实现
- 完善p24g_driver.c中的具体实现
- 适配具体硬件平台

### 2. 优化性能
- 优化驱动切换效率
- 减少内存占用
- 提高响应速度

### 3. 增强功能
- 添加驱动热插拔支持
- 添加多设备管理
- 添加电量管理增强

### 4. 完善测试
- 添加自动化测试
- 添加压力测试
- 添加兼容性测试

## 结论

通过将QMK Keychron无线驱动拆分为独立的蓝牙和2.4G驱动模块，我们实现了：

1. **清晰的代码架构**: 明确的职责分离和模块边界
2. **良好的可维护性**: 易于理解和修改的代码结构
3. **强大的扩展性**: 支持新协议和新功能的快速集成
4. **完全的向后兼容**: 平滑的迁移过程和风险控制

这种设计为项目的长期发展奠定了坚实的基础，既满足了当前的开发需求，又为未来的功能扩展预留了充足的空间。
