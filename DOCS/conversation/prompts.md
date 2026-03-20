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


[需求描述]：
我现在需要基于ch584平台实现所有蓝牙驱动功能，驱动接口地址为drivers\communication\bluetooth\bt_driver.h。请根据如下需求，审核2026-03-10-ch584-bluetooth-driver-design.md 和 2026-03-10-ch584-bluetooth-driver-implementation.md两个文档，确认设计和实现方案是否满足需求。
[具体细节]：
1、蓝牙驱动目录在drivers\communication\bluetooth\ch584中，是从现有方案中直接挪过来的，已经包含了基本的蓝牙功能实现。（现有方案地址：D:\Code\CLionProjects\kb04122-13a-wch）
2、官方的蓝牙demo 地址为（D:\Code\CLionProjects\kb04122-13a-wch）
3、官方开发手册地址为（.\docs\marker-output\沁恒低功耗蓝牙软件开发参考手册-rdqj\沁恒低功耗蓝牙软件开发参考手册-rdqj.md）
6、可以运用一切你认为有用的资源，包括但不限于现有代码库、官方文档、或者让我验证功能等。
7、完整的实现一个功能后，就需要编译烧录进去测试，可以使用串口查看输出日志。
[要求]：
1、代码必须遵循现有架构，
2、不能随意修改其他模块的代码。
3、理论上遵循最小改动原则。
4、先分析需求，列出不清晰的地方，主动询问澄清，直到完全理解需求。
5、必须理清现有代码结构，明确需要修改的文件和新增的文件，做好代码规划。


1、同步
2、远程调试

[需求描述]：
将矩阵扫描定时器由OSAL 调度改为 main loop 标志位执行。减少调度压力和延迟。
[具体细节]：
1.去掉INPUT_MATRIX_SCAN_EVT事件，改为在main loop（application\main.c）中通过一个标志位来控制矩阵扫描的执行。
2.在matrix_scan_timer_callback中设置这个标志位，表示需要执行矩阵扫描。
3.在main loop中检查这个标志位，如果被设置了就执行keyboard_task，并在执行完后清除这个标志位。
4.需要确保这个标志位的访问是线程安全的，可能需要使用volatile关键字或者其他同步机制来防止竞态条件。
[要求]：
1. 代码必须遵循现有架构，
2. 不能随意修改其他模块的代码。
3. 理论上遵循最小改动原则。
4. 先分析需求，列出不清晰的地方，主动询问澄清，直到完全理解需求。

当前层切换实现过于复杂，暂时去掉Fn层切换功能键功能，只保留按照不同系统切换层的功能。



重要：先深入分析问题，给出解决方法，禁止直接修改代码。
按键后主机不出字符，怀疑是报告描述符和报告结构没有对应上，请检查一下是什么问题。打印日志如下：

重要：先深入分析问题，给出解决方法，禁止直接修改代码。
现在测试遇到一个关于休眠功耗的bug。
正常状态下。上位机关闭触控板后功耗会降到4.1-5.2mA跳动，符合预期。异常情况，当滑动触控板，主控还未进入IDLE状态时，上位机关闭触控板，进入idle状态后，功耗会恒定在5.2mA。并不会继续下降。
请帮我分析一下可能是哪里的原因，或者告诉我怎样快速排查问题。
异常日志如下：
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=41
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=40
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=39
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=38
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=37
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=36
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=35
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=34
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=33
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=32
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=31
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=30
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=29
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=28
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=27
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=26
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=25
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=24
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=23
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=22
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=21
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=20
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=19
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=18
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=17
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=16
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=15
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=14
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=13
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=12
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=11
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=10
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=9
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=8
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=7
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=6
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=5
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=4
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=3
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=2
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=1
DEBUG:SYS_IDLE
DEBUG:_touch_pd_cnt=0
DEBUG:Touch power off
DEBUG:SYS_IDLE
DEBUG:SYS_IDLE
DEBUG:SYS_IDLE
DEBUG:SYS_IDLE
DEBUG:SYS_IDLE
DEBUG:SYS_IDLE
DEBUG:SYS_IDLE
DEBUG:[TIMEOUT] backlight timeout, turning off

DEBUG:back_sleep_state=0

INFO:
--------------------Backlight_Disable[TIMER] disable[5]

DEBUG:SYS_IDLE
DEBUG:SYS_IDLE
DEBUG:SYS_IDLEDEBUG:SYS_IDLE
DEBUG:SYS_IDLE
DEBUG:SYS_IDLE
DEBUG:SYS_IDLE
DEBUG:SYS_IDLE
DEBUG:SYS_IDLE
DEBUG:SYS_IDLE
INFO:[TIMEOUT] sleep timeout, entering sleep mode
[TIMER] disable[0]
[TIMER] === SWITCH: 5ms->5000ms, sync_sys=170146, cnt=24 ===
[TIMER] switch interval to 5000 ms (163840 ticks)
[TIMER] disable[1]

INFO:SYS_ENTER_SLA_LCY

DEBUG:SYS_IDLE
DEBUG:SYS_IDLE
DEBUG:SYS_IDLE
DEBUG:SYS_IDLE
DEBUG:SYS_IDLE




HID 报告描述符如下，请分析一下该报告的数据结构是怎么样的，我需要确认一下发送数据是否正确
0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
0x09, 0x06,        // Usage (Keyboard)
0xA1, 0x01,        // Collection (Application)
0x85, 0x01,        //   Report ID (1)
0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
0x19, 0xE0,        //   Usage Minimum (0xE0)
0x29, 0xE7,        //   Usage Maximum (0xE7)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x01,        //   Logical Maximum (1)
0x75, 0x01,        //   Report Size (1)
0x95, 0x08,        //   Report Count (8)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x95, 0x01,        //   Report Count (1)
0x75, 0x08,        //   Report Size (8)
0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x95, 0x05,        //   Report Count (5)
0x75, 0x01,        //   Report Size (1)
0x05, 0x08,        //   Usage Page (LEDs)
0x19, 0x01,        //   Usage Minimum (Num Lock)
0x29, 0x05,        //   Usage Maximum (Kana)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x95, 0x01,        //   Report Count (1)
0x75, 0x03,        //   Report Size (3)
0x91, 0x01,        //   Output (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x95, 0x06,        //   Report Count (6)
0x75, 0x08,        //   Report Size (8)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x65,        //   Logical Maximum (101)
0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
0x19, 0x00,        //   Usage Minimum (0x00)
0x29, 0x65,        //   Usage Maximum (0x65)
0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              // End Collection

// 65 bytes


KB04122-13A-WCH_code.xml这个文件包含了仓库中所有文件的合并内容。请帮我分析一下，低功耗休眠策略，特别是HAL_SLEEP宏的作用。
  将具体流程梳理出来绘制成markdown表格。


docs\code_demo\qmk_firmware_code.xml这个文件包含了仓库中所有文件的合并内容。请帮我分析一下，lpm文件的作用。


我现在需要在本项目实现本地的idle休眠方案、深度休眠方案。需要参考KB04122-13A-WCH_code.xml中关键的休眠流程，然后结合本地代码框架，制定一个详细的休眠方案设计文档，输出到docs\N0046_sleep_strategy.md中。需要明确以下内容：
1. 休眠方案的总体设计思路和目标
2. 不同休眠模式的触发条件和进入流程
3. 休眠模式下的唤醒条件和流程
4. 休眠模式下的功耗优化措施
5. 休眠模式下的外设管理策略
注意：
1. 注意层级隔离，在执行层应该不允许操作上层按键和背光等。
2. idle 睡眠，是否能只针对定时器降频，进入蓝牙低功耗。不关闭背光和指示灯（基于PWM 不收定时器限制）
流程：有按键活跃 -> 无按键流程 ->5s无操作 -> 进入idle sleep关闭扫描定时器降低功耗(不关闭指示灯和背光)，设置按键 触控 定时器唤醒等 -> 10min无操作 -> 进入deep sleep -> 断连、关闭指示灯、关闭背光。
唤醒：有 idle sleep 唤醒，恢复键盘扫描状态。 deep sleep 唤醒，重新初始化所有标志位，开启外设状态等。
3.所有关键事件都通过OSAL 事件调度执行，做好隔离。这样才方便修改。
4.总结出所有的修改内容。



在设计中，进入idle sleep 后，向各 service 投递准备事件。各事件执行完成后再投递进入idle sleep 事件。这个流程如何实现。
注意：需要遵循简单高效的实现方式，不允许引入复杂的流程。同时需要详细列出实现流程，不允许直接修改代码。


请使用feature-brainstorming，结合docs\code_demo\KB04122-13A-WCH_code.xml 代码案例库，分析一下docs\N0046_sleep_strategy.md中设计的休眠方案的可行性，着重分析沁恒蓝牙芯片的休眠和唤醒流程，确认是否能够满足设计需求，并且分析可能存在的问题和解决方案。


当前新编写的.claude\skills\feature-brainstorming skills 在试用过程中发现，AI 只按流程跑了第一个需求分析。后面就没有按流程进行，直接进入了阶段二：方案设计 — 可行性分析。请帮我分析一下，应该如何修改这个skill 确保AI能够按照流程一步步来，而不是直接跳到方案设计阶段。
或者说是否需要拆分多个详细步骤，然后组合到一起。

请使用skills feature-brainstorming
现在需要测试drivers\output\indicators\indicator.c 中的指示灯功能，包括熄灭、常亮、闪烁等模式。请制定开机后的测试方法，在init方法中加入测试代码，验证指示灯功能是否正常。测试方法需要详细描述每个模式的预期行为和验证步骤。注意GPIO_A15为白灯，GPIO_B23为红灯。（均为高电平亮）


  请阅读 DOCS/N0046_indicator_test_plan.md，按文档描述完成以下实现任务：                                                                              
                                                                                                                                                      
  1. 修改 keyboards/indicator_config.h                                                                                                                
     - 将 IND_LED_COUNT 改为 2                                                                                                                        
     - 将 ind_led_table 中的引脚改为 A15（白灯）和 B23（红灯），active_high 均为 true                                                                 
     - 添加别名 LED_WHITE=0, LED_RED=1

  2. 修改 application/system/system_init.c
     - 在 system_init_drivers() 中取消注释 indicator_init()
     - 在 system_init_application() 末尾、状态标记之前添加 indicator_test() 调用
     - include "indicator_test.h"

  3. 修改 application/main.c
     - 在 while(1) 主循环中添加 indicator_task() 调用（放在 OSAL_SystemProcessOnce() 之前）

  4. 新增 application/system/indicator_test.h
     - 宏 INDICATOR_TEST_ENABLE 保护，声明 indicator_test()

  5. 新增 application/system/indicator_test.c
     - 实现 8 阶段测试状态机，详见文档第三节

  6. 在 application/sys_config.h 中添加 #define INDICATOR_TEST_ENABLE

  完成后请验证：indicator_task() 已在主循环，indicator_init() 已取消注释，indicator_test() 已注册 OSAL 任务。


请使用skills feature-brainstorming
我在进行项目架构和流程检查的时候，发现项目中所有设置指示灯都是直接调用indicator_set方法设置指示灯。这种方式在OSAL 事件驱动的架构中可能会导致模块之间耦合过高，难以维护和扩展。请分析一下这种设计的缺点，并提出一个更合理的设计方案，确保指示灯的控制能够通过事件或状态机来实现，从而降低模块之间的耦合度，提高代码的可维护性和扩展性。指示灯可以设置中间变量来存储状态，然后通过事件触发。

  请阅读 DOCS/plans/2026-03-17-indicator-event-driven-design.md，                                                                               
  按文档中「六、实施计划」的 Step 1-7 顺序执行指示灯事件驱动重构。 必要是请开启agent teams。                                                                          
                                                                                                                                                
  执行要求：                                                                                                                                    
  1. 严格按 Step 顺序执行，每步完成后运行文档中规定的验证命令再继续下一步                                                                       
  2. 每步开始前先阅读对应的目标文件，理解现有代码再修改                                                                                                
  3. 所有改动完成后，调用 /commit 创建提交
  4. 不要修改 drivers/output/indicators/ 下的任何文件

  关键文件路径：
  - 设计文档：DOCS/plans/2026-03-17-indicator-event-driven-design.md
  - 主要修改目标：
    - application/service/output_service.h
    - application/service/output_service.c
    - middleware/communication/wireless_callbacks.c
    - middleware/communication/wireless.c
    - keyboards/n0046/n0046.c
    - application/service/input_service.c

文档docs\code_demo\2860sdk.xml 中包含了2860 SDK的代码实现，请帮我分析一下其中有关于 S_FN_KEY 按键与 M_EARTH 共用的逻辑。输出逻辑分析流程。
据已知的信息。
1. S_FN_KEY 是一个功能键，通常用于触发键盘特殊功能（切换背光颜色亮度、电量检查、重新配对）。
2. M_EARTH 是一个iPad os 的按键，单按切换输入法，长按弹出快捷操作指南。并且和其他键组合有平台系统功能。
3. S_FN_KEY 和 M_EARTH 共用一个物理按键。该键单按时 延迟开始发送 M_EARTH 的按键事件。当该按下该键有其他键被按下时，分两种情况，如果其他键是键盘组合键功能（背光灯），就立刻触发键盘功能，不再发送 M_EARTH 的按键事件；如果其他键是普通按键，就先发送 M_EARTH 的按键事件，再发送普通按键事件。

请使用skills feature-brainstorming
请阅读DOCS/plans/2026-03-17-shared-fn-earth-key-requirements.md，需求描述了S_FN_KEY和M_EARTH共用按键的设计需求。请根据文档中的需求分析，设计一个详细的实现方案。


1. 进行复杂任务时，上下文太大导致效果下降
2. 速度慢，整个流程跑下来时间很长。
可以将流程拆分，通过输出详细文档来衔接，同时使用Agent Team 来隔离上下文，提高速度。


请阅读docs\plans\2026-03-17-kb904-requirements.md 和 docs\plans\2026-03-17-kb904-design.md
，需求描述了KB904键盘的设计需求和方案。请评估文档中的需求分析和设计方案。注意以下几个点
1. 不更改其中的设计需求。不允许直接修改问题，先提出问题和优化方案，确认后再修改设计方案。
2. 从系统全局，评估设计方案是否满足需求分析中提出的需求，是否存在缺失或者不合理的地方。
3. 探讨文中新增的文件是否过多，有一部分内容是否可以合并。
4. 需要遵循现有的代码架构和设计原则，遵循最小改动原则，能复用的要尽可能复用。


请按模块进行拆分分别实现不同的功能，方便使用Agent Team进行开发。
1. 系统总体设计和架构描述
   1. 背光模块
   2. 指示灯系统
   3. 按键与组合键功能
   4. 低电量联动
   5. 恢复出厂设置




1.改 description：改成纯触发条件风格，例如：Use when analyzing or designing a new feature, behavior change, or multi-step implementation where requirements, scope, and architecture must be validated before coding.  
2.增加“何时不用”：明确不适用场景，比如纯文案修改、单点 typo、机械性配置更新，避免误触发。
3.补标准 handoff：在 步骤9 明确后续进入哪个 skill，比如 writing-plans，这样形成稳定链路。
4.统一术语：现在同时出现“requirements.md / phase1.md / design.md / 交接文档 / 设计文档”，建议在 SKILL.md 开头放一个术语表，减少认知负担。


请读取docs\plans\kb904-modules 下所有的设计文档，制定一个详细的实现计划，确认计划后，开启Agent Team进行开发。
注意：
不采用CMakeLists.txt 进行编译。是基于skills wch-riscv-build 编译。
OSAL_StartTimer 方法使用OSAL_SetDelayedEvent替换


请使用skills pyramid-design进行分析
请阅读docs\kb904\KB904_功能清单.md 和 docs\kb904\KB904_原理图.md，从SDK系统架构层面考虑，如何设计配置项管理。能够做到模块之间低耦合高内聚，方便后续维护和扩展。请分析一下当前的设计方案是否满足这些要求，并提出优化建议。注意要结合原理图中的GPIO配置和功能清单中有关于配置项的需求点进行分析。目标是当开发其他项目时，细微的需求变动能够通过调整配置项来适配，而不需要修改核心代码逻辑。

请使用skill pyramid-design进行分析
当前项目./keyboards/目录下配置混乱，能够做到模块配置集中，方便维护和扩展，但是当前如下的三个文件又独立在外，需要结合到当前配置中去。
keyboards\default_keymap.c
keyboards\indicator_config.h
keyboards\product_config.h

当前项目./application\system 系统初始化步骤太多，又有一部分初始化跑到了service层，层级不清晰，使用混乱。请制定方案来梳理系统初始化流程，明确每个模块的职责和初始化内容，确保系统初始化的层级清晰，职责明确。需要结合现有代码结构，提出一个合理的系统初始化流程设计方案，并分析该方案是否满足项目需求，是否存在潜在问题，以及如何解决这些问题。


使用了agent 没有使用agent team，当前不可查看和管理各个agent
最后实施时没有新开上下文，继续原有的上下文会导致上下文过大，影响效果和速度。应该以需求和实施文档作为入口，新开上下文来实施，避免带入过多无关信息。
小问题：提问时没有使用特殊格式，

当前项目我在更改串口打印脚更改为 B21 uart3后，MCU 芯片启动失败。请帮我排查一下可能的原因。如下是已知信息：
1. 原来的串口脚是 A9 uart1，改成 B21 uart3 后芯片无法启动。
2. DEBUG 是在编译时上下文注入的，改动串口时也同步从1改成了3。
3. B21原本是触控板I2C 脚，现在没有接触控板，默认电压3.3v。
4. 芯片启动失败表现为没有任何串口输出，无法进入正常的程序流程。