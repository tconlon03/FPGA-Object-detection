#include "xbuild_gaussian.h"

XBuild_gaussian setup_mog ()
{
	XBuild_gaussian gaussian_ip;
	XBuild_gaussian_Config* gaussian_ptr;
	
	//init build gaussian core
	gaussian_ptr = XBuild_gaussian_LookupConfig(XPAR_BUILD_GAUSSIAN_0_DEVICE_ID);
	if (!gaussian_ptr) {
		return XST_FAILURE;
	}
	Status = XBuild_gaussian_CfgInitialize(&gaussian_ip, gaussian_ptr );
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	//set input parameters
	XBuild_gaussian_Set_bg_thresh_V(&gaussian_ip, 0.96);
	XBuild_gaussian_Set_learning_rate_V(&gaussian_ip, 0.005);
	XBuild_gaussian_Set_min_var(&gaussian_ip, 4);
	XBuild_gaussian_EnableAutoRestart(&gaussian_ip);

	//Start the core
	XBuild_gaussian_Start(&gaussian_ip);
	Status = XBuild_gaussian_IsIdle(&gaussian_ip);
	printf("Gaussian Idle Status %u \n\r", (unsigned int) Status);
	Status = XBuild_gaussian_IsReady(&gaussian_ip);
	printf("Gaussian Idle Status %u \n\r", (unsigned int) Status);
	return gaussian_ip;
}
