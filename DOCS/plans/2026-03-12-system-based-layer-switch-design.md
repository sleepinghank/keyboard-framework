# 基于系统类型的层切换设计

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 根据主机系统类型自动切换键盘基础层，替代复杂的 Fn 键层切换逻辑

**Architecture:** wireless 层检测到系统类型变化时，调用 keyboard 层新增的接口函数，由 keyboard 层负责读取系统类型、映射到目标层并执行层切换

**Tech Stack:** C, 现有 layer API

---

## 背景与动机

当前 `process_layer_switch_key()` 实现复杂，需要依赖 Fn 键物理位置和矩阵状态检测。为简化实现，暂时移除 Fn 层切换功能，改为根据主机系统类型自动切换层。

## 设计决策

1. **触发时机**: 实时切换 - 当 `host_system_type` 变化时立即切换层
2. **映射关系**:
   | 系统类型 | 目标层 |
   |---------|--------|
   | MAC | LAYER_MACOS |
   | IOS | LAYER_MACOS |
   | WIN | LAYER_WINDOWS |
   | ANDROID | LAYER_WINDOWS |
3. **代码位置**: keyboard 层新建函数，wireless 层调用

## 架构

```
wireless 层                    keyboard 层
    │                              │
    │ host_system_type 变化        │
    ├──────────────────────────────▶ keyboard_update_base_layer_by_system()
    │                              │
    │                              ├── 读取 host_system_type
    │                              ├── 映射到目标层
    │                              └── 调用 layer_set()
```

## 组件设计

### 1. keyboard.c 新增函数

```c
/**
 * @brief 根据主机系统类型更新基础层
 * @note 供 wireless 层在连接状态变化时调用
 */
void keyboard_update_base_layer_by_system(void) {
    extern uint8_t host_system_type;
    layer_id_t target_layer;

    switch (host_system_type) {
        case MAC:
        case IOS:
            target_layer = LAYER_MACOS;
            break;
        case WIN:
        case ANDROID:
        default:
            target_layer = LAYER_WINDOWS;
            break;
    }

    layer_set(target_layer);
}
```

### 2. keyboard.h 新增声明

```c
/**
 * @brief 根据主机系统类型更新基础层
 * @note 供 wireless 层在连接状态变化时调用
 */
void keyboard_update_base_layer_by_system(void);
```

### 3. 调用时机

在 wireless 层的连接成功回调中调用:

```c
// wireless_state_set_connected() 或 wireless_enter_connected_kb() 中
keyboard_update_base_layer_by_system();
```

## 数据流

1. 主机连接 → BLE 协议栈获取系统类型 → 设置 `host_system_type`
2. 触发 `wireless_state_set_connected()` 或连接成功回调
3. 调用 `keyboard_update_base_layer_by_system()`
4. 函数读取 `host_system_type`
5. 根据映射表确定目标层
6. 调用 `layer_set()` 切换层

## 依赖

- `host_system_type` - wireless.h 中定义的全局变量
- `layer_set()` - keymap.h 中定义的层切换 API
- `layer_id_t` - keymap.h 中定义的层枚举 (LAYER_MACOS, LAYER_WINDOWS)

## 实现任务

1. 在 keyboard.h 添加函数声明
2. 在 keyboard.c 实现函数
3. 在 wireless 连接成功回调中调用该函数
4. 移除已注释的 process_layer_switch_key 相关代码（可选清理）
