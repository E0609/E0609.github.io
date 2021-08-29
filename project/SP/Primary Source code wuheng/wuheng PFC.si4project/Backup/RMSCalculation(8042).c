/*******************************************************************************
* file				RMSCalculation.c
* brief				Calculate the voltage RMS value of input.
* note
* author			vincent.liu
* version			01
* section History	2016/12/02 - 1st release
*******************************************************************************/
#include <MC9S08QG4.h>
#include "RMSCalculation.h"
#include "Config.h"
/*******************************************************************************
* declare variable
*******************************************************************************/
//=====================================
// RMS calaulate data structure
//=====================================
sRMSCalStr_t sRMSCal;
//=====================================
// square root table
//=====================================
const byte sqq_table[] = 
{
    0,   16,  22, 27, 32, 35, 39, 42, 45, 48, 50, 53, 55, 57,                
    59,  61,  64, 65, 67, 69, 71, 73, 75, 76, 78, 80, 81, 83,               
    84,  86,  87, 89, 90, 91, 93, 94, 96, 97, 98, 99, 101, 102,
    103, 104, 106, 107, 108, 109, 110, 112, 113, 114, 115, 116, 117, 118,
    119, 120, 121, 122, 123, 124, 125, 126, 128, 128, 129, 130, 131, 132,
    133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 144, 145,
    146, 147, 148, 149, 150, 150, 151, 152, 153, 154, 155, 155, 156, 157,
    158, 159, 160, 160, 161, 162, 163, 163, 164, 165, 166, 167, 167, 168,
    169, 170, 170, 171, 172, 173, 173, 174, 175, 176, 176, 177, 178, 178,
    179, 180, 181, 181, 182, 183, 183, 184, 185, 185, 186, 187, 187, 188,
    189, 189, 190, 191, 192, 192, 193, 193, 194, 195, 195, 196, 197, 197,
    198, 199, 199, 200, 201, 201, 202, 203, 203, 204, 204, 205, 206, 206,
    207, 208, 208, 209, 209, 210, 211, 211, 212, 212, 213, 214, 214, 215,
    215, 216, 217, 217, 218, 218, 219, 219, 220, 221, 221, 222, 222, 223,
    224, 224, 225, 225, 226, 226, 227, 227, 228, 229, 229, 230, 230, 231,
    231, 232, 232, 233, 234, 234, 235, 235, 236, 236, 237, 237, 238, 238,
    239, 240, 240, 241, 241, 242, 242, 243, 243, 244, 244, 245, 245, 246,
    246, 247, 247, 248, 248, 249, 249, 250, 250, 251, 251, 252, 252, 253,
    253, 254, 254, 255
};
/*******************************************************************************
* brief   square root calculator 
* para1:  the number to be calculated
* para2:  none
* return: square root value
*******************************************************************************/
/*
static word SquareRoot(dword x) 
{
    byte i;
    dword rem = 0;
    dword root = 0;
    
    for (i = 0; i < 16; i++)
    {
        root <<= 1;
        rem = (rem << 2) | (x >> 30);
        x <<= 2;
        if(root < rem)
        {
            root++;
            rem -= root;
            root++;
        }
    }
    
    return (word)(root >> 1);
}
*/
byte SquareRoot(word x) 
{
   word xn;
   xn = 0;
  
    if (x >= 0x100) 
    {
        if (x >= 0x1000)
           if (x >= 0x4000)
               xn = (sqq_table[x>>8] >> 0) + 1;
           else 
               xn = (sqq_table[x>>6] >> 1) + 1;
           
        else
           if (x >= 0x400)
               xn = (sqq_table[x>>4] >> 2) + 1;
           else
               xn = (sqq_table[x>>2] >> 3) + 1;
           
        //goto adj;
    } 
    else 
    {
        return sqq_table[x] >> 4;
    }

    /* Run two iterations of the standard convergence formula */
//   xn = (xn + 1 + x / xn) / 2;
//nr1:
//    xn = (xn + 1 + x / xn) / 2;
//adj:
    if (xn * xn > x) /* Correct rounding if necessary */
        xn-- ;
   
   return ((byte)xn); 
}
/*******************************************************************************
* brief   calculate the square value sum of each data 
* para1:  adc data
* para2:  none
* return: none
*******************************************************************************/
void ADC_AccSquare(word u16Data) 
{
    word u16DataTemp;
    //dword u32DataTemp;
        
    // find the cycle of input signal   
    if (u16Data > RMS_OFFSET) 
    {            
        if(sRMSCal.nRMSCalFlag.u8Bit.u1SampleStart == 0) 
        {               
            sRMSCal.u8SampleStartCnt++;
            if(sRMSCal.u8SampleStartCnt >= SAMPLE_START_DLY) 
            {                                    
       lrms:    sRMSCal.nRMSCalFlag.u8Bit.u1SampleStart = 1;
                sRMSCal.u8SampleStopCnt = 0;
                
                if(sRMSCal.nRMSCalFlag.u8Bit.u1SampleStop == 1) 
                {
                    sRMSCal.nRMSCalFlag.u8Bit.u1SampleStop = 0;
                   
                    if(sRMSCal.u8SampleDoneCnt > 0) sRMSCal.u8SampleDoneCnt--;
                    if(sRMSCal.u8SampleDoneCnt == 0) 
                    {
                        if(sRMSCal.nRMSCalFlag.u8Bit.u1SampleDone == 0) 
                        {
                            sRMSCal.nRMSCalFlag.u8Bit.u1SampleDone = 1;
                             
                            sRMSCal.u16TotalCount = sRMSCal.u16AccCount;
                            sRMSCal.u32VolTotalValue = sRMSCal.u32VolAccValue;                     
                        }
                           
                        sRMSCal.u16AccCount = 0;
                        sRMSCal.u32VolAccValue = 0;                                                
                        sRMSCal.u8SampleDoneCnt = SAMPLE_CYCLE_CNT; 
                   }
                }
            }
        }
    }
    else 
    {
        if(sRMSCal.nRMSCalFlag.u8Bit.u1SampleStart == 1) 
        {
             sRMSCal.u8SampleStopCnt++;         
             if(sRMSCal.u8SampleStopCnt >= SAMPLE_STOP_DLY) 
             {
                sRMSCal.nRMSCalFlag.u8Bit.u1SampleStart = 0;
                sRMSCal.nRMSCalFlag.u8Bit.u1SampleStop = 1;
                sRMSCal.u8SampleStartCnt = 0;
             }
        }
    }
   
    // check whether the input signal is DC or not
    if((sRMSCal.nRMSCalFlag.u8Bit.u1SampleStop == 0) && (sRMSCal.u16AccCount >= MAX_RMS_CNT))
    {       
        sRMSCal.nRMSCalFlag.u8Bit.u1SampleStop = 1;    
        sRMSCal.nRMSCalFlag.u8Bit.u1NotACSignal = 1;         
        goto  lrms;
    }              
    
    // acc the square value of each data
    u16DataTemp = (word)(u16Data >> 2);
    u16DataTemp = u16DataTemp * u16DataTemp;
    sRMSCal.u32VolAccValue = sRMSCal.u32VolAccValue + (dword)u16DataTemp;
    
    // acc the value of each data
    //u32DataTemp = (dword)u16Data;
    //sRMSCal.u32VolAccValue = sRMSCal.u32VolAccValue + u32DataTemp; 
      
    sRMSCal.u16AccCount++;
}
/*******************************************************************************
* brief   calculate the rms value of n half cycle data 
* para1:  none
* para2:  none
* return: none
*******************************************************************************/
void ADC_CalRMS(void) 
{
    word u16DataTemp;
    //static word u16PreData = 0;
    //dword u32DataTemp;
     
    if(sRMSCal.nRMSCalFlag.u8Bit.u1SampleDone == 0) return;
    sRMSCal.nRMSCalFlag.u8Bit.u1SampleDone = 0;
   
    if(sRMSCal.nRMSCalFlag.u8Bit.u1NotACSignal == 1) 
    {
        sRMSCal.nRMSCalFlag.u8Bit.u1NotACSignal = 0;
        sRMSCal.nRMSCalFlag.u8Bit.u1RMSCalPass = 0;
        sRMSCal.u16VolValue =0;   //[davidchchen]20161219 Added       
    }
    else 
    {
        sRMSCal.nRMSCalFlag.u8Bit.u1RMSCalPass = 1;  
        
        // calculate rms
        u16DataTemp = (word)(sRMSCal.u32VolTotalValue / sRMSCal.u16TotalCount);
        u16DataTemp = SquareRoot(u16DataTemp);
        
        /*
        if(sRMSCal.u16PreData == 0) 
        {
            sRMSCal.u16VolValue = u16DataTemp << 2;    
        } 
        else 
        {
            sRMSCal.u16VolValue = (sRMSCal.u16PreData + u16DataTemp) << 1;    
        }
        
        sRMSCal.u16PreData = u16DataTemp;
        */
        
        sRMSCal.u16VolValue = u16DataTemp << 2; 
        
        /*
        u16DataTemp = (word)(sRMSCal.u32VolTotalValue / sRMSCal.u16TotalCount);
        sRMSCal.u16VolValue = SquareRoot(u16DataTemp);
        sRMSCal.u16VolValue = sRMSCal.u16VolValue << 2;
        */
        
        // calculate mean
        //u32DataTemp = (dword)(sRMSCal.u32VolTotalValue * 1137 / sRMSCal.u16TotalCount) >> 10;
        //sRMSCal.u16VolValue = (word)u32DataTemp;     
    }  
}
/*******************************************************************************
* end of file
*******************************************************************************/
