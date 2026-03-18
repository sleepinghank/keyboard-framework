---
name: pyramid-design
description: "Use when analyzing or designing a new feature, behavior change, or multi-step implementation where requirements, scope, and architecture must be validated before coding. Not for: typo fixes, content-only changes, mechanical config updates, or single-line edits."
---

# Pyramid Design — 金字塔原理驱动的需求设计流程

## 何时使用

**适用场景：**
- 新增功能或模块
- 修改现有行为逻辑
- 涉及多文件、多模块的变更
- 需要架构决策的实现
- 用户需求不明确或需要澄清

**不适用场景（无需触发此 skill）：**
- 纯文案/注释修改
- 单点 typo 修复
- 机械性配置更新（如改端口号、版本号）
- 单文件 ≤20 行的简单修改且无架构影响
- 已有明确设计文档的纯实现任务

## 核心目标

增加前期需求确认和方案设计时间，减少后期返工次数。**一次分析到位，交付 100% 可用的功能。**

运用**金字塔原理**：结论先行，再展示支撑依据；自顶向下表达，自底向上思考。
遵循 **MECE 原则**：相互独立、完全穷尽，不遗漏、不重叠。

<HARD-GATE>
Do NOT invoke any implementation skill, write any code, scaffold any project, or take any implementation action until you have presented a design and the user has approved it.
</HARD-GATE>

## 术语表

| 术语 | 文件名 | 说明 |
|------|--------|------|
| 需求文档 | `<topic>-requirements.md` | 步骤 2 输出，记录澄清后的需求 |
| 阶段一交接文档 | `<topic>-phase1.md` | 阶段一输出，含需求 + 代码现状分析 |
| 阶段二交接文档 | `<topic>-phase2.md` | 阶段二输出，含选定方案 + 评审结论 |
| 设计文档 | `<topic>-design.md` | 最终输出，含实施计划，可直接 handoff |
| `<topic>` | — | 步骤 2 确定的功能/主题简称，全流程共用 |

---

## 架构设计：三阶段隔离 + Agent Team 并行

```
Phase 1                    Phase 2                    Phase 3
(步骤 1-3：需求确认)        (步骤 4-6：方案设计)        (步骤 7-9：计划输出)
        |                          |                          |
        ▼                          ▼                          ▼
  步骤1: 探索上下文          步骤4: 差距分析            步骤7: 制定实施计划
  步骤2: 需求澄清            步骤5: 方案展示            步骤8: 编写设计文档
  步骤3: 代码分析 ────►     步骤6: 多角度评审 ────►    步骤9: Handoff
    [并行 agent×N]          [并行 agent×5]
        |                          |                          |
        ▼                          ▼                          ▼
 <topic>-phase1.md  ────►   <topic>-phase2.md  ────►    <topic>-design.md
```

**核心设计原则：**
- 每个阶段结束时将结论写入标准化交接文档
- 下一阶段**只读交接文档，不依赖对话历史**
- 步骤 3（代码分析）和步骤 6（方案评审）使用并行 Agent Team

---

## 严格顺序执行规则

**每个步骤必须独立完成，在用户明确确认后才能进入下一步。**

### 每步开始时必须声明

```
【步骤 X/9 — <步骤名称>】
```

### Anti-Pattern: 批量执行多个步骤

不要在用户未确认的情况下连续执行步骤 1 → 2 → 3。这看起来像"效率高"，但实际上：
- 绕过了用户可以修正需求理解的时机
- 让需求澄清（步骤 2）变成走形式
- 后期发现方向偏差，返工成本比多几次确认高出数倍

### Anti-Pattern: "This Is Too Simple To Need A Design"

Every project goes through this process. "Simple" projects are where unexamined assumptions cause the most wasted work. The design can be short, but you MUST present it and get approval.

### Anti-Pattern: 上下文复读

**禁止**在后续步骤中重复展开前序步骤的详细分析过程。已完成步骤的信息通过交接文档传递，后续步骤只引用文档路径和关键结论。

---

## 交接文档规范

**只写结论，不写过程。**
- ✅ "模块 X 缺少 Y 接口，需要新增 Z"
- ❌ "我们首先检查了 A 文件，然后发现 B，经过分析认为 C……"

**命名规则：**
- 根名称在步骤 2 确定，格式：`<topic>-<stage>.md`
- 所有文档共享同一 `<topic>` 前缀
- 示例：`combo-key-requirements.md` → `combo-key-phase1.md` → `combo-key-design.md`

---

## Key Principles

- **金字塔原理** — 结论先行，再展示支撑依据
- **MECE 原则** — 分析和方案必须相互独立、完全穷尽
- **阶段隔离** — 每阶段通过文档交接，不依赖对话历史
- **真并行 Agent Team** — 步骤 3、6 派发独立 Agent Team 并行执行
- **只传结论** — 交接文档只写结论，过程信息不跨阶段传递
- **一次问一个** — 不要同时抛出多个问题
- **优先给选项** — 比开放式问题更容易回答
- **YAGNI 原则** — 从所有设计中去除不必要的功能

---

## 完整流程

### 阶段一：需求确认

详见 [references/phase1-workflow.md](references/phase1-workflow.md)

1. **探索项目上下文** — 检查文件、文档、近期提交
2. **需求澄清与 Agent Team 评审** — 逐一提问，多角色评审
3. **代码现状分析** — Agent Team 并行按模块拆解

**输出：** `docs/plans/<topic>-requirements.md` → `docs/plans/<topic>-phase1.md`

### 阶段二：方案设计

详见 [references/phase2-workflow.md](references/phase2-workflow.md)

4. **差距分析与方案制定** — 对比现状与目标，提出 2-3 个方案
5. **方案展示与用户确认** — 逐节展示设计，获得用户认可
6. **方案多角度评审** — 5 个 Agent Team 并行评审

**输入：** `docs/plans/<topic>-phase1.md`
**输出：** `docs/plans/<topic>-phase2.md`

### 阶段三：计划输出

详见 [references/phase3-workflow.md](references/phase3-workflow.md)

7. **制定实施计划** — SMART 原则，含验证标准
8. **编写设计文档** — 保存至 `docs/plans/<topic>-design.md`
9. **转入实施阶段** — 完成设计文档后，进入下一步 skill

**步骤 9 Handoff：**
设计文档完成后，根据任务性质选择后续 skill：
- **代码实现** → `superpowers:executing-plans` 或 `superpowers:writing-plans`
- **需要 TDD** → `superpowers:test-driven-development`
- **需要创建 PR** → `engineering-workflow-skills:pr`

**输入：** `docs/plans/<topic>-phase2.md`
**输出：** `docs/plans/<topic>-design.md`

---

## 入口：启动流程

收到功能需求时：

1. **声明**：`【启动 Pyramid Design 流程】`
2. **执行**：`【步骤 1/9 — 探索项目上下文】`
