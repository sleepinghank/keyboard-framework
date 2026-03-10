# Claude Code 公司培训演讲文稿设计文档

## 项目概述

**培训对象**：跨境电商公司150+员工（运营、人事、财务等非技术人员）
**培训时长**：30-45分钟
**培训目标**：让员工安装并能基本使用Claude Code
**API Key**：公司统一提供

## 演讲结构

| 部分 | 时长 | 内容要点 |
|------|------|----------|
| 一、开场引入 | 5分钟 | 禅道案例对比（3小时 vs 3分钟），建立期待 |
| 二、Claude Code 基本介绍 | 10分钟 | 金字塔原理：背景→冲突→问题→答案，用"大脑+手脚"比喻 |
| 三、实操演示 | 20分钟 | 安装→配置→对话（/clear）→项目（/init）→实际任务（/compact） |
| 四、工作案例讲解 | - | 由其他IT同事负责，展示具体岗位应用 |
| 五、收尾与行动号召 | 5分钟 | 回顾+岗位场景+行动步骤+结束语 |

## 各部分详细设计

### 一、开场引入（5分钟）

**设计思路**：用强烈的时间对比抓住注意力，让听众在30秒内产生好奇心。

**内容结构**：
1. 30秒悬念开场："如果告诉你，一个原本需要3小时的工作，现在只需要3分钟——而且不需要你会编程，你信吗？"
2. 禅道批量导入案例演示（Before/After对比）
3. 金句收尾："今天培训结束后，你也能做到。"

### 二、Claude Code 基本介绍（10分钟）

**设计思路**：遵循金字塔原理，用通俗比喻让非技术人员理解核心价值。

**内容结构**：
1. 背景：AI已经走进我们的生活（DeepSeek、Kimi、ChatGPT）
2. 冲突：这些AI只能"说"，不能"做"
3. 比喻对比表：DeepSeek（大脑）vs Claude Code（大脑+手脚）
4. 问题：为什么是Claude Code？（Anthropic公司背景、行业影响）
5. 答案：Claude Code 就是你的"数字员工"

**参考数据**：
- Anthropic由OpenAI前核心团队（Dario Amodei等）于2021年创立
- Claude 4被评为世界最佳编程模型
- Anthropic CEO表示AI可能影响50%初级白领工作
- Claude Code被Google工程师青睐

### 三、实操演示（20分钟）

**命令融入实操**：边用边学，配合记忆口诀。

| 步骤 | 内容 | 引入命令 | 口诀 |
|------|------|----------|------|
| 安装启动 | npm安装，启动Claude | - | - |
| 配置Key | 粘贴公司提供的API Key | - | - |
| 第一次对话 | 自我介绍、读取文件 | /clear | "清空" |
| 了解项目 | 进入工作文件夹 | /init | "认路" |
| 完成任务 | 批量处理文件 | /compact | "整理" |

**命令速查表**：
| 命令 | 什么时候用 | 记忆口诀 |
|------|------------|----------|
| /init | 开始新项目时 | "认路" |
| /clear | 想重新开始时 | "清空" |
| /compact | 对话太长时 | "整理" |
| /skill | 使用预设工作流 | "大招" |

### 四、工作案例讲解（其他同事负责）

**内容**：由IT同事讲解几个工作中的具体案例，展示不同岗位的应用场景。

### 五、收尾与行动号召（5分钟）

**内容结构**：
1. 快速回顾：一句话总结 + 三个关键命令
2. 岗位场景举例：运营、人事、财务、通用
3. 行动号召：领取API Key → 安装 → 试着用一次
4. 结束语：AI是解放而非威胁

## 核心设计原则

1. **震撼开场**：用数字对比抓住注意力
2. **通俗比喻**：大脑 vs 大脑+手脚
3. **命令融入实操**：边用边学，配合记忆口诀
4. **岗位相关**：每个部门都能看到应用场景
5. **降低门槛**：反复强调"不需要会编程"

## 参考资料来源

- [Anthropic Claude 4 发布](https://www.anthropic.com/news/claude-4)
- [WIRED: Claude Code如何改变软件和Anthropic](https://www.wired.com/story/claude-code-success-anthropic-business-model/)
- [Pragmatic Engineer: Claude Code如何构建](https://newsletter.pragmaticengineer.com/p/how-claude-code-is-built)
- [Medium: Claude Code成为Google工程师首选](https://tasmayshah12.medium.com/claude-code-how-a-side-project-became-the-ai-coding-tool-google-engineers-prefer-in-2025-73aaa6a54371)
- [Index.dev: DeepSeek vs Claude对比](https://www.index.dev/blog/deepseek-vs-claude-ai-comparison)
- [Vox: AI代理与白领工作](https://www.vox.com)

## 输出要求

最终输出：Markdown格式的演讲文稿

要求：
1. 深入浅出，面向非编程人员
2. 内容全面
3. 引用资料准确
