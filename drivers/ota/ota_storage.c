/********************************** (C) COPYRIGHT *******************************
 * 文件名称          : ota_storage.c
 * 作者             : 
 * 版本             : V1.0
 * 日期             : 2024/04/07
 * 描述             : OTA存储操作相关函数实现
 *********************************************************************************
 * 版权所有 (c) 2024 
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "ota_storage.h"
#include "CONFIG.h"
#include "crc16.h"

/*********************************************************************
 * 宏定义
 */

// 调试打印宏
#if 0
#define OTA_STORAGE_PRINT(...) printf(__VA_ARGS__)
#else
#define OTA_STORAGE_PRINT(...)
#endif

/*********************************************************************
 * 私有类型定义
 */
/* OTA储存区属性定义 */
typedef enum {
    OTA_STORAGE_ATTRIBUTE_READ_ONLY = 0x00, // 只读
    OTA_STORAGE_ATTRIBUTE_READ_WRITE = 0x01 // 读写
} ota_storage_attribute_t;

/* OTA存储区域信息结构体 */
typedef struct {
    ota_storage_tag_t tag;    // 标签
    uint32_t start_addr;    // 起始地址
    uint32_t size;          // 大小
    ota_storage_attribute_t attribute;      // 属性
} ota_storage_region_t;







/*********************************************************************
 * 私有变量
 */

//定义储存区起始地址
#define OTA_TAG_JUMP_FLAG_ADDR OTA_DATAFLASH_START_ADDR
#define OTA_TAG_DEVICE_INFO_ADDR (OTA_TAG_JUMP_FLAG_ADDR + OTA_DATAFLASH_BLOCK_SIZE)
#define OTA_TAG_FIRMWARE_INFO_ADDR (OTA_TAG_DEVICE_INFO_ADDR + OTA_DATAFLASH_BLOCK_SIZE)
#define APP_TAG_USER_CONFIG_INFO_ADDR (OTA_TAG_FIRMWARE_INFO_ADDR + OTA_DATAFLASH_BLOCK_SIZE)

/* OTA存储区域信息表 */
static const ota_storage_region_t ota_storage_regions[OTA_TAG_MAX] = {
    {OTA_TAG_JUMP_FLAG,         OTA_TAG_JUMP_FLAG_ADDR,       OTA_DATAFLASH_BLOCK_SIZE, OTA_STORAGE_ATTRIBUTE_READ_WRITE},  // IMAGE跳转标志
    {OTA_TAG_DEVICE_INFO,       OTA_TAG_DEVICE_INFO_ADDR,     OTA_DATAFLASH_BLOCK_SIZE, OTA_STORAGE_ATTRIBUTE_READ_WRITE},  // 设备信息
    {OTA_TAG_FIRMWARE_INFO,     OTA_TAG_FIRMWARE_INFO_ADDR,   OTA_DATAFLASH_BLOCK_SIZE, OTA_STORAGE_ATTRIBUTE_READ_WRITE},   // 固件信息
    {APP_TAG_USER_CONFIG_INFO,  APP_TAG_USER_CONFIG_INFO_ADDR,OTA_DATAFLASH_BLOCK_SIZE, OTA_STORAGE_ATTRIBUTE_READ_WRITE}   // 用户配置
};



/*********************************************************************
 * 函数实现
 */


/**
 * @brief   擦除OTA存储区域
 * 
 * @param   tag     存储标签
 * 
 * @return  ota_storage_result_t 擦除结果
 */
ota_storage_result_t OTA_Storage_Erase(ota_storage_tag_t tag)
{
    if(tag >= OTA_TAG_MAX) {
        OTA_STORAGE_PRINT("OTA Storage Erase: Invalid tag %d\r\n", tag);
        return OTA_STORAGE_INVALID_TAG;
    }

    // 检查储存区属性，只读区域不允许擦除
    if(ota_storage_regions[tag].attribute == OTA_STORAGE_ATTRIBUTE_READ_ONLY) {
        OTA_STORAGE_PRINT("OTA Storage Erase: Read-only region %d\r\n", tag);
        return OTA_STORAGE_READ_ONLY;
    }

    //判断ota_storage_regions[tag].size是否等于OTA_DATAFLASH_BLOCK_SIZE，如果等于则只需要擦除一次，否则需要擦除多次
    if(ota_storage_regions[tag].size <= OTA_DATAFLASH_BLOCK_SIZE) {
        EEPROM_ERASE(ota_storage_regions[tag].start_addr, ota_storage_regions[tag].size);
    } else {
        OTA_STORAGE_PRINT("OTA Storage Erase:Cross-region access denied\r\n");
    }
    OTA_STORAGE_PRINT("OTA Storage Erase: Tag %d\r\n", tag);
    return OTA_STORAGE_OK;
}



/**
 * @brief   读取OTA存储数据
 * 
 * @param   tag     存储标签
 * @param   buffer  数据缓冲区
 * @param   size    要读取的数据大小
 * 
 * @return  ota_storage_result_t 读取结果
 */
ota_storage_result_t OTA_Storage_Read(ota_storage_tag_t tag, uint8_t *buffer, uint16_t size)
{
    if(tag >= OTA_TAG_MAX) {
        OTA_STORAGE_PRINT("错误:标签无效%d\r\n", tag);
        return OTA_STORAGE_INVALID_TAG;
    }
    
    if(size > ota_storage_regions[tag].size) {
        OTA_STORAGE_PRINT("错误:大小超出范围%d-%d\r\n", size, tag);
        return OTA_STORAGE_INVALID_SIZE;
    }
    
    if (buffer == NULL) {
        OTA_STORAGE_PRINT("错误:缓冲区为空\r\n");
        return OTA_STORAGE_ERROR;
    }
    
    uint8_t pBuffer_read[OTA_DATAFLASH_BLOCK_SIZE] = {0};
    EEPROM_READ(ota_storage_regions[tag].start_addr, (uint32_t *)&pBuffer_read[0], OTA_DATAFLASH_BLOCK_SIZE); // 读取数据(包含2字节长度和2字节CRC)

    //读取长度
    uint16_t read_size = (pBuffer_read[1] << 8) | pBuffer_read[0];
    
    if(read_size != size) {
        OTA_STORAGE_PRINT("错误:读取长度不匹配%d-%d\r\n", read_size, size);
        return OTA_STORAGE_ERROR;
    }

    //计算CRC校验
    uint16_t crc = GetQuickCRC16(pBuffer_read, size + 2); // 计算包含长度的CRC

    //比较CRC校验
    uint16_t read_crc = (pBuffer_read[size + 3] << 8) | pBuffer_read[size + 2];
    if(crc != read_crc) {
        OTA_STORAGE_PRINT("错误:CRC校验失败\r\n");
        return OTA_STORAGE_ERROR;
    }

    //复制数据
    for(uint16_t i = 0; i < size; i++) {
        buffer[i] = pBuffer_read[i + 2];
    }

    return OTA_STORAGE_OK;
}

/**
 * @brief   写入OTA存储数据
 * 
 * @param   tag     存储标签
 * @param   buffer  数据缓冲区
 * @param   size    要写入的数据大小
 * 
 * @return  ota_storage_result_t 写入结果
 */
ota_storage_result_t OTA_Storage_Write(ota_storage_tag_t tag, const uint8_t *buffer, uint16_t size)
{
    uint8_t pBuffer[OTA_DATAFLASH_BLOCK_SIZE] = {0};
    uint16_t pBuffer_size = 0;

    if(tag >= OTA_TAG_MAX) {
        OTA_STORAGE_PRINT("OTA Storage Write: Invalid tag %d\r\n", tag);
        return OTA_STORAGE_INVALID_TAG;
    }
    
    // 检查储存区属性，只读区域不允许写入
    if(ota_storage_regions[tag].attribute == OTA_STORAGE_ATTRIBUTE_READ_ONLY) {
        OTA_STORAGE_PRINT("OTA Storage Write: Read-only region %d\r\n", tag);
        return OTA_STORAGE_READ_ONLY;
    }
    
    if(size > (ota_storage_regions[tag].size - 4)) {
        OTA_STORAGE_PRINT("OTA Storage Write: Invalid size %d for tag %d\r\n", size, tag);
        return OTA_STORAGE_INVALID_SIZE;
    }
    
    if (buffer == NULL) {
        OTA_STORAGE_PRINT("OTA Storage Write: NULL buffer\r\n");
        return OTA_STORAGE_ERROR;
    }
    //擦除
    ota_storage_result_t result = OTA_Storage_Erase(tag);
    if(result != OTA_STORAGE_OK) {
        OTA_STORAGE_PRINT("OTA Storage Write: Erase failed\r\n");
        return result;
    }

    //填充pBuffer长度
    pBuffer[pBuffer_size++] = size & 0xFF;
    pBuffer[pBuffer_size++] = (size >> 8) & 0xFF;

    //将buffer写入pBuffer
    for(uint16_t i = 0; i < size; i++) {
        pBuffer[pBuffer_size++] = buffer[i];
    }

    //计算CRC校验
    uint16_t crc = GetQuickCRC16(pBuffer, pBuffer_size);

    //将CRC校验写入pBuffer的最后2个字节 
    pBuffer[pBuffer_size++] = crc & 0xFF;
    pBuffer[pBuffer_size++] = (crc >> 8) & 0xFF;

    //写入
    EEPROM_WRITE(ota_storage_regions[tag].start_addr, pBuffer, pBuffer_size);

    //读出数据查看校验
    uint8_t pBuffer_read[OTA_DATAFLASH_BLOCK_SIZE] = {0};
    uint16_t pBuffer_read_size = 0;
    EEPROM_READ(ota_storage_regions[tag].start_addr, (uint32_t *)&pBuffer_read[0], OTA_DATAFLASH_BLOCK_SIZE);

    for(uint16_t i = 0; i < pBuffer_size; i++) {
        {
            if(pBuffer_read[i] != pBuffer[i]) {
                OTA_STORAGE_PRINT("OTA Storage Write: CRC error\r\n");
                return OTA_STORAGE_ERROR;
            }
        }
    }

    return OTA_STORAGE_OK;
}




//写入Image跳转标识
void OTA_Storage_Write_Jump_Flag(ota_jump_flag_t jump_flag)
{
    uint8_t pBuffer[OTA_DATAFLASH_BLOCK_SIZE] = {0};
    uint16_t pBuffer_index = 0;

    //Image跳转标识
    pBuffer[pBuffer_index++] = jump_flag;
    pBuffer[pBuffer_index++] = ~jump_flag;

    OTA_Storage_Write(OTA_TAG_JUMP_FLAG, pBuffer, pBuffer_index);
}

//读取Image跳转标识
ota_jump_flag_t OTA_Storage_Read_Jump_Flag(void)
{
    uint8_t pBuffer[OTA_DATAFLASH_BLOCK_SIZE] = {0};
    ota_jump_flag_t jump_flag = OTA_JUMP_FLAG_INVALID;

    // 读取前2字节
    if (OTA_Storage_Read(OTA_TAG_JUMP_FLAG, pBuffer, 2) == OTA_STORAGE_OK)
    {
        uint8_t flag_B = pBuffer[1];
        flag_B = ~flag_B;

        // 校验第二字节为第一字节的按位取反
        if (pBuffer[0] == flag_B)
        {
            jump_flag = (ota_jump_flag_t)pBuffer[0];
        }
        else
        {
            // 校验失败，返回无效
            jump_flag = OTA_JUMP_FLAG_INVALID;
        }
    }
    else
    {
        // 读取失败，返回0或可自定义错误码
        jump_flag = OTA_JUMP_FLAG_INVALID;
    }

    return jump_flag;
}