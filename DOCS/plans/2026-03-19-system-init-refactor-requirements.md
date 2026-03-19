# 系统初始化流程重构 - 需求文档

> 创建日期: 2026-03-19
> 状态: 需求确认完成，待方案设计

## 1. 需求背景

当前项目 `./application/system` 系统初始化步骤太多，又有一部分初始化跑到了 service 层，层级不清晰，使用混乱。

## 2. 核心目标

**代码可维护性** - 新人能快速理解初始化流程，添加新模块有明确入口。

## 3. 约束条件

可以适度重构 - 允许修改接口和调整模块边界以获得更清晰的架构。

## 4. 成功标准

1. **文档完备** - 有清晰的初始化流程图和每个模块的职责说明，新人能看懂
2. **编译通过 + 功能正常** - 现有功能全部正常工作，无新增 bug
3. **可扩展性验证** - 新增 service 模块仅需修改 ≤3 处代码

## 5. 重构范围

`system_init.c` + 4 个 service 模块（system_service, input_service, output_service, communication_service），统一生命周期管理，明确 setup/init/pre_task 等阶段职责。

**不涉及：** HAL 层、Driver 层底层实现（仅调整调用位置）。

## 6. 现状分析

### 6.1 核心问题

| 问题类别 | 具体表现 | 影响 |
|---------|---------|------|
| Setup 阶段空白 | 4 个 `_setup` 函数仅有状态标记，无实际初始化 | 无法提前分配资源 |
| Pre_task 阶段缺失 | 状态机定义了但未实现 | 无主循环前的检查机制 |
| 重复初始化 | storage_init、lpm_init、wireless_init、battery_init 多处调用 | 资源泄漏、幂等性问题 |
| 跨层调用 | service 层直接调用 driver 层 init 函数 | 违反分层原则 |
| 初始化延迟 | system_select_default_transport() 在事件中调用 | 启动后无默认通道 |

### 6.2 当前初始化状态机

```
system_init_coordinator()
├─ HAL_SETUP        → 空
├─ DRIVER_SETUP     → 空
├─ MIDDLEWARE_SETUP → 空
├─ APPLICATION_SETUP → 空
├─ HAL_INIT         → UART/GPIO/Timer
├─ DRIVER_INIT      → storage/bt_driver/indicator/timer
├─ MIDDLEWARE_INIT  → lpm/wireless/keyboard
└─ APPLICATION_INIT → 4个service初始化（含跨层调用）

缺失：PRE_TASK_*, TASK_*, POST_TASK_*
```

### 6.3 重复初始化问题清单

| 函数 | 调用位置1 | 调用位置2 | 问题 |
|------|----------|----------|------|
| `battery_init()` | 应在 system_init_drivers() | input_service_init() 内部 | 跨层调用 |
| `backlight_init()` | 应在 system_init_drivers() | output_service_init() 内部 | 跨层调用 |
| `storage_init()` | system_init_drivers() | SYSTEM_WAKEUP_EVT 事件处理 | 重复调用 |
| `wireless_init()` | system_init_middleware() | WL_INIT_EVT 事件处理 | 重复调用 |

### 6.4 Service 层初始化职责分析

| 模块 | 当前职责 | 应属层级 |
|------|---------|---------|
| `input_service_init()` | battery_init ❌、矩阵扫描定时器 ✓ | Driver/Application 混合 |
| `output_service_init()` | backlight_init ❌、背光默认配置 ✓ | Driver/Application 混合 |
| `system_service_init()` | OSAL 注册 ✓、LPM 检查任务 ✓、storage_init ❌ | Application |
| `commu_service_init()` | OSAL 注册 ✓、wireless_init ❌ | Application |

## 7. 评审意见汇总

### 用户/产品视角
- 可扩展性验证主观性强，建议量化（如"新增模块≤3处修改"）
- 选择 1 款复杂产品作为验证标杆
- 建议渐进式迁移

### 架构/编码视角
- 统一生命周期接口规范（_setup/_init/_pre_task）
- 建立初始化状态注册表防止重复
- Service 层只负责业务编排，不直接调用底层初始化

### 测试/质量视角
- 补充端到端测试用例
- 增加初始化超时保护
- 增加返回值检查，失败时记录具体模块名
