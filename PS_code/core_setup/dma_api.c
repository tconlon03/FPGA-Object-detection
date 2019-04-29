#include "xaxidma.h"

#define MEM_BASE_ADDR 0x01000000
#define TX_BUFFER_BASE (MEM_BASE_ADDR + 0X00100000)
//size of mog 320 * 480 = 0x960000
#define RX_BUFFER_BASE (MEM_BASE_ADDR + 0X02000000)
#define DMA_DEV_ID		XPAR_AXIDMA_0_DEVICE_ID
#define FRAME_SIZE (76800)

XAxiDma setup_dma()
{
	XAxiDma AxiDma;
	XAxiDma_Config *CfgPtr;

	//set up the DMA which stores the MOG structure
	int Status = XST_SUCCESS;

	CfgPtr = XAxiDma_LookupConfig(DMA_DEV_ID);
	if (!CfgPtr) {
		return XST_FAILURE;
	}

	Status = XAxiDma_CfgInitialize(&AxiDma, CfgPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

	//int *m_dma_buffer_TX = (int*) TX_BUFFER_BASE;
	int *m_dma_buffer_RX = (int*) RX_BUFFER_BASE;

	Status = XAxiDma_Selftest(&AxiDma); //reset
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	//Zero out 128 bits of memory
	int zero[4] = {0,0,0,0};

	Xil_DCacheFlushRange((u32)zero, 4 * sizeof(int));
	Xil_DCacheFlushRange((u32)m_dma_buffer_RX, 4 * sizeof(int));
	//setting MOG to zeros and read to confirm
	for (int i = 0; i < FRAME_SIZE; i++){
		XAxiDma_SimpleTransfer(&AxiDma, (u32)zero, 4*sizeof(int), XAXIDMA_DMA_TO_DEVICE);
		XAxiDma_SimpleTransfer(&AxiDma, (u32)m_dma_buffer_RX , 4 * sizeof(int), XAXIDMA_DEVICE_TO_DMA);
		while(XAxiDma_Busy(&AxiDma,XAXIDMA_DEVICE_TO_DMA));
	}
	//flush cache before reading
	Xil_DCacheInvalidateRange((u32)m_dma_buffer_RX,  FRAME_SIZE * 4 * sizeof(int));
	for (int i = 0; i < 4; i+=4){
		printf("Recv[%d]=%d,%d,%d,%d\n", i, m_dma_buffer_RX[i],m_dma_buffer_RX[i+1],m_dma_buffer_RX[i+2],m_dma_buffer_RX[i+3]);
	}
	return 0;
}
