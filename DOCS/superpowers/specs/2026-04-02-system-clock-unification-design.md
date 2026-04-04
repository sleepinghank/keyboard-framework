# 系统时钟基准统一设计

**日期**: 2026-04-02
**状态**: 待审核
**范围**: `keyboards/`、`hal/`、`application/`、`drivers/`、`middleware/`、`component/` 的自有代码，以及 `project/ch584m/` 下的工程构建配置文件（不修改 SDK 源码）

## 1. 背景与问题

当前项目的系统时钟配置存在“设置值”和“换算值”脱钩的问题。

已确认的现状如下：

1. 系统初始化在 `hal/platforms/ch584/_system_hal.c` 中调用：
   `SetSysClock(CLK_SOURCE_HSE_PLL_62_4MHz)`
2. CH584 SDK 头文件 `CH58x_common.h` 中默认：
   `FREQ_SYS = 62400000`
3. 但 `hal/platforms/ch584/_hw_timer.c` 内部单独写死：
   `FREQ_SYS_HZ = 78000000UL`
4. 结果导致 `5ms` 的矩阵扫描周期被换算成 `390000 tick`，在实际 `62.4MHz` 下运行成 `6.25ms`

这类问题说明当前工程缺少一份真正的“单一事实源”来统一：

- `SetSysClock(...)` 的系统时钟源
- SDK 与 HAL 中的 `FREQ_SYS`
- 所有以主频为基准的时间换算

另外，当前项目存在两个实现风险：

1. 多个非 SDK 文件直接包含 `CH58x_common.h`，如果未先经过产品配置头，可能看不到产品级 `FREQ_SYS`
2. SDK 源文件（如 `CH58x_sys.c`）是独立编译单元，仅靠 `config_product.h` 无法影响其编译期 `FREQ_SYS`，必须通过工程构建宏全局注入

## 2. 目标

本次设计目标如下：

1. 在 `keyboards/kb904/config_product.h` 中定义统一的产品级系统时钟源配置：
   - `SYSTEM_CLOCK_SOURCE`
2. 通过工程编译参数全局注入 `FREQ_SYS`
3. 将 `SetSysClock(...)` 与编译参数中的 `FREQ_SYS` 绑定为同一组系统时钟配置
4. 项目自有 HAL 计算统一使用 `FREQ_SYS`，不再保留任何局部系统主频常量
5. 让 SDK 编译单元与项目自有代码共享同一份 `FREQ_SYS`
6. 增加编译期一致性校验，防止再次出现“切频值”和“换算值”不匹配

## 3. 非目标

本次设计明确不做以下事情：

1. 不修改 `project/ch584m/StdPeriphDriver/`、`project/ch584m/platforms_HAL/` 等 SDK 源码
2. 不引入运行时 `GetSysClock()` 作为 HAL 标准换算路径
3. 不改动 OSAL/TMOS 定时框架设计
4. 不手工修改 `project/ch584m/obj/*.mk` 等自动生成文件
5. 不借本次统一时钟基准顺带做无关重构

## 4. 设计原则

### 4.1 单一事实源

系统主频相关配置只能来自产品配置层，禁止在模块内再次定义“本地真相”。

### 4.2 编译期优先

CH584 项目大量时间换算和 SDK 逻辑依赖编译期常量，本次统一基准以编译期宏 `FREQ_SYS` 为核心。

### 4.3 不修改 SDK 源码

所有与 SDK 的统一，均通过产品配置和工程构建配置完成，不直接编辑 SDK 文件。

### 4.4 最小功能改动

只修正系统主频配置与时间换算链路，不改变业务行为和模块边界。

## 5. 方案总览

采用“产品配置定义时钟源 + 工程级宏注入 `FREQ_SYS` + 自有代码统一替换”的方案。

### 5.1 产品配置层统一定义时钟源

在 `keyboards/kb904/config_product.h` 中新增并固定定义：

```c
#define SYSTEM_CLOCK_SOURCE    CLK_SOURCE_HSE_PLL_62_4MHz
```

要求：

1. `SYSTEM_CLOCK_SOURCE` 作为 `SetSysClock(...)` 的唯一来源
2. `FREQ_SYS` 不在 `config_product.h` 中定义，而是由工程编译参数全局注入
3. 后续产品如果切换主频，需要同时修改产品配置中的 `SYSTEM_CLOCK_SOURCE` 与工程编译参数中的 `FREQ_SYS`

### 5.2 HAL 计算统一使用 FREQ_SYS

项目自有代码中，凡是基于系统主频做时间换算的逻辑，统一改为使用 `FREQ_SYS`。

示例：

```c
#define MS_TO_TICKS(ms)    ((uint32_t)(ms) * (FREQ_SYS / 1000))
```

禁止继续存在以下写法：

```c
#define FREQ_SYS_HZ        78000000UL
#define TIMER_CLK_HZ       62400000UL
```

### 5.3 构建系统全局注入 FREQ_SYS

由于 SDK 源文件不会经过 `config_product.h`，仅靠头文件包含顺序无法保证 SDK 侧看到正确的 `FREQ_SYS`。

因此必须在 CH584 工程配置中全局加入：

```text
-DFREQ_SYS=<产品配置值>
```

实施边界：

1. 修改 `project/ch584m/.cproject` 中 C/C++ 编译器的 `Defined symbols`
2. 修改 `project/ch584m/keyboard-framework.wvproj` 中的 `defined_symbols`
3. 不手改 `project/ch584m/obj/*.mk`
4. 若工程模板链路会覆盖构建配置，再评估是否同步更新模板文件；但设计基线以 `.cproject` 与 `.wvproj` 为准

这样可以保证：

- SDK 的 `CH58x_sys.c`、`CH58x_uart*.c` 等编译单元
- 项目自有 HAL/驱动/应用编译单元

都使用同一份 `FREQ_SYS`

## 6. 详细设计

### 6.1 config_product.h 的职责调整

`keyboards/kb904/config_product.h` 将成为系统时钟基准的唯一产品级定义入口。

新增职责：

1. 定义 `SYSTEM_CLOCK_SOURCE`
2. 消费工程编译参数注入的 `FREQ_SYS`
3. 提供主频一致性校验宏

推荐放置位置：

- 位于 `CHIP_TYPE` 定义之后
- 位于 `#include "CH58x_common.h"` 之前或之后均可，但建议靠近芯片配置区定义时钟源，并在校验区直接使用编译器注入的 `FREQ_SYS`

### 6.2 SetSysClock 与产品配置绑定

`hal/platforms/ch584/_system_hal.c` 中现有硬编码：

```c
SetSysClock(CLK_SOURCE_HSE_PLL_62_4MHz);
```

应改为：

```c
SetSysClock(SYSTEM_CLOCK_SOURCE);
```

要求：

1. `system_hal_init()` 内不得再出现具体频率枚举字面量
2. 如果后续切换主频，只改 `config_product.h`

### 6.3 HAL 换算统一规则

本次统一范围包括所有自有 HAL/驱动层中基于系统主频的换算逻辑。

统一规则：

1. 毫秒转 tick：使用 `FREQ_SYS / 1000`
2. 微秒转 tick：使用 `FREQ_SYS / 1000000`
3. 最大周期推导：使用 `FREQ_SYS`
4. PWM 等外设时钟推导：使用 `FREQ_SYS`

本次已明确需要修正的功能点：

1. `hal/platforms/ch584/_hw_timer.c`
2. `hal/hw_timer.h` 中关于“78MHz”前提的注释与公式说明

本次需要全项目搜索确认的范围：

- `application/`
- `drivers/`
- `middleware/`
- `hal/`
- `keyboards/`
- `component/`
- `test/`

搜索目标：

- `FREQ_SYS_HZ`
- `62400000`
- `78000000`
- 其他局部系统主频常量

### 6.4 非 SDK 文件的包含顺序规范

为了避免项目自有源码在包含 `config_product.h` 之前先拿到 SDK 默认主频回退值，增加如下规范：

1. 自有代码如果需要 CH584 SDK 头文件，优先先包含 `kb904/config_product.h`
2. 不再允许在产品相关源码中直接以 `CH58x_common.h` 作为第一系统配置入口
3. 平台相关公共头若依赖 `FREQ_SYS`，应确保经过产品配置头后再触达 SDK 头文件

这条规则的目的不是替代工程级 `-DFREQ_SYS`，而是保证自有代码和产品配置保持一致，避免局部文件绕过产品配置层。

### 6.5 编译期一致性校验

在 `config_product.h` 末尾新增时钟一致性校验。

首先要求 CH584 工程必须通过编译参数提供 `FREQ_SYS`：

```c
#if (CHIP_TYPE == CHIP_CH584M)
    #ifndef FREQ_SYS
        #error "CH584 工程必须在编译参数中定义 FREQ_SYS"
    #endif
#endif
```

第一阶段至少覆盖当前可能使用的 HSE PLL 档位。

示例规则：

```c
#if (SYSTEM_CLOCK_SOURCE == CLK_SOURCE_HSE_PLL_62_4MHz)
    #if (FREQ_SYS != 62400000UL)
        #error "SYSTEM_CLOCK_SOURCE 与 FREQ_SYS 不匹配"
    #endif
#elif (SYSTEM_CLOCK_SOURCE == CLK_SOURCE_HSE_PLL_78MHz)
    #if (FREQ_SYS != 78000000UL)
        #error "SYSTEM_CLOCK_SOURCE 与 FREQ_SYS 不匹配"
    #endif
#endif
```

后续可按项目需要扩展到：

- `52MHz`
- `39MHz`
- `26MHz`
- `24MHz`
- `19.5MHz`
- `13MHz`

校验目标：

1. 防止 `SetSysClock` 与 `FREQ_SYS` 脱钩
2. 防止构建系统 `-DFREQ_SYS` 配置错误
3. 在编译期尽早失败，而不是运行后才暴露时间偏差

### 6.6 对 SDK 的统一方式

本次不修改 SDK 源码，但要让 SDK 与产品配置保持一致。

统一方式如下：

1. SDK 继续保留原始源码不动
2. 通过工程级 `-DFREQ_SYS=...` 覆盖 SDK 默认值
3. 通过产品配置中的一致性校验保证工程设置与产品配置一致

这样，SDK 中依赖 `FREQ_SYS` 的逻辑，例如：

- `CH58x_sys.c` 的 `mDelayuS()` / `mDelaymS()`
- UART 波特率换算
- 其他 SDK 编译期分支

都可自动与产品主频保持一致。

## 7. 影响文件范围

### 7.1 必改文件

- `keyboards/kb904/config_product.h`
- `hal/platforms/ch584/_system_hal.c`
- `hal/platforms/ch584/_hw_timer.c`
- `hal/hw_timer.h`
- `project/ch584m/.cproject`
- `project/ch584m/keyboard-framework.wvproj`

### 7.2 视搜索结果决定是否调整的自有代码文件

以下目录下若存在直接依赖 `CH58x_common.h`、且可能绕过产品配置层的文件，需要统一包含顺序：

- `application/service/`
- `drivers/communication/bluetooth/ch584/`
- `drivers/ota/`
- `drivers/system/`
- `hal/platforms/ch584/`
- `middleware/communication/`
- `component/ota_component/`
- `hal/wait.h`

### 7.3 明确不改文件

- `project/ch584m/StdPeriphDriver/*`
- `project/ch584m/platforms_HAL/*`
- `project/ch584m/obj/*`

## 8. 风险与对策

### 8.1 风险：工程级宏与产品配置不一致

如果 `.cproject` / `.wvproj` 中的 `-DFREQ_SYS` 与 `config_product.h` 不一致，SDK 与自有代码仍可能分裂。

对策：

- 在 `config_product.h` 中加入编译期一致性校验
- 在 CH584 工程中强制要求 `FREQ_SYS` 来自编译参数

### 8.2 风险：自动生成文件覆盖构建配置

如果某些构建流程会重写 `obj/*.mk`，手工改生成文件会丢失。

对策：

- 只修改工程源配置文件，不修改自动生成的 `obj/*.mk`

### 8.3 风险：局部文件继续使用硬编码主频

即使主路径修复，后续新增模块仍可能再次写入 `62400000UL`、`78000000UL`。

对策：

- 本次实现后，全项目搜索并清理硬编码
- 将“HAL 时间换算统一使用 `FREQ_SYS`”写入实现说明和代码评审标准

## 9. 验证方案

### 9.1 静态检查

检查以下项：

1. `SetSysClock(...)` 是否只使用 `SYSTEM_CLOCK_SOURCE`
2. 自有代码中是否已删除 `FREQ_SYS_HZ`
3. 自有代码中是否不存在系统主频硬编码字面量
4. `config_product.h` 是否对缺失的 `FREQ_SYS` 做出编译期阻断
5. 工程构建配置中是否存在全局 `-DFREQ_SYS=...`

### 9.2 编译验证

必须执行：

```bash
/wch-riscv-build
```

期望：

1. SDK 与项目自有代码都可通过编译
2. 无 `FREQ_SYS` 相关重定义错误
3. 无 `SYSTEM_CLOCK_SOURCE` / `FREQ_SYS` 不匹配的编译错误

### 9.3 板级验证

重点验证矩阵扫描定时器：

1. `MATRIX_SCAN_INTERVAL_MS = 5`
2. 逻辑分析仪抓取定时中断或扫描触发引脚
3. 实测周期应接近 `5ms`

期望：

- 不再出现 `6.25ms` 这种固定 25% 比例误差

### 9.4 回归关注项

重点关注以下依赖系统主频的路径：

1. `hw_timer`
2. PWM
3. 串口初始化
4. `wait_us` / `wait_ms`
5. 任何 `FREQ_SYS / N` 的外设时钟分频逻辑

## 10. 实施清单

- [ ] 在 `config_product.h` 中新增 `SYSTEM_CLOCK_SOURCE`
- [ ] 在 `config_product.h` 中新增对编译参数 `FREQ_SYS` 的存在性与一致性校验
- [ ] 将 `SetSysClock(...)` 改为使用 `SYSTEM_CLOCK_SOURCE`
- [ ] 将自有 HAL 时间换算统一改为 `FREQ_SYS`
- [ ] 清理自有代码中的局部主频常量
- [ ] 为自有代码补充必要的包含顺序调整
- [ ] 在 `.cproject` 与 `.wvproj` 中注入全局 `FREQ_SYS`
- [ ] 增加 `SYSTEM_CLOCK_SOURCE` 与 `FREQ_SYS` 的编译期一致性校验
- [ ] 执行全项目搜索确认无遗漏硬编码
- [ ] 执行 `/wch-riscv-build`
- [ ] 板级验证矩阵扫描周期恢复正确

## 11. 结论

本设计以 `config_product.h` 为产品级系统时钟源入口，以工程编译参数中的 `FREQ_SYS` 为全项目统一时间换算基准，通过工程级编译宏注入覆盖 SDK 默认值，在不修改 SDK 源码的前提下，统一 `SetSysClock`、SDK 延时逻辑与 HAL 时间换算。

该方案能直接消除当前矩阵扫描 `5ms -> 6.25ms` 的时钟失配问题，并为后续切换系统主频提供单点配置能力。
