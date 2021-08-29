/**
  @Generated PIC24 / dsPIC33 / PIC32MM MCUs Source File

  @Company:
    Microchip Technology Inc.

  @File Name:
    system.h

  @Summary:
    This is the sysetm.h file generated using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.167.0
        Device            :  dsPIC33EP64GS504
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.50
        MPLAB             :  MPLAB X v5.35
*/

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

// FSEC
#pragma config BWRP = OFF    //Boot Segment Write-Protect bit->Boot Segment may be written
#pragma config BSS = DISABLED    //Boot Segment Code-Protect Level bits->No Protection (other than BWRP)
#pragma config BSEN = OFF    //Boot Segment Control bit->No Boot Segment
#pragma config GWRP = OFF    //General Segment Write-Protect bit->General Segment may be written
#pragma config GSS = DISABLED    //General Segment Code-Protect Level bits->No Protection (other than GWRP)
#pragma config CWRP = OFF    //Configuration Segment Write-Protect bit->Configuration Segment may be written
#pragma config CSS = DISABLED    //Configuration Segment Code-Protect Level bits->No Protection (other than CWRP)
#pragma config AIVTDIS = OFF    //Alternate Interrupt Vector Table bit->Disabled AIVT

// FBSLIM
#pragma config BSLIM = 8191    //Boot Segment Flash Page Address Limit bits->8191

// FOSCSEL
#pragma config FNOSC = FRC    //Oscillator Source Selection->FRC
#pragma config IESO = OFF    //Two-speed Oscillator Start-up Enable bit->Start up with user-selected oscillator source

// FOSC
#pragma config POSCMD = NONE    //Primary Oscillator Mode Select bits->Primary Oscillator disabled
#pragma config OSCIOFNC = ON    //OSC2 Pin Function bit->OSC2 is general purpose digital I/O pin
#pragma config IOL1WAY = ON    //Peripheral pin select configuration bit->Allow only one reconfiguration
#pragma config FCKSM = CSECMD    //Clock Switching Mode bits->Clock switching is enabled,Fail-safe Clock Monitor is disabled
#pragma config PLLKEN = ON    //PLL Lock Enable Bit->Clock switch to PLL source will wait until the PLL lock signal is valid

// FWDT
#pragma config WDTPOST = PS32768    //Watchdog Timer Postscaler bits->1:32768
#pragma config WDTPRE = PR128    //Watchdog Timer Prescaler bit->1:128
#pragma config WDTEN = OFF    //Watchdog Timer Enable bits->WDT and SWDTEN disabled
#pragma config WINDIS = OFF    //Watchdog Timer Window Enable bit->Watchdog Timer in Non-Window mode
#pragma config WDTWIN = WIN25    //Watchdog Timer Window Select bits->WDT Window is 25% of WDT period

// FICD
#pragma config ICS = PGD1    //ICD Communication Channel Select bits->Communicate on PGEC1 and PGED1
#pragma config JTAGEN = OFF    //JTAG Enable bit->JTAG is disabled
#pragma config BTSWP = OFF    //BOOTSWP Instruction Enable/Disable bit->BOOTSWP instruction is disabled

// FDEVOPT
#pragma config PWMLOCK = ON    //PWMx Lock Enable bit->Certain PWM registers may only be written after key sequency
#pragma config ALTI2C1 = OFF    //Alternate I2C1 Pin bit->I2C1 mapped to SDA1/SCL1 pins
#pragma config ALTI2C2 = OFF    //Alternate I2C2 Pin bit->I2C2 mapped to SDA2/SCL2 pins
#pragma config DBCC = OFF    //DACx Output Cross Connection bit->No Cross Connection between DAC outputs

// FALTREG
#pragma config CTXT1 = OFF    //Specifies Interrupt Priority Level (IPL) Associated to Alternate Working Register 1 bits->Not Assigned
#pragma config CTXT2 = OFF    //Specifies Interrupt Priority Level (IPL) Associated to Alternate Working Register 2 bits->Not Assigned

// FBTSEQ
#pragma config BSEQ = 4095    //Relative value defining which partition will be active after device Reset; the partition containing a lower boot number will be active->4095
#pragma config IBSEQ = 0    //The one's complement of BSEQ; must be calculated by the user and written during device programming.->0

// FBOOT
#pragma config BTMODE = SINGLE    //Boot Mode Configuration Bits->Device is in Single Boot (legacy) mode

#include "Include_headers.h"
#include "Dsp_Interrupt.h"


/**
 Section: Driver Interface Function Definitions
*/
TMR_OBJ tmr1_obj;

void PIN_MANAGER_Initialize (void)
{
    /****************************************************************************
     * Setting the Output Latch SFR(s)
     ***************************************************************************/
    LATA = 0x0000;
    LATB = 0x0018;
    LATC = 0x0000;

    /****************************************************************************
     * Setting the GPIO Direction SFR(s)
     ***************************************************************************/
    TRISA = 0x000F;
    TRISB = 0xD7E7;
    TRISC = 0x1BFF;

    /****************************************************************************
     * Setting the Weak Pull Up and Weak Pull Down SFR(s)
     ***************************************************************************/
    CNPDA = 0x0000;
    CNPDB = 0x0000;
    CNPDC = 0x0000;
    CNPUA = 0x0000;
    CNPUB = 0x0000;
    CNPUC = 0x0000;

    /****************************************************************************
     * Setting the Open Drain SFR(s)
     ***************************************************************************/
    ODCA = 0x0000;
    ODCB = 0x0000;
    ODCC = 0x0000;

    /****************************************************************************
     * Setting the Analog/Digital Configuration SFR(s)
     ***************************************************************************/
    ANSELA = 0x0007;
    ANSELB = 0x0627;
    ANSELC = 0x1A37;
    
    /****************************************************************************
     * Set the PPS
     ***************************************************************************/
    __builtin_write_OSCCONL(OSCCON & 0xbf); // unlock PPS

    RPINR18bits.U1RXR = 54; /* Assign U1Rx to pin RP54/RC6 Pin4*/
    RPOR9bits.RP51R = 1; /* Assign U1Tx to pin RP51/RC3 Pin5*/
    /************************** Configure output of analog PFC PWM as fault input **************** */
    RPINR12bits.FLT1R = 177; // RP177 tied to FLT1R PFC A 
    RPOR16bits.RP177R = 0b011001; // RP177 tied to ACMP2
    RPINR12bits.FLT2R = 176; // RP176 tied to FLT2R PFC B
    RPOR16bits.RP176R = 0b011000; // RP176 tied to ACMP1
    
    __builtin_write_OSCCONL(OSCCON | 0x40); // lock PPS
}

void CLOCK_Initialize(void)
{
    // FRCDIV FRC/1; PLLPRE 2; DOZE 1:8; PLLPOST 1:2; DOZEN disabled; ROI disabled; 
    CLKDIV = 0x3000;
    // TUN Center frequency; 
    OSCTUN = 0x00;
    // ROON disabled; ROSEL FOSC; RODIV 0; ROSSLP disabled; 
    REFOCON = 0x00;
    // PLLDIV 63; 
    PLLFBD = 0x3F;
    // ENAPLL enabled; APSTSCLR 1:1; APLLCK disabled; FRCSEL FRC; SELACLK Auxiliary Oscillators; ASRCSEL No clock input; 
    ACLKCON = 0xA740;
    // LFSR 0; 
    LFSR = 0x00;
    // ADCMD enabled; T3MD enabled; T4MD enabled; T1MD enabled; U2MD enabled; T2MD enabled; U1MD enabled; SPI2MD enabled; SPI1MD enabled; PWMMD enabled; T5MD enabled; I2C1MD enabled; 
    PMD1 = 0x00;
    // IC4MD enabled; IC3MD enabled; OC1MD enabled; IC2MD enabled; OC2MD enabled; IC1MD enabled; OC3MD enabled; OC4MD enabled; 
    PMD2 = 0x00;
    // CMPMD enabled; I2C2MD enabled; 
    PMD3 = 0x00;
    // PWM2MD enabled; PWM1MD enabled; PWM4MD enabled; PWM3MD enabled; PWM5MD enabled; 
    PMD6 = 0x00;
    // CMP3MD enabled; CMP4MD enabled; PGA1MD enabled; CMP1MD enabled; CMP2MD enabled; 
    PMD7 = 0x00;
    // CCSMD enabled; PGA2MD enabled; ABGMD enabled; 
    PMD8 = 0x00;
    // CF no clock failure; NOSC FRCPLL; CLKLOCK unlocked; OSWEN Switch is Complete; 
    __builtin_write_OSCCONH((uint8_t) (0x01));
    __builtin_write_OSCCONL((uint8_t) (0x01));
    // Wait for Clock switch to occur
    while (OSCCONbits.OSWEN != 0);
    while (OSCCONbits.LOCK != 1);
}

bool CLOCK_AuxPllLockStatusGet()
{
    return ACLKCONbits.APLLCK;
}

void INTERRUPT_Initialize (void)
{
    //    UERI: UART1 Error
    //    Priority: 1
        IPC16bits.U1EIP = 1;
    //    UTXI: UART1 Transmitter
    //    Priority: 3
        IPC3bits.U1TXIP = 3;
    //    URXI: UART1 Receiver
    //    Priority: 4
        IPC2bits.U1RXIP = 4;
    //    ADCAN3: ADC AN3 Convert Done
    //    Priority: 6
        IPC28bits.ADCAN3IP = 6;
    //    TI: Timer 3
    //    Priority: 1
        IPC2bits.T3IP = 1;
    //    TI: Timer 1
    //    Priority: 1
        IPC0bits.T1IP = 1;
}
void CMP_Initialize(void)
{           
//CMP1
    // DACOE disabled; HYSPOL Rising Edge; ALTINP Comparator; FLTREN disabled; FCLKSEL System Clock; CMPSTAT disabled; EXTREF AVDD; CMPPOL Not Inverted; CMPSIDL disabled; CMPON enabled; HYSSEL No hysteresis; INSEL CMP1B; RANGE AVDD is the maximum DAC output voltage; 
    // Disable the CMP module before the initialization
    CMP1CON = 0x8041 & ~(0x8000);

    // CMREF 2538; 
    CMP1DAC = 0x9EA;
    CMP1CONbits.CMPON = 1;
	
//CMP2

    // DACOE disabled; HYSPOL Rising Edge; ALTINP Comparator; FLTREN disabled; FCLKSEL System Clock; CMPSTAT disabled; EXTREF AVDD; CMPPOL Not Inverted; CMPSIDL disabled; CMPON enabled; HYSSEL No hysteresis; INSEL CMP2A; RANGE AVDD is the maximum DAC output voltage; 
    // Disable the CMP module before the initialization
    CMP2CON = 0x8001 & ~(0x8000);

    // CMREF 3620; 
    CMP2DAC = 0xE24;
    
    /* Set Interrupt priority of Comparator */
    _AC2IP = 5;
    /* Clear Comparator interrupt flag */
    _AC2IF = 0;
    /* Analog Comparator Interrupt Enable */
    _AC2IE = 1;
    CMP2CONbits.CMPON = 1;

	
//CMP3
	// DACOE disabled; HYSPOL Rising Edge; ALTINP Comparator; FLTREN disabled; FCLKSEL System Clock; CMPSTAT disabled; EXTREF AVDD; CMPPOL Not Inverted; CMPSIDL disabled; CMPON enabled; HYSSEL No hysteresis; INSEL CMP3B; RANGE AVDD is the maximum DAC output voltage; 
    // Disable the CMP module before the initialization
    CMP3CON = 0x8041 & ~(0x8000);

    // CMREF 3227; 
    CMP3DAC = 3300; //0xC9B; 450V * 7.314 Value updated in below
    
    /* Set Interrupt priority of Comparator */
    _AC3IP = BULK_OVP_PRIO;
    /* Clear Comparator interrupt flag */
    _AC3IF = 0;
    /* Analog Comparator Interrupt Enable */
    _AC3IE = 1;
    CMP3CONbits.CMPON = 1; 

	    //====================PFC current limit initialization====================
    /* PFC A current limit */
 
    CMP2DAC = PFC_OC_CMP_SET;    // Set over current reference
	CMP3DAC = BULK_OVP_ADC;    // Set over voltage reference

    /* PFC B current limit */
    #if PFC_B_ON_BOARD
    CMP1DAC = PFC_OC_CMP_SET;      // Set over current reference
    #endif
}
void PWM_Initialize (void)
{
    // PCLKDIV 1; 
    PTCON2 = 0x00;
    // SYNCOEN disabled; SEIEN disabled; SESTAT disabled; SEVTPS 1; SYNCSRC SYNCI1; SYNCEN disabled; EIPU disabled; SYNCPOL disabled; 
    STCON = 0x00;
    // PCLKDIV 1; 
    STCON2 = 0x00;
    // STPER 65528; 
    STPER = 0xFFF8;
    // SSEVTCMP 0; 
    SSEVTCMP = 0x00;
    // PTPER 18609; 
    PTPER = 0x48B1;
    // SEVTCMP 0; 
    SEVTCMP = 0x00;
    // MDC 20480; 
    MDC = 0x5000;
    // CHOPCLK 0; CHPCLKEN disabled; 
    CHOP = 0x00;
    // PWMKEY 0; 
    PWMKEY = 0x00;
    // MDCS Primary; FLTIEN disabled; CAM Edge Aligned; DTC Dead-time function is disabled; TRGIEN disabled; XPRES disabled; ITB Master; IUE enabled; CLIEN disabled; MTBS disabled; 
    PWMCON1 = 0x81;
    // MDCS Primary; FLTIEN disabled; CAM Edge Aligned; DTC Dead-time function is disabled; TRGIEN disabled; XPRES disabled; ITB Master; IUE enabled; CLIEN disabled; MTBS disabled; 
    PWMCON2 = 0x81;
    // MDCS Primary; FLTIEN disabled; CAM Edge Aligned; DTC Dead-time function is disabled; TRGIEN disabled; XPRES disabled; ITB Primary; IUE enabled; CLIEN disabled; MTBS disabled; 
    PWMCON3 = 0x281;
    // MDCS Primary; FLTIEN disabled; CAM Edge Aligned; DTC Positive dead time for all Output modes; TRGIEN disabled; XPRES disabled; ITB Master; IUE disabled; CLIEN disabled; MTBS disabled; 
    PWMCON4 = 0x00;
    // MDCS Primary; FLTIEN disabled; CAM Edge Aligned; DTC Positive dead time for all Output modes; TRGIEN disabled; XPRES disabled; ITB Master; IUE disabled; CLIEN disabled; MTBS disabled; 
    PWMCON5 = 0x00;
    //FLTDAT PWM1L Low, PWM1H Low; SWAP disabled; OVRENH disabled; PENL disabled; PMOD True Independent Output Mode; OVRENL disabled; OSYNC disabled; POLL disabled; PENH enabled; CLDAT PWM1L Low, PWM1H Low; OVRDAT PWM1L Low, PWM1H Low; POLH disabled; 
    __builtin_write_PWMSFR(&IOCON1, 0x8C00, &PWMKEY);
    //FLTDAT PWM2L Low, PWM2H Low; SWAP disabled; OVRENH disabled; PENL disabled; PMOD True Independent Output Mode; OVRENL disabled; OSYNC disabled; POLL disabled; PENH enabled; CLDAT PWM2L Low, PWM2H Low; OVRDAT PWM2L Low, PWM2H Low; POLH disabled; 
    __builtin_write_PWMSFR(&IOCON2, 0x8C00, &PWMKEY);
    //FLTDAT PWM3L Low, PWM3H Low; SWAP disabled; OVRENH disabled; PENL disabled; PMOD True Independent Output Mode; OVRENL disabled; OSYNC disabled; POLL disabled; PENH enabled; CLDAT PWM3L Low, PWM3H Low; OVRDAT PWM3L Low, PWM3H Low; POLH disabled; 
    __builtin_write_PWMSFR(&IOCON3, 0x8C00, &PWMKEY);
    //FLTDAT PWM4L Low, PWM4H Low; SWAP disabled; OVRENH disabled; PENL enabled; PMOD Complementary Output Mode; OVRENL disabled; OSYNC disabled; POLL disabled; PENH enabled; CLDAT PWM4L Low, PWM4H Low; OVRDAT PWM4L Low, PWM4H Low; POLH disabled; 
    __builtin_write_PWMSFR(&IOCON4, 0xC000, &PWMKEY);
    //FLTDAT PWM5L Low, PWM5H Low; SWAP disabled; OVRENH disabled; PENL enabled; PMOD Complementary Output Mode; OVRENL disabled; OSYNC disabled; POLL disabled; PENH enabled; CLDAT PWM5L Low, PWM5H Low; OVRDAT PWM5L Low, PWM5H Low; POLH disabled; 
    __builtin_write_PWMSFR(&IOCON5, 0xC000, &PWMKEY);
    //FLTPOL disabled; CLPOL disabled; CLSRC FLT2; CLMOD enabled; FLTMOD Fault input is disabled; IFLTMOD enabled; FLTSRC FLT2; 
    __builtin_write_PWMSFR(&FCLCON1, 0x8913, &PWMKEY);
    //FLTPOL disabled; CLPOL disabled; CLSRC FLT2; CLMOD enabled; FLTMOD Fault input is disabled; IFLTMOD enabled; FLTSRC FLT2; 
    __builtin_write_PWMSFR(&FCLCON2, 0x8913, &PWMKEY);
    //FLTPOL disabled; CLPOL disabled; CLSRC FLT1; CLMOD enabled; FLTMOD Fault input is disabled; IFLTMOD enabled; FLTSRC FLT1; 
    __builtin_write_PWMSFR(&FCLCON3, 0x850B, &PWMKEY);
    //FLTPOL disabled; CLPOL disabled; CLSRC FLT1; CLMOD disabled; FLTMOD PWM4H, PWM4L pins to FLTDAT values- Latched; IFLTMOD disabled; FLTSRC FLT1; 
    __builtin_write_PWMSFR(&FCLCON4, 0x408, &PWMKEY);
    //FLTPOL disabled; CLPOL disabled; CLSRC FLT1; CLMOD disabled; FLTMOD PWM5H, PWM5L pins to FLTDAT values- Latched; IFLTMOD disabled; FLTSRC FLT1; 
    __builtin_write_PWMSFR(&FCLCON5, 0x408, &PWMKEY);
    // PDC1 4096; 
    PDC1 = 0;
    // PDC2 2048; 
    PDC2 = 0;
    // PDC3 0; 
    PDC3 = 0x00;
    // PDC4 0; 
    PDC4 = 0x00;
    // PDC5 0; 
    PDC5 = 0x00;
    // PHASE1 0; 
    PHASE1 = 0x00;
    // PHASE2 0; 
    PHASE2 = 0x00;
    // PHASE3 0; 
    PHASE3 = 0x00;
    // PHASE4 0; 
    PHASE4 = 0x00;
    // PHASE5 0; 
    PHASE5 = 0x00;
    IOCON1bits.FLTDAT = 0b00;
    FCLCON1 = 0x850B; // PWM Fault Current-Limit Control Register    
    FCLCON1bits.FLTMOD   = 0b01;      //Cycle by cycle 
    /* detail of FCLCON1
    FCLCON1bits.IFLTMOD  = 1;  // Independent Fault Mode, current limit input maps FLTDAT<1> to PWMH,fault input maps FLTDAT<0> to PWML
    FCLCON1bits.CLSRC    = 0b00001;  // current limit source select Fault 1
    FCLCON1bits.CLPOL    = 0;  // The selected current-limit source is active-high
    FCLCON1bits.CLMOD    = 1;  // Current-Limit mode is enabled
    FCLCON1bits.FLTSRC    = 0b00001;  // Fault source select Fault 1
    FCLCON1bits.FLTPOL    = 0;  // The selected fault source is active-high
    FCLCON1bits.FLTMOD   = 0b11;  // Fault input is disabled
     */
    IOCON2bits.FLTDAT = 0b00;
    FCLCON2 = 0x8913; // PWM Fault Current-Limit Control Register
    FCLCON2bits.FLTMOD   = 0b01;
  /* detail of FCLCON2
  FCLCON2bits.IFLTMOD  = 1;         // Independent Fault Mode, current limit input maps FLTDAT<1> to PWMH,fault input maps FLTDAT<0> to PWML
  FCLCON2bits.CLSRC    = 0b00010;   // Current limit source select Fault 2
  FCLCON2bits.CLPOL    = 0;         // The selected current-limit source is active-high
  FCLCON2bits.CLMOD    = 1;         // Current-Limit mode is enabled
  FCLCON2bits.FLTSRC    = 0b00010;  // Fault source select Fault 2
  FCLCON2bits.FLTPOL    = 0;        // The selected fault source is active-high
  FCLCON2bits.FLTMOD   = 0b11;      // Fault input is disabled
   */
    // DTR1 0; 
    DTR1 = 0x00;
    // DTR2 0; 
    DTR2 = 0x00;
    // DTR3 0; 
    DTR3 = 0x00;
    // DTR4 0; 
    DTR4 = 0x00;
    // DTR5 0; 
    DTR5 = 0x00;
    // ALTDTR1 0; 
    ALTDTR1 = 0x00;
    // ALTDTR2 0; 
    ALTDTR2 = 0x00;
    // ALTDTR3 0; 
    ALTDTR3 = 0x00;
    // ALTDTR4 0; 
    ALTDTR4 = 0x00;
    // ALTDTR5 0; 
    ALTDTR5 = 0x00;
    // SDC1 0; 
    SDC1 = 0x00;
    // SDC2 0; 
    SDC2 = 0x00;
    // SDC3 0; 
    SDC3 = 0x00;
    // SDC4 0; 
    SDC4 = 0x00;
    // SDC5 0; 
    SDC5 = 0x00;
    // SPHASE1 0; 
    SPHASE1 = 0x00;
    // SPHASE2 0; 
    SPHASE2 = 0x00;
    // SPHASE3 0; 
    SPHASE3 = 0x00;
    // SPHASE4 0; 
    SPHASE4 = 0x00;
    // SPHASE5 0; 
    SPHASE5 = 0x00;
    // TRGCMP 0; 
    TRIG1 = 0x00;
    // TRGCMP 0; 
    TRIG2 = 0x00;
    // TRGCMP 0; 
    TRIG3 = 0x00;
    // TRGCMP 0; 
    TRIG4 = 0x00;
    // TRGCMP 0; 
    TRIG5 = 0x00;
    // TRGDIV 2; TRGSTRT 0; DTM disabled; 
    TRGCON1 = 0x1000;
    // TRGDIV 2; TRGSTRT 0; DTM disabled; 
    TRGCON2 = 0x1000;
    // TRGDIV 2; TRGSTRT 0; DTM disabled; 
    TRGCON3 = 0x1000;
    // TRGDIV 1; TRGSTRT 0; DTM disabled; 
    TRGCON4 = 0x00;
    // TRGDIV 1; TRGSTRT 0; DTM disabled; 
    TRGCON5 = 0x00;
    // STRGCMP 0; 
    STRIG1 = 0x00;
    // STRGCMP 0; 
    STRIG2 = 0x00;
    // STRGCMP 0; 
    STRIG3 = 0x00;
    // STRGCMP 0; 
    STRIG4 = 0x00;
    // STRGCMP 0; 
    STRIG5 = 0x00;
    // PWMCAP 0; 
    PWMCAP1 = 0x00;
    // PWMCAP 0; 
    PWMCAP2 = 0x00;
    // PWMCAP 0; 
    PWMCAP3 = 0x00;
    // PWMCAP 0; 
    PWMCAP4 = 0x00;
    // PWMCAP 0; 
    PWMCAP5 = 0x00;
    // BPLL disabled; BPHH enabled; BPLH disabled; BCH enabled; FLTLEBEN disabled; PLR disabled; CLLEBEN enabled; BCL disabled; PLF disabled; PHR enabled; BPHL disabled; PHF disabled; 
    LEBCON1 = 0x8428;
    // BPLL disabled; BPHH enabled; BPLH disabled; BCH enabled; FLTLEBEN disabled; PLR disabled; CLLEBEN enabled; BCL disabled; PLF disabled; PHR enabled; BPHL disabled; PHF disabled; 
    LEBCON2 = 0x8428;
    // BPLL disabled; BPHH enabled; BPLH disabled; BCH enabled; FLTLEBEN disabled; PLR disabled; CLLEBEN enabled; BCL disabled; PLF disabled; PHR enabled; BPHL disabled; PHF disabled; 
    LEBCON3 = 0x8428;
    // BPLL disabled; BPHH disabled; BPLH disabled; BCH disabled; FLTLEBEN disabled; PLR disabled; CLLEBEN disabled; BCL disabled; PLF disabled; PHR disabled; BPHL disabled; PHF disabled; 
    LEBCON4 = 0x00;
    // BPLL disabled; BPHH disabled; BPLH disabled; BCH disabled; FLTLEBEN disabled; PLR disabled; CLLEBEN disabled; BCL disabled; PLF disabled; PHR disabled; BPHL disabled; PHF disabled; 
    LEBCON5 = 0x00;
    // LEB 0; 
    LEBDLY1 = 0x00;
    // LEB 0; 
    LEBDLY2 = 0x00;
    // LEB 0; 
    LEBDLY3 = 0x00;
    // LEB 0; 
    LEBDLY4 = 0x00;
    // LEB 0; 
    LEBDLY5 = 0x00;
    // CHOPLEN disabled; HRDDIS disabled; CHOPHEN disabled; BLANKSEL No state blanking; CHOPSEL No state blanking; HRPDIS disabled; 
    AUXCON1 = 0x00;
    // CHOPLEN disabled; HRDDIS disabled; CHOPHEN disabled; BLANKSEL No state blanking; CHOPSEL No state blanking; HRPDIS disabled; 
    AUXCON2 = 0x00;
    // CHOPLEN disabled; HRDDIS disabled; CHOPHEN disabled; BLANKSEL No state blanking; CHOPSEL No state blanking; HRPDIS disabled; 
    AUXCON3 = 0x00;
    // CHOPLEN disabled; HRDDIS disabled; CHOPHEN disabled; BLANKSEL No state blanking; CHOPSEL No state blanking; HRPDIS disabled; 
    AUXCON4 = 0x00;
    // CHOPLEN disabled; HRDDIS disabled; CHOPHEN disabled; BLANKSEL No state blanking; CHOPSEL No state blanking; HRPDIS disabled; 
    AUXCON5 = 0x00;

    // SYNCOEN disabled; SEIEN disabled; SESTAT disabled; SEVTPS 1; SYNCSRC SYNCI1; SYNCEN disabled; PTSIDL disabled; PTEN enabled; EIPU disabled; SYNCPOL disabled; 
    PTCON = 0x8000;
}

void TMR1_Initialize (void)
{
    //TMR1 0; 
    TMR1 = 0x00;
    //Period = 0.0001999958 s; Frequency = 59881250 Hz; PR1 11975; 
    PR1 = 0x2EC7;
    //TCKPS 1:1; TON enabled; TSIDL disabled; TCS FOSC/2; TSYNC disabled; TGATE disabled; 
    T1CON = 0x8000;

    Init_Timer1_Callback();

    IFS0bits.T1IF = false;
    IEC0bits.T1IE = true;
	
    tmr1_obj.timerElapsed = false;
}

void ADC1_Initialize (void)
{
    // ADSIDL disabled; ADON enabled; 
    ADCON1L = (0x8000 & 0x7FFF); //Disabling ADON bit
    // FORM Integer; SHRRES 12-bit resolution; 
    ADCON1H = 0x60;
    // SHRADCS 4; REFCIE disabled; SHREISEL Early interrupt is generated 1 TADCORE clock prior to data being ready; REFERCIE disabled; EIEN disabled; 
    ADCON2L = 0x02;
    // SHRSAMC 1; 
    ADCON2H = 0x01;
    // SWCTRG disabled; SHRSAMP disabled; SUSPEND disabled; SWLCTRG disabled; SUSPCIE disabled; CNVCHSEL AN0; REFSEL disabled; 
    ADCON3L = 0x00;
    // SHREN enabled; C3EN enabled; C2EN disabled; C1EN disabled; C0EN enabled; CLKDIV 1; CLKSEL APLL; 
    ADCON3H = (0xC089 & 0xFF00); //Disabling C0EN, C1EN, C2EN, C3EN and SHREN bits
    // SAMC3EN disabled; SYNCTRG3 disabled; SAMC0EN disabled; SYNCTRG2 disabled; SAMC1EN disabled; SAMC2EN disabled; SYNCTRG1 disabled; SYNCTRG0 disabled; 
    ADCON4L = 0x00;
    // C3CHS AN3; C0CHS AN0; C1CHS AN1; C2CHS AN2; 
    ADCON4H = 0x00;
    // SIGN0 disabled; SIGN4 disabled; SIGN3 disabled; SIGN2 disabled; SIGN1 disabled; SIGN7 disabled; SIGN6 disabled; DIFF0 disabled; SIGN5 disabled; DIFF1 disabled; DIFF2 disabled; DIFF3 disabled; DIFF4 disabled; DIFF5 disabled; DIFF6 disabled; DIFF7 disabled; 
    ADMOD0L = 0x00;
    // DIFF8 disabled; DIFF9 disabled; SIGN10 disabled; SIGN11 disabled; SIGN12 disabled; DIFF14 disabled; SIGN8 disabled; SIGN14 disabled; DIFF12 disabled; DIFF11 disabled; DIFF10 disabled; SIGN9 disabled; 
    ADMOD0H = 0x00;
    // DIFF19 disabled; DIFF18 disabled; SIGN20 disabled; DIFF17 disabled; SIGN21 disabled; SIGN17 disabled; DIFF21 disabled; SIGN18 disabled; DIFF20 disabled; SIGN19 disabled; 
    ADMOD1L = 0x00;
    // IE1 disabled; IE0 enabled; IE3 enabled; IE2 disabled; IE5 disabled; IE4 enabled; IE10 disabled; IE7 disabled; IE6 disabled; IE9 disabled; IE8 disabled; IE14 enabled; IE11 disabled; IE12 enabled; 
    ADIEL = 0x5019;
    // IE17 disabled; IE18 disabled; IE19 disabled; IE20 disabled; IE21 disabled; 
    ADIEH = 0x00;
    // CMPEN10 disabled; CMPEN11 disabled; CMPEN6 disabled; CMPEN5 disabled; CMPEN4 disabled; CMPEN3 disabled; CMPEN2 disabled; CMPEN1 disabled; CMPEN0 disabled; CMPEN14 disabled; CMPEN9 disabled; CMPEN8 disabled; CMPEN12 disabled; CMPEN7 disabled; 
    ADCMP0ENL = 0x00;
    // CMPEN10 disabled; CMPEN11 disabled; CMPEN6 disabled; CMPEN5 disabled; CMPEN4 disabled; CMPEN3 disabled; CMPEN2 disabled; CMPEN1 disabled; CMPEN0 disabled; CMPEN14 disabled; CMPEN9 disabled; CMPEN8 disabled; CMPEN12 disabled; CMPEN7 disabled; 
    ADCMP1ENL = 0x00;
    // CMPEN21 disabled; CMPEN20 disabled; CMPEN18 disabled; CMPEN19 disabled; CMPEN17 disabled; 
    ADCMP0ENH = 0x00;
    // CMPEN21 disabled; CMPEN20 disabled; CMPEN18 disabled; CMPEN19 disabled; CMPEN17 disabled; 
    ADCMP1ENH = 0x00;
    // CMPLO 0; 
    ADCMP0LO = 0x00;
    // CMPLO 0; 
    ADCMP1LO = 0x00;
    // CMPHI 0; 
    ADCMP0HI = 0x00;
    // CMPHI 0; 
    ADCMP1HI = 0x00;
    // OVRSAM 4x; MODE Oversampling Mode; FLCHSEL AN0; IE disabled; FLEN disabled; 
    ADFL0CON = 0x400;
    // OVRSAM 4x; MODE Oversampling Mode; FLCHSEL AN0; IE disabled; FLEN disabled; 
    ADFL1CON = 0x400;
    // HIHI disabled; LOLO disabled; HILO disabled; BTWN disabled; LOHI disabled; CMPEN disabled; IE disabled; 
    ADCMP0CON = 0x00;
    // HIHI disabled; LOLO disabled; HILO disabled; BTWN disabled; LOHI disabled; CMPEN disabled; IE disabled; 
    ADCMP1CON = 0x00;
    // LVLEN9 disabled; LVLEN8 disabled; LVLEN11 disabled; LVLEN7 disabled; LVLEN10 disabled; LVLEN6 disabled; LVLEN5 disabled; LVLEN12 disabled; LVLEN4 disabled; LVLEN3 disabled; LVLEN14 disabled; LVLEN2 disabled; LVLEN1 disabled; LVLEN0 disabled; 
    ADLVLTRGL = 0x00;
    // LVLEN20 disabled; LVLEN21 disabled; LVLEN17 disabled; LVLEN19 disabled; LVLEN18 disabled; 
    ADLVLTRGH = 0x00;
    // SAMC 0; 
    ADCORE0L = 0x00;
    // SAMC 0; 
    ADCORE1L = 0x00;
    // SAMC 0; 
    ADCORE2L = 0x00;
    // SAMC 0; 
    ADCORE3L = 0x00;
    // RES 12-bit resolution; EISEL Early interrupt is generated 1 TADCORE clock prior to data being ready; ADCS 4; 
    ADCORE0H = 0x302;
    // RES 12-bit resolution; EISEL Early interrupt is generated 1 TADCORE clock prior to data being ready; ADCS 4; 
    ADCORE1H = 0x302;
    // RES 12-bit resolution; EISEL Early interrupt is generated 1 TADCORE clock prior to data being ready; ADCS 4; 
    ADCORE2H = 0x302;
    // RES 12-bit resolution; EISEL Early interrupt is generated 1 TADCORE clock prior to data being ready; ADCS 4; 
    ADCORE3H = 0x302;
    // EIEN9 disabled; EIEN7 disabled; EIEN8 disabled; EIEN5 disabled; EIEN6 disabled; EIEN3 disabled; EIEN4 disabled; EIEN1 disabled; EIEN2 disabled; EIEN0 disabled; EIEN12 disabled; EIEN11 disabled; EIEN10 disabled; EIEN14 disabled; 
    ADEIEL = 0x00;
    // EIEN17 disabled; EIEN19 disabled; EIEN18 disabled; EIEN20 disabled; EIEN21 disabled; 
    ADEIEH = 0x00;
    // C0CIE disabled; C1CIE disabled; C2CIE disabled; C3CIE disabled; SHRCIE disabled; WARMTIME 32768 Source Clock Periods; 
    ADCON5H = (0xF00 & 0xF0FF); //Disabling WARMTIME bit
	
    //Assign Default Callbacks
    Init_ADC_Callback();
    
    // Clearing I_PFC_CT_A interrupt flag.
    IFS7bits.ADCAN3IF = 0;
    // Enabling I_PFC_CT_A interrupt.
    IEC7bits.ADCAN3IE = 1;

    // Setting WARMTIME bit
    ADCON5Hbits.WARMTIME = 0xF;
    // Enabling ADC Module
    ADCON1Lbits.ADON = 0x1;
    // Enabling Power for the Shared Core
    ADC1_SharedCorePowerEnable();
    // Calibrating the Shared Core
    ADC1_SharedCoreCalibration();
    // Enabling Power for Core0
    ADC1_Core0PowerEnable();
    // Calibrating Core0
    ADC1_Core0Calibration();
    // Enabling Power for Core3
    ADC1_Core3PowerEnable();
    // Calibrating Core3
    ADC1_Core3Calibration();

    //TRGSRC0 PWM2 Primary Trigger; TRGSRC1 None; 
    ADTRIG0L = 0x06;
    //TRGSRC3 PWM1 Primary Trigger; TRGSRC2 None; 
    ADTRIG0H = 0x500;
    //TRGSRC4 PWM1 Primary Trigger; TRGSRC5 None; 
    ADTRIG1L = 0x05;
    //TRGSRC6 None; TRGSRC7 None; 
    ADTRIG1H = 0x0505;
    //TRGSRC8 None; TRGSRC9 None; 
    ADTRIG2L = 0xC0C;
    //TRGSRC11 None; TRGSRC10 None; 
    ADTRIG2H = 0x00;
    //TRGSRC12 PWM1 Primary Trigger; 
    ADTRIG3L = 0x05;
    //TRGSRC14 PWM1 Primary Trigger; 
    ADTRIG3H = 0x05;
    //TRGSRC17 None; 
    ADTRIG4L = 0x00;
    //TRGSRC19 None; TRGSRC18 None; 
    ADTRIG4H = 0xC00;
    //TRGSRC20 None; TRGSRC21 None; 
    ADTRIG5L = 0x00;

}


void ADC1_Core0PowerEnable ( ) 
{
    ADCON5Lbits.C0PWR = 1; 
    while(ADCON5Lbits.C0RDY == 0);
    ADCON3Hbits.C0EN = 1;     
}

void ADC1_Core0Calibration ( ) 
{   
    ADCAL0Lbits.CAL0EN = 1;   
    ADCAL0Lbits.CAL0DIFF = 0;
    ADCAL0Lbits.CAL0RUN = 1;   
    while(ADCAL0Lbits.CAL0RDY == 0);
    ADCAL0Lbits.CAL0EN = 0;
}

void ADC1_Core3PowerEnable ( ) 
{
    ADCON5Lbits.C3PWR = 1; 
    while(ADCON5Lbits.C3RDY == 0);
    ADCON3Hbits.C3EN = 1;     
}

void ADC1_Core3Calibration ( ) 
{   
    ADCAL0Hbits.CAL3EN = 1;   
    ADCAL0Hbits.CAL3DIFF = 0;
    ADCAL0Hbits.CAL3RUN = 1;   
    while(ADCAL0Hbits.CAL3RDY == 0);
    ADCAL0Hbits.CAL3EN = 0;
}

void ADC1_SharedCorePowerEnable ( ) 
{
    ADCON5Lbits.SHRPWR = 1;   
    while(ADCON5Lbits.SHRRDY == 0);
    ADCON3Hbits.SHREN = 1;   
}

void ADC1_SharedCoreCalibration ( ) 
{
    ADCAL1Hbits.CSHREN = 1;   
    ADCAL1Hbits.CSHRDIFF = 0;
    ADCAL1Hbits.CSHRRUN = 1;   
    while(ADCAL1Hbits.CSHRRDY == 0);
    ADCAL1Hbits.CSHREN = 0;   
}
/**
 * Sets the CPU core control register operating mode to a value that is decided by the
 * SYSTEM_CORCON_MODES argument.
 * @param modeValue SYSTEM_CORCON_MODES initialization mode specifier
 * @example
 * <code>
 * SYSTEM_CORCONModeOperatingSet(CORCON_MODE_ENABLEALLSATNORMAL_ROUNDUNBIASED);
 * </code>
 */
inline static void SYSTEM_CORCONModeOperatingSet(SYSTEM_CORCON_MODES modeValue)
{
    CORCON = (CORCON & 0x00F2) | modeValue;
}

inline static void INTERRUPT_GlobalEnable(void)
{
    __builtin_enable_interrupts();
}

inline static void INTERRUPT_GlobalDisable(void)
{
    __builtin_disable_interrupts();
}

void SYSTEM_Initialize(void)
{
    PIN_MANAGER_Initialize();
    CLOCK_Initialize();
    INTERRUPT_Initialize();
    CMP_Initialize();
    PWM_Initialize();
    ADC1_Initialize();
    TMR1_Initialize();
    INTERRUPT_GlobalEnable();
    SYSTEM_CORCONModeOperatingSet(CORCON_MODE_PORVALUES);
}

/**
 End of File
*/

