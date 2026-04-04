# PRINTF_ENABLE 单开关日志控制 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 让 `PRINTF_ENABLE` 成为业务日志唯一公开开关；当其为 `FALSE` 时，`dprintf/println/xprintf/LOG_*` 全部编译为空，同时要求工程去掉 `DEBUG`；当其为 `TRUE` 时，再由编译参数 `-DDEBUG=Debug_UARTx` 提供 WCH SDK 的底层 `printf/_write` UART 后端。

**Architecture:** 这次不再通过 `NO_DEBUG`、`NO_PRINT` 做二次转义，而是在 [print.h](/D:/Code/VScode/keyboard-framework/drivers/output/logging/print.h) 和 [debug.h](/D:/Code/VScode/keyboard-framework/drivers/output/logging/debug.h) 里直接判断 `PRINTF_ENABLE`。`DEBUG` 的职责被收敛为“底层串口输出后端选择”，不再参与业务日志语义；业务日志是否生成代码，只由 `PRINTF_ENABLE` 决定，日志级别细分则继续由 `LOG_LEVEL` 负责。

**Tech Stack:** C11, 预处理器宏, CH584 WCH RISC-V 固件, MounRiver Studio 工程, PowerShell 构建和反汇编验证

---

## 设计约束

- 公开配置只保留一个业务日志总开关：`PRINTF_ENABLE`
- 不再新增或依赖 `NO_DEBUG`、`NO_PRINT` 作为外部配置入口
- `LOG_LEVEL` 不是总开关，只在 `PRINTF_ENABLE=TRUE` 时决定输出级别
- 编译参数 `DEBUG` 仅用于启用 WCH SDK 中的 `_write()` 和选择底层 UART，不再承担业务日志开关职责
- 推荐把工程参数从 `-DDEBUG=1` 改成可读性更高的 `-DDEBUG=Debug_UART1`
- `PRINTF_ENABLE=TRUE` 时，`DEBUG` 必须存在；`PRINTF_ENABLE=FALSE` 时，`DEBUG` 必须移除
- `PRINTF_UART` 与 `DEBUG` 必须指向同编号 UART；本次先做“存在性约束 + 文档约束”，不在业务层引入新的映射宏

---

## Task 1: 用 `PRINTF_ENABLE` 直接收口 `print.h`

**Files:**
- Modify: `drivers/output/logging/print.h`
- Verify: `application/main.c`
- Verify: `application/system/system_init.c`
- Verify: `middleware/communication/wireless.h`

- [ ] **Step 1: 记录当前失败基线**

先在当前代码上执行一次正式固件编译，并确认启动路径仍可能落入 `puts/iprintf`：

```powershell
cmd.exe /c "cd /d D:\Code\VScode\keyboard-framework\project\ch584m\obj && set PATH=D:\Software\Work\MounRiver_Studio2\resources\app\resources\win32\components\WCH\Toolchain\RISC-V Embedded GCC12\bin;D:\Software\Work\MounRiver_Studio2\resources\app\resources\win32\others\Build_Tools\Make\bin;%PATH% && make.exe main-build"

$objdump = 'D:\Software\Work\MounRiver_Studio2\resources\app\resources\win32\components\WCH\Toolchain\RISC-V Embedded GCC12\bin\riscv-wch-elf-objdump.exe'
& $objdump -d --disassemble=system_init_coordinator 'D:\Code\VScode\keyboard-framework\project\ch584m\obj\keyboard-framework.elf'
& $objdump -d --disassemble=main 'D:\Code\VScode\keyboard-framework\project\ch584m\obj\keyboard-framework.elf'
```

Expected:
- `system_init_coordinator` 中还能看到 `puts` 或等价输出调用
- `main` 中还能看到初始化成功/失败分支的打印调用

- [ ] **Step 2: 删除 `NO_PRINT` 作为控制入口，直接判断 `PRINTF_ENABLE`**

把 [print.h](/D:/Code/VScode/keyboard-framework/drivers/output/logging/print.h) 的主分支从：

```c
#ifndef NO_PRINT
...
#else
...
#endif
```

改成：

```c
#if (PRINTF_ENABLE == TRUE)
...
#else
...
#endif
```

要求：
- `print/println/xprintf/uprint/uprintln/uprintf` 的对外接口名保持不变
- `PRINTF_ENABLE=FALSE` 时这些宏全部展开为空
- 不再新增 `NO_PRINT` 桥接代码
- `PRINTF_ENABLE` 未定义的情况继续交由 [config_check.h](/D:/Code/VScode/keyboard-framework/keyboards/config_check.h) 报错

- [ ] **Step 3: 确认业务打印路径都已收口到 `print.h` / `debug.h`**

执行：

```powershell
rg -n "dprintf\(|dprint\(|println\(|xprintf\(|LOG_[EWID]\(|kc_printf" D:\Code\VScode\keyboard-framework\application D:\Code\VScode\keyboard-framework\middleware D:\Code\VScode\keyboard-framework\drivers D:\Code\VScode\keyboard-framework\component
```

Expected:
- 业务侧打印主要通过 `dprintf/println/xprintf/LOG_*/kc_printf`
- 无需逐文件删除日志调用点

- [ ] **Step 4: Commit**

```bash
git add drivers/output/logging/print.h
git commit -m "refactor(logging): gate print macros with PRINTF_ENABLE"
```

---

## Task 2: 用 `PRINTF_ENABLE` 直接收口 `debug.h`

**Files:**
- Modify: `drivers/output/logging/debug.h`
- Verify: `middleware/communication/wireless.h`

- [ ] **Step 1: 删除 `NO_DEBUG` 作为控制入口**

把 [debug.h](/D:/Code/VScode/keyboard-framework/drivers/output/logging/debug.h) 的主分支从：

```c
#ifndef NO_DEBUG
...
#else
...
#endif
```

改成：

```c
#if (PRINTF_ENABLE == TRUE)
...
#else
...
#endif
```

目标：
- `dprint/dprintln/dprintf/debug*` 在 `PRINTF_ENABLE=FALSE` 时全部编译为空
- [wireless.h](/D:/Code/VScode/keyboard-framework/middleware/communication/wireless.h) 中 `kc_printf -> dprintf` 的映射自然跟随关闭

- [ ] **Step 2: 让 `LOG_*` 直接跟随 `PRINTF_ENABLE` 和 `LOG_LEVEL`**

把 [debug.h](/D:/Code/VScode/keyboard-framework/drivers/output/logging/debug.h) 末尾的 `LOG_*` 宏条件改为以下模式：

```c
#if (PRINTF_ENABLE == TRUE) && (LOG_LEVEL >= LOG_LEVEL_ERROR)
#define LOG_E(fmt, ...) xprintf("[E] " fmt "\r\n", ##__VA_ARGS__)
#else
#define LOG_E(fmt, ...)
#endif
```

`LOG_W/LOG_I/LOG_D` 同理处理。

要求：
- `PRINTF_ENABLE=FALSE` 时，即使 `LOG_LEVEL` 默认值仍为 `LOG_LEVEL_WARN`，也不得生成任何输出代码
- `PRINTF_ENABLE=TRUE` 时，继续保留 `LOG_LEVEL` 的分级语义

- [ ] **Step 3: 搜索确认旧控制入口已从正式代码退出**

执行：

```powershell
rg -n "NO_DEBUG|NO_PRINT" D:\Code\VScode\keyboard-framework\drivers D:\Code\VScode\keyboard-framework\application D:\Code\VScode\keyboard-framework\middleware D:\Code\VScode\keyboard-framework\keyboards
```

Expected:
- 正式代码路径中不再依赖 `NO_DEBUG` / `NO_PRINT`
- 若仅剩历史文档或示例代码引用，可不作为本次实现阻塞项

- [ ] **Step 4: Commit**

```bash
git add drivers/output/logging/debug.h
git commit -m "refactor(logging): gate debug macros with PRINTF_ENABLE"
```

---

## Task 3: 明确 `PRINTF_ENABLE` 与编译参数 `DEBUG` 的配合规则

**Files:**
- Modify: `keyboards/config_check.h`
- Verify: `keyboards/kb904/config_product.h`
- Verify: `project/ch584m/.cproject`
- Verify: `project/ch584m/obj/application/system/subdir.mk`

- [ ] **Step 1: 在 `config_check.h` 中增加 `PRINTF_ENABLE` 与 `DEBUG` 的存在性约束**

在 [config_check.h](/D:/Code/VScode/keyboard-framework/keyboards/config_check.h) 中新增：

```c
#if (PRINTF_ENABLE == TRUE)
    #ifndef DEBUG
        #error "PRINTF_ENABLE=TRUE requires -DDEBUG=Debug_UARTx"
    #endif
#else
    #ifdef DEBUG
        #error "PRINTF_ENABLE=FALSE must not define DEBUG"
    #endif
#endif
```

要求：
- 不在这里引入新的 UART 映射宏
- 这里只校验“该有就有、该无就无”
- `DEBUG` 的具体值采用符号写法 `Debug_UART0..3`

- [ ] **Step 2: 统一编译参数写法**

检查工程配置 [project/ch584m/.cproject](/D:/Code/VScode/keyboard-framework/project/ch584m/.cproject)：

- 当 [config_product.h](/D:/Code/VScode/keyboard-framework/keyboards/kb904/config_product.h) 中 `PRINTF_ENABLE=FALSE` 时，删除 `-DDEBUG=...`
- 当 `PRINTF_ENABLE=TRUE` 且 `PRINTF_UART=PLATFORM_UART_1` 时，使用 `-DDEBUG=Debug_UART1`

说明：
- 不再推荐使用 `-DDEBUG=1`
- 统一使用 `Debug_UARTx` 形式，避免“裸数字”可读性差

- [ ] **Step 3: 验证生成的 makefile 已同步工程配置**

执行：

```powershell
rg -n --fixed-strings "DEBUG=" D:\Code\VScode\keyboard-framework\project\ch584m\.cproject
Get-ChildItem -Recurse -File 'D:\Code\VScode\keyboard-framework\project\ch584m\obj' | Select-String -Pattern " -DDEBUG=" | Select-Object Path, LineNumber, Line
```

Expected:
- `PRINTF_ENABLE=FALSE` 场景下，`.cproject` 与 `obj/**/*.mk` 都不再包含 `-DDEBUG=...`
- `PRINTF_ENABLE=TRUE` 场景下，生成结果使用 `-DDEBUG=Debug_UARTx`

- [ ] **Step 4: Commit**

```bash
git add keyboards/config_check.h
git commit -m "build(logging): validate DEBUG usage against PRINTF_ENABLE"
```

---

## Task 4: 构建与反汇编验证关闭态安全

**Files:**
- Verify: `application/main.c`
- Verify: `application/system/system_init.c`
- Verify: `project/ch584m/obj/keyboard-framework.elf`

- [ ] **Step 1: 执行正式固件编译**

Run:

```powershell
cmd.exe /c "cd /d D:\Code\VScode\keyboard-framework\project\ch584m\obj && set PATH=D:\Software\Work\MounRiver_Studio2\resources\app\resources\win32\components\WCH\Toolchain\RISC-V Embedded GCC12\bin;D:\Software\Work\MounRiver_Studio2\resources\app\resources\win32\others\Build_Tools\Make\bin;%PATH% && make.exe main-build"
```

Expected:
- 编译成功
- 生成 `project/ch584m/obj/keyboard-framework.elf`

- [ ] **Step 2: 反汇编确认启动路径不再调用输出函数**

Run:

```powershell
$objdump = 'D:\Software\Work\MounRiver_Studio2\resources\app\resources\win32\components\WCH\Toolchain\RISC-V Embedded GCC12\bin\riscv-wch-elf-objdump.exe'
& $objdump -d --disassemble=system_init_coordinator 'D:\Code\VScode\keyboard-framework\project\ch584m\obj\keyboard-framework.elf'
& $objdump -d --disassemble=main 'D:\Code\VScode\keyboard-framework\project\ch584m\obj\keyboard-framework.elf'
```

Expected:
- `system_init_coordinator` 中不再出现 `puts` / `iprintf` 等输出调用
- `main` 中不再出现初始化成功或失败分支对应的输出调用

- [ ] **Step 3: 检查 `_write()` 风险路径只受 `DEBUG` 控制**

执行：

```powershell
rg -n "#ifdef DEBUG|#if DEBUG == Debug_UART" D:\Code\VScode\keyboard-framework\project\ch584m\StdPeriphDriver
```

Expected:
- WCH SDK 底层输出后端仍只受 `DEBUG` 控制
- 业务层关闭后，不再因为 `dprintf/xprintf` 走到 `_write()`

- [ ] **Step 4: 记录最终尺寸**

Run:

```powershell
$size = 'D:\Software\Work\MounRiver_Studio2\resources\app\resources\win32\components\WCH\Toolchain\RISC-V Embedded GCC12\bin\riscv-wch-elf-size.exe'
& $size --format=berkeley 'D:\Code\VScode\keyboard-framework\project\ch584m\obj\keyboard-framework.elf'
```

Expected:
- 大小输出正常
- 无新增链接错误

---

## 交付说明要求

- 明确说明本次设计已经把业务日志总开关收敛为 `PRINTF_ENABLE`
- 明确说明 `DEBUG` 只是 WCH SDK 的底层串口输出开关和 UART 选择参数
- 明确说明 `PRINTF_ENABLE=FALSE` 时必须删除 `DEBUG`
- 明确说明 `PRINTF_ENABLE=TRUE` 时必须配置 `-DDEBUG=Debug_UARTx`，并与 `PRINTF_UART` 选同一 UART
- 若后续需要“保留 UART 初始化但关闭业务日志”，应新增独立配置项，不能继续复用 `PRINTF_ENABLE`
