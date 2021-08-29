
#include "p33Fxxxx.h"
#include "define.h"
#include "dsp.h"
#include "Standby.h"
#include "Pmbus.h"
#include "Process.h"
#include "Protection.h"
#include "Timer.h"
#include "PowerOnOff.h"
#include "Psfb.h"
#include "Isr.h"


//-------------------Global Variable----------------------------------------------------------
tSTB STB;
BYTE STB_SCP_Delay;
//-------------------Extern Variable-----------------------------------------------------------

extern tPS_FLAG PS;
extern tPSU_STATE gPS_State;
extern SHORT gStbVoutOffset;

//-------------------------------------------------------------------------------------------

void STBOringCntrl ( void )
{

}

void EnableSTBoutput ( void )
{
  //if ( ( _SD_Flag.STB_OCP | _SD_Flag.STB_OVP | _SD_Flag.STB_UVP | _SD_Flag.STB_OTP ) == FALSE )
  if ( ( _SD_Flag.STB_OCP | _SD_Flag.STB_OVP | _SD_Flag.STB_UVP ) == FALSE )    //[David ch chen]20141124 removed _SD_Flag.STB_OTP, not use _SD_Flag.STB_OTP
  {
      oSTB_ON = STB_ON;
      STB.Enabled = TRUE;
      
  }
}

void DisableSTBoutput ( void )
{
  STB.Enabled = FALSE;
  oSTB_ON = STB_OFF;
  //[Peter Chung] Disable Standby & main Protection
  PS.STB_UV_START = FALSE;
  PS.MAIN_PROTECT_START = FALSE;
  
}

void Check_STBOVP ( void )
{
  if ( ADC.StbVout > STB_OVW_REF )
  {
      gPagePlusStatus.PAGE[PAGE1].STATUS_VOUT.bits.VOUT_OV_WARNING = 1;
  }

  if ( ADC.StbVout > STB_OVP_REF )	// standby output volatge > 14V
  {
      gPagePlusStatus.PAGE[PAGE1].STATUS_VOUT.bits.VOUT_OV_FAULT = 1;
      if ( _SD_Flag.STB_OVP == FALSE )
      {
          _SD_Flag.STB_OVP = TRUE;
          CaptureFault ( );
          DisableSTBoutput ( );
          EmergencyPowerOff ( );
          PS.SaveBlackBox = 1;
      }
  }
}

void Check_STBSCP ( void )
{
  //if ( ( ADC.StbVout < STB_SC_V_L_REF ) && ( ADC.StbIout_FF > STB_SC_REF )/* && STB_SCP_Delay >= 4*/ )	// around 1V
  if ( ADC.StbIout_FF > STB_SC_REF  )	// [davidchchen]20150911 modify
  {
      gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.bits.IOUT_OC_WARNING = 1;     //[davidchchen]20170727 Added Modify can't clear_fault issue
      //gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.bits.IOUT_OC_FAULT = 1;     //[davidchchen]20160826 removed to Modify STB STATUS_WORD can't be clear by clear fault
      gPagePlusStatus.PAGE[PAGE1].STATUS_IOUT.bits.IOUT_OC_FAULT = 1;       //[davidchchen]20170619 Added

      MFR_Status.bits.VSB_UVW = 1;
      if ( _SD_Flag.STB_OCP == FALSE )
      {
          _SD_Flag.STB_OCP = TRUE;
          CaptureFault ( );
          DisableSTBoutput ( );
          //EmergencyPowerOff();
          PS.SaveBlackBox = 1;
      }
  }
}

void Check_STBUVP ( void )
{
  if ( PS.STB_UV_START == TRUE )
  {
      if ( ADC.StbVout_FF < STB_UVW_REF )
      {

//          gPagePlusStatus.PAGE[PAGE1].STATUS_VOUT.bits.VOUT_UV_WARNING = 1;
//          MFR_Status.bits.VSB_UVW = 1;

          Protect.STB_UVW.Flag = 1;         //[davidchchen]20160201 changed

      }
      else
      {
          Protect.STB_UVW.Flag = 0;         //[davidchchen]20160201 changed
          Protect.STB_UVW.delay = 0;        //[davidchchen]20160201 changed
      }
      if ( ADC.StbVout_FF < STB_UV_L_REF )
      {
          Protect.STB_UVP.Flag = 1;
      }
      else
      {
          Protect.STB_UVP.Flag = 0;
          Protect.STB_UVP.delay = 0;
      }
  }
  else
  {
      Protect.STB_UVP.Flag = 0;
      Protect.STB_UVP.delay = 0;
      Protect.STB_UVW.Flag = 0;         //[davidchchen]20160201 changed
      Protect.STB_UVW.delay = 0;        //[davidchchen]20160201 changed
  }
}

void Check_STBOCP ( void )
{
  if ( STB.Enabled == TRUE )
  {
      //first turn on, the delay time is 25ms
      // 3V --> 610AD
      if ( ADC.StbIout_FF > STB_OCW_REF )    //[davidchchen]20170619 added
      {
          Protect.STB_OCW.Flag = 1;         //[davidchchen]20170619 added
      }
      else
      {
          Protect.STB_OCW.Flag = 0;         //[davidchchen]20170619 added
          Protect.STB_OCW.delay = 0;        //[davidchchen]20170619 added
      }
      
      if ( ADC.StbIout_FF > STB_OC_REF )
      {
          Protect.STB_OCP.Flag = 1;
      }
      else
      {
          Protect.STB_OCP.Flag = 0;
          Protect.STB_OCP.delay = 0;
      }

  }
  else
  {
      Protect.STB_OCP.Flag = 0;
      Protect.STB_OCP.delay = 0;
      Protect.STB_OCW.Flag = 0;         //[davidchchen]20170619 added
      Protect.STB_OCW.delay = 0;        //[davidchchen]20170619 added
  }
}

void Check_STB ( void )
{
  if ( ( ADC.StbVout_FF > cSTB_OK_REFH ) && ( STB.Enabled == TRUE ) && ( STB.Counter_InRegulation >= 100 ) )
  {
      STB.InRegulation = TRUE;
  }
  else if ( ADC.StbVout_FF < cSTB_OK_REFL )
  {
      STB.InRegulation = FALSE;
      STB.Counter_InRegulation = 0;
  }
}

void StandbyVoltageControl ( void )
{
  SHORT target;
  SHORT Vref;

  Vref = SPHASE3;

  target = ( ( SHORT ) Vref - gStbVoutOffset );
  if ( target >= SPHASE3 )
  {
      target = SPHASE3;
  }
  else if ( target <= 0 )
  {
      target = 0;
  }

  SDC3 = target;

  Check_STB ( );
}

void init_Standby ( void )
{
  StandbyVoltageControl ( );
}

