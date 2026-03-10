# 项目提示词

> 此文件为使用cladue过程中的一些提示词。

```
/keyboard-spec-parser 请根据`docs\KB04122电子定义书V4-20250529.docx`中定义书的内容，转换为功能清单输出到`memory-bank`目录下
```

你是一名经验丰富的嵌入式键盘固件开发工程师。你善于根据功能清单和原理图文件，在本项目基础上开发完整的固件。请阅读`docs\N0046_功能清单.md`了解具体的功能需求，阅读`docs\N0046_原理图.md`了解相关GPIO设置和硬件控制。具体需求如下：

1. 需要完成功能清单中的所有功能，不允许有遗漏。
2. 有任何需求不理解的地方请直接询问，直到所有需求都弄懂。

```
  # N0046 键盘固件开发任务

  ## 角色定义

  你是本项目的嵌入式固件开发工程师，负责在 keyboard-framework 基础上实现 
  N0046 产品的完整固件。请输出实现文档，输出markdown格式到/docs文件夹下

  ## 项目上下文

  - **主控芯片**: PixArt PAR2860QN (蓝牙)、PCT1336QN (触控)
  - **目标平台**: iPadOS 16/17/18
  - **连接方式**: 蓝牙 5.3 单通道
  - **现有代码库**: 已实现矩阵扫描、防抖、HAL接口、无线通信框架等基础模块

  ## 必读文档

  开始前请阅读以下文档获取完整需求和硬件定义：

  1. `docs/N0046_功能清单.md` - 功能需求（P0约60项、P1约25项、P2约5项）  
  2. `docs/N0046_原理图.md` - GPIO配置、LED控制、触控板I2C、背光PWM等  

  ## 工作流程

  ### 阶段一：需求理解
  1. 阅读功能清单，按模块整理功能点
  2. 对照硬件配置验证GPIO分配是否可行
  3. 识别与现有代码库可复用的模块
  4. **列出不清晰或矛盾的需求，主动询问澄清**

  ### 阶段二：设计规划
  1. 制定开发计划，按功能模块拆分任务，我后期需要开多个分支来实现不同模块的功能。
  2. 设计模块依赖关系和接口定义
  3. 确定需要新增/修改的文件清单

  ### 阶段三：实现
  1. 遵循项目已有的模块生命周期：`_setup` → `_init` → `_pre_task` →      
  `_task` → `_post_task`
  2. 按优先级顺序实现功能，每个模块完成后进行测试
  3. 更新 `keyboards/product_config.h` 中的产品配置

  ## 代码规范

  - 遵循 `CLAUDE.md` 中定义的项目结构和编码风格
  - 新驱动放入 `drivers/` 对应子目录
  - 平台相关代码放入 `hal/platforms/par2860/`
  - 产品配置集中在 `keyboards/n0046/`
  - 基于本项目的现有代码库，尽量复用已有模块，避免重复开发。并且遵循最小改动原则，不要随意修改其他模块的代码。

  ## 需求澄清模板

  发现不清晰需求时，使用以下格式询问：

  需求编号: [模块名-功能项]
  当前理解: [你的理解]
  疑问点: [具体问题]
  可选方案: [如有多种理解，列出选项]

  ## 验收标准

  每个功能需满足：
  - [ ] 符合功能清单描述
  - [ ] GPIO配置与硬件文档一致
  - [ ] 有对应的测试用例或验证方法
  - [ ] 代码通过编译，无警告
  
  
  ## 重点
  当前触控板部分还未测试完成，此次不需要实现触控板部分。
  当前OTA功能还未测试完成，此次不需要实现OTA部分。
  自定义键 F6：本次仅实现默认 Siri 功能，自定义功能后续随 APP 一起实现
  软件开关机功能，本次不实现。后续专项实现。
```



```
头脑风暴理解需求
/superpowers:brainstorm
根据需求创建计划
/superpowers:write-plan
执行计划
/superpowers:execute-plan
```



是的，请按照superpower工作流继续，先使用using-git-worktrees 创建隔离工作区，然后使用writing-plans 创建详细的实现计划输出到docs\plans。并使用任务列表标志是否已完成。







[需求描述]：
我现在需要基于ch584平台调试蓝牙的全部功能。需要能正常广播、连接、回连、断联。
[具体细节]：
1、必须要实现的功能包括：广播、连接、回连、重新广播、断联等功能。
2、现在的代码是将蓝牙初始化直接放在了system_hal_init，将系统必须的初始化功能留下，将有关蓝牙的部分挪入驱动中。
3、蓝牙驱动目录在drivers\communication\bluetooth\ch584中，是从现有方案中直接挪过来的，已经包含了基本的蓝牙功能实现。（现有方案地址：D:\Code\CLionProjects\kb04122-13a-wch）
4、官方的蓝牙demo 地址为（D:\Code\CLionProjects\kb04122-13a-wch）
5、官方开发手册地址为（.\docs\marker-output\沁恒低功耗蓝牙软件开发参考手册-rdqj\沁恒低功耗蓝牙软件开发参考手册-rdqj.md）
6、可以运用一切你认为有用的资源，包括但不限于现有代码库、官方文档、或者让我验证功能等。
7、实现一个功能后，就需要编译烧录进去测试，可以使用串口查看输出日志。
[要求]：
1、代码必须遵循现有架构，
2、不能随意修改其他模块的代码。
3、理论上遵循最小改动原则。
4、先分析需求，列出不清晰的地方，主动询问澄清，直到完全理解需求。


本项目的硬件原理图我已经导出网格文件，文件路径为：docs\knowledge\KB04122-13A硬件原理图.tel。请解析这个网格文件，结合本项目的功能需求，输出一个详细的功能清单，列出每个功能点对应的硬件接口和GPIO配置。请将这个功能清单输出到`docs\N0046_原理图.md`文件中，并确保格式清晰易读。


我现在正在测试蓝牙功能，上电后可以发起蓝牙广播，但是在主机断连后并未进入广播回连，而是重新开始广播，我需要分析为什么它没有进入回连广播。以下是串口输出的日志：
B19 HAL initialized
HW Timer: Initialized successfully
hidEmuTaskId:d,centralTaskId:e
Input: Service initialized with task ID 15
HW Timer: Starting timer 0, interval=5ms, ticks=390000
HW Timer: Starting TMR0 with 390000 ticks
Output service init start
Task registered, ID=16
Output service init done
System initialized successfullyInitialized..
Advertising..
Connected..int 18
Pair state 2 ; status 0
Phy update Rx:2 Tx:2 ..
Send Security Req ...
Update 1 - Int 0xc - Latency 4
[DISC] res_num 0
adv state 0
IRK 86 2c 10 9c eb 10
e4 66 e5 47 9c b8
e4 67 e5 47 9c b8
[BOND] check: idx=1 flag=1 bond_flag=1 result=1
[ADV] enable=1 pairing=0 bonded=1 idx=1
[ADV] WHITE LIST mode (reconnect)
ADV timeout timer start 96000
[DISC] save_bond(FALSE) from disconnect handler
[BOND] save: mode=1 is_pairing=0 flag=1 before=1
[BOND] save: after=1
Disconnected.. Reason:13
Advertising..
SEND_DISCONNECT_EVT
ADV timeout -> request deep sleep
[BOND] check: idx=1 flag=1 bond_flag=1 result=1
[ADV] enable=0 pairing=0 bonded=1 idx=1
[ADV] GENERAL mode (pairing or not bonded)
Wireless: ADV timeout, enter deep sleep
可以参考demo工厂蓝牙流程：docs\knowledge\3mode-communication-flow.md



我需要增加CH584的eeprom.h实现，以存储相关配置，具体使用案例请参考 D:\Code\CLionProjects\kb04122-13a-wch\BLE\3mode\KEYBOARD\nvs_flash.c


请输出一份关于当前项目的3模流程。以markdown格式。
现在正基于该3模键盘开发自己的量产项目，现在针对该项目的通讯模块（USB\BLE\2.4G）代码比较混乱。需要重新梳理后封装简化通讯流程。请分析完整代码库，输出3模的完整流程，其中需要展示关键的代码路径。用于给新项目做参考设计。其中包括但不限于：
1、3模的切换和管理流程，每个模块切换时的生命周期。
2、着重分析BLE 模块，包括了广播、回连、断联、蓝牙地址管理。

2


继续实现n0046-branch5-backlight-system-requirements.md。但是该文档需要变更，请根据下面的需求描述重新修改该文件修改完成后再实施。
1. 不直接调用方法，而是通过事件或状态机来控制灯效的变化。状态机在output_process_event。
2. 所有的背光事件，都发送状态机，