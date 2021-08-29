/*******************************************************************************
 * Filename   : Dig_PFC.h
 * Project    : 
 * Created on : 
 * Author(s)  : Microchip reference SDC
 * 
 * Date of last Modification:
 * by $Author$:
 * 
 * Description: 
 *
 * Compiler info : MPLABX XC16 Version 1.5
 * Processor info: 
 *
 ******************************************************************************/

/******************************************************************************
                                Include Files
*******************************************************************************/
#include "Include_headers.h"

/******************************************************************************
                             Variable Declaration
*****************************************************************************/


/******************************************************************************
                             Constant Declaration
*****************************************************************************/
//======================= Digital PFC Configurations ===========================
#define HPFC                    0
#define PFC_CHANNEL		        2   // PFC channel number, 1 = single PFC or dual boost 
                                    // 2 = interleave traditional PFC or interleave HPFC
#define PFC_B_ON_BOARD          1   // 0 = means no channel B : mod for Hemi
#define PFC_A                   0
#define PFC_B                   1

#define EXTENDER_ON_BOARD       0

#define PFC_A_ALWAYS_OFF        0   // 1 = PFC_A keeps off
#define PFC_B_ALWAYS_OFF        0   // 1 = PFC_B keeps off

#define PFC_FREQ_CTRL           0           // PFC frequency changed by input current
#define JITTER_ENABLE           0           // 1 = switching frequency jitter

#define POWER_LIMIT_ENABLE      0   

#define FEEDFORWARD_ENABLE      1 //mod for hemi
#define DCM_FF_CAL_ENABLE       1 // disable DCM feedforward
#define FEEDFORWARD_SIMPLIFY    0

#define LOAD_BALANCING_ENABLE   1
//======================= Control Loop Configurations ==========================
#define PWMCLOCKFREQ        943360000       // 117.92*8*1000000

// make sure PFC_FREQ1 <= PFC_FREQ2 <= PFC_FREQ3 ... <= PFC_FREQ_AMX
#define PFC_FREQ1           (65E3)          // Hz, PFC switching frequency
#define PFC_FREQ_MAX        (90E3)      // Hz, maximum frequency


#define PFC_PWM_PERIOD1     (unsigned int)(PWMCLOCKFREQ / PFC_FREQ1)     // PFC PWM Period
#define PFC_PWM_PERIOD_MIN  (unsigned int)(PWMCLOCKFREQ / PFC_FREQ_MAX)  // minimum PWM Period to anti-mistake for division calculation
#define INITIAL_DUTY_CYCLE     0     // PFC initial PWM Duty
//======================= Parameter configuration ==============================
#define VinMax              509.76		
#define IinMax              50.461           //Amps
#define VbulkMax            560.175			 //Volts
#define PinMax              216.69		     //21669.997W = VinMax * IinMax //Nanyang no need    

#define IrefMax             31.9        //I_PFC_CT_A
							//(3.3/R) x CT Ratio * Voltage divider at Limit pin = 3.3 x (62//62//62) x 200 
#define IrefMaxCMP          52.7		//  I_PFC_CT_LIM_A    //max drain current of mosfet        
							//(3.3/R) x CT Ratio * Voltage divider at Limit pin = 3.3 x (62//62//62) x 200 x (15.4+10)/10

#define PFC_OC_CMP_set      20//30            //Clamp PWM Fault Current
#define PFC_OC_CMP_SET      Q12(PFC_OC_CMP_set/IrefMaxCMP) 

#define MAX_IREF            Q12(0.95)  // maxinmum current reference  
#define MIN_IREF            Q12(0.0)   // minimum current reference

#define	V_ERROR_MAX         (int16_t)Q12(10.0 / VbulkMax)  //(int16_t)Q12(20.0 / VbulkMax) ?
#define	V_ERROR_MIN         (int16_t)Q12(-10.0 / VbulkMax) //(int16_t)Q12(-20.0 / VbulkMax) ?

#define VBUS_GAP             Q12(20 / VbulkMax)//Q12(20.0 / VbulkMax)

#define	I_ERROR_MAX          Q12(0.05)

/* control loop parameter definition */
#define MAX_u0_I_HL         Q24(0.93)   //Q24(0.93)  // maximum duty @ high line 
#define MAX_u0_I_LL         Q24(0.95)  // maximum duty @ low line

#define MAX_u0_V            Q27(0.99)  // maxinmum voltage loop output
#define MIN_u0_V            Q27(0.0)   // minimum voltage loop output

#define MIN_u0_I            Q24(0.00)  // minimum duty



#define	K1_VIN              Q15(0.6)
#define	K2_VIN              Q15(0.4)

#define	K1_VBUS             Q15(0.2)
#define	K2_VBUS             Q15(0.8)


#define	KM		            Q12(VinMax / 60.0 / 1.41421356)  

//#define K1_V                2805// (on Hemi = 2000 )		//690 //5000//690  //25	//      // Q15 K1_V = KP + KI  	//mod for Hemi
//#define K2_V                -2700// (on Hemi =- 1000 )		//689// //-689 // 25	//     // Q15 K2_V  = -KP  	//mod for Hemi
//
//#define K1_I               	2200// (on Hemi = 2200 )		//18333 // 2200// 77	// 	//mod for Hemi
//#define K2_I               	-1750 // (on Hemi =- 300 )		//-2125// // -2125// 74	//	//mod for Hemi


//#define K1_V                2000// (on Hemi = 2000 )		//690 //5000//690  //25	//      // Q15 K1_V = KP + KI  	//mod for Hemi
//#define K2_V                -1000// (on Hemi =- 1000 )		//689// //-689 // 25	//     // Q15 K2_V  = -KP  	//mod for Hemi
//
//#define K1_I               	2200// (on Hemi = 2200 )		//18333 // 2200// 77	// 	//mod for Hemi
//#define K2_I               	-300 // (on Hemi =- 300 )		//-2125// // -2125// 74	//	//mod for Hemi
#define KP_V                  45000
//#define KI_V                  KP_V*4
#define KI_V_TS               11 //KI_V*1/65000
#define K1_V                  KP_V + KI_V_TS
#define K2_V                  KP_V - KI_V_TS
//#define K1_V                3100// (on Hemi = 2000 )		//690 //5000//690  //25	//      // Q15 K1_V = KP + KI  	//mod for Hemi
//#define K2_V                -1500// (on Hemi =- 1000 )	//-1800	//689// //-689 // 25	//     // Q15 K2_V  = -KP  	//mod for Hemi

//#define K1_I               5000//3600 two phase//4600 single// (on Hemi = 2200 )		//18333 // 2200// 77	// 	//mod for Hemi
//#define K2_I               -3000//-2000 two phase//-3000 single// (on Hemi =- 300 )		//-2125// // -2125// 74	//	//mod for Hemi
#define KP_I                  4000
//#define KI_I                  KP_I*4
#define KI_I_TS               1000 //KI_I*1/65000
#define K1_I                  KP_I + KI_I_TS
#define K2_I                  -KP_I + KI_I_TS
// K1_V	690
// K2_V	-689
	
// K1_I	18333
// K2_I	-17708

/* K = (2 * L / Ts) * (IrefMax / VinMax) = (2 * 600uH / 16.67us) * (27.71A / 496.108V) = 4.0215 */
#define	KL                  Q12(2.00)               //For Low Line       
#define	KL_115              Q12(2.30)               //Fo High Line (115% of KL)
//#define	KL                  Q12(4.0215)               //For Low Line       
//#define	KL_115              Q12(4.6247)               //Fo High Line (115% of KL)

//#define MAX_FFORWARD        Q12(1.0)
#define MAX_FFORWARD_CCM    Q12(0.98)  //  Q12(0.97)  
#define MAX_FFORWARD_DCM    Q12(0.93)   // Q12(0.93)  

#define	VLOOP_LOWLOAD_H     Q12(0.0035)
#define	VLOOP_LOWLOAD_L     Q12(0.003)

#define REFLOADBALCURRENT   Q12(0.0)
#define K1_LOADBAL          Q12(0.2)
#define K2_LOADBAL          Q12(0.4)
#define MAX_BALANCE         100//

/******************************************************************************
                             Macro Declaration
*****************************************************************************/

/******************************************************************************
                            Subroutine Declaration
*****************************************************************************/
void ControlLoopInit(void);
void __attribute__((__section__("APP_ISR_IMAGE"))) Control_Loop(void);


/*****************************************************************************/
/*
 * End of file
 */
