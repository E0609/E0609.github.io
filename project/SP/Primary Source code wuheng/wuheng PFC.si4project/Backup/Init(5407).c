/*****************************************

FILE: Init.c
COMPANY: Liteon Technology, Inc.
PROCESSOR: MC9S08QG4
PURPOSE: Initialize the registers
HISTORY:

DATE        REVISION    DESCRIPTION
-----------------------------------
2008/12/09  v0.1        Created

*****************************************/

#include <MC9S08QG4.h>
#include "Config.h"

/*************************************
    Register definition
*************************************/
#if (CW_IDE == 60)  //define for code worrior 6.0 only

extern volatile byte _NVICSTRM @0x0000FFAF;
#define NVICSTRM _NVICSTRM
volatile byte _NVICSTRM;

#endif

/*************************************
    Internal Function
*************************************/
static void Init_ICS(void);
static void Init_WDT(void);
static void Init_PortA(void);
static void Init_PortB(void);
static void Init_MTIM(void);
static void Init_ADC(void);
static void Init_SCI(void);

/*************************************
    Implement Begin
*************************************/
void Initialize(void)
{
    Init_ICS();
    Init_WDT();
    Init_PortA();
    Init_PortB();
    Init_MTIM();
    Init_ADC();
    Init_SCI();
}

static void Init_ICS(void)
{
    if(NVICSTRM != 0xFF)
        ICSTRM = NVICSTRM;          //2008/05/16 load trim value if location not blank
    
    ICSC1 = 0x06;
    
    while (ICSC1_CLKS != ICSSC_CLKST);       
  
    //ICSC2 = 0x40;                 //2007/05/23 to set the Bus frequence as 4MHz
    ICSC2 = 0x00;                   //now it is 8MHz

}

static void Init_WDT(void)
{
    SOPT1_COPE = 1;                 //watchdog time is 32ms
    SOPT1_COPT = 1;
}

static void Init_PortA(void)
{
    PTAD = 0; //reset to 0
    PTADD = 0x00; //config to input pin		
}

static void Init_PortB(void)
{
    PTBD = 0; //reset to 0        
    PTBDD = 0xFE; //config to output pin
    
	PFC_OK_CTRL = PFC_NOTOK;
	PFC_STB_CTRL = PFC_STB_NOTOK;
	AC_OK_CTRL = AC_NOTOK;
    pin_PFC_CTRL = PFC_OFF;
    PFC_ADJUST_PIN = PFC_ADJUST_ON;
    //PFC_ADJUST_PIN = PFC_ADJUST_OFF;
	RELAY_CTRL = RELAY_OFF;
	
}

static void Init_MTIM(void)
{
    MTIMCLK = 0x03;                 //MTIMCLK = Fbus / 8 = 8MHz / 8 = 1 MHz
    MTIMMOD = 0xA6;                 //0xA6 = 166(Decimal),0xFA = 250 us
    MTIMSC = 0x60;                  //MTIM counter enable    
}

static void Init_ADC(void)
{
    ADCCFG = 0x18;                  //ADCCFG bit 7(ADLPC) = 0(High speed configuration), 10 bits ADC
    APCTL1 = 0x07;                  //Peter: config AD 0,1,2 to AD Pin
    ADCSC2 = 0x00;
    //ADCSC1 = ADC_VDC | ADC_INT_EN;
    ADCSC1 = ADC_VDC;
}

static void Init_SCI(void)
{
	SCIC2 = 0x2C;					//Enable Rx Interrupt, Transmitter on, Receiver on
	SCIBD = 0x34;                   //Bus freq:8MHz,9600bps
	SCIS1 = 0xC0;
}

