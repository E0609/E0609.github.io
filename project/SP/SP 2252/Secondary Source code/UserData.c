
#include "UserData.h"
#include "Pmbus.h"
#include "memory.h"
#include "Timer.h"
#include <string.h>
#include <stdlib.h>
#include "Process.h"
#include "Isr.h"
#include "Util.h"
#include "Init.h"
#include "Parameter.h"      //[davidchchen]20150112 Added Compatible Code Fun
#include "RevisionBlock.h"  //[davidchchen]20160304 added RCB

tUSER_DATA_PAGE UserData;

#if SAMPLE_SET_SUPPORTED
static tSAMPLE_NODE* pStart = NULL;
static tSAMPLE_NODE* pEnd = NULL;
static BYTE gSampleCount = 0;
static BYTE gStopSample = FALSE;
#endif

SHORT CS_Slope_Temp = 0;
LONG CS_Offset_Temp = 0;

WORD gVoutAD_Fault;
WORD gIoutAD_Fault;

extern tPS_FLAG PS;
extern SHORT gVoutCmdOffset;
extern SHORT gVoutReadOffsetAdc;
extern SHORT gStbVoutOffset;

//BYTE HW_CompatibleCodeStr[2];   //[davidchchen] 20150112 Added Compatible Code Fun

void SaveUserDataPage1ToFlash ( )
{
  DWORD_VAL srcAddr;
  BYTE *buffer = UserData.Page1.data;

  //including FRU data & Calibration info
  srcAddr.Val = USER_DATA_ADDR;
  ErasePM ( 1, srcAddr );			//erase 1 page
  WritePM ( buffer, 2, srcAddr );               //write 512 bytes, 1 row is 256 bytes
  
}

void SaveUserDataPage2ToFlash ( )
{
  DWORD_VAL srcAddr;
  BYTE *buffer = UserData.Page2.data;

  //including BlackBox & Sample Sets & Time
  srcAddr.Val = MISC_ADDR;
  ErasePM ( 1, srcAddr );
  WritePM ( buffer, 4, srcAddr );	//write 1024 bytes, 1 row is 256 bytes
}

void SaveUserDataPage3ToFlash ( )
{
  DWORD_VAL srcAddr;
  BYTE *buffer = UserData.Page3.data;

#if 0
  if ( UserData.Page3.region.logGeneral.Write_Cycles.Val >= 1 )
  {
      UserData.Page3.region.logGeneral.Write_Cycles.Val -= 1;
  }
#else
  UserData.Page3.region.logGeneral.Write_Cycles.Val += 1;
#endif

  //including BlackBox & Sample Sets & Time
  srcAddr.Val = LOG_ADDR;
  ErasePM ( 1, srcAddr );
  WritePM ( buffer, 2, srcAddr );	//write 1024 bytes, 1 row is 256 bytes
}

BYTE CaluIPMIchecksum( BYTE* IpmiDataBuf, BYTE IpmiDataLen)  //[davidchchen]20170921 Added
{
    BYTE i,ChecSumCode;
    static WORD_VAL Sumdata;

    Sumdata.Val =0;

    //for ( i = 0 ; i < (IpmiDataLen -1) ; i ++ )
    for ( i = 0 ; i < IpmiDataLen ; i ++ )
    {
        //ipmichecksum += *(fruBufdata)++;
        //Temp = *(IpmiDataBuf + i);
        //ipmichecksum += Temp;

        //Temp = *IpmiDataBuf;
        Sumdata.Val += *IpmiDataBuf;
        IpmiDataBuf++;
    }
    ChecSumCode = Sumdata.v[0];
    ChecSumCode = ~ ChecSumCode + 1;

  return ChecSumCode;
}

void LoadIpmiDataFormat()              //[davidchchen]20170921 Added
{

    //[davidchchen]20170921 Debug test
    //strncpy((char *) &UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoData.SerialNumberBuf, (char *) MFR_SERIALNUMBER_EX, 16); //Model Name = "PS-2252-6A1U    "


//~~~~~~~~~~~~~~~ Config IPMI FRU DATA FORMAT (8 + 72 + 64 + 32 + 80 = 256 Bytes )~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~ Config COMMON HEADER Format (0X00 ~0X08), 8 Bytes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    UserData.Page1.region.FRU.area.CommonHeader.FormatVer = FRU_FORMAT_VERSION_NUM;
    UserData.Page1.region.FRU.area.CommonHeader.Internal_use_off = FRU_INTERNAL_AREA_OFFSET;            //not used
    UserData.Page1.region.FRU.area.CommonHeader.Chassis_info_off = FRU_CHASSIS_AREA_OFFSET;     //not used
    UserData.Page1.region.FRU.area.CommonHeader.Board_area_off = FRU_BOARD_AREA_OFFSET;         //not used
    UserData.Page1.region.FRU.area.CommonHeader.Product_area_off =FRU_PRODUCT_AREA_OFFSET;      //Product Area offset 22*8 = 176 -> addr 0xB0.
    UserData.Page1.region.FRU.area.CommonHeader.Multirecord_off = FRU_MULTI_RECORD_AREA_OFFSET; //not used
    UserData.Page1.region.FRU.area.CommonHeader.Pad = FRU_PAD;                                  //not used
    UserData.Page1.region.FRU.area.CommonHeader.Checksum = CaluIPMIchecksum (&UserData.Page1.region.FRU.area.CommonHeader.FormatVer, 7);

//~~~~~~~~~~~~~~~ Config MULTI RECORD AREA Format (0X08 ~0X4F),72 Bytes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    memset ( UserData.Page1.region.FRU.area.MulitiRecordInfoArea, 0, sizeof (UserData.Page1.region.FRU.area.MulitiRecordInfoArea ) );

//~~~~~~~~~~~~~~~ Config INTERNAL USE AREA Format (0X50 ~0X8F), 64 Bytes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    memset ( UserData.Page1.region.FRU.area.InternalUseArea, 0, sizeof (UserData.Page1.region.FRU.area.InternalUseArea ) );

//~~~~~~~~~~~~~~~ Config CHASSIS INFORMATION AREA (0X90 ~0XaF), 32 Bytes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    memset ( UserData.Page1.region.FRU.area.ChassisInfoArea, 0, sizeof (UserData.Page1.region.FRU.area.ChassisInfoArea ) );

//~~~~~~~~~~~~~~  Config Product Info Area Format (0XB0 ~0XFF), 80 Bytes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoData.ProductAreaVer = FRU_PRODUCT_FORMAT_VERSION_NUM; //Config Product Info Area Format Version Number = 1
    UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoData.ProductAreaLen = FRU_PRODUCT_FORMAT_LENGTH;      //Product Area length, 8*10 = 80 byte
    UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoData.LanguageCode = FRU_PRODUCT_FORMAT_LANGUAGE;      //Language, English = 0x19
    //Config Manufacture Name info
    UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoData.MfrNameTypeLen = FRU_PRODUCT_MFR_NAME_LENGTH;    //ASCII, Mrf name length -> 6bytes
    strncpy((char *) &UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoData.MfrNameBuf[0], (char *) MFR_NAME, 6);  //Mfr Name = "LITEON"
    //Config Product/Model Name info
    UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoData.ProductNameTypeLen = FRU_PRODUCT_NAME_LENGTH;   //ASCII, Product/Model Name length -> 16bytes
    strncpy((char *) &UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoData.ProductNameBuf[0], (char *) MFR_MODEL_INIT, MFR_MODEL_LEN); //Model Name = "PS-2252-6A1U    "
    //Config Part Number info
    UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoData.PartNumberTypeLen = FRU_PRODUCT_PARTNUM_LENGTH;    //ASCII, PartNumber length -> 16bytes
    memset ( UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoData.PartNumberBuf, 0, sizeof (UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoData.PartNumberBuf ) );
    //Config Product Version Number info
    UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoData.ProductVerTypeLen = FRU_PRODUCT_VERSION_NUM_LENGTH;    //ASCII,Product Version Number length  -> 2bytes
    strncpy ( ( char * ) &UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoData.ProductVerBuf, ( char * ) &UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoData.SerialNumberBuf[7], 2 );
    //Config Product Serial Number info
    UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoData.SerialNumberTypeLen = FRU_PRODUCT_SERIALNUM_LENGTH;  //ASCII,Product Serial Number length  -> 16bytes
    //Config Asset TAG info
    UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoData.AssetTagTypeLen = FRU_PRODUCT_ASSET_TAG_LENGTH;  //ASCII,Asset TAG info length  -> 8bytes
    memset ( UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoData.AssetTagBuf, 0, sizeof (UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoData.AssetTagBuf ) );
    //Config FRU File info
    UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoData.FruFileIDTypeLen = FRU_PRODUCT_FILE_ID_LENGTH; //ASCII,FRU File length  -> 4bytes
    memset ( UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoData.FruFileIDBuf, 0, sizeof (UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoData.FruFileIDBuf ) );
    //Config End of Fields Marker
    UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoData.EFM = FRU_PRODUCT_EFM;      //End of Fields Marker
    //Calculate Product Area checksum, 0xB0 ~0XFE ALL DATA
    UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoData.ProductCheckSum = CaluIPMIchecksum (&UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoBuf, (sizeof (UserData.Page1.region.FRU.area.ProductInfoArea.ProductInfoBuf )-1));

    SaveUserDataPage1ToFlash ( );
    
}
void LoadUserDataFromFlash ( )
{
  DWORD_VAL srcAddr;
  DWORD_VAL bl_version;
  static BYTE_VAL  HWFW_conflictStatus;    //[davidchchen]20160304 added RCB
  BYTE *buffer = UserData.Page1.data;

  memset ( buffer, 0, sizeof (buffer ) );
  memset ( gPmbusCmd.MFR_RCB_INFO, 0, MFR_RCB_INFO_LEN + 1 );       //[davidchchen]20160304 added RCB

  RevCtrlBlockFun();            //[davidchchen]20160304 added RCB

  //Load Page1
  srcAddr.Val = USER_DATA_ADDR;
  ReadPM ( buffer, sizeof (UserData.Page1.data ), srcAddr );

  if (UserData.Page1.region.FRU.frudata[0] == 0xff && UserData.Page1.region.FRU.frudata[1] == 0xff)    //[davidchchen]20170921 Added
  {
        LoadIpmiDataFormat();                       //[davidchchen]20170921 Added
  }

  //Update FRU READ buffer
  gPmbusCmd.FRU_READ[0] = FRU_READ_LEN;
  //memcpy ( ( BYTE* ) & gPmbusCmd.FRU_READ[1], ( BYTE* ) & UserData.Page1.region.FRU[0x00], FRU_READ_LEN );  //[davidchchen]20170921 Removed
  memcpy ( ( BYTE* ) & gPmbusCmd.FRU_READ[1], ( BYTE* ) & UserData.Page1.region.FRU.frudata[0x00], FRU_READ_LEN );  //[davidchchen]20170921 Added

  //Load Page2
  buffer = UserData.Page2.data;
  memset ( buffer, 0, sizeof (buffer ) );
  srcAddr.Val = MISC_ADDR;
  ReadPM ( buffer, sizeof (UserData.Page2.data ), srcAddr );

  //Load Page3
  buffer = UserData.Page3.data;
  memset ( buffer, 0, sizeof (buffer ) );
  srcAddr.Val = LOG_ADDR;
  ReadPM ( buffer, sizeof (UserData.Page3.data ), srcAddr );

  if ( UserData.Page2.region.Calibration_Flag.Val == 0xFFFF )
  {
      UserData.Page2.region.Calibration_Flag.Val = 0;
  }

  if ( UserData.Page2.region.VoutCommand.Val == 0xFFFF )
  {
    #if defined(Config_12V35_REF)                                   //[davidchchen]20160307 Added
        //UserData.Page2.region.VoutCommand.Val = DUTY_12V3_REF;    //[davidchchen] 20150105 modified
        UserData.Page2.region.VoutCommand.Val = DUTY_12V35_REF;    //[davidchchen]20160307 Added
    #elif defined(Config_12V25_REF)
        UserData.Page2.region.VoutCommand.Val = DUTY_12V25_REF;        //[davidchchen] 20151223 modified
    #elif defined(Config_12V5_REF)
        UserData.Page2.region.VoutCommand.Val = DUTY_12V5_REF;        //[davidchchen] 20150518 modified
    #endif

  }

  if ( UserData.Page2.region.VoutCommand_LinearFmt.Val == 0xFFFF )
  {
    #if defined(Config_12V35_REF)                                   //[davidchchen]20160307 Added
        //UserData.Page2.region.VoutCommand_LinearFmt.Val = 0x0C4D; //[davidchchen] 20150106 Modified, 12.3 * 256 = 3149 (0x0c4d)
        UserData.Page2.region.VoutCommand_LinearFmt.Val = 0x0C5a; //[davidchchen]20160307 Added, 12.35 * 256 = 3162 (0x0c5a)
    #elif defined(Config_12V25_REF)
        UserData.Page2.region.VoutCommand_LinearFmt.Val = 0x0C40; //[davidchchen] 20151223 Modified, 12.25 * 256 = 3136 (0x0c40)
    #elif defined(Config_12V5_REF)
        UserData.Page2.region.VoutCommand_LinearFmt.Val = 0x0C80; //[davidchchen] 20150518 Modified, 12.5 * 256 = 3200 (0x0c80)
    #endif
  }

  //[davidchchen]20170921 added RCB
  PS.Compatible_code = 1;
  if ( UserData.Page1.region.HW_CC_HB == 0xff)      //[davidchchen]20170921 added
  {
      PS.Compatible_code = 0;
      _SD_Flag.OPP_SD = 0;
      HWFW_conflictStatus.bits.b1 = 0;  //HW compatible code conflict, 0: not conflict, 1: conflict
  }
  else
  {
      if ( (UserData.Page1.region.HW_CC_HB == tsRevCtrlBlock.pu8HwCompCode[0] ) && (UserData.Page1.region.HW_CC_LB == tsRevCtrlBlock.pu8HwCompCode[1] ) )   //[davidchchen]20170921 added
      {
          PS.Compatible_code = 0;
          _SD_Flag.OPP_SD = 0;
          HWFW_conflictStatus.bits.b1 = 0;  //HW compatible code conflict, 0: not conflict, 1: conflict
      }
      else
      {
          PS.Compatible_code = 1;
          _SD_Flag.OPP_SD = 1;
          HWFW_conflictStatus.bits.b1 = 1;  //HW compatible code conflict, 0: not conflict, 1: conflict
      }
  }

  gPmbusCmd.MFR_RCB_INFO[0] = MFR_RCB_INFO_LEN ;    //[davidchchen]20170921 added RCB

  //[davidchchen]20160304 added RCB
  strncpy ( ( char * ) &gPmbusCmd.MFR_RCB_INFO[1], ( char * ) &tsRevCtrlBlock.pu8FwCompCode, FW_CC_LEN );                 //FW compatible code
  //strncpy ( ( char * ) &gPmbusCmd.MFR_RCB_INFO[1+FW_CC_LEN], ( char * ) &UserData.Page1.data[0], HW_CC_LEN );       //HW compatible code, //[davidchchen]20170921 Removed
  strncpy ( ( char * ) &gPmbusCmd.MFR_RCB_INFO[1+FW_CC_LEN], ( char * ) &UserData.Page1.region.HW_CC_HB, HW_CC_LEN );       //HW compatible code, //[davidchchen]20170921 Added
  HWFW_conflictStatus.bits.b0 = 0;
  strncpy ( ( char * ) &gPmbusCmd.MFR_RCB_INFO[MFR_RCB_INFO_LEN], ( char * ) &HWFW_conflictStatus.Val, 1 );            //FW/HW conflict status

//  //[davidchchen]20150112 Added Compatible Code to match HW version
//  PS.Compatible_code = 1;
//  strncpy ( ( char * ) &HW_CompatibleCodeStr[0], ( char * ) HW_Compatible_Code, 2 ); //
//
//  if ( UserData.Page1.data[0] == 0xff)
//  {
//      PS.Compatible_code = 0;
//      _SD_Flag.OPP_SD = 0;
//  }
//  else
//  {
//      if ( (UserData.Page1.data[0] == HW_CompatibleCodeStr[0]) && (UserData.Page1.data[1] == HW_CompatibleCodeStr[1] ) )
//      {
//          PS.Compatible_code = 0;
//          _SD_Flag.OPP_SD = 0;
//      }
//      else
//      {
//          PS.Compatible_code = 1;
//          _SD_Flag.OPP_SD = 1;
//      }
//  }
//

  //Load 12V Iout Calibration data
  //Iout1
  if ( UserData.Page1.region.Iout[OUTPUT_12V].slope == 0xFFFF )
  {
      //default slope value is 1 gain by 1024 => 1<<10 = 1024
      UserData.Page1.region.Iout[OUTPUT_12V].slope = 1024;
  }
  if ( UserData.Page1.region.Iout[OUTPUT_12V].slope == 1024 )
  {
      if ( UserData.Page2.region.Calibration_Flag.bits.b2 == 1 )
      {
          PS.IOUT1_CALIBRATED = TRUE;
      }
      else
      {
          PS.IOUT1_CALIBRATED = FALSE;
      }
  }
  else
  {
      PS.IOUT1_CALIBRATED = TRUE;
  }
  if ( UserData.Page1.region.Iout[OUTPUT_12V].Offset == 0xFFFFFFFF )
  {
      //default offset value is 0
      UserData.Page1.region.Iout[OUTPUT_12V].Offset = 0;
  }

  //Iout2
  if ( UserData.Page1.region.Iout[OUTPUT_12V].slope2 == 0xFFFF )
  {
      //default slope value is 1 gain by 1024 => 1<<10 = 1024
      UserData.Page1.region.Iout[OUTPUT_12V].slope2 = 1024;
  }
  if ( UserData.Page1.region.Iout[OUTPUT_12V].slope2 == 1024 )
  {
      if ( UserData.Page2.region.Calibration_Flag.bits.b3 == 1 )
      {
          PS.IOUT2_CALIBRATED = TRUE;
      }
      else
      {
          PS.IOUT2_CALIBRATED = FALSE;
      }
  }
  else
  {
      PS.IOUT2_CALIBRATED = TRUE;
  }
  if ( UserData.Page1.region.Iout[OUTPUT_12V].Offset2 == 0xFFFFFFFF )
  {
      //default offseet value is 0
      UserData.Page1.region.Iout[OUTPUT_12V].Offset2 = 0;
  }

  //Iout3
  if ( UserData.Page1.region.Iout[OUTPUT_12V].slope3 == 0xFFFF )
  {
      //default slope value is 1 gain by 1024 => 1<<10 = 1024
      UserData.Page1.region.Iout[OUTPUT_12V].slope3 = 1024;
  }
  if ( UserData.Page1.region.Iout[OUTPUT_12V].slope3 == 1024 )
  {
      if ( UserData.Page2.region.Calibration_Flag.bits.b4 == 1 )
      {
          PS.IOUT3_CALIBRATED = TRUE;
      }
      else
      {
          PS.IOUT3_CALIBRATED = FALSE;
      }
  }
  else
  {
      PS.IOUT3_CALIBRATED = TRUE;
  }
  if ( UserData.Page1.region.Iout[OUTPUT_12V].Offset3 == 0xFFFFFFFF )
  {
      //default offseet value is 0
      UserData.Page1.region.Iout[OUTPUT_12V].Offset3 = 0;
  }

  //Iout4
  if ( UserData.Page1.region.Iout[OUTPUT_12V].slope4 == 0xFFFF )
  {
      //default slope value is 1 gain by 1024 => 1<<10 = 1024
      UserData.Page1.region.Iout[OUTPUT_12V].slope4 = 1024;
  }
  if ( UserData.Page1.region.Iout[OUTPUT_12V].slope4 == 1024 )
  {
      if ( UserData.Page2.region.Calibration_Flag.bits.b7 == 1 )
      {
          PS.IOUT4_CALIBRATED = TRUE;
      }
      else
      {
          PS.IOUT4_CALIBRATED = FALSE;
      }
  }
  else
  {
      PS.IOUT4_CALIBRATED = TRUE;
  }
  if ( UserData.Page1.region.Iout[OUTPUT_12V].Offset4 == 0xFFFFFFFF )
  {
      //default offseet value is 0
      UserData.Page1.region.Iout[OUTPUT_12V].Offset4 = 0;
  }

  //Load Current Share Calibration data
  if ( UserData.Page1.region.VCS_Slope == 0xFFFF )
  {
      //default slope value is 1 gain by 1024 => 1<<10 = 1024
      UserData.Page1.region.VCS_Slope = 1024;
  }
  if ( UserData.Page1.region.VCS_Slope == 1024 )
  {
      if ( UserData.Page2.region.Calibration_Flag.bits.b12 == 1 )
      {
          PS.CS_CALIBTATED = TRUE;
      }
      else
      {
          PS.CS_CALIBTATED = FALSE;
      }
  }
  else
  {
      PS.CS_CALIBTATED = TRUE;
  }

  if ( UserData.Page1.region.VCS_Offset == 0xFFFFFFFF )
  {
      //default offseet value is 0
      UserData.Page1.region.VCS_Offset = 0;
  }

  gVCS_Slope = UserData.Page1.region.VCS_Slope;
  gVCS_Offset = UserData.Page1.region.VCS_Offset;

  if ( UserData.Page1.region.Vout[OUTPUT_12V].CS_Slope == 0xFFFF )
  {
      //default slope value is 1 gain by 1024 => 1<<10 = 1024
      UserData.Page1.region.Vout[OUTPUT_12V].CS_Slope = 1024;
  }

  if ( UserData.Page1.region.Vout[OUTPUT_12V].CS_Offset == 0xFFFFFFFF )
  {
      //default offseet value is 0
      UserData.Page1.region.Vout[OUTPUT_12V].CS_Offset = 0;
  }

  CS_Slope_Temp = UserData.Page1.region.Vout[OUTPUT_12V].CS_Slope;
  CS_Offset_Temp = UserData.Page1.region.Vout[OUTPUT_12V].CS_Offset;


  //Load 12V Vout Calibration data
  if ( UserData.Page1.region.Vout[OUTPUT_12V].CmdOffset == 0xFFFF )
  {
      gVoutCmdOffset = 0;
  }
  else
  {
      gVoutCmdOffset = UserData.Page1.region.Vout[OUTPUT_12V].CmdOffset;
  }
  if ( UserData.Page1.region.Vout[OUTPUT_12V].ReportOffset == 0xFFFF )
  {
      gVoutReadOffsetAdc = 0;
  }
  else
  {
      gVoutReadOffsetAdc = UserData.Page1.region.Vout[OUTPUT_12V].ReportOffset;
  }

    #if RS_ENABLE   //[davidchchen] 20150108 not used
  if ( UserData.Page1.region.Vout[OUTPUT_12V].MainBusOffset == 0xFFFF )
  {
      gMainBusVoutOffset = 0;
  }
  else
  {
      gMainBusVoutOffset = UserData.Page1.region.Vout[OUTPUT_12V].MainBusOffset;
  }
    #endif


  if ( UserData.Page1.region.StbVoutOffset == 0xFFFF )
  {
      gStbVoutOffset = 0;
  }
  else
  {
      gStbVoutOffset = UserData.Page1.region.StbVoutOffset;
  }

  /*************************************************************************************************/

  //[Peter Chung] 20110304 added for initial blackbox data
  if ( UserData.Page2.data[0] == 0xFF
       && UserData.Page2.data[1] == 0xFF
       && UserData.Page2.data[2] == 0xFF
       && UserData.Page2.data[3] == 0xFF )
  {
      memset ( UserData.Page2.region.BlackBox.FaultRecord, 0, sizeof (UserData.Page2.region.BlackBox ) );
  }

#if 1
  if ( UserData.Page2.region.POS.Val == 0xFFFFFFFF )
  {
      UserData.Page2.region.POS.Val = 0;
  }
#endif
  //Load Bootloader version
  srcAddr.Val = BL_VER_ADDR;
  bl_version.Val = ReadLatch ( srcAddr.word.HW, srcAddr.word.LW );

  if ( bl_version.byte.LB == 0xFF )
  {
      UserData.Page2.region.BL_VER = * ( ( BYTE* ) "0" );
  }
  else
  {
      UserData.Page2.region.BL_VER = bl_version.byte.LB;
  }

  if ( UserData.Page2.region.FruModified_Flag == 0xFF )
  {
      UserData.Page2.region.FruModified_Flag = 0;
  }

  //Load Page3 Data
  if ( UserData.Page3.region.logGeneral.Latest_Log_Index == 0xFF )
  {
      memset ( UserData.Page3.data, 0, sizeof (UserData.Page3.data ) );
      UserData.Page3.region.logGeneral.Write_Cycles.Val = INIT_WRITE_CYCLES;
      UserData.Page3.region.logGeneral.Log_Sum = 0;
  }
}

#if SAMPLE_SET_SUPPORTED

void RefreshMfrSampleSets ( tFAULT_RECORD *pRecord )
{
  tSAMPLE_NODE* pNode = pStart;
  BYTE index;

  gStopSample = TRUE;

  for ( index = gSampleCount; index >= 1; index -- )
  {
      memcpy ( &( pRecord->MFR_SAMPLE_SET[index - 1] ), &pNode->node, sizeof (tSAMPLE_SET ) );
      if ( pNode->pNextNode != NULL )
      {
          pNode = ( tSAMPLE_NODE* ) ( pNode->pNextNode );
      }
      else
      {
          break;
      }
  }

  gStopSample = FALSE;
}
#endif

void CaptureFault ( )
{
  if ( PS.SaveBlackBox == FALSE )
  {
      gVoutAD_Fault = ADC.Vout;
      gIoutAD_Fault = ADC.Iout;
      PS.SaveBlackBox = TRUE;
  }
}

#if BLACKBOX_SUPPORTED

void SaveToBlackBox ( )
{
  tFAULT_RECORD *pRecord = & UserData.Page2.region.BlackBox.FaultRecord[0];
  BYTE i;

  //shift Record
  //for(i=MAX_FAULT_RECORD;i > 0;--i){	//[Peter Chung] 20101213 removed
  for ( i = ( MAX_FAULT_RECORD - 1 ) ; i > 0 ; i -- )
  {	//[Peter Chung] 20101213 modified
      UserData.Page2.region.BlackBox.FaultRecord[i] = UserData.Page2.region.BlackBox.FaultRecord[i - 1];
  }

#if SAMPLE_SET_SUPPORTED		
  RefreshMfrSampleSets ( pRecord );
#endif

  //Immediate Reporting
  {
      LONG result = 0;
      char N;

      /*****Vout Reporting*****/
      result = ( ( SHORT ) gVoutAD_Fault + ( SHORT ) gVoutReadOffsetAdc );
      if ( result < 0 )
      {
          result = 0;
      }
      ADC.Vout_Cal = result;

      result = gVout = result * ADC_TO_VOUT;

      //N of Vout is fixed -9, Y = (X << 9) >> VOUT_GAIN => Y = X >> 2
      //result = result >> 2;         //[davidchchen]20170310 Removed
      //N of Vout is fixed -8, Y = (X << 8) >> VOUT_GAIN => Y = X >> 3
      result = result >> 3;         //[davidchchen]20170310 Modify

      gPmbusCmd.READ_VOUT[0] = result & 0xFF;
      gPmbusCmd.READ_VOUT[1] = ( result >> 8 ) & 0xFF;

      /*****Iout Reporting*****/
      //result = ( ( ( LONG ) gIoutAD_Fault * UserData.Page1.region.Iout[OUTPUT_12V].slope + ( ( LONG ) ( ( ( LONGLONG ) UserData.Page1.region.Iout[OUTPUT_12V].Offset * IOUT_TO_ADC ) >> IOUT_TO_ADC_GAIN ) ) ) >> 10 );   //[davidchchen]20170310 Removed
      
      if (gIoutAD_Fault == 0)   //[davidchchen]20170310 Modify
      {
        result = 0;             //[davidchchen]20170310 Modify
      }
      else
      {
        if ( PS.SR_Enable == 1 )        //[davidchchen]20170310 Modify
        {
            result = ( ( ( LONG ) gIoutAD_Fault * UserData.Page1.region.Iout[OUTPUT_12V].slope + ( ( LONG ) ( ( ( LONGLONG ) UserData.Page1.region.Iout[OUTPUT_12V].Offset * IOUT_TO_ADC ) >> IOUT_TO_ADC_GAIN ) ) ) >> 10 );   //[davidchchen]20170310 Modify
        }
        else if ( PS.SR_Enable == 0 )   //[davidchchen]20170310 Modify
        {
            result = ( ( ( LONG ) gIoutAD_Fault * UserData.Page1.region.Iout[OUTPUT_12V].slope2 + ( ( LONG ) ( ( ( LONGLONG ) UserData.Page1.region.Iout[OUTPUT_12V].Offset2 * IOUT_TO_ADC ) >> IOUT_TO_ADC_GAIN ) ) ) >> 10 );  //[davidchchen]20170310 Modify
        }

      }

//      if ( result <= 0 )    //[davidchchen]20170310 Removed
//      {
//          result = 0;       //[davidchchen]20170310 Removed
//      }

      gIout = result = ( DWORD ) result * ADC_TO_IOUT;


      //N = GetN ( N_IOUT, ( WORD ) ( result >> ADC_TO_IOUT_GAIN ) );		//[Peter Chung] 20101022 test,  //[davidchchen]20170310 Removed
      //result = LinearFmt_XtoY ( result, N, ADC_TO_IOUT_GAIN );
      //[davidchchen]20170310 Removed
      //ADC_TO_IOUT_GAIN is 2^13, IOUT_MODE -> N is -7, should right shift 6
      result = result >> 6;     //[davidchchen]20170310 Modify
      
      gPmbusCmd.READ_IOUT[0] = result & 0xFF;
      gPmbusCmd.READ_IOUT[1] = ( result >> 8 ) & 0xFF;


      /*****Pout Reporting*****/
      result = ( DWORD ) ( ( ( QWORD ) gVout * gIout ) >> ADC_TO_IOUT_GAIN );	//divide total gain for gIout first to prevent DWORD overflow
      gPout = ( WORD ) ( result >> VOUT_GAIN );

      //N = GetN ( N_POUT, gPout );	//shift VOUT_GAIN here to get the real value,   //[davidchchen]20170310 Removed
      //result = LinearFmt_XtoY ( result, N, VOUT_GAIN );	//shift VOUT_GAIN here to get the real value, //[davidchchen]20170310 Removed

//      gPmbusCmd.READ_POUT[0] = result & 0xFF;                                 //[davidchchen]20170418 Removed
//      gPmbusCmd.READ_POUT[1] = ( result >> 8 ) & 0xFF;                        //[davidchchen]20170418 Removed

      gPmbusCmd.READ_POUT[0] = gPout & 0xFF;                                    //[davidchchen]20170418 Modify
      gPmbusCmd.READ_POUT[1] = ( gPout >> 8 ) & 0xFF;                           //[davidchchen]20170418 Modify
  }
  
  memcpy ( pRecord->STATUS_WORD , ( BYTE* ) gPmbusCmd.STATUS_WORD , 2 );    //[davidchchen]20170310     AWS Need to Black Box Added
  memcpy ( pRecord->STATUS_VOUT , ( BYTE* ) gPmbusCmd.STATUS_VOUT , 1 );
  memcpy ( pRecord->STATUS_IOUT , ( BYTE* ) gPmbusCmd.STATUS_IOUT , 1 );
  memcpy ( pRecord->STATUS_INPUT , ( BYTE* ) gPmbusCmd.STATUS_INPUT , 1 );
  memcpy ( pRecord->STATUS_TEMPERATURE , ( BYTE* ) gPmbusCmd.STATUS_TEMPERATURE , 1 );
  memcpy ( pRecord->STATUS_CML , ( BYTE* ) gPmbusCmd.STATUS_CML , 1 );
  memcpy ( pRecord->STATUS_FAN_1_2 , &gPmbusCmd.STATUS_FAN_1_2.Val , 1 );
  //memcpy(pRecord->STATUS_OTHER , &gPmbusCmd.STATUS_OTHER.Val , 1);
  memcpy ( pRecord->READ_VIN , gPmbusCmd.READ_VIN , 2 );
  memcpy ( pRecord->READ_IIN , gPmbusCmd.READ_IIN , 2 );
  memcpy ( pRecord->READ_PIN , gPmbusCmd.READ_PIN , 2 );
  memcpy ( pRecord->READ_VOUT , gPmbusCmd.READ_VOUT , 2 );
  memcpy ( pRecord->READ_IOUT , gPmbusCmd.READ_IOUT , 2 );
  memcpy ( pRecord->READ_POUT , gPmbusCmd.READ_POUT , 2 );
  memcpy ( pRecord->READ_TEMPERATURE_1 , gPmbusCmd.READ_TEMPERATURE_1 , 2 );
  memcpy ( pRecord->READ_TEMPERATURE_2 , gPmbusCmd.READ_TEMPERATURE_2 , 2 );
  memcpy ( pRecord->READ_TEMPERATURE_3 , gPmbusCmd.READ_TEMPERATURE_3 , 2 );
  memcpy ( pRecord->READ_FAN_SPEED_1 , gPmbusCmd.READ_FAN_SPEED_1 , 2 );
  memcpy ( pRecord->READ_FAN_SPEED_2 , gPmbusCmd.READ_FAN_SPEED_2 , 2 );
  //memcpy ( pRecord->MFR_POS_TOTAL , &gPmbusCmd.MFR_POS_TOTAL.v[0] , 4 );        //[davidchchen]20170418 Removed
  memcpy ( pRecord->MFR_POS_TOTAL , gPmbusCmd.MFR_POS_TOTAL , 5 );        //[davidchchen]20170418 Added Black Box block read
  //memcpy ( pRecord->MFR_POS_LAST , &gPmbusCmd.MFR_POS_LAST.v[0] , 4 );        //[davidchchen]20170418 Removed
  memcpy ( pRecord->MFR_POS_LAST , gPmbusCmd.MFR_POS_LAST , 5 );          //[davidchchen]20170418 Added Black Box block read
//  memcpy ( pRecord->BMC_UNIX_TIMESTAMP , ( BYTE* ) gPmbusCmd.BMC_UNIX_TIMESTAMP.v[0] , 4 );     //[davidchchen]20170418 Removed
  memcpy ( pRecord->BMC_UNIX_TIMESTAMP , gPmbusCmd.BMC_UNIX_TIMESTAMP, 5 );     //[davidchchen]20170418 Added Black Box block read
  memcpy ( pRecord->MFR_FW_VERSION , gPmbusCmd.MFR_FW_VERSION , 9 );                            //[davidchchen]20170310     AWS Need to Black Box Added Block Read
  memcpy ( pRecord->DEBUG_INFO, ( BYTE* ) & _SD_Flag.Val, 2 );	//[Peter Chung] 20101008 added for debug

  PS.FlashWritePage2 = TRUE;

}
#endif

void SaveToLogContent ( )
{
  //Update LOG_INDEX
  if ( UserData.Page3.region.logGeneral.Latest_Log_Index < 23 )
  {
      if ( UserData.Page3.region.logGeneral.Log_Sum == 0 )
      {
          // not to increase index
      }
      else
      {
          UserData.Page3.region.logGeneral.Latest_Log_Index ++;
      }
  }
  else
  {
      UserData.Page3.region.logGeneral.Latest_Log_Index = 0;
  }

  //Update LOG_CONTENT
  {
      BYTE index;

      index = UserData.Page3.region.logGeneral.Latest_Log_Index;

      UserData.Page3.region.logContent[index].Index = index;
      UserData.Page3.region.logContent[index].Status_MFR_Specific = MFR_Status.Val;
      UserData.Page3.region.logContent[index].Status_Debug.Val = _SD_Flag.Val;
      if ( PS.isDCInput == FALSE )
      {
          UserData.Page3.region.logContent[index].Status_Word.Val = gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.Val;
      }
      else
      {
          UserData.Page3.region.logContent[index].Status_Word.Val = gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.Val;
      }

      //UserData.Page3.region.logContent[index].Happen_Time.Val = gPmbusCmd.MFR_POS_TOTAL.Val;    //[davidchchen]20170418 Removed
      UserData.Page3.region.logContent[index].Happen_Time.Val = MFR_POS_TOTAL.Val;    //[davidchchen]20170418 Added Black Box block read
  }

  //Update LOG_GENERAL
  if ( UserData.Page3.region.logGeneral.Log_Sum < 24 )
  {
      UserData.Page3.region.logGeneral.Log_Sum ++;
  }
  else
  {
      UserData.Page3.region.logGeneral.Log_Sum = 24;
  }

  UserData.Page3.region.logGeneral.Total_Output_Energy.Val = UserData.Page3.region.logGeneral.Total_Output_Power.Val / 3600;		//WH
 // UserData.Page3.region.logGeneral.Total_Operate_Time.Val = gPmbusCmd.MFR_POS_TOTAL.Val;    //[davidchchen]20170418 Removed
  UserData.Page3.region.logGeneral.Total_Operate_Time.Val = MFR_POS_TOTAL.Val;    //[davidchchen]20170418 Added Black Box block read

  PS.FlashWritePage3 = TRUE;
}

#if BLACKBOX_SUPPORTED

BYTE* GetDataFromBlackBox ( BYTE cmd )
{
  tFAULT_RECORD *pRecord;
  BYTE Index = gPmbusCmd.MFR_PAGE[0];
  if ( Index >= MAX_FAULT_RECORD ) return 0;

  pRecord = & UserData.Page2.region.BlackBox.FaultRecord[Index];
  switch ( cmd )
  {
      //[davidchchen]20170310     AWS Need to Black Box
      case 0x79: //STATUS_WORD                  //[davidchchen]20170310     AWS Need to Black Box
          return pRecord->STATUS_WORD;          //[davidchchen]20170310     AWS Need to Black Box
          break;
      case 0x7A: //STATUS_VOUT
          return pRecord->STATUS_VOUT;
          break;
      case 0X7B: //STATUS_IOUT
          return pRecord->STATUS_IOUT;
          break;
      case 0x7C: //STATUS_INPUT
          return pRecord->STATUS_INPUT;
          break;
      case 0x7D: //STATUS_TEMPERATURE
          return pRecord->STATUS_TEMPERATURE;
          break;
      case 0x7E: //STATUS_CML
          return pRecord->STATUS_CML;
          break;
#if 0
      case 0x7F: //STATUS_OTHER
          return pRecord->STATUS_OTHER;
          break;
#endif
      case 0x81: //STATUS_FANS_1_2
          return pRecord->STATUS_FAN_1_2;
          break;
      case 0x88: //READ_VIN
          return pRecord->READ_VIN;
          break;
      case 0x89: //READ_IIN
          return pRecord->READ_IIN;
          break;
      case 0x8B: //READ_VOUT
          return pRecord->READ_VOUT;
          break;
      case 0x8C: //READ_IOUT
          return pRecord->READ_IOUT;
          break;
      case 0x8D: //READ_TEMPERATURE_1
          return pRecord->READ_TEMPERATURE_1;
          break;
      case 0x8E: //READ_TEMPERATURE_2
          return pRecord->READ_TEMPERATURE_2;
          break;
      case 0x8F: //READ_TEMPERATURE_3
          return pRecord->READ_TEMPERATURE_3;
          break;
      case 0x90: //READ_FAN_SPEED1
          return pRecord->READ_FAN_SPEED_1;
          break;
      case 0x91: //READ_FAN_SPEED2
          return pRecord->READ_FAN_SPEED_2;
          break;
      case 0x96: //READ_POUT
          return pRecord->READ_POUT;
          break;
      case 0x97: //READ_PIN
          return pRecord->READ_PIN;
          break;
      //case 0xD5: //MFR_FW_VERSION
      case 0xD0: //MFR_FW_VERSION, [davidchchen]2014/12/11 modify,  
          return pRecord->MFR_FW_VERSION;                           
          break;                                                    
      case 0xE5: //MFR_POS_TOTAL
          return pRecord->MFR_POS_TOTAL;
          break;
      case 0xE6: //MFR_POS_LAST
          return pRecord->MFR_POS_LAST;
          break;
      //[davidchchen]20170310     AWS Need to Black Box
      case 0xE7: //BMC_UNIX_TIMESTAMP                   //[davidchchen]20170310     AWS Need to Black Box
          return pRecord->BMC_UNIX_TIMESTAMP;           //[davidchchen]20170310     AWS Need to Black Box
          break;
      case 0x77:
          return pRecord->DEBUG_INFO;
          break;

#if 0
      case 0x77: //DEBUG_INFO	[Peter Chung] 20101008 added for debug, only workable in factory mode
          return pRecord->DEBUG_INFO;
          break;
#endif
  }
  return 0;

}
#endif


#if SAMPLE_SET_SUPPORTED

void UpdateSampleSets ( )
{
  //Data Updated every 100 ms
  tSAMPLE_NODE* pCurr = NULL;

  if ( ! gStopSample )
  {
      //create a new sample node
      pCurr = ( tSAMPLE_NODE* ) malloc ( sizeof (tSAMPLE_NODE ) );

      //Fill data to node
      memcpy ( pCurr->node.READ_VIN, gPmbusCmd.READ_VIN, 2 );
      memcpy ( pCurr->node.READ_IIN, gPmbusCmd.READ_IIN, 2 );
      memcpy ( pCurr->node.READ_VOUT, gPmbusCmd.READ_VOUT, 2 );
      memcpy ( pCurr->node.READ_IOUT, gPmbusCmd.READ_IOUT, 2 );
      memcpy ( pCurr->node.READ_TEMPERATURE1, gPmbusCmd.READ_TEMPERATURE_1, 2 );
      memcpy ( pCurr->node.READ_TEMPERATURE2, gPmbusCmd.READ_TEMPERATURE_2, 2 );
      memcpy ( pCurr->node.READ_TEMPERATURE3, gPmbusCmd.READ_TEMPERATURE_3, 2 );
      memcpy ( pCurr->node.READ_FAN_SPEED_1, gPmbusCmd.READ_FAN_SPEED_1, 2 );
      memcpy ( pCurr->node.READ_POUT, gPmbusCmd.READ_POUT, 2 );
      memcpy ( pCurr->node.READ_PIN, gPmbusCmd.READ_PIN, 2 );

      pCurr->pNextNode = NULL;


      if ( gSampleCount == 0 )
      {
          pStart = pCurr;
          pEnd = pCurr;
          gSampleCount ++;
      }
      else if ( gSampleCount < MAX_SAMPLE_SET )
      {
          pEnd->pNextNode = ( BYTE* ) pCurr;
          pEnd = pCurr;
          gSampleCount ++;
      }
      else
      {
          tSAMPLE_NODE* pFree;
          pFree = pStart;
          pStart = ( tSAMPLE_NODE* ) ( pStart->pNextNode );
          //free oldest data node
          if ( pFree != NULL )
          {
              free ( pFree );
              pFree = NULL;
          }

          //Insert new data node
          pEnd->pNextNode = ( BYTE* ) pCurr;
          pEnd = pCurr;
      }
  }
}
#endif

