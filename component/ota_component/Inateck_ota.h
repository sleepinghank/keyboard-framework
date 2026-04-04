#ifndef _INATECK_OTA_H_
#define _INATECK_OTA_H_

#include <stdint.h>


//---------------------------------------------------------指令表---------------------------------------------------------//

#define OTA_CMD_READ_PRODUCT_CHIP_MODEL          0x10  //读取产品型号、芯片型号
#define OTA_CMD_READ_VERSION                     0x12  //读取软件、硬件版本号
#define OTA_CMD_READ_LAST_TIMESTAMP_CHECK        0x14  //读取上次升级的时间戳、校验码
#define OTA_CMD_READ_UUID                        0x16  //读取设备MAC地址、加密标志

#define OTA_CMD_START                            0x20  //开始OTA
#define OTA_CMD_TRANSMIT_DATA                    0x22  //传输OTA数据
#define OTA_CMD_END                              0x24  //结束OTA
#define OTA_CMD_RESTART                          0x26  //重启设备
#define OTA_CMD_SET_MTU                          0x28  //设置MTU长度
#define OTA_CMD_READ_MTU                         0x2A  //读取MTU长度
#define OTA_CMD_AUTHENTICATION                   0x2C  //加密身份认证



#define OTA_CMD_SET_AES_RANDOM_NUMBER            0x80  //设置加密随机数
//------------------------------------------------------------END------------------------------------------------------------//


//---------------------------------------------------------错误码---------------------------------------------------------//
#define ERR_NONE		    0x00//成功
#define ERR_INTERNAL		0xC1//内部错误
#define ERR_TIMEOUT		    0xC2//超时
#define ERR_OFFLINE		    0xC3//离线
#define ERR_ALREADY_EXIST	0xC4//（对象）已经存在
#define ERR_NOT_EXIST		0xC5//（对象）不存在
#define ERR_FORMAT		    0xC6//数据格式错误
#define ERR_TOO_MANY		0xC7//数据太长
#define ERR_BUSY		    0xC8//（设备）繁忙，正在处理其他事情
#define ERR_NOT_SUPPORTED	0xC9//指令不支持
#define ERR_CRC16	        0xCA//CRC16校验错误
//------------------------------------------------------------END------------------------------------------------------------//

#define APP_MAX_TX_SIZE 512


typedef union
{
    struct
    {
        uint8_t cmd;             //命令码
        uint8_t tag_data[APP_MAX_TX_SIZE - 1];  //TAG数据
    } tag_data;   //TAG数据
    
    struct
    {
        uint8_t cmd;             //命令码
        uint8_t random_numbers[16];  //随机数
        uint8_t rtc[4];            //秒级时间戳
    } encrypt_randomNumbers;   //设置加密随机数
    struct
    {
        uint8_t cmd;             //命令码
        uint8_t mtu[2];             //MTU长度
    } set_mtu;   //设置MTU长度
    struct
    {
        uint8_t cmd;             //命令码
        uint8_t rtc[4];            //秒级时间戳
        uint8_t crc32[4];          //软件校验码(CRC32)
    } end_ota;   //结束OTA
    struct
    {
        uint8_t cmd;             //命令码 
        uint8_t len;             //包长
        uint8_t crc[2];            //校验码
        uint8_t data[APP_MAX_TX_SIZE - 3]; /* 数据内容*/
    } transmit_data;       //传输OTA数据
    struct
    {
        uint8_t cmd;               //命令码 
        uint8_t firmware_len[4];   //固件长度
        uint8_t total_quantity[4]; //总分包数
    } start_ota;       //开始OTA
    struct
    {
        uint8_t cmd;             //命令码
        uint8_t key_index;       //密钥索引
    } authentication;   //身份认证
    
    struct
    {
        uint8_t cmd;         //命令码
    } simple_cmd;   //简单命令
    struct
    {
        uint8_t buf[APP_MAX_TX_SIZE]; /* 数据内容*/
    } data;
} ota_cmd_manage_t;


//---------------------------------------------------------快捷操作---------------------------------------------------------//

#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)
#define HI_UINT8(a)  (((a) >> 4) & 0x0F)
#define LO_UINT8(a)  ((a) & 0x0F)

#define BUILD_UINT16(loByte, hiByte) ((uint16_t)(((loByte) & 0x00FF)|(((hiByte) & 0x00FF)<<8)))

#define BUILD_UINT32(Byte0, Byte1, Byte2, Byte3)   ((uint32_t)(((uint32_t)(Byte0) & 0x00FF) + (((uint32_t)(Byte1) & 0x00FF) << 8) \
                                                  + (((uint32_t)(Byte2) & 0x00FF) << 16) + (((uint32_t)(Byte3) & 0x00FF) << 24)))

#define UINT32_BYTE0(dwTemp) ((uint8_t)( dwTemp        & 0x000000ff )) /*!< uint32_t 数据拆分 byte0 */
#define UINT32_BYTE1(dwTemp) ((uint8_t)((dwTemp >> 8)  & 0x000000ff )) /*!< uint32_t 数据拆分 byte1 */
#define UINT32_BYTE2(dwTemp) ((uint8_t)((dwTemp >> 16) & 0x000000ff )) /*!< uint32_t 数据拆分 byte2 */
#define UINT32_BYTE3(dwTemp) ((uint8_t)((dwTemp >> 24) & 0x000000ff )) /*!< uint32_t 数据拆分 byte3 */
	
//------------------------------------------------------------END------------------------------------------------------------//
#if (ChipModelCode == _CHIP_PAR2860)

/* 外设首选连接参数 */
#define OTA_FIRST_MIN_CONN_INTERVAL                                 6              /* 最小连接间隔 (12*1.25ms = 15 ms). */
#define OTA_FIRST_MAX_CONN_INTERVAL                                 6              /* 最大连接间隔 (12*1.25ms = 15 ms). */
#define OTA_FIRST_SLAVE_LATENCY                                     6              /* 从属延迟*/
#define OTA_FIRST_CONN_SUP_TIMEOUT                                  200             /* 连接监控超时(300*10ms = 3000ms)*/


#endif

typedef struct
{
    uint32_t Firmware_CRC32;//固件校验码
    uint32_t OTA_TimesTamp;//OTA时间戳
} Firmware_Info_t;





extern void _OTA_Init(void);
//OTA任务（1s调用一次）
extern uint8_t _OTA_ScheduledTask(void);
//OTA命令处理
extern void _OTA_Command_Processor(uint8_t* p_cmd, uint16_t sz);
//获取升级状态
extern uint8_t _OTA_GetStartFlag(void);
//命令发送
extern void _OTA_Command_Send(uint8_t command_code, uint8_t *ret_parm, uint8_t ret_len);
// 擦除扇区处理函数
extern uint8_t OTA_EraseSector_Process(void);
#endif //_OTA_H_






