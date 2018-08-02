#ifndef __SAI_H
#define __SAI_H
#include "sys.h"


extern SAI_HandleTypeDef SAI1A_Handler;  
extern SAI_HandleTypeDef SAI1B_Handler;   
extern DMA_HandleTypeDef SAI1_TXDMA_Handler; 
extern DMA_HandleTypeDef SAI1_RXDMA_Handler; 

extern void (*sai_tx_callback)(void);		 
extern void (*sai_rx_callback)(void);	

void SAIA_Init(u32 mode,u32 cpol,u32 datalen);
void SAIB_Init(u32 mode,u32 cpol,u32 datalen);
void SAIA_DMA_Enable(void);
void SAIB_DMA_Enable(void);
u8 SAIA_SampleRate_Set(u32 samplerate);
void SAIA_TX_DMA_Init(u8* buf0,u8 *buf1,u16 num,u8 width);
void SAIA_RX_DMA_Init(u8* buf0,u8 *buf1,u16 num,u8 width);
void SAI_Play_Start(void); 
void SAI_Play_Stop(void);  
void SAI_Rec_Start(void);
void SAI_Rec_Stop(void);
#endif
