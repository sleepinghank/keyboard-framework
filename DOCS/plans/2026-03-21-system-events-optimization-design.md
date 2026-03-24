# 系统事件优化设计文档

> **文档类型**: 架构优化设计
> **创建日期**: 2026-03-21
> **影响范围**: `application/service/system_service.h`, `system_service.c`, 相关调用者

## 1. 背景与目标

### 1.1 问题背景

当前 `system_service.h` 定义了 16 个系统事件，其中 LPM 相关事件占据了 7 个位置（bits 8-14）。事件定义存在以下问题：

1. **事件冗余**：存在两套类似的事件（如 `SYSTEM_DEEP_SLEEP_EVT` vs `SYSTEM_LPM_DEEP_REQ_EVT`）
2. **僵尸事件**：`SYSTEM_IDLE_EVT` 标记为兼容旧接口，但无实际调用者
3. **语义重叠**：`SYSTEM_WAKEUP_EVT` 与 `SYSTEM_LPM_WAKE_EVT` 功能高度重叠

### 1.2 优化目标

1. **简化事件表**：减少冗余事件，释放事件位资源
2. **统一 LPM 模型**：所有睡眠/唤醒操作通过 LPM 状态机管理
3. **保持兼容性**：确保现有功能不受影响

## 2. 当前事件架构分析

### 2.1 事件清单与使用状态

| 事件名 | 位 | 当前用途 | 实际调用者 | 优化建议 |
|--------|---|----------|------------|----------|
| `SYSTEM_LOW_BATTERY_SHUTDOWN_EVT` | 0 | 低电关机 | `battery.c` | **保留** |
| `SYSTEM_IDLE_EVT` | 1 | 兼容旧接口 | **无** | **删除** |
| `SYSTEM_SHUTDOWN_EVT` | 2 | 系统关机 | 无线状态机 | **保留** |
| `SYSTEM_DEEP_SLEEP_EVT` | 3 | 深度睡眠 | `wireless_callbacks.c`, `hidkbd.c` | **删除，迁移到 LPM** |
| `SYSTEM_STORAGE_EVT` | 4 | 存储操作 | 多处 | **保留** |
| `SYSTEM_WAKEUP_EVT` | 5 | 系统唤醒 | **无触发者** | **删除，合并到 LPM_WAKE** |
| `SYSTEM_FACTORY_RESET_EVT` | 6 | 恢复出厂 | 按键组合 | **保留** |
| `SYSTEM_OTA_EVT` | 7 | OTA升级 | BLE 协议 | **保留** |
| `SYSTEM_LPM_IDLE_REQ_EVT` | 8 | LPM Idle 请求 | `lpm_task()` | **保留** |
| `SYSTEM_LPM_DEEP_REQ_EVT` | 9 | LPM Deep 请求 | `lpm_task()`, `wireless_callbacks.c` | **保留** |
| `SYSTEM_LPM_STEP_DONE_EVT` | 10 | Prepare 完成 | `input_service.c` | **保留** |
| `SYSTEM_LPM_ENTER_IDLE_EVT` | 11 | 进入 Idle | system_service 自投递 | **保留** |
| `SYSTEM_LPM_ENTER_DEEP_EVT` | 12 | 进入 Deep | system_service 自投递 | **保留** |
| `SYSTEM_LPM_WAKE_EVT` | 13 | 唤醒恢复 | `matrix.c` GPIO ISR | **保留** |
| `SYSTEM_LPM_CHECK_EVT` | 14 | LPM 周期检查 | OSAL reload timer | **保留** |

### 2.2 事件依赖关系图

```
┌─────────────────────────────────────────────────────────────────┐
│                    基础系统事件 (bits 0-7)                        │
├─────────────────────────────────────────────────────────────────┤
│ LOW_BATTERY ──> STORAGE ──> SHUTDOWN                            │
│                                                                 │
│ FACTORY_RESET ──> STORAGE ──> REBOOT                            │
│                                                                 │
│ OTA ──> DISCONNECT ──> BOOTLOADER                               │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                    LPM 睡眠/唤醒流程 (bits 8-14)                 │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────┐    ┌──────────┐    ┌──────────┐    ┌───────────┐  │
│  │ LPM_CHK │───>│ LPM_REQ  │───>│ STEP_DONE │───>│ ENTER_xxx │  │
│  │ (1s周期) │    │(IDLE/DEEP)│    │ (汇聚)   │    │ (二次确认)│  │
│  └─────────┘    └──────────┘    └──────────┘    └───────────┘  │
│                                       ↑                         │
│                                       │                         │
│                              ┌───────────────┐                  │
│                              │ GPIO ISR/RTC  │                  │
│                              │ (LPM_WAKE_EVT)│                  │
│                              └───────────────┘                  │
└─────────────────────────────────────────────────────────────────┘
```

## 3. 优化方案

### 3.1 删除事件清单

以下事件将被删除：

| 事件名 | 删除理由 |
|--------|----------|
| `SYSTEM_IDLE_EVT` | 无实际调用者，僵尸事件 |
| `SYSTEM_DEEP_SLEEP_EVT` | 功能已被 `SYSTEM_LPM_DEEP_REQ_EVT` 完全覆盖 |
| `SYSTEM_WAKEUP_EVT` | 功能已被 `SYSTEM_LPM_WAKE_EVT` 覆盖且更完善 |

### 3.2 迁移映射

| 旧事件 | 新事件 | 调用点迁移 |
|--------|--------|------------|
| `SYSTEM_DEEP_SLEEP_EVT` | `SYSTEM_LPM_DEEP_REQ_EVT` | `wireless_callbacks.c:202`, `hidkbd.c:1038-1040` |
| `SYSTEM_WAKEUP_EVT` | `SYSTEM_LPM_WAKE_EVT` | 当前无触发者，处理逻辑合并 |

### 3.3 事件处理逻辑迁移

#### `SYSTEM_WAKEUP_EVT` → `SYSTEM_LPM_WAKE_EVT`

`SYSTEM_WAKEUP_EVT` 的处理逻辑：
```c
// 当前 SYSTEM_WAKEUP_EVT 的处理（将被删除）
if (events & SYSTEM_WAKEUP_EVT) {
    storage_init();
    lpm_timer_reset();
    if (get_transport() == TRANSPORT_BLUETOOTH) {
        wireless_connect();
    }
    return (events ^ SYSTEM_WAKEUP_EVT);
}
```

`SYSTEM_LPM_WAKE_EVT` 已有的处理逻辑：
```c
// 当前 SYSTEM_LPM_WAKE_EVT 的处理（已包含更完善的逻辑）
if (events & SYSTEM_LPM_WAKE_EVT) {
    lpm_mode_t mode = lpm_get_mode();
    lpm_set_state(LPM_STATE_WAKE_RESUME);

    OSAL_SetEvent(input_taskID, INPUT_LPM_RESUME_EVT);

    if (mode == LPM_MODE_DEEP) {
        OSAL_SetEvent(output_taskID, OUTPUT_LPM_RESUME_EVT);
        OSAL_SetEvent(commu_taskID,  COMMU_LPM_RESUME_EVT);
    }

    lpm_set_state(LPM_STATE_ACTIVE);
    lpm_timer_reset();
    return (events ^ SYSTEM_LPM_WAKE_EVT);
}
```

**差异分析**：
- `storage_init()`: Deep 唤醒时可能需要，但应由 `INPUT_LPM_RESUME_EVT` 或 `COMMU_LPM_RESUME_EVT` 按需调用
- `wireless_connect()`: 已通过 `COMMU_LPM_RESUME_EVT` 处理

**结论**：`SYSTEM_LPM_WAKE_EVT` 的扇出模式更灵活，无需在 system_service 中直接调用。

#### `SYSTEM_DEEP_SLEEP_EVT` → `SYSTEM_LPM_DEEP_REQ_EVT`

当前 `SYSTEM_DEEP_SLEEP_EVT` 处理：
```c
if (events & SYSTEM_DEEP_SLEEP_EVT) {
    /* 转发为 LPM Deep 请求，由 LPM 状态机处理 */
    OSAL_SetEvent(system_taskID, SYSTEM_LPM_DEEP_REQ_EVT);
    return (events ^ SYSTEM_DEEP_SLEEP_EVT);
}
```

**结论**：已经是转发模式，删除后直接使用 `SYSTEM_LPM_DEEP_REQ_EVT` 即可。

### 3.4 优化后的事件定义

```c
typedef enum {
    /* 基础系统事件 (bits 0-7) */
    SYSTEM_LOW_BATTERY_SHUTDOWN_EVT = 1 << 0,  /**< 低电关机事件 */
    // SYSTEM_IDLE_EVT               = 1 << 1,  [已删除]
    SYSTEM_SHUTDOWN_EVT              = 1 << 2,  /**< 系统关机事件 */
    // SYSTEM_DEEP_SLEEP_EVT         = 1 << 3,  [已删除，使用 SYSTEM_LPM_DEEP_REQ_EVT]
    SYSTEM_STORAGE_EVT               = 1 << 4,  /**< 系统存储事件 */
    // SYSTEM_WAKEUP_EVT             = 1 << 5,  [已删除，使用 SYSTEM_LPM_WAKE_EVT]
    SYSTEM_FACTORY_RESET_EVT         = 1 << 6,  /**< 恢复出厂设置事件 */
    SYSTEM_OTA_EVT                   = 1 << 7,  /**< OTA升级事件 */

    /* LPM 调度事件（bits 8-14） */
    SYSTEM_LPM_IDLE_REQ_EVT          = 1 << 8,  /**< 请求进入 Idle */
    SYSTEM_LPM_DEEP_REQ_EVT          = 1 << 9,  /**< 请求进入 Deep */
    SYSTEM_LPM_STEP_DONE_EVT         = 1 << 10, /**< prepare 完成汇聚 */
    SYSTEM_LPM_ENTER_IDLE_EVT        = 1 << 11, /**< 最终进入 Idle */
    SYSTEM_LPM_ENTER_DEEP_EVT        = 1 << 12, /**< 最终进入 Deep */
    SYSTEM_LPM_WAKE_EVT              = 1 << 13, /**< 唤醒恢复入口 */
    SYSTEM_LPM_CHECK_EVT             = 1 << 14, /**< LPM 周期检查 */

    /* 预留位 (bits 1, 3, 5) 供未来扩展 */
} system_task_event_t;
```

## 4. 影响分析

### 4.1 需要修改的文件

| 文件路径 | 修改类型 | 修改内容 |
|----------|----------|----------|
| `application/service/system_service.h` | 删除 | 移除 3 个事件定义 |
| `application/service/system_service.c` | 删除 | 移除 3 个事件处理分支 |
| `middleware/communication/wireless_callbacks.c:202-203` | 替换 | `SYSTEM_DEEP_SLEEP_EVT` → `SYSTEM_LPM_DEEP_REQ_EVT` |
| `drivers/communication/bluetooth/ch584/hidkbd.c:1038-1040` | 替换 | `SYSTEM_DEEP_SLEEP_EVT` → `SYSTEM_LPM_DEEP_REQ_EVT` |
| `drivers/communication/bluetooth/ch584/hidkbd.h:71` | 删除 | 移除 `SYSTEM_DEEP_SLEEP_EVT_TIMEOUT` 定义 |

### 4.2 不受影响的功能

| 功能 | 原因 |
|------|------|
| 低电关机 | 使用 `SYSTEM_LOW_BATTERY_SHUTDOWN_EVT`，不受影响 |
| 系统关机 | 使用 `SYSTEM_SHUTDOWN_EVT`，不受影响 |
| 恢复出厂 | 使用 `SYSTEM_FACTORY_RESET_EVT`，不受影响 |
| OTA 升级 | 使用 `SYSTEM_OTA_EVT`，不受影响 |
| 存储操作 | 使用 `SYSTEM_STORAGE_EVT`，不受影响 |
| LPM 睡眠/唤醒 | 使用 LPM 事件族，功能增强 |

### 4.3 风险评估

| 风险 | 等级 | 缓解措施 |
|------|------|----------|
| 遗漏调用点 | 低 | 全局搜索已确认所有使用点 |
| 时序变化 | 低 | `SYSTEM_LPM_DEEP_REQ_EVT` 与原事件行为一致 |
| 外部依赖 | 无 | 所有调用点均在项目内部 |

## 5. 实施计划

### Phase 1: 更新调用点

1. 修改 `wireless_callbacks.c:202` - 替换事件
2. 修改 `hidkbd.c:1038-1040` - 替换事件
3. 删除 `hidkbd.h:71` 的超时定义

### Phase 2: 清理事件定义

1. 删除 `system_service.h` 中的 3 个事件
2. 删除 `system_service.c` 中的 3 个事件处理分支

### Phase 3: 验证

1. 编译验证
2. 功能测试（睡眠/唤醒流程）

## 6. 优化收益

### 6.1 量化收益

| 指标 | 优化前 | 优化后 | 改善 |
|------|--------|--------|------|
| 系统事件数量 | 15 | 12 | -3 (20%↓) |
| 僵尸代码行数 | ~35 | 0 | -35 |
| 事件语义清晰度 | 混乱 | 统一 | ✓ |
| 事件位利用率 | 15/16 | 12/16 | 释放 3 bits |

### 6.2 架构收益

1. **单一睡眠模型**：所有睡眠/唤醒通过 LPM 状态机管理
2. **清晰的事件语义**：`LPM_*` 事件明确表示 LPM 子系统操作
3. **扩展空间**：释放的 bits (1, 3, 5) 可用于未来功能

## 7. 结论

本次优化通过删除 3 个冗余事件（`SYSTEM_IDLE_EVT`、`SYSTEM_DEEP_SLEEP_EVT`、`SYSTEM_WAKEUP_EVT`），简化了系统事件架构。所有睡眠/唤醒操作统一通过 LPM 事件族管理，架构更加清晰，且为未来扩展预留了 3 个事件位。

**建议实施**：按照 Phase 1-3 的顺序执行，预计修改量小，风险可控。

---

*文档结束*
