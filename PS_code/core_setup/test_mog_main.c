/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "sleep.h"
#include "xv_tpg.h"
#include "zed_iic.h"
//#include "zed_iic_axi.c"
#include "xvidc.h"
#include "xaxivdma.h"
#include "xbuild_gaussian.h"
#include "xaxidma.h"

unsigned int srcBuffer = (XPAR_PS7_DDR_0_S_AXI_BASEADDR  + 0x100000);

#define ADV7511_ADDR   0x72
#define CARRIER_HDMI_OUT_CONFIG_LEN  (40)
#define DMA_DEV_ID		XPAR_AXIDMA_0_DEVICE_ID
#define FRAME_SIZE (307200)
//ADV7511 Configuration

/*
 * The video input format of the ADV7511 is set to YCbCr, 16-bit, 4:2:2,
 * ID 1 (separate syncs), Style 1. The video output format is set to
 * YCbCr, 16-bit, 4:2:2, HDMI mode.
 */
#define ZC702_HDMI_CONFIG_LEN  16
Xuint8 zc702_hdmi_config[ZC702_HDMI_CONFIG_LEN][3] =
{
	{ADV7511_ADDR>>1,0x41, 0x10}, // Power Down Control
						//    R0x41[  6] = PowerDown = 0 (power-up)
	{ADV7511_ADDR>>1,0xD6, 0xC0}, // HPD Control
						//    R0xD6[7:6] = HPD Control = 11 (always high)
    {ADV7511_ADDR>>1,0x15, 0x01}, // Input YCbCr 4:2:2 with separate syncs
    {ADV7511_ADDR>>1,0x16, 0xB9}, // Output format 4:2:2, Input Color Depth = 8
                        //    R0x16[  7] = Output Video Format = 1 (4:2:2)
                        //    R0x16[5:4] = Input Video Color Depth = 11 (8 bits/color)
                        //    R0x16[3:2] = Input Video Style = 10 (style 1)
                        //    R0x16[  1] = DDR Input Edge = 0 (falling edge)
                        //    R0x16[  0] = Output Color Space = 1 (YCbCr)
    {ADV7511_ADDR>>1,0x48, 0x08}, // Video Input Justification
                        //    R0x48[8:7] = Video Input Justification = 01 (right justified)
    {ADV7511_ADDR>>1,0x55, 0x20}, // Set RGB in AVinfo Frame
                        //    R0x55[6:5] = Output Format = 01 (YCbCr)
    {ADV7511_ADDR>>1,0x56, 0x19}, // Aspect Ratio
                        //    R0x56[5:4] = Picture Aspect Ratio = 10 (16:9)
    					//    R0x56[5:4] = Picture Aspect Ratio = 01 (4:3)
                        //    R0x56[3:0] = Active Format Aspect Ratio = 1000 (Same as Aspect Ratio)
    {ADV7511_ADDR>>1,0x98, 0x03}, // ADI Recommended Write
    {ADV7511_ADDR>>1,0x9A, 0xE0}, // ADI Recommended Write
    {ADV7511_ADDR>>1,0x9C, 0x30}, // PLL Filter R1 Value
    {ADV7511_ADDR>>1,0x9D, 0x61}, // Set clock divide
    {ADV7511_ADDR>>1,0xA2, 0xA4}, // ADI Recommended Write
    {ADV7511_ADDR>>1,0xA3, 0xA4}, // ADI Recommended Write
    {ADV7511_ADDR>>1,0xAF, 0x06}, // HDMI/DVI Modes
                        //    R0xAF[  7] = HDCP Enable = 0 (HDCP disabled)
                        //    R0xAF[  4] = Frame Encryption = 0 (Current frame NOT HDCP encrypted)
                        //    R0xAF[3:2] = 01 (fixed)
                        //    R0xAF[  1] = HDMI/DVI Mode Select = 2 (HDMI Mode)
    {ADV7511_ADDR>>1,0xE0, 0xD0}, // Must be set to 0xD0 for proper operation
    {ADV7511_ADDR>>1,0xF9, 0x00}  // Fixed I2C Address (This should be set to a non-conflicting I2C address)
};

int main ()
{
	init_platform();
	int ret;
	XAxiVdma vdmaPtr;
	zed_iic_t hdmi_out_iic;
	XBuild_gaussian gaussian_ip;
	XV_tpg ptpg;
	XAxiDma AxiDma;
	print("Hello World\n\r");

	//set up the DMA which stores the MOG structure

	int Status = XST_SUCCESS;

	//configure tpg
	ptpg = setup_tpg();

	//configure DMA
	AxiDma = setup_dma();

	//init build gaussian core
	gaussian_ip = setup_mog();

	/* Calling the API to configure and start VDMA without frame counter interrupt*/
	ret = run_triple_frame_buffer(&vdmaPtr, XPAR_AXI_VDMA_0_DEVICE_ID, 320, 240, srcBuffer, 100, 0);
	if (ret != XST_SUCCESS) {
		xil_printf("Transfer of frames failed with error = %d\r\n",ret);
		return XST_FAILURE;
	} else {
		xil_printf("Transfer of frames started \r\n");
	}

	//HDMI output initialisation
	ret = zed_iic_axi_init(&hdmi_out_iic,"ZED HDMI I2C Controller", XPAR_AXI_IIC_0_BASEADDR);
	if ( !ret )
	{
		print( "ERROR : Failed to initialize IIC driver\n\r" );
		return -1;
	}

	//HDMI output initialisation
	Xuint8 num_bytes;
	int i;

	for ( i = 0; i < ZC702_HDMI_CONFIG_LEN; i++ )
	{
		//xil_printf( "[ZedBoard HDMI] IIC Write - Device = 0x%02X, Address = 0x%02X, Data = 0x%02X\n\r", carrier_hdmi_out_config[i][0]<<1, carrier_hdmi_out_config[i][1], carrier_hdmi_out_config[i][2] );
		num_bytes = hdmi_out_iic.fpIicWrite( &hdmi_out_iic, zc702_hdmi_config[i][0], zc702_hdmi_config[i][1], &(zc702_hdmi_config[i][2]), 1 );
		printf("Written %d to reg address %d\r\n", num_bytes, zc702_hdmi_config[i][1]);
	}

	//check monitor connected and wait to start movement
	int sleep_coutn = 0;
	u8 monitor_connected = 0;
	while(sleep_coutn < 10)
	{
			print("checking monitor\r\n");
			monitor_connected = check_hdmi_hpd_status(&hdmi_out_iic);
			if(monitor_connected)
			{
				print("HDMI Monitor connected\r\n");
			}
			else
			{
				print("No HDMI Monitor connected / Monitor Disconnected\r\n");
			}
			sleep(10);
			//}
			sleep_coutn +=2;
	}
	
	//3E register used to identify video format
	//Xuint8 buffer;
	//num_bytes = hdmi_out_iic.fpIicRead( &hdmi_out_iic, ADV7511_ADDR>>1, 0x3E, &buffer ,1);
	//print("Detected : %d\r\n", (int)buffer);
	start_movement(ptpg);

	cleanup_platform();
	return 0;
}
