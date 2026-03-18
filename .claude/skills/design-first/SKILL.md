---
name: design-first
description: "MUST USE before any feature development. Uses Pyramid Principle and MECE analysis for requirements confirmation, solution design, and implementation planning. Trigger on: '新增功能', '修改功能', '方案设计', '需求分析', 'implement feature', 'design solution', or any request to build/change behavior."
---

# Design First — 金字塔原理驱动的需求设计流程

## 核心目标

增加前期需求确认和方案设计时间，减少后期返工次数。**一次分析到位，交付 100% 可用的功能。**

运用**金字塔原理**：结论先行，再展示支撑依据；自顶向下表达，自底向上思考。
遵循 **MECE 原则**：相互独立、完全穷尽，不遗漏、不重叠。

<HARD-GATE>
Do NOT invoke any implementation skill, write any code, scaffold any project, or take any implementation action until you have presented a design and the user has approved it.
</HARD-GATE>

---

## 流程规模判断（步骤 0）

收到需求后，首先评估规模：

| 规模 | 判断标准 | 流程选择 |
|------|---------|---------|
| 简单 | 单文件、≤20行改动、无架构影响 | 快速通道（3步） |
| 标准 | 多文件、新接口、有架构变更 | 完整流程（9步） |

**快速通道：** 将 9 步合并为 3 个阶段确认，每阶段一次 checkpoint。
- 阶段一：需求确认（合并步骤 1-3）
- 阶段二：方案设计（合并步骤 4-6）
- 阶段三：计划输出（合并步骤 7-9）

**标准流程：** 继续执行下方的完整 9 步。

---

## 架构设计：三阶段隔离 + Subagent 并行

```
Phase 1                    Phase 2                    Phase 3
(步骤 1-3：需求确认)        (步骤 4-6：方案设计)        (步骤 7-9：计划输出)
        |                          |                          |
        ▼                          ▼                          ▼
  步骤1: 探索上下文          步骤4: 差距分析            步骤7: 制定实施计划
  步骤2: 需求澄清            步骤5: 方案展示            步骤8: 编写设计文档
  步骤3: 代码分析 ────►     步骤6: 多角度评审 ────►    步骤9: 转入实施
    [并行 subagent×N]          [并行 subagent×5]
        |                          |                          |
        ▼                          ▼                          ▼
 phase1-output.md  ────►   phase2-output.md  ────►    design.md (最终)
```

**核心设计原则：**
- 每个阶段结束时将结论写入标准化交接文档
- 下一阶段**只读交接文档，不依赖对话历史**
- 步骤 3（代码分析）和步骤 6（方案评审）使用并行 Subagent

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
- **真并行 Subagent** — 步骤 3、6 派发独立 subagent 并行执行
- **只传结论** — 交接文档只写结论，过程信息不跨阶段传递
- **一次问一个** — 不要同时抛出多个问题
- **优先给选项** — 比开放式问题更容易回答
- **YAGNI 原则** — 从所有设计中去除不必要的功能

---

## 完整流程

### 阶段一：需求确认

详见 [references/phase1-workflow.md](references/phase1-workflow.md)

1. **探索项目上下文** — 检查文件、文档、近期提交
2. **需求澄清与 Agent 评审** — 逐一提问，多角色评审
3. **代码现状分析** — Subagent 并行按模块拆解

**输出：** `docs/plans/<topic>-requirements.md` → `docs/plans/<topic>-phase1.md`

### 阶段二：方案设计

详见 [references/phase2-workflow.md](references/phase2-workflow.md)

4. **差距分析与方案制定** — 对比现状与目标，提出 2-3 个方案
5. **方案展示与用户确认** — 逐节展示设计，获得用户认可
6. **方案多角度评审** — 5 个 Subagent 并行评审

**输入：** `docs/plans/<topic>-phase1.md`
**输出：** `docs/plans/<topic>-phase2.md`

### 阶段三：计划输出

详见 [references/phase3-workflow.md](references/phase3-workflow.md)

7. **制定实施计划** — SMART 原则，含验证标准
8. **编写设计文档** — 保存至 `docs/plans/<topic>-design.md`
9. **转入实施阶段** — 准备实施

**输入：** `docs/plans/<topic>-phase2.md`
**输出：** `docs/plans/<topic>-design.md`

---

## 入口：启动流程

收到功能需求时：

1. **判断规模**（简单/标准）
2. **声明**：`【启动 Design First 流程 — 规模：<简单/标准>】`
3. **执行**：`【步骤 1/9 — 探索项目上下文】`
