# GPIO 中断管理重构 — 设计文档

> 日期：2026-03-21
> 状态：设计完成
> 需求文档：`DOCS/plans/2026-03-21-gpio-interrupt-management-requirements.md`

## 1. 需求摘要

将 GPIO 中断管理下沉到 HAL 层，提供简洁统一的回调接口，屏蔽芯片内部中断处理细节。

### 核心决策

| # | 决策项 | 结论 |
|---|--------|------|
| 1 | ISR 放置位置 | `hal/platforms/ch584/_gpio.c` |
| 2 | 回调函数签名 | `void (*gpio_int_callback_t)(pin_t pin)` — 带参，支持多 pin 共享回调 |
| 3 | 批量接口 | HAL 层提供 `gpio_enable_interrupt_batch` + `gpio_disable_interrupt_batch` |
| 4 | 唤醒源识别 | 不需要，唤醒后统一全矩阵扫描 |
| 5 | 回调执行上下文 | ISR 中直接调用回调 |
| 6 | PFIC/唤醒源配置 | 封装在 HAL 内部自动管理 |
| 7 | pin 存储方式 | 回调槽 + 位掩码，非逐 pin 数组 |

## 2. 现状分析

### 2.1 当前调用链

```
睡眠准备:
  input_service.c (INPUT_LPM_PREPARE_EVT)
    -> matrix_prepare_wakeup()
      -> GPIOA_ITModeCfg() / GPIOB_ITModeCfg()  -- 直接调芯片库
      -> PFIC_EnableIRQ()
      -> PWR_PeriphWakeUpCfg()

中断触发:
  input_service.c: GPIOA_IRQHandler()
    -> 读 R16_PA_INT_IF
    -> 锁存唤醒源
    -> OSAL_SetEvent(SYSTEM_LPM_WAKE_EVT)

唤醒恢复:
  input_service.c (INPUT_LPM_RESUME_EVT)
    -> matrix_resume_from_sleep()
      -> R16_PA_INT_EN &= ~mask  -- 直接操作寄存器
```

### 2.2 问题

1. CH584 平台 `_gpio.c` 未实现 `gpio_enable_interrupt` / `gpio_disable_interrupt`
2. `matrix.c` 绕过 HAL 直接调用芯片库函数
3. ISR 定义在 application 层 `input_service.c`

## 3. 架构设计

```
+-----------------------------------------------------------+
|  上层调用方                                                |
|  matrix.c        -> gpio_enable/disable_interrupt_batch   |
|  touchpad_service -> gpio_enable/disable_interrupt        |
|  input_service   -> 只处理事件，不再定义 ISR              |
+-----------------------------------------------------------+
|  HAL 接口层 (hal/gpio.h)                                  |
|  声明: enable/disable_interrupt + batch 变体              |
|  类型: gpio_int_callback_t(pin_t pin)                     |
+-----------------------------------------------------------+
|  HAL 平台实现 (hal/platforms/ch584/_gpio.c)               |
|  +-- 回调槽表 s_porta_slots / s_portb_slots ----------+  |
|  |  slot 0: pin_mask=0x003C  cb=wakeup_cb             |  |
|  |  slot 1: pin_mask=0x0800  cb=touch_cb              |  |
|  |  slot 2: 空                                        |  |
|  |  slot 3: 空                                        |  |
|  +----------------------------------------------------+  |
|  ISR: GPIOA_IRQHandler / GPIOB_IRQHandler                 |
|    -> 遍历槽 -> 按位匹配 -> 调用回调                      |
|  内部自动管理: PFIC 使能/禁用 + GPIO 唤醒源配置           |
+-----------------------------------------------------------+
```

### 目标调用链

```
睡眠准备:
  input_service.c (INPUT_LPM_PREPARE_EVT)
    -> matrix_prepare_wakeup()
      -> gpio_enable_interrupt_batch(row_pins, count, FALLING, matrix_wakeup_cb)
    -> gpio_enable_interrupt(PA2, FALLING, pa2_wakeup_cb)

中断触发:
  _gpio.c: GPIOA_IRQHandler()
    -> 遍历槽 -> GPIOA_ReadITFlagBit(slot.pin_mask)
    -> GPIOA_ClearITFlagBit(hit)
    -> 逐位调用 callback(PAL_LINE(PORTA, bit_index))

唤醒恢复:
  input_service.c (INPUT_LPM_RESUME_EVT)
    -> matrix_resume_from_sleep()
      -> gpio_disable_interrupt_batch(row_pins, count)
    -> gpio_disable_interrupt(PA2)
```

## 4. 接口设计

### 4.1 hal/gpio.h 变更

```c
// 回调类型变更（破坏性）
// 旧: typedef void (*gpio_int_callback_t)(void);
typedef void (*gpio_int_callback_t)(pin_t pin);

// 已有接口（签名不变，仅回调类型跟随变更）
void gpio_enable_interrupt(pin_t pin, gpio_int_trigger_t trigger,
                           gpio_int_callback_t callback);
void gpio_disable_interrupt(pin_t pin);

// 新增批量接口
bool gpio_enable_interrupt_batch(const pin_t *pins, uint8_t count,
                                 gpio_int_trigger_t trigger,
                                 gpio_int_callback_t callback);
void gpio_disable_interrupt_batch(const pin_t *pins, uint8_t count);
```

- `gpio_enable_interrupt_batch` 返回 `bool`：全部成功返回 `true`，失败回滚返回 `false`
- `gpio_disable_interrupt_batch` 无返回值：注销操作不会失败
- `pins` 参数加 `const`，HAL 不修改调用方数组

### 4.2 _gpio.c 内部数据结构

```c
#ifndef GPIO_INT_MAX_SLOTS
#define GPIO_INT_MAX_SLOTS  4  // 可在平台头文件中覆盖
#endif

typedef struct {
    uint32_t            pin_mask;   // 位掩码，0 表示空槽
    gpio_int_callback_t callback;   // 共享回调
    gpio_int_trigger_t  trigger;    // 触发类型
} gpio_int_slot_t;

static gpio_int_slot_t s_porta_slots[GPIO_INT_MAX_SLOTS];
static gpio_int_slot_t s_portb_slots[GPIO_INT_MAX_SLOTS];
```

### 4.3 内部辅助函数（static 私有）

| 函数 | 职责 |
|------|------|
| `_find_slot(slots, callback)` | 查找已有相同回调的槽，或返回空槽索引；返回 -1 表示无可用槽 |
| `_config_interrupt(port, mask, trigger)` | 调用 `GPIOA/B_ITModeCfg` 配置中断模式 + `GPIOA/B_ClearITFlagBit` 清标志 + `PFIC_ClearPendingIRQ` + `PFIC_EnableIRQ` + `PWR_PeriphWakeUpCfg` |
| `_disable_interrupt(port, mask)` | `R16_PA/PB_INT_EN &= ~mask`（无官方API） + `GPIOA/B_ClearITFlagBit` 清标志 + 判断所有槽是否为空，若空则 `PFIC_DisableIRQ` |
| `_gpio_irq_dispatch(slots, port)` | ISR 内调用：遍历槽 -> `ReadITFlagBit(slot.pin_mask)` -> `ClearITFlagBit(hit)` -> 逐位调用 `callback(PAL_LINE(port, bit))` |

### 4.4 CH58x 官方 API 映射

| HAL 操作 | CH58x API | 备注 |
|----------|-----------|------|
| 配置中断+使能 | `GPIOA_ITModeCfg(mask, mode)` | 内部自动 `INT_EN |= mask` |
| 读中断标志 | `GPIOA_ReadITFlagBit(mask)` | 宏 |
| 清中断标志 | `GPIOA_ClearITFlagBit(mask)` | 宏 |
| 关闭中断 | `R16_PA_INT_EN &= ~mask` | 无官方API，封装在 `_disable_interrupt` 中 |
| PB22/23 | `ReadITFlagBit` / `ClearITFlagBit` 宏已内置 `>>14` 处理 | disable 时也需同步处理 |

## 5. 关键设计细节

### 5.1 注册时重叠检测

`gpio_enable_interrupt` 注册前遍历所有槽，若 pin 已存在于其他槽的 `pin_mask` 中，先从旧槽移除（清对应位），再加入新槽。避免同一 pin 被两个槽同时匹配导致重复触发。

### 5.2 临界区保护

所有槽表读写操作（注册、注销、PFIC 管理）在 `__disable_irq()` / `__enable_irq()` 临界区内完成，防止 ISR 执行期间槽表被修改的竞态。

### 5.3 PFIC 自动管理

- **使能时机**：`_config_interrupt` 内部，配置完中断模式后自动 `PFIC_EnableIRQ`
- **禁用时机**：`_disable_interrupt` 内部，当该 port 所有槽的 `pin_mask` 全为 0 时才 `PFIC_DisableIRQ`
- 避免一个 pin 注销导致同 port 其他 pin 中断失效

### 5.4 批量接口原子语义

`gpio_enable_interrupt_batch` 内部：
1. 将 pins 数组按 port 分组，合并为 porta_mask / portb_mask
2. 查找可用槽（同回调复用同一槽）
3. 若无可用槽，返回 `false`（不做任何配置）
4. 全部槽位就绪后，一次性调用 `_config_interrupt` 配置
5. 失败时回滚已注册的槽位

### 5.5 低电平触发使用约定

低电平触发（`GPIO_INT_LOW_LEVEL`）在电平保持期间会持续产生中断。调用方**必须**在回调内主动调用 `gpio_disable_interrupt` 自锁，处理完成后再重新使能。此约定在 `hal/gpio.h` 接口注释中明确。

### 5.6 ISR 分发逻辑

```
GPIOA_IRQHandler:
  遍历 s_porta_slots:
    if slot.pin_mask == 0: continue
    hit = GPIOA_ReadITFlagBit(slot.pin_mask)
    if hit:
      GPIOA_ClearITFlagBit(hit)
      逐位遍历 hit:
        callback(PAL_LINE(PORTA, bit_index))
```

同一次中断可能多个 pin 同时触发，ISR 遍历所有槽逐位判断，确保不漏。

## 6. 改动文件清单

| 文件 | 改动类型 | 具体内容 |
|------|---------|---------|
| `hal/gpio.h` | 修改 | 回调类型改签 + 新增 batch 声明 + 使用约定注释 |
| `hal/platforms/ch584/_gpio.c` | 新增 | 回调槽表 + 4 个私有函数 + `gpio_enable/disable_interrupt` 实现 + batch 实现 + ISR |
| `hal/platforms/test/_gpio.c` | 修改 | 适配新签名空实现 |
| `component/touch_component/touchpad_service.c` | 修改 | `_touch_cb` 签名改为 `void _touch_cb(pin_t pin)` |
| `drivers/input/keyboard/matrix.c` | 修改 | `prepare_wakeup` / `resume_from_sleep` 改用 HAL 接口，新增 `matrix_wakeup_cb` |
| `application/service/input_service.c` | 删除+修改 | 删除 ISR，PA2 改用 HAL 接口，清理唤醒源锁存 |

## 7. 实施计划

### 执行顺序

```
S1 -> S2 --> S5 -> S6 -> S7
  +-> S3
  +-> S4
```

S2/S3/S4 可并行执行。

### 步骤详情

| 步骤 | 任务 | 具体内容 | 完成标准 | 前置依赖 |
|------|------|---------|---------|---------|
| S1 | 修改 `hal/gpio.h` | 1. 回调类型改为 `void(*)(pin_t)` 2. 新增 batch 声明 3. 增加使用约定注释 | 编译通过（允许 warning） | 无 |
| S2 | 实现 CH584 `_gpio.c` 中断 | 1. 定义槽结构体和静态表 2. 实现 4 个私有函数 3. 实现 enable/disable（含重叠检测、临界区） 4. 实现 batch（原子语义） 5. 实现 ISR | CH584 编译通过 | S1 |
| S3 | 适配 test `_gpio.c` | 新签名空实现 | test 编译通过 | S1 |
| S4 | 适配 `touchpad_service.c` | `_touch_cb` 签名加 `pin_t pin` 参数 | warning 消除 | S1 |
| S5 | 重构 `matrix.c` | 1. `prepare_wakeup` 改用 `gpio_enable_interrupt_batch` 2. `resume_from_sleep` 改用 `gpio_disable_interrupt_batch` 3. 新增 `matrix_wakeup_cb` 4. 移除芯片库直接调用 | 编译通过，无芯片库直调 | S1, S2 |
| S6 | 清理 `input_service.c` | 1. 删除 ISR 2. PA2 改用 HAL 接口 3. 清理唤醒源锁存代码 | 链接通过，无重复符号 | S2, S5 |
| S7 | 硬件验证 | 1. 睡眠->按键唤醒->矩阵扫描 2. 睡眠->PA2 唤醒 3. 触控板中断 4. 连续睡眠/唤醒循环 5. 唤醒后中断重配置专项 | 全部通过 | S1-S6 |

## 8. 评审要点回顾

| 维度 | 问题 | 应对 |
|------|------|------|
| 功能完整性 | disable 时清标志防误触发 | `_disable_interrupt` 内部自动 `ClearITFlagBit` |
| 功能完整性 | 低电平触发持续重入 | 接口注释约定调用方须自锁 |
| 技术可行性 | pin_mask 重叠 | 注册时检测并移除旧槽 |
| 技术可行性 | PFIC 竞态 | 临界区保护 |
| 可维护性 | 槽容量跨平台 | `GPIO_INT_MAX_SLOTS` 可在平台头文件覆盖 |
| 可测试性 | resume 路径 | 硬件验证补充专项 |
| 风险 | ISR 中调用 OSAL_SetEvent | 当前代码已在 ISR 中使用，确认中断安全 |
