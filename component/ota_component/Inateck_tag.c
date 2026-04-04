#include "kb904/config_product.h"
#include "CH58x_common.h"
#include "Inateck_tag.h"
#include "Inateck_ota.h"
#include "Inateck_tag_config.h"


// 声明外部变量
extern const tag_descriptor_t tag_list[TAG_LIST_SIZE];

// 全局TAG写入成功回调函数指针
// 当TAG写入成功时会调用此函数通知上层
static tag_write_success_cb_t g_tag_write_success_cb = NULL;

// TAG模块初始化函数实现
// 参数: write_success_cb - TAG写入成功的回调函数
void Tag_Init(tag_write_success_cb_t write_success_cb)
{
    TAG_DEBUG_LOG("Tag_Init called");
    g_tag_write_success_cb = write_success_cb;
}

// 判断是否为2字节TAG的宏定义
// 如果最高两位都是1,则为2字节TAG
#define IS_TWO_BYTE_TAG(tag_byte) (((tag_byte) & 0xC0) == 0xC0)

// 从数据中解析TAG值
// 参数: data - 输入数据
//       parsed_len - 输出参数,存储解析的字节数
// 返回: 解析出的TAG值
static uint16_t parse_tag_from_data(const uint8_t* data, uint16_t* parsed_len) 
{
    uint16_t tag;
    
    if(IS_TWO_BYTE_TAG(data[0])) {
        // 2字节TAG,将两个字节组合成16位TAG值
        tag = (uint16_t)((data[0] << 8) | data[1]);
        *parsed_len = 2;
        TAG_DEBUG_LOG("Parsed 2-byte TAG: 0x%04X", tag);
    } else {
        // 1字节TAG,直接使用第一个字节作为TAG值
        tag = data[0];
        *parsed_len = 1;
        TAG_DEBUG_LOG("Parsed 1-byte TAG: 0x%02X", tag);
    }
    return tag;
}

// 查找TAG描述符
// 参数: tag - 要查找的TAG值
// 返回: 找到的TAG描述符指针,未找到返回NULL
static const tag_descriptor_t* find_tag_descriptor(uint16_t tag)
{
    TAG_DEBUG_LOG("Searching for TAG: 0x%04X", tag);
    for(uint16_t i = 0; i < TAG_LIST_SIZE; i++) {
        if(tag_list[i].tag == tag) {
            TAG_DEBUG_LOG("Found TAG descriptor at index %d", i);
            return &tag_list[i];//返回TAG描述符
        }
    }
    TAG_ERROR_LOG("TAG 0x%04X not found in descriptor list", tag);
    return NULL;//未找到返回NULL
}

// 读取TAG值
// 参数: desc - TAG描述符
//       out_data - 输出缓冲区,用于存储读取的值
//       out_len - 输出参数,存储读取的数据长度
// 返回: 错误码,ERR_NONE表示成功
static uint8_t read_tag_value(const tag_descriptor_t* desc, uint8_t* out_data, uint16_t* out_len)
{
    uint16_t len = 0;
    uint8_t ret = ERR_NONE;
    
    TAG_DEBUG_LOG("Reading TAG value, type: %d, access: %d", desc->type.val_type, desc->type.access);
    
    // 检查访问权限,不允许读取只写TAG
    if(desc->type.access == TAG_ACCESS_TYPE_WRITE_ONLY) {
        TAG_ERROR_LOG("Cannot read write-only TAG");
        return ERR_NOT_SUPPORTED;
    }
    
    // 根据值类型处理
    switch(desc->type.val_type) {
        case TAG_VALUE_TYPE_HEX_ARRAY://16进制数组
        case TAG_VALUE_TYPE_STRING://字符串
            if(desc->type.handle_type == TAG_HANDLE_VARIABLE) {//变量类型
                len = desc->var_size;
                if(len > desc->max_len) {
                    TAG_DEBUG_LOG("Truncating length from %d to %d", len, desc->max_len);
                    len = desc->max_len;
                }
                memcpy(out_data, desc->var_ptr, len);
            }else if(desc->type.handle_type == TAG_HANDLE_CALLBACK && desc->read_cb) {//回调类型
                // 初始化长度为0，确保即使回调没有设置，也有默认值
                *out_len = 0;
                TAG_DEBUG_LOG("Calling read callback function");
                ret = desc->read_cb(out_data, out_len);
                // 验证回调函数设置的长度是否合理
                if(ret == ERR_NONE) {
                    len = *out_len;
                    if(len > desc->max_len) {
                        TAG_ERROR_LOG("Callback returned too large length: %d, max: %d", 
                                len, desc->max_len);
                        len = desc->max_len;
                        *out_len = len;
                    }
                }
                return ret;
            }
            break;
            
        case TAG_VALUE_TYPE_HEX://16进制
            if(desc->type.handle_type == TAG_HANDLE_NORMAL) {//普通类型
                out_data[0] = desc->value.data.byte;
                len = 1;
                TAG_DEBUG_LOG("Read normal type value, length: %d", len);
            } else if(desc->type.handle_type == TAG_HANDLE_VARIABLE) {//变量类型
                out_data[0] = *((uint8_t*)desc->var_ptr);
                len = 1;
                TAG_DEBUG_LOG("Read variable type value, length: %d", len);
            } else if(desc->type.handle_type == TAG_HANDLE_CALLBACK && desc->read_cb) {//回调类型
                // 初始化长度为0，确保即使回调没有设置，也有默认值
                *out_len = 0;
                TAG_DEBUG_LOG("Calling read callback function");
                ret = desc->read_cb(out_data, out_len);
                // 验证回调函数设置的长度是否合理
                if(ret == ERR_NONE) {
                    len = *out_len;
                    // 根据类型验证长度
                    uint16_t expected_len = 0;
                    switch(desc->type.val_type) {
                        case TAG_VALUE_TYPE_HEX:
                        case TAG_VALUE_TYPE_PCT:
                        case TAG_VALUE_TYPE_BOOL:
                            expected_len = 1;
                            break;
                        case TAG_VALUE_TYPE_WORD:
                            expected_len = 2;
                            break;
                        case TAG_VALUE_TYPE_DWORD:
                            expected_len = 4;
                            break;
                        case TAG_VALUE_TYPE_FLOAT:
                            expected_len = 4;
                            break;
                    }
                    
                    if(len != expected_len) {
                        TAG_ERROR_LOG("Callback returned invalid length: %d, expected: %d", 
                                len, expected_len);
                        len = expected_len;
                        *out_len = len;
                    }
                }
                return ret;
            }
            break;
            
        case TAG_VALUE_TYPE_WORD://2字节
            if(desc->type.handle_type == TAG_HANDLE_NORMAL) {//普通类型
                uint16_t value = desc->value.data.word;
                out_data[0] = (uint8_t)(value >> 8);  // 高字节
                out_data[1] = (uint8_t)(value);       // 低字节
                len = 2;
                TAG_DEBUG_LOG("Read normal type value, length: %d", len);
            } else if(desc->type.handle_type == TAG_HANDLE_VARIABLE) {//变量类型
                uint16_t value = *((uint16_t*)desc->var_ptr);
                out_data[0] = (uint8_t)(value >> 8);  // 高字节
                out_data[1] = (uint8_t)(value);       // 低字节
                len = 2;
                TAG_DEBUG_LOG("Read variable type value, length: %d", len);
            } else if(desc->type.handle_type == TAG_HANDLE_CALLBACK && desc->read_cb) {//回调类型
                // 初始化长度为0，确保即使回调没有设置，也有默认值
                *out_len = 0;
                TAG_DEBUG_LOG("Calling read callback function");
                ret = desc->read_cb(out_data, out_len);
                // 验证回调函数设置的长度是否合理
                if(ret == ERR_NONE) {
                    len = *out_len;
                    // 根据类型验证长度
                    uint16_t expected_len = 0;
                    switch(desc->type.val_type) {
                        case TAG_VALUE_TYPE_HEX:
                        case TAG_VALUE_TYPE_PCT:
                        case TAG_VALUE_TYPE_BOOL:
                            expected_len = 1;
                            break;
                        case TAG_VALUE_TYPE_WORD:
                            expected_len = 2;
                            break;
                        case TAG_VALUE_TYPE_DWORD:
                            expected_len = 4;
                            break;
                        case TAG_VALUE_TYPE_FLOAT:
                            expected_len = 4;
                            break;
                    }
                    
                    if(len != expected_len) {
                        TAG_ERROR_LOG("Callback returned invalid length: %d, expected: %d", 
                                len, expected_len);
                        len = expected_len;
                        *out_len = len;
                    }
                }
                return ret;
            }
            break;
            
        case TAG_VALUE_TYPE_DWORD://4字节
        case TAG_VALUE_TYPE_FLOAT://浮点数
            if(desc->type.handle_type == TAG_HANDLE_NORMAL) {//普通类型
                uint32_t value = desc->value.data.dword;
                out_data[0] = (uint8_t)(value >> 24);  // 最高字节
                out_data[1] = (uint8_t)(value >> 16);
                out_data[2] = (uint8_t)(value >> 8);
                out_data[3] = (uint8_t)(value);        // 最低字节
                len = 4;
                TAG_DEBUG_LOG("Read normal type value, length: %d", len);
            } else if(desc->type.handle_type == TAG_HANDLE_VARIABLE) {//变量类型
                uint32_t value = *((uint32_t*)desc->var_ptr);
                out_data[0] = (uint8_t)(value >> 24);  // 最高字节
                out_data[1] = (uint8_t)(value >> 16);
                out_data[2] = (uint8_t)(value >> 8);
                out_data[3] = (uint8_t)(value);        // 最低字节
                len = 4;
                TAG_DEBUG_LOG("Read variable type value, length: %d", len);
            } else if(desc->type.handle_type == TAG_HANDLE_CALLBACK && desc->read_cb) {//回调类型
                // 初始化长度为0，确保即使回调没有设置，也有默认值
                *out_len = 0;
                TAG_DEBUG_LOG("Calling read callback function");
                ret = desc->read_cb(out_data, out_len);
                // 验证回调函数设置的长度是否合理
                if(ret == ERR_NONE) {
                    len = *out_len;
                    // 根据类型验证长度
                    uint16_t expected_len = 0;
                    switch(desc->type.val_type) {
                        case TAG_VALUE_TYPE_HEX:
                        case TAG_VALUE_TYPE_PCT:
                        case TAG_VALUE_TYPE_BOOL:
                            expected_len = 1;
                            break;
                        case TAG_VALUE_TYPE_WORD:
                            expected_len = 2;
                            break;
                        case TAG_VALUE_TYPE_DWORD:
                            expected_len = 4;
                            break;
                        case TAG_VALUE_TYPE_FLOAT:
                            expected_len = 4;
                            break;
                    }
                    
                    if(len != expected_len) {
                        TAG_ERROR_LOG("Callback returned invalid length: %d, expected: %d", 
                                len, expected_len);
                        len = expected_len;
                        *out_len = len;
                    }
                }
                return ret;
            }
            break;
            
        case TAG_VALUE_TYPE_PCT://百分比
        case TAG_VALUE_TYPE_BOOL://布尔值
            if(desc->type.handle_type == TAG_HANDLE_NORMAL) {//普通类型
                out_data[0] = desc->value.data.byte;
                len = 1;
                TAG_DEBUG_LOG("Read normal type value, length: %d", len);
            } else if(desc->type.handle_type == TAG_HANDLE_VARIABLE) {//变量类型
                out_data[0] = *((uint8_t*)desc->var_ptr);
                len = 1;
                TAG_DEBUG_LOG("Read variable type value, length: %d", len);
            } else if(desc->type.handle_type == TAG_HANDLE_CALLBACK && desc->read_cb) {//回调类型
                // 初始化长度为0，确保即使回调没有设置，也有默认值
                *out_len = 0;
                TAG_DEBUG_LOG("Calling read callback function");
                ret = desc->read_cb(out_data, out_len);
                // 验证回调函数设置的长度是否合理
                if(ret == ERR_NONE) {
                    len = *out_len;
                    // 根据类型验证长度
                    uint16_t expected_len = 0;
                    switch(desc->type.val_type) {
                        case TAG_VALUE_TYPE_HEX:
                        case TAG_VALUE_TYPE_PCT:
                        case TAG_VALUE_TYPE_BOOL:
                            expected_len = 1;
                            break;
                        case TAG_VALUE_TYPE_WORD:
                            expected_len = 2;
                            break;
                        case TAG_VALUE_TYPE_DWORD:
                            expected_len = 4;
                            break;
                        case TAG_VALUE_TYPE_FLOAT:
                            expected_len = 4;
                            break;
                    }
                    
                    if(len != expected_len) {
                        TAG_ERROR_LOG("Callback returned invalid length: %d, expected: %d", 
                                len, expected_len);
                        len = expected_len;
                        *out_len = len;
                    }
                }
                return ret;
            }
            break;
            
        default:
            TAG_ERROR_LOG("Unsupported value type: %d", desc->type.val_type);
            return ERR_NOT_SUPPORTED;
    }
    
    *out_len = len;
    TAG_DEBUG_LOG("Successfully read TAG value, length: %d", len);
    return ERR_NONE;
}

// 写入TAG值
// 参数: desc - TAG描述符
//       data - 要写入的数据
//       len - 数据长度
// 返回: 错误码,ERR_NONE表示成功
static uint8_t write_tag_value(const tag_descriptor_t* desc, const uint8_t* data, uint16_t len)
{
    TAG_DEBUG_LOG("Writing TAG value, type: %d, access: %d, length: %d", 
            desc->type.val_type, desc->type.access, len);
    
    // 检查访问权限,不允许写入只读TAG
    if(desc->type.access == TAG_ACCESS_TYPE_READ_ONLY) {
        TAG_ERROR_LOG("Cannot write read-only TAG");
        return ERR_NOT_SUPPORTED;
    }
    
    // 根据值类型处理
    switch(desc->type.val_type) {
        case TAG_VALUE_TYPE_HEX_ARRAY:
        case TAG_VALUE_TYPE_STRING:
            if(len > desc->max_len) {
                TAG_ERROR_LOG("Data too long: %d, max allowed: %d", len, desc->max_len);
                return ERR_TOO_MANY;
            }
            if(desc->type.handle_type == TAG_HANDLE_VARIABLE) {
                memcpy(desc->var_ptr, data, len);
                if(desc->var_size > len) {
                    // 字符串类型,添加结束符
                    ((uint8_t*)desc->var_ptr)[len] = 0;
                }
                TAG_DEBUG_LOG("Wrote %d bytes to variable", len);
            }else if(desc->type.handle_type == TAG_HANDLE_CALLBACK && desc->write_cb) {
                TAG_DEBUG_LOG("Calling write callback function");
                return desc->write_cb(data, len);
            }
            break;
            
        case TAG_VALUE_TYPE_HEX://16进制
            if(desc->type.handle_type == TAG_HANDLE_NORMAL) {//普通类型
                // 使用memcpy替代直接赋值，因为desc是const指针
                uint8_t byte_val = data[0];
                memcpy((void*)&(desc->value.data.byte), &byte_val, sizeof(uint8_t));
                TAG_DEBUG_LOG("Wrote normal type value");
            } else if(desc->type.handle_type == TAG_HANDLE_VARIABLE) {//变量类型
                *((uint8_t*)desc->var_ptr) = data[0];
                TAG_DEBUG_LOG("Wrote variable type value");
            } else if(desc->type.handle_type == TAG_HANDLE_CALLBACK && desc->write_cb) {//回调类型
                TAG_DEBUG_LOG("Calling write callback function");
                return desc->write_cb(data, len);
            }
            break;
            
        case TAG_VALUE_TYPE_WORD://2字节
            if(desc->type.handle_type == TAG_HANDLE_NORMAL) {//普通类型
                // 使用memcpy替代直接赋值
                uint16_t word_val = ((uint16_t)data[0] << 8) | data[1];
                memcpy((void*)&(desc->value.data.word), &word_val, sizeof(uint16_t));
                TAG_DEBUG_LOG("Wrote normal type value");
            } else if(desc->type.handle_type == TAG_HANDLE_VARIABLE) {//变量类型
                *((uint16_t*)desc->var_ptr) = ((uint16_t)data[0] << 8) | data[1];
                TAG_DEBUG_LOG("Wrote variable type value");
            } else if(desc->type.handle_type == TAG_HANDLE_CALLBACK && desc->write_cb) {//回调类型
                TAG_DEBUG_LOG("Calling write callback function");
                return desc->write_cb(data, len);
            }
            break;
            
        case TAG_VALUE_TYPE_DWORD://4字节
        case TAG_VALUE_TYPE_FLOAT://浮点数
            if(desc->type.handle_type == TAG_HANDLE_NORMAL) {//普通类型
                // 使用memcpy替代直接赋值
                uint32_t dword_val = ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) | 
                                     ((uint32_t)data[2] << 8) | data[3];
                memcpy((void*)&(desc->value.data.dword), &dword_val, sizeof(uint32_t));
                TAG_DEBUG_LOG("Wrote normal type value");
            } else if(desc->type.handle_type == TAG_HANDLE_VARIABLE) {//变量类型
                *((uint32_t*)desc->var_ptr) = ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) | 
                                              ((uint32_t)data[2] << 8) | data[3];
                TAG_DEBUG_LOG("Wrote variable type value");
            } else if(desc->type.handle_type == TAG_HANDLE_CALLBACK && desc->write_cb) {//回调类型
                TAG_DEBUG_LOG("Calling write callback function");
                return desc->write_cb(data, len);
            }
            break;
            
        case TAG_VALUE_TYPE_PCT://百分比
        case TAG_VALUE_TYPE_BOOL://布尔值
            if(desc->type.handle_type == TAG_HANDLE_NORMAL) {//普通类型
                // 使用memcpy替代直接赋值
                uint8_t byte_val = data[0];
                memcpy((void*)&(desc->value.data.byte), &byte_val, sizeof(uint8_t));
                TAG_DEBUG_LOG("Wrote normal type value");
            } else if(desc->type.handle_type == TAG_HANDLE_VARIABLE) {//变量类型
                *((uint8_t*)desc->var_ptr) = data[0];
                TAG_DEBUG_LOG("Wrote variable type value");
            } else if(desc->type.handle_type == TAG_HANDLE_CALLBACK && desc->write_cb) {//回调类型
                TAG_DEBUG_LOG("Calling write callback function");
                return desc->write_cb(data, len);
            }
            break;
            
        default:
            TAG_ERROR_LOG("Unsupported value type: %d", desc->type.val_type);
            return ERR_NOT_SUPPORTED;
    }
    
    TAG_DEBUG_LOG("Successfully wrote TAG value");
    return ERR_NONE;
}

// 获取TAG值的长度
// 参数: desc - TAG描述符
// 返回: TAG值的长度
static uint16_t get_tag_value_length(const tag_descriptor_t* desc)
{
    uint16_t len = 0;
    switch(desc->type.val_type) {
        case TAG_VALUE_TYPE_HEX:
            len = 1;
            break;
        case TAG_VALUE_TYPE_WORD:
            len = 2;
            break;
        case TAG_VALUE_TYPE_DWORD:
        case TAG_VALUE_TYPE_FLOAT:
            len = 4;
            break;
        case TAG_VALUE_TYPE_PCT:
        case TAG_VALUE_TYPE_BOOL:
            len = 1;
            break;
        case TAG_VALUE_TYPE_HEX_ARRAY:
        case TAG_VALUE_TYPE_STRING:
            len = desc->max_len;
            break;
        default:
            len = 0;
            break;
    }
    TAG_DEBUG_LOG("TAG value length for type %d: %d", desc->type.val_type, len);
    return len;
}

// 添加TAG到响应缓冲区
// 参数: tag - TAG值
//       type - TAG类型
//       response - 响应缓冲区
//       resp_len - 输入输出参数,当前响应长度
// 返回: 错误码,ERR_NONE表示成功
static uint8_t add_tag_to_response(uint16_t tag, tag_type_t type, uint8_t* response, uint16_t* resp_len)
{
    TAG_DEBUG_LOG("Adding TAG 0x%04X to response at position %d", tag, *resp_len);
    
    // 检查缓冲区大小
    if(*resp_len + 3 > APP_MAX_TX_SIZE) { // TAG(2字节) + type(1字节)
        TAG_ERROR_LOG("Response buffer full at position %d", *resp_len);
        return ERR_TOO_MANY;
    }
    
    // 添加TAG (2字节)
    if(tag > 0xBF)
    {
        response[(*resp_len)++] = (uint8_t)(tag >> 8);
        response[(*resp_len)++] = (uint8_t)tag;
        TAG_DEBUG_LOG("Added 2-byte TAG: 0x%04X", tag);
    }
    else
    {
        response[(*resp_len)++] = (uint8_t)tag;
        TAG_DEBUG_LOG("Added 1-byte TAG: 0x%02X", tag);
    }
    
    // 添加type (1字节)
    response[(*resp_len)++] = *(uint8_t*)&type;
    TAG_DEBUG_LOG("Added TAG type: 0x%02X", *(uint8_t*)&type);
    
    return ERR_NONE;
}

// 命令回复函数声明
extern void _OTA_Command_Reply(uint8_t command_code, uint8_t error_code, uint8_t *ret_parm, uint8_t ret_len);

// TAG命令处理函数
// 参数: p_cmd - 命令数据
//       sz - 命令数据长度
// 返回: 错误码,ERR_NONE表示成功
uint8_t Tag_CommandProcessor(uint8_t* p_cmd, uint16_t sz)
{
    tag_cmd_manage_t *tag_cmd = (tag_cmd_manage_t *)p_cmd;
    uint8_t *data = tag_cmd->tag_data.tag_data;
    uint16_t data_len = sz - 1; // 减去命令码长度
    uint8_t ret = ERR_NONE;
    uint8_t response[APP_MAX_TX_SIZE];
    uint16_t resp_len = 0;
    uint16_t processed_len = 0; // 已处理的数据长度
    
    // 用于收集设置的TAG
    uint8_t set_tags[APP_MAX_TX_SIZE];
    uint16_t set_tag_count = 0;
    
    TAG_DEBUG_LOG("Processing command: 0x%02X, data length: %d", tag_cmd->tag_data.cmd, data_len);
    
    if(tag_cmd->tag_data.cmd == OTA_CMD_SET_TAGS || tag_cmd->tag_data.cmd == OTA_CMD_READ_TAGS) {
        while(processed_len < data_len) {
            // 解析TAG
            uint16_t parsed_len;
            uint16_t tag = parse_tag_from_data(data + processed_len, &parsed_len);
            processed_len += parsed_len;
            
            // 查找TAG描述符
            const tag_descriptor_t* desc = find_tag_descriptor(tag);
            if(!desc) {
                TAG_ERROR_LOG("TAG 0x%04X not found", tag);
                ret = ERR_NOT_EXIST;
                break;
            }
            
            switch(tag_cmd->tag_data.cmd) {
                case OTA_CMD_SET_TAGS: {//设置TAGs
                    TAG_DEBUG_LOG("Setting TAG 0x%04X", tag);
                    // 获取值的长度
                    uint16_t value_len;
                    
                    // 对于变长类型,需要从数据中获取实际长度
                    if(desc->type.val_type == TAG_VALUE_TYPE_HEX_ARRAY || 
                       desc->type.val_type == TAG_VALUE_TYPE_STRING) {
                        // 确保至少有一个字节表示长度
                        if(processed_len >= data_len) {
                            TAG_ERROR_LOG("Data length error: no length byte");
                            ret = ERR_FORMAT;
                            goto exit;
                        }
                        
                        // 读取长度字节
                        value_len = data[processed_len++];
                        TAG_DEBUG_LOG("Variable length data, length: %d", value_len);
                        
                        // 验证长度是否合理
                        if(value_len > desc->max_len) {
                            TAG_ERROR_LOG("Data length too large: %d, max: %d", value_len, desc->max_len);
                            ret = ERR_TOO_MANY;
                            goto exit;
                        }
                    } else {
                        // 固定长度类型使用get_tag_value_length
                        value_len = get_tag_value_length(desc);
                        TAG_DEBUG_LOG("Fixed length data, length: %d", value_len);
                    }
                    
                    // 检查剩余数据是否足够
                    if(processed_len + value_len > data_len) {
                        TAG_ERROR_LOG("Data length error: need %d, remain %d", 
                                value_len, data_len - processed_len);
                        ret = ERR_FORMAT;
                        goto exit;
                    }
                    
                    // 写入TAG值
                    ret = write_tag_value(desc, data + processed_len, value_len);
                    if(ret != ERR_NONE) {
                        TAG_ERROR_LOG("Failed to write TAG value, error: %d", ret);
                        goto exit;
                    }
                    
                    // 收集成功设置的TAG
                    if(tag >= 0xC0) {
                        // 2字节TAG
                        set_tags[set_tag_count++] = (uint8_t)(tag >> 8);
                        set_tags[set_tag_count++] = (uint8_t)tag;
                    } else {
                        // 1字节TAG
                        set_tags[set_tag_count++] = (uint8_t)tag;
                    }
                    
                    processed_len += value_len;
#if TAG_DEBUG_LOG_ENABLE
                    // 打印设置的值
                    TAG_DEBUG_LOG("Set TAG 0x%04X, value length: %d", tag, value_len);
                    switch(desc->type.val_type) {
                        case TAG_VALUE_TYPE_HEX:
                            TAG_DEBUG_LOG("Value: 0x%02X", *(uint8_t*)(data + processed_len - value_len));
                            break;
                        case TAG_VALUE_TYPE_WORD:
                            TAG_DEBUG_LOG("Value: 0x%04X", *(uint16_t*)(data + processed_len - value_len));
                            break;
                        case TAG_VALUE_TYPE_DWORD:
                        case TAG_VALUE_TYPE_QWORD:
                            TAG_DEBUG_LOG("Value: 0x%08X", *(uint32_t*)(data + processed_len - value_len));
                            break;
                        case TAG_VALUE_TYPE_PCT:
                        case TAG_VALUE_TYPE_BOOL:
                            TAG_DEBUG_LOG("Value: %d", *(uint8_t*)(data + processed_len - value_len));
                            break;
                        case TAG_VALUE_TYPE_FLOAT:
                            TAG_DEBUG_LOG("Value: %f", *(float*)(data + processed_len - value_len));
                            break;
                        case TAG_VALUE_TYPE_HEX_ARRAY:
                            TAG_DEBUG_LOG("Value: ");
                            for(uint8_t i = 0; i < value_len; i++) {
                                TAG_DEBUG_LOG("%02X ", *(uint8_t*)(data + processed_len - value_len + i));
                            }
                            break;
                        case TAG_VALUE_TYPE_STRING:
                            TAG_DEBUG_LOG("Value: %s", (char*)(data + processed_len - value_len));
                            break;
                    }
#endif
                    break;
                }
                
                case OTA_CMD_READ_TAGS: {//读取TAGs
                    TAG_DEBUG_LOG("Reading TAG 0x%04X", tag);
                    // 检查响应缓冲区是否足够
                    if(resp_len + 6 > APP_MAX_TX_SIZE) { // 预留6字节用于下一个可能的TAG
                        TAG_ERROR_LOG("Response buffer full at position %d", resp_len);
                        goto exit;
                    }
                    
                    // 在响应中添加TAG
                    if(tag > 0xBF)
                    {
                        response[resp_len++] = (uint8_t)(tag >> 8);
                        response[resp_len++] = (uint8_t)tag;
                        TAG_DEBUG_LOG("Added 2-byte TAG to response: 0x%04X", tag);
                    }
                    else
                    {
                        response[resp_len++] = (uint8_t)tag;
                        TAG_DEBUG_LOG("Added 1-byte TAG to response: 0x%02X", tag);
                    }
                    
                    // 对于变长类型,需要预留一个字节存放长度
                    uint16_t length_pos = 0;
                    if(desc->type.val_type == TAG_VALUE_TYPE_HEX_ARRAY || 
                       desc->type.val_type == TAG_VALUE_TYPE_STRING) {
                        // 预留一个字节用于存放长度
                        length_pos = resp_len;
                        resp_len++;
                        TAG_DEBUG_LOG("Reserved length byte at position %d", length_pos);
                    }
                    
                    // 读取TAG值
                    uint16_t value_len = 0;
                    
                    ret = read_tag_value(desc, response + resp_len, &value_len);
                    if(ret != ERR_NONE) {
                        TAG_ERROR_LOG("Failed to read TAG value, error: %d", ret);
                        goto exit;
                    }
                    
                    // 验证读取的数据长度不超过缓冲区
                    if(value_len > APP_MAX_TX_SIZE - resp_len) {
                        TAG_ERROR_LOG("Value too large for buffer: %d, max: %d", value_len, APP_MAX_TX_SIZE - resp_len);
                        value_len = APP_MAX_TX_SIZE - resp_len;
                    }
                    
                    // 对于变长类型,回填长度字节
                    if(desc->type.val_type == TAG_VALUE_TYPE_HEX_ARRAY || 
                       desc->type.val_type == TAG_VALUE_TYPE_STRING) {
                        if(value_len > 255) {
                            value_len = 255; // 长度字节最大为255
                        }
                        response[length_pos] = (uint8_t)value_len;
                        TAG_DEBUG_LOG("Filled length byte: %d", value_len);
                    }
                    
                    resp_len += value_len;
#if TAG_DEBUG_LOG_ENABLE
                    TAG_DEBUG_LOG("Read TAG 0x%04X, value length: %d", tag, value_len);
                    // 打印读取的值
                    switch(desc->type.val_type) {
                        case TAG_VALUE_TYPE_HEX:
                            TAG_DEBUG_LOG("Value: 0x%02X", *(uint8_t*)(response + resp_len - value_len));
                            break;
                        case TAG_VALUE_TYPE_WORD:
                            TAG_DEBUG_LOG("Value: 0x%04X", *(uint16_t*)(response + resp_len - value_len));
                            break;
                        case TAG_VALUE_TYPE_DWORD:
                        case TAG_VALUE_TYPE_QWORD:
                            TAG_DEBUG_LOG("Value: 0x%08X", *(uint32_t*)(response + resp_len - value_len));
                            break;
                        case TAG_VALUE_TYPE_PCT:
                        case TAG_VALUE_TYPE_BOOL:
                            TAG_DEBUG_LOG("Value: %d", *(uint8_t*)(response + resp_len - value_len));
                            break;
                        case TAG_VALUE_TYPE_FLOAT:
                            TAG_DEBUG_LOG("Value: %f", *(float*)(response + resp_len - value_len));
                            break;
                        case TAG_VALUE_TYPE_HEX_ARRAY:
                            TAG_DEBUG_LOG("Value: ");
                            for(uint8_t i = 0; i < value_len; i++) {
                                TAG_DEBUG_LOG("%02X ", *(uint8_t*)(response + resp_len - value_len + i));
                            }
                            break;
                        case TAG_VALUE_TYPE_STRING:
                            TAG_DEBUG_LOG("Value: %s", (char*)(response + resp_len - value_len));
                            break;
                    }
#endif
                    break;
                }
                
                default:
                    TAG_ERROR_LOG("Unsupported command: 0x%02X", tag_cmd->tag_data.cmd);
                    ret = ERR_NOT_SUPPORTED;
                    goto exit;
            }
        }
    }
    else if(tag_cmd->tag_data.cmd == OTA_CMD_PULL_TAGS) {
        TAG_DEBUG_LOG("Processing PULL_TAGS command");
        // 拉取所有TAG信息
        resp_len = 0;
        
        // 遍历tag_list
        for(uint16_t i = 0; i < TAG_LIST_SIZE; i++) {
            ret = add_tag_to_response(tag_list[i].tag, tag_list[i].type, response, &resp_len);
            if(ret != ERR_NONE) {
                TAG_ERROR_LOG("Failed to add TAG 0x%04X to response, error: %d", tag_list[i].tag, ret);
                goto exit;
            }
            TAG_DEBUG_LOG("Pull TAG 0x%04X, type: 0x%02X", tag_list[i].tag, *(uint8_t*)&tag_list[i].type);
        }
        TAG_DEBUG_LOG("Successfully pulled all TAGs, response length: %d", resp_len);
    }
    else {
        TAG_ERROR_LOG("Unsupported command: 0x%02X", tag_cmd->tag_data.cmd);
        ret = ERR_NOT_SUPPORTED;
        goto exit;
    }

exit:
    TAG_DEBUG_LOG("Command processing complete, result: %d", ret);

    // 发送响应
    if(ret == ERR_NONE && resp_len > 0) {
        TAG_DEBUG_LOG("Sending success response, length: %d", resp_len);
        _OTA_Command_Reply(tag_cmd->tag_data.cmd + 1, ret, response, resp_len);
    } else {
        TAG_DEBUG_LOG("Sending error response, code: %d", ret);
        _OTA_Command_Reply(tag_cmd->tag_data.cmd + 1, ret, NULL, 0);
    }

     // 如果是设置TAG命令且执行成功,调用回调函数
    if(ret == ERR_NONE && tag_cmd->tag_data.cmd == OTA_CMD_SET_TAGS && set_tag_count > 0 && g_tag_write_success_cb != NULL) {
        TAG_DEBUG_LOG("Calling write success callback for %d TAGs", set_tag_count);
        g_tag_write_success_cb(set_tags, set_tag_count);
    }   
    return ret;
}


//推送TAGs
uint8_t Tag_PushTags(uint8_t* tag_list, uint16_t tag_count)
{
    uint8_t response[APP_MAX_TX_SIZE];
    uint16_t resp_len = 0;
    uint16_t processed_len = 0; // 已处理的数据长度
    uint8_t ret = ERR_NONE;  // 添加返回值变量
    
    TAG_DEBUG_LOG("Starting TAG push, count: %d", tag_count);
    
    while(processed_len < tag_count) {
        uint16_t tag_len = 0;  // 修改为 uint16_t 类型
        // 解析TAG
        uint16_t tags = parse_tag_from_data(tag_list + processed_len, &tag_len);
        if(tags == 0) {
            TAG_ERROR_LOG("Failed to parse TAG");
            return ERR_NOT_EXIST;
        }

        // 查找TAG描述符
        const tag_descriptor_t* desc = find_tag_descriptor(tags);
        if(!desc) {
            TAG_ERROR_LOG("TAG 0x%04X not found in descriptor list", tags);
            return ERR_NOT_EXIST;
        }

        // 在推送中添加TAG
        if(tags > 0xBF)
        {
            response[resp_len++] = (uint8_t)(tags >> 8);
            response[resp_len++] = (uint8_t)tags;
            TAG_DEBUG_LOG("Added 2-byte TAG to push: 0x%04X", tags);
        }
        else
        {
            response[resp_len++] = (uint8_t)tags;
            TAG_DEBUG_LOG("Added 1-byte TAG to push: 0x%02X", tags);
        }     

        //如果TAG类型为HEX_ARRAY或STRING，则要预留一个字节存放长度
        uint16_t length_pos = 0;
        if(desc->type.val_type == TAG_VALUE_TYPE_HEX_ARRAY || 
            desc->type.val_type == TAG_VALUE_TYPE_STRING) {
            // 预留一个字节用于存放长度
            length_pos = resp_len;
            resp_len++;
            TAG_DEBUG_LOG("Reserved length byte at position %d", length_pos);
        }

        // 读取TAG值
        uint16_t value_len = 0;
        
        ret = read_tag_value(desc, response + resp_len, &value_len);
        if(ret != ERR_NONE) {
            TAG_ERROR_LOG("Failed to read TAG value, error: %d", ret);
            return ret;
        }

        resp_len += value_len;

        //如果TAG类型为HEX_ARRAY或STRING，则要回填长度字节
        if(desc->type.val_type == TAG_VALUE_TYPE_HEX_ARRAY || 
            desc->type.val_type == TAG_VALUE_TYPE_STRING) {
            response[length_pos] = (uint8_t)value_len;
            TAG_DEBUG_LOG("Filled length byte: %d", value_len);
        }

        processed_len += tag_len;
        TAG_DEBUG_LOG("Processed TAG 0x%04X, value length: %d", tags, value_len);
    }

    TAG_DEBUG_LOG("Sending push command, response length: %d", resp_len);
    _OTA_Command_Send(OTA_CMD_PUSH_TAGS, response, resp_len);
    
    return ERR_NONE;  // 添加返回值
}





