<!-- OPENSPEC:START -->
# OpenSpec Instructions

These instructions are for AI assistants working in this project.

Always open `@/openspec/AGENTS.md` when the request:
- Mentions planning or proposals (words like proposal, spec, change, plan)
- Introduces new capabilities, breaking changes, architecture shifts, or big performance/security work
- Sounds ambiguous and you need the authoritative spec before coding

Use `@/openspec/AGENTS.md` to learn:
- How to create and apply change proposals
- Spec format and conventions
- Project structure and guidelines

Keep this managed block so 'openspec update' can refresh the instructions.

<!-- OPENSPEC:END -->

# Keyboard Framework Agent Guide

This guide defines how coding agents should work in this repository.

## 1) Project Snapshot

- Language: C11, embedded-oriented.
- Build system: CMake (`CMakeLists.txt`).
- Event system: OSAL task/event model (`drivers/system/event_manager.h`).
- Test framework available: Unity (`unity/unity.c`, `unity/unity.h`).
- Current product config targets CH584 by default (`keyboards/product_config.h`).
- Note: repo contains a file named `nul`; wide recursive grep on Windows can fail.

## 2) Layered Architecture (must preserve)

Primary dependency direction:
`Application -> Middleware -> Drivers -> HAL`

### HAL (`hal/`)
- Hardware abstraction APIs and platform bindings.
- Platform code under `hal/platforms/<platform>/`.

### Drivers (`drivers/`)
- Domain drivers: communication, input, output, power, storage, system.
- Should use HAL, not application services.

### Middleware (`middleware/`)
- Keyboard processing pipeline, transport switching, report handling, low-power coordination.

### Application (`application/`)
- Boot/orchestration and service initialization.
- Startup coordinator: `application/system/system_init.c`.

### Product config (`keyboards/`)
- Matrix pins, feature flags, transport toggles, timing config.

## 3) Critical Runtime Files

- Entry executable source: `test_main.c`.
- Startup flow: `application/system/system_init.c`.
- Event APIs: `drivers/system/event_manager.h`.
- Keyboard task core: `middleware/keyboard/keyboard.c`.
- Transport state switching: `middleware/communication/transport.c`.
- Wireless abstraction: `middleware/communication/wireless.h`.

## 4) Build / Test / Lint Commands

Use MSYS2 MINGW64 shell.

### Build (default)

```bash
cmake -S . -B build-agent -G "MinGW Makefiles"
cmake --build build-agent -j4
```

### Build (toolchain file)

```bash
cmake -S . -B build-agent -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=mingw-toolchain.cmake
cmake --build build-agent -j4
```

Observed in this repo state:
- default configure may fail compiler probe depending on environment;
- toolchain configure currently reports missing `drivers/output/backlight/backlight_pwm.c`.

### Run built executable

```bash
./build-agent/keyboard-framework.exe
```

`test_main.c` uses an infinite loop (`OSAL_SystemProcess()`), so this is not a finite unit-test runner.

### Tests

No active `enable_testing()` / `add_test()` in root `CMakeLists.txt` currently.
So `ctest` patterns are optional/future guidance:

```bash
ctest --test-dir build-agent -N
ctest --test-dir build-agent -R "^TestName$"
ctest --test-dir build-agent -I 5
```

Current single-test example pattern (from `test/storage_test.c` comments):

```bash
gcc -DTEST_MODE -o storage_test.exe test/storage_test.c drivers/storage/storage.c drivers/storage/test/eeprom.c -Idrivers/storage -Ikeyboards -Ihal -Iutils
./storage_test.exe
```

### Lint / format

- No repo-root `.clang-format` / clang-tidy / cppcheck automation discovered.
- A `.clang-format` exists only under `code_example/qmk_firmware/` (reference only).
- Follow local file style; do not introduce a new global formatting regime.

## 5) Code Style Conventions

### Naming
- Functions/variables: mostly `snake_case`.
- Macros/constants: `UPPER_SNAKE_CASE`.
- Types commonly end with `_t` (`storage_config_t`, `wt_state_t`).

### Headers and includes
- `#pragma once` is the common header guard approach.
- Include direct dependencies explicitly.
- Include order is mixed across modules; follow nearest-file precedent for touched files.

### Types and returns
- Prefer fixed-width integer types (`uint8_t`, `uint16_t`, `uint32_t`).
- Use `bool` for semantic true/false state.
- Use existing status types (`error_code_t`, boolean return, driver status bytes).

### Error handling
- Validate pointer and range inputs early.
- Keep error paths explicit and side-effect safe (locks/events/callbacks).
- Keep logging style consistent with local module (`dprintf`, `println`, `xprintf`).

### Event-driven patterns
- Event handlers clear consumed bits and return remaining bits.
- Register via `OSAL_ProcessEventRegister`, trigger via `OSAL_SetEvent`.
- Keep state machine names explicit (`WT_*`, init status enums).

### Preprocessor usage
- Feature gating via `#ifdef` is core to portability.
- Do not remove feature flags casually; many modules are build-time switchable.

## 6) Agent Working Rules

1. Read `CMakeLists.txt` and touched module headers before editing.
2. Preserve layer boundaries; avoid direct cross-layer calls.
3. Reuse existing event/state-machine patterns before adding new control flow.
4. Keep diffs minimal and localized.
5. If build breaks, report exact missing file/symbol and owning layer.
6. If adding tests, include clear single-test run command in notes.

## 7) Additional Rule Files

- `.cursor/rules/`: not found.
- `.cursorrules`: not found.
- `.github/copilot-instructions.md`: not found.

If these are added later, merge their constraints into this guide.

## 8) OpenSpec Reminder

For planning/proposal/spec changes or major architecture updates,
follow `openspec/AGENTS.md` before implementation.

## 9) Important Notes

在实现任何需求时，严禁直接修改代码，必须先理清需求，确认方案，编写详细的设计文档，并经过评审后才能动手编码。未经批准的代码更改将被视为违规行为。
