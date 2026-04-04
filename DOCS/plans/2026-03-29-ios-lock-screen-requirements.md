# iOS Lock Screen 需求文档

## 需求概述

在 `kb_fn_action.c` 中新增 `Lock_IOS` 方法，实现 iOS 锁屏功能：发送 KC_IPOW 按下事件后，保持 200ms 再自动发送抬起事件。

## 需求 Q&A

| # | 问题 | 答案 |
|---|------|------|
| 1 | iOS 适配范围 | 仅 iOS 设备使用新逻辑，其他系统保持原有 Lock_Screen |
| 2 | 设备检测机制 | 设备检测在其他需求中实现，本需求只需新增 Lock_IOS 方法 |
| 3 | 默认模式 | 开机默认 iOS 模式 |
| 4 | 200ms 延迟行为 | 按下时立即发送 KC_IPOW 按下事件，200ms 后自动发送抬起事件 |
| 5 | 与现有 Lock_Screen 的关系 | 两个函数共存，调用处通过 iOS 标志位切换 |

## 评审意见汇总

### 共同关注点

1. **防重入**：200ms 窗口内若用户再次触发 Lock 键，需定义行为（建议：忽略新触发）
2. **异步状态机**：200ms 延迟不能阻塞当前按键处理流程，需要在 task 循环中轮询超时
3. **定时器选型**：复用 `timer_elapsed32()` + combo 引擎已有的 `press_time` 模式

### 架构发现

- `host_system_type` 全局变量已存在，默认 `IOS`
- `KC_IOS_POWER` (0x00CA) Consumer 键码已在 `report.h` 中映射
- KC_IPOW 属于 Consumer 键，report 流程走 consumer report 路径
- 抬起时需显式发送 consumer_report = 0

### 测试关注点

- 200ms 定时精度容差：建议 +/-20ms
- BLE 断连异常处理：按下发送失败时仍需 200ms 后清理状态
- iOS 标志位切换过渡期：切换时正在执行的 Lock_IOS 需完成或取消

## 范围边界

### 包含

- 新增 `Lock_IOS` 函数
- 200ms 延迟抬起状态机
- 调用处标志位切换逻辑
- 防重入机制

### 不包含

- 设备检测机制（其他需求）
- iOS 标志位管理（其他需求）

## 现状分析

### 结论

当前 FN action 机制是**同步一次性调用**（添加键码到数组、立即返回），缺乏维持"按键保持 N 毫秒后自动抬起"的异步能力。需要新增状态机 + 轮询机制。

### Lock_Screen 当前实现

```c
// kb_fn_action.c:315-324
uint8_t Lock_Screen(uint16_t* add_keys) {
    uint8_t idx = 0;
    if (FN_st == 1) {
        add_keys[idx++] = KC_F12;
    } else {
        add_keys[idx++] = KC_LEFT_GUI;
    }
    return idx;
}
```
- 函数签名：`uint8_t fn_name(uint16_t* add_keys)` — 同步添加键码，返回添加数量
- FN 动作注册：`FN_ADD_ACTION(fn_lock_screen)` 宏

### Consumer Report 路径

- 按下：`*consumer_report = KEYCODE2CONSUMER(keycode)` 赋值
- 发送：`consumer_report != last_consumer_report` 差分检测，变化时调用 `send_consumer()`
- 抬起：无独立 remove 函数，设置 `consumer_report = 0` 即释放
- **关键**：report 变量在 `report_update_proc()` 内部管理，外部无法直接操控

### keyboard_task 调用链

```
keyboard_task() 每次扫描周期执行：
  1. scan_and_debounce()           — 矩阵扫描 + 防抖
  2. update_key_code_list()        — 更新按键列表
  3. combo_task(key_st)            — combo 状态机推进（调用 FN action）
  4. report_update_proc(key_st)    — 生成并发送 HID 报告
  5. del_all_child(extend_list)    — 清空扩展键列表
```

### 定时器使用模式

```c
// combo 引擎已有模式
press_time = timer_read32();                    // 记录时间戳
elapsed = timer_elapsed32(combo->press_time);   // 计算经过毫秒
```

### 与需求的差距点

| 差距 | 说明 |
|------|------|
| 无异步保持机制 | FN action 一次性添加键码，无"保持后释放"能力 |
| Consumer report 外部不可控 | report 变量在 report_update_proc 内部，外部无法直接发 release |
| 需要轮询入口 | 200ms 超时检测需要周期性执行的入口点 |
