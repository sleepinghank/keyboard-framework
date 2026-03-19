# 系统初始化流程重构 - 设计文档

> 创建日期: 2026-03-19
> 状态: 设计完成，待实施

## 1. 需求摘要

### 1.1 背景
当前项目 `./application/system` 系统初始化步骤太多，又有一部分初始化跑到了 service 层，层级不清晰，使用混乱。

### 1.2 核心目标
**代码可维护性** - 新人能快速理解初始化流程，添加新模块有明确入口。

### 1.3 约束条件
可以适度重构 - 允许修改接口和调整模块边界以获得更清晰的架构。

### 1.4 成功标准
1. **文档完备** - 有清晰的初始化流程图和每个模块的职责说明
2. **编译通过 + 功能正常** - 现有功能全部正常工作，无新增 bug
3. **可扩展性验证** - 新增 service 模块仅需修改 ≤3 处代码

### 1.5 重构范围
`system_init.c` + 4 个 service 模块，统一生命周期管理。

---

## 2. 现状分析

### 2.1 核心问题

| 问题类别 | 具体表现 | 影响 |
|---------|---------|------|
| Setup 阶段空白 | 4 个 `_setup` 函数仅有状态标记，无实际初始化 | 无法提前分配资源 |
| 重复初始化 | storage_init、wireless_init、battery_init 多处调用 | 资源泄漏、幂等性问题 |
| 跨层调用 | service 层直接调用 driver 层 init 函数 | 违反分层原则 |
| 初始化延迟 | system_select_default_transport() 在事件中调用 | 启动后无默认通道 |

### 2.2 当前初始化状态机

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

### 2.3 重复初始化问题清单

| 函数 | 调用位置1 | 调用位置2 | 问题 |
|------|----------|----------|------|
| `battery_init()` | 应在 system_init_drivers() | input_service_init() 内部 | 跨层调用 |
| `backlight_init()` | 应在 system_init_drivers() | output_service_init() 内部 | 跨层调用 |
| `storage_init()` | system_init_drivers() | SYSTEM_WAKEUP_EVT 事件处理 | 重复调用（保留，有幂等检查） |
| `wireless_init()` | system_init_middleware() | WL_INIT_EVT 事件处理 | 重复调用（删除） |

---

## 3. 方案设计

### 3.1 架构设计

**生命周期简化：init → task**

```
┌─────────────────────────────────────────────────────────────────┐
│                        main.c / application                      │
│                              │                                   │
│                    system_init_coordinator()                     │
│                              │                                   │
├──────────────────────────────┼───────────────────────────────────┤
│  Init 阶段                   ▼                                   │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │ Layer 1: HAL                                                 ││
│  │   hal_init() → GPIO/UART/Timer 硬件初始化                   ││
│  └─────────────────────────────────────────────────────────────┘│
│                              ▼                                   │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │ Layer 2: Driver                                              ││
│  │   storage_init()     → EEPROM 抽象层                        ││
│  │   bt_driver_init()   → BLE 协议栈                           ││
│  │   indicator_init()   → 指示灯                               ││
│  │   battery_init()     → 电池管理 [从 service 移入]           ││
│  │   backlight_init()   → 背光控制 [从 service 移入]           ││
│  │   timer_init()       → 系统定时器                           ││
│  └─────────────────────────────────────────────────────────────┘│
│                              ▼                                   │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │ Layer 3: Middleware                                          ││
│  │   lpm_init()         → 低功耗管理                           ││
│  │   wireless_init()    → 无线状态机                           ││
│  │   keyboard_init()    → 键盘处理（矩阵/防抖/combo）          ││
│  └─────────────────────────────────────────────────────────────┘│
│                              ▼                                   │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │ Layer 4: Application (Service)                               ││
│  │   system_service_init()      → OSAL 注册、LPM 检查任务      ││
│  │   input_service_init()       → OSAL 注册、矩阵扫描定时器    ││
│  │   output_service_init()      → OSAL 注册、背光配置          ││
│  │   commu_service_init()       → OSAL 注册、默认通道选择      ││
│  └─────────────────────────────────────────────────────────────┘│
├──────────────────────────────────────────────────────────────────┤
│  Task 阶段（主循环）                                              │
│    keyboard_task() / indicator_task() / 各 service 事件处理      │
└──────────────────────────────────────────────────────────────────┘
```

### 3.2 状态机简化

**当前（10 状态）→ 简化后（4 状态）：**

```c
typedef enum {
    SYSTEM_INIT_IDLE,
    SYSTEM_INIT_HAL,
    SYSTEM_INIT_DRIVER,
    SYSTEM_INIT_MIDDLEWARE,
    SYSTEM_INIT_APPLICATION,
    SYSTEM_INIT_TASK,
    SYSTEM_INIT_COMPLETE
} system_init_status_t;
```

### 3.3 组件/接口改动

#### 3.3.1 system_init.c 改动

**删除内容：**
- 4 个空 setup 函数：`system_init_hal_setup()` 等
- 4 个 setup 状态枚举：`SYSTEM_INIT_HAL_SETUP` 等

**新增内容（system_init_drivers）：**
```c
static void system_init_drivers(void)
{
    storage_init();
    bt_driver_init(false);
    indicator_init();
    timer_init();
    battery_init();     // 新增：从 input_service 移入
    backlight_init();   // 新增：从 output_service 移入
}
```

#### 3.3.2 service 层改动

| 文件 | 改动 |
|------|------|
| `input_service.c` | 删除 `battery_init()` 调用 |
| `output_service.c` | 删除 `backlight_init()` 调用 |
| `system_service.c` | **保留** `SYSTEM_WAKEUP_EVT` 中的 `storage_init()`（有幂等检查） |
| `communication_service.c` | 删除 `WL_INIT_EVT` 中的 `wireless_init()` 调用 |

#### 3.3.3 幂等保护

**wireless_init() 添加幂等保护：**
```c
void wireless_init(void)
{
    static bool initialized = false;
    if (initialized) return;
    initialized = true;
    // ... 原有逻辑
}
```

#### 3.3.4 GPIO 安全状态

将 `system_hal_gpio_init_all()` 移至 `hal_init()` 开头调用。
（当前位于 `system_init_drivers()` 末尾被注释，需移至 HAL 层）

### 3.4 改动文件清单

| 文件 | 改动类型 | 改动内容 |
|------|---------|---------|
| `system_init.c` | 删除 | 4 个空 setup 函数及状态枚举 |
| `system_init.c` | 新增 | `battery_init()`、`backlight_init()` 调用 |
| `hal/platforms/ch584/_hal.c` | 新增 | `hal_init()` 开头调用 `system_hal_gpio_init_all()` |
| `wireless.c` | 新增 | 幂等保护标志 |
| `input_service.c` | 删除 | `battery_init()` 调用 |
| `output_service.c` | 删除 | `backlight_init()` 调用 |
| `communication_service.c` | 删除 | `WL_INIT_EVT` 中的 `wireless_init()` |
| `docs/system_init_startup_flow.md` | 更新 | 初始化流程图 |

### 3.5 依赖关系图

```
                    ┌─────────────┐
                    │   hal_init  │
                    └──────┬──────┘
                           │
           ┌───────────────┼───────────────┐
           ▼               ▼               ▼
    ┌────────────┐  ┌────────────┐  ┌────────────┐
    │storage_init│  │bt_driver   │  │timer_init  │
    │battery_init│  │backlight   │  │indicator   │
    └─────┬──────┘  └─────┬──────┘  └─────┬──────┘
          │               │               │
          ▼               ▼               ▼
    ┌────────────┐  ┌────────────┐  ┌────────────┐
    │  lpm_init  │  │wireless    │  │keyboard    │
    └─────┬──────┘  └─────┬──────┘  └─────┬──────┘
          │               │               │
          └───────────────┼───────────────┘
                          ▼
              ┌───────────────────────┐
              │   4个 service_init    │
              └───────────────────────┘
```

---

## 4. 实施计划

| Step | 内容 | 文件 | 预估改动 | 依赖 |
|------|------|------|---------|------|
| 1 | wireless_init 幂等保护 | `wireless.c` | 3 行 | 无 |
| 2 | GPIO 初始化移至 hal_init | `_hal.c` | 1 行 | 无 |
| 3 | 删除 setup 阶段 | `system_init.c/h` | ~50 行删除 | Step 2 |
| 4 | system_init_drivers 新增调用 | `system_init.c` | 2 行 | Step 3 |
| 5 | 移除 service 跨层调用 | 3 个 service 文件 | ~10 行删除 | Step 4 |
| 6 | 更新文档 | `docs/` | 文档 | Step 1-5 |
| 7 | 编译验证 | - | - | Step 1-6 |
| 8 | 功能回归测试 | - | - | Step 7 |
| 9 | 提交代码 | - | - | Step 8 |

---

## 5. 测试策略

### 5.1 编译验证
```bash
# Windows 仿真
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build -j4

# CH584M 固件
/wch-riscv-build
```

### 5.2 功能回归测试

| 场景 | 预期行为 | 验证方法 |
|------|---------|---------|
| 冷启动 | 指示灯正常亮起，无线可连接 | 烧录后上电观察 |
| 按键输入 | 按键响应正常，无漏键 | 按键测试 |
| 三模切换 | USB/BLE/2.4G 切换正常 | 拔插 USB、切换开关 |
| 低功耗 | 5 秒无操作进入 Idle | 观察电流或 LED |
| Idle 唤醒 | 按键唤醒，状态恢复 | Idle 后按键 |
| Deep 唤醒 | 配置恢复正常 | Deep 后验证背光/无线状态 |

### 5.3 可扩展性验证
新增 mock_service 模块，确认仅需修改 ≤3 处：
1. 新建 `mock_service.c/h`
2. `system_init_application()` 添加一行调用
3. （可选）main.c 的 task 循环添加 task 调用

---

## 6. 风险与缓解

| 风险 | 缓解措施 |
|------|---------|
| Deep 唤醒配置丢失 | 保留 `SYSTEM_WAKEUP_EVT` 中的 `storage_init()` |
| wireless_init 重复调用 | 添加幂等保护标志 |
| GPIO 安全状态丢失 | 移至 `hal_init()` 开头调用 |
| 状态机简化影响调试 | 更新文档，明确新状态机 |

---

## 7. 附录

### 7.1 评审意见汇总

**功能完整性：** Deep 唤醒场景需保留 storage_init（已采纳）

**技术可行性：** wireless_init 需幂等保护（已添加）

**可维护性：** 方向正确，需配套文档（已规划）

**可测试性：** 缺少幂等性验证（已添加幂等保护）

**风险识别：** Deep 唤醒配置丢失是最大风险（已通过保留 storage_init 解决）
