/*****************************************************************************************
 *
 *Copyright (C) 2012 Advanced Technology Development
 *                   Power SBG
 *                   LITE-ON TECHNOLOGY Corp.
 *
 *All Rights Reserved
 *
 *File Name : PowerOnOff.c
 *
 *Date : 2012.05.24
 *
 *Author :
 *
 *Description :This Program used for HP 2400W PowerOnOff Control Program.
 *
 *******************************************************************************************/
#include "PowerOnOff.h"
#include "Process.h"
#include "Timer.h"
#include "Isr.h"
#include "Pmbus.h"
#include "Status.h"
#include "Psfb.h"
#include "Protection.h"

//--------- Global variable ----------------------------------------------------------------
tTimerHandle hTimer;
BYTE gPSU_ONOFF_Status = PowerOff;

//--------- Extern variable ----------------------------------------------------------------
extern tPS_FLAG PS;
extern tPSFB PSFB;

//-------------------------- IsReadyToPowerOn -------------------------------------------

BYTE IsReadyToPowerOn ( )
{
#if PFC_ENABLE
  //if ( iPFC_OK == PFC_OK && ( _SD_Flag.Val & 0xDFFF ) == 0 && PS.VinOVFault == 0 && PS.VinUVFault == 0 )    //[davidchchen] 20150113 Added HW_FW_Compatible Code Function
  if ( iPFC_OK == PFC_OK && ( _SD_Flag.Val & 0xDFFF ) == 0 && PS.VinOVFault == 0 && PS.VinUVFault == 0 && PS.Compatible_code == 0 )
#else
  if ( ! _SD_Flag.Val )
#endif
  {
      return TRUE;
  }

  return FALSE;
}
//----------------------- IsReadyToPowerOff ------------------------------------------

BYTE IsReadyToPowerOff ( )
{
#if PFC_ENABLE
  //if ( ( _SD_Flag.Val & 0xDFFF ) != 0 || iPFC_OK == PFC_NOK || PS.VinOVFault == 1 || PS.VinUVFault == 1 )
  if ( ( _SD_Flag.Val & 0xDFFF ) != 0 || iPFC_OK == PFC_NOK || PS.VinOVFault == 1 || PS.VinUVFault == 1 || PS.Compatible_code == 1 )    //[davidchchen] 20150113 Added HW_FW_Compatible Code Function
#else
  if ( _SD_Flag.Val )
#endif
  {
      return TRUE;
  }

  return FALSE;
}
//--------------------------Power ON Timing---------------------------------------------

void HaltPowerOn ( )
{
  //if ( hTimer._Ton_delay )                //[davidchchen]20160427 removed
  //{
  //    KillTimer ( &hTimer._Ton_delay );
  //}

  if ( hTimer._Tvout_in_reg )
  {
      KillTimer ( &hTimer._Tvout_in_reg );
  }

  if ( hTimer._Tpwok_on )
  {
      KillTimer ( &hTimer._Tpwok_on );
  }
}
//--------------------------Power OFF Timing---------------------------------------------

void HaltPowerOff ( )
{
  //if ( hTimer._Tpwok_off )                  //[davidchchen]20160427 removed
  //{
  //    KillTimer ( &hTimer._Tpwok_off );
  //}

  if ( hTimer._Tmain_off )
  {
      KillTimer ( &hTimer._Tmain_off );
  }
}
//----------------------------------------------------------------------------------------

void ISR_Tmain_off ( )
{
  //KillTimer ( &hTimer._Tpwok_off );       //[davidchchen]20160427 removed
  TurnOffOutput ( );
  gPSU_ONOFF_Status = PowerOff;
}
//----------------------------------------------------------------------------------------

void ISR_Tpwok_off ( )
{
  //KillTimer ( &hTimer._Tpwok_off );   //[davidchchen]20160427 removed
  oPS_OK = P_N_OK;

  if ( hTimer._Tmain_off == 0 )
  {
      hTimer._Tmain_off = SetTimer ( Tmain_off, ISR_Tmain_off );
  }

  gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.bits._OFF = TRUE;
  gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.bits._OFF = TRUE;
  gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.bits.POWER_GOOD = TRUE;
  gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.bits.POWER_GOOD = TRUE;
}

//------------------------------ PowerOnSeq ------------------------------------------------

void ISR_Tpwok_on ( )
{
  KillTimer ( &hTimer._Tpwok_on );
  oPS_OK = P_OK;

  //gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.bits._OFF = FALSE;
  //gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.bits._OFF = FALSE;
  gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.bits.POWER_GOOD = FALSE;
  gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.bits.POWER_GOOD = FALSE;

  if ( PS.OT_Recover == FALSE && PS.Fan_Recover == FALSE )
  {
#if 0
      if ( gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.bits.UNIT_OFF )
      {
          //refer to 3.5-point 3, reset all status except "Unit Off for Low Input Voltage"
          ClearAllStatus ( PAGE0 );
          gPagePlusStatus.PAGE[PAGE0].STATUS_INPUT.bits.UNIT_OFF = 1;
      }
      else
      {
          ClearAllStatus ( PAGE0 );
      }
      if ( gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.bits.UNIT_OFF )
      {
          //refer to 3.5-point 3, reset all status except "Unit Off for Low Input Voltage"
          ClearAllStatus ( PAGE1 );
          gPagePlusStatus.PAGE[PAGE1].STATUS_INPUT.bits.UNIT_OFF = 1;
      }
      else
      {
          ClearAllStatus ( PAGE1 );
      }
#endif
      ClearAllStatus ( 0xFF );
  }
  else
  {
      PS.OT_Recover = FALSE;
      PS.Fan_Recover = FALSE;
  }

  MFR_Status.Val = 0;

  gPSU_ONOFF_Status = PowerGood;
}

//------------------------------ PowerOnSeq ------------------------------------------------

void ISR_Tvout_in_reg ( )
{
  if ( PSFB.InRegulation )
  {
      KillTimer ( &hTimer._Tvout_in_reg );
      gPSU_ONOFF_Status = PowerReady;
      if ( hTimer._Tpwok_on == 0 )
      {
          hTimer._Tpwok_on = SetTimer ( Tpwok_on, ISR_Tpwok_on );
      }

#if RS_ENABLE   //[davidchchen] 20150108 not used
      oRS_EN1 = RS_ON;
#endif
  }
}

//------------------------------- TurnOnOutput -------------------------------------------

void TurnOnOutput ( )
{
  PSFB.MainEnabled = TRUE;
#if ConfigDD_SlaveSW
  oSD1 = SLAVE_SW_EN;   //[davidchchen]20150731 Added
#endif
  oSD_LOW = MAIN_ON;

  //[davidchchen]20160108 added
#if ConfigVout_CompSW
  oRS_EN1 = 1;
#endif

  //gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.bits._OFF = FALSE;
  //gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.bits._OFF = FALSE;

  o12V_ORFET_CNTL = ORFET_ON;
  Protect.SoftStart.Flag = 1;

  if ( hTimer._Tvout_in_reg == 0 )
  {
      hTimer._Tvout_in_reg = SetTimer ( Tvout_in_reg, ISR_Tvout_in_reg );
  }
}
//------------------------------- TurnOffOutput -------------------------------------------

void TurnOffOutput ( )
{
  oSD_LOW = MAIN_OFF;
  o12V_ORFET_CNTL = ORFET_OFF;
  gPSU_ONOFF_Status = PowerOff;

  PDC4 = 0;
  PS.Softstart = FALSE;
  PS.SoftstartFinished = FALSE;
  Protect.SoftStart.Flag = 0;
  Protect.SoftStart.delay = 0;

  PSFB.MainEnabled = FALSE;
  Vref.SoftstartVoltage = 0;
  PS.MAIN_PROTECT_START = FALSE;

  Disable_CurrentShare ( );


  
#if RS_ENABLE   //[davidchchen] 20150108 not used
  oRS_EN1 = RS_OFF;
  gVoutRSenseOffset = 0;
#endif

}

//------------------------------ PowerOnSeq ------------------------------------------------

void PowerOnSeq ( BYTE* hTImer, WORD period, _Pfn isr )
{
  if ( gPSU_ONOFF_Status == PowerOning || gPSU_ONOFF_Status == PowerReady || gPSU_ONOFF_Status == PowerGood )
  {
      return;
  }
  else
  {
      if ( IsReadyToPowerOn ( ) )
      {
          HaltPowerOff ( );
          if ( *hTImer == 0 )
          {
              gPSU_ONOFF_Status = PowerOning;
              *hTImer = SetTimer ( period, isr );
          }
      }
  }
}
//------------------------------- PowerOffSeq -----------------------------------------------

void PowerOffSeq ( BYTE* hTImer, WORD period, _Pfn isr )
{
  if ( gPSU_ONOFF_Status == PowerOffing || gPSU_ONOFF_Status == PowerOff )
  {
      return;
  }
  else
  {
      if ( IsReadyToPowerOff ( ) )
      {
          HaltPowerOn ( );
          if ( *hTImer == 0 )
          {
              gPSU_ONOFF_Status = PowerOffing;
              *hTImer = SetTimer ( period, isr );
          }
      }
  }
}
//----------------------------------------NO USE-----------------------------------------------

void EmergencyPowerOff ( )
{
  gPSU_ONOFF_Status = PowerOffing;
  oPS_OK = P_N_OK;
  HaltPowerOn ( );
  TurnOffOutput ( );

  gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.bits._OFF = TRUE;
  gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.bits._OFF = TRUE;
  gPagePlusStatus.PAGE[PAGE0].STATUS_WORD.bits.POWER_GOOD = TRUE;
  gPagePlusStatus.PAGE[PAGE1].STATUS_WORD.bits.POWER_GOOD = TRUE;
}




