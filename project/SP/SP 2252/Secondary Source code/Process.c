/*****************************************************************************************
 *
 *Copyright (C) 2012 Advanced Technology Development
 *                   Power SBG
 *                   LITE-ON TECHNOLOGY Corp.
 *
 *All Rights Reserved
 *
 *File Name : Process.c
 *
 *Date : 2012.05.24
 *
 *Author :
 *
 *Description :This Program used for HP 2400W Process Program.
 *
 *******************************************************************************************/

#include "Process.h"
#include "Protection.h"
#include "Parameter.h"
#include "I2c.h"
#include "Fan.h"
#include "Util.h"
#include "Pmbus.h"
#include "Sci.h"
#include "Isr.h"
#include "UserData.h"
#include "Psfb.h"
#include "Temp_table.h"
#include "Led.h"

#define METER_IC_ENABLED	TRUE

//---------Global variable----------------------------------------------------------------
tSD_FlagBITS _SD_Flag;
tMFR_STATUS MFR_Status;
DWORD gVout;
DWORD gIout;
DWORD gPout;
DWORD gVin;
DWORD gIin;	//Peter
DWORD gPin;
WORD T_Pri;
WORD T_Sec;
WORD T_Inlet;

extern tSTB STB;    //david add

//---------Extern variable----------------------------------------------------------------
extern tPS_FLAG PS;
extern tPSFB PSFB;
extern tPSU_STATE gPS_State;
//------------------------------ GetN ------------ --------------------------------------

char GetN ( BYTE type, WORD input )
{
  /*Refer to dEll 11G spec. 6.5.2   IPMM Sensor Formatting Tables*/
  switch ( type )
  {
      case N_VOUT:
          return - 9;
          break;
      case N_FAN_CMD:
          return 0;
          break;
      case N_FAN_SPEED:
          return 4;
          break;
      case N_TEMPERATURE:
          return - 2;
          break;
      case N_IIN:
          if ( input < 2 )
          {
              return - 9;
          }
          else if ( input < 4 )
          {
              return - 8;
          }
      case N_IOUT:
          if ( input < 8 )
          {
              return - 7;
          }
          else if ( input < 16 )
          {
              return - 6;
          }
          else if ( input < 32 )
          {
              return - 5;
          }
      case N_VIN:
          if ( input < 64 )
          {
              return - 4;
          }
      case N_POUT:
          if ( input < 128 )
          {
              return - 3;
          }
      case N_PIN:
          if ( input < 256 )
          {
              return - 2;
          }
          else if ( input < 512 )
          {
              return - 1;
          }
          else if ( input < 1024 )
          {
              return 0;
          }
          else if ( input < 2048 )
          {
              return 1;
          }
          else
          {
              return 2;
          }
          break;
      default:
          return 0;
          break;
  }
}

//-----------------------------------------------------------------------------------

void VinProcess ( )
{
  DWORD result;

#if WithTeridian
  //if ( PS.Uart1Error == FALSE && PS.Uart2Error == FALSE )
  if ( PS.Uart2Error == FALSE )           //[davidchchen]20160503 added primary and secondary side communication error
  {
      if ( iAC_OK == AC_GOOD || Real_Vac >= 60 )
      {
          result = ( Uart.U2.Rx.VacMsb << 8 ) | Uart.U2.Rx.VacLsb;
          result = LinearFmt_YtoX ( result, 5 );		// VIN_MODE -> N = -5

          if(! PS.I2C_Processing)   //[davidchchen] 20170914 added
          {
            gPmbusCmd.READ_VIN[0] = result & 0xFF;
            gPmbusCmd.READ_VIN[1] = ( result >> 8 ) & 0xFF;
          }
      }
      else
      {
          gPmbusCmd.READ_VIN[0] = 0;
          gPmbusCmd.READ_VIN[1] = 0;
      }
  }
  else
  {
      gPmbusCmd.READ_VIN[0] = 0;
      gPmbusCmd.READ_VIN[1] = 0;
  }
#endif
}

void IinProcess ( )
{
  WORD Iin = 0;

#if WithTeridian
  //if ( PS.Uart1Error == FALSE && PS.Uart2Error == FALSE )
  if ( PS.Uart2Error == FALSE )           //[davidchchen]20160503 added primary and secondary side communication error
  {
      if ( iAC_OK == AC_GOOD || Real_Vac >= 60 )
      {
          Iin = Uart.U2.Rx.IinAvg;	//IIN_MODE -> N= -10
          if(! PS.I2C_Processing)   //[davidchchen] 20170914 added
          {
            gPmbusCmd.READ_IIN[0] = Iin & 0xFF;
            gPmbusCmd.READ_IIN[1] = ( Iin >> 8 ) & 0xFF;
          }
      }
      else
      {
          gPmbusCmd.READ_IIN[0] = 0;
          gPmbusCmd.READ_IIN[1] = 0;
      }
  }
  else
  {
      gPmbusCmd.READ_IIN[0] = 0;
      gPmbusCmd.READ_IIN[1] = 0;
  }
#endif	

}

void PinProcess ( )
{
  WORD Pin = 0;

#if WithTeridian
  //if ( PS.Uart1Error == FALSE && PS.Uart2Error == FALSE )
  if ( PS.Uart2Error == FALSE )           //[davidchchen]20160503 added primary and secondary side communication error
  {

      //Pin = LinearFmt_XtoY(Uart.U2.Rx.PinAvg, GetN(N_PIN, (Uart.U2.Rx.PinAvg >> PIN_GAIN)), PIN_GAIN);	//translate to Linear Format

      if ( iAC_OK == AC_GOOD || Real_Vac >= 60 )
      {
          Pin = Uart.U2.Rx.PinAvg >> PIN_GAIN;

          if(! PS.I2C_Processing)   //[davidchchen] 20170914 added
          {
            gPmbusCmd.READ_PIN[0] = Pin & 0xFF;
            gPmbusCmd.READ_PIN[1] = ( Pin >> 8 ) & 0xFF;
          }
      }
      else
      {
          gPmbusCmd.READ_PIN[0] = 0;
          gPmbusCmd.READ_PIN[1] = 0;
      }
  }
  else
  {
      gPmbusCmd.READ_PIN[0] = 0;
      gPmbusCmd.READ_PIN[1] = 0;
  }
#endif
}

void VoutProcess ( )
{
  LONG result = 0;

  //if(gIsFactoryMode || (PS.Uart1Error == FALSE && PS.Uart2Error == FALSE)){
  //if ( ( gIsFactoryMode || ( PS.Uart1Error == FALSE && PS.Uart2Error == FALSE ) ) && ( iAC_OK == AC_GOOD ) )
  if ( ( gIsFactoryMode || ( PS.Uart2Error == FALSE ) ) && ( iAC_OK == AC_GOOD ) )           //[davidchchen]20160503 added primary and secondary side communication error
  {	//Peter
      result = ( ( SHORT ) ADC.Vout_FF + ( SHORT ) gVoutReadOffsetAdc );	//[Peter Chung] 20100805 added
      if ( result < 0 )
      {
          result = 0;
      }
      ADC.Vout_Cal = result;

      result = gVout = result * ADC_TO_VOUT;

      //N of Vout is fixed -8, Y = (X << 8) >> VOUT_GAIN => Y = X >> 3
      result = result >> 3;

      if ( gPmbusCmd.PAGE[0] == 0 )
      {
          if ( PSFB.MainEnabled == TRUE )
          {
              if(! PS.I2C_Processing)   //[davidchchen] 20170914 added
              {
                gPmbusCmd.READ_VOUT[0] = result & 0xFF;
                gPmbusCmd.READ_VOUT[1] = ( result >> 8 ) & 0xFF;
              }
          }
          else
          {
              gPmbusCmd.READ_VOUT[0] = 0;
              gPmbusCmd.READ_VOUT[1] = 0;
          }
      }
      else
      {	//PAGE == 1
          result = ADC.StbVout_FF * ADC_TO_VOUT;
          result = result >> 3;

          if(! PS.I2C_Processing)   //[davidchchen] 20170914 added
          {
            gPmbusCmd.READ_VOUT[0] = result & 0xFF;
            gPmbusCmd.READ_VOUT[1] = ( result >> 8 ) & 0xFF;
          }
      }
  }
  else
  {
      gPmbusCmd.READ_VOUT[0] = 0;
      gPmbusCmd.READ_VOUT[1] = 0;
  }
}

void IoutProcess ( )
{
  DWORD result = 0;
  signed char N;

  //if(gIsFactoryMode || (PS.Uart1Error == FALSE && PS.Uart2Error == FALSE)){
  //if ( ( gIsFactoryMode || ( PS.Uart1Error == FALSE && PS.Uart2Error == FALSE ) ) && ( iAC_OK == AC_GOOD ) )
  if ( ( gIsFactoryMode || ( PS.Uart2Error == FALSE ) ) && ( iAC_OK == AC_GOOD ) )           //[davidchchen]20160503 added primary and secondary side communication error
  {	//Peter
      if ( PS.IOUT1_CALIBRATED )
      {
          static DWORD temp_LPF = 0;
          static DWORD temp_FF = 0;

          //gIout = result = ((DWORD)ADC.Iout_Cal * ADC_TO_IOUT);
          result = ( ( DWORD ) ADC.Iout_report_Cal * ADC_TO_IOUT );
          temp_LPF = temp_LPF + result - temp_FF;
          gIout = temp_FF = temp_LPF >> 1;	//>> 4;	//Peter
      }
      else
      {
          gIout = result = ( DWORD ) ADC.Iout_FF * ADC_TO_IOUT;
      }

      //N = GetN(N_IOUT, (WORD)(result >> ADC_TO_IOUT_GAIN));		//[Peter Chung] 20101022 test
      //result = LinearFmt_XtoY(result, N, ADC_TO_IOUT_GAIN);

      //ADC_TO_IOUT_GAIN is 2^13, IOUT_MODE -> N is -7, should right shift 6
      result = result >> 6;

      if ( PSFB.MainEnabled == TRUE )
      {
          if(! PS.I2C_Processing)   //[davidchchen] 20170914 added
          {
            gPmbusCmd.READ_IOUT[0] = result & 0xFF;
            gPmbusCmd.READ_IOUT[1] = ( result >> 8 ) & 0xFF;
          }
      }
      else
      {
          gPmbusCmd.READ_IOUT[0] = 0x00;
          gPmbusCmd.READ_IOUT[1] = 0x00;
      }

      gPmbusCmd.READ_IOUT_LS[0] = result & 0xFF;              //[davidchchen]20160408 removed
      gPmbusCmd.READ_IOUT_LS[1] = ( result >> 8 ) & 0xFF;     //[davidchchen]20160408 removed


      //gPmbusCmd.READ_IOUT_LS[0] = ADC.Iout_report_FF & 0xFF;            //[davidchchen]20160408 added, read Iout_report_ff
      //gPmbusCmd.READ_IOUT_LS[1] = ( ADC.Iout_report_FF >> 8 ) & 0xFF;   //[davidchchen]20160408 added, read Iout_report_ff

      //For Icurrent_share reporting
      {
          DWORD result = 0;

          result = ( DWORD ) ADC.IoutCS_FF * ADC_TO_IOUT;
          N = GetN ( N_IOUT, ( WORD ) ( result >> ADC_TO_IOUT_GAIN ) );
          result = LinearFmt_XtoY ( result, N, ADC_TO_IOUT_GAIN );
          gPmbusCmd.READ_IOUT_CS[0] = result & 0xFF;
          gPmbusCmd.READ_IOUT_CS[1] = ( result >> 8 ) & 0xFF;
      }


  }
  else
  {
      gPmbusCmd.READ_IOUT[0] = 0;
      gPmbusCmd.READ_IOUT[1] = 0;
  }
}

void PoutProcess ( )
{
  DWORD result = 0;
  BYTE totalIoutGain = 0;

  //if(gIsFactoryMode || (PS.Uart1Error == FALSE && PS.Uart2Error == FALSE)){
  //if ( ( gIsFactoryMode || ( PS.Uart1Error == FALSE && PS.Uart2Error == FALSE ) ) && ( iAC_OK == AC_GOOD ) )
  if ( ( gIsFactoryMode || ( PS.Uart2Error == FALSE ) ) && ( iAC_OK == AC_GOOD ) )           //[davidchchen]20160503 added primary and secondary side communication error
  {	//Peter
      totalIoutGain = ADC_TO_IOUT_GAIN;	//total gain for gIout
      result = ( DWORD ) ( ( ( QWORD ) gVout * gIout ) >> totalIoutGain );	//divide total gain for gIout first to prevent DWORD overflow
      gPout = ( WORD ) ( result >> VOUT_GAIN );

      //N = GetN(N_POUT, gPout);	//shift VOUT_GAIN here to get the real value
      //result = LinearFmt_XtoY(result, N, VOUT_GAIN);	//shift VOUT_GAIN here to get the real value

      if(! PS.I2C_Processing)   //[davidchchen] 20170914 added
      {
        gPmbusCmd.READ_POUT[0] = gPout & 0xFF;
        gPmbusCmd.READ_POUT[1] = ( gPout >> 8 ) & 0xFF;
      }
  }
  else
  {
      gPmbusCmd.READ_POUT[0] = 0;
      gPmbusCmd.READ_POUT[1] = 0;
  }

}

//-------------------------------- TpriProcess ---------------------------------------

static void Fan1Monitor ( )
{
  DWORD result = 0;

  //Detect if fan is availble
  //[Peter Chung] 20110124 modified begin =>
  if ( gFan1.Detected == FALSE )
  {
      Protect.Fan1Disappear.Flag = 1;
  }
  else
  {
      Protect.Fan1Disappear.Flag = 0;
      Protect.Fan1Disappear.delay = 0;
      PS.Fan1Disappear = 0;
  }
  //[Peter Chung] 20110124 modified end <=


  //Check Fan Lock
#if 1
  if ( iAC_OK == AC_GOOD )
  {
      if ( ! PS.FanLock_Disable && _SD_Flag.FAN_LOCK == FALSE )
      {
          //[Peter Chung] 20101220 modified for fan warning condition --->
#if 0
          if ( gFan1.CurrentRpm < 500 || gFan1.Detected == FALSE )
          {
              Protect.Fan1Warning.Flag = 1;
          }
          else
          {
              Protect.Fan1Warning.Flag = 0;
              Protect.Fan1Warning.delay = 0;
              gPmbusCmd.STATUS_FAN_1_2.bits.FAN1_WARNING = 0;
          }
#endif
          //[Peter Chung] 20101220 modified for fan warning condition <---

          if ( gFan1.CurrentRpm < 500 || gFan1.Detected == FALSE )
          {
              Protect.Fan1Fault.Flag = 1;
              Protect.Fan1Lock.Flag = 1;
          }
          else
          {
              Protect.Fan1Lock.Flag = 0;
              Protect.Fan1Lock.delay = 0;
              Protect.Fan1Fault.Flag = 0;
              Protect.Fan1Fault.delay = 0;
          }
      }
      else
      {
          Protect.Fan1Warning.Flag = 0;
          Protect.Fan1Warning.delay = 0;
          Protect.Fan1Lock.Flag = 0;
          Protect.Fan1Lock.delay = 0;
          Protect.Fan1Fault.Flag = 0;
          Protect.Fan1Fault.delay = 0;
      }
  }
  else
  {
      //[Peter Chung] 20100917 added for AC_NOK situation
      Protect.Fan1Warning.Flag = 0;
      Protect.Fan1Warning.delay = 0;
      Protect.Fan1Lock.Flag = 0;
      Protect.Fan1Lock.delay = 0;
      Protect.Fan1Fault.Flag = 0;
      Protect.Fan1Fault.delay = 0;
  }
#endif
  //if ( PS.Uart1Error == FALSE && PS.Uart2Error == FALSE && PS.Fan1Disappear == FALSE )
  if ( PS.Uart2Error == FALSE && PS.Fan1Disappear == FALSE )          //[davidchchen]20160503 added primary and secondary side communication error
  {
      if ( iFAN_CNTL == Fan_Enable )
      {
          result = LinearFmt_XtoY ( gFan1.CurrentRpm, 5, 0 );	//original N = 4, but it will get overflow
          if(! PS.I2C_Processing)   //[davidchchen] 20170914 added
          {
            gPmbusCmd.READ_FAN_SPEED_1[0] = result & 0xFF;
            gPmbusCmd.READ_FAN_SPEED_1[1] = ( result >> 8 ) & 0xFF;
          }
      }
      else
      {
          gPmbusCmd.READ_FAN_SPEED_1[0] = 0;
          gPmbusCmd.READ_FAN_SPEED_1[1] = 0;
      }
  }
  else
  {
      gPmbusCmd.READ_FAN_SPEED_1[0] = 0;
      gPmbusCmd.READ_FAN_SPEED_1[1] = 0;
  }

  gFan1.Detected = FALSE;

}

static void Fan2Monitor ( )
{
  DWORD result = 0;

  if ( ( gFan2.PwmH_Cnt == 0 && gFan2.PwmL_Cnt == 0 ) || PS.Fan2Disappear == TRUE )
  {
      gFan2.CurrentRpm = 0;
  }
  else
  {
      gFan2.CurrentRpm = 60000000 / ( ( gFan2.PwmH_Cnt + gFan2.PwmL_Cnt ) << 1 );
  }

  //Check Fan Lock
#if 1
  if ( iAC_OK == AC_GOOD )
  {
      if ( ! PS.FanLock_Disable && _SD_Flag.FAN_LOCK == FALSE )
      {
          //[Peter Chung] 20101220 modified for fan warning condition --->
#if 0
          if ( gFan2.CurrentRpm < 500 )
          {
              Protect.Fan2Warning.Flag = 1;
          }
          else
          {
              Protect.Fan2Warning.Flag = 0;
              Protect.Fan2Warning.delay = 0;
              gPmbusCmd.STATUS_FAN_1_2.bits.FAN2_WARNING = 0;
          }
#endif
          //[Peter Chung] 20101220 modified for fan warning condition <---

          if ( gFan2.CurrentRpm < 500 )
          {
              Protect.Fan2Fault.Flag = 1;
              Protect.Fan2Lock.Flag = 1;
          }
          else
          {
              Protect.Fan2Lock.Flag = 0;
              Protect.Fan2Lock.delay = 0;
              Protect.Fan2Fault.Flag = 0;
              Protect.Fan2Fault.delay = 0;
          }
      }
      else
      {
          Protect.Fan2Warning.Flag = 0;
          Protect.Fan2Warning.delay = 0;
          Protect.Fan2Lock.Flag = 0;
          Protect.Fan2Lock.delay = 0;
          Protect.Fan2Fault.Flag = 0;
          Protect.Fan2Fault.delay = 0;
      }
  }
  else
  {
      //[Peter Chung] 20100917 added for AC_NOK situation
      Protect.Fan2Warning.Flag = 0;
      Protect.Fan2Warning.delay = 0;
      Protect.Fan2Lock.Flag = 0;
      Protect.Fan2Lock.delay = 0;
      Protect.Fan2Fault.Flag = 0;
      Protect.Fan2Fault.delay = 0;
  }
#endif
  //if ( PS.Uart1Error == FALSE && PS.Uart2Error == FALSE )
  if ( PS.Uart2Error == FALSE )          //[davidchchen]20160503 added primary and secondary side communication error
  {
      if ( iFAN_CNTL == Fan_Enable )
      {
          result = LinearFmt_XtoY ( gFan2.CurrentRpm, 5, 0 );	//original N = 4, but it will get overflow
          if(! PS.I2C_Processing)   //[davidchchen] 20170914 added
          {
            gPmbusCmd.READ_FAN_SPEED_2[0] = result & 0xFF;
            gPmbusCmd.READ_FAN_SPEED_2[1] = ( result >> 8 ) & 0xFF;
          }
      }
      else
      {
          gPmbusCmd.READ_FAN_SPEED_2[0] = 0;
          gPmbusCmd.READ_FAN_SPEED_2[1] = 0;
      }
  }
  else
  {
      gPmbusCmd.READ_FAN_SPEED_2[0] = 0;
      gPmbusCmd.READ_FAN_SPEED_2[1] = 0;
  }
}

//------------------------- CalculateNTSE0103FZD73Temp -----------------------------

static WORD CalculateNTSE0103FZD73Temp ( WORD TempAD )
{
  //Tambient

  WORD ReportTemp = 0;

  if ( TempAD >= 901 )	// 0C
  {
      ReportTemp = 0;
  }
  else if ( TempAD >= 865 )	// 20C
  {
      ReportTemp = ( SHORT ) ( ( ( ( LONG ) TempAD * ( - 568 ) ) + ( 512568 ) ) >> 10 );
  }
  else if ( TempAD >= 784 )	//45C
  {
      ReportTemp = ( SHORT ) ( ( ( ( LONG ) TempAD * ( - 316 ) ) + ( 293862 ) ) >> 10 );
  }
  else if ( TempAD >= 602 )	//80C
  {
      ReportTemp = ( SHORT ) ( ( ( ( LONG ) TempAD * ( - 196 ) ) + ( 200467 ) ) >> 10 );
  }
  else
  {
      ReportTemp = 80;
  }

  //Calibration
#if 1
  if ( ReportTemp <= 39 )
  {
      ReportTemp = ReportTemp + 1;
  }
#endif

  return ReportTemp;
}

//------------------------- CalculateNCP18XH103F03RBTemp -----------------------------

static WORD CalculateNCP18XH103F03RBTemp ( WORD TempAD )
{
  //Tpri & Tsec

  WORD ReportTemp = 0;

  if ( TempAD >= 900 )	// 0C
  {
      ReportTemp = 0;
  }
  else if ( TempAD >= 839 )	// 30C
  {
      ReportTemp = ( SHORT ) ( ( ( ( LONG ) TempAD * ( - 503 ) ) + ( 453245 ) ) >> 10 );
  }
  else if ( TempAD >= 715 )	//60C
  {
      ReportTemp = ( SHORT ) ( ( ( ( LONG ) TempAD * ( - 247 ) ) + ( 238575 ) ) >> 10 );
  }
  else if ( TempAD >= 368 )	//120C
  {
      ReportTemp = ( SHORT ) ( ( ( ( LONG ) TempAD * ( - 177 ) ) + ( 188038 ) ) >> 10 );
  }
  else
  {
      ReportTemp = 120;
  }

  return ReportTemp;
}

//-------------------------------- TpriProcess ---------------------------------------

static void TpriProcess ( )
{
  //Temperature of Primary side
  DWORD result = 0;
  WORD temp;
  static WORD T_PriCnt = 0;            //[davidchchen]20170119 Modify Primary Temp Report Issue

  temp = Uart.U1.Rx.Pri_Temperature.Val;

#if 0
  if ( temp > Temp_AD_PARAMETER )
  {
      if ( ( temp - Temp_AD_PARAMETER ) >= Temp_Table_Total_Number )
      {
          T_Pri = 0;
      }
      else
      {
          T_Pri = temp_table[temp - Temp_AD_PARAMETER];
      }
  }
  else
  {
      T_Pri = 120;
  }
#else

  //if (T_PriCnt++ >= 20)                                                       //[davidchchen]20170119 Modify Primary Temp Report Issue
  if ( ( PS.Uart1Error == FALSE ) && (T_PriCnt++ >= 20) )                       //[davidchchen]20170216 Modify Primary Temp Report Issue
  {
    T_PriCnt = 25;                                                              //[davidchchen]20170119 Modify Primary Temp Report Issue
    T_Pri = CalculateNCP18XH103F03RBTemp ( temp );                              //[davidchchen]20170119 Modify Primary Temp Report Issue
  }
  else
  {
    T_Pri = 25;                                                                 //[davidchchen]20170119 Modify Primary Temp Report Issue
  }



#endif

  if ( T_Pri >= Parameter.OTP_Tpri_FAULT_LIMIT )
  {
      if ( _SD_Flag.OTP_SD == 0 )
      {
          Protect.OTP_Tpri.Flag = 1;
      }
  }
  else
  {
      Protect.OTP_Tpri.Flag = 0;
      Protect.OTP_Tpri.delay = 0;
  }

  if ( T_Pri >= Parameter.OTP_Tpri_WARN_LIMIT )
  {	//[Peter Chung] 20101206 added
      Protect.OTW_Tpri.Flag = 1;
  }
  else
  {
      Protect.OTW_Tpri.Flag = 0;
      Protect.OTW_Tpri.delay = 0;
      gLedWarningStatus.bits.ot2_warning = 0;
  }

  //if ( _SD_Flag.OTP_SD || _SD_Flag.STB_OTP )
  if ( _SD_Flag.OTP_SD )    //[David ch chen]20141124 removed _SD_Flag.STB_OTP, not use _SD_Flag.STB_OTP
  {
      if ( T_Pri < Parameter.OTP_Tpri_Recovery_LIMIT )
      {
          Protect.OT_Recover_Tpri.Flag = 1;
      }
      else
      {
          Protect.OT_Recover_Tpri.Flag = 0;
          Protect.OT_Recover_Tpri.delay = 0;
      }
  }

  //if ( gIsFactoryMode || ( PS.Uart1Error == FALSE && PS.Uart2Error == FALSE ) )
  if ( gIsFactoryMode || ( PS.Uart1Error == FALSE ) )          //[davidchchen]20160503 added primary and secondary side communication error
  {
      result = LinearFmt_XtoY ( T_Pri, 0, 0 );	//N is fixed 0
      if(! PS.I2C_Processing)   //[davidchchen] 20170914 added
      {
        gPmbusCmd.READ_TEMPERATURE_2[0] = result & 0xFF;
        gPmbusCmd.READ_TEMPERATURE_2[1] = ( result >> 8 ) & 0xFF;
      }
  }
  else
  {
      gPmbusCmd.READ_TEMPERATURE_2[0] = 0;
      gPmbusCmd.READ_TEMPERATURE_2[1] = 0;
  }

}
//-------------------------------- TsecProcess ---------------------------------------

static void TsecProcess ( )
{
  //Temperature of Sec
  DWORD result = 0;
  WORD temp;

  temp = ADC.Tsec_FF;

#if 0
  if ( temp > Temp_AD_PARAMETER )
  {
      if ( ( temp - Temp_AD_PARAMETER ) >= Temp_Table_Total_Number )
      {
          T_Sec = 0;
      }
      else
      {
          T_Sec = temp_table[temp - Temp_AD_PARAMETER];
      }
  }
  else
  {
      T_Sec = 120;
  }
#else

  T_Sec = CalculateNCP18XH103F03RBTemp ( temp );

#endif

  //Check Sec OTP
  if ( T_Sec >= Parameter.OTP_Tsec_FAULT_LIMIT )
  {
      if ( _SD_Flag.OTP_SD == 0 )
      {
          Protect.OTP_Tsec.Flag = 1;
      }
  }
  else
  {
      Protect.OTP_Tsec.Flag = 0;
      Protect.OTP_Tsec.delay = 0;
  }

  if ( T_Sec >= Parameter.OTP_Tsec_WARN_LIMIT )
  {
      Protect.OTW_Tsec.Flag = 1;
  }
  else
  {
      Protect.OTW_Tsec.Flag = 0;
      Protect.OTW_Tsec.delay = 0;
      gLedWarningStatus.bits.ot3_warning = 0;
  }

  //if ( _SD_Flag.OTP_SD || _SD_Flag.STB_OTP )
  if ( _SD_Flag.OTP_SD )    //[David ch chen]20141124 removed _SD_Flag.STB_OTP, not use _SD_Flag.STB_OTP
  {
      if ( T_Sec < Parameter.OTP_Tsec_Recovery_LIMIT )
      {
          Protect.OT_Recover_Tsec.Flag = 1;
      }
      else
      {
          Protect.OT_Recover_Tsec.Flag = 0;
          Protect.OT_Recover_Tsec.delay = 0;
      }
  }

  //if ( gIsFactoryMode || ( PS.Uart1Error == FALSE && PS.Uart2Error == FALSE ) )
  if ( gIsFactoryMode || ( PS.Uart2Error == FALSE ) )          //[davidchchen]20160503 added primary and secondary side communication error
  {
      result = LinearFmt_XtoY ( T_Sec, 0, 0 );	//N is fixed 0
      if(! PS.I2C_Processing)   //[davidchchen] 20170914 added
      {
        gPmbusCmd.READ_TEMPERATURE_3[0] = result & 0xFF;
        gPmbusCmd.READ_TEMPERATURE_3[1] = ( result >> 8 ) & 0xFF;
      }
  }
  else
  {
      gPmbusCmd.READ_TEMPERATURE_3[0] = 0;
      gPmbusCmd.READ_TEMPERATURE_3[1] = 0;
  }
}

//-------------------------------- TinletProcess ---------------------------------------

static void TinletProcess ( )
{
  //Temperature of Ambient
  DWORD result = 0;
  WORD temp;

  temp = ADC.Tinlet_FF;


#if 0
  if ( temp > Temp_AD_PARAMETER )
  {
      if ( ( temp - Temp_AD_PARAMETER ) >= Temp_Table_Total_Number )
      {
          T_Inlet = 0;
      }
      else
      {
          T_Inlet = temp_table[temp - Temp_AD_PARAMETER];
      }
  }
  else
  {
      T_Inlet = 120;
  }
#else

  T_Inlet = CalculateNTSE0103FZD73Temp ( temp );

#endif

  //Check Ambient OTP
  if ( T_Inlet >= Parameter.OTP_Tinlet_FAULT_LIMIT )
  {
      if ( _SD_Flag.OTP_SD == 0 )
      {
          Protect.OTP_Tinlet.Flag = 1;
      }
  }
  else
  {
      Protect.OTP_Tinlet.Flag = 0;
      Protect.OTP_Tinlet.delay = 0;
  }

  if ( T_Inlet >= Parameter.OTP_Tinlet_WARN_LIMIT )
  {
      Protect.OTW_Tinlet.Flag = 1;
  }
  else
  {
      Protect.OTW_Tinlet.Flag = 0;
      Protect.OTW_Tinlet.delay = 0;
      gLedWarningStatus.bits.ot1_warning = 0;
  }

  //if ( _SD_Flag.OTP_SD || _SD_Flag.STB_OTP )
  if ( _SD_Flag.OTP_SD )    //[David ch chen]20141124 removed _SD_Flag.STB_OTP, not use _SD_Flag.STB_OTP
  {
      if ( T_Inlet < Parameter.OTP_Tinlet_Recovery_LIMIT )
      {
          Protect.OT_Recover_Tinlet.Flag = 1;
      }
      else
      {
          Protect.OT_Recover_Tinlet.Flag = 0;
          Protect.OT_Recover_Tinlet.delay = 0;
      }
  }

  //if ( gIsFactoryMode || ( PS.Uart1Error == FALSE && PS.Uart2Error == FALSE ) )
  if ( gIsFactoryMode || ( PS.Uart2Error == FALSE ) )          //[davidchchen]20160503 added primary and secondary side communication error
  {
      result = LinearFmt_XtoY ( T_Inlet, 0, 0 ); //N is fixed 0
      if(! PS.I2C_Processing)   //[davidchchen] 20170914 added
      {
        gPmbusCmd.READ_TEMPERATURE_1[0] = result & 0xFF;
        gPmbusCmd.READ_TEMPERATURE_1[1] = ( result >> 8 ) & 0xFF;
      }
  }
  else
  {
      gPmbusCmd.READ_TEMPERATURE_1[0] = 0;
      gPmbusCmd.READ_TEMPERATURE_1[1] = 0;
  }

}

//-------------------------------- ResetSD_Flag ---------------------------------------

void ResetSD_Flag ( )
{
  _SD_Flag.FAN_LOCK = 0;
  _SD_Flag.OVP_SD = 0;
  _SD_Flag.OTP_SD = 0;
  _SD_Flag.OCP_SD = 0;
  _SD_Flag.UV_SD = 0;
  //_SD_Flag.OPP_SD = 0;      //[David ch chen]20141126 reomoved _SD_Flag.OPP_SD not use
  _SD_Flag.SCP_SD = 0;
  _SD_Flag.LATCH_SD = 0;
  _SD_Flag.STB_OCP = 0;
  //_SD_Flag.STB_OTP = 0;   //[David ch chen]20141124 removed _SD_Flag.STB_OTP, not use _SD_Flag.STB_OTP
  _SD_Flag.STB_UVP = 0;
  _SD_Flag.STB_OVP = 0;
  _SD_Flag.ISP_MODE = 0;
}

//-------------------------------- ResetSD_Flag ---------------------------------------

void PSOnOff_ResetSD_Flag ( )		// [Tommy YR Chen] 20110317 added
{
  _SD_Flag.FAN_LOCK = 0;
  _SD_Flag.OVP_SD = 0;
  _SD_Flag.OTP_SD = 0;
  _SD_Flag.OCP_SD = 0;
  _SD_Flag.UV_SD = 0;
  //_SD_Flag.OPP_SD = 0;      //[David ch chen]20141126 reomoved _SD_Flag.OPP_SD not use
  _SD_Flag.SCP_SD = 0;
  _SD_Flag.LATCH_SD = 0;
#if 0
  _SD_Flag.STB_OCP = 0;
  _SD_Flag.STB_OTP = 0;
  _SD_Flag.STB_UVP = 0;
  _SD_Flag.STB_OVP = 0;
#endif
  _SD_Flag.ISP_MODE = 0;
}

//-------------------------------- IsReportCmd ---------------------------------------

BYTE IsReportCmd ( BYTE cmd )
{
  if ( cmd == 0x88 ||	//READ_VIN
       cmd == 0x89 ||	//READ_IIN
       cmd == 0x97 ||	//READ_PIN
       cmd == 0x8B ||	//READ_VOUT
       cmd == 0x8C ||	//READ_IOUT
       cmd == 0x96 ||	//READ_POUT
       cmd == 0x8D ||	//T1
       cmd == 0x8E ||	//T2
       cmd == 0x8F ||	//T3
       cmd == 0x90 )
  {	//FAN_SPEED_1
      return TRUE;
  }
  else
  {
      return FALSE;
  }
}

//-------------------------------- UpdateDebugInfo ---------------------------------------

static void UpdateDebugInfo ( )
{
  //For Command 0x77
  gPmbusCmd.DEBUG_INFO[0] = _SD_Flag.Val & 0xFF;
  gPmbusCmd.DEBUG_INFO[1] = ( _SD_Flag.Val >> 8 ) & 0xFF;
  gPmbusCmd.DEBUG_INFO[2] = STB.Enabled; //0; david add
  gPmbusCmd.DEBUG_INFO[3] = MFR_Status.Val;         //[davidchchen]20160503 added primary and secondary side communication error
  gPmbusCmd.DEBUG_INFO[4] = gPS_State.mainState;
  gPmbusCmd.DEBUG_INFO[5] = gPS_State.subState;

}

//-------------------------------- UpdateCSCali---- ---------------------------------------

static void UpdateCSCali ( )
{
  gPmbusCmd.CALI_INFO[0] = UserData.Page2.region.Calibration_Flag.Val & 0xFF;
  gPmbusCmd.CALI_INFO[1] = ( UserData.Page2.region.Calibration_Flag.Val >> 8 ) & 0xFF;
}

//-------------------------------- ScanProcess ---------------------------------------

void ScanProcess ( )
{
#if 0	//for debug
  gPmbusCmd.IIN_OC_WARN_LIMIT[0] = gPmbusCmd.CS_PWM_FB[0] & 0xFF;
  gPmbusCmd.IIN_OC_WARN_LIMIT[1] = gPmbusCmd.CS_PWM_FB[1] & 0xFF;
  gPmbusCmd.PIN_OP_WARN_LIMIT[0] = gPmbusCmd.CS_IN[0] & 0xFF; //ADC.IoutCS & 0xFF;
  gPmbusCmd.PIN_OP_WARN_LIMIT[1] = gPmbusCmd.CS_IN[1] & 0xFF;	//(ADC.IoutCS>>8) & 0xFF;
#endif

  //Input Information
#if WithTeridian
  //if(PS.U2RX_Updated){
  PS.U2RX_Updated = FALSE;
  VinProcess ( );
  IinProcess ( );
  PinProcess ( );

  //}
#endif

  //Output Information
  //VoutProcess();

  IoutProcess ( );

  PoutProcess ( );

  //Temperature
  TpriProcess ( );

  TsecProcess ( );

  TinletProcess ( );

  //Fan
  Fan1Monitor ( );

  Fan2Monitor ( );

  //Debug
  UpdateDebugInfo ( );

  UpdateCSCali ( );

  UpdateLog ( gPmbusCmd.LOG_INDEX[0] );
}
//---------------- T5Interrupt ---> ScanProcess ---------------------------------------

void __attribute__ ( ( __interrupt__, no_auto_psv ) ) _T5Interrupt ( void )
{
  //PS.ReportDataUpdating = 1;      //[davidchchen]20160503 removed
  if ( PS.SaveBlackBox == FALSE )
  {
      ScanProcess ( );
  }
  //PS.ReportDataUpdating = 0;      //[davidchchen]20160503 removed

  IFS1bits.T5IF = 0;
}



