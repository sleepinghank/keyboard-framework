# 日志规范化设计

**日期**: 2026-03-30
**状态**: 待审核
**范围**: application/、middleware/、drivers/ 业务代码（不含第三方组件）

## 1. 问题分析

当前日志系统存在以下问题：

1. **打印方式混杂**: dprintf(137次)、printf(262次)、println(24次)、xprintf(28次)、LOG()(11次) 并存
2. **格式不统一**: 有的用 `[COMMU_STS]` 前缀，有的用 `[EVENT_MANAGER]`，有的无前缀
3. **冗余日志过多**: 调试期间的临时打印未清理，矩阵扫描、防抖状态等高频打印影响性能
4. **级别控制分散**: PRINTF_LEVEL 和 NO_DEBUG/NO_PRINT 两套机制并存

## 2. 设计方案

采用**轻量级宏封装**，在现有 `debug.h` 基础上新增带级别和模块前缀的宏，复用底层 `xprintf` 输出。

### 2.1 日志级别

```c
#define LOG_LEVEL_NONE    0   // 关闭所有日志
#define LOG_LEVEL_ERROR   1   // 仅错误
#define LOG_LEVEL_WARN    2   // 错误 + 警告（默认）
#define LOG_LEVEL_INFO    3   // 错误 + 警告 + 信息
#define LOG_LEVEL_DEBUG   4   // 全部日志
```

默认级别为 `LOG_LEVEL_WARN`，由 `config_product.h` 中的 `PRINTF_LEVEL` 映射。

### 2.2 日志宏

```c
// 以下宏定义在 debug.h 中新增，与现有 dprintf 等宏并存

#if LOG_LEVEL >= LOG_LEVEL_ERROR
#define LOG_E(fmt, ...) xprintf("[E] " fmt "\r\n", ##__VA_ARGS__)
#else
#define LOG_E(fmt, ...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_WARN
#define LOG_W(fmt, ...) xprintf("[W] " fmt "\r\n", ##__VA_ARGS__)
#else
#define LOG_W(fmt, ...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_INFO
#define LOG_I(fmt, ...) xprintf("[I] " fmt "\r\n", ##__VA_ARGS__)
#else
#define LOG_I(fmt, ...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_DEBUG
#define LOG_D(fmt, ...) xprintf("[D] " fmt "\r\n", ##__VA_ARGS__)
#else
#define LOG_D(fmt, ...)
#endif
```

### 2.3 使用规范

- 级别前缀 `[E]`/`[W]`/`[I]`/`[D]` 由宏自动添加
- 模块前缀 `[COMMU]`/`[BT]`/`[MATRIX]` 等由调用者在 fmt 字符串中手动添加
- 换行 `\r\n` 由宏自动添加，调用者不需要手动写

```c
// 正确用法
LOG_W("[COMMU] state=%d evt=%d", state, evt);
LOG_E("[BT] send failed, err=%d", err);
LOG_I("[COMMU] connected, host=%d", host);

// 错误用法
LOG_W("[COMMU] state=%d\n", state);  // 不需要手动加换行
```

### 2.4 模块前缀命名约定

| 层级 | 模块 | 前缀 |
|------|------|------|
| application/service | communication_service | `[COMMU]` |
| application/service | input_service | `[INPUT]` |
| application/service | output_service | `[OUTPUT]` |
| application/service | system_service | `[SYS]` |
| application/system | system_init | `[SYS]` |
| middleware/communication | wireless | `[WL]` |
| middleware/communication | transport | `[TRANS]` |
| middleware/communication | host | `[HOST]` |
| middleware/communication | report_buffer | `[RBUF]` |
| middleware/communication | wireless_callbacks | `[WL_CB]` |
| middleware/communication | PMU | `[PMU]` |
| middleware/keyboard | keyboard | `[KB]` |
| middleware/keyboard | report | `[RPT]` |
| middleware/keyboard | keymap | `[KMAP]` |
| middleware/keyboard/combo | kb_combo_engine | `[COMBO]` |
| middleware/keyboard/combo | kb_fn_action | `[FN]` |
| middleware/keyboard/combo | kb_sys_action | `[SYS_ACT]` |
| middleware/touchpad | touchpad | `[TP]` |
| drivers/input/keyboard | matrix | `[MATRIX]` |
| drivers/input/keyboard | debounce | `[DEBOUNCE]` |
| drivers/communication/bluetooth | _bt_driver | `[BT]` |
| drivers/communication/bluetooth | hidkbd | `[HID]` |
| drivers/power | battery | `[BAT]` |
| drivers/storage | storage | `[STOR]` |
| drivers/output/backlight | backlight_hal | `[BL]` |

## 3. 清理规则

### 3.1 保留标准

| 级别 | 保留标准 | 示例 |
|------|---------|------|
| ERROR | 硬件故障、通信失败、不可恢复错误 | `LOG_E("[BT] connection lost, reason=%d", reason)` |
| WARN | 异常但可恢复的情况、状态机异常跳转 | `LOG_W("[COMMU] unexpected state=%d", state)` |
| INFO | 仅关键状态变更：连接/断连、模式切换 | `LOG_I("[COMMU] connected, host=%d", host)` |
| DEBUG | 全部删除 | - |

### 3.2 删除规则

1. **删除所有 DEBUG 类 dprintf**: 矩阵扫描详情、防抖中间状态、按键码打印、HID 报告内容等
2. **删除所有 println 调用**: 调试用的分隔线/标记，无实际信息
3. **删除 printf 直接调用**: 迁移到 LOG_* 宏或删除
4. **删除 LOG() 系列调用**: 统一迁移到新宏
5. **保留的错误/警告类 dprintf**: 迁移为 LOG_E/LOG_W

### 3.3 不动的文件

- `drivers/output/logging/` (日志基础设施)
- `test/` (测试代码)
- `project/ch584m/StdPeriphDriver/` (厂商 SDK)
- `component/` (第三方组件)
- `drivers/communication/bluetooth/test/` (测试代码)

## 4. 预期效果

| 模块 | 当前日志数 | 清理后 |
|------|-----------|--------|
| application/service/ | ~85 | ~15 |
| middleware/communication/ | ~55 | ~10 |
| middleware/keyboard/ | ~45 | ~8 |
| drivers/ (业务部分) | ~70 | ~12 |
| **合计** | **~250** | **~45** |

## 5. config_product.h 日志配置调整

### 5.1 当前问题

```
PRINTF_ENABLE      TRUE           // 总开关
  ├─ DEBUG_DEBOUNCE           TRUE     // 防抖调试开关（独立，与级别无关）
  ├─ PRINTF_LEVEL             PRINTF_LEVEL_DEBUG   // 平台 LOG 系统级别
  ├─ LOG_COLOR_ENABLE         TRUE     // 颜色
  ├─ LOG_FMT_USING_DIR_ENABLE TRUE     // 文件路径
  ├─ LOG_FMT_USING_LINE_ENABLE TRUE    // 行号
  └─ ...
```

问题：
1. `DEBUG_DEBOUNCE` 是独立开关，绕过了日志级别控制
2. `PRINTF_LEVEL`（枚举 0-5）和新增的 `LOG_LEVEL`（宏 0-4）两套级别体系并存
3. `LOG_COLOR_ENABLE`、`LOG_FMT_USING_*` 等属于 CH584 平台 LOG 系统的格式化配置，与业务日志宏无关，但混在一起容易混淆
4. 缺少业务日志级别的统一入口

### 5.2 调整方案

将 `config_product.h` 中的日志配置区分为**两层**：

```c
/* ---------- 日志配置 ---------- */
#define PRINTF_ENABLE      TRUE

#if (PRINTF_ENABLE == TRUE)
    /* --- 业务日志级别（LOG_E/LOG_W/LOG_I/LOG_D 宏使用）--- */
    #define LOG_LEVEL           LOG_LEVEL_WARN

    /* --- 平台日志输出配置（CH584 LOG 系统底层）--- */
    #define PRINTF_LEVEL              PRINTF_LEVEL_WARN
    #define PRINTF_UART               PLATFORM_UART_3
    #define PRINTF_BAUDRATE           115200
    #define LOG_COLOR_ENABLE          FALSE
    #define LOG_FMT_USING_DIR_ENABLE  FALSE
    #define LOG_FMT_USING_LINE_ENABLE FALSE
    #define LOG_FMT_USING_FUNC_ENABLE FALSE
    #define LOG_LINE_BUF_SIZE         256
    #define LOG_NEWLINE_SIGN          "\r\n"
    #define LOG_LINE_NUM_MAX_LEN      4
    #define LOG_FILTER_TAG_MAX_LEN    20

    #if (CHIP_TYPE == CHIP_CH584M)
        #ifndef __HIGH_CODE
        #define __HIGH_CODE  __attribute__((section(".highcode")))
        #endif
    #else
        #define __HIGH_CODE
    #endif
#endif
```

### 5.3 变更明细

| 配置项 | 变更前 | 变更后 | 原因 |
|--------|--------|--------|------|
| `DEBUG_DEBOUNCE` | `TRUE` | **删除** | 由 LOG_LEVEL 统一控制，不再需要独立开关 |
| `LOG_LEVEL` | 无 | **新增** `LOG_LEVEL_WARN` | 业务日志级别统一入口 |
| `PRINTF_LEVEL` | `PRINTF_LEVEL_DEBUG` | `PRINTF_LEVEL_WARN` | 与 LOG_LEVEL 对齐，产品发布时不应为 DEBUG |
| `LOG_COLOR_ENABLE` | `TRUE` | `FALSE` | 串口终端不一定支持颜色，精简输出 |
| `LOG_FMT_USING_DIR_ENABLE` | `TRUE` | `FALSE` | 业务日志宏已自带模块前缀，无需文件路径 |
| `LOG_FMT_USING_LINE_ENABLE` | `TRUE` | `FALSE` | 同上，行号由 LOG_D 等宏控制即可 |
| `LOG_LINE_BUF_SIZE` | `1024` | `256` | 精简后单条日志不超过 100 字节，256 足够 |

### 5.4 LOG_LEVEL 与 PRINTF_LEVEL 的关系

两者职责不同：
- **`LOG_LEVEL`**：控制业务日志宏 `LOG_E/LOG_W/LOG_I/LOG_D` 的输出（在 `debug.h` 中）
- **`PRINTF_LEVEL`**：控制 CH584 平台 LOG 系统的底层输出（在 `log.h` 中，影响 `LOG()`/`LOG_DEBUG()`/`LOG_ERROR()` 等宏）

两者应保持一致。`LOG_LEVEL` 的值直接定义在 `config_product.h` 中，`PRINTF_LEVEL` 保持现有枚举值不变。

## 6. 实施步骤

1. 在 `debug.h` 中新增 LOG_E/LOG_W/LOG_I/LOG_D 宏定义及 LOG_LEVEL 常量
2. 在 `config_product.h` 中添加 LOG_LEVEL 配置，调整日志相关设置
3. 逐模块清理 application/service/ 日志
4. 逐模块清理 middleware/communication/ 日志
5. 逐模块清理 middleware/keyboard/ 日志
6. 逐模块清理 drivers/ 业务日志
7. 编译验证
