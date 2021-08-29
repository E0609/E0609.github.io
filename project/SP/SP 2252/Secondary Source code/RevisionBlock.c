/****************************************************************************
*	file	RevisionBlock.c
*	brief	pib table
*
*	author allen.lee
* 	version 1.0
*		-	2016/02/26: initial version by allen lee
*
****************************************************************************/
//#include "fsl_device_registers.h"
#include "RevisionBlock.h"
/****************************************************************************
*   Declared Structures & Variables
****************************************************************************/
sRevisionControlBlock_t   tsRevCtrlBlock;

//[davidchchen]20160304 added RCB
// must be set at 0x00014150 - 0x0001419F (80 bytes)
//__attribute__((section(".my_apprcb")))
const u8_t 	pu8RevCtrlBlock[RCB_SIZE] = {
    0x55, 0xAA, 0x11, 0xEE, 0x77, 0x88, 0xCC, 0x33,				// Start Pattern. FIXED
    (RCB_REV & 0x00FF),((RCB_REV & 0xFF00)>>0x08),       // RCB Revision

    (FWVER_A),
    (FWVER_B),
    ((FWVER_CC & 0xFF00)>>0x08), (FWVER_CC & 0x00FF),
    ((FWVER_DD & 0xFF00)>>0x08), (FWVER_DD & 0x00FF),
    ((FWVER_EE & 0xFF00)>>0x08), (FWVER_EE & 0x00FF),

    ((RVD1_AAAA & 0xFF000000)>>0x18), ((RVD1_AAAA & 0x00FF0000)>>0x10), ((RVD1_AAAA & 0x0000FF00)>>0x08), (RVD1_AAAA & 0x000000FF),
    ((RVD1_BBBB & 0xFF000000)>>0x18), ((RVD1_BBBB & 0x00FF0000)>>0x10), ((RVD1_BBBB & 0x0000FF00)>>0x08), (RVD1_BBBB & 0x000000FF),

    ((FWCC_4BYTE_1 & 0xFF000000)>>0x18), ((FWCC_4BYTE_1 & 0x00FF0000)>>0x10), ((FWCC_4BYTE_1 & 0x0000FF00)>>0x08), (FWCC_4BYTE_1 & 0x000000FF),
    ((FWCC_4BYTE_2 & 0xFF000000)>>0x18), ((FWCC_4BYTE_2 & 0x00FF0000)>>0x10), ((FWCC_4BYTE_2 & 0x0000FF00)>>0x08), (FWCC_4BYTE_2 & 0x000000FF),

    ((HWCC_2BYTE & 0xFF00)>>0x08), (HWCC_2BYTE & 0x00FF),

    ((RVD2_AAAA & 0xFF000000)>>0x18), ((RVD2_AAAA & 0x00FF0000)>>0x10), ((RVD2_AAAA & 0x0000FF00)>>0x08), (RVD2_AAAA & 0x000000FF),
    ((RVD2_BB & 0xFF00)>>0x08), (RVD2_BB & 0x00FF),

    ((DEVHWVER_4BYTE_1 & 0xFF000000)>>0x18), ((DEVHWVER_4BYTE_1 & 0x00FF0000)>>0x10), ((DEVHWVER_4BYTE_1 & 0x0000FF00)>>0x08), (DEVHWVER_4BYTE_1 & 0x000000FF),
    ((DEVHWVER_4BYTE_2 & 0xFF000000)>>0x18), ((DEVHWVER_4BYTE_2 & 0x00FF0000)>>0x10), ((DEVHWVER_4BYTE_2 & 0x0000FF00)>>0x08), (DEVHWVER_4BYTE_2 & 0x000000FF),

    ((RVD3_AAAA & 0xFF000000)>>0x18), ((RVD3_AAAA & 0x00FF0000)>>0x10), ((RVD3_AAAA & 0x0000FF00)>>0x08), (RVD3_AAAA & 0x000000FF),
    ((RVD3_BBBB & 0xFF000000)>>0x18), ((RVD3_BBBB & 0x00FF0000)>>0x10), ((RVD3_BBBB & 0x0000FF00)>>0x08), (RVD3_BBBB & 0x000000FF),

    ((RVD4_AAAA & 0xFF000000)>>0x18), ((RVD4_AAAA & 0x00FF0000)>>0x10), ((RVD4_AAAA & 0x0000FF00)>>0x08), (RVD4_AAAA & 0x000000FF),
    ((RVD4_BBBB & 0xFF000000)>>0x18), ((RVD4_BBBB & 0x00FF0000)>>0x10), ((RVD4_BBBB & 0x0000FF00)>>0x08), (RVD4_BBBB & 0x000000FF),

    (0x00FF &
     (  (RCB_REV & 0x00FF) + ((RCB_REV & 0xFF00)>>0x08)
	  + (FWVER_A)
	  + (FWVER_B)
	  + ((FWVER_CC & 0xFF00)>>0x08) + (FWVER_CC & 0x00FF)
	  + ((FWVER_DD & 0xFF00)>>0x08) + (FWVER_DD & 0x00FF)
	  + ((FWVER_EE & 0xFF00)>>0x08) + (FWVER_EE & 0x00FF)
	  + ((RVD1_AAAA & 0xFF000000)>>0x18) + ((RVD1_AAAA & 0x00FF0000)>>0x10) + ((RVD1_AAAA & 0x0000FF00)>>0x08) + (RVD1_AAAA & 0x000000FF)
	  + ((RVD1_BBBB & 0xFF000000)>>0x18) + ((RVD1_BBBB & 0x00FF0000)>>0x10) + ((RVD1_BBBB & 0x0000FF00)>>0x08) + (RVD1_BBBB & 0x000000FF)
	  + ((FWCC_4BYTE_1 & 0xFF000000)>>0x18) + ((FWCC_4BYTE_1 & 0x00FF0000)>>0x10) + ((FWCC_4BYTE_1 & 0x0000FF00)>>0x08) + (FWCC_4BYTE_1 & 0x000000FF)
	  + ((FWCC_4BYTE_2 & 0xFF000000)>>0x18) + ((FWCC_4BYTE_2 & 0x00FF0000)>>0x10) + ((FWCC_4BYTE_2 & 0x0000FF00)>>0x08) + (FWCC_4BYTE_2 & 0x000000FF)
	  + ((HWCC_2BYTE & 0xFF00)>>0x08) + (HWCC_2BYTE & 0x00FF)
	  + ((RVD2_AAAA & 0xFF000000)>>0x18) + ((RVD2_AAAA & 0x00FF0000)>>0x10) + ((RVD2_AAAA & 0x0000FF00)>>0x08) + (RVD2_AAAA & 0x000000FF)
	  + ((RVD2_BB & 0xFF00)>>0x08) + (RVD2_BB & 0x00FF)
	  + ((DEVHWVER_4BYTE_1 & 0xFF000000)>>0x18) + ((DEVHWVER_4BYTE_1 & 0x00FF0000)>>0x10) + ((DEVHWVER_4BYTE_1 & 0x0000FF00)>>0x08) + (DEVHWVER_4BYTE_1 & 0x000000FF)
	  + ((DEVHWVER_4BYTE_2 & 0xFF000000)>>0x18) + ((DEVHWVER_4BYTE_2 & 0x00FF0000)>>0x10) + ((DEVHWVER_4BYTE_2 & 0x0000FF00)>>0x08) + (DEVHWVER_4BYTE_2 & 0x000000FF)
	  + ((RVD3_AAAA & 0xFF000000)>>0x18) + ((RVD3_AAAA & 0x00FF0000)>>0x10) + ((RVD3_AAAA & 0x0000FF00)>>0x08) + (RVD3_AAAA & 0x000000FF)
	  + ((RVD3_BBBB & 0xFF000000)>>0x18) + ((RVD3_BBBB & 0x00FF0000)>>0x10) + ((RVD3_BBBB & 0x0000FF00)>>0x08) + (RVD3_BBBB & 0x000000FF)
	  + ((RVD4_AAAA & 0xFF000000)>>0x18) + ((RVD4_AAAA & 0x00FF0000)>>0x10) + ((RVD4_AAAA & 0x0000FF00)>>0x08) + (RVD4_AAAA & 0x000000FF)
	  + ((RVD4_BBBB & 0xFF000000)>>0x18) + ((RVD4_BBBB & 0x00FF0000)>>0x10) + ((RVD4_BBBB & 0x0000FF00)>>0x08) + (RVD4_BBBB & 0x000000FF)
     )
    ),
    ((0xFF00 &
	 (  (RCB_REV & 0x00FF) + ((RCB_REV & 0xFF00)>>0x08)
	  + (FWVER_A)
	  + (FWVER_B)
	  + ((FWVER_CC & 0xFF00)>>0x08) + (FWVER_CC & 0x00FF)
	  + ((FWVER_DD & 0xFF00)>>0x08) + (FWVER_DD & 0x00FF)
	  + ((FWVER_EE & 0xFF00)>>0x08) + (FWVER_EE & 0x00FF)
	  + ((RVD1_AAAA & 0xFF000000)>>0x18) + ((RVD1_AAAA & 0x00FF0000)>>0x10) + ((RVD1_AAAA & 0x0000FF00)>>0x08) + (RVD1_AAAA & 0x000000FF)
	  + ((RVD1_BBBB & 0xFF000000)>>0x18) + ((RVD1_BBBB & 0x00FF0000)>>0x10) + ((RVD1_BBBB & 0x0000FF00)>>0x08) + (RVD1_BBBB & 0x000000FF)
	  + ((FWCC_4BYTE_1 & 0xFF000000)>>0x18) + ((FWCC_4BYTE_1 & 0x00FF0000)>>0x10) + ((FWCC_4BYTE_1 & 0x0000FF00)>>0x08) + (FWCC_4BYTE_1 & 0x000000FF)
	  + ((FWCC_4BYTE_2 & 0xFF000000)>>0x18) + ((FWCC_4BYTE_2 & 0x00FF0000)>>0x10) + ((FWCC_4BYTE_2 & 0x0000FF00)>>0x08) + (FWCC_4BYTE_2 & 0x000000FF)
	  + ((HWCC_2BYTE & 0xFF00)>>0x08) + (HWCC_2BYTE & 0x00FF)
	  + ((RVD2_AAAA & 0xFF000000)>>0x18) + ((RVD2_AAAA & 0x00FF0000)>>0x10) + ((RVD2_AAAA & 0x0000FF00)>>0x08) + (RVD2_AAAA & 0x000000FF)
	  + ((RVD2_BB & 0xFF00)>>0x08) + (RVD2_BB & 0x00FF)
	  + ((DEVHWVER_4BYTE_1 & 0xFF000000)>>0x18) + ((DEVHWVER_4BYTE_1 & 0x00FF0000)>>0x10) + ((DEVHWVER_4BYTE_1 & 0x0000FF00)>>0x08) + (DEVHWVER_4BYTE_1 & 0x000000FF)
	  + ((DEVHWVER_4BYTE_2 & 0xFF000000)>>0x18) + ((DEVHWVER_4BYTE_2 & 0x00FF0000)>>0x10) + ((DEVHWVER_4BYTE_2 & 0x0000FF00)>>0x08) + (DEVHWVER_4BYTE_2 & 0x000000FF)
	  + ((RVD3_AAAA & 0xFF000000)>>0x18) + ((RVD3_AAAA & 0x00FF0000)>>0x10) + ((RVD3_AAAA & 0x0000FF00)>>0x08) + (RVD3_AAAA & 0x000000FF)
	  + ((RVD3_BBBB & 0xFF000000)>>0x18) + ((RVD3_BBBB & 0x00FF0000)>>0x10) + ((RVD3_BBBB & 0x0000FF00)>>0x08) + (RVD3_BBBB & 0x000000FF)
	  + ((RVD4_AAAA & 0xFF000000)>>0x18) + ((RVD4_AAAA & 0x00FF0000)>>0x10) + ((RVD4_AAAA & 0x0000FF00)>>0x08) + (RVD4_AAAA & 0x000000FF)
	  + ((RVD4_BBBB & 0xFF000000)>>0x18) + ((RVD4_BBBB & 0x00FF0000)>>0x10) + ((RVD4_BBBB & 0x0000FF00)>>0x08) + (RVD4_BBBB & 0x000000FF)
	 )
    )>>0x08),

    0x44, 0xBB, 0x22, 0xDD, 0x99, 0x66, 0x00, 0xFF
};

/****************************************************************************
*	name        : RevCtrlBlockFun
*	description :
*	return      : none
****************************************************************************/
void RevCtrlBlockFun(void)
{
	volatile const u8_t   *pu8RevCtrlBlockRom = &(pu8RevCtrlBlock[0]);
	u8_t    i, *pu8RevCtrlBlockRam = &(tsRevCtrlBlock.pu8StartPattern[0]);

	for(i=0;i<RCB_SIZE;i++)
	{
		*(pu8RevCtrlBlockRam) = *(pu8RevCtrlBlockRom);
		pu8RevCtrlBlockRom++;
		pu8RevCtrlBlockRam++;
	}
}












