# KB04122-13A-WCH 低功耗休眠策略分析

## 1. 文档范围

本文基于以下内容整理：

- `docs/code_demo/KB04122-13A-WCH_code.xml`
- 当前拆分源码中的 CH584 低功耗实现

重点回答两个问题：

1. `HAL_SLEEP` 宏到底起什么作用
2. 整套低功耗休眠流程是如何从业务层一路走到底层的

需要先说明一个结论：

- `KB04122-13A-WCH_code.xml` 中保留的是一套较完整的 WCH/TMOS 风格休眠链路，且其中出现了 `HAL_SLEEP=1`
- 当前仓库拆分后的 CH584 工程配置中，`HAL_SLEEP=0`
- 因此，`xml` 里的低功耗链路和当前拆分源码的实际启用状态并不完全一致

## 2. `HAL_SLEEP` 宏的作用

`HAL_SLEEP` 不是“休眠策略本身”，而是 WCH HAL 低功耗执行路径的编译期开关。

它主要控制三件事：

| 作用点 | 具体作用 | 代码位置 |
|---|---|---|
| 配置默认值 | 定义是否启用 HAL 睡眠能力，默认值为 `FALSE` | `project/ch584m/platforms_HAL/include/CONFIG.h:83` |
| 注册低功耗回调 | 打开后，BLE 配置会注册 `cfg.idleCB = CH58x_LowPower`，让协议栈在空闲窗口尝试进入低功耗 | `project/ch584m/platforms_HAL/MCU.c:121-122` |
| 初始化 RTC 唤醒 | 打开后，`HAL_Init()` 会调用 `HAL_SleepInit()`，配置 RTC 触发唤醒和 RTC 中断 | `project/ch584m/platforms_HAL/MCU.c:233-234`, `project/ch584m/platforms_HAL/SLEEP.c:85-95` |
| 真正执行睡眠指令 | 打开后，`CH58x_LowPower()` 才会计算唤醒时刻并调用 `LowPower_Sleep(...)` | `project/ch584m/platforms_HAL/SLEEP.c:27-72` |

### 2.1 `HAL_SLEEP` 打开后会发生什么

| 步骤 | 动作 | 说明 |
|---|---|---|
| 1 | `HAL_SleepInit()` 配置 `RB_SLP_RTC_WAKE` | 允许 RTC 成为唤醒源 |
| 2 | `HAL_SleepInit()` 配置 `RB_RTC_TRIG_EN` | 让 RTC 使用触发模式 |
| 3 | 使能 `RTC_IRQn` | 唤醒点到达后进入 RTC 中断 |
| 4 | BLE 栈空闲时回调 `CH58x_LowPower()` | 由 WCH 协议栈驱动睡眠进入 |
| 5 | `CH58x_LowPower()` 计算提前唤醒时间 | 预留晶振稳定时间 `WAKE_UP_RTC_MAX_TIME` |
| 6 | 设置 `RTC_SetTignTime()` | 写入 RTC 触发值 |
| 7 | 调用 `LowPower_Sleep(...)` | MCU 真正进入低功耗状态 |

### 2.2 `HAL_SLEEP` 关闭后意味着什么

| 结果 | 影响 |
|---|---|
| `HAL_SleepInit()` 不执行 | RTC 唤醒路径不会按这套 WCH HAL 逻辑初始化 |
| `cfg.idleCB = CH58x_LowPower` 不注册 | BLE 空闲时不会自动进入 `CH58x_LowPower()` |
| `CH58x_LowPower()` 中的 `LowPower_Sleep(...)` 路径失效 | 即使业务层设置了睡眠标志，也不会靠这条 HAL 路径真正睡下去 |

换句话说，`HAL_SLEEP` 控制的是“WCH HAL 这一条低功耗落地链路是否存在”，而不是控制上层是否允许进入睡眠状态机。

## 3. `KB04122-13A-WCH_code.xml` 中的低功耗总体结构

从 `xml` 内容看，整套休眠流程分为三层：

| 层级 | 主要职责 | 代表变量/函数 |
|---|---|---|
| 业务策略层 | 判断何时该睡，决定是 idle sleep 还是 deep sleep | `idel_sleep_flag`, `deep_sleep_flag`, `sleep_en` |
| 外设准备层 | 关 LED、关 ADC、调整矩阵行列、配置 GPIO/电源键唤醒 | `peripheral_enter_sleep()`, `peripheral_exit_sleep()` |
| 芯片低功耗执行层 | 通过 RTC/GPIO/USB 等唤醒源执行 `LowPower_Sleep()` 或 `LowPower_Shutdown()` | `CH58x_LowPower()`, `RF_LowPower()`, `LowPower_Sleep()`, `LowPower_Shutdown()` |

其中：

- `sleep_en` 表示系统已经处于睡眠态或准备睡眠
- `idel_sleep_flag` 表示浅一层的空闲休眠
- `deep_sleep_flag` 表示更深一层的休眠

## 4. XML 版完整休眠主流程

下表按“触发 -> 事件分发 -> 进入低功耗 -> 唤醒恢复”顺序整理。

| 阶段 | 触发条件 | 关键代码 | 实际行为 | 备注 |
|---|---|---|---|---|
| 1. 空闲计时到期 | 键扫/状态机判断设备空闲过久 | `KB04122-13A-WCH_code.xml:10496-10518` | 设置 `idel_sleep_flag = TRUE` 或 `deep_sleep_flag = TRUE` | 是否走 BT 或 RF 由 `work_mode` 决定 |
| 2. 蓝牙模式分发 | `work_mode == MODE_BT` | `KB04122-13A-WCH_code.xml:1221-1235` | `access_ble_enter_idel_sleep()` 发送 `ACCESS_IDEL_SLEEP_EVT`；深睡发送 `ACCESS_SLEEP_EVT` | 只是投递 TMOS 事件，还没真正执行睡眠 |
| 3. Access 任务处理深睡事件 | 收到 `ACCESS_SLEEP_EVT` | `KB04122-13A-WCH_code.xml:739-771` | 置 `sleep_en = TRUE`、`deep_sleep_flag = TRUE`，停止其他睡眠/唤醒事件，必要时断开 BLE 或停广播 | 深睡前会尽量把连接状态收敛到可安全休眠 |
| 4. Access 任务处理 idle 休眠事件 | 收到 `ACCESS_IDEL_SLEEP_EVT` | `KB04122-13A-WCH_code.xml:773-802` | 置 `sleep_en = TRUE`、`idel_sleep_flag = TRUE`，若在广播则停止广播，否则保留连接语义 | 这一路是“保持蓝牙状态尽量轻睡” |
| 5. 睡前关闭业务外设 | 深睡或 idle 休眠都要走 | `KB04122-13A-WCH_code.xml:764-770`, `KB04122-13A-WCH_code.xml:799-801` | 关闭电量通知、关闭触摸、关闭红灯，然后调用 `peripheral_enter_sleep()` | 这是外设准备层 |
| 6. 外设进入低功耗准备态 | `peripheral_enter_sleep()` | `KB04122-13A-WCH_code.xml:10583-10658` | 关闭 PWM/ADC，配置未用引脚，下拉/上拉矩阵，行引脚改中断唤醒，PA2 电源键改成唤醒中断，打开 `RB_SLP_GPIO_WAKE` | 让按键矩阵和电源键能把 MCU 唤醒 |
| 7. RF/HAL 层真正睡眠 | TMOS/RF 层触发低功耗入口 | `KB04122-13A-WCH_code.xml:5913-5942`, `KB04122-13A-WCH_code.xml:5998-6046` | 根据当前深度调用 `RF_LowPower()` 或 `LowPower_Shutdown(0)` | 这一步才真正执行芯片低功耗指令 |
| 8. RTC 睡眠窗口计算 | 进入 `RF_LowPower()` | `KB04122-13A-WCH_code.xml:5913-5943` | 根据当前 RTC 计数与目标唤醒时刻，设置 `RTC_SetTignTime()`，再调用 `LowPower_Sleep(RB_PWR_RAM96K | RB_PWR_EXTEND | RB_XT_PRE_EN)` | 这一步受 `HAL_SLEEP` 开关控制 |
| 9. 唤醒事件发生 | 矩阵行、电源键、USB 或 RTC 触发 | `KB04122-13A-WCH_code.xml:10704-10721`, `KB04122-13A-WCH_code.xml:6488-6499` | GPIO 中断置位，必要时调用 `peripheral_sleep_update()` 或从 `LowPower_Sleep()` 返回 | 唤醒源不止一个 |
| 10. Access 恢复业务状态 | 收到 `ACCESS_WAKE_UP_EVT` | `KB04122-13A-WCH_code.xml:805-834`, `KB04122-13A-WCH_code.xml:1249-1250` | 若当前 BLE 通道空闲则恢复广播，重新启动触摸任务，清除 `deep_sleep_flag/idel_sleep_flag/sleep_en`，恢复电量通知 | 业务层恢复 |
| 11. 恢复电源键中断 | 唤醒后必须补做 | `KB04122-13A-WCH_code.xml:833`, `KB04122-13A-WCH_code.xml:11489-11507` | 调用 `power_key_resume()` 重新配置 PA2 下拉沿唤醒中断 | 因为睡眠前后 GPIO 中断配置会被重置 |

## 5. `peripheral_enter_sleep()` 的作用

`peripheral_enter_sleep()` 是整套流程的关键中间层。它不直接执行 `LowPower_Sleep()`，但它决定“系统能不能安全睡下去，以及唤醒后能不能回来”。

| 动作分类 | 具体动作 | 代码位置 | 目的 |
|---|---|---|---|
| 关闭耗电外设 | `PWMX_LED_ALL_OFF()`、`ADC_DisablePower()`、关闭电量检测 | `KB04122-13A-WCH_code.xml:10583-10590` | 降低静态功耗 |
| 处理非关键 GPIO | 多个引脚改成上拉/下拉输入 | `KB04122-13A-WCH_code.xml:10591-10609` | 防止悬空漏电 |
| 配置矩阵列 | 先输出高，再按列逐个拉低 | `KB04122-13A-WCH_code.xml:10610-10629` | 把键盘矩阵切换到适合唤醒检测的状态 |
| 配置矩阵行中断 | 行引脚设为下降沿中断 | `KB04122-13A-WCH_code.xml:10638-10652` | 任意按键动作可以触发唤醒 |
| 配置电源键 PA2 | `GPIOA_ITModeCfg(GPIO_Pin_2, GPIO_ITMode_FallEdge)` | `KB04122-13A-WCH_code.xml:10633-10637` | 电源键可作为独立唤醒源 |
| 打开 GPIO 唤醒源 | `PWR_PeriphWakeUpCfg(ENABLE, RB_SLP_GPIO_WAKE, Long_Delay)` | `KB04122-13A-WCH_code.xml:10658` | 让 GPIO 中断能从低功耗态拉起芯片 |

一句话概括：

`peripheral_enter_sleep()` 负责“把整机切成可被按键/电源键唤醒的低功耗硬件姿态”。

## 6. `RF_LowPower()` 和 `HAL_SLEEP` 的关系

这部分最容易混淆，单独列出来。

| 项目 | 行为 | 说明 |
|---|---|---|
| 入口函数 | `RF_LowPower(uint32_t time)` | `xml` 里 RF 模块的低功耗执行入口 |
| 编译条件 | `#if(defined(HAL_SLEEP)) && (HAL_SLEEP == TRUE)` | 没开 `HAL_SLEEP`，这段核心逻辑不会生效 |
| 关闭 RTC 触发模式 | 先清 `RB_RTC_TRIG_EN` | 重新配置前先关 |
| 计算绝对唤醒时刻 | 使用当前 `RTC_GetCycle32k()` 计算 `time_set` | 目标是未来某个 RTC tick |
| 写入 RTC 触发值 | `RTC_SetTignTime(time_set)` | 设置 RTC 唤醒点 |
| 标记睡眠中 | `gSleepFlag = TRUE` | 标识底层已进入睡眠流程 |
| 重新打开 RTC 触发模式 | 再置 `RB_RTC_TRIG_EN` | 允许 RTC 到点触发 |
| 执行睡眠 | `LowPower_Sleep(RB_PWR_RAM96K | RB_PWR_EXTEND | RB_XT_PRE_EN)` | 真正进入低功耗 |
| 唤醒后恢复 HSE 电流 | `HSECFG_Current(HSE_RCur_100)` | 恢复时钟相关配置 |

因此可以把 `HAL_SLEEP` 理解为：

- 它不是“判断是否应该睡”
- 它也不是“配置矩阵按键唤醒”
- 它是“让底层的 RTC 定时睡眠执行代码被编译并真正调用”的总开关

## 7. XML 版唤醒恢复流程

| 唤醒源 | 触发位置 | 后续动作 | 结果 |
|---|---|---|---|
| 键盘矩阵行中断 | `GPIOA/GPIOB` 行中断 | 置 `key_scan_flag`，必要时调用 `peripheral_sleep_update()` | 系统退出睡眠，恢复键盘扫描 |
| 电源键 PA2 | `GPIOA_IRQHandler` 中检测 `GPIO_Pin_2` | 启动 `POWER_KEY_DEBOUNCE_EVT`，若系统在睡眠中则调用 `peripheral_sleep_update()` | 可同时承担唤醒和关机/长按逻辑 |
| RTC | `RTC_IRQHandler` | 清 RTC 标志并置 `RTCTigFlag = 1` | 让 `LowPower_Sleep()` 返回 |
| USB | 某些路径单独配置 USB GPIO/USB wake | 从 USB suspend/bus activity 唤醒 | 主要出现在 USB 相关分支 |

恢复侧关键步骤如下：

| 步骤 | 恢复动作 | 代码位置 |
|---|---|---|
| 1 | `peripheral_exit_sleep()` 触发上层 wakeup | `KB04122-13A-WCH_code.xml:10669-10681` |
| 2 | 进入 `ACCESS_WAKE_UP_EVT` | `KB04122-13A-WCH_code.xml:805-834` |
| 3 | 恢复 BLE 广播或连接态汇报 | `KB04122-13A-WCH_code.xml:807-818` |
| 4 | 重新启动触摸任务 | `KB04122-13A-WCH_code.xml:826` |
| 5 | 清空睡眠状态标记 | `KB04122-13A-WCH_code.xml:827-830` |
| 6 | 恢复电量通知 | `KB04122-13A-WCH_code.xml:831` |
| 7 | 调用 `power_key_resume()` 重新配置 PA2 中断 | `KB04122-13A-WCH_code.xml:833`, `KB04122-13A-WCH_code.xml:11489-11507` |

## 8. 当前拆分源码中的对应关系

当前仓库已经引入一套新的 CH584 HAL 封装，不再完全依赖旧 `platforms_HAL` 休眠链路。

### 8.1 当前源码中的主入口

| 当前入口 | 行为 | 位置 |
|---|---|---|
| `access_ble_enter_idel_sleep()` | 满足条件时直接调用 `system_hal_enter_sleep(SYSTEM_POWER_MODE_DEEP_SLEEP, ...)` | `application/service/wireless_callbacks.c:188-209` |
| `system_hal_enter_sleep()` | 按模式映射为 `LowPower_Idle()` / `LowPower_Sleep()` / `LowPower_Shutdown()` | `hal/platforms/ch584/_system_hal.c:137-176` |
| `SYSTEM_DEEP_SLEEP_EVT` | 另一条服务层路径，调用 `enter_power_mode(PM_STANDBY_WITH_RAM)` | `application/service/system_service.c:73-86` |

### 8.2 当前源码与 XML 的关键差异

| 差异点 | XML 中表现 | 当前拆分源码中表现 | 影响 |
|---|---|---|---|
| `HAL_SLEEP` 开关 | `HAL_SLEEP=1` | `HAL_SLEEP=0` | 旧 WCH HAL 休眠链路默认未启用 |
| 主睡眠入口 | 事件最终落到 `RF_LowPower()` / `LowPower_Sleep()` | 更直接地走 `system_hal_enter_sleep()` | 新代码路径更扁平 |
| 深睡服务事件 | 旧代码深睡事件最终真睡 | 当前 `enter_power_mode(PM_STANDBY_WITH_RAM)` 在 CH584 实现里对应的 `LowPower_Shutdown(RB_PWR_RAM2K)` 仍被注释 | 这条路当前并不能真正让芯片进入待机保持 RAM |
| 唤醒源映射 | 旧代码显式配置 GPIO、RTC、USB 等 | 新代码接口声明了 `SYSTEM_WAKEUP_KEYBOARD`、`SYSTEM_WAKEUP_BLE`，但 CH584 平台实现只真正映射了 `GPIO/RTC/USB` | `KEYBOARD/BLE` 参数在当前平台实现里没有单独落地 |

## 9. 当前 CH584 `system_hal_enter_sleep()` 的真实效果

这是当前仓库里最值得注意的点。

### 9.1 参数到硬件唤醒源的映射

| 接口参数 | CH584 平台是否有实际映射 | 实现位置 |
|---|---|---|
| `SYSTEM_WAKEUP_GPIO` | 有 | `hal/platforms/ch584/_system_hal.c:142-144` |
| `SYSTEM_WAKEUP_RTC` | 有 | `hal/platforms/ch584/_system_hal.c:145-147` |
| `SYSTEM_WAKEUP_USB` | 有 | `hal/platforms/ch584/_system_hal.c:148-150` |
| `SYSTEM_WAKEUP_KEYBOARD` | 没有单独映射 | 无对应分支 |
| `SYSTEM_WAKEUP_BLE` | 没有单独映射 | 无对应分支 |

所以这段调用：

```c
system_hal_enter_sleep(
    SYSTEM_POWER_MODE_DEEP_SLEEP,
    SYSTEM_WAKEUP_GPIO | SYSTEM_WAKEUP_KEYBOARD | SYSTEM_WAKEUP_BLE
);
```

在当前 CH584 实现里，实际主要等价于：

```c
system_hal_enter_sleep(
    SYSTEM_POWER_MODE_DEEP_SLEEP,
    SYSTEM_WAKEUP_GPIO
);
```

### 9.2 模式到硬件低功耗指令的映射

| 模式 | 当前 CH584 实现 | 说明 |
|---|---|---|
| `SYSTEM_POWER_MODE_IDLE` | `LowPower_Idle()` | 轻度睡眠 |
| `SYSTEM_POWER_MODE_SLEEP` | `LowPower_Sleep(RB_PWR_RAM96K | RB_PWR_EXTEND)` | 保留较多 RAM |
| `SYSTEM_POWER_MODE_DEEP_SLEEP` | `LowPower_Sleep(RB_PWR_RAM32K)` | 更深的睡眠，保留更少 RAM |
| `SYSTEM_POWER_MODE_SHUTDOWN` | `LowPower_Shutdown(0)` | 最深，RAM 不保留 |

见 `hal/platforms/ch584/_system_hal.c:157-176`。

## 10. 结论

可以把整套休眠设计总结为下面这四句话：

| 结论 | 说明 |
|---|---|
| `HAL_SLEEP` 是底层睡眠执行开关 | 它控制 WCH HAL 的 RTC 睡眠代码是否启用，不负责上层策略判断 |
| 上层策略靠事件和标志位驱动 | `sleep_en`、`idel_sleep_flag`、`deep_sleep_flag` 决定进入哪种睡眠 |
| `peripheral_enter_sleep()` 是睡前准备核心 | 它负责把矩阵、GPIO、电源键、LED、ADC 切成低功耗且可唤醒状态 |
| 当前拆分源码已经偏向新 HAL 路径 | 真正更可能执行到睡眠指令的是 `system_hal_enter_sleep()`，而不是旧 `HAL_SLEEP` 链路 |

## 11. 建议阅读顺序

如果后续要继续追这套低功耗逻辑，建议按以下顺序看代码：

1. `docs/code_demo/KB04122-13A-WCH_code.xml`
2. `project/ch584m/platforms_HAL/include/CONFIG.h`
3. `project/ch584m/platforms_HAL/SLEEP.c`
4. `project/ch584m/platforms_HAL/RTC.c`
5. `application/service/wireless_callbacks.c`
6. `hal/platforms/ch584/_system_hal.c`
7. `hal/platforms/ch584/_power.c`

