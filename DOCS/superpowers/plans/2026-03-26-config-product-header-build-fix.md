# Config Product Header Build Fix Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Restore the CH584 MounRiver build after the `config.h` to `config_product.h` rename by fixing remaining broken include paths and one missing low-power header declaration.

**Architecture:** The codebase has already converged on `kb904/config_product.h` as the stable product-config include path for cross-directory code. This fix keeps that convention, updates only the remaining translation units that still use bare `config_product.h`, and adds the missing `lpm.h` include so `keyboard.c` sees the `lpm_timer_reset()` declaration.

**Tech Stack:** C11 firmware, MounRiver Studio 2.4.0 generated makefiles, WCH RISC-V GCC12 toolchain, CH584 target.

---

## 1. Root Cause Summary

The failing build was reproduced with:

```bat
cmd.exe /c "cd /d D:\Code\VScode\keyboard-framework\project\ch584m\obj && set PATH=D:\Software\Work\MounRiver_Studio2\resources\app\resources\win32\components\WCH\Toolchain\RISC-V Embedded GCC12\bin;D:\Software\Work\MounRiver_Studio2\resources\app\resources\win32\others\Build_Tools\Make\bin;%PATH% && make.exe -j8 all"
```

Observed failures:

- `middleware/keyboard/combo/kb_sys_action.c:3` cannot find `config_product.h`
- `keyboards/kb904/keymaps/default.c:27` cannot find `config_product.h`
- `middleware/keyboard/keyboard.c:81` warns that `lpm_timer_reset()` is implicitly declared

### Why this happens

- The actual product header now lives at `keyboards/kb904/config_product.h`
- MounRiver generated compile commands include `-I .../keyboards`, but do **not** include `-I .../keyboards/kb904`
- Therefore, cross-directory sources must include the header as `kb904/config_product.h`
- `middleware/keyboard/keyboard.c` calls `lpm_timer_reset()` but does not include `middleware/communication/lpm.h`

## 2. Scope

### Files to modify

- `middleware/keyboard/combo/kb_sys_action.c`
- `keyboards/kb904/keymaps/default.c`
- `component/ota_component/AES128.c`
- `component/ota_component/Inateck_ota.c`
- `drivers/ota/Inateck_tag_config.c`
- `project/ch584m/Profile/hidkbdservice.c`
- `middleware/keyboard/keyboard.c`

### Files intentionally not modified

- `project/ch584m/.cproject`
- `project/ch584m/obj/**/*.mk`
- `CMakeLists.txt`
- `keyboards/kb904/indicator.c`

Reason:

- Fixing source includes is lower risk than expanding include search paths in multiple build systems
- `keyboards/kb904/indicator.c` is in the same directory as `config_product.h`, so it is not part of the current failure chain

## 3. Chosen Fix

### 3.1 Include path normalization

For every affected cross-directory source file, replace:

```c
#include "config_product.h"
```

with:

```c
#include "kb904/config_product.h"
```

### 3.2 Missing declaration fix

In `middleware/keyboard/keyboard.c`, add:

```c
#include "lpm.h"
```

This is a declaration-only fix. No runtime behavior change is intended.

## 4. Rejected Alternative

### Add `keyboards/kb904` to MounRiver include paths

Rejected because:

- It would require changing generated-project configuration instead of fixing the inconsistent source usage
- It would leave the codebase with two parallel include conventions
- It would still not explain the missing `lpm_timer_reset()` declaration warning

## 5. Implementation Tasks

### Task 1: Preserve the failing reproduction as the regression check

**Files:**
- Modify: none
- Test: existing MounRiver build command

- [ ] **Step 1: Run the failing build command before changes**

Run:

```bat
cmd.exe /c "cd /d D:\Code\VScode\keyboard-framework\project\ch584m\obj && set PATH=D:\Software\Work\MounRiver_Studio2\resources\app\resources\win32\components\WCH\Toolchain\RISC-V Embedded GCC12\bin;D:\Software\Work\MounRiver_Studio2\resources\app\resources\win32\others\Build_Tools\Make\bin;%PATH% && make.exe -j8 all"
```

Expected:

- FAIL with `fatal error: config_product.h: No such file or directory`
- WARN for implicit declaration of `lpm_timer_reset`

### Task 2: Fix remaining broken product-config includes

**Files:**
- Modify: `middleware/keyboard/combo/kb_sys_action.c`
- Modify: `keyboards/kb904/keymaps/default.c`
- Modify: `component/ota_component/AES128.c`
- Modify: `component/ota_component/Inateck_ota.c`
- Modify: `drivers/ota/Inateck_tag_config.c`
- Modify: `project/ch584m/Profile/hidkbdservice.c`

- [ ] **Step 1: Replace each bare product-config include with the repo-standard path**

Apply this exact change in each listed file:

```c
-#include "config_product.h"
+#include "kb904/config_product.h"
```

- [ ] **Step 2: Verify only cross-directory files were touched**

Run:

```powershell
rg -n --hidden --glob '!build*' --glob '!nul' '#include "config_product\.h"' D:\Code\VScode\keyboard-framework
```

Expected:

- Only same-directory include sites remain, or zero results if all are normalized intentionally

### Task 3: Fix the missing low-power declaration

**Files:**
- Modify: `middleware/keyboard/keyboard.c`

- [ ] **Step 1: Add the missing header for `lpm_timer_reset()`**

Insert:

```c
#include "lpm.h"
```

near the other middleware communication headers.

- [ ] **Step 2: Keep the change minimal**

Do not change call sites or LPM behavior. This task is declaration-only.

### Task 4: Rebuild and confirm the fix

**Files:**
- Modify: none
- Test: CH584 MounRiver build

- [ ] **Step 1: Re-run the same build command**

Run:

```bat
cmd.exe /c "cd /d D:\Code\VScode\keyboard-framework\project\ch584m\obj && set PATH=D:\Software\Work\MounRiver_Studio2\resources\app\resources\win32\components\WCH\Toolchain\RISC-V Embedded GCC12\bin;D:\Software\Work\MounRiver_Studio2\resources\app\resources\win32\others\Build_Tools\Make\bin;%PATH% && make.exe -j8 all"
```

Expected:

- No `config_product.h` missing-header errors
- No `implicit declaration of function 'lpm_timer_reset'` warning
- Build continues to link, or exposes the next independent error if additional unrelated breakage exists

- [ ] **Step 2: If a new error appears, classify it before changing code**

Document whether the new failure is:

- another leftover rename/include issue in the same migration
- an unrelated pre-existing compile problem

No opportunistic refactors.

## 6. Risk Check

- Low risk: include path normalization does not change runtime logic
- Low risk: adding `lpm.h` only exposes an existing API declaration already implemented in `middleware/communication/lpm.c`
- Main risk: after fixing the first batch, additional unrelated compile errors may surface because the worktree already contains many in-progress changes

## 7. Completion Criteria

- MounRiver build no longer fails on `config_product.h` lookup
- `keyboard.c` no longer emits the implicit declaration warning for `lpm_timer_reset()`
- No build-system include path changes are required for this repair
