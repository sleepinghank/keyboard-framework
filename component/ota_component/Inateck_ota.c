#include "kb904/config_product.h"
#include "Inateck_ProductCatalog.h"
#include "Inateck_ota.h"
#include "debug_log.h"
#include <stdio.h>
#include <string.h>
#include "crc16.h"
#include "AES128.h"
#include "Inateck_tag.h"

#if (ChipModelCode == _CHIP_PAR2860)
#include "ble.h"
#include "ota_imp.h"
#include "pxi_par2860_ble_lib.h"
#include "HID_OVER_GATT_KEYBOARD_OTA_Profile_PTP.h"
#elif (ChipModelCode == _CHIP_CH584M)
#include "ota_storage.h"
#elif (ChipModelCode == _CHIP_CH584M)
#include "ota_storage.h"
#endif



#define OTA_TIMEOUT 30 // OTA超时时间（单位：秒）
// extern void otaNotify(uint8_t *pValue, uint16_t len);
// extern uint16_t get_current_mtu();
// extern uint8_t update_mtu(uint16_t mtu);
// extern void ble_disconnect();
#if(EncryptFlag != 0x00)
//固件秘钥
uint8_t flash_passkey[16] = {0x49,0x4E,0x43,0x4B,0X5F,//固定秘钥（不能改）
	UINT32_BYTE3(ProductModelCode), UINT32_BYTE2(ProductModelCode), UINT32_BYTE1(ProductModelCode), (UINT32_BYTE0(ProductModelCode) & 0xF0),//产品型号代号
	UINT32_BYTE3(ChipModelCode),    UINT32_BYTE2(ChipModelCode),    UINT32_BYTE1(ChipModelCode),    UINT32_BYTE0(ChipModelCode),//芯片型号代号
	UINT32_BYTE2(HardwareVersion),  UINT32_BYTE1(HardwareVersion),  UINT32_BYTE0(HardwareVersion)//硬件版本号
};
#endif

static uint32_t _firmware_length_ = 0;   // 固件大小
static uint32_t _total_quantity_ = 0;    // 总分包数
static uint32_t _received_quantity_ = 0; // 已接收包数

static uint8_t OTAStart_Flag = 0; // OTA开始标志

static uint32_t _code_write_offset = 0;           // 写入的起始地址
static volatile uint32_t _fw_fast_write_size = 0; // 已擦除扇区中剩余可写入的字节数
static volatile uint32_t _fw_fast_Sector = 0;     // 擦除扇区序号
static uint16_t timeout_count = 0;                // 超时计数
static uint8_t BleDisconnect_flag = 0;            // BLE断开标志
static uint8_t DeviceReset_flag = 0;              // 设备复位标志
static uint8_t DeviceAddress[6];                  //设备地址
Firmware_Info_t Firmware_Info;

// 设置MTU
static void _OTA_SetMTU(uint16_t mtu);
// 读取MTU
uint16_t _OTA_ReadMTU(void);
// 读取支持的最大MTU长度
uint16_t _OTA_ReadMaxMTU(void);
// 读取MAC地址
static void _OTA_Readbase_mac(void);
// 擦除扇区
static uint8_t _OTA_EraseSector(uint32_t offset);
// 写入flash
static uint8_t _OTA_WriteFlash(uint32_t offset, uint8_t *p_buf, uint32_t len);
// 校验固件
uint8_t _OTA_Verify_Firmware(void);
// 命令回复
void _OTA_Command_Reply(uint8_t command_code, uint8_t error_code, uint8_t *ret_parm, uint8_t ret_len);
// 从flash读出固件信息
void _OTA_Read_FirmwareInfo(void);
// 写入固件信息到flash
uint8_t _OTA_Write_FirmwareInfo(void);
// 更新蓝牙连接参数
static void _OTA_UpdateConnection(void);
// 设备加密认证
static uint8_t _OTA_AUTH(uint8_t *auth_buf, uint8_t auth_len,uint8_t number);
// 复位所有OTA全局变量
static void _OTA_Reset_All_Variables(void);

#if (ChipModelCode == _CHIP_CH584M)

// extern void OTA_IAP_SendData(uint8_t *p_send_data, uint8_t send_len);
// extern void OTA_FlashErase_Start(void);
// // 开始OTA任务
// extern void OTA_StartTask(void);

// extern void OTA_Write_JumpFlag(void);
// extern void OTA_ResetExecute(void);
// // extern void update_conn_pamm(void);
// extern uint8_t OTA_GetBleName(uint8_t* buf);
/* Flash擦除过程相关变量 */
uint32_t EraseAdd = 0;      // 擦除起始地址
uint32_t EraseBlockNum = 0; // 需要擦除的块数
uint32_t EraseBlockCnt = 0; // 已擦除的块计数


#endif

#if (ChipModelCode == _CHIP_PAR2860)
static uint16_t _ota_checksum = 0; // 固件校验和

static void _checksum(uint16_t *checksum, uint16_t sz, uint8_t *data)
{
    uint32_t idx;

    for (idx = 0; idx < sz; idx++)
        *checksum += (uint16_t)data[idx];
}
#endif



void _OTA_Command_Processor(uint8_t *p_cmd, uint16_t sz)
{
    ota_cmd_manage_t ota_rec_data;
    uint8_t rbuff[20];
    uint8_t rbuff_len = 0;
    uint16_t crc_16 = 0, _buffer_crc16 = 0;
    uint8_t _ota_evt_buffer_len = 0; // 缓存的固件长度
    uint16_t Current_mtu = 0,Max_mtu = 0,_mtu = 0;

    memcpy((uint8_t *)&ota_rec_data, p_cmd, sz);

    if((ota_rec_data.simple_cmd.cmd >= 0x10 && ota_rec_data.simple_cmd.cmd <= 0x2F) || (ota_rec_data.simple_cmd.cmd == 0x80))
    {
        switch (ota_rec_data.simple_cmd.cmd)
        {
        case OTA_CMD_TRANSMIT_DATA: // 开始传输数据
            DEBUG_log("开始传输数据");

            _ota_evt_buffer_len = ota_rec_data.transmit_data.len;                  // 数据包长度
             memcpy(&_buffer_crc16,&ota_rec_data.transmit_data.crc[0],2);

            crc_16 = GetQuickCRC16(ota_rec_data.transmit_data.data, _ota_evt_buffer_len); //计算校验码
            if (crc_16 == _buffer_crc16)
            {
                DEBUG_log("校验码正确 len:%d",_ota_evt_buffer_len);
                // 判断是否需要擦除扇区
                if (_fw_fast_write_size <= 0)
                {
                    _OTA_EraseSector(_fw_fast_Sector);
                    _fw_fast_write_size = 0x1000; // 已擦除扇区中剩余可写入的字节数
                    _fw_fast_Sector += 0x1000;
                }
                // 写入flash
                _OTA_WriteFlash(_code_write_offset, ota_rec_data.transmit_data.data, _ota_evt_buffer_len);

#if (ChipModelCode == _CHIP_PAR2860)
                // 计算校验和
                _checksum(&_ota_checksum, _ota_evt_buffer_len, ota_rec_data.transmit_data.data);
#endif
                _code_write_offset += _ota_evt_buffer_len;  // 写入的起始地址
                _fw_fast_write_size -= _ota_evt_buffer_len; // 已擦除扇区中剩余可写入的字节数

                _received_quantity_ += 1; // 已接收包数

                rbuff_len = 0;
                rbuff[rbuff_len++] = (uint8_t)(crc_16 & 0x00ff);
                rbuff[rbuff_len++] = (uint8_t)((crc_16 >> 8) & 0x00ff);
                
                // 回复数据校验结果
                _OTA_Command_Reply(OTA_CMD_TRANSMIT_DATA + 1, ERR_NONE, rbuff, rbuff_len);
            }
            else
            {
                ERROR_log("crc_16 = 0x%x(%d)  _buffer_crc16 = 0x%x(%d)", crc_16, crc_16, _buffer_crc16, _buffer_crc16);
                rbuff_len = 0;
                rbuff[rbuff_len++] = (uint8_t)(crc_16 & 0x00ff);
                rbuff[rbuff_len++] = (uint8_t)((crc_16 >> 8) & 0x00ff);
                // 回复数据校验结果（CRC16校验错误）
                _OTA_Command_Reply(OTA_CMD_TRANSMIT_DATA + 1, ERR_CRC16, rbuff, rbuff_len);
            }

            _ota_evt_buffer_len = 0; // 缓存的固件长度
            timeout_count = 0;       // OTA超时清0
            break;

        case OTA_CMD_START: // 开始OTA
            DEBUG_log("开始OTA");
            memcpy(&_firmware_length_, &ota_rec_data.start_ota.firmware_len[0], 4);   // 固件长度 4
            memcpy(&_total_quantity_, &ota_rec_data.start_ota.total_quantity[0], 4);  // 总分包数
            DEBUG_log("_firmware_length_ = 0x%x(%d)", _firmware_length_, _firmware_length_);
            DEBUG_log("_total_quantity_ = 0x%x(%d)", _total_quantity_, _total_quantity_);
            if ((_firmware_length_ > 0) && (_firmware_length_ <= (512 * 1024)))
            {
                OTAStart_Flag = 1;       // OTA开始标志
         
                _OTA_UpdateConnection();// 更新蓝牙连接参数
#if (ChipModelCode == _CHIP_PAR2860)
                _code_write_offset = 0;  // 写入的起始地址
                _fw_fast_write_size = 0; // 已擦除扇区中剩余可写入的字节数
                _fw_fast_Sector = 0;     // 擦除扇区序号

                _mtu = _OTA_ReadMTU();//读取当前的MTU
                DEBUG_log("_mtu = 0x%x(%d)", _mtu, _mtu);
                rbuff_len = 0;
                // MTU
                rbuff[rbuff_len++] = _mtu;
                _OTA_Command_Reply(OTA_CMD_START + 1, ERR_NONE, rbuff, rbuff_len);
#elif (ChipModelCode == _CHIP_CH584M)
                _code_write_offset = IMAGE_B_START_ADD;// 写入的起始地址
                EraseAdd = IMAGE_B_START_ADD;      // 擦除起始地址
                EraseBlockNum = (_firmware_length_ / EEPROM_BLOCK_SIZE) + ((_firmware_length_ % EEPROM_BLOCK_SIZE) > 0 ? 1 : 0); // 需要擦除的块数
                EraseBlockCnt = 0; // 已擦除的块计数

                _fw_fast_write_size = EraseBlockNum * EEPROM_BLOCK_SIZE; // 已擦除扇区中剩余可写入的字节数
                _fw_fast_Sector = 0;     // 擦除扇区序号
                // OTA_StartTask();//开始OTA任务
                // OTA_FlashErase_Start();//启动擦除事件

#endif

                timeout_count = 0; // OTA超时清0
            }
            else
            {
                _OTA_Command_Reply(OTA_CMD_START + 1, ERR_FORMAT, rbuff, 0); // 数据格式错误
            }
            break;

        case OTA_CMD_END: // 结束OTA
            DEBUG_log("结束OTA");
            memcpy(&Firmware_Info.OTA_TimesTamp, &ota_rec_data.end_ota.rtc[0], 4);//OTA时间戳
            memcpy(&Firmware_Info.Firmware_CRC32, &ota_rec_data.end_ota.crc32[0], 4); //软件校验码(CRC32)
            DEBUG_log("_total_quantity_:%d,_received_quantity_:%d\r\n",_total_quantity_,_received_quantity_);
            // 判断已结束数据数量跟总数量是否一致
            if (_total_quantity_ == _received_quantity_)
            {
#if (ChipModelCode == _CHIP_PAR2860)                
                if (_OTA_Verify_Firmware() == 1)
                {
                    DEBUG_log("OTA_TimesTamp = 0x%x(%d)", Firmware_Info.OTA_TimesTamp, Firmware_Info.OTA_TimesTamp);
                    DEBUG_log("Firmware_CRC32 = 0x%x(%d)", Firmware_Info.Firmware_CRC32, Firmware_Info.Firmware_CRC32);
                    
                    pxi_bm_s_bond_info_delete_all();//BLE删除所有绑定设备信息
                    _OTA_Write_FirmwareInfo();
                    
                    _OTA_Command_Reply(OTA_CMD_END + 1, ERR_NONE, rbuff, 0);
                    DeviceReset_flag = 0x82; // 设备复位标志
    
                    DEBUG_log("OTA成功了！！！ 哈哈哈哈");
                    
                    // 复位所有OTA全局变量
                    _OTA_Reset_All_Variables();
                }
                else
                {
                    ERROR_log("ERR_COMMAND_UPDATE_FAIL\r\n");
                    _OTA_Command_Reply(OTA_CMD_END + 1, ERR_INTERNAL, rbuff, 0);

                    // 复位所有OTA全局变量
                    _OTA_Reset_All_Variables();
                }
#elif (ChipModelCode == _CHIP_CH584M)
                DEBUG_log("OTA_TimesTamp = 0x%x(%d)", Firmware_Info.OTA_TimesTamp, Firmware_Info.OTA_TimesTamp);
                DEBUG_log("Firmware_CRC32 = 0x%x(%d)", Firmware_Info.Firmware_CRC32, Firmware_Info.Firmware_CRC32);
                
                // _OTA_Write_FirmwareInfo();
                // DEBUG_log("回复OTA_CMD_END");
                // _OTA_Command_Reply(OTA_CMD_END + 1, ERR_NONE, rbuff, 0);
                
                // DEBUG_log("OTA成功了！！！ 哈哈哈哈\r\n");
                // OTA_Write_JumpFlag();
                // // 复位所有OTA全局变量
                // _OTA_Reset_All_Variables();

                
                DeviceReset_flag = 0x82; // 设备复位标志
                
#endif
            }
            else
            {
                ERROR_log("ERR_COMMAND_UPDATE_FAIL\r\n");
                _OTA_Command_Reply(OTA_CMD_END + 1, ERR_FORMAT, rbuff, 0);
                
                // 复位所有OTA全局变量
                _OTA_Reset_All_Variables();
            }

            timeout_count = 0; // OTA超时清0
            break;

        case OTA_CMD_RESTART: // 重启设备
            DEBUG_log("重启设备");
            _OTA_Command_Reply(OTA_CMD_RESTART + 1, ERR_NONE, rbuff, 0);
            DeviceReset_flag = 0x82; // 设备复位标志
            break;

        case OTA_CMD_SET_MTU: // 设置MTU长度
            Max_mtu = _OTA_ReadMaxMTU();//读取设备支持最大的MTU
            memcpy(&_mtu, &ota_rec_data.set_mtu.mtu[0], 2);  //MTU长度
            DEBUG_log("_mtu = 0x%x(%d)", _mtu, _mtu);
            if(_mtu <= Max_mtu)
            {
                _OTA_SetMTU(_mtu);//设置MTU长度
                Current_mtu = _OTA_ReadMTU();//读取当前的MTU
                DEBUG_log("Current_mtu = 0x%x(%d)", Current_mtu, Current_mtu);
                rbuff_len = 0;
                //当前的 MTU
                rbuff[rbuff_len++] = LO_UINT16(Current_mtu);
                rbuff[rbuff_len++] = HI_UINT16(Current_mtu);

                _OTA_Command_Reply(OTA_CMD_SET_MTU + 1, ERR_NONE, rbuff, rbuff_len);
                timeout_count = 0; // OTA超时清0
            }else
            {
                ERROR_log("ERR_SET_MTU_FAIL\r\n");
                _OTA_Command_Reply(OTA_CMD_SET_MTU + 1, ERR_NOT_SUPPORTED, rbuff, 0);
                timeout_count = 0; // OTA超时清0
            }

            break;

        case OTA_CMD_READ_MTU: // 读取MTU长度
            Current_mtu = _OTA_ReadMTU();//读取当前的MTU
            Max_mtu = _OTA_ReadMaxMTU();//读取设备支持最大的MTU
            DEBUG_log("Current_mtu = 0x%x(%d)", Current_mtu, Current_mtu);
            DEBUG_log("Max_mtu = 0x%x(%d)", Max_mtu, Max_mtu);
            rbuff_len = 0;
            //当前的 MTU
            rbuff[rbuff_len++] = LO_UINT16(Current_mtu);
            rbuff[rbuff_len++] = HI_UINT16(Current_mtu);

            //设备支持最大的MTU
            rbuff[rbuff_len++] = LO_UINT16(Max_mtu);
            rbuff[rbuff_len++] = HI_UINT16(Max_mtu);

            _OTA_Command_Reply(OTA_CMD_READ_MTU + 1, ERR_NONE, rbuff, rbuff_len);
            timeout_count = 0; // OTA超时清0
            break;

        case OTA_CMD_READ_PRODUCT_CHIP_MODEL: // 读取产品型号、芯片型号
            rbuff_len = 0;
            // 产品型号
            rbuff[rbuff_len++] = UINT32_BYTE0(ProductModelCode);
            rbuff[rbuff_len++] = UINT32_BYTE1(ProductModelCode);
            rbuff[rbuff_len++] = UINT32_BYTE2(ProductModelCode);
            rbuff[rbuff_len++] = UINT32_BYTE3(ProductModelCode);

            // 芯片型号
            rbuff[rbuff_len++] = UINT32_BYTE0(ChipModelCode);
            rbuff[rbuff_len++] = UINT32_BYTE1(ChipModelCode);
            rbuff[rbuff_len++] = UINT32_BYTE2(ChipModelCode);
            rbuff[rbuff_len++] = UINT32_BYTE3(ChipModelCode);

            _OTA_Command_Reply(OTA_CMD_READ_PRODUCT_CHIP_MODEL + 1, ERR_NONE, rbuff, rbuff_len);
            break;

        case OTA_CMD_READ_VERSION: // 读取软件、硬件版本号
            rbuff_len = 0;

            // 软件版本号
            rbuff[rbuff_len++] = UINT32_BYTE0(SoftwareVersion);
            rbuff[rbuff_len++] = UINT32_BYTE1(SoftwareVersion);
            rbuff[rbuff_len++] = UINT32_BYTE2(SoftwareVersion);

            // 硬件版本号
            rbuff[rbuff_len++] = UINT32_BYTE0(HardwareVersion);
            rbuff[rbuff_len++] = UINT32_BYTE1(HardwareVersion);
            rbuff[rbuff_len++] = UINT32_BYTE2(HardwareVersion);

            // 协议版本号
            rbuff[rbuff_len++] = UINT32_BYTE0(ProtocolVersion);
            rbuff[rbuff_len++] = UINT32_BYTE1(ProtocolVersion);
            rbuff[rbuff_len++] = UINT32_BYTE2(ProtocolVersion);

            _OTA_Command_Reply(OTA_CMD_READ_VERSION + 1, ERR_NONE, rbuff, rbuff_len);
            break;
        case OTA_CMD_READ_LAST_TIMESTAMP_CHECK: // 读取上次升级的时间戳、软件校验码
            rbuff_len = 0;
            memcpy(&rbuff[rbuff_len], &Firmware_Info.OTA_TimesTamp,4);
            rbuff_len += 4;

            memcpy(&rbuff[rbuff_len], &Firmware_Info.Firmware_CRC32,4);
            rbuff_len += 4;

            _OTA_Command_Reply(OTA_CMD_READ_LAST_TIMESTAMP_CHECK + 1, ERR_NONE, rbuff, rbuff_len);
            break;

        case OTA_CMD_READ_UUID: // 读取设备MAC地址、加密标志
            _OTA_Readbase_mac();// 读取MAC地址
            rbuff_len = 0;
            // MAC地址
            memcpy(&rbuff[rbuff_len], DeviceAddress, 6);
            rbuff_len += 6;

            rbuff[rbuff_len++] = EncryptFlag; // 加密标志

            _OTA_Command_Reply(OTA_CMD_READ_UUID + 1, ERR_NONE, rbuff, rbuff_len);
            break;
#if (EncryptFlag == 0x02)       // 设备加密标志
        case OTA_CMD_SET_AES_RANDOM_NUMBER: // 设置加密随机数

            _OTA_Command_Reply(OTA_CMD_SET_AES_RANDOM_NUMBER + 1, ERR_NONE, rbuff, rbuff_len);
            break;
#endif
        case OTA_CMD_AUTHENTICATION: // 设备身份认证，由主机决定采用的秘钥对
            {
                uint8_t index = ota_rec_data.authentication.key_index;
                #if (ChipModelCode == _CHIP_PAR2860)
                    uint8_t buf[] = ProductModel;
                    DEBUG_log("OTA_CMD_AUTHENTICATION key_index:%d\r\n",ota_rec_data.authentication.key_index);
                    _OTA_AUTH(buf,sizeof(buf),index);
                #elif (ChipModelCode == _CHIP_CH584M)
                    rbuff_len = 0;
                    // 产品型号
                    rbuff[rbuff_len++] = UINT32_BYTE0(ProductModelCode);
                    rbuff[rbuff_len++] = UINT32_BYTE1(ProductModelCode);
                    rbuff[rbuff_len++] = UINT32_BYTE2(ProductModelCode);
                    rbuff[rbuff_len++] = UINT32_BYTE3(ProductModelCode);

                    // uint8_t ble_name_len = 0;
                    // uint8_t buf[20] = {0};

                    // ble_name_len = OTA_GetBleName(buf);
                    
                    _OTA_AUTH(rbuff,rbuff_len,index);
                #endif
            }

            break;
        default:
            _OTA_Command_Reply(ota_rec_data.simple_cmd.cmd + 1, ERR_NOT_SUPPORTED, rbuff, 0);
            ERROR_log("ERR_NOT_SUPPORTED"); // 指令不支持
            break;
        }
    }
    // 添加TAG命令处理逻辑
    else if((ota_rec_data.simple_cmd.cmd >= 0x50) && (ota_rec_data.simple_cmd.cmd <= 0x5F))
    {
        Tag_CommandProcessor(p_cmd, sz);
    }
}

// 时钟  需要在主函数中添加，循环增
uint32_t rtc = 0;
// 随机数
uint32_t RandomNumber  = 0;

/// @brief 加密数据打包
/// @param command_code 命令码
/// @param error_code 错误码
/// @param number 秘钥ID
/// @param randomNumber 随机数
/// @param encrypt_buf 加密数据
/// @param encrypt_len 加密包长度
/// @param data_len 数据包长度
/// @return 表示打包是否成功
uint8_t Send_EncryptedData(uint8_t command_code, uint8_t error_code, uint8_t number, uint32_t randomNumber,
                           uint8_t *encrypt_buf, uint8_t encrypt_len,uint8_t data_len) {
    uint8_t buf[64], buf_len = 0, i = 0;
    uint16_t crc = 0;

    crc = GetQuickCRC16(encrypt_buf, encrypt_len);//计算CRC
    #if _OTA_DEBUG_EN
    DEBUG_log("crc:%d\n", crc);
    #endif
    buf_len = 0;

    buf[buf_len++] = command_code;//命令码
    buf[buf_len++] = error_code;//命令码

    buf[buf_len++] = data_len;//长度

    buf[buf_len++] = HI_UINT16(crc);//CRC16
    buf[buf_len++] = LO_UINT16(crc);

    buf[buf_len++] = number;//秘钥ID

    buf[buf_len++] = UINT32_BYTE3(randomNumber);//随机数
    buf[buf_len++] = UINT32_BYTE2(randomNumber);
    buf[buf_len++] = UINT32_BYTE1(randomNumber);
    buf[buf_len++] = UINT32_BYTE0(randomNumber);
    //复制加密数据
    for (i = 0; i < encrypt_len; i++) {
        buf[buf_len++] = encrypt_buf[i];
    }
    #if _OTA_DEBUG_EN
    DEBUG_log("randomNumber:%d\n", randomNumber);

    DEBUG_logs("\r\nSend_EncryptedData :{");
    for (i = 0; i < buf_len; i++) {
        DEBUG_log("0x%02x,", buf[i]);
    }
    DEBUG_logs("}\r\n");
    #endif
 #if (ChipModelCode == _CHIP_PAR2860)
    while ( ble_tx_data(INATECK_OTA_READ_WRITE_IDX,buf_len, buf) == 0)
        ;
#elif (ChipModelCode == _CHIP_CH584M)
        // otaNotify(buf,buf_len);
#endif

    return 1;
}


/// @brief 消息加密
/// @param output 输出的数据
/// @param output_len 输出的数据长度
/// @param input 输入的数据
/// @param input_len 输入的数据长度
/// @param number 秘钥ID
/// @param randomNumber 随机数
/// @return 加密后的长度，为0时表示加密失败
uint8_t _OTA_Message_Encrypt(uint8_t *output, uint8_t output_len, uint8_t *input, uint8_t input_len, uint8_t number,
                             uint32_t randomNumber) {
    uint8_t buf[64] = {0};
    uint8_t buf_len = input_len;
    uint32_t rtc_tmp = rtc;
    if (input_len == 0) {
        return 0;
    }

    buf_len = buf_len + 4;
    // len 不足16的倍数，补长度
    if (buf_len % 16 != 0) {
        buf_len = buf_len + (16 - buf_len % 16);
    }

    if (output_len < buf_len) {
        return 0;
    }

    memcpy(buf, &rtc_tmp, 4);
    memcpy(buf + 4, input, input_len);

    AES_CBC_encrypt(output, buf, buf_len, randomNumber, number);

    return buf_len;
}



/// @brief 设备加密认证
/// @param buf 写入的数据
/// @param len 写入的数据长度
/// @return 数据的长度
static uint8_t _OTA_AUTH(uint8_t *auth_buf, uint8_t auth_len,uint8_t number) {
    uint8_t buf[64] = {0};
    uint32_t randomNumber = RandomNumber; // 随机数
    uint8_t encrypt_len = 0;

    encrypt_len = _OTA_Message_Encrypt(buf, 64, auth_buf, auth_len, number, randomNumber);

    Send_EncryptedData(0x2C + 1, 0, number, randomNumber, buf, encrypt_len, auth_len);
    return 1;
}

uint16_t ble_mtu = 255;
// 设置MTU
static void _OTA_SetMTU(uint16_t mtu)
{
#if (ChipModelCode == _CHIP_PAR2860)
    pxi_att_c_mtu_req(mtu);
#elif (ChipModelCode == _CHIP_CH584M)
    DEBUG_log("_OTA_SetMTU: %d", mtu);
    // update_mtu(mtu);
    ble_mtu = mtu;
#endif
}

// 读取MTU
uint16_t _OTA_ReadMTU(void)
{
#if (ChipModelCode == _CHIP_PAR2860)
    uint16_t mtu = 0;
    mtu = pxi_att_c_get_mtu_size();
    //配置MTU四对齐
    mtu = (mtu - 3) & 0xFFFFFFFC;
    return mtu;
#elif (ChipModelCode == _CHIP_CH584M)
    uint16_t tmp_mtu = 0;
    // tmp_mtu = get_current_mtu();
    //配置MTU四对齐
    tmp_mtu = (tmp_mtu - 3) & 0xFFFFFFFC;
    DEBUG_log("_OTA_ReadMTU: %d", ble_mtu);
    return ble_mtu;
#endif

}
// 读取支持的最大MTU长度
uint16_t _OTA_ReadMaxMTU(void)
{
#if (ChipModelCode == _CHIP_PAR2860)
    return 512;
#elif (ChipModelCode == _CHIP_CH584M)
    return 255;
#endif
}
extern uint8_t base_mac[B_ADDR_LEN];
// 读取MAC地址
static void _OTA_Readbase_mac(void)
{

// #if (ChipModelCode == _CHIP_PAR2860)
//     static struct gap_ble_addr _dev_addr;
//     pxi_gap_s_ble_address_get(&_dev_addr);
//     memcpy(&DeviceAddress[0], &_dev_addr.addr[0],6);
//     // 多通道情况下，需要在不同通道下初始化秘钥
//     AES_Keyinitialization(DeviceAddress,ProductModelCode,ChipModelCode);//初始化秘钥
// #elif (ChipModelCode == _CHIP_CH584M)
//     DEBUG_log("base_mac: %02X:%02X:%02X:%02X:%02X:%02X", base_mac[0], base_mac[1], base_mac[2], base_mac[3], base_mac[4], base_mac[5]);

//     memcpy(&DeviceAddress[0], &base_mac[0],6);
//     AES_Keyinitialization(DeviceAddress,ProductModelCode,ChipModelCode);//初始化秘钥
// #endif



}




// 擦除扇区
static uint8_t _OTA_EraseSector(uint32_t offset)
{
#if (ChipModelCode == _CHIP_PAR2860)
    pxi_pmu_vote_not_sleep();
    return pxi_ota_code_erase_object(offset);
#elif (ChipModelCode == _CHIP_CH584M)
    uint8_t ret=0;
    DEBUG_log("FLASH_ROM_ERASE offset:%d,len:%d",offset,EEPROM_BLOCK_SIZE);
  //  ret = FLASH_ROM_ERASE(offset, EEPROM_BLOCK_SIZE);

    return ret;
#endif
}


// 擦除扇区处理函数
uint8_t OTA_EraseSector_Process(void)
{
    uint8_t ret=0;
    uint16_t _mtu = 0;
    uint8_t rbuff[2];
    uint8_t rbuff_len = 0;

    DEBUG_log("ERASE:%08x num:%d\r\n", (int)(EraseAdd + EraseBlockCnt * EEPROM_BLOCK_SIZE), (int)EraseBlockCnt);
    ret = FLASH_ROM_ERASE(EraseAdd + EraseBlockCnt * EEPROM_BLOCK_SIZE, EEPROM_BLOCK_SIZE);

    /* 擦除失败 */
    if(ret != SUCCESS)
    {
        ERROR_log("擦除失败\r\n");
        _mtu = _OTA_ReadMTU();//读取当前的MTU
        DEBUG_log("_mtu = 0x%x(%d)", _mtu, _mtu);
        rbuff_len = 0;
        // MTU
        rbuff[rbuff_len++] = _mtu;
        _OTA_Command_Reply(OTA_CMD_START + 1, ERR_INTERNAL, rbuff, rbuff_len);
        return FAILURE;
    }

    EraseBlockCnt++;

    /* 擦除结束 */
    if(EraseBlockCnt >= EraseBlockNum)
    {
        DEBUG_log("擦除结束\r\n");
        _mtu = _OTA_ReadMTU();//读取当前的MTU
        DEBUG_log("_mtu = 0x%x(%d)", _mtu, _mtu);
        rbuff_len = 0;
        // MTU
        rbuff[rbuff_len++] = _mtu;
        _OTA_Command_Reply(OTA_CMD_START + 1, ERR_NONE, rbuff, rbuff_len);
        return SUCCESS;
    }

    return 0xfe;
}

// 写入flash
static uint8_t _OTA_WriteFlash(uint32_t offset, uint8_t *p_buf, uint32_t len)
{
#if (ChipModelCode == _CHIP_PAR2860)
    uint8_t ret=0;
    if(len>0xffff)
    {
        ret += pxi_code_write(offset,0xffff,p_buf);
        ret += pxi_code_write(offset+0xffff,(uint16_t)(len-0xffff),p_buf+0xffff);
    }
    else
    {
        ret = pxi_code_write(offset,(uint16_t)len,p_buf);
    }
    return ret;

#elif (ChipModelCode == _CHIP_CH584M)
    uint8_t ret=0;
    __attribute__((aligned(8))) uint8_t flash_Data[1024];

    DEBUG_log("FLASH_ROM_WRITE offset:%d,len:%d",offset,len);

    // DEBUG_log("\r\n写入%X写入%d字节:{",offset,len);

    // for (uint16_t z = 0; z < len; z++)
    // {
    //     DEBUG_logs("%02X,",p_buf[z]);
    // }
    // DEBUG_logs("}\r\n");

    ret = FLASH_ROM_WRITE(offset, p_buf, (uint16_t)len);
    return ret;
#endif
}

// 命令回复
void _OTA_Command_Reply(uint8_t command_code, uint8_t error_code, uint8_t *ret_parm, uint8_t ret_len)
{
    uint8_t reply_buffer[255];     // 命令回复缓冲区
    uint8_t reply_buffer_len = 0; // 命令回复缓冲区长度

    PRINT("_OTA_Command_Reply.............");
    uint16_t mtu = _OTA_ReadMTU();

    PRINT("mtu : %d ret_len: %d",mtu, ret_len);
    if(ret_len > mtu - 2)
    {
        PRINT("ret_len > mtu - 2 (%d > %d)", ret_len, mtu - 2);
        _OTA_Command_Reply(command_code, ERR_NOT_SUPPORTED, NULL, 0);
        return;
    }

    reply_buffer_len = 0;
    reply_buffer[reply_buffer_len++] = command_code;
    reply_buffer[reply_buffer_len++] = error_code;

    if (ret_len != 0)
    {
        memcpy(&reply_buffer[reply_buffer_len], ret_parm, ret_len);
        reply_buffer_len += ret_len;
    }
#if (ChipModelCode == _CHIP_PAR2860)
    while ( ble_tx_data(INATECK_OTA_READ_WRITE_IDX,reply_buffer_len, reply_buffer) == 0)
        ;
#elif (ChipModelCode == _CHIP_CH584M)
    // otaNotify(reply_buffer,reply_buffer_len);
#endif  
}


// 更新蓝牙连接参数
static void _OTA_UpdateConnection(void)
{
#if (ChipModelCode == _CHIP_PAR2860)
    struct gap_update_params update;

    update.updateitv_min = OTA_FIRST_MIN_CONN_INTERVAL;
    update.updateitv_max = OTA_FIRST_MAX_CONN_INTERVAL;
    update.updatelatency = OTA_FIRST_SLAVE_LATENCY;
    update.updatesvto = OTA_FIRST_CONN_SUP_TIMEOUT;

    pxi_gap_s_connection_update(&update);
#elif (ChipModelCode == _CHIP_CH584M)
    // update_conn_pamm();
#endif

}


//命令发送
void _OTA_Command_Send(uint8_t command_code, uint8_t *ret_parm, uint8_t ret_len)
{
    uint8_t reply_buffer[255];     // 命令回复缓冲区
    uint8_t reply_buffer_len = 0; // 命令回复缓冲区长度

    uint16_t mtu = _OTA_ReadMTU();

    if(ret_len > mtu - 2)
    {
        PRINT("ret_len > mtu - 2 (%d > %d)", ret_len, mtu - 2);
        _OTA_Command_Reply(command_code, ERR_NOT_SUPPORTED, NULL, 0);
        return;
    }

    reply_buffer_len = 0;
    reply_buffer[reply_buffer_len++] = command_code;

    if (ret_len != 0)
    {
        memcpy(&reply_buffer[reply_buffer_len], ret_parm, ret_len);
        reply_buffer_len += ret_len;
    }
    // otaNotify(reply_buffer,reply_buffer_len);
}

// OTA任务（1s调用一次）
uint8_t _OTA_ScheduledTask(void)
{
    uint8_t rbuff[2];
    uint8_t flag = 0;

    if (OTAStart_Flag != 0)
    {
        timeout_count++;
        DEBUG_log("OTA timeout_count: %d\r\n", timeout_count);
        if (timeout_count >= OTA_TIMEOUT) // 已经开始OTA了
        {
            // 结束OTA
            _OTA_Command_Reply(OTA_CMD_END + 1, ERR_TIMEOUT, rbuff, 0);

            BleDisconnect_flag = 1; // BLE断开标志
            
            // 复位所有OTA全局变量
            _OTA_Reset_All_Variables();
            OTAStart_Flag = 0;
            timeout_count = 0;
            DEBUG_log("OTA 超时！！！！\r\n");
        }
        flag = 1;
    }

    if ((DeviceReset_flag & 0x80) != 0)
    {
        uint8_t reset_type = DeviceReset_flag & 0x7f;
        DEBUG_log("DeviceReset_flag: %d\r\n", DeviceReset_flag);
        if(reset_type > 0)
        {
            reset_type--;
            DeviceReset_flag = 0x80 | reset_type;
            DEBUG_log("准备复位...%x\r\n",DeviceReset_flag);
        }else{
            DeviceReset_flag = 0;
            DEBUG_log("开始复位...\r\n");
            #if (ChipModelCode == _CHIP_PAR2860)
                    pxi_system_reset(); // BLE复位系统
            #elif (ChipModelCode == _CHIP_CH584M)
                OTA_ResetExecute();
            #endif
        }
        flag = 1;
    }

    if (BleDisconnect_flag != 0)
    {
        BleDisconnect_flag = 0;
        DEBUG_log("Ble Disconnect\r\n");
#if (ChipModelCode == _CHIP_PAR2860)
        ble_disconnect(); // 用于断开Ble
#endif
        flag = 1;
    }

    return flag;
}

// 校验固件
uint8_t _OTA_Verify_Firmware(void)
{
#if (ChipModelCode == _CHIP_PAR2860)
    uint8_t desc[32],version[10], version_len = 0;

    version_len = 0;

    memcpy((uint8_t*)&desc, "This device supports Inateck OTA", 32);

    // 软件版本号
    version[version_len++] = (uint8_t)((SoftwareVersion >> 16) & 0x0000000f) + '0';
    version[version_len++] = '.';
    version[version_len++] = (uint8_t)((SoftwareVersion >> 8) & 0x0000000f) + '0';
    version[version_len++] = '.';
    version[version_len++] = (uint8_t)(SoftwareVersion & 0x0000000f) + '0';
    version[version_len++] = 0x8a;
    version[version_len++] = 0x4a;
    version[version_len++] = 0x4e;
    version[version_len++] = 0x8e;
    version[version_len++] = 0x8f;


    if (pxi_code_update(desc, version, _firmware_length_, _ota_checksum) == 1)
    {

        return 1; // 成功
    }
    else
    {
        return 0; // 失败
    }
#endif
}


// 写入固件信息到flash
uint8_t _OTA_Write_FirmwareInfo(void)
{
    uint8_t err = 0;
#if (ChipModelCode == _CHIP_PAR2860)
    
    err = pxi_gap_s_profile_data_write(FirmwareInfo_WriteAddress, FirmwareInfo_WriteSize, (uint8_t *)&Firmware_Info);
    DEBUG_log("_OTA_Write_FirmwareInfo   = %x\r\n", err);
#elif (ChipModelCode == _CHIP_CH584M)
    err = OTA_Storage_Write(OTA_TAG_FIRMWARE_INFO, (uint8_t *)&Firmware_Info, sizeof(Firmware_Info));
#endif  
   
    return err;
}

// 从flash读出固件信息
void _OTA_Read_FirmwareInfo(void)
{
#if (ChipModelCode == _CHIP_PAR2860)
    pxi_gap_s_profile_data_read(FirmwareInfo_WriteAddress, FirmwareInfo_WriteSize, (uint8_t *)&Firmware_Info);

    if (Firmware_Info.Firmware_CRC32 == 0xffffffff)
    {
        Firmware_Info.Firmware_CRC32 = 0;
    }
    if (Firmware_Info.OTA_TimesTamp == 0xffffffff)
    {
        Firmware_Info.OTA_TimesTamp = 946656000; // 2000-1-1 00:00:00
    }
#elif (ChipModelCode == _CHIP_CH584M)
    ota_storage_result_t result = OTA_Storage_Read(OTA_TAG_FIRMWARE_INFO, (uint8_t *)&Firmware_Info, sizeof(Firmware_Info));
    if (result == OTA_STORAGE_OK)
    {
        DEBUG_log("OTA_Storage_Read success\r\n");
    }
    else
    {
        Firmware_Info.Firmware_CRC32 = 0;
        Firmware_Info.OTA_TimesTamp = 946656000; // 2000-1-1 00:00:00
        ERROR_log("OTA_Storage_Read failed\r\n");
    }
    
#endif  
}

// 获取升级状态
uint8_t _OTA_GetStartFlag(void)
{
    return OTAStart_Flag;
}

void _OTA_Init(void)
{

    DEBUG_logs("\r\n------------------------------------");
    DEBUG_logs("\r _____            _            _    ");
    DEBUG_logs("\r|_   _|          | |          | |   ");
    DEBUG_logs("\r  | | _ __   __ _| |_ ___  ___| | __");
    DEBUG_logs("\r  | || '_ \\ / _` | __/ _ \\/ __| |/ /");
    DEBUG_logs("\r _| || | | | (_| | ||  __/ (__|   < ");
    DEBUG_logs("\r \\___/_| |_|\\__,_|\\__\\___|\\___|_|\\_\\");
    DEBUG_logs("\r------------------------------------\r\n");

    _OTA_Read_FirmwareInfo();
    _OTA_Readbase_mac();// 读取MAC地址
    _OTA_Reset_All_Variables();// 复位所有OTA全局变量
}



// 复位所有OTA全局变量
static void _OTA_Reset_All_Variables(void)
{
    _firmware_length_ = 0;   // 固件大小
    _total_quantity_ = 0;    // 总分包数
    _received_quantity_ = 0; // 已接收包数

    OTAStart_Flag = 0; // OTA开始标志

#if (ChipModelCode == _CHIP_PAR2860)
    _code_write_offset = 0;  // 写入的起始地址
    _ota_checksum = 0; // 固件校验和
#elif (ChipModelCode == _CHIP_CH584M)
    _code_write_offset = IMAGE_B_START_ADD;// 写入的起始地址
    EraseAdd = 0;      // 擦除起始地址
    EraseBlockNum = 0; // 需要擦除的块数
    EraseBlockCnt = 0; // 已擦除的块计数
#endif
    _fw_fast_write_size = 0; // 已擦除扇区中剩余可写入的字节数
    _fw_fast_Sector = 0;     // 擦除扇区序号
    timeout_count = 0;                // 超时计数
    BleDisconnect_flag = 0;            // BLE断开标志
    DeviceReset_flag = 0;              // 设备复位标志
    
    // DeviceAddress 在 _OTA_Readbase_mac 中初始化，这里不需要复位
    // Firmware_Info 保持原有值，不需要复位
}








