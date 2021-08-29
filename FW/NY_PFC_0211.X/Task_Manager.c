/*******************************************************************************
 * Filename   : Task_Manager.c
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
//Control Status Flags
FLAG_STA Control; 

//Primary Data
PRIMARY_DATA PRI_DATA;
VinVoltage Vin_Mon;

//AC ALARM Variables
unsigned int Dropout_Delay = 0, Bulk_OK_Var_Delay = 0;

//VIn Range Detection
unsigned int V_Range_Count = 0;

//Bus Control
signed int VbusRef = 0,VRefRead=0;

//Bulk and Temperature ADC Averaging
unsigned long ADC_Arr[4] = {0,0,0,0};
unsigned char ADC_Acc_Count = 0,T_ADC_Count = 0;

//For Control Loop
signed int Vavg_Inv_Sq;

//EEPROM Write
unsigned int Eep_Cntl_State = 0;
CAL_DATA_BUFFER Cal_Data_Buf;

//Power Metering
POWER_METER Pmeter;

//For Dig PFC
unsigned int Vbus = 0;
unsigned int Vin = 0;
unsigned int Iin = 0;

signed long Max_u0_i;

unsigned int PfcPeriod;
unsigned int PfcPeriodBase;

ADD_VARIABLES VAR;
unsigned char Av,Avcount;

/******************************************************************************
                             Constant Declaration
*****************************************************************************/

/******************************************************************************
                             Macro Declaration
*****************************************************************************/

/******************************************************************************
                            Subroutine Declaration
*****************************************************************************/

/*******************************************************************************
 * Function:        Input_Measurement *
 * Description:     input voltage fail detection
 * Calling:         every 200us
 ******************************************************************************/
void __attribute__((__section__("APP_IMAGE"))) Input_Measurement(void)
{
    static unsigned int Dropout_Dly = 0, Inst_UV_Dly = 0, Inst_OV_Dly = 0; 
    
   //Input Peak Sensing
    Vin_Mon.Count++;
    if(Vin > Vin_Mon.VPeak_Temp)
        Vin_Mon.VPeak_Temp = Vin;
    if(Iin > Vin_Mon.IPeak_Temp)
        Vin_Mon.IPeak_Temp = Iin;
    
    if(PRI_DATA.VPOL_2 != PRI_DATA.VPOL)
    {
        if(Vin_Mon.Count > 38 &&  Vin_Mon.Count < 56) //From 7.6mS to 11.2mS
        {
            PRI_DATA.VIN_PEAK_ADC = Vin_Mon.VPeak_Temp;
            PRI_DATA.IIN_PEAK_ADC = Vin_Mon.IPeak_Temp;
            Vin_Mon.VPeak_Temp = 0;
            Vin_Mon.IPeak_Temp = 0;
        }
        Vin_Mon.Count = 0;
    }
    else if(Vin_Mon.Count > AC_HALF_PERIOD_MAX)
    {
        PRI_DATA.VIN_PEAK_ADC = Vin_Mon.VPeak_Temp;
        PRI_DATA.IIN_PEAK_ADC = Vin_Mon.IPeak_Temp;
        Vin_Mon.VPeak_Temp = 0;
        Vin_Mon.IPeak_Temp = 0;
        Vin_Mon.Count = 0;        
    }
    
    PRI_DATA.VPOL_2 = PRI_DATA.VPOL;
    
    //Dropout Detection  <20V RMS for > 11mS
    if(!PRI_DATA.A_AC_Dropout)      // if Not Detected - Check for Dropout
    {
        if(Vin < DROPOUT_TRIG_ADC_PK)
        {            
            if(Dropout_Dly > Dropout_Delay)
            {
                PRI_DATA.A_AC_Dropout = 1;
                Dropout_Dly = 0;
            }
            else
                Dropout_Dly++;
            
            if(Dropout_Dly > DROPOUT_TRIG_DELAY_FL)
                PRI_DATA.A_ALERT_PFC = 1;
        }
        else
        {
            if(Dropout_Dly > DROPOUT_ALERT_DELAY)
                PRI_DATA.A_ALERT_PFC = 0;
                                  
            if(Dropout_Dly!=0)
                Dropout_Dly--;   
        }
    }
       
    //Instant Value Under Voltage Detection  <61V RMS for > 27mS
    if(!PRI_DATA.A_AC_IUV)      // if Not Detected - Check for Dropout
    {
        if(Vin < INST_UV_TRIG_ADC_PK)
        {            
            if(Inst_UV_Dly > INST_UV_TRIG_DELAY)
            {
                PRI_DATA.A_AC_IUV = 1;
                Inst_UV_Dly = 0;
            }
            else
                Inst_UV_Dly++;
            
            if(Inst_UV_Dly > INST_UV_ALERT_DELAY)
                PRI_DATA.A_ALERT_PFC = 1;
        }
        else
        {
            if(Inst_UV_Dly > INST_UV_ALERT_DELAY)
                PRI_DATA.A_ALERT_PFC = 0;
            Inst_UV_Dly = 0;
        }
    }
        
    //Instant Value Over Voltage Detection  >315V RMS for > 27mS
    if(!PRI_DATA.A_AC_IOV)      // if Not Detected - Check for Dropout
    {
        if(Vin > INST_OV_TRIG_ADC_PK)
        {            
            if(Inst_OV_Dly > INST_OV_TRIG_DELAY)
            {
                PRI_DATA.A_AC_IOV = 1;
                Inst_OV_Dly = 0;
            }
            else
                Inst_OV_Dly++;
        }
        else
        {            
            if(Inst_OV_Dly!=0)
                Inst_OV_Dly--;    
        }
    }
    
    //AC OK Pin Control
    if(PRI_DATA.AC_OK)       //AC OK Pin high(AC OK) - Test to make it Low(AC Fail)
    {     
        if(PRI_DATA.A_AC_Dropout || PRI_DATA.A_AC_IUV || PRI_DATA.A_AC_IOV || PRI_DATA.A_AC_Sag)
        {
            PRI_DATA.AC_OK = 0;

			AC_NOK_PRI_SetHigh();
            
        }
    }
    
}

/*******************************************************************************
 * Function:        Bulk_Measurement *
 * Description:     Bulk Voltage fail detection
 * Calling:         every 200us
 ******************************************************************************/
void __attribute__((__section__("APP_IMAGE"))) Bulk_Measurement(void)
{
    static unsigned int Bulk_UV1_Count = 0, Bulk_UV2_Count = 0, Bulk_Ok_Count = 0; 
    //Bulk UV 1 - Fast Response
    if(!PRI_DATA.A_BULK_UV1)        //if Bulk Pass Test for Under Voltage
    {
        if(Vbus < BULK_UV1_TRIG_ADC)
        {
            Bulk_UV1_Count++;
            if(Bulk_UV1_Count > BULK_UV1_TRIG_DELAY)
            {
                PRI_DATA.A_BULK_UV1 = 1;
                Bulk_UV1_Count = 0;
            }
        }
        else
            Bulk_UV1_Count = 0;
    }
    
    ///Bulk UV 2  - Slow Response
    if(!PRI_DATA.A_BULK_UV2)        //if Bulk Pass Test for Under Voltage
    {
        if(Vbus < BULK_UV2_TRIG_ADC)
        {
            Bulk_UV2_Count++;
            if(Bulk_UV2_Count > BULK_UV2_TRIG_DELAY)
            {
                PRI_DATA.A_BULK_UV2 = 1;
                Bulk_UV2_Count = 0;
            }
        }
        else
            Bulk_UV2_Count = 0;
    }
    
    if(PRI_DATA.A_BULK_UV1 || PRI_DATA.A_BULK_UV2)                         //if Bulk Under voltage test for Pass
    {
        if(Vbus > BULK_OK_ADC && PRI_DATA.PFC_En) 
        {
            Bulk_Ok_Count++;
            if(Bulk_Ok_Count > (BULK_OK_DELAY + Bulk_OK_Var_Delay))
            {
                PRI_DATA.A_BULK_UV1 = 0;
                PRI_DATA.A_BULK_UV2 = 0;
                Bulk_Ok_Count = 0;
                Bulk_OK_Var_Delay = 0;
            }
        }
        else
            Bulk_Ok_Count = 0;
    }
    
    //Bulk OV Detection from External Op Amp and Moved to Control Loop
    if (BUS_VOLTAGE_BUF > BULK_OV_ALERT_ADC)
    {
        PRI_DATA.A_BULK_OV_ALERT = 1;   

    }
    else if (BUS_VOLTAGE_BUF < BULK_OV_ALERT_REC_ADC)
    {
        PRI_DATA.A_BULK_OV_ALERT = 0;
    }
    
    //Bulk OK Pin Control
    if(PRI_DATA.BULK_OK)      //Bulk OK Pin Low(Bulk Fail) - Test to make it High(Bulk OK)
    {      
        if((PRI_DATA.A_BULK_UV2)||(PRI_DATA.A_BULK_UV1))   
        {
            PRI_DATA.BULK_OK = 0;
            //PIN_BULK_OK     = 1;   //disable
			PFC_NOK_PRI_SetHigh();

        }
    }
    else
    {
        if((!PRI_DATA.A_BULK_UV2)&&(!PRI_DATA.A_BULK_UV1))  
        {
            PRI_DATA.BULK_OK = 1;
            //PIN_BULK_OK     = 0; //enable
			PFC_NOK_PRI_SetLow();
        }
    }
}

/*******************************************************************************
 * Function:        Input_Judgement
 * Description:     Judge input peak voltage,fast input fail detetion, PLD
 * Calling:         every 1ms
 ******************************************************************************/
void __attribute__((__section__("APP_IMAGE"))) Input_Judgement(void)
{
    static unsigned int Vin_OK_Count  = 0, AC_Sag_Count = 0, AC_OV_Count = 0, AC_UVW_Count = 0, AC_OVW_Count = 0,AC_OCW_Count = 0;
                    
    //VIN Sag Detection            
    if(!PRI_DATA.A_AC_Sag)        //if Input OK Test for SAG
    {
        if(PRI_DATA.VRMS_Inst < AC_SAG_TRIG_THD)
        {            
            if(AC_Sag_Count > AC_SAG_TRIG_DELAY)
            {
                PRI_DATA.A_AC_Sag = 1;
                AC_Sag_Count = 0;
            }
            else
                AC_Sag_Count++;

            if(AC_Sag_Count > AC_SAG_ALERT_DELAY)
                PRI_DATA.A_ALERT_PFC = 1;
        }
        else
        {
            if(AC_Sag_Count > AC_SAG_ALERT_DELAY)
                PRI_DATA.A_ALERT_PFC = 0;

            if(AC_Sag_Count!=0)
                AC_Sag_Count--;                
        }
    }
    else
        AC_Sag_Count = 0;

    //Input Over Voltage
    if(!PRI_DATA.A_AC_OV)        //if Input OK Test for Over Voltage
    {
        if(PRI_DATA.VRMS_Inst > AC_OV_TRIG_THD)
        {
            AC_OV_Count++;
            if(AC_OV_Count > AC_OV_TRIG_DELAY)
            {
                PRI_DATA.A_AC_OV = 1;
                AC_OV_Count = 0;
            }
        }
        else
            AC_OV_Count = 0;
    }
    else                        //if Input Over voltage test for Pass
    {
        if(PRI_DATA.VRMS_Inst < AC_OV_REC_THD)
        {
            AC_OV_Count++;
            if(AC_OV_Count > AC_OV_REC_DELAY)
            {
                PRI_DATA.A_AC_OV = 0;
                AC_OV_Count = 0;
            }
        }
        else
            AC_OV_Count = 0;
    }

    //AC Warning Trigger
        //Input Under Voltage Warning
    if(!PRI_DATA.A_AC_UVW)        //if Input OK Test for Under Voltage Warning
    {
        if(PRI_DATA.VRMS_Inst < AC_UVW_TRIG_THD && !PRI_DATA.A_AC_Sag)
        {
            AC_UVW_Count++;
            if(AC_UVW_Count > AC_UVW_TRIG_DELAY)
            {
                PRI_DATA.A_AC_UVW = 1;
                AC_UVW_Count = 0;
            }
        }
        else
            AC_UVW_Count = 0;
    }
    else                        //if Input Under voltage Warning test for Pass
    {
        if(PRI_DATA.VRMS_Inst > AC_UVW_REC_THD)
        {
            AC_UVW_Count++;
            if(AC_UVW_Count > AC_UVW_REC_DELAY)
            {
                PRI_DATA.A_AC_UVW = 0;
                AC_UVW_Count = 0;
            }
        }
        else
            AC_UVW_Count = 0;
    }

    //Input Over Voltage Warning
    if(!PRI_DATA.A_AC_OVW)        //if Input OK Test for OVER  Voltage Warning
    {
        if(PRI_DATA.VRMS_Inst > AC_OVW_TRIG_THD && !PRI_DATA.A_AC_OV)
        {
            AC_OVW_Count++;
            if(AC_OVW_Count > AC_OVW_TRIG_DELAY)
            {
                PRI_DATA.A_AC_OVW = 1;
                AC_OVW_Count = 0;
            }
        }
        else
            AC_OVW_Count = 0;
    }
    else                        //if Input OVER  voltage Warning test for Pass
    {
        if(PRI_DATA.VRMS_Inst < AC_OVW_REC_THD)
        {
            AC_OVW_Count++;
            if(AC_OVW_Count > AC_OVW_REC_DELAY)
            {
                PRI_DATA.A_AC_OVW = 0;
                AC_OVW_Count = 0;
            }
        }
        else
            AC_OVW_Count = 0;
    }
    //Input Over Current Warning
    if(!PRI_DATA.A_AC_OCW)         
    {
        if(PRI_DATA.IRMS_Inst > AC_OCW_TRIG_THD)
        {
            AC_OCW_Count++;
            if(AC_OCW_Count > AC_OCW_TRIG_DELAY)
            {
                PRI_DATA.A_AC_OCW = 1;
                AC_OCW_Count = 0;
            }
        }
        else
            AC_OCW_Count = 0;
    }
    else                        //if Input OVER  voltage Warning test for Pass
    {
        if(PRI_DATA.IRMS_Inst < AC_OCW_REC_THD)
        {
            AC_OCW_Count++;
            if(AC_OCW_Count > AC_OCW_REC_DELAY)
            {
                PRI_DATA.A_AC_OCW = 0;
                AC_OCW_Count = 0;
            }
        }
        else
            AC_OCW_Count = 0;
    }
    //printf("%d\n",PRI_DATA.VIN_PEAK_ADC);
    //VIN OK 
    if(!PRI_DATA.AC_OK) //if AC not OK
    {
        if((PRI_DATA.VRMS_Inst > VIN_OK_TRIG_THD) && (PRI_DATA.VIN_PEAK_ADC < INST_OV_TRIG_ADC_PK))  //INST_OV_TRIG_ADC_PK = 3640
        {
            if(Vin_OK_Count > VIN_OK_DELAY) 
            {
                PRI_DATA.AC_OK = 1;
                PRI_DATA.A_AC_Dropout = 0;
                PRI_DATA.A_AC_IUV = 0;
                PRI_DATA.A_AC_IOV = 0;
                PRI_DATA.A_AC_Sag = 0;

                PRI_DATA.A_ALERT_PFC = 0;           

                Vin_OK_Count = 0;

                PRI_DATA.AC_OK = 1;

				AC_NOK_PRI_SetLow();
            }
            else
                Vin_OK_Count++;
        }
        else if(Vin_OK_Count > 0)
            Vin_OK_Count--; 
    }
}

/*******************************************************************************
 * Function:        Relay_Control
 * Description:     Relay control on/off timing
 * Calling:         every 1ms
 ******************************************************************************/
void __attribute__((__section__("APP_IMAGE"))) Relay_Control(void)
{
    static unsigned int Relay_Start_Count = 0, Relay_ON_Count = 0;
    if(PRI_DATA.VIN_PEAK_ADC > RELAY_ON_THD_ADC_PK)
    {
        if(!PRI_DATA.Relay_Start)
        {
            Relay_Start_Count++;
            //if( Relay_Start_Count > RELAY_START_DELAY)
            if((((int)PRI_DATA.VIN_PEAK_ADC - (int) Vbus) < (int) VIN_VBUS_DIFF) && Relay_Start_Count > RELAY_START_DELAY)
            {
                Relay_Start_Count = 0;
                PRI_DATA.Relay_Start = 1;
                //PDC3=0x4000;
				RELAY_START();
            }
        }
        else if(!PRI_DATA.Relay_PWM_ON)
        {
            Relay_ON_Count++;
            if(Relay_ON_Count > RELAY_ON_DELAY)
            {
                RELAY_PWM_ON();
				//RELAY_START(); //temp this line is added temporarily to disable relay PWM (Nanyang P1)
                PRI_DATA.Relay_PWM_ON = 1;
                Relay_ON_Count = 0;
				//RELAY_START(); //temp this line is added temporarily to disable relay PWM (Nanyang P1)
            }
        }
    }
    else if(PRI_DATA.VIN_PEAK_ADC < RELAY_OFF_THD_ADC_PK && !PRI_DATA.PFC_En)
    {
        PRI_DATA.Relay_PWM_ON = 0;
        PRI_DATA.Relay_Start = 0;
        RELAY_OFF();
		//PDC3=0;
        Relay_Start_Count = 0;
        Relay_ON_Count = 0;                
    }

}

/*******************************************************************************
 * Function:        PFC_Control
 * Description:     PFC control on/off timing
 * Calling:         every 1ms
 ******************************************************************************/
void __attribute__((__section__("APP_IMAGE"))) PFC_Control(void)
{    
    static unsigned int PFC_ON_Count = 0, PFC_OFF_Count = 0, PFC_Restart_Count = 0,Bulk_dly=0;
    static unsigned char PFC_State = 0,Bulk_Startup_Sta=0;
    //printf("%d",6 );
   switch(PFC_State)
    {
        case PFC_INIT: //Sense for AC OK and Turn ON PFC
            if(PRI_DATA.AC_OK && PRI_DATA.Relay_Start)
            {
                if(PFC_ON_Count > PFC_ON_DELAY)
                {
                    PFC_ON_Count = 0;
                    PRI_DATA.PFC_En = 1; 

                    Bulk_Startup_Sta = 1;
                    #if PFC_ALWAYS_OFF
                        PWM_DISABLE();
                    #else
                        PWM_ENABLE(); 
                    #endif             
                    PFC_State = PFC_ON;
                    
                    //To Provide Consistent Bulk OK Delay to Sec Side
                    if(PRI_DATA.VRMS_Inst > 800)    //>100V
                        Bulk_OK_Var_Delay = PRI_DATA.VRMS_Inst - 800;
                    else
                        Bulk_OK_Var_Delay = 0;
                }
                else
                    PFC_ON_Count++;
            }
            else
            {
                if(PFC_ON_Count > 0)
                    PFC_ON_Count--;
            }
            break;
        case PFC_ON: //Sense for AC BAD to Turn OFF PFC
            if(!PRI_DATA.AC_OK || !PRI_DATA.PFC_GO) 
            {
                PFC_OFF_Count++;
                if(PFC_OFF_Count > PFC_OFF_DELAY)
                {
                    PFC_OFF_Count = 0;
                    PWM_DISABLE();
                    PRI_DATA.PFC_En = 0;
                    PFC_State = PFC_OFF;
                }
            }
            break;
        case PFC_OFF:  //Wait for Restart Delay
            PFC_Restart_Count++;
            if(PFC_Restart_Count > PFC_RESTART_DELAY)
            {
                PFC_State = PFC_INIT;
                PFC_Restart_Count = 0;
            }
            break;
        default:
            PFC_State = PFC_INIT;
            PFC_ON_Count = 0;
            PFC_OFF_Count = 0;
            break;
    }
   
   if(Bulk_Startup_Sta == 1)
   {
       Bulk_dly++;
       if(Bulk_dly>299)  //300mS
       {
           Bulk_dly = 0;
           PRI_DATA.Bulk_High_Sta = 1;
           Bulk_Startup_Sta = 2;
       }
   }
   else if(Bulk_Startup_Sta == 2)
   {
       Bulk_dly++;
       if(Bulk_dly>999) //1Secs
       {
           Bulk_dly = 0;
           PRI_DATA.Bulk_High_Sta = 0;
           Bulk_Startup_Sta = 3;           
       }
   }
}

/*******************************************************************************
 * Function:        Bus_Control
 * Description:     High line/low line state conversion timing, PLD
 * Calling:         every 10ms
 ******************************************************************************/
void __attribute__((__section__("APP_IMAGE"))) Bus_Control(void)
{
    static unsigned int V_Range_Count = 0; 
    static unsigned char Droop_Mode_Dly = 0, Active_Mode_Dly = 0, Bulk_Status = 0;    
    static unsigned int Vbus_Ref[5] = {VBUS_REF0_ACT_ADC_HL, VBUS_REF1_ACT_ADC_HL, VBUS_REF2_ACT_ADC_HL, VBUS_REF3_ACT_ADC_HL, VBUS_REF4_ACT_ADC_HL};
    
    //Detect Line Voltage Level
    if(!PRI_DATA.VAC_Range_LL) //if High Line Sense for Low Line
    {
        if(PRI_DATA.VIN_PEAK_ADC < VIN_LOW_LINE_ADC_PK)  // < 166VAC
        {
            V_Range_Count++;
            if(PRI_DATA.VIN_PEAK_ADC < VIN_LOW_LINE_ADC_PK2)  // < 125VAC
            {
                if(V_Range_Count > 5)   // 60mS
                {
                    V_Range_Count = 0;
                    PRI_DATA.VAC_Range_LL = 1;
                    Max_u0_i = MAX_u0_I_LL;
                }
            }
            else if(PRI_DATA.VIN_PEAK_ADC < VIN_LOW_LINE_ADC_PK1)  // < 145VAC
            {
                if(V_Range_Count > 14)   // 150mS
                {
                    V_Range_Count = 0;
                    PRI_DATA.VAC_Range_LL = 1;
                    Max_u0_i = MAX_u0_I_LL;
                }
            }
            else
            {
                if(V_Range_Count > 29)   // 300mS
                {
                    V_Range_Count = 0;
                    PRI_DATA.VAC_Range_LL = 1;
                    Max_u0_i = MAX_u0_I_LL;
                }                                
            }                            
        }
        else
            V_Range_Count = 0;
    }
    else
    {
        if(PRI_DATA.VIN_PEAK_ADC > VIN_HIGH_LINE_ADC_PK)  // > 170VAC
        {
            V_Range_Count++;
            if(V_Range_Count > 0)   //in *10mS Steps
            {
                PRI_DATA.VAC_Range_LL = 0;
                Max_u0_i = MAX_u0_I_HL;
            }
        }
        else
            V_Range_Count = 0;
    }

    //Change Reference Voltage Based on Mode  Droop or Active
    if (!PRI_DATA.Sec_CMD_Droop_En) //Active Mode
    {
        Droop_Mode_Dly = 0;
        if (Active_Mode_Dly > 10)
        {
            if(!PRI_DATA.VAC_Range_LL)  //High LIne
            {
                Vbus_Ref[0] = VBUS_REF0_ACT_ADC_HL;
                Vbus_Ref[1] = VBUS_REF1_ACT_ADC_HL;
                Vbus_Ref[2] = VBUS_REF2_ACT_ADC_HL;
                Vbus_Ref[3] = VBUS_REF3_ACT_ADC_HL;
                Vbus_Ref[4] = VBUS_REF4_ACT_ADC_HL;
            }
            else
            {
				Vbus_Ref[0] = VBUS_REF0_ACT_ADC_HL;
                Vbus_Ref[1] = VBUS_REF1_ACT_ADC_HL;
                Vbus_Ref[2] = VBUS_REF2_ACT_ADC_HL;
                Vbus_Ref[3] = VBUS_REF3_ACT_ADC_HL;
                Vbus_Ref[4] = VBUS_REF4_ACT_ADC_HL;
                
//				  Vbus_Ref[0] = VBUS_REF0_ACT_ADC_LL;
//                Vbus_Ref[1] = VBUS_REF1_ACT_ADC_LL;
//                Vbus_Ref[2] = VBUS_REF2_ACT_ADC_LL;
//                Vbus_Ref[3] = VBUS_REF3_ACT_ADC_LL;
//                Vbus_Ref[4] = VBUS_REF4_ACT_ADC_LL;                
            }
            Active_Mode_Dly = 0;      
        }
        else
            Active_Mode_Dly++;  
    }
    else
    {
        Active_Mode_Dly = 0;
        if (Droop_Mode_Dly > 10)
        {                            
			Vbus_Ref[0] = VBUS_REF0_ACT_ADC_HL;
            Vbus_Ref[1] = VBUS_REF1_ACT_ADC_HL;
            Vbus_Ref[2] = VBUS_REF2_ACT_ADC_HL;
            Vbus_Ref[3] = VBUS_REF3_ACT_ADC_HL;
            Vbus_Ref[4] = VBUS_REF4_ACT_ADC_HL;
//            Vbus_Ref[0] = VBUS_REF0_DRP_ADC;
//            Vbus_Ref[1] = VBUS_REF1_DRP_ADC;
//            Vbus_Ref[2] = VBUS_REF2_DRP_ADC;
//            Vbus_Ref[3] = VBUS_REF3_DRP_ADC;
//            Vbus_Ref[4] = VBUS_REF4_DRP_ADC;
            Droop_Mode_Dly = 0;      
        }
        else
            Droop_Mode_Dly++;
    }  
	
	//UART1_WriteBuffer(&PRI_DATA,8);

    //Change Bus Voltage Reference Based on INput Power
    if(PRI_DATA.PFC_En)
    {
#if 0
        if(!PRI_DATA.CL_SoftStart)        //if Soft Start Completed
        {
            if(PRI_DATA.Bulk_High_Sta)
            {
                if(Bulk_Status != 5)
                {
                    if(Bus_Adj_Delay > 1) //20mS
                    {
                        PFC_ADIEL = PFC_ADIEL & 0xFFF7; //_ADCP3IE = 0; 
                        VbusRef = Vbus_Ref[4]; 
                        PRI_DATA.CL_1st_PASS = 1;
                        PRI_DATA.CL_SoftStart = 1;
                        Bulk_Status = 5;
                        PRI_DATA.Bulk_Range = 4; 
                        PFC_ADIEL = PFC_ADIEL | 0x8; //_ADCP3IE = 1; 
                    }
                    else
                        Bus_Adj_Delay++;
                } 
                else
                    Bus_Adj_Delay = 0;                
            }
            else if(PRI_DATA.PIN_Avg <= 152)     //  Power <80  PIN_BULK0_THR_H
            {
                if(Bulk_Status != 0)
                {
                    if(Bus_Adj_Delay > 1) //20mS
                    {
                        PFC_ADIEL = PFC_ADIEL & 0xFFF7; //_ADCP3IE = 0; 
                        VbusRef = Vbus_Ref[0]; 
                        PRI_DATA.CL_1st_PASS = 1;
                        PRI_DATA.CL_SoftStart = 1;
                        Bulk_Status = 0;
                        PRI_DATA.Bulk_Range = 0; 
                        PFC_ADIEL = PFC_ADIEL | 0x8; //_ADCP3IE = 1; 
                    }
                    else
                        Bus_Adj_Delay++;
                } 
                else
                    Bus_Adj_Delay = 0;
            }
        //    else if(PRI_DATA.PIN_Avg >= PIN_BULK1_THR_L && PRI_DATA.PIN_Avg <= PIN_BULK1_THR_H) // 100< Power <200
			else if(PRI_DATA.PIN_Avg >= 190 && PRI_DATA.PIN_Avg <= 380) // 100< Power <200
            {
                if(Bulk_Status != 1)
                {
                    if(Bus_Adj_Delay > 5)       //60mS
                    {
                        PFC_ADIEL = PFC_ADIEL & 0xFFFD; //_ADCP3IE = 0; 
                        VbusRef = Vbus_Ref[1]; 
                        PRI_DATA.CL_1st_PASS = 1;
                        PRI_DATA.CL_SoftStart = 1;
                        Bulk_Status = 1;
                        PRI_DATA.Bulk_Range = 1;
                        PFC_ADIEL = PFC_ADIEL | 0x2; //_ADCP3IE = 1; 
                    }
                    else
                        Bus_Adj_Delay++;
                }      
                else
                    Bus_Adj_Delay = 0;
            }
            //else if(PRI_DATA.PIN_Avg >= PIN_BULK2_THR_L && PRI_DATA.PIN_Avg <= PIN_BULK2_THR_H) //  220< Power <950
			else if(PRI_DATA.PIN_Avg >= 418 && PRI_DATA.PIN_Avg <= 1803) //  220< Power <950
            {
                if(Bulk_Status != 2)
                {
                    if(Bus_Adj_Delay > 2)       //30mS
                    {
                        PFC_ADIEL = PFC_ADIEL & 0xFFFD; //_ADCP3IE = 0; 
                        VbusRef = Vbus_Ref[2]; 
                        PRI_DATA.CL_1st_PASS = 1;
                        PRI_DATA.CL_SoftStart = 1;
                        Bulk_Status = 2;
                        PRI_DATA.Bulk_Range = 2;
                        PFC_ADIEL = PFC_ADIEL | 0x2; //_ADCP3IE = 1; 
                    }
                    else
                        Bus_Adj_Delay++;
                }      
                else
                    Bus_Adj_Delay = 0;
            }
            //else if(PRI_DATA.PIN_Avg >= PIN_BULK3_THR_L && PRI_DATA.PIN_Avg <= PIN_BULK3_THR_H) //  1000< Power <1200
			else if(PRI_DATA.PIN_Avg >= 1898 && PRI_DATA.PIN_Avg <= 2278) //  1000< Power <1200
            {
                if(Bulk_Status != 3)
                {
                    if(Bus_Adj_Delay > 1)       //20mS
                    {
                        PFC_ADIEL = PFC_ADIEL & 0xFFF7; //_ADCP3IE = 0; 
                        VbusRef = Vbus_Ref[3]; 
                        PRI_DATA.CL_1st_PASS = 1;
                        PRI_DATA.CL_SoftStart = 1;
                        Bulk_Status = 3;
                        PRI_DATA.Bulk_Range = 3;
                        PFC_ADIEL = PFC_ADIEL | 0x8; //_ADCP3IE = 1; 
                    }
                    else
                        Bus_Adj_Delay++;
                }      
                else
                    Bus_Adj_Delay = 0;
            }
//            if(PRI_DATA.PIN_Avg >= PIN_BULK4_THR_L) //  1250< Power
			else if(PRI_DATA.PIN_Avg >= 2373) //  1250> Power
            {
                if(Bulk_Status != 4)
                {
                    if(Bus_Adj_Delay > 1)       //20mS
                    {
                        PFC_ADIEL = PFC_ADIEL & 0xFFF7; //_ADCP3IE = 0; 
                        VbusRef = Vbus_Ref[4]; 
                        PRI_DATA.CL_1st_PASS = 1;
                        PRI_DATA.CL_SoftStart = 1;
                        Bulk_Status = 4;
                        PRI_DATA.Bulk_Range = 4;
                        PFC_ADIEL = PFC_ADIEL | 0x8; //_ADCP3IE = 1; 
                    }
                    else
                        Bus_Adj_Delay++;
                }      
                else
                    Bus_Adj_Delay = 0;
            }
        } 
#endif
    }
    else
    {
    	/*  for testing 
		if((VRefRead > 0)&&( VRefRead <= 50))
    	{
			VbusRef = 400 + VRefRead;
			//Tx_Buf[0] = VRefRead; 
    	}
		else
		{
			VbusRef = Vbus_Ref[2];
		}
		*/
		VbusRef = Vbus_Ref[2];
        PRI_DATA.CL_1st_PASS = 1; 
        PRI_DATA.CL_SoftStart = 1;
        Bulk_Status = 2;     
        PRI_DATA.Bulk_Range = 2;
    }
}

/*******************************************************************************
 * Function:        PowerLimit_Factor
 * Parameters:
 * Returned value:
 * Description:     Power limit factor calculation
 * Calling:         Every 10mS
 ******************************************************************************/

void __attribute__((__section__("APP_IMAGE"))) PowerLimit_Factor(void)
{
    static uint16_t VinAvgTemp = 0;
    uint16_t VavgInvSquareTmp = 0;
    uint16_t Remaindertemp;

    VinAvgTemp = PRI_DATA.VRms_ADC;
    
  //  UART1_WriteBuffer(&Vin,2);
    if (VinAvgTemp > VIN_AVG_MIN)
    {
        VavgInvSquareTmp = __builtin_divmodud(POWER_LIMIT_FACTOR_TEMP,
                                              ((__builtin_muluu(VinAvgTemp, VinAvgTemp)) >> 12),
                                              &Remaindertemp);
        if (VavgInvSquareTmp > Q12(1.0))
            Vavg_Inv_Sq = Q12(1.0);
        else if (VavgInvSquareTmp < Q12(0.2))
            Vavg_Inv_Sq = Q12(0.2);
        else
            Vavg_Inv_Sq = VavgInvSquareTmp;
    }
    else 
        Vavg_Inv_Sq = Q12(1.0);
}

/*******************************************************************************
 * Function:        CalcSquareRoot
 * Parameters:      ulValue - squared value
 * Returned value:  square root from ulValue
 * Description:     This function has the aim to calculate the square root from
 *                  ulValue. The result is limited to a maximum resolution of 12bit.
 * Calling:
 ******************************************************************************/
uint16_t __attribute__((__section__("APP_IMAGE"))) CalcSquareRoot(unsigned long ulValue)
{
    uint8_t ucBitPos;
    uint16_t uiLastResult;
    uint16_t uiResult = 0;
    uint32_t ulResultSqr;

    if (ulValue != 0)
    {
        // If ulValue is not zero then perform calculation
        for (ucBitPos = 12; ucBitPos >= 1; ucBitPos--)
        {
            uiLastResult = uiResult;
            uiResult |= (0x0001 << (ucBitPos - 1));
            ulResultSqr = (__builtin_mulss(uiResult, uiResult));
            if (ulValue < ulResultSqr)
                uiResult = uiLastResult;
        }
    }
    return ( uiResult);
} // CalcSquareRoot()

/*******************************************************************************
 * Function:        Func64Div16to32
 * Parameters:      unsigned long long dividend, unsigned int divisor
 * Returned value:  unsigned long quotient
 * Description:     return: quotient = dividend / divisor
 * Calling:
 ******************************************************************************/
uint32_t __attribute__((__section__("APP_IMAGE"))) Func64Div16to32(unsigned long long dividend, unsigned int divisor)
{
    uint16_t Remainder = 0;
    uint16_t DivResult[4];
    
    if (divisor > 0)
    {
        DivResult[3] = __builtin_divmodud((dividend >> 48), divisor, &Remainder);
        DivResult[2] = __builtin_divmodud((((dividend >> 32) & 0xFFFF) + ((unsigned long) Remainder << 16)), divisor, &Remainder);
        DivResult[1] = __builtin_divmodud((((dividend >> 16) & 0xFFFF) + ((unsigned long) Remainder << 16)), divisor, &Remainder);
        DivResult[0] = __builtin_divmodud(((dividend & 0xFFFF) + ((unsigned long) Remainder << 16)), divisor, &Remainder);

        return (((unsigned long long) DivResult[3] << 48) + ((unsigned long long) DivResult[2] << 32) +
          ((unsigned long) DivResult[1] << 16) + DivResult[0]);
    }
    else
        return (0);
}

/*******************************************************************************
 * Function:        AC_Cycle_Tasks
 * Description:     Tasks Related to AC Sampling and RMS Calculation
 * Calling:         every 1mS total < 8 Taks. Each Task to be completed in < 30uS
 *****************************************************************************/
void __attribute__((__section__("APP_IMAGE"))) AC_Cycle_Task(void)
{
    unsigned long Temp_ui32;
    unsigned int AC_Gain;
    unsigned int Temp_ui16, Temp_IADC;
    
    Control.AC_Tasks++;
    switch(Control.AC_Tasks)
    {
        case 1:
            if(PRI_DATA.New_AC_Data)
            {    
                //UART1_WriteBuffer(&Pmeter.Vin_Sum2,8);
                //printf("%d\n",Pmeter.Vin_Sum_Tmp);
                Pmeter.Vin_Sum2 = Pmeter.Vin_Sum;
                Pmeter.Iin_Sum2 = Pmeter.Iin_Sum;
                Pmeter.Pin_Sum2 = Pmeter.Pin_Sum;
                Pmeter.Sample_Cnt2 = Pmeter.Sample_Cnt;
                
                Pmeter.Vin_Sum = 0;
                Pmeter.Iin_Sum = 0;
                Pmeter.Pin_Sum = 0;
                Pmeter.Sample_Cnt = 0;
                
                PRI_DATA.New_AC_Data = 0;
                PRI_DATA.Start_AC_Calc = 1; 
            }
            break;
        case 2:
            //Vin RMS ADC Calculation
            if(PRI_DATA.Start_AC_Calc)
            {   //printf("Vin_Sum2=%d\n",Pmeter.Vin_Sum2);
                //UART1_WriteBuffer(&Pmeter.Sample_Cnt2,8);
                Temp_ui32 = Func64Div16to32(Pmeter.Vin_Sum2, Pmeter.Sample_Cnt2);
                PRI_DATA.VRms_ADC = CalcSquareRoot(Temp_ui32); 
               // UART1_WriteBuffer(&PRI_DATA.VRms_ADC,4);
                //printf("Temp_ui32=%d\n",Temp_ui32);
                     
                Pmeter.Vin_Sum2 = 0;
               
            }
            break;
        case 3:
            //Iin RMS ADC Calculation
            if(PRI_DATA.Start_AC_Calc)
            {
                Temp_ui32 = Func64Div16to32(Pmeter.Iin_Sum2, Pmeter.Sample_Cnt2);
                PRI_DATA.IRms_ADC = CalcSquareRoot(Temp_ui32); 
                Pmeter.Iin_Sum2 = 0;
            }
            break;
        case 4:
            //Pin RMS ADC Calculation
            if(PRI_DATA.Start_AC_Calc)
            {
                Temp_ui32 = Func64Div16to32(Pmeter.Pin_Sum2, Pmeter.Sample_Cnt2);
                PRI_DATA.PIN_Rms_ADC = (unsigned int) Temp_ui32;
                Pmeter.Pin_Sum2 = 0;
            }
            break;
        case 5:
            //Vin RMS Calculation in 125mV Resolution
            if(PRI_DATA.Start_AC_Calc)
            {
                if(PRI_DATA.VAC_Range_LL)   //if Low Line
                    AC_Gain = PRI_DATA.VIN_LL_Gain;
                else // High Line
                    AC_Gain = PRI_DATA.VIN_HL_Gain;   
                
                Temp_ui32 = (unsigned long) __builtin_muluu(PRI_DATA.VRms_ADC,AC_Gain);
                PRI_DATA.VRMS_Inst = (unsigned int) ( (Temp_ui32+2048) >> PRI_DATA.VIN_Div);
            }
            break;
        case 6:
            //Iin RMS Calculation in 15.625mA Resolution
            if(PRI_DATA.Start_AC_Calc)
            {
                if(PRI_DATA.VAC_Range_LL)   //if Low Line
                {
                    if(PRI_DATA.IRms_ADC > PRI_DATA.IIN_LL_4A_ADC) //>4A -> 10A Gain
                    {
                        AC_Gain = PRI_DATA.IIN_LL_10A_Gain;
                        Temp_IADC = PRI_DATA.IRms_ADC - PRI_DATA.IIN_LL_4A_ADC;
                        Temp_ui16 = PRI_DATA.IIN_LL_4A_Val;
                    }
                    else if(PRI_DATA.IRms_ADC > PRI_DATA.IIN_LL_1A_ADC) //>1A -> 4A Gain
                    {
                        AC_Gain = PRI_DATA.IIN_LL_4A_Gain;
                        Temp_IADC = PRI_DATA.IRms_ADC - PRI_DATA.IIN_LL_1A_ADC;
                        Temp_ui16 = PRI_DATA.IIN_LL_1A_Val;
                    }
                    else //<1A -> 1A Gain
                    {
                        AC_Gain = PRI_DATA.IIN_LL_1A_Gain;
                        Temp_IADC = PRI_DATA.IRms_ADC;
                        Temp_ui16 = 0;
                    }                    
                }
                else // High Line
                {
                    if(PRI_DATA.IRms_ADC > PRI_DATA.IIN_HL_2A_ADC) //>2A -> 5A Gain
                    {
                        AC_Gain = PRI_DATA.IIN_HL_5A_Gain;
                        Temp_IADC = PRI_DATA.IRms_ADC - PRI_DATA.IIN_HL_2A_ADC;
                        Temp_ui16 = PRI_DATA.IIN_HL_2A_Val;
                    }
                    else if(PRI_DATA.IRms_ADC > PRI_DATA.IIN_HL_0A5_ADC) //>0.5A -> 2A Gain
                    {
                        AC_Gain = PRI_DATA.IIN_HL_2A_Gain;
                        Temp_IADC = PRI_DATA.IRms_ADC - PRI_DATA.IIN_HL_0A5_ADC;
                        Temp_ui16 = PRI_DATA.IIN_HL_0A5_Val;
                    }
                    else //<0.5A -> 0.5A Gain
                    {
                        AC_Gain = PRI_DATA.IIN_HL_0A5_Gain;
                        Temp_IADC = PRI_DATA.IRms_ADC;
                        Temp_ui16 = 0;
                    }                                   
                }
                Temp_ui32 = (unsigned long) __builtin_muluu(Temp_IADC, AC_Gain); 
                PRI_DATA.IRMS_Inst = Temp_ui16 + (unsigned int) ((Temp_ui32+2048) >> PRI_DATA.IIN_Div);
            }
            break;
        case 7:
            //Pin RMS Calculation in 250mW Resolution
            if(PRI_DATA.Start_AC_Calc)
            {
                if(PRI_DATA.VAC_Range_LL)   //if Low Line
                {
                    if(PRI_DATA.PIN_Rms_ADC > PRI_DATA.PIN_LL_4A_ADC) //>4A -> 10A Gain
                    {
                        AC_Gain = PRI_DATA.PIN_LL_10A_Gain;
                        Temp_IADC = PRI_DATA.PIN_Rms_ADC - PRI_DATA.PIN_LL_4A_ADC;
                        Temp_ui16 = PRI_DATA.PIN_LL_4A_Val;
                    }
                    else if(PRI_DATA.PIN_Rms_ADC > PRI_DATA.PIN_LL_1A_ADC) //>1A -> 4A Gain
                    {
                        AC_Gain = PRI_DATA.PIN_LL_4A_Gain;
                        Temp_IADC = PRI_DATA.PIN_Rms_ADC - PRI_DATA.PIN_LL_1A_ADC;
                        Temp_ui16 = PRI_DATA.PIN_LL_1A_Val;
                    }
                    else //<1A -> 1A Gain
                    {
                        AC_Gain = PRI_DATA.PIN_LL_1A_Gain;
                        Temp_IADC = PRI_DATA.PIN_Rms_ADC;
                        Temp_ui16 = 0;
                    }                    
                }
                else // High Line
                {
                    if(PRI_DATA.PIN_Rms_ADC > PRI_DATA.PIN_HL_2A_ADC) //>2A -> 5A Gain
                    {
                        AC_Gain = PRI_DATA.PIN_HL_5A_Gain;
                        Temp_IADC = PRI_DATA.PIN_Rms_ADC - PRI_DATA.PIN_HL_2A_ADC;
                        Temp_ui16 = PRI_DATA.PIN_HL_2A_Val;
                    }
                    else if(PRI_DATA.PIN_Rms_ADC > PRI_DATA.PIN_HL_0A5_ADC) //>0.5A -> 2A Gain
                    {
                        AC_Gain = PRI_DATA.PIN_HL_2A_Gain;
                        Temp_IADC = PRI_DATA.PIN_Rms_ADC - PRI_DATA.PIN_HL_0A5_ADC;
                        Temp_ui16 = PRI_DATA.PIN_HL_0A5_Val;
                    }
                    else //<0.5A -> 0.5A Gain
                    {
                        AC_Gain = PRI_DATA.PIN_HL_0A5_Gain;
                        Temp_IADC = PRI_DATA.PIN_Rms_ADC;
                        Temp_ui16 = 0;
                    }                                   
                }
                Temp_ui32 = (unsigned long) __builtin_muluu(Temp_IADC, AC_Gain); 
                PRI_DATA.PIN_Inst = Temp_ui16 + (unsigned int) ((Temp_ui32+512) >> PRI_DATA.PIN_Div);
            }
            break;
        case 8:
            if(PRI_DATA.Start_AC_Calc)
            {
                Pmeter.VRms_Sum += PRI_DATA.VRMS_Inst;
                Pmeter.IRms_Sum += PRI_DATA.IRMS_Inst;
                Pmeter.PRms_Sum += PRI_DATA.PIN_Inst;
                Pmeter.VRms_ADC_Sum+=PRI_DATA.VRms_ADC;
                Pmeter.IRms_ADC_Sum+=PRI_DATA.IRms_ADC;
                Pmeter.PRms_ADC_Sum+=PRI_DATA.PIN_Rms_ADC;
                Pmeter.Rms_Avg_Count++;
                if(Pmeter.Rms_Avg_Count > 31)       //32 Samples
                {
                    PRI_DATA.VRMS_Avg = (unsigned int) (Pmeter.VRms_Sum >>5); //Divide by 5
                    PRI_DATA.IRMS_Avg = (unsigned int) (Pmeter.IRms_Sum >>5); //Divide by 5
                    PRI_DATA.PIN_Avg = (unsigned int) (Pmeter.PRms_Sum >>5); //Divide by 5
                    PRI_DATA.VRMS_ADC_Avg = (unsigned int) (Pmeter.VRms_ADC_Sum >>5); //Divide by 5
                    PRI_DATA.IRMS_ADC_Avg = (unsigned int) (Pmeter.IRms_ADC_Sum >>5); //Divide by 5
                    PRI_DATA.PIN_ADC_Avg = (unsigned int) (Pmeter.PRms_ADC_Sum >>5); //Divide by 5
                    Pmeter.VRms_Sum   = 0;
                    Pmeter.IRms_Sum = 0;
                    Pmeter.PRms_Sum = 0;
                    Pmeter.VRms_ADC_Sum = 0;
                    Pmeter.IRms_ADC_Sum = 0;
                    Pmeter.PRms_ADC_Sum = 0;
                    Pmeter.Rms_Avg_Count = 0;                    
	 
                    //Calculate Drop Out Time
                    if(PRI_DATA.PIN_Avg < DROPOUT_MAXLOAD)
                    {
                        Temp_ui16 = (unsigned long) DROPOUT_MAXLOAD - PRI_DATA.PIN_Avg;
                        Temp_ui32 =  __builtin_muluu(Temp_ui16, ADD_DO_GAIN); 
                        Temp_ui16 = (unsigned int) (Temp_ui32 >> 12);
                        Dropout_Delay = DROPOUT_TRIG_DELAY_FL + Temp_ui16;
                    }
                    else
                        Dropout_Delay = DROPOUT_TRIG_DELAY_FL;
                }
                PRI_DATA.Start_AC_Calc = 0;
            }
            Control.AC_Tasks = 0; 
            break;
        default:
            Control.AC_Tasks = 0; 
            break;
    }
}
#if 0
/*******************************************************************************
 * Function:        Cal Data Write Sequence
 * Description:     Tasks Related to Writing CAl DAta to Flash
 * Calling:         every 10mS
 *****************************************************************************/
void __attribute__((__section__("APP_IMAGE"))) Cal_Data_Write(void)
{
    unsigned char i,j;
    static unsigned char Flash_Dly = 0;
    
    switch(Eep_Cntl_State)
    {
        case 1://Turn OFF PFC   
            PRI_DATA.PFC_GO = 0;
            if(!PRI_DATA.PFC_En)
                Eep_Cntl_State = 2;
            break;
        case 2:   
            //Verify Cal DAta CRC            
            Validate_Cal_CRC();
            if(PRI_DATA.Cal_CRC_Err)    //if CRC Not Matches then Load Default Values                
            {
                Eep_Cntl_State = 0;
                PRI_DATA.Eep_Wr_Sta = 0;
            }
            else    
            {
                Eep_Cntl_State = 3;
                _SWDTEN = 0; // Disable watch dog                   
            }
            break;
        case 3:   
            //Erase Flash            
            if(Flash_Dly == 0)
                Flash_Erase_Page(CALIBRATION_DATA_ADDR);
            Flash_Dly++;
            if(Flash_Dly>2)
            {
                Eep_Cntl_State = 4;
                Flash_Dly = 0;
            }
            break;
        case 4: //Write Calibration Data to Flash   
            j=0;
            for(i=0; i<48; i++)
            {
                FLASH_WR_DATA.i16[j++] = Cal_Data_Buf.i16[i];   
                FLASH_WR_DATA.i16[j++] = 0;   
            }
            
            for(i=96; i<128; i++)
                FLASH_WR_DATA.i16[i] = 0;
            
            //Start Write
            Flash_WR_Row(CALIBRATION_DATA_ADDR);
            Eep_Cntl_State = 5;           
            break;
        case 5: //Wait for write to Complete   
            Flash_Dly++;
            if(Flash_Dly>19)    // Wait for 300mS
            {
                //then reset                
                __asm__ volatile ("RESET"); 
            }  
            break;
        default:
            Eep_Cntl_State = 0;
            PRI_DATA.Eep_Wr_Sta = 0;
            break;                    
    }  
}
#endif 
/*******************************************************************************
 * Function:        Task_200uS
 * Description:     200uS/1mS/10mS Taskes called from T1 interrupt
 * Calling:         every 200us
 *****************************************************************************/

void __attribute__((__section__("APP_IMAGE"))) Task_200uS()
{    
    unsigned long Temp_ui32;
    
    //200uS Tasks
    
    //Debug Data's    
        
    //Input Fail Measurement
    Input_Measurement();
  
    //Bulk Voltage Fail Measurement
    Bulk_Measurement();
    
    //Latch the ALARM Bits for Debugging    
    PRI_DATA.ALA_LATCH = PRI_DATA.ALA_LATCH | PRI_DATA.ALA_STAT;
	//Thermistor measurement
	ADC_Arr[1] += PFC_FET_A_TEMP;	
	ADC_Arr[2] += PFC_FET_B_TEMP; 
	ADC_Arr[3] += BOOSTDIODE_TEMP;
	T_ADC_Count++;
	
	if(T_ADC_Count > 7)
	{
		PRI_DATA.T_PFCFET_A = (unsigned int)( (ADC_Arr[1]+4) >> 3 ); 
		PRI_DATA.T_PFCFET_B = (unsigned int)( (ADC_Arr[2]+4) >> 3 );  
		PRI_DATA.T_BOOSTDIODE = (unsigned int)( (ADC_Arr[3]+4) >> 3 );  
		ADC_Arr[1] = 0;
	   	ADC_Arr[2] = 0;
		ADC_Arr[3] = 0;
		T_ADC_Count = 0;
	}
	
	//end of Thermistor measurement
	
    Control.Tasks_1mS++;
    switch(Control.Tasks_1mS)   //1mS Tasks
    {
        case 1:
            //Input Voltage Judgement 
            Input_Judgement();         
            //Relay Control
            Relay_Control();   
            break;

        case 2:            
            //PFC Control
            PFC_Control();          
            //Bulk Voltage Calculation and Thermistor ADC Averaging
            ADC_Arr[0] += Vbus;
          
            ADC_Acc_Count++;
            if(ADC_Acc_Count > 31)            
            {               
                PRI_DATA.BULK_ADC_AVG = (unsigned int)( (ADC_Arr[0]+16) >> 5 );                
                Temp_ui32 = (unsigned long) __builtin_muluu(PRI_DATA.BULK_ADC_AVG, PRI_DATA.BULK_GAIN); 
                PRI_DATA.DC_BULK = (unsigned int) ((Temp_ui32 + 2048) >> PRI_DATA.BULK_DIV);
                
                ADC_Arr[0] = 0;
                ADC_Acc_Count = 0;                 
            }
            break;
        case 3:
            //UART Tx Processing to Secondary

            break;
        case 4:
            //AC Cycle Tasks
            AC_Cycle_Task();
            break;
        case 5:
            Control.Tasks_1mS = 0;
            Control.Tasks_10mS++;
            switch(Control.Tasks_10mS)  //10mS Tasks
            {
                case 1:
                    //Process Receiver Data once Every 10mS
                    break;
                case 2:
                    //PFC Bus Reference Voltage Control 
                    Bus_Control();                   
                    break;
                case 3:
                    //Update Power Limit Factor
                  	PowerLimit_Factor();
                    break;
                case 4:

                    
                    break;

                case 5:
/*                    //Feed forward Boost Changeover
                    if(!PRI_DATA.FF_Boost_1) //Power is < 190W Check for High Range
                    {
                        if(PRI_DATA.PIN_Avg > FF_BOOST_PWR_RANGE)   //>210W
                        {
                            FF_Boost_Count1++;
                            if(FF_Boost_Count1 > 49)    //50*10mS 500mS
                            {
                                PRI_DATA.FF_Boost_1 = 1;
                                FF_Boost_Count1 = 0;
                            }
                        }
                        else
                            FF_Boost_Count1 = 0;
                    }
                    else //Power is > 210W Check for Low Range
                    {
                        if(PRI_DATA.PIN_Avg < FF_BOOST_PWR_REC_RANGE)   //<190W
                        {
                            FF_Boost_Count1++;
                            if(FF_Boost_Count1 > 49)    //50*10mS 500mS
                            {
                                PRI_DATA.FF_Boost_1 = 0;
                                FF_Boost_Count1 = 0;
                            }
                        }
                        else
                            FF_Boost_Count1 = 0;
                    }
*/
                    break;
                case 6:
                    break;
                case 7:
                    break;
                case 8:
                    break;
                case 9:
                    break;
                case 10:
                    Control.Tasks_10mS = 0;                       
                    Control.Tasks_100mS++;
                    switch(Control.Tasks_100mS) //100mS Tasks
                    {
                        case 1:
							Avcount++;
							if(Avcount > 10)
							{
								Av++;
								if(Av  > 0xFE)
								{
									Av = 0;
								}
								Avcount = 0;
							}
                            break;
                        case 2:
                            break;
                        case 3:
                            break;
                        case 4:
                            break;
                        case 5:
                            break;
                        case 6:
                            break;
                        case 7:
                            break;
                        case 8:
                            break;
                        case 9:
                            break;
                        case 10:
                            Control.Tasks_100mS = 0;   
                            break;
                        default:
                            Control.Tasks_100mS = 0;           
                            break;
                    }
                    break;
                default:
                    Control.Tasks_10mS = 0;           
                    break;
            }
            break;
		// case 6:	printf("%d\n",4);
			// break;
			
        default:
            Control.Tasks_1mS = 0;           
            break;
    }           
   
   // ClrWdt(); // clear watch dog counter  
    return;
}

/*****************************************************************************/

/* End of file */
