
#include "p33Fxxxx.h"
#include "Define.h"
#include "Sci.h"
#include "Protection.h"
#include "Process.h"
#include "Fan.h"
#include <string.h>
#include "Pmbus.h"
#include "UserData.h"
#include "Util.h"
#include "Isr.h"
#include "Led.h"

#if Config_78M6610   //[davidchchen]20160114 added
#include "math.h"
#endif

extern tPSFB PSFB;
extern tPS_FLAG PS;

/////////////////////////////////////////////////////////////////////////////

//[Peter Chung] UART Start ->

/*
 *	+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 *	| "D" | "S" | "E" | Vlsb| Vmsb| Ilsb| Imsb| Plsb| Pmsb| Tadc| Vfrq|Chksm|
 *	+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */

#define TERIDIAN_CALI_RETRY		TRUE
#define MAX_RETRY_COUNT			3

#define FIRST_CHAR		'D'
#define SECOND_CHAR		'S'
#define THIRD_CHAR		'E'

#define LARGER_SIGN		">"
#define CHAR_I			'I'
#define CHAR_C			'C'
#define CHAR_a			'a'
#define CHAR_l			'l'
#define CHAR_SPACE		' '
#define CHAR_O			'O'
#define CHAR_K			'K'

static BYTE gCmdIndex = 0;
static BYTE gByteCount = 0;
static volatile BYTE gRxFifo[UART2_RX_FIFO_SIZE];
static volatile WORD gRxFifoCount = 0;
static volatile BYTE gStartFlag = 0;
static volatile BYTE gSCI_CheckSum = 0;

BYTE gTeridanCaliType = CALI_NONE;	//[Peter Chung] 20110110 added
static BYTE gRetryCount = 0;
//char resp_str[50];                    //[davidchchen]20160114 removed
//static BYTE resetBuffer_Flag = 0;

BYTE recv_pntr = 0;
BYTE start_pntr = 0;

#define FIRST_HEADER_P		0
#define SECOND_HEADER_P		1
#define THIRD_HEADER_P		2
#define VIN_LSB_P			3
#define VIN_MSB_P			4
#define IIN_LSB_P			5
#define IIN_MSB_P			6
#define PIN_LSB_P			7
#define PIN_MSB_P			8
#define TEMPER_P			9
#define FREQ_P				10
#define CHKSUM_P			11

#define MAX_COMM_ERR_COUNT	100

#define IIN_BUF_SIZE	32 
#define IIN_BUF_EXP		5 

#define PIN_BUF_SIZE	8
#define PIN_BUF_EXP		3

tSCI_DATA Uart;
tSTATUS_TO_PRIMARY StatusToPrimary;
//BYTE tRxData[10];                 //[davidchchen]20160407 removed
static volatile BYTE tRxData[10];   //[davidchchen]20160407 changed
// ------ TX  ------ 
#define HEADER	0xAA;

void init_UART1 ( DWORD u32_baudRate )
{
  // ------ TX  ------
  U1MODEbits.UARTEN = 0;
  CONFIG_BAUDRATE_UART1 ( u32_baudRate );
  CONFIG_PDSEL_UART1 ( UXMODE_PDSEL_8DATA_NOPARITY );        // 8-bit data, no parity
  CONFIG_STOPBITS_UART1 ( 1 );            	                // 1 Stop bit

  //U1MODEbits.URXINV = 1;
  U1MODEbits.UARTEN = 1;
  U1STAbits.UTXEN = 1;

  // ------ RX  ------
  _U1RXIF = 0;							//clear the flag
  _U1RXIP = 3;							//choose a priority
  _U1RXIE = 1;

  //init rx buffer for preventing the primary side OTP
  Uart.U1.Rx.Pri_Temperature.Val = 820;	// 25 degree Celsius
}

void init_UART2 ( DWORD u32_baudRate )
{
  // configure U2MODE
  U2MODEbits.UARTEN = 0;					// Bit15 TX, RX DISABLED, ENABLE at end of func

  //UART macros defined in "uart.h"
  CONFIG_BAUDRATE_UART2 ( u32_baudRate );   	//baud rate
  CONFIG_PDSEL_UART2 ( UXMODE_PDSEL_8DATA_NOPARITY );        // 8-bit data, no parity
  CONFIG_STOPBITS_UART2 ( 1 );            	// 1 Stop bit

  _U2RXIF = 0;              				//clear the flag
  _U2RXIP = 3; 							//choose a priority
  _U2RXIE = 1;              				//enable the interrupt

  memset ( ( char * ) gRxFifo, 0, UART2_RX_FIFO_SIZE );

  //init uart rx buffer
  Uart.U2.Rx.VacMsb = 0;
  Uart.U2.Rx.VacLsb = 0;
  Uart.U2.Rx.IacMsb = 0;
  Uart.U2.Rx.IacLsb = 0;
  Uart.U2.Rx.PinMsb = 0;
  Uart.U2.Rx.PinLsb = 0;
  Uart.U2.Rx.TempPri = 0;	// 8 Bit Adc
  Uart.U2.Rx.Freq = 0;
  PS.U2RX_Updated = TRUE;

  InitEnergyCount ( );

  ENABLE_UART2 ( ); 					   //enable the UART

}

/**
 * Check UART1 RX for error, call \em reportError() if error found.
 *
 */
void CheckRxError ( void )
{
  BYTE u8_c;
  //check for errors, reset if detected.
  if ( U2STAbits.PERR )
      u8_c = U2RXREG;        //clear error

  if ( U2STAbits.FERR )
      u8_c = U2RXREG;       //clear error
      
  if ( U2STAbits.OERR )
      U2STAbits.OERR = 0;   //clear 
      
}


//************ Uart for Primary side IC Begin *************************/

static void UpdatePrimaryData ( UART_FROM_PRI_RECVDATA* rx_data )
{

  switch ( ( *rx_data ).Cmd )
  {
      case CMD_PRI_FW_VER:
      {
          Uart.U1.Rx.Pri_FW_VER.Val = ( *rx_data ).Data.Val;
#if 1
          //Update primary side FW version
          //gPmbusCmd.MFR_FW_VERSION[4] = Uart.U1.Rx.Pri_FW_VER.byte.LB;
          //gPmbusCmd.MFR_FW_VERSION[5] = Uart.U1.Rx.Pri_FW_VER.byte.HB;
          gPmbusCmd.MFR_FW_VERSION[3] = Uart.U1.Rx.Pri_FW_VER.byte.LB;  //[davidchchen]2014/12/11 modify
          gPmbusCmd.MFR_FW_VERSION[4] = Uart.U1.Rx.Pri_FW_VER.byte.HB;  //[davidchchen]2014/12/11 modify
          strncpy ( ( char * ) &UserData.Page2.region.PriFwRev.Val, ( char * ) &Uart.U1.Rx.Pri_FW_VER.Val, MFR_PRI_VERSION_LEN ); //Primary FW version
          PS.PriFwUpdated = TRUE;
#endif
      }

          break;
      case CMD_PRI_TEMPERATURE:
          Uart.U1.Rx.Pri_Temperature.Val = ( *rx_data ).Data.Val;
          break;
      case CMD_PRI_VIN:
          Uart.U1.Rx.Pri_Vin.Val =  ( *rx_data ).Data.Val;
          break;
      case CMD_PRI_VBUS:
          Uart.U1.Rx.Pri_VBus.Val =  ( *rx_data ).Data.Val;
          break;
      case CMD_PRI_FW_VER_INTERNAL:
          Uart.U1.Rx.Pri_FW_VER_Internal.Val = ( *rx_data ).Data.Val;
          gPmbusCmd.PRI_FW_VER_INTERNAL[0] = Uart.U1.Rx.Pri_FW_VER_Internal.byte.LB;
          gPmbusCmd.PRI_FW_VER_INTERNAL[1] = Uart.U1.Rx.Pri_FW_VER_Internal.byte.HB;
          break;
      case CMD_PRI_IIN:
          Uart.U1.Rx.Pri_Iin.Val =  ( *rx_data ).Data.Val;
          break;

      default:
          break;

  }
  //Update Py side Status
  Uart.U1.Rx.Pri_Status.Val = ( *rx_data ).Status;
  
  //[davidchchen]20160503 added primary and secondary side communication error
  if ( Uart.U1.Rx.Pri_Status.bits.PriCommError == 0 )   // 0=> no comm error
  {
      gLedStatus.bits.PriRXcomm_err = 0;         //[davidchchen]20160503 added primary and secondary side communication error
      MFR_Status.bits.SCI_PriRx_FAIL = 0;   //[davidchchen]20160503 added primary and secondary side communication error
      Protect. u1PriRxWDT.Flag = 0;         //[davidchchen]20160503 added primary and secondary side communication error
      Protect. u1PriRxWDT.delay = 0;        //[davidchchen]20160503 added primary and secondary side communication error
  }
  else
  {
      Protect. u1PriRxWDT.Flag = 0;   //[davidchchen]20160503 added primary and secondary side communication error,//[davidchchen]20170109 Disable PriRxWDT.Flag
  }
  

  //Update buffer for debugging Primary side information
  gPmbusCmd.PRI_STATUS[0] = Uart.U1.Rx.Pri_Status.Val;
  gPmbusCmd.PRI_STATUS[1] = Uart.U1.Rx.Pri_Temperature.byte.LB;
  gPmbusCmd.PRI_STATUS[2] = Uart.U1.Rx.Pri_Temperature.byte.HB;
  gPmbusCmd.PRI_STATUS[3] = Uart.U1.Rx.Pri_FW_VER.byte.LB;
  gPmbusCmd.PRI_STATUS[4] = Uart.U1.Rx.Pri_FW_VER.byte.HB;
  gPmbusCmd.PRI_STATUS[5] = Uart.U1.Rx.Pri_VBus.byte.LB;
  gPmbusCmd.PRI_STATUS[6] = Uart.U1.Rx.Pri_VBus.byte.HB;
  gPmbusCmd.PRI_STATUS[7] = Uart.U1.Rx.Pri_Vin.byte.LB;
  gPmbusCmd.PRI_STATUS[8] = Uart.U1.Rx.Pri_Vin.byte.HB;

  MFR_Status.bits.PFC_OV = Uart.U1.Rx.Pri_Status.bits.PFCOV;
}

static void UpdateTxData ( U1_TX_CMD cmd )
{
  WORD result = 0;
  //WORD result2 = 0;     //[davidchchen]20160920 added VTHD ISSUE

  switch ( cmd )
  {
      case CMD_IOUT:
          //transmit Iout adc
          if ( PSFB.MainEnabled == TRUE )
          {
              result = ( ( ( DWORD ) ADC.Iout * ADC_TO_IOUT ) >> ADC_TO_IOUT_GAIN );
          }
          else
          {
              result = 0;
          }

          Uart.U1.Tx.cmd[cmd].byte.LB = result & 0xFF;
          Uart.U1.Tx.cmd[cmd].byte.HB = ( result >> 8 ) & 0xFF;
          break;

      //[davidchchen]20160920 added VTHD ISSUE
#if Config_TERIDIAN_VinToPrimary        //[davidchchen]20161212 78M6610 Vin sent to Primary, add Vthd Fun,but cause AC glitch.
      case CMD_VIN:    
          result2 = Real_Vac;
          if ( PS.IsTeridianCaliMode == TRUE )
          {
              result2 = 230;
          }

          Uart.U1.Tx.cmd[cmd].byte.LB = result2 & 0xFF;
          Uart.U1.Tx.cmd[cmd].byte.HB = ( result2 >> 8 ) & 0xFF;
          break;
#endif

      default:
          break;
  }

  Uart.U1.Tx.status.Val = StatusToPrimary.Val;
}

static void SendU1TxData ( BYTE data )
{
  U1TXREG = data;
}

static void SendU2TxData ( BYTE data )
{
  U2TXREG = data;
}

void CheckU1OverRun ( )
{
  if ( U1STAbits.OERR ) U1STAbits.OERR = 0;	//[Peter Chung] 20101222 added solved for can't go into interrupt
}

void CheckU2OverRun ( )
{
  if ( U2STAbits.OERR ) U2STAbits.OERR = 0;	//[Peter Chung] 20101222 added solved for can't go into interrupt
}

void SCI_U1_Transmit ( )
{
  static U1_RX_STATE uartState = STATE_SENDING_HEADER;
  static BYTE checksum = 0;
  static U1_TX_CMD cmdIndex = CMD_IOUT;
  BYTE data;

  if ( U1STAbits.TRMT == 1 && U1STAbits.UTXBF == 0 )
  {
      switch ( uartState )
      {
          case STATE_SENDING_HEADER:
              UpdateTxData ( cmdIndex );
              data = HEADER;
              SendU1TxData ( data );
              checksum = data;
              uartState ++;
              break;
          case STATE_SENDING_STATUS:
              SendU1TxData ( Uart.U1.Tx.status.Val );
              checksum += Uart.U1.Tx.status.Val;
              uartState ++;
              break;
          case STATE_SENDING_CMD:
              SendU1TxData ( cmdIndex );
              checksum += cmdIndex;
              uartState ++;
              break;
          case STATE_SENDING_LB:
              SendU1TxData ( Uart.U1.Tx.cmd[cmdIndex].byte.LB );
              checksum += Uart.U1.Tx.cmd[cmdIndex].byte.LB;
              uartState ++;
              break;
          case STATE_SENDING_HB:
              SendU1TxData ( Uart.U1.Tx.cmd[cmdIndex].byte.HB );
              checksum += Uart.U1.Tx.cmd[cmdIndex].byte.HB;
              uartState ++;
              break;
          case STATE_SENDING_CS:
              //calc 2's complement
              checksum = ~ checksum + 1;
              SendU1TxData ( checksum );
              //next cmd
              cmdIndex ++;
              if ( cmdIndex >= TOTAL_TX_CMD_NUMBER )
              {
                  cmdIndex = CMD_IOUT;	//the first tx cmd
              }
              //reset tx state
              uartState = STATE_SENDING_HEADER;
              break;

          default:
              break;
      }
  }

}

#if 1

static void UpdateSCIBuffer ( BYTE data )
{
  static BYTE count = 0;

  gPmbusCmd.SCI[count] = data;

  count ++;
  if ( count >= ( sizeof (gPmbusCmd.SCI ) ) )
  {
      count = 0;
  }
}
#endif

//[davidchchen]20160114 removed
//static void ResetRespBuffer ( )
//{
  //memset ( resp_str, 0, sizeof (resp_str ) ); //[davidchchen]20160114 removed
//  resetBuffer_Flag = 1;
//}

void __attribute__ ( ( __interrupt__, no_auto_psv ) ) _U1RXInterrupt ( )
{
  static BYTE RXState = STATE_RX_HEADER;
  static UART_FROM_PRI_RECVDATA RecvData = { 0 };
  static BYTE CheckSum = 0;
  static BYTE header = HEADER;
  static BYTE u1err2_cnt = 0;
  BYTE RecvByte;

  Protect.u1WDT.Flag = 0;		//Chris
  Protect.u1WDT.delay = 0;	//Chris
  PS.Uart1Error = 0;            //[davidchchen]20160503 added primary and secondary side communication error

  RecvByte = U1RXREG;

  switch ( RXState )
  {
      case STATE_RX_HEADER:
      {
          if ( RecvByte == header )
          {
              RXState = STATE_RX_STATUS;
          }
          else
          {
              RXState = STATE_RX_HEADER;
          }

          /*Reset values*/
          CheckSum = 0;
      }
          break;

      case STATE_RX_STATUS:
      {
          RXState = STATE_RX_CMD;
          RecvData.Status = RecvByte;
      }
          break;

      case STATE_RX_CMD:
      {
          RXState = STATE_RX_LB;
          RecvData.Cmd = RecvByte;
      }
          break;

      case STATE_RX_LB:
      {
          RXState = STATE_RX_HB;
          RecvData.Data.byte.LB = RecvByte;
      }
          break;

      case STATE_RX_HB:
      {
          RXState = STATE_RX_CS;
          RecvData.Data.byte.HB = RecvByte;
      }
          break;

      case STATE_RX_CS:
      {
          RXState = STATE_RX_HEADER;
          CheckSum = ( ~ CheckSum ) + 1; // 2's complement

          /*Correct checksum*/
          if ( CheckSum == RecvByte )
          {
              /*Update secondary data*/
              UpdatePrimaryData ( &RecvData );
              //gPSU.SecondaryStatus.Val = RecvData.Status;

              u1err2_cnt = 0;
              gLedStatus.bits.comm_err = 0;
          }
          else
          {
              u1err2_cnt ++;
              if ( u1err2_cnt >= MAX_COMM_ERR_COUNT )
              {
                  gLedStatus.bits.comm_err = 1;	//Chris, for Lenovo spec
                  MFR_Status.bits.SCI_P2S_FAIL = 1;     //[davidchchen]20160503 added primary and secondary side communication error
                  //_SD_Flag.UART1_COMM_ERR2 = 1;
                  //_SD_Flag.LATCH_SD = 1;
              }
          }
      }
          break;
  }

  CheckSum += RecvByte;

  IFS0bits.U1RXIF = 0;

}

//************ Uart for Primary side IC End *************************/


//************ Uart for Teridian IC Begin *************************/

void CalcIinAvg ( )
{
  static WORD IinBuf[IIN_BUF_SIZE];
  static BYTE index = 0;
  static DWORD IinSum = 0;
  WORD Iin;

#if 0	//[Peter Chung] 20101012 removed for faster average
  WORD Iin;
  static DWORD Iin_LPF = 0;
  static WORD Iin_AVG = 0;

  Iin = ( Uart.U2.Rx.IacMsb << 8 ) | Uart.U2.Rx.IacLsb;
  Iin = ( WORD ) ( LinearFmt_YtoX ( Iin, IIN_GAIN ) );	//Gain by 1024, translate to real world value
  Iin_LPF = ( ( Iin_LPF + Iin ) - Iin_AVG );
  Iin_AVG = ( Iin_LPF >> 8 );
  Uart.U2.Rx.IinAvg = Iin_AVG;
#endif

#if WithTeridian
  Iin = ( Uart.U2.Rx.IacMsb << 8 ) | Uart.U2.Rx.IacLsb;
  Iin = ( WORD ) ( LinearFmt_YtoX ( Iin, IIN_GAIN ) );	//Gain by 1024, translate to real world value
  Uart.U2.Rx.IinAvg = GetAvg ( IinBuf, Iin, &index, IIN_BUF_SIZE, &IinSum, IIN_BUF_EXP );
#endif

}

void CalcPinAvg ( )
{
  static WORD PinBuf[PIN_BUF_SIZE];
  static BYTE index = 0;
  static DWORD PinSum = 0;
  WORD Pin;
#if 0
  WORD Pin = 0;
  static DWORD Pin_LPF = 0;
  static WORD Pin_AVG = 0;

  Pin = ( ( Uart.U2.Rx.PinMsb << 8 ) + Uart.U2.Rx.PinLsb );
  Pin = ( WORD ) ( LinearFmt_YtoX ( Pin, PIN_GAIN ) );
  Pin_LPF = ( ( Pin_LPF + Pin ) - Pin_AVG );
  Pin_AVG = ( Pin_LPF >> 8 );
  Uart.U2.Rx.PinAvg = Pin_AVG;
#endif

#if WithTeridian
  Pin = ( ( Uart.U2.Rx.PinMsb << 8 ) + Uart.U2.Rx.PinLsb );
  Pin = ( WORD ) ( LinearFmt_YtoX ( Pin, PIN_GAIN ) );
  Uart.U2.Rx.PinAvg = GetAvg ( PinBuf, Pin, &index, PIN_BUF_SIZE, &PinSum, PIN_BUF_EXP );
#endif

}

void UpdateInputInfo ( )
{
  CalcIinAvg ( );
  CalcPinAvg ( );
}

void WriteBuf ( )
{
  Uart.U2.Rx.VacMsb = gRxFifo[VIN_MSB_P];
  Uart.U2.Rx.VacLsb = gRxFifo[VIN_LSB_P];
  Uart.U2.Rx.IacMsb = gRxFifo[IIN_MSB_P];
  Uart.U2.Rx.IacLsb = gRxFifo[IIN_LSB_P];
  Uart.U2.Rx.PinMsb = gRxFifo[PIN_MSB_P];
  Uart.U2.Rx.PinLsb = gRxFifo[PIN_LSB_P];
  Uart.U2.Rx.TempPri = gRxFifo[TEMPER_P];	// 8 Bit Adc
  Uart.U2.Rx.Freq = gRxFifo[FREQ_P];
  PS.U2RX_Updated = TRUE;
  PS.U2RX_CommOK = TRUE;

  

  //[Peter Chung] 20110303 removed for ALERT glitch issue due to VIN_UV_WARNING was triggered
  //CheckLineStatus(LinearFmt_YtoX((((WORD)Uart.U2.Rx.VacMsb << 8) | Uart.U2.Rx.VacLsb),0), Uart.U2.Rx.Freq);

  //UpdateEnergyCount();	//[Peter Chung] 20101020 modified for updating following 4 AC cycle period.
  //UpdateInputInfo();
}

#if TERIDIAN_CALI_RETRY

void ClearRetryTimer ( )
{
  PS.SCI_Retry = 0;
  Protect.SCI_Retry.Flag = 0;
  Protect.SCI_Retry.delay = 0;
  gRetryCount = 0;
}

void CheckRetryTimer ( BYTE RetryState )
{
  if ( gRetryCount < MAX_RETRY_COUNT )  //[davidchchen]20150114 note -> 0.5s retry,
  {
      Protect.SCI_Retry.Flag = 1;
      if ( PS.SCI_Retry )
      {
          gRetryCount ++;
          PS.SCI_Retry = 0;
          Uart.U2.Tx.SubMode = RetryState;
          Protect.U2TxDelay.Flag = 1;
          //ResetRespBuffer ( );        ResetRespBuffer
      }
  }
}
#endif

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U2RXInterrupt ( void ) // the interrupt should change to every once
{
  //static U2_RX_STATE uartState = STATE_WAITING_D;                 //[davidchchen]20160407 Removed
  static BYTE uartState = STATE_WAITING_D;                          //[davidchchen]20160407 changed
  //static eTeridian_RX_COMM_STATE uartTeridianState = STATE_HD;    //[davidchchen]20160407 Removed
  static BYTE uartTeridianState = STATE_HD;                         //[davidchchen]20160407 changed
  static BYTE CheckSum = 0;

  //UART_count = 0;//reset UART protection timer
//  IFS1bits.U2RXIF = 0;        //[davidchchen]20160407 Removed
//  _U2RXIE = 0;                //[davidchchen]20160407 Removed

  //Reset U2RxErr1 timer
  Protect.u2WDT.Flag = 0;
  Protect.u2WDT.delay = 0;
  PS.Uart2Error = 0;

  CheckRxError ( );
  
  if ( ! PS.IsTeridianCaliMode )
  {
      static BYTE u2err2_cnt = 0;

      gRxFifo[recv_pntr] = U2RXREG;

      UpdateSCIBuffer ( gRxFifo[recv_pntr] );

      switch ( uartState )
      {
          case STATE_WAITING_D:
              if ( gRxFifo[recv_pntr] == ( BYTE ) FIRST_CHAR )
              {
                  uartState = STATE_WAITING_S;
                  gSCI_CheckSum += gRxFifo[recv_pntr];
                  recv_pntr ++;
              }
              break;
          case STATE_WAITING_S:
              if ( gRxFifo[recv_pntr] == ( BYTE ) SECOND_CHAR )
              {
                  uartState = STATE_WAITING_E;
                  gSCI_CheckSum += gRxFifo[recv_pntr];
                  recv_pntr ++;
              }
              else
              {
                  uartState = STATE_WAITING_D;
                  recv_pntr = 0;
                  gSCI_CheckSum = 0;
              }
              break;
          case STATE_WAITING_E:
              if ( gRxFifo[recv_pntr] == ( BYTE ) THIRD_CHAR )
              {
                  uartState = STATE_DATA_RECV;
                  gSCI_CheckSum += gRxFifo[recv_pntr];
                  recv_pntr ++;
              }
              else
              {
                  uartState = STATE_WAITING_D;
                  recv_pntr = 0;
                  gSCI_CheckSum = 0;
              }
              break;
          case STATE_DATA_RECV:
              if ( recv_pntr >= ( UART2_RX_FIFO_SIZE - 1 ) )
              {
                  gSCI_CheckSum += gRxFifo[recv_pntr];
                  if ( gSCI_CheckSum == 0 )
                  {
                      WriteBuf ( );
                      u2err2_cnt = 0;
                      gLedStatus.bits.U2comm_err = 0;         //[davidchchen]20160503 added primary and secondary side communication error
                      MFR_Status.bits.SCI_T2S_FAIL = 0;     //[davidchchen]20160503 added primary and secondary side communication error
                  }
                  else
                  {
                      u2err2_cnt ++;
                      if ( u2err2_cnt >= MAX_COMM_ERR_COUNT )
                      {
                          gLedStatus.bits.U2comm_err = 1;         //[davidchchen]20160503 added primary and secondary side communication error
                          MFR_Status.bits.SCI_T2S_FAIL = 1;     //[davidchchen]20160503 added primary and secondary side communication error
                      }
                  }
                  recv_pntr = 0;
                  gSCI_CheckSum = 0;
                  uartState = STATE_WAITING_D;
              }
              else
              {
                  gSCI_CheckSum += gRxFifo[recv_pntr];
                  recv_pntr ++;
              }
              break;
          default:
              break;
      }
  }
  else
  {
      //Teridian Calibration mode
      //BYTE rxData;                //[davidchchen] 20160407 removed
      static volatile BYTE rxData;  //[davidchchen] 20160407 added
      static BYTE u2err2_cnt = 0;
      static BYTE data_len = 0;

      rxData = U2RXREG;
#if 0
      UpdateRespBuffer ( rxData );
      memcpy ( gPmbusCmd.SCI, resp_str, 50 );
#else
      switch ( uartTeridianState )
      {
          case STATE_HD:
          {

                if (1)// rxData == 0xAD ) //can't solve this issue....
              {
                  PS.Teridian_ACK = 1;
		  uartTeridianState = STATE_HD;
              }
              else if(rxData == 0xAA )
              {
                  uartTeridianState = STATE_LEN;
              }
              else
              {
                  uartTeridianState = STATE_HD;//header error
              }

/*
                if ( rxData == 0xAA )
                {
                    uartTeridianState = STATE_LEN;
                }
#if Config_78M6610   //[davidchchen]20160114 added
                else if ( rxData == 0xAD )                   //[davidchchen]20160114 added
                {
                    u2err2_cnt = 0;                     //[davidchchen]20160114 added
                    PS.Teridian_ACK = 1;                //[davidchchen]20160114 added
                    uartTeridianState = STATE_HD;       //[davidchchen]20160114 added
                }
#endif
                else
                {
                    uartTeridianState = STATE_HD;
                }
*/
                /*Reset values*/
                //CheckSum = 0;
                CheckSum = rxData;
          }
              break;

          case STATE_LEN:
          {
          #if Config_78M6610   //[davidchchen]20160114 added
              if ( rxData <=3 )                     //[davidchchen]20160407 changed, 78M6610 Calibrate Issue
              {
                  uartTeridianState = STATE_HD;     //[davidchchen]20160407 changed, 78M6610 Calibrate Issue
              }
              else
              {
                  uartTeridianState = STATE_DATA;   //[davidchchen]20160407 changed, 78M6610 Calibrate Issue
                  data_len = rxData-3;              //[davidchchen]20160407 changed, 78M6610 Calibrate Issue
                  CheckSum += rxData;               //[davidchchen]20160407 changed, 78M6610 Calibrate Issue
              }

          #else
              if ( rxData == 0 )
              {
                  uartTeridianState = STATE_CS;
              }
              else
              {
                  uartTeridianState = STATE_DATA;
              }
              data_len = rxData;              
              CheckSum += rxData;
          #endif
          }
              break;

          case STATE_DATA:
          {
              static BYTE index = 0;

              data_len --;
              tRxData[index] = rxData;
              index ++;
              if ( data_len <= 0 )
              {
                  uartTeridianState = STATE_CS;
                  index = 0;
              }

              CheckSum += rxData;
          }
              break;

          case STATE_CS:
          {
              uartTeridianState = STATE_HD;
              CheckSum = ( ~ CheckSum ) + 1; // 2's complement

              /*Correct checksum*/
              if ( CheckSum == rxData )
              {
                  u2err2_cnt = 0;
                  PS.Teridian_ACK = 1;
              }
              else
              {
                  u2err2_cnt ++;
                  PS.Teridian_ACK = 0;
                  if ( u2err2_cnt >= MAX_COMM_ERR_COUNT )
                  {

                  }
              }
          }
              break;
      }

      //CheckSum += rxData;
#endif

  }

  //_U2RXIE = 1;                //[davidchchen]20160407 Removed
  IFS1bits.U2RXIF = 0;          //[davidchchen]20160407 Added
}

void ResetU2State ( )
{
  //enter Teridian calibration mode
  PS.IsTeridianCaliMode = TRUE;
  Protect.U2TxDelay.Flag = 1;
  //Reset State
#if 0
  Uart.U2.Tx.Mode = STATE_STOP_TERIDIAN_TX;
  Uart.U2.Tx.SubMode = STATE_SENDING_CNTL_Z;
#else
  Uart.U2.Tx.Mode = STATE_DISABLE_AUTO_REPORTING;
  Uart.U2.Tx.SubMode = STATE_SENDING_DISABLE_AUTO_REPORTING;
#endif
  gPmbusCmd.CAL_TERIDIAN_FINISHED_NOTIFY[0] = 0;
  gPmbusCmd.CAL_TERIDIAN_FINISHED_NOTIFY[1] = 0;
  gPmbusCmd.CAL_TERIDIAN_FINISHED_NOTIFY[2] = 0;
  gCmdIndex = 0;
  gByteCount = 0;
  //PS.SCI_Delay = 0;
  //Protect.SCI_Delay.Flag = 0;
  //Protect.SCI_Delay.delay = 0;
  PS.SCI_Retry = 0;
  Protect.SCI_Retry.Flag = 0;
  Protect.SCI_Retry.delay = 0;
  gRetryCount = 0;
  //[Peter Chung] 20110505 added
  //ResetRespBuffer ( );  //[davidchchen]20160114 removed
}

void SCI_U2_Transmit ( )
{

#if Config_78M6610   //[davidchchen]20160114 added
  static BYTE Tx_Data[10];
  BYTE Result[4];

  if ( U2STAbits.TRMT == 1 && U2STAbits.UTXBF == 0 )
  {
      switch ( Uart.U2.Tx.Mode )
      {
          case STATE_DISABLE_AUTO_REPORTING:
          {
              switch ( Uart.U2.Tx.SubMode )
              {
                  case  STATE_SENDING_DISABLE_AUTO_REPORTING:

                        //vincent change
                        //TX:0xAA 0A
                        //A3 06 00 D3 11 1A 02 A3
                        Tx_Data[0] = 0xA3;
                        Tx_Data[1] = 0x06;
                        Tx_Data[2] = 0x00;
                        Tx_Data[3] = 0xD3;
                        Tx_Data[4] = 0x11;
                        Tx_Data[5] = 0x1A;
                        Tx_Data[6] = 0x02;

                      if ( PS.U2TxSend )
                      {
                          //vincent change
                          if ( TeridianNormalCMDWrite (0xAA, 0x0A , &Tx_Data[0] ) == TRUE )
                          {
                              PS.U2TxSend = FALSE;
                              PS.Teridian_ACK = FALSE;
                              Protect.U2TxDelay.Flag = 0;  //disable Tx transmit
                              Uart.U2.Tx.SubMode = STATE_WAITING_ACK;
                          }
                      }
                      break;

                  case STATE_WAITING_ACK:
                      if ( PS.Teridian_ACK == 1)
                      {
                          PS.Teridian_ACK = FALSE;
                          Protect.U2TxDelay.Flag = 1; //Receive Meter's ACK, delay 0.5s

                      #if TERIDIAN_CALI_RETRY
                          ClearRetryTimer ( );
                      #endif

                          if ( gTeridanCaliType == CALI_NORMAL_LOAD )
                          {
                              Uart.U2.Tx.Mode = STATE_INPUT_CURRENT_DATA;
                              Uart.U2.Tx.SubMode = STATE_SENDING_INPUT_CURRENT_DATA;
                          }
                          else if ( gTeridanCaliType == CALI_LIGHT_LOAD )
                          {
                              Uart.U2.Tx.Mode = STATE_INPUT_VOLTAGE_DATA;
                              Uart.U2.Tx.SubMode = STATE_SENDING_INPUT_VOLTAGE_DATA;
                          }
                          else if ( gTeridanCaliType == CALI_VDC_OFFSET )       //[davidchchen]20160223 added
                          {
                              Uart.U2.Tx.Mode = STATE_INPUT_VDC_DATA;
                              Uart.U2.Tx.SubMode = STATE_SENDING_INPUT_VDC_DATA;
                          }
                          else if ( gTeridanCaliType == CALI_IDC_OFFSET )       //[davidchchen]20160223 added
                          {
                              Uart.U2.Tx.Mode = STATE_INPUT_IDC_DATA;
                              Uart.U2.Tx.SubMode = STATE_SENDING_INPUT_IDC_DATA;
                          }
                          else
                          {
                              //Do nothing
                          }
                      }
                      else
                      {
                        #if TERIDIAN_CALI_RETRY
                            CheckRetryTimer ( STATE_SENDING_DISABLE_AUTO_REPORTING );
                        #endif
                      }
                      break;
              }
          }
              break;

//---------------------volt cali below---------------------------------------//
          //vincent change
          case STATE_INPUT_VOLTAGE_DATA:
          {
              switch ( Uart.U2.Tx.SubMode )
              {
                  case  STATE_SENDING_INPUT_VOLTAGE_DATA:
                        //vincent change
                        // TX:0xAA 0A A3 7B 00 D3 C0 D4 01 C6
                        Tx_Data[0] = 0xA3;
                        Tx_Data[1] = 0x78;
                        Tx_Data[2] = 0x00;
                        Tx_Data[3] = 0xD3;
                        //Input voltage value
                        Float_Conver_to_Decimal(gPmbusCmd.CAL_TERIDIAN_IC, Result);   //vincent, CMD 0XBB

                        Tx_Data[4] = Result[0];
                        Tx_Data[5] = Result[1];
                        Tx_Data[6] = Result[2];

                      //Tx_Data[0] = gPmbusCmd.CAL_TERIDIAN_IC[1];
                      //Tx_Data[1] = gPmbusCmd.CAL_TERIDIAN_IC[2];
                      //Tx_Data[2] = gPmbusCmd.CAL_TERIDIAN_IC[3];
                      //Tx_Data[3] = gPmbusCmd.CAL_TERIDIAN_IC[4];

                      if ( PS.U2TxSend )
                      {
                          if ( TeridianNormalCMDWrite ( 0xAA, 0x0A, &Tx_Data[0] ) == TRUE )
                          {
                              PS.U2TxSend = FALSE;
                              PS.Teridian_ACK = FALSE;
                              Protect.U2TxDelay.Flag = 0;
                              Uart.U2.Tx.SubMode = STATE_WAITING_ACK;
                          }
                      }
                      break;
                  case STATE_WAITING_ACK:
                      if ( PS.Teridian_ACK )
                      {
                          PS.Teridian_ACK = FALSE;
                          Protect.U2TxDelay.Flag = 1;
                          Uart.U2.Tx.Mode = STATE_CLV_CALIBRATE;
                          Uart.U2.Tx.SubMode = STATE_SENDING_CLV_CMD;
                      }
                      else
                      {
                      }
                      break;
              }
          }
              break;

          case STATE_CLV_CALIBRATE:
          {
              switch ( Uart.U2.Tx.SubMode )
              {
                  case STATE_SENDING_CLV_CMD:
                        //vincent change
                        //TX:0xAA 0A
                        //A3 00 00 D3 20 00 CA EC
                        Tx_Data[0] = 0xA3;
                        Tx_Data[1] = 0x00;
                        Tx_Data[2] = 0x00;
                        Tx_Data[3] = 0xD3;
                        Tx_Data[4] = 0x20;
                        Tx_Data[5] = 0x00;
                        Tx_Data[6] = 0xCA;

                      if ( PS.U2TxSend )
                      {
                          if ( TeridianNormalCMDWrite ( 0xAA, 0x0A, &Tx_Data[0] ) == TRUE )     //vincent change
                          {
                              PS.U2TxSend = FALSE;
                              PS.Teridian_ACK = FALSE;
                              Protect.U2TxDelay.Flag = 0;
                              Uart.U2.Tx.SubMode = STATE_WAITING_ACK;
                          }
                      }
                      break;
                  case STATE_WAITING_ACK:
                      if ( PS.Teridian_ACK )
                      {
                          PS.Teridian_ACK = FALSE;
                          Protect.U2TxDelay.Flag = 1;
                          Uart.U2.Tx.Mode = STATE_CHECKING_CALIBRATION_DONE;        //vincent change
                          Uart.U2.Tx.SubMode = STATE_SENDING_CALIBRATION_PROCESS;   //vincent change

                      }
                      else
                      {
                      }
                      break;
              }
          }
              break;
//---------------------------Volt cali above-------------------------//

//---------------------------Current cali below---------------------------//
          case STATE_INPUT_CURRENT_DATA:
          {
              switch ( Uart.U2.Tx.SubMode )
              {
                  case  STATE_SENDING_INPUT_CURRENT_DATA:
                        //vincent change
                        //TX:0xAA 0A
                        //A3 75 00 D3 -- -- -- crc
                        Tx_Data[0] = 0xA3;
                        Tx_Data[1] = 0x75;
                        Tx_Data[2] = 0x00;
                        Tx_Data[3] = 0xD3;
                        //Input current value
                        Float_Conver_to_Decimal(gPmbusCmd.CAL_TERIDIAN_IC, Result);   //vincent

                        Tx_Data[4] = Result[0];
                        Tx_Data[5] = Result[1];
                        Tx_Data[6] = Result[2];

                      if ( PS.U2TxSend )
                      {
                          if ( TeridianNormalCMDWrite ( 0xAA, 0x0A, &Tx_Data[0] ) == TRUE )
                          {
                              PS.U2TxSend = FALSE;
                              PS.Teridian_ACK = FALSE;
                              Protect.U2TxDelay.Flag = 0;
                              Uart.U2.Tx.SubMode = STATE_WAITING_ACK;
                          }
                      }
                      break;
                  case STATE_WAITING_ACK:
                      if ( PS.Teridian_ACK )
                      {
                          PS.Teridian_ACK = FALSE;
                          Protect.U2TxDelay.Flag = 1;
                          Uart.U2.Tx.Mode = STATE_CLC_CALIBRATE;
                          Uart.U2.Tx.SubMode = STATE_SENDING_CLC_CMD;
                      }
                      else
                      {
                      }
                      break;
              }
          }
              break;

          case STATE_CLC_CALIBRATE:
          {
              switch ( Uart.U2.Tx.SubMode )
              {
                  case  STATE_SENDING_CLC_CMD:
                        //vincent change
                        //TX:0xAA 0A
                        //A3 00 00 D3 10 00 CA FC
                        // Initiate a Current gain calibration
                        Tx_Data[0] = 0xA3;
                        Tx_Data[1] = 0x00;
                        Tx_Data[2] = 0x00;
                        Tx_Data[3] = 0xD3;
                        Tx_Data[4] = 0x10;
                        Tx_Data[5] = 0x00;
                        Tx_Data[6] = 0xCA;

                      if ( PS.U2TxSend )
                      {
                          if ( TeridianNormalCMDWrite ( 0xAA, 0x0A, &Tx_Data[0] ) == TRUE )
                          {
                              PS.U2TxSend = FALSE;
                              PS.Teridian_ACK = FALSE;
                              Protect.U2TxDelay.Flag = 0;
                              Uart.U2.Tx.SubMode = STATE_WAITING_ACK;
                          }
                      }
                      break;
                  case STATE_WAITING_ACK:
                      if ( PS.Teridian_ACK )
                      {
                          PS.Teridian_ACK = FALSE;
                          Protect.U2TxDelay.Flag = 1;
                          Uart.U2.Tx.Mode = STATE_CHECKING_CALIBRATION_DONE;        //vincent change
                          Uart.U2.Tx.SubMode = STATE_SENDING_CALIBRATION_PROCESS;   //vincent change
                      }
                      else
                      {
                      }
                      break;
              }
          }
              break;

//--------------------------- VDC OFFSET cali below---------------------------//
          case STATE_INPUT_VDC_DATA:
          {
              switch ( Uart.U2.Tx.SubMode )
              {
                  case  STATE_SENDING_INPUT_VDC_DATA:
                        //vincent change
                        //TX:0xAA 0A
                        //A3 75 00 D3 -- -- -- crc
                        Tx_Data[0] = 0xA3;
                        Tx_Data[1] = 0x72;
                        Tx_Data[2] = 0x00;
                        Tx_Data[3] = 0xD3;
                        //Input current value
                        Float_Conver_to_Decimal(gPmbusCmd.CAL_TERIDIAN_IC, Result);   //vincent

                        Tx_Data[4] = Result[0];
                        Tx_Data[5] = Result[1];
                        Tx_Data[6] = Result[2];

                      if ( PS.U2TxSend )
                      {
                          if ( TeridianNormalCMDWrite ( 0xAA, 0x0A, &Tx_Data[0] ) == TRUE )
                          {
                              PS.U2TxSend = FALSE;
                              PS.Teridian_ACK = FALSE;
                              Protect.U2TxDelay.Flag = 0;
                              Uart.U2.Tx.SubMode = STATE_WAITING_ACK;
                          }
                      }
                      break;
                  case STATE_WAITING_ACK:
                      if ( PS.Teridian_ACK )
                      {
                          PS.Teridian_ACK = FALSE;
                          Protect.U2TxDelay.Flag = 1;
                          Uart.U2.Tx.Mode = STATE_VDC_CALIBRATE;
                          Uart.U2.Tx.SubMode = STATE_SENDING_VDC_CMD;
                      }
                      else
                      {
                      }
                      break;
              }
          }
              break;

          case STATE_VDC_CALIBRATE:
          {
              switch ( Uart.U2.Tx.SubMode )
              {
                  case  STATE_SENDING_VDC_CMD:
                        //vincent change
                        //TX:0xAA 0A
                        //A3 00 00 D3 10 00 CA FC
                        // Initiate a Current gain calibration
                        Tx_Data[0] = 0xA3;
                        Tx_Data[1] = 0x00;
                        Tx_Data[2] = 0x00;
                        Tx_Data[3] = 0xD3;
                        Tx_Data[4] = 0x08;
                        Tx_Data[5] = 0x00;
                        Tx_Data[6] = 0xCA;

                      if ( PS.U2TxSend )
                      {
                          if ( TeridianNormalCMDWrite ( 0xAA, 0x0A, &Tx_Data[0] ) == TRUE )
                          {
                              PS.U2TxSend = FALSE;
                              PS.Teridian_ACK = FALSE;
                              Protect.U2TxDelay.Flag = 0;
                              Uart.U2.Tx.SubMode = STATE_WAITING_ACK;
                          }
                      }
                      break;
                  case STATE_WAITING_ACK:
                      if ( PS.Teridian_ACK )
                      {
                          PS.Teridian_ACK = FALSE;
                          Protect.U2TxDelay.Flag = 1;
                          Uart.U2.Tx.Mode = STATE_CHECKING_CALIBRATION_DONE;        //vincent change
                          Uart.U2.Tx.SubMode = STATE_SENDING_CALIBRATION_PROCESS;   //vincent change
                      }
                      else
                      {
                      }
                      break;
              }
          }
              break;

//--------------------------- IDC OFFSET cali below---------------------------//
          case STATE_INPUT_IDC_DATA:
          {
              switch ( Uart.U2.Tx.SubMode )
              {
                  case  STATE_SENDING_INPUT_IDC_DATA:
                        //vincent change
                        //TX:0xAA 0A
                        //A3 75 00 D3 -- -- -- crc
                        Tx_Data[0] = 0xA3;
                        Tx_Data[1] = 0x6F;
                        Tx_Data[2] = 0x00;
                        Tx_Data[3] = 0xD3;
                        //Input current value
                        Float_Conver_to_Decimal(gPmbusCmd.CAL_TERIDIAN_IC, Result);   //vincent

                        Tx_Data[4] = Result[0];
                        Tx_Data[5] = Result[1];
                        Tx_Data[6] = Result[2];

                      if ( PS.U2TxSend )
                      {
                          if ( TeridianNormalCMDWrite ( 0xAA, 0x0A, &Tx_Data[0] ) == TRUE )
                          {
                              PS.U2TxSend = FALSE;
                              PS.Teridian_ACK = FALSE;
                              Protect.U2TxDelay.Flag = 0;
                              Uart.U2.Tx.SubMode = STATE_WAITING_ACK;
                          }
                      }
                      break;
                  case STATE_WAITING_ACK:
                      if ( PS.Teridian_ACK )
                      {
                          PS.Teridian_ACK = FALSE;
                          Protect.U2TxDelay.Flag = 1;
                          Uart.U2.Tx.Mode = STATE_IDC_CALIBRATE;
                          Uart.U2.Tx.SubMode = STATE_SENDING_IDC_CMD;
                      }
                      else
                      {
                      }
                      break;
              }
          }
              break;

          case STATE_IDC_CALIBRATE:
          {
              switch ( Uart.U2.Tx.SubMode )
              {
                  case  STATE_SENDING_IDC_CMD:
                        //vincent change
                        //TX:0xAA 0A
                        //A3 00 00 D3 10 00 CA FC
                        // Initiate a Current gain calibration
                        Tx_Data[0] = 0xA3;
                        Tx_Data[1] = 0x00;
                        Tx_Data[2] = 0x00;
                        Tx_Data[3] = 0xD3;
                        Tx_Data[4] = 0x04;
                        Tx_Data[5] = 0x00;
                        Tx_Data[6] = 0xCA;

                      if ( PS.U2TxSend )
                      {
                          if ( TeridianNormalCMDWrite ( 0xAA, 0x0A, &Tx_Data[0] ) == TRUE )
                          {
                              PS.U2TxSend = FALSE;
                              PS.Teridian_ACK = FALSE;
                              Protect.U2TxDelay.Flag = 0;
                              Uart.U2.Tx.SubMode = STATE_WAITING_ACK;
                          }
                      }
                      break;
                  case STATE_WAITING_ACK:
                      if ( PS.Teridian_ACK )
                      {
                          PS.Teridian_ACK = FALSE;
                          Protect.U2TxDelay.Flag = 1;
                          Uart.U2.Tx.Mode = STATE_CHECKING_CALIBRATION_DONE;        //vincent change
                          Uart.U2.Tx.SubMode = STATE_SENDING_CALIBRATION_PROCESS;   //vincent change
                      }
                      else
                      {
                      }
                      break;
              }
          }
              break;

//----------------------------Current cali above ----------------//
          //vincent change
          case STATE_CHECKING_CALIBRATION_DONE:
          {
              switch ( Uart.U2.Tx.SubMode )
              {
                  case  STATE_SENDING_CALIBRATION_PROCESS:
                        //vincent change
                        //TX:0xAA 07
                        //A3 00 00 E3 C9
                        Tx_Data[0] = 0xA3;
                        Tx_Data[1] = 0x00;
                        Tx_Data[2] = 0x00;
                        Tx_Data[3] = 0xE3;

                      if ( PS.U2TxSend )
                      {
                          if ( TeridianNormalCMDWrite ( 0xAA, 0x07, &Tx_Data[0] ) == TRUE ) //vincent change
                          {
                              PS.U2TxSend = FALSE;
                              PS.Teridian_ACK = FALSE;
                              Protect.U2TxDelay.Flag = 0;
                              Uart.U2.Tx.SubMode = STATE_WAITING_ACK;
                          }
                      }
													                      break;
                  case STATE_WAITING_ACK:
                      //if ( gRetryCount > 20 ) //PS.Teridian_ACK , //vincent change
                      if ( PS.Teridian_ACK )        //[davidchchen]20160114 added
                      {
                          PS.Teridian_ACK = FALSE;
                          Protect.U2TxDelay.Flag = 1;
                      #if TERIDIAN_CALI_RETRY
                          ClearRetryTimer();
                      #endif
                          //[davidchchen]20160114 added
                          //if ( ( tRxData[0]== 0x00 || tRxData[0]== 0x10 || tRxData[0]== 0x20 ) && ( tRxData[2]== 0x00 ) )                                          //[davidchchen]20160114 added

                          /*
                          if ( tRxData[0]== 0x00 && tRxData[2]== 0x00 )
                          {
                                Uart.U2.Tx.Mode = STATE_UPDATE_FLASH;                 //vincent change
                                Uart.U2.Tx.SubMode = STATE_SENDING_UPDATE_FLASH;      //vincent change
                          }
                          else if ( tRxData[2]== 0xCA )                                     //[davidchchen]20160114 added
                          {
                                Uart.U2.Tx.Mode = STATE_CHECKING_CALIBRATION_DONE;                      //[davidchchen]20160114 added
                                Uart.U2.Tx.SubMode = STATE_SENDING_CALIBRATION_PROCESS;                 //[davidchchen]20160114 added
                          }
                          else
                          {

                          }
                          */
                          
                          
                          //if ( tRxData[2]== 0xCA )                                      //[davidchchen]20160407 changed
                          //{
                          //      Uart.U2.Tx.Mode = STATE_CHECKING_CALIBRATION_DONE;      //[davidchchen]20160407 changed
                          //      Uart.U2.Tx.SubMode = STATE_SENDING_CALIBRATION_PROCESS; //[davidchchen]20160407 changed
                          //}
                          //else if ( tRxData[2]== 0x00 )
                          //{
                                Uart.U2.Tx.Mode = STATE_UPDATE_FLASH;                 //[davidchchen]20160407 changed
                                Uart.U2.Tx.SubMode = STATE_SENDING_UPDATE_FLASH;      //[davidchchen]20160407 changed
                          //}
                          

                      }
                      else
                      {
#if TERIDIAN_CALI_RETRY
                         CheckRetryTimer(STATE_SENDING_CALIBRATION_PROCESS);    //vincent change
#endif
                      }
                      break;
              }
          }
              break;

          case STATE_UPDATE_FLASH:
          {
              switch ( Uart.U2.Tx.SubMode )
              {
                  case  STATE_SENDING_UPDATE_FLASH:
                        //vincent change
                        //TX:0xAA 0A
                        //A3 00 00 D3 00 C2 AC 68
                        Tx_Data[0] = 0xA3;
                        Tx_Data[1] = 0x00;
                        Tx_Data[2] = 0x00;
                        Tx_Data[3] = 0xD3;
                        Tx_Data[4] = 0x00;
                        Tx_Data[5] = 0xC2;
                        Tx_Data[6] = 0xAC;
                      if ( PS.U2TxSend )
                      {
                          if ( TeridianNormalCMDWrite ( 0xAA, 0x0A, &Tx_Data[0] ) == TRUE )
                          {
                              PS.U2TxSend = FALSE;
                              PS.Teridian_ACK = FALSE;
                              Protect.U2TxDelay.Flag = 0;
                              Uart.U2.Tx.SubMode = STATE_WAITING_ACK;
                          }
                      }
                      break;
                  case STATE_WAITING_ACK:
                      if ( PS.Teridian_ACK )
                      {
                          PS.Teridian_ACK = FALSE;
                          Protect.U2TxDelay.Flag = 1;
#if TERIDIAN_CALI_RETRY
                          //ClearRetryTimer();
#endif
                          Uart.U2.Tx.Mode = STATE_ENABLE_AUTO_REPORTING;            //vincent change
                          Uart.U2.Tx.SubMode = STATE_SENDING_ENABLE_AUTO_REPORTING; //vincent change
                      }
                      else
                      {
#if TERIDIAN_CALI_RETRY
                          //CheckRetryTimer(STATE_SENDING_UPDATE_FLASH);
#endif
                      }
                      break;
              }
          }
              break;

          case STATE_ENABLE_AUTO_REPORTING:
          {
              switch ( Uart.U2.Tx.SubMode )
              {
                  case  STATE_SENDING_ENABLE_AUTO_REPORTING:
                        //vincent change
                        //TX:0xAA 0A
                        //A3 06 00 D3 19 1A 02 9B
                        Tx_Data[0] = 0xA3;
                        Tx_Data[1] = 0x06;
                        Tx_Data[2] = 0x00;
                        Tx_Data[3] = 0xD3;
                        Tx_Data[4] = 0x19;
                        Tx_Data[5] = 0x1A;
                        Tx_Data[6] = 0x02;

                      if ( PS.U2TxSend )
                      {
                          //vincent change
                          if ( TeridianNormalCMDWrite ( 0xAA, 0x0A, &Tx_Data[0] ) == TRUE )
                          {
                              PS.U2TxSend = FALSE;
                              PS.Teridian_ACK = FALSE;
                              Protect.U2TxDelay.Flag = 0;
                              Uart.U2.Tx.SubMode = STATE_WAITING_ACK;
                          }
                      }
                      break;
                  case STATE_WAITING_ACK:
                      if ( PS.Teridian_ACK )
                      {
                          PS.Teridian_ACK = FALSE;
                          Protect.U2TxDelay.Flag = 1;
#if TERIDIAN_CALI_RETRY
                          ClearRetryTimer ( );
#endif
                          Uart.U2.Tx.SubMode = STATE_CALIBRATION_FINISH;
                      }
                      else
                      {
#if TERIDIAN_CALI_RETRY
                          CheckRetryTimer ( STATE_ENABLE_AUTO_REPORTING );
#endif
                      }
                      break;
                  case STATE_CALIBRATION_FINISH:
                      PS.IsTeridianCaliMode = FALSE;
                      break;
              }
          }
              break;

          default:
              break;
      }

  }

#else
  static BYTE Tx_Data[10];

  if ( U2STAbits.TRMT == 1 && U2STAbits.UTXBF == 0 )
  {
      switch ( Uart.U2.Tx.Mode )
      {
          case STATE_DISABLE_AUTO_REPORTING:
          {
              switch ( Uart.U2.Tx.SubMode )
              {
                  case STATE_SENDING_DISABLE_AUTO_REPORTING:
                      Tx_Data[0] = 0x00;
                      if ( PS.U2TxSend )
                      {
                          if ( TeridianSpecialCMDWrite ( 0x6A, 1, &Tx_Data[0] ) == TRUE )
                          {
                              PS.U2TxSend = FALSE;
                              PS.Teridian_ACK = FALSE;
                              Protect.U2TxDelay.Flag = 0;
                              Uart.U2.Tx.SubMode = STATE_WAITING_ACK;
                          }
                      }
                      break;

                  case STATE_WAITING_ACK:
                      if ( PS.Teridian_ACK )
                      {
                          PS.Teridian_ACK = FALSE;
                          Protect.U2TxDelay.Flag = 1;
#if TERIDIAN_CALI_RETRY
                          ClearRetryTimer ( );
#endif
                          if ( gTeridanCaliType == CALI_NORMAL_LOAD )
                          {
                              Uart.U2.Tx.Mode = STATE_INPUT_CURRENT_DATA;
                              Uart.U2.Tx.SubMode = STATE_SENDING_INPUT_CURRENT_DATA;
                          }
                          else if ( gTeridanCaliType == CALI_LIGHT_LOAD )
                          {
                              Uart.U2.Tx.Mode = STATE_INPUT_VOLTAGE_DATA;
                              Uart.U2.Tx.SubMode = STATE_SENDING_INPUT_VOLTAGE_DATA;
                          }
                          else if ( gTeridanCaliType == GET_TERIDIAN_FW_REV )
                          {
                              Uart.U2.Tx.Mode = STATE_GET_TERIDIAN_FW_REV;
                              Uart.U2.Tx.SubMode = STATE_SENDING_QUERY_FWREV_CMD;
                          }
                          else
                          {
                              //Do nothing
                          }
                      }
                      else
                      {
#if TERIDIAN_CALI_RETRY
                          CheckRetryTimer ( STATE_SENDING_DISABLE_AUTO_REPORTING );
#endif
                      }
                      break;
              }
          }
              break;

          case STATE_GET_TERIDIAN_FW_REV:
          {
              switch ( Uart.U2.Tx.SubMode )
              {
                  case STATE_SENDING_QUERY_FWREV_CMD:
                      Tx_Data[0] = 0x00;
                      if ( PS.U2TxSend )
                      {
                          if ( TeridianSpecialCMDWrite ( 0x64, 0, &Tx_Data[0] ) == TRUE )
                          {
                              PS.U2TxSend = FALSE;
                              PS.Teridian_ACK = FALSE;
                              Protect.U2TxDelay.Flag = 0;
                              Uart.U2.Tx.SubMode = STATE_WAITING_ACK;
                          }
                      }
                      break;
                  case STATE_WAITING_ACK:
                      if ( PS.Teridian_ACK )
                      {
                          PS.Teridian_ACK = FALSE;
                          Protect.U2TxDelay.Flag = 1;
#if TERIDIAN_CALI_RETRY
                          //ClearRetryTimer();
#endif
                          //Get FW revision
                          memcpy ( gPmbusCmd.CAL_TERIDIAN_GET_FW_VER, tRxData, 3 );

                          Uart.U2.Tx.Mode = STATE_ENABLE_AUTO_REPORTING;
                          Uart.U2.Tx.SubMode = STATE_SENDING_ENABLE_AUTO_REPORTING;
                      }
                      else
                      {
#if TERIDIAN_CALI_RETRY
                          //CheckRetryTimer(STATE_SENDING_CLC_CMD);
#endif
                      }
                      break;
              }
          }
              break;

          case STATE_INPUT_CURRENT_DATA:
          {
              switch ( Uart.U2.Tx.SubMode )
              {
                  case STATE_SENDING_INPUT_CURRENT_DATA:
                      Tx_Data[0] = gPmbusCmd.CAL_TERIDIAN_IC[1];
                      Tx_Data[1] = gPmbusCmd.CAL_TERIDIAN_IC[2];
                      Tx_Data[2] = gPmbusCmd.CAL_TERIDIAN_IC[3];
                      Tx_Data[3] = gPmbusCmd.CAL_TERIDIAN_IC[4];
                      if ( PS.U2TxSend )
                      {
                          if ( TeridianNormalCMDWrite ( 0x10, 7, 0x055C, 4, &Tx_Data[0] ) == TRUE )
                          {
                              PS.U2TxSend = FALSE;
                              PS.Teridian_ACK = FALSE;
                              Protect.U2TxDelay.Flag = 0;
                              Uart.U2.Tx.SubMode = STATE_WAITING_ACK;
                          }
                      }
                      break;
                  case STATE_WAITING_ACK:
                      if ( PS.Teridian_ACK )
                      {
                          PS.Teridian_ACK = FALSE;
                          Protect.U2TxDelay.Flag = 1;
#if TERIDIAN_CALI_RETRY
                          //ClearRetryTimer();
#endif
                          Uart.U2.Tx.Mode = STATE_CLC_CALIBRATE;
                          Uart.U2.Tx.SubMode = STATE_SENDING_CLC_CMD;
                      }
                      else
                      {
#if TERIDIAN_CALI_RETRY
                          //CheckRetryTimer(STATE_SENDING_INPUT_CURRENT_DATA);
#endif
                      }
                      break;
              }
          }
              break;

          case STATE_CLC_CALIBRATE:
          {
              switch ( Uart.U2.Tx.SubMode )
              {
                  case STATE_SENDING_CLC_CMD:
                      Tx_Data[0] = 0x04;
                      Tx_Data[1] = 0x01;
                      if ( PS.U2TxSend )
                      {
                          if ( TeridianSpecialCMDWrite ( 0x68, 2, &Tx_Data[0] ) == TRUE )
                          {
                              PS.U2TxSend = FALSE;
                              PS.Teridian_ACK = FALSE;
                              Protect.U2TxDelay.Flag = 0;
                              Uart.U2.Tx.SubMode = STATE_WAITING_ACK;
                          }
                      }
                      break;
                  case STATE_WAITING_ACK:
                      if ( PS.Teridian_ACK )
                      {
                          PS.Teridian_ACK = FALSE;
                          Protect.U2TxDelay.Flag = 1;
#if TERIDIAN_CALI_RETRY
                          //ClearRetryTimer();
#endif
                          Uart.U2.Tx.Mode = STATE_UPDATE_FLASH_MPU;
                          Uart.U2.Tx.SubMode = STATE_SENDING_UPDATE_FLASH_MPU;
                      }
                      else
                      {
#if TERIDIAN_CALI_RETRY
                          //CheckRetryTimer(STATE_SENDING_CLC_CMD);
#endif
                      }
                      break;
              }
          }
              break;

          case STATE_INPUT_VOLTAGE_DATA:
          {
              switch ( Uart.U2.Tx.SubMode )
              {
                  case STATE_SENDING_INPUT_VOLTAGE_DATA:
                      Tx_Data[0] = gPmbusCmd.CAL_TERIDIAN_IC[1];
                      Tx_Data[1] = gPmbusCmd.CAL_TERIDIAN_IC[2];
                      Tx_Data[2] = gPmbusCmd.CAL_TERIDIAN_IC[3];
                      Tx_Data[3] = gPmbusCmd.CAL_TERIDIAN_IC[4];
                      if ( PS.U2TxSend )
                      {
                          if ( TeridianNormalCMDWrite ( 0x10, 7, 0x0560, 4, &Tx_Data[0] ) == TRUE )
                          {
                              PS.U2TxSend = FALSE;
                              PS.Teridian_ACK = FALSE;
                              Protect.U2TxDelay.Flag = 0;
                              Uart.U2.Tx.SubMode = STATE_WAITING_ACK;
                          }
                      }
                      break;
                  case STATE_WAITING_ACK:
                      if ( PS.Teridian_ACK )
                      {
                          PS.Teridian_ACK = FALSE;
                          Protect.U2TxDelay.Flag = 1;
#if TERIDIAN_CALI_RETRY
                          //ClearRetryTimer();
#endif
                          Uart.U2.Tx.Mode = STATE_CLV_CALIBRATE;
                          Uart.U2.Tx.SubMode = STATE_SENDING_CLV_CMD;
                      }
                      else
                      {
#if TERIDIAN_CALI_RETRY
                          //CheckRetryTimer(STATE_SENDING_INPUT_VOLTAGE_DATA);
#endif
                      }
                      break;
              }
          }
              break;

          case STATE_CLV_CALIBRATE:
          {
              switch ( Uart.U2.Tx.SubMode )
              {
                  case STATE_SENDING_CLV_CMD:
                      Tx_Data[0] = 0x02;
                      Tx_Data[1] = 0x01;
                      if ( PS.U2TxSend )
                      {
                          if ( TeridianSpecialCMDWrite ( 0x68, 2, &Tx_Data[0] ) == TRUE )
                          {
                              PS.U2TxSend = FALSE;
                              PS.Teridian_ACK = FALSE;
                              Protect.U2TxDelay.Flag = 0;
                              Uart.U2.Tx.SubMode = STATE_WAITING_ACK;
                          }
                      }
                      break;
                  case STATE_WAITING_ACK:
                      if ( PS.Teridian_ACK )
                      {
                          PS.Teridian_ACK = FALSE;
                          Protect.U2TxDelay.Flag = 1;
#if TERIDIAN_CALI_RETRY
                          //ClearRetryTimer();
#endif
                          Uart.U2.Tx.Mode = STATE_UPDATE_FLASH_MPU;
                          Uart.U2.Tx.SubMode = STATE_SENDING_UPDATE_FLASH_MPU;
                      }
                      else
                      {
#if TERIDIAN_CALI_RETRY
                          //CheckRetryTimer(STATE_SENDING_CLV_CMD);
#endif
                      }
                      break;
              }
          }
              break;

          case STATE_UPDATE_FLASH_MPU:
          {
              switch ( Uart.U2.Tx.SubMode )
              {
                  case STATE_SENDING_UPDATE_FLASH_MPU:
                      Tx_Data[0] = 0x01;
                      if ( PS.U2TxSend )
                      {
                          if ( TeridianSpecialCMDWrite ( 0x66, 1, &Tx_Data[0] ) == TRUE )
                          {
                              PS.U2TxSend = FALSE;
                              PS.Teridian_ACK = FALSE;
                              Protect.U2TxDelay.Flag = 0;
                              Uart.U2.Tx.SubMode = STATE_WAITING_ACK;
                          }
                      }
                      break;
                  case STATE_WAITING_ACK:
                      if ( PS.Teridian_ACK )
                      {
                          PS.Teridian_ACK = FALSE;
                          Protect.U2TxDelay.Flag = 1;
#if TERIDIAN_CALI_RETRY
                          //ClearRetryTimer();
#endif
                          Uart.U2.Tx.Mode = STATE_UPDATE_FLASH_CE;
                          Uart.U2.Tx.SubMode = STATE_SENDING_UPDATE_FLASH_CE;
                      }
                      else
                      {
#if TERIDIAN_CALI_RETRY
                          //CheckRetryTimer(STATE_SENDING_UPDATE_FLASH_MPU);
#endif
                      }
                      break;
              }
          }
              break;

          case STATE_UPDATE_FLASH_CE:
          {
              switch ( Uart.U2.Tx.SubMode )
              {
                  case STATE_SENDING_UPDATE_FLASH_CE:
                      Tx_Data[0] = 0x00;
                      if ( PS.U2TxSend )
                      {
                          if ( TeridianSpecialCMDWrite ( 0x66, 1, &Tx_Data[0] ) == TRUE )
                          {
                              PS.U2TxSend = FALSE;
                              PS.Teridian_ACK = FALSE;
                              Protect.U2TxDelay.Flag = 0;
                              Uart.U2.Tx.SubMode = STATE_WAITING_ACK;
                          }
                      }
                      break;
                  case STATE_WAITING_ACK:
                      if ( PS.Teridian_ACK )
                      {
                          PS.Teridian_ACK = FALSE;
                          Protect.U2TxDelay.Flag = 1;
#if TERIDIAN_CALI_RETRY
                          //ClearRetryTimer();
#endif
                          Uart.U2.Tx.Mode = STATE_ENABLE_AUTO_REPORTING;
                          Uart.U2.Tx.SubMode = STATE_SENDING_ENABLE_AUTO_REPORTING;
                      }
                      else
                      {
#if TERIDIAN_CALI_RETRY
                          //CheckRetryTimer(STATE_SENDING_UPDATE_FLASH_CE);
#endif
                      }
                      break;
              }
          }
              break;

          case STATE_ENABLE_AUTO_REPORTING:
          {
              switch ( Uart.U2.Tx.SubMode )
              {
                  case STATE_SENDING_ENABLE_AUTO_REPORTING:
                      Tx_Data[0] = 0x02;
                      if ( PS.U2TxSend )
                      {
                          if ( TeridianSpecialCMDWrite ( 0x6A, 1, &Tx_Data[0] ) == TRUE )
                          {
                              PS.U2TxSend = FALSE;
                              PS.Teridian_ACK = FALSE;
                              Protect.U2TxDelay.Flag = 0;
                              Uart.U2.Tx.SubMode = STATE_WAITING_ACK;
                          }
                      }
                      break;
                  case STATE_WAITING_ACK:
                      if ( PS.Teridian_ACK )
                      {
                          PS.Teridian_ACK = FALSE;
                          Protect.U2TxDelay.Flag = 1;
#if TERIDIAN_CALI_RETRY
                          ClearRetryTimer ( );
#endif
                          Uart.U2.Tx.SubMode = STATE_CALIBRATION_FINISH;
                      }
                      else
                      {
#if TERIDIAN_CALI_RETRY
                          CheckRetryTimer ( STATE_ENABLE_AUTO_REPORTING );
#endif
                      }
                      break;
                  case STATE_CALIBRATION_FINISH:
                      PS.IsTeridianCaliMode = FALSE;
                      break;
              }
          }
              break;

          default:
              break;
      }

  }
#endif

  

  //[Peter Chung] 20101026 added for reporting Tx status
  gPmbusCmd.CAL_TERIDIAN_FINISHED_NOTIFY[0] = Uart.U2.Tx.Mode;
  gPmbusCmd.CAL_TERIDIAN_FINISHED_NOTIFY[1] = Uart.U2.Tx.SubMode;
  gPmbusCmd.CAL_TERIDIAN_FINISHED_NOTIFY[2] = Uart.U2.Rx.AckState;
}


//************ Uart for Teridian IC End ***************************/


//[Peter Chung] UART END <-

BYTE TeridianSpecialCMDWrite ( BYTE cmd, BYTE byteCnt, BYTE* data )
{
  static BYTE state = SEND_SP_HD;
  static BYTE index = 0;
  BYTE temp;
  static BYTE checksum = 0;

  if ( U2STAbits.TRMT == 1 && U2STAbits.UTXBF == 0 )
  {
      switch ( state )
      {
          case SEND_SP_HD:
              temp = 0xA5;
              SendU2TxData ( temp );
              checksum = temp;
              state = SEND_SP_CMD;
              break;
          case SEND_SP_CMD:
              temp = cmd;
              SendU2TxData ( cmd );
              checksum += temp;
              state = SEND_SP_LEN;
              break;
          case SEND_SP_LEN:
              temp = byteCnt;
              SendU2TxData ( temp );
              checksum += temp;
              state = SEND_SP_DAT;
              break;
          case SEND_SP_DAT:
              if ( index < byteCnt )
              {
                  temp = data[index];
                  SendU2TxData ( temp );
                  checksum += temp;
                  index ++;
              }
              else
              {
                  state = SEND_SP_CS;
              }
              break;
          case SEND_SP_CS:
              checksum = ~ checksum + 1;
              SendU2TxData ( checksum );
              index = 0;
              temp = 0;
              state = SEND_SP_HD;
              return TRUE;
              break;
      }
      return FALSE;
  }
  return FALSE;
}

#if Config_78M6610   //[davidchchen]20160114 added
//vincent change
BYTE TeridianNormalCMDWrite (BYTE Header, BYTE byteCnt, BYTE* data )
{
  static BYTE state = SEND_HD;
  static BYTE index = 0;
  BYTE temp;
  static BYTE checksum = 0;

  if ( U2STAbits.TRMT == 1 && U2STAbits.UTXBF == 0 )
  {
      switch ( state )
      {
          case SEND_HD:
              temp = Header;
              SendU2TxData ( temp );
              checksum = temp;
              state = SEND_BYTECOUNT;
              break;
          case SEND_BYTECOUNT:
              temp = byteCnt;
              SendU2TxData ( temp );
              checksum += temp;
              state = SEND_DATA;
              break;
          case SEND_DATA:
              if ( index < (byteCnt - 3) )
              {
                  temp = data[index];
                  SendU2TxData ( temp );
                  checksum += temp;
                  index ++;
              }
              else
              {
                  state = SEND_CS;
              }
              break;
          case SEND_CS:
              checksum = ~ checksum + 1;
              SendU2TxData (checksum );
              index = 0;
              temp = 0;
              state = SEND_HD;
              return TRUE;
              break;
      }
      return FALSE;
  }
  return FALSE;
}

//add by vincent
BYTE Float_Conver_to_Decimal(BYTE* value, BYTE* Output)
{
    DWORD raw, sign,mantissa;
    CHAR exp;
    DWORD result;
    raw = ((DWORD)value[1]<<24) | ((DWORD)value[2]<<16) | ((DWORD)value[3] << 8 ) | (value[4]);
    sign = raw >> 31;                            // 0
    mantissa = (raw & 0x7FFFFF) | 0x800000;
    exp = ((raw >> 23) & 0xFF) - 127 - 23;
    result = (DWORD)(mantissa * pow(2.0, exp) * 1000);

    Output[0] = result;
    Output[1] = result >> 8;
    Output[2] = result >> 16;

    return 0;
}
#else
BYTE TeridianNormalCMDWrite ( BYTE cmd, BYTE payload, WORD addr, BYTE byteCnt, BYTE* data )
{
  static BYTE state = SEND_HD;
  static BYTE index = 0;
  BYTE temp;
  static BYTE checksum = 0;
  WORD_VAL address_temp;

  address_temp.Val = addr;

  if ( U2STAbits.TRMT == 1 && U2STAbits.UTXBF == 0 )
  {
      switch ( state )
      {
          case SEND_HD:
              temp = 0xA5;
              SendU2TxData ( temp );
              checksum = temp;
              state = SEND_CMD;
              break;
          case SEND_CMD:
              temp = cmd;
              SendU2TxData ( cmd );
              checksum += temp;
              state = SEND_PAYLOAD;
              break;
          case SEND_PAYLOAD:
              temp = payload;
              SendU2TxData ( temp );
              checksum += temp;
              state = SEND_REG_ADD_H;
              break;
          case SEND_REG_ADD_H:
              temp = address_temp.byte.HB;
              SendU2TxData ( temp );
              checksum += temp;
              state = SEND_REG_ADD_L;
              break;
          case SEND_REG_ADD_L:
              temp = address_temp.byte.LB;
              SendU2TxData ( temp );
              checksum += temp;
              state = SEND_BYTECOUNT;
              break;
          case SEND_BYTECOUNT:
              temp = byteCnt;
              SendU2TxData ( temp );
              checksum += temp;
              state = SEND_DATA;
              break;
          case SEND_DATA:
              if ( index < byteCnt )
              {
                  temp = data[index];
                  SendU2TxData ( temp );
                  checksum += temp;
                  index ++;
              }
              else
              {
                  state = SEND_CS;
              }
              break;
          case SEND_CS:
              checksum = ~ checksum + 1;
              SendU2TxData ( checksum );
              index = 0;
              temp = 0;
              state = SEND_HD;
              return TRUE;
              break;
      }
      return FALSE;
  }
  return FALSE;
}
#endif




