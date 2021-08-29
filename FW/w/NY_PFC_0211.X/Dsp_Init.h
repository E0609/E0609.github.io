/**
  @Generated PIC24 / dsPIC33 / PIC32MM MCUs Source File

  @Company:
    Microchip Technology Inc.

  @File Name:
    system.h

  @Summary:
    This is the system.h file generated using PIC24 / dsPIC33 / PIC32MM MCUs

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

#ifndef _XTAL_FREQ
#define _XTAL_FREQ  119762500UL
#endif

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef DSP_INIT_H
#define	DSP_INIT_H
/** 
 * CORCON initialization type enumerator. Supported types:
 * CORCON_MODE_PORVALUES
 * CORCON_MODE_ENABLEALLSATNORMAL_ROUNDBIASED
 * CORCON_MODE_ENABLEALLSATNORMAL_ROUNDUNBIASED
 * CORCON_MODE_DISABLEALLSAT_ROUNDBIASED
 * CORCON_MODE_DISABLEALLSAT_ROUNDUNBIASED
 * CORCON_MODE_ENABLEALLSATSUPER_ROUNDBIASED
 * CORCON_MODE_ENABLEALLSATSUPER_ROUNDUNBIASED
 */
typedef enum tagCORCON_MODE_TYPE
{ 
    CORCON_MODE_PORVALUES   = 0x0020,                       /** Use POR values of CORCON */
    CORCON_MODE_ENABLEALLSATNORMAL_ROUNDBIASED = 0x00E2,    /** Enable saturation for ACCA, ACCB
                                                             *  and Dataspace write, enable normal
                                                             *  ACCA/ACCB saturation mode and set
                                                             *  rounding to Biased (conventional)
                                                             *  mode. Rest of CORCON settings are
                                                             *  set to the default POR values.
                                                             *  */
    CORCON_MODE_ENABLEALLSATNORMAL_ROUNDUNBIASED = 0x00E0,  /** Enable saturation for ACCA, ACCB
                                                             *  and Dataspace write, enable normal
                                                             *  ACCA/ACCB saturation mode and set
                                                             *  rounding to Unbiased (convergent)
                                                             *  mode. Rest of CORCON settings are
                                                             *  set to the default POR values.
                                                             *  */
    CORCON_MODE_DISABLEALLSAT_ROUNDBIASED = 0x0022,         /** Disable saturation for ACCA, ACCB
                                                             *  and Dataspace write and set
                                                             *  rounding to Biased (conventional)
                                                             *  mode. Rest of CORCON settings are
                                                             *  set to the default POR values.
                                                             *  */
    CORCON_MODE_DISABLEALLSAT_ROUNDUNBIASED = 0x0020,       /** Disable saturation for ACCA, ACCB
                                                             *  and Dataspace write and set
                                                             *  rounding to Unbiased (convergent)
                                                             *  mode. Rest of CORCON settings are
                                                             *  set to the default POR values.
                                                             *  */
    CORCON_MODE_ENABLEALLSATSUPER_ROUNDBIASED = 0x00F2,    /** Enable saturation for ACCA, ACCB
                                                             *  and Dataspace write, enable super
                                                             *  ACCA/ACCB saturation mode and set
                                                             *  rounding to Biased (conventional)
                                                             *  mode. Rest of CORCON settings are
                                                             *  set to the default POR values.
                                                             *  */
    CORCON_MODE_ENABLEALLSATSUPER_ROUNDUNBIASED = 0x00F0,  /** Enable saturation for ACCA, ACCB
                                                             *  and Dataspace write, enable super
                                                             *  ACCA/ACCB saturation mode and set
                                                             *  rounding to Unbiased (convergent)
                                                             *  mode. Rest of CORCON settings are
                                                             *  set to the default POR values.
                                                             *  */
} SYSTEM_CORCON_MODES;

#define I_PFC_CT_B_SetHigh()            (_LATA0 = 1)
#define I_PFC_CT_B_SetLow()             (_LATA0 = 0)
#define I_PFC_CT_B_Toggle()             (_LATA0 ^= 1)
#define I_PFC_CT_B_GetValue()           _RA0
#define I_PFC_CT_B_SetDigitalInput()    (_TRISA0 = 1)
#define I_PFC_CT_B_SetDigitalOutput()   (_TRISA0 = 0)
#define I_PFC_CT_A_SetHigh()            (_LATB0 = 1)
#define I_PFC_CT_A_SetLow()             (_LATB0 = 0)
#define I_PFC_CT_A_Toggle()             (_LATB0 ^= 1)
#define I_PFC_CT_A_GetValue()           _RB0
#define I_PFC_CT_A_SetDigitalInput()    (_TRISB0 = 1)
#define I_PFC_CT_A_SetDigitalOutput()   (_TRISB0 = 0)
#define V_PFC_OVP_SetHigh()             (_LATB10 = 1)
#define V_PFC_OVP_SetLow()              (_LATB10 = 0)
#define V_PFC_OVP_Toggle()              (_LATB10 ^= 1)
#define V_PFC_OVP_GetValue()            _RB10
#define V_PFC_OVP_SetDigitalInput()     (_TRISB10 = 1)
#define V_PFC_OVP_SetDigitalOutput()    (_TRISB10 = 0)
#define PFC_OFF_PRI_SetHigh()           (_LATB12 = 1)
#define PFC_OFF_PRI_SetLow()            (_LATB12 = 0)
#define PFC_OFF_PRI_Toggle()            (_LATB12 ^= 1)
#define PFC_OFF_PRI_GetValue()          _RB12
#define PFC_OFF_PRI_SetDigitalInput()   (_TRISB12 = 1)
#define PFC_OFF_PRI_SetDigitalOutput()  (_TRISB12 = 0)
#define PFC_NOK_PRI_SetHigh()           (_LATB3 = 1)
#define PFC_NOK_PRI_SetLow()            (_LATB3 = 0)
#define PFC_NOK_PRI_Toggle()            (_LATB3 ^= 1)
#define PFC_NOK_PRI_GetValue()          _RB3
#define PFC_NOK_PRI_SetDigitalInput()   (_TRISB3 = 1)
#define PFC_NOK_PRI_SetDigitalOutput()  (_TRISB3 = 0)
#define AC_NOK_PRI_SetHigh()            (_LATB4 = 1)
#define AC_NOK_PRI_SetLow()             (_LATB4 = 0)
#define AC_NOK_PRI_Toggle()             (_LATB4 ^= 1)
#define AC_NOK_PRI_GetValue()           _RB4
#define AC_NOK_PRI_SetDigitalInput()    (_TRISB4 = 1)
#define AC_NOK_PRI_SetDigitalOutput()   (_TRISB4 = 0)
#define V_PFC_FB_SetHigh()              (_LATB9 = 1)
#define V_PFC_FB_SetLow()               (_LATB9 = 0)
#define V_PFC_FB_Toggle()               (_LATB9 ^= 1)
#define V_PFC_FB_GetValue()             _RB9
#define V_PFC_FB_SetDigitalInput()      (_TRISB9 = 1)
#define V_PFC_FB_SetDigitalOutput()     (_TRISB9 = 0)
#define VAC_L_DET_SetHigh()             (_LATC11 = 1)
#define VAC_L_DET_SetLow()              (_LATC11 = 0)
#define VAC_L_DET_Toggle()              (_LATC11 ^= 1)
#define VAC_L_DET_GetValue()            _RC11
#define VAC_L_DET_SetDigitalInput()     (_TRISC11 = 1)
#define VAC_L_DET_SetDigitalOutput()    (_TRISC11 = 0)
#define VAC_N_DET_SetHigh()             (_LATC12 = 1)
#define VAC_N_DET_SetLow()              (_LATC12 = 0)
#define VAC_N_DET_Toggle()              (_LATC12 ^= 1)
#define VAC_N_DET_GetValue()            _RC12
#define VAC_N_DET_SetDigitalInput()     (_TRISC12 = 1)
#define VAC_N_DET_SetDigitalOutput()    (_TRISC12 = 0)

#ifndef _XTAL_FREQ
#define _XTAL_FREQ  119762500UL
#endif

#define CLOCK_SystemFrequencyGet()        (119762500UL)
#define CLOCK_PeripheralFrequencyGet()    (CLOCK_SystemFrequencyGet() / 2)
#define CLOCK_InstructionFrequencyGet()   (CLOCK_SystemFrequencyGet() / 2)

#define BULK_OVP_PRIO       5
#define TMR1_INTERRUPT_TICKER_FACTOR    1

#define ADC1_SCAN_MODE_SELECTED true


typedef enum
{
    
	CMP1_INPUT_CMP1D  = 0x3,
	CMP1_INPUT_CMP1C  = 0x2,
	CMP1_INPUT_CMP1B  = 0x1,
	CMP1_INPUT_CMP1A  = 0x0,
	CMP1_INPUT_PGA2  = 0x1,
	CMP1_INPUT_PGA1  = 0x0
	
}CMP1_INPUT;


typedef enum
{
    
	CMP2_INPUT_CMP2D  = 0x3,
	CMP2_INPUT_CMP2C  = 0x2,
	CMP2_INPUT_CMP2B  = 0x1,
	CMP2_INPUT_CMP2A  = 0x0,
	CMP2_INPUT_PGA2  = 0x1,
	CMP2_INPUT_PGA1  = 0x0
	
}CMP2_INPUT;


typedef enum
{
    
	CMP3_INPUT_CMP3D  = 0x3,
	CMP3_INPUT_CMP3C  = 0x2,
	CMP3_INPUT_CMP3B  = 0x1,
	CMP3_INPUT_CMP3A  = 0x0,
	CMP3_INPUT_PGA2  = 0x1,
	CMP3_INPUT_PGA1  = 0x0
	
}CMP3_INPUT;
	
typedef struct _TMR_OBJ_STRUCT
{
    /* Timer Elapsed */
    volatile bool           timerElapsed;
    /*Software Counter value*/
    volatile uint8_t        count;

} TMR_OBJ;



typedef enum 
{
    V_PFC_FB,//Channel Name:AN4   Assigned to:Shared Channel
    VAC_L_DET,//Channel Name:AN12   Assigned to:Shared Channel
    VAC_N_DET,//Channel Name:AN14   Assigned to:Shared Channel
    I_PFC_CT_B,//Channel Name:AN0   Assigned to:Dedicated Core0
    I_PFC_CT_A,//Channel Name:AN3   Assigned to:Dedicated Core3
} ADC1_CHANNEL;

void ADC1_Initialize (void);
void ADC1_Core0PowerEnable(void);
void ADC1_Core0Calibration(void);
void ADC1_Core3PowerEnable(void);
void ADC1_Core3Calibration(void);
void ADC1_SharedCorePowerEnable(void);
void ADC1_SharedCoreCalibration(void);

void PIN_MANAGER_Initialize (void);
void CLOCK_Initialize(void);
bool CLOCK_AuxPllLockStatusGet();
void CMP_Initialize(void);
void PWM_Initialize (void);
void TMR1_Initialize (void);


/**
 * @Param
    none
 * @Returns
    none
 * @Description
    Initializes the device to the default states configured in the
 *                  MCC GUI
 * @Example
    SYSTEM_Initialize(void);
 */
void SYSTEM_Initialize(void);
void __attribute__((__section__("APP_IMAGE"))) Env_Init(void);
extern TMR_OBJ tmr1_obj;

#endif	/*DSP_INIT_H*/
/**
 End of File
*/

