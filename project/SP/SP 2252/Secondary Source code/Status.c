
#include "Define.h"
#include "Pmbus.h"
#include "Process.h"
#include "Led.h"
#include "Fan.h"
#include "Isr.h"
#include "Standby.h"        //[davidchchen]20170619 added

extern tPS_FLAG PS;

#define STATUS_FAN_MASK	0b11110000	//[Peter Chung] 20110118 modified

static BYTE IsAlert ( )
{
  //[Peter Chung] 20100901 modified for IPMM spec. X03-00
  //[Peter Chung] 20101223 modified

  //Common
  if ( gPmbusCmd.STATUS_FAN_1_2.Val & ~ ( gAlertMask[PAGE0].STATUS_FAN_1_2_MASK & gAlertMask[PAGE1].STATUS_FAN_1_2_MASK ) )
  {
      return TRUE;
  }
  //if(gPmbusCmd.STATUS_OTHER.Val & ~(gAlertMask[PAGE0].STATUS_OTHER_MASK & gAlertMask[PAGE1].STATUS_OTHER_MASK)){return TRUE;}

  //PAGE 0
  if ( gPagePlusStatus.PAGE[PAGE0].STATUS_VOUT.Val & ~ ( gAlertMask[PAGE0].STATUS_VOUT_MASK ) )
  {
      return TRUE;
  }
  if ( gPagePlusStatus.PAGE[PAGE0].STATUS_IOUT.Val & ~ ( gAlertMask[PAGE0].STATUS_IOUT_MASK ) )
  {
      return TRUE;
  }
  if ( gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.Val & ~ ( gAlertMask[PAGE0].STATUS_INPUT_MASK ) )
  {
      return TRUE;
  }
  if ( gPagePlusStatus.PAGE[PAGE0].STATUS_TEMPERATURE.Val & ~ ( gAlertMask[PAGE0].STATUS_TEMPERATURE_MASK ) )
  {
      return TRUE;
  }
  if ( gPagePlusStatus.PAGE[PAGE0].STATUS_CML.Val & ~ ( gAlertMask[PAGE0].STATUS_CML_MASK ) )
  {
      return TRUE;
  }
  //if(gPmbusCmd.STATUS_WORD.byte.LB & ~(gAlertMask.STATUS_WORD_LB_MASK)){return TRUE;}
  //if(gPmbusCmd.STATUS_WORD.byte.HB & ~(gAlertMask.STATUS_WORD_HB_MASK)){return TRUE;}

  //PAGE 1
  if ( gPagePlusStatus.PAGE[PAGE1].STATUS_VOUT.Val & ~ ( gAlertMask[PAGE1].STATUS_VOUT_MASK ) )
  {
      return TRUE;
  }
  if ( gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.Val & ~ ( gAlertMask[PAGE1].STATUS_IOUT_MASK ) )
  {
      return TRUE;
  }
  if ( gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.Val & ~ ( gAlertMask[PAGE1].STATUS_INPUT_MASK ) )
  {
      return TRUE;
  }
  if ( gPagePlusStatus.PAGE[PAGE1].STATUS_TEMPERATURE.Val & ~ ( gAlertMask[PAGE1].STATUS_TEMPERATURE_MASK ) )
  {
      return TRUE;
  }
  if ( gPagePlusStatus.PAGE[PAGE1].STATUS_CML.Val & ~ ( gAlertMask[PAGE1].STATUS_CML_MASK ) )
  {
      return TRUE;
  }

  //STATUS_WORD
  if ( gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.bits.POWER_GOOD == 1 ||
       gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.bits.POWER_GOOD == 1 ||
       gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.bits._OFF == 1 ||
       gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.bits._OFF == 1 )
  {
      return TRUE;
  }

  return FALSE;
}

inline void UnsetSMBAlert ( )
{
  if ( iALERT_N )
  {
      //check if we can clear Alert signal or not
      if ( ! IsAlert ( ) )
      {
          //don't need to assert alert signal anymore
          oALERT_N = ALERT_OFF;
      }
  }
}

inline void SendSMBAlert ( )
{
  if ( IsAlert ( ) )
  {
      //Not alert yet, try to alert
      oALERT_N = ALERT_ON;
  }
  else
  {
      UnsetSMBAlert ( );
  }
}

void CheckFan_1_2_Status ( )
{
  if ( gPmbusCmd.STATUS_FAN_1_2.bits.FAN1_FAULT == 1 )
  {
      if ( gFan1.CurrentRpm > 500 && gFan1.Detected == TRUE )
      {
          gPmbusCmd.STATUS_FAN_1_2.bits.FAN1_FAULT = 0;
      }
  }

  if ( gPmbusCmd.STATUS_FAN_1_2.bits.FAN2_FAULT == 1 )
  {
      if ( gFan2.CurrentRpm > 500 )
      {
          gPmbusCmd.STATUS_FAN_1_2.bits.FAN2_FAULT = 0;
      }
  }

  if ( gPmbusCmd.STATUS_FAN_1_2.bits.FAN1_OVERRIDE == 1 )
  {
      if ( gFan1.OverridedDuty < GetFanMinDuty ( ) )
      {
          gPmbusCmd.STATUS_FAN_1_2.bits.FAN1_OVERRIDE = 0;
      }
  }

  if ( gPmbusCmd.STATUS_FAN_1_2.bits.FAN2_OVERRIDE == 1 )
  {
      if ( gFan1.OverridedDuty < GetFanMinDuty ( ) )
      {
          gPmbusCmd.STATUS_FAN_1_2.bits.FAN2_OVERRIDE = 0;
      }
  }

  gPmbusCmd.STATUS_FAN_1_2.bits.b0 = 0;
  gPmbusCmd.STATUS_FAN_1_2.bits.b1 = 0;
  gPmbusCmd.STATUS_FAN_1_2.bits.b4 = 0;
  gPmbusCmd.STATUS_FAN_1_2.bits.b5 = 0;
}

void CheckVoutStatus ( BYTE page )
{
  SHORT result;

  result = ( SHORT ) ADC.Vout_FF + ( SHORT ) gVoutReadOffsetAdc;
  if ( result < 0 )
  {
      result = 0;
  }

  if ( gPagePlusStatus.PAGE[page].STATUS_VOUT.bits.VOUT_OV_WARNING == 1 )
  {
      if ( page == PAGE0 )
      {
          if ( result < Parameter.OVP_VOUT_OV_WARN_LIMIT )
          {
              gPagePlusStatus.PAGE[PAGE0].STATUS_VOUT.bits.VOUT_OV_WARNING = 0;
          }
      }
      else if ( page == PAGE1 )
      {
          if ( ADC.StbVout_FF < STB_OVW_REF )
          {
              gPagePlusStatus.PAGE[PAGE1].STATUS_VOUT.bits.VOUT_OV_WARNING = 0;
              gPagePlusStatus.PAGE[PAGE1].STATUS_VOUT.bits.VOUT_OV_FAULT = 0; //[davidchchen]20170619 added
          }
      }
  }

#if 1
  if ( gPagePlusStatus.PAGE[page].STATUS_VOUT.bits.VOUT_UV_WARNING == 1 )
  {
      if ( page == PAGE0 )
      {
          if ( result > Parameter.UVP_VOUT_UV_WARN_LIMIT )
          {
              gPagePlusStatus.PAGE[PAGE0].STATUS_VOUT.bits.VOUT_UV_WARNING = 0;
          }
          
      }
      else if ( page == PAGE1 )
      {
          if ( ADC.StbVout_FF > STB_UVW_REF )
          {
              gPagePlusStatus.PAGE[PAGE1].STATUS_VOUT.bits.VOUT_UV_WARNING = 0;
              gPagePlusStatus.PAGE[PAGE1].STATUS_VOUT.bits.VOUT_UV_FAULT = 0;   //[davidchchen]20170619 added
          }
      }
  }
#endif

  gPagePlusStatus.PAGE[page].STATUS_VOUT.bits.b0 = 0;
  gPagePlusStatus.PAGE[page].STATUS_VOUT.bits.b1 = 0;
  gPagePlusStatus.PAGE[page].STATUS_VOUT.bits.b2 = 0;
  gPagePlusStatus.PAGE[page].STATUS_VOUT.bits.b3 = 0;
  //gPagePlusStatus.PAGE[page].STATUS_VOUT.bits.VOUT_UV_FAULT = 0;      //[David ch chen]20141205 not to clear VOUT_OV_FAULT by clear_fault command.
  //gPagePlusStatus.PAGE[page].STATUS_VOUT.bits.VOUT_OV_FAULT = 0;	//not to clear VOUT_OV_FAULT by clear_fault command

}

void CheckIoutStatus ( BYTE page )
{
  WORD result;

  if ( PS.IOUT1_CALIBRATED )
  {
      result = ADC.Iout_Cal;
  }
  else
  {
      result = ADC.Iout;
  }

#if 1
  if ( gPagePlusStatus.PAGE[page].STATUS_IOUT.bits.IOUT_OC_WARNING == 1 )
  {
      if ( page == PAGE0 )                                                      //[davidchchen]20170619 added
      {
            if ( result < Parameter.OCP_IOUT_OC_WARN_LIMIT )
            {
                gPagePlusStatus.PAGE[PAGE0].STATUS_IOUT.bits.IOUT_OC_WARNING = 0;
            }
      }
      else if ( page == PAGE1 )                                                 //[davidchchen]20170619 added
      {
            if ( ADC.StbIout_FF < STB_OCW_REF )                                 //[davidchchen]20170619 added
            {
                gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.bits.IOUT_OC_WARNING = 0;
                gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.bits.IOUT_OC_FAULT = 0;
            }
      }
      
  }
#endif

  gPagePlusStatus.PAGE[page].STATUS_IOUT.bits.b0 = 0;
  gPagePlusStatus.PAGE[page].STATUS_IOUT.bits.b1 = 0;
  gPagePlusStatus.PAGE[page].STATUS_IOUT.bits.b2 = 0;
  gPagePlusStatus.PAGE[page].STATUS_IOUT.bits.b3 = 0;
  gPagePlusStatus.PAGE[page].STATUS_IOUT.bits.b4 = 0;
  gPagePlusStatus.PAGE[page].STATUS_IOUT.bits.b6 = 0;
  
  //gPagePlusStatus.PAGE[page].STATUS_IOUT.bits.IOUT_OC_FAULT = 0;	//not to clear IOUT_OC_FAULT by clear_fault command
}

void CheckInputStatus ( BYTE page )
{
  if ( gPagePlusStatus.PAGE[page].STATUS_INPUT.bits.UNIT_OFF == 1 )
  {
      if ( iAC_OK == AC_GOOD )
      {
          gPagePlusStatus.PAGE[page].STATUS_INPUT.bits.UNIT_OFF = 0;
      }
  }

  if ( gPagePlusStatus.PAGE[page].STATUS_INPUT.bits.VIN_UV_FAULT == 1 )
  {
      if ( gInputLedStatus.bits.vin_uv_fault == 0 )
      {
          gPagePlusStatus.PAGE[page].STATUS_INPUT.bits.VIN_UV_FAULT = 0;
      }
  }

  if ( gPagePlusStatus.PAGE[page].STATUS_INPUT.bits.VIN_UV_WARNING == 1 )
  {
      if ( gInputLedStatus.bits.vin_uv_warning == 0 )
      {
          gPagePlusStatus.PAGE[page].STATUS_INPUT.bits.VIN_UV_WARNING = 0;
      }
  }

  if ( gPagePlusStatus.PAGE[page].STATUS_INPUT.bits.VIN_OV_WARNING == 1 )
  {
      if ( gInputLedStatus.bits.vin_ov_warning == 0 )
      {
          gPagePlusStatus.PAGE[page].STATUS_INPUT.bits.VIN_OV_WARNING = 0;
      }
  }

  if ( gPagePlusStatus.PAGE[page].STATUS_INPUT.bits.VIN_OV_FAULT == 1 )
  {
      if ( gInputLedStatus.bits.vin_ov_fault == 0 )
      {
          gPagePlusStatus.PAGE[page].STATUS_INPUT.bits.VIN_OV_FAULT = 0;
      }
  }

  gPagePlusStatus.PAGE[page].STATUS_INPUT.bits.b0 = 0;
  gPagePlusStatus.PAGE[page].STATUS_INPUT.bits.b1 = 0;
  gPagePlusStatus.PAGE[page].STATUS_INPUT.bits.b2 = 0;
}

void CheckTemperatureStatus ( BYTE page )
{
  if ( gPagePlusStatus.PAGE[page].STATUS_TEMPERATURE.bits.OT_WARNING == 1 )
  {
      if ( T_Inlet < Parameter.OTP_Tinlet_WARN_LIMIT &&
           T_Pri < Parameter.OTP_Tpri_WARN_LIMIT &&
           T_Sec < Parameter.OTP_Tsec_WARN_LIMIT )
      {
          gPagePlusStatus.PAGE[page].STATUS_TEMPERATURE.bits.OT_WARNING = 0;
      }
  }

  if ( gPagePlusStatus.PAGE[page].STATUS_TEMPERATURE.bits.OT_FAULT == 1 )
  {
      if ( T_Inlet < Parameter.OTP_Tinlet_FAULT_LIMIT &&
           T_Pri < Parameter.OTP_Tpri_FAULT_LIMIT &&
           T_Sec < Parameter.OTP_Tsec_FAULT_LIMIT )
      {
          gPagePlusStatus.PAGE[page].STATUS_TEMPERATURE.bits.OT_FAULT = 0;
      }
  }

  gPagePlusStatus.PAGE[page].STATUS_TEMPERATURE.bits.b0 = 0;
  gPagePlusStatus.PAGE[page].STATUS_TEMPERATURE.bits.b1 = 0;
  gPagePlusStatus.PAGE[page].STATUS_TEMPERATURE.bits.b2 = 0;
  gPagePlusStatus.PAGE[page].STATUS_TEMPERATURE.bits.b3 = 0;
  gPagePlusStatus.PAGE[page].STATUS_TEMPERATURE.bits.b4 = 0;
  gPagePlusStatus.PAGE[page].STATUS_TEMPERATURE.bits.b5 = 0;
}

void ClearFaultByCommand ( BYTE page )
{
  //[Peter Chung] 20101223 modified for PAGE PLUS function

  switch ( page )
  {
      case 0x00:
      case 0x01:
          CheckFan_1_2_Status ( );
          CheckVoutStatus ( page );
          CheckIoutStatus ( page );
          CheckInputStatus ( page );
          CheckTemperatureStatus ( page );
          gPagePlusStatus.PAGE[page].STATUS_CML.Val = 0;
          gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.bits._OFF = FALSE;
          gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.bits._OFF = FALSE;

          break;

      default:
          break;
  }

  gPagePlusStatus.PAGE[page].STATUS_CML.Val = 0;
  UnsetSMBAlert ( );
  gPagePlusStatus.PAGE[page].STATUS_CML.Val = 0;
}

void ClearAllStatus ( BYTE page )
{
  //[Peter Chung] 20101223 modified for PAGE PLUS function

  switch ( page )
  {
      case 0x00:
          //Common
          gPmbusCmd.STATUS_FAN_1_2.Val = 0;
          //gPmbusCmd.STATUS_OTHER.Val = 0;
          //Regarding to PagePlus
          gPagePlusStatus.PAGE[PAGE0].STATUS_VOUT.Val = 0;
          gPagePlusStatus.PAGE[PAGE0].STATUS_IOUT.Val = 0;
          gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.Val = 0;
          gPagePlusStatus.PAGE[PAGE0].STATUS_TEMPERATURE.Val = 0;
          gPagePlusStatus.PAGE[PAGE0].STATUS_CML.Val = 0;
          gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.Val = 0;
          break;
      case 0x01:
          //Common
          gPmbusCmd.STATUS_FAN_1_2.Val = 0;
          //gPmbusCmd.STATUS_OTHER.Val = 0;
          //Regarding to PagePlus
          gPagePlusStatus.PAGE[PAGE1].STATUS_VOUT.Val = 0;
          gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.Val = 0;
          gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.Val = 0;
          gPagePlusStatus.PAGE[PAGE1].STATUS_TEMPERATURE.Val = 0;
          gPagePlusStatus.PAGE[PAGE1].STATUS_CML.Val = 0;
          gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.Val = 0;
          break;
      case 0xFF:
          //Common
          gPmbusCmd.STATUS_FAN_1_2.Val = 0;
          //gPmbusCmd.STATUS_OTHER.Val = 0;
          //Regarding to PagePlus
          gPagePlusStatus.PAGE[PAGE0].STATUS_VOUT.Val = 0;
          gPagePlusStatus.PAGE[PAGE0].STATUS_IOUT.Val = 0;
          gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.Val = 0;
          gPagePlusStatus.PAGE[PAGE0].STATUS_TEMPERATURE.Val = 0;
          gPagePlusStatus.PAGE[PAGE0].STATUS_CML.Val = 0;
          gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.Val = 0;
          gPagePlusStatus.PAGE[PAGE1].STATUS_VOUT.Val = 0;
          gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.Val = 0;
          gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.Val = 0;
          gPagePlusStatus.PAGE[PAGE1].STATUS_TEMPERATURE.Val = 0;
          gPagePlusStatus.PAGE[PAGE1].STATUS_CML.Val = 0;
          gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.Val = 0;

          break;

      default:
          break;

  }

  UnsetSMBAlert ( );
}

void ClearStatusBit ( BYTE statusCmd, BYTE bitsToClear, BYTE page )
{
  //Only "Busy" bit can be cleared in STATUS_BYTE
  if ( statusCmd == 0x78 )
  {	//STATUS_BYTE
      if ( bitsToClear | 0b01111111 )
      {
          gPagePlusStatus.PAGE[gPmbusCmd.PAGE[0]].STATUS_CML.bits.INVALID_DATA = 1;
          return;
      }
      bitsToClear &= 0b10000000;
  }

  //[Peter Chung] 20110222 modified
  switch ( statusCmd )
  {
      case 0x78:	//STATUS_BYTE
          if ( page == 0xFF )
          {
              gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.byte.LB &= ~ bitsToClear;
              gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.byte.LB &= ~ bitsToClear;
          }
          else if ( page == 0x00 )
          {
              gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.byte.LB &= ~ bitsToClear;
          }
          else if ( page == 0x01 )
          {
              gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.byte.LB &= ~ bitsToClear;
          }
          break;
      case 0x7A:	//STATUS_VOUT
          if ( page == 0xFF )
          {
              gPagePlusStatus.PAGE[PAGE0].STATUS_VOUT.Val &= ~ bitsToClear;
              gPagePlusStatus.PAGE[PAGE1].STATUS_VOUT.Val &= ~ bitsToClear;
          }
          else if ( page == 0x00 )
          {
              gPagePlusStatus.PAGE[PAGE0].STATUS_VOUT.Val &= ~ bitsToClear;
          }
          else if ( page == 0x01 )
          {
              gPagePlusStatus.PAGE[PAGE1].STATUS_VOUT.Val &= ~ bitsToClear;
          }
          break;
      case 0x7B:	//STATUS_IOUT
          if ( page == 0xFF )
          {
              gPagePlusStatus.PAGE[PAGE0].STATUS_IOUT.Val &= ~ bitsToClear;
              gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.Val &= ~ bitsToClear;
          }
          else if ( page == 0x00 )
          {
              gPagePlusStatus.PAGE[PAGE0].STATUS_IOUT.Val &= ~ bitsToClear;
          }
          else if ( page == 0x01 )
          {
              gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.Val &= ~ bitsToClear;
          }
          break;
      case 0x7C:	//STATUS_INPUT
          if ( page == 0xFF )
          {
              gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.Val &= ~ bitsToClear;
              gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.Val &= ~ bitsToClear;
          }
          else if ( page == 0x00 )
          {
              gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.Val &= ~ bitsToClear;
          }
          else if ( page == 0x01 )
          {
              gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.Val &= ~ bitsToClear;
          }
          break;
      case 0x7D:	//STATUS_TEMPERATURE
          if ( page == 0xFF )
          {
              gPagePlusStatus.PAGE[PAGE0].STATUS_TEMPERATURE.Val &= ~ bitsToClear;
              gPagePlusStatus.PAGE[PAGE1].STATUS_TEMPERATURE.Val &= ~ bitsToClear;
          }
          else if ( page == 0x00 )
          {
              gPagePlusStatus.PAGE[PAGE0].STATUS_TEMPERATURE.Val &= ~ bitsToClear;
          }
          else if ( page == 0x01 )
          {
              gPagePlusStatus.PAGE[PAGE1].STATUS_TEMPERATURE.Val &= ~ bitsToClear;
          }
          break;
      case 0x7E:	//STATUS_CML
          if ( page == 0xFF )
          {
              gPagePlusStatus.PAGE[PAGE0].STATUS_CML.Val &= ~ bitsToClear;
              gPagePlusStatus.PAGE[PAGE1].STATUS_CML.Val &= ~ bitsToClear;
          }
          else if ( page == 0x00 )
          {
              gPagePlusStatus.PAGE[PAGE0].STATUS_CML.Val &= ~ bitsToClear;
          }
          else if ( page == 0x01 )
          {
              gPagePlusStatus.PAGE[PAGE1].STATUS_CML.Val &= ~ bitsToClear;
          }
          break;
#if 0
      case 0x7F:	//STATUS_OTHER
          gPmbusCmd.STATUS_OTHER.Val &= ~ bitsToClear;
          break;
#endif
      case 0x81:	//STATUS_FANS_1_2
          gPmbusCmd.STATUS_FAN_1_2.Val &= ~ bitsToClear;
          break;
  }

  UnsetSMBAlert ( );
}

void RefreshStatus ( )
{
  //Update STATUS_WORD / STATUS_BYTE here.

  //[Peter Chung] 20101223 modified for PAGE PLUS function
  BYTE page;

  //Common
  if ( gPmbusCmd.STATUS_FAN_1_2.Val )
  {
      gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.bits.FANS = 1;
      gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.bits.FANS = 1;
  }
  else
  {
      gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.bits.FANS = 0;
      gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.bits.FANS = 0;
  }

  //[Peter Chung] 20110419 added for STATUS_OTHER
#if 0
  if ( gPmbusCmd.STATUS_OTHER.Val )
  {
      gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.bits.OTHER = 1;
      gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.bits.OTHER = 1;
  }
  else
  {
      gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.bits.OTHER = 0;
      gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.bits.OTHER = 0;
  }
#endif

  //Regarding to PagePlus
  for ( page = PAGE0 ; page < TOTAL_PAGE_COUNT ; page ++ )
  {
      //[Peter Chung] 20120711 modified begin -->
      if ( gPagePlusStatus.PAGE[page].STATUS_VOUT.Val )
      {
          gPagePlusStatus.PAGE[page].STATUS_WORD.bits.VOUT = 1;
      }
      else
      {
          gPagePlusStatus.PAGE[page].STATUS_WORD.bits.VOUT = 0;
      }

      if ( gPagePlusStatus.PAGE[page].STATUS_VOUT.bits.VOUT_OV_FAULT )
      {
          gPagePlusStatus.PAGE[page].STATUS_WORD.bits.VOUT_OV = 1;
      }
      else
      {
          gPagePlusStatus.PAGE[page].STATUS_WORD.bits.VOUT_OV = 0;
      }
      //[Peter Chung] 20120711 modified end <--

      if ( gPagePlusStatus.PAGE[page].STATUS_IOUT.Val )
      {
          gPagePlusStatus.PAGE[page].STATUS_WORD.bits.IOUT = 1;
      }
      else
      {
          gPagePlusStatus.PAGE[page].STATUS_WORD.bits.IOUT = 0;
      }

      //[Peter Chung] 20110222 modified
      if ( gPagePlusStatus.PAGE[page].STATUS_IOUT.bits.IOUT_OC_FAULT )
      {
          gPagePlusStatus.PAGE[page].STATUS_WORD.bits.IOUT_OC = 1;
      }
      else
      {
          gPagePlusStatus.PAGE[page].STATUS_WORD.bits.IOUT_OC = 0;
      }

      if ( gPagePlusStatus.PAGE[page].STATUS_INPUT.Val )
      {
          gPagePlusStatus.PAGE[page].STATUS_WORD.bits._INPUT = 1;
      }
      else
      {
          gPagePlusStatus.PAGE[page].STATUS_WORD.bits._INPUT = 0;
      }

      //[Peter Chung] 20111110 added for VIN_UV
      if ( gPagePlusStatus.PAGE[page].STATUS_INPUT.bits.VIN_UV_FAULT )
      {
          gPagePlusStatus.PAGE[page].STATUS_WORD.bits.VIN_UV = 1;
      }
      else
      {
          gPagePlusStatus.PAGE[page].STATUS_WORD.bits.VIN_UV = 0;
      }

      if ( gPagePlusStatus.PAGE[page].STATUS_TEMPERATURE.Val )
      {
          gPagePlusStatus.PAGE[page].STATUS_WORD.bits.TEMPERATURE = 1;
      }
      else
      {
          gPagePlusStatus.PAGE[page].STATUS_WORD.bits.TEMPERATURE = 0;
      }

      if ( gPagePlusStatus.PAGE[page].STATUS_CML.Val )
      {
          gPagePlusStatus.PAGE[page].STATUS_WORD.bits.CML = 1;
      }
      else
      {
          gPagePlusStatus.PAGE[page].STATUS_WORD.bits.CML = 0;
      }
  }

  //[Peter Chung] 20111003 modified for new IPMM spec. X08-02
  //if(iPS_OK == P_OK){
  SendSMBAlert ( );
  //}
  //else{
  //UnsetSMBAlert();	//It's 12G spec request
  //}
}

BYTE isWarning ( )
{
  //Including OC Warning / OT Warning / FAN Warning
#if 0
  if ( ( gPagePlusStatus.PAGE[PAGE0].STATUS_IOUT.bits.IOUT_OC_WARNING == 1 ) ||
       ( gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.bits.IOUT_OC_WARNING == 1 ) ||
       ( gPagePlusStatus.PAGE[PAGE0].STATUS_TEMPERATURE.bits.OT_WARNING == 1 ) ||
       ( gPagePlusStatus.PAGE[PAGE1].STATUS_TEMPERATURE.bits.OT_WARNING == 1 ) ||
       ( gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.bits.VIN_OV_WARNING == 1 ) ||
       ( gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.bits.VIN_OV_WARNING == 1 ) )
  {
      return TRUE;
  }
  else
  {
      return FALSE;
  }
#else

  if ( gLedWarningStatus.Val != 0 )
  {
      return TRUE;
  }
  else
  {
      return FALSE;
  }

#endif
}


