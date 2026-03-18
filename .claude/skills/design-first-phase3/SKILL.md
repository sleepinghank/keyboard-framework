---
name: design-first-phase3
description: "Phase 3 of design-first: Plan Output (步骤7-9). Reads phase2-output.md as its sole context input. Covers implementation plan (SMART), design document writing with parallel spec review, and handoff to writing-plans skill."
---

# Phase 3 — 计划输出（步骤 7-9）

## 本阶段启动规则

**读取上下文：**
```
read: docs/plans/YYYY-MM-DD-<topic>-phase2-output.md
```

本阶段**只从 phase2-output.md 读取上下文**，不依赖任何前序对话历史。
如果文件不存在，停止并提示用户先完成 Phase 2。

**本阶段上下文已大幅压缩：**
- Phase 1 上下文：phase1-output.md（结论文件，≤50行）
- Phase 2 上下文：phase2-output.md（结论文件，≤60行）
- 不含任何过程信息，上下文清洁，质量有保障

---

## 步骤 7 — 制定实施计划

声明：`【步骤 7/9 — 制定实施计划】`

**从 phase2-output.md 读取：**
- 选定方案
- 架构变更
- 关键接口
- 测试要点

**每个实施步骤遵循 SMART 原则：**

| 原则 | 要求 |
|------|------|
| **S**pecific（具体） | 明确到文件、函数、接口名，不留模糊空间 |
| **M**easurable（可量化） | 定义完成标准和验证命令 |
| **A**chievable（可实现） | 单步可在一次会话内完成 |
| **R**elevant（相关） | 每步直接服务于需求目标 |
| **T**ime-bound（有顺序） | 明确依赖关系和前置条件 |

**计划格式：**

```
## 实施步骤

### 步骤 1：<名称>
- 修改内容：<具体文件/函数>
- 完成标准：<可验证的判定条件>
- 验证命令：<命令或测试用例>
- 前置条件：<依赖的上一步骤>

### 步骤 2：<名称>
...
```

**计划编写规则：**
- 步骤粒度：每步 1-3 小时工作量
- 先写核心路径，再写边界处理
- 每步必须有可执行的验证命令
- 不添加需求范围外的步骤（YAGNI）

> **[CHECKPOINT 7 — 必须暂停]**
>
> 展示完整实施计划。
>
> 明确说："步骤 7 完成，请确认计划后进入步骤 8（编写设计文档）。"
>
> **在此停止，等待用户确认后才能继续。**

---

## 步骤 8 — 编写设计文档（并行 Spec 评审）

声明：`【步骤 8/9 — 编写设计文档】`

### Part A：写入设计文档

将以下内容写入 `docs/plans/YYYY-MM-DD-<topic>-design.md`：

```markdown
# <topic> 设计文档
日期：<YYYY-MM-DD>
状态：待实施

## 一、需求摘要
<来自 phase1-output.md 的需求摘要>

## 二、范围边界
在范围内：[...]
不在范围内：[...]

## 三、现状分析
<来自 phase1-output.md 的代码现状结论>

## 四、方案设计
### 选定方案
<方案描述>

### 架构变更
<来自 phase2-output.md>

### 关键接口
<来自 phase2-output.md>

### 数据流
<来自 phase2-output.md>

### 错误处理
<来自 phase2-output.md>

## 五、测试策略
<来自 phase2-output.md 的测试要点>

## 六、实施计划
<来自步骤7的完整实施计划>

## 七、风险与规避
<来自 phase2-output.md 的评审结论中的主要风险>
```

### Part B：并行 Spec 评审

文档写入后，同时派发 3 个 Spec Reviewer Subagent 并行检查：

```
Reviewer 1 — 完整性检查
  检查：七个章节是否都有内容？是否有空白或占位符？
  输出：✅通过 / ❌缺失列表

Reviewer 2 — 一致性检查
  检查：实施计划的步骤是否与方案设计对齐？接口定义是否一致？
  输出：✅通过 / ❌不一致列表

Reviewer 3 — 可执行性检查
  检查：实施步骤的验证命令是否具体可执行？完成标准是否可判定？
  输出：✅通过 / ❌模糊项列表
```

**评审循环规则：**
1. 收集三个 Reviewer 结论
2. 如有 ❌ 问题：修正文档，重新并行派发评审，直至全部 ✅
3. 循环超过 5 次：升级给用户处理，说明卡点

> **[CHECKPOINT 8 — 必须暂停]**
>
> 文档写入并通过全部评审后，展示评审通过报告，告知用户文档路径。
>
> 明确说："设计文档已完成，路径：`docs/plans/YYYY-MM-DD-<topic>-design.md`。是否继续步骤 9（转入实施）？"
>
> **在此停止，等待用户确认后才能继续。**

---

## 步骤 9 — 转入实施

声明：`【步骤 9/9 — 转入实施】`

**执行检查：**
- [ ] phase1-output.md 已存在
- [ ] phase2-output.md 已存在
- [ ] design.md 已存在且通过评审
- [ ] 用户已确认进入实施

**调用下一个 skill：**
```
read: docs/skills/writing-plans.md
```

并告知用户：
```
阶段一至三已全部完成。设计文档位于：
  docs/plans/YYYY-MM-DD-<topic>-design.md

现在移交 writing-plans skill 创建详细实施计划。
```

**禁止调用：** frontend-design、mcp-builder 或其他任何实现类 skill。
writing-plans 是本流程结束后唯一调用的 skill。

---

## 产出文件清单

| 文件 | 用途 | 消费者 |
|------|------|--------|
| `requirements.md` | 需求+现状分析（含 Agent 评审） | Phase 2 参考 |
| `phase1-output.md` | Phase 1 结论交接 | Phase 2 主输入 |
| `phase2-output.md` | Phase 2 结论交接 | Phase 3 主输入 |
| `design.md` | 完整设计文档 | writing-plans 输入 |

---

## 上下文管理总结

本阶段上下文组成：
- phase2-output.md（≤60行，仅结论）
- 步骤7实施计划（本阶段产生，约30行）
- design.md（汇总文档，引用前两者内容）

**整个三阶段流程中，任意单阶段的对话上下文峰值 ≤ 当前阶段内容 + 两个交接文档（≤110行）**，相比原始单线程方案（所有过程堆积在同一对话）减少约 70-80% 的上下文占用。
