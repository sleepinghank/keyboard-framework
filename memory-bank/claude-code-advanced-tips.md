# Claude Code 30个高阶使用技巧完整指南

> 本文整理自视频教程：《最火AI编程Claude Code详细攻略，一期视频精通》
>
> 视频来源：技术爬爬虾 (Bilibili: BV1XGbazvEuh)
>
> 整理日期：2026-01-25

---

## 目录

1. [第一部分：安装与设置](#第一部分安装与设置)
2. [第二部分：核心命令与交互模式](#第二部分核心命令与交互模式)
3. [第三部分：与 VS Code 的无缝集成](#第三部分与-vs-code-的无缝集成)
4. [第四部分：自动化与外部知识扩展](#第四部分自动化与外部知识扩展)
5. [第五部分：精细化控制与个性化定制](#第五部分精细化控制与个性化定制)
6. [第六部分：高级协作与开发集成](#第六部分高级协作与开发集成)
7. [最终章：便捷操作与历史追溯](#最终章便捷操作与历史追溯)
8. [附录：CLAUDE.md 记忆管理最佳实践](#附录claudemd-记忆管理最佳实践)

---

## 第一部分：安装与设置

### 1.1 环境准备 - 安装 Node.js

在安装 Claude Code 之前，请确保您的电脑已经安装了 Node.js。

```bash
# 检查 Node.js 是否已安装
node -v
```

如果看到版本号信息，说明 Node.js 已成功安装。

### 1.2 安装 Claude Code

使用 npm（Node.js 包管理器）进行全局安装：

```bash
# 全局安装 Claude Code
npm install -g @anthropic-ai/claude-code
```

> **说明**: `-g` 参数表示进行全局安装，安装完成后，您可以在任何项目中使用 Claude Code。

### 1.3 启动 Claude Code

安装完成后，使用 `cd` 命令跳转到需要编写代码的项目目录。

**方式一：使用官方 API 密钥（推荐）**

```bash
# 直接启动，然后选择登录官网
claude
```

启动后选择第一项，去官网登录即可。

**方式二：通过第三方 API 启动**

```bash
cd your-project-folder

# 设置环境变量
export ANTHROPIC_API_KEY="sk-xxxxxxxxxxxxxxxxx"
export ANTHROPIC_MODEL="claude-3-5-sonnet-20241022"
export CLAUDE_MAX_TOKENS="4096"
export CLAUDE_TEMPERATURE="0.7"
export ANTHROPIC_BASE_URL="https://api.anthropic.com"
export CLAUDE_TIMEOUT="3000"
export CLAUDE_MAX_RETRIES="3"

# 启动 Claude Code
claude
```

### 1.4 初始化项目

安装并启动成功后，进入您的项目文件夹，让 Claude Code 开始学习您的项目：

```bash
# 在 Claude Code 交互界面中输入
/init
```

> **作用**: 这个关键命令会扫描整个项目的文件结构和代码，为AI建立初始的项目认知。

### 1.5 理解项目上下文文件 (`CLAUDE.md`)

`/init` 命令执行完毕后，Claude Code 会在您的项目根目录下创建：
- 一个名为 `.claude` 的隐藏文件夹
- 其中包含一个 `CLAUDE.md` 文件

**CLAUDE.md 示例内容：**

```markdown
# 项目概述
本项目是一个键盘固件框架...

## 技术栈
- 语言: C
- 构建工具: CMake
- 目标平台: CH584M, PAR2860

## 代码规范
- 使用 4 空格缩进
- 函数命名使用 snake_case
```

> **提示**: 您可以随时手动编辑和补充这个文件，添加项目目标、重要模块说明、代码规范等关键信息。

---

## 第二部分：核心命令与交互模式

### 2.1 管理对话上下文 - `/compact` 命令

当对话变得冗长时，使用 `/compact` 命令压缩总结对话内容：

```bash
/compact
```

**作用**:
- 智能总结之前的交流内容
- 形成更简洁的上下文
- 节省 Token 消耗
- 让后续指令更清晰

### 2.2 开启全新任务 - `/clear` 命令

当需要切换到完全不相关的任务时：

```bash
/clear
```

**作用**: 清空当前会话历史，相当于开启一个全新的对话窗口。

### 2.3 提升AI思考深度 - 思维长度修改器

面对复杂的逻辑推理或代码生成任务，可以使用思维长度修改器：

| 修改器 | 思考深度 | 适用场景 |
|--------|----------|----------|
| `think` | 标准 | 一般问题 |
| `think hard` | 较深 | 复杂逻辑 |
| `think harder` | 深度 | 架构设计 |
| `ultrathink` | 最深 | 超复杂问题 |

**使用示例：**

```
ultrathink 请帮我重构这个模块的架构，考虑可扩展性和性能
```

### 2.4 临时执行终端命令 - `!` 前缀

在对话过程中快速执行终端命令：

```bash
# 查看文件列表
!ls -l

# 安装依赖
!npm install

# 运行测试
!npm test
```

**特点**:
- 直接在当前环境下执行
- 结果输出到对话中
- 结果会成为AI的临时上下文

### 2.5 赋予AI长期记忆 - `#` 前缀

将关键信息存入 AI 的长期记忆中：

```bash
# 本项目使用 React 18 和 TypeScript 5.0
# 所有组件必须使用函数式组件和 Hooks
# 代码必须通过 ESLint 检查才能提交
```

**作用**: 这些信息会被永久记录在项目的 `CLAUDE.md` 文件中，成为AI处理所有未来请求时的基础知识。

---

## 第三部分：与 VS Code 的无缝集成

### 3.1 安装 Claude Code 官方扩展

1. 打开 VS Code
2. 点击左侧的扩展图标
3. 搜索 "Claude Code"
4. 找到官方发布的扩展并点击安装

### 3.2 连接终端与编辑器 - `/ide` 命令

在正在运行 Claude Code 的终端会话中执行：

```bash
/ide
```

**作用**: 通知 Claude Code 开始与 VS Code 扩展进行通信，将两者连接起来。

### 3.3 智能感知选中代码

连接成功后，Claude Code 能实时感知到您在 VS Code 编辑器中选中的代码块：

1. 在 VS Code 中选中任意一段代码
2. 直接到终端中向 Claude Code 提问
3. 无需复制粘贴，它已经知道您问的是哪部分代码

### 3.4 可视化代码修改 - Diff 视图

当让 Claude Code 修改代码时，修改建议会以 Diff 形式呈现在 VS Code 中：

- 清晰看到每一行代码的变化
- 可以选择接受或拒绝修改
- 代码审查变得简单和安全

---

## 第四部分：自动化与外部知识扩展

### 4.1 非交互模式 - `-P` 参数

让 AI 独立完成一次性任务：

```bash
# 基本用法
claude -P "请为 src/utils 目录下的所有函数添加 JSDoc 注释"

# 复杂任务示例
claude -P "分析 package.json 中的依赖，找出可能存在安全漏洞的包"
```

**特点**:
- 启动、执行、输出结果、自动退出
- 适合编写自动化脚本
- 适合执行耗时较长的后台任务

### 4.2 模型上下文协议 (MCP) - 为 AI 安装"插件"

MCP 就像是为 Claude Code 设计的插件系统，可以连接外部工具和数据源。

**MCP 能做什么：**
- 连接最新的官方文档
- 访问数据库
- 调用外部 API
- 获取实时信息

**使用 MCP 后的效果：**
- 不再受限于过时的训练数据
- 能够获取实时、准确的信息
- 可以根据最新的官方文档来升级项目代码

### 4.3 常用 MCP 服务器

| MCP 服务器 | 功能 |
|-----------|------|
| `context7` | 获取最新框架文档 |
| `filesystem` | 文件系统操作 |
| `github` | GitHub API 集成 |
| `postgres` | PostgreSQL 数据库 |
| `web-search` | 网络搜索 |

---

## 第五部分：精细化控制与个性化定制

### 5.1 查看当前权限设置 - `/permissions`

```bash
/permissions
```

查看所有工具和命令的权限设置。

### 5.2 自动授权常用工具 - `allow` 规则

将可信工具加入"白名单"：

```bash
# 允许自动执行 eslint 修复
permissions:allow eslint --fix

# 允许自动执行测试
permissions:allow npm test
```

### 5.3 禁止危险操作 - `deny` 规则

创建"黑名单"，阻止危险操作：

```bash
# 禁止删除操作
permissions:deny rm -rf

# 禁止强制推送
permissions:deny git push --force
```

### 5.4 创建自定义命令

**第一步：创建命令文件**

在项目的 `.claude/commands/` 目录下创建 Markdown 文件：

```
.claude/
└── commands/
    ├── test.md      # 对应命令 /test
    ├── deploy.md    # 对应命令 /deploy
    └── review.md    # 对应命令 /review
```

**第二步：编写命令逻辑**

```markdown
<!-- .claude/commands/test.md -->
请对 $arguments 执行以下测试流程：

1. 运行单元测试
2. 检查代码覆盖率
3. 生成测试报告
4. 如果测试失败，分析失败原因并给出修复建议
```

**第三步：使用自定义命令**

```bash
/test src/components/Button.tsx
```

---

## 第六部分：高级协作与开发集成

### 6.1 子代理 (Subagents) - 拆解复杂任务

**创建专职的子代理：**

```bash
# 创建前端代理
/subagent create frontend-agent "负责所有前端组件的开发、样式编写和状态管理"

# 创建后端代理
/subagent create backend-agent "负责 API 开发、数据库操作和服务端逻辑"

# 创建测试代理
/subagent create test-agent "负责编写和维护测试用例，确保代码质量"
```

**分配顶层任务：**

```
请开发一个完整的用户登录注册功能，包括：
- 前端登录/注册表单
- 后端认证 API
- 数据库用户表设计
- 完整的测试覆盖
```

> **效果**: 主 AI 会智能地将任务分解，并分配给最合适的子代理并行处理。

### 6.2 GitHub 集成

**前提条件**: 安装并登录 GitHub 官方命令行工具 (`gh`)

**让 AI 直接读取 GitHub Issue：**

```
请查看并修复这个 Issue: https://github.com/user/repo/issues/42
```

Claude Code 会：
1. 自动读取 Issue 的全部上下文
2. 理解标题、描述、标签和评论
3. 在本地进行代码修改
4. 编写符合规范的 Commit Message
5. 将代码分支推送到 GitHub

**Commit Message 示例：**

```
fix: resolve authentication timeout issue (#42)

- Increase token expiry time to 24 hours
- Add retry logic for failed auth requests
- Update error handling for edge cases
```

---

## 最终章：便捷操作与历史追溯

### 7.1 随时回到过去 - `/resume` 命令

```bash
/resume
```

**作用**:
- 列出最近的几个对话主题
- 选择其中一个立即恢复到当时的上下文
- 无缝衔接此前的思路

### 7.2 记录与分享 - `/export` 命令

```bash
/export
```

**作用**: 当前会话的全部内容以 Markdown 格式复制到剪贴板，方便归档和分享。

### 7.3 图形化界面 - Claude Desktop

Claude Desktop 是一个专为 Claude Code 打造的开源图形化界面，功能包括：

- 可视化的项目管理
- 图形化的高级配置
- 编辑 AI 的项目记忆（`CLAUDE.md`）
- 添加 MCP 外部工具
- 创建自定义命令

### 7.4 "时间航线"功能

Claude Desktop 的独特功能，为 AI 协作过程引入版本控制能力：

- 同时保存项目文件状态和对话历史
- 可以一键回滚到之前的任意检查点
- 放心大胆地进行任何尝试

---

## 附录：CLAUDE.md 记忆管理最佳实践

### A.1 三级记忆体系

| 内存类型 | 文件路径 | 核心定位 | 使用场景 |
|----------|----------|----------|----------|
| **用户内存** | `~/.claude/CLAUDE.md` | 个人编码哲学 | 个人代码风格、Git Commit 格式、常用工具快捷指令 |
| **项目内存** | `./CLAUDE.md` | 团队共同语言 | 项目技术栈、API设计规范、部署测试流程 |
| **项目本地内存** | `./CLAUDE.local.md` | 个人项目配置（已废弃） | 本地环境URL、测试数据 |

### A.2 文件搜索机制

Claude Code 会从当前终端工作目录开始，向上递归搜索，直到系统根目录，加载沿途发现的所有 `CLAUDE.md` 文件。

### A.3 使用 `@import` 构建模块化配置

**基础用法：**

```markdown
# CLAUDE.md

## 项目概览
本项目是一个基于 Next.js 的电商网站。详细信息请参考：@README.md

## 技术规范
- **编码规范**: @./docs/coding-style.md
- **API 文档**: @./docs/api-reference.md
- **可用脚本**: 请参考 @package.json 中的 "scripts" 部分。
```

**高级用法：组合不同来源的配置**

```markdown
# ./CLAUDE.md (项目记忆)

# 导入团队共享的 React 最佳实践
@/path/to/shared/react-best-practices.md

# 导入我个人的项目特定快捷指令
@~/.claude/my-project-shortcuts.md
```

**技术限制：**
- 最大支持 5 层递归导入
- 代码块内 `@import` 不会生效

### A.4 快速编辑方法

**`#` 快捷添加：**

```bash
# 在创建组件时，总是同时创建对应的测试文件。
```

Claude 会询问您希望将其保存在哪个记忆文件中。

**`/memory` 命令：**

```bash
/memory
```

在系统默认编辑器中打开相关的记忆文件进行编辑。

### A.5 最佳实践

1. **指令要具体**
   - ✅ 推荐: `使用 Prettier 格式化代码，缩进 2 空格。`
   - ❌ 不推荐: `请保持代码整洁。`

2. **结构化组织**: 使用 Markdown 的标题和列表来组织记忆文件

3. **定期审查**: 每个迭代或每个月回顾并更新 `CLAUDE.md`

4. **善用注释**: 使用 `<!-- 注释 -->` 解释复杂指令的原因

---

## 命令速查表

| 命令/前缀 | 功能 | 示例 |
|-----------|------|------|
| `/init` | 初始化项目认知 | `/init` |
| `/compact` | 压缩对话上下文 | `/compact` |
| `/clear` | 清空会话历史 | `/clear` |
| `/ide` | 连接 VS Code | `/ide` |
| `/permissions` | 查看权限设置 | `/permissions` |
| `/resume` | 恢复历史对话 | `/resume` |
| `/export` | 导出对话内容 | `/export` |
| `/memory` | 编辑记忆文件 | `/memory` |
| `!` | 执行终端命令 | `!npm test` |
| `#` | 添加长期记忆 | `# 使用 TypeScript` |
| `think hard` | 深度思考模式 | `think hard 重构这个模块` |
| `ultrathink` | 最深度思考 | `ultrathink 设计系统架构` |
| `-P` | 非交互模式 | `claude -P "任务描述"` |

---

## 总结

通过掌握这 30 个高阶技巧，您可以将 Claude Code 从一个简单的代码助手，转变为：

1. **智能编程伙伴** - 深度理解您的项目和编码风格
2. **自动化平台** - 处理重复性任务和后台作业
3. **团队协作工具** - 与 GitHub 无缝集成，参与开发流程
4. **可扩展系统** - 通过 MCP 连接外部工具和数据源

记住：`CLAUDE.md` 是一个需要持续维护和演进的"活文档"，定期更新它将最大化 AI 辅助开发的潜能。

---

*本文档整理自技术爬爬虾的视频教程，完整视频请访问：*
*https://www.bilibili.com/video/BV1XGbazvEuh/*
