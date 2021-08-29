
#include "Led.h"
#include "Timer.h"
#include "Pmbus.h"
#include "Process.h"
#include "Sci.h"

static BYTE gSysCntl;
tLED_STATUS gLedStatus;
tLED_WARNING_STATUS gLedWarningStatus;
tINPUT_LED_STATUS gInputLedStatus;

extern tPS_FLAG PS;

void init_Led ( )
{
  gSysCntl = FALSE;
  gLedStatus.Val = 0;
}

void SetLedGreen ( )
{
  PDC5 = 0;
  SDC5 = 20000;
}

void SetLedYellow ( )
{
  PDC5 = 20000;
  SDC5 = 0;
}

void SetLedOff ( )
{
  PDC5 = 20000;
  SDC5 = 20000;
}

void SetLedAlternated ( )
{
  static BYTE onoff = ON;

  if ( onoff )
  {
      SetLedGreen ( );
  }
  else
  {
      SetLedYellow ( );
  }

  onoff = ! onoff;
}

void SetLedBlinkGreen ( )
{
  static BYTE onoff = ON;

  if ( onoff )
  {
      SetLedGreen ( );
  }
  else
  {
      SetLedOff ( );
  }

  onoff = ! onoff;
}

void SetLedBlinkYellow ( )
{
  static BYTE onoff = ON;

  if ( onoff )
  {
      SetLedYellow ( );
  }
  else
  {
      SetLedOff ( );
  }


  onoff = ! onoff;
}

void SetLedBlinkYellow2s ( )
{
  static BYTE onoff = ON;
  static BYTE cnt = 0;
#if 0
  if ( onoff )
  {
      SetLedYellow ( );
  }
  else
  {
      SetLedOff ( );
  }
#endif

  cnt ++;
  if ( onoff == ON )
  {
      SetLedYellow ( );
      if ( cnt >= 4 )
      {	// 4 = 2s /LED_BLINK_FREQ
          cnt = 0;
          onoff = OFF;
      }
  }
  else
  {
      SetLedOff ( );
      if ( cnt >= 2 )
      {	// 2 = 1s /LED_BLINK_FREQ
          cnt = 0;
          onoff = ON;
      }
  }
}

void SetLedBlink ( )
{
  static BYTE onoff = ON;

  if ( onoff )
  {
      if ( gLedStatus.bits.fault == 1 )
      {
          SetLedYellow ( );
      }
      else
      {
          SetLedGreen ( );
      }
  }
  else
  {
      SetLedOff ( );
  }

  onoff = ! onoff;
}

void SetInputLedBlinkGreen ( )
{
  static BYTE onoff = ON;

  if ( onoff )
  {
      oLED_INPUT_CNTL = LED_INPUT_ON;
  }
  else
  {
      oLED_INPUT_CNTL = LED_INPUT_OFF;
  }

  onoff = ! onoff;
}

static void LedControl ( BYTE status )
{
  static BYTE ghLedTimer = 0;
  static BYTE previous_status = 0;

  if ( status != previous_status )
  {
      if ( ghLedTimer )
      {
          KillTimer ( &ghLedTimer );
      }

      switch ( status )
      {
          case SOLID_GREEN:
              SetLedGreen ( );
              break;
          case SOLID_YELLOW:
              SetLedYellow ( );
              break;
          case ALTERNATED:
              ghLedTimer = SetTimer ( LED_BLINK_FREQ, SetLedAlternated );
              break;
          case BLINK:
              ghLedTimer = SetTimer ( LED_BLINK_FREQ, SetLedBlink );
              break;
          case BLINK_GREEN:
              ghLedTimer = SetTimer ( LED_BLINK_FREQ, SetLedBlinkGreen );	// 1 Hz
              break;
          case BLINK_YELLOW:
              ghLedTimer = SetTimer ( LED_BLINK_FREQ, SetLedBlinkYellow );	// 1 Hz
              break;
          case LED_OFF:
              SetLedOff ( );
              break;
          case BLINK_GREEN_TYPE2:
              ghLedTimer = SetTimer ( LED_BLINK_FREQ_TYPE2, SetLedBlinkGreen );	// 0.5 Hz
              break;
          case BLINK_YELLOW_TYPE2:
              ghLedTimer = SetTimer ( LED_BLINK_FREQ_TYPE2, SetLedBlinkYellow );	// 0.5 Hz
              break;
          case BLINK_YELLOW_TYPE3:
              ghLedTimer = SetTimer ( LED_BLINK_FREQ_TYPE3, SetLedBlinkYellow ); // 0.2 Hz
              break;

          default:
              break;
      }
  }

  previous_status = status;
}

static void InputLedControl ( BYTE status )
{
  static BYTE ghLedTimer = 0;
  static BYTE previous_status = 0;

  if ( status != previous_status )
  {
      if ( ghLedTimer )
      {
          KillTimer ( &ghLedTimer );
      }

      switch ( status )
      {
          case SOLID_GREEN:
              oLED_INPUT_CNTL = LED_INPUT_ON;
              break;
          case BLINK_GREEN:
              ghLedTimer = SetTimer ( LED_BLINK_FREQ, SetInputLedBlinkGreen );	// 1 Hz
              break;
          case LED_OFF:
              oLED_INPUT_CNTL = LED_INPUT_OFF;
              break;

          default:
              break;
      }
  }

  previous_status = status;
}

//[Peter Chung] 20101013 updated for X04-00

void Led_SYS_Control ( BYTE cmd )
{
  gSysCntl = TRUE;
  switch ( cmd )
  {
      case LED_SYS_CNTL_DISABLE:
          gSysCntl = FALSE;
          break;
      case LED_BLINK_GREEN:
          LedControl ( BLINK_GREEN );
          break;
      case LED_BLINK_YELLOW:
          LedControl ( BLINK_YELLOW );
          break;
      case LED_SOLID_GREEN:
          LedControl ( SOLID_GREEN );
          break;
      case LED_SOLID_YELLOW:
          LedControl ( SOLID_YELLOW );
          break;

      default:
          gSysCntl = FALSE;
          break;
  }
}

void Led_PSU_Control ( )
{
  if ( gLedStatus.bits.ac_loss == 1 )
  {
      InputLedControl ( LED_OFF );
  }
  else if ( gInputLedStatus.bits.vin_ov_fault == 1 )
  {
      InputLedControl ( LED_OFF );
  }
  else if ( gInputLedStatus.bits.vin_uv_fault == 1 )
  {
      InputLedControl ( LED_OFF );
  }
  else if ( gInputLedStatus.bits.vin_uv_warning == 1 || gInputLedStatus.bits.vin_ov_warning == 1 )
  {
      //Blink 1 Hz
      InputLedControl ( BLINK_GREEN );
  }
  else
  {
      InputLedControl ( SOLID_GREEN );
  }

  if ( gLedStatus.bits.ac_loss == 1 || gInputLedStatus.bits.vin_ov_fault == 1 || gInputLedStatus.bits.vin_uv_fault == 1 )
  {
      LedControl ( LED_OFF );
  }
  else
  {
      if ( gLedStatus.bits.standby == 1 )
      {
          if ( PS.Stb_PSON_L == 1 )
          //if ( PS.Stb_PSON_L == 1 || PS.MFR_PSON_CONTROL == MFR_PSON_DISABLE )      //[davidchchen]20170216 added PSON Signal Enable/disable
          {
              LedControl ( BLINK_YELLOW );
          }
          else if ( PS.Stb_OP_OFF == 1 )
          {
              LedControl ( BLINK_YELLOW_TYPE3 );    // 0.2Hz
          }
          else if (PS.STB_Fault == 1)               //[davidchchen]20160513 Added, When STB_SCP,OCP,OVP,UVP setting LED TO solid yellow
          {
              LedControl ( SOLID_YELLOW );          //[davidchchen]20160513 Added, When STB_SCP,OCP,OVP,UVP setting LED TO solid yellow
          }
          else
          {
              LedControl ( LED_OFF );
              
          }
      }
      else
      {
          if ( gLedStatus.bits.fault == 1 )
          {
              LedControl ( SOLID_YELLOW );
          }
          else if ( gLedStatus.bits.warning == 1 )
          {
              LedControl ( BLINK_GREEN );
          }
          else if (gLedStatus.bits.comm_err == 1 || gLedStatus.bits.PriRXcomm_err == 1 || gLedStatus.bits.U2comm_err == 1 )      //[davidchchen]20160503 added primary and secondary side communication error
          {
              LedControl ( BLINK_YELLOW_TYPE2 );        // 0.5Hz, //[davidchchen]20160503 added primary and secondary side communication error
          }
          else
          {
              LedControl ( SOLID_GREEN );
          }
      }
  }
}


