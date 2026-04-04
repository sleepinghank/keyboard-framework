/********************************** (C) COPYRIGHT *******************************
 * File Name          : hidkbd.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/12/10
 * Description        : 蓝牙键盘应用程序，初始化广播连接参数，然后广播，直至连接主机后，定时上传键值
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "CONFIG.h"
#include "devinfoservice.h"
#include "battservice.h"
#include "hidkbdservice.h"
#include "storage.h"
#include "hiddev.h"
#include "hidkbd.h"
#include "_bt_driver.h"
#include "kb904/config_product.h"
#include "ble_adv_policy.h"
#include "debug.h"
/*********************************************************************
 * MACROS
 */
// Company Identifier: iN
#define iN_COMPANY_ID                        0x696E  // "iN" in ASCII (0x69='i', 0x6E='n')

// HID keyboard input report length
#define HID_KEYBOARD_IN_RPT_LEN              8

// HID keyboard Hot input report length
#define HID_KEYBOARD_HOT_IN_RPT_LEN          2

// HID LED output report length
#define HID_LED_OUT_RPT_LEN                  1

/*********************************************************************
 * CONSTANTS
 */
// Param update delay
#define START_PARAM_UPDATE_EVT_DELAY         3200

// Param update delay
#define START_PHY_UPDATE_DELAY               1600

// HID idle timeout in msec; set to zero to disable timeout
#define DEFAULT_HID_IDLE_TIMEOUT             60000

// What is the advertising interval when device is discoverable (units of 625us, 80=50ms)
#define DEFAULT_ADVERTISING_INTERVAL         40




// Default passcode
#define DEFAULT_PASSCODE                     0

// Default GAP pairing mode
#define DEFAULT_PAIRING_MODE                 GAPBOND_PAIRING_MODE_WAIT_FOR_REQ

// Default MITM mode (TRUE to require passcode or OOB when pairing)
#define DEFAULT_MITM_MODE                    FALSE

// Default bonding mode, TRUE to bond
#define DEFAULT_BONDING_MODE                 TRUE

// Default GAP bonding I/O capabilities
#define DEFAULT_IO_CAPABILITIES              GAPBOND_IO_CAP_NO_INPUT_NO_OUTPUT

// Battery level is critical when it is less than this %
#define DEFAULT_BATT_CRITICAL_LEVEL          6

#define DEFAULT_BATT_CRITICAL_POWER_STATE    0xAF

#define HIDEMU_LEGACY_ADV_BUFFER_CAPACITY    31u
#define HIDEMU_ADV_BUFFER_CAPACITY           \
    ((BLE_EFFECTIVE_ADV_MODE == BLE_ADV_MODE_EXTENDED) ? B_MAX_ADV_EXT_LEN : HIDEMU_LEGACY_ADV_BUFFER_CAPACITY)

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// Task ID
uint8_t hidEmuTaskId = INVALID_TASK_ID;
uint8_t base_mac[B_ADDR_LEN] = {0};
uint8_t ble_init_done = FALSE;
#define BLE_ADDR_SLOT_COUNT                  (BLE_INDEX_MAX - 1u)

/*********************************************************************
 * EXTERNAL VARIABLES
 */
static uint8_t discoverability_mode = 1u;
/*********************************************************************
 * EXTERNAL FUNCTIONS
 */
extern void restartDevLowAdvertising(void);
extern void hidDevInitialAdvertising(void);

/*********************************************************************
 * LOCAL VARIABLES
 */

static uint8_t advertData[HIDEMU_ADV_BUFFER_CAPACITY];
static uint8_t scanRspData[HIDEMU_ADV_BUFFER_CAPACITY];
static uint16_t advertDataLen = 0u;
static uint16_t scanRspLen = 0u;
static uint8_t advEventType = GAP_ADTYPE_ADV_IND;
    

// Device name attribute value
// static CONST uint8_t attDeviceName[GAP_DEVICE_NAME_LEN] = ProductModel;

// HID Dev configuration
static hidDevCfg_t hidEmuCfg = {
    DEFAULT_HID_IDLE_TIMEOUT, // Idle timeout
    HID_FEATURE_FLAGS         // HID feature flags
};

uint16_t hidEmuConnHandle = GAP_CONNHANDLE_INIT;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void    hidEmu_ProcessTMOSMsg(tmos_event_hdr_t *pMsg);
// static void    hidEmuSendKbdReport(uint8_t keycode);
static uint8_t hidEmuRcvReport(uint8_t len, uint8_t *pData);
static uint8_t hidEmuRptCB(uint8_t id, uint8_t type, uint16_t uuid,
                           uint8_t oper, uint16_t *pLen, uint8_t *pData);
static void    hidEmuEvtCB(uint8_t evt);
static void    hidEmuStateCB(gapRole_States_t newState, gapRoleEvent_t *pEvent);
static uint8_t  hidEmu_get_discoverability_flags(void);
static uint16_t hidEmu_append_ad_structure(uint8_t *buffer, uint16_t offset, uint8_t ad_type,
                                           const uint8_t *payload, uint8_t payload_len);
static void     hidEmu_build_adv_payload(const uint8_t *ble_name, uint8_t ble_name_len);
static void     hidEmu_apply_adv_payload(const uint8_t *ble_name, uint8_t ble_name_len);

/**
 * @brief 返回当前可发现模式对应的 GAP Flags 字节。
 */
static uint8_t hidEmu_get_discoverability_flags(void)
{
    if(discoverability_mode == 1u)
    {
        return GAP_ADTYPE_FLAGS_LIMITED | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED;
    }

    return GAP_ADTYPE_FLAGS_GENERAL | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED;
}

/**
 * @brief 以 [len][type][payload...] 形式追加一个 AD 结构。
 *
 * @return 追加后的偏移；若空间不足则返回原 offset。
 */
static uint16_t hidEmu_append_ad_structure(uint8_t *buffer, uint16_t offset, uint8_t ad_type,
                                           const uint8_t *payload, uint8_t payload_len)
{
    uint16_t total_len = (uint16_t)payload_len + 2u;

    if((offset + total_len) > (uint16_t)HIDEMU_ADV_BUFFER_CAPACITY)
    {
        PRINT("GAP: ad overflow offset=%u payload=%u cap=%u\n",
              (unsigned)offset,
              (unsigned)payload_len,
              (unsigned)HIDEMU_ADV_BUFFER_CAPACITY);
        return offset;
    }

    buffer[offset++] = (uint8_t)(payload_len + 1u);
    buffer[offset++] = ad_type;

    if((payload != NULL) && (payload_len != 0u))
    {
        tmos_memcpy(&buffer[offset], payload, payload_len);
        offset = (uint16_t)(offset + payload_len);
    }

    return offset;
}

/**
 * @brief 按编译期固定的广播模式构建 advertData / scanRspData。
 *
 * Legacy 路径使用可连接广播 + 扫描响应；
 * Extended 路径使用可连接扩展广播，并将名称与附加字段全部放入 advertData。
 */
static void hidEmu_build_adv_payload(const uint8_t *ble_name, uint8_t ble_name_len)
{
    uint8_t flags_payload[1];
    uint8_t appearance_payload[2];
    uint8_t manufacturer_payload[2];
    uint8_t service_uuid_payload[4];
    uint8_t conn_interval_payload[4];
    uint8_t tx_power_payload[1];

    flags_payload[0] = hidEmu_get_discoverability_flags();
    appearance_payload[0] = LO_UINT16(GAP_APPEARE_HID_KEYBOARD);
    appearance_payload[1] = HI_UINT16(GAP_APPEARE_HID_KEYBOARD);
    manufacturer_payload[0] = LO_UINT16(iN_COMPANY_ID);
    manufacturer_payload[1] = HI_UINT16(iN_COMPANY_ID);
    service_uuid_payload[0] = LO_UINT16(HID_SERV_UUID);
    service_uuid_payload[1] = HI_UINT16(HID_SERV_UUID);
    service_uuid_payload[2] = LO_UINT16(BATT_SERV_UUID);
    service_uuid_payload[3] = HI_UINT16(BATT_SERV_UUID);
    conn_interval_payload[0] = LO_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL);
    conn_interval_payload[1] = HI_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL);
    conn_interval_payload[2] = LO_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL);
    conn_interval_payload[3] = HI_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL);
    tx_power_payload[0] = 0u;

    memset(advertData, 0, sizeof(advertData));
    memset(scanRspData, 0, sizeof(scanRspData));

    advertDataLen = 0u;
    scanRspLen = 0u;
    advEventType = GAP_ADTYPE_ADV_IND;

    advertDataLen = hidEmu_append_ad_structure(advertData, advertDataLen,
                                               GAP_ADTYPE_FLAGS, flags_payload, sizeof(flags_payload));
    advertDataLen = hidEmu_append_ad_structure(advertData, advertDataLen,
                                               GAP_ADTYPE_APPEARANCE, appearance_payload, sizeof(appearance_payload));
    advertDataLen = hidEmu_append_ad_structure(advertData, advertDataLen,
                                               GAP_ADTYPE_MANUFACTURER_SPECIFIC, manufacturer_payload, sizeof(manufacturer_payload));
    advertDataLen = hidEmu_append_ad_structure(advertData, advertDataLen,
                                               GAP_ADTYPE_16BIT_MORE, service_uuid_payload, sizeof(service_uuid_payload));

    if(BLE_EFFECTIVE_ADV_MODE == BLE_ADV_MODE_EXTENDED)
    {
        advEventType = GAP_ADTYPE_EXT_CONN_UNDIRECT;
        advertDataLen = hidEmu_append_ad_structure(advertData, advertDataLen,
                                                   GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
                                                   conn_interval_payload,
                                                   sizeof(conn_interval_payload));
        advertDataLen = hidEmu_append_ad_structure(advertData, advertDataLen,
                                                   GAP_ADTYPE_POWER_LEVEL, tx_power_payload, sizeof(tx_power_payload));
        advertDataLen = hidEmu_append_ad_structure(advertData, advertDataLen,
                                                   GAP_ADTYPE_LOCAL_NAME_COMPLETE, ble_name, ble_name_len);
    }
    else
    {
        scanRspLen = hidEmu_append_ad_structure(scanRspData, scanRspLen,
                                                GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
                                                conn_interval_payload,
                                                sizeof(conn_interval_payload));
        scanRspLen = hidEmu_append_ad_structure(scanRspData, scanRspLen,
                                                GAP_ADTYPE_POWER_LEVEL, tx_power_payload, sizeof(tx_power_payload));
        scanRspLen = hidEmu_append_ad_structure(scanRspData, scanRspLen,
                                                GAP_ADTYPE_LOCAL_NAME_COMPLETE, ble_name, ble_name_len);
    }
}

/**
 * @brief 下发当前编译期固定路径的广播参数与载荷。
 */
static void hidEmu_apply_adv_payload(const uint8_t *ble_name, uint8_t ble_name_len)
{
    hidEmu_build_adv_payload(ble_name, ble_name_len);
    GAPRole_SetParameter(GAPROLE_ADV_EVENT_TYPE, sizeof(advEventType), &advEventType);
    GAPRole_SetParameter(GAPROLE_ADVERT_DATA, advertDataLen, advertData);
    GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, scanRspLen, scanRspData);
    PRINT("GAP: adv_mode=%u adv_event_type=0x%02X adv_len=%u scan_len=%u name_len=%u\n",
          (unsigned)BLE_EFFECTIVE_ADV_MODE,
          (unsigned)advEventType,
          (unsigned)advertDataLen,
          (unsigned)scanRspLen,
          (unsigned)ble_name_len);
}

/**
 * @brief 将广播可发现模式设为 Limited 或 General，等待统一 helper 下发到协议栈。
 *
 * @param mode 发现类别选择：1 = Limited Discoverable；其它值 = General Discoverable。
 */
void hidEmu_apply_advert_discoverability_flags(uint8_t mode)
{
    discoverability_mode = mode;
}

/**
 * @brief 基于 base_mac / BLE 通道 / 通道版本号更新可追溯的 Static Address。
 * @note  回连和配对都会调用本方法；重配场景下可选择先推进通道版本号。 1 
 * 
 * @details 生成方法：以 base_mac 为种子，针对每个 BLE 索引计算一个偏移量（slot + version * slot_count），
 * 将该偏移量加到 base_mac 的某个 word 上（本例中为第 3、4 字节组成的 word），并将结果写回地址的同一位置，
 * 
 * @param ble_idx BLE 索引，取值范围 1-3（对应 BLE_INDEX_1 - BLE_INDEX_3）
 * @param advance_version 是否在生成地址前将版本号推进（通常配对时推进，回连时不变以保持地址稳定）true = 版本号加1；false = 版本号不变
 */
static void hidEmu_regenerate_random_static_address(access_ble_idx_t ble_idx, uint8_t advance_version)
{
    storage_config_t *cfg = storage_get_config_ptr();
    uint8_t           newAddr[B_ADDR_LEN];
    uint8_t           slot;
    uint8_t           ver;
    uint8_t           is_zero = TRUE;
    uint8_t           is_ff = TRUE;
    uint8_t           i;
    uint16_t          offset;
    uint16_t          base_word;
    uint16_t          derived_word;

    if(cfg == NULL)
    {
        PRINT("[BT_ADDR] update skip: cfg=NULL\n");
        return;
    }

    for(i = 0u; i < B_ADDR_LEN; i++)
    {
        if(base_mac[i] != 0u)
        {
            is_zero = FALSE;
        }
        if(base_mac[i] != 0xFFu)
        {
            is_ff = FALSE;
        }
    }

    if(is_zero || is_ff)
    {
        PRINT("[BT_ADDR] update skip: invalid base=%02X:%02X:%02X:%02X:%02X:%02X\n",
               base_mac[5], base_mac[4], base_mac[3], base_mac[2], base_mac[1], base_mac[0]);
        return;
    }

    slot = (uint8_t)(ble_idx - BLE_INDEX_1);
    if(slot >= BLE_ADDR_SLOT_COUNT)
    {
        PRINT("[BT_ADDR] update skip: invalid slot=%x idx=%x\n", slot, ble_idx);
        return;
    }
    ver = cfg->ble_addr_ver[slot];
    if(advance_version)
    {
        ver++;
    }

    offset = (uint16_t)(slot + (ver * BLE_ADDR_SLOT_COUNT));

    memcpy(newAddr, base_mac, sizeof(newAddr));
    base_word = (uint16_t)(((uint16_t)base_mac[4] << 8) | base_mac[3]);
    derived_word = (uint16_t)(base_word + offset);
    newAddr[3] = LO_UINT16(derived_word);
    newAddr[4] = HI_UINT16(derived_word);
    newAddr[0] = (uint8_t)((newAddr[0] & 0x3Fu) | 0xC0u);

    GAP_ConfigDeviceAddr(ADDRTYPE_STATIC, newAddr);
    PRINT("BLE: traceable static addr idx=%u ver=%u adv=%u => %02X:%02X:%02X:%02X:%02X:%02X\n",
          (unsigned)ble_idx,
          (unsigned)ver,
          (unsigned)advance_version,
          newAddr[5], newAddr[4], newAddr[3], newAddr[2], newAddr[1], newAddr[0]);
}


/*********************************************************************
 * @fn      hidEmu_is_ble_bonded
 *
 * @brief   判断指定 BLE 索引的绑定标志位是否有效
 *
 * @param   ble_idx - BLE 索引，取值范围 1-3（对应 BLE_INDEX_1 - BLE_INDEX_3）
 *
 * @return  1 = 已绑定；0 = 未绑定或参数无效
 */
uint8_t hidEmu_is_ble_bonded(uint8_t ble_idx)
{
    storage_config_t *cfg = storage_get_config_ptr();
    uint8_t           bond_flag = 0;

    if (cfg == NULL) {
        return 0u;
    }
    switch (ble_idx) {
    case BLE_INDEX_1:
        bond_flag = BLE_BOND_FLAG_1;
        break;
    case BLE_INDEX_2:
        bond_flag = BLE_BOND_FLAG_2;
        break;
    case BLE_INDEX_3:
        bond_flag = BLE_BOND_FLAG_3;
        break;
    default:
        return 0u;
    }
    return (cfg->ble_bond_flag & bond_flag) != 0u ? 1u : 0u;
}

/*********************************************************************
 * @fn      hidEmu_save_ble_bonded
 *
 * @brief   存储相关绑定标志信息到flash，自动调用此函数，调用前已经将绑定信息存到flash中
 *
 * @return  none.
 */
void hidEmu_save_ble_bonded(access_ble_idx_t ble_idx,uint8_t is_pairing)
{
    storage_config_t *cfg = storage_get_config_ptr();
    if(cfg == NULL)
    {
        PRINT("[BOND] save failed: cfg=NULL\n");
        return;
    }

    switch(ble_idx)
    {
        case BLE_INDEX_1:
            cfg->ble_bond_flag |= BLE_BOND_FLAG_1;
            if (is_pairing) {
                // 配对流程，推进版本号以生成新地址
                cfg->ble_addr_ver[0]++;
                // cfg->ble_mac_flag ^= BLE_BOND_FLAG_1;
            }
            break;
        case BLE_INDEX_2:
            cfg->ble_bond_flag |= BLE_BOND_FLAG_2;
            if (is_pairing) {
                // 配对流程，推进版本号以生成新地址
                cfg->ble_addr_ver[1]++;
                // cfg->ble_mac_flag ^= BLE_BOND_FLAG_2;
            }
            break;
        case BLE_INDEX_3:
            cfg->ble_bond_flag |= BLE_BOND_FLAG_3;
            if (is_pairing) {
                // 配对流程，推进版本号以生成新地址
                cfg->ble_addr_ver[2]++;
                // cfg->ble_mac_flag ^= BLE_BOND_FLAG_3;
            }
            break;
        default:
            PRINT("[BOND] save failed: ble_idx=%x invalid\n", ble_idx);
            return;
    }
    PRINT("[BOND] save: idx=%x bond_flag=%02X  ver=[%u,%u,%u]\n",
           (unsigned)ble_idx, cfg->ble_bond_flag, 
           (unsigned)cfg->ble_addr_ver[0], (unsigned)cfg->ble_addr_ver[1], (unsigned)cfg->ble_addr_ver[2]);
    storage_save();
}

/*********************************************************************
 * @fn      hidEmu_is_ble_mac_change
 *
 * @brief   判断mac地址是否需要+1
 *
 * @return  none.
 */
uint8_t hidEmu_is_ble_mac_change( access_ble_idx_t ble_idx )
{
    storage_config_t *cfg = storage_get_config_ptr();
    uint8_t bond_flag = 0;

    if(cfg == NULL)
    {
        return 0;
    }

    switch(ble_idx)
    {
        case BLE_INDEX_1:
            bond_flag = BLE_BOND_FLAG_1;
            break;
        case BLE_INDEX_2:
            bond_flag = BLE_BOND_FLAG_2;
            break;
        case BLE_INDEX_3:
            bond_flag = BLE_BOND_FLAG_3;
            break;
        default:
            PRINT("work mode err %x\n",ble_idx);
            return 0;
    }

    return (cfg->ble_mac_flag & bond_flag) != 0u ? 1u : 0u;
}

/*********************************************************************
 * @fn      hidEmu_delete_ble_bonded_by_idx
 *
 * @brief   清除指定 ble_idx 的绑定标志（不依赖 con_work_mode）
 *
 * @param   idx - 要清除绑定的 BLE index
 *
 * @return  none
 */
void hidEmu_delete_ble_bonded_by_idx(access_ble_idx_t idx)
{
    storage_config_t *cfg = storage_get_config_ptr();
    uint8_t bond_flag = 0;

    if(cfg == NULL)
    {
        return;
    }

    switch(idx)
    {
        case BLE_INDEX_1:
            bond_flag = BLE_BOND_FLAG_1;
            break;
        case BLE_INDEX_2:
            bond_flag = BLE_BOND_FLAG_2;
            break;
        case BLE_INDEX_3:
            bond_flag = BLE_BOND_FLAG_3;
            break;
        default:
            PRINT("[BOND] delete_by_idx: invalid idx=%x\n", idx);
            return;
    }

    PRINT("[BOND] delete_by_idx: idx=%x flag=%x before=%x\n",
           idx, bond_flag, cfg->ble_bond_flag);
    cfg->ble_bond_flag &= (uint8_t)(~bond_flag);
    cfg->ble_mac_flag  &= (uint8_t)(~bond_flag);
    storage_save();
}

/*********************************************************************
* @fn      hidEmu_delete_ble_bonded
*
* @brief   清除所有绑定标志（包括 mac 标志和 irk 标志）
*
* @param   none
*
* @return  none
*/
void hidEmu_delete_ble_bonded(void){
    storage_config_t *cfg = storage_get_config_ptr();
    if(cfg == NULL)
    {
        return;
    }
    cfg->ble_bond_flag = 0;
    cfg->ble_mac_flag = 0;
    for(int i=0; i<BLE_INDEX_MAX; i++)
    {
        cfg->ble_irk_flag[i] = 0;
        // 版本号推进以生成新地址,
        cfg->ble_addr_ver[i]++;
    }
    // 删除所有绑定信息。
    HidDev_SetParameter(HIDDEV_ERASE_ALLBONDS,0,NULL);
    HidDev_SetParameter(HIDDEV_ERASE_ALLBONDS,0,NULL);
    GAPBondMgr_SetParameter(GAPBOND_ERASE_ALLBONDS, 0, NULL);
    GAPBondMgr_SetParameter(GAPBOND_ERASE_ALLBONDS, 0, NULL);
    // storage_save();
}
ble_adv_purpose_t adv_purpose;

void hidEmu_start_advertising_for_purpose(ble_adv_purpose_t purpose, uint8_t idx)
{
    const uint8_t ble_name_len = (uint8_t)(sizeof(BLE_PAIRING_NAME) - 1u);

    hidDev_adv_enable(0); // 停止当前广播，准备切换参数

    if (purpose ==BLE_ADV_PURPOSE_RECONNECT && hidEmu_is_ble_bonded(idx) == 0u) {
        PRINT("BLE: reconnect purpose but no bond (idx=%u), use initial adv\n", (unsigned)idx);
        purpose = BLE_ADV_PURPOSE_PAIRING_NEW; // 无绑定信息时退回配对流程
    }
    adv_purpose = purpose;
    switch (purpose) {
    case BLE_ADV_PURPOSE_PAIRING_NEW:
        hidEmu_regenerate_random_static_address((access_ble_idx_t)idx,TRUE);
        hidEmu_apply_advert_discoverability_flags(1);
        
        break;
    case BLE_ADV_PURPOSE_RECONNECT:
        hidEmu_regenerate_random_static_address((access_ble_idx_t)idx, FALSE);
        hidEmu_apply_advert_discoverability_flags(2);
        break;

    default:
        hidEmu_apply_advert_discoverability_flags(1);
        break;
    }

    hidEmu_apply_adv_payload((const uint8_t *)BLE_PAIRING_NAME, ble_name_len);
    access_ble_notify_advertising(purpose, idx);
    hidDevInitialAdvertising();
}
void hidEmu_SetConnParams(){
    GAPRole_PeripheralConnParamUpdateReq(hidEmuConnHandle,
        DEFAULT_DESIRED_MIN_CONN_INTERVAL,
        DEFAULT_DESIRED_MAX_CONN_INTERVAL,
        DEFAULT_DESIRED_SLAVE_LATENCY,
        DEFAULT_DESIRED_CONN_TIMEOUT,
        hidEmuTaskId); 
}
// 获取当前mtu
uint16_t hidEmu_get_current_mtu(){
    uint16_t read_length = 0;
    if (hidEmuConnHandle!= GAP_CONNHANDLE_INIT) {
        read_length = ATT_GetMTU(hidEmuConnHandle);
    }
    return read_length;
}
// 更新mtu
uint8_t hidEmu_update_mtu(uint16_t mtu){
    uint8_t result = 0;
    attExchangeMTUReq_t req = {
        .clientRxMTU = mtu - 4,
    };
    result = GATT_ExchangeMTU(hidEmuConnHandle,&req,hidEmuTaskId);
    return result;
}
/*********************************************************************
 * @fn      hidEmu_disconnect
 *
 * @brief   强制断开连接
 *
 * @return  none.
 */
void hidEmu_disconnect()
{
    GAPRole_TerminateLink(hidEmuConnHandle);
    if( tmos_get_task_timer( hidEmuTaskId, WAIT_TERMINATE_EVT ) || (tmos_get_event(hidEmuTaskId)&WAIT_TERMINATE_EVT) )
    {

    }
    else
    {
        tmos_start_task( hidEmuTaskId , WAIT_TERMINATE_EVT, TERMINATE_EVT_TIMEOUT );
    }
}
/*********************************************************************
 * 
 * @fn      hidEmu_GetGAP_State
 * @brief   获取当前 GAP 状态
 * @return  当前 GAP 状态值
 */
uint8_t hidEmu_GetGAP_State(){
    uint8_t ble_state;
    GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
    PRINT("GAP State: %x\n", ble_state);
    return ble_state;
}
/*********************************************************************
 * PROFILE CALLBACKS
 */

static hidDevCB_t hidEmuHidCBs = {
    hidEmuRptCB,
    hidEmuEvtCB,
    NULL,
    hidEmuStateCB};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      HidEmu_Init
 *
 * @brief   Initialization function for the HidEmuKbd App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by TMOS.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void HidEmu_Init()
{
    uint8_t initial_advertising_enable = FALSE;
    hidEmuTaskId = TMOS_ProcessEventRegister(HidEmu_ProcessEvent);
    
    GATT_InitClient();
    const char        ble_pairing_name[] = BLE_PAIRING_NAME;
    size_t            ble_name_len = sizeof(ble_pairing_name) - 1;
    memset(base_mac, 0, sizeof(base_mac));
    {
        uint16_t desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
        uint16_t desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
        
        // Set the GAP Role Parameters
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &initial_advertising_enable);
        hidEmu_apply_adv_payload((const uint8_t *)ble_pairing_name, (uint8_t)ble_name_len);

        GAPRole_SetParameter(GAPROLE_MIN_CONN_INTERVAL, sizeof(uint16_t), &desired_min_interval);
        GAPRole_SetParameter(GAPROLE_MAX_CONN_INTERVAL, sizeof(uint16_t), &desired_max_interval);
    }

    
    GGS_SetParameter(GGS_DEVICE_NAME_ATT, ble_name_len, (void *)ble_pairing_name);

    // Setup the GAP Bond Manager
    {
        // uint32_t passkey = DEFAULT_PASSCODE;
        uint8_t  pairMode = DEFAULT_PAIRING_MODE;
        uint8_t  mitm = DEFAULT_MITM_MODE;
        uint8_t  ioCap = DEFAULT_IO_CAPABILITIES;
        uint8_t  bonding = DEFAULT_BONDING_MODE;
        // GAPBondMgr_SetParameter(GAPBOND_PERI_DEFAULT_PASSCODE, sizeof(uint32_t), &passkey);
        GAPBondMgr_SetParameter(GAPBOND_PERI_PAIRING_MODE, sizeof(uint8_t), &pairMode);
        GAPBondMgr_SetParameter(GAPBOND_PERI_MITM_PROTECTION, sizeof(uint8_t), &mitm);
        GAPBondMgr_SetParameter(GAPBOND_PERI_IO_CAPABILITIES, sizeof(uint8_t), &ioCap);
        GAPBondMgr_SetParameter(GAPBOND_PERI_BONDING_ENABLED, sizeof(uint8_t), &bonding);
    }
    {
        // uint8_t param;

        GAP_SetParamValue(TGAP_DISC_ADV_INT_MIN, DEFAULT_ADVERTISING_INTERVAL);
        GAP_SetParamValue(TGAP_DISC_ADV_INT_MAX, DEFAULT_ADVERTISING_INTERVAL);
        GAP_SetParamValue(TGAP_LIM_ADV_TIMEOUT, 60);
    }
    
    {
        gapPeriConnectParams_t ConnectParams;
        ConnectParams.intervalMin = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
        ConnectParams.intervalMax = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
        ConnectParams.latency = DEFAULT_DESIRED_SLAVE_LATENCY;
        ConnectParams.timeout = DEFAULT_DESIRED_CONN_TIMEOUT;
        GGS_SetParameter(GGS_PERI_CONN_PARAM_ATT, sizeof(gapPeriConnectParams_t), &ConnectParams);
    }
    // Setup Battery Characteristic Values
    {
        uint8_t critical = DEFAULT_BATT_CRITICAL_LEVEL;
        Batt_SetParameter(BATT_PARAM_CRITICAL_LEVEL, sizeof(uint8_t), &critical);

        // critical = DEFAULT_BATT_CRITICAL_POWER_STATE;
        // Batt_SetParameter(BATT_PARAM_CRITICAL_POWER_STATE, sizeof(uint8_t), &critical);
    }

    // Set up HID keyboard service
    Hid_AddService();

    // Register for HID Dev callback
    HidDev_Register(&hidEmuCfg, &hidEmuHidCBs);

    // Setup a delayed profile startup
    OSAL_SetEvent(hidEmuTaskId, START_DEVICE_EVT);
}

/*********************************************************************
 * @fn      HidEmu_ProcessEvent
 *
 * @brief   HidEmuKbd Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16_t HidEmu_ProcessEvent(uint8_t task_id, uint16_t events)
{
    // static uint8_t send_char = 4;

    if(events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;

        if((pMsg = tmos_msg_receive(hidEmuTaskId)) != NULL)
        {
            hidEmu_ProcessTMOSMsg((tmos_event_hdr_t *)pMsg);

            // Release the TMOS message
            tmos_msg_deallocate(pMsg);
        }

        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    if(events & START_DEVICE_EVT)
    {
        return (events ^ START_DEVICE_EVT);
    }

    if(events & START_PARAM_UPDATE_EVT)//连接参数更新
    {

            GAPRole_PeripheralConnParamUpdateReq(hidEmuConnHandle,
                                                DEFAULT_DESIRED_MIN_CONN_INTERVAL,
                                                DEFAULT_DESIRED_MAX_CONN_INTERVAL,
                                                DEFAULT_DESIRED_SLAVE_LATENCY,
                                                DEFAULT_DESIRED_CONN_TIMEOUT,
                                                hidEmuTaskId);

        return (events ^ START_PARAM_UPDATE_EVT);
    }

    if(events & START_PHY_UPDATE_EVT)
    {
        // start phy update
        dprintf("Send Phy Update %x...\n", GAPRole_UpdatePHY(hidEmuConnHandle, 0, 
                    GAP_PHY_BIT_LE_2M, GAP_PHY_BIT_LE_2M, 0));

        return (events ^ START_PHY_UPDATE_EVT);
    }
    if(events & PERI_SECURITY_REQ_EVT)//外设安全请求
    {
        // start phy update
        uint8_t ble_state;
        GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
        if(ble_state == GAPROLE_CONNECTED)
        {
            dprintf("Send Security Req ...\n");
            if(GAPBondMgr_PeriSecurityReq(hidEmuConnHandle))
            {
                tmos_start_task(hidEmuTaskId, PERI_SECURITY_REQ_EVT, 4800);
            }
        }

        return (events ^ PERI_SECURITY_REQ_EVT);
    }

    if(events & WAIT_TERMINATE_EVT)
    {
        if(hidEmu_GetGAP_State() == GAPROLE_CONNECTED)
        {
            HCI_MB_DisconnectCmd(hidEmuConnHandle, 0x16);
        }
        return (events ^ WAIT_TERMINATE_EVT);
    }

    return 0;
}



/*********************************************************************
 * @fn      hidEmu_ProcessTMOSMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void hidEmu_ProcessTMOSMsg(tmos_event_hdr_t *pMsg)
{
    switch(pMsg->event)
    {
        default:
            break;
    }
}


/*********************************************************************
 * @fn      hidEmuStateCB
 *
 * @brief   GAP state change callback.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void hidEmuStateCB(gapRole_States_t newState, gapRoleEvent_t *pEvent)
{
    uint8_t opcode = 0xFF;
    if(pEvent != NULL)
    {
        opcode = pEvent->gap.opcode;
    }
    PRINT("[BT_CB] state=%x opcode=%x work_mode=%d pairing=%d ble_idx=%d con_work_mode=%d disc_mode=%d\n",
          newState, opcode, access_state.work_mode, access_state.pairing_state,
          access_state.ble_idx, con_work_mode, discoverability_mode);
    switch(newState & GAPROLE_STATE_ADV_MASK)
    {
        case GAPROLE_STARTED:
        {
            GAPRole_GetParameter(GAPROLE_BD_ADDR, base_mac);
            GAP_ConfigDeviceAddr(ADDRTYPE_STATIC, base_mac);
            access_ble_init_done();
            ble_init_done = TRUE;
            PRINT("Initialized..\n");
        }
        break;

        case GAPROLE_ADVERTISING:
            if(pEvent->gap.opcode == GAP_MAKE_DISCOVERABLE_DONE_EVENT)
            {
                PRINT("Advertising..\n");
            }
            break;

        case GAPROLE_CONNECTED://连接成功
            if(pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT)
            {
                gapEstLinkReqEvent_t *event = (gapEstLinkReqEvent_t *)pEvent;

                // get connection handle
                hidEmuConnHandle = event->connectionHandle;
                tmos_start_task(hidEmuTaskId, PERI_SECURITY_REQ_EVT, 4800);
                tmos_start_task(hidEmuTaskId, START_PARAM_UPDATE_EVT, START_PARAM_UPDATE_EVT_DELAY);


                PRINT("[BT_CONN] established handle=%x interval=%u latency=%u timeout=%u work_mode=%d pairing=%d ble_idx=%d\n",
                      event->connectionHandle, event->connInterval, event->connLatency,
                      event->connTimeout, access_state.work_mode, access_state.pairing_state,
                      access_state.ble_idx);
                PRINT("Connected..\n");
                // pairedLedStop();
            }
            break;

        case GAPROLE_CONNECTED_ADV:
            if(pEvent->gap.opcode == GAP_MAKE_DISCOVERABLE_DONE_EVENT)
            {
                PRINT("Connected Advertising..\n");
            }
            break;

        case GAPROLE_WAITING:
            if(pEvent->gap.opcode == GAP_END_DISCOVERABLE_DONE_EVENT)
            {
                // 1、切换到其他模式，命令停止的广播，则判断模式，不开启新的广�?
                // 2、还是当前模式没变，只是limit广播自动停止，则继续广播(注意是否是OTA模式)
                // 3、切换到另外的蓝牙模式，命令停止的广播，则修改mac地址，判断是否已经绑定过，是则开启广播，并开启过滤，否则不开启广播，等待配对命令�?
                if (access_state.work_mode == WORK_PARING) {
                    access_state.work_mode = WORK_IDEL;
                    hidEmu_pairing_adv(access_state.ble_idx );
                } else if (access_state.work_mode == WORK_RECONNECT) {
                    access_state.work_mode = WORK_IDEL;
                    hidEmu_connect_adv(access_state.ble_idx );
                } else if (access_state.work_mode == WORK_DISCONNECTED) {
                    access_state.work_mode = WORK_IDEL;
                    access_ble_notify_advertis_end(adv_purpose, access_state.ble_idx);
                }else {
                    if(discoverability_mode==1)
                    {
                        PRINT("RESTART LIMITED ADVERTISING..\n");
                        hidEmu_apply_advert_discoverability_flags(2);
                    }
                    hidEmu_apply_adv_payload((const uint8_t *)BLE_PAIRING_NAME,
                                             (uint8_t)(sizeof(BLE_PAIRING_NAME) - 1u));
                    hidDevInitialAdvertising();
                } 
            }
            else if(pEvent->gap.opcode == GAP_LINK_TERMINATED_EVENT)
            {
                if(tmos_get_task_timer(hidEmuTaskId, WAIT_TERMINATE_EVT))
                {
                    tmos_stop_task(hidEmuTaskId, WAIT_TERMINATE_EVT);
                }
                PRINT("Disconnected.. Reason:%x\n", pEvent->linkTerminate.reason);
                PRINT("[BT_DISC] reason=%x work_mode=%d pairing=%d ble_idx=%d con_work_mode=%d hid_state=%x\n",
                      pEvent->linkTerminate.reason, access_state.work_mode,
                      access_state.pairing_state, access_state.ble_idx,
                      con_work_mode, hidEmu_GetGAP_State());

                if (access_state.work_mode == WORK_PARING) {
                    access_state.work_mode = WORK_IDEL;
                    PRINT("RESTART PAIRING ADVERTISING..\n");
                    hidEmu_pairing_adv(access_state.ble_idx );
                } else if (access_state.work_mode == WORK_RECONNECT) {
                    access_state.work_mode = WORK_IDEL;
                    PRINT("RESTART RECONNECT ADVERTISING..\n");
                    hidEmu_connect_adv(access_state.ble_idx );
                } else {
                    access_ble_notify_disconnected(BLE_INDEX_1, pEvent->linkTerminate.reason);
                } 
            }
            else if(pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT)
            {
                PRINT("Advertising timeout..\n");
                
                // if(hasFactoryReset!=1){
                //     needStartLowAdvertising(1);
                // }
                
            }
            break;

        case GAPROLE_ERROR:
            PRINT("Error %x ..\n", pEvent->gap.opcode);
            break;

        default:
            break;
    }
}

/*********************************************************************
 * @fn      hidEmuRcvReport
 *
 * @brief   Process an incoming HID keyboard report.
 *
 * @param   len - Length of report.
 * @param   pData - Report data.
 *
 * @return  status
 */
static uint8_t hidEmuRcvReport(uint8_t len, uint8_t *pData)
{
    // verify data length
    if(len == HID_LED_OUT_RPT_LEN)
    {
        dprintf("LED OUT report: len=%u data=%02X\n", len, pData[0]);
        bt_driver_notify_led_state(pData[0]);
        return SUCCESS;
    }
    else
    {
        return ATT_ERR_INVALID_VALUE_SIZE;
    }
}

/*********************************************************************
 * @fn      hidEmuRptCB
 *
 * @brief   HID Dev report callback.
 *
 * @param   id - HID report ID.
 * @param   type - HID report type.
 * @param   uuid - attribute uuid.
 * @param   oper - operation:  read, write, etc.
 * @param   len - Length of report.
 * @param   pData - Report data.
 *
 * @return  GATT status code.
 */
static uint8_t hidEmuRptCB(uint8_t id, uint8_t type, uint16_t uuid,
                           uint8_t oper, uint16_t *pLen, uint8_t *pData)
{
    uint8_t status = SUCCESS;

    // write
    if(oper == HID_DEV_OPER_WRITE)
    {
        if(uuid == REPORT_UUID)
        {
            // process write to LED output report; ignore others
            if(type == HID_REPORT_TYPE_OUTPUT)
            {
                status = hidEmuRcvReport(*pLen, pData);
            }
        }

        if(status == SUCCESS)
        {
            status = Hid_SetParameter(id, type, uuid, *pLen, pData);
        }
    }
    // read
    else if(oper == HID_DEV_OPER_READ)
    {
        status = Hid_GetParameter(id, type, uuid, pLen, pData);
    }
    // notifications enabled
    else if(oper == HID_DEV_OPER_ENABLE)
    {
        // tmos_start_task(hidEmuTaskId, START_REPORT_EVT, MS1_TO_SYSTEM_TIME(5));
    }
    return status;
}

/*********************************************************************
 * @fn      hidEmuEvtCB
 *
 * @brief   HID Dev event callback.
 *
 * @param   evt - event ID.
 *
 * @return  HID response code.
 */
static void hidEmuEvtCB(uint8_t evt)
{
    // process enter/exit suspend or enter/exit boot mode
    return;
}

/*********************************************************************
 * @fn      hidEmu_pairing_adv
 *
 * @brief   启动配对广播（全开放过滤，幂等地址计算）
 *          对应 BLE_INTENT_PAIRING
 *
 * @param   idx - 目标 BLE index
 *
 * @return  none
 */
void hidEmu_pairing_adv(uint8_t idx)
{
    con_work_mode = idx;
    PRINT("[BT_ADV] pairing idx=%d work_mode=%d pairing=%d ble_idx=%d\n",
          idx, access_state.work_mode, access_state.pairing_state, access_state.ble_idx);
    hidEmu_start_advertising_for_purpose(BLE_ADV_PURPOSE_PAIRING_NEW, idx);
}


/*********************************************************************
 * @fn      hidEmu_connect_adv
 *
 * @brief   启动回连广播（已绑定：低占空比 + General；未绑定：退化为可发现初始广播）
 *          对应 BLE_INTENT_RECONNECT / bt_driver_connect_ex
 *
 * @param   idx - 目标 BLE index
 *
 * @return  none
 */
void hidEmu_connect_adv(uint8_t idx)
{
    con_work_mode = idx;
    PRINT("[BT_ADV] reconnect idx=%d work_mode=%d pairing=%d ble_idx=%d\n",
          idx, access_state.work_mode, access_state.pairing_state, access_state.ble_idx);
    hidEmu_start_advertising_for_purpose(BLE_ADV_PURPOSE_RECONNECT, idx);
}
