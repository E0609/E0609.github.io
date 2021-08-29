/*****************************************************************************************
 *
 *Copyright (C) 2012 Advanced Technology Development
 *                   Power SBG
 *                   LITE-ON TECHNOLOGY Corp.
 *
 *All Rights Reserved
 *
 *File Name : Parameter
 *
 *Date : 2012.05.24
 *
 *Author :
 *
 *Description :This Program used for HP 2400W Parameter Program.
 *
 *******************************************************************************************/
#include "Parameter.h"
#include "Sci.h"
#include "Process.h"        //[davidchchen]20170119 Modify Primary Temp Report Issue
#include "I2C.h"            //[davidchchen]20170216 added PSON Signal Enable/disable
#include "Protection.h"     //[davidchchen]20170216 added PSON Signal Enable/disable
tPARAMETERS_VAL Parameter;
extern tPS_FLAG PS;         //[davidchchen]20160408 added , debug for DC_Brown Issue

//-------------------------- init_Parameter -----------------------------------------------

void init_Parameter ( )
{
  Parameter.OTP_Tpri_FAULT_LIMIT = DEFAULT_TPRI_FAULT;
  Parameter.OTP_Tpri_WARN_LIMIT = DEFAULT_TPRI_WARN;
  Parameter.OTP_Tsec_FAULT_LIMIT = DEFAULT_TSEC_FAULT;
  Parameter.OTP_Tsec_WARN_LIMIT = DEFAULT_TSEC_WARN;
  Parameter.OTP_Tinlet_FAULT_LIMIT = DEFAULT_TINLET_FAULT;
  Parameter.OTP_Tinlet_WARN_LIMIT = DEFAULT_TINLET_WARN;
  Parameter.OTP_Tpri_Recovery_LIMIT = DEFAULT_TPRI_OT_RECOVERY;
  Parameter.OTP_Tsec_Recovery_LIMIT = DEFAULT_TSEC_OT_RECOVERY;
  Parameter.OTP_Tinlet_Recovery_LIMIT = DEFAULT_TINLET_OT_RECOVERY;	//Chris, for thermal request

  Parameter.CCM_IOUT_FAULT_HLIMIT = Io_Slave_HREF;   //[davidchchen]20150731 Added, 30A
  Parameter.CCM_IOUT_FAULT_LLIMIT = Io_Slave_LREF;   //[davidchchen]20150731 Added, 10A

  Parameter.CCM_VOUT_COMP_HLIMIT = VoutComp_HREF;   //[davidchchen]20150731 Added, 60A
  Parameter.CCM_VOUT_COMP_LLIMIT = VoutComp_LREF;   //[davidchchen]20150731 Added, 10A

  Parameter.OCP_IOUT_OC_FAULT_LIMIT = OCP_H_REF;
  Parameter.OCP_IOUT_OC_WARN_LIMIT = OCW_H_REF;
  Parameter.SCP_FAULT_LIMIT = ( WORD ) ( ( ( DWORD ) DEFAULT_SC_FAULT * IOUT_TO_ADC ) >> IOUT_TO_ADC_GAIN );
  Parameter.OPP_POUT_OP_WARN_LIMIT = DEFAULT_POUT_OP_WARN;
  Parameter.IIN_OC_FAULT_LL_LIMIT = DEFAULT_IIN_OC_FAULT_LL;
  Parameter.IIN_OC_FAULT_HL_LIMIT = DEFAULT_IIN_OC_FAULT_HL;
  Parameter.IIN_OC_WARN_HL_LIMIT = DEFAULT_IIN_OC_WARN_HL;	//Peter
  //Parameter.IIN_OC_WARN_LL_LIMIT = DEFAULT_IIN_OC_WARN_LL;	//Peter
  Parameter.PIN_OP_WARN_LIMIT = DEFAULT_PIN_OP_WARN;			//Peter

  Parameter.OVP_VOUT_OV_FAULT_LIMIT = OVP_REF;
  Parameter.OVP_VOUT_OV_WARN_LIMIT = OVW_REF;
  Parameter.UVP_VOUT_UV_WARN_LIMIT = UVW_REF;
  Parameter.UVP_VOUT_UV_FAULT_LIMIT = UV_L_REF;

  Parameter.VOUT_RSENSE_VREFL = ( WORD ) ( ( ( DWORD ) 1250 * 2048 / 100 ) / ADC_TO_VOUT );
  Parameter.VOUT_RSENSE_VREFH = ( WORD ) ( ( ( DWORD ) 1255 * 2048 / 100 ) / ADC_TO_VOUT );

  Parameter.MFR_VIN_MAX = DEFAULT_MFR_VIN_MAX_AC;
  Parameter.MFR_VIN_MIN = DEFAULT_MFR_VIN_MIN_AC;

#if 0
  if ( Uart.U1.Rx.Pri_Status.bits.DC_INPUT == 0 )
  {
      Parameter.VIN_OV_WARN_LIMIT[0] = DEFAULT_VIN_OV_WARN_AC;
      Parameter.VIN_OV_FAULT_LIMIT[0] = DEFAULT_VIN_OV_FAULT_AC;
      Parameter.VIN_OV_FAULT_RECOVER_LIMIT[0] = DEFAULT_VIN_OV_RECOVER_AC;
      Parameter.VIN_UV_WARN_LIMIT[0] = DEFAULT_VIN_UV_WARN_AC;
      Parameter.VIN_UV_FAULT_LIMIT[0] = DEFAULT_VIN_UV_FAULT_AC;
      Parameter.VIN_UV_FAULT_RECOVER_LIMIT[0] = DEFAULT_VIN_UV_RECOVER_AC;
  }
  else
  {
      Parameter.VIN_OV_WARN_LIMIT[1] = DEFAULT_VIN_OV_WARN_DC;
      Parameter.VIN_OV_FAULT_LIMIT[1] = DEFAULT_VIN_OV_FAULT_DC;
      Parameter.VIN_OV_FAULT_RECOVER_LIMIT[1] = DEFAULT_VIN_OV_RECOVER_DC;
      Parameter.VIN_UV_WARN_LIMIT[1] = DEFAULT_VIN_UV_WARN_DC;
      Parameter.VIN_UV_FAULT_LIMIT[1] = DEFAULT_VIN_UV_FAULT_DC;
      Parameter.VIN_UV_FAULT_RECOVER_LIMIT[1] = DEFAULT_VIN_UV_RECOVER_DC;
  }
#endif

  //AC Input
  Parameter.VIN_OV_WARN_LIMIT[0] = DEFAULT_VIN_OV_WARN_AC;
  Parameter.VIN_OV_FAULT_LIMIT[0] = DEFAULT_VIN_OV_FAULT_AC;
  Parameter.VIN_OV_FAULT_RECOVER_LIMIT[0] = DEFAULT_VIN_OV_RECOVER_AC;
  Parameter.VIN_UV_WARN_LIMIT[0] = DEFAULT_VIN_UV_WARN_AC;
  Parameter.VIN_UV_FAULT_LIMIT[0] = DEFAULT_VIN_UV_FAULT_AC;
  Parameter.VIN_UV_FAULT_RECOVER_LIMIT[0] = DEFAULT_VIN_UV_RECOVER_AC;

  //DC Input
  Parameter.VIN_OV_WARN_LIMIT[1] = DEFAULT_VIN_OV_WARN_DC;
  Parameter.VIN_OV_FAULT_LIMIT[1] = DEFAULT_VIN_OV_FAULT_DC;
  Parameter.VIN_OV_FAULT_RECOVER_LIMIT[1] = DEFAULT_VIN_OV_RECOVER_DC;
  Parameter.VIN_UV_WARN_LIMIT[1] = DEFAULT_VIN_UV_WARN_DC;
  Parameter.VIN_UV_FAULT_LIMIT[1] = DEFAULT_VIN_UV_FAULT_DC;
  Parameter.VIN_UV_FAULT_RECOVER_LIMIT[1] = DEFAULT_VIN_UV_RECOVER_DC;

  PS.VinUVFault = 1;        //[davidchchen]20160408 added , debug for DC_Brown Issue
  T_Pri = 25;       //[davidchchen]20170119 Modify Primary Temp Report Issue
  T_Sec =25;        //[davidchchen]20170119 Modify Primary Temp Report Issue
  T_Inlet =25;      //[davidchchen]20170119 Modify Primary Temp Report Issue

  Protect.HotPlugAddr.Flag = 1;     //[davidchchen]20170216 added PSON Signal Enable/disable
  OneTimeFlag = 0;    //[davidchchen]20170216 added PSON Signal Enable/disable
}


