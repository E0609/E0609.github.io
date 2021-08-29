/*****************************************************************************************
 *
 *Copyright (C) 2012 Advanced Technology Development
 *                   Power SBG
 *                   LITE-ON TECHNOLOGY Corp.
 *
 *All Rights Reserved
 *
 *File Name : Timer.c
 *
 *Date : 2012.05.24
 *
 *Author :
 *
 *Description :This Program used for HP 2400W Timer Program.
 *
 *******************************************************************************************/
#include "Timer.h"
#include "Standby.h"
#include "Pmbus.h"
#include "Isr.h"
#include "Process.h"
#include "Psfb.h"
#include "Sci.h"

#define MAX_SYS_TIMER 8 //15    //[davidchchen] 20150108 modified

//------------------Global Variable-----------------------------------------------------
SYS_TIMER SysTimer[MAX_SYS_TIMER];
QWORD SysTimerCount = 0;
static WORD UnitOffBlankingTime = 0;	//[Peter Chung] 20110812 added for unit off not show issue
WORD AcOffBlankingTime = 0;
WORD AcOnBlankingTime = 0;
//WORD VrefCompDelay = 0;   //[davidchchen]20160408 removed, the Function no used
WORD Tsb_off_delay = 0;
BYTE isInputDetected = FALSE;

BYTE Tsb_off_delay_Flag = 0;
//------------------Exported Variable-----------------------------------------------------
extern tPS_FLAG PS;
extern tPSFB PSFB;
//-------------------------------- T1Interrupt 1ms --------------------------------------

void __attribute__ ( ( __interrupt__, no_auto_psv ) ) _T1Interrupt ( void )
{
  BYTE i;
  QWORD DiffTime;

  SysTimerCount ++;

  for ( i = 0; i < MAX_SYS_TIMER; i ++ )
  {
      if ( SysTimer[i].Active )
      {
          DiffTime = SysTimerCount - SysTimer[i].StartTime;
          if ( DiffTime == SysTimer[i].Elapse )
          {
              if ( SysTimer[i].pTimerFunc ) SysTimer[i].pTimerFunc ( );
              SysTimer[i].StartTime = SysTimerCount;
          }
      }
  }


  //For SB SCP
  if ( iAC_OK == AC_GOOD )
  {
      if ( STB.Enabled )
      {
          if ( STB_SCP_Delay < 0xFE )
          {
              STB_SCP_Delay ++;
          }
      }
      else
      {
          STB_SCP_Delay = 0;
      }
  }
  else
  {
      STB_SCP_Delay = 0;
  }

  //For Main SCP
  {
      static BYTE count = 0;

      if ( PS.Softstart )
      {
          if ( count >= 3 )
          {
              PS.SC_CheckStart = 1;
          }
          else
          {
              count ++;
          }
      }
      else
      {
          PS.SC_CheckStart = 0;
      }
  }

  //[Peter Chung] 20110503 added, should be the same priority with the uart data updating
  Count_4AC_Cycle ( );

  if ( ADC.StbVout_FF > cSTB_OK_REFH && STB.Enabled )
  {
      if ( STB.Counter_InRegulation < 0xFFFE ) STB.Counter_InRegulation ++;
  }

  //FW V42 Source code
//  if ( isInputDetected == FALSE && ( iAC_OK == AC_GOOD || PS.U2RX_CommOK == TRUE ) )
//  {
//      static BYTE lastInput = 0;
//      static WORD cnt = 0;
//
//
//      if ( lastInput == Uart.U1.Rx.Pri_Status.bits.DC_INPUT )
//      {
//          if ( cnt < 2000 )
//          {
//              cnt ++;
//          }
//          else
//          {
//              PS.isDCInput = Uart.U1.Rx.Pri_Status.bits.DC_INPUT;
//              isInputDetected = TRUE;
//          }
//      }
//      else
//      {
//          lastInput = Uart.U1.Rx.Pri_Status.bits.DC_INPUT;
//          cnt = 0;
//      }
//  }

//david add
{
      static BYTE lastInput = 0;
      static WORD cnt = 0;

      if ( iAC_OK == AC_GOOD || ( PS.U2RX_CommOK == TRUE && Real_Vac > 60 ) )
      {
          if ( lastInput == Uart.U1.Rx.Pri_Status.bits.DC_INPUT )
          {
              if ( cnt < 2000 )      //[davidchchen]20161026 Modify UNIT_OFF issue
              //if ( cnt < 62 )      //[davidchchen]20160408 changed, debug for DC_Brown Issue
              {
                  cnt ++;
              }
              else
              {
                  PS.isDCInput = Uart.U1.Rx.Pri_Status.bits.DC_INPUT;
              }
          }
          else
          {
              lastInput = Uart.U1.Rx.Pri_Status.bits.DC_INPUT;
              cnt = 0;
          }
      }
      else
      {
          cnt = 0;
      }
}

  //[Peter Chung] 20110812 added for unit off not show issue
  if ( UnitOffBlankingTime < 5000 )
  {
      UnitOffBlankingTime ++;
  }
  else
  {
      //if ( iAC_OK == AC_N_GOOD && ( ( _SD_Flag.STB_OCP == 0 ) && ( _SD_Flag.STB_OVP == 0 ) && ( _SD_Flag.STB_UVP == 0 ) && ( _SD_Flag.STB_OTP == 0 ) ) /*&& (AcOffBlankingTime >= 5)*/ )
      if ( iAC_OK == AC_N_GOOD && ( ( _SD_Flag.STB_OCP == 0 ) && ( _SD_Flag.STB_OVP == 0 ) && ( _SD_Flag.STB_UVP == 0 ) ) /*&& (AcOffBlankingTime >= 5)*/ )    //[David ch chen]20141124 removed _SD_Flag.STB_OTP, not use _SD_Flag.STB_OTP
      {

          if ( iPS_OK == P_N_OK )	// [Tommy YR Chen] 20111014 added
          {
              if ( PS.isDCInput == FALSE )
              {
                  gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.bits.UNIT_OFF = 1;
                  if ( Real_Vac < Parameter.VIN_UV_WARN_LIMIT[0] )
                  {
                      gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.bits.VIN_UV_WARNING = 1;
                  }
                  if ( Real_Vac < Parameter.VIN_UV_FAULT_LIMIT[0] )
                  {
                      gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.bits.VIN_UV_FAULT = 1;
                  }

              }
              else
              {
                  gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.bits.UNIT_OFF = 1;
                  if ( Real_Vac < Parameter.VIN_UV_WARN_LIMIT[1] )
                  {
                      gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.bits.VIN_UV_WARNING = 1;
                  }
                  if ( Real_Vac < Parameter.VIN_UV_FAULT_LIMIT[1] )
                  {
                      gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.bits.VIN_UV_FAULT = 1;
                  }
              }
          }
      }
  }

  //[Peter Chung] 20110825 added
  if ( iAC_OK == AC_N_GOOD )
  {
      AcOnBlankingTime = 0;
      if ( AcOffBlankingTime >= 60 )
      {	//60ms
          PS.AC_OFF_ClearFault = TRUE;
      }
      else
      {
          AcOffBlankingTime ++;
      }
  }
  else
  {
      AcOffBlankingTime = 0;
      if ( AcOnBlankingTime >= 5000 )
      {	//5000ms
          PS.AC_ON_CheckLineStatus = TRUE;
      }
      else
      {
          AcOnBlankingTime ++;
      }
  }


  /*SR control*/
  if ( PSFB.InRegulation == TRUE )
  {
      if ( PSFB.Counter_InRegulation < 0xFFFE )
      {
          PSFB.Counter_InRegulation ++;
      }
  }
  else
  {
      PSFB.Counter_InRegulation = 0;
  }

  if ( ADC.Iout_FF > cSR_ON_IOUT_REFH )
  {
      if ( PSFB.SR_ON_Delay < 0xFFFE ) PSFB.SR_ON_Delay ++;
  }

//[davidchchen]20160408 removed, the Function no used
//  if ( Iout_Real > 55 )
//  {	//around 15A
//      if ( VrefCompDelay < 0xFFFE ) VrefCompDelay ++;
//  }
//  else if ( Iout_Real < 40 )
//  {
//      VrefCompDelay = 0;
//  }

  /*Stb output control*/
  if ( iPS_OK == P_N_OK && iAC_OK == AC_N_GOOD )
  {
      if ( Tsb_off_delay < 0xFFFE ) 
      {
          Tsb_off_delay ++;
      }
  }
  else
  {
      Tsb_off_delay = 0;
      Tsb_off_delay_Flag = 0;
  }

  IFS0bits.T1IF = 0;
}
//-------------------------------- SetTimer ms--------------------------------------

BYTE SetTimer ( WORD Elapse, _Pfn pTimerFunc )
{
  BYTE i;

  if ( Elapse == 0 )
  {
      if ( pTimerFunc != NULL )
      {
          pTimerFunc ( );
      }
  }
  else
  {
      for ( i = 0; i < MAX_SYS_TIMER; i ++ )
      {
          if ( SysTimer[i].Active == 0 )
          {
              SysTimer[i].Active = 1;
              SysTimer[i].pTimerFunc = pTimerFunc;
              SysTimer[i].Elapse = Elapse;
              SysTimer[i].StartTime = SysTimerCount;
              return i + 1;
          }
      }
  }
  return 0;
}
//-------------------------------- RestartTimer --------------------------------------

void RestartTimer ( BYTE *pTimerID )
{
  if ( *pTimerID <= 0 || * pTimerID > MAX_SYS_TIMER ) return;

  if ( SysTimer[*pTimerID - 1].Active == 0 ) return;

  SysTimer[*pTimerID - 1].StartTime = SysTimerCount;
}

//-------------------------------- KillTimer ms --------------------------------------

void KillTimer ( BYTE *pTimerID )
{
  if ( *pTimerID <= 0 || * pTimerID > MAX_SYS_TIMER ) return;

  SysTimer[*pTimerID - 1].Active = 0;

  *pTimerID = 0;
}





