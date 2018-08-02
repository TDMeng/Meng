#ifndef _USART_H
#define _USART_H
#include "sys.h"
#include "stdio.h"	
	
#define USART_REC_LEN  			200  	
#define EN_USART2_RX 			1		

typedef enum
{
	NO_MESSAGE_GET_FLAG = 0,
	RECV_MESSAGE_FLAG
}MSGFLAG;

typedef struct
{
	u8 msgHead;
	u8 msgLen;
	u8 msgData[240];
	u8 msgType;
	uint32_t totleLen;
	MSGFLAG msgFlag;
}RECV_FIFO;			
			
extern u8  USART_RX_BUF[USART_REC_LEN]; 
extern u16 USART_RX_STA;       
extern UART_HandleTypeDef UART2_Handler; 
extern bool recvMsgFlag;
extern u8 recvBuffer[240];
extern u8 phoneDataBuffer[30];
extern uint32_t recvBufLen;
extern u8 activeCPUcmd[3];
extern u8 dataType;
extern RECV_FIFO recvbuf;

#define RXBUFFERSIZE   1 
extern u8 aRxBuffer[RXBUFFERSIZE];

void uart_init(u32 bound);
void PrintFloat(float value);
#endif
