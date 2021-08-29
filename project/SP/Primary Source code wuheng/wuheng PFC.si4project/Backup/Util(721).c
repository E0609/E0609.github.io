/*****************************************

FILE: Util.c
COMPANY: Liteon Technology, Inc.
PROCESSOR: MC9S08QG4
PURPOSE: Utility functions
HISTORY:

DATE        REVISION    DESCRIPTION
-----------------------------------
2008/12/12  v0.1        Created

*****************************************/
#include <MC9S08QG4.h>
#include <math.h>
#include "Config.h"
#include "main.h"
#include "Util.h"
#include "RMSCalculation.h"
/*************************************
    Internal Function
*************************************/
//static byte Sqrt_16(unsigned int M);  //[davidchchen]20161201 removed,not used
//static byte MySqrt(word N);           //[davidchchen]20161201 removed,not used

/*************************************
    Implement Begin
*************************************/

/*====================================
Get Vac and also set frequency counter
====================================*/

void Get_Vac(word Vac, word* Vpeak, word* VpeakAvg, byte* Frequency, byte bReset)
{
	static byte VacLCounter = 0;
	static word v_peak = 0;
	static word vpeak_LPF = 0;
	static word vpeak_FF = 0;

	if(bReset)
	{
		v_peak = 0;
		vpeak_LPF = 0;
		vpeak_FF = 0;
		*Vpeak = 0;
		*VpeakAvg = 0;
		
		sRMSCal.u16AccCount = 0;                    //[davidchchen]20161219 Clear RMS ac cycle.
    sRMSCal.u32VolAccValue = 0;                 //[davidchchen]20161219 Clear RMS ac cycle.
    sRMSCal.u16VolValue = 0;                    //[davidchchen]20161219 Clear RMS ac cycle.
    sRMSCal.u16PreData = 0;                     //[davidchchen]20161219 Clear RMS ac cycle.
    sRMSCal.u8SampleStartCnt = 0;               //[davidchchen]20161219 Clear RMS ac cycle.
    sRMSCal.u8SampleStopCnt = 0;                //[davidchchen]20161219 Clear RMS ac cycle.                                                
    sRMSCal.u8SampleDoneCnt = SAMPLE_CYCLE_CNT; //[davidchchen]20161219 Clear RMS ac cycle.
    sRMSCal.nRMSCalFlag.u8All = 0;              //[davidchchen]20161219 Clear RMS ac cycle.
	  VacLCounter = 0;                            //[davidchchen]20161219 Clear Peak ac cycle counter
	}

	VacLCounter++;

	if(Vac >= v_peak) v_peak = Vac;

	if(VacLCounter >= 72)	// 166us* counter = ~12ms, //[davidchchen]20161202 note-> 166us*73=12.118ms
	{
		vpeak_LPF = vpeak_LPF + v_peak - vpeak_FF;
		vpeak_FF = vpeak_LPF >> 1;
	
		*VpeakAvg = vpeak_FF;
		*Vpeak = v_peak;
		
		v_peak = 0;
		VacLCounter = 0;
	}
	
	*Frequency = 1;	//dummy
}

/*====================================
Get Average value
====================================*/

void GetAverage(tADC* AD_DataSet)
{
	(*AD_DataSet).LPF = (*AD_DataSet).LPF + (*AD_DataSet).AD - (*AD_DataSet).FF;
	(*AD_DataSet).FF = (*AD_DataSet).LPF >> (*AD_DataSet).exp;
}

/*====================================
TODO: need to be checked    
====================================*/
/*
static byte Sqrt_16(unsigned int M)
{
    byte  h_temp;
	unsigned int temp01,temp02;		
	
	temp01=M>>1;
	for(h_temp=0;h_temp<15;h_temp++)
	{	    	
		temp02=M/temp01;
		temp01=temp01+temp02;
		temp01=temp01>>1;
	}
	return (byte)temp01;
}
*/

/*
static byte MySqrt(word N)
{
    word Result = 0;
    
    while (N > 0) 
    {
        N -= ((Result << 1) + 1);
        ++Result;
    }

    return Result - 1;
}
*/
/*====================================
LinearFormatDecode
====================================*/

/*====================================
rms
====================================*/
/*
word isqrt(dword val)
{
  byte  bshft = 15;
  word  g = 0, b = 0x8000;
  dword temp = 0; 

  do
  {
    temp = ((g << 1) + b);              // calc root data
    temp = (temp << bshft);             // shift data
        
    bshft--;                            // dec shift counter
     
    if (val >= temp)                    //
    {                                   //
       g   += b;                        //
       val -= temp;                     //
    }
    
  } while (b >>= 1);                    // loop while > 0

  return(g);                            // return the value
}
*/
  
