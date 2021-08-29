/*******************************************************************************
 * Filename   : Task_Handler.h
 * Project    : 
 * Created on :
 * Author(s)  : 
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
//Control Status Flags
typedef union {
    unsigned int i16[4];
    struct {
        //0
        unsigned Tsk_200uS:1;
        unsigned :1;
        unsigned :1;
        unsigned :1;
        unsigned :1;
        unsigned :1;
        unsigned :1;
        unsigned :1;
        unsigned :1;
        unsigned :1;
        unsigned :1;
        unsigned :1;
        unsigned :1;
        unsigned :1;
        unsigned :1;
        unsigned :1;
        //1
        unsigned Tasks_1mS:8;
        unsigned Tasks_10mS:8;
        
        //2
        unsigned Tasks_100mS:8;
        unsigned AC_Tasks:8;
        
        //3
        unsigned :16;
    };
}FLAG_STA;
extern FLAG_STA Control;

typedef struct
{
    unsigned int VPeak_Temp;
    unsigned int IPeak_Temp;
    unsigned int Count;
}VinVoltage;
extern VinVoltage Vin_Mon;

//Primary Data
typedef union {
    unsigned int i16[80];
    unsigned char i8[160];
    struct{//Integer 
        // Main Data - 21 Words
        unsigned int STATUS1;                   //0
        unsigned int STATUS2;                   //1
        unsigned int ALA_STAT;                  //2
        unsigned int ALA_LATCH;                 //3
        unsigned int VRMS_Avg;                  //4
        unsigned int IRMS_Avg;                  //5
        unsigned int PIN_Avg;                   //6
        unsigned int DC_BULK;                   //7
        unsigned int T_PFCFET_A;             //8
        unsigned int T_PFCFET_B;             //9
        unsigned int T_BOOSTDIODE;            //10
        unsigned int FREQ_PF;                   //11
        unsigned int VRMS_Inst;                 //12
        unsigned int IRMS_Inst;                 //13
        unsigned int PIN_Inst;                  //14
        unsigned int BL_STATUS;                 //15
        unsigned int REV;                  //16 modified unsigned int Microchip reference_REV;
        unsigned int CIS_REV;                   //17
        unsigned int RES_1;                     //18
        unsigned int RES_2;                     //19
        unsigned int RES_3;                     //20
        
        //Debug Data 's 12 Words
        unsigned int VRms_ADC;                  //21
        unsigned int IRms_ADC;                  //22
        unsigned int PIN_Rms_ADC;               //23
        unsigned int VRMS_ADC_Avg;              //24
        unsigned int IRMS_ADC_Avg;              //25
        unsigned int PIN_ADC_Avg;               //26
        unsigned int VIN_PEAK_ADC;              //27
        unsigned int IIN_PEAK_ADC;              //28
        unsigned int BULK_ADC_AVG;              //29
        unsigned int D_Data[3];                 //30 - 32
        
        //Cal Data 's 32 - 76
        unsigned int VIN_LL_Gain;               //33    
        unsigned int VIN_HL_Gain;               //34
        unsigned int VIN_Div;                   //35
                
        unsigned int IIN_LL_1A_Gain;            //36 - 53
        unsigned int IIN_LL_1A_ADC;             //  
        unsigned int IIN_LL_1A_Val;             //  
        unsigned int IIN_LL_4A_Gain;            //  
        unsigned int IIN_LL_4A_ADC;             //  
        unsigned int IIN_LL_4A_Val;             //  
        unsigned int IIN_LL_10A_Gain;           //  
        unsigned int IIN_LL_10A_ADC;            //  
        unsigned int IIN_LL_10A_Val;            //  
        unsigned int IIN_HL_0A5_Gain;           //  
        unsigned int IIN_HL_0A5_ADC;            //  
        unsigned int IIN_HL_0A5_Val;            //  
        unsigned int IIN_HL_2A_Gain;            //   
        unsigned int IIN_HL_2A_ADC;             //  
        unsigned int IIN_HL_2A_Val;             //  
        unsigned int IIN_HL_5A_Gain;            //  
        unsigned int IIN_HL_5A_ADC;             //  
        unsigned int IIN_HL_5A_Val;             //  
         
        unsigned int PIN_LL_1A_Gain;            //54 - 71
        unsigned int PIN_LL_1A_ADC;             //  
        unsigned int PIN_LL_1A_Val;             //  
        unsigned int PIN_LL_4A_Gain;            //  
        unsigned int PIN_LL_4A_ADC;             //  
        unsigned int PIN_LL_4A_Val;             //  
        unsigned int PIN_LL_10A_Gain;           //  
        unsigned int PIN_LL_10A_ADC;            //  
        unsigned int PIN_LL_10A_Val;            //  
        unsigned int PIN_HL_0A5_Gain;           //  
        unsigned int PIN_HL_0A5_ADC;            //  
        unsigned int PIN_HL_0A5_Val;            //  
        unsigned int PIN_HL_2A_Gain;            //  
        unsigned int PIN_HL_2A_ADC;             //  
        unsigned int PIN_HL_2A_Val;             //  
        unsigned int PIN_HL_5A_Gain;            //  
        unsigned int PIN_HL_5A_ADC;             //  
        unsigned int PIN_HL_5A_Val;             //          
        
        unsigned int BULK_GAIN;                 //72
        unsigned int CAL_RES1;                  //73          
        unsigned int CAL_RES2;                  //74
        
        unsigned int IIN_Div;                   //75
        unsigned int PIN_Div;                   //76    
        unsigned int BULK_DIV;                  //77           
        
        unsigned int CAL_RES3;                  //78    
        unsigned int CAL_LOCK_KEY;              //79         
        unsigned int CAL_DATA_CRC;              //80
    };
    struct{//Bits
        //Status1 - 0
        unsigned Startup:1;
        unsigned AC_OK:1;
        unsigned BULK_OK:1;
        unsigned Relay_Start:1;

        unsigned Relay_PWM_ON:1;
        unsigned PFC_GO:1;
        unsigned PFC_En:1;
        unsigned Sec_CMD_PFC_Dis:1;
        
        unsigned Sec_CMD_Droop_En:1;
        unsigned VHVDC_IN:1;
        unsigned VAC_Range_LL:1;

        unsigned Eep_Wr_Sta:1;
        unsigned Cal_CRC_Err:1;
        unsigned Cal_Unlock_Sta:1;                      //0 - Disabled , 1 = Enabled
        unsigned Flash_Wr_Sta:2;

         //Status2 -  Digital PFC Status bits
        unsigned CL_1st_PASS:1;
        unsigned CL_SoftStart:1;
        unsigned FF_Boost_1:1;
        unsigned Bulk_High_Sta:1;

        unsigned Bulk_Range:4;
        
        unsigned VPOL:1;
        unsigned VPOL_1:1;
        unsigned VPOL_2:1;
        unsigned New_AC_Data:1;
        
        unsigned Start_AC_Calc:1;
        unsigned :1;
        unsigned :1;
        unsigned :1;
        
        //Alarm - 2
        unsigned A_AC_Dropout:1;
        unsigned A_AC_IUV:1;            //Instant UV
        unsigned A_AC_IOV:1;            //Instant OV
        unsigned A_AC_Sag:1;
        unsigned A_AC_OV:1;             //Cycle RMS OV    
        unsigned :1;
        unsigned A_ALERT_PFC:1;          //Alert to Control Loop  
        unsigned A_ALERT_SEC:1;          //Alert to Secondary Side for PG requirements
        unsigned A_AC_UVW:1;
        unsigned A_AC_OVW:1;
        unsigned A_AC_OCW:1;
        unsigned A_AC_OPW:1;
        unsigned A_BULK_UV1:1;
        unsigned A_BULK_UV2:1;
        unsigned A_BULK_OV_ALERT:1;
        unsigned A_BULK_OVP:1;
        
        //Alarm Latch - 3
        unsigned :16;
        
        //4 - 7
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        
        //8 - 19
        unsigned :16;
        unsigned :16;
        
        unsigned VIN_FREQ:8;
        unsigned VIN_PF:8;
        
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;        
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        
        //20 - 35
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
                
        //36 - 38 - VCAL Data
        unsigned :16;        
        unsigned :16;
        unsigned :16;
        
        //39 - 41 - IIn PIN Div Data
        unsigned :16;
        unsigned :16;
        unsigned :16;
        
        //42 - 50 - IIN LL Cal Data
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        
        //51 - 59 - IIN HL Cal Data
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        
                
        //60 - 68 - PIN LL Cal Data
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        
        //69 - 77 - PIN HL Cal Data
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        unsigned :16;
        
        //78 - 79 - Bulk CAL Data
        unsigned :16;        
        unsigned :16;
    };
}PRIMARY_DATA;
extern PRIMARY_DATA PRI_DATA;

//AC ALARM Variables
extern unsigned int Dropout_Delay;

//Bus Control
extern unsigned int Vbus_Ref[5];
extern signed int VbusRef,VRefRead;

//For Control Loop
extern signed int Vavg_Inv_Sq;

//EEPROM Write
extern unsigned int Eep_Cntl_State;
typedef union 
{
    unsigned int i16[48];
    unsigned char i8[96];
}CAL_DATA_BUFFER;
extern CAL_DATA_BUFFER Cal_Data_Buf;

//Power Metering
typedef struct
{
    unsigned int Sample_Cnt_Tmp;
    unsigned long long Vin_Sum_Tmp;
    unsigned long long Iin_Sum_Tmp;
    unsigned long long Pin_Sum_Tmp;
    
    unsigned int Sample_Cnt;
    unsigned long long Vin_Sum;
    unsigned long long Iin_Sum;
    unsigned long long Pin_Sum;
    
    unsigned int Sample_Cnt2;
    unsigned long long Vin_Sum2;
    unsigned long long Iin_Sum2;
    unsigned long long Pin_Sum2;
    
    unsigned long VRms_Sum;
    unsigned long IRms_Sum;
    unsigned long PRms_Sum;
    unsigned long VRms_ADC_Sum;
    unsigned long IRms_ADC_Sum;
    unsigned long PRms_ADC_Sum;
    unsigned int Rms_Avg_Count;    
}POWER_METER;
extern POWER_METER Pmeter;

extern unsigned int Vbus;
extern unsigned int Vin;
extern unsigned int Iin; 
extern unsigned int PfcPeriod;
extern unsigned int PfcPeriodBase;
extern signed long Max_u0_i;

//Future Variables
typedef union{
    unsigned int i16[32];
    unsigned char i8[64];
    struct{
        unsigned :16;       //0
        unsigned :16;       //2
        unsigned :16;       //4
        unsigned :16;       //6

        unsigned :16;       //8
        unsigned :16;       //10
        unsigned :16;       //12
        unsigned :16;       //14

        unsigned :16;       //16
        unsigned :16;       //18
        unsigned :16;       //20
        unsigned :16;       //22

        unsigned :16;       //24
        unsigned :16;       //26
        unsigned :16;       //28
        unsigned :16;       //30
        
        //Second Group
        unsigned :16;       //0
        unsigned :16;       //2
        unsigned :16;       //4
        unsigned :16;       //6

        unsigned :16;       //8
        unsigned :16;       //10
        unsigned :16;       //12
        unsigned :16;       //14

        unsigned :16;       //16
        unsigned :16;       //18
        unsigned :16;       //20
        unsigned :16;       //22

        unsigned :16;       //24
        unsigned :16;       //26
        unsigned :16;       //28
        unsigned :16;       //30
    };
}ADD_VARIABLES;
extern ADD_VARIABLES VAR;
/** IIR filter structure, used in AC-RMS and DC-Notch filtering */
/*
typedef struct IIR_lowpass_int
{
	int16_t a1[3];				//< Filter numerator coefficients, section 1. Q format. 
	int16_t b1[3];				//< Filter denominator coefficients, section 1. Q format.
	int32_t vbus_filter[3];			//< Intermediate calculation, section 1. 
	int16_t a2[3];				//< Filter numerator coefficients, section 2. Q format.
	int16_t b2[3];				//< Filter denominator coefficients, section 2. Q format.
	int32_t Qvbus[3];				//< Intermediate calculation, section 2. 
	int32_t output1;			//< Output of the filter, section 1. 
	int32_t output;				//< Output of the filter. Q format 

	int32_t calc11;				//< Intermediate calculation for section 1
	int32_t calc21;				//< Intermediate calculation for section 1 
	int32_t calc12;				//< Intermediate calculation for section 2 
	int32_t calc22;				//< Intermediate calculation for section 2 

} IIR_lowpass_int_t;
*/
typedef struct IIR_lowpass_int
{
	int16_t k1;					/**< Filter input gain, section 1. Q format. */
	int16_t a1[3];				/**< Filter numerator coefficients, section 1. Q format. */
	int16_t b1[3];				/**< Filter denominator coefficients, section 1. Q format. */
	int32_t u1[3];				/**< Intermediate calculation, section 1. */
	int16_t k2;					/**< Filter input gain, section 2. Q format. */
	int16_t a2[3];				/**< Filter numerator coefficients, section 2. Q format. */
	int16_t b2[3];				/**< Filter denominator coefficients, section 2. Q format. */
	int32_t u2[3];				/**< Intermediate calculation, section 2. */
	int32_t output1;			/**< Output of the filter, section 1. */
	int32_t output;				/**< Output of the filter. Q format */
    int16_t vbus_filt1[3];
    int16_t vbus1[3];
    int16_t vbus_filt2[3];
    int16_t vbus2[3];
    
	int32_t calc11;				/**< Intermediate calculation for section 1*/
	int32_t calc21;				/**< Intermediate calculation for section 1 */
	int32_t calc12;				/**< Intermediate calculation for section 2 */
	int32_t calc22;				/**< Intermediate calculation for section 2 */
    
    int32_t tmpb0u0;				/**< Intermediate calculation for section 1*/
	int32_t tmpb1u1;				/**< Intermediate calculation for section 1 */
	int32_t tmpb2u2;				/**< Intermediate calculation for section 2 */
	int32_t tmpa1y1;	
 	int32_t tmpa2y2;	  
} IIR_lowpass_int_t;
extern volatile IIR_lowpass_int_t DCfilter;
/******************************************************************************
                             Constant Declaration
*****************************************************************************/
#define VBULK_RESOLUTION        0.5       //in V
#define VBULK_RES_INV           (unsigned int)(1/VBULK_RESOLUTION)

#define VIN_RESOLUTION          0.125       //in V
#define VIN_RES_INV             (unsigned int)(1/VIN_RESOLUTION)
 
#define IIN_RESOLUTION          0.015625    //in Amps
#define IIN_RES_INV             (unsigned int)(1/IIN_RESOLUTION)

#define PIN_RESOLUTION          0.25       //in Watts
#define PIN_RES_INV             (unsigned int)(1/PIN_RESOLUTION)

//Default Calibration Settings
    //For VIN
// ACV Gain value for 125mV resolution 
#define DEF_AC_VGAIN_LL         4078//4052
#define DEF_AC_VGAIN_HL         4078//4009
// ACV Divisor value for Both Lines for 125mV Resolution
#define DEF_AC_VDIV             12

    //For IIN for 0.15625 mA Resolution
#define DEF_AC_1A_IGAIN_LL      1665
#define DEF_AC_1A_ADC_LL        123
#define DEF_AC_1A_CUR_LL        50
#define DEF_AC_4A_IGAIN_LL      1457
#define DEF_AC_4A_ADC_LL        227
#define DEF_AC_4A_CUR_LL        87
#define DEF_AC_10A_IGAIN_LL     1516
#define DEF_AC_10A_ADC_LL       2175
#define DEF_AC_10A_CUR_LL       808

#define DEF_AC_0A5_IGAIN_HL     2107
#define DEF_AC_0A5_ADC_HL       70
#define DEF_AC_0A5_CUR_HL       36
#define DEF_AC_2A_IGAIN_HL      1365
#define DEF_AC_2A_ADC_HL        142
#define DEF_AC_2A_CUR_HL        60
#define DEF_AC_5A_IGAIN_HL      1502
#define DEF_AC_5A_ADC_HL        1410
#define DEF_AC_5A_CUR_HL        525

    //For PIN for 0.25 W Resolution
#define DEF_AC_1A_PGAIN_LL      1568
#define DEF_AC_1A_PADC_LL       410
#define DEF_AC_1A_PWR_LL        314
#define DEF_AC_4A_PGAIN_LL      1540
#define DEF_AC_4A_PADC_LL       797 
#define DEF_AC_4A_PWR_LL        605 
#define DEF_AC_10A_PGAIN_LL     1536
#define DEF_AC_10A_PADC_LL      7637
#define DEF_AC_10A_PWR_LL       5734

#define DEF_AC_0A5_PGAIN_HL     1766
#define DEF_AC_0A5_PADC_HL      472
#define DEF_AC_0A5_PWR_HL       407
#define DEF_AC_2A_PGAIN_HL      1477
#define DEF_AC_2A_PADC_HL       1021
#define DEF_AC_2A_PWR_HL        803 
#define DEF_AC_5A_PGAIN_HL      1489
#define DEF_AC_5A_PADC_HL       10206
#define DEF_AC_5A_PWR_HL        7483
          
// ACI Divisor value for Both Lines for 15.625mA Resolution
#define DEF_AC_IDIV             12
// ACI Divisor value for Both Lines for 250 mW Resolution
#define DEF_AC_PDIV             11

    //For Bulk
// (char) - Gain for Bulk Voltage @ 500mV resolution 
#define DEF_BULK_GAIN           1120
// (char) - Divide by 4096
#define DEF_BULK_SCALE          12

#define CALIB_LOCK_KEY          0xCD
#define CALIB_UNLOCK_KEY        0xCA

#define CAL_DATA_CRC8_VAL       0x8A        //Run Debug Mode and Find CRC if any change in Default Value

//Fault Trigger & Recover Thresholds Delays
//Dropout - Compare with Peak Instant Value -
#define DROPOUT_TRIG_VOLTAGE    20        // in RMS 
#define DROPOUT_TRIG_THD        (DROPOUT_TRIG_VOLTAGE * VIN_RES_INV)  // in 0.125V Resolution
#define DROPOUT_TRIG_ADC_RMS    (unsigned int)(((unsigned long)DROPOUT_TRIG_THD<<DEF_AC_VDIV)/(unsigned int)DEF_AC_VGAIN_LL)
#define DROPOUT_TRIG_ADC_PK     (unsigned int)((float)DROPOUT_TRIG_ADC_RMS * 1.41421356)

//Dynamic Dropout Calculation
#define DROPOUT_TRIG_DELAY_FL       149        // >30mS in 200uS Resolution
#define DROPOUT_ALERT_DELAY         17         // >3.4mS Delay

#define DROPOUT_MAXLOAD_ACT     3000
#define DROPOUT_MAXLOAD         (DROPOUT_MAXLOAD_ACT * PIN_RES_INV)  // in 250mW Resolution
#define ADD_DO_GAIN             237         //Additinal Dropout Gain for +60mS with Divider <<12


//Instant Value Under Voltage - Compare with Peak Instant Value
#define INST_UV_TRIG_VOLTAGE    VIN_OK_TRIG_VOLTAGE*0.7 //70         // in RMS Volts // PFC will only "stop" switching if Vin is below this voltage. (shutdown condition)
#define INST_UV_TRIG_THD        (INST_UV_TRIG_VOLTAGE * VIN_RES_INV)  //  in 0.125V Resolution
#define INST_UV_TRIG_ADC_RMS    (unsigned int)(((unsigned long)INST_UV_TRIG_THD<<DEF_AC_VDIV)/(unsigned int)DEF_AC_VGAIN_LL)
#define INST_UV_TRIG_ADC_PK     (unsigned int)((float)INST_UV_TRIG_ADC_RMS * 1.41421356)

#define INST_UV_TRIG_DELAY      175         // >35mS in 200uS Resolution
#define INST_UV_ALERT_DELAY     150         // >30mS in 200uS Resolution

//Instant Value Over Voltage - Compare with Peak Instant Value
#define INST_OV_TRIG_VOLTAGE    280         // in RMS Volts
#define INST_OV_TRIG_THD        (INST_OV_TRIG_VOLTAGE * VIN_RES_INV)  // in Converted RMS Volts
#define INST_OV_TRIG_ADC_RMS    (unsigned int)(((unsigned long)INST_OV_TRIG_THD<<DEF_AC_VDIV)/(unsigned int)DEF_AC_VGAIN_HL)
#define INST_OV_TRIG_ADC_PK     (unsigned int)((float)INST_OV_TRIG_ADC_RMS * 1.41421356)

#define INST_OV_TRIG_DELAY      2           // >0.4mS in 200uS Resolution

//RMS VIN SAG Voltage - Compared with RMS VIN ADC
#define AC_SAG_TRIG_VOLTAGE     VIN_OK_TRIG_VOLTAGE*0.8         // in RMS Volts
#define AC_SAG_TRIG_THD         (AC_SAG_TRIG_VOLTAGE * VIN_RES_INV)  // in Converted RMS Volts

#define AC_SAG_TRIG_DELAY       600         // >600mS in 1mS Resolution
#define AC_SAG_ALERT_DELAY      100         // >100mS in 1mS Resolution

//RMS VIN OV Trigger - Compared with RMS VIN ADC 
#define AC_OV_TRIG_VOLTAGE      280         // in RMS Volts
#define AC_OV_TRIG_THD          (AC_OV_TRIG_VOLTAGE * VIN_RES_INV)  // in Converted RMS Volts

#define AC_OV_REC_VOLTAGE       270         // in RMS Volts
#define AC_OV_REC_THD           (AC_OV_REC_VOLTAGE * VIN_RES_INV)  // in Converted RMS Volts

#define AC_OV_TRIG_DELAY        100         // >100mS in 1mS Resolution
#define AC_OV_REC_DELAY         200         // >200mS in 1mS Resolution

//RMS VIN OK Voltage - Compared with RMS VIN ADC
#define VIN_OK_TRIG_VOLTAGE    	175//150         // in RMS Volts // PFC will only "start" switching if Vin is above this voltage. (startup condition)
#define VIN_OK_TRIG_THD         (VIN_OK_TRIG_VOLTAGE * VIN_RES_INV)  // in Converted RMS Volts

#define	VIN_OK_DELAY           100   // 100 * 1ms = 100mS

#define	AC_HALF_PERIOD_MAX     56          // 56 * 200us = 11.2mS 45Hz

//AC Warning Thresholds
    //UVW
#define AC_UVW_TRIG_VOLTAGE      VIN_OK_TRIG_VOLTAGE*0.5 //80//90         // in RMS Volts
#define AC_UVW_TRIG_THD          (AC_UVW_TRIG_VOLTAGE * VIN_RES_INV)  // in Converted RMS Volts

#define AC_UVW_REC_VOLTAGE       AC_UVW_TRIG_VOLTAGE*1.05 //83//93         // in RMS Volts
#define AC_UVW_REC_THD           (AC_UVW_REC_VOLTAGE * VIN_RES_INV)  // in Converted RMS Volts

#define AC_UVW_TRIG_DELAY        AC_SAG_TRIG_DELAY + 50         // >650mS in 1mS Resolution
#define AC_UVW_REC_DELAY         200         // >200mS in 1mS Resolution

    //OVW
#define AC_OVW_TRIG_VOLTAGE      275         // in RMS Volts
#define AC_OVW_TRIG_THD          (AC_OVW_TRIG_VOLTAGE * VIN_RES_INV)  // in Converted RMS Volts

#define AC_OVW_REC_VOLTAGE       270         // in RMS Volts
#define AC_OVW_REC_THD           (AC_OVW_REC_VOLTAGE * VIN_RES_INV)  // in Converted RMS Volts

#define AC_OVW_TRIG_DELAY        150         // >150mS in 1mS Resolution
#define AC_OVW_REC_DELAY         200         // >200mS in 1mS Resolution

//OCW
#define AC_OCW_TRIG_CURRENT      18         // 18A in 0.015625mA Res
#define AC_OCW_TRIG_THD          (AC_OCW_TRIG_CURRENT * IIN_RES_INV)  

#define AC_OCW_REC_CURRENT       17.2         // 17.2A in 0.015625mA Res
#define AC_OCW_REC_THD           (AC_OCW_REC_CURRENT * IIN_RES_INV)  

#define AC_OCW_TRIG_DELAY        200         // >200mS in 1mS Resolution
#define AC_OCW_REC_DELAY         200         // >200mS in 1mS Resolution

//VBUS Reference for Droop and Active Mode
//Active Mode HL

#define REFERENCE_OUTPUT_VOLTAGE	       430//380         // in Volts 

#define VBUS_REF0_ACT_VHL       REFERENCE_OUTPUT_VOLTAGE  //380      // in Volts  
#define VBUS_REF0_ACT_THD_HL    (VBUS_REF0_ACT_VHL * VBULK_RES_INV)  // in 500mV Resolution
#define VBUS_REF0_ACT_ADC_HL    (unsigned int)(((unsigned long)VBUS_REF0_ACT_THD_HL<<DEF_BULK_SCALE)/(unsigned int)DEF_BULK_GAIN) 

#define VBUS_REF1_ACT_VHL       REFERENCE_OUTPUT_VOLTAGE  //380         // in Volts 
#define VBUS_REF1_ACT_THD_HL    (VBUS_REF1_ACT_VHL * VBULK_RES_INV)  // in 500mV Resolution
#define VBUS_REF1_ACT_ADC_HL    (unsigned int)(((unsigned long)VBUS_REF1_ACT_THD_HL<<DEF_BULK_SCALE)/(unsigned int)DEF_BULK_GAIN) 

#define VBUS_REF2_ACT_VHL       REFERENCE_OUTPUT_VOLTAGE //400         //
#define VBUS_REF2_ACT_THD_HL    (VBUS_REF2_ACT_VHL * VBULK_RES_INV)  // in 500mV Resolution
#define VBUS_REF2_ACT_ADC_HL    (unsigned int)(((unsigned long)VBUS_REF2_ACT_THD_HL<<DEF_BULK_SCALE)/(unsigned int)DEF_BULK_GAIN) 

#define VBUS_REF3_ACT_VHL       REFERENCE_OUTPUT_VOLTAGE //415          //
#define VBUS_REF3_ACT_THD_HL    (VBUS_REF3_ACT_VHL * VBULK_RES_INV)  // in 500mV Resolution
#define VBUS_REF3_ACT_ADC_HL    (unsigned int)(((unsigned long)VBUS_REF3_ACT_THD_HL<<DEF_BULK_SCALE)/(unsigned int)DEF_BULK_GAIN) 

#define VBUS_REF4_ACT_VHL       REFERENCE_OUTPUT_VOLTAGE //415         //
#define VBUS_REF4_ACT_THD_HL    (VBUS_REF4_ACT_VHL * VBULK_RES_INV)  // in 500mV Resolution
#define VBUS_REF4_ACT_ADC_HL    (unsigned int)(((unsigned long)VBUS_REF4_ACT_THD_HL<<DEF_BULK_SCALE)/(unsigned int)DEF_BULK_GAIN) 

//Active Mode LL
#define VBUS_REF0_ACT_VLL       REFERENCE_OUTPUT_VOLTAGE//375         // in Volts
#define VBUS_REF0_ACT_THD_LL    (VBUS_REF0_ACT_VLL * VBULK_RES_INV)  // in 500mV Resolution
#define VBUS_REF0_ACT_ADC_LL    (unsigned int)(((unsigned long)VBUS_REF0_ACT_THD_LL<<DEF_BULK_SCALE)/(unsigned int)DEF_BULK_GAIN) 

#define VBUS_REF1_ACT_VLL       REFERENCE_OUTPUT_VOLTAGE//375         // in Volts
#define VBUS_REF1_ACT_THD_LL    (VBUS_REF1_ACT_VLL * VBULK_RES_INV)  // in 500mV Resolution
#define VBUS_REF1_ACT_ADC_LL    (unsigned int)(((unsigned long)VBUS_REF1_ACT_THD_LL<<DEF_BULK_SCALE)/(unsigned int)DEF_BULK_GAIN) 

#define VBUS_REF2_ACT_VLL       REFERENCE_OUTPUT_VOLTAGE//375         //
#define VBUS_REF2_ACT_THD_LL    (VBUS_REF2_ACT_VLL * VBULK_RES_INV)  // in 500mV Resolution
#define VBUS_REF2_ACT_ADC_LL    (unsigned int)(((unsigned long)VBUS_REF2_ACT_THD_LL<<DEF_BULK_SCALE)/(unsigned int)DEF_BULK_GAIN) 

#define VBUS_REF3_ACT_VLL       REFERENCE_OUTPUT_VOLTAGE//385          //
#define VBUS_REF3_ACT_THD_LL    (VBUS_REF3_ACT_VLL * VBULK_RES_INV)  // in 500mV Resolution
#define VBUS_REF3_ACT_ADC_LL    (unsigned int)(((unsigned long)VBUS_REF3_ACT_THD_LL<<DEF_BULK_SCALE)/(unsigned int)DEF_BULK_GAIN) 

#define VBUS_REF4_ACT_VLL       REFERENCE_OUTPUT_VOLTAGE//390         //
#define VBUS_REF4_ACT_THD_LL    (VBUS_REF4_ACT_VLL * VBULK_RES_INV)  // in 500mV Resolution
#define VBUS_REF4_ACT_ADC_LL    (unsigned int)(((unsigned long)VBUS_REF4_ACT_THD_LL<<DEF_BULK_SCALE)/(unsigned int)DEF_BULK_GAIN) 

  //Droop Mode
#define VBUS_REF0_DRP_V         REFERENCE_OUTPUT_VOLTAGE//380         //
#define VBUS_REF0_DRP_THD       (VBUS_REF0_DRP_V * VBULK_RES_INV)  // in 500mV Resolution
#define VBUS_REF0_DRP_ADC       (unsigned int)(((unsigned long)VBUS_REF0_DRP_THD<<DEF_BULK_SCALE)/(unsigned int)DEF_BULK_GAIN) 

#define VBUS_REF1_DRP_V         REFERENCE_OUTPUT_VOLTAGE//384         //
#define VBUS_REF1_DRP_THD       (VBUS_REF1_DRP_V * VBULK_RES_INV)  // in 500mV Resolution
#define VBUS_REF1_DRP_ADC       (unsigned int)(((unsigned long)VBUS_REF1_DRP_THD<<DEF_BULK_SCALE)/(unsigned int)DEF_BULK_GAIN) 

#define VBUS_REF2_DRP_V         REFERENCE_OUTPUT_VOLTAGE//388         //
#define VBUS_REF2_DRP_THD       (VBUS_REF2_DRP_V * VBULK_RES_INV)  // in 500mV Resolution
#define VBUS_REF2_DRP_ADC       (unsigned int)(((unsigned long)VBUS_REF2_DRP_THD<<DEF_BULK_SCALE)/(unsigned int)DEF_BULK_GAIN) 

#define VBUS_REF3_DRP_V         REFERENCE_OUTPUT_VOLTAGE//400   //     
#define VBUS_REF3_DRP_THD       (VBUS_REF3_DRP_V * VBULK_RES_INV)  // in 500mV Resolution
#define VBUS_REF3_DRP_ADC       (unsigned int)(((unsigned long)VBUS_REF3_DRP_THD<<DEF_BULK_SCALE)/(unsigned int)DEF_BULK_GAIN) 

#define VBUS_REF4_DRP_V         REFERENCE_OUTPUT_VOLTAGE//407  //       
#define VBUS_REF4_DRP_THD       (VBUS_REF4_DRP_V * VBULK_RES_INV)  // in 500mV Resolution
#define VBUS_REF4_DRP_ADC       (unsigned int)(((unsigned long)VBUS_REF4_DRP_THD<<DEF_BULK_SCALE)/(unsigned int)DEF_BULK_GAIN) 

#define VBUS_DEBUG_REF          REFERENCE_OUTPUT_VOLTAGE*1.3//407  //       
#define VBUS_DEBUG_REF_THD       (VBUS_DEBUG_REF * VBULK_RES_INV)  // in 500mV Resolution
#define VBUS_DEBUG_REF_ADC       (unsigned int)(((unsigned long)VBUS_DEBUG_REF_THD<<DEF_BULK_SCALE)/(unsigned int)DEF_BULK_GAIN) 


//Bulk UV Fault 1 - Fast Response
#define BULK_UV1_TRIG           330//REFERENCE_OUTPUT_VOLTAGE*0.7         // <275V
#define BULK_UV1_TRIG_THD       (BULK_UV1_TRIG * VBULK_RES_INV)  // in 500mV Resolution
#define BULK_UV1_TRIG_ADC       (unsigned int)(((unsigned long)BULK_UV1_TRIG_THD<<DEF_BULK_SCALE)/(unsigned int)DEF_BULK_GAIN) 
#define BULK_UV1_TRIG_DELAY     24          // >5mS in 200uS Resolution

//Bulk UV Fault 2 - Slow Response
#define BULK_UV2_TRIG           370 //REFERENCE_OUTPUT_VOLTAGE*0.75         // <310V
#define BULK_UV2_TRIG_THD       (BULK_UV2_TRIG * VBULK_RES_INV)  // in 500mV Resolution
#define BULK_UV2_TRIG_ADC       (unsigned int)(((unsigned long)BULK_UV2_TRIG_THD<<DEF_BULK_SCALE)/(unsigned int)DEF_BULK_GAIN) 
#define BULK_UV2_TRIG_DELAY     2          // >0.6mS in 200uS Resolution

//Bulk OK Threshold
#define BULK_OK_THRES           410//400 //REFERENCE_OUTPUT_VOLTAGE*0.8	//360         // >360V
#define BULK_OK_THD             (BULK_OK_THRES * VBULK_RES_INV)  // in 500mV Resolution
#define BULK_OK_ADC             (unsigned int)(((unsigned long)BULK_OK_THD<<DEF_BULK_SCALE)/(unsigned int)DEF_BULK_GAIN) 
#define BULK_OK_DELAY           10//4       // >1mS in 200uS Resolution

//Bulk OV Alert 
#define BULK_OV_ALERT           REFERENCE_OUTPUT_VOLTAGE*1.2//440         // >440V
#define BULK_OV_ALERT_THD       (BULK_OV_ALERT * VBULK_RES_INV)  // in 500mV Resolution
#define BULK_OV_ALERT_ADC       (unsigned int)(((unsigned long)BULK_OV_ALERT_THD<<DEF_BULK_SCALE)/(unsigned int)DEF_BULK_GAIN) 

#define BULK_OV_ALERT_REC       REFERENCE_OUTPUT_VOLTAGE*1.3//430         // <430V
#define BULK_OV_ALERT_REC_THD   (BULK_OV_ALERT_REC * VBULK_RES_INV)  // in 500mV Resolution
#define BULK_OV_ALERT_REC_ADC   (unsigned int)(((unsigned long)BULK_OV_ALERT_REC_THD<<DEF_BULK_SCALE)/(unsigned int)DEF_BULK_GAIN) 

//Bulk OVP 
#define BULK_OVP           		REFERENCE_OUTPUT_VOLTAGE*1.3//440         // >440V//475   //
#define BULK_OVP_THD       		(BULK_OVP * VBULK_RES_INV)  // in 500mV Resolution
#define BULK_OVP_ADC       		(unsigned int)(((unsigned long)BULK_OVP_THD<<DEF_BULK_SCALE)/(unsigned int)DEF_BULK_GAIN) 

//===================== Relay Configurations ===================================
#define RELAY_FREQ            100E3
#define PWM_RELAY_PERIOD      (PWMCLOCKFREQ / RELAY_FREQ)           //PWM Clock Freq Defined in Dig_PFC.h

//Relay Control 
#define RELAY_ON_VOLTAGE        VIN_OK_TRIG_VOLTAGE*0.8        // in RMS 
#define RELAY_ON_THD            (RELAY_ON_VOLTAGE * VIN_RES_INV)  // in 0.125V Resolution
#define RELAY_ON_THD_ADC_RMS    (unsigned int)(((unsigned long)RELAY_ON_THD<<DEF_AC_VDIV)/(unsigned int)DEF_AC_VGAIN_LL)
#define RELAY_ON_THD_ADC_PK     (unsigned int)((float)RELAY_ON_THD_ADC_RMS * 1.41421356)

#define RELAY_OFF_VOLTAGE       5        // in RMS 
#define RELAY_OFF_THD           (RELAY_OFF_VOLTAGE * VIN_RES_INV)  // in 0.125V Resolution
#define RELAY_OFF_THD_ADC_RMS   (unsigned int)(((unsigned long)RELAY_OFF_THD<<DEF_AC_VDIV)/(unsigned int)DEF_AC_VGAIN_LL)
#define RELAY_OFF_THD_ADC_PK    (unsigned int)((float)RELAY_OFF_THD_ADC_RMS * 1.41421356)

#define	RELAY_START_DELAY       250//      // 150 = 150 ms // 1ms = 150mS
#define	RELAY_ON_DELAY          100//10000	  // * 1ms = 10 Sec

//===================== VBUS Configurations / Settings==========================
#define VIN_VBUS_DIFF          Q12(30.0 / VinMax)
//#define VIN_VBUS_DIFF           Q12(5.0 / VinMax)
//PFC Delays
#define PFC_ON_DELAY            30//100   // * 1ms
#define PFC_OFF_DELAY           5     // * 1ms
#define PFC_RESTART_DELAY       1000  // * 1ms

//===================== PFC control states =====================================
#define PFC_ALWAYS_OFF          0
#define PFC_INIT                0
#define PFC_ON                  1
#define PFC_OFF                 2   

//Vin Range Detection
#define VIN_LOW_LINE_TRIG_VOL   1//166         // <166V in RMS Volts
#define VIN_LOW_LINE_THD        (VIN_LOW_LINE_TRIG_VOL * VIN_RES_INV)  // in Converted RMS Volts
#define VIN_LOW_LINE_ADC_RMS    (unsigned int)(((unsigned long)VIN_LOW_LINE_THD<<DEF_AC_VDIV)/(unsigned int)DEF_AC_VGAIN_LL)
#define VIN_LOW_LINE_ADC_PK     (unsigned int)((float)VIN_LOW_LINE_ADC_RMS * 1.41421356)

#define VIN_LOW_LINE_TRIG_VOL1  1//145         // <145V in RMS Volts
#define VIN_LOW_LINE_THD1       (VIN_LOW_LINE_TRIG_VOL1 * VIN_RES_INV)  // in Converted RMS Volts
#define VIN_LOW_LINE_ADC_RMS1   (unsigned int)(((unsigned long)VIN_LOW_LINE_THD1<<DEF_AC_VDIV)/(unsigned int)DEF_AC_VGAIN_LL)
#define VIN_LOW_LINE_ADC_PK1    (unsigned int)((float)VIN_LOW_LINE_ADC_RMS1 * 1.41421356)

#define VIN_LOW_LINE_TRIG_VOL2  1//125         // <125V in RMS Volts
#define VIN_LOW_LINE_THD2       (VIN_LOW_LINE_TRIG_VOL2 * VIN_RES_INV)  // in Converted RMS Volts
#define VIN_LOW_LINE_ADC_RMS2   (unsigned int)(((unsigned long)VIN_LOW_LINE_THD2<<DEF_AC_VDIV)/(unsigned int)DEF_AC_VGAIN_LL)
#define VIN_LOW_LINE_ADC_PK2    (unsigned int)((float)VIN_LOW_LINE_ADC_RMS2 * 1.41421356)

#define VIN_HIGH_LINE_TRIG_VOL  1//170         // >170V in RMS Volts
#define VIN_HIGH_LINE_THD       (VIN_HIGH_LINE_TRIG_VOL * VIN_RES_INV)  // in Converted RMS Volts
#define VIN_HIGH_LINE_ADC_RMS   (unsigned int)(((unsigned long)VIN_HIGH_LINE_THD<<DEF_AC_VDIV)/(unsigned int)DEF_AC_VGAIN_LL)
#define VIN_HIGH_LINE_ADC_PK    (unsigned int)((float)VIN_HIGH_LINE_ADC_RMS * 1.41421356)

#define VIN_DEBUG_REF           1         
#define VIN_DEBUG_REF_THD       (VIN_DEBUG_REF * VIN_RES_INV)  // in Converted RMS Volts
#define VIN_DEBUG_REF_ADC_RMS   (unsigned int)(((unsigned long)VIN_DEBUG_REF_THD<<DEF_AC_VDIV)/(unsigned int)DEF_AC_VGAIN_LL)
#define VIN_DEBUG_REF_ADC_PK    (unsigned int)((float)VIN_DEBUG_REF_ADC_RMS * 1.41421356)

//Bulk Change Over Power Thresholds
//Range 0 
#define PIN_BULK0_THR_H_W       190         // in Watts
#define PIN_BULK0_THR_H         (PIN_BULK0_THR_H_W * PIN_RES_INV)  // in 250mW Resolution

//Range 1
#define PIN_BULK1_THR_L_W       228         // in Watts
#define PIN_BULK1_THR_L         (PIN_BULK1_THR_L_W * PIN_RES_INV)  // in 250mW Resolution
#define PIN_BULK1_THR_H_W       76//760         // in Watts
#define PIN_BULK1_THR_H         (PIN_BULK1_THR_H_W * PIN_RES_INV)  // in 250mW Resolution

//Range 2 220 - 950W
#define PIN_BULK2_THR_L_W       798         // in Watts
#define PIN_BULK2_THR_L         (PIN_BULK2_THR_L_W * PIN_RES_INV)  // in 250mW Resolution
#define PIN_BULK2_THR_H_W       114//1140         // in Watts
#define PIN_BULK2_THR_H         (PIN_BULK2_THR_H_W * PIN_RES_INV)  // in 250mW Resolution

//Range 3 1000 - 1200
#define PIN_BULK3_THR_L_W       1178        // in Watts
#define PIN_BULK3_THR_L         (PIN_BULK3_THR_L_W * PIN_RES_INV)  // in 250mW Resolution
#define PIN_BULK3_THR_H_W       152//1520        // in Watts
#define PIN_BULK3_THR_H         (PIN_BULK3_THR_H_W * PIN_RES_INV)  // in 250mW Resolution

//Range 4 >1250
#define PIN_BULK4_THR_L_W       1558        // in Watts
#define PIN_BULK4_THR_L         (PIN_BULK4_THR_L_W * PIN_RES_INV)  // in 250mW Resolution

//Feed Forward Boost Power Range - By 1.15%
#define FF_BOOST_PWR_RANGE_W    	210         // in Watts
#define FF_BOOST_PWR_RANGE      (FF_BOOST_PWR_RANGE_W * PIN_RES_INV)  // in 250mW Resolution
#define FF_BOOST_PWR_REC_RANGE_W    210         // in Watts
#define FF_BOOST_PWR_REC_RANGE      (FF_BOOST_PWR_REC_RANGE_W * PIN_RES_INV)  // in 250mW Resolution

//Power Limit Configurations
#define VIN_AVG_MIN                Q12(VIN_OK_TRIG_VOLTAGE / VinMax)//Q12(90.0 / VinMax)  // average value of minimum input voltage
#define POWER_LIMIT_FACTOR_TEMP     (int32_t)VIN_AVG_MIN * VIN_AVG_MIN
extern unsigned char Av;

/******************************************************************************
                             Macro Declaration
*****************************************************************************/

/******************************************************************************
                            Subroutine Declaration
*****************************************************************************/
void Validate_Cal_CRC(void);
unsigned int GetAppCodeCRC(void);
void Get_Crc16( unsigned int *crc, unsigned char data );
void __attribute__((__section__("APP_IMAGE"))) Task_200uS();
/*****************************************************************************/