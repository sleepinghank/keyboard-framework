# Shared Fn/Earth Key 需求重新分析

## 1. 需求背景

本文档为 2026-03-23 对 `2026-03-17-shared-fn-earth-key-requirements.md` 的重新梳理。
原始需求已有一版实现，但检查后发现多个场景存在问题，需全面重新实现。

原始需求文档：`DOCS/plans/2026-03-17-shared-fn-earth-key-requirements.md`

## 2. 需求摘要（来自原始需求文档）

### 2.1 平台行为映射

| 平台 | 单按行为 | 长按行为 |
|------|---------|---------|
| Apple (iOS/iPadOS/macOS) | 发 `M_EARTH` | 维持 `M_EARTH`（系统地球键长按语义） |
| Windows | 发 `Win+Space` | 无特殊行为 |
| Android | 发 `Shift+Space` | 无特殊行为 |

### 2.2 四个核心场景

**场景 A — 单按共用键（无其他键参与）**
- 按下 S_FN_KEY → 等待 → 松开 S_FN_KEY（无其他键）
- 发送对应平台的 Earth 事件（按下+抬起）
- FN_st=1 → 松开时发送 Earth → FN_st=0

**场景 B — 共用键 + Fn 功能组合键**
- 按下 S_FN_KEY → 按下 Fn 功能键 → 执行功能 → 不发 Earth
- active_fn_combo=1，combinations_flag=0，禁止发 Earth

**场景 C — 共用键 + 普通键（非 Fn 组合）**
- 按下 S_FN_KEY → 按下普通键 → 立即补发 Earth（按下普通键瞬间触发）
- Earth 和普通键同时按住，直到各自松开
- 不额外约束精细时序
- combinations_flag=0

**场景 D — 长按共用键超过 500ms**
- 按下 S_FN_KEY → 持续按住超过 500ms → 进入长按状态
- Apple 平台：持续维持 M_EARTH（系统解释为地球键长按）
- 非 Apple 平台：无特殊长按行为
- 场景 D 期间若再按普通键：Earth 继续维持，普通键并行上报

### 2.3 键码定义

```c
S_FN_KEY = QK_USER_0   // 物理共用键
M_EARTH  = QK_USER_1   // Earth 键码（Apple 平台）
```

### 2.4 长按阈值

新定义 500ms 常量（现有项目中未有对应常量可复用）。

## 3. 需求 Q&A 澄清记录

| 问题 | 回答 |
|------|------|
| 哪些场景有问题？ | 多个场景都有问题，需全面重实现 |
| 是否有检查文档？ | 没有，需重新分析代码 |
| 场景 D 是否必须实现？ | 必须实现 |
| 长按阈值？ | 新定义 500ms |
| 场景 D + 普通键交叉？ | Earth 继续维持，普通键并行上报 |

## 4. 多角度评审发现

### 4.1 用户/产品视角
- 场景 C 补发 Earth 时机：普通键按下瞬间触发（无超时延迟）
- 长按 500ms 后再按普通键：Earth 维持，普通键并行上报（已确认）
- 平台切换实时生效（复用现有平台检测机制）

### 4.2 架构/编码视角（已知 Bug）

**Bug 1（场景 C，明确缺陷）：**
`earth_post_loop_decision()` 第 120 行条件错误：
```c
// 当前错误：要求两个列表都有普通键
if (has_normal_key_in_list(key_list) && has_normal_key_in_list(key_list_extend))
// 应改为：只检查 key_list
if (has_normal_key_in_list(key_list))
```
`key_list_extend` 在此时刚被清空，永远为空，导致场景 C 永远不会触发。

**Bug 2（场景 D，combinations_flag 未清）：**
长按进入场景 D 时，`combinations_flag` 未被置 0，会干扰场景 B 的 Fn 功能键判断。
需在 LONG_PRESS_START 回调中将 `combinations_flag` 置 0。

**风险 1（场景 A，多键 tap 时序）：**
Win+Space / Shift+Space 需要在同一帧保持按下再释放，现有 `add_keys[]` 机制需确认可支持多键 tap 时序。

### 4.3 测试/质量视角

**测试用例矩阵（最小集）：**

| ID | 场景 | 输入 | 预期输出 |
|----|------|------|---------|
| A1 | 单按 Apple | 按下+松开 S_FN_KEY | M_EARTH 按下+抬起 |
| A2 | 单按 Windows | 按下+松开 S_FN_KEY | Win+Space 按下+抬起 |
| A3 | 单按 Android | 按下+松开 S_FN_KEY | Shift+Space 按下+抬起 |
| B1 | Fn 组合 | S_FN_KEY + Fn 功能键 | 执行功能，无 Earth |
| C1 | 普通键组合 | S_FN_KEY + 普通键 | Earth 先发，普通键后发 |
| C2 | 多普通键 | S_FN_KEY + 2 个普通键 | Earth 先发，普通键并行 |
| D1 | 长按 499ms | 按住 499ms 松开 | 不触发长按，走场景 A |
| D2 | 长按 Apple | 按住 500ms+ | 维持 M_EARTH |
| D3 | 长按非 Apple | 按住 500ms+ (Win) | 无特殊行为 |
| D4 | 长按+普通键 | 长按中再按普通键 | Earth 维持，普通键并行 |

**回归风险：**
- 场景 C 的补发逻辑修改可能影响现有 combo 系统
- 需对普通按键路径做全量回归

## 5. 现有代码现状（步骤 1 分析结果）

### 5.1 已存在的代码

| 文件 | 状态 | 内容 |
|------|------|------|
| `middleware/keyboard/combo/kb_fn_action.c` | 已有但有 Bug | earth_post_loop_decision(), FN_DOWN_KEY(), FN_UP_KEY(), Earth() |
| `middleware/keyboard/combo/kb_fn_action.h` | 已有 | 接口声明 |
| `middleware/keyboard/combo/kb_combo_engine.c` | 已有 | FN_st 变量，earth_post_loop_decision() 调用 |
| `middleware/keyboard/combo/kb_combo_map.c` | 已有 | FN_DOWN_combo[], FN_UP_combo[] 注册 |
| `middleware/keyboard/keycode.h` | 已有 | S_FN_KEY, M_EARTH 定义 |
| `middleware/keyboard/kb_sys_action.c` | 部分注释 | Earth_PRESS_UP_Handler（被注释掉） |

### 5.2 已知缺失/Bug

1. 场景 C 触发条件 Bug（永远不会触发）
2. 场景 D 未实现（无长按计时和 Earth 维持逻辑）
3. 场景 D 进入时 `combinations_flag` 未清零
4. `Earth_PRESS_UP_Handler` 被注释掉（场景 A 的核心发送函数）
5. 非 Apple 平台 Win+Space / Shift+Space 的精确实现待确认

## 6. 代码详细分析（步骤 3）

### 6.1 场景 A — 单按（已基本正常）

`FN_UP_KEY()` 在 `EARTH_PENDING` 分支按平台发 tap：Apple → M_EARTH，Win → Win+Space，Android → Shift+Space。
`Earth_PRESS_UP_Handler`（kb_sys_action.c）已被注释掉，逻辑已迁移至 `FN_UP_KEY()`，**场景 A 正常工作**。

### 6.2 场景 B — Fn 功能组合（基本正常）

`earth_post_loop_decision(fn_fired=1)` → 转 `EARTH_FN_TAKEN`，`FN_UP_KEY` 中 FN_TAKEN case 不发 Earth。
依赖 `fn_function_fired` 外部变量，逻辑正确。

### 6.3 场景 C — 普通键组合（有 2 个 Bug）

**Bug 1 — 平台限制（严重）：**
`earth_post_loop_decision()` 的补发 Earth 逻辑包裹在 `is_apple_platform()` 下，
Win/Android 平台的场景 C 完全未处理，这两个平台有普通键时仍走 PENDING 路径，
FN_UP 才发 Earth tap，顺序颠倒（Earth 在普通键之后发送）。

**Bug 2 — 条件错误（严重）：**
```c
// 当前（第 120 行）：要求两个列表同时有普通键
if (is_apple_platform() && has_normal_key_in_list(key_list) && has_normal_key_in_list(key_list_extend))
```
`key_list_extend` 在场景 C（普通键非 Fn 组合）时为空（combo 引擎不会向其添加普通键），
导致条件永远为 false，**场景 C 在 Apple 平台也无法触发**。

正确条件：只检查 `key_list`（物理原始键列表），且去掉 `is_apple_platform()` 限制。

### 6.4 场景 D — 长按（完全缺失）

combo 引擎现有 `long_press_ticks` 机制，但 `FN_DOWN_ID` / `FN_UP_ID` 使用默认 `LONG_TICKS`（2500ms），
且未绑定 `LONG_PRESS_START` 回调。场景 D 需要：
1. 新增 500ms 长按定时器（在 FN_DOWN_KEY 启动，FN_UP_KEY 取消）
2. 定时器到期后（Apple 平台）发送 M_EARTH down → 转新状态 `EARTH_HOLD`
3. `FN_UP_KEY` 中处理 `EARTH_HOLD` → 发 M_EARTH up
4. 场景 D + 普通键：Earth 继续维持，普通键并行上报（无需特殊处理）

### 6.5 关键接口和变量

| 变量/接口 | 位置 | 说明 |
|-----------|------|------|
| `is_apple_platform()` | kb_fn_action.c:62 | 检查 IOS/MAC 平台 |
| `earth_state` | kb_fn_action.c | 4 状态：IDLE/PENDING/DOWN/FN_TAKEN |
| `FN_st` | kb_combo_engine.c | Fn 键按下标志 |
| `combinations_flag` | kb_combo_engine.c | Fn 激活期间为 1 |
| `active_fn_combo` | kb_combo_engine.c | 每帧重置，有 Fn 组合触发时为 1 |
| `_key_code_list` | 传入参数 | 物理原始键列表 |
| `_key_code_list_extend` | 传入参数 | combo 生成的虚拟键列表（场景 C 时为空） |

### 6.6 需要新增的状态

场景 D 需要在现有 4 状态基础上新增 `EARTH_HOLD`（或复用现有状态并增加定时器标志）：
- IDLE → PENDING（FN_DOWN_KEY）
- PENDING → HOLD（500ms 定时器到期 && Apple 平台）
- PENDING → DOWN（普通键按下）
- PENDING → FN_TAKEN（Fn 功能键触发）
- PENDING → IDLE（FN_UP_KEY，发 Earth tap）
- HOLD → IDLE（FN_UP_KEY，发 M_EARTH up）

## 7. 成功标准

- 场景 A：主机端收到正确平台的输入法切换行为
- 场景 B：Fn 功能正常，无误触 Earth
- 场景 C：主机端先收到 Earth，再收到普通键，顺序正确
- 场景 D：Apple 平台长按地球键语义正常；非 Apple 无长按特殊行为
- 现有 Fn 层切换体验不被破坏
