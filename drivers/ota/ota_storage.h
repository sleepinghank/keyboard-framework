/********************************** (C) COPYRIGHT *******************************
 * 文件名称          : ota_storage.h
 * 作者             : 
 * 版本             : V1.0
 * 日期             : 2024/04/07
 * 描述             : OTA存储操作相关函数声明
 *********************************************************************************
 * 版权所有 (c) 2024 
 *******************************************************************************/

 #ifndef OTA_STORAGE_H
 #define OTA_STORAGE_H
 
 #include "CONFIG.h"
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 /*
 1、CH592D Flash分区
 
 | 地址区间                | 用途                        | 描述    |
 |:----------------------:|:--------------------------:|:-------:|
 | 0x00000000-0x0006FFFF  | 用户应用程序存储区 CodeFlash | 448KB   |
 | 0x00070000-0x00077FFF  | 用户非易失数据存储区 DataFlash | 32KB    |
 | 0x00078000-0x0007DFFF  | 系统引导程序存储区 BootLoader | 24KB    |
 | 0x0007E000-0x0007FFFF  | 系统非易失配置信息区 InfoFlash | 8KB     |
 
 双备份分区表:
 | 分区名称 | 起始地址     | 大小     | 结束地址     | 说明                                 |
 |:-------:|:------------:|:--------:|:------------:|:------------------------------------:|
 | Jump    | 0x00000000   | 4KB      | 0x00000FFF   | 引导程序区,用于跳转到IAP或APP         |
 | IAP     | 0x00001000   | 32KB     | 0x00008FFF   | IAP升级程序区                        |
 | IMAGE A | 0x00009000   | 108KB    | 0x00023FFF   | 应用程序区                           |
 | IMAGE B | 0x00024000   | 108KB    | 0x0003EFFF   | 待升级应用存储区                     |
 | USER DATA| 0x0003F000   | 4KB    | 0x0003FFFF   | 用户数据区                     |
 | BLE LIB | 0x00040000   | 192KB    | 0x0006FFFF   | 静态库                               |
 
 | 系统配置区 | 0x00070000 | 28KB | 0x00076FFF | 存储系统配置、键盘映射等数据 |
| 蓝牙连接信息 | 0x00077000 | 4KB | 0x00077FFF | 存储蓝牙连接信息 |
 

 
 单备份分区表:
 | 分区名称 | 起始地址     | 大小     | 结束地址     | 说明                                 |
 |:-------:|:------------:|:--------:|:------------:|:------------------------------------:|
 | Jump    | 0x00000000   | 4KB      | 0x00000FFF   | 引导程序区,用于跳转到IAP或APP         |
 | IAP     | 0x00001000   | 32KB     | 0x00008FFF   | IAP升级程序区                        |
 | IMAGE   | 0x00009000   | 220KB    | 0x0003FFFF   | 应用程序区                           |
 | BLE LIB | 0x00040000   | 192KB    | 0x0006FFFF   | 静态库                               |
 
 | 系统配置区 | 0x00070000 | 32KB | 0x00077FFF | 存储系统配置、键盘映射等数据 |
 */
 
 //---------------------------------------------------------FLASH定义---------------------------------------------------------//
 /* BootLoader定义 */
 #define BOOTLOADER_START_ADD    0x00001000 //BootLoader升级起始地址
 #define BOOTLOADER_SIZE         32 * 1024 //BootLoader升级大小
 
 /* IMAGE_A定义 */
 #define IMAGE_A_START_ADD    (BOOTLOADER_START_ADD + BOOTLOADER_SIZE) //APP升级起始地址
 #define IMAGE_A_SIZE         108 * 1024 //APP升级大小
 
 /* IMAGE_B定义 */
 #define IMAGE_B_START_ADD    (IMAGE_A_START_ADD + IMAGE_A_SIZE) //APP升级起始地址
 #define IMAGE_B_SIZE         108 * 1024 //APP升级大小
 
 
 #define jumpApp              ((void (*)(void))((uint32_t *)IMAGE_A_START_ADD)) //跳转APP
 #define jumpIAP              ((void (*)(void))((uint32_t *)BOOTLOADER_START_ADD)) //跳转IAP
 
 /* DataFlash起始地址 */
#define OTA_DATAFLASH_START_ADDR    0x0 //在这里使用相对地址，对应的绝对地址是0x00070000
/* DataFlash每一块的大小（每次擦除的大小） */
#define OTA_DATAFLASH_BLOCK_SIZE    EEPROM_PAGE_SIZE

 /*********************************************************************
  * 宏定义
  */
 
 /* OTA存储标签定义 */
 typedef enum {
     OTA_TAG_JUMP_FLAG = 0,     // Image跳转标识
     OTA_TAG_DEVICE_INFO,       // 设备信息
     OTA_TAG_FIRMWARE_INFO,     // 固件信息
     APP_TAG_USER_CONFIG_INFO,  //用户配置信息 
     OTA_TAG_MAX                // 标签数量
 } ota_storage_tag_t;
 
 
 
 /* OTA存储操作结果 */
 typedef enum {
     OTA_STORAGE_OK = 0,        // 操作成功
     OTA_STORAGE_ERROR,         // 操作失败
     OTA_STORAGE_INVALID_TAG,   // 无效标签
     OTA_STORAGE_INVALID_SIZE,  // 无效大小
     OTA_STORAGE_VERIFY_FAIL,   // 验证失败
     OTA_STORAGE_READ_ONLY      // 只读区域
 } ota_storage_result_t;
 
 /*********************************************************************
  * 函数声明
  */
 ota_storage_result_t OTA_Storage_Erase(ota_storage_tag_t tag);
 ota_storage_result_t OTA_Storage_Write(ota_storage_tag_t tag, const uint8_t *buffer, uint16_t size);
 ota_storage_result_t OTA_Storage_Read(ota_storage_tag_t tag, uint8_t *buffer, uint16_t size);
 
 
 
 
 //Image标识枚举
 typedef enum {
     //无效
     OTA_JUMP_FLAG_INVALID = 0x00,
     //手动唤起OTA
     OTA_JUMP_FLAG_MANUAL = 0x01,
     //OTA跳转到APP，待确认是否成功
     OTA_JUMP_FLAG_AUTO = 0x02,
     //跳转到APP成功
     OTA_JUMP_FLAG_SUCCESS = 0x03,
 } ota_jump_flag_t;
 
 
 //写入Image跳转标识
 void OTA_Storage_Write_Jump_Flag(ota_jump_flag_t jump_flag);
 //读取Image跳转标识
 ota_jump_flag_t OTA_Storage_Read_Jump_Flag(void);
 
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /* OTA_STORAGE_H */ 