
#include "p33Fxxxx.h"
#include "define.h"
#include "Parameter.h"
#include "dsp.h"
#include "Process.h"
#include "Util.h"
#include "Psfb.h"
#include "UserData.h"
#include "Protection.h"
#include "Pmbus.h"
#include "PowerOnOff.h"
#include "Fan.h"
#include "Isr.h"
#include "Timer.h"
#include <string.h>


//-------------Definition ------------------------------------------------------------

#define DUMMY_LOAD_ON_LIMIT (((WORD)1 * IOUT_TO_ADC) >> IOUT_TO_ADC_GAIN)
#define DUMMY_LOAD_OFF_LIMIT (((WORD)3 * IOUT_TO_ADC) >> IOUT_TO_ADC_GAIN)

//-------------Global variable---------------------------------------------------------
volatile unsigned long CaliCSIoutOffset = 0;
volatile unsigned long CaliCShareOffset = 0;

WORD CurrentShareError = 0;
SHORT Iout_Real = 0;
SHORT CSHARE_AVG = 0;

LONG CSIntegralOutput = 0;
LONG CSPropOutput = 0;
LONG CSDerivativeOutput = 0;

signed int cs_kp = 2293;	// 0.07 x 32767
signed int cs_ki = 2293;	// 0.07 x 32767
signed int cs_kd = 0;		// 0.25 x 32767

//< -------------------------------- 20121217 new current share method
MyPID CurrentSharePID;
SHORT CurrentShareABC[3];
SHORT CurrentShareHistory[3];
SHORT CurrentShareABC_SS[3];

//--------------------------------->

SHORT gVoutCshareOffset = 0;
SHORT i16CSStableCnt = 0;           //[davidchchen] 20160909 Current Sharing Issue modified

#if RS_ENABLE   //[davidchchen] 20150108 not used
SHORT gVoutRSenseOffset = 0;
#endif


//-------------Extern variable---------------------------------------------------------
extern tPS_FLAG PS;
extern tPSFB PSFB;

//----------------------------------------------------------------------------------
void SRControl ( )
{
  if ( ADC.Iout_FF >= cSR_ON_IOUT_REFH_IM )
  {
      oSR1 = SR_ON;
      oSR2 = SR_ON;
      PS.SR_Enable = 1;
  }
  else if ( PSFB.Counter_InRegulation >= cSR_ON_DELAY && PSFB.SR_ON_Delay >= 100 && ADC.Iout_FF >= cSR_ON_IOUT_REFH )
  {
      oSR1 = SR_ON;
      oSR2 = SR_ON;
      PS.SR_Enable = 1;
  }
  else if ( ADC.Vout_FF < cSR_OFF_VOUT_REF || ADC.Iout_FF < cSR_ON_IOUT_REFL )
  {
      oSR1 = SR_OFF;
      oSR2 = SR_OFF;
      PS.SR_Enable = 0;

      PSFB.SR_ON_Delay = 0;
  }

}

void CS_Control ( void )
{
  static unsigned int cs_pwm_duty = 0;
  static LONG current_cs_pwm_duty = 0;
  static DWORD tcs_pwm = 0;
  static WORD cs_temp = 0;
  WORD Iout_AD_Buf;

  if ( PSFB.CurrentShare == TRUE )
  {
      Iout_AD_Buf = ADC.Iout;

      cs_temp = ( Iout_AD_Buf << 3 ) - Iout_AD_Buf;

      tcs_pwm = __builtin_mulss ( cs_temp, CS_Slope_Temp );
      tcs_pwm = tcs_pwm >> 10;
      current_cs_pwm_duty = ( LONG ) tcs_pwm + ( CS_Offset_Temp >> 1 );

      if ( current_cs_pwm_duty <= 0 || Iout_AD_Buf == 0 )   //[davidchchen]20160829 added
      {
          current_cs_pwm_duty = 0;
          cs_pwm_duty = 0;
      }
      else
      {
          cs_pwm_duty = current_cs_pwm_duty;
      }

      if ( cs_pwm_duty >= SPHASE2 )
      {
          SDC2 = SPHASE2;
      }
      else
      {
          SDC2 = cs_pwm_duty;
      }
  }
  else
  {
      SDC2 = 0;
  }
}

void ResetPID ( )
{
  memset ( CurrentShareHistory, 0, sizeof (CurrentShareHistory ) );

  CurrentSharePID.controlReference = 0;
  CurrentSharePID.measuredOutput = 0;
  CurrentSharePID.controlOutput = 0;
  CurrentSharePID.historyCount = 0;
}


void PIDApp ( MyPID *controller )
{

#if 1
  SHORT Error = 0;
  LONG Sum = 0;
  CHAR N1 = 0;
  CHAR N2 = 0;

  Error = controller->controlReference - controller->measuredOutput;

#if CS_OPTIMIZTION_EN   //[davidchchen] 20161026 Current Sharing optimization Enable
    if ((Error >192) && (i16CSStableCnt == 500))          //[davidchchen] 20160909 Current Sharing Issue modified
      Error = 192;                                        //[davidchchen] 20160909 Current Sharing Issue modified
#endif
                                       
  controller->controlHistory[controller->historyCount] = Error;

  N1 = controller->historyCount - 1;
  N2 = controller->historyCount - 2;

  if ( N1 < 0 ) N1 += 3;
  if ( N2 < 0 ) N2 += 3;

      Sum = ( ( ( LONG ) controller->controlHistory[controller->historyCount] * controller->abcCoefficients[P_ENUM] ) >> 15 )
          + ( ( ( LONG ) controller->controlHistory[N1] * controller->abcCoefficients[I_ENUM] ) >> 15 )
          + ( ( ( LONG ) controller->controlHistory[N2] * controller->abcCoefficients[D_ENUM] ) >> 15 )
          + ( controller->controlOutput );

  controller->historyCount = ( controller->historyCount < 2 ) ? ( controller->historyCount + 1 ) : ( 0 );
  //historyCount = (historyCount < 2)? (historyCount + 1) : (0);

  if ( Sum > Q15_UNSIGN_MAX )
      controller->controlOutput = Q15_UNSIGN_MAX;
  else if ( Sum < Q15_UNSIGN_MIN )
      controller->controlOutput = Q15_UNSIGN_MIN;
  else
      controller->controlOutput = Sum;
#endif	

}

void init_CurrentShareLoop ( void )
{
  memset ( CurrentShareABC, 0, sizeof (CurrentShareABC ) );
  memset ( CurrentShareABC_SS, 0, sizeof (CurrentShareABC_SS ) );
  memset ( CurrentShareHistory, 0, sizeof (CurrentShareHistory ) );

  CurrentSharePID.abcCoefficients = & CurrentShareABC_SS[0];
  CurrentSharePID.controlHistory = & CurrentShareHistory[0];

  PIDApp ( &CurrentSharePID );

#if 0
  CurrentSharePID.abcCoefficients[0] = PID_CURRENTSHARE_A;
  CurrentSharePID.abcCoefficients[1] = PID_CURRENTSHARE_B;
  CurrentSharePID.abcCoefficients[2] = PID_CURRENTSHARE_C;
#endif

  CurrentShareABC_SS[0] = PID_CURRENTSHARE_A_SS;
  CurrentShareABC_SS[1] = PID_CURRENTSHARE_B_SS;
  CurrentShareABC_SS[2] = PID_CURRENTSHARE_C_SS;

  CurrentSharePID.controlReference = 0;
  CurrentSharePID.measuredOutput = 0;
}

void CurrentShareOffsetInit ( )	// [Tommy YR Chen] 20110927 added
{
  static BYTE tflag = FALSE;

  if ( PS.CS_CALIBTATED == TRUE )	// [Tommy YR Chen] 20111103 modified
  {
      if ( tflag == FALSE )
      {
          tflag = TRUE;
          CaliCSIoutOffset = ( ( LONG ) ( ( ( LONGLONG ) UserData.Page1.region.Iout[OUTPUT_12V].Offset * IOUT_TO_ADC ) >> 10 ) );
          CaliCShareOffset = ( ( LONG ) ( ( ( LONGLONG ) UserData.Page1.region.Iout[OUTPUT_12V].CS_Offset * IOUT_TO_ADC ) >> 10 ) );
      }
  }
  else
  {
      tflag = FALSE;
      CaliCSIoutOffset = ( ( LONG ) ( ( ( LONGLONG ) UserData.Page1.region.Iout[OUTPUT_12V].Offset * IOUT_TO_ADC ) >> 10 ) );
      CaliCShareOffset = ( ( LONG ) ( ( ( LONGLONG ) UserData.Page1.region.Iout[OUTPUT_12V].CS_Offset * IOUT_TO_ADC ) >> 10 ) );
  }
}

static void UpdatePIDBuffer ( BYTE* data, BYTE size )
{
  static BYTE count = 0;

  memcpy ( &gPmbusCmd.PID_LOG[count], data, size );

  count += size;
  if ( count >= ( sizeof (gPmbusCmd.PID_LOG ) ) )
  {
      count = 0;
  }
}

void Current_Share ( void )
{
  static SHORT cshare_error = 0;
  static LONG CurrentRef = 0;

  static unsigned int preIoAD = 0;      //[davidchchen] 20160909 Current Sharing Issue modified

  Iout_Real = ADC.CS_PWM_FB_FF;
  CSHARE_AVG = ( ( ( LONG ) ADC.IoutCS_FF * gVCS_Slope ) + gVCS_Offset ) >> 10;

  if ( CSHARE_AVG <= 0 )
  {
      CSHARE_AVG = 0;
  }

  if ( ! PS.I2C_Processing )
  {
      memcpy ( gPmbusCmd.CS_PWM_FB, ( BYTE* ) & ADC.CS_PWM_FB_FF, 2 );
      memcpy ( gPmbusCmd.CS_IN, ( BYTE* ) & ADC.IoutCS_FF, 2 );
  }

  if ( PS.CS_CALIBTATED == TRUE && PSFB.CurrentShare == TRUE )
  {
      CurrentSharePID.controlReference = ( SHORT ) ( CSHARE_AVG << 5 );
      CurrentSharePID.measuredOutput = ( SHORT ) ( Iout_Real << 5 );

      cshare_error = CurrentSharePID.controlReference  - CurrentSharePID.measuredOutput;

#if CS_OPTIMIZTION_EN   //[davidchchen] 20161026 Current Sharing optimization Enable
//***************** //[davidchchen] 20160909 Current Sharing Issue modified   ***********************************
      if(abs(ADC.Iout - preIoAD) <  10)
      {
          if (++i16CSStableCnt > 500)
              i16CSStableCnt = 500;
      }
      else
          i16CSStableCnt = 0;

      if ( cshare_error <= 192 ) // 6*32 = 192, 4*32 = 128, 
      {
              preIoAD = ADC.Iout;

              if(i16CSStableCnt == 500)
              {
                  if(ADC.Iout > I_Error_ADD_H)
                  {
                    CurrentSharePID.controlReference = CurrentSharePID.controlReference - 192;
                  }
              }
              else
              {
                  CurrentSharePID.controlReference = CurrentSharePID.controlReference - 192;
              }
       }

          PIDApp ( &CurrentSharePID );

          CurrentRef = ( __builtin_mulss ( CurrentSharePID.controlOutput, 200 ) >> 15 );

           if ( CurrentRef < 0 )
                CurrentRef = 0;

          gVoutCshareOffset = CurrentRef;

//***************** //[davidchchen] 20160909 Current Sharing Issue modified ***********************************
#else                   //[davidchchen] 20161026 Current Sharing optimization Disable
     if ( cshare_error <= 192 ) // 6*32 = 192
      {
          CurrentSharePID.controlReference = CurrentSharePID.controlReference - 192;
      }

      PIDApp ( &CurrentSharePID );

      if ( CurrentSharePID.controlOutput > 32767 )
            CurrentSharePID.controlOutput = 32767;
      else if ( CurrentSharePID.controlOutput < 0 )
            CurrentSharePID.controlOutput = 0;

      CurrentRef = ( __builtin_muluu ( CurrentSharePID.controlOutput, 800 ) >> 15 );

      if ( CurrentRef > 800 )
            CurrentRef = 800;
      else if ( CurrentRef < - 800 )
            CurrentRef = - 800;

      gVoutCshareOffset = CurrentRef;

      if ( gVoutCshareOffset <= 0 )
      {
          gVoutCshareOffset = 0;
      }
      if ( gVoutCshareOffset >= 200 )
      {
          gVoutCshareOffset = 200;
      }
#endif
      UpdatePIDBuffer ( ( BYTE* ) & CSHARE_AVG, 2 );
      UpdatePIDBuffer ( ( BYTE* ) & Iout_Real, 2 );
      UpdatePIDBuffer ( ( BYTE* ) & CurrentSharePID.controlOutput, 2 );
      UpdatePIDBuffer ( ( BYTE* ) & gVoutCshareOffset, 2 );
  }
  else
  {
      ResetPID ( );
      gVoutCshareOffset = 0;
      CurrentRef = 0;
  }

}

void IoutCali ( )
{

  if ( PS.IOUT1_CALIBRATED )
  {
      LONG temp;
      //ADC.Iout_Cal = (((LONG)ADC.Iout_FF * UserData.Page1.region.Iout[OUTPUT_12V].slope + ((LONG)(((LONGLONG)UserData.Page1.region.Iout[OUTPUT_12V].Offset * IOUT_TO_ADC)>>IOUT_TO_ADC_GAIN)))>>10);
#if 1
      //[davidchchen]20150924 Added no Load, dispaly zero.
      if (ADC.Iout_report_FF == 0)
      {
        temp = 0;
      }
      else
      {
            if ( PS.SR_Enable == 1 )    //[davidchchen]20160713 Added two step Iout Calibration.
            {
                temp = ( ( ( LONG ) ADC.Iout_report_FF * UserData.Page1.region.Iout[OUTPUT_12V].slope + ( ( LONG ) ( ( ( LONGLONG ) UserData.Page1.region.Iout[OUTPUT_12V].Offset * IOUT_TO_ADC ) >> IOUT_TO_ADC_GAIN ) ) ) >> 10 );    //[davidchchen]20160713 Added two step Iout Calibration.
            }
            else if ( PS.SR_Enable == 0 ) //[davidchchen]20160713 Added two step Iout Calibration.
            {
                temp = ( ( ( LONG ) ADC.Iout_report_FF * UserData.Page1.region.Iout[OUTPUT_12V].slope2 + ( ( LONG ) ( ( ( LONGLONG ) UserData.Page1.region.Iout[OUTPUT_12V].Offset2 * IOUT_TO_ADC ) >> IOUT_TO_ADC_GAIN ) ) ) >> 10 );  //[davidchchen]20160713 Added two step Iout Calibration.
            }
      }


      if ( temp <= 0 )
      {
          ADC.Iout_report_Cal = 0;
      }
      else
      {
          ADC.Iout_report_Cal = temp;
      }

      //[davidchchen]20150924 Added no Load, dispaly zero.
      if (ADC.Iout_FF == 0)
      {
          temp = 0;
      }
      else
      {
          if ( PS.SR_Enable == 1 )    //[davidchchen]20160713 Added two step Iout Calibration.
          {
                temp = ( ( ( LONG ) ADC.Iout_FF * UserData.Page1.region.Iout[OUTPUT_12V].slope + ( ( LONG ) ( ( ( LONGLONG ) UserData.Page1.region.Iout[OUTPUT_12V].Offset * IOUT_TO_ADC ) >> IOUT_TO_ADC_GAIN ) ) ) >> 10 );   //[davidchchen]20160713 Added two step Iout Calibration.
          }
          else if ( PS.SR_Enable == 0 ) //[davidchchen]20160713 Added two step Iout Calibration.
          {
                temp = ( ( ( LONG ) ADC.Iout_FF * UserData.Page1.region.Iout[OUTPUT_12V].slope2 + ( ( LONG ) ( ( ( LONGLONG ) UserData.Page1.region.Iout[OUTPUT_12V].Offset2 * IOUT_TO_ADC ) >> IOUT_TO_ADC_GAIN ) ) ) >> 10 ); //[davidchchen]20160713 Added two step Iout Calibration.
          }
      }
      
      if ( temp <= 0 )
      {
          ADC.Iout_Cal = 0;
      }
      else
      {
          ADC.Iout_Cal = temp;
      }

#endif
  }
  else
  {
      ADC.Iout_Cal = ADC.Iout_FF;
#if 1
      ADC.Iout_report_Cal = ADC.Iout_report_FF;
#endif
  }
}

void DummyLoad_Control ( )
{
  if ( ADC.Iout_Cal <=  DUMMY_LOAD_ON_LIMIT )
  {
      oG_DUMMY = HIGH;
      PS.DummyLoadOff = 0;
  }
  else if ( ADC.Iout_Cal >=  DUMMY_LOAD_OFF_LIMIT )	//[davidchchen]20150122 modified
  {
      Protect.DummyLoadOFF.Flag = 1;
      if ( PS.DummyLoadOff == 1 )
      {
          oG_DUMMY = LOW;
      }
  }
}

void OutputControl ( )
{
  //Check Output is in regulation
  if ( ADC.Vout_FF >= cVOUT_REG_REFH && PS.Softstart == FALSE && PSFB.MainEnabled )
  {
      PSFB.InRegulation = TRUE;
  }
  else if ( ADC.Vout_FF <= cVOUT_REG_REFL )
  {
      PSFB.InRegulation = FALSE;
  }

  SRControl ( );

  CurrentShareOffsetInit ( );

  //CS_Control();

  //Current_Share();

  IoutCali ( );

  DummyLoad_Control ( );
}

void Disable_CurrentShare ( )
{
  oSHARE_OFF = TRUE;

  //PS.CS_CATCH = 0;  //[davidchchen] 20150108 removed
  //PS.CS_START = 0;                  //[davidchchen] 20150108 removed
  //Protect.CS_CATCH.Flag = FALSE;    //[davidchchen] 20150108 removed
  //Protect.CS_CATCH.delay = 0;       //[davidchchen] 20150108 removed
  //Protect.CS_START.Flag = 0;        //[davidchchen] 20150108 removed
  //Protect.CS_START.delay = 0;       //[davidchchen] 20150108 removed
  PSFB.CurrentShare = FALSE;
  gVoutCshareOffset = 0;
}

void Enable_CurrentShare ( )
{
  if ( ( PSFB.MainEnabled == TRUE ) && ( PS.SoftstartFinished == TRUE ) )
  {
      oSHARE_OFF = FALSE;

      PSFB.CurrentShare = TRUE;
      //Protect.CS_CATCH.Flag = 1;    //[davidchchen] 20150108 removed
  }
}

