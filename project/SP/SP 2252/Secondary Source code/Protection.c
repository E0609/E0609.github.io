/*****************************************************************************************
 *
 *Copyright (C) 2012 Advanced Technology Development
 *                   Power SBG
 *                   LITE-ON TECHNOLOGY Corp.
 *
 *All Rights Reserved
 *
 *File Name : Protection.c
 *
 *Date : 2012.05.24
 *
 *Author :
 *
 *Description :This Program used for HP 2400W Protection Program.
 *
 *******************************************************************************************/
#include "Protection.h"
#include "Process.h"
#include "Isr.h"
#include "Parameter.h"
#include "I2c.h"
#include "Standby.h"
#include "PowerOnOff.h"
#include "Fan.h"
#include "Pmbus.h"
#include "UserData.h"
#include "Led.h"
#include "Psfb.h"
#include "Sci.h"

extern QWORD SysTimerCount;
static QWORD LastTimerCount = 0;
extern tPSU_STATE gPS_State;
extern tPS_FLAG PS;
extern tPSFB PSFB;
extern unsigned int T_Pri;
extern unsigned int T_Sec;
extern unsigned int T_Inlet;
tProtectDelay Protect;
BYTE ConverterChangeInd = 0;
extern void GetRapidOnStatus ( );	//for softstart

//------------------------ CheckDebounce ---------------------------------------------------------

void CheckDebounce ( BYTE* Flag, WORD* delay, WORD timeLimit, WORD timeDifference, _Pfn handler )
{
  if ( *Flag )
  {
      if ( *delay >= timeLimit )
      {
          *delay = 0;
          *Flag = 0;
          handler ( );
      }
      else
      {
          *delay += timeDifference;
      }
  }
  else
  {
      *delay = 0;
  }
}

//------------------------------ OCP_Handler -------------------------------------------------

static void OCP_Handler ( )
{
  //FW V42 Source code
  //if ( ! _SD_Flag.Val )
  
  //david add
  if ( PSFB.MainEnabled == TRUE )
  {
      _SD_Flag.OCP_SD = 1;
      _SD_Flag.LATCH_SD = 1;
      gPagePlusStatus.PAGE[PAGE0].STATUS_IOUT.bits.IOUT_OC_FAULT = 1;
      //gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.bits.IOUT_OC_FAULT = 1;     //[davidchchen]20141225 removed
      //gPagePlusStatus.PAGE[PAGE0].STATUS_IOUT.bits.IOUT_OC_WARNING = 1;
      //gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.bits.IOUT_OC_WARNING = 1;
      CaptureFault ( );
      EmergencyPowerOff ( );
  }
}
//------------------------------ OCW_Handler -------------------------------------------------

static void OCW_Handler ( )
{
  gPagePlusStatus.PAGE[PAGE0].STATUS_IOUT.bits.IOUT_OC_WARNING = 1;
  //gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.bits.IOUT_OC_WARNING = 1;   //[davidchchen]20141225 removed
  gLedWarningStatus.bits.iout_oc_warning = 1;
  MFR_Status.bits.V12_OCW = 1;
}
//------------------------------ CheckOCP -------------------------------------------------

static void CheckOCP ( )
{
  WORD result;

  if ( PSFB.MainEnabled == TRUE )
  {
      if ( PS.IOUT1_CALIBRATED )
      {
          result = ADC.Iout_Cal;
      }
      else
      {
          result = ADC.Iout;
      }

      if ( result > Parameter.OCP_IOUT_OC_FAULT_LIMIT )
      {
          Protect.OCP.Flag = 1;
      }
      else
      {
          Protect.OCP.Flag = 0;
          Protect.OCP.delay = 0;
      }

      if ( result > Parameter.OCP_IOUT_OC_WARN_LIMIT )
      {
          Protect.OCW.Flag = 1;
      }
      else
      {
          Protect.OCW.Flag = 0;
          Protect.OCW.delay = 0;
          gLedWarningStatus.bits.iout_oc_warning = 0;
      }
  }
  else
  {
      Protect.OCP.Flag = 0;
      Protect.OCP.delay = 0;
      Protect.OCW.Flag = 0;
      Protect.OCW.delay = 0;
  }

}

//------------------------------ SW_SLAVE_Handler -------------------------------------------------
//[davidchchen]20150731 Added
#if ConfigDD_SlaveSW
static void SW_SLAVE_Handler ( )
{
    oSD1 = SLAVE_SW_DIS;   //[davidchchen]20150731 Added
}
#endif

//[davidchchen]20160108 added
#if ConfigVout_CompSW
static void VOUTCOMP_SW_Handler ()
{
  oRS_EN1 = 1;
}
#endif


//------------------------------ SCP_Handler -------------------------------------------------
static void SCP_Handler ( )
{
  //FW V42 Source code
  //if ( ! _SD_Flag.Val )

  //david add
  if ( PSFB.MainEnabled == TRUE )
  {
      CaptureFault ( );
      EmergencyPowerOff ( );
      _SD_Flag.SCP_SD = 1;
      _SD_Flag.LATCH_SD = 1;
      //[Peter Chung] 20101209 added
      gPagePlusStatus.PAGE[PAGE0].STATUS_IOUT.bits.IOUT_OC_WARNING = 1;
      //gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.bits.IOUT_OC_WARNING = 1;   //[davidchchen]20141225 removed
      gPagePlusStatus.PAGE[PAGE0].STATUS_IOUT.bits.IOUT_OC_FAULT = 1;
      //gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.bits.IOUT_OC_FAULT = 1;     //[davidchchen]20141225 removed
      PS.SC_CheckStart = 0;
  }
}

#if 0
//------------------------------ OPP_Handler -------------------------------------------------

static void OPP_Handler ( )
{
  if ( ! _SD_Flag.Val )
  {
      CaptureFault ( );
      _SD_Flag.OPP_SD = 1;
      _SD_Flag.LATCH_SD = 1;
      gPagePlusStatus.PAGE[PAGE0].STATUS_IOUT.bits.POUT_OP_FAULT = 1;
      gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.bits.POUT_OP_FAULT = 1;
      gPagePlusStatus.PAGE[PAGE0].STATUS_IOUT.bits.POUT_OP_WARNING = 1;
      gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.bits.POUT_OP_WARNING = 1;
      EmergencyPowerOff ( );
  }
}

//------------------------------ OPW_Handler -------------------------------------------------

static void OPW_Handler ( )
{
  gPagePlusStatus.PAGE[PAGE0].STATUS_IOUT.bits.POUT_OP_WARNING = 1;
  gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.bits.POUT_OP_WARNING = 1;
}
#endif

//------------------------------ CheckPinOP -------------------------------------------------
#if 0

static void CheckPinOP ( )
{
  if ( gPin >= Parameter.PIN_OP_WARN_LIMIT )
  {
      Protect.PinOPW.Flag = 1;
  }
  else
  {
      Protect.PinOPW.Flag = 0;
      Protect.PinOPW.delay = 0;
  }
}

//------------------------------ PIN_OPW_Handler --------------------------------------------

static void PIN_OPW_Handler ( )
{
  gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.bits.PIN_OP_WARNING = 1;
  gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.bits.PIN_OP_WARNING = 1;
}
#endif
//------------------------------ CheckIinOC -------------------------------------------------
#if 0

static void CheckIinOC ( )
{
  if ( gPmbusCmd.MFR_LINE_STATUS[0].bits.IsHighLine )
  {
      if ( gIin >= Parameter.IIN_OC_WARN_HL_LIMIT )
      {
          Protect.IinOCW.Flag = 1;
      }
      else
      {
          Protect.IinOCW.Flag = 0;
          Protect.IinOCW.delay = 0;
      }
  }
  else
  {
      if ( gIin >= Parameter.IIN_OC_WARN_LL_LIMIT )
      {
          Protect.IinOCW.Flag = 1;
      }
      else
      {
          Protect.IinOCW.Flag = 0;
          Protect.IinOCW.delay = 0;
      }
  }
}

//------------------------------ IIN_OCW_Handler --------------------------------------------

static void IIN_OCW_Handler ( )
{
  gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.bits.IIN_OC_WARNING = 1;
  gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.bits.IIN_OC_WARNING = 1;
}
#endif
#if 0
//------------------------------ CheckOPP -------------------------------------------------

static void CheckOPP ( )
{
  WORD result;

  if ( PSFB.MainEnabled == TRUE )
  {
      if ( PS.IOUT1_CALIBRATED )
      {
          result = ADC.Iout_Cal;
      }
      else
      {
          result = ADC.Iout;
      }

      if ( gPout > OPP_H_REF )
      {
          Protect.OPP.Flag = 1;
      }
      else
      {
          Protect.OPP.Flag = 0;
          Protect.OPP.delay = 0;
      }

      if ( gPout > Parameter.OPP_POUT_OP_WARN_LIMIT )
      {
          Protect.OPW.Flag = 1;
      }
      else
      {
          Protect.OPW.Flag = 0;
          Protect.OPW.delay = 0;
      }
  }
  else
  {
      Protect.OPP.Flag = 0;
      Protect.OPP.delay = 0;
      Protect.OPW.Flag = 0;
      Protect.OPW.delay = 0;
  }
}
#endif
//------------------------------ UVP_Handler -------------------------------------------------

static void UVP_Handler ( )
{
  //FW V42 Source code
  //if ( ! _SD_Flag.Val )

  //david add
  if ( PSFB.MainEnabled == TRUE )
  {
      _SD_Flag.UV_SD = 1;
      _SD_Flag.LATCH_SD = 1;
      gPagePlusStatus.PAGE[PAGE0].STATUS_VOUT.bits.VOUT_UV_WARNING = 1;
      gPagePlusStatus.PAGE[PAGE0].STATUS_VOUT.bits.VOUT_UV_FAULT = 1;
      CaptureFault ( );
      EmergencyPowerOff ( );
  }
}

//------------------------------ CheckUVP -------------------------------------------------

static void CheckUVP ( )
{
  SHORT result;

  if ( PSFB.MainEnabled )
  {
      result = ( SHORT ) ADC.Vout_FF + ( SHORT ) gVoutReadOffsetAdc;
      if ( result < 0 )
      {
          result = 0;
      }

      if ( PS.MAIN_PROTECT_START )
      {
          if ( result < Parameter.UVP_VOUT_UV_WARN_LIMIT )
          {
              gPagePlusStatus.PAGE[PAGE0].STATUS_VOUT.bits.VOUT_UV_WARNING = 1;
              gLedWarningStatus.bits.vout_uv_warning = 1;
              MFR_Status.bits.V12_UVW = 1;
          }
          else
          {
              gLedWarningStatus.bits.vout_uv_warning = 0;
          }
      }

      if ( result < Parameter.UVP_VOUT_UV_FAULT_LIMIT )
      {
          Protect.UVP.Flag = 1;
      }
      else
      {
          Protect.UVP.Flag = 0;
          Protect.UVP.delay = 0;
      }
  }
  else
  {
      Protect.UVP.Flag = 0;
      Protect.UVP.delay = 0;
  }
}
//------------------------------ OTP_Handler -------------------------------------------------

static void OTP_HotSpot_Handler ( )
{
#if OT_PROTECT
  //FW V42 Source code
  //if ( ! _SD_Flag.Val )

  //david add
  if ( PSFB.MainEnabled == TRUE )
  {
      _SD_Flag.OTP_SD = 1;
      gPagePlusStatus.PAGE[PAGE0].STATUS_TEMPERATURE.bits.OT_FAULT = 1;
      CaptureFault ( );
      EmergencyPowerOff ( );
  }
#endif
}

static void OTP_Ambient_Handler ( )
{
#if OT_PROTECT
  //FW V42 Source code
  //if ( ! _SD_Flag.Val )

  //david add
  if ( PSFB.MainEnabled == TRUE )
  {
      _SD_Flag.OTP_SD = 1;
      gPagePlusStatus.PAGE[PAGE1].STATUS_TEMPERATURE.bits.OT_FAULT = 1;
      CaptureFault ( );
      EmergencyPowerOff ( );
  }
#endif
}

//------------------------------ OTW_Handler -------------------------------------------------

static void OTW_HotSpot_Handler ( )
{
  gPagePlusStatus.PAGE[PAGE0].STATUS_TEMPERATURE.bits.OT_WARNING = 1;
  gLedWarningStatus.bits.ot2_warning = 1;
  gLedWarningStatus.bits.ot3_warning = 1;
}

static void OTW_Ambient_Handler ( )
{
  gPagePlusStatus.PAGE[PAGE1].STATUS_TEMPERATURE.bits.OT_WARNING = 1;
  gLedWarningStatus.bits.ot1_warning = 1;
}

//--------------------------- OT_Recover_Handler ----------------------------------------------

static void OT_Recover_Handler ( )
{
  //if(T_Pri < DEFAULT_OT_RECOVERY && T_Sec < DEFAULT_OT_RECOVERY && T_Inlet < DEFAULT_OT_RECOVERY)
  if ( _SD_Flag.OTP_SD )
  {
      if ( T_Pri < Parameter.OTP_Tpri_Recovery_LIMIT && T_Sec < Parameter.OTP_Tsec_Recovery_LIMIT && T_Inlet < Parameter.OTP_Tinlet_Recovery_LIMIT )	//Chris, for thermal request
      {
          _SD_Flag.OTP_SD = 0;
          PS.OT_Recover = TRUE;
      }
  }
}
//--------------------------- PSON_H_Handler ----------------------------------------------

static void PSON_H_Handler ( )
{
#if PSON_ENABLE
  PS.PS_ON_OFF = PS_ON_H;
#endif
}
//--------------------------- PSON_L_Handler ----------------------------------------------

static void PSON_L_Handler ( )
{
#if PSON_ENABLE
  PS.PS_ON_OFF = PS_ON_L;
  //Protect.RMC_Removed.Flag = 1; //[davidchchen]20141226 Added, //[davidchchen]20170216 Removed
#endif
}
//--------------------------- PRESENT_H_Handler ----------------------------------------------
#if PRESENT_ENABLE  //[davidchchen] 20150107 not used PRESENT
static void PRESENT_H_Handler ( )
{
#if PRESENT_ENABLE
  _SD_Flag.PRESENT_OFF = TRUE;
#endif
}
//--------------------------- PRESENT_L_Handler ----------------------------------------------
static void PRESENT_L_Handler ( )
{
#if PRESENT_ENABLE
  _SD_Flag.PRESENT_OFF = FALSE;
#endif
}
#endif

//--------------------------- PRESENT_H_Handler ----------------------------------------------

static void PS_KILL_H_Handler ( )
{
#if PS_KILL_ENABLE
    _SD_Flag.PS_KILL_OFF = TRUE;
    DisableSTBoutput ( );
#endif
}
//--------------------------- PRESENT_L_Handler ----------------------------------------------

static void PS_KILL_L_Handler ( )
{
#if PS_KILL_ENABLE
  _SD_Flag.PS_KILL_OFF = FALSE;
#endif
}

//--------------------------- FanLock_Handler ------------------------------------------------

static void Fan1Lock_Handler ( )
{
#if FAN_LOCK_PROTECT
#if 0
  if ( ! _SD_Flag.Val )
  {
      _SD_Flag.FAN_LOCK = 1;
      _SD_Flag.LATCH_SD = 1;

      gPmbusCmd.STATUS_FAN_1_2.bits.FAN1_FAULT = 1;

      CaptureFault ( );
      EmergencyPowerOff ( );
  }
#else
  //FW V42 Source code
  //if ( ! _SD_Flag.Val )

  //david add
  if ( _SD_Flag.FAN_LOCK == 0 )
  {
      _SD_Flag.FAN_LOCK = 1;
      gPmbusCmd.STATUS_FAN_1_2.bits.FAN1_FAULT = 1;
      CaptureFault ( );
      EmergencyPowerOff ( );
  }
#endif
#endif
}

static void Fan1Fault_Handler ( )
{
  gPmbusCmd.STATUS_FAN_1_2.bits.FAN1_FAULT = 1;	//[Peter Chung] 20101220 added
}

#if 0

static void Fan1Warning_Handler ( )
{
#if 1
  gPmbusCmd.STATUS_FAN_1_2.bits.FAN1_WARNING = 1;	//[Peter Chung] 20101220 added
#endif
}
#endif

static void Fan1DisappearHandler ( )
{
  PS.Fan1Disappear = 1;
}

static void Fan2Lock_Handler ( )
{
#if FAN_LOCK_PROTECT
#if 0
  if ( ! _SD_Flag.Val )
  {
      _SD_Flag.FAN_LOCK = 1;
      _SD_Flag.LATCH_SD = 1;

      gPmbusCmd.STATUS_FAN_1_2.bits.FAN2_FAULT = 1;

      CaptureFault ( );
      EmergencyPowerOff ( );
  }
#else
  //FW V42 Source code
  //if ( ! _SD_Flag.Val )

  //david add
  if ( _SD_Flag.FAN_LOCK == 0 )
  {
      _SD_Flag.FAN_LOCK = 1;
      gPmbusCmd.STATUS_FAN_1_2.bits.FAN2_FAULT = 1;
      CaptureFault ( );
      EmergencyPowerOff ( );
  }

#endif
#endif
}

static void Fan2Fault_Handler ( )
{
  gPmbusCmd.STATUS_FAN_1_2.bits.FAN2_FAULT = 1;	//[Peter Chung] 20101220 added
}

#if 0

static void Fan2Warning_Handler ( )
{
#if 1
  gPmbusCmd.STATUS_FAN_1_2.bits.FAN2_WARNING = 1;	//[Peter Chung] 20101220 added
#endif
}
#endif

void FanRecover_Handler ( )
{
  if ( _SD_Flag.FAN_LOCK )
  {
      if ( gFan1.CurrentRpm > 500 && gFan1.Detected == TRUE && gFan2.CurrentRpm > 500 )
      {
          _SD_Flag.FAN_LOCK = 0;
          PS.Fan_Recover = TRUE;
      }

  }
  else
  {
      Protect.FanRecovery.delay = 0;
  }
}

static void FanDutyCtrl_Handler ( )
{
  PS.FanDutyCtrl = 1;
}

static void T_FanControlHandler ( )
{
  PS.FanControl = 1;	//Fan control timer
#if 1
  //Check FanOverride
  if ( gFan1.OverridedDuty > GetFanMinDuty ( ) )
  {
      gPmbusCmd.STATUS_FAN_1_2.bits.FAN1_OVERRIDE = 1;
      gPmbusCmd.STATUS_FAN_1_2.bits.FAN2_OVERRIDE = 1;
  }
#endif
}
//------------------------ SoftStart_Delay_Handler ----------------------------------------------

static void SoftStart_Delay_Handler ( )
{
  PS.Softstart = TRUE;
  PS.SoftstartFinished = FALSE;
}

//------------------------ STB_OCP_Handler ----------------------------------------------------

void STB_OCP_Handler ( )
{
#if STB_OC_PROTECT
  if ( _SD_Flag.STB_OCP == FALSE )
  {
      _SD_Flag.STB_OCP = TRUE;
      //gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.bits.IOUT_OC_FAULT = 1;   //[davidchchen]20160826 removed to Modify STB STATUS_WORD can't be clear by clear fault
      gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.bits.IOUT_OC_FAULT = 1;       //[davidchchen]20170619 Added

      CaptureFault ( );
      DisableSTBoutput ( );
      //EmergencyPowerOff();
      PS.SaveBlackBox = 1;
  }
#endif
}

//[davidchchen]20170619 added
void STB_OCW_Handler ( )
{
    gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.bits.IOUT_OC_WARNING = 1;   //[davidchchen]20170619 added
}

//------------------------ STB_UVP_Handler ----------------------------------------------------

void STB_UVP_Handler ( )
{
#if STB_UV_PROTECT
  gPagePlusStatus.PAGE[PAGE1].STATUS_VOUT.bits.VOUT_UV_FAULT = 1;
  if ( _SD_Flag.STB_UVP == FALSE && _SD_Flag.STB_OCP == FALSE )
  {
      _SD_Flag.STB_UVP = TRUE;
      CaptureFault ( );
      DisableSTBoutput ( );
      EmergencyPowerOff ( );
      PS.SaveBlackBox = 1;

  }
#endif
}

//[davidchchen]20160122 added
void STB_UVW_Handler ( )
{

    gPagePlusStatus.PAGE[PAGE1].STATUS_VOUT.bits.VOUT_UV_WARNING = 1;   //[davidchchen]20160201 added
    MFR_Status.bits.VSB_UVW = 1;                                        //[davidchchen]20160201 added

}

//------------------------ STB_RETRY_Handler ---------------------------------------------------

static void STB_RETRY_Handler ( )
{
  _SD_Flag.STB_OCP = FALSE;
  _SD_Flag.STB_OVP = FALSE;
  _SD_Flag.STB_UVP = FALSE;
}

//------------------------ CheckI2C-------------------------------------------------------------

static void CheckI2C ( )
{
#if 1
  //if ( I2C1CONbits.SCLREL == 0 )  //[davidchchen]20160223_I2C removed
  //if ( ( I2C1STATbits.S == 1 && I2C1CONbits.SCLREL == 0 ) || ( I2C1STATbits.S == 1 && I2C1STATbits.TBF == 1 ) )    //[davidchchen]20160223_I2C
  if ( I2C1STATbits.S == 1 )    //[davidchchen]20160321 changed
  {
      Protect.I2C_SCL_Fault.Flag = 1;
  }
  else
  {
      Protect.I2C_SCL_Fault.Flag = 0;
      Protect.I2C_SCL_Fault.delay = 0;
  }
#endif
#if 0
  if ( iSDA == LOW )
  {
      Protect.I2C_SDA_Fault.Flag = 1;
  }
  else
  {
      Protect.I2C_SDA_Fault.Flag = 0;
      Protect.I2C_SDA_Fault.delay = 0;
  }
#endif
}

//------------------------ TON_MAX_FAULT_Handler-----------------------------------------------
#if 0

static void TON_MAX_FAULT_Handler ( )
{
  gPagePlusStatus.PAGE[PAGE0].STATUS_VOUT.bits.TON_MAX_FAULT = 1;
  gPagePlusStatus.PAGE[PAGE1].STATUS_VOUT.bits.TON_MAX_FAULT = 1;
}
#endif
//------------------------ FW Current Share------------------------------------------------------

//[davidchchen] 20150108 removed
//static void CSStartupHandler ( )
//{
//  PS.CS_START = 1;
//}

//[davidchchen] 20150108 removed
//static void CSDelayHandler ( )
//{
//  PS.CS_CATCH = 1;
//}

//------------------------ Uart_WDT_Timeout ----------------------------------------------------

static void Uart1_WDT_Timeout ( )
{
  gLedStatus.bits.comm_err = 1;	//Chris
  MFR_Status.bits.SCI_P2S_FAIL = 1;
  PS.Uart1Error = 1;            //[davidchchen]20160503 added primary and secondary side communication error
}

static void Uart1_PriRxWDT_Timeout ( )      //[davidchchen]20160503 added primary and secondary side communication error
{
  gLedStatus.bits.PriRXcomm_err = 1;             //[davidchchen]20160503 added primary and secondary side communication error
  MFR_Status.bits.SCI_PriRx_FAIL = 1;         //[davidchchen]20160503 added primary and secondary side communication error
}

static void Uart2_WDT_Timeout ( )
{
  gLedStatus.bits.U2comm_err = 1;	//Chris
  MFR_Status.bits.SCI_T2S_FAIL = 1;
  //PS.U2_Comm_Timeout = 1;             //[davidchchen]20160503 removed
  PS.Uart2Error = 1;                    //[davidchchen]20160503 added primary and secondary side communication error
}

static void U2TxDelayHandler ( )
{
  PS.U2TxSend = TRUE;
}

//------------------------ CumulatePOS ----------------------------------------------------------

static void CumulatePOS ( WORD timeDifference )
{
  static WORD timer_1s = 0;
  static WORD BMCtimer_1s = 0;  //[davidchchen]20170310 Added BMC_UNIX_TIMESTAMP cmd
  static DWORD timer_24hr = 0;

  //start couting after delivering output
  if ( ( gPS_State.mainState == STATE_NORMAL ) && ( PSFB.MainEnabled == TRUE ) )
  {
      timer_1s += timeDifference;
      if ( timer_1s >= 1000 )
      {	//count 1 sec

//          if ( gPmbusCmd.MFR_POS_LAST.Val < 0xFFFFFFFF )      //[davidchchen]20170418 rRemoved
//          {
//              gPmbusCmd.MFR_POS_LAST.Val ++;                  //[davidchchen]20170418 rRemoved
//          }
//          else
//          {
//              gPmbusCmd.MFR_POS_LAST.Val = 0;                 //[davidchchen]20170418 rRemoved
//          }

          if ( MFR_POS_LAST.Val < 0xFFFFFFFF )        //[davidchchen]20170418 Added Black Box block read
          {
              MFR_POS_LAST.Val ++;                    //[davidchchen]20170418 Added Black Box block read
          }
          else
          {
              MFR_POS_LAST.Val = 0;                   //[davidchchen]20170418 Added Black Box block read
          }
          strncpy ( ( char * ) &gPmbusCmd.MFR_POS_LAST[1], ( char * ) &MFR_POS_LAST.v[0], MFR_POS_LAST_LEN ); //[davidchchen]20170418 Added Black Box block read

//          if ( gPmbusCmd.MFR_POS_TOTAL.Val < 0xFFFFFFFF )         //[davidchchen]20170418 Removed
//          {
//              gPmbusCmd.MFR_POS_TOTAL.Val ++;                     //[davidchchen]20170418 Removed
//          }
//          else
//          {
//              gPmbusCmd.MFR_POS_TOTAL.Val = 0;                    //[davidchchen]20170418 Removed
//          }

          if ( MFR_POS_TOTAL.Val < 0xFFFFFFFF )     //[davidchchen]20170418 Added Black Box block read
          {
              MFR_POS_TOTAL.Val ++;                 //[davidchchen]20170418 Added Black Box block read
          }
          else
          {
              MFR_POS_TOTAL.Val = 0;                //[davidchchen]20170418 Added Black Box block read
          }
          strncpy ( ( char * ) &gPmbusCmd.MFR_POS_TOTAL[1], ( char * ) &MFR_POS_TOTAL.v[0], MFR_POS_TOTAL_LEN ); //[davidchchen]20170418 Added Black Box block read


          //Calculate W*H
          UserData.Page3.region.logGeneral.Total_Output_Power.Val += gPout;

          //UserData.Page2.region.POS.Val = gPmbusCmd.MFR_POS_TOTAL.Val;        //[davidchchen]20170418 Removed
          UserData.Page2.region.POS.Val = MFR_POS_TOTAL.Val;                    //[davidchchen]20170418 Added Black Box block read
          PS.FlashWritePage2 = TRUE;
          PS.FlashWritePage3 = TRUE;

          timer_1s = 0;
      }
  }

  BMCtimer_1s += timeDifference;    //[davidchchen]20170310 Added BMC_UNIX_TIMESTAMP cmd
  if ( BMCtimer_1s >= 1000 )        //[davidchchen]20170310 Added BMC_UNIX_TIMESTAMP cmd
  {
//        //count 1 sec
//        if ( gPmbusCmd.BMC_UNIX_TIMESTAMP.Val < 0xFFFFFFFF )    //[davidchchen]20170418 Removed
//        {
//            gPmbusCmd.BMC_UNIX_TIMESTAMP.Val ++;                //[davidchchen]20170418 Removed
//        }
//        else
//        {
//            gPmbusCmd.BMC_UNIX_TIMESTAMP.Val = 0;               //[davidchchen]20170418 Removed
//        }

        //count 1 sec
        if ( BMC_UNIX_TIMESTAMP.Val < 0xFFFFFFFF )    //[davidchchen]20170418 Added Black Box block read
        {
            BMC_UNIX_TIMESTAMP.Val ++;                //[davidchchen]20170418 Added Black Box block read
        }
        else
        {
            BMC_UNIX_TIMESTAMP.Val = 0;               //[davidchchen]20170418 Added Black Box block read
        }

        strncpy ( ( char * ) &gPmbusCmd.BMC_UNIX_TIMESTAMP[1], ( char * ) &BMC_UNIX_TIMESTAMP.v[0], BMC_UNIX_TIMESTAMP_LEN ); //[davidchchen]20170418 Added Black Box block read
        BMCtimer_1s = 0;
  }

  

  timer_24hr += timeDifference;

  if ( timer_24hr >= 86400000 )
  {
      PS.Hr24_Expiry = 1;
      timer_24hr = 0;
  }

  Protect.u1WDT.Flag = 1; //Chris, U1Rx interrupt clear flag
  Protect.u2WDT.Flag = 1;

  
}

//------------------------ PriIS_Handler ----------------------------------------------------------

void PriIS_Handler ( )
{
  EmergencyPowerOff ( );
  _SD_Flag.LATCH_SD = 1;
}

//------------------------ DcRapidOnLow_Handler--------------------------------------------------

void DcRapidOnLow_Handler ( )
{
  PS.DC_RAPID_ON_LOW = 1;
}

//------------------------ RS_Det_Handler -------------------------------------------------------

void RS_Det_Handler ( )
{

//#if 1
  #if RS_ENABLE     //[davidchchen] 20150108 not used
  SHORT result;

  if ( UserData.Page2.region.Calibration_Flag.bits.b0 )
  {
      result = ( ( SHORT ) ADC.VoutBus_FF + ( SHORT ) gMainBusVoutOffset + ( SHORT ) gVoutReadOffsetAdc );
      if ( result < 0 )
      {
          result = 0;
      }
      ADC.VoutBus_Cal = result;

      if ( ADC.VoutBus_FF < 50 )
      {
          //Remote Sense dissappear
          gVoutRSenseOffset = 0;
          return;
      }

      if ( gIsFactoryMode == FALSE )
      {
          if ( PSFB.MainEnabled && PSFB.InRegulation )
          {
              if ( ADC.VoutBus_Cal < Parameter.VOUT_RSENSE_VREFL )  //[davidchchen]20151225 Note=> ADC.VoutBus_Cal<12.50v
              {
                  gVoutRSenseOffset += 2;
                  if ( gVoutRSenseOffset >= 400 )
                  {
                      gVoutRSenseOffset = 400;
                  }
              }
              else if ( ADC.VoutBus_Cal > Parameter.VOUT_RSENSE_VREFH ) //[davidchchen]20151225 Note=> ADC.VoutBus_Cal<12.55v
              {
                  if ( gVoutRSenseOffset >= 2 )
                  {
                      gVoutRSenseOffset -= 2;
                  }
              }
          }
          else
          {
              gVoutRSenseOffset = 0;
          }
      }
      else
      {
          gVoutRSenseOffset = 0;
      }
  }
  else
  {
      gVoutRSenseOffset = 0;
  }

#endif
}

//-----------------------------------DummyLoadOFF_Handler-------------------------------------

void DummyLoadOFF_Handler ( )
{
  PS.DummyLoadOff = 1;
}

//----------------------------------- SciRetryHandler -------------------------------------------

static void SciRetryHandler ( )
{
  PS.SCI_Retry = 1;
}

//--------------------------------- RmcRemoved_Handler ---------------------------------------

//void RmcRemoved_Handler ( )
//{
//  if ( gIsFactoryMode == FALSE )
//  {
//      if ( _SD_Flag.LATCH_SD == 0 )
//      {
//          PS.op_cmdChanged = TRUE;
//          gPmbusCmd.OPERATION[0] = 0x80;
//      }
//  }
//}

//[davidchchen]20170216 added PSON Signal Enable/disable
//--------------------------------- HotPlugAddr_Handler ---------------------------------------

void HotPlugAddr_Handler ( )
{
    OneTimeFlag = 1;    //[davidchchen]20170216 added PSON Signal Enable/disable
}

//---------------------------------  VinUVFaultHandler   -----------------------------------------

void VinUVFaultHandler ( )
{
  PS.VinUVFaultDetected = 1;
}

//--------------------------------  VinUVWarningHandler  ----------------------------------------

void VinUVWarningHandler ( )
{
  PS.VinUVWarnDetected = 1;
}

//-------------------------------------- ScanFault ----------------------------------------------

void ScanFault ( )
{
  WORD timeDifference;

  timeDifference = ( WORD ) ( SysTimerCount - LastTimerCount );
  LastTimerCount = SysTimerCount;

  //**************************************************************************************************/
  //*************************	Main Output Protection *****************************************************/
  //**************************************************************************************************/
  //*************************	OPP	********************************************************************/
#if 0
  CheckOPP ( );
  CheckDebounce ( &Protect.OPP.Flag, &Protect.OPP.delay, OPP_DEBOUNCE_TIME, timeDifference, OPP_Handler );
  CheckDebounce ( &Protect.OPW.Flag, &Protect.OPW.delay, OPW_DEBOUNCE_TIME, timeDifference, OPW_Handler );
#endif
  //*************************	OCP	********************************************************************/
#if OC_PROTECT
  CheckOCP ( );
  CheckDebounce ( &Protect.OCP.Flag, &Protect.OCP.delay, OCP_DEBOUNCE_TIME, timeDifference, OCP_Handler );
  CheckDebounce ( &Protect.OCW.Flag, &Protect.OCW.delay, OCW_DEBOUNCE_TIME, timeDifference, OCW_Handler );
#endif
  //*************************	UVP	********************************************************************/
  CheckUVP ( );
#if UV_PROTECT
  CheckDebounce ( &Protect.UVP.Flag, &Protect.UVP.delay, UVP_DEBOUNCE_TIME, timeDifference, UVP_Handler );
#endif
  //*************************	SCP	********************************************************************/
#if SC_PROTECT
  CheckDebounce ( &Protect.SCP.Flag, &Protect.SCP.delay, SCP_DEBOUNCE_TIME, timeDifference, SCP_Handler );
#endif
  
  //*************************	Disable DC/DC Slave Switch UCC28950 	********************************************************************/
  //[davidchchen]20150731 Added
  #if ConfigDD_SlaveSW
  CheckDebounce ( &Protect.DD_SlaveSW_OFF.Flag, &Protect.DD_SlaveSW_OFF.delay, SW_SLAVE_DEBOUNCE_TIME, timeDifference, SW_SLAVE_Handler );
  #endif

  //[davidchchen]20160108 added
  #if ConfigVout_CompSW
  CheckDebounce ( &Protect.MainVoutComp.Flag, &Protect.MainVoutComp.delay, VOUT_COMP_DEBOUNCE_TIME, timeDifference, VOUTCOMP_SW_Handler );
  #endif

  //*************************	OTP	********************************************************************/
  CheckDebounce ( &Protect.OTP_Tpri.Flag, &Protect.OTP_Tpri.delay, OTP_TIMER, timeDifference, OTP_HotSpot_Handler );
  CheckDebounce ( &Protect.OTP_Tsec.Flag, &Protect.OTP_Tsec.delay, OTP_TIMER, timeDifference, OTP_HotSpot_Handler );
  CheckDebounce ( &Protect.OTP_Tinlet.Flag, &Protect.OTP_Tinlet.delay, OTP_TIMER, timeDifference, OTP_Ambient_Handler );

  CheckDebounce ( &Protect.OTW_Tpri.Flag, &Protect.OTW_Tpri.delay, OTW_TIMER, timeDifference, OTW_HotSpot_Handler );
  CheckDebounce ( &Protect.OTW_Tsec.Flag, &Protect.OTW_Tsec.delay, OTW_TIMER, timeDifference, OTW_HotSpot_Handler );
  CheckDebounce ( &Protect.OTW_Tinlet.Flag, &Protect.OTW_Tinlet.delay, OTW_TIMER, timeDifference, OTW_Ambient_Handler );

  CheckDebounce ( &Protect.OT_Recover_Tpri.Flag, &Protect.OT_Recover_Tpri.delay, OT_RECOVER_TIMER, timeDifference, OT_Recover_Handler );
  CheckDebounce ( &Protect.OT_Recover_Tsec.Flag, &Protect.OT_Recover_Tsec.delay, OT_RECOVER_TIMER, timeDifference, OT_Recover_Handler );
  CheckDebounce ( &Protect.OT_Recover_Tinlet.Flag, &Protect.OT_Recover_Tinlet.delay, OT_RECOVER_TIMER, timeDifference, OT_Recover_Handler );

  //*************************	PIN	********************************************************************/
#if 0
  CheckPinOP ( );	//Peter
  CheckDebounce ( &Protect.PinOPW.Flag, &Protect.PinOPW.delay, PIN_OP_WARN_DELAY, timeDifference, PIN_OPW_Handler );	//Peter
#endif
  //*************************	IIN	********************************************************************/
#if 0
  CheckIinOC ( );	//Peter
  CheckDebounce ( &Protect.IinOCW.Flag, &Protect.IinOCW.delay, IIN_OC_WARN_DELAY, timeDifference, IIN_OCW_Handler );	//Peter
#endif

  //**************************************************************************************************/
  //*************************	Standby Protection	********************************************************/
  //**************************************************************************************************/
  CheckDebounce ( &Protect.STB_RETRY.Flag, &Protect.STB_RETRY.delay, STB_RETRY_TIMER, timeDifference, STB_RETRY_Handler );
  //*************************	STB OCP	****************************************************************/
#if STB_OC_PROTECT
  Check_STBOCP ( );
  CheckDebounce ( &Protect.STB_OCP.Flag, &Protect.STB_OCP.delay, STB_OCP_DELAY_TIME, timeDifference, STB_OCP_Handler );
  CheckDebounce ( &Protect.STB_OCW.Flag, &Protect.STB_OCW.delay, STB_OCW_DELAY_TIME, timeDifference, STB_OCW_Handler ); //[davidchchen]20170619 added
#endif
  //*************************	STB UVP	****************************************************************/
#if STB_UV_PROTECT
  Check_STBUVP ( );
  CheckDebounce ( &Protect.STB_UVP.Flag, &Protect.STB_UVP.delay, STB_UVP_DELAY_TIME, timeDifference, STB_UVP_Handler );
  CheckDebounce ( &Protect.STB_UVW.Flag, &Protect.STB_UVW.delay, STB_UVW_DELAY_TIME, timeDifference, STB_UVW_Handler ); //[davidchchen]20160201 added
#endif
  //*************************	STB OTP	****************************************************************/




  //**************************************************************************************************/
  //*************************	Others	****************************************************************/
  //**************************************************************************************************/

  //*************************	FAN	********************************************************************/
  CheckDebounce ( &Protect.Fan1Fault.Flag, &Protect.Fan1Fault.delay, FAN_FAULT_TIMER, timeDifference, Fan1Fault_Handler );
#if 0
  CheckDebounce ( &Protect.Fan1Warning.Flag, &Protect.Fan1Warning.delay, FAN_WARN_TIMER, timeDifference, Fan1Warning_Handler );
#endif
  CheckDebounce ( &Protect.Fan1Lock.Flag, &Protect.Fan1Lock.delay, FAN_LOCK_TIMER, timeDifference, Fan1Lock_Handler );
  CheckDebounce ( &Protect.Fan1Disappear.Flag, &Protect.Fan1Disappear.delay, FAN_DISAPPEAR_TIMER, timeDifference, Fan1DisappearHandler );

  CheckDebounce ( &Protect.Fan2Fault.Flag, &Protect.Fan2Fault.delay, FAN_FAULT_TIMER, timeDifference, Fan2Fault_Handler );
#if 0
  CheckDebounce ( &Protect.Fan2Warning.Flag, &Protect.Fan2Warning.delay, FAN_WARN_TIMER, timeDifference, Fan2Warning_Handler );
#endif
  CheckDebounce ( &Protect.Fan2Lock.Flag, &Protect.Fan2Lock.delay, FAN_LOCK_TIMER, timeDifference, Fan2Lock_Handler );
  CheckDebounce ( &Protect.FanDutyCtrl.Flag, &Protect.FanDutyCtrl.delay, FAN_DUTY_CNTL_TIMER, timeDifference, FanDutyCtrl_Handler );
  CheckDebounce ( &Protect.T_FanControl.Flag, &Protect.T_FanControl.delay, FAN_CONTROL_TIMER, timeDifference, T_FanControlHandler );
  Protect.FanRecovery.Flag = 1;
  CheckDebounce ( &Protect.FanRecovery.Flag, &Protect.FanRecovery.delay, FAN_RECOVER_TIMER, timeDifference, FanRecover_Handler );
  
  //*************************	I2C	********************************************************************/
  CheckI2C ( ); //[davidchchen]20160223_I2C
  CheckDebounce ( &Protect.I2C_SCL_Fault.Flag, &Protect.I2C_SCL_Fault.delay, I2C_RESET_TIMER, timeDifference, init_I2C ); //[davidchchen]20160223_I2C
  //CheckDebounce ( &Protect.I2C_SDA_Fault.Flag, &Protect.I2C_SDA_Fault.delay, I2C_RESET_TIMER, timeDifference, init_I2C );   //[davidchchen] 20150107 removed

  //Protect.RMC_Removed.Flag = 1;	//Start timer, [davidchchen]20141226 removed
  //CheckDebounce ( &Protect.RMC_Removed.Flag, &Protect.RMC_Removed.delay, RMC_REMOVED_TIME, timeDifference, RmcRemoved_Handler );  //[davidchchen]20170216 Removed
  CheckDebounce ( &Protect.HotPlugAddr.Flag, &Protect.HotPlugAddr.delay, HOTPLUG_ADDR_TIME, timeDifference, HotPlugAddr_Handler );    //[davidchchen]20170216 added PSON Signal Enable/disable
  //*************************	Uart ********************************************************************/
  CheckDebounce ( &Protect.u1WDT.Flag, &Protect.u1WDT.delay, UART_WDT_TIME, timeDifference, Uart1_WDT_Timeout );	//Chris, for Lenovo spec
  CheckDebounce ( &Protect. u1PriRxWDT.Flag, &Protect. u1PriRxWDT.delay, UART_WDT_TIME, timeDifference, Uart1_PriRxWDT_Timeout );    //[davidchchen]20160503 added primary and secondary side communication error
  CheckDebounce ( &Protect.u2WDT.Flag, &Protect.u2WDT.delay, UART_WDT_TIME, timeDifference, Uart2_WDT_Timeout );
  CheckDebounce ( &Protect.U2TxDelay.Flag, &Protect.U2TxDelay.delay, U2TX_DELAY_TIMER, timeDifference, U2TxDelayHandler ); //Chris, for Teridian
  CheckDebounce ( &Protect.SCI_Retry.Flag, &Protect.SCI_Retry.delay, SCI_RETRY_TIME, timeDifference, SciRetryHandler );
  //************************* Soft Start ****************************************************************/
  CheckDebounce ( &Protect.SoftStart.Flag, &Protect.SoftStart.delay, SOFTSTART_DELAY_TIME, timeDifference, SoftStart_Delay_Handler );

  //*************************	Switch Signal*************************************************************/
  CheckDebounce ( &Protect.PSON_H.Flag, &Protect.PSON_H.delay, PSON_H_DEBOUNCE_TIME, timeDifference, PSON_H_Handler );
  CheckDebounce ( &Protect.PSON_L.Flag, &Protect.PSON_L.delay, PSON_L_DEBOUNCE_TIME, timeDifference, PSON_L_Handler );
    #if PRESENT_ENABLE  //[davidchchen] 20150107 not used PRESENT
  CheckDebounce ( &Protect.PRESENT_H.Flag, &Protect.PRESENT_H.delay, PRESENT_H_DEBOUNCE_TIME, timeDifference, PRESENT_H_Handler );
  CheckDebounce ( &Protect.PRESENT_L.Flag, &Protect.PRESENT_L.delay, PRESENT_L_DEBOUNCE_TIME, timeDifference, PRESENT_L_Handler );
    #endif
  CheckDebounce ( &Protect.PS_KILL_H.Flag, &Protect.PS_KILL_H.delay, PS_KILL_H_DEBOUNCE_TIME, timeDifference, PS_KILL_H_Handler );
  CheckDebounce ( &Protect.PS_KILL_L.Flag, &Protect.PS_KILL_L.delay, PS_KILL_L_DEBOUNCE_TIME, timeDifference, PS_KILL_L_Handler );

  //************************* PMBus*******************************************************************/
  //CheckDebounce(&Protect.TON_MAX_Fault.Flag, &Protect.TON_MAX_Fault.delay, TON_MAX_TIMER, timeDifference, TON_MAX_FAULT_Handler);
  CumulatePOS ( timeDifference );
#if SAMPLE_SET_SUPPORTED
  //Refresh Sample set
  if ( ! _SD_Flag.Val )
  {	//[Peter Chung] 20101214 added
      Protect.SAMPLE.Flag = 1;	//make the counter repeatly
      CheckDebounce ( &Protect.SAMPLE.Flag, &Protect.SAMPLE.delay, SAMPLE_TIME, timeDifference, UpdateSampleSets );
  }
  else
  {
      Protect.SAMPLE.Flag = 0;
      Protect.SAMPLE.delay = 0;
  }
#endif

  CheckDebounce ( &Protect.VinUVFault.Flag, &Protect.VinUVFault.delay, VIN_UV_TIME, timeDifference, VinUVFaultHandler );
  CheckDebounce ( &Protect.VinUVWarning.Flag, &Protect.VinUVWarning.delay, VIN_UV_TIME, timeDifference, VinUVWarningHandler );

  //************************* Current Share ************************************************************/
  //CheckDebounce ( &Protect.CS_CATCH.Flag, &Protect.CS_CATCH.delay, CS_CATCH_TIMER, timeDifference, CSDelayHandler );        //[davidchchen] 20150108 removed
  //CheckDebounce ( &Protect.CS_START.Flag, &Protect.CS_START.delay, CS_START_TIMER, timeDifference, CSStartupHandler );      //[davidchchen] 20150108 removed
  CheckDebounce ( &Protect.PriIS.Flag, &Protect.PriIS.delay, PRI_IS_TIMER, timeDifference, PriIS_Handler );
  //[davidchchen] 20150107 removed
  //CheckDebounce ( &Protect.DcRapidOnLowDelay.Flag, &Protect.DcRapidOnLowDelay.delay, DC_RAPID_ON_LOW_DELAY, timeDifference, DcRapidOnLow_Handler );

  //************************* Remote Sense ************************************************************/
  #if RS_ENABLE   //[davidchchen] 20150108 not used
  Protect.RS_Detect.Flag = 1;
  CheckDebounce ( &Protect.RS_Detect.Flag, &Protect.RS_Detect.delay, RS_DETECTION_TIME, timeDifference, RS_Det_Handler );
  #endif


  //************************* Dummy Load  ************************************************************/
  CheckDebounce ( &Protect.DummyLoadOFF.Flag, &Protect.DummyLoadOFF.delay, DUMMY_LOADOFF_DELAY_TIME, timeDifference, DummyLoadOFF_Handler );

}




