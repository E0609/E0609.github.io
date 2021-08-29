
#include "p33Fxxxx.h"
#include "define.h"
#include "dsp.h"

#include "pec.h"
#include "I2C.h"
#include "Pmbus.h"
#include "UserData.h"
#include "Query.h"
#include "Protection.h"
#include "Process.h"
#include <string.h>

//#define MajorRev_MinorRev	0x2000

//[Peter Chung] Add Begin ->
BYTE gSlaveAddrW = 0x00;
BYTE gSlaveAddrR = 0x00;

tD_I2C_DATA I2C;
static BYTE gDataLength = 0;
extern tPSU_STATE gPS_State;
extern tPS_FLAG PS;
WORD i2c_reset_cnt;

BYTE OneTimeFlag ;    //[davidchchen]20170216 added PSON Signal Enable/disable

static BYTE ADDR[16] = { 0xB0,	// Reserved     //[davidchchen]20170110 added, fix HW.
                        0xB0, 	// J1
                        0x00,	// Reserved
                        0xB2,	// J2
                        0x00,	// Reserved
                        0xB4,	// J3
                        0x00,	// Reserved
                        0xB6,	// J4
                        0x00,	// Reserved
                        0xB8,	// J5
                        0x00,	// Reserved
                        0xBA,	// J6
                        0x00,	// Reserved
                        0xBC,	// J7
                        0x00,	// Reserved
                        0xBE,	// J8
};



//[Peter Chung] Add End < -

void init_I2cStruct ( )
{
  I2C.cmdDir = CMD_R;
  I2C.dataIndex = 0;
  I2C.pRxBuf = NULL;
  I2C.pTxBuf = NULL;
  I2C.state = STATE_WAIT_FOR_CMD;
  I2C.isPecFail = FALSE;
  I2C.currentCmd = 0x00;
  I2C.isBlockMode = FALSE;
  I2C.accessNotAllow = FALSE; //[Peter Chung] 20100907 added
  I2C.PermitToWrite = FALSE;

  memset ( I2C.readBuffer, 0, sizeof (I2C.readBuffer ) );
  I2C.pRxBuf = I2C.readBuffer;
  I2C.PEC = 0;

  gDataLength = 0;
}

void init_I2C ( void )
{
  //[Peter Chung] I2C add begin->
  I2C1CON = 0; //reset by manual

  //I2C1CON = 0x9040;
  //I2C1CON = 0xB300;
  I2C1CON = 0xB340;
  IFS1bits.SI2C1IF = 0;
  IPC4bits.SI2C1IP = 4;
  IEC1bits.SI2C1IE = 1;

  i2c_reset_cnt ++;

  //[Peter Chung] I2C add end <-

  //[Peter Chung] 20101223 added for reset I2C structure variable
  init_I2cStruct ( );

  //[Peter Chung] 20110225 Stop Counting 25ms
  Protect.I2C_SCL_Fault.Flag = 0;     //[davidchchen]20160223_I2C
  Protect.I2C_SCL_Fault.delay = 0;    //[davidchchen]20160223_I2C
  //Protect.I2C_SDA_Fault.Flag = 0;     //[davidchchen] 20150107 removed
  //Protect.I2C_SDA_Fault.delay = 0;    //[davidchchen] 20150107 removed

  //[Peter Chung] 20110511 added
  PS.I2C_Processing = FALSE;
  
}

void GetI2cAddr ( )
{
  BYTE addr = 0x00;
  BYTE pin = 0x00;
  

  pin = ( iADDR0 << 3 ) | ( iADDR1 << 2 ) | ( iADDR2 << 1 ) | iADDR3;
  addr = ADDR[pin];
  I2C1ADD = addr >> 1;

  if ( OneTimeFlag == 1 )
  {
      //if ( I2C1ADD != 0x58 )                                          //[davidchchen]20170921 Removed PSON Signal Enable/disable
      if ( ( I2C1ADD == 0x5E ) ||  ( I2C1ADD == 0x5F ) )               //[davidchchen]20170921 added PSON Signal Enable/disable
      {
          PS.MFR_PSON_CONTROL = TRUE;    //[davidchchen]20170216 added PSON Signal Enable/disable
      }
//      else          //[davidchchen]20180801 addr pin debounce isse. removed
//      {
//          PS.MFR_PSON_CONTROL = FALSE;    //[davidchchen]20170216 added PSON Signal Enable/disable
//
//      }
      OneTimeFlag = 0;
  }

  gPmbusCmd.MFR_PSON_CONTROL[0] = PS.MFR_PSON_CONTROL;                          //[davidchchen]20180801 addr pin debounce isse. removed

  gSlaveAddrW = addr;
  gSlaveAddrR = addr + 1;
}

//[Peter Chung] Add Begin ->

static void TransmitData ( BYTE data )
{
  do
  {
      asm("clrwdt" ); //clear WDT
      //delay
      asm volatile("repeat #40" ); //Delay 10us

      I2C1STATbits.IWCOL = 0;
      I2C1TRN = data;
  }
  while ( I2C1STATbits.IWCOL );
}

void HoldSCL ( )
{
  //hold I2C
  //I2C1CONbits.STREN = 1;
  I2C1CONbits.SCLREL = 0;
}

void ReleaseSCL ( )
{
  //release I2C
  I2C1CONbits.SCLREL = 1;
  //I2C1CONbits.STREN = 0;
}

void CheckI2COV ( )
{
  BYTE Temp;

  if ( I2C1STATbits.I2COV )
  {
      Temp = I2C1RCV;
      I2C1STATbits.I2COV = 0; //[Peter Chung] 20100503 modified for buffer overflow issue.
  }
}

BYTE Get_BW_DataLen ( BYTE cmd )
{
  BYTE byte_cnt;

  switch ( cmd )
  {
      case 0x05:	//PAGE_PLUS_WRITE
          byte_cnt = 4;
          break;
      case 0x06:	//PAGE_PLUS_READ
          byte_cnt = 3;
          break;
      case 0x1A:	//QUERY
          byte_cnt = 2;
          break;
      case 0x30:	//COEFFICIENTS
          byte_cnt = 3;
          break;
//      case 0xE7:        //BMC_UNIX_TIMESTAMP        //[davidchchen]20170418 Removed
//          byte_cnt = 4;                             //[davidchchen]20170418 Removed
//          break;
      case 0xBE:	//GEN_CAL_R
          byte_cnt = 7;
          break;
      default:
          byte_cnt = 0;
          break;
  }

  return byte_cnt;

}

BYTE CheckDataContent ( BYTE cmd )
{
  switch ( cmd )
  {
      case 0x05:	//PAGE_PLUS_WRITE
          if ( I2C.readBuffer[0] != 0x03 &&
               I2C.readBuffer[0] != 0x04 )
          {
              return FALSE;
          }	//byte count
          if ( I2C.readBuffer[1] != 0x00 && I2C.readBuffer[1] != 0x01 )
          {
              return FALSE;
          }	//Page
          if ( I2C.readBuffer[0] == 0x03 )
          {
              if ( I2C.readBuffer[2] != 0x79 && 	//STATUS_WORD
                   I2C.readBuffer[2] != 0x7A && 	//STATUS_VOUT
                   I2C.readBuffer[2] != 0x7B && 	//STATUS_IOUT
                   I2C.readBuffer[2] != 0x7C && 	//STATUS_INPUT
                   I2C.readBuffer[2] != 0x7D && 	//STATUS_TEMPERATURE
                   I2C.readBuffer[2] != 0x7E )
              {		//STATUS_CML
                  return FALSE;
              }
          }
          if ( I2C.readBuffer[0] == 0x04 )
          {
              if ( I2C.readBuffer[3] != 0x79 && 	//STATUS_WORD
                   I2C.readBuffer[3] != 0x7A && 	//STATUS_VOUT
                   I2C.readBuffer[3] != 0x7B && 	//STATUS_IOUT
                   I2C.readBuffer[3] != 0x7C && 	//STATUS_INPUT
                   I2C.readBuffer[3] != 0x7D && 	//STATUS_TEMPERATURE
                   I2C.readBuffer[3] != 0x7E )
              {		//STATUS_CML
                  return FALSE;
              }
          }
          return TRUE;
          break;

      case 0x06:	//PAGE_PLUS_READ
          if ( I2C.readBuffer[0] != 0x02 && I2C.readBuffer[0] != 0x03 )
          {
              return FALSE;
          }	//byte count
          if ( I2C.readBuffer[1] != 0x00 && I2C.readBuffer[1] != 0x01 )
          {
              return FALSE;
          }	//Page
          if ( I2C.readBuffer[0] == 0x02 )
          {
              if ( I2C.readBuffer[2] != 0x79 && 	//STATUS_WORD
                   I2C.readBuffer[2] != 0x7A && 	//STATUS_VOUT
                   I2C.readBuffer[2] != 0x7B && 	//STATUS_IOUT
                   I2C.readBuffer[2] != 0x7C && 	//STATUS_INPUT
                   I2C.readBuffer[2] != 0x7D && 	//STATUS_TEMPERATURE
                   I2C.readBuffer[2] != 0x7E )
              {		//STATUS CML
                  return FALSE;
              }
          }
          if ( I2C.readBuffer[0] == 0x03 )
          {
              if ( I2C.readBuffer[3] != 0x7A && 	//STATUS_VOUT
                   I2C.readBuffer[3] != 0x7B && 	//STATUS_IOUT
                   I2C.readBuffer[3] != 0x7C &&	//STATUS_INPUT
                   I2C.readBuffer[3] != 0x7D &&	//STATUS_TEMPERATURE
                   I2C.readBuffer[3] != 0x7E )
              { 	//STATUS_CML
                  return FALSE;
              }
          }
          return TRUE;
          break;

      case 0x1A:	//QUERY
          if ( I2C.readBuffer[0] != 0x01 )
          {
              return FALSE;
          }	//byte count
          return TRUE;
          break;
      case 0x1B:	//SMBALERT_MASK
          return TRUE;
          break;

      case 0x30:	//COEFFICIENTS
          if ( I2C.readBuffer[0] != 0x02 )
          {
              return FALSE;
          }	//byte count
          if ( I2C.readBuffer[1] != 0x86 && I2C.readBuffer[1] != 0x87 )
          {
              return FALSE;
          }	//Direct format command
          if ( I2C.readBuffer[2] != 0x00 && I2C.readBuffer[2] != 0x01 )
          {
              return FALSE;
          }	//Coefficient of W cmd or R cmd
          return TRUE;
          break;
//      case 0xE7:	//[davidchchen]20170418 Removed
//          return TRUE;  //[davidchchen]20170418 Removed
//          break;
      case 0xBE:	//GEN_CAL_R
          return TRUE;
          break;
      default:
          break;
  }

  return FALSE;
}

void __attribute__ ( ( __interrupt__, no_auto_psv ) ) _T4Interrupt ( void )
{
  /*Execute I2C command P received*/
  if ( I2C1STATbits.P )
  {

      //[Peter Chung] 20110225 Stop Counting 25ms
      //Protect.I2C_SCL_Fault.Flag = 0;     //[davidchchen] 20150107 removed
      //Protect.I2C_SCL_Fault.delay = 0;    //[davidchchen] 20150107 removed
      //Protect.I2C_SDA_Fault.Flag = 0;     //[davidchchen] 20150107 removed
      //Protect.I2C_SDA_Fault.delay = 0;    //[davidchchen] 20150107 removed


      if ( I2C.cmdDir == CMD_W && I2C.PermitToWrite == TRUE )
      {

          //Check Write Protect
          if ( ! gIsFactoryMode )
          {
              if ( gPmbusCmd.WRITE_PROTECT[0] == 0x80 )
              {
                  if ( I2C.currentCmd != 0x10 && I2C.currentCmd != 0xC9 )
                  {
                      gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_CMD = 1;
                      goto SKIP;
                  }
              }
              else if ( gPmbusCmd.WRITE_PROTECT[0] == 0x40 )
              {
                  if ( I2C.currentCmd != 0x10 && I2C.currentCmd != 0x01 && I2C.currentCmd != 0x00 && I2C.currentCmd != 0xC9 )
                  {
                      gPagePlusStatus.PAGE[PAGE0].STATUS_CML.bits.INVALID_CMD = 1;
                      gPagePlusStatus.PAGE[PAGE1].STATUS_CML.bits.INVALID_CMD = 1;
                      goto SKIP;
                  }
              }
              else if ( gPmbusCmd.WRITE_PROTECT[0] == 0x20 )
              {
                  if ( I2C.currentCmd != 0x10 && I2C.currentCmd != 0x01 && I2C.currentCmd != 0x00 &&
                       I2C.currentCmd != 0x02 && I2C.currentCmd != 0x21 && I2C.currentCmd != 0xC9 )
                  {
                      gPagePlusStatus.PAGE[PAGE0].STATUS_CML.bits.INVALID_CMD = 1;
                      gPagePlusStatus.PAGE[PAGE1].STATUS_CML.bits.INVALID_CMD = 1;
                      goto SKIP;
                  }

              }
              else if ( gPmbusCmd.WRITE_PROTECT[0] == 0x00 )
              {
                  //Enable write commands
              }
              else
              {
                  gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_CMD = 1;
                  goto SKIP;
              }
          }

          //[Peter Chung] 20110126 added to avoid invalid data write to I2C buffer
          if ( I2C.isBlockMode )
          {
              if ( ( I2C.dataIndex == gDataLength ) || ( I2C.dataIndex == gDataLength + 1 ) )
              {		//with PEC or without PEC byte
                  //Check data content
                  if ( CheckDataContent ( I2C.currentCmd ) )
                  {
                      //Data is correct
                  }
                  else
                  {
                      //Data is invalid
                      gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
                      goto SKIP;
                  }
              }
              else
              {
                  gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
                  goto SKIP;
              }
          }
          else
          {
              if ( ! gIsFactoryMode )
              {
                  if ( ( I2C.dataIndex == gDataLength ) || ( I2C.dataIndex == gDataLength + 1 ) )
                  {
                      //Do nothing
                  }
                  else
                  {
                      gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
                      goto SKIP;
                  }
              }
          }

          if ( ( ( tQuery ) gCmd[I2C.currentCmd].type ).bits.SupportWrite == 0 )
          {
              goto SKIP;
          }

          //Check if PEC fail or access not allowed
          if ( I2C.isPecFail || I2C.accessNotAllow )
          {
              goto SKIP;
          }

          HoldSCL ( );
          WriteCmdHandler ( );	//execute write cmd here
          ReleaseSCL ( );
      }
  }
SKIP:
  IFS1bits.T4IF = 0;
}

void __attribute__ ( ( __interrupt__, no_auto_psv ) ) _SI2C1Interrupt ( )
{
  BYTE Temp;

  //[Peter Chung] 20100601 HoldSCL();

  //[Peter Chung] 20110225 Stop Counting 25ms
  //Protect.I2C_SCL_Fault.Flag = 0;     //[davidchchen] 20150107 removed
  //Protect.I2C_SCL_Fault.delay = 0;    //[davidchchen] 20150107 removed
  //Protect.I2C_SDA_Fault.Flag = 0;       //[davidchchen] 20150107 removed
  //Protect.I2C_SDA_Fault.delay = 0;      //[davidchchen] 20150107 removed

  //[Peter Chung] 20130704 RMC remove should turn on the PSU.
  //Protect.RMC_Removed.Flag = 0;   //[davidchchen]20170216 Removed
  //Protect.RMC_Removed.delay = 0;  //[davidchchen]20170216 Removed

  if ( ( I2C1STATbits.R_W == 0 ) && ( I2C1STATbits.D_A == 0 ) )	//Address matched
  {
      Protect.I2C_SCL_Fault.Flag = 0;   //[davidchchen]20160321 changed
      Protect.I2C_SCL_Fault.delay = 0;  //[davidchchen]20160321 changed

      //Initialization
      Temp = I2C1RCV;

      init_I2cStruct ( );

      //[Peter Chung] 20110511 added
      //PS.I2C_Processing = TRUE;       //[davidchchen] 20170914 Removed

      CalcPEC ( &I2C.PEC, gSlaveAddrW );
  }
  else if ( ( I2C1STATbits.R_W == 0 ) && ( I2C1STATbits.D_A == 1 ) ) //Write, check for data
  {

      BYTE read_byte = I2C1RCV;

      I2C.cmdDir = CMD_W;	//[Peter Chung] 20100730 modified

      if ( I2C.state == STATE_WAIT_FOR_CMD )
      {
          I2C.currentCmd = read_byte;
          CalcPEC ( &I2C.PEC, I2C.currentCmd );

          I2C.state = STATE_WAIT_FOR_WRITE_DATA;	//asume next will be write data

          //Check if facotry mode or not
          if ( IsFactoryCmd ( I2C.currentCmd ) )
          {
              if ( ! gIsFactoryMode )
              {
                  //treat it as invalid command
                  gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_CMD = 1;
                  I2C.accessNotAllow = TRUE;	//[Peter Chung] 20100907 added
                  goto OUT;
              }
          }

          //Check commands can be executed in page1 or not
          if ( isPermitToReadInPage1 ( I2C.currentCmd ) == FALSE && isPermitToWriteInPage1 ( I2C.currentCmd ) == FALSE )
          {
              if ( gPmbusCmd.PAGE[0] == 1 )
              {
                  gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_CMD = 1;
                  I2C.accessNotAllow = TRUE;	//[Peter Chung] 20100907 added
                  goto OUT;
              }
          }

          //[Peter Chung] 20100930 added
          if ( ( ( tQuery ) gCmd[I2C.currentCmd].type ).bits.SupportCmd == FALSE )
          {
              gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_CMD = 1;
              I2C.accessNotAllow = TRUE;	//[Peter Chung] 20100907 added
              goto OUT;
          }

          if ( gCmd[I2C.currentCmd].pBuf != NULL )
          {

              if ( gPmbusCmd.MFR_PAGE[0] != 0xFF )	//for Blackbox
              {
#if BLACKBOX_SUPPORTED
                  I2C.pTxBuf = GetDataFromBlackBox ( I2C.currentCmd );
#endif

                  if ( I2C.pTxBuf == NULL )
                  {
                      //if the command is not support for BlackBox, point it to the specific data buffer
                      I2C.pTxBuf = gCmd[I2C.currentCmd].pBuf;	//assgin TxBuffer point to the specified Cmd buffer for reading
                  }
              }
              else
              {
                  I2C.pTxBuf = gCmd[I2C.currentCmd].pBuf;	//assgin TxBuffer point to the specified Cmd buffer for reading
              }


#if 0           //[davidchchen]20160503 removed
              if ( IsReportCmd ( I2C.currentCmd ) )
              {
                  if ( PS.ReportDataUpdating )
                  {
                      ScanProcess ( );
                  }
              }
#endif

#if 1
              if ( ! PS.EIN_DataUpdating && PS.EIN_DataUpdated )
              {
                  PS.EIN_DataUpdated = 0;
                  memcpy ( gPmbusCmd.READ_EIN_TEMP, gPmbusCmd.READ_EIN, 7 );
              }

              if ( ! PS.EOUT_DataUpdating && PS.EOUT_DataUpdated )
              {
                  PS.EOUT_DataUpdated = 0;
                  memcpy ( gPmbusCmd.READ_EOUT_TEMP, gPmbusCmd.READ_EOUT, 7 );
              }

              if ( I2C.currentCmd == 0x86 )
              {
                  if ( PS.EIN_DataUpdating )
                  {
                      I2C.pTxBuf = gPmbusCmd.READ_EIN_TEMP;
                  }
                  else
                  {
                      I2C.pTxBuf = gPmbusCmd.READ_EIN;
                  }
              }
              if ( I2C.currentCmd == 0x87 )
              {
                  if ( PS.EOUT_DataUpdating )
                  {
                      I2C.pTxBuf = gPmbusCmd.READ_EOUT_TEMP;
                  }
                  else
                  {
                      I2C.pTxBuf = gPmbusCmd.READ_EOUT;
                  }
              }
#endif
          }
          else
          {
              if ( ! IsSendCmd ( I2C.currentCmd ) )
              {
                  gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
                  I2C.accessNotAllow = TRUE;	//[Peter Chung] 20100907 added
                  goto OUT;
              }
              else
              {
                  I2C.PermitToWrite = TRUE;
              }
          }

      }

      else if ( I2C.state == STATE_WAIT_FOR_WRITE_DATA )
      {
          //Check if in standby mode or not to make sure the command valid to access or not...
          //if ( gPS_State.mainState == STATE_NORMAL )  //[davidchchen]20160303 removed
          #if 0     //[davidchchen]20160429  STATUS_CML can be settign NORMAL and STANDBY mode
          if ( ( gPS_State.mainState == STATE_NORMAL ) || ( gPS_State.mainState == STATE_STANDBY ) )    //[davidchchen]20160303 STATUS_CML ISSUE
          {
              //Check if cmd support write or process call command
#if 1
              if ( ( ( tQuery ) gCmd[I2C.currentCmd].type ).bits.SupportWrite == 0 &&
                   ! ( CheckBlockMode ( I2C.currentCmd ) ) )
              {
                  gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
                  I2C.accessNotAllow = TRUE;	//[Peter Chung] 20100907 added
                  goto OUT;
              }

                #if !Config_STATUS_ALL_READONLY     //[davidchchen]20160303 STATUS_CML ISSUE
              else if ( ( ( ( tQuery ) gCmd[I2C.currentCmd].type ).bits.SupportWrite == TRUE ) && ( ( ( tQuery ) gCmd[I2C.currentCmd].type ).bits.SupportRead == TRUE ) &&
                   ( ! ( CheckBlockMode ( I2C.currentCmd ) ) ) )
              {
                  I2C.accessNotAllow = TRUE;	//[Peter Chung] 20100907 added
                  goto OUT;

              }
                #endif
#endif
#if 0
              if ( ( ( tQuery ) gCmd[I2C.currentCmd].type ).bits.SupportWrite == TRUE ||
                   CheckBlockMode ( I2C.currentCmd ) )
              {
                  //Do nothing
              }
              else
              {
                  gPmbusCmd.STATUS_CML.bits.INVALID_DATA = 1;
                  I2C.accessNotAllow = TRUE;	//[Peter Chung] 20100907 added
                  goto OUT;
              }
#endif
          }
          else
          {
#if 0
              if ( ( ( tQuery ) gCmd[I2C.currentCmd].type ).bits.StbSupported == R ||
                   ( ( tQuery ) gCmd[I2C.currentCmd].type ).bits.StbSupported == NOT_SUPPORTED )
              {

                  gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
                  I2C.accessNotAllow = TRUE;	//[Peter Chung] 20100907 added
                  goto OUT;
              }
#endif  
          }

          #else   //[davidchchen]20160429  STATUS_CML can be settign all mode

          if ( ( ( tQuery ) gCmd[I2C.currentCmd].type ).bits.SupportWrite == 0 &&
                   ! ( CheckBlockMode ( I2C.currentCmd ) ) )
              {
                  gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
                  I2C.accessNotAllow = TRUE;	//[Peter Chung] 20100907 added
                  goto OUT;
              }

          #endif

          //Check if this is process call command or not here.
          if ( I2C.dataIndex == 0 )
          {	//enter only first time
              //Check if Block-Write-Block-Read cmd
              I2C.isBlockMode = CheckBlockMode ( I2C.currentCmd );

              if ( I2C.isBlockMode )
              {
                  if ( I2C.currentCmd == 0x1B )
                  {
                      //Exception for 0x1B SMBALERT_MASK
                      //(support Write Word & BlockWrite-BlockRead at the same time)
                      if ( read_byte == 1 )
                      {
                          gDataLength = 2;	//BlockWrite-BlockRead mode, (block count byte + Status_x command code byte)
                      }
                      else
                      {
                          gDataLength = gCmd[I2C.currentCmd].Len;	//Write word mode
                      }
                  }
                  else if ( I2C.currentCmd == 0x05 )
                  {
                      //[Peter Chung] 20111005 add exception for PAGE_PLUS_WRITE SMBALERT_MASK
                      if ( read_byte == 3 )
                      {
                          gDataLength = 4;
                      }
                      else if ( read_byte == 4 )
                      {
                          gDataLength = 5;
                      }
                      else
                      {
                          gDataLength = gCmd[I2C.currentCmd].Len;
                      }
                  }
                  else if ( I2C.currentCmd == 0x06 )
                  {
                      //[Peter Chung] 20111005 add exception for PAGE_PLUS_READ SMBALERT_MASK
                      if ( read_byte == 2 )
                      {
                          gDataLength = 3;
                      }
                      else if ( read_byte == 3 )
                      {
                          gDataLength = 4;
                      }
                      else
                      {
                          gDataLength = gCmd[I2C.currentCmd].Len;
                      }
                  }
                  else if ( I2C.currentCmd == 0xE7 )
                  {

                      if ( read_byte == 4 )
                      {
                          gDataLength = 5;
                      }
                      else
                      {
                          gDataLength = gCmd[I2C.currentCmd].Len;
                      }
                  }
                  else
                  {
                      //gDataLength = read_byte + 1;	//get byte count here, 1 is the byte of byte count itself
                      gDataLength = Get_BW_DataLen ( I2C.currentCmd );	//[Peter Chung] 20110124 modified
                  }
              }
              else
              {
#if 0
                  if ( I2C.currentCmd == 0xF0 )
                  {	//Exception for 0xF0 PSU_FACTORY_MODE
                      gDataLength = 6;	//FACOTRY_MODE_KEY size("Liteon")
                  }
                  else
                  {
                      gDataLength = gCmd[I2C.currentCmd].Len;
                  }
#endif
                  gDataLength = gCmd[I2C.currentCmd].Len;
              }

          }

          if ( I2C.dataIndex < gDataLength )
          {
              *I2C.pRxBuf = read_byte;
              I2C.pRxBuf ++;
              CalcPEC ( &I2C.PEC, read_byte );
              if ( ! gIsFactoryMode )
              {
                  if ( I2C.dataIndex == ( gDataLength - 1 ) )
                  {
                      I2C.PermitToWrite = TRUE;
                  }
              }
              else
              {
                  I2C.PermitToWrite = TRUE;
              }
          }
          else if ( I2C.dataIndex == gDataLength )
          {	//recognized as PEC byte
              BYTE PEC_byte;

              PEC_byte = read_byte;

              //if ( ( PEC_byte != I2C.PEC ) && ( I2C.isBlockMode == 0 ) )
              if ( ( PEC_byte != I2C.PEC ) && ( I2C.isBlockMode == 0 || I2C.isBlockMode == 1) )
              {
                  I2C.isPecFail = TRUE;
                  gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.PEC_FAILED = 1;
              }

          }
          else
          {
              Temp = read_byte;	//dummy Rx byte
          }

          I2C.dataIndex ++;
      }

  }
  else if ( ( I2C1STATbits.R_W == 1 ) && ( I2C1STATbits.D_A == 1 ) )	//Read data
  {
      BYTE return_byte;

      //Temp = I2C1RCV;
      //I2C1STATbits.RBF = 0;
      //I2C1STATbits.I2COV = 0;

      //[Peter Chung] 20100930 added
      if ( I2C.pTxBuf == NULL || I2C.accessNotAllow )
      {
          return_byte = 0xFF;
          TransmitData ( return_byte );
          //ToDo : set status flag here
          goto OUT;
      }

      if ( I2C.dataIndex < gDataLength )
      {

          ////HoldSCL(); //[Peter Chung] 20100601

          //[Peter Chung] 20110511 added
          if ( I2C.dataIndex == ( gDataLength - 1 ) )
          {
              PS.I2C_Processing = FALSE;
          }

          if ( I2C.pTxBuf == NULL )
          {
              return_byte = 0xFF;
              TransmitData ( return_byte );
              //ToDo : set status flag here
              goto OUT;
          }
          else
          {
              return_byte = I2C.pTxBuf[I2C.dataIndex];
          }
          CalcPEC ( &I2C.PEC, return_byte );
      }
      else if ( I2C.dataIndex == gDataLength )
      {
          //return PEC byte
          return_byte = I2C.PEC;
      }
      else
      {
          //return dummy byte
          return_byte = 0xFF;
      }

      TransmitData ( return_byte );
      //I2C1CONbits.SCLREL = 1; 	//release clock line so MASTER can drive it
      I2C.dataIndex ++;
  }
  else if ( ( I2C1STATbits.R_W == 1 ) && ( I2C1STATbits.D_A == 0 ) )	//Read address
  {
      BYTE return_byte;

      ////HoldSCL(); //[Peter Chung] 20100601
      PS.I2C_Processing = TRUE;         //[davidchchen] 20170914 added

      Temp = I2C1RCV;

      I2C.cmdDir = CMD_R;
      I2C.dataIndex = 0;
      CalcPEC ( &I2C.PEC, gSlaveAddrR );

      //Check BlockWrite-BlockRead Cmd
      if ( I2C.isBlockMode )
      {
          //Handle BlockWritten data
          HandleWRBlock ( I2C.currentCmd );
          //update data length
          gDataLength = gCmd[I2C.currentCmd].pBuf[0] + 1;	//the first byte is byte count
      }
      else
      {
          gDataLength = gCmd[I2C.currentCmd].Len;
      }


      if ( I2C.pTxBuf == NULL || I2C.accessNotAllow )
      {
          return_byte = 0xFF;
          TransmitData ( return_byte );
          //ToDo : set status flag here
          goto OUT;
      }
      else
      {
          return_byte = I2C.pTxBuf[I2C.dataIndex];
      }

      CalcPEC ( &I2C.PEC, return_byte );

      TransmitData ( return_byte );
      //I2C1CONbits.SCLREL = 1;
      I2C.dataIndex ++;
  }


OUT:

#if 0
  if ( I2C1STATbits.I2COV )
  {
      Temp = I2C1RCV;
      I2C1STATbits.I2COV = 0; //[Peter Chung] 20100503 modified for buffer overflow issue.
  }
#endif

  //I2C1STATbits.RBF = 0;
  if ( I2C1STATbits.RBF )
  {
      Temp = I2C1RCV;
      I2C1STATbits.RBF = 0;
  }
  I2C1STATbits.IWCOL = 0;
  _SI2C1IF = 0;	//clear I2C1 Slave interrupt flag

  ReleaseSCL ( );
}


//[Peter Chung] Add End <-

