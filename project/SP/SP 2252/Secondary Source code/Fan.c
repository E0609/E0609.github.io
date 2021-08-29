/*****************************************************************************************
 *
 *Copyright (C) 2012 Advanced Technology Development
 *                   Power SBG
 *                   LITE-ON TECHNOLOGY Corp.
 *
 *All Rights Reserved
 *
 *File Name : Fan.c
 *
 *Date : 2012.05.24
 *
 *Author :
 *
 *Description :This Program used for HP 2400W Fan Control Program.
 *
 *******************************************************************************************/
#include "Fan.h"
#include "Isr.h"
#include "Process.h"
#include "I2c.h"
#include "Parameter.h"
#include "Protection.h"

#define FAN_PWM_INVERSED	FALSE

#define FULL_LOAD	200
#define IOUT_LOAD_PERCENT(p) (WORD)(((DWORD)p * FULL_LOAD / 100 * IOUT_TO_ADC) >> 10)
#define IOUT_7A ((WORD)7*IOUT_TO_ADC) >> 10
#define IOUT_10A ((WORD)10*IOUT_TO_ADC) >> 10

#define GetDuty(rpm) ((DWORD)rpm * FAN_PWM_PERIOD) / FAN_MAX_RPM
#define SetDuty(percent) ((DWORD)percent * FAN_PWM_PERIOD) / 100

#if 0
WORD FanDuty_Table[3][6] = {
    {SetDuty ( 6 ), SetDuty ( 6 ), SetDuty ( 13 ), SetDuty ( 23 ), SetDuty ( 40 ), SetDuty ( 68 ) },	// 25C
    {SetDuty ( 16 ), SetDuty ( 11 ), SetDuty ( 16 ), SetDuty ( 28 ), SetDuty ( 48 ), SetDuty ( 80 ) },	// 35C
    {SetDuty ( 19 ), SetDuty ( 14 ), SetDuty ( 23 ), SetDuty ( 43 ), SetDuty ( 71 ), SetDuty ( 100 ) },	// 55C
};
#else
WORD FanDuty_Table[3][10] = {
//    {SetDuty ( 30 ), SetDuty ( 30 ), SetDuty ( 30 ), SetDuty ( 34 ), SetDuty ( 36 ), SetDuty ( 41 ), SetDuty ( 50 ), SetDuty ( 58 ), SetDuty ( 69 ), SetDuty ( 80 ) },	// 25C
//    {SetDuty ( 30 ), SetDuty ( 30 ), SetDuty ( 30 ), SetDuty ( 36 ), SetDuty ( 47 ), SetDuty ( 58 ), SetDuty ( 70 ), SetDuty ( 81 ), SetDuty ( 91 ), SetDuty ( 100 ) },	// 35C
//    {SetDuty ( 30 ), SetDuty ( 34 ), SetDuty ( 41 ), SetDuty ( 49 ), SetDuty ( 58 ), SetDuty ( 68 ), SetDuty ( 78 ), SetDuty ( 92 ), SetDuty ( 100 ), SetDuty ( 100 ) },	// 55C

    //[davidchchen]20161229 when Tinlet>40, 15%,25%,35%load fan duty changed.
    #if ConfigDD_SlaveSW
    {SetDuty ( 50 ), SetDuty ( 15 ), SetDuty ( 30 ), SetDuty ( 34 ), SetDuty ( 36 ), SetDuty ( 41 ), SetDuty ( 50 ), SetDuty ( 58 ), SetDuty ( 69 ), SetDuty ( 80 ) },	// 25C
    {SetDuty ( 50 ), SetDuty ( 26 ), SetDuty ( 30 ), SetDuty ( 36 ), SetDuty ( 47 ), SetDuty ( 58 ), SetDuty ( 70 ), SetDuty ( 81 ), SetDuty ( 91 ), SetDuty ( 100 ) },	// 35C
    {SetDuty ( 75 ), SetDuty ( 25 ), SetDuty ( 30 ), SetDuty ( 38 ), SetDuty ( 58 ), SetDuty ( 68 ), SetDuty ( 78 ), SetDuty ( 92 ), SetDuty ( 100 ), SetDuty ( 100 ) },	// 55C
    #else
    {SetDuty ( 30 ), SetDuty ( 30 ), SetDuty ( 30 ), SetDuty ( 34 ), SetDuty ( 36 ), SetDuty ( 41 ), SetDuty ( 50 ), SetDuty ( 58 ), SetDuty ( 69 ), SetDuty ( 80 ) },	// 25C
    {SetDuty ( 30 ), SetDuty ( 30 ), SetDuty ( 30 ), SetDuty ( 36 ), SetDuty ( 47 ), SetDuty ( 58 ), SetDuty ( 70 ), SetDuty ( 81 ), SetDuty ( 91 ), SetDuty ( 100 ) },	// 35C
    {SetDuty ( 34 ), SetDuty ( 34 ), SetDuty ( 41 ), SetDuty ( 49 ), SetDuty ( 58 ), SetDuty ( 68 ), SetDuty ( 78 ), SetDuty ( 92 ), SetDuty ( 100 ), SetDuty ( 100 ) },	// 55C
    #endif
};

WORD FanDuty_Table2[3][10] = {
//    {SetDuty ( 30 ), SetDuty ( 30 ), SetDuty ( 30 ), SetDuty ( 34 ), SetDuty ( 36 ), SetDuty ( 41 ), SetDuty ( 50 ), SetDuty ( 58 ), SetDuty ( 69 ), SetDuty ( 80 ) },	// 25C
//    {SetDuty ( 30 ), SetDuty ( 30 ), SetDuty ( 30 ), SetDuty ( 36 ), SetDuty ( 47 ), SetDuty ( 58 ), SetDuty ( 70 ), SetDuty ( 81 ), SetDuty ( 91 ), SetDuty ( 100 ) },	// 35C
//    {SetDuty ( 30 ), SetDuty ( 34 ), SetDuty ( 41 ), SetDuty ( 49 ), SetDuty ( 58 ), SetDuty ( 68 ), SetDuty ( 78 ), SetDuty ( 92 ), SetDuty ( 100 ), SetDuty ( 100 ) },	// 55C

    //[davidchchen]20150731 Added
    #if ConfigDD_SlaveSW
    {SetDuty ( 75 ), SetDuty ( 30 ), SetDuty ( 30 ), SetDuty ( 34 ), SetDuty ( 36 ), SetDuty ( 41 ), SetDuty ( 50 ), SetDuty ( 58 ), SetDuty ( 69 ), SetDuty ( 80 ) },	// 25C
    {SetDuty ( 75 ), SetDuty ( 30 ), SetDuty ( 30 ), SetDuty ( 36 ), SetDuty ( 47 ), SetDuty ( 58 ), SetDuty ( 70 ), SetDuty ( 81 ), SetDuty ( 91 ), SetDuty ( 100 ) },	// 35C
    {SetDuty ( 75 ), SetDuty ( 34 ), SetDuty ( 41 ), SetDuty ( 49 ), SetDuty ( 58 ), SetDuty ( 68 ), SetDuty ( 78 ), SetDuty ( 92 ), SetDuty ( 100 ), SetDuty ( 100 ) },	// 55C
    #else
    {SetDuty ( 30 ), SetDuty ( 30 ), SetDuty ( 30 ), SetDuty ( 34 ), SetDuty ( 36 ), SetDuty ( 41 ), SetDuty ( 50 ), SetDuty ( 58 ), SetDuty ( 69 ), SetDuty ( 80 ) },	// 25C
    {SetDuty ( 30 ), SetDuty ( 30 ), SetDuty ( 30 ), SetDuty ( 36 ), SetDuty ( 47 ), SetDuty ( 58 ), SetDuty ( 70 ), SetDuty ( 81 ), SetDuty ( 91 ), SetDuty ( 100 ) },	// 35C
    {SetDuty ( 34 ), SetDuty ( 34 ), SetDuty ( 41 ), SetDuty ( 49 ), SetDuty ( 58 ), SetDuty ( 68 ), SetDuty ( 78 ), SetDuty ( 92 ), SetDuty ( 100 ), SetDuty ( 100 ) },	// 55C
    #endif
};

#endif

//--------------- Global variable -------------------------------------------------------------
unsigned int Duty_Gain = 0;
tFAN_CNTL gFan1;
tFAN_CNTL gFan2;

//--------------- Extern variable -------------------------------------------------------------
extern tPSU_STATE gPS_State;
extern tPS_FLAG PS;

//---------------Function declare--------------------------------------------------------------

void SetFanDuty ( WORD duty );

//-------------------------------------------------------------------------------------------

void init_Fan ( )
{
  //Fan A Speed Monitor
  IC1CON = 0;
  IC1CON = 0x0023;				//Time3 contents are captured on capture event, Interrupt on every capture event, every 2nd rising edge
  IFS0bits.IC1IF = 0;
  IPC0bits.IC1IP = 2;
  IEC0bits.IC1IE = 1;

  //Fan A Control
  IOCON4bits.PENL = 1;			//PWM module controls PWMxL pin
  IOCON4bits.PMOD = 3;			//PWM I/O pin pair is in the True Independent Output mode
  PWMCON4bits.DTC = 2;			//Dead time function is disabled
  PWMCON4bits.ITB = 1;			//PHASEx/SPHASEx registers provide time base period for this PWM generator
  FCLCON4bits.FLTMOD = 3;
  SPHASE4 = FAN_PWM_PERIOD;
  //SetFanDuty(FAN_PWM_DEFAULT_DUTY);
  SetFanDuty ( 0 );
}
//-------------------------------------------------------------------------------------------

void CheckICAOV ( )
{
  WORD tempbuf;

  if ( IC1CONbits.ICOV == 1 )
  {
      tempbuf = IC1BUF;
      tempbuf = IC1BUF;
      tempbuf = IC1BUF;
      tempbuf = IC1BUF;
  }
}

/*********************************************************************************************
------- Corresponding timer 2 is 200 us
------- Input Capture moudule will capture the counter every 4th raising edge to ICxBuf
------- 1sec -> 5000 count of 200us, 60sec -> 300000 count of 200us
------- Since the time taken by every 4 raising edge represent the time spent of every 2 rpm
------- So that, we should divide the time of (Rpm_Capture2 - Rpm_Capture1) by 2 
------- for caculating how much time does it take every 1 rpm
 *********************************************************************************************/
void UpdateFanSpeedA ( )
{
  WORD Rpm_Capture1, Rpm_Capture2, ErrCount;
  WORD Fan_Speed;
  DWORD Fan_period;

  Rpm_Capture1 = IC1BUF;
  Rpm_Capture2 = IC1BUF;

  if ( Rpm_Capture2 > Rpm_Capture1 )
  {
      ErrCount = Rpm_Capture2 - Rpm_Capture1;
  }
  else
  {
      ErrCount = 65535 - Rpm_Capture1 + Rpm_Capture2;
  }

  Fan_period = ErrCount << 1;

  Fan_Speed = ( WORD ) ( ( QWORD ) 4687500 * 2 / Fan_period );	//Timer 3 prescaler is 1:128
  gFan1.CurrentRpm = Fan_Speed;

  CheckICAOV ( );
}

//--------------------------- FanA Speed Update ---------------------------------------------

void __attribute__ ( ( __interrupt__, no_auto_psv ) ) _IC1Interrupt ( )
{
  gFan1.Detected = TRUE;

  UpdateFanSpeedA ( );

  IFS0bits.IC1IF = 0;
}

//--------------------------- SetFanDuty  -------------------------------------------------

void SetFanDuty ( WORD duty )
{
#if FAN_PWM_INVERSED
  SDC4 = ( ( WORD ) FAN_PWM_PERIOD - duty );
#else
  SDC4 = duty;
#endif
}

//--------------------------- FanSoftAdjust  -------------------------------------------------

void FanSoftAdjust ( )
{
  if ( PS.FanDutyCtrl )
  {
      PS.FanDutyCtrl = 0;
      if ( SDC4 < gFan1.TargetDuty )
      {
          SDC4 = SDC4 + 5;
      }
      else if ( SDC4 > gFan1.TargetDuty )
      {
          SDC4 = SDC4 - 1;
      }
      else
      {
          // Do nothing
      }
  }
  else
  {
      Protect.FanDutyCtrl.Flag = 1;
  }
}

//----------------------------- LoadHysteresis ----- --------------------------------------
BYTE    LoadHysteresis()//[davidchchen]20170707 Added Fan Issue modify
{
    WORD Adc_Io = ADC.Iout_Cal;
    static BYTE LoadCounter = 0;

    if ( Adc_Io < IOUT_LOAD_PERCENT ( 15 ) )                                                        //[davidchchen]20170707 Added Fan Issue modify
    {
        if ( ( Adc_Io > IOUT_LOAD_PERCENT ( 12 ) ) && ( LoadCounter == 1 ) )       //load_cnt = 0+1,  //[davidchchen]20170707 Added Fan Issue modify
            return  LoadCounter;
        LoadCounter = 0;
    }
    else if ( ( Adc_Io >= IOUT_LOAD_PERCENT ( 15 ) ) && ( Adc_Io < IOUT_LOAD_PERCENT ( 25 ) ) )     //[davidchchen]20170707 Added Fan Issue modify
    {
        if ( ( Adc_Io > IOUT_LOAD_PERCENT ( 22 ) ) && ( LoadCounter == 2 ) )       //load_cnt = 1+1,  //[davidchchen]20170707 Added Fan Issue modify
            return  LoadCounter;
        LoadCounter = 1;
    }
    else if ( ( Adc_Io >= IOUT_LOAD_PERCENT ( 25 ) ) && ( Adc_Io < IOUT_LOAD_PERCENT ( 35 ) ) )     //[davidchchen]20170707 Added Fan Issue modify
    {
        if ( ( Adc_Io > IOUT_LOAD_PERCENT ( 32 ) ) && ( LoadCounter == 3 ) )       //load_cnt = 2+1,  //[davidchchen]20170707 Added Fan Issue modify
            return  LoadCounter;
        LoadCounter = 2;
    }
    else if ( ( Adc_Io >= IOUT_LOAD_PERCENT ( 35 ) ) && ( Adc_Io < IOUT_LOAD_PERCENT ( 45 ) ) )     //[davidchchen]20170707 Added Fan Issue modify
    {
        if ( ( Adc_Io > IOUT_LOAD_PERCENT ( 42 ) ) && ( LoadCounter == 4 ) )       //load_cnt = 3+1,  //[davidchchen]20170707 Added Fan Issue modify
            return  LoadCounter;
        LoadCounter = 3;
    }
    else if ( ( Adc_Io >= IOUT_LOAD_PERCENT ( 45 ) ) && ( Adc_Io < IOUT_LOAD_PERCENT ( 55 ) ) )     //[davidchchen]20170707 Added Fan Issue modify
    {
        if ( ( Adc_Io > IOUT_LOAD_PERCENT ( 52 ) ) && ( LoadCounter == 5 ) )       //load_cnt = 4+1,  //[davidchchen]20170707 Added Fan Issue modify
            return  LoadCounter;
        LoadCounter = 4;
    }
    else if ( ( Adc_Io >= IOUT_LOAD_PERCENT ( 55 ) ) && ( Adc_Io < IOUT_LOAD_PERCENT ( 65 ) ) )     //[davidchchen]20170707 Added Fan Issue modify
    {
        if ( ( Adc_Io > IOUT_LOAD_PERCENT ( 62 ) ) && ( LoadCounter == 6 ) )       //load_cnt = 5+1,  //[davidchchen]20170707 Added Fan Issue modify
            return  LoadCounter;
        LoadCounter = 5;
    }
    else if ( ( Adc_Io >= IOUT_LOAD_PERCENT ( 65 ) ) && ( Adc_Io < IOUT_LOAD_PERCENT ( 75 ) ) )     //[davidchchen]20170707 Added Fan Issue modify
    {
        if ( ( Adc_Io > IOUT_LOAD_PERCENT ( 72 ) ) && ( LoadCounter == 7 ) )       //load_cnt = 6+1,  //[davidchchen]20170707 Added Fan Issue modify
            return  LoadCounter;
        LoadCounter = 6;
    }
    else if ( ( Adc_Io >= IOUT_LOAD_PERCENT ( 75 ) ) && ( Adc_Io < IOUT_LOAD_PERCENT ( 85 ) ) )     //[davidchchen]20170707 Added Fan Issue modify
    {
        if ( ( Adc_Io > IOUT_LOAD_PERCENT ( 82 ) ) && ( LoadCounter == 8 ) )       //load_cnt = 7+1,  //[davidchchen]20170707 Added Fan Issue modify
            return  LoadCounter;
        LoadCounter = 7;
    }
    else if ( ( Adc_Io >= IOUT_LOAD_PERCENT ( 85 ) ) && ( Adc_Io < IOUT_LOAD_PERCENT ( 95 ) ) )     //[davidchchen]20170707 Added Fan Issue modify
    {
        if ( ( Adc_Io > IOUT_LOAD_PERCENT ( 92 ) ) && ( LoadCounter == 9 ) )       //load_cnt = 8+1,  //[davidchchen]20170707 Added Fan Issue modify
            return  LoadCounter;
        LoadCounter = 8;
    }
    else if ( Adc_Io >= IOUT_LOAD_PERCENT ( 95 ) )
    {
      LoadCounter = 9;
    }

    return  LoadCounter;

}

//----------------------------- TempHysteresis ----- --------------------------------------
BYTE    TempHysteresis()//[davidchchen]20170707 Added Fan Issue modify
{
    static BYTE TempCounter = 0;
    //Give a default temp_cnt

    if ( T_Inlet < 30 )
    {
        if ( ( T_Inlet > 27 ) && ( TempCounter == 1 ) )       //temp_cnt = 0+1,  //[davidchchen]20170707 Added Fan Issue modify
            return  TempCounter;
      TempCounter = 0;
    }
    else if ( T_Inlet >= 30 && T_Inlet < 40 )                                   //[davidchchen]20170707 Added Fan Issue modify
    {
        if ( ( T_Inlet > 37 ) && ( TempCounter == 2 ) )       //temp_cnt = 1+1,  //[davidchchen]20170707 Added Fan Issue modify
            return  TempCounter;
        TempCounter = 1;
    }
    else if ( T_Inlet >= 40 )
    {
        TempCounter = 2;
    }

    return  TempCounter;
}

//----------------------------- GetFanMinDuty ----- --------------------------------------
WORD GetFanMinDuty ( )
{
  //WORD Adc_Io = ADC.Iout_Cal;         //[davidchchen]20170707 Removed
  static BYTE load_cnt = 0;
  static BYTE temp_cnt = 0;
  static WORD minduty = SetDuty ( 75 );
  //WORD minduty = SetDuty ( 30 );  //[davidchchen]20151223 Added

#if 0        //[davidchchen]20170707 Removed
  if ( Adc_Io < IOUT_LOAD_PERCENT ( 12 ) )
  {
      load_cnt = 0;
  }
  else if ( ( Adc_Io >= IOUT_LOAD_PERCENT ( 15 ) ) && ( Adc_Io < IOUT_LOAD_PERCENT ( 22 ) ) )
  {
      load_cnt = 1;
  }
  else if ( ( Adc_Io >= IOUT_LOAD_PERCENT ( 25 ) ) && ( Adc_Io < IOUT_LOAD_PERCENT ( 32 ) ) )
  {
      load_cnt = 2;
  }
  else if ( ( Adc_Io >= IOUT_LOAD_PERCENT ( 35 ) ) && ( Adc_Io < IOUT_LOAD_PERCENT ( 42 ) ) )
  {
      load_cnt = 3;
  }
  else if ( ( Adc_Io >= IOUT_LOAD_PERCENT ( 45 ) ) && ( Adc_Io < IOUT_LOAD_PERCENT ( 52 ) ) )
  {
      load_cnt = 4;
  }
  else if ( ( Adc_Io >= IOUT_LOAD_PERCENT ( 55 ) ) && ( Adc_Io < IOUT_LOAD_PERCENT ( 62 ) ) )
  {
      load_cnt = 5;
  }
  else if ( ( Adc_Io >= IOUT_LOAD_PERCENT ( 65 ) ) && ( Adc_Io < IOUT_LOAD_PERCENT ( 72 ) ) )
  {
      load_cnt = 6;
  }
  else if ( ( Adc_Io >= IOUT_LOAD_PERCENT ( 75 ) ) && ( Adc_Io < IOUT_LOAD_PERCENT ( 82 ) ) )
  {
      load_cnt = 7;
  }
  else if ( ( Adc_Io >= IOUT_LOAD_PERCENT ( 85 ) ) && ( Adc_Io < IOUT_LOAD_PERCENT ( 92 ) ) )
  {
      load_cnt = 8;
  }
  else if ( Adc_Io >= IOUT_LOAD_PERCENT ( 95 ) )
  {
      load_cnt = 9;
  }
  
  //Give a default temp_cnt
  if ( T_Inlet <= 27 )
  {
      temp_cnt = 0;
  }
  else if ( T_Inlet > 30 && T_Inlet <= 37 )
  {
      temp_cnt = 1;
  }
  else if ( T_Inlet >= 40 )
  {
      temp_cnt = 2;
  }
#else           //[davidchchen]20170707 Added Fan Issue modify

    load_cnt = LoadHysteresis();
    temp_cnt = TempHysteresis();
#endif

    //[davidchchen]20150731 Added
    #if ConfigDD_SlaveSW
/*
    //if ( T_Pri < 36 && T_Sec < 38 )
    if ( T_Sec < 50 )
    {
        minduty = FanDuty_Table[temp_cnt][load_cnt];  //[davidchchen]20151223 Added
    }
    //else if ( T_Pri >= 38 || T_Sec >= 40 )
    else if ( T_Sec >= 53 )
    {
        minduty = FanDuty_Table2[temp_cnt][load_cnt];  //[davidchchen]20151223 Added
    }
*/
	minduty = FanDuty_Table[temp_cnt][load_cnt];  //[davidchchen]20151223 Added

    #else
    if ( Adc_Io <= IOUT_7A )
    {
      minduty = SetDuty ( 75 );
    }
    else if ( Adc_Io >= IOUT_10A )
    {
      minduty = FanDuty_Table[temp_cnt][load_cnt];
    }
    #endif
   
  return minduty;

}

//----------------------------- Fan_MinimumDuty_Modify ----- ------------------------------

void Fan_MinimumDuty_Modify ( void )
{
  WORD minDuty;

  //[Peter Chung] 20101208 modified
  minDuty = GetFanMinDuty ( );

  //[Peter Chung] 20101101 added for Fan override behavior
  gFan1.minDuty = ( minDuty > gFan1.OverridedDuty ) ? minDuty : gFan1.OverridedDuty;
}

//----------------------------- ConvertRatioToDuty  ------------------------------------

static WORD ConvertRatioToDuty ( BYTE ratio )
{
  WORD result;

  //mapping 0 - 100% to duty cycle 0 - 19000
  result = ( WORD ) ( ( DWORD ) FAN_PWM_PERIOD * ratio / 100 );

  return result;
}

//----------------------------- CalcFanTargetSpeed  ------------------------------------

void CalcFanTargetSpeed ( BYTE ratio )
{
  WORD minDuty;
  WORD overrideDuty;
  WORD target;

  minDuty = GetFanMinDuty ( );
  overrideDuty = ConvertRatioToDuty ( ratio );
  target = minDuty;

  if ( overrideDuty > minDuty )
  {
      //Fan Override
      target = overrideDuty;
      SetFanDuty ( overrideDuty );
      gFan1.TargetDuty = target;
  }
  else
  {
      SetFanDuty ( minDuty );
      gFan1.TargetDuty = target;
  }

  gFan1.OverridedDuty = overrideDuty;	//[Peter Chung] 20110118 modified
}

//--------------------------- Fan Duty Control By Load --------------------------------------

void UpdateFanDuty ( )
{
  if ( PS.StartCalibrate )
  {
      gFan1.TargetDuty = ( ( DWORD ) FAN_PWM_PERIOD * 50 ) / 100;
      SetFanDuty ( gFan1.TargetDuty );
      return;
  }

#if 0
  if ( gFan1.OverridedDuty != 0 )
  {
      gFan1.TargetDuty = gFan1.OverridedDuty;
      SetFanDuty ( gFan1.TargetDuty );
  }
  else
#endif
  {
      if ( gPS_State.mainState == STATE_NORMAL )
      {
          if ( T_Sec > FAN_ADJ_SEC_TEMP_H || T_Pri > FAN_ADJ_PRI_TEMP_H )
          {			// [Tommy YR Chen] 20100916 added
              //increse fan duty if py or sy over temperature
              Protect.T_FanControl.Flag = 1;
              if ( PS.FanControl )
              {
                  gFan1.TargetDuty += 175;

                  if ( gFan1.TargetDuty >= FAN_PWM_PERIOD )
                  {
                      gFan1.TargetDuty = FAN_PWM_PERIOD;
                  }
                  PS.FanControl = 0;
              }
          }
          else if ( T_Sec < FAN_ADJ_SEC_TEMP_L && T_Pri < FAN_ADJ_PRI_TEMP_L )
          {	// [Tommy YR Chen] 20100916 added
              //decrese fan duty if py and sy under temperature
              Protect.T_FanControl.Flag = 1;
              if ( PS.FanControl )
              {
                  if ( gFan1.TargetDuty >= 100 )
                  {
                      gFan1.TargetDuty -= 100;
                  }
                  else
                  {
                      gFan1.TargetDuty = 0;
                  }
                  //min limit
                  Fan_MinimumDuty_Modify ( );

                  if ( gFan1.TargetDuty < gFan1.minDuty )
                  {
                      gFan1.TargetDuty = gFan1.minDuty;
                  }

                  PS.FanControl = 0;
              }
          }
          else
          {
              Protect.T_FanControl.Flag = 0;
              Protect.T_FanControl.delay = 0;
              //Fan_MinimumDuty_Modify();
              //gFan1.TargetDuty = gFan1.minDuty;
              if ( ADC.Iout_Cal <= IOUT_7A )
              {
                  gFan1.TargetDuty = SetDuty ( 75 );
                  if ( gFan1.OverridedDuty >= gFan1.TargetDuty )
                  {
                      gFan1.TargetDuty = gFan1.OverridedDuty;
                  }
                  SetFanDuty ( gFan1.TargetDuty );
              }
          }
      }
      else if ( ( gPS_State.mainState == STATE_STANDBY ) || ( gPS_State.mainState == STATE_LATCH ) )
      {
          gFan1.TargetDuty = ( ( DWORD ) FAN_PWM_PERIOD * 15 ) / 100;
      }
      else if ( gPS_State.mainState == STATE_ONING )
      {
          if ( T_Sec > 60 || T_Pri > 75 )
          {
              gFan1.TargetDuty = FAN_PWM_PERIOD;
              SetFanDuty ( gFan1.TargetDuty );
          }
          else
          {
              gFan1.TargetDuty = FAN_PWM_DEFAULT_DUTY;
          }

          //SetFanDuty(gFan1.TargetDuty);
      }

      FanSoftAdjust ( );
  }
}

//--------------------------- DisableFan ----------------------------------------------------------

void DisableFan ( )
{
  PS.FanLock_Disable = TRUE;
  oFAN_CNTL = Fan_Disable;
  //oDummy_CNTL = Fan_Disable;
}

//--------------------------- EnableFan ----------------------------------------------------------

void EnableFan ( )
{
  PS.FanLock_Disable = FALSE;
  oFAN_CNTL = Fan_Enable;
  //oDummy_CNTL = Fan_Enable;
}



