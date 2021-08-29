/*****************************************

FILE: main.c
COMPANY: Liteon Technology, Inc.
PROCESSOR: MC9S08QG4
PURPOSE: Entry point
HISTORY:

DATE        REVISION    DESCRIPTION
-----------------------------------
2008/12/12  v0.0.1        Created

==========================================
Code Convention:

1. Prefix: "g" represents global variable    
2. Tab width: 4

*****************************************/
#include <hidef.h>                            
#include <MC9S08QG4.h>
#include <stdio.h>
#include <stdlib.h>

#include "Init.h"
#include "Config.h"
#include "Util.h"
#include "main.h"
#include "SCI.h"
#include "Timer.h"
#include "RMSCalculation.h"       //[davidchchen]20161219 Added Vrms Calculation.

/*************************************
    Internal Function
*************************************/
static void ProgramInit(void);

static void Do_StateInit(void);
static void Do_StatePFCNotOK(void);
static void Do_StatePFCOK(void);
static void Do_StateACNotOK(void);
static void Do_StateACOK(void);
//static void Do_StateACOff(void);

static void CheckVinSD(void);
static void CheckPFCDrop(void);
static void CheckPFCIC(void);

/*************************************
    Global Variable
*************************************/
volatile PSU gPSU;
//volatile word SysTimerCount = 0;
static byte counter1 = 1, counter2 = 3, counter3 = 5;

/*************************************
    Extern Function
*************************************/
extern void GetAverage(tADC* AD_DataSet);

/*====================================
    ADC update
====================================*/
//[vincent.liu]20161202 Removed
/*
interrupt VectorNumber_Vadc void ISR_ADC(void)
{

   unsigned char adcCH = (ADCSC1 & 0x1F);
    
        switch(adcCH)
        {
            case ADC_VDC:
				          gPSU.ADC.Vdc.AD = ADCR;
				          GetAverage(&gPSU.ADC.Vdc);
                  ADCSC1 = ADC_VAC_RECT | ADC_INT_EN;
                break;
            
            case ADC_VAC_RECT:
                gPSU.ADC.Vac.AD = ADCR;
				        GetAverage(&gPSU.ADC.Vac);
				        //gPSU.ADC.Vrms.AD = gPSU.ADC.Vac.AD;
				        //GetAverage(&gPSU.ADC.Vrms);
                ADCSC1 = ADC_TEMPERATURE1 | ADC_INT_EN;            
                break;

            case ADC_TEMPERATURE1:
                gPSU.ADC.Temperature.AD = ADCR;
				        GetAverage(&gPSU.ADC.Temperature);
                ADCSC1 = ADC_VDC | ADC_INT_EN;
                break;

            default:
				        ADCSC1 = ADC_VDC | ADC_INT_EN;
                break;
        }

}
*/

void InitPSU_Data()
{
	/*Iac*/
	#if 0
	gPSU.ADC.Iac.AD = 0;
	gPSU.ADC.Iac.FF = 0;
	gPSU.ADC.Iac.LPF = 0;
	gPSU.ADC.Iac.exp = 2;
	#endif
	
	/*Vac*/
	gPSU.ADC.Vac.AD = 0;
	gPSU.ADC.Vac.FF = 0;
	gPSU.ADC.Vac.LPF = 0;
	gPSU.ADC.Vac.exp = 2;

	/*Vdc*/
	gPSU.ADC.Vdc.AD = 0;
	gPSU.ADC.Vdc.FF = 0;
	gPSU.ADC.Vdc.LPF = 0;
	gPSU.ADC.Vdc.exp = 2;

	/*Temperature*/
	gPSU.ADC.Temperature.AD = 0;
	gPSU.ADC.Temperature.FF = 0;
	gPSU.ADC.Temperature.LPF = 0;
	gPSU.ADC.Temperature.exp = 2;

	/*Vpeak*/
	gPSU.ADC.Vpeak = 0;

	/*Vrms*/
	#if 0
	gPSU.ADC.Vrms.AD = 0;
	gPSU.ADC.Vrms.FF = 0;
	gPSU.ADC.Vrms.LPF = 0;
	gPSU.ADC.Vrms.exp = 10;	
	#endif

	/*Frequency*/
	gPSU.Frequency = 0;

	/*State*/
	gPSU.State = STATE_INIT;

	/*SyStatus*/
	gPSU.SyStatus.Val = 0;

	/*PyStatus*/
	gPSU.PyStatus.Val = 0;

	gPSU.PFCNOK_Ref = PFC_NOTOK_REF_LL;
	
}

/*====================================
    Program Init
====================================*/
static void ProgramInit(void)
{
  Initialize(); /*MCU init*/

  SetFWVersion();

	InitPSU_Data();

	EnableInterrupts;		
}

/*====================================
    PFCAdjust
====================================*/
static void PFCAdjust(void)
{
	word iout_reading;

	iout_reading = gSyData[SY_CMD_IOUT];

	if((gPSU.SyStatus.bit.STB == TRUE) || (AC_OK_CTRL == AC_NOTOK))
	{
		PFC_ADJUST_PIN = PFC_ADJUST_ON;

		ResetCounter(COUNTER_PFC_ADJUST_OFF);         //[davidchchen]20161202 note->1000ms
		ResetCounter(COUNTER_PFC_ADJUST_ON_DELAY);    //[davidchchen]20161202 note->20ms		
	}
	else
	{
		if(iout_reading < LOAD_55P)
		{
			StartDelay(COUNTER_PFC_ADJUST_OFF);         //[davidchchen]20161202 note->1000ms
			if(IsDelay(COUNTER_PFC_ADJUST_OFF))
			{
				StopDelay(COUNTER_PFC_ADJUST_OFF);
				gPSU.PFCNOK_Ref = PFC_NOTOK_REF_LL;
				PFC_ADJUST_PIN = PFC_ADJUST_OFF;
			}

			ResetCounter(COUNTER_PFC_ADJUST_ON_DELAY);  //[davidchchen]20161202 note->20ms
		}
		else if(iout_reading > LOAD_60P)
		{
			PFC_ADJUST_PIN = PFC_ADJUST_ON;

			/*Change the PFC_NOK REF*/
			StartDelay(COUNTER_PFC_ADJUST_ON_DELAY);    //[davidchchen]20161202 note->20ms
			if(IsDelay(COUNTER_PFC_ADJUST_ON_DELAY))
			{
				StopDelay(COUNTER_PFC_ADJUST_ON_DELAY);
				gPSU.PFCNOK_Ref = PFC_NOTOK_REF_HL;
			}

			ResetCounter(COUNTER_PFC_ADJUST_OFF);       //[davidchchen]20161202 note->1000ms
		}
		else
		{
			ResetCounter(COUNTER_PFC_ADJUST_OFF);       //[davidchchen]20161202 note->1000ms
			ResetCounter(COUNTER_PFC_ADJUST_ON_DELAY);  //[davidchchen]20161202 note->20ms
		}
	}
} 

/*====================================
    CheckPriOTP
====================================*/
void CheckPriOTP()
{
	// Self OTP if Uart communication got problem 
	if(gPSU.ADC.Temperature.FF <= 371)  // 371 is around 110C, please check the temp_table in sec. side FW source code.
	{		
		StartDelay(COUNTER_OTP_SD);     //[davidchchen]20161202 note->10000ms
		if(IsDelay(COUNTER_OTP_SD))
		{		
			gPSU.PyStatus.bits.PRI_OTP = 1;
		}
	}
	else
	{
		ResetCounter(COUNTER_OTP_SD);  //[davidchchen]20161202 note->10000ms
	}

	// OTP Recover
	if(gPSU.PyStatus.bits.PRI_OTP == 1)
	{
		if(gPSU.ADC.Temperature.FF >= 661)  // 661 is around 60C, please check the temp_table in sec. side FW source code.
		{	
			StartDelay(COUNTER_OTP_RECOVER);   //[davidchchen]20161202 note->10000ms
			if(IsDelay(COUNTER_OTP_RECOVER))
			{
				gPSU.PyStatus.bits.PRI_OTP = 0;
			}
		}
		else
		{
			ResetCounter(COUNTER_OTP_RECOVER);  //[davidchchen]20161202 note->10000ms
		}
	}
}

/*====================================
    CheckVinOV
====================================*/
void CheckVinOV()
{
	word Input_OVF_Limit;
	word Input_OVW_Limit;
	//word Vpeak = gPSU.ADC.Vpeak_Avg;    //[davidchchen]20161219 Removed
	word Vpeak;                           //[davidchchen]20161219 Change to Rms calculation when Input is AC; 	

	if(AC_OK_CTRL == AC_OK)
	{
		if(gPSU.PyStatus.bits.DC_INPUT == 0)
		{
			//Input_OVF_Limit = AC_IN_OVF; //[davidchchen]20161202 note->295VAC 
			//Input_OVW_Limit = AC_IN_OVW; //[davidchchen]20161202 note->290VAC 
			Vpeak = sRMSCal.u16VolValue;    //[davidchchen]20161219 Change to Rms calculation when Input is AC;
			Input_OVF_Limit = ACRMS_IN_OVF; //[davidchchen]20161219 Change to Rms calculation when Input is AC;
			Input_OVW_Limit = ACRMS_IN_OVW; //[davidchchen]20161219 Change to Rms calculation when Input is AC;
			
		}
		else
		{
		  Vpeak = gPSU.ADC.Vpeak_Avg;   //[davidchchen]20161219 Change to Peak calculation when Input is DC;
			Input_OVF_Limit = DC_IN_OVF;  //[davidchchen]20161202 note->305VDC
			Input_OVW_Limit = DC_IN_OVW;  //[davidchchen]20161202 note->300VDC
		}

		if(Vpeak > Input_OVW_Limit)//[davidchchen]20161202 note->290VAC/300VDC
		{
			gPSU.PyStatus.bits.INPUT_OVW = 1;
		}
		else
		{
			gPSU.PyStatus.bits.INPUT_OVW = 0;
		}

		if(Vpeak > Input_OVF_Limit)//[davidchchen]20161202 note->295VAC/305VDC
		{
			StartDelay(COUNTER_VIN_OV); //[davidchchen]20161202 note->72ms
            			            
			if(IsDelay(COUNTER_VIN_OV))
			{
				gPSU.PyStatus.bits.INPUT_OVP = 1;
			}
		}
		else
		{
			ResetCounter(COUNTER_VIN_OV); //[davidchchen]20161202 note->72ms
		}
	}
}

/*====================================
    Do StateInit
====================================*/
static void Do_StateInit(void)
{
    StartDelay(COUNTER_MCU_START);  //[davidchchen]20161202 note->20ms
            			            
    if(IsDelay(COUNTER_MCU_START))
    {
      //StopDelay(COUNTER_MCU_START);
		  gPSU.State = STATE_PFC_NOTOK;                                                
    }
}

/*====================================
    Check PFC IC
====================================*/
static void CheckPFCIC(void)
{   
	/*Prevent update by interrupt*/
	//word VpeakCompare = gPSU.ADC.Vpeak; //[davidchchen]20161219 Removed
	word VpeakCompare;                    //[davidchchen]20161219 Change to Rms calculation when Input is AC;
	word AcNGRef;
  word Temp = (gPSU.ADC.Vpeak*45)>>6;   //[davidchchen]20161219 (Vpek*45)/64 ==> Vpeak*0.707 = Vrms;   

	if(gPSU.PyStatus.bits.DC_INPUT == 0)
	{
		//AcNGRef = ACNG_REF;                   //[davidchchen]20161202 note->167VAC
		//VpeakCompare = sRMSCal.u16VolValue;   //[davidchchen]20161219 Removed 
		//AcNGRef = ACNGRMS_REF;                //[davidchchen]20161219 Removed
		
		if(sRMSCal.u16VolValue > Temp )         //[davidchchen]20161219 if Vrms > Vpeak, change to RMS Calculation;   
		{
		    VpeakCompare = sRMSCal.u16VolValue; //[davidchchen]20161219 if Vrms > Vpeak, change to RMS Calculation,when Input is AC;  
		    AcNGRef = ACNGRMS_REF;              //[davidchchen]20161219 if Vrms > Vpeak, change to RMS Calculation,when Input is AC; 
		} 
		else                                    //[davidchchen]20161219 if Vpeak > Vrms, change to Peak Calculation;
		{
		    VpeakCompare = gPSU.ADC.Vpeak;      //[davidchchen]20161219 if Vpeak > Vrms, change to Peak Calculation,when Input is DC;  
		    AcNGRef = ACNG_REF;                 //[davidchchen]20161219 if Vpeak > Vrms, change to Peak Calculation,when Input is DC;
		}
		
	}
	else
	{
	  VpeakCompare = gPSU.ADC.Vpeak;      //[davidchchen]20161219 Change to Peak calculation when Input is DC;
		AcNGRef = DCNG_REF;                 //[davidchchen]20161202 note->17VDC                 
	}
	
    /*Check Vpeak, add PFC_ADJUST_PIN condition*/
    //if(RELAY_CTRL == RELAY_ON && VpeakCompare > BROWN_HIGH_REF && PFC_ADJUST_PIN == PFC_ADJUST_ON)
    if((RELAY_CTRL == RELAY_ON) && (AC_OK_CTRL == AC_OK) && (gPSU.PyStatus.bits.INPUT_OVP == 0))
    {
		StartDelay(COUNTER_PFC_ON);         //[davidchchen]20161202 note->10ms

		if(IsDelay(COUNTER_PFC_ON))
		{
			//if(!IsOVP()) PFC_CTRL = PFC_ON;
			//PFC_CTRL = PFC_ON;
			if(gPSU.PyStatus.bits.PFC_OV_LATCHED == 0)
			{
				PFC_CTRL = PFC_ON;
			}
		}

		ResetCounter(COUNTER_BROWN_OUT);      //[davidchchen]20161202 note->30ms
	}
    
	//else if(VpeakCompare < BROWN_LOW_REF || PFC_ADJUST_PIN == PFC_ADJUST_OFF)
	//else if(AC_OK_CTRL == AC_NOTOK)
	else if(((AC_OK_CTRL == AC_NOTOK) && (VpeakCompare < AcNGRef && gPSU.Vrms_BrownIn == 0)) || (gPSU.PyStatus.bits.INPUT_OVP == 1))
	{
		StartDelay(COUNTER_BROWN_OUT);  //remove due to inrush issue, //[davidchchen]20161202 note->30ms
        
		if(IsDelay(COUNTER_BROWN_OUT))  //[davidchchen]20161202 note->30ms
		{
			StopDelay(COUNTER_BROWN_OUT);
			
			PFC_CTRL = PFC_OFF;
			RELAY_CTRL = RELAY_OFF;
			/*Sync the signal for secondary side*/
			//if(VpeakCompare < BROWN_LOW_REF) AC_OK_CTRL = AC_NOTOK;
		}

		//StartDelay(COUNTER_RELAY_OFF);  //remove due to inrush issue

		//if(IsDelay(COUNTER_RELAY_OFF))
		//{
			//if(PFC_CTRL == PFC_OFF)
			//{
				//StopDelay(COUNTER_RELAY_OFF);
				//RELAY_CTRL = RELAY_OFF;
			//}
		//}

		ResetCounter(COUNTER_PFC_ON);     //[davidchchen]20161202 note->10ms
    }
    else
    {
		ResetCounter(COUNTER_BROWN_OUT);  //[davidchchen]20161202 note->30ms
		ResetCounter(COUNTER_PFC_ON);     //[davidchchen]20161202 note->10ms
    }
}
/*====================================
    Do StatePFCNotOK
====================================*/
static void Do_StatePFCNotOK(void)
{   
	/*Check PFC IC*/
	CheckPFCIC();

	/*Check PFC OK pin, add PFC_CTRL condition*/
	//if(gPSU.ADC.Vdc.FF > PFC_OK_REF  && PFC_ADJUST_PIN == PFC_ADJUST_ON)
	if(gPSU.ADC.Vdc.FF > PFC_OK_REF)
	{
		ResetCounter(COUNTER_PFC_NOTOK);    //[davidchchen]20161202 note->5ms

		if((gPSU.PyStatus.bits.PRI_OTP == 1) || (PFC_CTRL == PFC_OFF))
		{
			PFC_OK_CTRL = PFC_NOTOK;
			
			gPSU.State = STATE_AC_NOTOK;
			return;
		}
		
		gPSU.State = STATE_PFC_OK;
		return;
	}
	//else if(gPSU.ADC.Vdc.FF < PFC_NOTOK_REF  || PFC_ADJUST_PIN == PFC_ADJUST_OFF)
	else if(gPSU.ADC.Vdc.FF < gPSU.PFCNOK_Ref)
	{                                          
		ResetCounter(COUNTER_PFC_OK);       //[davidchchen]20161202 note->50ms

		StartDelay(COUNTER_PFC_NOTOK);      //[davidchchen]20161202 note->5ms

		if(IsDelay(COUNTER_PFC_NOTOK))
		{
			StopDelay(COUNTER_PFC_NOTOK);
			PFC_OK_CTRL = PFC_NOTOK;
		}
                                              
		if(gPSU.ADC.Vdc.FF < PFC_STB_NOTOK_REF) //[davidchchen]20161202 note->250VDC
		{
			PFC_STB_CTRL = PFC_STB_NOTOK;
		}
  }
	else
	{        
		StopDelay(COUNTER_PFC_NOTOK);       //[davidchchen]20161202 note->5ms
		StopDelay(COUNTER_PFC_OK);          //[davidchchen]20161202 note->50ms
	}

	gPSU.State = STATE_AC_NOTOK;
}
/*====================================
    Do StatePFCOK
====================================*/
static void Do_StatePFCOK(void)
{
    StartDelay(COUNTER_PFC_OK);         //[davidchchen]20161202 note->50ms
            			            
    if(IsDelay(COUNTER_PFC_OK))
    {
        StopDelay(COUNTER_PFC_OK);

		if(gPSU.PyStatus.bits.PFC_OV_LATCHED == 0 && PFC_CTRL == PFC_ON)  //Chris,fix for PFC_OK earlier than PFC_ON
		{
    	    PFC_OK_CTRL = PFC_OK;
		}
		
		PFC_STB_CTRL = PFC_STB_OK;
	}

	//PFCAdjust();
	
    gPSU.State = STATE_AC_NOTOK;            
}
/*====================================
    Do StateACNotOK
====================================*/
static void Do_StateACNotOK(void)
{
	static word ACNotOkCounter = 0;

	word AcGdRef = 0;
	word AcNGRef = 0;
	word RecoverRef = 0;
	word AcSagRef = 0;

	/*Prevent update by interrupt*/
	//word VpeakCompare = gPSU.ADC.Vpeak_Avg;  //[davidchchen]20161219 Removed
	//word Vpeak_BrownOut = gPSU.ADC.Vpeak;    //[davidchchen]20161219 Removed
	word VpeakCompare;      //[davidchchen]20161219 Change to Rms calculation when Input is AC;
	word Vpeak_BrownOut;    //[davidchchen]20161219 Change to Rms calculation when Input is AC;
  
	CheckPriOTP();	//Prevent Uart communication was dead
	CheckVinOV();

	if(gPSU.PyStatus.bits.DC_INPUT == 0)
	{
		//AcGdRef = ACGD_REF;           //[davidchchen]20161202 note->174VAC
		//AcNGRef = ACNG_REF;           //[davidchchen]20161202 note->167VAC
		//RecoverRef = AC_RECOVER_REF;  //[davidchchen]20161202 note->264VAC
		//AcSagRef = ACSAG_REF;         //[davidchchen]20161202 note->157VAC
		
		VpeakCompare = sRMSCal.u16VolValue;     //[davidchchen]20161219 Change to Rms calculation when Input is AC;
	  Vpeak_BrownOut = sRMSCal.u16VolValue;   //[davidchchen]20161219 Change to Rms calculation when Input is AC;
		AcGdRef = ACGDRMS_REF;           //[davidchchen]20161219 note->174VAC Change to Rms calculation when Input is AC;
		AcNGRef = ACNGRMS_REF;           //[davidchchen]20161219 note->167VAC Change to Rms calculation when Input is AC;
		RecoverRef = ACRMS_RECOVER_REF;  //[davidchchen]20161219 note->264VAC Change to Rms calculation when Input is AC;
		AcSagRef = ACSAGRMS_REF;         //[davidchchen]20161219 note->157VAC Change to Rms calculation when Input is AC;
	}
	else
	{
	  VpeakCompare = gPSU.ADC.Vpeak_Avg;  //[davidchchen]20161219 Change to Peak calculation when Input is DC;
	  Vpeak_BrownOut = gPSU.ADC.Vpeak;    //[davidchchen]20161219 Change to Peak calculation when Input is DC;
		AcGdRef = DCGD_REF;           //[davidchchen]20161202 note->184VDC
		AcNGRef = DCNG_REF;           //[davidchchen]20161202 note->177VDC
		RecoverRef = DC_RECOVER_REF;  //[davidchchen]20161202 note->300VDC
		AcSagRef = DCSAG_REF;         //[davidchchen]20161202 note->167VDC
	}
	
	if((AC_OK_CTRL == AC_OK) && (gPSU.PyStatus.bits.INPUT_OVP == 0))
	{
		//ResetCounter(COUNTER_RELAY_OFF);    //[davidchchen]20161201 removed,not used

		StartDelay(COUNTER_RELAY_ON);         //[davidchchen]20161202 note->250ms

		if(IsDelay(COUNTER_RELAY_ON))
		{
			StopDelay(COUNTER_RELAY_ON);
			RELAY_CTRL = RELAY_ON;
		}
	}
	else
	{
		ResetCounter(COUNTER_RELAY_ON);       //[davidchchen]20161202 note->250ms
	}

	//if(VpeakCompare > AcGdRef || gPSU.Vrms_BrownIn == 1)
	//if(VpeakCompare > AcGdRef || gPSU.ADC.Vpeak_Avg > ACGD_REF || gPSU.Vrms_BrownIn == 1)
	if(VpeakCompare > AcGdRef || gPSU.ADC.Vpeak_Avg > ACGD_REF)   //[davidchchen]20161219 Modify.
  {
		//ResetCounter(COUNTER_AC_NOTOK);  //[davidchchen]20161201 removed,not used
		ResetCounter(COUNTER_SAG);         //[davidchchen]20161202 note->500ms

		if(gPSU.PyStatus.bits.INPUT_OVP == 1 && VpeakCompare <= RecoverRef)
		{
			gPSU.PyStatus.bits.INPUT_OVP = 0;
		}

		gPSU.State = STATE_AC_OK;

		return;
  }
	else if(Vpeak_BrownOut < AcNGRef && Vpeak_BrownOut >= AcSagRef && gPSU.Vrms_BrownIn == 0)
	{
		ResetCounter(COUNTER_AC_OK);      //[davidchchen]20161202 note->200ms
		//ResetCounter(COUNTER_AC_NOTOK); //[davidchchen]20161201 removed,not used
	
		StartDelay(COUNTER_SAG);          //[davidchchen]20161202 note->500ms
	        
		if(IsDelay(COUNTER_SAG))
		{
			StopDelay(COUNTER_SAG);
			Get_Vac(gPSU.ADC.Vac.AD, &gPSU.ADC.Vpeak, &gPSU.ADC.Vpeak_Avg, &gPSU.Frequency, RESET);
			AC_OK_CTRL = AC_NOTOK;
		}		
	}
  else if(Vpeak_BrownOut < AcSagRef && gPSU.Vrms_BrownIn == 0)
  {
		ResetCounter(COUNTER_AC_OK);      //[davidchchen]20161202 note->200ms
		ResetCounter(COUNTER_SAG);        //[davidchchen]20161202 note->500ms
		
		//StartDelay(COUNTER_AC_NOTOK);
        
		//if(IsDelay(COUNTER_AC_NOTOK))
        //{
        	//StopDelay(COUNTER_AC_NOTOK);
            //AC_OK_CTRL = AC_NOTOK;
        //}
		if(AC_OK_CTRL == AC_OK)
		{
			Get_Vac(gPSU.ADC.Vac.AD, &gPSU.ADC.Vpeak, &gPSU.ADC.Vpeak_Avg, &gPSU.Frequency, RESET);
			AC_OK_CTRL = AC_NOTOK; //Critial timimg
		}
  }
  else
  {        
		StopDelay(COUNTER_AC_OK);       //[davidchchen]20161202 note->200ms
		//StopDelay(COUNTER_AC_NOTOK);  //[davidchchen]20161201 removed,not used
		StopDelay(COUNTER_SAG);         //[davidchchen]20161202 note->500ms
  }

    gPSU.State = STATE_PFC_NOTOK;
}

/*====================================
    Do StateACOK
====================================*/
static void Do_StateACOK(void)
{
	if(AC_OK_CTRL == AC_NOTOK)
	{
		StartDelay(COUNTER_AC_OK);    //[davidchchen]20161202 note->200ms

		if(IsDelay(COUNTER_AC_OK))
		{
			StopDelay(COUNTER_AC_OK);   //[davidchchen]20161202 note->200ms
			AC_OK_CTRL = AC_OK;
		}
	}
	else
	{
		ResetCounter(COUNTER_AC_OK);  //[davidchchen]20161202 note->200ms
	}

	gPSU.State = STATE_PFC_NOTOK;
}

/*====================================
    Check Vin Off
====================================*/
static void CheckVinSD(void)
{
	static word SDCounter = 0;
	word Vpeak_BrownOut = gPSU.ADC.Vpeak;   
	word AcNGRef = 0;

	if(!IsDelay(COUNTER_MCU_START)) return; //Process after MCU start

	if(gPSU.PyStatus.bits.DC_INPUT == 0)
	{
		AcNGRef = ACSD_REF;                 //[davidchchen]20161202 note->150VAC
	}
	else
	{
		AcNGRef = DCSD_REF;                 //[davidchchen]20161202 note->160VDC
	}

	if((gPSU.ADC.Vac.AD < SD_REF) || (Vpeak_BrownOut < AcNGRef))   //[davidchchen]20161202 note->15VAC
	{
		SDCounter++;

		//if(SDCounter >= SD_COUNTER_REF) //must > 20ms
		if(SDCounter >= 9)	// 166us*5 = 830us, fix for meet 2ms Alert low requirement.
		{
			SDCounter = 0;

			if(AC_OK_CTRL == AC_OK)
			{
				AC_OK_CTRL = AC_NOTOK; //Critial timimg
				Get_Vac(gPSU.ADC.Vac.AD, &gPSU.ADC.Vpeak, &gPSU.ADC.Vpeak_Avg, &gPSU.Frequency, RESET);
			}

		}
	}
	else
		SDCounter = 0;                    
}

/*====================================
    Check PFC Drop
====================================*/
static void CheckPFCDrop(void)
{
	static word DropCounter = 0;

	if(!IsDelay(COUNTER_MCU_START)) return; //Process after MCU start

	if(gPSU.ADC.Vdc.AD < gPSU.PFCNOK_Ref)   //[davidchchen]20161202 note->335VDC/330VDC
	{           
		DropCounter++;

		if(DropCounter >= 1)	// 1ms
		{
			DropCounter = 0;
			PFC_OK_CTRL = PFC_NOTOK;		
		}
	}
	else
		DropCounter = 0;

}

static void CheckPFCOV()
{
	/*Check Vdc if OV*/
		
	#if 1 /*PFC IC will protect itself for the OVP*/
		if(gPSU.ADC.Vdc.AD >= PFC_OV_LATCHED_REF_H)  //[davidchchen]20161202 note->460VDC
		{
			StartDelay(COUNTER_PFC_OV_H_LATCHED);     //[davidchchen]20161202 note->2ms
	
			if(IsDelay(COUNTER_PFC_OV_H_LATCHED))
			{
				StopDelay(COUNTER_PFC_OV_H_LATCHED); /*turn off PFC IC after delay*/
				gPSU.PyStatus.bits.PFC_OV_LATCHED = 1;
				PFC_CTRL = PFC_OFF;
				PFC_OK_CTRL = PFC_NOTOK;
			}				
		}
		else
		{
			ResetCounter(COUNTER_PFC_OV_H_LATCHED);   //[davidchchen]20161202 note->2ms
		}

		if(gPSU.ADC.Vdc.AD >= PFC_OV_LATCHED_REF)   //[davidchchen]20161202 note->450VDC
		{
			StartDelay(COUNTER_PFC_OV_LATCHED);       //[davidchchen]20161202 note->120ms
	
			if(IsDelay(COUNTER_PFC_OV_LATCHED))
			{
				StopDelay(COUNTER_PFC_OV_LATCHED); /*turn off PFC IC after delay*/
				gPSU.PyStatus.bits.PFC_OV_LATCHED = 1;
				PFC_CTRL = PFC_OFF;
				PFC_OK_CTRL = PFC_NOTOK;
			}	
		}	
		else
		{
			ResetCounter(COUNTER_PFC_OV_LATCHED);     //[davidchchen]20161202 note->120ms
		}
	#endif

}

void CheckVrms()
{
	//word VpeakCompare = gPSU.ADC.Vpeak_Avg; //[davidchchen]20161219 Removed
	word VpeakCompare; //[davidchchen]20161219  Change to RMS calculation when Input is AC;

	if(gPSU.PyStatus.bits.DC_INPUT == 0)
	{
	  VpeakCompare = sRMSCal.u16VolValue;       //[davidchchen]20161219 Change to RMS calculation when Input is AC;
		//if(VpeakCompare > VAC_CONVERT(155))     //[davidchchen]20161219 Removed
		if(VpeakCompare > VAC_INS_CONVERT(155))   //[davidchchen]20161219 Change to RMS calculation when Input is AC;
		{
			if(VRMS_DET == VRMS_BROWN_IN)
			{
				gPSU.Vrms_BrownIn = 1;
			}
			else
			{
				gPSU.Vrms_BrownIn = 0;
			}
		}
		else
		{
			gPSU.Vrms_BrownIn = 0;
		}
	}
	else//vincent add 2015 0603
	{
	  VpeakCompare = gPSU.ADC.Vpeak_Avg; //[davidchchen]20161219 Change to Peak calculation when Input is DC;
		//if(VpeakCompare > VDCIN_CONVERT(155))
		if(VpeakCompare > VDCIN_CONVERT(182))   //davidchchen modify 20160425
		{
			if(VRMS_DET == VRMS_BROWN_IN)
			{
				  gPSU.Vrms_BrownIn = 1;
			}
			else
			{
	        gPSU.Vrms_BrownIn = 0;
			}
		}
		else
		{
			gPSU.Vrms_BrownIn = 0;
		}
	}
	
}

/*====================================
    Main loop    
====================================*/

void main(void) 
{	
    ProgramInit();        		
	
	for(;;)
	{
        __RESET_WATCHDOG();
            
      if(counter1 >= 6)           //[vincent.liu]20161202 Added 
      {
        counter1 = counter1 - 6;
        
        ADC_CalRMS();          
	    }
	    
	    if(counter2 >= 6)           //[vincent.liu]20161202 Added
	    {
        counter2 = counter2 - 6;
            
        ProcessCounter(6);
        CheckPFCDrop();
    		CheckPFCOV();		        
	    }
	    
	    if(counter3 >= 6)           //[vincent.liu]20161202 Added 
	    {
        counter3 = counter3 - 6;
            
        UartDataRefresh();
        SCI_Transmit();
		    CheckVrms();
		    PFCAdjust();
            
		    switch (gPSU.State)
            {
                case STATE_INIT:
                    Do_StateInit();
                    break;

                case STATE_PFC_NOTOK:
                    Do_StatePFCNotOK();
                    break;

                case STATE_PFC_OK:
                    Do_StatePFCOK();
                    break;

                case STATE_AC_NOTOK:
                    Do_StateACNotOK();
                    break;

                case STATE_AC_OK:
                    Do_StateACOK();
                    break; 

                //case STATE_AC_OFF: //do not used currently    //[davidchchen]20161201 removed,not used  
                    //Do_StateACOff();
                //    break;                                    //[davidchchen]20161201 removed,not used
    				
    			default:
    				break;
            } 	        
	    }
	}	
}

void IsAcInput(word Vac)
{
	static word vin_max = 0;
	static word vin_min = 1024;
	static byte vin_counter = 0;
	word diff = 0;
	byte isAcInput = 0;

	if(vin_max < Vac)
	{
		vin_max = Vac;
	}

	if(vin_min > Vac)
	{
		vin_min = Vac;
	}

	vin_counter++;

	if(vin_counter >= 72)   //[davidchchen]20161202 note-> 166us*73=12.118ms
	{
	//vincent change 20150604
	/*	static byte ac_in_counter = 0;
		static byte dc_in_counter = 0;
		
		vin_counter = 0;
		diff = vin_max - vin_min;
		if(diff > VAC_CONVERT(50)){
			if(ac_in_counter >= 3){
				gPSU.PyStatus.bits.DC_INPUT = 0;
			}else{
				ac_in_counter++;
			}
			dc_in_counter = 0;
		}else if(diff < VAC_CONVERT(20)){
			if(dc_in_counter >= 3){
				gPSU.PyStatus.bits.DC_INPUT = 1;
			}else{
				dc_in_counter++;
			}
			ac_in_counter = 0;
		}

		vin_max = 0;
		vin_min = 1024;     */
		
		vin_counter = 0;		

		diff = vin_max - vin_min;
				
		if(diff > VAC_CONVERT(50))
		{
		   gPSU.PyStatus.bits.DC_INPUT = 0;
		} 
		else if (diff < VAC_CONVERT(20))
		{
		   gPSU.PyStatus.bits.DC_INPUT = 1;
		}

		vin_max = 0;
		vin_min = 1024;
	}

}

/*====================================
    ISR Timer
====================================*/

interrupt VectorNumber_Vmtim void ISR_TIMER(void)
{
  static word TempAdcCnt = 0; //[vincent.liu]20161202 Added

	MTIMSC &= 0x7F;
	
	counter1++;       //[vincent.liu]20161202 Added
  counter2++;       //[vincent.liu]20161202 Added
  counter3++;       //[vincent.liu]20161202 Added
    
  TempAdcCnt++;     //[vincent.liu]20161202 Added
    
    if(ADCSC1 & 0x80)                     //[vincent.liu]20161202 Added
    {
        if(TempAdcCnt >= 1506)            //[vincent.liu]20161202 Added 
        {
            TempAdcCnt = 0;               //[vincent.liu]20161202 Added
            
            gPSU.ADC.Vdc.AD = ADCR;       //[vincent.liu]20161202 Added
            ADCSC1 = ADC_VAC_RECT;        //[vincent.liu]20161202 Added
    		    GetAverage(&gPSU.ADC.Vdc);    //[vincent.liu]20161202 Added
    		
    	  	  while((ADCSC1 & 0x80) == 0);  //[vincent.liu]20161202 Added
    		    gPSU.ADC.Vac.AD = ADCR;       //[vincent.liu]20161202 Added
    	      ADCSC1 = ADC_TEMPERATURE1;	  //[vincent.liu]20161202 Added
    		    ADC_AccSquare((word)gPSU.ADC.Vac.AD); //[vincent.liu]20161202 Added
            
            while((ADCSC1 & 0x80) == 0);      //[vincent.liu]20161202 Added
            gPSU.ADC.Temperature.AD = ADCR;   //[vincent.liu]20161202 Added
            ADCSC1 = ADC_VDC;                 //[vincent.liu]20161202 Added
    		    GetAverage(&gPSU.ADC.Temperature);//[vincent.liu]20161202 Added
        } 
        else                            
        {
            gPSU.ADC.Vdc.AD = ADCR;               //[vincent.liu]20161202 Added
            ADCSC1 = ADC_VAC_RECT;                //[vincent.liu]20161202 Added
    		    GetAverage(&gPSU.ADC.Vdc);            //[vincent.liu]20161202 Added
    		
    		    while((ADCSC1 & 0x80) == 0);          //[vincent.liu]20161202 Added
    		    gPSU.ADC.Vac.AD = ADCR;               //[vincent.liu]20161202 Added
    	      ADCSC1 = ADC_VDC;	                    //[vincent.liu]20161202 Added
    		    ADC_AccSquare((word)gPSU.ADC.Vac.AD); //[vincent.liu]20161202 Added
        }
    } 
		
	IsAcInput(gPSU.ADC.Vac.AD);

	Get_Vac(gPSU.ADC.Vac.AD, &gPSU.ADC.Vpeak, &gPSU.ADC.Vpeak_Avg, &gPSU.Frequency, NOT_RESET);
	
	CheckVinSD();   //Critical timing for H/W's requirement

	__RESET_WATCHDOG();
}

