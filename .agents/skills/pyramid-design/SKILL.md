---
name: pyramid-design
description: "You MUST use this before any feature development work - creating features, building components, adding functionality, or modifying behavior. Uses Pyramid Principle and MECE analysis with parallel TaskCreate teams and document-based phase handoff to confirm requirements, analyze current state, design solutions, and eliminate rework. Design phase presents core flow diagrams first, then details. Implementation hands off to a new session with executing-plans skill for clean context. Trigger on phrases like '新增功能', '修改功能', '方案设计', '需求分析', 'implement feature', 'design solution', or any request to build or change behavior."
---

# Pyramid Design — 金字塔原理驱动的需求设计流程

## 核心目标

增加前期需求确认和方案设计时间，减少后期返工次数。**一次分析到位，交付 100% 可用的功能。**

- **金字塔原理**：结论先行，自顶向下表达，自底向上思考
- **MECE 原则**：相互独立、完全穷尽，不遗漏、不重叠
- **每阶段有文档产出**，作为下一阶段的唯一输入

<HARD-GATE>
Do NOT invoke any implementation skill, write any code, scaffold any project, or take any implementation action until you have presented a design and the user has approved it. This applies to EVERY project regardless of perceived simplicity.
</HARD-GATE>

---

## 三阶段架构与上下文管理

### 核心原则：主对话 = 轻量协调器

主对话**不直接读取源码文件**。所有重型工作（代码探索、代码分析、多角度评审）委派给 Task 子任务，子任务完成后只返回**结构化摘要**。阶段间通过**文档**传递完整上下文，避免对话历史累积。

**并行任务管理**：使用 `TaskCreate` 派遣并行任务，通过 `TaskList` 查看状态，`TaskGet`/`TaskOutput` 获取结果，`TaskStop` 停止失败任务。每个任务有独立 ID，可精确追踪和重试。

```
Phase 1 需求确认
  ├─ Task: 探索项目上下文 → 结构化摘要
  ├─ 主对话: 逐一提问用户
  ├─ TaskCreate x3 并行: 需求评审 → TaskList 追踪 → TaskOutput 收集意见
  ├─ TaskCreate: 代码现状分析 → 分析摘要
  └─ 输出: docs/plans/YYYY-MM-DD-<topic>-requirements.md
     [用户确认]

Phase 2 方案设计
  ├─ 主对话: 读 requirements.md → 差距分析
  ├─ 主对话: 核心流程图/伪代码 → 逐节展示方案 → 用户确认
  ├─ TaskCreate x5 并行: 方案评审 → TaskList 追踪 → 评审报告
  └─ 输出: docs/plans/YYYY-MM-DD-<topic>-design.md
     [用户确认]

Phase 3 计划输出与交接
  ├─ 主对话: 读 design.md → 制定实施计划
  ├─ TaskCreate: 文档质量评审
  └─ 输出实施启动指令 → 用户新开会话执行 superpowers:executing-plans
```

### 严格顺序规则

- 每个步骤独立完成，用户确认后才进入下一步
- 禁止单次 turn 内跳过步骤
- 每步开始声明：`【步骤 X/9 — <名称>】`

### Anti-Patterns

| 错误做法 | 正确做法 |
|---------|---------|
| 主对话直接读取大量源码 | 派遣 Task 分析，只接收摘要 |
| 单次 turn 跳过多个步骤 | 每步独立确认 |
| "太简单不需要设计" | 所有项目都走流程，设计可以短但必须有 |
| Task 返回完整代码内容 | Task 返回结构化摘要，限制字数 |
| 评审 Task 串行执行 | 独立评审维度使用 TaskCreate 并行 |
| 设计完在当前会话直接实施 | 文档交接后新开会话实施，保持上下文干净 |
| 方案展示直接列细节 | 先展示核心流程图/伪代码，再逐节深入 |

---

## Phase 1：需求确认

### 步骤 1 — 探索项目上下文

声明：`【步骤 1/9 — 探索项目上下文】`

**派遣 Task 子任务**探索项目（不要在主对话中直接探索）：

```
使用 TaskCreate 派遣子任务，prompt 要点：
- 围绕「<用户需求一句话摘要>」探索项目
- 检查文件结构、文档（CLAUDE.md, README 等）、近期 git 提交
- 评估需求规模：小 / 中 / 大
- 如涉及多个独立子系统，标记并建议拆解
- 输出限制 500 字以内的结构化摘要：
  1. 项目结构概览
  2. 与需求相关的模块列表（路径 + 一句话职责）
  3. 规模判断及依据
  4. 如规模过大，拆解建议
```

使用 `TaskGet` 获取结果后，将摘要展示给用户。

> **[CHECKPOINT 1]** 使用 `AskUserQuestion` 询问："步骤 1 完成。是否继续步骤 2（需求澄清）？"

---

### 步骤 2 — 需求澄清与评审

声明：`【步骤 2/9 — 需求澄清与评审】`

**Part A — 逐一提问（使用 `AskUserQuestion` 工具）：**
- 每次使用 `AskUserQuestion` 只问一个问题，等待用户回答后再问下一个
- 优先给选项，聚焦：目的、约束条件、成功标准
- 所有问题回答完毕后，整理为「需求 Q&A 摘要」

**Part B — 并行评审（TaskCreate 管理）：**

所有问题回答完毕后，使用 `TaskCreate` **并行**派遣 3 个评审任务（参考 `references/agent-roles.md` 中「需求评审团」）：

| Task | 角色 | 输入 |
|------|------|------|
| Task 1 | 用户/产品视角 | 需求 Q&A 摘要 + 步骤 1 项目摘要 |
| Task 2 | 架构/编码视角 | 需求 Q&A 摘要 + 步骤 1 项目摘要 |
| Task 3 | 测试/质量视角 | 需求 Q&A 摘要 + 步骤 1 项目摘要 |

每个 Task 输出限制 200 字以内。三个 Task 在同一个 turn 中通过 `TaskCreate` 并行派遣。派遣后使用 `TaskList` 查看状态，`TaskOutput` 收集结果。如某个 Task 失败，用 `TaskStop` 停止后重新派遣。

汇总评审意见，使用 `Write` 工具写入 `docs/plans/YYYY-MM-DD-<topic>-requirements.md`（需求部分）。

> **[CHECKPOINT 2]** 展示需求摘要 + 各 Task 评审意见 + 范围边界。使用 `AskUserQuestion` 询问："步骤 2 完成。是否继续步骤 3？"

---

### 步骤 3 — 代码现状分析

声明：`【步骤 3/9 — 代码现状分析】`

根据步骤 1 识别的相关模块，使用 `TaskCreate` **派遣 Task 子任务**进行代码分析（不要在主对话中直接读代码）：

- 相关模块 ≤3 个：派遣 1 个 Task 分析全部
- 相关模块 >3 个：按模块分组，**并行**派遣多个 Task（`TaskCreate` 并行）

```
每个代码分析 Task 的 prompt 要点：
- 分析指定模块与「<需求摘要>」的关系
- 需求关键点：<从 requirements.md 提取>
- 模块路径：<具体路径列表>
- 输出限制 500 字以内的结构化分析：
  1. 结论：当前缺少什么、需要修改什么（结论先行）
  2. 模块职责与关键接口
  3. 数据流与调用链
  4. 与需求的具体差距点
```

汇总分析结果，使用 `Edit` 工具附加至 `requirements.md`。

> **[CHECKPOINT 3 — Phase 1 完成]** 展示现状分析结论。使用 `AskUserQuestion` 询问："阶段一（需求确认）完成。是否进入阶段二（方案设计）？"

---

## Phase 2：方案设计

**本阶段上下文起点**：使用 `Read` 工具读取 `requirements.md` 作为主要输入，不依赖 Phase 1 的对话历史。

### 步骤 4 — 差距分析与方案制定

声明：`【步骤 4/9 — 差距分析与方案制定】`

- 基于 `requirements.md` 中的现状分析和需求，明确差距，抓住主要矛盾
- 先确认核心功能范围，避免 YAGNI
- 使用 `AskUserQuestion` 逐个提问完善想法（每次一个，等待回答后再问下一个）
- 提出 2-3 个方案，**推荐方案放最前**，含利弊权衡

> **[CHECKPOINT 4]** 展示差距分析和候选方案。使用 `AskUserQuestion` 询问："步骤 4 完成。请选定方向。"

---

### 步骤 5 — 方案展示与用户确认

声明：`【步骤 5/9 — 方案展示与用户确认】`

**结论先行 — 先展示核心流程全貌，再逐节深入细节。**

**逐节展示**，每节确认后再继续（不要一次全部输出）：

1. **核心流程总览**（必须包含 Mermaid 流程图或伪代码）→ 使用 `AskUserQuestion` 询问："核心流程是否符合预期？"
   - 用 Mermaid 流程图展示关键模块间的调用/数据流向
   - 或用伪代码展示核心逻辑的执行顺序
   - 目标：让用户 30 秒内理解"整体怎么跑起来的"
   - 这是金字塔原理的核心体现 — 先给结论（全貌），再给支撑（细节）
2. 架构设计 → 使用 `AskUserQuestion` 确认
3. 组件/接口设计 → 使用 `AskUserQuestion` 确认
4. 数据流与错误处理 → 使用 `AskUserQuestion` 确认
5. 测试策略 → 使用 `AskUserQuestion` 确认

每节按复杂度缩放：简单几句话，复杂 200-300 字。

**设计隔离原则**：每个模块只做一件事，接口明确，可独立理解和测试。

> **[CHECKPOINT 5]** 使用 `AskUserQuestion` 询问："方案各节已确认。是否继续步骤 6（多角度评审）？"

---

### 步骤 6 — 方案多角度评审

声明：`【步骤 6/9 — 方案多角度评审】`

使用 `TaskCreate` **并行**派遣 5 个评审任务（参考 `references/agent-roles.md` 中「方案评审团」）：

| Task | 评审维度 | 输入 |
|------|---------|------|
| Task 1 | 功能完整性 | 方案摘要 + requirements.md |
| Task 2 | 技术可行性 | 方案摘要 |
| Task 3 | 可维护性 | 方案摘要 |
| Task 4 | 可测试性 | 方案摘要 |
| Task 5 | 风险识别 | 方案摘要 |

每个 Task 输出限制 200 字以内。五个 Task 在同一个 turn 中通过 `TaskCreate` 并行派遣。使用 `TaskList` 监控状态，`TaskOutput` 收集结果。如某个 Task 失败，用 `TaskStop` 停止后单独重试，无需重跑全部。

如发现问题，返回步骤 4 修订。评审通过后使用 `Write` 工具将方案写入 `docs/plans/YYYY-MM-DD-<topic>-design.md`。

> **[CHECKPOINT 6 — Phase 2 完成]** 展示评审报告。使用 `AskUserQuestion` 询问："阶段二完成。是否进入阶段三？"

---

## Phase 3：计划输出

**本阶段上下文起点**：使用 `Read` 工具读取 `design.md` 作为主要输入。

### 步骤 7 — 制定实施计划

声明：`【步骤 7/9 — 制定实施计划】`

基于 `design.md`，每步遵循 **SMART 原则**：

- **S**pecific — 具体到文件、函数、接口
- **M**easurable — 完成标准和验证命令
- **A**chievable — 单步可在一次会话内完成
- **R**elevant — 直接服务于需求目标
- **T**ime-bound — 依赖顺序和前置条件明确

> **[CHECKPOINT 7]** 展示实施计划。使用 `AskUserQuestion` 询问："步骤 7 完成。请确认计划。"

---

### 步骤 8 — 编写设计文档

声明：`【步骤 8/9 — 编写设计文档】`

使用 `Write` 工具将完整设计写入 `docs/plans/YYYY-MM-DD-<topic>-design.md`（更新之前的草稿），包含：
- 需求摘要
- 现状分析
- 方案设计
- 实施计划

**文档评审**：使用 `TaskCreate` 派遣评审任务检查文档质量（参考 `references/agent-roles.md` 中「文档评审员」）。使用 `TaskOutput` 获取评审结果。如发现问题，修正后重新派遣，最多 5 次。

提交到 git。

> **[CHECKPOINT 8]** 使用 `AskUserQuestion` 询问："设计文档已完成。是否继续步骤 9？"

---

### 步骤 9 — 交接实施

声明：`【步骤 9/9 — 交接实施】`

<IMPORTANT>
**禁止在当前会话中开始实施。** 当前会话已积累大量设计讨论上下文，继续实施会导致上下文过大、注意力分散、实施质量下降。必须新开会话，以文档为唯一入口。
</IMPORTANT>

**交接步骤：**

1. 确认 `requirements.md` 和 `design.md` 已完成并提交到 git
2. 向用户展示实施启动指令：

```
请新开一个会话，输入以下指令开始实施：

请按照 docs/plans/YYYY-MM-DD-<topic>-design.md 的实施计划进行开发。
使用 /superpowers:executing-plans 来执行实施。
```

3. 说明新会话的优势：
   - 干净的上下文，只包含设计文档和代码
   - `executing-plans` skill 提供分步执行和 review checkpoint
   - 实施过程中遇到问题可回溯设计文档，而非依赖对话记忆

**禁止**在当前会话调用 `writing-plans` 或 `executing-plans` 或任何实施 skill。

---

## 后续阶段参考（由新会话中的 executing-plans 接管）

**实施阶段**：新会话中使用 `superpowers:executing-plans`，按计划逐步实施，每步完成后验证。
**代码评审阶段**：多角度评审（Bug、质量、规范），输出 `docs/plans/YYYY-MM-DD-<topic>-review.md`。
**交付确认**：对照需求逐条确认，功能 100% 可用后收尾。

---

## Key Principles

- **金字塔原理** — 结论先行，再展示支撑依据
- **MECE 原则** — 分析和方案必须相互独立、完全穷尽
- **上下文隔离** — 重型工作委派 Task 子任务，主对话只接收摘要；实施阶段新开会话
- **文档衔接** — 阶段间通过文档传递上下文，不依赖对话历史
- **并行加速** — 独立评审维度使用 TaskCreate 并行执行，TaskList 追踪状态
- **核心流程先行** — 方案展示先用流程图/伪代码呈现全貌，再逐节深入细节
- **一次问一个** — 不要同时抛出多个问题
- **优先给选项** — 比开放式问题更容易回答
- **YAGNI** — 去除不必要的功能
