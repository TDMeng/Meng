#include "sai.h"
#include "delay.h"

SAI_HandleTypeDef SAI1A_Handler;        
SAI_HandleTypeDef SAI1B_Handler;     
DMA_HandleTypeDef SAI1_TXDMA_Handler;
DMA_HandleTypeDef SAI1_RXDMA_Handler; 

void SAIA_Init(u32 mode,u32 cpol,u32 datalen)
{
    HAL_SAI_DeInit(&SAI1A_Handler);                 
    SAI1A_Handler.Instance=SAI1_Block_A;                
    SAI1A_Handler.Init.AudioMode=mode;                     
    SAI1A_Handler.Init.Synchro=SAI_ASYNCHRONOUS;           
    SAI1A_Handler.Init.OutputDrive=SAI_OUTPUTDRIVE_ENABLE;  
    SAI1A_Handler.Init.NoDivider=SAI_MASTERDIVIDER_ENABLE;  
    SAI1A_Handler.Init.FIFOThreshold=SAI_FIFOTHRESHOLD_1QF; 
    SAI1A_Handler.Init.MonoStereoMode=SAI_STEREOMODE;       
    SAI1A_Handler.Init.Protocol=SAI_FREE_PROTOCOL;         
    SAI1A_Handler.Init.DataSize=datalen;                
    SAI1A_Handler.Init.FirstBit=SAI_FIRSTBIT_MSB;          
    SAI1A_Handler.Init.ClockStrobing=cpol;            
    

    SAI1A_Handler.FrameInit.FrameLength=64;              
    SAI1A_Handler.FrameInit.ActiveFrameLength=32;         
    SAI1A_Handler.FrameInit.FSDefinition=SAI_FS_CHANNEL_IDENTIFICATION;
    SAI1A_Handler.FrameInit.FSPolarity=SAI_FS_ACTIVE_LOW;  
    SAI1A_Handler.FrameInit.FSOffset=SAI_FS_BEFOREFIRSTBIT; 

 
    SAI1A_Handler.SlotInit.FirstBitOffset=0;                
    SAI1A_Handler.SlotInit.SlotSize=SAI_SLOTSIZE_32B;      
    SAI1A_Handler.SlotInit.SlotNumber=2;                 
    SAI1A_Handler.SlotInit.SlotActive=SAI_SLOTACTIVE_0|SAI_SLOTACTIVE_1;
    
    HAL_SAI_Init(&SAI1A_Handler);                        
    __HAL_SAI_ENABLE(&SAI1A_Handler);                    
}

void SAIB_Init(u32 mode,u32 cpol,u32 datalen)
{
    HAL_SAI_DeInit(&SAI1B_Handler);                      
    SAI1B_Handler.Instance=SAI1_Block_B;                
    SAI1B_Handler.Init.AudioMode=mode;                   
    SAI1B_Handler.Init.Synchro=SAI_SYNCHRONOUS;          
    SAI1B_Handler.Init.OutputDrive=SAI_OUTPUTDRIVE_ENABLE; 
    SAI1B_Handler.Init.NoDivider=SAI_MASTERDIVIDER_ENABLE; 
    SAI1B_Handler.Init.FIFOThreshold=SAI_FIFOTHRESHOLD_1QF; 
    SAI1B_Handler.Init.MonoStereoMode=SAI_STEREOMODE;
    SAI1B_Handler.Init.Protocol=SAI_FREE_PROTOCOL;      
    SAI1B_Handler.Init.DataSize=datalen;                
    SAI1B_Handler.Init.FirstBit=SAI_FIRSTBIT_MSB;     
    SAI1B_Handler.Init.ClockStrobing=cpol;          
    

    SAI1B_Handler.FrameInit.FrameLength=64;        
    SAI1B_Handler.FrameInit.ActiveFrameLength=32;      
    SAI1B_Handler.FrameInit.FSDefinition=SAI_FS_CHANNEL_IDENTIFICATION;
    SAI1B_Handler.FrameInit.FSPolarity=SAI_FS_ACTIVE_LOW;   
    SAI1B_Handler.FrameInit.FSOffset=SAI_FS_BEFOREFIRSTBIT; 	

    
    SAI1B_Handler.SlotInit.FirstBitOffset=0;               
    SAI1B_Handler.SlotInit.SlotSize=SAI_SLOTSIZE_32B;      
    SAI1B_Handler.SlotInit.SlotNumber=2;               
    SAI1B_Handler.SlotInit.SlotActive=SAI_SLOTACTIVE_0|SAI_SLOTACTIVE_1;
    
    HAL_SAI_Init(&SAI1B_Handler);
    SAIB_DMA_Enable();                                  
    __HAL_SAI_ENABLE(&SAI1B_Handler);    
}



void HAL_SAI_MspInit(SAI_HandleTypeDef *hsai)
{
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_SAI1_CLK_ENABLE();    
    __HAL_RCC_GPIOE_CLK_ENABLE();          
    
   
    GPIO_Initure.Pin=GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;  
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;        
    GPIO_Initure.Pull=GPIO_PULLUP;           
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;      
    GPIO_Initure.Alternate=GPIO_AF6_SAI1;      
    HAL_GPIO_Init(GPIOE,&GPIO_Initure);     
}


const u16 SAI_PSC_TBL[][5]=
{
	{800 ,344,7,1,12},	//8Khz
	{1102,429,2,19,2},	//11.025Khz
	{1600,344,7, 1,6},	//16Khz
	{2205,429,2,19,1},	//22.05Khz
	{3200,344,7, 1,3},	//32Khz
	{4410,429,2,19,0},	//44.1Khz
	{4800,344,7, 1,2},	//48Khz
	{8820,271,2, 3,1},	//88.2Khz
	{9600,344,7, 1,1},	//96Khz
	{17640,271,2,3,0},	//176.4Khz
	{19200,344,7,1,0},	//192Khz
}; 


void SAIA_DMA_Enable(void)
{
    u32 tempreg=0;
    tempreg=SAI1_Block_A->CR1;  
	tempreg|=1<<17;					 
	SAI1_Block_A->CR1=tempreg;		  
}

void SAIB_DMA_Enable(void)
{
    u32 tempreg=0;
    tempreg=SAI1_Block_B->CR1;    
	tempreg|=1<<17;					 
	SAI1_Block_B->CR1=tempreg;		 
}


u8 SAIA_SampleRate_Set(u32 samplerate)
{   
    u8 i=0; 
    RCC_PeriphCLKInitTypeDef RCCSAI1_Sture;  
    
	for(i=0;i<(sizeof(SAI_PSC_TBL)/10);i++)
	{
		if((samplerate/10)==SAI_PSC_TBL[i][0])break;
	}
    if(i==(sizeof(SAI_PSC_TBL)/10))return 1;
    RCCSAI1_Sture.PeriphClockSelection=RCC_PERIPHCLK_SAI1;     
    RCCSAI1_Sture.Sai1ClockSelection=RCC_SAI1CLKSOURCE_PLLSAI;  
    RCCSAI1_Sture.PLLSAI.PLLSAIN=(u32)SAI_PSC_TBL[i][1];       
    RCCSAI1_Sture.PLLSAI.PLLSAIQ=(u32)SAI_PSC_TBL[i][2];    
    RCCSAI1_Sture.PLLSAIDivQ=SAI_PSC_TBL[i][3];               
    HAL_RCCEx_PeriphCLKConfig(&RCCSAI1_Sture);               
    
    __HAL_SAI_DISABLE(&SAI1A_Handler);                      
    SAI1A_Handler.Init.AudioFrequency=samplerate;        
    HAL_SAI_Init(&SAI1A_Handler);                           
    SAIA_DMA_Enable();                                     
    __HAL_SAI_ENABLE(&SAI1A_Handler);                    
    return 0;
}   


void SAIA_TX_DMA_Init(u8* buf0,u8 *buf1,u16 num,u8 width)
{ 
    u32 memwidth=0,perwidth=0;   
    switch(width)
    {
        case 0:       
            memwidth=DMA_MDATAALIGN_BYTE;
            perwidth=DMA_PDATAALIGN_BYTE;
            break;
        case 1:        
            memwidth=DMA_MDATAALIGN_HALFWORD;
            perwidth=DMA_PDATAALIGN_HALFWORD;
            break;
        case 2:      
            memwidth=DMA_MDATAALIGN_WORD;
            perwidth=DMA_PDATAALIGN_WORD;
            break;
            
    }
    __HAL_RCC_DMA2_CLK_ENABLE();                                    
    __HAL_LINKDMA(&SAI1A_Handler,hdmatx,SAI1_TXDMA_Handler);       
    SAI1_TXDMA_Handler.Instance=DMA2_Stream3;                        
    SAI1_TXDMA_Handler.Init.Channel=DMA_CHANNEL_0;               
    SAI1_TXDMA_Handler.Init.Direction=DMA_MEMORY_TO_PERIPH;     
    SAI1_TXDMA_Handler.Init.PeriphInc=DMA_PINC_DISABLE;           
    SAI1_TXDMA_Handler.Init.MemInc=DMA_MINC_ENABLE;              
    SAI1_TXDMA_Handler.Init.PeriphDataAlignment=perwidth;       
    SAI1_TXDMA_Handler.Init.MemDataAlignment=memwidth;            
    SAI1_TXDMA_Handler.Init.Mode=DMA_CIRCULAR;                  
    SAI1_TXDMA_Handler.Init.Priority=DMA_PRIORITY_HIGH;          
    SAI1_TXDMA_Handler.Init.FIFOMode=DMA_FIFOMODE_DISABLE;      
    SAI1_TXDMA_Handler.Init.MemBurst=DMA_MBURST_SINGLE;          
    SAI1_TXDMA_Handler.Init.PeriphBurst=DMA_PBURST_SINGLE;        
    HAL_DMA_DeInit(&SAI1_TXDMA_Handler);                         
    HAL_DMA_Init(&SAI1_TXDMA_Handler);	                       

    HAL_DMAEx_MultiBufferStart(&SAI1_TXDMA_Handler,(u32)buf0,(u32)&SAI1_Block_A->DR,(u32)buf1,num);
    __HAL_DMA_DISABLE(&SAI1_TXDMA_Handler);                       
    delay_us(10);                                             
    __HAL_DMA_ENABLE_IT(&SAI1_TXDMA_Handler,DMA_IT_TC);          
    __HAL_DMA_CLEAR_FLAG(&SAI1_TXDMA_Handler,DMA_FLAG_TCIF3_7);  
    HAL_NVIC_SetPriority(DMA2_Stream3_IRQn,0,0);                
    HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
}
 
void SAIA_RX_DMA_Init(u8* buf0,u8 *buf1,u16 num,u8 width)
{ 
    u32 memwidth=0,perwidth=0;  
    switch(width)
    {
        case 0:        
            memwidth=DMA_MDATAALIGN_BYTE;
            perwidth=DMA_PDATAALIGN_BYTE;
            break;
        case 1:       
            memwidth=DMA_MDATAALIGN_HALFWORD;
            perwidth=DMA_PDATAALIGN_HALFWORD;
            break;
        case 2:        
            memwidth=DMA_MDATAALIGN_WORD;
            perwidth=DMA_PDATAALIGN_WORD;
            break;
            
    }
    __HAL_RCC_DMA2_CLK_ENABLE();                                   
    __HAL_LINKDMA(&SAI1B_Handler,hdmarx,SAI1_RXDMA_Handler);     
    SAI1_RXDMA_Handler.Instance=DMA2_Stream5;                                 
    SAI1_RXDMA_Handler.Init.Channel=DMA_CHANNEL_0;                
    SAI1_RXDMA_Handler.Init.Direction=DMA_PERIPH_TO_MEMORY;     
    SAI1_RXDMA_Handler.Init.PeriphInc=DMA_PINC_DISABLE;            
    SAI1_RXDMA_Handler.Init.MemInc=DMA_MINC_ENABLE;                 
    SAI1_RXDMA_Handler.Init.PeriphDataAlignment=perwidth;           
    SAI1_RXDMA_Handler.Init.MemDataAlignment=memwidth;           
    SAI1_RXDMA_Handler.Init.Mode=DMA_CIRCULAR;                     
    SAI1_RXDMA_Handler.Init.Priority=DMA_PRIORITY_MEDIUM;          
    SAI1_RXDMA_Handler.Init.FIFOMode=DMA_FIFOMODE_DISABLE;         
    SAI1_RXDMA_Handler.Init.MemBurst=DMA_MBURST_SINGLE;           
    SAI1_RXDMA_Handler.Init.PeriphBurst=DMA_PBURST_SINGLE;         
    HAL_DMA_DeInit(&SAI1_RXDMA_Handler);                         
    HAL_DMA_Init(&SAI1_RXDMA_Handler);	                           
    
    HAL_DMAEx_MultiBufferStart(&SAI1_RXDMA_Handler,(u32)&SAI1_Block_B->DR,(u32)buf0,(u32)buf1,num);
    __HAL_DMA_DISABLE(&SAI1_RXDMA_Handler);                      
    delay_us(10);                                                   
    __HAL_DMA_CLEAR_FLAG(&SAI1_RXDMA_Handler,DMA_FLAG_TCIF1_5); 
    __HAL_DMA_ENABLE_IT(&SAI1_RXDMA_Handler,DMA_IT_TC);           
	
    HAL_NVIC_SetPriority(DMA2_Stream5_IRQn,0,1);                 
    HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);
}


void (*sai_tx_callback)(void);	
void (*sai_rx_callback)(void);	


void DMA2_Stream3_IRQHandler(void)
{   
    if(__HAL_DMA_GET_FLAG(&SAI1_TXDMA_Handler,DMA_FLAG_TCIF3_7)!=RESET)
    {
        __HAL_DMA_CLEAR_FLAG(&SAI1_TXDMA_Handler,DMA_FLAG_TCIF3_7);  
        if(sai_tx_callback!=NULL)sai_tx_callback();	
    }  											 
} 


void DMA2_Stream5_IRQHandler(void)
{  
    if(__HAL_DMA_GET_FLAG(&SAI1_RXDMA_Handler,DMA_FLAG_TCIF1_5)!=RESET)
    {
        __HAL_DMA_CLEAR_FLAG(&SAI1_RXDMA_Handler,DMA_FLAG_TCIF1_5);  
        if(sai_rx_callback!=NULL)sai_rx_callback();	
    }        
} 


void SAI_Play_Start(void)
{   	
    __HAL_DMA_ENABLE(&SAI1_TXDMA_Handler);
}

void SAI_Play_Stop(void)
{   
    __HAL_DMA_DISABLE(&SAI1_TXDMA_Handler);
} 

void SAI_Rec_Start(void)
{ 
    __HAL_DMA_ENABLE(&SAI1_RXDMA_Handler); 		
}

void SAI_Rec_Stop(void)
{   
    __HAL_DMA_DISABLE(&SAI1_RXDMA_Handler);
}
