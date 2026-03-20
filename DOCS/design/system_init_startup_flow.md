# 系统初始化启动流程

## 生命周期

简化后的生命周期：**init → task**

## 初始化阶段

### 1. HAL 层初始化 (system_hal_init / system_init_hal)
- GPIO 安全状态初始化
- 时钟配置
- 电源配置
- UART 初始化
- 硬件定时器初始化

### 2. Driver 层初始化 (system_init_drivers)
- timer_init() - 系统时钟
- storage_init() - EEPROM 存储
- bt_driver_init() - BLE 协议栈
- battery_init() - 电池管理
- backlight_init() - 背光控制
- indicator_init() - 指示灯

### 3. Middleware 层初始化 (system_init_middleware)
- lpm_init() - 低功耗管理
- wireless_init() - 无线状态机（含幂等保护）
- keyboard_init() - 键盘处理

### 4. Application 层初始化 (system_init_application)
- system_service_init() - 系统服务
- input_service_init() - 输入服务
- output_service_init() - 输出服务
- commu_service_init() - 通信服务

## 状态机

```
NOT_STARTED → HAL → DRIVER → MIDDLEWARE → APPLICATION → TASK → COMPLETED
```

## 添加新服务模块

1. 创建 `xxx_service.c/h` 文件
2. 实现 `xxx_service_init()` 函数
3. 在 `system_init_application()` 添加 `xxx_service_init()` 调用
4. （可选）在 main.c 的 task 循环添加 task 调用

## 设计原则

- **层级分离**：驱动初始化在 Driver 层，不在 Service 层
- **幂等保护**：关键初始化函数有幂等保护（如 wireless_init）
- **依赖顺序**：按 HAL → Driver → Middleware → Application 顺序初始化
