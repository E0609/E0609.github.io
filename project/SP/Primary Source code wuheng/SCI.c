/*****************************************

FILE: SCI.c
COMPANY: Liteon Technology, Inc.
PROCESSOR: MC9S08QG4
PURPOSE: SCI implementation
HISTORY:

DATE        REVISION    DESCRIPTION
-----------------------------------
2008/12/12  v0.1        Created


==========================================
Code Convention:

1. Prefix: "g_" represents global variabl
2. Tab width: 4
*****************************************/
#include <MC9S08QG4.h>
#include <string.h>
#include "Config.h"
#include "SCI.h"
#include "Util.h"
#include "RMSCalculation.h"

/*************************************
    Global variable
*************************************/
//#pragma DATA_SEG MY_ZEROPAGE //data will be allocated in Z_RAM                        

#if SCI_DEUBG
#define PY_DEBUG_BYTES	4
static volatile byte gPyDebugData[PY_DEBUG_BYTES] = {0};
#endif

volatile word gSyData[SY_CMD_TOTAL] = {0};
volatile word gPyData[PY_CMD_TOTAL] = {0};


/*************************************
    Extern Variable
*************************************/
extern volatile PSU gPSU;

/*************************************
    Implementation Begin
*************************************/
interrupt 15 void ISR_SCI_Receive(void)
{
	static byte RXState = STATE_UART_FROM_SEC_HEADER;
	static UART_FROM_SEC_RECVDATA RecvData = {0};	
	static byte CheckSum = 0;

	byte RecvByte;

	if(SCIS1_RDRF == 1)
	{

		RecvByte = SCID;
		gPSU.PyStatus.bits.PriCommError = 0;    //[davidchchen]20160503 added primary and secondary side communication error

		switch (RXState)
		{
			case STATE_UART_FROM_SEC_HEADER:
			{			
				if(RecvByte == UART_FROM_SEC_HEADER_INFO)			
					RXState = STATE_UART_FROM_SEC_STATUS;			
				else
					RXState = STATE_UART_FROM_SEC_HEADER;

				/*Reset values*/
				CheckSum = 0;			
			}
			break;
			
			case STATE_UART_FROM_SEC_STATUS:
			{
				RXState = STATE_UART_FROM_SEC_CMD;
				RecvData.Status = RecvByte;
			}
			break;
			
			case STATE_UART_FROM_SEC_CMD:
			{
				RXState = STATE_UART_FROM_SEC_DATA_LB;
				RecvData.Cmd = RecvByte;
			}
			break;

			case STATE_UART_FROM_SEC_DATA_LB:
			{
				RXState = STATE_UART_FROM_SEC_DATA_HB;
				RecvData.Data_LB = RecvByte;
			}
			break;

			case STATE_UART_FROM_SEC_DATA_HB:
			{
				RXState = STATE_UART_FROM_SEC_CHECKSUM;
				RecvData.Data_HB = RecvByte;
			}
			break;

			case STATE_UART_FROM_SEC_CHECKSUM:
			{
				RXState = STATE_UART_FROM_SEC_HEADER;
				CheckSum = (~CheckSum) + 1; // 2's complement

				/*Correct checksum*/
				if(CheckSum == RecvByte)
				{
					/*Update secondary data*/
					if(RecvData.Cmd < SY_CMD_TOTAL){
						
						//UpdateSyData(RecvData.Cmd, RecvData.Data.Val);
						gSyData[RecvData.Cmd] = ((word)RecvData.Data_HB << 8) | RecvData.Data_LB;
						//gPSU.PyStatus.bits.PriCommError = 0;    //[davidchchen]20160503 added primary and secondary side communication error
					}

					gPSU.SyStatus.Val = RecvData.Status;
				}
			}
			break;
		}

		CheckSum += RecvByte;
	}
}

/*====================================
Transmit the SCI data
====================================*/
void UpdatePyData(byte Addr, word Data)
{
	gPyData[Addr] = Data;
}
	
void UartDataRefresh(void)
{
	/*Primary information for secondary*/
	UpdatePyData(PY_CMD_TEMPERATURE, gPSU.ADC.Temperature.FF);
	//UpdatePyData(PY_CMD_VAC, gPSU.ADC.Vpeak);
	UpdatePyData(PY_CMD_VAC, sRMSCal.u16VolValue);
	//UpdatePyData(PY_CMD_VDC, gPSU.ADC.Vdc.FF);
	UpdatePyData(PY_CMD_VDC, gPSU.ADC.Vpeak_Avg);
	//UpdatePyData(PY_CMD_IAC, gPSU.ADC.Iac.AD);
}

#if SCI_DEUBG
void UpdatePyDebugData(void)
{
	word temp;

	temp = gPSU.ADC.Vpeak;
	gPyDebugData[0] = 0xAA;
	gPyDebugData[1] = 0x11;
	gPyDebugData[2] = 0x55;
	gPyDebugData[3] = 0xCC;
}

void SCI_Transmit(void)
{
	byte TxData = 0;
	static byte index = 0;                    
	
	
	if (!SCIS1_TC) return; //return if transmit not complete

	SCIS1_TDRE = 1; //set transmit data register empty	

	TxData = gPyDebugData[index];

	SCID = TxData;		

	index++;
	
	if(index >= PY_DEBUG_BYTES){
		index = 0;
		UpdatePyDebugData();
	}
}

#else
void SCI_Transmit(void)
{
	static byte TXState = STATE_UART_TO_SEC_HEADER;
	static byte Cmd = PY_CMD_TEMPERATURE;
	static byte CheckSum = 0;
	static word SendVal;
	byte TxData = 0;

  gPSU.PyStatus.bits.PriCommError = 1;    //[davidchchen]20160503 added primary and secondary side communication error
  
	if (!SCIS1_TC) return; //return if transmit not complete

	SCIS1_TDRE = 1; //set transmit data register empty	

	switch(TXState)
	{
		case STATE_UART_TO_SEC_HEADER:

			TXState = STATE_UART_TO_SEC_STATUS;			
			TxData = UART_TO_SEC_HEADER_INFO;			
			CheckSum = 0; /*Reset checksum*/
			break;
			
		case STATE_UART_TO_SEC_STATUS:
						
			TXState = STATE_UART_TO_SEC_CMD;
			TxData = gPSU.PyStatus.Val;		
			break;			
			
		case STATE_UART_TO_SEC_CMD:
			
			TXState = STATE_UART_TO_SEC_DATA_LB;			
			TxData = Cmd;			
			break;
		
		case STATE_UART_TO_SEC_DATA_LB:

			TXState = STATE_UART_TO_SEC_DATA_HB;			
			SendVal = gPyData[Cmd];
			TxData = SendVal & 0xFF;

			break;
			
		case STATE_UART_TO_SEC_DATA_HB:
			
			TXState = STATE_UART_TO_SEC_CHECKSUM;
			TxData = (SendVal >> 8) & 0xFF;

			break;

		case STATE_UART_TO_SEC_CHECKSUM:

			TXState = STATE_UART_TO_SEC_HEADER;						
			TxData = (byte)(0xFF - CheckSum) + 1;							/*2's complement for the checksum*/

			//Cmd = (Cmd < (PY_CMD_TOTAL-1))? (Cmd + 1) : (0);
			if(Cmd < (PY_CMD_TOTAL-1)){
				Cmd++;
			}else{
				Cmd = 0;
				UartDataRefresh();
			}
			

			break;

		default:
			break;			
	}

	SCID = TxData;
	CheckSum += TxData;

}
#endif

/*====================================
Adjust Pin data with Vin & Iin
====================================*/
//#pragma MESSAGE DISABLE C5904 //C5904: Division by one
//#pragma MESSAGE DISABLE C5905 //C5905: Multiplication with one
//#pragma MESSAGE DISABLE C4003 //C4003: Shift count converted to unsigned char

/*====================================
Set FW version
====================================*/
void SetFWVersion(void)
{
	word FW_ver;
	word FW_ver_int;

	(void)memcpy(&FW_ver, FW_VERSION, 2); 
	(void)memcpy(&FW_ver_int, FW_VERSION_INT, 2); 

	UpdatePyData(PY_CMD_FW_REV, FW_ver);
	UpdatePyData(PY_CMD_FW_REV_INTERNAL, FW_ver_int);	
}


