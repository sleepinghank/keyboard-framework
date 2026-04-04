# 系统时钟基准统一实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将 CH584 产品配置中的系统时钟源、MRS 工程编译宏中的 `FREQ_SYS`、SDK 编译期主频与自有 HAL 时间换算统一到同一份系统时钟基准，修复矩阵扫描 `5ms` 被实际执行为 `6.25ms` 的时钟失配问题。

**Architecture:** 以 `keyboards/kb904/config_product.h` 为产品级系统时钟源配置入口，只定义 `SYSTEM_CLOCK_SOURCE` 并对编译参数中的 `FREQ_SYS` 做一致性校验；`hal/` 内所有主频换算只使用 `FREQ_SYS`；MRS 工程通过 `.cproject` 与 `keyboard-framework.wvproj` 全局注入 `-DFREQ_SYS=...`，让 SDK 与自有代码共享同一份主频。全程不修改 `project/ch584m` 下的 SDK 源文件，只修改工程配置文件和仓库自有代码。

**Tech Stack:** C11、CH584/WCH SDK、MounRiver Studio 工程配置（`.cproject` / `.wvproj`）、WCH RISC-V GCC、`/wch-riscv-build`

---

## 文件结构与职责

| 路径 | 处理方式 | 职责 |
|------|----------|------|
| `keyboards/kb904/config_product.h` | 修改 | 定义产品级 `SYSTEM_CLOCK_SOURCE`，并对编译参数中的 `FREQ_SYS` 做一致性校验 |
| `hal/platforms/ch584/_system_hal.c` | 修改 | 让 `SetSysClock(...)` 只依赖 `SYSTEM_CLOCK_SOURCE` |
| `hal/platforms/ch584/_hw_timer.c` | 修改 | 删除本地主频常量，全部改用 `FREQ_SYS` 做 tick 换算 |
| `hal/hw_timer.h` | 修改 | 修正文档注释，删除“78MHz 固定前提”描述 |
| `application/service/system_service.c` | 视需要修改 | 先引入产品配置头，再接触 `CH58x_common.h` |
| `component/ota_component/Inateck_tag.c` | 视需要修改 | 同上 |
| `component/ota_component/Inateck_tag.h` | 视需要修改 | 同上 |
| `drivers/communication/bluetooth/ch584/_bt_driver.c` | 视需要修改 | 同上 |
| `drivers/ota/Inateck_tag_config.h` | 视需要修改 | 同上 |
| `hal/platforms/ch584/_adc.c` | 视需要修改 | 同上 |
| `hal/platforms/ch584/_gpio.c` | 视需要修改 | 同上 |
| `hal/platforms/ch584/_i2c_master.c` | 视需要修改 | 同上 |
| `hal/platforms/ch584/_power.c` | 视需要修改 | 同上 |
| `hal/platforms/ch584/_pwm.c` | 视需要修改 | 同上，并确保继续使用 `FREQ_SYS` |
| `hal/platforms/ch584/_spi_master.c` | 视需要修改 | 同上 |
| `hal/platforms/ch584/_uart.c` | 视需要修改 | 同上 |
| `hal/platforms/ch584/_watchdog.c` | 视需要修改 | 同上 |
| `hal/wait.h` | 视需要修改 | 保证 `mDelaymS/mDelayuS` 所在头在产品配置链路下使用 |
| `middleware/communication/PMU.c` | 视需要修改 | 先引入产品配置头，再接触 `CH58x_common.h` |
| `project/ch584m/.cproject` | 修改 | 为 C/C++ 编译配置注入全局 `FREQ_SYS` 宏 |
| `project/ch584m/keyboard-framework.wvproj` | 修改 | 为 MRS 项目定义同步注入 `FREQ_SYS` |

## 约束

- 不修改 `project/ch584m/StdPeriphDriver/*`
- 不修改 `project/ch584m/platforms_HAL/*`
- 不手工修改 `project/ch584m/obj/*.mk`
- 所有代码注释与新增文档使用简体中文
- HAL/驱动层新增或修改的系统主频换算统一使用 `FREQ_SYS`

### Task 1: 在产品配置中建立统一时钟源与主频校验

**Files:**
- Modify: `keyboards/kb904/config_product.h`

- [ ] **Step 1: 在产品配置中定义 `SYSTEM_CLOCK_SOURCE`**

将 `config_product.h` 中芯片配置区扩展为：

```c
/* ---------- 系统时钟配置 ---------- */
#ifndef SYSTEM_CLOCK_SOURCE
#define SYSTEM_CLOCK_SOURCE    CLK_SOURCE_HSE_PLL_62_4MHz
#endif
```

- [ ] **Step 2: 在文件末尾加入对编译参数 `FREQ_SYS` 的存在性和一致性校验**

在 `#include "config_check.h"` 之前加入最小可用的编译期校验：

```c
#if (CHIP_TYPE == CHIP_CH584M)
    #ifndef FREQ_SYS
        #error "CH584 工程必须在编译参数中定义 FREQ_SYS"
    #endif
#endif

#if (SYSTEM_CLOCK_SOURCE == CLK_SOURCE_HSE_PLL_62_4MHz)
    #if (FREQ_SYS != 62400000UL)
        #error "SYSTEM_CLOCK_SOURCE 与 FREQ_SYS 不匹配"
    #endif
#elif (SYSTEM_CLOCK_SOURCE == CLK_SOURCE_HSE_PLL_78MHz)
    #if (FREQ_SYS != 78000000UL)
        #error "SYSTEM_CLOCK_SOURCE 与 FREQ_SYS 不匹配"
    #endif
#else
    #error "当前 SYSTEM_CLOCK_SOURCE 尚未建立 FREQ_SYS 映射"
#endif
```

- [ ] **Step 3: 自查 `config_product.h` 中的时钟源与校验入口**

确认以下事实成立：

- `SYSTEM_CLOCK_SOURCE` 位于产品级配置区
- `config_product.h` 不再自行定义 `FREQ_SYS`
- `config_product.h` 会对缺失或错误的 `FREQ_SYS` 直接编译失败

Run: `rg -n "SYSTEM_CLOCK_SOURCE|FREQ_SYS|CH58x_common.h" keyboards/kb904/config_product.h`

Expected: 可以看到 `SYSTEM_CLOCK_SOURCE` 定义和 `FREQ_SYS` 校验，但看不到 `#define FREQ_SYS ...`

- [ ] **Step 4: 提交本任务**

```bash
git add keyboards/kb904/config_product.h
git commit -m "refactor(clock): define system clock source and validate FREQ_SYS"
```

### Task 2: 将 HAL 主频换算统一到 FREQ_SYS

**Files:**
- Modify: `hal/platforms/ch584/_system_hal.c`
- Modify: `hal/platforms/ch584/_hw_timer.c`
- Modify: `hal/hw_timer.h`

- [ ] **Step 1: 让 `SetSysClock(...)` 只依赖 `SYSTEM_CLOCK_SOURCE`**

将 `system_hal_init()` 内的硬编码替换为：

```c
SetSysClock(SYSTEM_CLOCK_SOURCE);
```

不要在 `_system_hal.c` 中保留任何 `CLK_SOURCE_HSE_PLL_62_4MHz` 字面量。

- [ ] **Step 2: 删除 `_hw_timer.c` 的本地主频常量**

删除以下局部定义：

```c
#define FREQ_SYS_HZ         78000000UL
#define MS_TO_TICKS(ms)     ((uint32_t)(ms) * (FREQ_SYS_HZ / 1000))
#define MAX_DIRECT_MS       (MAX_HW_TIMER_TICKS / (FREQ_SYS_HZ / 1000))
```

改为：

```c
#define MS_TO_TICKS(ms)     ((uint32_t)(ms) * (FREQ_SYS / 1000))
#define MAX_DIRECT_MS       (MAX_HW_TIMER_TICKS / (FREQ_SYS / 1000))
```

- [ ] **Step 3: 修正 `hal/hw_timer.h` 的注释和上限说明**

把“78MHz 固定前提”的描述改成“基于当前 `FREQ_SYS` 配置计算”，例如：

```c
// 硬件定时器最大定时周期 (毫秒)
// 26位计数器，上限由 FREQ_SYS 决定
#define HW_TIMER_MAX_MS         (67108864UL / (FREQ_SYS / 1000))
```

如果头文件宏不适合直接改成表达式，至少同步修正文档注释，避免继续误导为“固定 78MHz”。

- [ ] **Step 4: 搜索 HAL 中是否仍残留系统主频硬编码**

Run: `rg -n "\\bFREQ_SYS_HZ\\b|\\b62400000UL\\b|\\b78000000UL\\b" hal application drivers middleware component keyboards`

Expected: 结果中不再包含自有代码里的系统主频硬编码；若还有残留，先补到当前任务一并清理

- [ ] **Step 5: 提交本任务**

```bash
git add hal/platforms/ch584/_system_hal.c hal/platforms/ch584/_hw_timer.c hal/hw_timer.h
git commit -m "fix(clock): unify ch584 hal timing math on FREQ_SYS"
```

### Task 3: 收口自有代码的 `config_product.h` / `CH58x_common.h` 包含顺序

**Files:**
- Modify: `application/service/system_service.c`
- Modify: `component/ota_component/Inateck_tag.c`
- Modify: `component/ota_component/Inateck_tag.h`
- Modify: `drivers/communication/bluetooth/ch584/_bt_driver.c`
- Modify: `drivers/ota/Inateck_tag_config.h`
- Modify: `hal/platforms/ch584/_adc.c`
- Modify: `hal/platforms/ch584/_gpio.c`
- Modify: `hal/platforms/ch584/_hw_timer.c`
- Modify: `hal/platforms/ch584/_i2c_master.c`
- Modify: `hal/platforms/ch584/_power.c`
- Modify: `hal/platforms/ch584/_pwm.c`
- Modify: `hal/platforms/ch584/_spi_master.c`
- Modify: `hal/platforms/ch584/_system_hal.c`
- Modify: `hal/platforms/ch584/_uart.c`
- Modify: `hal/platforms/ch584/_watchdog.c`
- Modify: `hal/wait.h`
- Modify: `middleware/communication/PMU.c`

- [ ] **Step 1: 先处理 `.c` 文件的直接包含顺序**

对所有直接 `#include "CH58x_common.h"` 且未先包含产品配置头的 `.c` 文件，统一调整为：

```c
#include "kb904/config_product.h"
#include "CH58x_common.h"
```

如果该文件已经通过别的头间接拿到产品配置，但文件头仍先直接包含 `CH58x_common.h`，仍要调整为显式先包含 `config_product.h`，减少隐式依赖。

- [ ] **Step 2: 处理头文件与公共头的顺序**

重点检查：

- `component/ota_component/Inateck_tag.h`
- `drivers/ota/Inateck_tag_config.h`
- `hal/wait.h`

若头文件直接依赖 `CH58x_common.h`，应保证在不形成循环依赖的前提下先引入 `kb904/config_product.h`。

`hal/wait.h` 目标形式：

```c
#include "kb904/config_product.h"
#include "CH58x_common.h"
```

- [ ] **Step 3: 跑一次包含顺序回归搜索**

Run: `rg -n '#include "CH58x_common\\.h"|#include <CH58x_common\\.h>' application drivers middleware hal keyboards component test`

Expected: 所有自有源码中，凡直接包含 `CH58x_common.h` 的文件，都已经在文件头先显式包含 `kb904/config_product.h`，或者有明确的注释说明该文件不能这样做且原因充分

- [ ] **Step 4: 提交本任务**

```bash
git add application/service/system_service.c component/ota_component/Inateck_tag.c component/ota_component/Inateck_tag.h drivers/communication/bluetooth/ch584/_bt_driver.c drivers/ota/Inateck_tag_config.h hal/platforms/ch584/_adc.c hal/platforms/ch584/_gpio.c hal/platforms/ch584/_hw_timer.c hal/platforms/ch584/_i2c_master.c hal/platforms/ch584/_power.c hal/platforms/ch584/_pwm.c hal/platforms/ch584/_spi_master.c hal/platforms/ch584/_system_hal.c hal/platforms/ch584/_uart.c hal/platforms/ch584/_watchdog.c hal/wait.h middleware/communication/PMU.c
git commit -m "refactor(clock): normalize product config include order"
```

### Task 4: 在 MRS 工程配置中全局注入 FREQ_SYS

**Files:**
- Modify: `project/ch584m/.cproject`
- Modify: `project/ch584m/keyboard-framework.wvproj`

- [ ] **Step 1: 在 `.cproject` 的编译器宏列表中加入 `FREQ_SYS`**

定位现有 `Defined symbols (-D)` 配置块，当前已有：

```xml
<listOptionValue builtIn="false" value="CLK_OSC32K=1"/>
<listOptionValue builtIn="false" value="DCDC_ENABLE=1"/>
<listOptionValue builtIn="false" value="HAL_SLEEP=1"/>
<listOptionValue builtIn="false" value="BLE_SNV_NUM=2"/>
```

在同一列表中加入：

```xml
<listOptionValue builtIn="false" value="FREQ_SYS=62400000"/>
```

注意：

- `.cproject` 中存在重复配置块，两个 C 编译配置都要同步更新
- 若 C++ 宏列表也参与扫描配置，也同步补上 `FREQ_SYS=62400000`

- [ ] **Step 2: 在 `keyboard-framework.wvproj` 的 `defined_symbols` 数组中加入 `FREQ_SYS`**

将：

```json
"defined_symbols": [
    "CLK_OSC32K=1",
    "DCDC_ENABLE=1",
    "HAL_SLEEP=1",
    "BLE_SNV_NUM=2"
]
```

改为：

```json
"defined_symbols": [
    "CLK_OSC32K=1",
    "DCDC_ENABLE=1",
    "HAL_SLEEP=1",
    "BLE_SNV_NUM=2",
    "FREQ_SYS=62400000"
]
```

- [ ] **Step 3: 清理旧产物后验证生成的编译命令**

先清理构建目录，再重新执行标准构建，确认新的编译命令中已经出现 `-DFREQ_SYS=62400000`。

Run: `/wch-riscv-build`

Expected:

- 编译命令中可见 `-DFREQ_SYS=62400000`
- `StdPeriphDriver/CH58x_sys.c` 等 SDK 编译单元也带上该宏

- [ ] **Step 4: 提交本任务**

```bash
git add project/ch584m/.cproject project/ch584m/keyboard-framework.wvproj
git commit -m "build(clock): inject FREQ_SYS into ch584 project settings"
```

### Task 5: 完成验证并收尾

**Files:**
- Verify: `keyboards/kb904/config_product.h`
- Verify: `hal/platforms/ch584/_system_hal.c`
- Verify: `hal/platforms/ch584/_hw_timer.c`
- Verify: `hal/hw_timer.h`
- Verify: `project/ch584m/.cproject`
- Verify: `project/ch584m/keyboard-framework.wvproj`

- [ ] **Step 1: 执行静态搜索确认无遗漏**

Run: `rg -n "\\bFREQ_SYS_HZ\\b|\\b78000000UL\\b|SetSysClock\\(\\s*CLK_SOURCE_|SYSTEM_CLOCK_SOURCE|FREQ_SYS" hal application drivers middleware keyboards component project/ch584m`

Expected:

- 自有代码中不再有 `FREQ_SYS_HZ`
- `_system_hal.c` 中不再有 `SetSysClock(CLK_SOURCE_...)` 硬编码
- `.cproject` / `.wvproj` 中能搜到 `FREQ_SYS=62400000`

- [ ] **Step 2: 执行完整编译验证**

Run: `/wch-riscv-build`

Expected:

- 编译通过
- 无 `FREQ_SYS` 重定义告警/错误
- 无 “CH584 工程必须在编译参数中定义 FREQ_SYS” 错误
- 无 `SYSTEM_CLOCK_SOURCE` / `FREQ_SYS` 不匹配触发的 `#error`

- [ ] **Step 3: 执行板级验证**

1. 下载固件到 CH584 板
2. 保持 `MATRIX_SCAN_INTERVAL_MS=5`
3. 逻辑分析仪抓取矩阵扫描定时器中断或扫描触发 GPIO

Expected:

- 周期接近 `5ms`
- 不再出现固定 `6.25ms`

- [ ] **Step 4: 记录验证结果并提交**

若验证通过，提交本轮改动：

```bash
git add keyboards/kb904/config_product.h hal/platforms/ch584/_system_hal.c hal/platforms/ch584/_hw_timer.c hal/hw_timer.h application/service/system_service.c component/ota_component/Inateck_tag.c component/ota_component/Inateck_tag.h drivers/communication/bluetooth/ch584/_bt_driver.c drivers/ota/Inateck_tag_config.h hal/platforms/ch584/_adc.c hal/platforms/ch584/_gpio.c hal/platforms/ch584/_i2c_master.c hal/platforms/ch584/_power.c hal/platforms/ch584/_pwm.c hal/platforms/ch584/_spi_master.c hal/platforms/ch584/_uart.c hal/platforms/ch584/_watchdog.c hal/wait.h middleware/communication/PMU.c project/ch584m/.cproject project/ch584m/keyboard-framework.wvproj
git commit -m "refactor(clock): unify system clock baseline across product and build"
```

- [ ] **Step 5: 若构建命令中仍缺少 `-DFREQ_SYS`，先回到 Task 4 排查工程配置入口**

排查顺序：

1. 确认 `.cproject` 中所有活跃配置块都已加入 `FREQ_SYS`
2. 确认 `keyboard-framework.wvproj` 的 `defined_symbols` 已同步
3. 清理旧构建产物后再次执行 `/wch-riscv-build`
4. 只有在确认工程配置仍被其他源覆盖时，才扩展排查是否需要同步调整 `project/ch584m/.template`
