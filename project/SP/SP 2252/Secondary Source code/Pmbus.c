
#include "Pmbus.h"
#include "I2c.h"
#include "Util.h"
#include "UserData.h"
#include "Fan.h"
#include "Status.h"
#include "Parameter.h"
#include "Query.h"
#include "Sci.h"
#include "Process.h"
#include "Protection.h"
#include "Psfb.h"
#include "Isr.h"
#include <string.h>
#include "Init.h"
#include "Led.h"

#include "memory.h" //[davidchchen] 20150113 Added
#include "RevisionBlock.h"  //[davidchchen]20160304 added RCB

//EIN / EOUT =>
#define Prollover 0x7FFF
#define Max_Rollover_Cnt 0xFF
#define MAX_Sample_Cnt 0xFFFFFF
//EIN / EOUT <=

//SMART_ON =>
#define SMART_ON_MASK	0b00000001
//SMART_ON <=

#define TEMP_MFR_DATA 			FALSE
#define PSU_FACTORY_MODE_MASK		0b11111110
#define PSU_FACTORY_MODE_EN_MASK	0b00000001
#define FRU_RW_GRANTED			0b10

SHORT gVoutCmd = 0;
SHORT gVoutCmdOffset = 0;
SHORT gVoutReadOffsetAdc = 0;
SHORT gMainBusVoutOffset = 0;
SHORT gStbVoutOffset = 0;
SHORT gVCS_Slope = 1024;
LONG gVCS_Offset = 0;

DWORD_VAL  MFR_POS_TOTAL;           //[davidchchen]20170418 Added Black Box block read
DWORD_VAL  MFR_POS_LAST;            //[davidchchen]20170418 Added Black Box block read
DWORD_VAL  BMC_UNIX_TIMESTAMP;      //[davidchchen]20170418 Added Black Box block read

BYTE isOpON;
BYTE marginState;



extern tD_I2C_DATA I2C;

tPMBUS_CMDS gPmbusCmd;
tDIRECT_FMT_CMD gCoef;
tSMBALERT_MASK gAlertMask[TOTAL_PAGE_COUNT];
tPAGE_PLUS_STATUS gPagePlusStatus;
tISP_DATA gISP;
BYTE gIsFactoryMode = FALSE;

extern tPS_FLAG PS;
extern tPSU_STATE gPS_State;
extern BYTE isInputDetected;

#if 1
extern signed int cs_kp;
extern signed int cs_ki;
#endif
WORD Real_Vac = 0;

/*static*/ tCMDS /*__attribute__((space(data),aligned(16)))*/ gCmd[256] = {
                                                                           /*0x00*/
    {gPmbusCmd.PAGE, sizeof(gPmbusCmd.PAGE), QUERY_PAGE},
                                                                           /*0x01*/
    {gPmbusCmd.OPERATION, sizeof(gPmbusCmd.OPERATION), QUERY_OPERATION},
                                                                           /*0x02*/
    {gPmbusCmd.ON_OFF_CONFIG, sizeof(gPmbusCmd.ON_OFF_CONFIG), QUERY_ON_OFF_CONFIG},
                                                                           /*0x03*/
    {0, 0, QUERY_CLEAR_FAULTS}, //{&CLEAR_FAULTS, 0}, /*There is no data for CLEAR_FAULT command*/
                                                                           /*0x04*/
    {0, 0, 0},
                                                                           /*0x05*/
    {gPmbusCmd.PAGE_PLUS_WRITE, sizeof(gPmbusCmd.PAGE_PLUS_WRITE), QUERY_PAGE_PLUS_W},
                                                                           /*0x06*/
    {gPmbusCmd.PAGE_PLUS_READ, sizeof(gPmbusCmd.PAGE_PLUS_READ), QUERY_PAGE_PLUS_R},
                                                                           /*0x07*/
    {gPmbusCmd.DISABLE_U1_TX, sizeof(gPmbusCmd.DISABLE_U1_TX), QUERY_DISABLE_U1_TX},
                                                                           /*0x08*/
    {0, 0, 0},
                                                                           /*0x09*/
    {0, 0, 0},
                                                                           /*0x0A*/
    {0, 0, 0},
                                                                           /*0x0B*/
    {0, 0, 0},
                                                                           /*0x0C*/
    {0, 0, 0},
                                                                           /*0x0D*/
    {0, 0, 0},
                                                                           /*0x0E*/
    {0, 0, 0},
                                                                           /*0x0F*/
    {0, 0, 0},
                                                                           /*0x10*/
    {gPmbusCmd.WRITE_PROTECT, sizeof(gPmbusCmd.WRITE_PROTECT), QUERY_WRITE_PROTECT},
                                                                           /*0x11*/
    {0, 0, QUERY_STORE_DEFAULT_ALL},
                                                                           /*0x12*/
    {0, 0, 0},
                                                                           /*0x13*/
    {0, 0, 0},
                                                                           /*0x14*/
    {0, 0, 0},
                                                                           /*0x15*/
    {0, 0, 0},
                                                                           /*0x16*/
    {0, 0, 0},
                                                                           /*0x17*/
    {0, 0, 0},
                                                                           /*0x18*/
    {0, 0, 0},
                                                                           /*0x19*/
    {gPmbusCmd.CAPABILITY, sizeof(gPmbusCmd.CAPABILITY), QUERY_CAPABILITY},
                                                                           /*0x1A*/
    {gPmbusCmd.QUERY, sizeof(gPmbusCmd.QUERY), QUERY_QUERY},
                                                                           /*0x1B*/
    {gPmbusCmd.SMBALERT_MASK, sizeof(gPmbusCmd.SMBALERT_MASK), QUERY_SMBALERT_MASK},
                                                                           /*0x1C*/
    {0, 0, 0},
                                                                           /*0x1D*/
    {0, 0, 0},
                                                                           /*0x1E*/
    {0, 0, 0},
                                                                           /*0x1F*/
    {0, 0, 0},
                                                                           /*0x20*/
    {gPmbusCmd.VOUT_MODE, sizeof(gPmbusCmd.VOUT_MODE), QUERY_VOUT_MODE},
                                                                           /*0x21*/
    {gPmbusCmd.VOUT_COMMAND, sizeof(gPmbusCmd.VOUT_COMMAND), QUERY_VOUT_COMMAND},
                                                                           /*0x22*/
    {0, 0, 0},
                                                                           /*0x23*/
    {0, 0, 0},
                                                                           /*0x24*/
    {0, 0, 0},
                                                                           /*0x25*/
    {0, 0, 0},
                                                                           /*0x26*/
    {0, 0, 0},
                                                                           /*0x27*/
    {0, 0, 0},
                                                                           /*0x28*/
    {0, 0, 0},
                                                                           /*0x29*/
    {0, 0, 0},
                                                                           /*0x2A*/
    {0, 0, 0},
                                                                           /*0x2B*/
    {0, 0, 0},
                                                                           /*0x2C*/
    {0, 0, 0},
                                                                           /*0x2D*/
    {0, 0, 0},
                                                                           /*0x2E*/
    {0, 0, 0},
                                                                           /*0x2F*/
    {0, 0, 0},
                                                                           /*0x30*/
    {gPmbusCmd.COEFFICIENT, sizeof(gPmbusCmd.COEFFICIENT), QUERY_COEFFICIENT},
                                                                           /*0x31*/
    {0, 0, 0},
                                                                           /*0x32*/
    {0, 0, 0},
                                                                           /*0x33*/
    {0, 0, 0},
                                                                           /*0x34*/
    {0, 0, 0},
                                                                           /*0x35*/
    {0, 0, 0},
                                                                           /*0x36*/
    {0, 0, 0},
                                                                           /*0x37*/
    {0, 0, 0},
                                                                           /*0x38*/
    {0, 0, 0},
                                                                           /*0x39*/
    {0, 0, 0},
                                                                           /*0x3A*/
    {gPmbusCmd.FAN_CONFIG_1_2, sizeof(gPmbusCmd.FAN_CONFIG_1_2), QUERY_FAN_CONFIG_1_2},
                                                                           /*0x3B*/
    {gPmbusCmd.FAN_COMMAND_1, sizeof(gPmbusCmd.FAN_COMMAND_1), QUERY_FAN_COMMAND_1},
                                                                           /*0x3C*/
    {0, 0, 0}, //{gPmbusCmd.FAN_COMMAND_2, sizeof(gPmbusCmd.FAN_COMMAND_2), 0xE3},
                                                                           /*0x3D*/
    {0, 0, 0}, //{gPmbusCmd.FAN_CONFIG_3_4, sizeof(gPmbusCmd.FAN_CONFIG_3_4), 0xB0},
                                                                           /*0x3E*/
    {0, 0, 0}, //{gPmbusCmd.FAN_COMMAND_3, sizeof(gPmbusCmd.FAN_COMMAND_1), 0xE3},
                                                                           /*0x3F*/
    {0, 0, 0}, //{gPmbusCmd.FAN_COMMAND_4, sizeof(gPmbusCmd.FAN_COMMAND_1), 0xE3},
                                                                           /*0x40*/
    {gPmbusCmd.VOUT_OV_FAULT_LIMIT, sizeof(gPmbusCmd.VOUT_OV_FAULT_LIMIT), QUERY_VOUT_OV_FAULT_LIMIT},
                                                                           /*0x41*/
    {gPmbusCmd.VOUT_OV_FAULT_RESPONSE, sizeof(gPmbusCmd.VOUT_OV_FAULT_RESPONSE), QUERY_VOUT_OV_FAULT_RESPONSE},
                                                                           /*0x42*/
    {gPmbusCmd.VOUT_OV_WARN_LIMIT, sizeof(gPmbusCmd.VOUT_OV_WARN_LIMIT), QUERY_VOUT_OV_WARN_LIMIT},
                                                                           /*0x43*/
    {gPmbusCmd.VOUT_UV_WARN_LIMIT, sizeof(gPmbusCmd.VOUT_UV_WARN_LIMIT), QUERY_VOUT_UV_WARN_LIMIT},
                                                                           /*0x44*/
    {gPmbusCmd.VOUT_UV_FAULT_LIMIT, sizeof(gPmbusCmd.VOUT_UV_FAULT_LIMIT), QUERY_VOUT_UV_FAULT_LIMIT},
                                                                           /*0x45*/
    {0, 0, 0},
                                                                           /*0x46*/
    {gPmbusCmd.IOUT_OC_FAULT_LIMIT, sizeof(gPmbusCmd.IOUT_OC_FAULT_LIMIT), QUERY_IOUT_OC_FAULT_LIMIT},
                                                                           /*0x47*/
    {gPmbusCmd.IOUT_OC_FAULT_RESPONSE, sizeof(gPmbusCmd.IOUT_OC_FAULT_RESPONSE), QUERY_IOUT_OC_FAULT_RESPONSE},
                                                                           /*0x48*/
    {0, 0, 0},
                                                                           /*0x49*/
    {0, 0, 0},
                                                                           /*0x4A*/
    {gPmbusCmd.IOUT_OC_WARN_LIMIT, sizeof(gPmbusCmd.IOUT_OC_WARN_LIMIT), QUERY_IOUT_OC_WARN_LIMIT},
                                                                           /*0x4B*/
    {0, 0, 0},
                                                                           /*0x4C*/
    {0, 0, 0},
                                                                           /*0x4D*/
    {0, 0, 0},
                                                                           /*0x4E*/
    {0, 0, 0},
                                                                           /*0x4F*/
    {gPmbusCmd.OT_FAULT_LIMIT, sizeof(gPmbusCmd.OT_FAULT_LIMIT), QUERY_OT_FAULT_LIMIT},
                                                                           /*0x50*/
    {gPmbusCmd.OT_FAULT_RESPONSE, sizeof(gPmbusCmd.OT_FAULT_RESPONSE), QUERY_OT_FAULT_RESPONSE},
                                                                           /*0x51*/
    {gPmbusCmd.OT_WARN_LIMIT, sizeof(gPmbusCmd.OT_WARN_LIMIT), QUERY_OT_WARN_LIMIT},
                                                                           /*0x52*/
    {0, 0, 0},
                                                                           /*0x53*/
    {0, 0, 0},
                                                                           /*0x54*/
    {0, 0, 0},
                                                                           /*0x55*/
    {gPmbusCmd.VIN_OV_FAULT_LIMIT, sizeof(gPmbusCmd.VIN_OV_FAULT_LIMIT), QUERY_VIN_OV_FAULT_LIMIT},
                                                                           /*0x56*/
    {gPmbusCmd.VIN_OV_FAULT_RESPONSE, sizeof(gPmbusCmd.VIN_OV_FAULT_RESPONSE), QUERY_VIN_OV_FAULT_RESPONSE},
                                                                           /*0x57*/
    {gPmbusCmd.VIN_OV_WARN_LIMIT, sizeof(gPmbusCmd.VIN_OV_WARN_LIMIT), QUERY_VIN_OV_WARN_LIMIT},
                                                                           /*0x58*/
    {gPmbusCmd.VIN_UV_WARN_LIMIT, sizeof(gPmbusCmd.VIN_UV_WARN_LIMIT), QUERY_VIN_UV_WARN_LIMIT},
                                                                           /*0x59*/
    {gPmbusCmd.VIN_UV_FAULT_LIMIT, sizeof(gPmbusCmd.VIN_UV_FAULT_LIMIT), QUERY_VIN_UV_FAULT_LIMIT},
                                                                           /*0x5A*/
    {gPmbusCmd.VIN_UV_FAULT_RESPONSE, sizeof(gPmbusCmd.VIN_UV_FAULT_RESPONSE), QUERY_VIN_UV_FAULT_RESPONSE},
                                                                           /*0x5B*/
    {gPmbusCmd.IIN_OC_FAULT_LIMIT, sizeof(gPmbusCmd.IIN_OC_FAULT_LIMIT), QUERY_IIN_OC_FAULT_LIMIT},
                                                                           /*0x5C*/
    {0, 0, 0},
                                                                           /*0x5D*/
    {gPmbusCmd.IIN_OC_WARN_LIMIT, sizeof(gPmbusCmd.IIN_OC_WARN_LIMIT), QUERY_IIN_OC_WARN_LIMIT},
                                                                           /*0x5E*/
    {0, 0, 0},
                                                                           /*0x5F*/
    {0, 0, 0},
                                                                           /*0x60*/
    {0, 0, 0},
                                                                           /*0x61*/
    {0, 0, 0},
                                                                           /*0x62*/
    {0, 0, 0},
                                                                           /*0x63*/
    {0, 0, 0},
                                                                           /*0x64*/
    {0, 0, 0},
                                                                           /*0x65*/
    {0, 0, 0},
                                                                           /*0x66*/
    {0, 0, 0},
                                                                           /*0x67*/
    {0, 0, 0},
                                                                           /*0x68*/
    {0, 0, 0},
                                                                           /*0x69*/
    {0, 0, 0},
                                                                           /*0x6A*/
    {gPmbusCmd.POUT_OP_WARN_LIMIT, sizeof(gPmbusCmd.POUT_OP_WARN_LIMIT), QUERY_POUT_OP_WARN_LIMIT},
                                                                           /*0x6B*/
    {gPmbusCmd.PIN_OP_WARN_LIMIT, sizeof(gPmbusCmd.PIN_OP_WARN_LIMIT), QUERY_PIN_OP_WARN_LIMIT},
                                                                           /*0x6C*/
    {0, 0, 0},
                                                                           /*0x6D*/
    {0, 0, 0},
                                                                           /*0x6E*/
    {0, 0, 0},
                                                                           /*0x6F*/
    {0, 0, 0},
                                                                           /*0x70*/
    {0, 0, 0}, //{gPmbusCmd.MFR_FW_VER_DATE, sizeof(gPmbusCmd.MFR_FW_VER_DATE), QUERY_MFR_FW_VER_DATE},
                                                                           /*0x71*/
    {0, 0, 0},
                                                                           /*0x72*/
    {0, 0, 0},
                                                                           /*0x73*/
    {0, 0, 0},
                                                                           /*0x74*/
    {0, 0, 0},
                                                                           /*0x75*/
    {0, 0, 0},
                                                                           /*0x76*/
    {0, 0, 0},
                                                                           /*0x77*/
    {gPmbusCmd.DEBUG_INFO, sizeof(gPmbusCmd.DEBUG_INFO), QUERY_DEBUG_INFO},
                                                                           /*0x78*/
    {0, 1, QUERY_STATUS_BYTE}, //STATUS_BYTE
                                                                           /*0x79*/
    {0, sizeof(gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.Val), QUERY_STATUS_WORD},
                                                                           /*0x7A*/
    {0, sizeof(gPagePlusStatus.PAGE[PAGE0].STATUS_VOUT.Val), QUERY_STATUS_VOUT},
                                                                           /*0x7B*/
    {0, sizeof(gPagePlusStatus.PAGE[PAGE0].STATUS_IOUT.Val), QUERY_STATUS_IOUT},
                                                                           /*0x7C*/
    {0, sizeof(gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.Val), QUERY_STATUS_INPUT},
                                                                           /*0x7D*/
    {0, sizeof(gPagePlusStatus.PAGE[PAGE0].STATUS_TEMPERATURE.Val), QUERY_STATUS_TEMPERATURE},
                                                                           /*0x7E*/
    {0, sizeof(gPagePlusStatus.PAGE[PAGE0].STATUS_CML.Val), QUERY_STATUS_CML},
                                                                           /*0x7F*/
    {0, 0, 0}, //{(BYTE*)&gPmbusCmd.STATUS_OTHER.Val, sizeof(gPmbusCmd.STATUS_OTHER.Val), QUERY_STATUS_OTHER},
                                                                           /*0x80*/
    {0, 0, 0},
                                                                           /*0x81*/
    {(BYTE*) & gPmbusCmd.STATUS_FAN_1_2.Val, sizeof(gPmbusCmd.STATUS_FAN_1_2.Val), QUERY_STATUS_FAN_1_2},
                                                                           /*0x82*/
    {0, 0, 0},
                                                                           /*0x83*/
    {0, 0, 0},
                                                                           /*0x84*/
    {0, 0, 0},
                                                                           /*0x85*/
    {0, 0, 0},
                                                                           /*0x86*/
    {gPmbusCmd.READ_EIN, sizeof(gPmbusCmd.READ_EIN), QUERY_READ_EIN},
                                                                           /*0x87*/
    {gPmbusCmd.READ_EOUT, sizeof(gPmbusCmd.READ_EOUT), QUERY_READ_EOUT},
                                                                           /*0x88*/
    {gPmbusCmd.READ_VIN, sizeof(gPmbusCmd.READ_VIN), QUERY_READ_VIN},
                                                                           /*0x89*/
    {gPmbusCmd.READ_IIN, sizeof(gPmbusCmd.READ_IIN), QUERY_READ_IIN},
                                                                           /*0x8A*/
    {0, 0, 0},
                                                                           /*0x8B*/
    {gPmbusCmd.READ_VOUT, sizeof(gPmbusCmd.READ_VOUT), QUERY_READ_VOUT},
                                                                           /*0x8C*/
    {gPmbusCmd.READ_IOUT, sizeof(gPmbusCmd.READ_IOUT), QUERY_READ_IOUT},
                                                                           /*0x8D*/
    {gPmbusCmd.READ_TEMPERATURE_1, sizeof(gPmbusCmd.READ_TEMPERATURE_1), QUERY_READ_TEMPERATURE1},
                                                                           /*0x8E*/
    {gPmbusCmd.READ_TEMPERATURE_2, sizeof(gPmbusCmd.READ_TEMPERATURE_2), QUERY_READ_TEMPERATURE2},
                                                                           /*0x8F*/
    {gPmbusCmd.READ_TEMPERATURE_3, sizeof(gPmbusCmd.READ_TEMPERATURE_3), QUERY_READ_TEMPERATURE3},
                                                                           /*0x90*/
    {gPmbusCmd.READ_FAN_SPEED_1, sizeof(gPmbusCmd.READ_FAN_SPEED_1), QUERY_READ_FAN_SPEED1},
                                                                           /*0x91*/
    {gPmbusCmd.READ_FAN_SPEED_2, sizeof(gPmbusCmd.READ_FAN_SPEED_2), QUERY_READ_FAN_SPEED2},
                                                                           /*0x92*/
    {0, 0, 0},
                                                                           /*0x93*/
    {0, 0, 0},
                                                                           /*0x94*/
    {0, 0, 0},
                                                                           /*0x95*/
    {gPmbusCmd.READ_FREQUENCY, sizeof(gPmbusCmd.READ_FREQUENCY), QUERY_READ_FREQUENCY},
                                                                           /*0x96*/
    {gPmbusCmd.READ_POUT, sizeof(gPmbusCmd.READ_POUT), QUERY_READ_POUT},
                                                                           /*0x97*/
    {gPmbusCmd.READ_PIN, sizeof(gPmbusCmd.READ_PIN), QUERY_READ_PIN},
                                                                           /*0x98*/
    {gPmbusCmd.PMBUS_REVISION, sizeof(gPmbusCmd.PMBUS_REVISION), QUERY_PMBUS_REVSION},
                                                                           /*0x99*/
    {gPmbusCmd.MFR_ID, sizeof(gPmbusCmd.MFR_ID), QUERY_MFR_ID}, //MFR_ID
                                                                           /*0x9A*/
    {gPmbusCmd.MFR_MODEL, sizeof(gPmbusCmd.MFR_MODEL), QUERY_MFR_MODEL}, //MFR_MODEL
                                                                           /*0x9B*/
    {gPmbusCmd.MFR_REVISION, sizeof(gPmbusCmd.MFR_REVISION), QUERY_MFR_REVSION},
                                                                           /*0x9C*/
    {gPmbusCmd.MFR_LOCATION, sizeof(gPmbusCmd.MFR_LOCATION), QUERY_MFR_LOCATION},
                                                                           /*0x9D*/
    {gPmbusCmd.MFR_DATE, sizeof(gPmbusCmd.MFR_DATE), QUERY_MFR_DATE},
                                                                           /*0x9E*/
    {gPmbusCmd.MFR_SERIAL, sizeof(gPmbusCmd.MFR_SERIAL), QUERY_MFR_SERIAL}, //MFR_SERIAL
                                                                           /*0x9F*/
    {0, 0, 0},
                                                                           /*0xA0*/
    {gPmbusCmd.MFR_VIN_MIN, sizeof(gPmbusCmd.MFR_VIN_MIN), QUERY_MFR_VIN_MIN},
                                                                           /*0xA1*/
    {gPmbusCmd.MFR_VIN_MAX, sizeof(gPmbusCmd.MFR_VIN_MAX), QUERY_MFR_VIN_MAX},
                                                                           /*0xA2*/
    {gPmbusCmd.MFR_IIN_MAX, sizeof(gPmbusCmd.MFR_IIN_MAX), QUERY_MFR_IIN_MAX},
                                                                           /*0xA3*/
    {gPmbusCmd.MFR_PIN_MAX, sizeof(gPmbusCmd.MFR_PIN_MAX), QUERY_MFR_PIN_MAX},
                                                                           /*0xA4*/
    {gPmbusCmd.MFR_VOUT_MIN, sizeof(gPmbusCmd.MFR_VOUT_MIN), QUERY_MFR_VOUT_MIN},
                                                                           /*0xA5*/
    {gPmbusCmd.MFR_VOUT_MAX, sizeof(gPmbusCmd.MFR_VOUT_MAX), QUERY_MFR_VOUT_MAX},
                                                                           /*0xA6*/
    {gPmbusCmd.MFR_IOUT_MAX, sizeof(gPmbusCmd.MFR_IOUT_MAX), QUERY_MFR_IOUT_MAX},
                                                                           /*0xA7*/
    {gPmbusCmd.MFR_POUT_MAX, sizeof(gPmbusCmd.MFR_POUT_MAX), QUERY_MFR_POUT_MAX},
                                                                           /*0xA8*/
    {gPmbusCmd.MFR_TAMBIENT_MAX, sizeof(gPmbusCmd.MFR_TAMBIENT_MAX), QUERY_MFR_TAMBIENT_MAX},
                                                                           /*0xA9*/
    {gPmbusCmd.MFR_TAMBIENT_MIN, sizeof(gPmbusCmd.MFR_TAMBIENT_MIN), QUERY_MFR_TAMBIENT_MIN},
                                                                           /*0xAA*/
    {0, 0, 0}, //{gPmbusCmd.MFR_EFFICIENCY_LL, sizeof(gPmbusCmd.MFR_EFFICIENCY_LL), QUERY_MFR_EFFICIENCY_LL},
                                                                           /*0xAB*/
    {gPmbusCmd.MFR_EFFICIENCY_HL, sizeof(gPmbusCmd.MFR_EFFICIENCY_HL), QUERY_MFR_EFFICIENCY_HL},
                                                                           /*0xAC*/
    {0, 0, 0},
                                                                           /*0xAD*/
    {0, 0, 0},
                                                                           /*0xAE*/
    {0, 0, 0},
                                                                           /*0xAF*/
    {0, 0, 0},
                                                                           /*0xB0*/
    {gPmbusCmd.FRU_WRITE, sizeof(gPmbusCmd.FRU_WRITE), QUERY_FRU_DATA_OFFSET},
                                                                           /*0xB1*/
    {gPmbusCmd.FRU_READ, sizeof(gPmbusCmd.FRU_READ), QUERY_READ_FRU_DATA},
                                                                           /*0xB2*/
    {gPmbusCmd.FRU_FLASH_WRITE, sizeof(gPmbusCmd.FRU_FLASH_WRITE), QUERY_FRU_FLASH_WRITE}, //Created by Liteon
                                                                           /*0xB3*/
    {gPmbusCmd.FRU_FLASH_ERASE, sizeof(gPmbusCmd.FRU_FLASH_ERASE), QUERY_FRU_FLASH_ERASE}, //Created by Liteon
                                                                           /*0xB4*/
    {gPmbusCmd.MFR_FIRMWARE_VER, sizeof(gPmbusCmd.MFR_FIRMWARE_VER), QUERY_MFR_FW_VER}, //Created by Liteon
                                                                           /*0xB5*/
    {0, 0, 0}, //{gPmbusCmd.MFR_IOUT_CAL, sizeof(gPmbusCmd.MFR_IOUT_CAL)},
                                                                           /*0xB6*/
    {0, 0, 0}, //{gPmbusCmd.MFR_IIN_CAL, sizeof(gPmbusCmd.MFR_IIN_CAL)},
                                                                           /*0xB7*/
    {0, 0, 0},
                                                                           /*0xB8*/
    {0, 0, 0},
                                                                           /*0xB9*/
    {gPmbusCmd.FRU_SLICE_BUF, sizeof(gPmbusCmd.FRU_SLICE_BUF), QUERY_FULL_FRU_FLASH_WRITE}, //cmd for updating FRU data //Created by Liteon
                                                                           /*0xBA*/
    {gPmbusCmd.CAL_VOUT_OFFSET, sizeof(gPmbusCmd.CAL_VOUT_OFFSET), QUERY_CAL_VOUT}, //Created by Liteon
                                                                           /*0xBB*/
    {gPmbusCmd.CAL_TERIDIAN_IC, sizeof(gPmbusCmd.CAL_TERIDIAN_IC), QUERY_CAL_TERIDIAN_IC}, //Created by Liteon
                                                                           /*0xBC*/
    {gPmbusCmd.CAL_TERIDIAN_FINISHED_NOTIFY, sizeof(gPmbusCmd.CAL_TERIDIAN_FINISHED_NOTIFY), QUERY_CAL_TERIDIAN_FINISHED_NOTIFY}, //Created by Liteon
                                                                           /*0xBD*/
    {gPmbusCmd.GEN_CAL_W, sizeof(gPmbusCmd.GEN_CAL_W), QUERY_GEN_CAL_W},
                                                                           /*0xBE*/
    {gPmbusCmd.GEN_CAL_R, sizeof(gPmbusCmd.GEN_CAL_R), QUERY_GEN_CAL_R},
                                                                           /*0xBF*/
    {gPmbusCmd.CAL_TERIDIAN_GET_FW_VER, sizeof(gPmbusCmd.CAL_TERIDIAN_GET_FW_VER), QUERY_CAL_TERIDIAN_GET_FW_VER},
                                                                           /*0xC0*/
    {gPmbusCmd.MFR_MAX_TEMP_1, sizeof(gPmbusCmd.MFR_MAX_TEMP_1), QUERY_MFR_MAX_TEMP_1},
                                                                           /*0xC1*/
    {gPmbusCmd.MFR_MAX_TEMP_2, sizeof(gPmbusCmd.MFR_MAX_TEMP_2), QUERY_MFR_MAX_TEMP_2},
                                                                           /*0xC2*/
    {0, 0, 0},
                                                                            /*0xC3*/
    {0, 0, 0},
                                                                           /*0xC4*/
    {0, 0, 0},
                                                                           /*0xC5*/
    {0, 0, 0},
                                                                           /*0xC6*/
    {0, 0, 0},
                                                                           /*0xC7*/
    {0, 0, 0},
                                                                           /*0xC8*/
    {0, 0, 0},
                                                                           /*0xC9*/
    {gPmbusCmd.MFR_ISP_KEY, sizeof(gPmbusCmd.MFR_ISP_KEY), QUERY_MFR_ISP_KEY}, //{0, 0, 0},	//for Lenovo AP
                                                                           /*0xCA*/
    {0, 0, 0}, //{gPmbusCmd.ADE_VIN, sizeof(gPmbusCmd.ADE_VIN), QUERY_ADE_VIN},
                                                                           /*0xCB*/
    {0, 0, 0}, //{gPmbusCmd.ADE_IIN, sizeof(gPmbusCmd.ADE_IIN), QUERY_ADE_IIN},
                                                                           /*0xCC*/
    {0, 0, 0}, //{gPmbusCmd.ADE_PIN, sizeof(gPmbusCmd.ADE_PIN), QUERY_ADE_PIN},
                                                                           /*0xCD*/
    {0, 0, 0},
                                                                           /*0xCE*/
    {0, 0, 0},
                                                                           /*0xCF*/
    {gPmbusCmd.MFR_RCB_INFO, sizeof(gPmbusCmd.MFR_RCB_INFO), QUERY_RCB_INFO}, //[davidchchen]20160304 added RCB
                                                                           /*0xD0*/
    {gPmbusCmd.MFR_FW_VERSION, sizeof(gPmbusCmd.MFR_FW_VERSION), QUERY_FW_VERSION}, //{gPmbusCmd.SMART_ON_CONFIG, sizeof(gPmbusCmd.SMART_ON_CONFIG), QUERY_SMART_ON_CONFIG},//{gPmbusCmd.MFR_DEVICE_CODE, sizeof(gPmbusCmd.MFR_DEVICE_CODE), QUERY_MFR_DEVICE_CODE},
                                                                           /*0xD1*/
    {gPmbusCmd.PSU_MODEL_NUM, sizeof(gPmbusCmd.PSU_MODEL_NUM), QUERY_PSU_MODEL_NUM}, //{gPmbusCmd.MFR_ISP_KEY, sizeof(gPmbusCmd.MFR_ISP_KEY), QUERY_MFR_ISP_KEY},
                                                                           /*0xD2*/
    {gPmbusCmd.MFR_ISP_STATUS_CMD, sizeof(gPmbusCmd.MFR_ISP_STATUS_CMD), QUERY_MFR_ISP_STATUS_CMD},
                                                                           /*0xD3*/
    {0, 0, 0}, //{gPmbusCmd.MFR_ISP_MEMORY_ADDR, sizeof(gPmbusCmd.MFR_ISP_MEMORY_ADDR), QUERY_MFR_ISP_MEMORY_ADDR},
                                                                           /*0xD4*/
    {gPmbusCmd.VIN_MODE, sizeof(gPmbusCmd.VIN_MODE), QUERY_VIN_MODE}, //{gPmbusCmd.MFR_ISP_MEMORY, sizeof(gPmbusCmd.MFR_ISP_MEMORY), QUERY_MFR_ISP_MEMORY},
                                                                           /*0xD5*/
    {gPmbusCmd.IIN_MODE, sizeof(gPmbusCmd.IIN_MODE), QUERY_IIN_MODE},
                                                                           /*0xD6*/
    {gPmbusCmd.PIN_MODE, sizeof(gPmbusCmd.PIN_MODE), QUERY_PIN_MODE},
                                                                           /*0xD7*/
    {gPmbusCmd.IOUT_MODE, sizeof(gPmbusCmd.IOUT_MODE), QUERY_IOUT_MODE}, //{gPmbusCmd.MFR_SYSTEM_LED_CNTL, sizeof(gPmbusCmd.MFR_SYSTEM_LED_CNTL), QUERY_SYSTEM_LED_CNTL},
                                                                           /*0xD8*/
    {gPmbusCmd.POUT_MODE, sizeof(gPmbusCmd.POUT_MODE), QUERY_POUT_MODE}, //{(BYTE*)gPmbusCmd.MFR_LINE_STATUS, sizeof(gPmbusCmd.MFR_LINE_STATUS[0].Val), QUERY_LINE_STATUS},
                                                                           /*0xD9*/
    {0, 0, 0}, //{gPmbusCmd.MFR_SMB_ALERT_MASKING, sizeof(gPmbusCmd.MFR_SMB_ALERT_MASKING), QUERY_MFR_SMBALERT_MASK},
                                                                           /*0xDA*/
    {0, 0, 0}, //{gPmbusCmd.MFR_TOT_POUT_MAX, sizeof(gPmbusCmd.MFR_TOT_POUT_MAX), QUERY_TOT_MFR_POUT_MAX},
                                                                           /*0xDB*/
    {0, 0, 0},
                                                                           /*0xDC*/
    {0, 0, 0},
                                                                           /*0xDD*/
    {gPmbusCmd.LOG_GENERAL, sizeof(gPmbusCmd.LOG_GENERAL), QUERY_LOG_GENERAL},
                                                                           /*0xDE*/
    {gPmbusCmd.LOG_INDEX, sizeof(gPmbusCmd.LOG_INDEX), QUERY_LOG_INDEX},
                                                                           /*0xDF*/
    {gPmbusCmd.LOG_CONTENT, sizeof(gPmbusCmd.LOG_CONTENT), QUERY_LOG_CONTENT},
                                                                           /*0xE0*/
    //{0, 0, 0}, //{(BYTE*)gPmbusCmd.MFR_ROA_CONTROL, sizeof(gPmbusCmd.MFR_ROA_CONTROL[0].Val), QUERY_MFR_ROA_CONTROL},
    //[davidchchen]20170216 added PSON Signal Enable/disable
    {gPmbusCmd.MFR_PSON_CONTROL, sizeof(gPmbusCmd.MFR_PSON_CONTROL), QUERY_MFR_PSON_CONTROL},
                                                                           /*0xE1*/
    {0, 0, 0}, //{gPmbusCmd.MFR_SLEEP_TRIP, sizeof(gPmbusCmd.MFR_SLEEP_TRIP), QUERY_MFR_SLEEP_TRIP},
                                                                           /*0xE2*/
    {0, 0, 0}, //{gPmbusCmd.MFR_WAKE_TRIP, sizeof(gPmbusCmd.MFR_WAKE_TRIP), QUERY_MFR_WAKE_TRIP},
                                                                           /*0xE3*/
    {0, 0, 0}, //{gPmbusCmd.MFR_TRIP_LATENCY, sizeof(gPmbusCmd.MFR_TRIP_LATENCY), QUERY_MFR_TRIP_LATENCY},
                                                                           /*0xE4*/
    {gPmbusCmd.MFR_PAGE, sizeof(gPmbusCmd.MFR_PAGE), QUERY_MFR_PAGE},
//                                                                           /*0xE5*/               //[davidchchen]20170418 Removed
//    {&gPmbusCmd.MFR_POS_TOTAL.v[0], sizeof(gPmbusCmd.MFR_POS_TOTAL), QUERY_MFR_POS_TOTAL},        //[davidchchen]20170418 Removed
                                                                            /*0xE5*/                //[davidchchen]20170418 Added Black Box block read
    {gPmbusCmd.MFR_POS_TOTAL, sizeof(gPmbusCmd.MFR_POS_TOTAL), QUERY_MFR_POS_TOTAL},                //[davidchchen]20170418 Added Black Box block read
//                                                                           /*0xE6*/               //[davidchchen]20170418 Removed
//    {&gPmbusCmd.MFR_POS_LAST.v[0], sizeof(gPmbusCmd.MFR_POS_LAST), QUERY_MFR_POS_LAST},           //[davidchchen]20170418 Removed
                                                                            /*0xE6*/                //[davidchchen]20170418 Added Black Box block read
    {gPmbusCmd.MFR_POS_LAST, sizeof(gPmbusCmd.MFR_POS_LAST), QUERY_MFR_POS_LAST},                   //[davidchchen]20170418 Added Black Box block read
                                                                           /*0xE7*/         //[davidchchen]20170310 Removed MFR_CLEAR_HISTORY
    //{0, 0, QUERY_CLEAR_HISTORY}, /*There is no data for CLEAR_HISTORY command*/           //[davidchchen]20170310 Removed MFR_CLEAR_HISTORY
//                                                                           /*0xE7*/
//    {&gPmbusCmd.BMC_UNIX_TIMESTAMP.v[0], sizeof(gPmbusCmd.BMC_UNIX_TIMESTAMP), QUERY_BMC_UNIX_TIMESTAMP}, //[davidchchen]20170418 Removed
                                                                            /*0xE7*/
    {gPmbusCmd.BMC_UNIX_TIMESTAMP, sizeof(gPmbusCmd.BMC_UNIX_TIMESTAMP), QUERY_BMC_UNIX_TIMESTAMP}, //[davidchchen]20170418 Added Black Box block read
                                                                           /*0xE8*/
    {0, 0, 0}, //{&gPmbusCmd.MFR_PFC_DISABLE.Val, sizeof(gPmbusCmd.MFR_PFC_DISABLE.Val), QUERY_PFC_DISABLE},
                                                                           /*0xE9*/
    {0, 0, 0}, //{gPmbusCmd.MFR_VRAPID_ON_SET, sizeof(gPmbusCmd.MFR_VRAPID_ON_SET), QUERY_VRAPID_ON_SET},
                                                                           /*0xEA*/
    {0, 0, 0}, //{gPmbusCmd.MFR_VDETECT, sizeof(gPmbusCmd.MFR_VDETECT), QUERY_VDETECT},
                                                                           /*0xEB*/
    {0, 0, 0}, //{(BYTE*)&gPmbusCmd.PSU_FEATURES, sizeof(gPmbusCmd.PSU_FEATURES[0].Val), QUERY_PSU_FEATURES},
                                                                           /*0xEC*/
    {0, 0, 0}, //{0, 160, QUERY_MFR_SAMPLE_SET},
                                                                           /*0xED*/
    {gPmbusCmd.PID_LOG, sizeof(gPmbusCmd.PID_LOG), QUERY_PID_LOG},
                                                                           /*0xEE*/
    {gPmbusCmd.PRI_DEBUG, sizeof(gPmbusCmd.PRI_DEBUG), QUERY_PRI_DEBUG},
                                                                           /*0xEF*/
    {0, 0, 0},
                                                                           /*0xF0*/
    {(BYTE*) & gPmbusCmd.PSU_FACTORY_MODE, sizeof(gPmbusCmd.PSU_FACTORY_MODE[0].Val), QUERY_PSU_FACTORY_MODE},
                                                                           /*0xF1*/
    {gPmbusCmd.READ_ALL_CALI_INFO, sizeof(gPmbusCmd.READ_ALL_CALI_INFO), QUERY_READ_ALL_CALI_INFO},
                                                                           /*0xF2*/
    {gPmbusCmd.DISABLE_FUNC, sizeof(gPmbusCmd.DISABLE_FUNC), QUERY_DISABLE_FUNC},
                                                                           /*0xF3*/
    {gPmbusCmd.CS_PWM_FB, sizeof(gPmbusCmd.CS_PWM_FB), QUERY_CS_PWM_FB},
                                                                           /*0xF4*/
    {gPmbusCmd.CS_IN, sizeof(gPmbusCmd.CS_IN), QUERY_CS_IN},

                                                                           /*0xF5*/
    {gPmbusCmd.READ_IOUT_LS, sizeof(gPmbusCmd.READ_IOUT_LS), QUERY_READ_IOUT_LS},
                                                                           /*0xF6*/
    {gPmbusCmd.READ_IOUT_SS, sizeof(gPmbusCmd.READ_IOUT_SS), QUERY_READ_IOUT_SS},
                                                                           /*0xF7*/
    {gPmbusCmd.READ_IOUT_CS, sizeof(gPmbusCmd.READ_IOUT_CS), QUERY_READ_IOUT_CS},
                                                                           /*0xF8*/
    {gPmbusCmd.SCI, sizeof(gPmbusCmd.SCI), QUERY_SCI},

                                                                           /*0xF9*/
    {gPmbusCmd.PRI_STATUS, sizeof(gPmbusCmd.PRI_STATUS), QUERY_PRI_STATUS},
                                                                           /*0xFA*/
    {gPmbusCmd.PRI_FW_VER_INTERNAL, sizeof(gPmbusCmd.PRI_FW_VER_INTERNAL), QUERY_PRI_FW_VER_INTERNAL},
                                                                           /*0xFB*/
    {gPmbusCmd.READ_IOUT_LS_2, sizeof(gPmbusCmd.READ_IOUT_LS_2), QUERY_READ_IOUT_LS_2},
                                                                           /*0xFC*/
    {gPmbusCmd.CALI_INFO, sizeof(gPmbusCmd.CALI_INFO), QUERY_CALI_INFO},

                                                                           /*0xFD*/
    {gPmbusCmd.READ_TEMPERATURE_4, sizeof(gPmbusCmd.READ_TEMPERATURE_4), QUERY_READ_TEMPERATURE_4},
                                                                           /*0xFE*/
    {gPmbusCmd.PID_P, sizeof(gPmbusCmd.PID_P), QUERY_PID_P},
                                                                           /*0xFF*/
    {0, 0, 0}
};

static inline void UpdateDataBuf()
{
    if(gCmd[I2C.currentCmd].Len > 0)
    {
        memcpy((BYTE*) gCmd[I2C.currentCmd].pBuf, (BYTE*) I2C.readBuffer, gCmd[I2C.currentCmd].Len);
    }
}

static void CMD_PAGE_Handler()
{
    WORD result;

    //[Peter Chung] 20101223 modified, page only accept 0x00, 0x01
    if(I2C.readBuffer[0] <= 1)
    {
        static BYTE page;
        //static BYTE lastPage = 0;
        page = I2C.readBuffer[0];

        UpdateDataBuf();
        //Refresh Status regarding to PAGE_PLUS
        gPmbusCmd.STATUS_WORD = & (gPagePlusStatus.PAGE[page].STATUS_WORD);
        gPmbusCmd.STATUS_VOUT = & (gPagePlusStatus.PAGE[page].STATUS_VOUT);
        gPmbusCmd.STATUS_INPUT = & (gPagePlusStatus.PAGE[page].STATUS_INPUT);
        gPmbusCmd.STATUS_TEMPERATURE = & (gPagePlusStatus.PAGE[page].STATUS_TEMPERATURE);
        gPmbusCmd.STATUS_IOUT = & (gPagePlusStatus.PAGE[page].STATUS_IOUT);
        gPmbusCmd.STATUS_CML = & (gPagePlusStatus.PAGE[page].STATUS_CML);

        gCmd[0x78].pBuf = (BYTE*) (gPmbusCmd.STATUS_WORD); //STATUS_BYTE
        gCmd[0x79].pBuf = (BYTE*) (gPmbusCmd.STATUS_WORD); //STATUS_WORD
        gCmd[0x7A].pBuf = (BYTE*) (gPmbusCmd.STATUS_VOUT);
        gCmd[0x7C].pBuf = (BYTE*) (gPmbusCmd.STATUS_INPUT);
        gCmd[0x7D].pBuf = (BYTE*) (gPmbusCmd.STATUS_TEMPERATURE);
        gCmd[0x7B].pBuf = (BYTE*) (gPmbusCmd.STATUS_IOUT);
        gCmd[0x7E].pBuf = (BYTE*) (gPmbusCmd.STATUS_CML);

#        if 0
        if(lastPage != page)
        {
            if(page == 0)
            {
                Parameter.VIN_OV_WARN_LIMIT[0] = DEFAULT_VIN_OV_WARN_AC;
                Parameter.VIN_OV_FAULT_LIMIT[0] = DEFAULT_VIN_OV_FAULT_AC;
                Parameter.VIN_OV_FAULT_RECOVER_LIMIT[0] = DEFAULT_VIN_OV_RECOVER_AC;
                Parameter.VIN_UV_WARN_LIMIT[0] = DEFAULT_VIN_UV_WARN_AC;
                Parameter.VIN_UV_FAULT_LIMIT[0] = DEFAULT_VIN_UV_FAULT_AC;
                Parameter.VIN_UV_FAULT_RECOVER_LIMIT[0] = DEFAULT_VIN_UV_RECOVER_AC;
                Parameter.MFR_VIN_MAX = DEFAULT_MFR_VIN_MAX_AC;
                Parameter.MFR_VIN_MIN = DEFAULT_MFR_VIN_MIN_AC;
            }
            else
            {
                Parameter.VIN_OV_WARN_LIMIT[1] = DEFAULT_VIN_OV_WARN_DC;
                Parameter.VIN_OV_FAULT_LIMIT[1] = DEFAULT_VIN_OV_FAULT_DC;
                Parameter.VIN_OV_FAULT_RECOVER_LIMIT[1] = DEFAULT_VIN_OV_RECOVER_DC;
                Parameter.VIN_UV_WARN_LIMIT[1] = DEFAULT_VIN_UV_WARN_DC;
                Parameter.VIN_UV_FAULT_LIMIT[1] = DEFAULT_VIN_UV_FAULT_DC;
                Parameter.VIN_UV_FAULT_RECOVER_LIMIT[1] = DEFAULT_VIN_UV_RECOVER_DC;
                Parameter.MFR_VIN_MAX = DEFAULT_MFR_VIN_MAX_DC;
                Parameter.MFR_VIN_MIN = DEFAULT_MFR_VIN_MIN_DC;
            }
            lastPage = page;
        }
#        endif

        if(page == 0)
        {
            memcpy(gPmbusCmd.OT_FAULT_LIMIT, (BYTE*) & Parameter.OTP_Tsec_FAULT_LIMIT, 2);
            memcpy(gPmbusCmd.OT_WARN_LIMIT, (BYTE*) & Parameter.OTP_Tsec_WARN_LIMIT, 2);

#            if 0
            gPmbusCmd.VIN_OV_FAULT_LIMIT[0] = 0x3A;
            gPmbusCmd.VIN_OV_FAULT_LIMIT[1] = 0xFA;
            gPmbusCmd.VIN_OV_WARN_LIMIT[0] = 0x26;
            gPmbusCmd.VIN_OV_WARN_LIMIT[1] = 0xFA;
            gPmbusCmd.VIN_UV_FAULT_LIMIT[0] = 0x54;
            gPmbusCmd.VIN_UV_FAULT_LIMIT[1] = 0xF9;
            gPmbusCmd.VIN_UV_WARN_LIMIT[0] = 0x5E;
            gPmbusCmd.VIN_UV_WARN_LIMIT[1] = 0xF9;
#            endif

            result = LinearFmt_XtoY(Parameter.VIN_OV_FAULT_LIMIT[0], - 1, 0);
            memcpy(gPmbusCmd.VIN_OV_FAULT_LIMIT, &result, 2);

            result = LinearFmt_XtoY(Parameter.VIN_OV_WARN_LIMIT[0], - 1, 0);
            memcpy(gPmbusCmd.VIN_OV_WARN_LIMIT, &result, 2);

            result = LinearFmt_XtoY(Parameter.VIN_UV_FAULT_LIMIT[0], - 1, 0);
            memcpy(gPmbusCmd.VIN_UV_FAULT_LIMIT, &result, 2);

            result = LinearFmt_XtoY(Parameter.VIN_UV_WARN_LIMIT[0], - 1, 0);
            memcpy(gPmbusCmd.VIN_UV_WARN_LIMIT, &result, 2);

            result = LinearFmt_XtoY(DEFAULT_MFR_VIN_MAX_AC, N_MFR_VIN_MAX, 0); //N = 0
            memcpy(gPmbusCmd.MFR_VIN_MAX, &result, 2);

            result = LinearFmt_XtoY(DEFAULT_MFR_VIN_MIN_AC, N_MFR_VIN_MIN, 0); //N = 0
            memcpy(gPmbusCmd.MFR_VIN_MIN, &result, 2);

            result = LinearFmt_XtoY(DEFAULT_MFR_IIN_HL_MAX_AC, N_MFR_IIN_HL_MAX, 7); //N = -7
            memcpy(gPmbusCmd.MFR_IIN_MAX, &result, 2);

            result = LinearFmt_XtoY(DEFAULT_MFR_PIN_HL_MAX_AC, N_MFR_PIN_HL_MAX, 0); //N = 1
            memcpy(gPmbusCmd.MFR_PIN_MAX, &result, 2);

            Parameter.MFR_VIN_MAX = DEFAULT_MFR_VIN_MAX_AC;
            Parameter.MFR_VIN_MIN = DEFAULT_MFR_VIN_MIN_AC;


        }
        else
        { // page == 1
            memcpy(gPmbusCmd.OT_FAULT_LIMIT, (BYTE*) & Parameter.OTP_Tinlet_FAULT_LIMIT, 2);
            memcpy(gPmbusCmd.OT_WARN_LIMIT, (BYTE*) & Parameter.OTP_Tinlet_WARN_LIMIT, 2);

#            if 0
            gPmbusCmd.VIN_OV_FAULT_LIMIT[0] = 0x62;
            gPmbusCmd.VIN_OV_FAULT_LIMIT[1] = 0xFA;
            gPmbusCmd.VIN_OV_WARN_LIMIT[0] = 0x58;
            gPmbusCmd.VIN_OV_WARN_LIMIT[1] = 0xFA;
            gPmbusCmd.VIN_UV_FAULT_LIMIT[0] = 0x68;
            gPmbusCmd.VIN_UV_FAULT_LIMIT[1] = 0xF9;
            gPmbusCmd.VIN_UV_WARN_LIMIT[0] = 0x72;
            gPmbusCmd.VIN_UV_WARN_LIMIT[1] = 0xF9;
#            endif

            result = LinearFmt_XtoY(Parameter.VIN_OV_FAULT_LIMIT[1], - 1, 0);
            memcpy(gPmbusCmd.VIN_OV_FAULT_LIMIT, &result, 2);

            result = LinearFmt_XtoY(Parameter.VIN_OV_WARN_LIMIT[1], - 1, 0);
            memcpy(gPmbusCmd.VIN_OV_WARN_LIMIT, &result, 2);

            result = LinearFmt_XtoY(Parameter.VIN_UV_FAULT_LIMIT[1], - 1, 0);
            memcpy(gPmbusCmd.VIN_UV_FAULT_LIMIT, &result, 2);

            result = LinearFmt_XtoY(Parameter.VIN_UV_WARN_LIMIT[1], - 1, 0);
            memcpy(gPmbusCmd.VIN_UV_WARN_LIMIT, &result, 2);

            result = LinearFmt_XtoY(DEFAULT_MFR_VIN_MAX_DC, N_MFR_VIN_MAX, 0); //N = 0
            memcpy(gPmbusCmd.MFR_VIN_MAX, &result, 2);

            result = LinearFmt_XtoY(DEFAULT_MFR_VIN_MIN_DC, N_MFR_VIN_MIN, 0); //N = 0
            memcpy(gPmbusCmd.MFR_VIN_MIN, &result, 2);

            result = LinearFmt_XtoY(DEFAULT_MFR_IIN_HL_MAX_DC, N_MFR_IIN_HL_MAX, 7); //N = -7
            memcpy(gPmbusCmd.MFR_IIN_MAX, &result, 2);

            result = LinearFmt_XtoY(DEFAULT_MFR_PIN_HL_MAX_DC, N_MFR_PIN_HL_MAX, 0); //N = 1
            memcpy(gPmbusCmd.MFR_PIN_MAX, &result, 2);

            Parameter.MFR_VIN_MAX = DEFAULT_MFR_VIN_MAX_DC;
            Parameter.MFR_VIN_MIN = DEFAULT_MFR_VIN_MIN_DC;
        }

    }
    else
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
    }
}

void IsOperationOn(BYTE data, BYTE* isOn, BYTE* MarginState)
{
    BYTE_VAL op;
    BYTE result;

    op.Val = data;
    result = op.Val & 0b11000000;

    if(result == 0x00 || result == 0x40)
    {
        *isOn = FALSE;
        *MarginState = MARGIN_NA;
    }
    else if(result == 0x80)
    {
        result = op.Val & 0b11110000;
        if(result == 0x80)
        {
            *isOn = TRUE;
            *MarginState = MARGIN_OFF;
        }
        else
        {
            result = op.Val & 0b11111100;
            if(result == 0x94)
            {
                *isOn = TRUE;
                *MarginState = MARGIN_LOW_IGNORE_FAULT;
            }
            else if(result == 0x98)
            {
                *isOn = TRUE;
                *MarginState = MARGIN_LOW_ACT_ON_FAULT;
            }
            else if(result == 0xA4)
            {
                *isOn = TRUE;
                *MarginState = MARGIN_HIGH_IGNORE_FAULT;
            }
            else if(result == 0xA8)
            {
                *isOn = TRUE;
                *MarginState = MARGIN_HIGH_ACT_ON_FAULT;
            }
            else
            {
                *isOn = 0xFF; //Invalid data
                *MarginState = MARGIN_NA;
            }
        }
    }
    else
    {
        *isOn = 0xFF; //Invalid data
        *MarginState = MARGIN_NA;
    }
}

static void CMD_OPERATION_Handler()
{
    IsOperationOn(I2C.readBuffer[0], &isOpON, &marginState);

    if(isOpON != 0xFF)
    {
        UpdateDataBuf();
        PS.op_cmdChanged = TRUE;
    }
    else
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
    }
}

static void CMD_POUT_OP_WARN_LIMIT_Handler()
{
    DWORD result;

    result = ((WORD) I2C.readBuffer[0] | ((WORD) I2C.readBuffer[1] << 8));
    result = LinearFmt_YtoX((WORD) result, 0);

    UpdateDataBuf();

    Parameter.OPP_POUT_OP_WARN_LIMIT = result;

}

static void CMD_IIN_OC_WARN_Handler() //Peter modified for PMBus issue
{
    DWORD result;
    WORD amp;

    result = ((WORD) I2C.readBuffer[0] | ((WORD) I2C.readBuffer[1] << 8));
    result = LinearFmt_YtoX((WORD) result, 0);

    amp = (WORD) result;

    UpdateDataBuf();
    //Set Iin OC Warning point
    if(gPmbusCmd.MFR_LINE_STATUS[0].bits.IsHighLine)
    {
        if(result > DEFAULT_IIN_OC_WARN_HL)
        {
            Parameter.IIN_OC_WARN_HL_LIMIT = DEFAULT_IIN_OC_WARN_HL;
            result = LinearFmt_XtoY(DEFAULT_IIN_OC_WARN_HL, N_MFR_IIN_HL_MAX, 0);
            gPmbusCmd.IIN_OC_WARN_LIMIT[0] = result & 0xFF;
            gPmbusCmd.IIN_OC_WARN_LIMIT[1] = (result >> 8) & 0xFF;
        }
        else
        {
            Parameter.IIN_OC_WARN_HL_LIMIT = result;
        }
    }
    else
    {
        if(result > DEFAULT_IIN_OC_WARN_LL)
        {
            Parameter.IIN_OC_WARN_LL_LIMIT = DEFAULT_IIN_OC_WARN_LL;
            result = LinearFmt_XtoY(DEFAULT_IIN_OC_WARN_LL, N_MFR_IIN_LL_MAX, 0);
            gPmbusCmd.IIN_OC_WARN_LIMIT[0] = result & 0xFF;
            gPmbusCmd.IIN_OC_WARN_LIMIT[1] = (result >> 8) & 0xFF;
        }
        else
        {
            Parameter.IIN_OC_WARN_LL_LIMIT = result;
        }
    }

}

static void CMD_VOUT_COMMAND_Handler()
{
    DWORD result = 0;

    result = ((WORD) I2C.readBuffer[0] | ((WORD) I2C.readBuffer[1] << 8));

    //if ( result >= 3040 && result <= 3360 )   // 3040/256 = 11.875,  3360/256 = 13.125
    if(result >= 2304 && result <= 3584) //[davidchchen]20160408 changed, VOUT_COMMAND range to 9~14v
    {
        UpdateDataBuf();
        result = (WORD) gPmbusCmd.VOUT_COMMAND[0] | ((WORD) gPmbusCmd.VOUT_COMMAND[1] << 8);

#        if defined(Config_12V35_REF)                                     //[davidchchen]20160307 Added
        //gVoutCmd = ( result * DUTY_12V3_REF ) / MAINVOUT_COMMAND_REF; //[davidchchen] 20140121 modified
        gVoutCmd = (result * DUTY_12V35_REF) / MAINVOUT_COMMAND_REF; //[davidchchen]20160307 Added
#        elif defined(Config_12V25_REF)
        gVoutCmd = (result * DUTY_12V25_REF) / MAINVOUT_COMMAND_REF; // [davidchchen] 20151223 modified
#        elif defined(Config_12V5_REF)
        gVoutCmd = (result * DUTY_12V5_REF) / MAINVOUT_COMMAND_REF; // [davidchchen] 20150518 modified
#        endif

    }
    else
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
    }
}

static void CMD_VOUT_OV_FAULT_LIMIT_Handler()
{
    DWORD result;
    BYTE gain;

#    if 1
    gain = 8; //N = -8
    result = ((WORD) I2C.readBuffer[0] | ((WORD) I2C.readBuffer[1] << 8));

    if(result >= 2304 && result <= 3584)
    { //refer to spec. , value range is 9<x<14, N=-8
        UpdateDataBuf();
        result = (result * VOUT_TO_ADC) >> gain; //translate to ADC value
        //Set OV point
        Parameter.OVP_VOUT_OV_FAULT_LIMIT = (WORD) result;
    }
    else
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
    }
#    endif

#    if 0
    result = ((WORD) I2C.readBuffer[0] | ((WORD) I2C.readBuffer[1] << 8));
    cs_kp = result;
    UpdateDataBuf();
#    endif
}

static void CMD_VOUT_OV_WARN_LIMIT_Handler()
{
    DWORD result;
    BYTE gain;

#    if 1
    gain = 8; //N = -8
    result = ((WORD) I2C.readBuffer[0] | ((WORD) I2C.readBuffer[1] << 8));

    if(result >= 2304 && result <= 3584)
    { //refer to spec. , value range is 9<x<14, N=-8
        UpdateDataBuf();
        result = (result * VOUT_TO_ADC) >> gain; //translate to ADC value
        //Set OV point
        Parameter.OVP_VOUT_OV_WARN_LIMIT = (WORD) result;
    }
    else
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
    }
#    endif

#    if 0
    result = ((WORD) I2C.readBuffer[0] | ((WORD) I2C.readBuffer[1] << 8));
    cs_kp = result;
    UpdateDataBuf();
#    endif
}

static void CMD_VOUT_UV_WARN_LIMIT_Handler()
{
    DWORD result;
    BYTE gain;
#    if 1
    gain = 8; //N = -8
    result = ((WORD) I2C.readBuffer[0] | ((WORD) I2C.readBuffer[1] << 8));

    if(result >= 2304 && result <= 3584)
    { //refer to spec. , value range is 9<x<14
        UpdateDataBuf();
        result = (result * VOUT_TO_ADC) >> gain; //translate to ADC value
        //Set UV point
        Parameter.UVP_VOUT_UV_WARN_LIMIT = (WORD) result;
    }
    else
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
    }
#    endif

#    if 0
    result = ((WORD) I2C.readBuffer[0] | ((WORD) I2C.readBuffer[1] << 8));
    cs_ki = result;
    UpdateDataBuf();
#    endif
}

static void CMD_VOUT_UV_FAULT_LIMIT_Handler()
{
    DWORD result;
    BYTE gain;
#    if 1
    gain = 8; //N = -8
    result = ((WORD) I2C.readBuffer[0] | ((WORD) I2C.readBuffer[1] << 8));

    if(result >= 2304 && result <= 3584)
    { //refer to spec. , value range is 9<x<14
        UpdateDataBuf();
        result = (result * VOUT_TO_ADC) >> gain; //translate to ADC value
        //Set UV point
        Parameter.UVP_VOUT_UV_FAULT_LIMIT = (WORD) result;
    }
    else
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
    }
#    endif

#    if 0
    result = ((WORD) I2C.readBuffer[0] | ((WORD) I2C.readBuffer[1] << 8));
    cs_ki = result;
    UpdateDataBuf();
#    endif
}

static void CMD_IOUT_OC_FAULT_LIMIT_Handler()
{
    DWORD result;
    static BYTE gain = 2;
    DWORD amp;
    //WORD ocWarnLimit;
    WORD maxLimit;
    WORD minLimit;

    result = ((WORD) I2C.readBuffer[0] | ((WORD) I2C.readBuffer[1] << 8));

    if(result & 0x0400)
    {
        //Y is negative
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
        return;
    }

    result = LinearFmt_YtoX((WORD) result, gain); //Gain by 128
    //[Peter Chung] 20101208 modified for value range is 22<x<109
    //amp = (WORD)(result >> gain);
    amp = result;


    maxLimit = 230 << gain;
    minLimit = 50 << gain;

    if(amp <= maxLimit && amp >= minLimit)
    {
        UpdateDataBuf();
        /*		if(amp >= DEFAULT_IOUT_OC_FAULT){
                                amp = DEFAULT_IOUT_OC_FAULT;
                                //Set as default value
                                result = LinearFmt_XtoY(DEFAULT_IOUT_OC_FAULT, 0, 0);
                                gPmbusCmd.IOUT_OC_FAULT_LIMIT[0] = result & 0xFF;
                                gPmbusCmd.IOUT_OC_FAULT_LIMIT[1] = (result >> 8) & 0xFF;
                                result = ((DWORD)DEFAULT_IOUT_OC_FAULT * IOUT_TO_ADC) >> (IOUT_TO_ADC_GAIN);
                        }else{
         */
        result = (result * IOUT_TO_ADC) >> (gain + IOUT_TO_ADC_GAIN); // result Gain by 128 and IOUT_TO_ADC Gain by 4, translate to ADC value
        //		}

        //Set OCP point
        Parameter.OCP_IOUT_OC_FAULT_LIMIT = (WORD) result;

#        if 0
        //Update OC Warn limit here
        ocWarnLimit = (WORD) (((DWORD) Parameter.OCP_IOUT_OC_WARN_LIMIT * ADC_TO_IOUT) >> (ADC_TO_IOUT_GAIN - gain));
        if(ocWarnLimit > amp)
        { //Warning limit is greater than Fault
            WORD new_OC_Warn;
            WORD temp;

            new_OC_Warn = amp - ((WORD) (DEFAULT_IOUT_OC_FAULT - DEFAULT_IOUT_OC_WARN) << gain);
            ocWarnLimit = (new_OC_Warn >= minLimit) ? new_OC_Warn : minLimit;
            temp = LinearFmt_XtoY(ocWarnLimit, GetN(N_IOUT, (ocWarnLimit >> gain)), gain);
            gPmbusCmd.IOUT_OC_WARN_LIMIT[0] = temp & 0xFF;
            gPmbusCmd.IOUT_OC_WARN_LIMIT[1] = (temp >> 8) & 0xFF;
            temp = (WORD) (((DWORD) ocWarnLimit * IOUT_TO_ADC) >> (IOUT_TO_ADC_GAIN + gain));
            Parameter.OCP_IOUT_OC_WARN_LIMIT = (WORD) temp;
        }
#        endif

    }
    else
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
    }

}

static void CMD_IOUT_OC_WARN_LIMIT_Handler()
{
    DWORD result;
    static BYTE gain = 2;
    DWORD amp;
    //WORD ocFaultLimit;
    WORD maxLimit;
    WORD minLimit;

    result = ((WORD) I2C.readBuffer[0] | ((WORD) I2C.readBuffer[1] << 8));

    if(result & 0x0400)
    {
        //Y is negative
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
        return;
    }

    result = LinearFmt_YtoX((WORD) result, gain); //Gain by 4
    //[Peter Chung] 20101208 modified for value range is 22<x<105
    //amp = (WORD)(result >> gain);
    amp = result;

    maxLimit = 230 << gain;
    minLimit = 50 << gain;

    if(amp <= maxLimit && amp >= minLimit)
    {
        UpdateDataBuf();

        result = (result * IOUT_TO_ADC) >> (gain + IOUT_TO_ADC_GAIN); // result Gain by 128 and IOUT_TO_ADC Gain by 4, translate to ADC value


        //Set OC Warning point
        Parameter.OCP_IOUT_OC_WARN_LIMIT = (WORD) (result);

#        if 0
        //Update OC Fault limit here
        ocFaultLimit = (WORD) (((DWORD) Parameter.OCP_IOUT_OC_FAULT_LIMIT * ADC_TO_IOUT) >> (ADC_TO_IOUT_GAIN - gain));

        if(amp > ocFaultLimit)
        { //Warn limit is greater than Fault
            WORD new_OC_Fault;
            WORD temp;

            new_OC_Fault = amp + ((WORD) (DEFAULT_IOUT_OC_FAULT - DEFAULT_IOUT_OC_WARN) << gain);
            ocFaultLimit = (new_OC_Fault >= maxLimit ? maxLimit : new_OC_Fault);
            //			ocFaultLimit = (new_OC_Fault >= ((WORD)(DEFAULT_IOUT_OC_FAULT) << gain)) ? ((WORD)(DEFAULT_IOUT_OC_FAULT) << gain) : new_OC_Fault;
            temp = LinearFmt_XtoY(ocFaultLimit, GetN(N_IOUT, (ocFaultLimit >> gain)), gain);
            gPmbusCmd.IOUT_OC_FAULT_LIMIT[0] = temp & 0xFF;
            gPmbusCmd.IOUT_OC_FAULT_LIMIT[1] = (temp >> 8) & 0xFF;
            temp = (WORD) (((DWORD) ocFaultLimit * IOUT_TO_ADC) >> (IOUT_TO_ADC_GAIN + gain));
            Parameter.OCP_IOUT_OC_FAULT_LIMIT = (WORD) temp;
        }
#        endif

    }
    else
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
    }

}

static void CMD_OT_FAULT_LIMIT_Handler()
{
    DWORD result;

    result = ((WORD) I2C.readBuffer[0] | ((WORD) I2C.readBuffer[1] << 8));
    if(result & 0x0400)
    {
        //Y is negative
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
        return;
    }

    result = LinearFmt_YtoX((WORD) result, 0);

    if(result <= 120)
    {
        UpdateDataBuf();
        if(gPmbusCmd.PAGE[0] == 0)
        {
            Parameter.OTP_Tpri_FAULT_LIMIT = result;
            if(result >= 10)
            {
                Parameter.OTP_Tpri_Recovery_LIMIT = result - 10;
            }
            else
            {
                Parameter.OTP_Tpri_Recovery_LIMIT = 0;
            }

            Parameter.OTP_Tsec_FAULT_LIMIT = result;
            if(result >= 10)
            {
                Parameter.OTP_Tsec_Recovery_LIMIT = result - 10;
            }
            else
            {
                Parameter.OTP_Tsec_Recovery_LIMIT = 0;
            }

        }
        else
        { // page == 1
            Parameter.OTP_Tinlet_FAULT_LIMIT = result;
            if(result >= 10)
            {
                Parameter.OTP_Tinlet_Recovery_LIMIT = result - 10;
            }
            else
            {
                Parameter.OTP_Tinlet_Recovery_LIMIT = 0;
            }
        }
    }
    else
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
    }
}

static void CMD_OT_WARN_LIMIT_Handler()
{
    DWORD result;

    result = ((WORD) I2C.readBuffer[0] | ((WORD) I2C.readBuffer[1] << 8));

    if(result & 0x0400)
    {
        //Y is negative
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
        return;
    }

    result = LinearFmt_YtoX((WORD) result, 0);

    if(result <= 120)
    {
        UpdateDataBuf();
        if(gPmbusCmd.PAGE[0] == 0)
        {
            Parameter.OTP_Tpri_WARN_LIMIT = result;
            Parameter.OTP_Tsec_WARN_LIMIT = result;
        }
        else
        { // page == 1
            Parameter.OTP_Tinlet_WARN_LIMIT = result;
        }
    }
    else
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
    }
}

void CMD_VIN_OV_FAULT_LIMIT_Handler()
{
    DWORD result;

    result = ((WORD) I2C.readBuffer[0] | ((WORD) I2C.readBuffer[1] << 8));

    if(result & 0x0400)
    {
        //Y is negative
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
        return;
    }

    result = LinearFmt_YtoX((WORD) result, 0);

    if(result <= (Parameter.MFR_VIN_MAX + 15) && result >= (Parameter.MFR_VIN_MIN - 30))
    {
        UpdateDataBuf();
        Parameter.VIN_OV_FAULT_LIMIT[gPmbusCmd.PAGE[0]] = result;
        if(result >= 7)
        {
            Parameter.VIN_OV_FAULT_RECOVER_LIMIT[gPmbusCmd.PAGE[0]] = result - 7;
        }
    }
    else
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
    }
}

void CMD_VIN_OV_WARN_LIMIT_Handler()
{
    DWORD result;

    result = ((WORD) I2C.readBuffer[0] | ((WORD) I2C.readBuffer[1] << 8));

    if(result & 0x0400)
    {
        //Y is negative
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
        return;
    }

    result = LinearFmt_YtoX((WORD) result, 0);

    if(result <= (Parameter.MFR_VIN_MAX + 15) && result >= (Parameter.MFR_VIN_MIN - 30))
    {
        UpdateDataBuf();
        Parameter.VIN_OV_WARN_LIMIT[gPmbusCmd.PAGE[0]] = result;
    }
    else
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
    }

}

void CMD_VIN_UV_FAULT_LIMIT_Handler()
{
    DWORD result;

    result = ((WORD) I2C.readBuffer[0] | ((WORD) I2C.readBuffer[1] << 8));

    if(result & 0x0400)
    {
        //Y is negative
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
        return;
    }

    result = LinearFmt_YtoX((WORD) result, 0);

    if(result <= (Parameter.MFR_VIN_MAX + 15) && result >= (Parameter.MFR_VIN_MIN - 30))
    {
        UpdateDataBuf();
        Parameter.VIN_UV_FAULT_LIMIT[gPmbusCmd.PAGE[0]] = result;
        if(result >= 7)
        {
            Parameter.VIN_UV_FAULT_RECOVER_LIMIT[gPmbusCmd.PAGE[0]] = result + 7;
        }
    }
    else
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
    }

}

void CMD_VIN_UV_WARN_LIMIT_Handler()
{
    DWORD result;

    result = ((WORD) I2C.readBuffer[0] | ((WORD) I2C.readBuffer[1] << 8));

    if(result & 0x0400)
    {
        //Y is negative
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
        return;
    }

    result = LinearFmt_YtoX((WORD) result, 0);

    if(result <= (Parameter.MFR_VIN_MAX + 15) && result >= (Parameter.MFR_VIN_MIN - 30))
    {
        UpdateDataBuf();
        Parameter.VIN_UV_WARN_LIMIT[gPmbusCmd.PAGE[0]] = result;
    }
    else
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
    }
}

#if 0

static void CMD_PIN_OP_WARN_LIMIT_Handler() //Peter modified for PMBus issue
{
    DWORD result;

    result = ((WORD) I2C.readBuffer[0] | ((WORD) I2C.readBuffer[1] << 8));
    result = LinearFmt_YtoX((WORD) result, 0);

    UpdateDataBuf();
    //Set Pin OP Warning point
    if(gPmbusCmd.MFR_LINE_STATUS[0].bits.IsHighLine)
    {
        if(result > DEFAULT_MFR_PIN_HL_MAX)
        {
            Parameter.PIN_OP_WARN_LIMIT = DEFAULT_MFR_PIN_HL_MAX;
            result = LinearFmt_XtoY(DEFAULT_MFR_PIN_HL_MAX, N_MFR_PIN_HL_MAX, 0);
            gPmbusCmd.PIN_OP_WARN_LIMIT[0] = result & 0xFF;
            gPmbusCmd.PIN_OP_WARN_LIMIT[1] = (result >> 8) & 0xFF;
        }
        else
        {
            Parameter.PIN_OP_WARN_LIMIT = result;
        }
    }
    else
    {
        if(result > DEFAULT_MFR_PIN_LL_MAX)
        {
            Parameter.PIN_OP_WARN_LIMIT = DEFAULT_MFR_PIN_LL_MAX;
            result = LinearFmt_XtoY(DEFAULT_MFR_PIN_LL_MAX, N_MFR_PIN_LL_MAX, 0);
            gPmbusCmd.PIN_OP_WARN_LIMIT[0] = result & 0xFF;
            gPmbusCmd.PIN_OP_WARN_LIMIT[1] = (result >> 8) & 0xFF;
        }
        else
        {
            Parameter.PIN_OP_WARN_LIMIT = result;
        }
    }

}
#endif

static void CMD_FRU_DATA_OFFSET_Handler()
{
    WORD addr;
    DWORD_VAL srcAddr; //[davidchchen]20150113 Added Compatible Code to match HW version
    BYTE *buffer = UserData.Page1.data; //[davidchchen]20150113 Added Compatible Code to match HW version

    //if(gPmbusCmd.PSU_FACTORY_MODE[0].bits.FruAccessControl == FRU_RW_GRANTED){
    addr = ((WORD) I2C.readBuffer[1] | ((WORD) I2C.readBuffer[2] << 8));

    //[davidchchen]20150113 Added Compatible Code to match HW version
    if(addr == 0x00)
    {
        //Load Page1
        srcAddr.Val = USER_DATA_ADDR;
        ReadPM(buffer, 256, srcAddr);
    }

    if(addr <= 0xF0)
    { //0xE0 + 16 - 1 = 0xFF, if the addr > 0xF0, system will invalidly access the data out of FRU range(0x00 ~ 0xFF).
        UpdateDataBuf();
        //Update FRU READ buffer
        gPmbusCmd.FRU_READ[0] = FRU_READ_LEN;
        //memcpy((BYTE*) & gPmbusCmd.FRU_READ[1], (BYTE*) & UserData.Page1.region.FRU[addr], FRU_READ_LEN);         //[davidchchen]20170921 Removed
        memcpy((BYTE*) & gPmbusCmd.FRU_READ[1], (BYTE*) & UserData.Page1.region.FRU.frudata[addr], FRU_READ_LEN);   //[davidchchen]20170921 Added
    }
    else
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
    }
    //}else{
    //	gPagePlusStatus.PAGE[PAGE0].STATUS_CML.bits.INVALID_DATA = 1;
    //	gPagePlusStatus.PAGE[PAGE1].STATUS_CML.bits.INVALID_DATA = 1;
    //}
}

static void CMD_FRU_IMAGE_OVERWRITE_Handler()
{
    WORD addr;

    if(gPmbusCmd.PSU_FACTORY_MODE[0].bits.FruAccessControl == 0b10)
    {
        addr = ((WORD) gPmbusCmd.FRU_WRITE[1] | ((WORD) gPmbusCmd.FRU_WRITE[2] << 8));
        //memcpy((BYTE*) & UserData.Page1.region.FRU[addr], (BYTE*) & I2C.readBuffer[1], 16);     //[davidchchen]20170921 Removed
        //if ( (addr < 256 ) && ( (addr & 0x0f ) == 0 ) )                                                      //[davidchchen]20170921 Removed
        //{
            memcpy((BYTE*) & UserData.Page1.region.FRU.frudata[addr], (BYTE*) & I2C.readBuffer[1], 16);     //[davidchchen]20170921 Added
            memcpy((BYTE*) & gPmbusCmd.FRU_READ[1], (BYTE*) & I2C.readBuffer[1], 16);
            gPmbusCmd.PSU_FACTORY_MODE[0].bits.FruModified = 1;
            UserData.Page2.region.FruModified_Flag = 1;
            PS.FlashWritePage1 = TRUE;
            PS.FlashWritePage2 = TRUE;
        //}
    }
    else
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
    }
}

static void CMD_FAN_COMMAND_1_Handler()
{
    DWORD result;

    result = ((WORD) I2C.readBuffer[0] | ((WORD) I2C.readBuffer[1] << 8));

    if(result & 0x0400)
    {
        //Y is negative
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
        return;
    }

    result = LinearFmt_YtoX(result, 0); //translate to real value

    if(result >= 0 && result <= 100)
    {
        //[Peter Chung] 20101006 implement for new behavior defined in JT0F5-ESG PSU spec X02-09
        //Fan command wouldn't take effect in the following condition
        // 1. AC fail
        // 2. Latch mode
        // 3. Sleep mode

        UpdateDataBuf();
        CalcFanTargetSpeed((BYTE) result);
#        if 0
        if(iAC_OK == AC_GOOD &&
           _SD_Flag.LATCH_SD == FALSE &&
           _SD_Flag.OTP_SD == FALSE)
        {

            //gFan1.OverridedDuty = (WORD)(((DWORD)FAN_PWM_PERIOD * result) / 100);
            CalcFanTargetSpeed((BYTE) result);
        }
#        endif

#        if 0	//[Peter Chung] 20101006 removed due to new behavior defined in JT0F5-ESG PSU spec X02-09
        //if PSU is in sleep mode, and Fan duty received is larger than 0,
        //PSU should leave sleep mode and Enable the fan behavior again.
        if(gPmbusCmd.MFR_ROA_CONTROL[0].bits.IsSleep)
        {
            if(result > 0)
            {
                WakeUpFromSleep();
            }
        }
#        endif
    }
    else
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
    }
}

static void CMD_MFR_PAGE_Handler()
{
    BYTE MfrPage;

    MfrPage = I2C.readBuffer[0];
    if(MfrPage < MAX_FAULT_RECORD || MfrPage == 0xFF)
    { //Redirect MFR_SAMPLE_SET
        UpdateDataBuf();
#        if SAMPLE_SET_SUPPORTED
        if(gPmbusCmd.PSU_FACTORY_MODE[0].bits.MfrSampleSetControl)
        {
            gCmd[0xEC].pBuf = (BYTE*)&(UserData.Page2.region.BlackBox.FaultRecord[MfrPage].MFR_SAMPLE_SET[0]);
        }
        else
        {
            gCmd[0xEC].pBuf = NULL;
        }
#        endif
    }
    else
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
    }
}

static void CMD_MFR_SMBALERT_MASK_Handler()
{
    UpdateDataBuf();

    gAlertMask[gPmbusCmd.PAGE[0]].GLOBAL_MASK = gPmbusCmd.MFR_SMB_ALERT_MASKING[0];
    gAlertMask[gPmbusCmd.PAGE[0]].STATUS_VOUT_MASK = gPmbusCmd.MFR_SMB_ALERT_MASKING[1];
    gAlertMask[gPmbusCmd.PAGE[0]].STATUS_IOUT_MASK = gPmbusCmd.MFR_SMB_ALERT_MASKING[2];
    gAlertMask[gPmbusCmd.PAGE[0]].STATUS_TEMPERATURE_MASK = gPmbusCmd.MFR_SMB_ALERT_MASKING[3];
    gAlertMask[gPmbusCmd.PAGE[0]].STATUS_INPUT_MASK = gPmbusCmd.MFR_SMB_ALERT_MASKING[4];
    gAlertMask[gPmbusCmd.PAGE[0]].STATUS_FAN_1_2_MASK = gPmbusCmd.MFR_SMB_ALERT_MASKING[5];
    gAlertMask[gPmbusCmd.PAGE[0]].STATUS_CML_MASK = gPmbusCmd.MFR_SMB_ALERT_MASKING[6];
}

static void CMD_LOG_INDEX_Handler()
{
    BYTE result;

    result = I2C.readBuffer[0];

    if(result <= 23)
    {
        UpdateDataBuf();

        gPmbusCmd.LOG_CONTENT[0] = 10;
        gPmbusCmd.LOG_CONTENT[1] = UserData.Page3.region.logContent[result].Happen_Time.byte.LB;
        gPmbusCmd.LOG_CONTENT[2] = UserData.Page3.region.logContent[result].Happen_Time.byte.HB;
        gPmbusCmd.LOG_CONTENT[3] = UserData.Page3.region.logContent[result].Happen_Time.byte.UB;
        gPmbusCmd.LOG_CONTENT[4] = UserData.Page3.region.logContent[result].Happen_Time.byte.MB;
        gPmbusCmd.LOG_CONTENT[5] = UserData.Page3.region.logContent[result].Status_Word.byte.LB;
        gPmbusCmd.LOG_CONTENT[6] = UserData.Page3.region.logContent[result].Status_Word.byte.HB;
        gPmbusCmd.LOG_CONTENT[7] = UserData.Page3.region.logContent[result].Status_Debug.byte.LB;
        gPmbusCmd.LOG_CONTENT[8] = UserData.Page3.region.logContent[result].Status_Debug.byte.HB;
        gPmbusCmd.LOG_CONTENT[9] = UserData.Page3.region.logContent[result].Status_MFR_Specific;
        gPmbusCmd.LOG_CONTENT[10] = UserData.Page3.region.logContent[result].Index;

    }
    else
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
    }

}

static void GetCoefficient()
{
    BYTE cmdCode, rw;

    memset(gPmbusCmd.COEFFICIENT, 0, sizeof(gPmbusCmd.COEFFICIENT));
    gPmbusCmd.COEFFICIENT[0] = 5;
    cmdCode = I2C.readBuffer[1];
    rw = I2C.readBuffer[2];

    switch(cmdCode)
    {
        case 0x1B: //SMBALERT_MASK
            if(rw == 0x01)
            { //get Read Coefficient
                memcpy(&gPmbusCmd.COEFFICIENT[1], gCoef.SMBALERT_MASK.R.val, 5);
            }
            else if(rw == 0x00)
            { //get Write Coefficient
                memcpy(&gPmbusCmd.COEFFICIENT[1], gCoef.SMBALERT_MASK.W.val, 5);
            }
            else
            {
                //Do Nothing
            }
            break;

        case 0x86: //READ_EIN
            if(rw == 0x01)
            { //get Read Coefficient
                memcpy(&gPmbusCmd.COEFFICIENT[1], gCoef.EIN.R.val, 5);
            }
            else if(rw == 0x00)
            { //get Write Coefficient
                memcpy(&gPmbusCmd.COEFFICIENT[1], gCoef.EIN.W.val, 5);
            }
            else
            {
                //Do Nothing
            }
            break;

        case 0x87: //READ_EOUT
            if(rw == 0x01)
            { //get Read Coefficient
                memcpy(&gPmbusCmd.COEFFICIENT[1], gCoef.EOUT.R.val, 5);
            }
            else if(rw == 0x00)
            { //get Write Coefficient
                memcpy(&gPmbusCmd.COEFFICIENT[1], gCoef.EOUT.W.val, 5);
            }
            else
            {
                //Do Nothing
            }
            break;

        default:
            gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
            break;

    }

}

static void SetSmbAlertMask(BYTE statusCmdCode, BYTE maskByte, BYTE page)
{
    //BYTE statusCmdCode, maskByte;

    //statusCmdCode = I2C.readBuffer[0];
    //maskByte = I2C.readBuffer[1];

    switch(statusCmdCode)
    {
        case 0x7A://STATUS_VOUT
            if(page == 0xFF)
            {
                gAlertMask[PAGE0].STATUS_VOUT_MASK = maskByte;
                gAlertMask[PAGE1].STATUS_VOUT_MASK = maskByte;
            }
            else
            {
                gAlertMask[page].STATUS_VOUT_MASK = maskByte;
            }
            break;
        case 0x7B://STATUS_IOUT
            if(page == 0xFF)
            {
                gAlertMask[PAGE0].STATUS_IOUT_MASK = maskByte;
                gAlertMask[PAGE1].STATUS_IOUT_MASK = maskByte;
            }
            else
            {
                gAlertMask[page].STATUS_IOUT_MASK = maskByte;
            }
            break;
        case 0x7C://STATUS_INPUT
            if(page == 0xFF)
            {
                gAlertMask[PAGE0].STATUS_INPUT_MASK = maskByte;
                gAlertMask[PAGE1].STATUS_INPUT_MASK = maskByte;
            }
            else
            {
                gAlertMask[page].STATUS_INPUT_MASK = maskByte;
            }
            break;
        case 0x7D://STATUS_TEMPERATURE
            if(page == 0xFF)
            {
                gAlertMask[PAGE0].STATUS_TEMPERATURE_MASK = maskByte;
                gAlertMask[PAGE1].STATUS_TEMPERATURE_MASK = maskByte;
            }
            else
            {
                gAlertMask[page].STATUS_TEMPERATURE_MASK = maskByte;
            }
            break;
        case 0x7E://STATUS_CML
            if(page == 0xFF)
            {
                gAlertMask[PAGE0].STATUS_CML_MASK = maskByte;
                gAlertMask[PAGE1].STATUS_CML_MASK = maskByte;
            }
            else
            {
                gAlertMask[page].STATUS_CML_MASK = maskByte;
            }
            break;
#            if 0
        case 0x7F://STATUS_OTHER
            if(page == 0xFF)
            {
                gAlertMask[PAGE0].STATUS_OTHER_MASK = maskByte;
                gAlertMask[PAGE1].STATUS_OTHER_MASK = maskByte;
            }
            else
            {
                gAlertMask[page].STATUS_OTHER_MASK = maskByte;
            }
            break;
#            endif
        case 0x81://STATUS_FANS_1_2
            if(page == 0xFF)
            {
                gAlertMask[PAGE0].STATUS_FAN_1_2_MASK = maskByte;
                gAlertMask[PAGE1].STATUS_FAN_1_2_MASK = maskByte;
            }
            else
            {
                gAlertMask[page].STATUS_FAN_1_2_MASK = maskByte;
            }
            break;

        default:
            gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
            break;
    }

}

void GetSmbAlertMask(BYTE statusCmdCode)
{
    //prepare SMBALERT_MASK buffer content
    gPmbusCmd.SMBALERT_MASK[0] = 1;
    //statusCmdCode = I2C.readBuffer[1];

    switch(statusCmdCode)
    {
        case 0x7A://STATUS_VOUT
            gPmbusCmd.SMBALERT_MASK[1] = gAlertMask[gPmbusCmd.PAGE[0]].STATUS_VOUT_MASK;
            break;
        case 0x7B://STATUS_IOUT
            gPmbusCmd.SMBALERT_MASK[1] = gAlertMask[gPmbusCmd.PAGE[0]].STATUS_IOUT_MASK;
            break;
        case 0x7C://STATUS_INPUT
            gPmbusCmd.SMBALERT_MASK[1] = gAlertMask[gPmbusCmd.PAGE[0]].STATUS_INPUT_MASK;
            break;
        case 0x7D://STATUS_TEMPERATURE
            gPmbusCmd.SMBALERT_MASK[1] = gAlertMask[gPmbusCmd.PAGE[0]].STATUS_TEMPERATURE_MASK;
            break;
        case 0x7E://STATUS_CML
            gPmbusCmd.SMBALERT_MASK[1] = gAlertMask[gPmbusCmd.PAGE[0]].STATUS_CML_MASK;
            break;
#            if 0
        case 0x7F://STATUS_OTHER
            gPmbusCmd.SMBALERT_MASK[1] = gAlertMask[gPmbusCmd.PAGE[0]].STATUS_OTHER_MASK;
            break;
#            endif
        case 0x81://STATUS_FANS_1_2
            gPmbusCmd.SMBALERT_MASK[1] = gAlertMask[gPmbusCmd.PAGE[0]].STATUS_FAN_1_2_MASK;
            break;
        default:
            //do nothing
            gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
            gPmbusCmd.SMBALERT_MASK[1] = 0xFF;
            break;
    }

}

void GetPagePlusReadSmbAlertMask(BYTE statusCmdCode, BYTE tpage)
{
    //prepare SMBALERT_MASK buffer content
    gPmbusCmd.SMBALERT_MASK[0] = 1;
    //statusCmdCode = I2C.readBuffer[1];

    switch(statusCmdCode)
    {
        case 0x7A://STATUS_VOUT
            gPmbusCmd.SMBALERT_MASK[1] = gAlertMask[tpage].STATUS_VOUT_MASK;
            break;
        case 0x7B://STATUS_IOUT
            gPmbusCmd.SMBALERT_MASK[1] = gAlertMask[tpage].STATUS_IOUT_MASK;
            break;
        case 0x7C://STATUS_INPUT
            gPmbusCmd.SMBALERT_MASK[1] = gAlertMask[tpage].STATUS_INPUT_MASK;
            break;
        case 0x7D://STATUS_TEMPERATURE
            gPmbusCmd.SMBALERT_MASK[1] = gAlertMask[tpage].STATUS_TEMPERATURE_MASK;
            break;
        case 0x7E://STATUS_CML
            gPmbusCmd.SMBALERT_MASK[1] = gAlertMask[tpage].STATUS_CML_MASK;
            break;
#            if 0
        case 0x7F://STATUS_OTHER
            gPmbusCmd.SMBALERT_MASK[1] = gAlertMask[tpage].STATUS_OTHER_MASK;
            break;
#            endif
        case 0x81://STATUS_FANS_1_2
            gPmbusCmd.SMBALERT_MASK[1] = gAlertMask[tpage].STATUS_FAN_1_2_MASK;
            break;
        default:
            //do nothing
            gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
            gPmbusCmd.SMBALERT_MASK[1] = 0xFF;
            break;
    }

}

#if 1

static BYTE isValidQuery(BYTE q_cmd)
{
    switch(q_cmd)
    {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x05:
        case 0x10:
        case 0x11:
        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x20:
        case 0x21:
        case 0x3A:
        case 0x3B:
        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x46:
        case 0x47:
        case 0x4A:
        case 0x4F:
        case 0x50:
        case 0x51:
        case 0x55:
        case 0x56:
        case 0x57:
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x79:
        case 0x7A:
        case 0x7B:
        case 0x7C:
        case 0x7D:
        case 0x7E:
        case 0x81:
        case 0x88:
        case 0x89:
        case 0x8B:
        case 0x8C:
        case 0x8D:
        case 0x8E:
        case 0x8F:
        case 0x90:
        case 0x91:
        case 0x95:
        case 0x96:
        case 0x97:
        case 0x98:
        case 0x99:
        case 0x9A:
        case 0x9B:
        case 0x9C:
        case 0x9D:
        case 0x9E:
        case 0xA0:
        case 0xA1:
        case 0xA2:
        case 0xA3:
        case 0xA4:
        case 0xA5:
        case 0xA6:
        case 0xA7:
        case 0xA8:
        case 0xA9:
        case 0xB0: //FRU_WRITE          //[davidchchen]20170704 Added IPMI FRU DATA FORMAT
        case 0xB1: //FRU_READ           //[davidchchen]20170704 Added IPMI FRU DATA FORMAT
        case 0xCF:  //[davidchchen]20160304 added RCB
        case 0xD0:
        case 0xD1:
        case 0xD3:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:
        case 0xD8:
        case 0xD9:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:
        case 0xE0:  //[davidchchen]20170216 added PSON Signal Enable/disable
        case 0xE5: //MFR_POS_TOTAL          //[davidchchen]20170310 Added
        case 0xE6: //MFR_POS_LAST           //[davidchchen]20170310 Added
        case 0xE7: //BMC_UNIX_TIMESTAMP     //[davidchchen]20170310 Added BMC_UNIX_TIMESTAMP cmd
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }
}
#endif

BYTE isPermitToWriteInPage1(BYTE cmd)
{
    if(cmd == 0x00 || //PAGE
       cmd == 0x03 || //CLEAR_FAULT
       cmd == 0x10 || //WRITE_PROTECT
       cmd == 0x1A || //QUERY
       cmd == 0x1B || //SMBALERT_MASK
       cmd == 0x4F || //OT_FAULT_LIMIT
       cmd == 0x51 || //OT_WARN_LIMIT
       cmd == 0x55 || //VIN_OV_FAULT_LIMIT
       cmd == 0x57 || //VIN_OV_WARN_LIMIT
       cmd == 0x58 || //VIN_UV_WARN_LIMIT
       cmd == 0x59) //VIN_UV_FAULT_LIMIT
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BYTE isPermitToReadInPage1(BYTE cmd)
{
    if(cmd == 0x00 || //PAGE
       cmd == 0x10 || //WRITE_PROTECT
       cmd == 0x1A || //QUERY
       cmd == 0x1B || //SMBALERT_MASK
       cmd == 0x20 || //VOUT_MODE
       cmd == 0x4F || //OT_FAULT_LIMIT
       cmd == 0x50 || //OT_FAULT_RESPONSE
       cmd == 0x51 || //OT_WARN_LIMIT
       cmd == 0x55 || //VIN_OV_FAULT_LIMIT
       cmd == 0x56 || //VIN_OV_FAULT_RESPONSE
       cmd == 0x57 || //VIN_OV_WARN_LIMIT
       cmd == 0x58 || //VIN_UV_WARN_LIMIT
       cmd == 0x59 || //VIN_UV_FAULT_LIMIT
       cmd == 0x5A || //VIN_UV_FAULT_RESPONSE
       cmd == 0x79 || //STATUS_WORD
       cmd == 0x7A || //STATUS_VOUT
       cmd == 0x7B || //STATUS_IOUT     //[davidchchen]20170619
       cmd == 0x7C || //STATUS_INPUT
       cmd == 0x7D || //STATUS_TEMPERATURE
       cmd == 0x7E || //STATUS_CML
       cmd == 0x8B || //READ_VOUT
       cmd == 0xA0 || //MFR_VIN_MIN
       cmd == 0xA1 || //MFR_VIN_MAX
       cmd == 0xA2 || //MFR_IIN_MAX
       cmd == 0xA3) //MFR_PIN_MAX
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void GetQuery()
{
    BYTE cmdCode;
    BYTE queryByte;

    cmdCode = I2C.readBuffer[1];
    //[Peter Chung] 20101222 modified
#    if 1
    if(isValidQuery(cmdCode))
    {
        if(gPmbusCmd.PAGE[0] == 0)
        {
            queryByte = gCmd[cmdCode].type & QUERY_REPORT_MASK;
        }
        else
        {
            if(isPermitToWriteInPage1(cmdCode) || isPermitToReadInPage1(cmdCode))
            {
                queryByte = gCmd[cmdCode].type & QUERY_REPORT_MASK;
            }
            else
            {
                queryByte = 0x00;
            }
        }
    }
    else
    {
        queryByte = 0x00;
    }
#    else
    queryByte = gCmd[cmdCode].type & QUERY_REPORT_MASK;
#    endif

    gPmbusCmd.QUERY[0] = 1;
    gPmbusCmd.QUERY[1] = queryByte;
}

void RefreshAllCaliInfo()
{
    //Vout Calibration
    gPmbusCmd.READ_ALL_CALI_INFO[0] = 8;
    gPmbusCmd.READ_ALL_CALI_INFO[1] = UserData.Page1.region.Vout[OUTPUT_12V].CmdOffset & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[2] = (UserData.Page1.region.Vout[OUTPUT_12V].CmdOffset >> 8) & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[3] = UserData.Page1.region.Vout[OUTPUT_12V].ReportOffset & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[4] = (UserData.Page1.region.Vout[OUTPUT_12V].ReportOffset >> 8) & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[5] = 0;
    gPmbusCmd.READ_ALL_CALI_INFO[6] = 0;
    gPmbusCmd.READ_ALL_CALI_INFO[7] = UserData.Page1.region.StbVoutOffset & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[8] = (UserData.Page1.region.StbVoutOffset >> 8) & 0xFF;
    //Iout1 Calibration
    gPmbusCmd.READ_ALL_CALI_INFO[9] = 6;
    gPmbusCmd.READ_ALL_CALI_INFO[10] = UserData.Page1.region.Iout[OUTPUT_12V].slope & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[11] = (UserData.Page1.region.Iout[OUTPUT_12V].slope >> 8) & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[12] = UserData.Page1.region.Iout[OUTPUT_12V].Offset & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[13] = (UserData.Page1.region.Iout[OUTPUT_12V].Offset >> 8) & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[14] = (UserData.Page1.region.Iout[OUTPUT_12V].Offset >> 16) & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[15] = (UserData.Page1.region.Iout[OUTPUT_12V].Offset >> 24) & 0xFF;
    //Iout2 Calibration
    gPmbusCmd.READ_ALL_CALI_INFO[16] = 6;
    gPmbusCmd.READ_ALL_CALI_INFO[17] = UserData.Page1.region.Iout[OUTPUT_12V].slope2 & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[18] = (UserData.Page1.region.Iout[OUTPUT_12V].slope2 >> 8) & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[19] = UserData.Page1.region.Iout[OUTPUT_12V].Offset2 & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[20] = (UserData.Page1.region.Iout[OUTPUT_12V].Offset2 >> 8) & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[21] = (UserData.Page1.region.Iout[OUTPUT_12V].Offset2 >> 16) & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[22] = (UserData.Page1.region.Iout[OUTPUT_12V].Offset2 >> 24) & 0xFF;
    //Iout3 Calibration
    gPmbusCmd.READ_ALL_CALI_INFO[23] = 6;
    gPmbusCmd.READ_ALL_CALI_INFO[24] = UserData.Page1.region.Iout[OUTPUT_12V].slope3 & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[25] = (UserData.Page1.region.Iout[OUTPUT_12V].slope3 >> 8) & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[26] = UserData.Page1.region.Iout[OUTPUT_12V].Offset3 & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[27] = (UserData.Page1.region.Iout[OUTPUT_12V].Offset3 >> 8) & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[28] = (UserData.Page1.region.Iout[OUTPUT_12V].Offset3 >> 16) & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[29] = (UserData.Page1.region.Iout[OUTPUT_12V].Offset3 >> 24) & 0xFF;
    //Iout_CS Calibration
    gPmbusCmd.READ_ALL_CALI_INFO[30] = 6;
    gPmbusCmd.READ_ALL_CALI_INFO[31] = UserData.Page1.region.Vout[OUTPUT_12V].CS_Slope & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[32] = (UserData.Page1.region.Vout[OUTPUT_12V].CS_Slope >> 8) & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[33] = UserData.Page1.region.Vout[OUTPUT_12V].CS_Offset & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[34] = (UserData.Page1.region.Vout[OUTPUT_12V].CS_Offset >> 8) & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[35] = (UserData.Page1.region.Vout[OUTPUT_12V].CS_Offset >> 16) & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[36] = (UserData.Page1.region.Vout[OUTPUT_12V].CS_Offset >> 24) & 0xFF;
    //Vout_CS Calibration
    gPmbusCmd.READ_ALL_CALI_INFO[37] = 6;
    gPmbusCmd.READ_ALL_CALI_INFO[38] = UserData.Page1.region.VCS_Slope & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[39] = (UserData.Page1.region.VCS_Slope >> 8) & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[40] = UserData.Page1.region.VCS_Offset & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[41] = (UserData.Page1.region.VCS_Offset >> 8) & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[42] = (UserData.Page1.region.VCS_Offset >> 16) & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[43] = (UserData.Page1.region.VCS_Offset >> 24) & 0xFF;
    //Iout4 Calibration
    gPmbusCmd.READ_ALL_CALI_INFO[44] = 6;
    gPmbusCmd.READ_ALL_CALI_INFO[45] = UserData.Page1.region.Iout[OUTPUT_12V].slope4 & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[46] = (UserData.Page1.region.Iout[OUTPUT_12V].slope4 >> 8) & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[47] = UserData.Page1.region.Iout[OUTPUT_12V].Offset4 & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[48] = (UserData.Page1.region.Iout[OUTPUT_12V].Offset4 >> 8) & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[49] = (UserData.Page1.region.Iout[OUTPUT_12V].Offset4 >> 16) & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[50] = (UserData.Page1.region.Iout[OUTPUT_12V].Offset4 >> 24) & 0xFF;

    //Large_Iout_CS Calibration
    gPmbusCmd.READ_ALL_CALI_INFO[51] = 6;
    gPmbusCmd.READ_ALL_CALI_INFO[52] = 0;
    gPmbusCmd.READ_ALL_CALI_INFO[53] = 0;
    gPmbusCmd.READ_ALL_CALI_INFO[54] = 0;
    gPmbusCmd.READ_ALL_CALI_INFO[55] = 0;
    gPmbusCmd.READ_ALL_CALI_INFO[56] = 0;
    gPmbusCmd.READ_ALL_CALI_INFO[57] = 0;
    //Vin Calibration
    gPmbusCmd.READ_ALL_CALI_INFO[58] = 6;
    gPmbusCmd.READ_ALL_CALI_INFO[59] = UserData.Page1.region.VinGain & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[60] = (UserData.Page1.region.VinGain >> 8) & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[61] = 0;
    gPmbusCmd.READ_ALL_CALI_INFO[62] = 0;
    gPmbusCmd.READ_ALL_CALI_INFO[63] = 0;
    gPmbusCmd.READ_ALL_CALI_INFO[64] = 0;
    //Iin Calibration
    gPmbusCmd.READ_ALL_CALI_INFO[65] = 6;
    gPmbusCmd.READ_ALL_CALI_INFO[66] = UserData.Page1.region.IinGain & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[67] = (UserData.Page1.region.IinGain >> 8) & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[68] = 0;
    gPmbusCmd.READ_ALL_CALI_INFO[69] = 0;
    gPmbusCmd.READ_ALL_CALI_INFO[70] = 0;
    gPmbusCmd.READ_ALL_CALI_INFO[71] = 0;
    //Pin Calibration
    gPmbusCmd.READ_ALL_CALI_INFO[72] = 6;
    gPmbusCmd.READ_ALL_CALI_INFO[73] = UserData.Page1.region.PinGain & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[74] = (UserData.Page1.region.PinGain >> 8) & 0xFF;
    gPmbusCmd.READ_ALL_CALI_INFO[75] = 0;
    gPmbusCmd.READ_ALL_CALI_INFO[76] = 0;
    gPmbusCmd.READ_ALL_CALI_INFO[77] = 0;
    gPmbusCmd.READ_ALL_CALI_INFO[78] = 0;
}

static void GetCaliInfo()
{
    BYTE type;

    memset(gPmbusCmd.GEN_CAL_R, 0, sizeof(gPmbusCmd.GEN_CAL_R));
    type = I2C.readBuffer[1];
    gPmbusCmd.GEN_CAL_R[0] = 6; //byte count

    switch(type)
    {
        case CAL_VOUT:
            break;
        case CAL_IOUT:
            //slope
            gPmbusCmd.GEN_CAL_R[1] = UserData.Page1.region.Iout[OUTPUT_12V].slope & 0xFF;
            gPmbusCmd.GEN_CAL_R[2] = (UserData.Page1.region.Iout[OUTPUT_12V].slope >> 8) & 0xFF;
            //offset
            gPmbusCmd.GEN_CAL_R[3] = UserData.Page1.region.Iout[OUTPUT_12V].Offset & 0xFF;
            gPmbusCmd.GEN_CAL_R[4] = (UserData.Page1.region.Iout[OUTPUT_12V].Offset >> 8) & 0xFF;
            gPmbusCmd.GEN_CAL_R[5] = (UserData.Page1.region.Iout[OUTPUT_12V].Offset >> 16) & 0xFF;
            gPmbusCmd.GEN_CAL_R[6] = (UserData.Page1.region.Iout[OUTPUT_12V].Offset >> 24) & 0xFF;

            break;
        case CAL_VIN:
            break;
        case CAL_IIN:
            break;

        default:
            break;
    }
}

void UpdateAccumulatedPower(DWORD *Paccum, WORD Psample, BYTE *rollover_cnt, DWORD *sample_cnt)
{
    if(((*Paccum) + Psample) > Prollover)
    {
        if((*rollover_cnt) < Max_Rollover_Cnt)
        {
            *rollover_cnt += 1;
        }
        else
        {
            *rollover_cnt = 0;
        }
        *Paccum = Psample - (Prollover - (*Paccum));
    }
    else
    {
        *Paccum += Psample;
    }

    if(*sample_cnt > MAX_Sample_Cnt)
    {
        *sample_cnt = 0;
    }
    else
    {
        *sample_cnt += 1;
    }
}

void UpdateEIN() //Peter modified for PMBus issue
{
    static DWORD Paccum = 0;
    static BYTE rollover_cnt = 0;
    static DWORD sample_cnt = 0;

    WORD Psample = 0;
    WORD D_FMT_Paccum;
    WORD pin;

    //get PIN value
    pin = ((WORD) gPmbusCmd.READ_PIN[0] | (gPmbusCmd.READ_PIN[1] << 8));
    Psample = (WORD) (LinearFmt_YtoX(pin, 0));
    UpdateAccumulatedPower(&Paccum, Psample, &rollover_cnt, &sample_cnt);

    //Trasnlate to Direct format
    D_FMT_Paccum = EncodeAsDirectFmt(Paccum, gCoef.EIN.R.data.m.Val, gCoef.EIN.R.data.b.Val, gCoef.EIN.R.data.r);

    //Fill data buffer
    PS.EIN_DataUpdating = 1;
    gPmbusCmd.READ_EIN[0] = 6; //Block count
    gPmbusCmd.READ_EIN[1] = D_FMT_Paccum & 0xFF; //Accumulated power low byte
    gPmbusCmd.READ_EIN[2] = (D_FMT_Paccum >> 8)&0xFF; //Accumulated power high byte
    gPmbusCmd.READ_EIN[3] = rollover_cnt; //Rollover count
    gPmbusCmd.READ_EIN[4] = sample_cnt & 0xFF; //Sample count low byte
    gPmbusCmd.READ_EIN[5] = (sample_cnt >> 8)&0xFF; //Sample count mid byte
    gPmbusCmd.READ_EIN[6] = (sample_cnt >> 16)&0xFF; //Sample count high byte

    PS.EIN_DataUpdated = 1;
    PS.EIN_DataUpdating = 0;

}

void UpdateEOUT() //Peter modified for PMBus issue
{
    static DWORD Paccum = 0;
    static BYTE rollover_cnt = 0;
    static DWORD sample_cnt = 0;

    WORD Psample = 0;
    WORD D_FMT_Paccum;
    WORD pout;

    //get POUT value
    pout = ((WORD) gPmbusCmd.READ_POUT[0] | (gPmbusCmd.READ_POUT[1] << 8));
    Psample = (WORD) (LinearFmt_YtoX(pout, 0));

    UpdateAccumulatedPower(&Paccum, Psample, &rollover_cnt, &sample_cnt);

    //Trasnlate to Direct format
    D_FMT_Paccum = EncodeAsDirectFmt(Paccum, gCoef.EOUT.R.data.m.Val, gCoef.EOUT.R.data.b.Val, gCoef.EOUT.R.data.r);

    PS.EOUT_DataUpdating = 1;
    gPmbusCmd.READ_EOUT[0] = 6; //Block count
    gPmbusCmd.READ_EOUT[1] = D_FMT_Paccum & 0xFF; //Accumulated power low byte
    gPmbusCmd.READ_EOUT[2] = (D_FMT_Paccum >> 8)&0xFF; //Accumulated power high byte
    gPmbusCmd.READ_EOUT[3] = rollover_cnt; //Rollover count
    gPmbusCmd.READ_EOUT[4] = sample_cnt & 0xFF; //Sample count low byte
    gPmbusCmd.READ_EOUT[5] = (sample_cnt >> 8)&0xFF; //Sample count mid byte
    gPmbusCmd.READ_EOUT[6] = (sample_cnt >> 16)&0xFF; //Sample count high byte

    PS.EOUT_DataUpdated = 1;
    PS.EOUT_DataUpdating = 0;
}

void InitEnergyCount()
{
    gPmbusCmd.READ_EIN[0] = 6; //Block count
    gPmbusCmd.READ_EIN[1] = 0; //Accumulated power low byte
    gPmbusCmd.READ_EIN[2] = 0; //Accumulated power high byte
    gPmbusCmd.READ_EIN[3] = 0; //Rollover count
    gPmbusCmd.READ_EIN[4] = 0; //Sample count low byte
    gPmbusCmd.READ_EIN[5] = 0; //Sample count mid byte
    gPmbusCmd.READ_EIN[6] = 0; //Sample count high byte

    gPmbusCmd.READ_EOUT[0] = 6; //Block count
    gPmbusCmd.READ_EOUT[1] = 0; //Accumulated power low byte
    gPmbusCmd.READ_EOUT[2] = 0; //Accumulated power high byte
    gPmbusCmd.READ_EOUT[3] = 0; //Rollover count
    gPmbusCmd.READ_EOUT[4] = 0; //Sample count low byte
    gPmbusCmd.READ_EOUT[5] = 0; //Sample count mid byte
    gPmbusCmd.READ_EOUT[6] = 0; //Sample count high byte
}

void UpdateEnergyCount()
{
    UpdateEIN();
    UpdateEOUT();
}

void Count_4AC_Cycle()
{
    static WORD updateTime = 80; //ms
    static WORD cnt = 0;
    static WORD acCycle_cnt = 0;

    if(cnt >= updateTime)
    {
        if(! PS.I2C_Processing)
        {
            UpdateEnergyCount();
            //UpdateInputInfo();
            cnt = 0;
        }
    }
    else
    {
        cnt ++;
    }

    //CalcPinAvg();

    if(acCycle_cnt >= 20)
    { //around 1 AC cycle
        acCycle_cnt = 0;
        UpdateInputInfo();

        //Uart.U2.Rx.VacLsb = 0X98;     //[davidchchen]debug
        //Uart.U2.Rx.VacMsb = 0XF3;     //[davidchchen]debug
#        if WithTeridian
        Real_Vac = LinearFmt_YtoX((((WORD) Uart.U2.Rx.VacMsb << 8) | Uart.U2.Rx.VacLsb), 0);
#        endif

        //FW V42 source code
        //      if ( isInputDetected == TRUE )
        //      {
        //          CheckLineStatus ( Real_Vac, Uart.U2.Rx.Freq );
        //      }

        //david add
        CheckLineStatus(Real_Vac, Uart.U2.Rx.Freq);

    }
    else
    {
        acCycle_cnt ++;
    }

}

void init_SmbAlertMask()
{
    //Refer to 12G Spec.
    //[Peter Chung] 20100901 modified for IPMM spec. X03-00
    //[Peter Chung] 20101223 modified for PAGE PLUS WR function
    //PAGE 0
    gAlertMask[PAGE0].GLOBAL_MASK = 0xFF; //enable alert
    //gAlertMask.STATUS_WORD_LB_MASK = 0xFF;
    //gAlertMask.STATUS_WORD_HB_MASK = 0xFF;
    gAlertMask[PAGE0].STATUS_VOUT_MASK = 0x0F;
    gAlertMask[PAGE0].STATUS_IOUT_MASK = 0x5F;
    gAlertMask[PAGE0].STATUS_INPUT_MASK = 0x07;
    gAlertMask[PAGE0].STATUS_TEMPERATURE_MASK = 0x3F;
    gAlertMask[PAGE0].STATUS_CML_MASK = 0xFF;
    //gAlertMask[PAGE0].STATUS_OTHER_MASK = 0xFF;
    gAlertMask[PAGE0].STATUS_FAN_1_2_MASK = 0x33;

    //PAGE 1
    gAlertMask[PAGE1].GLOBAL_MASK = 0xFF; //enable alert
    gAlertMask[PAGE1].STATUS_VOUT_MASK = 0x0F;
    gAlertMask[PAGE1].STATUS_IOUT_MASK = 0x5F;
    gAlertMask[PAGE1].STATUS_INPUT_MASK = 0x07;
    gAlertMask[PAGE1].STATUS_TEMPERATURE_MASK = 0x3F;
    gAlertMask[PAGE1].STATUS_CML_MASK = 0xFF;
    //gAlertMask[PAGE1].STATUS_OTHER_MASK = 0xFF;
    gAlertMask[PAGE1].STATUS_FAN_1_2_MASK = 0x33;
}

void init_MFR_EFFICIENCY_LL()
{
    WORD temp;

    gPmbusCmd.MFR_EFFICIENCY_LL[0] = MFR_EFFICIENCY_LL_LEN; //byte count

    temp = DEFAULT_MFR_EFFICIENCY_LL_INPUT_VOL; //N=0
    gPmbusCmd.MFR_EFFICIENCY_LL[1] = temp & 0xFF;
    gPmbusCmd.MFR_EFFICIENCY_LL[2] = temp >> 8;
    //power 220W(20%)
    temp = DEFAULT_MFR_EFFICIENCY_LL_POWER_20_LOAD; //N=0
    gPmbusCmd.MFR_EFFICIENCY_LL[3] = temp & 0xFF;
    gPmbusCmd.MFR_EFFICIENCY_LL[4] = temp >> 8;
    //efficiency 91(20%)
    temp = DEFAULT_MFR_EFFICIENCY_LL_EFF_20_LOAD; //N=0
    gPmbusCmd.MFR_EFFICIENCY_LL[5] = temp & 0xFF;
    gPmbusCmd.MFR_EFFICIENCY_LL[6] = temp >> 8;
    //power 550W(50%)
    temp = DEFAULT_MFR_EFFICIENCY_LL_POWER_50_LOAD; //N=0
    gPmbusCmd.MFR_EFFICIENCY_LL[7] = temp & 0xFF;
    gPmbusCmd.MFR_EFFICIENCY_LL[8] = temp >> 8;
    //efficiency 92.5(50%)
    temp = LinearFmt_XtoY(DEFAULT_MFR_EFFICIENCY_LL_EFF_50_LOAD, N_MFR_EFFICIENCY_LL_EFF_50_LOAD, 1); //N=-3
    gPmbusCmd.MFR_EFFICIENCY_LL[9] = temp & 0xFF;
    gPmbusCmd.MFR_EFFICIENCY_LL[10] = temp >> 8;
    //power 1100W(100%)
    //temp = DEFAULT_MFR_EFFICIENCY_LL_POWER_100_LOAD >> 1; //N=1
    //temp = temp | (0x08 << 8);
    temp = LinearFmt_XtoY(DEFAULT_MFR_EFFICIENCY_LL_POWER_100_LOAD, N_MFR_EFFICIENCY_LL_POWER_100_LOAD, 0); //N=1
    gPmbusCmd.MFR_EFFICIENCY_LL[11] = temp & 0xFF;
    gPmbusCmd.MFR_EFFICIENCY_LL[12] = temp >> 8;
    //efficiency 90(100%)
    temp = DEFAULT_MFR_EFFICIENCY_LL_EFF_100_LOAD; //N=0
    gPmbusCmd.MFR_EFFICIENCY_LL[13] = temp & 0xFF;
    gPmbusCmd.MFR_EFFICIENCY_LL[14] = temp >> 8;
}

void init_MFR_EFFICIENCY_HL()
{
    WORD temp;

    gPmbusCmd.MFR_EFFICIENCY_HL[0] = MFR_EFFICIENCY_HL_LEN; //byte count

    //input voltage 230V
    temp = DEFAULT_MFR_EFFICIENCY_HL_INPUT_VOL; //N=0
    gPmbusCmd.MFR_EFFICIENCY_HL[1] = temp & 0xFF;
    gPmbusCmd.MFR_EFFICIENCY_HL[2] = temp >> 8;
    //power 500W(20%)
    temp = DEFAULT_MFR_EFFICIENCY_HL_POWER_20_LOAD; //N=0
    gPmbusCmd.MFR_EFFICIENCY_HL[3] = temp & 0xFF;
    gPmbusCmd.MFR_EFFICIENCY_HL[4] = temp >> 8;
    //efficiency 90(20%)
    temp = DEFAULT_MFR_EFFICIENCY_HL_EFF_20_LOAD; //N=0
    gPmbusCmd.MFR_EFFICIENCY_HL[5] = temp & 0xFF;
    gPmbusCmd.MFR_EFFICIENCY_HL[6] = temp >> 8;
    //power 1250W(50%)
    temp = LinearFmt_XtoY(DEFAULT_MFR_EFFICIENCY_HL_POWER_50_LOAD, N_MFR_EFFICIENCY_HL_POWER_50_LOAD, 0);
    gPmbusCmd.MFR_EFFICIENCY_HL[7] = temp & 0xFF;
    gPmbusCmd.MFR_EFFICIENCY_HL[8] = temp >> 8;
    //efficiency 94(50%)
    temp = LinearFmt_XtoY(DEFAULT_MFR_EFFICIENCY_HL_EFF_50_LOAD, N_MFR_EFFICIENCY_HL_EFF_50_LOAD, 0);
    gPmbusCmd.MFR_EFFICIENCY_HL[9] = temp & 0xFF;
    gPmbusCmd.MFR_EFFICIENCY_HL[10] = temp >> 8;
    //power 2500W(100%)
    temp = LinearFmt_XtoY(DEFAULT_MFR_EFFICIENCY_HL_POWER_100_LOAD, N_MFR_EFFICIENCY_HL_POWER_100_LOAD, 0);
    gPmbusCmd.MFR_EFFICIENCY_HL[11] = temp & 0xFF;
    gPmbusCmd.MFR_EFFICIENCY_HL[12] = temp >> 8;
    //efficiency 92(100%)
    temp = DEFAULT_MFR_EFFICIENCY_HL_EFF_100_LOAD; //N=0
    gPmbusCmd.MFR_EFFICIENCY_HL[13] = temp & 0xFF;
    gPmbusCmd.MFR_EFFICIENCY_HL[14] = temp >> 8;
}

void Init_MFR_SMB_ALERT_MASKING()
{
    gPmbusCmd.MFR_SMB_ALERT_MASKING[GLOBAL_MASK] = gAlertMask[PAGE0].GLOBAL_MASK;
    gPmbusCmd.MFR_SMB_ALERT_MASKING[VOUT_MASK] = gAlertMask[PAGE0].STATUS_VOUT_MASK;
    gPmbusCmd.MFR_SMB_ALERT_MASKING[IOUT_MASK] = gAlertMask[PAGE0].STATUS_IOUT_MASK;
    gPmbusCmd.MFR_SMB_ALERT_MASKING[TEMPERATURE_MASK] = gAlertMask[PAGE0].STATUS_TEMPERATURE_MASK;
    gPmbusCmd.MFR_SMB_ALERT_MASKING[INPUT_MASK] = gAlertMask[PAGE0].STATUS_INPUT_MASK;
    gPmbusCmd.MFR_SMB_ALERT_MASKING[FAN_1_2_MASK] = gAlertMask[PAGE0].STATUS_FAN_1_2_MASK;
    gPmbusCmd.MFR_SMB_ALERT_MASKING[CML_MASK] = gAlertMask[PAGE0].STATUS_CML_MASK;
}

void reverseStr(char *strPointer, BYTE len)
{
    char temp;
    char *p, *q;

    p = strPointer;
    q = & strPointer[len - 1];
    for(; p <= q; p ++, q --)
    {
        temp = * p;
        *p = * q;
        *q = temp;
    }
}

static void RefreshMFR()
{
    BYTE *FRU_tbl_ptr;
    //BYTE *FRU_table = UserData.Page1.region.FRU;              //[davidchchen]20170921 Removed
    BYTE *FRU_table = UserData.Page1.region.FRU.frudata;        //[davidchchen]20170921 Added
    BYTE year;

    //MFR_ID
    memset(gPmbusCmd.MFR_ID, '\0', sizeof(gPmbusCmd.MFR_ID));
    gPmbusCmd.MFR_ID[0] = MFR_ID_LEN;
    strncpy((char *) &gPmbusCmd.MFR_ID[1], (char *) "LITEON", 6);

    //MFR_MODEL
    memset(gPmbusCmd.MFR_MODEL, '\0', sizeof(gPmbusCmd.MFR_MODEL));
    gPmbusCmd.MFR_MODEL[0] = MFR_MODEL_LEN;
    strncpy((char *) &gPmbusCmd.MFR_MODEL[1], (char *) MFR_MODEL_INIT, MFR_MODEL_LEN); // 10 is based on FRU spec

    //MFR_REVISION
    memset(gPmbusCmd.MFR_REVISION, '\0', sizeof(gPmbusCmd.MFR_REVISION));
    FRU_tbl_ptr = (BYTE *) & FRU_table[FRU_REVISION_OFFSET];
    gPmbusCmd.MFR_REVISION[0] = MFR_REVISION_LEN;
    strncpy((char *) &gPmbusCmd.MFR_REVISION[1], (char *) FRU_tbl_ptr, FRU_REVISION_LEN); // 3 is based on FRU spec

    //MFR_LOCATION
    memset(gPmbusCmd.MFR_LOCATION, '\0', sizeof(gPmbusCmd.MFR_LOCATION));
    gPmbusCmd.MFR_LOCATION[0] = MFR_LOCATION_LEN;
    strncpy((char *) &gPmbusCmd.MFR_LOCATION[1], (char *) "CHINA", MFR_LOCATION_INIT_LEN);

    //MFR_SERIAL
    memset(gPmbusCmd.MFR_SERIAL, '\0', sizeof(gPmbusCmd.MFR_SERIAL));
    FRU_tbl_ptr = (BYTE *) & FRU_table[FRU_LENOVO_BARCODE_OFFSET];
    gPmbusCmd.MFR_SERIAL[0] = MFR_SERIAL_LEN;
    strncpy((char *) &gPmbusCmd.MFR_SERIAL[1], (char *) FRU_tbl_ptr, 15);

    //MFR_DATE -->
    memset(gPmbusCmd.MFR_DATE, '\0', sizeof(gPmbusCmd.MFR_DATE));
    FRU_tbl_ptr = (BYTE *) & FRU_table[FRU_DATE_CODE_OFFSET];
    gPmbusCmd.MFR_DATE[0] = MFR_DATE_LEN;
    //year
    year = * FRU_tbl_ptr - 0x41 + 10; //A->10
    gPmbusCmd.MFR_DATE[1] = (year / 10 + 0x30)&0xFF;
    gPmbusCmd.MFR_DATE[2] = (year % 10 + 0x30)&0xFF;
    //month
    gPmbusCmd.MFR_DATE[3] = * (FRU_tbl_ptr + 1);
    gPmbusCmd.MFR_DATE[4] = * (FRU_tbl_ptr + 2);



#    if 0
    year = (BYTE) date[0];
    if(year < 0x40)
        year = year - 0x30;
    else
        year = year - 0x41 + 10; //A->10
    //gPmbusCmd.MFR_DATE[1] = (year/10 + 0x30)&0xFF;
    gPmbusCmd.MFR_DATE[1] = (year / 10 + 0x30)&0xFF; //[Peter Chung] 20110127 modified
    gPmbusCmd.MFR_DATE[2] = (year % 10 + 0x30)&0xFF;

    month = (BYTE) date[1];
    if(month < 0x40)
        month = month - 0x30;
    else
        month = month - 0x41 + 10; //A->Oct.
    gPmbusCmd.MFR_DATE[3] = (month / 10 + 0x30)&0xFF;
    gPmbusCmd.MFR_DATE[4] = (month % 10 + 0x30)&0xFF;

    day = (BYTE) date[2];
    if(day < 0x40)
        day = day - 0x30;
    else
        day = day - 0x41 + 10; //A->10
    gPmbusCmd.MFR_DATE[5] = (day / 10 + 0x30)&0xFF;
    gPmbusCmd.MFR_DATE[6] = (day % 10 + 0x30)&0xFF;
    //MFR_DATE <--
#    endif
}

void init_MFR()
{
    WORD result;

    result = LinearFmt_XtoY(DEFAULT_MFR_VIN_MAX_AC, N_MFR_VIN_MAX, 0); //N = 0
    memcpy(gPmbusCmd.MFR_VIN_MAX, &result, 2);

    result = LinearFmt_XtoY(DEFAULT_MFR_VIN_MIN_AC, N_MFR_VIN_MIN, 0); //N = 0
    memcpy(gPmbusCmd.MFR_VIN_MIN, &result, 2);

    Parameter.MFR_VIN_MAX = DEFAULT_MFR_VIN_MAX_AC;
    Parameter.MFR_VIN_MIN = DEFAULT_MFR_VIN_MIN_AC;

#    if 0
    if(gPmbusCmd.MFR_LINE_STATUS[0].bits.IsHighLine)
    {
        result = LinearFmt_XtoY(DEFAULT_MFR_IIN_HL_MAX, N_MFR_IIN_HL_MAX, 7); //N = -7
        memcpy(gPmbusCmd.MFR_IIN_MAX, &result, 2);
        result = LinearFmt_XtoY(DEFAULT_MFR_POUT_HL_MAX_12V, N_MFR_POUT_HL_MAX_12V, 0); //N = 1
        memcpy(gPmbusCmd.MFR_POUT_MAX, &result, 2);
        result = LinearFmt_XtoY(DEFAULT_MFR_TOTAL_POUT_HL_MAX, N_MFR_TOTAL_POUT_HL_MAX, 0); //N = 1
        memcpy(gPmbusCmd.MFR_TOT_POUT_MAX, &result, 2);

    }
    else
    {
        result = LinearFmt_XtoY(DEFAULT_MFR_IIN_LL_MAX, N_MFR_IIN_LL_MAX, 7); //N = -6, Gain with 128
        memcpy(gPmbusCmd.MFR_IIN_MAX, &result, 2);
        result = LinearFmt_XtoY(DEFAULT_MFR_POUT_LL_MAX_12V, N_MFR_POUT_LL_MAX_12V, 0); //N = 1
        memcpy(gPmbusCmd.MFR_POUT_MAX, &result, 2);
        result = LinearFmt_XtoY(DEFAULT_MFR_TOTAL_POUT_LL_MAX, N_MFR_TOTAL_POUT_LL_MAX, 0); //N = 1
        memcpy(gPmbusCmd.MFR_TOT_POUT_MAX, &result, 2);

    }
#    else

    result = LinearFmt_XtoY(DEFAULT_MFR_IIN_HL_MAX_AC, N_MFR_IIN_HL_MAX, 7); //N = -7
    memcpy(gPmbusCmd.MFR_IIN_MAX, &result, 2);
    result = LinearFmt_XtoY(DEFAULT_MFR_POUT_HL_MAX_12V, N_MFR_POUT_HL_MAX_12V, 0); //N = 1
    memcpy(gPmbusCmd.MFR_POUT_MAX, &result, 2);
    result = LinearFmt_XtoY(DEFAULT_MFR_TOTAL_POUT_HL_MAX, N_MFR_TOTAL_POUT_HL_MAX, 0); //N = 1
    memcpy(gPmbusCmd.MFR_TOT_POUT_MAX, &result, 2);

#    endif

    result = LinearFmt_XtoY(DEFAULT_MFR_PIN_HL_MAX_AC, N_MFR_PIN_HL_MAX, 0); //N = 1
    memcpy(gPmbusCmd.MFR_PIN_MAX, &result, 2);

    result = DEFAULT_MFR_VOUT_MAX_12V; //N = fixed -8
    memcpy(gPmbusCmd.MFR_VOUT_MAX, &result, 2);

    result = DEFAULT_MFR_VOUT_MIN_12V; //N = fixed -8
    memcpy(gPmbusCmd.MFR_VOUT_MIN, &result, 2);

    result = LinearFmt_XtoY(DEFAULT_MFR_IOUT_MAX_12V, N_MFR_IOUT_MAX_12V, 7); //N = -3, Gain with 128
    memcpy(gPmbusCmd.MFR_IOUT_MAX, &result, 2);

    result = LinearFmt_XtoY(DEFAULT_MFR_TAMBIENT_MAX, N_MFR_TAMBIENT_MAX, 0); //N = 5
    memcpy(gPmbusCmd.MFR_TAMBIENT_MAX, &result, 2);

    result = LinearFmt_XtoY(DEFAULT_MFR_TAMBIENT_MIN, N_MFR_TAMBIENT_MIN, 0); //N = 5
    memcpy(gPmbusCmd.MFR_TAMBIENT_MIN, &result, 2);

    result = LinearFmt_XtoY(50, N_MFR_MAX_TEMP_1, 0); //N = 0
    memcpy(gPmbusCmd.MFR_MAX_TEMP_1, &result, 2);

    result = LinearFmt_XtoY(DEFAULT_TPRI_WARN, N_MFR_MAX_TEMP_2, 0); //N = 0
    memcpy(gPmbusCmd.MFR_MAX_TEMP_2, &result, 2);

    init_MFR_EFFICIENCY_LL();
    init_MFR_EFFICIENCY_HL();
    Init_MFR_SMB_ALERT_MASKING();
    RefreshMFR();

}

static void init_MFR_Specific()
{
    //initial value

    memset(gPmbusCmd.MFR_DEVICE_CODE, 0, 4);
    memset(gPmbusCmd.MFR_FW_VERSION, 0, 9);
    memset(gPmbusCmd.MFR_FIRMWARE_VER, 0, MFR_FIRMWARE_VER_LEN + 1);

    //Init POS timer
//    memset(&gPmbusCmd.MFR_POS_TOTAL.v[0], 0, 4);                        //[davidchchen]20170418 Removed
//    gPmbusCmd.MFR_POS_TOTAL.Val = UserData.Page2.region.POS.Val;        //[davidchchen]20170418 Removed
    MFR_POS_TOTAL.Val = 0;                                              //[davidchchen]20170418 Added Black Box block read
    memset(&gPmbusCmd.MFR_POS_TOTAL[0], 0, 4);                          //[davidchchen]20170418 Added Black Box block read
    gPmbusCmd.MFR_POS_TOTAL[0] = MFR_POS_TOTAL_LEN;                     //[davidchchen]20170418 Added Black Box block read
    MFR_POS_TOTAL.Val = UserData.Page2.region.POS.Val;                  //[davidchchen]20170418 Added Black Box block read
    strncpy ( ( char * ) &gPmbusCmd.MFR_POS_TOTAL[1], ( char * ) &MFR_POS_TOTAL.v[0], MFR_POS_TOTAL_LEN ); //[davidchchen]20170418 Added Black Box block read

    //gPmbusCmd.MFR_POS_LAST.Val = 0; //set defrault MFR_POS_LAST to 0, //[davidchchen]20170418 Removed
    MFR_POS_LAST.Val = 0; //set defrault MFR_POS_LAST to 0, //[davidchchen]20170418 Added Black Box block read
    memset(gPmbusCmd.MFR_POS_LAST, 0, 5);                   //[davidchchen]20170418 Added Black Box block read
    gPmbusCmd.MFR_POS_LAST[0] = MFR_POS_LAST_LEN;   //[davidchchen]20170418 Added Black Box block read

    //gPmbusCmd.BMC_UNIX_TIMESTAMP.Val = 0;   //set defrault BMC_UNIX_TIMESTAMP to 0, //[davidchchen]20170418 Removed
    BMC_UNIX_TIMESTAMP.Val = 0;                                 //[davidchchen]20170418 Added Black Box block read
    memset(gPmbusCmd.BMC_UNIX_TIMESTAMP, 0, 5);                 //set defrault BMC_UNIX_TIMESTAMP to 0, //[davidchchen]20170418 Added Black Box block read
    gPmbusCmd.BMC_UNIX_TIMESTAMP[0] = BMC_UNIX_TIMESTAMP_LEN;   //[davidchchen]20170418 Added Black Box block read

    //strncpy((char *)&MFR_DEVICE_CODE[0], (char *)DEVICE_CODE, MFR_DEVICE_CODE_LEN);//MFR_DEVICE_CODE
    gPmbusCmd.MFR_DEVICE_CODE[0] = (DEVICE_CODE >> 0) & 0xFF;
    gPmbusCmd.MFR_DEVICE_CODE[1] = (DEVICE_CODE >> 8) & 0xFF;
    gPmbusCmd.MFR_DEVICE_CODE[2] = (DEVICE_CODE >> 16) & 0xFF;
    gPmbusCmd.MFR_DEVICE_CODE[3] = (DEVICE_CODE >> 24) & 0xFF;

    gPmbusCmd.MFR_FW_VERSION[0] = MFR_FW_VERSION_LEN;
    strncpy((char *) &gPmbusCmd.MFR_FW_VERSION[1], (char *) FW_VERSION, MFR_FW_VERSION_LEN); //Firmware version
    //update bootloader version here
    strncpy((char *) &gPmbusCmd.MFR_FW_VERSION[2], (char *) &UserData.Page2.region.BL_VER, MFR_BL_VERSION_LEN); //Bootloader version

    gPmbusCmd.MFR_FIRMWARE_VER[0] = MFR_FIRMWARE_VER_LEN;
    strncpy((char *) &gPmbusCmd.MFR_FIRMWARE_VER[1], (char *) LITEON_FW_VERSION, MFR_FIRMWARE_VER_LEN); //Firmware version for Liteon use only
    gPmbusCmd.MFR_PAGE[0] = 0xFF; //MFR_PAGE
    init_SmbAlertMask();

    //no initial value
    memset(&gPmbusCmd.MFR_ISP_KEY[0], 0, 4); //set defrault MFR_ISP_KEY to 0
    memset(&gPmbusCmd.MFR_ISP_STATUS_CMD[0], 0, 1); //set defrault MFR_ISP_STATUS_CMD to 0
    memset(&gPmbusCmd.MFR_LINE_STATUS[0], 0, 1); //set defrault MFR_LINE_STATUS to 0
    memset(&gPmbusCmd.MFR_SYSTEM_LED_CNTL[0], 0, 1); //set defrault MFR_SYSTEM_LED_CNTL to 0

    if(! PS.PriFwUpdated)
    {
        //strncpy ( ( char * ) &gPmbusCmd.MFR_FW_VERSION[4], ( char * ) &UserData.Page2.region.PriFwRev.Val, MFR_PRI_VERSION_LEN ); //Primary FW version
        strncpy((char *) &gPmbusCmd.MFR_FW_VERSION[3], (char *) &UserData.Page2.region.PriFwRev.Val, MFR_PRI_VERSION_LEN); //Primary FW version, [davidchchen]2014/12/11 modify
    }
}

void CheckLineStatus(WORD vin, BYTE freq)
{
    static BYTE previousLine = 0;
    //static BYTE previousInput = 0;
    static BYTE flag_AC_Present = FALSE;
    WORD vin_ov_fault;
    WORD vin_ov_warning;
    WORD vin_ov_recover;
    WORD vin_uv_fault;
    WORD vin_uv_warning;
    WORD vin_uv_recover;

    //bit 0 : No AC or not
    if(vin < 50)
    {
        gPmbusCmd.MFR_LINE_STATUS[0].bits.NoAC = 1;
        gPmbusCmd.MFR_LINE_STATUS[0].bits.IsHighLine = 0;
        gPmbusCmd.MFR_LINE_STATUS[0].bits.Freq60Hz = 0;
    }
    else
    {

        if(PS.isDCInput == 1)
        {
            gPmbusCmd.MFR_LINE_STATUS[0].Val = 0x07;
        }
        else
        {

            gPmbusCmd.MFR_LINE_STATUS[0].bits.NoAC = 0;

            //bit 1 : High / Low Line
            if(vin > 160)
            {
                //High line
                gPmbusCmd.MFR_LINE_STATUS[0].bits.IsHighLine = 1;

                //-------------------->	[Tommy YR Chen] 20110218 added
                if(vin > 170)
                {
                    if(gPmbusCmd.MFR_PFC_DISABLE.bits.b1 == 1 && gPS_State.mainState == STATE_STANDBY)
                    {
                        StatusToPrimary.bits.PFC_Disable = TRUE;
                    }
                    else
                    {
                        StatusToPrimary.bits.PFC_Disable = FALSE;
                    }
                }
                //<--------------------	[Tommy YR Chen] 20110218 added

            }
            else
            {
                //Low line
                gPmbusCmd.MFR_LINE_STATUS[0].bits.IsHighLine = 0;
                //[Peter Chung] 20101209 added for turn on PFC when PSU is in LL
                StatusToPrimary.bits.PFC_Disable = FALSE;
            }

            //bit 2 : Vin Frequency 50Hz or 60 Hz
            if(freq > 55)
            {
                gPmbusCmd.MFR_LINE_STATUS[0].bits.Freq60Hz = 1;
            }
            else
            {
                gPmbusCmd.MFR_LINE_STATUS[0].bits.Freq60Hz = 0;
            }
        }
    }

#    if 0
    if(previousInput != Uart.U1.Rx.Pri_Status.bits.DC_INPUT)
    {

        if(Uart.U1.Rx.Pri_Status.bits.DC_INPUT == 0)
        {
            Parameter.VIN_OV_WARN_LIMIT = DEFAULT_VIN_OV_WARN_AC;
            Parameter.VIN_OV_FAULT_LIMIT = DEFAULT_VIN_OV_FAULT_AC;
            Parameter.VIN_OV_FAULT_RECOVER_LIMIT = DEFAULT_VIN_OV_RECOVER_AC;
            Parameter.VIN_UV_WARN_LIMIT = DEFAULT_VIN_UV_WARN_AC;
            Parameter.VIN_UV_FAULT_LIMIT = DEFAULT_VIN_UV_FAULT_AC;
            Parameter.VIN_UV_FAULT_RECOVER_LIMIT = DEFAULT_VIN_UV_RECOVER_AC;
            Parameter.MFR_VIN_MAX = DEFAULT_MFR_VIN_MAX_AC;
            Parameter.MFR_VIN_MIN = DEFAULT_MFR_VIN_MIN_AC;
        }
        else
        {
            Parameter.VIN_OV_WARN_LIMIT = DEFAULT_VIN_OV_WARN_DC;
            Parameter.VIN_OV_FAULT_LIMIT = DEFAULT_VIN_OV_FAULT_DC;
            Parameter.VIN_OV_FAULT_RECOVER_LIMIT = DEFAULT_VIN_OV_RECOVER_DC;
            Parameter.VIN_UV_WARN_LIMIT = DEFAULT_VIN_UV_WARN_DC;
            Parameter.VIN_UV_FAULT_LIMIT = DEFAULT_VIN_UV_FAULT_DC;
            Parameter.VIN_UV_FAULT_RECOVER_LIMIT = DEFAULT_VIN_UV_RECOVER_DC;
            Parameter.MFR_VIN_MAX = DEFAULT_MFR_VIN_MAX_DC;
            Parameter.MFR_VIN_MIN = DEFAULT_MFR_VIN_MIN_DC;
        }

        previousInput = Uart.U1.Rx.Pri_Status.bits.DC_INPUT;
    }
#    endif

    //[Peter Chung] 20101222 added
    //Check VIN_UV_WARNING here

    if(PS.isDCInput == 0)
    {
        if(vin > 180)
        {
            flag_AC_Present = TRUE;
        }
        else if(vin < 60)
        {
            flag_AC_Present = FALSE;
        }
        vin_ov_fault = Parameter.VIN_OV_FAULT_LIMIT[0];
        vin_ov_warning = Parameter.VIN_OV_WARN_LIMIT[0];
        vin_ov_recover = Parameter.VIN_OV_FAULT_RECOVER_LIMIT[0];
        vin_uv_fault = Parameter.VIN_UV_FAULT_LIMIT[0];
        vin_uv_warning = Parameter.VIN_UV_WARN_LIMIT[0];
        vin_uv_recover = Parameter.VIN_UV_FAULT_RECOVER_LIMIT[0];
    }
    else
    {
        if(vin > 190)
        {
            flag_AC_Present = TRUE;
        }
        else if(vin < 60)
        {
            flag_AC_Present = FALSE;
        }
        vin_ov_fault = Parameter.VIN_OV_FAULT_LIMIT[1];
        vin_ov_warning = Parameter.VIN_OV_WARN_LIMIT[1];
        vin_ov_recover = Parameter.VIN_OV_FAULT_RECOVER_LIMIT[1];
        vin_uv_fault = Parameter.VIN_UV_FAULT_LIMIT[1];
        vin_uv_warning = Parameter.VIN_UV_WARN_LIMIT[1];
        vin_uv_recover = Parameter.VIN_UV_FAULT_RECOVER_LIMIT[1];
    }

#    if 1
    {
        //FW V42 Source code
        //static BYTE count = 0;

        //david add
        static WORD count = 0;
        static WORD dc_count = 0; //[davidchchen]20150722 ADDED

        if(PS.U2RX_CommOK && (vin < vin_uv_warning) /*&& (iAC_OK == AC_GOOD)*/ /*&& (flag_AC_Present == TRUE)*/ /*&& (iPS_OK == P_OK)*/)
        {
            //FW V42 Source code
            //if ( count >= 2 )

            //david add
            //          if ( count >= 100 )
            //          {
            //              if ( PS.isDCInput == 0 )
            //              {
            //                  gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.bits.VIN_UV_WARNING = 1;
            //                  gInputLedStatus.bits.vin_uv_warning = 1;
            //              }
            //              else
            //              {
            //                  if ( PS.AC_ON_CheckLineStatus )
            //                  {
            //                      gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.bits.VIN_UV_WARNING = 1;
            //                      gInputLedStatus.bits.vin_uv_warning = 1;
            //                  }
            //              }
            //          }
            //          else
            //          {
            //              count ++;
            //          }

            //[davidchchen]20150722 ADDED
            if(PS.isDCInput == 0)
            {
                dc_count = 0;
                if(count >= DEFAULT_VIN_AC_UV_TIME)
                {
                    gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.bits.VIN_UV_WARNING = 1;
                    gInputLedStatus.bits.vin_uv_warning = 1;
                }
                else
                {
                    count ++;
                }
            }
            else if(PS.isDCInput == 1)
            {
                count = 0;
                //if ( dc_count >= 100 )
                if(dc_count >= DEFAULT_VIN_DC_UV_TIME) //[davidchchen]20150924 modify
                {
                    if(PS.AC_ON_CheckLineStatus) //[davidchchen]20160826 Hot plugging Issue
                    {
                        gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.bits.VIN_UV_WARNING = 1;
                        gInputLedStatus.bits.vin_uv_warning = 1;
                    }
                }
                else
                {
                    dc_count ++;
                }
            }

        }
#            if 0
        else if((flag_AC_Present == FALSE) && (vin < vin_uv_warning))
        {
            // For the condition which's in redundant mode, and the PSU without input should show VIN_UV_WARN in both page.
            if(PS.VinUVWarnDetected == 0)
            {
                Protect.VinUVWarning.Flag = 1;
            }
            else
            {
                if(PS.isDCInput == 0)
                {
                    gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.bits.VIN_UV_WARNING = 1;
                    gInputLedStatus.bits.vin_uv_warning = 1;
                }
                else
                {
                    if(PS.AC_ON_CheckLineStatus)
                    {
                        gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.bits.VIN_UV_WARNING = 1;
                        gInputLedStatus.bits.vin_uv_warning = 1;
                    }
                }
            }
        }
#            endif
        else
        {
            count = 0;
            dc_count = 0; //[davidchchen]20150722 ADDED
#            if 0
            Protect.VinUVWarning.Flag = 0;
            Protect.VinUVWarning.delay = 0;
#            endif
            gInputLedStatus.bits.vin_uv_warning = 0;
        }
    }
#    endif

#    if 0	// [Tommy YR Chen] 20130204 debug
    {
        static BYTE led_count = 0;

        if(PS.U2RX_CommOK && ((vin < Parameter.VIN_UV_WARN_LIMIT) || (vin < Parameter.VIN_UV_FAULT_LIMIT)) && (iAC_OK == AC_GOOD) && (iPS_OK == P_OK))
        { // [Tommy YR Chen] 20131121 modified
            if(led_count >= 2)
            {
                gLedWarningStatus.bits.vin_uv_warning = 1;
            }
            else
            {
                led_count ++;
            }
        }
        else if(PS.U2RX_CommOK && (vin > 175) && (iAC_OK == AC_GOOD) && (iPS_OK == P_OK))
        { // [Tommy YR Chen] 20131121 modified
            led_count = 0;
            gLedWarningStatus.bits.vin_uv_warning = 0;
        }
        else if(iAC_OK == AC_N_GOOD)
        {
            led_count = 0;
            gLedWarningStatus.bits.vin_uv_warning = 0;
        }
    }
#    endif

    //FW V42 Source code
    //  if ( /*(flag_AC_Present == TRUE) &&*/ ( vin < vin_uv_fault ) &&
    //       ( ( _SD_Flag.STB_OCP == 0 ) && ( _SD_Flag.STB_OVP == 0 ) && ( _SD_Flag.STB_UVP == 0 ) && ( _SD_Flag.STB_OTP == 0 ) ) /*&& (AcOffBlankingTime >= 5)*/ )
    //  {
    //      if ( PS.isDCInput == 0 )
    //      {
    //          gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.bits.VIN_UV_FAULT = 1; //[Peter Chung] 20101222 added
    //          PS.VinUVFault = 1;
    //          gInputLedStatus.bits.vin_uv_fault = 1;
    //      }
    //      else
    //      {
    //          if ( PS.AC_ON_CheckLineStatus )
    //          {
    //              gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.bits.VIN_UV_FAULT = 1;
    //              PS.VinUVFault = 1;
    //              gInputLedStatus.bits.vin_uv_fault = 1;
    //          }
    //      }
    //  }
    //  else if ( vin >= vin_uv_recover )
    //  {
    //      PS.VinUVFault = 0;
    //      gInputLedStatus.bits.vin_uv_fault = 0;
    //  }
    //  else
    //  {
    //      // For the condition which's in redundant mode, and the PSU without input should show VIN_UV_FAULT in both page.
    //#if 0
    //      if ( ( flag_AC_Present == FALSE ) && ( vin < vin_uv_fault ) )
    //      {
    //          if ( PS.VinUVFaultDetected == 0 )
    //          {
    //              Protect.VinUVFault.Flag = 1;
    //          }
    //          else
    //          {
    //              if ( PS.isDCInput == 0 )
    //              {
    //                  gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.bits.VIN_UV_FAULT = 1; //[Peter Chung] 20101222 added
    //                  PS.VinUVFault = 1;
    //                  gInputLedStatus.bits.vin_uv_fault = 1;
    //              }
    //              else
    //              {
    //                  if ( PS.AC_ON_CheckLineStatus )
    //                  {
    //                      gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.bits.VIN_UV_FAULT = 1;
    //                      PS.VinUVFault = 1;
    //                      gInputLedStatus.bits.vin_uv_fault = 1;
    //                  }
    //              }
    //          }
    //      }
    //      else
    //      {
    //          PS.VinUVFaultDetected = 0;
    //          Protect.VinUVFault.Flag = 0;
    //          Protect.VinUVFault.delay = 0;
    //      }
    //#endif
    //  }

    //david add
    {
        static WORD uv_fault_count = 0;
        static WORD dc_uv_fault_count = 0; //[davidchchen]20150911 ADDED
        static BYTE uv_recovery_count = 0; //[davidchchen]20160408 added , debug for DC_Brown Issue

        //if ( /*(flag_AC_Present == TRUE) &&*/ PS.U2RX_CommOK == TRUE && ( vin < vin_uv_fault ) && ( ( _SD_Flag.STB_OCP == 0 ) && ( _SD_Flag.STB_OVP == 0 ) && ( _SD_Flag.STB_UVP == 0 ) && ( _SD_Flag.STB_OTP == 0 ) ) /*&& (AcOffBlankingTime >= 5)*/ )
        if(/*(flag_AC_Present == TRUE) &&*/ PS.U2RX_CommOK == TRUE && (vin < vin_uv_fault) && ((_SD_Flag.STB_OCP == 0) && (_SD_Flag.STB_OVP == 0) && (_SD_Flag.STB_UVP == 0)) /*&& (AcOffBlankingTime >= 5)*/) //[David ch chen]20141124 removed _SD_Flag.STB_OTP, not use _SD_Flag.STB_OTP
        {
            //          if ( uv_fault_count >= 100 )
            //          {
            //              if ( PS.isDCInput == 0 )
            //              {
            //                  gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.bits.VIN_UV_FAULT = 1; //[Peter Chung] 20101222 added
            //                  PS.VinUVFault = 1;
            //                  gInputLedStatus.bits.vin_uv_fault = 1;
            //              }
            //              else
            //              {
            //                  if ( PS.AC_ON_CheckLineStatus )
            //                  {
            //                      gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.bits.VIN_UV_FAULT = 1;
            //                      PS.VinUVFault = 1;
            //                      gInputLedStatus.bits.vin_uv_fault = 1;
            //                  }
            //              }
            //          }
            //          else
            //          {
            //              uv_fault_count ++;
            //          }

            //[davidchchen]20150911 ADDED
            if(PS.isDCInput == 0)
            {

                dc_uv_fault_count = 0;
                uv_recovery_count = 0; //[davidchchen]20160408 added , debug for DC_Brown Issue
                if(uv_fault_count >= DEFAULT_VIN_AC_UV_TIME)
                {
                    gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.bits.VIN_UV_FAULT = 1; //[Peter Chung] 20101222 added
                    PS.VinUVFault = 1;
                    gInputLedStatus.bits.vin_uv_fault = 1;
                }
                else
                {
                    uv_fault_count ++;
                }
            }
            else if(PS.isDCInput == 1)
            {
                uv_fault_count = 0;
                uv_recovery_count = 0; //[davidchchen]20160408 added , debug for DC_Brown Issue
                //if ( dc_uv_fault_count >= 100 )
                if(dc_uv_fault_count >= DEFAULT_VIN_DC_UV_TIME) //[davidchchen]20150924 modify
                {
                    if(PS.AC_ON_CheckLineStatus) //[davidchchen]20160826 Hot plugging Issue
                    {
                        gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.bits.VIN_UV_FAULT = 1;
                        PS.VinUVFault = 1;
                        gInputLedStatus.bits.vin_uv_fault = 1;
                    }
                }
                else
                {
                    dc_uv_fault_count ++;
                }
            }

        }
        else if(vin >= vin_uv_recover)
        {
            if(uv_recovery_count >= 10) //[davidchchen]20160408 added , debug for DC_Brown Issue
            {
                PS.VinUVFault = 0;
                gInputLedStatus.bits.vin_uv_fault = 0;
                uv_fault_count = 0;
                dc_uv_fault_count = 0; //[davidchchen]20150911 ADDED
                if ( PS.isDCInput == FALSE )                                              //[davidchchen]20170216 Modify AC Recovery issue.
                {
                      gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.Val = gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.Val & 0xC0;             //[davidchchen]20170216 Modify AC Recovery issue.
                }
                else                                                                    //[davidchchen]20170216 Modify AC Recovery issue.
                {
                      gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.Val = gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.Val & 0xC0;             //[davidchchen]20170216 Modify AC Recovery issue.
                }
            }
            else
                uv_recovery_count ++; //[davidchchen]20160408 added , debug for DC_Brown Issue

            //[davidchchen]20160408 removed , debug for DC_Brown Issue
            //          PS.VinUVFault = 0;
            //          gInputLedStatus.bits.vin_uv_fault = 0;
            //          uv_fault_count = 0;
            //          dc_uv_fault_count = 0;    //[davidchchen]20150911 ADDED

        }
        else
        {
            uv_recovery_count = 0; //[davidchchen]20160408 added , debug for DC_Brown Issue
            uv_fault_count = 0;
            dc_uv_fault_count = 0; //[davidchchen]20150911 ADDED
            //
        }
    }

    // Check Vin OV Warn & Fault
    if(vin > vin_ov_warning)
    {
        if(PS.isDCInput == 0)
        {
            gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.bits.VIN_OV_WARNING = 1;
        }
        else
        {
            gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.bits.VIN_OV_WARNING = 1;
        }
        gInputLedStatus.bits.vin_ov_warning = 1;
    }
    else
    {
        gInputLedStatus.bits.vin_ov_warning = 0;
    }

    if(vin > vin_ov_fault)
    {
        if(PS.isDCInput == 0)
        {
            gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.bits.VIN_OV_FAULT = 1;
        }
        else
        {
            gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.bits.VIN_OV_FAULT = 1;
        }
        PS.VinOVFault = 1;
        gInputLedStatus.bits.vin_ov_fault = 1;
    }
    else if(vin < vin_ov_recover)
    {
        PS.VinOVFault = 0;
        gInputLedStatus.bits.vin_ov_fault = 0;
        if ( PS.isDCInput == FALSE )                                              //[davidchchen]20170216 Modify AC Recovery issue.
        {
              gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.Val = gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.Val & 0X30;             //[davidchchen]20170216 Modify AC Recovery issue.
        }
        else                                                                    //[davidchchen]20170216 Modify AC Recovery issue.
        {
              gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.Val = gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.Val & 0X30;             //[davidchchen]20170216 Modify AC Recovery issue.
        }

    }

    if(iAC_OK == AC_GOOD || (PS.U2RX_CommOK == TRUE && Real_Vac > 60))
    {
        if(PS.isDCInput == 0)
        {
            gPmbusCmd.READ_FREQUENCY[0] = Uart.U2.Rx.Freq;
            gPmbusCmd.READ_FREQUENCY[1] = 0x00;
        }
        else
        {
            gPmbusCmd.READ_FREQUENCY[0] = 0x00;
            gPmbusCmd.READ_FREQUENCY[1] = 0x00;
        }
    }
    else
    {
        gPmbusCmd.READ_FREQUENCY[0] = 0x00;
        gPmbusCmd.READ_FREQUENCY[1] = 0x00;
    }

    previousLine = gPmbusCmd.MFR_LINE_STATUS[0].bits.IsHighLine;
}

static void RefreshFRUBuf()
{
    WORD offset;

    offset = gPmbusCmd.FRU_WRITE[1] | (gPmbusCmd.FRU_WRITE[2] << 8);

    gPmbusCmd.FRU_READ[0] = FRU_READ_LEN;
    //memcpy(&gPmbusCmd.FRU_READ[1], &UserData.Page1.region.FRU[offset], FRU_READ_LEN);   //[davidchchen]20170921 Removed
    memcpy(&gPmbusCmd.FRU_READ[1], &UserData.Page1.region.FRU.frudata[offset], FRU_READ_LEN);   //[davidchchen]20170921 Added
    
}

static void CollectFRU()
{
    static BYTE fullfru[FRU_BUFFER_SIZE];
    static WORD size = 0;

    UpdateDataBuf(); //[Peter Chung] 20100924 added
    //collect fru data sent from AP side
    memcpy((fullfru + size), gPmbusCmd.FRU_SLICE_BUF, FRU_SLICE_BUF_SIZE);
    size += FRU_SLICE_BUF_SIZE;

    if(size == FRU_BUFFER_SIZE)
    {
        //reset size
        size = 0;
        //update user data
        //memcpy(UserData.Page1.region.FRU, fullfru, FRU_BUFFER_SIZE);    //[davidchchen]20170921 Removed
        memcpy(UserData.Page1.region.FRU.frudata, fullfru, FRU_BUFFER_SIZE);    //[davidchchen]20170921 Added

        PS.FlashWritePage1 = TRUE;          //[davidchchen]20170921 added
        RefreshMFR();                       //[davidchchen]20170921 added
        RefreshFRUBuf();                    //[davidchchen]20170921 added

        //write fru data to Program memory

        //      PS.FlashWritePage1 = TRUE;  //[davidchchen] 20150113 removed
        //      RefreshMFR ( );             //[davidchchen] 20150113 removed
        //      RefreshFRUBuf ( );          //[davidchchen] 20150113 removed

        //[davidchchen] 20150113 Added Compatible Code Function
        //if ( (UserData.Page1.data[0] == HW_CompatibleCodeStr[0]) && (UserData.Page1.data[1] == HW_CompatibleCodeStr[1] ) )

        //[davidchchen]20170921 Removed
        /*
        if((UserData.Page1.region.HW_CC_HB == tsRevCtrlBlock.pu8HwCompCode[0]) && (UserData.Page1.region.HW_CC_LB == tsRevCtrlBlock.pu8HwCompCode[1]))
        {
            PS.Compatible_code = 0;
            _SD_Flag.OPP_SD = 0;
            gPmbusCmd.MFR_RCB_INFO[MFR_RCB_INFO_LEN] = 0; //HW compatible code conflict, 0: not conflict, 2: conflict
            PS.FlashWritePage1 = TRUE;
            RefreshMFR();
            RefreshFRUBuf();
        }
        else
        {
            PS.FlashWritePage1 = FALSE;
            PS.Compatible_code = 1;
            _SD_Flag.OPP_SD = 1;
            gPmbusCmd.MFR_RCB_INFO[MFR_RCB_INFO_LEN] = 2; //HW compatible code conflict, 0: not conflict, 2: conflict
        }
        */

    }

}

//[davidchchen]20170711 Removed, not used
//BYTE IsISPKeyMatched()
//{
//    if(! strncmp(((char *) DELL_ISP_KEY), ((char *) (gPmbusCmd.MFR_ISP_KEY)), 4) ||
//       ! strncmp(((char *) DELL_FACTORY_MODE_KEY), ((char *) (gPmbusCmd.MFR_ISP_KEY)), 4) ||
//       ! strncmp(((char *) LITEON_FACOTRY_MODE_KEY), ((char *) (gPmbusCmd.MFR_ISP_KEY)), 4))
//    {
//
//        return TRUE;
//    }
//    else
//    {
//        return FALSE;
//    }
//}

//[davidchchen]20170216 added PSON Signal Enable/disable
static void CMD_MFR_PSON_CONTROL_Handler()
{
    BYTE result;
    result = I2C.readBuffer[0];
    UpdateDataBuf();                            //[davidchchen]20170216 added PSON Signal Enable/disable

    //if(I2C1ADD == 0x58)                                   //[davidchchen]20170921 Removed PSON Signal Enable/disable
    if ( ( I2C1ADD != 0x5E ) ||  ( I2C1ADD != 0x5F ) )      //[davidchchen]20170921 added PSON Signal Enable/disable
    {
        if(result == MFR_PSON_DISABLE )      //disable PSON  //[davidchchen]20170216 added PSON Signal Enable/disable
        {
            PS.MFR_PSON_CONTROL = MFR_PSON_DISABLE;     //[davidchchen]20170216 added PSON Signal Enable/disable
            
        }
        else if(result == MFR_PSON_ENABLE )      //disable PSON  //[davidchchen]20170216 added PSON Signal Enable/disable
        {
            PS.MFR_PSON_CONTROL = MFR_PSON_ENABLE;     //[davidchchen]20170216 added PSON Signal Enable/disable

        }
        else
        {
            gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
        }
    }
    else
    {
        PS.MFR_PSON_CONTROL = MFR_PSON_ENABLE;
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
    }

}

static void VerifyISPKey()
{
    WORD temp;

    UpdateDataBuf();
    if(! strncmp(((char *) DELL_ISP_KEY), ((char *) (gPmbusCmd.MFR_ISP_KEY)), 4))
    {
        gISP.ISP_Key_Unlock = TRUE;
        gPmbusCmd.MFR_ISP_STATUS_CMD[0] &= ~ Def_Bit3; //clear bKeyErr
    }
    else if(! strncmp(((char *) DELL_FACTORY_MODE_KEY), ((char *) (gPmbusCmd.MFR_ISP_KEY)), 4))
    {
        //Update PSU_FACTORY_MODE buffer
        gPmbusCmd.PSU_FACTORY_MODE[0].bits.FactoryMode = 1;
        //Set OCP to 115A
#        if 1	//[Peter Chung] 20110217 removed
        temp = LinearFmt_XtoY(FACTORY_IOUT_OC_FAULT, - 2, 0);
        gPmbusCmd.IOUT_OC_FAULT_LIMIT[0] = temp & 0xFF;
        gPmbusCmd.IOUT_OC_FAULT_LIMIT[1] = (temp >> 8) & 0xFF;
        Parameter.OCP_IOUT_OC_FAULT_LIMIT = (WORD) (((DWORD) FACTORY_IOUT_OC_FAULT * IOUT_TO_ADC) >> 10); //ADC
#        endif
    }
    else if(! strncmp(((char *) LITEON_FACOTRY_MODE_KEY), ((char *) (gPmbusCmd.MFR_ISP_KEY)), 4))
    {
        gIsFactoryMode = TRUE;
        //Update PSU_FACTORY_MODE buffer
        gPmbusCmd.PSU_FACTORY_MODE[0].bits.FactoryMode = 1;
        gPmbusCmd.PSU_FACTORY_MODE[0].bits.FruAccessControl = FRU_RW_GRANTED;
        gPmbusCmd.PSU_FACTORY_MODE[0].bits.MfrSampleSetControl = 1;
        gPmbusCmd.PSU_FACTORY_MODE[0].bits.BlackBoxClrControl = 1;
        //Set OCP to 115A
#        if 1	//[Peter Chung] 20110217 removed
        temp = LinearFmt_XtoY(FACTORY_IOUT_OC_FAULT, - 2, 0);
        gPmbusCmd.IOUT_OC_FAULT_LIMIT[0] = temp & 0xFF;
        gPmbusCmd.IOUT_OC_FAULT_LIMIT[1] = (temp >> 8) & 0xFF;
        Parameter.OCP_IOUT_OC_FAULT_LIMIT = (WORD) (((DWORD) FACTORY_IOUT_OC_FAULT * IOUT_TO_ADC) >> 10); //ADC
#        endif
    }
    else
    {
        gISP.ISP_Key_Unlock = FALSE;
        //[Peter Chung] 20101228 added
        gPmbusCmd.MFR_ISP_STATUS_CMD[0] |= Def_Bit3; //set bKeyErr
    }
}

static void ISPStatusCmd()
{
    UpdateDataBuf();
    switch(gPmbusCmd.MFR_ISP_STATUS_CMD[0])
    {
        case 0x00://cmdClearStat
            break;
        case 0x01://cmdResetSeq
            break;
        case 0x02://cmdBootISP
            if(gISP.ISP_Key_Unlock)
            {
                _SD_Flag.ISP_MODE = TRUE;
            }
            break;
        case 0x03://cmdBootPM
            break;
        default:
            break;
    }
#    if 0	//[Peter Chung] 20101228 removed
    gPmbusCmd.MFR_ISP_STATUS_CMD[0] = 0;
    if(! gISP.ISP_Key_Unlock)
    {
        gPmbusCmd.MFR_ISP_STATUS_CMD[0] |= Def_Bit3; //bKeyErr
    }
    else
    {
        gPmbusCmd.MFR_ISP_STATUS_CMD[0] &= ~ Def_Bit3; //bKeyErr
    }
#    endif
}

static void CMD_SYSTEM_LED_CNTL_Handler()
{
    UpdateDataBuf();
    //gLedStatus.SWAP_LED_STATUS = gPmbusCmd.MFR_SYSTEM_LED_CNTL[0];

    //For 11G Led control
#    if 0
    if(! (_SD_Flag.Val & 0x00007B9F))
    {
        Led_SYS_Control(gPmbusCmd.MFR_SYSTEM_LED_CNTL[0]);
    }
#    endif
}

static void CMD_PFC_DISABLE_Handler()
{
#    if 1

    tPFC_DISABLE pfcCntlByte;

    //[Peter Chung] 20110419 added
    if((I2C.readBuffer[0] != 0x00) && (I2C.readBuffer[0] != 0x02))
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
        return;
    }

    UpdateDataBuf();
    pfcCntlByte.Val = gPmbusCmd.MFR_PFC_DISABLE.Val;

    if(pfcCntlByte.bits.PFC_Control == PFC_CONTROL_DISABLE)
    {
        if(gPmbusCmd.MFR_LINE_STATUS[0].bits.IsHighLine)
        {
            //[Peter Chung] 20101209 remove "is not ROA status"
            //if(gPS_State.mainState == STATE_STANDBY && gPmbusCmd.MFR_ROA_CONTROL[0].bits.ROACntl != ROA_ACTIVE){
            if(gPS_State.mainState == STATE_STANDBY)
            {
                StatusToPrimary.bits.PFC_Disable = TRUE;
            }
            else
            {
                StatusToPrimary.bits.PFC_Disable = FALSE;
            }
        }
        else
        {
            StatusToPrimary.bits.PFC_Disable = FALSE;
        }
    }
    else if(pfcCntlByte.bits.PFC_Control == PFC_CONTROL_ENABLE)
    {
        StatusToPrimary.bits.PFC_Disable = FALSE;
    }
    else
    {
        //Do nothing
    }

    //Update PFC status
    gPmbusCmd.MFR_PFC_DISABLE.Val = pfcCntlByte.Val;

#    endif
}
//[davidchchen]20170310 Added BMC_UNIX_TIMESTAMP cmd
static void CMD_BMC_UNIX_TIMESTAMP_Handler()
{
    UpdateDataBuf();
    if (gPmbusCmd.BMC_UNIX_TIMESTAMP[0] == 0x04 )                   //[davidchchen]20170418 Added Black Box block read
    {
        BMC_UNIX_TIMESTAMP.v[0] = gPmbusCmd.BMC_UNIX_TIMESTAMP[1] ; //[davidchchen]20170418 Added Black Box block read
        BMC_UNIX_TIMESTAMP.v[1] = gPmbusCmd.BMC_UNIX_TIMESTAMP[2] ; //[davidchchen]20170418 Added Black Box block read
        BMC_UNIX_TIMESTAMP.v[2] = gPmbusCmd.BMC_UNIX_TIMESTAMP[3] ; //[davidchchen]20170418 Added Black Box block read
        BMC_UNIX_TIMESTAMP.v[3] = gPmbusCmd.BMC_UNIX_TIMESTAMP[4] ; //[davidchchen]20170418 Added Black Box block read
    }
    else
    {
        gPmbusCmd.BMC_UNIX_TIMESTAMP[0] = 0x04;                                     //[davidchchen]20170418 Added Black Box block read
    }
}

static void CMD_MFR_CLEAR_HISTORY_Handler()
{
#    if BLACKBOX_SUPPORTED
#    if 0
    if(gPmbusCmd.PSU_FACTORY_MODE[0].bits.BlackBoxClrControl)
    {
        memset(UserData.Page2.region.BlackBox.FaultRecord, 0, sizeof(UserData.Page2.region.BlackBox));
        gPmbusCmd.PSU_FACTORY_MODE[0].bits.BlackBoxClrControl = 0;
    }
    else
    {
        gPagePlusStatus.PAGE[PAGE0].STATUS_CML.bits.INVALID_DATA = 1;
        gPagePlusStatus.PAGE[PAGE1].STATUS_CML.bits.INVALID_DATA = 1;
    }
#    endif

    memset(UserData.Page2.region.BlackBox.FaultRecord, 0, sizeof(UserData.Page2.region.BlackBox));

    UserData.Page3.region.logGeneral.Log_Sum = 0;
    UserData.Page3.region.logGeneral.Latest_Log_Index = 0;
    UserData.Page3.region.logIndex = 0;
    memset(&UserData.Page3.region.logContent[0], 0, sizeof(tLOG_CONTENT) * MAX_LOG_COUNT);

    gPmbusCmd.PSU_FACTORY_MODE[0].bits.BlackBoxClrControl = 0;

#    endif
}

static void CMD_CAL_VOUT_OFFSET_Handler()
{
    SHORT offset;
    SHORT offsetlimit;

    UpdateDataBuf();

    //[Peter Chung] 20110901 added for avoid the wrong calibration
    if((gPS_State.mainState == STATE_NORMAL) || (gPS_State.mainState == STATE_ONING))
    {
        offset = gVoutCmdOffset + ((SHORT) (gPmbusCmd.CAL_VOUT_OFFSET[0] | (gPmbusCmd.CAL_VOUT_OFFSET[1] << 8)) << 1);
        if((offset < 1000) && (offset > - 1000))
        {
            gVoutCmdOffset += ((SHORT) gPmbusCmd.CAL_VOUT_OFFSET[0] | (gPmbusCmd.CAL_VOUT_OFFSET[1] << 8)) << 1; //translate to duty cycle, Tommy
            gVoutReadOffsetAdc += (SHORT) ((((LONG) (gPmbusCmd.CAL_VOUT_OFFSET[2] | (gPmbusCmd.CAL_VOUT_OFFSET[3] << 8))) - ((SHORT) gPmbusCmd.CAL_VOUT_OFFSET[0] | (gPmbusCmd.CAL_VOUT_OFFSET[1] << 8))) * VOUT_TO_ADC / 1000);
            //gMainBusVoutOffset += MAINBUS_12V_ADC - ((SHORT)PSFB.Main_Output_Voltage_FF + (SHORT)(((LONG)(gPmbusCmd.CAL_VOUT_OFFSET[0] | (gPmbusCmd.CAL_VOUT_OFFSET[1] << 8)) * MAINBUS_VOUT_TO_ADC) >> MAINBUS_GAIN));

#            if RS_ENABLE   //[davidchchen] 20150108 not used
            gMainBusVoutOffset = (SHORT) ADC.Vout_FF - (SHORT) ADC.VoutBus_FF;
#            endif

            UserData.Page1.region.Vout[OUTPUT_12V].CmdOffset = gVoutCmdOffset;
            UserData.Page1.region.Vout[OUTPUT_12V].ReportOffset = gVoutReadOffsetAdc;
#            if RS_ENABLE   //[davidchchen] 20150108 not used
            UserData.Page1.region.Vout[OUTPUT_12V].MainBusOffset = gMainBusVoutOffset;
#            endif

            UserData.Page2.region.Calibration_Flag.bits.b0 = 1;
        }

        offset = gStbVoutOffset + (gPmbusCmd.CAL_VOUT_OFFSET[6] | (gPmbusCmd.CAL_VOUT_OFFSET[7] << 8));
        if((offset < 1000) && (offset > - 1000))
        {
            gStbVoutOffset += (gPmbusCmd.CAL_VOUT_OFFSET[6] | (gPmbusCmd.CAL_VOUT_OFFSET[7] << 8));
            offsetlimit = SPHASE3;
            if(gStbVoutOffset >= offsetlimit)
            {
                gStbVoutOffset = offsetlimit;
            }
            if(gStbVoutOffset <= 0)
            {
                gStbVoutOffset = 0;
            }
            UserData.Page1.region.StbVoutOffset = gStbVoutOffset;
            SDC3 = ((SHORT) SPHASE3 - gStbVoutOffset);
            UserData.Page2.region.Calibration_Flag.bits.b1 = 1;
        }
    }

    RefreshAllCaliInfo();
    //Flash need to be written
    PS.FlashWritePage1 = TRUE;
}

static void CMD_PSU_FACTORY_MODE_Handler()
{
#    if 0
    if(! strncmp(((char *) FACOTRY_MODE_KEY), ((char *) I2C.readBuffer), 5))
    {
        gIsFactoryMode = TRUE;
        //Update PSU_FACTORY_MODE buffer
        gPmbusCmd.PSU_FACTORY_MODE[0].bits.FactoryMode = 1;
        gPmbusCmd.PSU_FACTORY_MODE[0].bits.FruAccessControl = 0b10;
        gPmbusCmd.PSU_FACTORY_MODE[0].bits.MfrSampleSetControl = 1;
        gPmbusCmd.PSU_FACTORY_MODE[0].bits.BlackBoxClrControl = 1;
    }
    else
    {
        gPmbusCmd.STATUS_CML.bits.INVALID_DATA = 1;
    }
#    endif

    BYTE IsFmodEnabled;
    WORD temp;

    //[Peter Chung] 20110419 added
    if(I2C.readBuffer[0] & 0b10000000)
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
        return;
    }

    IsFmodEnabled = I2C.readBuffer[0] & PSU_FACTORY_MODE_EN_MASK;

    if(IsFmodEnabled == FALSE)
    {
        //WORD temp;
        //Disable Factory Mode
        gPmbusCmd.PSU_FACTORY_MODE[0].Val = ((gPmbusCmd.PSU_FACTORY_MODE[0].Val & 0b10000000) | 0);
        //Set OCP to 109A
#        if 1	//[Peter Chung] 20110217 removed
        temp = LinearFmt_XtoY(DEFAULT_IOUT_OC_FAULT, - 2, 0);
        gPmbusCmd.IOUT_OC_FAULT_LIMIT[0] = temp & 0xFF;
        gPmbusCmd.IOUT_OC_FAULT_LIMIT[1] = (temp >> 8) & 0xFF;
        Parameter.OCP_IOUT_OC_FAULT_LIMIT = (WORD) (((DWORD) DEFAULT_IOUT_OC_FAULT * IOUT_TO_ADC) >> 10); //ADC
#        endif
    }
    else if(gPmbusCmd.PSU_FACTORY_MODE[0].bits.FactoryMode)
    {
        gPmbusCmd.PSU_FACTORY_MODE[0].Val = ((gPmbusCmd.PSU_FACTORY_MODE[0].Val & 0b10000000) | I2C.readBuffer[0]); //[Peter Chung] 20110104 modified
    }
    else
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
    }
}

static void CMD_DISABLE_U1_TX_Handler()
{
    UpdateDataBuf();
}

static void CMD_CAL_TERIDIAN_IC_Handler() //Chris, add for Teridian
{
    if(I2C.readBuffer[0] == 0x00)
    { //CLC
        UpdateDataBuf();
        ResetU2State();
        gTeridanCaliType = CALI_NORMAL_LOAD;
    }
    else if(I2C.readBuffer[0] == 0x01)
    { //CLV
        UpdateDataBuf();
        ResetU2State();
        gTeridanCaliType = CALI_LIGHT_LOAD;
    }
    else if(I2C.readBuffer[0] == 0x02)
    { //FW rev.
        UpdateDataBuf();
        ResetU2State();
        //gTeridanCaliType = GET_TERIDIAN_FW_REV; //[davidchchen]20160223 removed
        gTeridanCaliType = CALI_VDC_OFFSET; //[davidchchen]20160223 added
    }
    else if(I2C.readBuffer[0] == 0x03)
    { //FW rev.
        UpdateDataBuf();
        ResetU2State();
        gTeridanCaliType = CALI_IDC_OFFSET; //[davidchchen]20160223 added
    }
    else
    {
        gPagePlusStatus.PAGE[PAGE0].STATUS_CML.bits.INVALID_DATA = 1;
        gPagePlusStatus.PAGE[PAGE1].STATUS_CML.bits.INVALID_DATA = 1;
    }
}

static void CaliIout(BYTE IOUT_TYPE)
{
    SHORT slope;

    if(IOUT_TYPE == CAL_IOUT_1)
    {
        slope = I2C.readBuffer[1] | (I2C.readBuffer[2] << 8);
        if((slope > 512) && (slope < 2048))
        {
            UserData.Page1.region.Iout[OUTPUT_12V].slope = I2C.readBuffer[1] | (I2C.readBuffer[2] << 8);
            UserData.Page1.region.Iout[OUTPUT_12V].Offset = (LONG) I2C.readBuffer[3] | ((LONG) I2C.readBuffer[4] << 8) | ((LONG) I2C.readBuffer[5] << 16) | ((LONG) I2C.readBuffer[6] << 24);
            PS.IOUT1_CALIBRATED = TRUE;
            UserData.Page2.region.Calibration_Flag.bits.b2 = 1;
        }
    }
    else if(IOUT_TYPE == CAL_IOUT_2)
    {
        slope = I2C.readBuffer[1] | (I2C.readBuffer[2] << 8);
        if((slope > 512) && (slope < 4096))
        {
            UserData.Page1.region.Iout[OUTPUT_12V].slope2 = I2C.readBuffer[1] | (I2C.readBuffer[2] << 8);
            UserData.Page1.region.Iout[OUTPUT_12V].Offset2 = (LONG) I2C.readBuffer[3] | ((LONG) I2C.readBuffer[4] << 8) | ((LONG) I2C.readBuffer[5] << 16) | ((LONG) I2C.readBuffer[6] << 24);
            PS.IOUT2_CALIBRATED = TRUE;
            UserData.Page2.region.Calibration_Flag.bits.b3 = 1;
        }
    }
    else if(IOUT_TYPE == CAL_IOUT_3)
    {
        slope = I2C.readBuffer[1] | (I2C.readBuffer[2] << 8);
        if((slope > 512) && (slope < 2048))
        {
            UserData.Page1.region.Iout[OUTPUT_12V].slope3 = I2C.readBuffer[1] | (I2C.readBuffer[2] << 8);
            UserData.Page1.region.Iout[OUTPUT_12V].Offset3 = (LONG) I2C.readBuffer[3] | ((LONG) I2C.readBuffer[4] << 8) | ((LONG) I2C.readBuffer[5] << 16) | ((LONG) I2C.readBuffer[6] << 24);
            PS.IOUT3_CALIBRATED = TRUE;
            UserData.Page2.region.Calibration_Flag.bits.b4 = 1;
        }
    }
    else if(IOUT_TYPE == CAL_IOUT_4)
    { // [Tommy YR Chen] 20110526 added
        slope = I2C.readBuffer[1] | (I2C.readBuffer[2] << 8);
        if((slope > 512) && (slope < 2048))
        {
            UserData.Page1.region.Iout[OUTPUT_12V].slope4 = I2C.readBuffer[1] | (I2C.readBuffer[2] << 8);
            UserData.Page1.region.Iout[OUTPUT_12V].Offset4 = (LONG) I2C.readBuffer[3] | ((LONG) I2C.readBuffer[4] << 8) | ((LONG) I2C.readBuffer[5] << 16) | ((LONG) I2C.readBuffer[6] << 24);
            PS.IOUT4_CALIBRATED = TRUE;
            UserData.Page2.region.Calibration_Flag.bits.b7 = 1;
        }
    }
    else if(IOUT_TYPE == CAL_IOUT_CS)
    {
        slope = I2C.readBuffer[1] | (I2C.readBuffer[2] << 8);
        if((slope > 512) && (slope < 2048))
        {
            UserData.Page1.region.Iout[OUTPUT_12V].CS_slope = I2C.readBuffer[1] | (I2C.readBuffer[2] << 8);
            UserData.Page1.region.Iout[OUTPUT_12V].CS_Offset = (LONG) I2C.readBuffer[3] | ((LONG) I2C.readBuffer[4] << 8) | ((LONG) I2C.readBuffer[5] << 16) | ((LONG) I2C.readBuffer[6] << 24);
            //PS.CS_CALIBTATED = TRUE;
            UserData.Page2.region.Calibration_Flag.bits.b6 = 1;
        }
    }
    else if(IOUT_TYPE == CAL_IOUT_LCS)
    {
        UserData.Page1.region.Iout[OUTPUT_12V].LCS_slope = I2C.readBuffer[1] | (I2C.readBuffer[2] << 8);
        UserData.Page1.region.Iout[OUTPUT_12V].LCS_Offset = (LONG) I2C.readBuffer[3] | ((LONG) I2C.readBuffer[4] << 8) | ((LONG) I2C.readBuffer[5] << 16) | ((LONG) I2C.readBuffer[6] << 24);
        PS.LCS_CALIBRATED = TRUE;
        UserData.Page2.region.Calibration_Flag.bits.b8 = 1;
    }
    else if(IOUT_TYPE == CAL_CS_IN)
    {
        UserData.Page1.region.VCS_Slope = I2C.readBuffer[1] | (I2C.readBuffer[2] << 8);
        UserData.Page1.region.VCS_Offset = (LONG) I2C.readBuffer[3] | ((LONG) I2C.readBuffer[4] << 8) | ((LONG) I2C.readBuffer[5] << 16) | ((LONG) I2C.readBuffer[6] << 24);
        gVCS_Slope = UserData.Page1.region.VCS_Slope;
        gVCS_Offset = UserData.Page1.region.VCS_Offset;
        PS.CS_CALIBTATED = TRUE;
        UserData.Page2.region.Calibration_Flag.bits.b12 = 1;
    }

    //Flash need to be written
    PS.FlashWritePage1 = TRUE;
}

static void CaliCSVout(void)
{
    SHORT slope;

    slope = I2C.readBuffer[1] | (I2C.readBuffer[2] << 8);

    if((slope > 512) && (slope < 2048))
    {
        UserData.Page1.region.Vout[OUTPUT_12V].CS_Slope = I2C.readBuffer[1] | (I2C.readBuffer[2] << 8);
        UserData.Page1.region.Vout[OUTPUT_12V].CS_Offset = (LONG) I2C.readBuffer[3] | ((LONG) I2C.readBuffer[4] << 8) | ((LONG) I2C.readBuffer[5] << 16) | ((LONG) I2C.readBuffer[6] << 24);

        CS_Slope_Temp = UserData.Page1.region.Vout[OUTPUT_12V].CS_Slope;
        CS_Offset_Temp = UserData.Page1.region.Vout[OUTPUT_12V].CS_Offset;

        UserData.Page2.region.Calibration_Flag.bits.b5 = 1;
        PS.FlashWritePage1 = TRUE;
    }
}

static void CMD_GEN_CAL_W_Handler()
{
    BYTE type;

    type = I2C.readBuffer[0];

    switch(type)
    {
        case CAL_VOUT:
            return;
        case CAL_IOUT:
            return;
            //case CAL_VIN:
            //CaliADE7953(CAL_VIN);
            //return;
            //case CAL_IIN:
            //CaliADE7953(CAL_IIN);
            //return;
        case CAL_CLEAR_ALL:
            //Clear Vout Cali data
            gVoutCmdOffset = 0;
            gVoutReadOffsetAdc = 0;
#            if RS_ENABLE   //[davidchchen] 20150108 not used
            gMainBusVoutOffset = 0;
#            endif
            gStbVoutOffset = 0;

            UserData.Page1.region.Vout[OUTPUT_12V].CmdOffset = gVoutCmdOffset;
            UserData.Page1.region.Vout[OUTPUT_12V].ReportOffset = gVoutReadOffsetAdc;
#            if RS_ENABLE   //[davidchchen] 20150108 not used
            UserData.Page1.region.Vout[OUTPUT_12V].MainBusOffset = gMainBusVoutOffset;
#            endif
            UserData.Page1.region.StbVoutOffset = gStbVoutOffset;
            //Clear Iout Cali data
            UserData.Page1.region.Iout[OUTPUT_12V].slope = 1024;
            UserData.Page1.region.Iout[OUTPUT_12V].Offset = 0;
            UserData.Page1.region.Iout[OUTPUT_12V].slope2 = 1024;
            UserData.Page1.region.Iout[OUTPUT_12V].Offset2 = 0;
            UserData.Page1.region.Iout[OUTPUT_12V].slope3 = 1024;
            UserData.Page1.region.Iout[OUTPUT_12V].Offset3 = 0;
            UserData.Page1.region.Iout[OUTPUT_12V].slope4 = 1024; // [Tommy YR Chen] 20111103
            UserData.Page1.region.Iout[OUTPUT_12V].Offset4 = 0;

            UserData.Page1.region.Iout[OUTPUT_12V].CS_slope = 1024;
            UserData.Page1.region.Iout[OUTPUT_12V].CS_Offset = 0;

            UserData.Page1.region.Vout[OUTPUT_12V].CS_Slope = 1024;
            UserData.Page1.region.Vout[OUTPUT_12V].CS_Offset = 0;

            CS_Slope_Temp = UserData.Page1.region.Vout[OUTPUT_12V].CS_Slope;
            CS_Offset_Temp = UserData.Page1.region.Vout[OUTPUT_12V].CS_Offset;

            UserData.Page1.region.VCS_Slope = 1024;
            UserData.Page1.region.VCS_Offset = 0;

            gVCS_Slope = UserData.Page1.region.VCS_Slope;
            gVCS_Offset = UserData.Page1.region.VCS_Offset;

            PS.CS_CALIBTATED = FALSE;
            PS.IOUT1_CALIBRATED = FALSE;
            PS.IOUT2_CALIBRATED = FALSE;
            PS.IOUT3_CALIBRATED = FALSE;
            PS.IOUT4_CALIBRATED = FALSE;
            UserData.Page2.region.Calibration_Flag.Val = 0;
            PS.StartCalibrate = TRUE;
            break;
        case CAL_CLEAR_VOUT:
            gVoutCmdOffset = 0;
            gVoutReadOffsetAdc = 0;
#            if RS_ENABLE   //[davidchchen] 20150108 not used
            gMainBusVoutOffset = 0;
#            endif
            gStbVoutOffset = 0;

            UserData.Page1.region.Vout[OUTPUT_12V].CmdOffset = gVoutCmdOffset;
            UserData.Page1.region.Vout[OUTPUT_12V].ReportOffset = gVoutReadOffsetAdc;
#            if RS_ENABLE   //[davidchchen] 20150108 not used
            UserData.Page1.region.Vout[OUTPUT_12V].MainBusOffset = gMainBusVoutOffset;
#            endif
            UserData.Page1.region.StbVoutOffset = gStbVoutOffset;
            UserData.Page2.region.Calibration_Flag.bits.b0 = 0;
            UserData.Page2.region.Calibration_Flag.bits.b1 = 0;
            break;
        case CAL_CLEAR_IOUT:
            //Clear Iout Cali data
            UserData.Page1.region.Iout[OUTPUT_12V].slope = 1024;
            UserData.Page1.region.Iout[OUTPUT_12V].Offset = 0;
            UserData.Page1.region.Iout[OUTPUT_12V].slope2 = 1024;
            UserData.Page1.region.Iout[OUTPUT_12V].Offset2 = 0;
            UserData.Page1.region.Iout[OUTPUT_12V].slope3 = 1024;
            UserData.Page1.region.Iout[OUTPUT_12V].Offset3 = 0;
            UserData.Page1.region.Iout[OUTPUT_12V].slope4 = 1024; // [Tommy YR Chen] 20111103
            UserData.Page1.region.Iout[OUTPUT_12V].Offset4 = 0;

            UserData.Page1.region.Iout[OUTPUT_12V].CS_slope = 1024;
            UserData.Page1.region.Iout[OUTPUT_12V].CS_Offset = 0;
            UserData.Page1.region.Vout[OUTPUT_12V].CS_Slope = 1024; // [Tommy YR Chen] 20110511
            UserData.Page1.region.Vout[OUTPUT_12V].CS_Offset = 0;

            CS_Slope_Temp = UserData.Page1.region.Vout[OUTPUT_12V].CS_Slope;
            CS_Offset_Temp = UserData.Page1.region.Vout[OUTPUT_12V].CS_Offset;

            UserData.Page1.region.VCS_Slope = 1024;
            UserData.Page1.region.VCS_Offset = 0;

            gVCS_Slope = UserData.Page1.region.VCS_Slope;
            gVCS_Offset = UserData.Page1.region.VCS_Offset;

            PS.CS_CALIBTATED = FALSE;
            PS.IOUT1_CALIBRATED = FALSE;
            PS.IOUT2_CALIBRATED = FALSE;
            PS.IOUT3_CALIBRATED = FALSE;
            PS.IOUT4_CALIBRATED = FALSE;

            UserData.Page2.region.Calibration_Flag.bits.b2 = 0;
            UserData.Page2.region.Calibration_Flag.bits.b3 = 0;
            UserData.Page2.region.Calibration_Flag.bits.b4 = 0;
            UserData.Page2.region.Calibration_Flag.bits.b5 = 0;
            UserData.Page2.region.Calibration_Flag.bits.b6 = 0;
            PS.StartCalibrate = TRUE;
            break;
        case CAL_CLEAR_VIN:
            break;
        case CAL_CLEAR_IIN:
            break;
        case CAL_IOUT_1:
            CaliIout(CAL_IOUT_1);
            break;
        case CAL_IOUT_2:
            CaliIout(CAL_IOUT_2);
            break;
        case CAL_IOUT_3:
            CaliIout(CAL_IOUT_3);
            break;
        case CAL_IOUT_4:
            CaliIout(CAL_IOUT_4);
            break;
        case CAL_IOUT_CS:
            CaliIout(CAL_IOUT_CS);
            break;
        case CAL_VOUT_CS:
            CaliCSVout();
            break;
        case CAL_IOUT_LCS: // [Tommy YR Chen] 2011110 added
            CaliIout(CAL_IOUT_LCS);
            break;
            //case CAL_PIN:
            //CaliADE7953(CAL_PIN);
            //break;
        case CAL_CS_IN:
            CaliIout(CAL_CS_IN);
            break;
        default:
            return;
    }

    //debug
    gPmbusCmd.GEN_CAL_W[0] = I2C.readBuffer[0];
    gPmbusCmd.GEN_CAL_W[1] = I2C.readBuffer[1];
    gPmbusCmd.GEN_CAL_W[2] = I2C.readBuffer[2];
    gPmbusCmd.GEN_CAL_W[3] = I2C.readBuffer[3];
    gPmbusCmd.GEN_CAL_W[4] = I2C.readBuffer[4];
    gPmbusCmd.GEN_CAL_W[5] = I2C.readBuffer[5];
    gPmbusCmd.GEN_CAL_W[6] = I2C.readBuffer[6];

    RefreshAllCaliInfo();

    //Flash need to be written
    PS.FlashWritePage1 = TRUE;

}

static void CMD_SMART_ON_CONFIG_Handler()
{
    UpdateDataBuf();

    if((gPmbusCmd.SMART_ON_CONFIG[0] & SMART_ON_MASK) == 0x01)
    {
        //Function ON
        PS.SmartOnEnabled = 1;
    }
    else
    {
        //Function OFF
        PS.SmartOnEnabled = 0;
    }
}

static void CMD_PAGE_PLUS_WRITE_Handler()
{
    BYTE page;
    BYTE status_cmd;
    BYTE byte_count;
    BYTE cmd = 0;

    //reset buffer
    memset(gPmbusCmd.PAGE_PLUS_WRITE, 0, sizeof(gPmbusCmd.PAGE_PLUS_WRITE));

    byte_count = I2C.readBuffer[0];
    //[Peter Chung] 20101221 modified
    page = I2C.readBuffer[1];

    if(byte_count == 3)
    {
        //Clear Status Command
        cmd = 1;
    }
    else if(byte_count == 4)
    {
        //SMBALERT_MASK
        cmd = 2;
    }

    if(page > (TOTAL_PAGE_COUNT - 1))
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
        return;
    }

    UpdateDataBuf();
    switch(cmd)
    {
        case 1:
            status_cmd = I2C.readBuffer[2];
            switch(gPmbusCmd.PAGE_PLUS_WRITE[2])
            {
                case 0x7A: //STATUS_VOUT
                case 0x7B: //STATUS_IOUT
                case 0x7C: //STATUS_INPUT
                case 0x7D: //STATUS_TEMPERATURE
                case 0x7E: //STATUS_CML
                    ClearStatusBit(gPmbusCmd.PAGE_PLUS_WRITE[2], gPmbusCmd.PAGE_PLUS_WRITE[3], gPmbusCmd.PAGE_PLUS_WRITE[1]);
                    break;
                default:
                    gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
                    return;
                    break;
            }
            break;

        case 2:
            switch(gPmbusCmd.PAGE_PLUS_WRITE[3])
            {
                case 0x7A: //STATUS_VOUT
                case 0x7B: //STATUS_IOUT
                case 0x7C: //STATUS_INPUT
                case 0x7D: //STATUS_TEMPERATURE
                case 0x7E: //STATUS_CML
                    SetSmbAlertMask(gPmbusCmd.PAGE_PLUS_WRITE[3], gPmbusCmd.PAGE_PLUS_WRITE[4], gPmbusCmd.PAGE_PLUS_WRITE[1]);
                    break;
                default:
                    gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
                    break;
            }
            break;

        default:
            gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
            return;
            break;
    }
}

static void GetPagePlusRead()
{
    BYTE page;
    BYTE status_cmd;
    BYTE byte_count;
    BYTE cmd = 0;

    //reset buffer
    memset(gPmbusCmd.PAGE_PLUS_READ, 0, sizeof(gPmbusCmd.PAGE_PLUS_READ));
    byte_count = I2C.readBuffer[0];
    //[Peter Chung] 20101221 modified
    page = I2C.readBuffer[1];

    if(byte_count == 2)
    {
        //Read Status Command
        cmd = 1;
    }
    else if(byte_count == 3)
    {
        //Read SMBALERT_MASK
        cmd = 2;
    }

    if(page > (TOTAL_PAGE_COUNT - 1))
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
        return;
    }

    switch(cmd)
    {
        case 1:
            status_cmd = I2C.readBuffer[2];
            switch(status_cmd)
            {
                case 0x7A: //STAUTS_VOUT
                    gPmbusCmd.PAGE_PLUS_READ[0] = 1; //byte count
                    gPmbusCmd.PAGE_PLUS_READ[1] = gPagePlusStatus.PAGE[page].STATUS_VOUT.Val;
                    break;
                case 0x7B: //STATUS_IOUT
                    gPmbusCmd.PAGE_PLUS_READ[0] = 1; //byte count
                    gPmbusCmd.PAGE_PLUS_READ[1] = gPagePlusStatus.PAGE[page].STATUS_IOUT.Val;
                    break;
                case 0x7C: //STATUS_INPUT
                    gPmbusCmd.PAGE_PLUS_READ[0] = 1; //byte count
                    gPmbusCmd.PAGE_PLUS_READ[1] = gPagePlusStatus.PAGE[page].STATUS_INPUT.Val;
                    break;
                case 0x7D: //STATUS_TEMPERATURE
                    gPmbusCmd.PAGE_PLUS_READ[0] = 1; //byte count
                    gPmbusCmd.PAGE_PLUS_READ[1] = gPagePlusStatus.PAGE[page].STATUS_TEMPERATURE.Val;
                    break;
                case 0x7E: //STATUS_CML
                    gPmbusCmd.PAGE_PLUS_READ[0] = 1; //byte count
                    gPmbusCmd.PAGE_PLUS_READ[1] = gPagePlusStatus.PAGE[page].STATUS_CML.Val;
                    break;
                case 0x79: //STATUS_WORD
                    gPmbusCmd.PAGE_PLUS_READ[0] = 2; //byte count
                    gPmbusCmd.PAGE_PLUS_READ[1] = gPagePlusStatus.PAGE[page].STATUS_WORD.byte.LB;
                    gPmbusCmd.PAGE_PLUS_READ[2] = gPagePlusStatus.PAGE[page].STATUS_WORD.byte.HB;
                    break;
                default:
                    gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
                    return;
                    break;
            }
            break;

        case 2:
            status_cmd = I2C.readBuffer[3];
            switch(status_cmd)
            {
                case 0x7A: //STATUS_VOUT
                case 0x7B: //STATUS_IOUT
                case 0x7C: //STATUS_INPUT
                case 0x7D: //STATUS_TEMPERATURE
                case 0x7E: //STATUS_CML
                    //GetSmbAlertMask(status_cmd);
                    GetPagePlusReadSmbAlertMask(status_cmd, page);
                    gPmbusCmd.PAGE_PLUS_READ[0] = 1; //byte count
                    gPmbusCmd.PAGE_PLUS_READ[1] = gPmbusCmd.SMBALERT_MASK[1];
                    break;
                default:
                    gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
                    break;
            }
            break;
    }
}

void CMD_FAN_CONFIG_Handler()
{
    gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
}

void CMD_DISABLE_FUNC()
{
    UpdateDataBuf();

    if(gPmbusCmd.DISABLE_FUNC[0] == 1)
    {
        //Disable Fan & Fan lock protection
        DisableFan();
    }
    else
    {
        //Enable Fan & Fan lock protection
        EnableFan();
    }
}

void CMD_PRI_DEBUG()
{
    UpdateDataBuf();
}

extern MyPID CurrentSharePID;

void CMD_PID_PARA()
{
    SHORT pid_cs_a;
    SHORT pid_cs_b;
    SHORT pid_cs_c;

    UpdateDataBuf();

    pid_cs_a = (WORD) gPmbusCmd.PID_P[0] | ((WORD) gPmbusCmd.PID_P[1] << 8);
    pid_cs_b = (WORD) gPmbusCmd.PID_P[2] | ((WORD) gPmbusCmd.PID_P[3] << 8);
    pid_cs_c = (WORD) gPmbusCmd.PID_P[4] | ((WORD) gPmbusCmd.PID_P[5] << 8);

    CurrentShareABC_SS[0] = pid_cs_a;
    CurrentShareABC_SS[1] = pid_cs_b;
    CurrentShareABC_SS[2] = pid_cs_c;
}

void CMD_WRITE_PROTECT_Handler()
{
    BYTE data;

    data = I2C.readBuffer[0];

    if(data == 0x80 || data == 0x40 || data == 0x20 || data == 0x00)
    {
        UpdateDataBuf();
    }
    else
    {
        gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
    }
}

void StoreDefaultAll()
{
    UserData.Page2.region.VoutCommand.Val = gVoutCmd;
    UserData.Page2.region.VoutCommand_LinearFmt.byte.LB = gPmbusCmd.VOUT_COMMAND[0];
    UserData.Page2.region.VoutCommand_LinearFmt.byte.HB = gPmbusCmd.VOUT_COMMAND[1];
    PS.FlashWritePage2 = TRUE;
}

#if 0

BYTE isForbidWriteCmd(BYTE cmd)
{
    if(cmd == 0x01 || //OPERATION
       cmd == 0x1B || //SMBALERT_MASK
       cmd == 0x21 || //VOUT_COMMAND
       cmd == 0x3B || //FAN_COMMAND_1
       cmd == 0x40 || //VOUT_OV_FAULT_LIMIT
       cmd == 0x42 || //VOUT_OV_WARN_LIMIT
       cmd == 0x43 || //VOUT_UV_WARN_LIMIT
       cmd == 0x46 || //IOUT_OC_FAULT_LIMIT
       cmd == 0x4A) //IOUT_OC_WARN_LIMIT
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BYTE isForbidReadCmd(BYTE cmd)
{
    if(cmd == 0x01 || //OPERATION
       cmd == 0x1B || //SMBALERT_MASK
       cmd == 0x21 || //VOUT_COMMAND
       cmd == 0x3B || //FAN_COMMAND_1
       cmd == 0x40 || //VOUT_OV_FAULT_LIMIT
       cmd == 0x42 || //VOUT_OV_WARN_LIMIT
       cmd == 0x43 || //VOUT_UV_WARN_LIMIT
       cmd == 0x46 || //IOUT_OC_FAULT_LIMIT
       cmd == 0x4A) //IOUT_OC_WARN_LIMIT
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#endif

void WriteCmdHandler()
{
    if(isPermitToWriteInPage1(I2C.currentCmd) == FALSE)
    {
        if(gPmbusCmd.PAGE[0] == 1)
        {
            gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_CMD = 1;
            goto STOP_W;
        }
    }

    switch(I2C.currentCmd)
    {
        case 0x00: //PAGE
            CMD_PAGE_Handler();
            break;
        case 0x01: //OPERATION
            CMD_OPERATION_Handler();
            break;
        case 0x03: //CLEAR_FAULTS
            ClearFaultByCommand(gPmbusCmd.PAGE[0]);
            break;
        case 0x05: //PAGE_PLUS_WRITE
            CMD_PAGE_PLUS_WRITE_Handler();
            break;
        case 0x06: //PAGE_PLUS_READ
            break;
        case 0x07: //DISABLE_U1_TX
            CMD_DISABLE_U1_TX_Handler();
            break;
        case 0x10: //WRITE_PROTECT
            CMD_WRITE_PROTECT_Handler();
            break;
        case 0x11: //STORE_DEFAULT_ALL
            StoreDefaultAll();
            break;
        case 0x1A: //QUERY
            break;
        case 0x1B: //SMBALERT_MASK
            SetSmbAlertMask(I2C.readBuffer[0], I2C.readBuffer[1], gPmbusCmd.PAGE[0]);
            break;
        case 0x21: //VOUT_COMMAND
            CMD_VOUT_COMMAND_Handler();
            break;
        case 0x30: //COEFFICIENTS
            break;
        case 0x3A: //FAN_CONFIG
            CMD_FAN_CONFIG_Handler();
            break;
        case 0x3B: //FAN_COMMAND_1
            CMD_FAN_COMMAND_1_Handler();
            break;
        case 0x40: //VOUT_OV_FAULT_LIMIT
            CMD_VOUT_OV_FAULT_LIMIT_Handler();
            break;
        case 0x42: //VOUT_OV_WARN_LIMIT
            CMD_VOUT_OV_WARN_LIMIT_Handler();
            break;
        case 0x43: //VOUT_UV_WARN_LIMIT
            CMD_VOUT_UV_WARN_LIMIT_Handler();
            break;
        case 0x44: //VOUT_UV_FAULT_LIMIT
            CMD_VOUT_UV_FAULT_LIMIT_Handler();
            break;
        case 0x46: //IOUT_OC_FAULT_LIMIT
            CMD_IOUT_OC_FAULT_LIMIT_Handler();
            break;
        case 0x4A: //IOUT_OC_WARN_LIMIT
            CMD_IOUT_OC_WARN_LIMIT_Handler();
            break;
        case 0x4F: //OT_FAULT_LIMIT
            CMD_OT_FAULT_LIMIT_Handler();
            break;
        case 0x51: //OT_WARN_LIMIT
            CMD_OT_WARN_LIMIT_Handler();
            break;
        case 0x55:
            CMD_VIN_OV_FAULT_LIMIT_Handler();
            break;
        case 0x57:
            CMD_VIN_OV_WARN_LIMIT_Handler();
            break;
        case 0x58:
            CMD_VIN_UV_WARN_LIMIT_Handler();
            break;
        case 0x59:
            CMD_VIN_UV_FAULT_LIMIT_Handler();
            break;
        case 0x5D: //IIN_OC_WARN_LIMIT
            CMD_IIN_OC_WARN_Handler();
            break;
        case 0x6A: //POUT_OP_WARN_LIMIT
            CMD_POUT_OP_WARN_LIMIT_Handler();
            break;
        case 0x6B: //PIN_OP_WARN_LIMIT
            //CMD_PIN_OP_WARN_LIMIT_Handler();
            break;
#            if 0
        case 0x78: //STATUS_BYTE
        case 0x7A: //STATUS_VOUT
        case 0x7B: //STATUS_IOUT
        case 0x7C: //STATUS_INPUT
        case 0x7D: //STATUS_TEMPERATURE
        case 0x7E: //STATUS_CML
        case 0x81: //STATUS_FANS_1_2
            ClearStatusBit(I2C.currentCmd, I2C.readBuffer[0], gPmbusCmd.PAGE[0]);
            break;
#            endif
        case 0xB0: //FRU_DATA_OFFSET
            CMD_FRU_DATA_OFFSET_Handler();
            break;
        case 0xB1: //FRU_IMAGE_OVERWRITE
            CMD_FRU_IMAGE_OVERWRITE_Handler();
            break;
        case 0xB9: //FRU_UPDATE
            CollectFRU();
            break;
        case 0xBA: //CAL_VOUT_OFFSET
            CMD_CAL_VOUT_OFFSET_Handler();
            break;
        case 0xBB: //CAL_TERIDIAN_IC
            CMD_CAL_TERIDIAN_IC_Handler();
            break;
        case 0xBD: //GEN_CAL_W
            CMD_GEN_CAL_W_Handler();
            break;
        case 0xC9: //ISP_KEY		//Chris20130117 for Lenovo AP
            VerifyISPKey();
            break;
        case 0xD0:
            CMD_SMART_ON_CONFIG_Handler();
            break;
            //case 0xD1:	//ISP_KEY
            //VerifyISPKey();
            //break;
        case 0xD2: //ISP_STATUS_CMD
            ISPStatusCmd();
            break;
        case 0xD7: //SYSTEM_LED_CNTL
            CMD_SYSTEM_LED_CNTL_Handler();
            break;
        case 0xD9: //MFR_SMBALERT_MASK
            CMD_MFR_SMBALERT_MASK_Handler();
            break;
        case 0xDE: //LOG_INDEX
            CMD_LOG_INDEX_Handler();
            break;
        case 0xE0: //MFR_PSON_CONTROL           //[davidchchen]20170216 added PSON Signal Enable/disable
            CMD_MFR_PSON_CONTROL_Handler();     //[davidchchen]20170216 added PSON Signal Enable/disable
            break;
        case 0xE4: //MFR_PAGE
            CMD_MFR_PAGE_Handler();
            break;
        //case 0xE7: //MFR_CLEAR_HISTORY        //[davidchchen]20170310 Removed MFR_CLEAR_HISTORY
        //    CMD_MFR_CLEAR_HISTORY_Handler();  //[davidchchen]20170310 Removed MFR_CLEAR_HISTORY

        case 0xE7: //BMC_UNIX_TIMESTAMP          //[davidchchen]20170310 Added BMC_UNIX_TIMESTAMP cmd
            CMD_BMC_UNIX_TIMESTAMP_Handler();
            break;
        case 0xE8: //PFC_DISABLE
            CMD_PFC_DISABLE_Handler();
            break;
        case 0xEE: //PRI_DEBUG
            CMD_PRI_DEBUG();
            break;
        case 0xF0: //PSU_FACTORY_MODE
            CMD_PSU_FACTORY_MODE_Handler();
            break;
        case 0xF2: //DISABLE_FUNC
            CMD_DISABLE_FUNC();
            break;
        case 0xFE:
            CMD_PID_PARA();
            break;

        default:
            //Unsupported command, set status CML here
            gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_CMD = 1;
            break;
    }

STOP_W:
    //Stop Write Command
    I2C.cmdDir = CMD_R;
}

void init_Coefficient()
{
    //SMBALERT_MASK
    gCoef.SMBALERT_MASK.R.data.m.Val = DEFAULT_R_M;
    gCoef.SMBALERT_MASK.R.data.b.Val = DEFAULT_R_B;
    gCoef.SMBALERT_MASK.R.data.r = DEFAULT_R_R;
    gCoef.SMBALERT_MASK.W.data.m.Val = DEFAULT_W_M;
    gCoef.SMBALERT_MASK.W.data.b.Val = DEFAULT_W_B;
    gCoef.SMBALERT_MASK.W.data.r = DEFAULT_W_R;

    //READ_EIN
    gCoef.EIN.R.data.m.Val = DEFAULT_R_M;
    gCoef.EIN.R.data.b.Val = DEFAULT_R_B;
    gCoef.EIN.R.data.r = DEFAULT_R_R;
    gCoef.EIN.W.data.m.Val = DEFAULT_W_M;
    gCoef.EIN.W.data.b.Val = DEFAULT_W_B;
    gCoef.EIN.W.data.r = DEFAULT_W_R;

    //READ_EOUT
    gCoef.EOUT.R.data.m.Val = DEFAULT_R_M;
    gCoef.EOUT.R.data.b.Val = DEFAULT_R_B;
    gCoef.EOUT.R.data.r = DEFAULT_R_R;
    gCoef.EOUT.W.data.m.Val = DEFAULT_W_M;
    gCoef.EOUT.W.data.b.Val = DEFAULT_W_B;
    gCoef.EOUT.W.data.r = DEFAULT_W_R;


}

void UpdateLog(BYTE index)
{
    if(index <= 23)
    {
        //LOG
        gPmbusCmd.LOG_GENERAL[0] = 14;
        gPmbusCmd.LOG_GENERAL[1] = UserData.Page3.region.logGeneral.Write_Cycles.byte.LB;
        gPmbusCmd.LOG_GENERAL[2] = UserData.Page3.region.logGeneral.Write_Cycles.byte.HB;
        gPmbusCmd.LOG_GENERAL[3] = UserData.Page3.region.logGeneral.Write_Cycles.byte.UB;
        gPmbusCmd.LOG_GENERAL[4] = UserData.Page3.region.logGeneral.Write_Cycles.byte.MB;
        gPmbusCmd.LOG_GENERAL[5] = UserData.Page3.region.logGeneral.Total_Operate_Time.byte.LB;
        gPmbusCmd.LOG_GENERAL[6] = UserData.Page3.region.logGeneral.Total_Operate_Time.byte.HB;
        gPmbusCmd.LOG_GENERAL[7] = UserData.Page3.region.logGeneral.Total_Operate_Time.byte.UB;
        gPmbusCmd.LOG_GENERAL[8] = UserData.Page3.region.logGeneral.Total_Operate_Time.byte.MB;
        gPmbusCmd.LOG_GENERAL[9] = UserData.Page3.region.logGeneral.Total_Output_Energy.byte.LB;
        gPmbusCmd.LOG_GENERAL[10] = UserData.Page3.region.logGeneral.Total_Output_Energy.byte.HB;
        gPmbusCmd.LOG_GENERAL[11] = UserData.Page3.region.logGeneral.Total_Output_Energy.byte.UB;
        gPmbusCmd.LOG_GENERAL[12] = UserData.Page3.region.logGeneral.Total_Output_Energy.byte.MB;
        gPmbusCmd.LOG_GENERAL[13] = UserData.Page3.region.logGeneral.Latest_Log_Index;
        gPmbusCmd.LOG_GENERAL[14] = UserData.Page3.region.logGeneral.Log_Sum;

        gPmbusCmd.LOG_INDEX[0] = index;

        gPmbusCmd.LOG_CONTENT[0] = 10;
        gPmbusCmd.LOG_CONTENT[1] = UserData.Page3.region.logContent[index].Happen_Time.byte.LB;
        gPmbusCmd.LOG_CONTENT[2] = UserData.Page3.region.logContent[index].Happen_Time.byte.HB;
        gPmbusCmd.LOG_CONTENT[3] = UserData.Page3.region.logContent[index].Happen_Time.byte.UB;
        gPmbusCmd.LOG_CONTENT[4] = UserData.Page3.region.logContent[index].Happen_Time.byte.MB;
        gPmbusCmd.LOG_CONTENT[5] = UserData.Page3.region.logContent[index].Status_Word.byte.LB;
        gPmbusCmd.LOG_CONTENT[6] = UserData.Page3.region.logContent[index].Status_Word.byte.HB;
        gPmbusCmd.LOG_CONTENT[7] = UserData.Page3.region.logContent[index].Status_Debug.byte.LB;
        gPmbusCmd.LOG_CONTENT[8] = UserData.Page3.region.logContent[index].Status_Debug.byte.HB;
        gPmbusCmd.LOG_CONTENT[9] = UserData.Page3.region.logContent[index].Status_MFR_Specific;
        gPmbusCmd.LOG_CONTENT[10] = UserData.Page3.region.logContent[index].Index;
    }

}

void init_Pmbus()
{
    WORD temp;
    //initial Pmbus buffer
    gPmbusCmd.PAGE[0] = 0; //12V

    //Operation
    gPmbusCmd.OPERATION[0] = OPERATION_ON;
    gPmbusCmd.MFR_PSON_CONTROL[0] = MFR_PSON_DISABLE;   //[davidchchen]20170216 added PSON Signal Enable/disable

    //On_Off_Config
    gPmbusCmd.ON_OFF_CONFIG[0] = 0x1F;

    //Write_Protect
    gPmbusCmd.WRITE_PROTECT[0] = 0x80;

    //VOUT MODE
    gPmbusCmd.VOUT_MODE[0] = 0x18; // N=-8

    //MFR_FW_VER_DATE
    gPmbusCmd.MFR_FW_VER_DATE[0] = 0x31;
    gPmbusCmd.MFR_FW_VER_DATE[1] = 0x32;
    gPmbusCmd.MFR_FW_VER_DATE[2] = 0x31;
    gPmbusCmd.MFR_FW_VER_DATE[3] = 0x31;
    gPmbusCmd.MFR_FW_VER_DATE[4] = 0x32;
    gPmbusCmd.MFR_FW_VER_DATE[5] = 0x33;

    //[Peter Chung] 20101223 added for init Status buffer
    gPmbusCmd.STATUS_WORD = & (gPagePlusStatus.PAGE[0].STATUS_WORD);
    gPmbusCmd.STATUS_VOUT = & (gPagePlusStatus.PAGE[0].STATUS_VOUT);
    gPmbusCmd.STATUS_INPUT = & (gPagePlusStatus.PAGE[0].STATUS_INPUT);
    gPmbusCmd.STATUS_TEMPERATURE = & (gPagePlusStatus.PAGE[0].STATUS_TEMPERATURE);
    gPmbusCmd.STATUS_IOUT = & (gPagePlusStatus.PAGE[0].STATUS_IOUT);
    gPmbusCmd.STATUS_CML = & (gPagePlusStatus.PAGE[0].STATUS_CML);

    gCmd[0x78].pBuf = (BYTE*) (gPmbusCmd.STATUS_WORD); //STATUS_BYTE
    gCmd[0x79].pBuf = (BYTE*) (gPmbusCmd.STATUS_WORD); //STATUS_WORD
    gCmd[0x7A].pBuf = (BYTE*) (gPmbusCmd.STATUS_VOUT);
    gCmd[0x7C].pBuf = (BYTE*) (gPmbusCmd.STATUS_INPUT);
    gCmd[0x7D].pBuf = (BYTE*) (gPmbusCmd.STATUS_TEMPERATURE);
    gCmd[0x7B].pBuf = (BYTE*) (gPmbusCmd.STATUS_IOUT);
    gCmd[0x7E].pBuf = (BYTE*) (gPmbusCmd.STATUS_CML);


    gPmbusCmd.CAPABILITY[0] = 0x90; //supports PEC, 100kHz, SMBALERT

    gPmbusCmd.FAN_CONFIG_1_2[0] = 0x99; //Fan1, PWM, 2 pulses/revolution, bit0 set to 1 as advising by documnet

    //default VOUT_COMMAND is 0x0C80 mapped to 12V5, Linear-16
    gPmbusCmd.VOUT_COMMAND[0] = UserData.Page2.region.VoutCommand_LinearFmt.byte.LB;
    gPmbusCmd.VOUT_COMMAND[1] = UserData.Page2.region.VoutCommand_LinearFmt.byte.HB;

    //default VOUT_OV_FAULT_LIMIT is 0x0D80 mapped to 14.2V, N = -8
    gPmbusCmd.VOUT_OV_FAULT_LIMIT[0] = 0x80;
    gPmbusCmd.VOUT_OV_FAULT_LIMIT[1] = 0x0D;

    //VOUT_OV_WARN, 13.2V, N = -8
    gPmbusCmd.VOUT_OV_WARN_LIMIT[0] = 0x33;
    gPmbusCmd.VOUT_OV_WARN_LIMIT[1] = 0x0D;

    //VOUT_UV_WARN, 10.7V, 20141106 david fix 11.5
    gPmbusCmd.VOUT_UV_WARN_LIMIT[0] = 0x80; //0xB3;
    gPmbusCmd.VOUT_UV_WARN_LIMIT[1] = 0x0b; //0x0A;

    ////default VOUT_UV_FAULT_LIMIT, 10.5V, N=-9, 20141106 david fix
    gPmbusCmd.VOUT_UV_FAULT_LIMIT[0] = 0x80;
    gPmbusCmd.VOUT_UV_FAULT_LIMIT[1] = 0x0A;

    //VIN_OV_FAULT_LIMIT, 285V, N = -1
    gPmbusCmd.VIN_OV_FAULT_LIMIT[0] = 0x3A;
    gPmbusCmd.VIN_OV_FAULT_LIMIT[1] = 0xFA;

    //VIN_OV_WARN_LIMIT, 275V, N = -1
    gPmbusCmd.VIN_OV_WARN_LIMIT[0] = 0x26;
    gPmbusCmd.VIN_OV_WARN_LIMIT[1] = 0xFA;

    //VIN_UV_FAULT_LIMIT, 170V, N = -2
    gPmbusCmd.VIN_UV_FAULT_LIMIT[0] = 0x54;
    gPmbusCmd.VIN_UV_FAULT_LIMIT[1] = 0xF9;

    //VIN_UV_WARN_LIMIT, 175V(0XF95E),173V(0XF95A), N = -2, //[davidchchen]20160408 changed Vin_uv_warn
    gPmbusCmd.VIN_UV_WARN_LIMIT[0] = 0x5a; //[davidchchen]20160408 changed Vin_uv_warn
    gPmbusCmd.VIN_UV_WARN_LIMIT[1] = 0xF9; //[davidchchen]20160408 changed Vin_uv_warn

    //default IOUT_OC_FAULT_LIMIT is MFR_IOUT_MAX, Linear 11, n=-2, 230A(0XF398), 220A(0XF370)
    gPmbusCmd.IOUT_OC_FAULT_LIMIT[0] = 0x70; //0x98;      //[davidchchen]20160919 OCP change to 220A(0XF370)
    gPmbusCmd.IOUT_OC_FAULT_LIMIT[1] = 0xF3; //0xF3;      //[davidchchen]20160919 OCP change to 220A(0XF370)

    //default IOUT_OC_WARN_LIMIT is MFR_IOUT_MAX
    temp = LinearFmt_XtoY(DEFAULT_IOUT_OC_WARN, - 2, 0);
    //temp= LinearFmt_XtoY(53934, -3, 10);
    gPmbusCmd.IOUT_OC_WARN_LIMIT[0] = temp & 0xFF;
    gPmbusCmd.IOUT_OC_WARN_LIMIT[1] = (temp >> 8) & 0xFF;

    // have to judge LL or HL to determine DEFAULT_IIN_OC_WARN_HL or DEFAULT_IIN_OC_WARN_LL
    //temp = LinearFmt_XtoY(DEFAULT_IIN_OC_WARN_HL, -5, 7);
    temp = LinearFmt_XtoY(DEFAULT_IIN_OC_WARN_HL, - 5, 0); //Peter
    gPmbusCmd.IIN_OC_WARN_LIMIT[0] = temp & 0xFF;
    gPmbusCmd.IIN_OC_WARN_LIMIT[1] = (temp >> 8) & 0xFF;
    //memcpy(gPmbusCmd.IIN_OC_WARN_LIMIT, gPmbusCmd.MFR_IIN_MAX, 2);

    temp = LinearFmt_XtoY(DEFAULT_IIN_OC_FAULT_HL, - 5, 0);
    gPmbusCmd.IIN_OC_FAULT_LIMIT[0] = temp & 0xFF;
    gPmbusCmd.IIN_OC_FAULT_LIMIT[1] = (temp >> 8) & 0xFF;

    temp = LinearFmt_XtoY(DEFAULT_PIN_OP_WARN, 2, 0);
    gPmbusCmd.PIN_OP_WARN_LIMIT[0] = temp & 0xFF;
    gPmbusCmd.PIN_OP_WARN_LIMIT[1] = (temp >> 8) & 0xFF;

    //POUT_OP_WARN_LIMIT, Default is 3000W
    temp = LinearFmt_XtoY(DEFAULT_POUT_OP_WARN, GetN(N_POUT, DEFAULT_POUT_OP_WARN), 0);
    gPmbusCmd.POUT_OP_WARN_LIMIT[0] = temp & 0xFF;
    gPmbusCmd.POUT_OP_WARN_LIMIT[1] = (temp >> 8) & 0xFF;

    //OT_FAULT_LIMIT
    temp = LinearFmt_XtoY(DEFAULT_TSEC_FAULT, 0, 0);
    gPmbusCmd.OT_FAULT_LIMIT[0] = temp & 0xFF;
    gPmbusCmd.OT_FAULT_LIMIT[1] = (temp >> 8) & 0xFF;

    //OT_WARN_LIMIT, Default is 57C
    temp = LinearFmt_XtoY(DEFAULT_TSEC_WARN, 0, 0);
    gPmbusCmd.OT_WARN_LIMIT[0] = temp & 0xFF;
    gPmbusCmd.OT_WARN_LIMIT[1] = (temp >> 8) & 0xFF;

    init_Coefficient();

    init_MFR();
    init_MFR_Specific();

    gPmbusCmd.PMBUS_REVISION[0] = 0x22; //support PMBus Part I II rev1.2
    gPmbusCmd.PSU_FACTORY_MODE[0].Val = 0;
    if(UserData.Page2.region.FruModified_Flag)
    {
        gPmbusCmd.PSU_FACTORY_MODE[0].bits.FruModified = 1;
    }

    //PSU Features
    gPmbusCmd.PSU_FEATURES[0].bits.RapidOn = 1;
    gPmbusCmd.PSU_FEATURES[0].bits.PFC_Disable = 1;
    gPmbusCmd.PSU_FEATURES[0].bits.FaultProtectProg = 1;
    gPmbusCmd.PSU_FEATURES[0].bits.HighAccuracy = 1;
    gPmbusCmd.PSU_FEATURES[0].bits.DiamondEff = 0;

    //POS timer
    //gPmbusCmd.MFR_POS_TOTAL.Val = UserData.Page2.region.POS.Val;        //[davidchchen]20170418 Removed
    MFR_POS_TOTAL.Val = UserData.Page2.region.POS.Val;                  //[davidchchen]20170418 Added Black Box block read        //[davidchchen]20170418 Added Black Box block read

    //Init EIN / EOUT
    InitEnergyCount();

    RefreshAllCaliInfo();

    gPmbusCmd.VIN_MODE[0] = 0x1B; //N = -5
    gPmbusCmd.IIN_MODE[0] = 0x16; //N = -10
    gPmbusCmd.PIN_MODE[0] = 0x00; //N = 0
    gPmbusCmd.IOUT_MODE[0] = 0x19; //N = -7
    gPmbusCmd.POUT_MODE[0] = 0x00; //N = 0

    gPmbusCmd.PSU_MODEL_NUM[0] = 0x01; //0x01;    //[davidchchen]20151225 modified n+1 =0x00, n+n=0x01;
    gPmbusCmd.PSU_MODEL_NUM[1] = 0x00;

    gPmbusCmd.VOUT_OV_FAULT_RESPONSE[0] = 0xC0;
    gPmbusCmd.IOUT_OC_FAULT_RESPONSE[0] = 0xC0;
    gPmbusCmd.OT_FAULT_RESPONSE[0] = 0xB8;
    gPmbusCmd.VIN_OV_FAULT_RESPONSE[0] = 0xB8;
    gPmbusCmd.VIN_UV_FAULT_RESPONSE[0] = 0xB8;

    UpdateLog(0);

    //reset flag
    PS.FlashWritePage1 = FALSE;
    PS.FlashWritePage2 = FALSE;

}

BYTE CheckBlockMode(BYTE cmd)
{
    switch(cmd)
    {
        case 0x05: //PAGE_PLUS_WRITE
        case 0x06: //PAGE_PLUS_READ
        case 0x1A: //QUERY
        case 0x1B: //SMBALERT_MASK
        case 0x30: //COEFFICIENTS
        //case 0xE7: //BMC_UNIX_TIMESTAMP    //[davidchchen]20170418 Added Black Box block read
        case 0xBE: //GEN_CAL_R
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }
}

void HandleWRBlock(BYTE cmd)
{
    switch(cmd)
    {
        case 0x06: //PAGE_PLUS_READ
            GetPagePlusRead();
            break;
        case 0x1A: //QUERY
            GetQuery();
            break;
        case 0x1B: //SMBALERT_MASK
            GetSmbAlertMask(I2C.readBuffer[1]);
            break;
        case 0x30: //COEFFICIENTS
            GetCoefficient();
            break;
        case 0xBE: //GEN_CAL_R
            GetCaliInfo();
            break;
        default:
            break;
    }

}

BYTE IsSendCmd(BYTE cmd)
{
    switch(cmd)
    {
        case 0x03: //CLEAR_FAULT
        case 0x11: //STORE_DEFAULT_ALL
        //case 0xE7: //CLEAR_HISTORY          //[davidchchen]20170310 Removed MFR_CLEAR_HISTORY

            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }
}

BYTE IsFactoryCmd(BYTE cmd)
{
    switch(cmd)
    {
        case 0x05: //PAGE_PLUS_WRITE
        case 0x06: //PAGE_PLUS_READ
        case 0x07: //DISABLE_U1_TX
        case 0x5B: //IIN_OC_FAULT_LIMIT
        case 0x5D: //IIN_OC_WARN_LIMIT
        case 0x6A: //POUT_OP_WARN_LIMIT
        case 0x6B: //PIN_OP_WARN_LIMIT
        case 0x77: //DEBUG_INFO
        case 0x86: //READ_EIN
        case 0x87: //READ_EOUT
        case 0xAB: //MFR_EFFICIENCY_HL
        //case 0xB0: //FRU_WRITE          //[davidchchen]20170704 Removed FRU Factory mode
        //case 0xB1: //FRU_READ           //[davidchchen]20170704 Removed FRU Factory mode
        case 0xB2: //FRU_FLASH_WRITE
        case 0xB3: //FRU_FLASH_ERASE
        case 0xB4: //MFR_FIRMWARE_VER
        case 0xB9: //UPDATE_FULL_FRU
        case 0xBA: //CAL_VOUT_OFFSET
        case 0xBB: //CAL_TERIDIAN_IC
        case 0xBC: //CAL_TERIDIAN_FINISHED_NOTIFY
        case 0xBD: //GEN_CAL_W
        case 0xBE: //GEN_CAL_R
        case 0xBF: //CAL_TERIDIAN_GET_FW_VER
        case 0xC0: //MFR_MAX_TEMP_1
        case 0xC1: //MFR_MAX_TEMP_2
        //case 0xE5: //MFR_POS_TOTAL        //[davidchchen]20170310 Removed
        //case 0xE6: //MFR_POS_LAST         //[davidchchen]20170310 Removed
        //case 0xE7: //BMC_UNIX_TIMESTAMP     //[davidchchen]20170310 Removed
        case 0xED: //PID_LOG
        case 0xEE: //PRI_DEBUG
        case 0xF1: //READ_ALL_CALI_INFO
        case 0xF2: //DISABLE_FUNC
        case 0xF3: //CS_PWM_FB
        case 0xF4: //CS_IN
        case 0xF5: //READ_IOUT_LS
        case 0xF6: //READ_IOUT_SS
        case 0xF8: //SCI
        case 0xF9: //PRI_STATUS	//[Peter Chung] 20100928 added
        case 0xFA: //PRI_FW_VER_INTERNAL
        case 0xFB: //READ_IOUT_LS_2
        case 0xFC: //CALI_INFO
        case 0xFD: //READ_TEMPERATURE_4(T_Sec)
        case 0xFE: //PID_PARA

            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }

}

