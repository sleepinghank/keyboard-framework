# 需求文档：触控板 PTP BLE 流控补齐

## 1. 需求概述

修复 BLE 连接状态下，触控板连续滑动发送 PTP 报文时频繁出现：

```text
[I] send ptp data
[E] [BT] send_ptp fail, ret=22,len:19
PMU_Update: state=3
```

已知原因是当前发送路径缺少链路流控，连续滑动时会在前序包尚未完成确认时继续提交新包，导致 `HidDev_Report()` 返回 `ret=22`（`blePending`），进一步放大连接阻塞与日志噪音。

本次改动目标：

- 在公共无线发送路径中补齐基于 `LL_GetNumberOfUnAckPacket()` 的窗口等待能力
- 当发送失败或链路仍存在未确认包时，不继续盲发下一包
- 保持触控板上层业务逻辑不变，尽量在 `report_buffer` 层完成收敛

## 2. 需求 Q&A

| 问题 | 决定 |
|------|------|
| 修复范围放在哪一层 | 放在公共 `report_buffer` / `wireless_transport` 发送层，不放在触控板业务层 |
| 是否只处理 PTP | 问题由 PTP 暴露，但实现放在公共发送层，避免同类问题继续潜伏 |
| 本次是否新开独立 PTP 通道 | 不开新通道，继续复用现有 `wireless_send_ptp()` + `REPORT_SEND_EVT` |
| 是否修改触控板手势/坐标算法 | 不修改 |
| 成功标准 | 连续滑动时不持续刷 `ret=22` 错误，发送链路可在窗口释放后继续推进 |

## 3. 评审意见

### 产品视角

- 范围合理，直接对准“连续滑动报错并影响连接”的主问题
- 不扩展到大规模重构，风险更可控

### 架构视角

- 真正缺失的是链路窗口管理，而不是 `touchpad_service` 业务逻辑
- 将流控下沉到公共发送层，符合分层边界，避免业务层感知 BLE 细节

### 质量视角

- 除窗口等待外，还需同步修复 PTP 长度透传问题
- 还需补齐 `REPORT_SEND_EVT` 的续调度闭环，否则等待窗口释放时可能无人再次唤醒发送任务

## 4. 范围边界

### 包含

- 公共发送路径的当前报文状态管理
- `LL_GetNumberOfUnAckPacket()` 的查询封装与使用
- 发送失败后的等待与重试语义调整
- `REPORT_SEND_EVT` 续调度条件补齐
- PTP 实际负载长度透传
- BLE 忙态日志降噪

### 不包含

- 触控板原始数据采集、坐标转换、手势识别逻辑
- HID 描述符与 PTP 报文格式定义
- USB / 2.4G 发送策略重构
- 连接参数、配对流程与 PMU 策略调整

## 5. 代码现状分析

### 5.1 当前发送链路

当前 PTP 路径如下：

```text
touchpad_service.c
  -> send_ptp_data()
  -> wireless_send_ptp(report, len)
  -> report_buffer_enqueue()
  -> REPORT_SEND_EVT
  -> report_buffer_task()
  -> bt_driver_send_ptp()
  -> HidDev_Report()
```

涉及关键文件：

| 文件 | 职责 |
|------|------|
| `component/touch_component/touchpad_service.c` | 生成 PTP 报文并调用 `wireless_send_ptp()` |
| `middleware/communication/wireless.c` | 将 PTP 报文写入公共发送队列并触发 `REPORT_SEND_EVT` |
| `middleware/communication/report_buffer.c` | 统一出队、发送、失败重试 |
| `application/service/communication_service.c` | 处理 `REPORT_SEND_EVT` 并决定是否继续调度 |
| `drivers/communication/bluetooth/ch584/_bt_driver.c` | 实际调用 `HidDev_Report()` 发送 BLE PTP 报文 |

### 5.2 已确认的问题点

1. `wireless_send_ptp()` 没有保存调用方传入的 `len`
   - `report_buffer_t` 已经存在 `payload_len` 字段
   - 但 `wireless_send_ptp()` 当前未赋值
   - `report_buffer_task()` 发送 PTP 时仍固定使用 `sizeof(report_ptp_t) - 1`

2. `report_buffer_task()` 只有“失败重试”，没有“成功后等待窗口释放”
   - 当前成功发送后会立即清空 `kb_rpt`
   - 没有任何状态表示“报文已交给 BLE，但链路尚未释放窗口”
   - 因此下一轮调度仍可能继续发送下一包

3. 当前没有使用 `LL_GetNumberOfUnAckPacket()`
   - SDK 已提供接口：`LL_GetNumberOfUnAckPacket(handle)`
   - 当前工程已存在 `hidEmuConnHandle`
   - 但无线发送层还未接入“未确认包数量”判断

4. `REPORT_SEND_EVT` 续调度条件不完整
   - `communication_service.c` 当前只在“队列非空”时延迟重投事件
   - 没有覆盖“当前报文仍在 retry”或“正在等待窗口释放”的状态
   - 这会导致引入窗口等待后，发送任务可能无法自行继续推进

5. 蓝牙忙态日志过于激进
   - `_bt_driver.c` 中 `bt_driver_send_ptp()` 对所有非 0 返回值都直接报错
   - `ret=22` 实际上属于忙态流控信号，不宜每帧都按错误打印

### 5.3 根因结论

当前问题不是 PTP 报文内容错误，而是：

**公共发送队列缺少“发送成功后等待 BLE 窗口释放”的状态机，导致连续输入时在链路仍有未确认包的情况下继续提交新报文；同时 PTP 长度语义在入队到发送过程中丢失，使问题更难观测和验证。**

### 5.4 与需求的具体差距

| 差距 | 当前状态 | 目标状态 |
|------|---------|---------|
| 窗口流控 | 无，仅靠固定发送间隔与失败重试 | 发送后根据未确认包数决定何时继续 |
| PTP 长度透传 | `len` 入参未落地，发送端写死长度 | 入队长度与发送长度保持一致 |
| 续调度闭环 | 只看队列是否为空 | 同时覆盖队列、重试、等待窗口释放 |
| 忙态日志语义 | `blePending` 每次都报错 | 忙态降噪，真正异常再报错 |

### 5.5 不受影响的部分

- `touchpad_service.c` 中的触控板报文生成逻辑
- 触控板驱动读数与中断处理
- 上层无线状态机、配对/回连入口
- HID 描述符与现有报告格式
