# AI Coding

## 分享大纲

### 一、开场引入

1. 自我介绍
2. 分享主题
3. 为什么要分析这个主题
4. 大纲介绍

### 二、Claude Code 简介

什么是claude code，特点极其背景

和市面上的其他编程工具对比

主要功能和特点

### 三、核心功能

如下为介绍claude code ai编程详细攻略内容，请根据如下结构和内容扩充并整理一份技术分享文档，要求内容有介绍，作用，实战等。输出markdown格式

🚀00:00 介绍30个Cloud Code高阶技巧，一期视频精通。

💻00:20 安装Node.js，复制Cloud Code安装命令，使用CD命令跳转到项目目录。 

🔑00:35 Cloud官网Pro/Max用户直接输入`cloud`启动，或通过Cloud Code Router接入API。 

📚01:03 `\INIT`命令让Cloud Code通读所有文件，保存项目知识到`.cloud.md`文件。 

🧹01:39 `\COMPACT`压缩上下文，提高AI专注力，降低token消耗。 

🗑️01:59 `\CLEAR`清除对话记录，开始新任务时使用。 

🧠02:13 使用`think think hard`等增加AI思考长度，官方支持。 

!02:37 叹号`!`切换到命令行模式，执行临时命令，结果加入上下文。 

#03:05 井号`#`进入记忆模式，记录为文件，作为AI长期记忆，可选择项目或用户级别。 

🔗03:58 `\IDE`命令打通Cloud Code与IDE（如VS Code），实现代码感知和修改对比。

🤖04:49 非交互模式使用`cloud -p`，使Cloud Code成为命令行AI助手。 

🧰05:13 MCP (模型上下文协议) 作为AI与外部工具的中间层，使用`cloud mcp add`安装，可选择项目或用户级别。 

🔎06:20 演示使用Context7 MCP查找最新代码文档，并升级Talwind项目。 

❌07:11 使用`cloud mcp remove`删除MCP server。

📡07:24 可以通过SSE或STREAMABLEHTTP协议远程调用MCP server。 

🔒08:19 `\permissions`自定义权限规则，允许或禁止Cloud Code使用工具或MCP。 

🚩09:28 启动Cloud Code时添加`--dangerously-allow-permissions`赋予最高权限。 

🛠️09:56 可自定义命令，在`.cloud/commands`文件夹下创建文件，文件名为命令名。 

⚙️11:29 Hook让Cloud Code在特定节点执行操作，如使用Prettier检查代码格式，通过`settings.json`配置。 

🧵13:09 Sub Agent类似子线程，可并行执行多个子任务，提高效率，通过`\agents`创建。

🌍15:25 Cloud Code与GitHub集成，借助GitHub CLI执行操作，演示读取issue并自动修改。 

🔙16:41 `\resume`找回历史话题，敲击ESC跳转到具体对话，cc-undo可回退对话和代码。 

📤17:26 `\export`导出对话内容，可保存或用于其他AI验证。 

✨17:47 CLAUDIA是基于Cloud Code的可视化桌面应用，提供安装包，需配置API环境变量（如CCR）。 

⏱️20:06 CLAUDIA提供时间导航线和检查点功能，可回退文件改动和对话历史。 

✅21:02 总结30个Cloud Code进阶技巧，建议参考官方文档。

### 四、实战演练

以键盘嵌入式固件开发为案例，将之前先来分析一下**现有工作流**

接收定义书

分析定义书形成大概的功能清单

接收原理图

分析原理图

创建项目

配置gpio口

分模块实现功能

刷入固件调试。

修复问题

自测

输出提测。

**现在是怎么做的**

接收定义书

输入定义书 运用skill 形成功能清单

检查清单

接收原理图

输入原理图 形成硬件接口清单

检查清单

结合输入，形成需求清单

检查清单

根据清单形成计划

根据计划逐一实施。















