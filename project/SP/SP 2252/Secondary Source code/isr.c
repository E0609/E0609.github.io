/*****************************************************************************************
 *
 *Copyright (C) 2012 Advanced Technology Development
 *                   Power SBG
 *                   LITE-ON TECHNOLOGY Corp.
 *
 *All Rights Reserved
 *
 *File Name : ISR.c
 *
 *Date : 2012.05.24
 *
 *Author :
 *
 *Description :This Program used for HP 2400W ISR Program.
 *
 *******************************************************************************************/
#include "p33Fxxxx.h"
#include "Isr.h"
#include "Protection.h"
#include "Process.h"
#include "parameter.h"
#include "define.h"
#include "PowerOnOff.h"
#include "Init.h"
#include "I2c.h"
#include "Standby.h"
#include "PowerOnOff.h"
#include "Pmbus.h"
#include "Standby.h"
#include "Userdata.h"
#include "Psfb.h"
#include "Fan.h"
#include "Led.h"
// -------------Global Variable --------------------------------------------
tADC ADC;
tVref Vref;

// -------------Extern Variable --------------------------------------------
extern BYTE gPSU_ONOFF_Status;
extern tPSU_STATE gPS_State;
extern tTimerHandle hTimer;
extern tPS_FLAG PS;
extern tPSFB PSFB;
extern WORD Tsb_off_delay;

extern BYTE Tsb_off_delay_Flag;
// -----------------------------------------------------------------------

extern void ControlVref ( );

// -----------------------------------------------------------------------

void __attribute__ ( ( __interrupt__, no_auto_psv ) ) _ADCP0Interrupt ( )
{
  /*
  AN0 : CSHARE_IN
  AN1 : CS_PWM_FB
   */

  ADC.IoutCS = ADCBUF0;
  ADC.CS_PWM_FB = ADCBUF1;

  ADC.IoutCS_LPF = ADC.IoutCS_LPF + ADC.IoutCS - ADC.IoutCS_FF;
  ADC.IoutCS_FF = ADC.IoutCS_LPF >> 7;	// 2

  ADC.CS_PWM_FB_LPF = ADC.CS_PWM_FB_LPF + ADC.CS_PWM_FB - ADC.CS_PWM_FB_FF;
  ADC.CS_PWM_FB_FF = ADC.CS_PWM_FB_LPF >> 7;

  {	//New Current Share Scheme
      static BYTE count = 0;
      if ( count >= 10 )
      {	// 200us
          CS_Control ( );
          Current_Share ( );
          ControlVref ( );
          count = 0;
      }
      else
      {
          count ++;
      }
  }

  IFS6bits.ADCP0IF = 0;

}
// -----------------------------------------------------------------------

void __attribute__ ( ( __interrupt__, no_auto_psv ) ) _ADCP1Interrupt ( )
{
  /*
  AN2 : 12V_DET
  AN3 : 12VSB_DET1
   */

  SHORT result;

  ADC.Vout = ADCBUF2;
  ADC.StbVout = ADCBUF3;

  ADC.Vout_LPF = ADC.Vout_LPF + ADC.Vout - ADC.Vout_FF;
  ADC.Vout_FF = ADC.Vout_LPF >> 9;

  ADC.StbVout_LPF = ADC.StbVout_LPF + ADC.StbVout - ADC.StbVout_FF;
  ADC.StbVout_FF = ADC.StbVout_LPF >> 2;

  //SB SCP
#if STB_SC_PROTECT
  Check_STBSCP ( );
#endif

  //SB OVP
#if STB_OV_PROTECT
  Check_STBOVP ( );
#endif

  //OVP
#if OV_PROTECT
  result = ( SHORT ) ADC.Vout + ( SHORT ) gVoutReadOffsetAdc;
  if ( result < 0 )
  {
      result = 0;
  }

  if ( result > Parameter.OVP_VOUT_OV_WARN_LIMIT )
  {
      gPagePlusStatus.PAGE[PAGE0].STATUS_VOUT.bits.VOUT_OV_WARNING = 1;
      gLedWarningStatus.bits.vout_ov_warning = 1;
  }
  else
  {
      gLedWarningStatus.bits.vout_ov_warning = 0;
  }

  //FW V42 source code
  //if ( ! _SD_Flag.Val )
  
  //david add
  if ( PSFB.MainEnabled == TRUE )
  {
      if ( result > Parameter.OVP_VOUT_OV_FAULT_LIMIT )
      {
          CaptureFault ( );
          EmergencyPowerOff ( );
          _SD_Flag.OVP_SD = 1;
          _SD_Flag.LATCH_SD = 1;
          gPagePlusStatus.PAGE[PAGE0].STATUS_VOUT.bits.VOUT_OV_FAULT = 1;
      }
  }
#endif

  IFS6bits.ADCP1IF = 0;

}
// -----------------------------------------------------------------------

void __attribute__ ( ( __interrupt__, no_auto_psv ) ) _ADCP2Interrupt ( )
{
  /*
  AN4 : T_INLET
  AN5 : T_SEC_2
   */

  ADC.Tinlet = ADCBUF4;
  ADC.Tsec = ADCBUF5;

  ADC.Tinlet_LPF = ADC.Tinlet_LPF + ADC.Tinlet - ADC.Tinlet_FF;
  ADC.Tinlet_FF = ADC.Tinlet_LPF >> 5;

  ADC.Tsec_LPF = ADC.Tsec_LPF + ADC.Tsec - ADC.Tsec_FF;
  ADC.Tsec_FF = ADC.Tsec_LPF >> 5;

  //------------------------- Main output Softstart--------------------------
  {
      static WORD cnt = 0;

      if ( cnt >= SOFTSTART_SLOPE_ADJUSTMENT )
      {
          cnt = 0;
          if ( PS.Softstart )
          {
              if ( Vref.SoftstartVoltage < 16581 )
              {
                  Vref.SoftstartVoltage += 256;
              }
              else
              {
                  if ( Vref.SoftstartVoltage < 22108 )
                  {
                      Vref.SoftstartVoltage += 128;
                  }
                  else if ( Vref.SoftstartVoltage < Vref.SetVoltage )
                  {
                      Vref.SoftstartVoltage += 40;
                      if ( Vref.SoftstartVoltage >= Vref.SetVoltage )
                      {
                          Vref.SoftstartVoltage = Vref.SetVoltage;
                      }
                  }
              }
          }
          else
          {
              Vref.SoftstartVoltage = 0;
          }
      }
      else
      {
          cnt ++;
      }
  }
  // -----------------------------------------------------------------------

  IFS7bits.ADCP2IF = 0;
}

// -----------------------------------------------------------------------

void __attribute__ ( ( __interrupt__, no_auto_psv ) ) _ADCP4Interrupt ( )
{
  /*
  AN9 : 12VSB_IOUT1
   */

  ADC.StbIout = ADCBUF9;

  ADC.StbIout_LPF = ADC.StbIout_LPF + ADC.StbIout - ADC.StbIout_FF;
  ADC.StbIout_FF = ADC.StbIout_LPF >> 2;

  if ( ( iPFC_STB == PFC_STB_NOK ) ||
       ( Tsb_off_delay > 200 ) )
  {
      //PFC voltage is under 260V, disable standby
      if (Tsb_off_delay_Flag == 0 )
      {
        DisableSTBoutput ( );
        Tsb_off_delay_Flag = 1;

      }
      
  }

  IFS7bits.ADCP4IF = 0;

}
// -----------------------------------------------------------------------

void __attribute__ ( ( __interrupt__, no_auto_psv ) ) _ADCP5Interrupt ( )
{
  /*
  AN10 : 12V_IOUT_DET
  AN11 : 12VBUS_DET
   */

  ADC.Iout = ADCBUF10;
  ADC.VoutBus = ADCBUF11;

  ADC.VoutBus_LPF = ADC.VoutBus_LPF + ADC.VoutBus - ADC.VoutBus_FF;
  ADC.VoutBus_FF = ADC.VoutBus_LPF >> 9;

  ADC.Iout_LPF = ADC.Iout_LPF + ADC.Iout - ADC.Iout_FF;
  ADC.Iout_FF = ADC.Iout_LPF >> 7;	// 2

  ADC.Iout_report_LPF = ADC.Iout_report_LPF + ADC.Iout - ADC.Iout_report_FF;
  ADC.Iout_report_FF = ADC.Iout_report_LPF >> 13;

#if 0
  ADC.Iout_report_LPF = ADC.Iout_report_LPF + ADC.Iout_report - ADC.Iout_report_FF;
  ADC.Iout_report_FF = ADC.Iout_report_LPF >> 10;
#endif

  //SCP
#if SC_PROTECT
#if 0
  if ( ( ADC.Vout < VOUT_1V && PS.Softstart == TRUE && PSFB.MainEnabled && PS.SC_CheckStart && ADC.Iout > Parameter.SCP_FAULT_LIMIT ) ||
       ( ADC.Iout > Parameter.SCP_FAULT_LIMIT && ADC.Vout < VOUT_2V && PS.Softstart == FALSE && PSFB.MainEnabled )
       )
  {

      if ( ! _SD_Flag.Val )
      {
          EmergencyPowerOff ( );
          _SD_Flag.SCP_SD = 1;
          _SD_Flag.LATCH_SD = 1;
          //[Peter Chung] 20101209 added
          gPagePlusStatus.PAGE[PAGE0].STATUS_IOUT.bits.IOUT_OC_WARNING = 1;
          gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.bits.IOUT_OC_WARNING = 1;
          gPagePlusStatus.PAGE[PAGE0].STATUS_IOUT.bits.IOUT_OC_FAULT = 1;
          gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.bits.IOUT_OC_FAULT = 1;
          SaveToBlackBox ( );
          PS.SC_CheckStart = 0;
      }
  }
#endif

  if ( iPS_OK == P_N_OK )
  {
      if ( ADC.Vout < VOUT_1V /*&& PS.Softstart == TRUE*/ && PSFB.MainEnabled /*&& PS.SC_CheckStart*/ && ADC.Iout > Parameter.SCP_FAULT_LIMIT )
      {
          Protect.SCP.Flag = 1;
      }
      else
      {
          Protect.SCP.Flag = 0;
          Protect.SCP.delay = 0;
      }
  }
  else
  {
      if ( ADC.Iout > Parameter.SCP_FAULT_LIMIT && ADC.Vout < VOUT_2V && /*PS.Softstart == FALSE && */ PSFB.MainEnabled )
      {
          //FW V42 source code
//          if ( ! _SD_Flag.Val )
//          {
//              CaptureFault ( );
//              EmergencyPowerOff ( );
//              _SD_Flag.SCP_SD = 1;
//              _SD_Flag.LATCH_SD = 1;
//              //[Peter Chung] 20101209 added
//              gPagePlusStatus.PAGE[PAGE0].STATUS_IOUT.bits.IOUT_OC_WARNING = 1;
//              gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.bits.IOUT_OC_WARNING = 1;
//              gPagePlusStatus.PAGE[PAGE0].STATUS_IOUT.bits.IOUT_OC_FAULT = 1;
//              gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.bits.IOUT_OC_FAULT = 1;
//              PS.SC_CheckStart = 0;
//          }

          //david add
          //if(_SD_Flag.SCP_SD == 0){
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
          //}
      }

      //[davidchchen]20150731 Added
      #if ConfigDD_SlaveSW
      if (ADC.Iout >= Parameter.CCM_IOUT_FAULT_HLIMIT)  // Iout>= 30A
      {
        
          oSD1 = SLAVE_SW_EN;   //[davidchchen]20150731 Added
          Protect.DD_SlaveSW_OFF.Flag = 0;
          Protect.DD_SlaveSW_OFF.delay = 0;
        
      }
      else if (ADC.Iout < Parameter.CCM_IOUT_FAULT_LLIMIT)  // Iout < 10A
      {
          Protect.DD_SlaveSW_OFF.Flag = 1;

      }
      #endif

      //[davidchchen]20160108 added
      #if ConfigVout_CompSW
      if (ADC.Iout >= Parameter.CCM_VOUT_COMP_HLIMIT )  // Iout>= 60A
      {
          //[davidchchen]20160108 added
          #if ConfigVout_CompSW
          Protect.MainVoutComp.Flag = 1;
          #endif

      }
      else if (ADC.Iout < Parameter.CCM_VOUT_COMP_LLIMIT)  // Iout < 10A
      {
          oRS_EN1 = 0;                      //[davidchchen]20160108 added
          Protect.MainVoutComp.Flag = 0;    //[davidchchen]20160108 added
          Protect.MainVoutComp.delay = 0;   //[davidchchen]20160108 added
      }
      #endif
      
  }
#endif


  IFS7bits.ADCP5IF = 0;

}
// -----------------------------------------------------------------------

void __attribute__ ( ( __interrupt__, no_auto_psv ) ) _ADCP6Interrupt ( )
{
  /*
  AN12 : MASTER_PRI_IS_DSP
  AN13 : SLAVE_PRI_IS_DSP
   */

  WORD diff;
  WORD master_is;

  ADC.MasterPriIS = ADCBUF12;
  ADC.SlavePriIS = ADCBUF13;

  ADC.MasterPriIS_LPF = ADC.MasterPriIS_LPF + ADC.MasterPriIS - ADC.MasterPriIS_FF;
  ADC.MasterPriIS_FF = ADC.MasterPriIS_LPF >> 2;

  ADC.SlavePriIS_LPF = ADC.SlavePriIS_LPF + ADC.SlavePriIS - ADC.SlavePriIS_FF;
  ADC.SlavePriIS_FF = ADC.SlavePriIS_LPF >> 2;


  //if ( PS.SR_Enable )                                     //[davidchchen]20160421 removed, Master/Slave DD Switch ISSUE
  if( (iSLAVE_SW == SLAVE_SW_EN ) && ( PS.SR_Enable ) )     //[davidchchen]20160421 Added, Master/Slave DD Switch ISSUE
  {
      if ( ADC.MasterPriIS_FF > ADC.SlavePriIS_FF )
      {
          diff = ADC.MasterPriIS_FF - ADC.SlavePriIS_FF;
          master_is = ADC.MasterPriIS_FF;
      }
      else
      {
          diff = ADC.SlavePriIS_FF - ADC.MasterPriIS_FF;
          master_is = ADC.SlavePriIS_FF;
      }

      if ( diff > ( master_is >> 2 ) )
      {
          Protect.PriIS.Flag = 1;
      }
      else
      {
          Protect.PriIS.Flag = 0;
          Protect.PriIS.delay = 0;
      }
  }
  else
  {
      Protect.PriIS.Flag = 0;
      Protect.PriIS.delay = 0;
  }

  IFS7bits.ADCP6IF = 0;

}
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------

void __attribute__ ( ( __interrupt__, no_auto_psv ) ) _ADCP7Interrupt ( )
{
  /*
  AN14 : 12VSB_BUS_DET
   */

  ADC.StbVbus = ADCBUF14;

  ADC.StbVbus_LPF = ADC.StbVbus_LPF + ADC.StbVbus - ADC.StbVbus_FF;
  ADC.StbVbus_FF = ADC.StbVbus_LPF >> 2;

  {	//Fan2 Rpm detection
      static DWORD pwm_H_cnt = 0;
      static DWORD pwm_L_cnt = 0;
      static BYTE pFanState = LOW;

       if ( iFan2_Rpm == HIGH )
      {
          if ( pFanState == LOW )
          {
              gFan2.PwmL_Cnt = pwm_L_cnt;
              pwm_L_cnt = 0;
          }
          pwm_H_cnt += 20;
          pFanState = HIGH;

          if ( pwm_H_cnt >= ( DWORD ) 1500000 )
          {
              //It should be fan lock
              PS.Fan2Disappear = TRUE;
          }
          else
          {
              PS.Fan2Disappear = FALSE;
          }

      }
      else
      {
          if ( pFanState == HIGH )
          {
              gFan2.PwmH_Cnt = pwm_H_cnt;
              pwm_H_cnt = 0;
          }
          pwm_L_cnt += 20;
          pFanState = LOW;

          //PS.Fan2Disappear = FALSE;
          if ( pwm_L_cnt >= ( DWORD ) 1500000 )
          {
              //It should be fan lock
              PS.Fan2Disappear = TRUE;
          }
          else
          {
              PS.Fan2Disappear = FALSE;
          }
      }
  }


#if DITHER_ENABLED
  {
      static BYTE cnt =  0;
      static WORD period = PWM_PERIOD_LOW;

      //Dither with 500Hz, 50/100
      //Dither with 2000Hz, 14/28
      if ( cnt >= 14 )
      {
          if ( cnt >= 28 )
          {
              cnt = 0;
          }
          else
          {
              cnt ++;

              if ( period < PWM_PERIOD_LOW )
              {
                  period = period + DIFF_COUNT;
              }
              PTPER = period;
              MDC = ( period >> 1 );
          }
      }
      else
      {
          cnt ++;

          if ( period > PWM_PERIOD_HIGH )
          {
              period = period - DIFF_COUNT;
          }
          PTPER = period;
          MDC = ( period >> 1 );
      }

  }
#endif

  IFS7 = IFS7 & 0b1111111111011111;
}
// -----------------------------------------------------------------------
#if Config_Input_CN       //[davidchchen]20160427 added Input Change Notification
//[davidchchen]20160427 added
void __attribute__ ( ( __interrupt__, no_auto_psv ) ) _CNInterrupt ( )
{
    // Insert ISR code here
    IFS1bits.CNIF = 0; // Clear CN interrupt

    if ( iPFC_OK == PFC_NOK )
    {
        oPS_OK = P_N_OK;                      //[davidchchen]20160427 added
    }

}
#endif



