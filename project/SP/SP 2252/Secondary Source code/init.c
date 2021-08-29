/*****************************************************************************************
 *
 *Copyright (C) 2012 Advanced Technology Development
 *                   Power SBG
 *                   LITE-ON TECHNOLOGY Corp.
 *
 *All Rights Reserved
 *
 *File Name : Init.c
 *
 *Date : 2012.05.24
 *
 *Author :
 *
 *Description :This Program used for HP 2400W Inital Status Program.
 *
 *******************************************************************************************/

#include "p33Fxxxx.h"
#include "define.h"
#include "Init.h"
#include "Isr.h"
#include "Process.h"
#include "Userdata.h"

extern tPS_FLAG PS;
extern SHORT gVoutCmd;
//------------------------------------ SetupClock ------------------------------------------------

void init_SetupClock ( void )
{
  PLLFBD = 41;
  CLKDIVbits.PLLPOST = 0;
  
  __builtin_write_OSCCONH ( 0x01 );
  __builtin_write_OSCCONL ( 0x01 );

  while ( OSCCONbits.COSC != 0b001 );
  while ( OSCCONbits.LOCK != 1 );

  ACLKCON = 0xA740;		               // Auxiliary Clock Output Divider by 2

  while ( ACLKCONbits.APLLCK != 1 );
}
//------------------------------------ GPIO ------------------------------------------------

void init_GPIO ( void )
{
  LATB = 0x0000;
  LATC = 0x0000;
  LATD = 0x0000;
  LATE = 0x0000;
  LATF = 0x0000;
  LATG = 0x0000;

  TRISB = 0b0111111011111111;
  TRISC = 0b1000111111111111;
  #if Config_debug_IO    //[davidchchen]20160303 added
  TRISD = 0b1111111100011001;
#else
  TRISD = 0b1111111100001001;
#endif

  
  TRISE = 0b1111111111010111;
  TRISF = 0b1111111110111100;	//Chris
  TRISG = 0b1111110111111111;

  oSD_LOW = MAIN_OFF;
  oSD1 = SLAVE_SW_EN;   //[davidchchen]20150731 Added
  oPS_OK = P_N_OK;
  oLED_INPUT_CNTL = LED_INPUT_OFF;

#if PSON_ENABLE
  _SD_Flag.PS_ON_OFF = TRUE;
#endif

#if RS_ENABLE   //[davidchchen] 20150108 not used
  oRS_EN1 = RS_OFF;
#endif
  
//[davidchchen]20160108 added
#if ConfigVout_CompSW
  oRS_EN1 = 1;
#else
  oRS_EN1 = 1;
#endif
    

  o12V_ORFET_CNTL = ORFET_OFF;

}
//------------------------------------ init_Timer ------------------------------------------------

void init_Timer ( void )
{
  //------ Timer1:1ms  ------
  PR1 = 40000;
  IEC0bits.T1IE = 1;
  T1CONbits.TON = 1;

  //------ Timer1:20us for ADC  sample and conversion------
  PR2 = 800;
  T2CONbits.TON = 1;

  //------ Timer3: for Fan RPM    ------
  T3CONbits.TCKPS = 3;
  T3CONbits.TON = 1;
#if 1
  //------ Timer4:8us for I2C write cmd------//8us = (8MHz*64*1)//16us = (8MHz*64*2)
  PR4 = 1;
  T4CON = 0;
  T4CONbits.TCKPS = 2;

  IPC6bits.T4IP = 4;
  IFS1bits.T4IF = 0;
  IEC1bits.T4IE = 1;
  T4CONbits.TON = 1;
#endif
  //------ Timer5:100ms for I2C buffer cache issue------
  PR5 = 15625;
  T5CONbits.TCKPS = 3;
  IPC7bits.T5IP = 3;
  IEC1bits.T5IE = 1;
  T5CONbits.TON = 1;
}
//------------------------------------ init_ADC ------------------------------------------------

void init_ADC ( void )
{
  /*
  AN0 : CSHARE_IN
  AN1 : 12V_IOUT_DET
  AN2 : 12V_DET
  AN3 : 12VSB_DET1
  AN4 : T_INLET
  AN5 : T_SEC_2
  AN6 : --
  AN7 : --
  AN8 : --
  AN9 : 12VSB_IOUT1
  AN10 : CS_PWM_FB
  AN11 : 12VBUS_DET
  AN12 : MASTER_PRI_IS_DSP
  AN13 : SLAVE_PRI_IS_DSP
  AN14 : 12VSB_BUS_DET
  AN15 : --
   */

  ADCON = 0b0001000000000011;
  ADCONbits.ADCS = 4;					// 1 if Fpwm = 120MHz/2 = 60MHz,  default 3 if Fpwm = 120MHz
  ADPCFG = 0b1000000111000000;
  ADSTAT = 0;							// clear Conversion Data for all Pair

  //ADCP0
  ADCPC0bits.IRQEN0 = 1;				// Enable IRQ generation when requested conversion of channels AN0 and AN1 is completed
  ADCPC0bits.TRGSRC0 = 31;			//Timer2 period match

  //ADCP1
  ADCPC0bits.IRQEN1 = 1; 				// Enable IRQ generation when requested conversion of channels AN2 and AN3 is completed
  ADCPC0bits.TRGSRC1 = 31;			//Timer2 period match

  //ADCP2
  ADCPC1bits.IRQEN2 = 1;  			// Enable IRQ generation when requested conversion of channels AN4 and AN5 is completed
  ADCPC1bits.TRGSRC2 = 31;			//Timer2 period match

  //ADCP4
  ADCPC2bits.IRQEN4 = 1;  			// Enable IRQ generation when requested conversion of channels AN8 and AN9 is completed
  ADCPC2bits.TRGSRC4 = 31;			//Timer2 period match

  //ADCP5
  ADCPC2bits.IRQEN5 = 1;  			// Enable IRQ generation when requested conversion of channels AN10 and AN11 is completed
  ADCPC2bits.TRGSRC5 = 31;			//Timer2 period match

  //ADCP6
  ADCPC3bits.IRQEN6 = 1;  			// Enable IRQ generation when requested conversion of channels AN12 and AN13 is completed
  ADCPC3bits.TRGSRC6 = 31;			//Timer2 period match

  //ADCP7
  ADCPC3bits.IRQEN7 = 1;  			// Enable IRQ generation when requested conversion of channels AN14 and AN15 is completed
  ADCPC3bits.TRGSRC7 = 31;			//Timer2 period match

  IFS6bits.ADCP0IF = 0;
  IPC27bits.ADCP0IP = 5;
  IEC6bits.ADCP0IE = 1;

  IFS6bits.ADCP1IF = 0;
  IPC27bits.ADCP1IP = 5;
  IEC6bits.ADCP1IE = 1;

  IFS7bits.ADCP2IF = 0;
  IPC28bits.ADCP2IP = 3;
  IEC7bits.ADCP2IE = 1;

  IFS7bits.ADCP4IF = 0;
  IPC28bits.ADCP4IP = 3;
  IEC7bits.ADCP4IE = 1;

  IFS7bits.ADCP5IF = 0;
  IPC28bits.ADCP5IP = 5;
  IEC7bits.ADCP5IE = 1;

  IFS7bits.ADCP6IF = 0;
  IPC29bits.ADCP6IP = 5;
  IEC7bits.ADCP6IE = 1;

  IFS7 = IFS7 & 0b1111111111011111;
  IPC29 = IPC29 | 0b0000000000110000;	// ADC Pair 7 Conversion Done Interrupt Priority = 3
  IEC7 = IEC7 | 0b0000000000100000;

}
//------------------------------------ init_PWM ------------------------------------------------

void init_PWM ( )
{
  PTCON2bits.PCLKDIV = 1;			//PWM Input Clock Prescaler divided by 2
  PTPER = PWM_PERIOD;
  MDC = ( PWM_PERIOD >> 1 );

  //PWM 1 configuration MASTER		//Complementary Mode with No Phase Shift
  /*
  PWM1H : SLAVE_SYNC
  PWM1L : MASTER_SYNC
   */
  IOCON1bits.PENL = 1;
  IOCON1bits.PENH = 1;
  IOCON1bits.PMOD = 0;
  PWMCON1bits.MDCS = 1;
  PWMCON1bits.DTC = 2;			//Dead time function is disabled
  PHASE1 = 0;
  FCLCON1bits.FLTMOD = 3;

  //PWM 2	configuration				//True Independent Mode
  /*
  PWM2L : CSHARE_PWM
   */
  IOCON2bits.PENL = 1;
  IOCON2bits.PMOD = 3;
  PWMCON2bits.MDCS = 0;
  PWMCON2bits.DTC = 2;			//Dead time function is disabled
  PWMCON2bits.ITB = 1;			//PHASEx/SPHASEx registers provide time base period for this PWM generator
  SPHASE2 = 5114;	// 94kHz;
  SDC2 = 0;
  FCLCON2bits.FLTMOD = 3;

  //PWM 3 configuration				//True Independent Mode
  /*
  PWM3L : SB_REF
   */
  IOCON3bits.PENL = 1;
  IOCON3bits.PMOD = 3;
  PWMCON3bits.MDCS = 0;
  PWMCON3bits.DTC = 2;			//Dead time function is disabled
  PWMCON3bits.ITB = 1;			//PHASEx/SPHASEx registers provide time base period for this PWM generator
  SPHASE3 = 658;
  SDC3 = 0;
  FCLCON3bits.FLTMOD = 3;

  //PWM 4 configuration				 //True Independent Mode
  /*
  PWM4H : 12V_REF
  PWM4L : FAN_PWMA (Please refer the settings in Fan.c)
   */
  IOCON4bits.PENH = 1;			//PWM module controls PWMxH pin
  IOCON4bits.PENL = 1;			//PWM module controls PWMxL pin
  IOCON4bits.PMOD = 3;			//PWM I/O pin pair is in the True Independent Output mode
  PWMCON4bits.MDCS = 0;
  PWMCON4bits.DTC = 2;			//Dead time function is disabled
  PWMCON4bits.ITB = 1;			//PHASEx/SPHASEx registers provide time base period for this PWM generator
  FCLCON4bits.FLTMOD = 3;
  PHASE4 = 33000;
  PDC4 = 0;
  gVoutCmd = UserData.Page2.region.VoutCommand.Val;
  Vref.SetVoltage = gVoutCmd;
  Vref.SoftstartVoltage = 0;

  //PWM 5 configuration				 //True Independent Mode
  /*
  PWM5H : LED_PWR
  PWM5L : LED_FAIL
   */

  IOCON5bits.PENH = 1;			//PWM module controls PWMxH pin
  IOCON5bits.PENL = 1;			//PWM module controls PWMxL pin
  IOCON5bits.PMOD = 3;			//PWM I/O pin pair is in the True Independent Output mode
  PWMCON5bits.MDCS = 0;
  PWMCON5bits.DTC = 2;			//Dead time function is disabled
  PWMCON5bits.ITB = 1;			//PHASEx/SPHASEx registers provide time base period for this PWM generator
  FCLCON5bits.FLTMOD = 3;
  PHASE5 = 20000;
  PDC5 = 20000;
  SPHASE5 = 20000;
  SDC5 = 20000;

  //PWM 6 configuration				 //True Independent Mode
  /*
  PWM6L : ISSLAVE_REF
   */

#if 0
  IOCON6bits.PENL = 1;			//PWM module controls PWMxL pin
  IOCON6bits.PMOD = 3;			//PWM I/O pin pair is in the True Independent Output mode
  PWMCON6bits.MDCS = 0;
  PWMCON6bits.DTC = 2;			//Dead time function is disabled
  PWMCON6bits.ITB = 1;			//PHASEx/SPHASEx registers provide time base period for this PWM generator
  FCLCON6bits.FLTMOD = 3;
  PHASE6 = 4807;					// 100k
  PDC6 = 0;
#endif
}

#if Config_Input_CN       //[davidchchen]20160427 added Input Change Notification
//---------------------------------- init_Input_CN ----------------------------------------------
void init_Input_CN ( void )     //[davidchchen]20160427 added
{
    CNEN1bits.CN8IE = 1; // Enable CN8 pin for interrupt detection
    IEC1bits.CNIE = 1; // Enable CN interrupts
    IFS1bits.CNIF = 0; // Reset CN interrupt
}
#endif

//[davidchchen]20160427 removed
/*
//---------------------------------- init_Comparator ----------------------------------------------

void init_Comparator ( void )
{

}
//------------------------------- init_External_Interrupt -----------------------------------------

void init_External_Interrupt ( void )
{

}
*/


