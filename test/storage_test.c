/**
 * @file storage_test.c
 * @brief 存储模块单元测试
 *
 * 简单的测试用例，验证存储模块的基本功能
 * 编译时需要定义TEST_MODE宏来模拟EEPROM操作
 */

#include <stdio.h>
#include <string.h>
#include "storage.h"

#ifdef TEST_MODE
/* 测试模式下的EEPROM模拟 */

// 模拟EEPROM存储
static uint8_t mock_eeprom[sizeof(storage_pool_t)] = {0};

// 模拟EEPROM读函数
void eeprom_read_block(void *dst, const void *src, size_t n) {
    memcpy(dst, src, n);
}

// 模拟EEPROM写函数
void eeprom_write_block(const void *src, void *dst, size_t n) {
    memcpy(dst, src, n);
    memcpy((void*)mock_eeprom, dst, n);  // 同时写入模拟EEPROM
}
#endif

/* 测试用例 */

static int test_count = 0;
static int pass_count = 0;
static int fail_count = 0;

#define TEST_ASSERT(condition, message) \
    do { \
        test_count++; \
        if (condition) { \
            pass_count++; \
            printf("  [PASS] %s\n", message); \
        } else { \
            fail_count++; \
            printf("  [FAIL] %s\n", message); \
        } \
    } while(0)

void test_storage_init(void) {
    printf("\n测试: 存储模块初始化\n");
    storage_init();
    TEST_ASSERT(storage_is_initialized(), "存储模块初始化成功");
}

void test_basic_read_write(void) {
    printf("\n测试: 基本读写功能\n");

    // 读取当前配置
    storage_config_t config1;
    storage_load_config(&config1);

    // 修改配置
    config1.fn_lock_state = 1;
    config1.backlight_brightness = 90;
    config1.language = 2;

    // 保存配置
    bool save_result = storage_save_config(&config1);
    TEST_ASSERT(save_result, "配置保存成功");

    // 重新读取验证
    storage_config_t config2;
    storage_load_config(&config2);

    TEST_ASSERT(config2.fn_lock_state == 1, "Fn锁状态保存正确");
    TEST_ASSERT(config2.backlight_brightness == 90, "背光亮度保存正确");
    TEST_ASSERT(config2.language == 2, "语言设置保存正确");
}

void test_convenience_macros(void) {
    printf("\n测试: 便捷宏功能\n");

    // 使用便捷宏设置配置
    STORAGE_SET_FN_LOCK(1);
    STORAGE_SET_BACKLIGHT(75);
    STORAGE_SET_LANGUAGE(1);
    STORAGE_SET_LED_MODE(3);

    // 保存配置
    storage_save_config(storage_get_config_ptr());

    // 验证读取
    uint8_t fn_lock = STORAGE_GET_FN_LOCK();
    uint8_t brightness = STORAGE_GET_BACKLIGHT();
    uint8_t language = STORAGE_GET_LANGUAGE();
    uint8_t led_mode = STORAGE_GET_LED_MODE();

    TEST_ASSERT(fn_lock == 1, "便捷宏设置Fn锁正确");
    TEST_ASSERT(brightness == 75, "便捷宏设置背光正确");
    TEST_ASSERT(language == 1, "便捷宏设置语言正确");
    TEST_ASSERT(led_mode == 3, "便捷宏设置LED模式正确");
}

void test_complex_data(void) {
    printf("\n测试: 复杂数据结构\n");

    storage_config_t* config = storage_get_config_ptr();

    // 设置手势映射
    for (int i = 0; i < 32; i++) {
        config->gesture_map[i] = i + 1;
    }

    // 设置宏数据
    config->macro_data[0] = 1;
    for (int i = 1; i < 32; i++) {
        config->macro_data[i] = i;
    }

    // 设置快捷键
    for (int i = 0; i < 16; i++) {
        config->shortcuts[i] = 0x1000 + i;
    }

    // 设置用户偏好
    for (int i = 0; i < 4; i++) {
        config->user_preferences[i] = 0x11111111 * (i + 1);
    }

    // 保存配置
    storage_save_config(config);

    // 重新读取验证
    storage_config_t config2;
    storage_load_config(&config2);

    TEST_ASSERT(config2.gesture_map[0] == 1, "手势映射保存正确");
    TEST_ASSERT(config2.gesture_map[31] == 32, "手势映射完整保存");
    TEST_ASSERT(config2.macro_data[0] == 1, "宏数据保存正确");
    TEST_ASSERT(config2.macro_data[31] == 31, "宏数据完整保存");
    TEST_ASSERT(config2.shortcuts[0] == 0x1000, "快捷键保存正确");
    TEST_ASSERT(config2.user_preferences[0] == 0x11111111, "用户偏好保存正确");
}

void test_factory_reset(void) {
    printf("\n测试: 出厂重置\n");

    // 先修改一些配置
    STORAGE_SET_FN_LOCK(1);
    STORAGE_SET_BACKLIGHT(100);
    storage_save_config(storage_get_config_ptr());

    // 执行出厂重置
    storage_factory_reset();

    // 验证恢复默认值
    TEST_ASSERT(STORAGE_GET_FN_LOCK() == 0, "出厂重置Fn锁正确");
    TEST_ASSERT(STORAGE_GET_BACKLIGHT() == 80, "出厂重置背光正确");
    TEST_ASSERT(STORAGE_GET_LANGUAGE, "出厂重() == 0置语言正确");
    TEST_ASSERT(STORAGE_GET_LED_MODE() == 1, "出厂重置LED模式正确");
}

void test_validation(void) {
    printf("\n测试: 数据验证\n");

    // 验证EEPROM数据
    bool eeprom_valid = storage_validate_eeprom_data();
    TEST_ASSERT(eeprom_valid, "EEPROM数据验证通过");

    // 验证当前配置
    storage_config_t* config = storage_get_config_ptr();
    bool config_valid = storage_validate_config(config);
    TEST_ASSERT(config_valid, "当前配置验证通过");

    // 验证版本号
    uint8_t version = storage_get_version();
    TEST_ASSERT(version == STORAGE_VERSION, "版本号正确");
}

void test_pointer_access(void) {
    printf("\n测试: 指针直接访问\n");

    storage_config_t* config = storage_get_config_ptr();

    // 直接修改指针指向的数据
    config->fn_lock_state = 1;
    config->device_type = 5;
    config->backlight_brightness = 50;

    // 通过便捷宏验证
    TEST_ASSERT(STORAGE_GET_FN_LOCK() == 1, "指针直接修改Fn锁");
    TEST_ASSERT(STORAGE_GET_DEVICE_TYPE() == 5, "指针直接修改设备类型");
    TEST_ASSERT(STORAGE_GET_BACKLIGHT() == 50, "指针直接修改背光亮度");
}

int main(void) {
    printf("======================================\n");
    printf("     存储模块单元测试\n");
    printf("======================================\n");

    // 初始化测试
    test_storage_init();

    // 运行各项测试
    test_basic_read_write();
    test_convenience_macros();
    test_complex_data();
    test_factory_reset();
    test_validation();
    test_pointer_access();

    // 输出测试结果
    printf("\n======================================\n");
    printf("测试结果统计:\n");
    printf("  总测试数: %d\n", test_count);
    printf("  通过数: %d\n", pass_count);
    printf("  失败数: %d\n", fail_count);
    printf("  成功率: %.1f%%\n", (float)pass_count / test_count * 100);
    printf("======================================\n");

    return (fail_count == 0) ? 0 : 1;
}

/*
编译说明:

1. 测试模式编译:
   gcc -DTEST_MODE -o storage_test storage_test.c storage.c -I../drivers/storage

2. 实际硬件编译:
   gcc -o storage_test storage_test.c storage.c -I../drivers/storage

3. 运行测试:
   ./storage_test

预期结果:
- 所有测试用例应该通过
- 成功率应为100%
*/
