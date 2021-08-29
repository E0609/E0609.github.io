/*****************************************

FILE: Timer.h
COMPANY: Liteon Technology, Inc.
PROCESSOR: MC9S08QG4
PURPOSE: Timer functions
HISTORY:

DATE        REVISION    DESCRIPTION
-----------------------------------
2008/03/03  v0.1        Created

*****************************************/

void ProcessCounter(word timediff);
byte IsDelay(byte CounterType);
void StartDelay(byte CounterType);
void StopDelay(byte CounterType);
void ResetCounter(byte CounterType);
//byte SetPin(byte CounterType, byte Pin, byte OnOff);  
//byte IsOVP(void);                                   