/**
 * @file storage_example.c
 * @brief 存储模块使用示例和测试代码
 */

#include <stdio.h>
#include "storage.h"

/* 写入完成回调函数 */
void storage_write_callback(bool success) {
    if (success) {
        printf("配置保存成功!\n");
    } else {
        printf("配置保存失败!\n");
    }
}

/* 示例1: 基本使用流程 */
void example_basic_usage(void) {
    printf("\n=== 示例1: 基本使用流程 ===\n");

    // 1. 初始化存储模块
    storage_init();
    printf("存储模块初始化完成\n");

    // 2. 读取当前配置
    storage_config_t config;
    storage_load_config(&config);
    printf("当前Fn锁状态: %d\n", config.fn_lock_state);
    printf("当前背光亮度: %d\n", config.backlight_brightness);

    // 3. 修改配置
    config.fn_lock_state = 1;           // 启用Fn锁
    config.backlight_brightness = 90;   // 设置背光亮度为90%
    config.language = 1;                // 设置语言为中文

    // 4. 保存配置
    bool result = storage_save_config(&config);
    if (result) {
        printf("配置保存成功\n");
    } else {
        printf("配置保存失败\n");
    }
}

/* 示例2: 使用便捷宏 */
void example_convenience_macros(void) {
    printf("\n=== 示例2: 使用便捷宏 ===\n");

    // 直接读取配置 (推荐方式)
    uint8_t fn_lock = STORAGE_GET_FN_LOCK();
    uint8_t brightness = STORAGE_GET_BACKLIGHT();
    uint8_t language = STORAGE_GET_LANGUAGE();

    printf("Fn锁状态: %d\n", fn_lock);
    printf("背光亮度: %d\n", brightness);
    printf("语言设置: %d\n", language);

    // 直接修改配置
    STORAGE_SET_FN_LOCK(1);
    STORAGE_SET_BACKLIGHT(75);
    STORAGE_SET_LANGUAGE(0);

    printf("修改后的配置:\n");
    printf("  Fn锁状态: %d\n", STORAGE_GET_FN_LOCK());
    printf("  背光亮度: %d\n", STORAGE_GET_BACKLIGHT());
    printf("  语言设置: %d\n", STORAGE_GET_LANGUAGE());

    // 保存修改
    storage_save_config(storage_get_config_ptr());
}

/* 示例3: 访问复杂数据结构 */
void example_complex_data(void) {
    printf("\n=== 示例3: 访问复杂数据结构 ===\n");

    // 获取配置指针
    storage_config_t* config = storage_get_config_ptr();

    // 设置手势映射
    config->gesture_map[0] = 0x0101;  // 手势1
    config->gesture_map[1] = 0x0202;  // 手势2
    config->gesture_map[2] = 0x0303;  // 手势3

    printf("手势映射前3个:\n");
    for (int i = 0; i < 3; i++) {
        printf("  手势[%d]: 0x%04X\n", i, config->gesture_map[i]);
    }

    // 设置宏数据
    config->macro_data[0] = 1;        // 启用宏
    config->macro_data[1] = 0xAA;     // 宏参数1
    config->macro_data[2] = 0xBB;     // 宏参数2

    printf("宏数据前3个字节:\n");
    for (int i = 0; i < 3; i++) {
        printf("  宏数据[%d]: 0x%02X\n", i, config->macro_data[i]);
    }

    // 设置快捷键
    config->shortcuts[0] = 0x0100 | 0x01;  // Ctrl+A
    config->shortcuts[1] = 0x0100 | 0x02;  // Ctrl+B

    printf("快捷键前2个:\n");
    for (int i = 0; i < 2; i++) {
        printf("  快捷键[%d]: 0x%04X\n", i, config->shortcuts[i]);
    }

    // 设置用户偏好
    config->user_preferences[0] = 0x12345678;  // 偏好1
    config->user_preferences[1] = 0x87654321;  // 偏好2

    printf("用户偏好:\n");
    for (int i = 0; i < 2; i++) {
        printf("  偏好[%d]: 0x%08X\n", i, config->user_preferences[i]);
    }

    // 保存配置
    storage_save_config(config);
}

/* 示例4: 出厂重置 */
void example_factory_reset(void) {
    printf("\n=== 示例4: 出厂重置 ===\n");

    printf("重置前配置:\n");
    printf("  Fn锁状态: %d\n", STORAGE_GET_FN_LOCK());
    printf("  背光亮度: %d\n", STORAGE_GET_BACKLIGHT());

    // 执行出厂重置
    storage_factory_reset();
    printf("执行出厂重置\n");

    printf("重置后配置:\n");
    printf("  Fn锁状态: %d (应该为0)\n", STORAGE_GET_FN_LOCK());
    printf("  背光亮度: %d (应该为80)\n", STORAGE_GET_BACKLIGHT());
}

/* 示例5: 配置验证 */
void example_config_validation(void) {
    printf("\n=== 示例5: 配置验证 ===\n");

    // 验证EEPROM数据
    bool eeprom_valid = storage_validate_eeprom_data();
    printf("EEPROM数据有效性: %s\n", eeprom_valid ? "有效" : "无效");

    // 验证当前配置
    storage_config_t* config = storage_get_config_ptr();
    bool config_valid = storage_validate_config(config);
    printf("当前配置有效性: %s\n", config_valid ? "有效" : "无效");

    // 获取版本信息
    uint8_t version = storage_get_version();
    printf("配置版本号: %d\n", version);
}

/* 示例6: 批量操作 */
void example_batch_operations(void) {
    printf("\n=== 示例6: 批量操作 ===\n");

    // 批量设置系统配置
    storage_config_t config;
    storage_load_config(&config);

    // 系统配置
    config.fn_lock_state = 0;
    config.device_type = 1;
    config.backlight_brightness = 60;
    config.language = 2;
    config.led_mode = 3;

    // 用户配置
    for (int i = 0; i < 32; i++) {
        config.gesture_map[i] = i + 1;
    }

    memset(config.macro_data, 0xFF, sizeof(config.macro_data));

    for (int i = 0; i < 16; i++) {
        config.shortcuts[i] = 0x1000 + i;
    }

    for (int i = 0; i < 4; i++) {
        config.user_preferences[i] = 0x11111111 * (i + 1);
    }

    printf("批量设置完成，保存配置...\n");
    bool result = storage_save_config(&config);

    if (result) {
        printf("批量设置成功\n");

        // 验证设置
        printf("验证设置结果:\n");
        printf("  Fn锁状态: %d\n", STORAGE_GET_FN_LOCK());
        printf("  背光亮度: %d\n", STORAGE_GET_BACKLIGHT());
        printf("  手势映射[0]: 0x%04X (期望: 0x0001)\n", config.gesture_map[0]);
        printf("  快捷键[0]: 0x%04X (期望: 0x1000)\n", config.shortcuts[0]);
        printf("  用户偏好[0]: 0x%08X (期望: 0x11111111)\n", config.user_preferences[0]);
    } else {
        printf("批量设置失败\n");
    }
}

/* 主函数 */
int main(void) {
    printf("存储模块使用示例\n");
    printf("==================\n");

    // 设置写入回调
    storage_set_write_callback(storage_write_callback);

    // 运行各种示例
    example_basic_usage();
    example_convenience_macros();
    example_complex_data();
    example_factory_reset();
    example_config_validation();
    example_batch_operations();

    printf("\n所有示例运行完成!\n");

    return 0;
}

/* 编译说明
 *
 * 编译此示例需要:
 * 1. 包含storage.h和storage.c
 * 2. 包含eeprom.h (底层EEPROM驱动)
 * 3. 设置适当的编译器标志
 *
 * GCC编译示例:
 * gcc -o storage_example storage_example.c storage.c -I../drivers/storage -I../drivers/storage -std=c99
 */
