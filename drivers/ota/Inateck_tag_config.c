#include "kb904/config_product.h"
#include "CH58x_common.h"
#include "Inateck_tag_config.h"


// ===== 全局变量定义 =====
#define GET_BIT(bitmap, n) (((bitmap) & (1 << (n))) >> (n))
#define SET_BIT(bitmap, n)    ((bitmap) |= (1 << (n)))

// 系统相关变量
uint16_t max_mtu = 0;                // 最大支持的MTU
static uint16_t curr_mtu = 0;             // 当前的MTU
uint8_t find_switch = 0;                // 查找开关，默认关闭
uint8_t find_enable = 0;                // 是否开始查找
uint8_t gain_level = 0;                 // 增益调节
uint8_t battery_calibration_status = 0; // 电池电量校准完成状态
// uint8_t testSC7A20H = 0;
//定义键盘预设快捷键的行数量
#define KEYBOARD_SHORTCUT_ROW_NUM 1
//定义键盘预设快捷键的列数量
#define KEYBOARD_SHORTCUT_COL_NUM 16

uint16_t keyboard_shortcut_bitmap = {0xe000}; // 键盘预设快捷键启用BITMAP
/** !!!! 注意这里的keyboard_shortcut实际上应该采用uint8_t的二维数组，但是为了简化，这里采用一维数组，最多支持16个按键**/
uint8_t keyboard_shortcut[1][16] = {0};  // 键盘预设快捷键

// 设备配置相关变量
uint16_t sleep_time = 0;        // 待机时间
uint16_t shutdown_time = 0;     // 关机时间


// 读取MTU
extern uint16_t _OTA_ReadMTU(void);
// 读取支持的最大MTU长度
extern uint16_t _OTA_ReadMaxMTU(void);
// ===== 回调函数声明 =====

int8_t get_current_mcu_size(void* value, uint16_t* len);
// 电池相关回调
int8_t calibrate_battery_cb(const void* value, uint16_t len);
int8_t battery_level_read_cb(void* value, uint16_t* len);
// 系统操作相关回调
int8_t reset_device_cb(const void* value, uint16_t len);
int8_t factory_reset_cb(const void* value, uint16_t len);
int8_t reset_settings_cb(const void* value, uint16_t len);
int8_t disconnect_cb(const void* value, uint16_t len);

// 键盘相关回调
int8_t keyboard_shortcut_read_cb(void* value, uint16_t* len);
int8_t keyboard_shortcut_write_cb(const void* value, uint16_t len);
int8_t keyboard_shortcut_bitmap_read_cb(void* value, uint16_t* len);
int8_t keyboard_shortcut_bitmap_write_cb(const void* value, uint16_t len);

int8_t ble_name_read_cb(void* value, uint16_t* len);
int8_t ble_name_write_cb(const void* value, uint16_t len);

int8_t device_model_name_read_cb(void* value, uint16_t* len);
// ===== TAG列表定义 =====
const tag_descriptor_t tag_list[TAG_LIST_SIZE] = {
    // 系统相关（单字节）
    {
        .tag = TAG_ID_MAX_MTU,  // 最大支持的MTU
        .type = {
            .val_type = TAG_VALUE_TYPE_WORD,    // 双字节
            .access = TAG_ACCESS_TYPE_READ_ONLY,// 只读
            .handle_type = TAG_HANDLE_VARIABLE  // 变量
        },
        .var_ptr = &max_mtu,
        .var_size = 2
    },
    
    {
        .tag = TAG_ID_CURRENT_MTU,  // 当前的MTU
        .type = {
            .val_type = TAG_VALUE_TYPE_WORD,    // 双字节
            .access = TAG_ACCESS_TYPE_READ_WRITE,// 读写
            .handle_type = TAG_HANDLE_CALLBACK  // 回调
        },
        .read_cb = get_current_mcu_size
    },

    // 设备配置（单字节）
    {
        .tag = TAG_ID_BATTERY_LEVEL,  // 电池电量
        .type = {
            .val_type = TAG_VALUE_TYPE_PCT,    // 百分比
            .access = TAG_ACCESS_TYPE_READ_ONLY,// 只读
            .handle_type = TAG_HANDLE_CALLBACK  // 变量
        },
        .read_cb = battery_level_read_cb
    },


    // 系统相关（双字节）
    {
        .tag = TAG_ID_RESET_DEVICE,  // 重置设备
        .type = {
            .val_type = TAG_VALUE_TYPE_BOOL,   // 布尔值
            .access = TAG_ACCESS_TYPE_WRITE_ONLY,// 只写
            .handle_type = TAG_HANDLE_CALLBACK  // 回调
        },
        .write_cb = reset_device_cb  // 需要实现重置设备回调
    },
    
    {
        .tag = TAG_ID_FACTORY_RESET,  // 恢复出厂设置
        .type = {
            .val_type = TAG_VALUE_TYPE_BOOL,   // 布尔值
            .access = TAG_ACCESS_TYPE_WRITE_ONLY,// 只写
            .handle_type = TAG_HANDLE_CALLBACK  // 回调
        },
        .write_cb = factory_reset_cb  // 需要实现恢复出厂设置回调
    },
    
    {
        .tag = TAG_ID_RESET_SETTINGS,  // 重置系统设置
        .type = {
            .val_type = TAG_VALUE_TYPE_BOOL,   // 布尔值
            .access = TAG_ACCESS_TYPE_WRITE_ONLY,// 只写
            .handle_type = TAG_HANDLE_CALLBACK  // 回调
        },
        .write_cb = reset_settings_cb  // 需要实现重置系统设置回调
    },
    
    {
        .tag = TAG_ID_DISCONNECT,  // 断开连接
        .type = {
            .val_type = TAG_VALUE_TYPE_BOOL,   // 布尔值
            .access = TAG_ACCESS_TYPE_WRITE_ONLY,// 只写
            .handle_type = TAG_HANDLE_CALLBACK  // 回调
        },
        .write_cb = disconnect_cb  // 需要实现断开连接回调
    },
    
    // 设备配置（双字节）
    {
        .tag = TAG_ID_SHUTDOWN_TIME,  // 关机时间
        .type = {
            .val_type = TAG_VALUE_TYPE_WORD,   // 双字节
            .access = TAG_ACCESS_TYPE_READ_WRITE,// 读写
            .handle_type = TAG_HANDLE_VARIABLE  // 变量
        },
        .var_ptr = &shutdown_time,
        .var_size = sizeof(shutdown_time)
    },
    
    {
        .tag = TAG_ID_SLEEP_TIME,  // 待机时间
        .type = {
            .val_type = TAG_VALUE_TYPE_WORD,   // 双字节
            .access = TAG_ACCESS_TYPE_READ_WRITE,// 读写
            .handle_type = TAG_HANDLE_VARIABLE  // 变量
        },
        .var_ptr = &sleep_time,
        .var_size = sizeof(sleep_time)
    },

    {
        .tag = TAG_ID_CALIBRATE_BATTERY,  // 校准电池电量
        .type = {
            .val_type = TAG_VALUE_TYPE_BOOL,   // 布尔值
            .access = TAG_ACCESS_TYPE_WRITE_ONLY,// 只写
            .handle_type = TAG_HANDLE_CALLBACK  // 回调
        },
        .write_cb = calibrate_battery_cb  // 需要实现校准电池电量回调
    },
    
    {
        .tag = TAG_ID_BATTERY_CALIBRATION_STATUS,  // 电池电量校准完成
        .type = {
            .val_type = TAG_VALUE_TYPE_HEX,    // 单字节
            .access = TAG_ACCESS_TYPE_READ_ONLY,// 只读
            .handle_type = TAG_HANDLE_VARIABLE  // 变量
        },
        .var_ptr = &battery_calibration_status,
        .var_size = 1
    },

    {
        .tag = TAG_ID_KEYBOARD_SHORTCUT_BITMAP,  // 键盘预设快捷键启用BITMAP
        .type = {
            .val_type = TAG_VALUE_TYPE_HEX_ARRAY,// 字节数组
            .access = TAG_ACCESS_TYPE_READ_WRITE,// 读写
            .handle_type = TAG_HANDLE_CALLBACK  // 回调
        },
        .max_len = 4,
        .write_cb = keyboard_shortcut_bitmap_write_cb,  // 需要实现键盘预设快捷键BITMAP写入回调
        .read_cb = keyboard_shortcut_bitmap_read_cb  // 需要实现键盘预设快捷键BITMAP读取回调
    },
    
    {
        .tag = TAG_ID_KEYBOARD_SHORTCUT,  // 键盘预设快捷键
        .type = {
            .val_type = TAG_VALUE_TYPE_HEX_ARRAY,// 字节数组
            .access = TAG_ACCESS_TYPE_READ_WRITE,// 读写
            .handle_type = TAG_HANDLE_CALLBACK  // 回调
        },
        .max_len = 32,
        .write_cb = keyboard_shortcut_write_cb,  // 需要实现键盘预设快捷键写入回调
        .read_cb = keyboard_shortcut_read_cb  // 需要实现键盘预设快捷键读取回调
    },
    // {
    //     .tag = TAG_ID_PRODUCT_MODEL_NAME,  // 产品型号名称
    //     .type = {
    //         .val_type = TAG_VALUE_TYPE_STRING,// 字节数组
    //         .access = TAG_ACCESS_TYPE_READ_ONLY,// 读写
    //         .handle_type = TAG_HANDLE_CALLBACK  // 回调
    //     },
    //     .max_len = 32,
    //     .read_cb = device_model_name_read_cb  // 需要实现键盘预设快捷键读取回调
    // }
};

// ===== 回调函数实现 =====
// 获取电量回调
int8_t battery_level_read_cb(void* value, uint16_t* len){
    uint8_t* data = (uint8_t*)value;
    // data[0] =  get_bat_level(); // 获取电池电量
    *len = 1; // 设置长度为1字节
    return 0;
}

int8_t get_current_mcu_size(void* value, uint16_t* len){
    uint16_t temp_mtu = 0;
    uint8_t* data = (uint8_t*)value;
    temp_mtu = _OTA_ReadMTU();
    data[0] = HI_UINT16(temp_mtu);
    data[1] = LO_UINT16(temp_mtu);
    *len = 2;
    return 0;
}

// 校准电池电量回调
int8_t calibrate_battery_cb(const void* value, uint16_t len){
    battery_calibration_status = 1;
    // 重置电量
    // battery_level_detection(0);
    battery_calibration_status = 2;
    push_battery_calibration_status_change(); // 主动推送电池电量校准完成状态
    PRINT("Battery calibration completed\n");
    return 0;
}

// 重置设备回调
int8_t reset_device_cb(const void* value, uint16_t len){
    PRINT("reset_device_cb");
    SYS_ResetExecute(); // BLE复位系统
    return 0;
}

// 恢复出厂设置回调
int8_t factory_reset_cb(const void* value, uint16_t len){
    PRINT("factory_reset_cb");
    // hasFactoryReset=1;
    // set_all_tags_to_default();
    // factoryRestoration();
    return 0;
}

// 重置系统设置回调
int8_t reset_settings_cb(const void* value, uint16_t len){
    PRINT("reset_settings_cb");
    // set_all_tags_to_default();

    // nvs_flash_reset();

    // set_shutdown_time(nvs_flash_info.shutdown_time);
    // set_sleep_time(nvs_flash_info.sleep_time);
    // find_switch = 0;
    // find_flag = find_switch;

    // start_find_flag = 0;
    // find_enable = 0;
    // gain_position = gain_level;
    // MCP4017T_SetWiperPosition(nvs_flash_info.gain_level);
    // startStorageTask(1);
    // set_all_system_value();
    return 0;
}

// 断开连接回调
int8_t disconnect_cb(const void* value, uint16_t len){
    PRINT("disconnect_cb");
    // ble_disconnect();
    // 实现断开连接的逻辑
    return 0;
}

// 键盘预设快捷键BITMAP读取回调
int8_t keyboard_shortcut_bitmap_read_cb(void* value, uint16_t* len){
    uint8_t* data = (uint8_t*)value;
    uint16_t z = 0;

    data[0] = (uint8_t)(KEYBOARD_SHORTCUT_ROW_NUM);
    data[1] = (uint8_t)(KEYBOARD_SHORTCUT_COL_NUM);
    // if(nvs_flash_info.single_click_func_idx & 0x80)
    // {
    //     z |= (1 << 15);
    // }
    // if(nvs_flash_info.double_click_func_idx & 0x80)
    // {
    //     z |= (1 << 14);
    // }
    // if(nvs_flash_info.triple_click_func_idx & 0x80)
    // {
    //     z |= (1 << 13);
    // }

    data[2] = (z >> 8) & 0xFF;  // 高8位
    data[3] = z & 0xFF;         // 低8位

    *len = 4; // 设置长度为8字节
    return 0;
}

// 键盘预设快捷键BITMAP写入回调
int8_t keyboard_shortcut_bitmap_write_cb(const void* value, uint16_t len){
    const uint8_t* data = (const uint8_t*)value;

    if(len < 4)
    {
        PRINT("keyboard_shortcut_bitmap_write_cb: len < 4\n");
        return -1;
    }
    //抛弃前2个字节
    //读取BITMAP
    keyboard_shortcut_bitmap = (data[2] << 8) | data[3];
    // if (GET_BIT(keyboard_shortcut_bitmap,15) == 1) {
    //     // 如果启用BIT15，表示启用键盘预设快捷键功能，将BIT15设为1
    //     nvs_flash_info.single_click_func_idx |= 0x80;
    // } else {
    //     // 如果不启用BIT15，表示禁用键盘预设快捷键功能，将BIT15设为0
    //     nvs_flash_info.single_click_func_idx &= ~0x80;
    // }
    // if (GET_BIT(keyboard_shortcut_bitmap,14) == 1) {
    //     nvs_flash_info.double_click_func_idx |= 0x80;
    // } else {
    //     nvs_flash_info.double_click_func_idx &= ~0x80;
    // }
    // if (GET_BIT(keyboard_shortcut_bitmap,13) == 1) {
    //     nvs_flash_info.triple_click_func_idx |= 0x80;
    // } else {
    //     nvs_flash_info.triple_click_func_idx &= ~0x80;
    // }
    // startStorageTask(1);
    return 0;
}

// 键盘预设快捷键读取回调
int8_t keyboard_shortcut_read_cb(void* value, uint16_t* len){
    uint8_t* data = (uint8_t*)value;
    
    // 循环16次处理每个快捷键
    for(uint8_t i = 0; i < 16; i++) {
        uint8_t row = 0;
        uint8_t col = i;
        uint8_t func = keyboard_shortcut[row][col];
        
        // 组装数据
        uint16_t shortcut_data = ((uint16_t)row << 12) | ((uint16_t)col << 7) | func;
        
        // 写入2个字节的数据
        data[i*2] = (shortcut_data >> 8) & 0xFF;
        data[i*2 + 1] = shortcut_data & 0xFF;
        
        PRINT("Shortcut %d: row=%d, col=%d, func=%d\n", i, row, col, func);
    }
    
    *len = 32; // 设置长度为32字节(16个快捷键 * 2字节)
    return 0;
}

// 键盘预设快捷键写入回调
int8_t keyboard_shortcut_write_cb(const void* value, uint16_t len){
    const uint8_t* data = (const uint8_t*)value;
    
    // 检查数据长度是否有效
    if(len % 2 != 0 || len > 32) {
        PRINT("Invalid data length: %d\n", len);
        return -1;
    }
    
    // 计算快捷键数量
    uint8_t shortcut_count = len / 2;
    
    // 循环处理每个快捷键,最多16个
    for(uint8_t i = 0; i < shortcut_count && i < 16; i++) {
        // 读取2个字节的数据
        uint16_t shortcut_data = (data[i*2] << 8) | data[i*2 + 1];
        
        // 解析数据
        uint8_t row = (shortcut_data >> 12) & 0x0F;     // bit15 ~ bit12为行坐标
        uint8_t col = (shortcut_data >> 7) & 0x1F;      // bit11 ~ bit7为列坐标
        uint8_t func = shortcut_data & 0x7F;            // bit6 ~ bit0为功能编号
        
        // 存储到keyboard_shortcut数组
        keyboard_shortcut[row][col] = func;
        
        PRINT("Shortcut %d: row=%d, col=%d, func=%d\n", i, row, col, func);
    }
    // // 将nvs_flash_info.ai_key_func_idx 低15个bit 设置为 keyboard_shortcut[0][0] 的值
    // nvs_flash_info.single_click_func_idx = 0x80; // 清除
    // nvs_flash_info.single_click_func_idx |= (keyboard_shortcut[0][0] & 0x7F); // 设置低15BIT为keyboard_shortcut[0][0]的值
    
    // nvs_flash_info.double_click_func_idx = 0x80; // 清除
    // nvs_flash_info.double_click_func_idx |= (keyboard_shortcut[0][1] & 0x7F);

    // nvs_flash_info.triple_click_func_idx = 0x80; // 清除
    // nvs_flash_info.triple_click_func_idx |= (keyboard_shortcut[0][2] & 0x7F);
    // startStorageTask(1);
    return 0;
}

// 蓝牙名称读取回调
int8_t ble_name_read_cb(void* value, uint16_t* len){
    uint8_t* data = (uint8_t*)value;
    
    // tmos_memcpy(data, nvs_flash_info.ble_name_data, nvs_flash_info.ble_name_len);
    
    // *len = nvs_flash_info.ble_name_len;
    return 0;
}

// 蓝牙名称写入回调
int8_t ble_name_write_cb(const void* value, uint16_t len){
    const uint8_t* data = (const uint8_t*)value;
    uint8_t i =0;
    
    // 检查数据长度是否有效
    if(len > 14) {
        PRINT("Invalid data length: %d\n", len);
        return -1;
    }

    // nvs_flash_info.ble_name_len = len;
    // tmos_memcpy(nvs_flash_info.ble_name_data, data, len);
    
    // startStorageTask(1);
    return 0;
}

// 设备型号名称读取回调
int8_t device_model_name_read_cb(void* value, uint16_t* len){
    uint8_t* data = (uint8_t*)value;
    const uint8_t nameLen = strlen(ExternalCode);

    tmos_memcpy(data, ExternalCode, nameLen);
    
    *len = nameLen;
    return 0;
}
// 全局TAG写入成功回调函数
void tag_write_success_cb(const uint8_t* tags, uint16_t tag_count){
    uint16_t processed_len = 0;
    uint16_t tag = 0;
    while(processed_len < tag_count) {

        if (tags[processed_len] > 0xbf) {
            tag = (tags[processed_len] << 8) | tags[processed_len + 1];
            processed_len += 2;
        } else {
            tag = tags[processed_len];
            processed_len += 1;
        }

        switch (tag) {
            case TAG_ID_FIND_SWITCH:
                PRINT("Find switch: %d\n", find_switch);
                
                // find_flag = find_switch; // 查找开关 1 开 0 关
                // nvs_flash_info.find_flag = find_flag;
                // if(find_flag==0)
                // {
                //     start_find_flag = 0;
                //     find_enable = 0;
                // }
                // change_find_status();
                // // needStartLowAdvertising(2);
                // startStorageTask(1);
                // checkShutdown();
                break;
            case TAG_ID_FIND_ENABLE:
                PRINT("Find switch: %d\n", find_enable);
                
                // start_find_flag = find_enable; // 是否开始查找 1 开始 0 停止
                // change_find_status();
                break;
            case TAG_ID_GAIN_LEVEL:
                // gain_position = gain_level;
                // nvs_flash_info.gain_level = gain_position;
                // // 增益调节数值
                // MCP4017T_SetWiperPosition(nvs_flash_info.gain_level);
                // startStorageTask(1);
                break;
            case TAG_ID_SHUTDOWN_TIME:
                PRINT("Shutdown time: %d\n", shutdown_time);
                // 关机时间 更改 nvs_flash_info 中关机时间，并将_pd_cnt_th 设置为关机时间 最高位为0表示秒，最高位为1表示分钟，范围为0-32767，0为不关机
                // nvs_flash_info.shutdown_time = shutdown_time;
                // set_shutdown_time(nvs_flash_info.shutdown_time);
                // startStorageTask(1);
                break;
            case TAG_ID_SLEEP_TIME:
                PRINT("Sleep time: %d\n", sleep_time);
                // 关机时间 更改 nvs_flash_info 中关机时间，并将_pd_cnt_th 设置为关机时间 最高位为0表示秒，最高位为1表示分钟，范围为0-32767，0为不关机
                // nvs_flash_info.sleep_time = sleep_time;
                // set_sleep_time(nvs_flash_info.sleep_time);
                // startStorageTask(1);
                break;
            default:
                break;
        }

    }

}

void set_all_system_value(void){
    uint8_t temp = 0;
    uint8_t need_write = 0;
    uint16_t temp_bitmap  = 0;
    // never_power_down_flag = 0;
    // if (nvs_flash_info.shutdown_time == 0xffff)
    // {
    //     need_write = 1;
    //     nvs_flash_info.shutdown_time = shutdown_time;
    // } else {
    //     shutdown_time = nvs_flash_info.shutdown_time;
    // }
    // if (nvs_flash_info.sleep_time == 0xffff)
    // {
    //     need_write = 1;
    //     nvs_flash_info.sleep_time = sleep_time;
    // } else {
    //     sleep_time = nvs_flash_info.sleep_time;
    // }
    // if (nvs_flash_info.single_click_func_idx == 0xff){
    //     need_write = 1;
    //     nvs_flash_info.single_click_func_idx =  6|0x80;
    // } else {
    //     // PRINT("nvs_flash_info.ai_key_func_idx:%d",nvs_flash_info.single_click_func_idx);
    //     if (nvs_flash_info.single_click_func_idx & 0x80) {
    //         SET_BIT(temp_bitmap,15);
    //     }
    //     keyboard_shortcut[0][0] = (nvs_flash_info.single_click_func_idx & 0x7f);
    // }

    // if (nvs_flash_info.double_click_func_idx == 0xff){
    //     need_write = 1;
    //     nvs_flash_info.double_click_func_idx = 2|0x80;
    // } else {
    //     if (nvs_flash_info.double_click_func_idx & 0x80) {
    //         SET_BIT(temp_bitmap,14);
    //     }
    //     keyboard_shortcut[0][1] = (nvs_flash_info.double_click_func_idx & 0x7f);
    // }

    // if (nvs_flash_info.triple_click_func_idx == 0xff){
    //     need_write = 1;
    //     nvs_flash_info.triple_click_func_idx = 3|0x80;
    // } else {
    //     if (nvs_flash_info.triple_click_func_idx & 0x80) {
    //         SET_BIT(temp_bitmap,13);
    //     }
    //     keyboard_shortcut[0][2] = (nvs_flash_info.triple_click_func_idx & 0x7f);
    // }
    // find_switch = nvs_flash_info.find_flag;
    // gain_level = nvs_flash_info.gain_level;
    // set_shutdown_time(nvs_flash_info.shutdown_time);
    // set_sleep_time(nvs_flash_info.sleep_time);
    // find_enable = 0;
    // if (need_write == 1)
    // {
    //     startStorageTask(1);
    // }
}
// ===== 设置所有TAG默认值的函数 =====
void set_all_tags_to_default(void) 
{
    // 系统相关变量默认值
    max_mtu = _OTA_ReadMaxMTU();                // 最大支持的MTU
    curr_mtu =  _OTA_ReadMTU();            // 当前的MTU
    find_switch = 0;        // 查找功能开关，默认关闭
    gain_level = 0x4E;
    find_enable = 0;
    battery_calibration_status = 0; // 电池电量校准完成状态

    // 键盘预设快捷键默认值
    keyboard_shortcut_bitmap = 0xe000; // 键盘预设快捷键启用BITMAP
    memset(keyboard_shortcut, 0, sizeof(keyboard_shortcut)); // 键盘预设快捷键

    // 设备配置相关变量默认值
    sleep_time = 60;           // 待机时间，默认60秒
    shutdown_time = 0x8000 | ((10 * 60) / 60);

    PRINT("All tags have been set to default values\n");
}

// 如果电池电量校准完成则主动推送
void push_battery_calibration_status_change(void)
{
    uint8_t data[2];
    uint8_t len = 0;
    PRINT("push_battery_calibration_status_change..........\n");

    data[len++] = TAG_ID_BATTERY_CALIBRATION_STATUS >> 8;
    data[len++] = TAG_ID_BATTERY_CALIBRATION_STATUS & 0xFF;

    Tag_PushTags(data, len);
}

// 如果停止查找则主动推送
void push_start_find_status_change(void)
{
    uint8_t data[2];
    uint8_t len = 0;

    data[len++] = TAG_ID_FIND_ENABLE;
    Tag_PushTags(data, len);
}
