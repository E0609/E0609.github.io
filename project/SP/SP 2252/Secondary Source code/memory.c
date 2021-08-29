/********************************************************************
 *
 * PIC24F Serial Bootloader
 *
 *********************************************************************
 * FileName:		memory.c
 * Dependencies: boot.c
 * Processor:	PIC24F Family
 * Compiler:		C30 v3.00 or later
 * Company:		Microchip Technology, Inc.
 *
 * Software License Agreement:
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the ?ompany? for its PICmicro?Microcontroller is intended and
 * supplied to you, the Company? customer, for use solely and
 * exclusively on Microchip PICmicro Microcontroller products. The
 * software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN ?S IS?CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *
 * File Description:
 *
 * Flash program memory read and write functions for use with
 * PIC24F Serial Bootloader.
 *
 * Change History:
 *
 * Author      	Revision #      Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Brant Ivey	1.00   			1-17-2008	Initial release with AN1157
 *********************************************************************/

#include "memory.h"
#include "p33Fxxxx.h"

/********************************************************************
; Function: 	void WriteMem(WORD cmd)
;
; PreCondition: Appropriate data written to latches with WriteLatch
;
; Input:    	cmd - type of memory operation to perform
;                               
; Output:   	None.
;
; Side Effects: 
;
; Overview: 	Write stored registers to flash memory
;*********************************************************************/
void WriteMem ( WORD cmd )
{
  NVMCON = cmd;

  //Disable Interrupts For 5 instructions
  asm volatile("disi #5" );
  __builtin_write_NVM ( );

  while ( NVMCONbits.WR == 1 )
      asm("clrwdt" ); //clear WDT 2008/12/23 02:49下午

}

/********************************************************************
; Function: 	void WriteLatch(WORD page, WORD addrLo, 
;				        		WORD dataHi, WORD dataLo)
;
; PreCondition: None.
;
; Input:    	page 	- upper byte of address
;				addrLo 	- lower word of address
;				dataHi 	- upper byte of data
;				addrLo	- lower word of data
;                               
; Output:   	None.
;
; Side Effects: TBLPAG changed
;
; Overview: 	Stores data to write in hardware latches
;*********************************************************************/
void WriteLatch ( WORD page, WORD addrLo, WORD dataHi, WORD dataLo )
{
  TBLPAG = page;

  __builtin_tblwtl ( addrLo, dataLo );
  __builtin_tblwth ( addrLo, dataHi );

}

/********************************************************************
; Function: 	DWORD ReadLatch(WORD page, WORD addrLo)
;
; PreCondition: None.
;
; Input:    	page 	- upper byte of address
;				addrLo 	- lower word of address
;                               
; Output:   	data 	- 32-bit data in W1:W0
;
; Side Effects: TBLPAG changed
;
; Overview: 	Read from location in flash memory
;*********************************************************************/
DWORD ReadLatch ( WORD page, WORD addrLo )
{
  DWORD_VAL temp;

  TBLPAG = page;

  temp.word.LW = __builtin_tblrdl ( addrLo );
  temp.word.HW = __builtin_tblrdh ( addrLo );

  return temp.Val;
}

/*********************************************************************
; Function: 	void ResetDevice(WORD addr);
;
; PreCondition: None.
;
; Input:    	addr 	- 16-bit address to vector to
;                               
; Output:   	None.
;
; Side Effects: None.
;
; Overview: 	used to vector to user code
;**********************************************************************/
void ResetDevice ( WORD addr )
{
  asm("goto %0" : : "r"( addr ) );
}

/********************************************************************
; Function: 	void Erase(WORD page, WORD addrLo, WORD cmd);
;
; PreCondition: None.
;
; Input:    	page 	- upper byte of address
;				addrLo 	- lower word of address
;				cmd		- type of memory operation to perform
;                               
; Output:   	None.
;
; Side Effects: TBLPAG changed
;
; Overview: 	Erases page of flash memory at input address
 *********************************************************************/
void Erase ( WORD page, WORD addrLo, WORD cmd )
{
  WORD temp;

  temp = TBLPAG;
  TBLPAG = page;

  NVMCON = cmd;

  __builtin_tblwtl ( addrLo, addrLo );

  //Disable Interrupts For 5 instructions
  asm volatile("disi #5" );
  __builtin_write_NVM ( );

  while ( NVMCONbits.WR == 1 )
      asm("clrwdt" ); //clear WDT 2008/12/23 02:49下午

  TBLPAG = temp;
}

/********************************************************************
 * Function:     void ReadFRU(WORD length, DWORD_VAL sourceAddr)
 *
 * PreCondition: None
 *
 * Input:		length		- number of instructions to read
 *				sourceAddr 	- address to read from
                                buffer      - output buffer
 *
 * Output:		None
 *
 * Side Effects:	Puts read instructions into buffer.
 *
 * Overview:		Reads from FRU, stores data into buffer.
 *
 * Note:			None
 ********************************************************************/
void ReadPM ( BYTE *buffer, WORD bytes, DWORD_VAL sourceAddr )
{
  WORD bytesRead = 0;
  DWORD_VAL temp;

  //Read length instructions from flash
  while ( bytesRead < bytes )
  {
      asm("clrwdt" ); //clear WDT 2008/12/23 02:49下午
      //read flash
      temp.Val = ReadLatch ( sourceAddr.word.HW, sourceAddr.word.LW );

      buffer[bytesRead + 0] = temp.v[0];   	//put read data onto
      buffer[bytesRead + 1] = temp.v[1];	//response buffer
      //buffer[bytesRead+2] = temp.v[2];
      //buffer[bytesRead+3] = temp.v[3];

      // 4 bytes per instruction: low word, high byte, phantom byte
      bytesRead += 2;

      sourceAddr.Val = sourceAddr.Val + 2;  //increment addr by 2
  }//end while(bytesRead < length*PM_INSTR_SIZE)
}//end ReadFRU(BYTE *buffer, WORD length, DWORD_VAL sourceAddr)

/********************************************************************
 * Function:     void WritePM(WORD length, DWORD_VAL sourceAddr)
 *
 * PreCondition: Page containing rows to write should be erased.
 *
 * Input:		length		- number of rows to write
 *				sourceAddr 	- row aligned address to write to
 *
 * Output:		None.
 *
 * Side Effects:	None.
 *
 * Overview:		Writes number of rows indicated from buffer into
 *				flash memory
 *
 * Note:			None
 ********************************************************************/
void WritePM ( BYTE* buffer, WORD length, DWORD_VAL sourceAddr )
{
  WORD bytesWritten;
  WORD index;
  DWORD_VAL data;

  bytesWritten = 0;	//first 5 buffer locations are cmd,len,addr
  index = 0;

  //write length rows to flash
  while ( ( bytesWritten ) < length * PM_ROW_SIZE * 2 )
  {
      asm("clrwdt" );

      //get data to write from buffer
      data.v[0] = buffer[index + 0];
      data.v[1] = buffer[index + 1];
      data.v[2] = 0x00;
      data.v[3] = 0x00;

      // 4 bytes per instruction: low word, high byte, phantom byte
      bytesWritten += PM_INSTR_SIZE;
      index += 2;

#ifdef USE_BOOT_PROTECT			//do not erase bootloader & reset vector
      if ( sourceAddr.Val < BOOT_ADDR_LOW || sourceAddr.Val > BOOT_ADDR_HI )
      {
#endif

#ifdef USE_CONFIGWORD_PROTECT	//do not erase last page
          if ( sourceAddr.Val < ( CONFIG_WORD_1 & 0xFFFC00 ) )
          {
#endif

#ifdef USE_VECTOR_PROTECT		//do not erase first page
              if ( sourceAddr.Val >= PM_PAGE_SIZE / 2 )
              {
#endif

                  //write data into latches
                  WriteLatch ( sourceAddr.word.HW, sourceAddr.word.LW,
                               data.word.HW, data.word.LW );

#ifdef USE_VECTOR_PROTECT	
              }  //end vectors protect
#endif

#ifdef USE_CONFIGWORD_PROTECT
          } //end config protect
#endif

#ifdef USE_BOOT_PROTECT
      } //end bootloader protect
#endif

      //write to flash memory if complete row is finished
      if ( ( bytesWritten % PM_ROW_SIZE ) == 0 )
      {

#ifdef USE_BOOT_PROTECT			//Protect the bootloader & reset vector
          if ( ( sourceAddr.Val < BOOT_ADDR_LOW || sourceAddr.Val > BOOT_ADDR_HI ) )
          {
#endif

#ifdef USE_CONFIGWORD_PROTECT	//do not erase last page
              if ( sourceAddr.Val < ( CONFIG_WORD_1 & 0xFFFC00 ) )
              {
#endif

#ifdef USE_VECTOR_PROTECT		//do not erase first page
                  if ( sourceAddr.Val >= 0x400 )
                  {
#endif

                      //execute write sequence
                      WriteMem ( PM_ROW_WRITE );

#ifdef USE_VECTOR_PROTECT	
                  }//end vectors protect
#endif

#ifdef USE_CONFIGWORD_PROTECT
              }//end config protect
#endif	

#ifdef USE_BOOT_PROTECT
          }//end boot protect
#endif

      }

      sourceAddr.Val = sourceAddr.Val + 2;  //increment addr by 2
  }//end while((bytesWritten-5) < length*PM_ROW_SIZE)
}//end WritePM(WORD length, DWORD_VAL sourceAddr)

/********************************************************************
 * Function:     void ErasePM(WORD length, DWORD_VAL sourceAddr)
 *
 * PreCondition:
 *
 * Input:		length		- number of pages to erase
 *				sourceAddr 	- page aligned address to erase
 *
 * Output:		None.
 *
 * Side Effects:	None.
 *
 * Overview:		Erases number of pages from flash memory
 *
 * Note:			None
 ********************************************************************/
void ErasePM ( WORD length, DWORD_VAL sourceAddr )
{
  WORD i = 0;

  while ( i < length )
  {
      asm("clrwdt" ); //clear WDT 2008/12/23 02:49下午
      i ++;

      //if protection enabled, protect BL and reset vector
#ifdef USE_BOOT_PROTECT		
      if ( sourceAddr.Val < BOOT_ADDR_LOW ||	//do not erase bootloader
           sourceAddr.Val > BOOT_ADDR_HI )
      {
#endif

#ifdef USE_CONFIGWORD_PROTECT		//do not erase last page
          if ( sourceAddr.Val < ( CONFIG_WORD_1 & 0xFFFC00 ) )
          {
#endif

#ifdef USE_VECTOR_PROTECT			//do not erase first page
              if ( sourceAddr.Val >= PM_PAGE_SIZE / 2 )
              {
#endif

                  //perform erase
                  Erase ( sourceAddr.word.HW, sourceAddr.word.LW, PM_PAGE_ERASE );

#ifdef USE_VECTOR_PROTECT	
              }//end vectors protect

#elif  defined(USE_BOOT_PROTECT)
#if 0
                  //Replace the bootloader reset vector
                  DWORD_VAL blResetAddr;

                  if ( sourceAddr.Val < PM_PAGE_SIZE / 2 )
                  {

                      //Replace BL reset vector at 0x00 and 0x02 if erased
                      blResetAddr.Val = 0;

                      replaceBLReset ( blResetAddr );

                  }
#endif
#endif

#ifdef USE_CONFIGWORD_PROTECT
          }//end config protect
#endif

#ifdef USE_BOOT_PROTECT
      }//end bootloader protect
#endif


      sourceAddr.Val += PM_PAGE_SIZE / 2;	//increment by a page

  }//end while(i<length)
}//end ErasePM
