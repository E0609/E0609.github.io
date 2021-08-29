/*****************************************

FILE: Config.h
COMPANY: Liteon Technology, Inc.
PROCESSOR: MC9S08QG4
PURPOSE: Project define
HISTORY:

DATE        REVISION    DESCRIPTION
-----------------------------------
2008/12/09  v0.1        Created


*****************************************/


/*====================================
    Program Version definition
====================================*/
//#define FW_VERSION          "90"			//rev. 09
#define FW_VERSION          "E0"			//rev. 0A //[davidchchen]20161201 modify to 0D
#define FW_VERSION_INT      "30"			//rev. 03

/*====================================
    SCI Debug definition
====================================*/
#define SCI_DEUBG 0

/*====================================
    Load definition
====================================*/
#define LOAD_55P	114
#define LOAD_60P	125

/*====================================
    IDE definition
====================================*/
#define CW_IDE 63 //code worrior version
//#define CW_IDE 60 //code worrior version

/*====================================
    PIN definition
====================================*/
#define pin_PFC_CTRL			PTBD_PTBD2
#define PFC_ON				1
#define PFC_OFF				0

#define pin_AC_OK_CTRL			PTBD_PTBD4
#define AC_OK				0
#define AC_NOTOK			1

#define pin_RELAY_CTRL			PTBD_PTBD7
#define RELAY_ON			1
#define RELAY_OFF			0

#define pin_PFC_OK_CTRL			PTBD_PTBD3
#define PFC_OK				0
#define PFC_NOTOK			1

#define pin_PFC_ADJUST_PIN		PTBD_PTBD6
#define PFC_ADJUST_ON		1
#define PFC_ADJUST_OFF		0

#define pin_PFC_STB_CTRL		PTBD_PTBD5
#define PFC_STB_OK			1
#define PFC_STB_NOTOK		0

#define pin_PFC_STB_CTRL			PTAD_PTAD3
#define VRMS_BROWN_IN		0
#define VRMS_BROWN_OUT		1


/*====================================
    ADC Resolution definition
====================================*/
typedef enum
{
    ADC_8_BIT = 0,
    ADC_10_BIT
};

/*====================================
    ADC Channel definition
====================================*/
#define ADC_VDC				0x00
#define ADC_VAC_RECT		0x01
#define ADC_TEMPERATURE1	0x02
#define ADC_VRMS			0x03

#define ADC_INT_EN			0x40

/*====================================
    Converter definition
====================================*/
#define VAC_CONVERT(v)			(v*318L)/100	// (8/(560*2+8))/3.3*1024*sqrt(2) = 3.11
#define VDCIN_CONVERT(v)		(v*225L)/100	// (8/(560*2+8))/3.3*1024 = 2.2
#define VDC_CONVERT(v)			(v*2164L)/1000	// (5.62/200*4+5.62)/3.3*1024 = 2.16467197
#define TIMER_CONVERT(t)		(t*1000L)/166	//ms (166us is the timer trigger point, setting in init.c)

/*====================================
    Reference Value definition
====================================*/
// PFC output and status
#define PFC_OK_REF              VDC_CONVERT(390)
#define PFC_NOTOK_REF_HL        VDC_CONVERT(335)    //[davidchchen]20161201 modify 
#define PFC_NOTOK_REF_LL        VDC_CONVERT(330)
#define PFC_STB_NOTOK_REF       VDC_CONVERT(250)
#define PFC_OV_LATCHED_REF		VDC_CONVERT(450)    //(435) //Chris, spec change, [davidchchen]20150105 modified
#define PFC_OV_LATCHED_REF_H	VDC_CONVERT(460)    //[davidchchen]20150105 modified
#define PFC_OV_ON_REF		    VDC_CONVERT(425)
#define PFC_OV_OFF_REF		    VDC_CONVERT(400)

//#define BROWN_LOW_REF           VAC_CONVERT(169) //For PFC_IC
//#define BROWN_HIGH_REF          VAC_CONVERT(177)
#define BROWN_SL_REF            VAC_CONVERT(169)

#define AC_UVP_RECOVER_REF      VAC_CONVERT(174) //(175)//For AC_OK_PIN, //davidchchen modify 20160425
#define ACNG_REF                VAC_CONVERT(167)
#define AC_UVP_REF              VAC_CONVERT(157)
#define ACSD_REF				VAC_CONVERT(150)
#define AC_OVP_RECOVER_REF 		VAC_CONVERT(264)

#define DC_UVP_RECOVER_REF      VDCIN_CONVERT(184) //(185) //For AC_OK_PIN, //davidchchen modify 20160425
#define DCNG_REF                VDCIN_CONVERT(177)
#define DC_UVP_REF              VDCIN_CONVERT(167)
#define DCSD_REF				VDCIN_CONVERT(160)
#define DC_RECOVER_REF			VDCIN_CONVERT(300)

#define AC_IN_OVF				VAC_CONVERT(295)
#define AC_IN_OVW				VAC_CONVERT(290)

#define DC_IN_OVF				VDCIN_CONVERT(305)
#define DC_IN_OVW				VDCIN_CONVERT(300)

#define SD_REF                  VAC_CONVERT(15) // 60 //For shut down condition
#define LINE_BOUNDARY			VAC_CONVERT(150)
                                           
/*====================================
    Reference Counter definition
====================================*/
#define MCU_START_COUNTER_REF   TIMER_CONVERT(20) //ms

#define PFC_OK_COUNTER_REF      TIMER_CONVERT(50)
#define PFC_NOTOK_COUNTER_REF   TIMER_CONVERT(5) //100 - > 1000, for ME's requirement

//#define AC_OK_COUNTER_REF       TIMER_CONVERT(40)
#define AC_OK_COUNTER_REF       TIMER_CONVERT(200) //[davidchchen]20150310 modify
#define AC_NOTOK_COUNTER_REF    TIMER_CONVERT(10)//(100) //25 ->100   //Chris20130116 for droupout issue 

#define RELAY_ON_COUNTER_REF	TIMER_CONVERT(250)
#define RELAY_OFF_COUNTER_REF	TIMER_CONVERT(260)

#define PFC_ON_COUNTER_REF		TIMER_CONVERT(10)

#define PFC_ADJUST_OFF_REF		TIMER_CONVERT(1000)

#define PFC_ADJUST_ON_DELAY_REF	TIMER_CONVERT(20)

#define PFC_OV_LATCHED_COUNTER_REF      TIMER_CONVERT(120)	// 16 ////[davidchchen]20150610 modify

#define PFC_OV_H_LATCHED_COUNTER_REF    TIMER_CONVERT(2)
	
#define PFC_OV_ON_COUNTER_REF			TIMER_CONVERT(5)

#define PFC_OV_OFF_COUNTER_REF			TIMER_CONVERT(5)


#define BROWN_OUT_COUNTER_REF   TIMER_CONVERT(30) //Extend the timing to slove -40 shut down issue

//#define AC_OFF_COUNTER_REF      TIMER_CONVERT(10)
//#define AC_WINDOW_COUNTER_REF   TIMER_CONVERT(11) //must >= 10.6 ms

#define SCI_COUNTER_REF         TIMER_CONVERT(10)

#define SD_COUNTER_REF          TIMER_CONVERT(3)
#define PFC_DROP_COUNTER_REF    TIMER_CONVERT(1)    //For shut down condition

#define OTP_SD_COUNTER_REF		TIMER_CONVERT(10000)
#define OTP_RECOVER_COUNTER_REF	TIMER_CONVERT(10000)

#define VIN_OV_REF				TIMER_CONVERT(72)
#define SAG_TIME_REF			TIMER_CONVERT(500)

typedef struct _ADC_FILTER
{
	word AD;
	word FF;
	word LPF;
	byte exp;
}tADC;

typedef struct _ADC_DATA
{
    tADC Vac;
    tADC Vdc;
    tADC Temperature;
    //tADC Iac;
    word Vpeak;
	word Vpeak_Avg;
	//tADC Vrms;
}ADCData;

typedef union _SECONDARY_STATUS
{
	byte Val;
    struct
    {
        byte STB:1;        
        byte PFCDISABLE:1;
        byte SLEEP:1;
        byte b3:1;
        byte b4:1;
        byte b5:1;
        byte b6:1;
        byte b7:1;
    }bit;
} SECONDARY_STATUS;

typedef union _PRIMARY_STATUS
{
	byte Val;
    struct
    {
        byte PFCDISABLE:1;
        byte PFC_OV_LATCHED:1;
        byte INPUT_OVP:1;
        byte DC_INPUT:1;
        byte PRI_OTP:1;
        byte INPUT_OVW:1;
        byte PriCommError:1;     //[davidchchen]20160503 added primary and secondary side communication error 
        byte b7:1;
    }bits;
} PRIMARY_STATUS;

typedef struct _PSU
{
	byte State;	
	byte Frequency;
	ADCData ADC;
	word PFCNOK_Ref;
	byte Vrms_BrownIn;
	
	/*Secondary status*/
	SECONDARY_STATUS	SyStatus;

	/*Primary status*/
	PRIMARY_STATUS		PyStatus;

	/*Debug Info*/
	//byte DebugInfo;
	
}PSU;


