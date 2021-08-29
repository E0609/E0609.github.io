/*****************************************

FILE: Util.h
COMPANY: Liteon Technology, Inc.
PROCESSOR: MC9S08QG4
PURPOSE: Utility functions
HISTORY:

DATE        REVISION    DESCRIPTION
-----------------------------------
2008/12/09  v0.1        Created


*****************************************/

/*************************************
    Marco Function    
*************************************/
#define ABS(a) ((a) < 0 ? -(a) : (a))

#define VALUE_TO_LINEAR(X,N,GAIN)  ((N >= 0) ?  (((X >> N) & 0x7FF) | (N << 11)) : \
				  (((X << ABS(N)) / GAIN) & 0x7FF) | (0x8000 | ((16+N) << 11)))
				  
#define LINEAR_TO_VALUE(Y,GAIN)  (Y & 0x8000) ?  (((long)((Y & 0x7FF) * GAIN)) / (1 << (16-((Y >> 11) & 0x0F)))) : \
				  ((Y & 0x7FF) * (1 << ((Y >> 11) & 0x0F)))

#define NOT_RESET	0
#define RESET		1

/*************************************
    External Function    
*************************************/			  				  
void Get_Vac(word Vac, word* Vpeak, word* VpeakAvg, byte* Frequency, byte bReset);
//word isqrt(dword val);  


