/*******************************************************************************
* Filename   : Init_dsPIC.c
 * Project    : 
 * Created on : 
 * Author(s)  : Microchip reference SG
 * 
 * Date of last Modification:
 * by $Author$:
 * 
 * Description: 
 *
 * Compiler info : MPLABX XC16 Version 1.5x
 * Processor info: 
 *
 ******************************************************************************/

/******************************************************************************
                                Include Files
*******************************************************************************/
#include "Include_headers.h"

/*******************************************************************************
 * Function:        Init_Data
 * Description:     Initialize the secondary data informations.
 ******************************************************************************/
void __attribute__((__section__("APP_IMAGE"))) Init_Data(void)
{      
    unsigned int Temp_Addr;

	
    //Init Status registers
    PRI_DATA.STATUS1 = 0; 
    PRI_DATA.STATUS2 = 0; 
    
    //Init Alarm Registers    
    //AC
    PRI_DATA.A_AC_Dropout = 1;
    PRI_DATA.A_AC_Sag = 1;
    PRI_DATA.A_AC_IUV = 1;
    PRI_DATA.Bulk_High_Sta = 0;  
        
    //Bulk
    PRI_DATA.A_BULK_UV1 = 1;
    PRI_DATA.A_BULK_UV2 = 1;   
    
   
    //Init Startup Internal Parameters/Registers
    Vin_Mon.VPeak_Temp = 0;
    Vin_Mon.IPeak_Temp = 0;
    Vin_Mon.Count = 0;    
    Control.i16[0] = 0;
    Control.i16[1] = 0;
    Control.i16[2] = 0;
    Control.i16[3] = 0;
        
    Pmeter.Vin_Sum_Tmp = 0;
    Pmeter.Iin_Sum_Tmp = 0;
    Pmeter.Pin_Sum_Tmp = 0;    
    Pmeter.VRms_Sum = 0;
    Pmeter.PRms_Sum = 0;
    Pmeter.IRms_Sum = 0;
    Pmeter.Rms_Avg_Count = 0;
    
    //Init Thresholds and Delays
    Dropout_Delay = DROPOUT_TRIG_DELAY_FL;
        
    //Init Digital PFC Settings
    Max_u0_i = MAX_u0_I_HL;    
            
    //Load Calibration Data from Flash    
    Temp_Addr = CALIBRATION_DATA_ADDR;

    PRI_DATA.VIN_LL_Gain = DEF_AC_VGAIN_LL;
    PRI_DATA.VIN_HL_Gain = DEF_AC_VGAIN_HL;
    PRI_DATA.VIN_Div = DEF_AC_VDIV;

    PRI_DATA.IIN_LL_1A_Gain = DEF_AC_1A_IGAIN_LL;
    PRI_DATA.IIN_LL_1A_ADC = DEF_AC_1A_ADC_LL;    
    PRI_DATA.IIN_LL_1A_Val = DEF_AC_1A_CUR_LL;
    PRI_DATA.IIN_LL_4A_Gain = DEF_AC_4A_IGAIN_LL;
    PRI_DATA.IIN_LL_4A_ADC = DEF_AC_4A_ADC_LL;    
    PRI_DATA.IIN_LL_4A_Val = DEF_AC_4A_CUR_LL;
    PRI_DATA.IIN_LL_10A_Gain = DEF_AC_10A_IGAIN_LL;
    PRI_DATA.IIN_LL_10A_ADC = DEF_AC_10A_ADC_LL;    
    PRI_DATA.IIN_LL_10A_Val = DEF_AC_10A_CUR_LL;

    PRI_DATA.IIN_HL_0A5_Gain = DEF_AC_0A5_IGAIN_HL;
    PRI_DATA.IIN_HL_0A5_ADC = DEF_AC_0A5_ADC_HL;    
    PRI_DATA.IIN_HL_0A5_Val = DEF_AC_0A5_CUR_HL; 
    PRI_DATA.IIN_HL_2A_Gain = DEF_AC_2A_IGAIN_HL;
    PRI_DATA.IIN_HL_2A_ADC = DEF_AC_2A_ADC_HL;    
    PRI_DATA.IIN_HL_2A_Val = DEF_AC_2A_CUR_HL; 
    PRI_DATA.IIN_HL_5A_Gain = DEF_AC_5A_IGAIN_HL;
    PRI_DATA.IIN_HL_5A_ADC = DEF_AC_5A_ADC_HL;    
    PRI_DATA.IIN_HL_5A_Val = DEF_AC_5A_CUR_HL; 

    PRI_DATA.IIN_Div = DEF_AC_IDIV;

    PRI_DATA.PIN_LL_1A_Gain = DEF_AC_1A_PGAIN_LL;
    PRI_DATA.PIN_LL_1A_ADC = DEF_AC_1A_PADC_LL;    
    PRI_DATA.PIN_LL_1A_Val = DEF_AC_1A_PWR_LL;
    PRI_DATA.PIN_LL_4A_Gain = DEF_AC_4A_PGAIN_LL;
    PRI_DATA.PIN_LL_4A_ADC = DEF_AC_4A_PADC_LL;    
    PRI_DATA.PIN_LL_4A_Val = DEF_AC_4A_PWR_LL;
    PRI_DATA.PIN_LL_10A_Gain = DEF_AC_10A_PGAIN_LL;
    PRI_DATA.PIN_LL_10A_ADC = DEF_AC_10A_PADC_LL;    
    PRI_DATA.PIN_LL_10A_Val = DEF_AC_10A_PWR_LL;

    PRI_DATA.PIN_HL_0A5_Gain = DEF_AC_0A5_PGAIN_HL;
    PRI_DATA.PIN_HL_0A5_ADC = DEF_AC_0A5_PADC_HL;    
    PRI_DATA.PIN_HL_0A5_Val = DEF_AC_0A5_PWR_HL;
    PRI_DATA.PIN_HL_2A_Gain = DEF_AC_2A_PGAIN_HL;
    PRI_DATA.PIN_HL_2A_ADC = DEF_AC_2A_PADC_HL;    
    PRI_DATA.PIN_HL_2A_Val = DEF_AC_2A_PWR_HL;
    PRI_DATA.PIN_HL_5A_Gain = DEF_AC_5A_PGAIN_HL;
    PRI_DATA.PIN_HL_5A_ADC = DEF_AC_5A_PADC_HL;    
    PRI_DATA.PIN_HL_5A_Val = DEF_AC_5A_PWR_HL;

    PRI_DATA.PIN_Div = DEF_AC_PDIV;

    PRI_DATA.BULK_GAIN =  DEF_BULK_GAIN;
    PRI_DATA.BULK_DIV =  DEF_BULK_SCALE;
    
    PRI_DATA.CAL_LOCK_KEY = CALIB_UNLOCK_KEY;
    PRI_DATA.Cal_Unlock_Sta = 1;
}
/*******************************************************************************
 * Function:        Init_Cmp
 * Description:     
 ******************************************************************************/
void Init_Cmp(void)
{
    //====================PFC current limit initialization====================
    /* PFC A current limit */
 
    CMP2DAC = PFC_OC_CMP_SET;    // Set over current reference
	CMP3DAC = BULK_OVP_ADC;    // Set over voltage reference

    /* PFC B current limit */
    #if PFC_B_ON_BOARD
    CMP1DAC = PFC_OC_CMP_SET;      // Set over current reference
    #endif

}
/*******************************************************************************
 * Function:        Init_dsPIC
 * Description:     Initialize All the Peripherals called from Main 
 ******************************************************************************/
void __attribute__((__section__("APP_IMAGE"))) Env_Init(void)
{  
    Init_Data();
    ControlLoopInit();
    //PTPER  = 0x38B1;
    IIR_param_init();
    PRI_DATA.PFC_GO = 1;    //Enable PFC to Start   
}

void IIR_param_init(void)
{
#if NOTCH_FILETER_S_120Hz 
	//Fsw=65kHz
	//Fc (frequency to attenuate) = 120Hz
	//Attenuation factor = 0.1
	DCfilter.a1[0]=0;//15486;
	DCfilter.a1[1]=1;//-21901;
	DCfilter.a1[2]=0;//15486;
	DCfilter.b1[0]=-32544;//-21901;
	DCfilter.b1[1]=16162;//14588;


	//Fc (frequency to attenuate) = 100Hz
	// DCfilter.a1[0]  = 15629;
	// DCfilter.a1[1]  = -22103;	
	// DCfilter.a1[2]  = 15629;
	// DCfilter.b1[0]  = -22103;
	// DCfilter.b1[1]  = 14873;
	
	
		// DC filter intermediate and output state initialization
	DCfilter.vbus_filt1[0] = 0;
	DCfilter.vbus_filt1[1] = 0;
	DCfilter.vbus_filt1[2] = 0;
	DCfilter.vbus1[0] = 0;
	DCfilter.vbus1[1] = 0;
	DCfilter.vbus1[2] = 0;
	DCfilter.output1 = 0;
	DCfilter.output = 0;
	DCfilter.calc11 = 0;
	DCfilter.calc12 = 0;
#endif 


#if NOTCH_FILETER_S_100Hz 
// DC Filter coefficients IIR section 2 (notch 100Hz), Q15
    
/*
b0 = 16376
b1 = -32751
b2 = 16376
a1 = -32751
a2 = 16368
*/
    
DCfilter.b1[0]	= 16376;
DCfilter.b1[1]	= -32751;	//-31304;
DCfilter.b1[2]	= 16376;	//15279;
DCfilter.a1[1]	= -32751;	//-32396;
DCfilter.a1[2]	= 16368;


DCfilter.vbus_filt1[0] = 0;
DCfilter.vbus_filt1[1] = 0;
DCfilter.vbus_filt1[2] = 0;
DCfilter.vbus1[0] = 0;
DCfilter.vbus1[1] = 0;
DCfilter.vbus1[2] = 0;
DCfilter.output1 = 0;
DCfilter.output = 0;
DCfilter.calc11 = 0;
DCfilter.calc12 = 0;

/*
	//Fsw=65kHz
	//Fc (frequency to attenuate) = 120Hz
	//Attenuation factor = 0.1
	// DCfilter.a1[0]=15486;
	// DCfilter.a1[1]=-21901;
	// DCfilter.a1[2]=15486;
	// DCfilter.b1[0]=-21901;
	// DCfilter.b1[1]=14588;


	//Fc (frequency to attenuate) = 100Hz
	DCfilter.a1[0]  = 15629;
	DCfilter.a1[1]  = -22103;	
	DCfilter.a1[2]  = 15629;
	DCfilter.b1[0]  = -22103;
	DCfilter.b1[1]  = 14873;
	
	
		// DC filter intermediate and output state initialization
	DCfilter.vbus_filt1[0] = 0;
	DCfilter.vbus_filt1[1] = 0;
	DCfilter.vbus_filt1[2] = 0;
	DCfilter.vbus1[0] = 0;
	DCfilter.vbus1[1] = 0;
	DCfilter.vbus1[2] = 0;
	DCfilter.output1 = 0;
	DCfilter.output = 0;
	DCfilter.calc11 = 0;
	DCfilter.calc12 = 0;
	*/
#endif

#if CASCADE_NOTCH_FILETER_INFENION
	// DC Filter coefficients IIR section 1 (notch 100Hz), Q14
	// 5kHz / 10kHz (comment)
	DCfilter.k1  	= 15368;	//15831;
	DCfilter.a1[0]  = 16384; //1-Q14
	DCfilter.a1[1]  = -31738;	//-32509;
	DCfilter.a1[2]  = 16384;
	DCfilter.b1[0]  = 16384;
	DCfilter.b1[1]  = -29771;	//-31413;
	DCfilter.b1[2]  = 14352;	//15279;

	// DC Filter coefficients IIR section 2 (notch 120Hz), Q14
	DCfilter.k2  	= 15368;	//15831;
	DCfilter.a2[0]  = 16384;
	DCfilter.a2[1]  = -31289;	//-32396;
	DCfilter.a2[2]  = 16384;
	DCfilter.b2[0]  = 16384;
	DCfilter.b2[1]  = -29349;	//-31304;
	DCfilter.b2[2]  = 14352;	//15279;

	// DC filter intermediate and output state initialization
	DCfilter.u1[0] = 0;
	DCfilter.u1[1] = 0;
	DCfilter.u1[2] = 0;
	DCfilter.u2[0] = 0;
	DCfilter.u2[1] = 0;
	DCfilter.u2[2] = 0;
	DCfilter.output1 = 0;
	DCfilter.output = 0;
	DCfilter.calc11 = 0;
	DCfilter.calc12 = 0;
	DCfilter.calc21 = 0;
	DCfilter.calc22 = 0;    
#endif

#if CASCADE_NOTCH_FILETER_S
    //Fsw=65kHz
	//Fc (frequency to attenuate) = 120Hz
	//Attenuation factor = 0.1
	DCfilter.a1[0] = 15486;
	DCfilter.a1[1] = -21901;
	DCfilter.a1[2] = 15486;
	DCfilter.b1[0] = -21901;
	DCfilter.b1[1] = 14588;

	// DC Filter coefficients IIR section 2 (notch 100Hz), Q14
	DCfilter.a2[0]  = 15629;
	DCfilter.a2[1]  = -21901;	
	DCfilter.a2[2]  = 15629;
	DCfilter.b2[0]  = -21901;
	DCfilter.b2[1]  = 14873;

	
		// DC filter intermediate and output state initialization
	DCfilter.vbus_filt1[0] = 0;
	DCfilter.vbus_filt1[1] = 0;
	DCfilter.vbus_filt1[2] = 0;
	DCfilter.vbus1[0] = 0;
	DCfilter.vbus1[1] = 0;
	DCfilter.vbus1[2] = 0;
    DCfilter.vbus_filt2[0] = 0;
	DCfilter.vbus_filt2[1] = 0;
	DCfilter.vbus_filt2[2] = 0;
	DCfilter.vbus2[0] = 0;
	DCfilter.vbus2[1] = 0;
	DCfilter.vbus2[2] = 0;
	DCfilter.output1 = 0;
	DCfilter.output = 0;
	DCfilter.calc11 = 0;
	DCfilter.calc12 = 0;
	DCfilter.calc21 = 0;
	DCfilter.calc22 = 0;
#endif    
}

/*---------------------------------------------------------------------------*/
/**
 * \brief   IIR coefficients initialization.<BR>
 * 			IIR cascaded low-pass filter of the AC voltage to obtain the RMS value.<BR>
 * 			IIR notch filter of the DC voltage to remove 100/120Hz ripple.<BR>
 * 			Sampling frequency of both filters is 2500Hz, every 2 cycles of CCU40.CC41 @ 5kHz.
 * \return  None
 */

/*****************************************************************************/
/*
 * End of file
 */




