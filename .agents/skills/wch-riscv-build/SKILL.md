---
name: wch-riscv-build
description: |
  Build and flash WCH RISC-V BLE projects (CH58x/CH59x chips). Use this skill when the user wants to compile, build, or flash firmware for WCH沁恒 BLE chips using MounRiver Studio toolchain. Triggers on phrases like "编译WCH项目", "build BLE firmware", "烧录CH58x", "编译沁恒项目", "flash WCH chip", or when working with projects containing .cproject, .wvproj, or obj/makefile in MounRiver structure.
---

# WCH RISC-V BLE 项目编译技能

编译和烧录沁恒（WCH）RISC-V BLE 芯片项目（CH58x/CH59x 系列）。

## 项目识别

此 skill 适用于具有以下特征的项目：
- 包含 `.cproject` 和 `.project` 文件（Eclipse/MounRiver 项目）
- 包含 `.wvproj` 文件（MounRiver 项目）
- `obj/` 目录下有 `makefile`
- 源码中使用 `CH58x_*.c` 或 `CH59x_*.c` 标准外设驱动

## 工具链检测

自动检测 MounRiver Studio 安装位置，按以下顺序搜索：

```
1. D:/Software/Work/MounRiver_Studio2
2. C:/Program Files/MounRiver/MounRiver_Studio2
3. C:/Program Files (x86)/MounRiver/MounRiver_Studio2
4. 用户指定路径
```

工具链组件路径（相对于 MounRiver 根目录）：
- **GCC 编译器**: `resources/app/resources/win32/components/WCH/Toolchain/RISC-V Embedded GCC12/bin/`
- **Make 工具**: `resources/app/resources/win32/others/Build_Tools/Make/bin/`
- **ISP 烧录工具**: `resources/app/resources/win32/components/WCH/Others/WCHISPTool/default/WCHISPTool_CH57x-59x/`

## 编译命令

### 1. 查找项目 obj 目录
```bash
# 在项目根目录或子目录中查找 obj/makefile
```

### 2. 执行编译（通过 cmd.exe）
由于 makefile 使用 Windows 路径格式，必须通过 cmd.exe 执行：

```bash
cmd.exe /c "cd /d <WINDOWS_OBJ_PATH> && set PATH=<TOOLCHAIN_PATH>;<MAKE_PATH>;%PATH% && make.exe main-build"
```

**完整命令模板：**
```bash
# 假设 MounRiver 安装在 D:\Software\Work\MounRiver_Studio2
# 项目路径为 D:\Code\MyProject\BLE\obj

cmd.exe /c "cd /d D:\\Code\\MyProject\\BLE\\obj && \
  set PATH=D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\components\\WCH\\Toolchain\\RISC-V Embedded GCC12\\bin;D:\\Software\\Work\\MounRiver_Studio2\\resources\\app\\resources\\win32\\others\\Build_Tools\\Make\\bin;%PATH% && \
  make.exe main-build"
```

### 3. 清理并重新编译
```bash
cmd.exe /c "cd /d <WINDOWS_OBJ_PATH> && set PATH=<TOOLCHAIN_PATH>;<MAKE_PATH>;%PATH% && make.exe clean && make.exe main-build"
```

## 编译输出

成功编译后生成以下文件（在 obj 目录）：
- `<project>.elf` - ELF 可执行文件
- `<project>.hex` - HEX 固件（用于烧录）
- `<project>.map` - 内存映射文件
- `<project>.lst` - 反汇编列表

编译输出示例：
```
Memory region         Used Size  Region Size  %age Used
           FLASH:      194608 B       216 KB     87.98%
             RAM:       39448 B        96 KB     40.13%
riscv-wch-elf-size --format=berkeley "WCH_3mode_kb_584.elf"
   text    data     bss     dec     hex filename
 192300    2308   13376  207984   32c70 WCH_3mode_kb_584.elf
```

## 烧录固件

### WCH-Link 方式（推荐）
使用 WCH-Link 调试器通过 SWD 接口烧录：

```bash
# 打开 WCH-LinkUtility 进行烧录
"<MOUNTRIVER_PATH>/resources/app/resources/win32/components/WCH/Others/WCH-LinkUtility/WCH-LinkUtility.exe"
```

### ISP 串口烧录
使用 USB 串口进行 ISP 烧录：

```bash
# CH57x-59x 系列 ISP 工具
"<MOUNTRIVER_PATH>/resources/app/resources/win32/components/WCH/Others/WCHISPTool/default/WCHISPTool_CH57x-59x/WCHISPTool_CH57x-59x.exe"
```

**注意**: ISP 烧录需要：
1. 芯片进入 Bootloader 模式（通常通过特定引脚组合）
2. USB 串口连接
3. 选择对应的 .hex 文件

## 常见问题

### 1. 找不到 riscv-wch-elf-gcc
确保通过 cmd.exe 执行命令，而不是直接在 WSL bash 中执行。

### 2. make: *** No targets specified
确保在 `obj/` 目录下执行，而不是项目根目录。

### 3. 路径包含空格
Windows 路径需要用引号包裹，或使用 8.3 短路径格式。

### 4. 编译成功但无法烧录
- 检查 WCH-Link 驱动是否安装
- 检查芯片是否正确连接（SWDIO, SWCLK, VCC, GND）
- 尝试降低 SWD 时钟频率

## 编译器信息

- **编译器**: riscv-wch-elf-gcc (xPack GNU RISC-V Embedded GCC) 12.2.0
- **目标架构**: RV32IMC with XW extension（沁恒私有扩展）
- **ABI**: ilp32
- **代码模型**: medany
