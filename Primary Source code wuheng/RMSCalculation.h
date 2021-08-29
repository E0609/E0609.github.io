/*******************************************************************************
* file				RMSCalculation.h
* brief				The file is the header file of RMSCalculation.c.
* note
* author			vincent.liu
* version			01
* section History	2016/12/02 - 1st release
*******************************************************************************/

/*******************************************************************************
* declare compile condition
*******************************************************************************/
#define VAC_INS_CONVERT(v)      (v*222L)/100	    // (8.2/(374*3+8.2))/3.3*1024 = 2.25, fine tune is 2.22

#define ACRMS_IN_OVF				  VAC_INS_CONVERT(295)  //[davidchchen]20161219 Added RMS setting.
#define ACRMS_IN_OVW				  VAC_INS_CONVERT(290)  //[davidchchen]20161219 Added RMS setting.
#define ACRMS_RECOVER_REF			VAC_INS_CONVERT(264)  //[davidchchen]20161219 Added RMS setting.
#define ACGDRMS_REF           VAC_INS_CONVERT(174)  //[davidchchen]20161219 Added RMS setting.
#define ACNGRMS_REF           VAC_INS_CONVERT(167)  //[davidchchen]20161219 Added RMS setting.
#define ACSAGRMS_REF          VAC_INS_CONVERT(157)  //[davidchchen]20161219 Added RMS setting.
#define ACSDRMS_REF				    VAC_INS_CONVERT(150)  //[davidchchen]20161219 Added RMS setting.

#define RMS_OFFSET              VAC_INS_CONVERT(50)

#define SAMPLE_START_DLY        1
#define SAMPLE_STOP_DLY         1
#define SAMPLE_CYCLE_CNT        2                   // 2 half cycle
#define MAX_RMS_CNT             6024                // 1Hz
/*******************************************************************************
* declare compile structure
*******************************************************************************/
typedef union 
{
    struct 
    {
        byte u1SampleStart  :1;
        byte u1SampleStop   :1;
        byte u1SampleDone   :1;
        byte u1NA01         :1;
        byte u1NotACSignal  :1;
        byte u1RMSCalPass   :1;
        byte u2NA02         :2;
    } u8Bit;

    byte u8All;
}nRMSCalFlag_t;

typedef struct 
{
    byte u8SampleStartCnt;
    byte u8SampleStopCnt;
    byte u8SampleDoneCnt;
    
    word u16AccCount;
    word u16TotalCount;
    word u16PreData;
    word u16VolValue;

    dword u32VolAccValue;
    dword u32VolTotalValue;

    nRMSCalFlag_t nRMSCalFlag;
   
} sRMSCalStr_t;
/*******************************************************************************
* declare extern variable
*******************************************************************************/
extern sRMSCalStr_t sRMSCal;
/*******************************************************************************
* declare extern function
*******************************************************************************/
extern void ADC_AccSquare(word u16Data);
extern void ADC_CalRMS(void);
extern byte SquareRoot(word x);
/*******************************************************************************
* end of file
*******************************************************************************/
