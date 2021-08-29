/*****************************************

FILE: main.h
COMPANY: Liteon Technology, Inc.
PROCESSOR: MC9S08QG4
PURPOSE: Global variable define
HISTORY:

DATE        REVISION    DESCRIPTION
-----------------------------------
2008/12/09  v0.1        Created


*****************************************/

/*************************************
    State Define
*************************************/
typedef enum _STATE
{
    STATE_INIT = 0,    
    STATE_PFC_NOTOK,
    STATE_PFC_OK,    
    STATE_AC_NOTOK,
    STATE_AC_OK,
    STATE_SCI,
    //STATE_AC_OFF,  
};
/*************************************
    Type Define
*************************************/

typedef enum _CONTER_TYPE
{
	COUNTER_MCU_START = 0,
	COUNTER_PFC_OK,
	COUNTER_PFC_NOTOK,
	COUNTER_PFC_OV_LATCHED,
	COUNTER_PFC_OV_H_LATCHED,
	COUNTER_BROWN_OUT,
	//COUNTER_AC_OFF,
	//COUNTER_SCI,        
	//COUNTER_AC_WINDOW,
	//COUNTER_SD,
	COUNTER_SAG,
	//COUNTER_AC_NOTOK,   
	COUNTER_AC_OK,
	COUNTER_RELAY_ON,
	//COUNTER_RELAY_OFF,  
	COUNTER_PFC_ON,
	COUNTER_PFC_ADJUST_ON_DELAY,
	COUNTER_PFC_ADJUST_OFF,
	COUNTER_OTP_SD,
	COUNTER_OTP_RECOVER,
	COUNTER_VIN_OV,

	COUNTER_TOTAL_TYPE //Total number of the counter type
};

typedef enum _SYNC_TYPE
{
	START_FLAG = 0,
	NOTIFY_FLAG,

	FLAG_TOTAL_TYPE //Total number of the counter type
};


