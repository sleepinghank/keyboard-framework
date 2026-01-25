# Claude Code AI 编程高阶技巧完全指南

## 目录

1. [概述](#概述)
2. [安装与启动](#安装与启动)
3. [核心命令详解](#核心命令详解)
4. [高级功能](#高级功能)
5. [MCP 模型上下文协议](#mcp-模型上下文协议)
6. [权限与安全](#权限与安全)
7. [自定义扩展](#自定义扩展)
8. [GitHub 集成](#github-集成)
9. [历史与导出](#历史与导出)
10. [可视化工具 CLAUDIA](#可视化工具-claudia)
11. [最佳实践总结](#最佳实践总结)

---

## 概述

### 什么是 Claude Code

Claude Code 是 Anthropic 官方推出的命令行 AI 编程助手，它能够：

- **理解整个代码库**：自动分析项目结构、依赖关系和代码逻辑
- **执行复杂编程任务**：包括代码编写、重构、调试、测试等
- **与开发工具集成**：支持 VS Code、GitHub 等主流开发工具
- **扩展能力**：通过 MCP 协议连接外部服务和工具

### 适用场景

| 场景 | 说明 |
|------|------|
| 项目初始化 | 快速理解新项目代码结构 |
| 代码重构 | 大规模代码改动和优化 |
| Bug 修复 | 定位问题并自动修复 |
| 功能开发 | 根据需求编写新功能 |
| 代码审查 | 检查代码质量和潜在问题 |
| 文档生成 | 自动生成技术文档 |

---

## 安装与启动

### 环境要求

- **Node.js**: 18.0+ 版本
- **操作系统**: Windows / macOS / Linux
- **账户**: Claude Pro 或 Max 订阅用户

### 安装步骤

```bash
# 1. 安装 Node.js (如未安装)
# 访问 https://nodejs.org 下载安装

# 2. 全局安装 Claude Code
npm install -g @anthropic-ai/claude-code

# 3. 验证安装
claude --version
```

### 启动方式

#### 方式一：官方 Pro/Max 用户直接启动

```bash
# 进入项目目录
cd /path/to/your/project

# 启动 Claude Code
claude
```

首次启动会引导登录 Claude 账户进行授权。

#### 方式二：通过 API Router 接入

如果使用第三方 API 代理或自建服务：

```bash
# 设置 API 端点
export ANTHROPIC_API_KEY="your-api-key"
export ANTHROPIC_BASE_URL="https://your-router-endpoint"

# 启动
claude
```

#### 方式三：非交互模式

```bash
# 使用 -p 参数执行单次命令
claude -p "解释这个项目的架构"

# 结合管道使用
cat error.log | claude -p "分析这个错误日志"
```

**作用**：非交互模式使 Claude Code 成为强大的命令行 AI 助手，可集成到脚本和自动化流程中。

---

## 核心命令详解

### `/init` - 项目初始化

#### 介绍

`/init` 命令让 Claude Code 全面扫描并理解项目，生成项目知识库文件。

#### 作用

- 自动遍历所有代码文件
- 分析项目结构和技术栈
- 生成 `CLAUDE.md` 知识文件
- 为后续对话建立上下文基础

#### 实战

```bash
# 进入项目目录后执行
> /init

# Claude Code 会：
# 1. 扫描所有源代码文件
# 2. 识别框架、依赖、配置
# 3. 理解代码组织结构
# 4. 生成项目说明文档
```

**生成的 CLAUDE.md 示例**：

```markdown
# Project Overview
This is a React + TypeScript web application...

## Tech Stack
- Frontend: React 18, TypeScript, Tailwind CSS
- State: Redux Toolkit
- Build: Vite

## Key Files
- src/App.tsx - 主入口组件
- src/store/ - Redux 状态管理
...
```

---

### `/compact` - 上下文压缩

#### 介绍

在长时间对话后，上下文会变得臃肿，`/compact` 可以智能压缩历史记录。

#### 作用

- **提高 AI 专注力**：减少无关信息干扰
- **降低 Token 消耗**：节省 API 调用成本
- **保持核心记忆**：关键信息不丢失

#### 实战

```bash
# 当对话变长时执行
> /compact

# 或指定保留的重点
> /compact 保留关于数据库设计的讨论
```

**使用时机**：
- 对话超过 50 轮时
- 切换到新子任务时
- 感觉 AI 回复质量下降时

---

### `/clear` - 清除对话

#### 介绍

完全清除当前对话记录，开始全新会话。

#### 作用

- 清空所有对话历史
- 重置 AI 状态
- 适合开始全新任务

#### 实战

```bash
# 完成一个任务后，开始新任务
> /clear

# 注意：项目知识（CLAUDE.md）会保留
```

**与 `/compact` 的区别**：

| 命令 | 效果 | 适用场景 |
|------|------|----------|
| `/compact` | 压缩但保留关键信息 | 继续当前任务 |
| `/clear` | 完全清空 | 开始新任务 |

---

### 思考模式 - `think` / `think hard`

#### 介绍

通过特定关键词触发 AI 进行更深度的思考，这是官方支持的功能。

#### 作用

- 增加 AI 推理链长度
- 处理复杂问题时获得更好答案
- 减少 AI "偷懒" 的情况

#### 实战

```bash
# 简单问题
> 解释这个函数的作用

# 复杂问题 - 使用 think
> think 分析这个模块的设计模式是否合理

# 更复杂问题 - 使用 think hard
> think hard 重构这个支付模块，需要考虑并发、幂等性、错误恢复
```

**思考强度级别**：

| 关键词 | 思考深度 | 适用场景 |
|--------|----------|----------|
| (无) | 标准 | 简单查询 |
| `think` | 深入 | 设计决策 |
| `think hard` | 最深 | 架构规划 |
| `ultrathink` | 极限 | 极复杂问题 |

---

### `!` - 命令行模式

#### 介绍

使用叹号 `!` 前缀可以临时切换到命令行模式，执行 shell 命令。

#### 作用

- 快速执行系统命令
- 命令结果自动加入 AI 上下文
- 无需退出 Claude Code

#### 实战

```bash
# 查看 git 状态
> !git status

# 运行测试
> !npm test

# 查看文件内容
> !cat package.json

# 执行后 Claude Code 会分析结果并继续对话
```

**实际场景**：

```bash
> !npm run build
# 输出: Error: Cannot find module 'lodash'

> 帮我修复这个构建错误
# Claude Code 已经知道错误内容，直接给出解决方案
```

---

### `/ide` - IDE 集成

#### 介绍

`/ide` 命令将 Claude Code 与 VS Code 等 IDE 打通，实现深度集成。

#### 作用

- **代码感知**：AI 能看到 IDE 中的诊断信息
- **差异对比**：代码修改以 diff 形式展示
- **快速跳转**：支持点击跳转到代码位置
- **错误同步**：IDE 中的语法错误自动传递给 AI

#### 实战

**安装 VS Code 扩展**：

1. 在 VS Code 中搜索 "Claude Code"
2. 安装官方扩展
3. 在 Claude Code 中执行：

```bash
> /ide
# 选择 VS Code 进行连接
```

**集成后的工作流**：

```bash
# Claude Code 修改代码后
> 修改 src/utils.ts 添加防抖函数

# VS Code 中会显示：
# - 代码差异高亮
# - 左侧显示原代码，右侧显示新代码
# - 可以逐个接受或拒绝改动
```

---

## 高级功能

### Sub Agent - 并行子任务

#### 介绍

Sub Agent 允许 Claude Code 创建多个子任务并行执行，类似多线程处理。

#### 作用

- 同时处理多个独立任务
- 提高复杂任务执行效率
- 每个子任务有独立上下文

#### 实战

```bash
# 查看可用的 Agent 类型
> /agents

# 创建并行任务示例
> 并行执行以下任务：
> 1. 分析 src/api/ 目录的代码质量
> 2. 检查 src/components/ 的组件结构
> 3. 审查 src/utils/ 的工具函数

# Claude Code 会创建 3 个 Sub Agent 同时工作
```

**Agent 类型说明**：

| Agent 类型 | 用途 |
|------------|------|
| Explore | 快速探索代码库 |
| Plan | 制定实施计划 |
| Bash | 执行命令行任务 |
| general-purpose | 通用任务处理 |

---

## MCP 模型上下文协议

### 介绍

MCP (Model Context Protocol) 是 AI 与外部工具交互的标准协议，作为中间层连接 Claude Code 和各种服务。

### 作用

- 扩展 AI 能力边界
- 连接外部数据源
- 调用第三方服务
- 实现自动化工作流

### 安装 MCP Server

```bash
# 查看已安装的 MCP
> claude mcp list

# 安装 MCP server
> claude mcp add <server-name>

# 选择安装级别
# - Project: 仅当前项目可用
# - User: 所有项目可用
```

### 实战：使用 Context7 MCP 查询最新文档

**场景**：需要将 Tailwind CSS 从 v3 升级到 v4

```bash
# 1. 安装 Context7 MCP
> claude mcp add context7

# 2. 查询最新 Tailwind v4 文档
> 使用 context7 查询 Tailwind CSS v4 的迁移指南

# 3. Claude Code 会：
#    - 调用 Context7 API 获取最新文档
#    - 分析当前项目的 Tailwind 使用情况
#    - 生成迁移方案并执行
```

### 删除 MCP Server

```bash
> claude mcp remove context7
```

### 远程 MCP Server

MCP 支持通过网络协议远程调用：

```bash
# SSE 协议
> claude mcp add --transport sse https://your-mcp-server.com/sse

# Streamable HTTP 协议
> claude mcp add --transport http https://your-mcp-server.com/mcp
```

**应用场景**：
- 团队共享的 MCP 服务
- 需要认证的企业内部服务
- 资源密集型任务外包到服务器

---

## 权限与安全

### `/permissions` - 权限管理

#### 介绍

控制 Claude Code 可以执行哪些操作，保护系统安全。

#### 作用

- 限制文件访问范围
- 控制命令执行权限
- 管理 MCP 调用权限

#### 实战

```bash
# 查看当前权限设置
> /permissions

# 添加允许规则
> /permissions allow "Bash(npm *)"  # 允许所有 npm 命令
> /permissions allow "Read(src/*)"   # 允许读取 src 目录

# 添加禁止规则
> /permissions deny "Bash(rm -rf *)" # 禁止危险删除
> /permissions deny "Write(.env)"     # 禁止修改环境变量
```

**权限规则格式**：

```
Tool(pattern)
```

| Tool | 说明 | 示例 |
|------|------|------|
| Bash | Shell 命令 | `Bash(git *)` |
| Read | 读取文件 | `Read(*.ts)` |
| Write | 写入文件 | `Write(src/*)` |
| Edit | 编辑文件 | `Edit(*.md)` |
| MCP | MCP 调用 | `MCP(context7)` |

### 最高权限模式

**警告**：仅在完全信任的环境中使用！

```bash
# 启动时赋予最高权限
claude --dangerously-allow-permissions

# 此模式下 Claude Code 可以：
# - 执行任何命令
# - 读写任何文件
# - 调用任何 MCP
```

---

## 自定义扩展

### 自定义命令

#### 介绍

创建自己的快捷命令，封装常用操作。

#### 创建方法

在 `.claude/commands/` 目录下创建文件，文件名即为命令名：

```bash
# 创建命令目录
mkdir -p .claude/commands

# 创建自定义命令
touch .claude/commands/review.md
```

#### 实战

**创建代码审查命令** (`.claude/commands/review.md`)：

```markdown
# Code Review Command

请对以下代码进行全面审查：

## 审查要点
1. 代码规范性
2. 潜在 Bug
3. 性能问题
4. 安全隐患
5. 可维护性

## 输出格式
- 严重问题
- 建议改进
- 良好实践

请审查：$ARGUMENTS
```

**使用自定义命令**：

```bash
> /review src/utils/payment.ts
```

### Hook 机制

#### 介绍

Hook 让 Claude Code 在特定事件节点自动执行操作。

#### 作用

- 自动格式化代码
- 运行 lint 检查
- 执行测试
- 触发构建

#### 配置方法

在 `.claude/settings.json` 中配置：

```json
{
  "hooks": {
    "PreToolUse": [
      {
        "matcher": "Write|Edit",
        "command": "echo 'About to modify files...'"
      }
    ],
    "PostToolUse": [
      {
        "matcher": "Write|Edit",
        "command": "npx prettier --write $FILE"
      }
    ],
    "Notification": [
      {
        "matcher": ".*",
        "command": "notify-send 'Claude Code' '$MESSAGE'"
      }
    ]
  }
}
```

#### 实战：自动格式化

```json
{
  "hooks": {
    "PostToolUse": [
      {
        "matcher": "Write|Edit",
        "command": "npx prettier --check $FILE && npx eslint $FILE"
      }
    ]
  }
}
```

**Hook 事件类型**：

| 事件 | 触发时机 |
|------|----------|
| PreToolUse | 工具执行前 |
| PostToolUse | 工具执行后 |
| Notification | 通知消息时 |
| Stop | 任务停止时 |

---


## 历史与导出

### `/resume` - 恢复历史对话

#### 介绍

找回之前的对话记录，继续未完成的任务。

#### 实战

```bash
# 查看历史对话列表
> /resume

# 会显示类似：
# 1. [2024-01-15] 重构支付模块
# 2. [2024-01-14] 修复登录 Bug
# 3. [2024-01-13] 添加单元测试

# 按 ESC 可以快速跳转到具体对话
```

### `cc-undo` - 回退操作

```bash
# 回退到上一步
> cc-undo

# 回退多步
> cc-undo 3
```

**注意**：`cc-undo` 不仅回退对话，还会回退代码改动。

### `/export` - 导出对话

#### 介绍

将对话内容导出，用于保存或分享。

#### 实战

```bash
# 导出为 Markdown
> /export

# 导出后可以：
# - 保存为文档
# - 发送给团队成员
# - 用其他 AI 验证结果
```

---

## 最佳实践总结

### 命令速查表

| 命令/操作 | 用途 | 示例 |
|-----------|------|------|
| `/init` | 初始化项目 | `/init` |
| `/compact` | 压缩上下文 | `/compact` |
| `/clear` | 清除对话 | `/clear` |
| `think` | 深度思考 | `think 分析架构` |
| `!` | 执行命令 | `!git status` |
| `#` | 记录记忆 | `# 使用4空格缩进` |
| `/ide` | IDE集成 | `/ide` |
| `/agents` | 子任务 | `/agents` |
| `/permissions` | 权限管理 | `/permissions` |
| `/resume` | 恢复对话 | `/resume` |
| `/export` | 导出对话 | `/export` |
| `claude mcp add` | 添加MCP | `claude mcp add context7` |
| `claude -p` | 非交互模式 | `claude -p "问题"` |

### 工作流建议

#### 新项目启动

```bash
cd your-project
claude
> /init
> # 记录项目规范和偏好
```

#### 日常开发

```bash
# 1. 恢复昨天的对话（如需要）
> /resume

# 2. 开始新任务前清理
> /clear

# 3. 执行开发任务
> 实现用户登录功能

# 4. 定期压缩
> /compact
```

#### 复杂任务处理

```bash
# 1. 使用深度思考
> think hard 设计微服务架构

# 2. 启用并行处理
> /agents

# 3. 检查结果
> !npm test
```

### 注意事项

1. **定期压缩**：长对话记得 `/compact`
2. **善用记忆**：将规范用 `#` 记录
3. **权限控制**：生产环境谨慎授权
4. **及时导出**：重要对话记得 `/export`
5. **版本控制**：大改动前先 `git commit`

---

## 参考资源

- [Claude Code 官方文档](https://docs.anthropic.com/claude-code)
- [MCP 协议规范](https://modelcontextprotocol.io)
- [GitHub CLI 文档](https://cli.github.com/manual)
- [CLAUDIA 项目主页](https://github.com/anthropics/claudia)

---

*本文档基于 Claude Code 最新版本整理，部分功能可能随版本更新有所变化，请以官方文档为准。*
