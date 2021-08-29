
/*****************************************************************************************
 *
 *Copyright (C) 2012 Advanced Technology Development
 *                   Power SBG
 *                   LITE-ON TECHNOLOGY Corp.
 *
 *All Rights Reserved
 *
 *File Name : Primary_traps.c
 *
 *Date : 2012.05.24
 *
 *Author :
 *
 *Description :This Program used for HP 2400W Primary_traps Program.
 *
 *******************************************************************************************/
#ifdef __dsPIC30F__
#include "p30fxxxx.h"
#endif

#ifdef __dsPIC33F__
#include "p33Fxxxx.h"
#endif

#ifdef __PIC24F__
#include "p24Fxxxx.h"
#endif

#ifdef __PIC24H__
#include "p24Hxxxx.h"
#endif

/* ****************************************************************
 * Standard Exception Vector handlers if ALTIVT (INTCON2<15>) = 0  *
 * Not required for labs but good to always include                *
 ******************************************************************/
void __attribute__ ( ( __interrupt__, no_auto_psv ) ) _OscillatorFail ( void )
{
  INTCON1bits.OSCFAIL = 0;
  PTCONbits.PTEN = 0;
  while ( 1 );
}
//------------------------------------------------------------------

void __attribute__ ( ( __interrupt__, no_auto_psv ) )  _AddressError ( void )
{
  INTCON1bits.ADDRERR = 0;
  PTCONbits.PTEN = 0;
  while ( 1 );
}
//------------------------------------------------------------------

void __attribute__ ( ( __interrupt__, no_auto_psv ) )  _StackError ( void )
{
  INTCON1bits.STKERR = 0;
  PTCONbits.PTEN = 0;

  IOCON1bits.PENH = 0;
  IOCON1bits.PENL = 0;
  IOCON2bits.PENH = 0;
  IOCON2bits.PENL = 0;

  while ( 1 );
}
//------------------------------------------------------------------

void __attribute__ ( ( __interrupt__, no_auto_psv ) ) _ISR _MathError ( void )
{
  INTCON1bits.MATHERR = 0;
  PTCONbits.PTEN = 0;
  while ( 1 );
}

/* ****************************************************************
 * Alternate Exception Vector handlers if ALTIVT (INTCON2<15>) = 1 *
 * Not required for labs but good to always include                *
 ******************************************************************/
void __attribute__ ( ( __interrupt__, no_auto_psv ) )  _AltOscillatorFail ( void )
{
  INTCON1bits.OSCFAIL = 0;
  while ( 1 );
}
//------------------------------------------------------------------

void __attribute__ ( ( __interrupt__, no_auto_psv ) ) _AltAddressError ( void )
{
  INTCON1bits.ADDRERR = 0;
  while ( 1 );
}
//------------------------------------------------------------------

void __attribute__ ( ( __interrupt__, no_auto_psv ) ) _AltStackError ( void )
{
  INTCON1bits.STKERR = 0;
  while ( 1 );
}
//------------------------------------------------------------------

void __attribute__ ( ( __interrupt__, no_auto_psv ) ) _AltMathError ( void )
{
  INTCON1bits.MATHERR = 0;
  while ( 1 );
}






