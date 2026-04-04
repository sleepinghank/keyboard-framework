#ifndef _INATECK_TAG_H_
#define _INATECK_TAG_H_

#include "kb904/config_product.h"
#include "CH58x_common.h"
#include "Inateck_ota.h"

// 打印开关宏定义
#define TAG_DEBUG_LOG_ENABLE    0   // 设置为1启用打印，设置为0禁用打印

// 根据打印开关定义打印宏
#if TAG_DEBUG_LOG_ENABLE
    #define TAG_DEBUG_LOG(...)     PRINT(__VA_ARGS__)    // 调试日志打印
    #define TAG_ERROR_LOG(...)     PRINT(__VA_ARGS__)    // 错误日志打印
#else
    #define TAG_DEBUG_LOG(...)     // 禁用调试日志打印
    #define TAG_ERROR_LOG(...)     // 禁用错误日志打印
#endif

// TAG命令定义 (0x50-0x5F 为配置命令)

// 设置TAGs相关命令
#define OTA_CMD_SET_TAGS                         0x50    // 设置TAGs命令码
#define OTA_CMD_SET_TAGS_REPLY                   0x51    // 设置TAGs回复命令码

// 读取TAGs相关命令
#define OTA_CMD_READ_TAGS                        0x52    // 读取TAGs命令码
#define OTA_CMD_READ_TAGS_REPLY                  0x53    // 读取TAGs回复命令码

// 拉取TAGs相关命令
#define OTA_CMD_PULL_TAGS                        0x54    // 拉取TAGs命令码
#define OTA_CMD_PULL_TAGS_REPLY                  0x55    // 拉取TAGs回复命令码

// 推送TAGs命令
#define OTA_CMD_PUSH_TAGS                        0x56    // 推送TAGs命令码

// TAG值类型定义（范围为0-15）
typedef enum {
    TAG_VALUE_TYPE_HEX = 0,    // 单字节十六进制值（0-255）
    TAG_VALUE_TYPE_WORD,       // 双字节值（0-65535）
    TAG_VALUE_TYPE_DWORD,      // 三字节值（0-16777215）
    TAG_VALUE_TYPE_QWORD,      // 四字节值（0-4294967295）
    TAG_VALUE_TYPE_PCT,        // 百分比值（0-100）
    TAG_VALUE_TYPE_BOOL,       // 布尔值（0-1）
    TAG_VALUE_TYPE_FLOAT,      // 浮点数值
    TAG_VALUE_TYPE_HEX_ARRAY,  // 十六进制数组
    TAG_VALUE_TYPE_STRING      // 字符串类型
} tag_value_type_t;

// TAG访问权限枚举定义
typedef enum {
    TAG_ACCESS_TYPE_READ_ONLY = 0,  // 只读权限
    TAG_ACCESS_TYPE_WRITE_ONLY,     // 只写权限
    TAG_ACCESS_TYPE_READ_WRITE      // 读写权限
} tag_access_type_t;

// TAG处理方式定义
typedef enum {
    TAG_HANDLE_NORMAL = 0,    // 普通处理方式(直接存储在TAG节点中)
    TAG_HANDLE_VARIABLE,      // 变量处理方式(直接读写变量)
    TAG_HANDLE_CALLBACK       // 回调处理方式(通过回调函数处理)
} tag_handle_type_t;

// TAG类型定义结构体
typedef struct {
    tag_value_type_t val_type : 4;    // 值类型，4位，参考tag_value_type_t枚举定义
    tag_access_type_t access : 2;      // 访问权限，2位，参考tag_access_type_t枚举定义
    tag_handle_type_t handle_type : 2; // 处理方式，2位，参考tag_handle_type_t定义
} tag_type_t;

// TAG回调函数类型定义
typedef int8_t (*tag_read_cb_t)(void* value, uint16_t* len);   // 读取回调函数
typedef int8_t (*tag_write_cb_t)(const void* value, uint16_t len); // 写入回调函数

// TAG值联合体定义
typedef struct {
    union {
        uint8_t  byte;           // 单字节值存储
        uint16_t word;           // 双字节值存储
        uint32_t dword;          // 四字节值存储
        uint8_t  bytes[4];       // 字节数组形式访问存储
    } data;
} tag_value_t;

// TAG完整描述结构体
typedef struct {
    uint16_t tag;                // TAG标识符
    tag_type_t type;             // TAG类型(包含值类型、处理方式和访问权限)
    uint8_t max_len;             // 最大长度（用于数组和字符串类型）
    tag_value_t value;           // TAG值(用于普通处理方式)
    void* var_ptr;               // 变量指针(用于变量处理方式)
    uint8_t var_size;            // 当前数据长度(用于变量处理方式)
    tag_read_cb_t read_cb;       // 读取回调函数(用于回调处理方式)
    tag_write_cb_t write_cb;     // 写入回调函数(用于回调处理方式)
} tag_descriptor_t;

// TAG命令管理联合体
typedef union
{
    struct
    {
        uint8_t cmd;             // 命令码
        uint8_t tag_data[APP_MAX_TX_SIZE - 1];  // TAG数据
    } tag_data;   // TAG数据结构
} tag_cmd_manage_t;

// TAG写入成功回调函数类型定义
typedef void (*tag_write_success_cb_t)(const uint8_t* tags, uint16_t tag_count);

// 函数声明
void Tag_Init(tag_write_success_cb_t write_success_cb);    // TAG模块初始化函数
uint8_t Tag_CommandProcessor(uint8_t* p_cmd, uint16_t sz); // TAG命令处理函数
uint8_t Tag_PushTags(uint8_t* tag_list, uint16_t tag_count); // TAG推送函数

#endif // _INATECK_TAG_H_ 



