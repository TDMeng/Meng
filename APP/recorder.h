#ifndef __RECORDER_H
#define __RECORDER_H
#include "sys.h"

#define SAI_RX_DMA_BUF_SIZE    	64		//Set Buffer Size Here !!!

#define REC_SAMPLERATE			44100	   //Set Sample rate here!!!

void rec_sai_dma_rx_callback(void);
 
void wav_recorder(void);

#endif












