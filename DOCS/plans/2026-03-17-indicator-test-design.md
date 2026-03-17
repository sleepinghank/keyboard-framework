# N0046 指示灯测试方案设计

## 概述

为验证 `drivers/output/indicators/indicator.c` 驱动层各灯效模式在 N0046 硬件上的正确性，
设计一套开机自动执行的非阻塞测试序列，通过 OSAL 周期事件逐阶段推进。

详细测试方案见：`DOCS/N0046_indicator_test_plan.md`

## 核心决策

| 决策点 | 选择 | 原因 |
|--------|------|------|
| 测试入口 | 独立 `indicator_test()` 函数，宏 `INDICATOR_TEST_ENABLE` 控制 | 方便开关，不污染生产代码 |
| 执行方式 | 非阻塞，OSAL 1s 周期事件驱动状态机 | 复用现有 OSAL 模式，系统正常运行 |
| 测试覆盖 | OFF、ON、ON定时、慢闪、快闪、闪N次、延迟启动、批量关闭 | 覆盖全部 B 级需求用例 |

## 前置修改（3 处）

1. `keyboards/indicator_config.h`：IND_LED_COUNT=2，A15 白灯，B23 红灯
2. `application/system/system_init.c`：取消注释 `indicator_init()`
3. `application/main.c`：主循环添加 `indicator_task()`

## 新增文件

- `application/system/indicator_test.h`
- `application/system/indicator_test.c`
