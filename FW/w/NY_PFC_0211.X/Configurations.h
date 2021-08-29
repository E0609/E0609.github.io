/*******************************************************************************
 * Filename   : Configuration.h
 * Project    : 
 * Created on : 13 December, 2017, 9:37 AM
 * Author(s)  : Microchip reference SDC
 * 
 * Date of last Modification:
 * by $Author$:
 * 
 * Description: 
 *
 * Compiler info : MPLABX XC16 Version 1.26
 * Processor info: 
 *
 ******************************************************************************/

/******************************************************************************
                                Include Files
*******************************************************************************/
// 
 #include "xc.h"

/******************************************************************************
                             Variable Declaration
*****************************************************************************/

/******************************************************************************
                             Constant Declaration
*****************************************************************************/
// FW Revisions
#define NANYANG_FW_REV_MAJ    1     //1 -
#define MINOR_REV           1     //0 - 1.0  First Release 
                                  //1 - 1.1  P1B Release - Current Loop Slowed

#define BL_FW_REVISION      0x01    //01 - Initial Release                                         

#define FW_REVISION_DATA   0x4E05

#define TRUE    1
#define FALSE   0

#define RELEASE_FOR_TESTING     FALSE       //for BL testing

#if RELEASE_FOR_TESTING
#define Microchip reference_FW_REVISION     (FW_REVISION_DATA | 0x0080)
#define NANYANG_FW_REV_MIN    MINOR_REV + 80          
#define NANYANG_FW_REV        (NANYANG_FW_REV_MAJ<<8) + NANYANG_FW_REV_MIN  
#define APP_CODE_CRC        0xDFA0
#else
#define Microchip reference_FW_REVISION     FW_REVISION_DATA
#define NANYANG_FW_REV_MIN    MINOR_REV       
#define NANYANG_FW_REV        (NANYANG_FW_REV_MAJ<<8) + NANYANG_FW_REV_MIN  
#define APP_CODE_CRC        0x7EEB
#endif

//======================= Global Defines========================================
#define OFF       0
#define ON        !OFF

#define DOWN      0
#define UP        !DOWN

typedef union 
{
  unsigned long Val32;
  unsigned int  Val16[2];
  unsigned char Val8[4];
  struct {
    unsigned int L;
    unsigned int H;
  } Word;
} REG32;

/*********************************************************************************************************************
 * Type definition
 ********************************************************************************************************************/

/*******************************************************************************
 * Global data types (typedefs / structs / enums)
 ******************************************************************************/
typedef   signed char      int8_t;
typedef unsigned char      uint8_t;
typedef   signed int       int16_t;
typedef unsigned int       uint16_t;
typedef   signed long      int32_t;
typedef unsigned long      uint32_t;
typedef   signed long long int64_t;
typedef unsigned long long uint64_t;


//Output pin Declarations (With Input Sense)
#define PWM_ENABLE_LOP          _LATA4              /* pin12 */
#define PWM_ENABLE_DIRECTION    _TRISA4
 
#define RELAY_ON_HOP            _LATB11             /* pin 8,RB11 */
#define RELAY_ON_DIRECTION      _TRISB11 
#define RELAY_IO_OUPUT_VAL      _RB11
/************** ADC buffer Definition **************/
#define CT_A_CURRENT_BUF            ADCBUF3    /* CS+_PFC_A MOSFET current of phase A:I_PFC_CT_A */
#define CT_B_CURRENT_BUF            ADCBUF0    /* CS+_PFC_B MOSFET current of phase B:I_PFC_CT_B */
#define INPUT_CURRENT_DET           ADCBUF6    /* PFC input current：I_PFC_DET*/
#define INPUT_CURRENT_BUF           ADCBUF7    /* PFC input current：I_PFC_METER*/
#define L_VOLTAGE_BUF               ADCBUF12       /* Line L voltage:VAC_L_DET */
#define N_VOLTAGE_BUF               ADCBUF14       /* Line N voltage:VAC_N_DET */
#define BUS_VOLTAGE_BUF             ADCBUF4   	   /* Bus  voltage:V_PFC_FB */
#define BUS_VOLTAGE_OVP             ADCBUF5
#define PFC_FET_A_TEMP              ADCBUF9
#define PFC_FET_B_TEMP              ADCBUF8
#define BOOSTDIODE_TEMP             ADCBUF19
#define LLC_PRI_TEMP                ADCBUF11

#define REF_2V5_CALI            Q10(2.495/3.287)

/* DSP registers definition */
#define PFC_PWMPERIOD           PTPER   
#define PFC_PHASE_SHIFT1        PHASE1   
#define PFC_PHASE_SHIFT2        PHASE2
#define PFC_PHASE_SHIFT3        PHASE3
#define PFC_PWMDUTY1            PDC1
#define PFC_PWMDUTY2            PDC2
#define ADCTRIG1                TRIG1   
#define ADCTRIG2                TRIG2
#define PFC_ADIEL               ADIEL  //for hemi add,not used?
#define PFC_PWM2LDUTY           PDC3   //for relay PWM3
//Interrupt Priority
/* Interrupt priority 7 = HIGH .. 1 = LOW */
#define ADC_INT_PRIO        6
#define UART_RX_INT_PRIO    4
#define UART_TX_INT_PRIO    3
#define IC1_INT_PRIO        2
#define IC2_INT_PRIO        2
#define T1_INT_PRIO         1

//Calibration Data
#define CALIBRATION_DATA_ADDR   0x0C00
#define CALIBRATION_DATA_PAGE   3

/******************************************************************************
                             Macro Declaration
*****************************************************************************/
//PFC - PWM  Control Macro's

#define PWM_ENABLE()        {PWM_ENABLE_LOP = 0; PWM_ENABLE_DIRECTION = 0;} //PWM_ENABLE_DIRECTION = 0 means output
#define PWM_DISABLE()       {PWM_ENABLE_LOP = 0; PWM_ENABLE_DIRECTION = 1;} //PWM_ENABLE_DIRECTION = 1 means input

//#define RELAY_START()        { RELAY_ON_DIRECTION = 0; RELAY_ON_HOP = 1; IOCON2bits.PENL = 0; RELAY_IO_OUPUT_VAL = 1;}
#define RELAY_START()        	{ PDC3 = 0xB83F;}//PHASE3 = 0xB838 (20kHz), PDC3 = 0x8A2A (75% duty), 0xB83F (100%)
#define RELAY_PWM_ON()      	{ PDC3 = 0x9C9C;}//PHASE3 = 0xB838 (20kHz), PDC3 = 0x8A2A (75% duty), 0xB83F (100%) 0c9C9C (85%)
#define RELAY_OFF()         	{ PDC3 = 0x0000;}//PHASE3 = 0xB838 (20kHz), PDC3 = 0x8A2A (75% duty), 0xB83F (100%)


#define HIBYTE(DATA)   ((uint8_t)((DATA >> 8) & 0x00FF))
#define LOBYTE(DATA)   ((uint8_t)(DATA & 0x00FF))

#define Q10(X)	((X < 0.0) ? (int)(1024*(X) - 0.5) : (int)(1023*(X) + 0.5))
#define Q12(X)	((X < 0.0) ? (int)(4096*(X) - 0.5) : (int)(4095*(X) + 0.5))
#define Q15(X)	((X < 0.0) ? (int)(32768*(X) - 0.5) : (int)(32767*(X) + 0.5))
#define Q24(X)  ((X < 0.0) ? (long)(16777216*(X) - 0.5) : (long)(16777215*(X) + 0.5))
#define Q27(X)  ((X < 0.0) ? (long)(134217728*(X) - 0.5) : (long)(134217727*(X) + 0.5))

#define QP16(X) (unsigned int)(65536*(X) - 0.5)
/** RMS filter coefficient format */
#define IIR_QFORMAT  14
/** Shift of DC filter output (notch) in voltage loop */
#define NOTCH_TO_Q15  14
#define NOTCH_FILETER_S_120Hz 0
#define NOTCH_FILETER_S_100Hz 0
#define CASCADE_NOTCH_FILETER_INFENION 0
#define CASCADE_NOTCH_FILETER_S 0
// Square Root Approximation (res=sqrt(a))
// before calling has to be ensured: a>=0
// 10 lin.Intervals, Rel.Error<3% for operand>310, 12 asm instructions, comments below
#define  SQRTQ15lin10_FS(a) (   											  \
  ( 					 	   												  \
  ((a) < 1800) ?		  	   												  \
    ( ((a) < 700) ? 		       											  \
        ( ((a) < 30) ? (701 + 23 * ((a) - 15))								  \
          :  	       ( ((a) < 200) ? (1619 + 10 * ((a) - 80)) 			  \
                         :             (3840 + (((a) - 450)<<2)) 			  \
            )																  \
        )																	  \
      : ( ((a) < 1000) ? (5277 + 3 * ((a) - 850))     						  \
	      :              (6527 + (((long)(20564) * ((a) - 1300))>>13))		  \
		)																	  \
    )																		  \
  :	( ((a) < 9000) ? 														  \
	    ( ((a) < 4500) ? (9832 + (((long)(27302) * ((a) - 2950))>>14)) 		  \
          :              (14594 + (((long)(18393) * ((a) - 6500))>>14)) 	  \
        )																	  \
      :	( ((a) < 15000) ? (19829 + (((long)(27073) * ((a) - 12000))>>15))	  \
    	  :	( ((a) < 22000) ? (24286 + (((long)(22105) * ((a) - 18000))>>15)) \
			  :               (29955 + (((long)(17922) * ((a) - 27384))>>15)) \
			) 																  \
		)																	  \
    )																		  \
  ))

/******************************************************************************
                            Subroutine Declaration
*****************************************************************************/

/*****************************************************************************/

/*
 * End of file
 */
