/*****************************************

FILE: Timer.c
COMPANY: Liteon Technology, Inc.
PROCESSOR: MC9S08QG4
PURPOSE: Timer functions
HISTORY:

DATE        REVISION    DESCRIPTION
-----------------------------------
2008/03/03  v0.1        Created

*****************************************/
#include <MC9S08QG4.h>
#include "main.h"
#include "stdlib.h" //for boolean data type
#include "Config.h"

/*************************************
    Global variable
*************************************/
static volatile word gCounterArray[COUNTER_TOTAL_TYPE];
static volatile byte gSyncArray[COUNTER_TOTAL_TYPE][FLAG_TOTAL_TYPE];

/*************************************
    Internal Function
*************************************/
static unsigned int GetCounterRef(byte CounterType);

/*====================================
Process counter in ISR
====================================*/
void ProcessCounter(word timediff)
{    
    byte Index = 0;        
        
    for (Index = 0; Index < COUNTER_TOTAL_TYPE; Index++)
    {                        	    
        if (gSyncArray[Index][START_FLAG] == 1) 
        {
            gCounterArray[Index] += timediff;
            
            if (gCounterArray[Index] >= GetCounterRef(Index))
            {
                gSyncArray[Index][START_FLAG] = 0;
                gSyncArray[Index][NOTIFY_FLAG] = 1;
                gCounterArray[Index] = 0; //reset
            }            
        }
		#if 0
        else
            gCounterArray[Index] = 0; //reset        
		#endif
            
        __RESET_WATCHDOG(); //Maybe we need to reset watch dog due to lots of counters           
    }
}

/*====================================
Check if delay done or not
====================================*/
byte IsDelay(byte CounterType)
{
    return gSyncArray[CounterType][NOTIFY_FLAG];
}

/*====================================
Set the Sync array to start counter
====================================*/
void StartDelay(byte CounterType)
{       
    gSyncArray[CounterType][START_FLAG] = 1;    
}

/*====================================
Set the Sync array to stop counter
====================================*/
void StopDelay(byte CounterType)
{        
    gSyncArray[CounterType][NOTIFY_FLAG] = 0;
	gSyncArray[CounterType][START_FLAG] = 0;
}

/*====================================
Reset the counter
====================================*/

void ResetCounter(byte CounterType)
{
    gSyncArray[CounterType][NOTIFY_FLAG] = 0;
    gSyncArray[CounterType][START_FLAG] = 0;
    gCounterArray[CounterType] = 0;
}


/*====================================
Get Counter Reference
====================================*/
static unsigned int GetCounterRef(byte CounterType)
{
	switch (CounterType)
	{                                                                                                             
		case COUNTER_MCU_START:            
			return MCU_START_COUNTER_REF;                        
            
		case COUNTER_PFC_OK:            
			return PFC_OK_COUNTER_REF;
            
		case COUNTER_PFC_NOTOK:            
			return PFC_NOTOK_COUNTER_REF;                                                                    
          
		case COUNTER_PFC_OV_LATCHED:
			return PFC_OV_LATCHED_COUNTER_REF;

		case COUNTER_PFC_OV_H_LATCHED:
			return PFC_OV_H_LATCHED_COUNTER_REF;
			
		case COUNTER_BROWN_OUT:            
			return BROWN_OUT_COUNTER_REF;
            
		//case COUNTER_AC_OFF:
		//    return AC_OFF_COUNTER_REF;
            
		//case COUNTER_SCI:             
		//	return SCI_COUNTER_REF;     
            
		//case COUNTER_AC_WINDOW:
		//    return AC_WINDOW_COUNTER_REF;
           
		//case COUNTER_SD:
		//    return SD_COUNTER_REF;
            
		//case COUNTER_AC_NOTOK:          
		//	return AC_NOTOK_COUNTER_REF;  
            
		case COUNTER_AC_OK:
			return AC_OK_COUNTER_REF;

		case COUNTER_RELAY_ON:
			return RELAY_ON_COUNTER_REF;

		//case COUNTER_RELAY_OFF:         
		//	return RELAY_OFF_COUNTER_REF; 

		case COUNTER_PFC_ON:
			return PFC_ON_COUNTER_REF;

		case COUNTER_PFC_ADJUST_OFF:
			return PFC_ADJUST_OFF_REF;

		case COUNTER_PFC_ADJUST_ON_DELAY:
			return PFC_ADJUST_ON_DELAY_REF;

		case COUNTER_OTP_SD:
			return OTP_SD_COUNTER_REF;

		case COUNTER_OTP_RECOVER:
			return OTP_RECOVER_COUNTER_REF;

		case COUNTER_VIN_OV:
			return VIN_OV_REF;

		case COUNTER_SAG:
			return SAG_TIME_REF;
    }               
}