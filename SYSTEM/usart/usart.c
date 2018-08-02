#include "usart.h"
#include "led.h"
#include "nfc.h"

#if SYSTEM_SUPPORT_OS
#include "includes.h"					
#endif

#define REVNBUFLEN 240

bool recvMsgFlag = FALSE;
u8 recvBuffer[REVNBUFLEN] = {0};
uint32_t recvBufLen = 0;

RECV_FIFO recvbuf;


#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

#pragma import(__use_no_semihosting)             

struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
 
void _sys_exit(int x) 
{ 
	x = x; 
} 

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the EVAL_COM1 and Loop until the end of transmission */
  HAL_UART_Transmit(&UART2_Handler, (uint8_t *)&ch, 1, 0xFFFF); 

  return ch;
}


#if EN_USART2_RX   
u8 USART_RX_BUF[USART_REC_LEN];   

u16 USART_RX_STA=0;    

u8 aRxBuffer[RXBUFFERSIZE];
UART_HandleTypeDef UART2_Handler;

void uart_init(u32 bound)
{	
	
	UART2_Handler.Instance=USART2;		
	UART2_Handler.Init.BaudRate=bound;				  
	UART2_Handler.Init.WordLength=UART_WORDLENGTH_8B; 
	UART2_Handler.Init.StopBits=UART_STOPBITS_1;	
	UART2_Handler.Init.Parity=UART_PARITY_NONE;		   
	UART2_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE; 
	UART2_Handler.Init.Mode=UART_MODE_TX_RX;		  
	HAL_UART_Init(&UART2_Handler);			
	
	HAL_UART_Receive_IT(&UART2_Handler, (u8 *)aRxBuffer, RXBUFFERSIZE);
	
	recvbuf.totleLen = 0;
	recvbuf.msgFlag = NO_MESSAGE_GET_FLAG;
}


void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
 
	GPIO_InitTypeDef GPIO_Initure;
	
	if(huart->Instance==USART2)
	{
		__HAL_RCC_GPIOA_CLK_ENABLE();		
		__HAL_RCC_USART2_CLK_ENABLE();
	
		GPIO_Initure.Pin=GPIO_PIN_2;	
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;	
		GPIO_Initure.Pull=GPIO_PULLUP;	
		GPIO_Initure.Speed=GPIO_SPEED_FAST;	
		GPIO_Initure.Alternate=GPIO_AF7_USART2;
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	 

		GPIO_Initure.Pin=GPIO_PIN_3;	
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	 
		
#if EN_USART2_RX
		__HAL_UART_ENABLE_IT(huart,UART_IT_RXNE);	
		HAL_NVIC_EnableIRQ(USART2_IRQn);	
		HAL_NVIC_SetPriority(USART2_IRQn,3,3);	
#endif	
	}

}

/*
*Receive data here
* 0xAA msg head
*/
void USART2_IRQHandler(void)                	
{
	u32 timeout=0;
	u8 res = 0;

	if((__HAL_UART_GET_FLAG(&UART2_Handler,UART_FLAG_RXNE)!=RESET))
	{
		HAL_UART_IRQHandler(&UART2_Handler);
		
		timeout = 0;
		while (HAL_UART_GetState(&UART2_Handler) != HAL_UART_STATE_READY)
		{
			timeout++;
			if(timeout>HAL_MAX_DELAY) break;		
		}
     
		timeout=0;
		while(HAL_UART_Receive_IT(&UART2_Handler, (u8 *)aRxBuffer, RXBUFFERSIZE) != HAL_OK)
		{
			timeout++;
			if(timeout>HAL_MAX_DELAY) break;	
		}
		
		res = aRxBuffer[0];
	
		//recv NFC data
		if (res == PHONECARDMARK && recvbuf.msgFlag == 0)
		{
			recvbuf.msgHead = res;
			recvbuf.totleLen++;
			recvbuf.msgFlag = RECV_MESSAGE_FLAG;
		}

		if (recvbuf.msgFlag == RECV_MESSAGE_FLAG && res != PHONECARDMARK)
		{	
			if (recvbuf.totleLen == 1)
			{
				recvbuf.msgLen = (u32)res;
				recvbuf.totleLen++;
			}
			else
			{
				recvbuf.msgData[recvbuf.totleLen - 2] = res;
				recvbuf.totleLen++;
				if (recvbuf.totleLen == (recvbuf.msgLen + 2))
				{
					recvMsgFlag = TRUE;			
				}
			}
		}

	}

} 
#endif	


//float l2l[50]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0.050028,0.028937,0.038473,0.04682,0.051918,0.051467,
//0.043236,0.025413,-0.0030325,-0.042057,-0.090362,-0.14538,-0.20342,-0.26006,-0.31054,-0.35038,-0.37591,
//3.8897,-0.37591,-0.35038,-0.31054,-0.26006,-0.20342,-0.14538,-0.090362,-0.042057,-0.0030325,0.025413,
//0.043236,0.051467,0.051918,0.04682,0.038473,0.028937,0.050028};
/*
* print float number
*/
void PrintFloat(float value)
{
    int tmp;
		int tmp1,tmp2,tmp3,tmp4,tmp5,tmp6,tmp7;
		if (value < 0)
		{
			tmp = (int)value;
			if (tmp <= -1)
			{
				tmp1=(int)((tmp-value)*10)%10;
				tmp2=(int)((tmp-value)*100)%10;
				tmp3=(int)((tmp-value)*1000)%10;
				tmp4=(int)((tmp-value)*10000)%10;
				tmp5=(int)((tmp-value)*100000)%10;
				tmp6=(int)((tmp-value)*1000000)%10;	
				tmp7=(int)((tmp-value)*10000000)%10;	
				printf("%d.%d%d%d%d%d%d%d\r",tmp,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6,tmp7);
			}
			else
			{
				tmp1=(int)((tmp-value)*10)%10;
				tmp2=(int)((tmp-value)*100)%10;
				tmp3=(int)((tmp-value)*1000)%10;
				tmp4=(int)((tmp-value)*10000)%10;
				tmp5=(int)((tmp-value)*100000)%10;
				tmp6=(int)((tmp-value)*1000000)%10;	
				tmp7=(int)((tmp-value)*10000000)%10;	
				printf("-%d.%d%d%d%d%d%d%d\r",tmp,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6,tmp7);
			}

		}
		else
		{
			tmp = (int)value;
			tmp1=(int)((value-tmp)*10)%10;
			tmp2=(int)((value-tmp)*100)%10;
			tmp3=(int)((value-tmp)*1000)%10;
			tmp4=(int)((value-tmp)*10000)%10;
			tmp5=(int)((value-tmp)*100000)%10;
			tmp6=(int)((value-tmp)*1000000)%10;	
			tmp7=(int)((value-tmp)*10000000)%10;	
			printf("%d.%d%d%d%d%d%d%d\r",tmp,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6,tmp7);
		}
}


