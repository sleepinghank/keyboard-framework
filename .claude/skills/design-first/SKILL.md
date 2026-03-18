---
name: design-first
description: "You MUST use this before any feature development work - creating features, building components, adding functionality, or modifying behavior. Uses Pyramid Principle and MECE analysis to confirm requirements, analyze current state, design solutions, and eliminate rework through thorough upfront design. Trigger on phrases like '新增功能', '修改功能', '方案设计', '需求分析', 'implement feature', 'design solution', or any request to build or change behavior."
---

# Design First — 金字塔原理驱动的需求设计流程

## 核心目标 / Ultimate Goal

增加前期需求确认和方案设计时间，减少后期返工次数。**一次分析到位，交付 100% 可用的功能。**

运用**金字塔原理**：结论先行，再展示支撑依据；自顶向下表达，自底向上思考。
遵循 **MECE 原则**：相互独立、完全穷尽，不遗漏、不重叠。
每个阶段结束必须有**文档产出**，并作为下一阶段的唯一上下文输入。

<HARD-GATE>
Do NOT invoke any implementation skill, write any code, scaffold any project, or take any implementation action until you have presented a design and the user has approved it. This applies to EVERY project regardless of perceived simplicity.
</HARD-GATE>

---

## 架构设计：三阶段隔离 + Subagent 并行

本 skill 分为三个独立阶段，每个阶段是独立的 skill 文件。

**核心设计原则：**
- 每个阶段结束时将结论写入标准化交接文档（`docs/plans/` 下）
- 下一阶段 skill **只读交接文档，不依赖对话历史**，彻底解决上下文膨胀问题
- 步骤3（代码分析）和步骤6（方案评审）使用真并行 Subagent，减少等待时间

```
Phase 1 Skill               Phase 2 Skill               Phase 3 Skill
(步骤 1-3：需求确认)          (步骤 4-6：方案设计)          (步骤 7-9：计划输出)
        |                           |                           |
        ▼                           ▼                           ▼
  步骤1: 探索上下文           步骤4: 差距分析             步骤7: 制定实施计划
  步骤2: 需求澄清             步骤5: 方案展示             步骤8: 编写设计文档
  步骤3: 代码分析 ──────►    步骤6: 多角度评审 ──────►   步骤9: 转入实施
    [并行 subagent×N]           [并行 subagent×5]
        |                           |                           |
        ▼                           ▼                           ▼
 phase1-output.md  ─────►  phase2-output.md  ─────►   design.md (最终)
（交接文档，仅含结论）       （交接文档，仅含结论）
```

**并行 Subagent 分工：**

步骤3 — 代码现状分析并行：
- 按模块拆分，每个 subagent 负责一个子模块
- 各自独立读取文件，输出"结论 + 关键接口"摘要
- 主线程汇总，不保留 subagent 的详细过程

步骤6 — 方案多角度评审并行：
- 5 个 subagent 各自独立审查一个维度（见 Phase 2 skill）
- 主线程收集各维度结论，汇总为评审报告

---

## 严格顺序执行规则（核心约束）

**每个步骤必须独立完成，在用户明确确认后才能进入下一步。**

原因：
- 步骤 2（需求澄清）的问题和回答是步骤 3（代码分析）的输入；跳过会导致分析方向偏差
- 步骤 3（现状分析）的结论是步骤 4（差距分析）的前提；没有现状就无法定义差距
- 用户在任意步骤都可能有需要补充的信息或修正方向；批量执行会让这些纠正机会消失

**禁止在单次 turn 内连续完成多个步骤。** 即使某步骤看起来很短，也必须展示给用户确认后再继续。

### 每步开始时必须声明

在每个步骤开始时，先输出：

```
【步骤 X/9 — <步骤名称>】
```

---

## Anti-Pattern: "This Is Too Simple To Need A Design"

Every project goes through this process — a single-function change, a config tweak, all of them. "Simple" projects are where unexamined assumptions cause the most wasted work. The design can be short (a few sentences), but you MUST present it and get approval.

## Anti-Pattern: 批量执行多个步骤

不要在用户未确认的情况下连续执行步骤 1 → 2 → 3。这看起来像"效率高"，但实际上：
- 绕过了用户可以修正需求理解的时机
- 让需求澄清（步骤 2）变成走形式，而不是真正的对话
- 后期发现方向偏差，返工成本比多几次确认高出数倍

## Anti-Pattern: 上下文复读

**禁止**在后续步骤中重复展开前序步骤的详细分析过程。已完成步骤的信息通过交接文档传递，后续步骤只引用文档路径和关键结论，不重复输出原始内容。

---

## Checklist（按顺序完成）

**阶段一：需求确认** → 读 `phase1-output.md` → [调用 Phase 1 Skill]
1. **探索项目上下文** — 检查文件、文档、近期提交
2. **需求澄清与 Agent 评审** — 逐一提问，多角色 Agent 团队审视需求
3. **代码现状分析** — Subagent 并行按模块拆解，汇总结论

**阶段二：方案设计** → 读 `phase2-output.md` → [调用 Phase 2 Skill]
4. **差距分析与方案制定** — 对比现状与目标，聚焦核心矛盾，提出 2-3 个方案
5. **方案展示与用户确认** — 逐节展示设计，获得用户认可
6. **方案多角度评审** — 5 个 Subagent 并行评审，汇总结论

**阶段三：计划输出** → [调用 Phase 3 Skill]
7. **制定实施计划** — SMART 原则，核心功能描述完善，含验证标准
8. **编写设计文档** — 保存至 `docs/plans/YYYY-MM-DD-<topic>-design.md` 并提交
9. **转入实施阶段** — 调用 writing-plans skill

---

## 交接文档规范

每个阶段结束时写入的交接文档必须遵循以下规范：

**只写结论，不写过程。**
- ✅ "模块 X 缺少 Y 接口，需要新增 Z"
- ❌ "我们首先检查了 A 文件，然后发现 B，经过分析认为 C……"

**结构化，可被后续阶段直接引用。**
- 使用固定章节标题（见各阶段 skill）
- 关键信息用列表或表格，不用长段落

**自包含，不依赖对话历史。**
- 下一阶段 Claude 实例只看此文件，不看之前的对话
- 文档中必须包含所有必要的上下文

---

## Process Flow

```
[阶段一：需求确认]
  步骤1 探索上下文 → 步骤2 需求澄清 → 步骤3 代码分析(并行)
                                              ↓
                                    写入 phase1-output.md
                                              ↓
[阶段二：方案设计]  读取 phase1-output.md
  步骤4 差距分析 → 步骤5 方案展示(逐节确认) → 步骤6 评审(并行)
                                              ↓
                                    写入 phase2-output.md
                                              ↓
[阶段三：计划输出] 读取 phase2-output.md
  步骤7 实施计划 → 步骤8 设计文档 → 步骤9 调用 writing-plans
```

**终止状态是调用 writing-plans。** 禁止调用 frontend-design、mcp-builder 或其他实现类 skill。

---

## 入口：启动流程

收到功能需求时，直接读取并执行 Phase 1 Skill：

```
read: docs/skills/design-first-phase1.md
```

---

## Key Principles

- **金字塔原理** — 结论先行，再展示支撑依据
- **MECE 原则** — 分析和方案必须相互独立、完全穷尽
- **阶段隔离** — 每阶段通过文档交接，不依赖对话历史，控制上下文规模
- **真并行 Subagent** — 步骤3、6 派发独立 subagent 并行执行，汇总结论而非过程
- **只传结论** — 交接文档只写结论，过程信息不跨阶段传递
- **一次问一个** — 不要同时抛出多个问题
- **优先给选项** — 比开放式问题更容易回答
- **YAGNI 原则** — 从所有设计中去除不必要的功能
- **增量验证** — 展示设计、获得认可，再推进
