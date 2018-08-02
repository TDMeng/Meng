#include "recorder.h" 
#include "usart.h"
#include "wm8978.h"
#include "sai.h"
#include "led.h"
#include "delay.h"

extern u8 sairecbuf1[];
extern u8 sairecbuf2[];
extern u8 saiplaybuf1[];
extern u8 saiplaybuf2[];

extern u8 intFlag;
extern u16 ptr;

extern uint32_t orderFlag;

void rec_sai_dma_rx_callback(void) 
{  
	if(Get_DCahceSta())SCB_CleanInvalidateDCache();
	
	if(DMA2_Stream5->CR&(1<<19)){
		intFlag|=0x01;
	}
	else {
		intFlag|=0x04;
	}
}

void wav_recorder(void)
{ 

	WM8978_ADDA_Cfg(1,1);	
	WM8978_Input_Cfg(0,1,0);//change first paramer to 0 for closing the microphone input
	WM8978_Output_Cfg(1,0);
	WM8978_MIC_Gain(5);		
	WM8978_LINEIN_Gain(3);	
	WM8978_HPvol_Set(60,60);
	WM8978_SPKvol_Set(0);	
	WM8978_I2S_Cfg(2,0);	

  SAIA_Init(SAI_MODEMASTER_TX,SAI_CLOCKSTROBING_RISINGEDGE,SAI_DATASIZE_16);
  SAIB_Init(SAI_MODESLAVE_RX,SAI_CLOCKSTROBING_RISINGEDGE,SAI_DATASIZE_16);
	SAIA_SampleRate_Set(REC_SAMPLERATE);
	
	SAIA_TX_DMA_Init(saiplaybuf1,saiplaybuf2,SAI_RX_DMA_BUF_SIZE/2,1); 
		
  __HAL_DMA_DISABLE_IT(&SAI1_TXDMA_Handler,DMA_IT_TC);
	SAIA_RX_DMA_Init(sairecbuf1,sairecbuf2,SAI_RX_DMA_BUF_SIZE/2,1);
  sai_rx_callback=rec_sai_dma_rx_callback;
 	SAI_Play_Start();
	SAI_Rec_Start(); 	
	
}




















