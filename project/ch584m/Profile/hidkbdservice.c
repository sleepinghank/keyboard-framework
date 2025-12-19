/********************************** (C) COPYRIGHT *******************************
 * File Name          : hidkbdservice.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/12/10
 * Description        : ????????
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*********************************************************************
 * ??????
 */
#include "CONFIG.h"
#include "hidkbdservice.h"
#include "hiddev.h"
#include "battservice.h"

/*********************************************************************
 * ???
 */

/*********************************************************************
 * ????
 */

/*********************************************************************
 * ????
 */

/*********************************************************************
 * ????
 */
// HID??UUID
const uint8_t hidServUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(HID_SERV_UUID), HI_UINT16(HID_SERV_UUID)};

// HID??????????UUID
const uint8_t hidBootKeyInputUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(BOOT_KEY_INPUT_UUID), HI_UINT16(BOOT_KEY_INPUT_UUID)};

// HID??????????UUID
const uint8_t hidBootKeyOutputUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(BOOT_KEY_OUTPUT_UUID), HI_UINT16(BOOT_KEY_OUTPUT_UUID)};

// HID????UUID
const uint8_t hidInfoUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(HID_INFORMATION_UUID), HI_UINT16(HID_INFORMATION_UUID)};

// HID??????UUID
const uint8_t hidReportMapUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(REPORT_MAP_UUID), HI_UINT16(REPORT_MAP_UUID)};

// HID?????UUID
const uint8_t hidControlPointUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(HID_CTRL_PT_UUID), HI_UINT16(HID_CTRL_PT_UUID)};

// HID????UUID
const uint8_t hidReportUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(REPORT_UUID), HI_UINT16(REPORT_UUID)};

// HID??????UUID
const uint8_t hidProtocolModeUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(PROTOCOL_MODE_UUID), HI_UINT16(PROTOCOL_MODE_UUID)};

/*********************************************************************
 * ????
 */

/*********************************************************************
 * ????
 */

/*********************************************************************
 * ????
 */

// HID?????
static const uint8_t hidInfo[HID_INFORMATION_LEN] = {
    LO_UINT16(0x0111), HI_UINT16(0x0111), // bcdHID (USB HID??)
    0x00,                                 // bCountryCode (????)
    HID_FEATURE_FLAGS                     // ????
};

// HID??????? - ??????????
static const uint8_t hidReportMap[] = {
    0x05, 0x01, // ???(????)
    0x09, 0x06, // ??(??)
    0xA1, 0x01, // ??(??)
                //
    0x05, 0x07, // ???(?????)
    0x19, 0xE0, // ?????(224)
    0x29, 0xE7, // ?????(231)
    0x15, 0x00, // ?????(0)
    0x25, 0x01, // ?????(1)
                //
                // ?????
    0x75, 0x01, // ????(1)
    0x95, 0x08, // ????(8)
    0x81, 0x02, // ??(??,??,???)
                //
                // ????
    0x95, 0x01, // ????(1)
    0x75, 0x08, // ????(8)
    0x81, 0x01, // ??(??)
                //
                // LED??
    0x95, 0x05, // ????(5)
    0x75, 0x01, // ????(1)
    0x05, 0x08, // ???(LED)
    0x19, 0x01, // ?????(1)
    0x29, 0x05, // ?????(5)
    0x91, 0x02, // ??(??,??,???)
                //
                // LED????
    0x95, 0x01, // ????(1)
    0x75, 0x03, // ????(3)
    0x91, 0x01, // ??(??)
                //
                // ???(6??)
    0x95, 0x06, // ????(6)
    0x75, 0x08, // ????(8)
    0x15, 0x00, // ?????(0)
    0x25, 0x65, // ?????(101)
    0x05, 0x07, // ???(?????)
    0x19, 0x00, // ?????(0)
    0x29, 0x65, // ?????(101)
    0x81, 0x00, // ??(??,??)
                //
    0xC0        // ????
};

// HID??????
uint16_t hidReportMapLen = sizeof(hidReportMap);

// HID?????
static hidRptMap_t hidRptMap[HID_NUM_REPORTS];

/*********************************************************************
 * Profile?? - ??
 */

// HID????
static const gattAttrType_t hidService = {ATT_BT_UUID_SIZE, hidServUUID};

// ????(????)
static uint16_t include = GATT_INVALID_HANDLE;

// HID??????
static uint8_t hidInfoProps = GATT_PROP_READ;

// HID????????
static uint8_t hidReportMapProps = GATT_PROP_READ;

// HID?????????
static uint8_t hidExtReportRefDesc[ATT_BT_UUID_SIZE] =
    {LO_UINT16(BATT_LEVEL_UUID), HI_UINT16(BATT_LEVEL_UUID)};

// HID???????
static uint8_t hidControlPointProps = GATT_PROP_WRITE_NO_RSP;
static uint8_t hidControlPoint;

// HID????????
static uint8_t hidProtocolModeProps = GATT_PROP_READ | GATT_PROP_WRITE_NO_RSP;
uint8_t        hidProtocolMode = HID_PROTOCOL_MODE_REPORT;

// HID????,????
static uint8_t       hidReportKeyInProps = GATT_PROP_READ | GATT_PROP_NOTIFY;
static uint8_t       hidReportKeyIn;
static gattCharCfg_t hidReportKeyInClientCharCfg[GATT_MAX_NUM_CONN];

// HID?????????,????
static uint8_t hidReportRefKeyIn[HID_REPORT_REF_LEN] =
    {HID_RPT_ID_KEY_IN, HID_REPORT_TYPE_INPUT};

// HID????,LED??
static uint8_t hidReportLedOutProps = GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP;
static uint8_t hidReportLedOut;

// HID?????????,LED??
static uint8_t hidReportRefLedOut[HID_REPORT_REF_LEN] =
    {HID_RPT_ID_LED_OUT, HID_REPORT_TYPE_OUTPUT};

// HID????????
static uint8_t       hidReportBootKeyInProps = GATT_PROP_READ | GATT_PROP_NOTIFY;
static uint8_t       hidReportBootKeyIn;
static gattCharCfg_t hidReportBootKeyInClientCharCfg[GATT_MAX_NUM_CONN];

// HID????????
static uint8_t hidReportBootKeyOutProps = GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP;
static uint8_t hidReportBootKeyOut;

// ????
static uint8_t hidReportFeatureProps = GATT_PROP_READ | GATT_PROP_WRITE;
static uint8_t hidReportFeature;

// HID?????????,??
static uint8_t hidReportRefFeature[HID_REPORT_REF_LEN] =
    {HID_RPT_ID_FEATURE, HID_REPORT_TYPE_FEATURE};

/*********************************************************************
 * Profile?? - ?
 */

// HID?????
static gattAttribute_t hidAttrTbl[] = {
    // HID????
    {
        {ATT_BT_UUID_SIZE, primaryServiceUUID}, /* ?? */
        GATT_PERMIT_READ,                       /* ?? */
        0,                                      /* ?? */
        (uint8_t *)&hidService                  /* ??? */
    },

    // ????(??)
    {
        {ATT_BT_UUID_SIZE, includeUUID},
        GATT_PERMIT_READ,
        0,
        (uint8_t *)&include},

    // HID??????
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &hidInfoProps},

    // HID????
    {
        {ATT_BT_UUID_SIZE, hidInfoUUID},
        GATT_PERMIT_ENCRYPT_READ,
        0,
        (uint8_t *)hidInfo},

    // HID???????
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &hidControlPointProps},

    // HID?????
    {
        {ATT_BT_UUID_SIZE, hidControlPointUUID},
        GATT_PERMIT_ENCRYPT_WRITE,
        0,
        &hidControlPoint},

    // HID????????
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &hidProtocolModeProps},

    // HID??????
    {
        {ATT_BT_UUID_SIZE, hidProtocolModeUUID},
        GATT_PERMIT_ENCRYPT_READ | GATT_PERMIT_ENCRYPT_WRITE,
        0,
        &hidProtocolMode},

    // HID????????
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &hidReportMapProps},

    // HID??????
    {
        {ATT_BT_UUID_SIZE, hidReportMapUUID},
        GATT_PERMIT_ENCRYPT_READ,
        0,
        (uint8_t *)hidReportMap},

    // HID?????????
    {
        {ATT_BT_UUID_SIZE, extReportRefUUID},
        GATT_PERMIT_READ,
        0,
        hidExtReportRefDesc
    },

    // HID????,??????
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &hidReportKeyInProps},

    // HID????,????
    {
        {ATT_BT_UUID_SIZE, hidReportUUID},
        GATT_PERMIT_ENCRYPT_READ,
        0,
        &hidReportKeyIn},

    // HID???????????
    {
        {ATT_BT_UUID_SIZE, clientCharCfgUUID},
        GATT_PERMIT_READ | GATT_PERMIT_ENCRYPT_WRITE,
        0,
        (uint8_t *)&hidReportKeyInClientCharCfg},

    // HID?????????,????
    {
        {ATT_BT_UUID_SIZE, reportRefUUID},
        GATT_PERMIT_READ,
        0,
        hidReportRefKeyIn},

    // HID????,LED????
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &hidReportLedOutProps},

    // HID????,LED??
    {
        {ATT_BT_UUID_SIZE, hidReportUUID},
        GATT_PERMIT_ENCRYPT_READ | GATT_PERMIT_ENCRYPT_WRITE,
        0,
        &hidReportLedOut},

    // HID?????????,LED??
    {
        {ATT_BT_UUID_SIZE, reportRefUUID},
        GATT_PERMIT_READ,
        0,
        hidReportRefLedOut},

    // HID??????????
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &hidReportBootKeyInProps},

    // HID????????
    {
        {ATT_BT_UUID_SIZE, hidBootKeyInputUUID},
        GATT_PERMIT_ENCRYPT_READ,
        0,
        &hidReportBootKeyIn},

    // HID???????????????
    {
        {ATT_BT_UUID_SIZE, clientCharCfgUUID},
        GATT_PERMIT_READ | GATT_PERMIT_ENCRYPT_WRITE,
        0,
        (uint8_t *)&hidReportBootKeyInClientCharCfg},

    // HID??????????
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &hidReportBootKeyOutProps},

    // HID????????
    {
        {ATT_BT_UUID_SIZE, hidBootKeyOutputUUID},
        GATT_PERMIT_ENCRYPT_READ | GATT_PERMIT_ENCRYPT_WRITE,
        0,
        &hidReportBootKeyOut},

    // ??????
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &hidReportFeatureProps},

    // ????
    {
        {ATT_BT_UUID_SIZE, hidReportUUID},
        GATT_PERMIT_ENCRYPT_READ | GATT_PERMIT_ENCRYPT_WRITE,
        0,
        &hidReportFeature},

    // HID?????????,??
    {
        {ATT_BT_UUID_SIZE, reportRefUUID},
        GATT_PERMIT_READ,
        0,
        hidReportRefFeature},
};

// ?????? - ?????????????
enum
{
    HID_SERVICE_IDX,             // HID??
    HID_INCLUDED_SERVICE_IDX,    // ?????
    HID_INFO_DECL_IDX,          // HID??????
    HID_INFO_IDX,               // HID????
    HID_CONTROL_POINT_DECL_IDX, // HID???????
    HID_CONTROL_POINT_IDX,      // HID?????
    HID_PROTOCOL_MODE_DECL_IDX, // HID????????
    HID_PROTOCOL_MODE_IDX,      // HID??????
    HID_REPORT_MAP_DECL_IDX,    // HID????????
    HID_REPORT_MAP_IDX,         // HID??????
    HID_EXT_REPORT_REF_DESC_IDX,// HID?????????
    HID_REPORT_KEY_IN_DECL_IDX, // HID????,??????
    HID_REPORT_KEY_IN_IDX,      // HID????,????
    HID_REPORT_KEY_IN_CCCD_IDX, // HID???????????
    HID_REPORT_REF_KEY_IN_IDX,  // HID?????????,????
    HID_REPORT_LED_OUT_DECL_IDX,// HID????,LED????
    HID_REPORT_LED_OUT_IDX,     // HID????,LED??
    HID_REPORT_REF_LED_OUT_IDX, // HID?????????,LED??
    HID_BOOT_KEY_IN_DECL_IDX,   // HID??????????
    HID_BOOT_KEY_IN_IDX,        // HID????????
    HID_BOOT_KEY_IN_CCCD_IDX,   // HID???????????????
    HID_BOOT_KEY_OUT_DECL_IDX,  // HID??????????
    HID_BOOT_KEY_OUT_IDX,       // HID????????
    HID_FEATURE_DECL_IDX,       // ??????
    HID_FEATURE_IDX,            // ????
    HID_REPORT_REF_FEATURE_IDX  // HID?????????,??
};

/*********************************************************************
 * ????
 */

/*********************************************************************
 * PROFILE??
 */

// ????
gattServiceCBs_t hidKbdCBs = {
    HidDev_ReadAttrCB,  // ????????
    HidDev_WriteAttrCB, // ????????
    NULL                // ????????
};

/*********************************************************************
 * ????
 */

/*********************************************************************
 * @fn      Hid_AddService
 *
 * @brief   ???HID??,???GATT?????GATT??
 *
 * @return  ?????
 */
bStatus_t Hid_AddService(void)
{
    uint8_t status = SUCCESS;

    // ????????????
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, hidReportKeyInClientCharCfg);
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, hidReportBootKeyInClientCharCfg);

    // ?GATT???????GATT???????
    status = GATTServApp_RegisterService(hidAttrTbl, GATT_NUM_ATTRS(hidAttrTbl), GATT_MAX_ENCRYPT_KEY_SIZE, &hidKbdCBs);

    // ???????
    Batt_GetParameter(BATT_PARAM_SERVICE_HANDLE,
                      &GATT_INCLUDED_HANDLE(hidAttrTbl, HID_INCLUDED_SERVICE_IDX));

    // ????????????
    // ???????ID???????

    // ??????
    hidRptMap[0].id = hidReportRefKeyIn[0];
    hidRptMap[0].type = hidReportRefKeyIn[1];
    hidRptMap[0].handle = hidAttrTbl[HID_REPORT_KEY_IN_IDX].handle;
    hidRptMap[0].cccdHandle = hidAttrTbl[HID_REPORT_KEY_IN_CCCD_IDX].handle;
    hidRptMap[0].mode = HID_PROTOCOL_MODE_REPORT;

    // LED????
    hidRptMap[1].id = hidReportRefLedOut[0];
    hidRptMap[1].type = hidReportRefLedOut[1];
    hidRptMap[1].handle = hidAttrTbl[HID_REPORT_LED_OUT_IDX].handle;
    hidRptMap[1].cccdHandle = 0;
    hidRptMap[1].mode = HID_PROTOCOL_MODE_REPORT;

    // ????????
    // ????????????ID???
    hidRptMap[2].id = hidReportRefKeyIn[0];
    hidRptMap[2].type = hidReportRefKeyIn[1];
    hidRptMap[2].handle = hidAttrTbl[HID_BOOT_KEY_IN_IDX].handle;
    hidRptMap[2].cccdHandle = hidAttrTbl[HID_BOOT_KEY_IN_CCCD_IDX].handle;
    hidRptMap[2].mode = HID_PROTOCOL_MODE_BOOT;

    // ????????
    // ???LED???????ID???
    hidRptMap[3].id = hidReportRefLedOut[0];
    hidRptMap[3].type = hidReportRefLedOut[1];
    hidRptMap[3].handle = hidAttrTbl[HID_BOOT_KEY_OUT_IDX].handle;
    hidRptMap[3].cccdHandle = 0;
    hidRptMap[3].mode = HID_PROTOCOL_MODE_BOOT;

    // ????
    hidRptMap[4].id = hidReportRefFeature[0];
    hidRptMap[4].type = hidReportRefFeature[1];
    hidRptMap[4].handle = hidAttrTbl[HID_FEATURE_IDX].handle;
    hidRptMap[4].cccdHandle = 0;
    hidRptMap[4].mode = HID_PROTOCOL_MODE_REPORT;

    // ????????
    Batt_GetParameter(BATT_PARAM_BATT_LEVEL_IN_REPORT, &(hidRptMap[5]));

    // ????ID??
    HidDev_RegisterReports(HID_NUM_REPORTS, hidRptMap);

    return (status);
}

/*********************************************************************
 * @fn      Hid_SetParameter
 *
 * @brief   ??HID????
 *
 * @param   id - HID??ID
 * @param   type - HID????
 * @param   uuid - ??uuid
 * @param   len - ????????
 * @param   pValue - ???????????????????,??????????????
 *          (??:uint16_t??????????uint16_t??)
 *
 * @return  GATT???
 */
uint8_t Hid_SetParameter(uint8_t id, uint8_t type, uint16_t uuid, uint8_t len, void *pValue)
{
    bStatus_t ret = SUCCESS;

    switch(uuid)
    {
        case REPORT_UUID:
            if(type == HID_REPORT_TYPE_OUTPUT)
            {
                if(len == 1)
                {
                    hidReportLedOut = *((uint8_t *)pValue);
                }
                else
                {
                    ret = ATT_ERR_INVALID_VALUE_SIZE;
                }
            }
            else if(type == HID_REPORT_TYPE_FEATURE)
            {
                if(len == 1)
                {
                    hidReportFeature = *((uint8_t *)pValue);
                }
                else
                {
                    ret = ATT_ERR_INVALID_VALUE_SIZE;
                }
            }
            else
            {
                ret = ATT_ERR_ATTR_NOT_FOUND;
            }
            break;

        case BOOT_KEY_OUTPUT_UUID:
            if(len == 1)
            {
                hidReportBootKeyOut = *((uint8_t *)pValue);
            }
            else
            {
                ret = ATT_ERR_INVALID_VALUE_SIZE;
            }
            break;

        default:
            // ????
            break;
    }

    return (ret);
}

/*********************************************************************
 * @fn      Hid_GetParameter
 *
 * @brief   ??HID????
 *
 * @param   id - HID??ID
 * @param   type - HID????
 * @param   uuid - ??uuid
 * @param   pLen - ????????
 * @param   pValue - ??????????????????,??????????????
 *          (??:uint16_t??????????uint16_t??)
 *
 * @return  GATT???
 */
uint8_t Hid_GetParameter(uint8_t id, uint8_t type, uint16_t uuid, uint16_t *pLen, void *pValue)
{
    switch(uuid)
    {
        case REPORT_UUID:
            if(type == HID_REPORT_TYPE_OUTPUT)
            {
                *((uint8_t *)pValue) = hidReportLedOut;
                *pLen = 1;
            }
            else if(type == HID_REPORT_TYPE_FEATURE)
            {
                *((uint8_t *)pValue) = hidReportFeature;
                *pLen = 1;
            }
            else
            {
                *pLen = 0;
            }
            break;

        case BOOT_KEY_OUTPUT_UUID:
            *((uint8_t *)pValue) = hidReportBootKeyOut;
            *pLen = 1;
            break;

        default:
            *pLen = 0;
            break;
    }

    return (SUCCESS);
}

/*********************************************************************
*********************************************************************/
