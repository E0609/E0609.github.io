#include "Include_headers.h"
#include "Dsp_Interrupt.h"

void (*TMR1_InterruptHandler)(void) = NULL;
void (*ADC1_I_PFC_CT_ADefaultInterruptHandler)(uint16_t adcVal);

void __attribute__ ( ( interrupt, no_auto_psv ) ) _T1Interrupt (  )
{
    /* Check if the Timer Interrupt/Status is set */

    //***User Area Begin

    // ticker function call;
    // ticker is 1 -> Callback function gets called everytime this ISR executes
    if(TMR1_InterruptHandler) 
    { 
           TMR1_InterruptHandler(); 
    }

    //***User Area End

    tmr1_obj.count++;
    tmr1_obj.timerElapsed = true;
    IFS0bits.T1IF = false;
}

void __attribute__ ((weak)) TMR1_CallBack(void)
{
    Control.Tsk_200uS = 1;
}

void  TMR1_SetInterruptHandler(void (* InterruptHandler)(void))
{ 
    IEC0bits.T1IE = false;
    TMR1_InterruptHandler = InterruptHandler; 
    IEC0bits.T1IE = true;
}

void __attribute__ ((weak)) ADC1_I_PFC_CT_A_CallBack( uint16_t adcVal )
{ 
    Control_Loop();
 //AC_NOK_PRI_Toggle();
}

void ADC1_SetI_PFC_CT_AInterruptHandler(void* handler)
{
    ADC1_I_PFC_CT_ADefaultInterruptHandler = handler;
}

void __attribute__ ( ( __interrupt__ , auto_psv, weak ) ) _ADCAN3Interrupt ( void )
{
    uint16_t valI_PFC_CT_A;
    //Read the ADC value from the ADCBUF
    valI_PFC_CT_A = ADCBUF3;

    if(ADC1_I_PFC_CT_ADefaultInterruptHandler) 
    { 
        ADC1_I_PFC_CT_ADefaultInterruptHandler(valI_PFC_CT_A); 
    }

    //clear the I_PFC_CT_A interrupt flag
    IFS7bits.ADCAN3IF = 0;
}

/*******************************************************************************
 * Function:        _CMP3Interrupt
 *
 * Parameters:      -
 * Returned value:  -
 *
 * Description:     BULK OVP interrupt - CMP3B
 *
 ******************************************************************************/

void __attribute__((__interrupt__, no_auto_psv)) _CMP3Interrupt()
{
    /*Turn OFF PFC - Latch*/
    PWM_DISABLE();
    PRI_DATA.PFC_En = 0;
    _AC3IF = 0;

} 

void __attribute__((__interrupt__, no_auto_psv)) _CMP2Interrupt()
{

    if(!PRI_DATA.CL_SoftStart)
    {
        /*Turn OFF PFC - Latch*/
        PWM_DISABLE();
        PRI_DATA.PFC_En = 0;
    }
  _AC2IF = 0;

}

void Init_Timer1_Callback()
{
    if(TMR1_InterruptHandler == NULL)
    {
        TMR1_SetInterruptHandler(&TMR1_CallBack);
    }
}

void Init_ADC_Callback()
{
  ADC1_SetI_PFC_CT_AInterruptHandler(&ADC1_I_PFC_CT_A_CallBack);
}