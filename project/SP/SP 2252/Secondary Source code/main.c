/***********************************************************************
 *
 *Copyright (C) 2012 Advanced Technology Development
 *                   Power SBG
 *                   LITE-ON TECHNOLOGY Corp.
 *
 *All Rights Reserved
 *
 *File Name : Main.c
 *
 *Date : 2012.05.24
 *
 *Author :
 *
 *Description :This Program used for HP 2400W Function Control Program.
 *
 **********************************************************************/

#include "p33Fxxxx.h"
#include "define.h"
#include "Init.h"
#include "I2c.h"
#include "Process.h"
#include "Fan.h"
#include "Protection.h"
#include "Parameter.h"
#include "PowerOnOff.h"
#include "Psfb.h"
#include "Isr.h"
#include "Sci.h"
#include "Timer.h"
#include "Standby.h"
#include "Pmbus.h"
#include "UserData.h"
#include "PowerOnOff.h"
#include "Memory.h"
#include "Status.h"
#include "Led.h"
#include "Pec.h"

_FGS ( GWRP_OFF )
_FOSCSEL ( FNOSC_FRC )
_FOSC ( FCKSM_CSECMD & OSCIOFNC_ON )
#if !Config_WATCHDOG_TIMER    //[davidchchen]20160412 added watch-dog timer
_FWDT ( FWDTEN_OFF )                                // default setting to WDTPRE_PR128, WDTPOST_PS32768,Use watchdog timer with 32768*4 ms = 128s period
#endif
_FPOR ( FPWRT_PWR2 )
_FICD ( ICS_PGD2 & JTAGEN_OFF )

#if Config_WATCHDOG_TIMER
_FWDT(FWDTEN_OFF & WDTPRE_PR32 & WDTPOST_PS512)  // Use watchdog timer with 1 ms period //[davidchchen]20160412 added watch-dog timer
#endif

//*****************************************************************************************//
//*****************************************************************************************//

//Global Variable -->
tPSU_STATE gPS_State = { STATE_INIT, STATE_SUB_NONE };
tPS_FLAG PS;
tPSFB PSFB;
static BYTE hTimerMainRaise = 0;	//[Peter Chung] 20100825 added
static BYTE hTimerStbRaise = 0;		//[Peter Chung] 20100825 added

//[davidchchen]20171006 Modify PIB CODE
//************************* PIB CODE **********************************************
//data structure of PIB code
//=====================================
typedef struct
{
	u8_t  pu8StartPattern[8];
	u8_t  pu16PIBVersion[2];
	u8_t  pu8DeviceType[1];
	u8_t  pu8McuType[1];
	u8_t  pu8CodeID[12];
	u8_t  pu8StartAddr[4];
	u8_t  pu8EndAddr[4];
//[davidchchen]20171006 Modify PIB CODE
	u8_t  pu16EraseTime[2];
	u8_t  pu16ProgramTime[2];
	u8_t  pu16ProgramLength[2];
	u8_t  pu8BufFillVal[1];
	u8_t  pu8Reserved[11];
	u8_t  pu8Checksum[2];
	u8_t  pu8StopPattern[8];
}sPIBStr_t;

const sPIBStr_t tDefaultPIB =
{
	{0xC0, 0x6C, 0x3A, 0x7B, 0xB2, 0x2F, 0x5D, 0x92},	// fixed start pattern
	{0x00, 0x01},                                           // PIB version
	{0x05},							// device type, PSU = 05
	{0x01},							// mcu type, PSU:1-secondary, 2-primary
	{0X00, 0X00, 'U', '1', 'L', '6',			// code id, PS22526L1U
	 '2', '5', '2', '2', 'S', 'P'},
	{0x00, 0x00, 0x00, 0x00},							// start address,0x00000000
	{0xFF, 0x57, 0x01, 0x00},							// end address,0x00ABFF *2 =0X00157FF
                                                        //[davidchchen]20171006 Modify PIB CODE
	{0x0F, 0x00},										// erase time, EX:1.5sec, unit:0.1sec
	{0x0A, 0x00},										// program time, EX:1.0msec, unit:0.1msec
	{0x10, 0x00},										// program length
	{0xFF},												// buffer_fill value
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00,				// reserved data
	 0x00, 0x00, 0x00, 0x00, 0x00},
	{0xFC, 0x04},										// checksum
	{0x40, 0x0E, 0x75, 0x1F, 0x71, 0x5E, 0x3A, 0x29}	// fixed stop pattern

};


//Global Variable <--

static void InitPSU ( )
{
  init_SetupClock ( );
  init_GPIO ( );
  #if Config_Input_CN       //[davidchchen]20160427 added Input Change Notification
  init_Input_CN();          //[davidchchen]20160427 added
  #endif
  LoadUserDataFromFlash ( );
  init_Timer ( );
  init_ADC ( );
  init_PWM ( );
  init_Fan ( );
  //init_Comparator ( );            //[davidchchen]20160427 removed
  //init_External_Interrupt ( );    //[davidchchen]20160427 removed
  init_Pmbus ( );
  init_I2C ( );
  //init_EEPROM();
  GetI2cAddr ( );
  init_Parameter ( );
  init_I2C ( );
  //init_ADE7953();
  init_UART1 ( DEFAULT_U1_BAUDRATE );
  init_UART2 ( DEFAULT_U2_BAUDRATE );
  init_Standby ( );
  init_Led ( );
  init_crc8 ( );	//[Peter Chung] added for PEC 20100506
  ClearAllStatus ( 0xFF );
  ResetSD_Flag ( );
  init_CurrentShareLoop ( );

  ADCONbits.ADON = 1;		//Turn on ADC module
  PTCONbits.PTEN = 1;		//Turn on PWM module

  ENABLE_UART1 ( );
  ENABLE_UART2 ( );

  //Initial main state
  gPS_State.mainState = STATE_STANDBY;
  gPS_State.subState = STATE_SUB_STANDBY_NORMAL;

  #if Config_WATCHDOG_TIMER
  //enable software watch dog timer
  _SWDTEN = 1;          //[davidchchen]20160412 added watch-dog timer
  #endif

}
//-------------------------Chech iAC_OK and iPFC_OK ------------------------------

static void CheckInputOK ( )
{
  static BYTE previousStatus = AC_N_GOOD;

  if ( iAC_OK == AC_GOOD )
  {
      if ( PS.VinOVFault == 1 || PS.VinUVFault == 1 )
      {
          oVIN_GOOD = VIN_N_GOOD;
      }
      else
      {
          oVIN_GOOD = VIN_GOOD;
      }

      if ( previousStatus == AC_N_GOOD )
      {
          //recover from AC_NOK
          if ( gPS_State.mainState == STATE_LATCH )
          {
              //leave Latch mode
              gPS_State.mainState = STATE_STANDBY;
              gPS_State.subState = STATE_SUB_STANDBY_NORMAL;
          }
      }

      if ( PS.AC_OFF_ClearFault )
      {
            PS.AC_OFF_ClearFault = FALSE;
            ResetSD_Flag ( );
      }

      gLedStatus.bits.ac_loss = 0;
      PS.AC_ON_TO_OFF = FALSE;

      MFR_Status.bits.AC_NOTOK = FALSE;

      previousStatus = AC_GOOD;
  }
  else
  {
      oVIN_GOOD = VIN_N_GOOD;

      if ( previousStatus == AC_GOOD )
      {
#if 0
          if ( _SD_Flag.Val == 0 )
          {
              PS.SaveBlackBox = 1;
          }
#endif
          PS.AC_ON_TO_OFF = TRUE;
      }

      gLedStatus.bits.ac_loss = 1;
      MFR_Status.bits.AC_NOTOK = TRUE;

      previousStatus = AC_N_GOOD;
  }

  //Check PFC Status
#if PFC_ENABLE
  if ( iPFC_OK == PFC_OK )
  {
      _SD_Flag.PFC_NOTOK = FALSE;
  }
  else
  {
      _SD_Flag.PFC_NOTOK = TRUE;
  }

  if ( Uart.U1.Rx.Pri_Status.bits.PFCDISABLE )
  {
      gPmbusCmd.MFR_PFC_DISABLE.bits.b0 = PFC_STATUS_DISABLE;	//update PFC status
  }
  else
  {
      gPmbusCmd.MFR_PFC_DISABLE.bits.b0 = PFC_STATUS_ENABLE;	//update PFC status
  }
#endif

  //Check LED Status
  //if ( _SD_Flag.LATCH_SD == FALSE && _SD_Flag.OTP_SD == FALSE && _SD_Flag.FAN_LOCK == FALSE && _SD_Flag.STB_OTP == FALSE && PSFB.MainEnabled == FALSE/* && STB.InRegulation*/ )
  if ( _SD_Flag.LATCH_SD == FALSE && _SD_Flag.OTP_SD == FALSE && _SD_Flag.FAN_LOCK == FALSE && PSFB.MainEnabled == FALSE/* && STB.InRegulation*/ )    //[David ch chen]20141124 removed _SD_Flag.STB_OTP, not use _SD_Flag.STB_OTP
  {
      gLedStatus.bits.standby = 1;
      //[davidchchen]20160513 Added, When STB_SCP,OCP,OVP,UVP setting LED TO solid yellow
      if (_SD_Flag.STB_OCP == TRUE || _SD_Flag.STB_OVP == TRUE || _SD_Flag.STB_UVP == TRUE )
      {
          PS.STB_Fault = 1;     //[davidchchen]20160513 Added, When STB_SCP,OCP,OVP,UVP setting LED TO solid yellow
      }
      else
      {
        PS.STB_Fault = 0;       //[davidchchen]20160513 Added, When STB_SCP,OCP,OVP,UVP setting LED TO solid yellow
      }
  }
  else
  {
      gLedStatus.bits.standby = 0;
  }

  if ( _SD_Flag.Val & 0x00007B9F )
  {
      gLedStatus.bits.fault = 1;
  }
  else
  {
      gLedStatus.bits.fault = 0;
  }

  if ( isWarning ( ) )
  {
      gLedStatus.bits.warning = 1;
  }
  else
  {
      gLedStatus.bits.warning = 0;
  }
}
//------------------------------------------------------------------------------------------------

static inline void U2_SendData ( )
{
  //Send data to U2Tx(Teridian side)
  if ( PS.IsTeridianCaliMode )
  {
      SCI_U2_Transmit ( );
  }
}

static void CheckPowerOn ( )
{
#if PFC_ENABLE
  //if(iPFC_OK == PFC_OK && _SD_Flag.Val == 0 && STB.InRegulation == TRUE && iAC_OK == AC_GOOD  && PS.VinOVFault == 0 && PS.VinUVFault == 0)	//Chris20130116 add AC_OK for dropout issue
  //if ( iPFC_OK == PFC_OK && ( _SD_Flag.Val & 0xDFFF ) == 0 && iAC_OK == AC_GOOD  && PS.VinOVFault == 0 && PS.VinUVFault == 0 )	//Chris20130116 add AC_OK for dropout issue
  if ( iPFC_OK == PFC_OK && ( _SD_Flag.Val & 0xDFFF ) == 0 && iAC_OK == AC_GOOD  && PS.VinOVFault == 0 && PS.VinUVFault == 0 && PS.Compatible_code == 0 )	//[davidchchen] 20150113 Added HW_FW_Compatible Code Function
#else
  if ( _SD_Flag.Val == 0 )
#endif
  {
#if 1	
      PowerOnSeq ( &hTimer._Ton_delay, Ton_delay, TurnOnOutput );
      gPS_State.mainState = STATE_ONING;
      gPS_State.subState = STATE_SUB_ON;
#endif
#if 0
      TurnOnOutput ( );
      oPS_OK = P_OK;
      gPS_State.mainState = STATE_NORMAL;
      gPS_State.subState = STATE_SUB_NORMAL;
#endif
  }
}
//------------------------------------------------------------------------------------------------

static void CheckPowerOff ( )
{
#if PFC_ENABLE
  //if(_SD_Flag.Val || iPFC_OK == PFC_NOK || PS.VinOVFault == 1 || PS.VinUVFault == 1)
  //if ( ( _SD_Flag.Val & 0xDFFF ) != 0 || iPFC_OK == PFC_NOK || PS.VinOVFault == 1 || PS.VinUVFault == 1 )   //[davidchchen] 20150113 Added HW_FW_Compatible Code Funtcion
  if ( ( _SD_Flag.Val & 0xDFFF ) != 0 || iPFC_OK == PFC_NOK || PS.VinOVFault == 1 || PS.VinUVFault == 1 || PS.Compatible_code == 1 )
#else
  if ( _SD_Flag.Val )
#endif
  {
      if ( _SD_Flag.LATCH_SD )
      {
          //if ( _SD_Flag.OVP_SD || _SD_Flag.UV_SD || _SD_Flag.OPP_SD || _SD_Flag.OCP_SD || _SD_Flag.UV_SD || _SD_Flag.SCP_SD )
          if ( _SD_Flag.OVP_SD || _SD_Flag.UV_SD || _SD_Flag.OCP_SD || _SD_Flag.SCP_SD )   //[David ch chen]20141126 modified, reomoved _SD_Flag.OPP_SD not use
          {
              gPS_State.mainState = STATE_LATCH;
              gPS_State.subState = STATE_SUB_PSON_H;
          }
          else
          {
              gPS_State.mainState = STATE_STANDBY;
              gPS_State.subState = STATE_SUB_STANDBY_NORMAL;
          }
      }
      else
      {
#if 0
          EmergencyPowerOff ( );

          gPS_State.mainState = STATE_STANDBY;
          gPS_State.subState = STATE_SUB_STANDBY_NORMAL;
#endif

          //Including PSON, PSKILL, OTP, FAN_LOCK
          //Turn Off PSU
          if ( _SD_Flag.STB_OCP == FALSE )
          {
              if ( gPS_State.mainState != STATE_STANDBY )
              {
                  PowerOffSeq ( &hTimer._Tpwok_off, Tpwok_off, ISR_Tpwok_off );
                  gPS_State.mainState = STATE_OFFING;
                  gPS_State.subState = STATE_SUB_NONE;
              }
          }
      }
  }
}
//------------------------------------------------------------------------------------------------

static void CheckNextState ( )
{
  if ( _SD_Flag.LATCH_SD == 1 )
  {
      gPS_State.mainState = STATE_LATCH;
      gPS_State.subState = STATE_SUB_PSON_H;
  }
  else
  {
      gPS_State.mainState = STATE_STANDBY;
      gPS_State.subState = STATE_SUB_STANDBY_NORMAL;
  }
}

//------------------------------------------------------------------------------------------------

static void CheckSwitch ( )
{
  static BYTE prePSONState = PS_ON_H;
  static BYTE preOPState = OP_H;
  static BYTE preOP = 0x80;
  BYTE pson_state_changed = FALSE;
  BYTE op_state_changed = FALSE;

  //------------------ iPS_ON ------------------------------------------
/*
  if ( iPS_ON == HIGH )
  {
      if ( PS.PS_ON_OFF == PS_ON_L )
      {
          Protect.PSON_H.Flag = 1;
      }
      Protect.PSON_L.Flag = 0;
      Protect.PSON_L.delay = 0;
  }
  else
  {
      if ( PS.PS_ON_OFF == PS_ON_H )
      {
          Protect.PSON_L.Flag = 1;
      }
      Protect.PSON_H.Flag = 0;
      Protect.PSON_H.delay = 0;
  }
*/

  if(PS.MFR_PSON_CONTROL == 1 )         //[davidchchen]20170216 added PSON Signal Enable/disable
  {
        if ( iPS_ON == HIGH )
        {
          if ( PS.PS_ON_OFF == PS_ON_L )
          {
              Protect.PSON_H.Flag = 1;
          }
          Protect.PSON_L.Flag = 0;
          Protect.PSON_L.delay = 0;
        }
        else
        {
          if ( PS.PS_ON_OFF == PS_ON_H )
          {
              Protect.PSON_L.Flag = 1;
          }
          Protect.PSON_H.Flag = 0;
          Protect.PSON_H.delay = 0;
        }
  }
  else
  {
      _SD_Flag.PS_ON_OFF = TRUE;
      PS.PS_ON_OFF = PS_ON_L;
      pson_state_changed = FALSE;
      PS.Stb_PSON_L = 1;
      PS.Stb_OP_OFF = 0;
  }

  if ( prePSONState != PS.PS_ON_OFF )
  {
      pson_state_changed = TRUE;
      prePSONState = PS.PS_ON_OFF;
  }

  // PMBus OPERATION
  //if ( preOP != gPmbusCmd.OPERATION[0] || PS.op_cmdChanged == TRUE )
  if ( (PS.MFR_PSON_CONTROL == 1 ) && ( preOP != gPmbusCmd.OPERATION[0] || PS.op_cmdChanged == TRUE ) )     //[davidchchen]20170216 added PSON Signal Enable/disable
  {
      op_state_changed = TRUE;
      PS.op_cmdChanged = FALSE;

      if ( gPmbusCmd.OPERATION[0] == 0x00 || gPmbusCmd.OPERATION[0] == 0x40 )
      {
          preOPState = OP_H;
      }
      else if ( gPmbusCmd.OPERATION[0] == 0x80 || gPmbusCmd.OPERATION[0] == 0x94 ||
            gPmbusCmd.OPERATION[0] == 0x98 || gPmbusCmd.OPERATION[0] == 0xA4 || gPmbusCmd.OPERATION[0] == 0xA8 )
      {
          preOPState = OP_L;
      }

      preOP = gPmbusCmd.OPERATION[0];
  }

  if ( _SD_Flag.PS_ON_OFF == TRUE )
  {
      /*Turn On PSU*/
      if ( pson_state_changed == TRUE && prePSONState == PS_ON_H )
      //if ( pson_state_changed == TRUE && prePSONState == PS_ON_H && PS.MFR_PSON_CONTROL == 1)
      {
          _SD_Flag.PS_ON_OFF = FALSE;
          PS.Stb_PSON_L = 0;
          PS.Stb_OP_OFF = 0;
          //ClearAllStatus(0xFF);
          PSOnOff_ResetSD_Flag ( );
          pson_state_changed = FALSE;

          //david add
          PS.OT_Recover = FALSE;
          PS.Fan_Recover = FALSE;
      }
      else if ( op_state_changed == TRUE && preOPState == OP_L )
      {
          _SD_Flag.PS_ON_OFF = FALSE;
          PS.Stb_PSON_L = 0;
          PS.Stb_OP_OFF = 0;
          //ClearAllStatus(0xFF);
          PSOnOff_ResetSD_Flag ( );
          op_state_changed = FALSE;
          
          //david add
          PS.OT_Recover = FALSE;
          PS.Fan_Recover = FALSE;
      }
      else
      {
      }
  }
  else
  {
      /*Turn Off PSU*/
      //if ( pson_state_changed == TRUE && prePSONState == PS_ON_L )
      if ( pson_state_changed == TRUE && prePSONState == PS_ON_L )
      {
#if 0
          if ( _SD_Flag.Val == 0 )
          {
              PS.SaveBlackBox = 1;
          }
#endif
          _SD_Flag.PS_ON_OFF = TRUE;
          pson_state_changed = FALSE;
          PS.Stb_PSON_L = 1;
          PS.Stb_OP_OFF = 0;
      }
      else if ( op_state_changed == TRUE && preOPState == OP_H )
      {
#if 0
          if ( _SD_Flag.Val == 0 )
          {
              PS.SaveBlackBox = 1;
          }
#endif
          _SD_Flag.PS_ON_OFF = TRUE;
          op_state_changed = FALSE;
          PS.Stb_PSON_L = 0;
          PS.Stb_OP_OFF = 1;
      }
      else
      {
      }
  }

  //------------------ iPRESENT ---------------------------------------
#if PRESENT_ENABLE
  if ( iPRESENT == HIGH )
  {
      if ( _SD_Flag.PRESENT_OFF == FALSE )
      {
          Protect.PRESENT_H.Flag = 1;
      }

      Protect.PRESENT_L.Flag = 0;
      Protect.PRESENT_L.delay = 0;
  }
  else
  {
      if ( _SD_Flag.PRESENT_OFF == TRUE )
      {
          Protect.PRESENT_L.Flag = 1;
      }

      Protect.PRESENT_H.Flag = 0;
      Protect.PRESENT_H.delay = 0;
  }
#endif
  //------------------ iPS_KILL ---------------------------------------
  if ( iPS_KILL == HIGH )
  {
      if ( _SD_Flag.PS_KILL_OFF == FALSE )
      {
          Protect.PS_KILL_H.Flag = 1;
      }

      Protect.PS_KILL_L.Flag = 0;
      Protect.PS_KILL_L.delay = 0;
  }
  else
  {
      if ( _SD_Flag.PS_KILL_OFF == TRUE )
      {
          Protect.PS_KILL_L.Flag = 1;
      }

      Protect.PS_KILL_H.Flag = 0;
      Protect.PS_KILL_H.delay = 0;
  }

}

//------------------------------------------------------------------------------------------------

static void GoBLMode ( )
{
  #if Config_WATCHDOG_TIMER
  //disable watch dog timer
  _SWDTEN = 0;      //[davidchchen]20160412 added watch-dog timer
  #endif

  //disable all interrupts before jump to bootloader -->
  //disable Timer

  IEC0 = 0;
  IEC1 = 0;
  IEC2 = 0;
  IEC3 = 0;
  IEC4 = 0;
  IEC5 = 0;
  IEC6 = 0;
  IEC7 = 0;
  PTCONbits.PTEN = 0;				//Disable PWM

  //disable all interrupts before jump to bootloader <--

  RCON = 0;	//Reset control register

  asm("goto %0" : : "r"( BOOTLOADER_ADDR ) );
}

//------------------------------------------------------------------------------------------------

static void MAIN_UV_START_Handler ( )
{
  KillTimer ( &hTimerMainRaise );
  PS.MAIN_PROTECT_START = TRUE;
}

static void STB_UV_START_Handler ( )
{
  KillTimer ( &hTimerStbRaise );
  PS.STB_UV_START = TRUE;
}

//------------------------------------------------------------------------------------------------

static void DoStateStandby ( )
{
  StatusToPrimary.bits.isStandbyMode = 1;

  if ( _SD_Flag.ISP_MODE )
  {
      GoBLMode ( );
  }

  if ( _SD_Flag.STB_OCP || _SD_Flag.STB_OVP || _SD_Flag.STB_UVP )
  {
      gPS_State.subState = STATE_SUB_STANDBY_HICCUP;
  }
  else
  {
      gPS_State.subState = STATE_SUB_STANDBY_NORMAL;
  }

  gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.bits._OFF = TRUE;
  gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.bits._OFF = TRUE;
  gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.bits.POWER_GOOD = TRUE;
  gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.bits.POWER_GOOD = TRUE;

  if ( hTimerMainRaise )
  {
      KillTimer ( &hTimerMainRaise );
      PS.MAIN_PROTECT_START = FALSE;
  }


  //Protect.TON_MAX_Fault.Flag = 0;     //[David ch chen]20141126 removed.
  //Protect.TON_MAX_Fault.delay = 0;    //[David ch chen]20141126 removed.

  switch ( gPS_State.subState )
  {
      case STATE_SUB_STANDBY_HICCUP:
          //Disable Standby /
          if ( hTimerStbRaise )
          {
              KillTimer ( &hTimerStbRaise );
              PS.STB_UV_START = FALSE;
          }

          PS.MAIN_PROTECT_START = FALSE;
          PS.STB_UV_START = FALSE;

          DisableSTBoutput ( );
          Protect.STB_RETRY.Flag = 1; //start retry timer 1000 ms
          break;

      case STATE_SUB_STANDBY_NORMAL:
          //wait a standby raising time & turn on standby protection
          //CheckPowerOn();

          //Enable Standby output
#if PFC_ENABLE
          if ( iPFC_OK == PFC_OK )
          {	//[Peter Chung] 20100825
              if ( ! PS.STB_UV_START )
              {
                  //if ( hTimerStbRaise == 0 )
                  if ( hTimerStbRaise == 0 && _SD_Flag.PS_KILL_OFF == FALSE )       //[davidchchen]20160201 changed
                  {
                      hTimerStbRaise = SetTimer ( STB_RAISE_TIME, STB_UV_START_Handler );
                  }
              }

              if ( _SD_Flag.PS_KILL_OFF == FALSE )
              {
                  EnableSTBoutput ( );
              }
              else
              {
                  DisableSTBoutput ( );
              }
          }
#else
          EnableSTBoutput ( );
#endif

          break;


      default:

          break;
  }

  CheckPowerOn ( );
  EnableFan ( );
}

static void DoStateOning ( )
{
  switch ( gPS_State.subState )
  {
      case STATE_SUB_ON:
          switch ( gPSU_ONOFF_Status )
          {
              case PowerOning:
                  //reset variable
                  //gPmbusCmd.MFR_POS_LAST.Val = 0; //Reset POS Last    //[davidchchen]20170418 rRemoved
                  MFR_POS_LAST.Val = 0; //Reset POS Last                //[davidchchen]20170418 Added Black Box block read

                  //wait a main output raising time & turn on main output protection
                  //[Peter Chung] 20100825 added
                  if ( ! PS.MAIN_PROTECT_START )
                  {
                      if ( PSFB.MainEnabled && ( hTimerMainRaise == 0 ) )
                      {
                          hTimerMainRaise = SetTimer ( MAIN_RAISE_TIME, MAIN_UV_START_Handler );
                      }
                  }

                  //[Peter Chung] 20100901 added for TON_MAX_FAULT detect
                  //if ( iPS_OK == P_N_OK )                 //[David ch chen]20141126 removed.
                  //{
                  //    Protect.TON_MAX_Fault.Flag = 1;     //[David ch chen]20141126 removed.
                  //}

                  break;

              case PowerReady:
                  break;

              case PowerGood:
                  gPS_State.mainState = STATE_ONING;
                  gPS_State.subState = STATE_SUB_WAIT_OUTPUT_STABLE;
                  break;

              case PowerOffing:
                  gPS_State.mainState = STATE_OFFING;
                  gPS_State.subState = STATE_SUB_NONE;
                  break;

              case PowerOff:
                  CheckNextState ( );
                  break;
          }
          break;

      case STATE_SUB_WAIT_OUTPUT_STABLE:
          if ( gPSU_ONOFF_Status == PowerGood )
          {
              gPS_State.mainState = STATE_NORMAL;
              gPS_State.subState = STATE_SUB_NORMAL;
          }

          break;

      default:

          break;
  }
  CheckPowerOff ( );
}
//------------------------------------------------------------------------------------------------

static void DoStateNormal ( )
{
  SHORT result;

  StatusToPrimary.bits.isStandbyMode = 0;

  result = ( SHORT ) ADC.Vout_FF + ( SHORT ) gVoutReadOffsetAdc;
  if ( result < 0 )
  {
      result = 0;
  }

//[davidchchen]20170727 Removed STATUS_WROD.Power_GOOD Issue
//  if ( result >= REG_HREF )
//  {
//      gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.bits.POWER_GOOD = FALSE;
//      gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.bits.POWER_GOOD = FALSE;
//  }
//  else if ( result < REG_LREF )
//  {
//      gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.bits.POWER_GOOD = TRUE;
//      gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.bits.POWER_GOOD = TRUE;
//  }

  //Protect.TON_MAX_Fault.Flag = 0;     //[David ch chen]20141126 removed.
  //Protect.TON_MAX_Fault.delay = 0;    //[David ch chen]20141126 removed.

  //Standby Hiccup mode
  if ( _SD_Flag.STB_OCP )
  {
      //Disable Standby /
      if ( hTimerStbRaise )
      {
          KillTimer ( &hTimerStbRaise );
          //PS.STB_UV_START = FALSE;    //[David ch chen]20141124 removed PS.STB_UV_START = FALSE
      }

      PS.STB_UV_START = FALSE;

      DisableSTBoutput ( );
      Protect.STB_RETRY.Flag = 1; //start retry timer 1000 ms
  }
  else
  {
      if ( iPFC_OK == PFC_OK )
      {	//[Peter Chung] 20100825
          if ( ! PS.STB_UV_START )
          {
              //if ( hTimerStbRaise == 0 )
              if ( hTimerStbRaise == 0 && _SD_Flag.PS_KILL_OFF == FALSE )       //[davidchchen]20160201 changed
              {
                  hTimerStbRaise = SetTimer ( STB_RAISE_TIME, STB_UV_START_Handler );
              }
          }

          if ( _SD_Flag.PS_KILL_OFF == FALSE )
          {
              EnableSTBoutput ( );
              
          }
          else
          {
              DisableSTBoutput ( );
          }
      }
  }

  //Check the condition to shutdown 12V main
  CheckPowerOff ( );
}
//------------------------------------------------------------------------------------------------

static void DoStateOffing ( )
{
  switch ( gPSU_ONOFF_Status )
  {
      case PowerOning:
          gPS_State.mainState = STATE_ONING;
          gPS_State.subState = STATE_SUB_ON;
          break;

      case PowerReady:

          break;

      case PowerGood:

          break;

      case PowerOffing:

          break;

      case PowerOff:
          CheckNextState ( );
          break;
  }

  if ( hTimerMainRaise )
  {
      KillTimer ( &hTimerMainRaise );
      PS.MAIN_PROTECT_START = FALSE;
  }

  CheckPowerOn ( );
}
//------------------------------------------------------------------------------------------------

static void DoStateLatch ( )
{
  static BYTE previousState;

  StatusToPrimary.bits.isStandbyMode = 1;

  gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.bits._OFF = TRUE;
  gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.bits._OFF = TRUE;
  gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.bits.POWER_GOOD = TRUE;
  gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.bits.POWER_GOOD = TRUE;

  //Protect.TON_MAX_Fault.Flag = 0;     //[David ch chen]20141126 removed.
  //Protect.TON_MAX_Fault.delay = 0;    //[David ch chen]20141126 removed.

  if ( hTimerMainRaise )
  {
      KillTimer ( &hTimerMainRaise );
      PS.MAIN_PROTECT_START = FALSE;
  }

  switch ( gPS_State.subState )
  {
      case STATE_SUB_PSON_L:
          if ( _SD_Flag.PS_ON_OFF == FALSE )
          {
              previousState = gPS_State.subState;
              gPS_State.mainState = STATE_LATCH;
              gPS_State.subState = STATE_SUB_PSON_H;
          }
          break;

      case STATE_SUB_PSON_H:
          if ( _SD_Flag.PS_ON_OFF == TRUE )
          {
              previousState = gPS_State.subState;
              gPS_State.mainState = STATE_LATCH;
              gPS_State.subState = STATE_SUB_PSON_L;
          }
          else
          {
              if ( previousState == STATE_SUB_PSON_L )
              {
                  previousState = gPS_State.subState;
                  gPS_State.mainState = STATE_STANDBY;
                  gPS_State.subState = STATE_SUB_STANDBY_NORMAL;
              }
          }
          break;

      default:

          break;
  }

  //david add
  //Standby Hiccup mode
  if ( _SD_Flag.STB_OCP )
  {
      //Disable Standby /
      if ( hTimerStbRaise )
      {
          KillTimer ( &hTimerStbRaise );
          PS.STB_UV_START = FALSE;
      }

      PS.STB_UV_START = FALSE;

      DisableSTBoutput ( );
      Protect.STB_RETRY.Flag = 1; //start retry timer 1000 ms
  }
  else
  {
      if ( iPFC_OK == PFC_OK )
      {	//[Peter Chung] 20100825
          if ( ! PS.STB_UV_START )
          {
              //if ( hTimerStbRaise == 0 )
              if ( hTimerStbRaise == 0 && _SD_Flag.PS_KILL_OFF == FALSE )       //[davidchchen]20160201 changed
              {
                  hTimerStbRaise = SetTimer ( STB_RAISE_TIME, STB_UV_START_Handler );
              }
          }

          if ( _SD_Flag.PS_KILL_OFF == FALSE )
          {
              EnableSTBoutput ( );
          }
          else
          {
              DisableSTBoutput ( );
          }
      }
  }

}

//--------------------------- SoftStart Vref.SoftstartVoltage ------------------------

void ControlVref ( )
{
  if ( PSFB.MainEnabled )
  {
      #if RS_ENABLE ////[davidchchen] 20150108 not used
      Vref.SetVoltage = ( WORD ) ( gVoutCmd + gVoutCmdOffset + gVoutCshareOffset + gVoutRSenseOffset );
      #else
      Vref.SetVoltage = ( WORD ) ( gVoutCmd + gVoutCmdOffset + gVoutCshareOffset );
      #endif


      if ( PS.Softstart )
      {
          if ( Vref.SetVoltage > Vref.SoftstartVoltage )
          {
              PDC4 = Vref.SoftstartVoltage;
              Vref.OldSetVoltage = 0;
          }
          else
          {
              PS.Softstart = FALSE;
              PS.SoftstartFinished = TRUE;
              Enable_CurrentShare ( );
          }
      }
      else
      {
          if ( PS.SoftstartFinished )
          {
              if ( Vref.SetVoltage != Vref.OldSetVoltage )
              {
                  PDC4 = Vref.SetVoltage;
                  Vref.OldSetVoltage = Vref.SetVoltage;
              }
          }
      }
  }
}

static void FlashWrite ( )
{
  static BYTE OCW_Count = 0;
  static BYTE UVW_Count = 0;

  if ( PS.SaveBlackBox == TRUE )
  {
      PS.SaveBlackBox = FALSE;
      SaveToBlackBox ( );
  }

  //Update latest Log in RAM
  {
      static tSTATUS_WORD previous_StatusWord[2] = {
          {0x2848 },
          {0x2848 } };
      static WORD previous_SD_FLAG = 0x0000;
      static BYTE previous_MFR_Status = 0x00;
      //BYTE status_changed = FALSE;        //[davidchchen]20160329 removed
      static BYTE status_changed = FALSE;   //[davidchchen]20160329 added

      //Changed in Status_Iout
      if ( ( gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.bits.IOUT == 1 && previous_StatusWord[PAGE0].bits.IOUT == 0 )
           || ( gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.bits.IOUT == 1 && previous_StatusWord[PAGE1].bits.IOUT == 0 ) )
      {
          if ( gPagePlusStatus.PAGE[PAGE0].STATUS_IOUT.bits.IOUT_OC_WARNING == 1 || gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.bits.IOUT_OC_WARNING == 1 )
          {
              if ( OCW_Count < 3 )
              {
                  status_changed = TRUE;
                  OCW_Count ++;
              }
          }
          else
          {
              status_changed = TRUE;
          }
      }

      //Changed in Status_Vout
      if ( ( gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.bits.VOUT == 1 && previous_StatusWord[PAGE0].bits.VOUT == 0 )
           || ( gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.bits.VOUT == 1 && previous_StatusWord[PAGE1].bits.VOUT == 0 ) )
      {
          if ( gPagePlusStatus.PAGE[PAGE0].STATUS_VOUT.bits.VOUT_UV_WARNING == 1 || gPagePlusStatus.PAGE[PAGE1].STATUS_VOUT.bits.VOUT_UV_WARNING == 1 )
          {
              if ( UVW_Count < 3 )
              {
                  status_changed = TRUE;
                  UVW_Count ++;
              }
          }
      }

      //Changed in Status_Temperature
      if ( ( gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.bits.TEMPERATURE == 1 && previous_StatusWord[PAGE0].bits.TEMPERATURE == 0 )
           || ( gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.bits.TEMPERATURE == 1 && previous_StatusWord[PAGE1].bits.TEMPERATURE == 0 ) )
      {
          status_changed = TRUE;
      }

      //Changed in PG
      if ( ( gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.bits.POWER_GOOD == 1 && previous_StatusWord[PAGE0].bits.POWER_GOOD == 0 )
           || ( gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.bits.POWER_GOOD == 1 && previous_StatusWord[PAGE1].bits.POWER_GOOD == 0 ) )
      {
          status_changed = TRUE;
      }

      //Changed in SD_FLAG
      if ( _SD_Flag.Val != 0 && previous_SD_FLAG != _SD_Flag.Val )
      {
          status_changed = TRUE;
      }
      previous_SD_FLAG = _SD_Flag.Val;

      //Changed in MFR_Status
      if ( MFR_Status.Val != 0 && previous_MFR_Status != MFR_Status.Val )
      {
          status_changed = TRUE;
      }
      previous_MFR_Status = MFR_Status.Val;

      if ( status_changed == TRUE )
      {
          status_changed = FALSE;
          SaveToLogContent ( );
      }

      previous_StatusWord[PAGE0].Val = gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.Val;
      previous_StatusWord[PAGE1].Val = gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.Val;
  }

  //Write FRU or Calibration data to Flash if needed
  if ( PS.FlashWritePage1 == TRUE )
  {
      SaveUserDataPage1ToFlash ( );
      PS.FlashWritePage1 = FALSE;
  }

  //Updating blackbox once AC loss
  if ( PS.AC_ON_TO_OFF == TRUE )
  {
      if ( ( PS.FlashWritePage2 == TRUE ) && ( PSFB.MainEnabled == FALSE ) )
      {
          SaveUserDataPage2ToFlash ( );
          PS.FlashWritePage2 = FALSE;
      }

      if ( ( PS.FlashWritePage3 == TRUE ) && ( PSFB.MainEnabled == FALSE ) )
      {
          SaveUserDataPage3ToFlash ( );
          PS.FlashWritePage3 = FALSE;
      }
  }

  //Updating blackbox every 24hr
  if ( PS.Hr24_Expiry == 1 )
  {
      PS.Hr24_Expiry = 0;
      OCW_Count = 0;
      UVW_Count = 0;
      if ( PS.FlashWritePage3 == TRUE )
      {
          SaveUserDataPage3ToFlash ( );
          PS.FlashWritePage3 = FALSE;
      }
  }
}

#if RS_ENABLE   //[davidchchen] 20150108 not used
static void RS_Control ( )
{

  if ( PSFB.MainEnabled )
  {
      if ( PSFB.InRegulation )
      {
          oRS_EN1 = RS_ON;
      }
      else
      {
          oRS_EN1 = RS_OFF;
      }
  }
  else
  {
      oRS_EN1 = RS_OFF;
  }
  
}
#endif

/*****************************************************************************************/
/*****************************************************************************************/

/*****************************************************************************************/
int main ( )
{
  InitPSU ( );

  while ( 1 )
  {
      #if Config_WATCHDOG_TIMER
      ClrWdt(); // Clear WDT Counter, //[davidchchen]20160412 added watch-dog timer
      #endif

      switch ( gPS_State.mainState )
      {
          case STATE_STANDBY:
              DoStateStandby ( );
              break;

          case STATE_ONING:
              DoStateOning ( );
              break;

          case STATE_NORMAL:
              DoStateNormal ( );
              break;

          case STATE_OFFING:
              DoStateOffing ( );
              break;

          case STATE_LATCH:
              DoStateLatch ( );
              break;

          default:

              break;
      }

      //------ Routine Job ----------

      GetI2cAddr ( );

      CheckSwitch ( );

      ScanFault ( );

      UpdateFanDuty ( );

      CheckInputOK ( );

      //ControlVref();

      OutputControl ( );

      Led_PSU_Control ( );

      CheckU1OverRun ( );

      CheckU2OverRun ( );

      CheckICAOV ( );

      CheckI2COV ( );

      SCI_U1_Transmit ( );

      //SCI_U2_Transmit();
      U2_SendData ( );	//Chris, meter IC change to Teridian

      StandbyVoltageControl ( );

      #if RS_ENABLE
      RS_Control ( );
      #endif


      //GetRapidOnStatus();	//for softstart

      RefreshStatus ( );

      FlashWrite ( );

      VoutProcess ( );	//Move here for Inventec issue
  }
  return 0;
}
/*****************************************************************************************/
/*****************************************************************************************/
/*****************************************************************************************/








