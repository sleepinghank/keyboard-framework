# OTA移植说明

[TOC]



## 一、文件说明

OTA组件一共有6个文件

| 文件名                   | 说明                                       |
| ------------------------ | ------------------------------------------ |
| crc16.c                  | ota过程中用于数据校验，移植时不需要修改    |
| crc16.h                  |                                            |
| Inateck_ota.c            | ota核心组件，移植时不需要修改              |
| Inateck_ota.h            |                                            |
| Inateck_ProductInfo.h    | 项目相关的信息，需要自行创建该文件         |
| Inateck_ProductCatalog.h | 产品型号、芯片型号定义表，移植时不需要修改 |

在移植时需要在工程的SDK目录下创建一个Inateck_ProductInfo.h文件，Inateck_ProductInfo.h在OTA_Component的doc目录下有一个参考文件可以直接复制到工程下进行修改。

## 二、组件调用

OTA_Component一共有四个函数需要调用

| 函数名称                                                  | 说明                  |
| --------------------------------------------------------- | --------------------- |
| void _OTA_Init(void);                                     | 组件初始化            |
| void _OTA_ScheduledTask(void);                            | OTA任务（1s调用一次） |
| void _OTA_Command_Processor(uint8_t* p_cmd, uint16_t sz); | OTA命令处理           |
| uint8_t _OTA_GetStartFlag(void);                          | 获取升级状态          |

### 1、_OTA_Init调用

需要在main函数中对_OTA_Init进行调用，建议与ota_imp_init放在一起。

```c
    ota_imp_init();
    _OTA_Init();
```

### 2、_OTA_ScheduledTask调用

_OTA_ScheduledTask用于处理OTA的超时、重启等事件，所以需要在main函数或者定时器中对_OTA_ScheduledTask进行周期性调用。调用间隔约为1s

```c

    while (1)
    {
        _sys_evt_pmu_task();

        _sys_evt_task();

        _ble_evt_task();

        ble_task();
        if(i < 1400) i++;
        else
        {
            i = 0;
            _OTA_ScheduledTask();
        }
    }
```

### 3、_OTA_Command_Processor调用

_OTA_Command_Processor为OTA数据处理接口，需要在_ble.c的ble_gatt_write里面对其进行调用

```c
if (evt.uuid == OTA_READ_WRITE_UUID)
    {
        if (ble_slave_check_latency_on() == 1)
            ble_slave_latency_off();
        ota_cmd(evt.data, evt.sz,0);
    }
    else if(evt.uuid == OTA_RETRANSMIT_UUID)
    {
        DBGPRINTF(("OTA_RETRANSMIT_UUID\r\n"));
        ota_cmd(evt.data, evt.sz, 1);
    }
    else if(evt.uuid == OTA_RESET_UUID)
    {
        DBGPRINTF(("OTA_RESET_UUID\r\n"));
        ota_cmd(evt.data, evt.sz, 1);
    }
    else if (evt.uuid == INATECK_OTA_READ_WRITE_UUID)//FF04
    {
        //DBGPRINTF(("INATECK_OTA_UUID\r\n"));
        if (ble_slave_check_latency_on() == 1)
            ble_slave_latency_off();
            _OTA_Command_Processor(evt.data, evt.sz);
    }

```

### 4、_OTA_GetStartFlag调用

为了防止系统在OTA过程中休眠、关机，所以需要在main.c中查找ota_check并在ota_check旁边添加_OTA_GetStartFlag

```c
static void _check_task_state(uint8_t keyboard_st,uint8_t touch_st)
{

    if ((ble_is_connected()) && ble_check_notify_enabled() && (keyboard_st == 0) && (touch_st == 0)
            && (_pbtn_cnt == 0) && (ota_check() == 0) && (_OTA_GetStartFlag() == 0)/*&& pxi_get_latency() != 0 */&& is_matrix_empty() == 1 && is_bouncing_over() == 1 /*&& (pxi_hw_check_tx_fifo_empty() == 1)*/)
    {
```

```c
static uint8_t _check_idle()
{
	uint8_t state = 0;
	
	state = _get_evt_state();
	
    if (state == 0 && (ota_check() == 0) && (_OTA_GetStartFlag() == 0))
        return 1;
    else
        return 0;
}
```

```c
static void _sys_evt_pmu_task(void)
{
    if (_check_evt(SYS_ST, SYS_LOW_POWER))
    {
        _clr_evt(SYS_ST, SYS_LOW_POWER);
        
        _io_pin_wakeup = 0x0000;
        _touch_pin_wakeup = 0x0000;
        DBG(" SYS_LOW_POWER-------------------\n");
        if((ota_check() != 1) && (_OTA_GetStartFlag() != 1))
        _system_power_down();
    }
```

## 三、修改蓝牙报文

为了与原厂的OTA兼容，需要在原有的基础上增加一个0xFFE0的Handles。在BtGatt.exe的.xml文件里面增加下面这一段代码，再使用BtGatt.exe打开.xml重新生成.h文件即可。具体操作可以参考BtGatt.exe的说明文档

```xml
<Service XmlName="org.bluetooth.service.vender_define_service" Name="INATECK_OTA" Uuid="FFE0" Handle="00EB">
<Characteristic XmlName="org.bluetooth.characteristic.vender_define_characteristic" Name="INATECK_OTA_READ_WRITE" Uuid="FFE1" Properties="30" Handle="00EC" ValueHandle="00ED" IsEncrypt="False">
<Field Name="Vender Define Value" Value="0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00" IsReverse="True" Format=""/>
<Descriptor XmlName="org.bluetooth.descriptor.gatt.client_characteristic_configuration" Name="CLIENT_CHARACTERISTIC_CONFIGURATION" Uuid="2902" Handle="00EE" IsEncrypt="False">
<Field Name="Properties" Value="0x00 0x00" IsReverse="True" Format="(16bit)"/>
</Descriptor>
</Characteristic>
</Service>
```

## 四、固件加密

在Inateck_ProductInfo.h中通过修改EncryptFlag的值可以修改项目固件是否加密，如果启用加密则需要在ble.c中声明flash_passkey，并且在ble_init使用宏定义决定要不要将flash_passkey传入pxi_gap_s_ble_init。

```c
#if(EncryptFlag != 0x00)
extern uint8_t flash_passkey[16];
#endif

void ble_init()
{
    struct gap_evt_callback evt;
    struct smp_pairing_req sec_para;
    struct gatt_profile_struct gatt;
	

	    gatt.report_handle_address = (uint32_t)_gatt_database_report_handle_ptp;
        gatt.primary_address = (uint32_t)_gatt_database_primary_ptp;
        gatt.include_address = (uint32_t)_gatt_database_include_ptp;
        gatt.characteristic_address = (uint32_t)_gatt_database_characteristic_ptp;
        gatt.value_address = (uint32_t)_gatt_database_value_ptp;
	
	
    //DBG(">GAPProfile\r\n");
    pxi_gap_s_gatt_profiles_set(&gatt);
    //DBG("<GAPProfile\r\n");

    //DBGPRINTF((">BleInit\r\n"));
#if(EncryptFlag != 0x00)
    pxi_gap_s_ble_init(flash_passkey);
#else
     pxi_gap_s_ble_init(0);
#endif
```

## 五、固件打包

在IDE的User/After Build/Rebuild的Run #2中添加如下代码，并且将固件打包工具.exe放到工程文件同级目录

```
"固件打包工具.exe" “E:\ProjectCode\kb01104-pixart” "par2860_K+T_keyboard.bin"
```

