#include "xv_tpg.h"

XV_tpg setup_tpg()
{
	//configure tpg
	XV_tpg ptpg;
	XV_tpg_Config *ptpg_config;

	ptpg_config = XV_tpg_LookupConfig(XPAR_V_TPG_0_DEVICE_ID);
	XV_tpg_CfgInitialize(&ptpg, ptpg_config, ptpg_config->BaseAddress);

	printf("Hello World\n\r");

	printf("TPG Initialization\r\n");

	u32 height,width,status;

	status = XV_tpg_IsReady(&ptpg);
	printf("Status %u \n\r", (unsigned int) status);
	status = XV_tpg_IsIdle(&ptpg);
	printf("Status %u \n\r", (unsigned int) status);
	XV_tpg_Set_height(&ptpg, (u32)1080);
	XV_tpg_Set_width(&ptpg, (u32)1920);
	height = XV_tpg_Get_height(&ptpg);
	width = XV_tpg_Get_width(&ptpg);
	XV_tpg_Set_colorFormat(&ptpg,XVIDC_CSF_YCRCB_422);
	XV_tpg_Set_maskId(&ptpg, 0x0);
	XV_tpg_Set_motionSpeed(&ptpg, 0x4);
	printf("info from tpg %u %u \n\r", (unsigned int)height, (unsigned int)width);
	XV_tpg_Set_bckgndId(&ptpg, XTPG_BKGND_SOLID_RED);
	status = XV_tpg_Get_bckgndId(&ptpg);
	printf("Status %x \n\r", (unsigned int) status);
	XV_tpg_EnableAutoRestart(&ptpg);
	XV_tpg_Start(&ptpg);
	status = XV_tpg_IsIdle(&ptpg);
	printf("Status %u \n\r", (unsigned int) status);
	return ptpg;
}

int start_movement(XV_tpg ptpg){
	XV_tpg_Set_ovrlayId(&ptpg, XTPG_BKGND_H_RAMP);
	XV_tpg_Set_boxSize(&ptpg , 50);
	XV_tpg_Set_boxColorR(&ptpg , 149);//Y
	XV_tpg_Set_boxColorG(&ptpg , 43);//U
	XV_tpg_Set_boxColorB(&ptpg , 21);//V
	XV_tpg_Set_motionSpeed(&ptpg, 2);
}