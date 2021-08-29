/*****************************************

FILE: SCI.h
COMPANY: Liteon Technology, Inc.
PROCESSOR: MC9S08QG4
PURPOSE: SCI implementation
HISTORY:

DATE        REVISION    DESCRIPTION
-----------------------------------
2008/12/09  v0.1        Created


*****************************************/

#define UART_FROM_SEC_HEADER_INFO			0xAA
#define UART_TO_SEC_HEADER_INFO				0xAA

typedef enum
{
	STATE_UART_FROM_SEC_HEADER,
	STATE_UART_FROM_SEC_STATUS,
	STATE_UART_FROM_SEC_CMD,
	STATE_UART_FROM_SEC_DATA_LB,
	STATE_UART_FROM_SEC_DATA_HB,
	STATE_UART_FROM_SEC_CHECKSUM,

	STATE_UART_FROM_SEC_TOTAL
	
}STATE_UART_FROM_SECONDARY;

typedef enum
{
	STATE_UART_TO_SEC_HEADER,
	STATE_UART_TO_SEC_STATUS,
	STATE_UART_TO_SEC_CMD,
	STATE_UART_TO_SEC_DATA_LB,
	STATE_UART_TO_SEC_DATA_HB,
	STATE_UART_TO_SEC_CHECKSUM,

	STATE_UART_TO_SEC_TOTAL
	
}STATE_UART_TO_SECONDARY;

typedef struct _UART_FROM_SEC_RECVDATA
{
	byte Status;
	byte Data_LB;
	byte Data_HB;
	byte Cmd;
}
UART_FROM_SEC_RECVDATA;

typedef enum
{
	SY_CMD_IOUT,

	SY_CMD_TOTAL

}UART_SY_CMD;

typedef enum
{
	PY_CMD_TEMPERATURE = 0,
	PY_CMD_FW_REV,
	PY_CMD_VAC,
	PY_CMD_VDC,
	PY_CMD_FW_REV_INTERNAL,
	PY_CMD_IAC,

	PY_CMD_TOTAL

}UART_PY_CMD;


/*************************************
    External Variable
*************************************/
extern volatile word gSyData[SY_CMD_TOTAL];

/*************************************
    External Function
*************************************/
void SCI_Transmit(void);

unsigned int GetIinData(unsigned int Vpeak, word RMS_Value);
unsigned int GetPinData(unsigned int Vin, unsigned int Iin);
unsigned int GetVinData(unsigned int VPeak);

void Set_SCIData(unsigned int Vin, unsigned int Iin, unsigned int Pin, byte Temperature1, byte Temperature2, word IinADC);
void Set_SCICheckSum(void);

void SetFWVersion(void);
void UartDataRefresh(void);

