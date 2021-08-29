
/******************************************************************************
                                Include Files
*******************************************************************************/
#include "Include_headers.h"
#define DEBUG_DATA  FALSE

/******************************************************************************
                             Variable Declaration
*****************************************************************************/
volatile uint16_t *PFC_AB_SHIFT_Ptr1;
volatile uint16_t *PFC_AB_SHIFT_Ptr2;
volatile uint16_t *PFC_PWM_PERIOD_Ptr;
volatile uint16_t *PFC_DUTY_Ptr[PFC_CHANNEL];
volatile uint16_t *PFC_ADC_TRIG_Ptr[PFC_CHANNEL];
volatile uint16_t *ADCBUF_CT_Ptr[PFC_CHANNEL];

uint16_t Vin_filt = 0;
uint16_t VbusRefTemp = 0;
uint16_t Soft_Start_Iteration = 5; // 1 = ~ 5ms 
uint16_t Vbus_filt = 0;

//IIN Variables
int16_t Iref = 0;
int16_t k1_i = K1_I;
int16_t k2_i = K2_I;

//Feed forward Control
int16_t FForwardFactor = 0;
int32_t FForward = 0;
int16_t Duty_LoadBal = 0;
/** Bulk-Notch Filter instance */
volatile IIR_lowpass_int_t DCfilter;
void LoadBalance(void);

static inline void Notch_filter2_Vbulk()
{
/*
b0 = 16376   Q15(b0/2)
b1 = -32751  Q15(b1/2) 
b2 = 16376   Q15(b2/2)
a1 = -32751  Q15(a1/2)
a2 = 16368   Q15(a2/2)

	*/
	/*--------------------------------------------------------------------------*
	 *  DC Voltage filter --> cascaded notch - START							*
	 *--------------------------------------------------------------------------*/

//vbus_filter[n] = b0*vbus[n] + b1*vbus[n-1] + b2*vbus[n-2] - a1*vbus_filter[n-1] - a2*vbus_filter[n-2]	
	//section1 
	DCfilter.vbus1[0] = Vin_filt;
    DCfilter.tmpb0u0 = __builtin_mulss(DCfilter.b1[0],DCfilter.vbus1[0]);
    DCfilter.tmpb1u1 = __builtin_mulss(DCfilter.b1[1],DCfilter.vbus1[1]);
    DCfilter.tmpb2u2 = __builtin_mulss(DCfilter.b1[2],DCfilter.vbus1[2]);
    
	DCfilter.output1 = DCfilter.tmpb0u0 + DCfilter.tmpb1u1 + DCfilter.tmpb2u2;
	
	// Section 1 calculation
	DCfilter.tmpa1y1 = __builtin_mulss(DCfilter.a1[1],DCfilter.vbus_filt1[1]);
   	DCfilter.tmpa2y2 = __builtin_mulss(DCfilter.a1[2],DCfilter.vbus_filt1[2]);
    
	DCfilter.vbus_filt1[0] = (DCfilter.output1 - DCfilter.tmpa1y1 - DCfilter.tmpa2y2) >> IIR_QFORMAT;
	
	//output1 = (a1[0]*u1[0] + a1[1]*u1[1] + a1[2]*u1[2]) >> IIR_QFORMAT; //Q15 output is the input of the nevbust section

	// Save section 1 values for nevbust iteration
	DCfilter.vbus_filt1[2] = DCfilter.vbus_filt1[1];
	DCfilter.vbus_filt1[1] = DCfilter.vbus_filt1[0];

	DCfilter.vbus1[2] = DCfilter.vbus1[1];
	DCfilter.vbus1[1] = DCfilter.vbus1[0];

	// DC value for control loop
	//Vbus_filt = DCfilter.vbus_filt1[0];
	Vin_filt = DCfilter.vbus_filt1[0];

	/*--------------------------------------------------------------------------*
	 *  DC Voltage filter --> cascaded notch - END								*
	 *--------------------------------------------------------------------------*/
}
static inline void Notch_filter_Vbulk()
{
	/*--------------------------------------------------------------------------*
	 *  DC Voltage filter --> cascaded notch - START							*
	 *--------------------------------------------------------------------------*/

//vbus_filter[n] = a0*vbus[n] + a1*vbus[n-1] + a2*vbus[n-2] ? b1*vbus_filter[n-1] ? b2*vbus_filter[n-2]

//Fsw=65kHz
//Fc (frequency to eliminate) = 120Hz
//Attenuation factor = 0.1
// a1[0]=15486;
// a1[1]=-30970;
// a1[2]=15486;
// b1[0]=-30970;
// b1[1]=14588;



//Fsw=65kHz
//Fc (frequency to eliminate) = 100Hz
//Attenuation factor = 0.1
// a1[0]=15629;
// a1[1]=-31256;
// a1[2]=15629;
// b1[0]=-31256;
// b1[1]=14873;

//vbus_filter[n] = a0*vbus[n] + a1*vbus[n-1] + a2*vbus[n-2] - b0*vbus_filter[n-1] - b1*vbus_filter[n-2]	
	//section1 
	//DCfilter.vbus1[0] = Vbus_filt;
	DCfilter.vbus1[0] = Vin_filt;
	DCfilter.output1 = (DCfilter.a1[0]*DCfilter.vbus1[0] + DCfilter.a1[1]*DCfilter.vbus1[1] + DCfilter.a1[2]*DCfilter.vbus1[2]) >> IIR_QFORMAT;
	
	// Section 1 calculation
	DCfilter.calc11 = ((int64_t)DCfilter.vbus_filt1[1]*(int64_t)DCfilter.b1[0]) >> IIR_QFORMAT;
	DCfilter.calc21 = ((int64_t)DCfilter.vbus_filt1[2]*(int64_t)DCfilter.b1[1]) >> IIR_QFORMAT;

	DCfilter.vbus_filt1[0] = DCfilter.output1 - DCfilter.calc11 - DCfilter.calc21; //Intermediate state
	
	//output1 = (a1[0]*u1[0] + a1[1]*u1[1] + a1[2]*u1[2]) >> IIR_QFORMAT; //Q15 output is the input of the nevbust section

	// Save section 1 values for nevbust iteration
	DCfilter.vbus_filt1[2] = DCfilter.vbus_filt1[1];
	DCfilter.vbus_filt1[1] = DCfilter.vbus_filt1[0];

	DCfilter.vbus1[2] = DCfilter.vbus1[1];
	DCfilter.vbus1[1] = DCfilter.vbus1[0];
	
	// Limit to avoid negative numbers
	if(DCfilter.vbus_filt1[0] < 1)
		DCfilter.vbus_filt1[0] = 1;                                                                         

	// DC value for control loop
	//Vbus_filt = DCfilter.vbus_filt1[0];
	Vin_filt = DCfilter.vbus_filt1[0];

	/*--------------------------------------------------------------------------*
	 *  DC Voltage filter --> cascaded notch - END								*
	 *--------------------------------------------------------------------------*/
}
static inline void Cascade_notch_filter_Vbulk()
{
	/*--------------------------------------------------------------------------*
	 *  DC Voltage filter --> cascaded notch - START							*
	 *--------------------------------------------------------------------------*/

//vbus_filter[n] = a0*vbus[n] + a1*vbus[n-1] + a2*vbus[n-2] ? b1*vbus_filter[n-1] ? b2*vbus_filter[n-2]

//Fsw=65kHz
//Fc (frequency to eliminate) = 120Hz
//Attenuation factor = 0.1
// a1[0]=15486;
// a1[1]=-30970;
// a1[2]=15486;
// b1[0]=-30970;
// b1[1]=14588;



//Fsw=65kHz
//Fc (frequency to eliminate) = 100Hz
//Attenuation factor = 0.1
// a1[0]=15629;
// a1[1]=-31256;
// a1[2]=15629;
// b1[0]=-31256;
// b1[1]=14873;

//vbus_filter[n] = a0*vbus[n] + a1*vbus[n-1] + a2*vbus[n-2] - b0*vbus_filter[n-1] - b1*vbus_filter[n-2]	
	//section1 
	DCfilter.vbus1[0] = Vbus_filt;
	DCfilter.output1 = (DCfilter.a1[0]*DCfilter.vbus1[0] + DCfilter.a1[1]*DCfilter.vbus1[1] + DCfilter.a1[2]*DCfilter.vbus1[2]) >> IIR_QFORMAT;
	
	// Section 1 calculation
	DCfilter.calc11 = ((int64_t)DCfilter.vbus_filt1[1]*(int64_t)DCfilter.b1[0]) >> IIR_QFORMAT;
	DCfilter.calc21 = ((int64_t)DCfilter.vbus_filt1[2]*(int64_t)DCfilter.b1[1]) >> IIR_QFORMAT;

	DCfilter.vbus_filt1[0] = DCfilter.output1 - DCfilter.calc11 - DCfilter.calc21; //Intermediate state
	
	//output1 = (a1[0]*u1[0] + a1[1]*u1[1] + a1[2]*u1[2]) >> IIR_QFORMAT; //Q15 output is the input of the nevbust section

	// Save section 1 values for nevbust iteration
	DCfilter.vbus_filt1[2] = DCfilter.vbus_filt1[1];
	DCfilter.vbus_filt1[1] = DCfilter.vbus_filt1[0];

	DCfilter.vbus1[2] = DCfilter.vbus1[1];
	DCfilter.vbus1[1] = DCfilter.vbus1[0];
    //section2
    DCfilter.vbus2[0] = DCfilter.vbus_filt1[0];
	DCfilter.output1 = (DCfilter.a1[0]*DCfilter.vbus2[0] + DCfilter.a1[1]*DCfilter.vbus2[1] + DCfilter.a1[2]*DCfilter.vbus2[2]) >> IIR_QFORMAT;
	

	DCfilter.calc12 = ((int64_t)DCfilter.vbus_filt2[1]*(int64_t)DCfilter.b1[0]) >> IIR_QFORMAT;
	DCfilter.calc22 = ((int64_t)DCfilter.vbus_filt2[2]*(int64_t)DCfilter.b1[1]) >> IIR_QFORMAT;

	DCfilter.vbus_filt2[0] = DCfilter.output1 - DCfilter.calc12 - DCfilter.calc22; //Intermediate state
		
	DCfilter.vbus_filt2[2] = DCfilter.vbus_filt2[1];
	DCfilter.vbus_filt2[1] = DCfilter.vbus_filt2[0];

	DCfilter.vbus2[2] = DCfilter.vbus2[1];
	DCfilter.vbus2[1] = DCfilter.vbus2[0];
	
	// Limit to avoid negative numbers
	if(DCfilter.vbus_filt2[0] < 1)
		DCfilter.vbus_filt2[0] = 1;                                                                         

	// DC value for control loop
	Vbus_filt = DCfilter.vbus_filt2[0];

	/*--------------------------------------------------------------------------*
	 *  DC Voltage filter --> cascaded notch - END								*
	 *--------------------------------------------------------------------------*/
}


/*******************************************************************************
 * Function:        ControlLoopInit
 * Description:     Control loop init
 * Calling:
 ******************************************************************************/
void __attribute__((__section__("APP_IMAGE"))) ControlLoopInit(void)
{
    PFC_PWM_PERIOD_Ptr = &PFC_PWMPERIOD;
    PFC_DUTY_Ptr[PFC_A] = &PFC_PWMDUTY1;      // PDC1
    PFC_ADC_TRIG_Ptr[PFC_A] = &ADCTRIG1;      // TRIG1
    ADCBUF_CT_Ptr[PFC_A] = &CT_A_CURRENT_BUF; // ADCBUF3
    PFC_AB_SHIFT_Ptr1 = &PFC_PHASE_SHIFT1;
	PFC_AB_SHIFT_Ptr2 = &PFC_PHASE_SHIFT2;
  //#if PFC_B_ON_BOARD  
    PFC_DUTY_Ptr[PFC_B] = &PFC_PWMDUTY2;      // PDC2
    PFC_ADC_TRIG_Ptr[PFC_B] = &ADCTRIG2;      // TRIG2
    ADCBUF_CT_Ptr[PFC_B] = &CT_B_CURRENT_BUF; // ADCBUF0
  //#endif

    PfcPeriod = PFC_PWM_PERIOD1;
    PfcPeriodBase = PFC_PWM_PERIOD1;
	PTPER=PFC_PWM_PERIOD1;
	AC_NOK_PRI_SetHigh();
	PDC1=0;
	PDC2=0;
	PHASE1=PFC_PWM_PERIOD1/2;
	PHASE3=0xB838;
	RELAY_OFF();
	// PDC3=0x4000;
	
}

/*******************************************************************************
 * Function:        Control Loop Called from ADC3 Interrupt
 * Parameters:
 * Returned value:
 * Description:     Control loop
 * Calling:
 ******************************************************************************/
void __attribute__((__section__("APP_ISR_IMAGE"))) Control_Loop(void)
{	
    static volatile int32_t u0_v = 0;
    static volatile int32_t u1_v = 0;
    static volatile int16_t e0_v = 0;
    static volatile int16_t e1_v = 0;
    static volatile int16_t vloop_out = 0;

    static volatile int32_t u0_i[PFC_CHANNEL];
    static volatile int32_t u1_i[PFC_CHANNEL];
    static volatile int16_t e0_i[PFC_CHANNEL];
    static volatile int16_t e1_i[PFC_CHANNEL];
    static volatile int16_t Iin_CT[PFC_CHANNEL];
    static volatile int32_t u0_K_i[PFC_CHANNEL];
    static volatile int16_t PfcDuty[PFC_CHANNEL];

    static uint16_t ClrKey = 0;
    static uint8_t K_dyn = 0;
    static uint16_t DynCnt = 0;
    static int16_t FForward_ccm = 0;
    static uint16_t SoftStartCnt = 0;

    static int16_t Remaindertemp;
    static int32_t wCalTemp;
    static uint16_t uiBusVinDifference;
    static int16_t VavgInvSquareTemp = Q12(1.0);
    static uint32_t ulDummy;
    static uint16_t k1_v = K1_V;
    static uint16_t k2_v = K2_V;
    k1_i = K1_I;
    k2_i = K2_I;

    if((*PFC_PWM_PERIOD_Ptr) != PfcPeriod)
    {
        *PFC_PWM_PERIOD_Ptr = PfcPeriod;
        *PFC_AB_SHIFT_Ptr1 = (*PFC_PWM_PERIOD_Ptr) >> 1;
		//*PFC_AB_SHIFT_Ptr2 = (*PFC_PWM_PERIOD_Ptr) >> 1;
    }
 
    if (L_VOLTAGE_BUF > N_VOLTAGE_BUF)
        Vin = (L_VOLTAGE_BUF - N_VOLTAGE_BUF);
    else
        Vin = (N_VOLTAGE_BUF - L_VOLTAGE_BUF);

    //Vbus = (__builtin_divud(__builtin_muluu(BUS_VOLTAGE_BUF, 4096), 4096));
    Vbus = BUS_VOLTAGE_BUF;
	//PRI_DATA.PFC_En=1;
    if(PRI_DATA.PFC_En) // PFC is enabled by main control loop //PRI_DATA.PFC_En 
    {    
        // *********************Input and Output Sense Filtering********************         
        Vin_filt = (__builtin_mulss(Vin_filt, K1_VIN) >> 15) + (__builtin_mulss(Vin, K2_VIN) >> 15);
		Vbus_filt = (__builtin_mulss(Vbus_filt, K1_VBUS) >> 15) + (__builtin_mulss(Vbus, K2_VBUS) >> 15);
        #if NOTCH_FILETER_S_120Hz		
		//Notch_filter_Vbulk();
        #endif
		
		#if NOTCH_FILETER_S_100Hz		
		Notch_filter2_Vbulk();
        #endif
		
        #if CASCADE_NOTCH_FILETER_INFENION
       // Infenion_cascade_notch_filter_Vbulk();
        #endif
        #if CASCADE_NOTCH_FILETER_S
        //Cascade_notch_filter_Vbulk();
        #endif
        if(((int) Vbus_filt - (int) Vin_filt) > (int) VBUS_GAP)
            uiBusVinDifference = Vbus_filt - Vin_filt;
        else
            uiBusVinDifference = VBUS_GAP;

        
      
        // *********************Input and Output Sense Filtering END********************        
        // ************************************ current B loop *****************************
      #if PFC_B_ON_BOARD  
        wCalTemp = __builtin_divmodsd((__builtin_mulss(PfcDuty[PFC_B], Vbus_filt)), uiBusVinDifference, &Remaindertemp);
        if(wCalTemp < 4096)
            Iin_CT[PFC_B] = (__builtin_mulss(((*ADCBUF_CT_Ptr[PFC_B])), wCalTemp)) >> 12;
        else
            Iin_CT[PFC_B] = *ADCBUF_CT_Ptr[PFC_B];

        e0_i[PFC_B] = Iref - Iin_CT[PFC_B];

        if(e0_i[PFC_B] > I_ERROR_MAX)
            e0_i[PFC_B] = I_ERROR_MAX;

        u0_i[PFC_B] = u1_i[PFC_B] + (long) __builtin_mulss(e0_i[PFC_B], k1_i) + (long) __builtin_mulss(e1_i[PFC_B], k2_i);
        u0_K_i[PFC_B] = u0_i[PFC_B] + FForward;

        if(u0_K_i[PFC_B] > Max_u0_i)
        {
            u0_K_i[PFC_B] = Max_u0_i;
            u0_i[PFC_B] = Max_u0_i - FForward;
        }
        else if(u0_K_i[PFC_B] < MIN_u0_I)
        {
            u0_i[PFC_B] = MIN_u0_I - FForward;
            e0_i[PFC_B] = 0;
            u0_K_i[PFC_B] = 0;
        }

        u1_i[PFC_B] = u0_i[PFC_B];
        e1_i[PFC_B] = e0_i[PFC_B];

        PfcDuty[PFC_B] = (int) (u0_K_i[PFC_B] >> 12);
        PfcDuty[PFC_B] = PfcDuty[PFC_B] - Duty_LoadBal;
        #if PFC_B_ALWAYS_OFF 
            *PFC_DUTY_Ptr[PFC_B] = 0;
        #else
            *PFC_DUTY_Ptr[PFC_B] = ((__builtin_mulss(PfcDuty[PFC_B], (*PFC_PWM_PERIOD_Ptr))) >> 12);
			if(Vbus_filt > VBUS_DEBUG_REF_ADC)
				{
				 *PFC_DUTY_Ptr[PFC_B] = 0;
				 } 
        #endif
        *PFC_ADC_TRIG_Ptr[PFC_B] = ((*PFC_DUTY_Ptr[PFC_B]) >> 1);        
      #else
        *PFC_DUTY_Ptr[PFC_B] = 0;
        *PFC_ADC_TRIG_Ptr[PFC_B] = 0;        
      #endif
        // ************************************current B loop END************************
  
////            ************************************Soft Start *******************************
        if(PRI_DATA.CL_1st_PASS) // Check for First Iteration of the loop
        {
            VbusRefTemp = Vbus; // Assign Ref Voltage to DCBus Voltage

            Vbus_filt = Vbus;
            PRI_DATA.CL_1st_PASS = 0; // Clear the First Pass Flag
        }
        else
        {
            if(PRI_DATA.CL_SoftStart) // Check for SoftStart Flag Status
            {
                if (SoftStartCnt > Soft_Start_Iteration)
                {
                    if (VbusRefTemp < VbusRef)
                        VbusRefTemp++;
                    else if (VbusRefTemp > VbusRef)
                        VbusRefTemp--;
                    else
                        PRI_DATA.CL_SoftStart = 0;
                    SoftStartCnt = 0;
                }
                else
                    SoftStartCnt++;
            }
        }
        // ************************************Soft Start END****************************
        
        // *********************************** Voltage Loop ******************************
       e0_v = VbusRefTemp - Vbus_filt;

        
        // *********************************** Voltage Loop ******************************
       //  e0_v = VBUS_REF0_ACT_ADC_HL - Vbus_filt;
 
        if((e0_v > V_ERROR_MAX) || (e0_v < V_ERROR_MIN))
        {
            if(PRI_DATA.A_ALERT_PFC)
            {
                if(K_dyn < 4)
                    K_dyn++;
            }
            else
            {
                if(K_dyn < 4)
                    K_dyn++;
            }

            k1_v = K1_V + 5;
            k2_v = K2_V;

            DynCnt = 0;
            if(e0_v > V_ERROR_MAX)
                e0_v = V_ERROR_MAX;
            else if (e0_v < V_ERROR_MIN)
                e0_v = V_ERROR_MIN;
        }
        else
        {
            k1_v = K1_V;
            k2_v = K2_V;
        }

        if(K_dyn > 0)
        {
            e0_v *= K_dyn;
            if(DynCnt > 5)
            {
                DynCnt = 0;
                K_dyn--;
            }
            else
                DynCnt++;
        }

        u0_v = u1_v + (long) __builtin_mulsu(e0_v, k1_v) - (long) __builtin_mulsu(e1_v, k2_v);
	
					
        if(u0_v > MAX_u0_V)
            u0_v = MAX_u0_V;
        else if(u0_v < MIN_u0_V)
        {	
            u0_v = MIN_u0_V;
            e0_v = 0;
        }

        if(PRI_DATA.A_BULK_OV_ALERT)
        {
           u0_v = 0;
           e0_v = 0;

        }

        u1_v = u0_v;
        e1_v = e0_v;

        vloop_out = (int) (u0_v >> 15);
        // *********************************** Voltage Loop END *************************
        
        // *********  Clear voltage loop output at light load ***************************
       
        if (vloop_out < VLOOP_LOWLOAD_L)
            ClrKey = 0x55;
        else if (vloop_out > VLOOP_LOWLOAD_H)
            ClrKey = 0;

        if(ClrKey == 0x55)
        {
            vloop_out = 0;
            FForwardFactor = 0;
            e1_i[PFC_A] = 0; 
            u1_i[PFC_A] = 0;

            #if PFC_B_ON_BOARD
                e1_i[PFC_B] = 0;
                u1_i[PFC_B] = 0;
            #endif
        }
        
        // *********  Clear voltage loop output at light load END ***********************
       
        // ************************************current A loop ***************************
        wCalTemp = __builtin_divmodsd((__builtin_mulss(PfcDuty[PFC_A], Vbus_filt)), uiBusVinDifference, &Remaindertemp);
        if(wCalTemp < 4096)
            Iin_CT[PFC_A] = (__builtin_mulss(((*ADCBUF_CT_Ptr[PFC_A])), wCalTemp)) >> 12;
        else
            Iin_CT[PFC_A] = *ADCBUF_CT_Ptr[PFC_A];

        e0_i[PFC_A] = Iref - Iin_CT[PFC_A];
		
        if (e0_i[PFC_A] > I_ERROR_MAX)
            e0_i[PFC_A] = I_ERROR_MAX;
		

		
        u0_i[PFC_A] = u1_i[PFC_A] + (long) __builtin_mulss(e0_i[PFC_A], k1_i) + (long) __builtin_mulss(e1_i[PFC_A], k2_i);

        u0_K_i[PFC_A] = u0_i[PFC_A] + FForward;
		
        if(u0_K_i[PFC_A] > Max_u0_i)                                                               
        {
            u0_K_i[PFC_A] = Max_u0_i;
            u0_i[PFC_A] = Max_u0_i - FForward;
        }
        else if(u0_K_i[PFC_A] < MIN_u0_I)
        {	
            u0_i[PFC_A] = MIN_u0_I - FForward;
            e0_i[PFC_A] = 0;
            u0_K_i[PFC_A] = 0;
        }

		
        u1_i[PFC_A] = u0_i[PFC_A];
        e1_i[PFC_A] = e0_i[PFC_A];

        PfcDuty[PFC_A] = (int) (u0_K_i[PFC_A] >> 12);
        PfcDuty[PFC_A] = PfcDuty[PFC_A] + Duty_LoadBal;
         #if PFC_A_ALWAYS_OFF
            *PFC_DUTY_Ptr[PFC_A] = 0;
        #else 

            *PFC_DUTY_Ptr[PFC_A] = ((__builtin_mulss(PfcDuty[PFC_A], 0x38B1)) >> 12);


		
       #endif
           
		if(Vbus_filt > VBUS_DEBUG_REF_ADC)
		{
		 *PFC_DUTY_Ptr[PFC_A] = 0;
		 } 
        *PFC_ADC_TRIG_Ptr[PFC_A] = ((*PFC_DUTY_Ptr[PFC_A]) >> 1);   

        // ************************************current A loop END************************

        //**************************** Current loop Reference ***************************
        
        if(VavgInvSquareTemp < Vavg_Inv_Sq)
            VavgInvSquareTemp += 5;
        else if(VavgInvSquareTemp > Vavg_Inv_Sq)
            VavgInvSquareTemp--;

        wCalTemp = (int) ((__builtin_mulss(vloop_out, Vin_filt)) >> 12);
        wCalTemp = (int) ((__builtin_mulss(wCalTemp, VavgInvSquareTemp)) >> 12);
        

		
        Iref = (int) ((__builtin_mulss(wCalTemp, KM)) >> 12);
       
        if(Iref > MAX_IREF)
            Iref = MAX_IREF;

        //**************************** Current loop Reference END ***********************

        /* ***************************** Duty feed forward ******************************************************************************* */
		
        /* ***************************** Duty feed forward ******************************************************************************* */
        #if FEEDFORWARD_ENABLE

          #if DCM_FF_CAL_ENABLE    
            if(vloop_out > 10)
            {
                if(FForwardFactor < Q12(1.0))
                    FForwardFactor = FForwardFactor + Q12(0.01);

                if(FForwardFactor > Q12(1.0))
                    FForwardFactor = Q12(1.0);
            }
            else if(vloop_out < 5)
            {
                if(FForwardFactor > 0)
                    FForwardFactor--;
            }

            /* ************************ CCM duty feed forward ************************************** */
            if(VbusRefTemp > Vin_filt)
            {
                FForward_ccm = __builtin_divmodsd((((long) VbusRefTemp - Vin_filt) << 12), VbusRefTemp, &Remaindertemp);
                if(FForward_ccm > MAX_FFORWARD_CCM)
                    FForward_ccm = MAX_FFORWARD_CCM;
            }
            else
                FForward_ccm = 0;
            /* ************************ CCM duty feed forward END  ************************************** */

        /*  **************************************************************************************************
        D = SquareRoot( 2*L*Iref*(Uo - Ui)/(Uo * Ui * Ts) = SquareRoot( ((2 * L / Ts) * Iref * Dccm)/Ui  )       
            K = (2 * L / Ts) * (IrefMax / VinMax) = 
         ****************************************************************************************************	*/

         /* ************************ DCM duty feed forward ****************************************** */
            if(PRI_DATA.VAC_Range_LL)       //if LOW Line
                ulDummy = __builtin_muluu(Iref, KL) >> 12;  
            else//Hifgh Line
                ulDummy = __builtin_muluu(Iref, KL_115) >> 12;            //
            
            if(ulDummy < Q12(0.015))            // if Iref = 0, Vin = 0, Feedford change too fast
                ulDummy = Q12(0.015);

            if((ulDummy < Vin_filt) )           
            {
                ulDummy = __builtin_divud((ulDummy << 12), Vin_filt);
                ulDummy = (uint16_t) SQRTQ15lin10_FS(__builtin_muluu(ulDummy, FForward_ccm) >> 9); // Q15 /* Execute Time: 1.5us, SquareRoot: 1us */
                ulDummy = (__builtin_muluu(ulDummy, 32767)) >> 18; // Q12

                if(PRI_DATA.FF_Boost_1)
                    ulDummy = (__builtin_muluu(ulDummy, Q10(1.15))) >> 10;

                if((ulDummy > FForward_ccm))
                    FForward = FForward_ccm;
                else
                    FForward = ulDummy;
            }
            else
                FForward = FForward_ccm;
        /* ************************ DCM duty feed forward END*********************************** */

            FForward = (__builtin_mulss(FForward, FForwardFactor));

          #else
            if(vloop_out > 30)
            {
                if(FForwardFactor < Q12(1.0))
                    FForwardFactor++;
            }
            else if(vloop_out < 10)
            {
                if(FForwardFactor > 0)
                    FForwardFactor--;
            }
            #if FEEDFORWARD_SIMPLIFY // Duty = 1 - Vin / Vinmax
                FForward_ccm = Q12(1.0) - Vin_filt;
            #else // Duty = 1 - Vin / Vo
                if(Vbus_filt > Vin_filt)
                {
                    FForward_ccm = __builtin_divmodsd((((long) Vbus_filt - Vin_filt) << 12), Vbus_filt, &Remaindertemp);
                    if(FForward_ccm > MAX_FFORWARD_CCM)
                        FForward_ccm = MAX_FFORWARD_CCM;
                }
                else
                    FForward_ccm = 0;
            #endif
            FForward = (__builtin_mulss(FForward_ccm, FForwardFactor));
          #endif

        #else
            FForward = 0;
        #endif
        /* ***************************** Duty feed forward END *********************************************************************** */   

        /* ***************************** Duty feed forward END *********************************************************************** */   

    } // if (PRI_DATA.PFC_EN)
    else
    {
        PDC1 = 0;
        PWM_DISABLE();
        PRI_DATA.CL_1st_PASS = 1;
        PRI_DATA.CL_SoftStart = 1;
        e1_v = 0;
        u1_v = 0;

        *PFC_DUTY_Ptr[PFC_A] = 0;
        *PFC_ADC_TRIG_Ptr[PFC_A] = 0;
        e1_i[PFC_A] = 0;
        u1_i[PFC_A] = 0;

        #if PFC_B_ON_BOARD
            *PFC_DUTY_Ptr[PFC_B] = 0;
            *PFC_ADC_TRIG_Ptr[PFC_B] = 0;
            e1_i[PFC_B] = 0;
            u1_i[PFC_B] = 0;
        #endif
    }    

    //***************************** Power Meter ********************************

    if(PRI_DATA.VPOL == 0)
    {
        if((L_VOLTAGE_BUF > N_VOLTAGE_BUF + 10) && (Pmeter.Sample_Cnt_Tmp > 100))
            PRI_DATA.VPOL = 1;
    }
    else
    {
        if((N_VOLTAGE_BUF > L_VOLTAGE_BUF + 10) && (Pmeter.Sample_Cnt_Tmp > 100))
            PRI_DATA.VPOL = 0;
    }
    
   // Iin = INPUT_CURRENT_BUF << 2;
    Iin = INPUT_CURRENT_BUF;
    Pmeter.Vin_Sum_Tmp += ((uint32_t) __builtin_muluu(Vin, Vin));
    Pmeter.Iin_Sum_Tmp += ((uint32_t) __builtin_muluu(Iin, Iin));
    Pmeter.Pin_Sum_Tmp += ((uint32_t) __builtin_muluu(Vin, Iin));

    Pmeter.Sample_Cnt_Tmp++;

 
    
    if(((PRI_DATA.VPOL_1 != PRI_DATA.VPOL) && (Pmeter.Sample_Cnt_Tmp > 240)) || (Pmeter.Sample_Cnt_Tmp > 1200)) // 14ms   
    {
        PRI_DATA.New_AC_Data = 1;
        Pmeter.Vin_Sum = Pmeter.Vin_Sum_Tmp;
        Pmeter.Iin_Sum = Pmeter.Iin_Sum_Tmp;
        Pmeter.Pin_Sum = Pmeter.Pin_Sum_Tmp >> 8;
        Pmeter.Sample_Cnt = Pmeter.Sample_Cnt_Tmp;
        Pmeter.Vin_Sum_Tmp = 0;
        Pmeter.Iin_Sum_Tmp = 0;
        Pmeter.Pin_Sum_Tmp = 0;
        Pmeter.Sample_Cnt_Tmp = 0;
        

    }
      
    PRI_DATA.VPOL_1 = PRI_DATA.VPOL; 		

    
}
// Load Balancing between boost converters 
// Description: Due to small difference on inductor resistance and MOSFET resistance, load balancing is required. This subroutine compares two inductor 
// currents and adjusts duty cycle to compensate for that difference. A PI controller is used for this compensation as well.
// Load Balance Variables
// void LoadBalance(void);

void LoadBalance(void)
{
	#if 0//LOAD_BALANCING_ENABLE
    static volatile int16_t LoadBalError = 0;
    static volatile int16_t DiffMosfetCurrent = 0;
    static volatile int16_t LoadBalPoutput = 0;
    static volatile int32_t LoadBalIoutputTemp_long = 0;
    static volatile int32_t LoadBalIoutput = 0; 
    static volatile int32_t LoadBalPIoutputTemp_long = 0;
    static volatile int32_t LoadBalPIoutput = 0;
    
	// Difference of the two Mosfet currents
	DiffMosfetCurrent = (((*ADCBUF_CT_Ptr[PFC_B]) - (*ADCBUF_CT_Ptr[PFC_A]))>>2);					
	LoadBalError = (int)REFLOADBALCURRENT - DiffMosfetCurrent;	// Load balance error

	if(LoadBalError > 2047)			//4095)									// Check and saturate the error between -1 and 1
		LoadBalError = 2047;		//4095;										
	else if(LoadBalError < -2047)	//-4095)
		LoadBalError = -2047;		//-4095;

	// Error * Proportional Gain
	LoadBalPoutput = ( (__builtin_mulss(LoadBalError,K1_LOADBAL)) >> 12);

	// Integral Term
	LoadBalIoutputTemp_long = LoadBalIoutputTemp_long + ( (__builtin_mulss(LoadBalError,K2_LOADBAL)) >> 12);
	
	// Check for Integral term exceeding MAX_BALANCE, If true, saturate the integral term to MAX_BALANCE.
	// Check for Integrak term going below -MAX_BALANCE, If true, saturate the integral term to -MAX_BALANCE

	if(LoadBalIoutputTemp_long > MAX_BALANCE)
		LoadBalIoutputTemp_long = MAX_BALANCE;
	else if(LoadBalIoutputTemp_long < -MAX_BALANCE)
		LoadBalIoutputTemp_long = -MAX_BALANCE;
	
	LoadBalIoutput = (int)LoadBalIoutputTemp_long;

	// PI Output = Proportional Term + Integral Term
	LoadBalPIoutputTemp_long = (long) LoadBalPoutput + (long) LoadBalIoutput;
	
	// Check for PI Output exceeding MAX_BALANCE (based on max allowable delta)
	// If true, saturate the PI Output to MAX_BALANCE
	// Check for PI Output going below -MAX_BALANCE, If true, saturate the PI Output to -MAX_BALANCE

	if(LoadBalPIoutputTemp_long > MAX_BALANCE)
		LoadBalPIoutputTemp_long = MAX_BALANCE;
	else if(LoadBalPIoutputTemp_long < -MAX_BALANCE)
		LoadBalPIoutputTemp_long = -MAX_BALANCE;
	
	LoadBalPIoutput = (int)LoadBalPIoutputTemp_long;
	Duty_LoadBal = ( (__builtin_mulss(LoadBalPIoutput,PFC_PWM_PERIOD1)) >> 15);
	// Duty difference to correct the two duty cycles
	//Duty_LoadBal = ( (__builtin_mulss(LoadBalPIoutput,(*PFC_PWM_PERIOD_Ptr))) >> 12);
#endif
}
/*****************************************************************************/

/*
 * End of file
 */