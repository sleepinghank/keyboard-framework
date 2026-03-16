# timer_read32 计时准确性修复设计（最小改动）

## 1. 背景与目标

### 1.1 背景
- 当前 `timer_read32()` 使用 RTC 周期计数换算毫秒，关键实现位于 `drivers/system/timer.c`。
- 现网 CH584 工程配置为 `CLK_OSC32K=1`（内部 32k，实际按 32000Hz 使用），但 `timer.c` 写死 `32768Hz` 换算。
- 当前溢出处理按 `UINT32_MAX` 近似处理，而 CH584 RTC 计数器上限是 `RTC_MAX_COUNT=0xA8C00000`，两者不一致。
- 系统启动路径里 `timer_init()` 目前被注释，导致计时基线未显式初始化。

### 1.2 目标
- 在不改变接口、不改模块分层的前提下，修复 `timer_read32()` 计时精度与回绕行为。
- 继续保持 `timer.h` 现有 API：`timer_init/timer_read32/timer_elapsed32`。
- 修改范围最小化，优先只动 `drivers/system/timer.c` 与初始化调用点。

### 1.3 非目标
- 不重构 OSAL/TMOS 时间系统。
- 不改变业务层使用方式。
- 不引入新的驱动层抽象接口。

---

## 2. 现状问题（代码级）

### 2.1 频率常量硬编码错误
- `drivers/system/timer.c` 当前使用：
  - `#define RTC_FREQUENCY 32768UL`
- 与 CH584 工程配置 `CLK_OSC32K=1`（内部 32000Hz）不一致，产生持续系统性慢计时（约 2.34375%）。

### 2.2 回绕处理基准错误
- 当前代码使用 `OVERFLOW_ADJUST_CYCLES = floor(UINT32_MAX / RTC_FREQUENCY) * RTC_FREQUENCY`。
- CH584 的 RTC 计数器真实模数是 `RTC_MAX_COUNT (0xA8C00000)`，非 `2^32`。
- 导致跨 RTC 回绕时出现大幅跳变风险。

### 2.3 初始化路径缺失
- `application/system/system_init.c` 中 `timer_init()` 被注释。
- 会导致计时起点依赖静态初值，不具备显式、可控的启动基线。

---

## 3. 约束与设计原则

- 维持架构边界：Application -> Middleware -> Drivers -> HAL。
- 不改变公开头文件接口和调用关系。
- 不修改业务模块逻辑，仅修正计时实现与初始化时序。
- 兼容现有 `timer_elapsed32()` 的差值语义。

---

## 4. 修复方案（最小改动）

## 4.1 文件改动范围
- `drivers/system/timer.c`
- `application/system/system_init.c`

不改动其他文件。

## 4.2 `timer.c` 设计调整

### 4.2.1 RTC 频率选择改为配置驱动
- 将 `RTC_FREQUENCY` 从硬编码改为条件编译：
  - `CLK_OSC32K==1` -> `32000`
  - 其他 -> `32768`
- 目的是与 CH584 工程实际时钟源保持一致，消除持续比例误差。

### 4.2.2 计数回绕按 RTC 真实模数处理
- 使用 CH584 RTC 真实模数（`0xA8C00000`）作为环形计数器上限。
- 计算环形差值时使用：
  - 若 `now >= base`，`diff = now - base`
  - 否则 `diff = (RTC_COUNTER_MOD - base) + now`
- 不再使用 `UINT32_MAX` 相关近似修正值。

### 4.2.3 保留现有读数模型，修正回绕累计
- 保留当前 `cycles_offset + last_cycles + ms_offset` 思路以减少变更。
- 将“检测到回绕后”的累计补偿改为 `RTC_WRAP_MS = CYCLES_TO_MS(RTC_COUNTER_MOD)`。
- 这样每次 RTC 回绕只增加一次“真实一圈”的毫秒量，避免小时级跳变。

### 4.2.4 原子访问策略保持不变
- 继续使用现有 `ATOMIC_BLOCK_FORCEON` 包裹 `timer_read32()` 核心读改写逻辑。
- 本次不改 `atomic_util` 实现，避免扩大改动面。

## 4.3 初始化流程调整

- 在 `application/system/system_init.c` 的 driver init 阶段恢复 `timer_init()` 调用。
- 放置原则：在系统时钟/RTC 可用后执行，保证基线取值有效。

---

## 5. 关键伪代码

```c
// timer_clear()
cycles_offset = get_rtc_cycles();
last_cycles   = 0;
ms_offset     = 0;

// timer_read32()
cycles = get_rtc_cycles();
cycles_elapsed = rtc_diff_mod(cycles, cycles_offset, RTC_COUNTER_MOD);

if (cycles_elapsed < last_cycles) {
    ms_offset += RTC_WRAP_MS;
}
last_cycles = cycles_elapsed;

return CYCLES_TO_MS(cycles_elapsed) + ms_offset;
```

说明：
- `rtc_diff_mod()` 基于 RTC 真实模数，不依赖 `2^32` 回绕。
- `ms_offset` 在每次“相对计数回卷”时累加一圈毫秒值。

---

## 6. 影响评估

### 6.1 正向影响
- 修复持续慢计时问题（CH584 `CLK_OSC32K=1` 场景）。
- 修复 RTC 回绕后大跳变问题。
- `timer_elapsed32()`、上层超时逻辑无需改动。

### 6.2 风险点
- 若某些构建配置未定义 `CLK_OSC32K`，需有合理默认值（默认 32768）。
- 若系统超过一个 RTC 周期（约 24h）都不调用 `timer_read32()`，环形差值理论上不可区分多次回绕；该限制与现有设计一致，不在本次扩大处理。

---

## 7. 验证方案

## 7.1 静态检查
- 检查 `timer.c` 中不再出现 `UINT32_MAX` 参与回绕补偿。
- 检查 `RTC_FREQUENCY` 与 `CLK_OSC32K` 条件分支一致。
- 检查 `system_init.c` 中 `timer_init()` 恢复调用。

## 7.2 功能验证（软件侧）
- 用可控 RTC 序列验证：
  - 单调递增场景：`timer_read32()` 单调不减。
  - 跨回绕场景：前后差值连续，无小时级突跳。
  - `timer_elapsed32(last)` 与手工差值一致。

## 7.3 板级验证（CH584）
- 测试项 A：60 秒真实时间窗口
  - 记录 `t0=timer_read32()`，约 60 秒后记录 `t1`。
  - 期望 `t1 - t0` 接近 60000ms（误差进入晶振/校准允许范围）。
- 测试项 B：回绕边界
  - 将 RTC 设置到接近上限后运行，观察跨界前后 `timer_read32()` 连续性。
  - 期望无秒级以上异常跳变。

## 7.4 回归验证
- 关注模块：
  - `middleware/communication/report_buffer.c`
  - `middleware/communication/lpm.c`
  - `middleware/communication/transport.c`
  - `drivers/output/indicators/indicator.c`
- 验证这些依赖 `timer_read32()` 的路径行为不回退。

---

## 8. 回滚策略

- 若出现兼容性问题，可仅回退两处提交：
  - `drivers/system/timer.c`
  - `application/system/system_init.c`
- 回滚后恢复原行为，不影响其他模块接口。

---

## 9. 实施清单（编码前）

- [ ] 评审通过本设计文档
- [ ] 确认 `timer.c` 条件编译分支方案
- [ ] 确认 `timer_init()` 恢复调用位置
- [ ] 执行代码修改与验证

