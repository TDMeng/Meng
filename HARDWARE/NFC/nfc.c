#include "nfc.h"
#include "usart.h"
#include "led.h"

u8 activeCPUcmd[3] = {0xAA, 0x01, 0x15};
u8 closeAutoSearch[6] = {0xAA,0x04,0x95,0x00,0x14,0x02};

/*this buffer is the comond for asking fliter paramers from phone*/
u8 DataCmd0[14] = {0xAA,0x0C,0x17,0x00,0xA4,0x04,0xFE,0x06,0x11,0x22,0x33,0x44,0x55,0x66};//ask phone card for user paramer 
u8 DataCmd1[14] = {0xAA,0x0C,0x17,0x00,0xA4,0x04,0xFF,0x06,0x11,0x22,0x33,0x44,0x55,0x66};//respone phone data recv successfully

u8 DataCmd2[14] = {0xAA,0x0C,0x17,0x00,0xA4,0x04,0x02,0x06,0x11,0x22,0x33,0x44,0x55,0x66};
u8 DataCmd3[14] = {0xAA,0x0C,0x17,0x00,0xA4,0x04,0x03,0x06,0x11,0x22,0x33,0x44,0x55,0x66};
u8 DataCmd4[14] = {0xAA,0x0C,0x17,0x00,0xA4,0x04,0x04,0x06,0x11,0x22,0x33,0x44,0x55,0x66};
u8 DataCmd5[14] = {0xAA,0x0C,0x17,0x00,0xA4,0x04,0x05,0x06,0x11,0x22,0x33,0x44,0x55,0x66};
u8 DataCmd6[14] = {0xAA,0x0C,0x17,0x00,0xA4,0x04,0x06,0x06,0x11,0x22,0x33,0x44,0x55,0x66};
u8 DataCmd7[14] = {0xAA,0x0C,0x17,0x00,0xA4,0x04,0x07,0x06,0x11,0x22,0x33,0x44,0x55,0x66};
u8 DataCmd8[14] = {0xAA,0x0C,0x17,0x00,0xA4,0x04,0x08,0x06,0x11,0x22,0x33,0x44,0x55,0x66};
u8 DataCmd9[14] = {0xAA,0x0C,0x17,0x00,0xA4,0x04,0x09,0x06,0x11,0x22,0x33,0x44,0x55,0x66};
u8 DataCmd10[14] = {0xAA,0x0C,0x17,0x00,0xA4,0x04,0x0A,0x06,0x11,0x22,0x33,0x44,0x55,0x66};
u8 DataCmd11[14] = {0xAA,0x0C,0x17,0x00,0xA4,0x04,0x0B,0x06,0x11,0x22,0x33,0x44,0x55,0x66};
u8 DataCmd12[14] = {0xAA,0x0C,0x17,0x00,0xA4,0x04,0x0C,0x06,0x11,0x22,0x33,0x44,0x55,0x66};
u8 DataCmd13[14] = {0xAA,0x0C,0x17,0x00,0xA4,0x04,0x0D,0x06,0x11,0x22,0x33,0x44,0x55,0x66};
u8 DataCmd14[14] = {0xAA,0x0C,0x17,0x00,0xA4,0x04,0x0E,0x06,0x11,0x22,0x33,0x44,0x55,0x66};
u8 DataCmd15[14] = {0xAA,0x0C,0x17,0x00,0xA4,0x04,0x0F,0x06,0x11,0x22,0x33,0x44,0x55,0x66};
u8 DataCmd16[14] = {0xAA,0x0C,0x17,0x00,0xA4,0x04,0x10,0x06,0x11,0x22,0x33,0x44,0x55,0x66};


bool phoneFindFlag = FALSE;
bool phoneCardFlag = FALSE;
bool fliterChangeFlag = FALSE;


//save all the 16 buffers' datas from phone
int fliterParameter_Int[FLITERBUFFERINDEX][FLITERPARAMERSIZE] = {0}; // NO USE
float fliterParameter_Float[FLITERBUFFERINDEX][FLITERPARAMERSIZE] = {0};

float fliterInit_Float[16] = {0};

//float left_diff[8] = {0};
//float right_diff[8] = {0};

float left_diff[8] = {-12.0, -12.0, 10.0, -18, -20, 12.8, -13.2, -13.2};
float right_diff[8] = {-6.2, -6.2, 10, -5.1, 3.5, 14.2, -6.3, -6.3};

//save the differ data from phone
int DBDifference = 0;

u8 fliterBufferIndex = 0;
uint32_t timerCount = 0;
uint32_t fliterParamerOrder = 0;

/*
* the NFC device return check
* no use
*/
uint32_t NFC_Card_check(u8 passwd)
{
	u8 passwdTmp;
	passwdTmp = ~passwd;
	
	if (passwdTmp == 1)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

/*
* NFC search phone card
* send command for activing the CPU card reader function
* maybe need some delay for NFC device return checksum
*/
void NFC_Get_Phone_Card(void)
{
	HAL_UART_Transmit(&UART2_Handler, activeCPUcmd, 3, 5000);
}

/*
* trans a buffer(AID) to NFC card 
*/
void NFC_Get_Phone_data(u8 order)
{
	switch(order)
	{
		case 0:
			HAL_UART_Transmit(&UART2_Handler, DataCmd0, 14, 5000);
			break;
		case 1:
			HAL_UART_Transmit(&UART2_Handler, DataCmd1, 14, 5000);
			break;
		case 2:
			HAL_UART_Transmit(&UART2_Handler, DataCmd2, 14, 5000);
			break;
		case 3:
			HAL_UART_Transmit(&UART2_Handler, DataCmd3, 14, 5000);
			break;
		case 4:
			HAL_UART_Transmit(&UART2_Handler, DataCmd4, 14, 5000);
			break;
		case 5:
			HAL_UART_Transmit(&UART2_Handler, DataCmd5, 14, 5000);
			break;
		case 6:
			HAL_UART_Transmit(&UART2_Handler, DataCmd6, 14, 5000);
			break;
		case 7:
			HAL_UART_Transmit(&UART2_Handler, DataCmd7, 14, 5000);
			break;
		case 8:
			HAL_UART_Transmit(&UART2_Handler, DataCmd8, 14, 5000);
			break;
		case 9:
			HAL_UART_Transmit(&UART2_Handler, DataCmd9, 14, 5000);
			break;
		case 0x0A:
			HAL_UART_Transmit(&UART2_Handler, DataCmd10, 14, 5000);
			break;
		case 0x0B:
			HAL_UART_Transmit(&UART2_Handler, DataCmd11, 14, 5000);
			break;
		case 0x0C:
			HAL_UART_Transmit(&UART2_Handler, DataCmd12, 14, 5000);
			break;
		case 0x0D:
			HAL_UART_Transmit(&UART2_Handler, DataCmd13, 14, 5000);
			break;
		case 0x0E:
			HAL_UART_Transmit(&UART2_Handler, DataCmd14, 14, 5000);
			break;
		case 0x0F:
			HAL_UART_Transmit(&UART2_Handler, DataCmd15, 14, 5000);
			break;
		case 0x10:
			HAL_UART_Transmit(&UART2_Handler, DataCmd16, 14, 5000);
			break;
		default:
			break;
	}
}

/*
* get the msg type for distinguishing msg function
*/
u8 GetMsgType(RECV_FIFO msg)
{
	return msg.msgData[0];
}
	
/*
* NFC device init
* we need to close the auto card searching function for phone card using
* flow: 
* 1. comfirm auto card searching function is closed
* 2. send "0xAA 0x01 0x15" to active the NFC device for phone card, return's "0xAA 0x01 0xFE" is actived successfully
* 3. send "AID" for taking data cmd
*/
void NFC_init(void)
{
	HAL_UART_Transmit(&UART2_Handler, closeAutoSearch, 6, 5000);
}

 
/*
* For data change function
* paramer IN u8 *srcBuffer        recvbuf from NFC card(with msg type)
* paramer IN uint32_t length      recvbuf length from NFC card 
* paramer INOUT float *destData   fliterParamer buffer for arithmetic active
*/
void NFC_GetThefloatFliterParamerBuffer(u8 *srcBuffer, uint32_t length, float *destData)
{
	uint32_t i = 0;
	uint32_t j = 0;
	uint32_t k = 0;
	u8 tmpBuffer[4] = {0};
	
	for (i = 2, j = 0; i < length; i++)
	{
		if(j < 4)
		{
			tmpBuffer[j] = srcBuffer[i];
			j++;
		}
		
		if ((i - 1) % 4 == 0)
		{
			j = 0;
			ChangeByteToFloat(tmpBuffer,&destData[k]);
			k++;
		}
		
	}
}

/*
* chang byte to floate, change four byte to one float
* for example: {00 00 80 3F} -> 1.000, {00 00 C0 3F} -> 1.500 
*/
void ChangeByteToFloat(u8 *srcByte, float *destFloat)
{
	uint32_t i = 0;
	u8 *floatTmp = (u8*)destFloat;
	
	for (i = 0; i < 4; i++)
	{
		*(floatTmp + i) = *(srcByte + i);
	}
}

/*
* chang u8 buffer to int buffer
* IN paramer u8 *srcBuffer 
* IN paramer uint32_t length
* INOUT int *destData
*/
void NFC_GetTheIntFliterParamerBuffer(u8 *srcBuffer, uint32_t length, int *destData)
{
	uint32_t i = 0;
	uint32_t j = 0;
	uint32_t k = 0;
	u8 tmpBuffer[4] = {0};
	
	for (i = 2,j = 0; i < length; i++)
	{
		if(j < 4)
		{
			tmpBuffer[j] = srcBuffer[i];
			j++;
		}
		
		if ((i - 1) % 4 == 0)
		{
			j = 0;
			ChangeByteToInt(tmpBuffer,&destData[k]);
			k++;
		}
	}
}

/*
* change byte to int 
* 85 92 56 00
*/
void ChangeByteToInt(u8 *srcByte, int *destInt)
{
	*destInt = (int)((srcByte[0] & 0xFF)
							| ((srcByte[1] & 0xFF) << 8)	
							| ((srcByte[2] & 0xFF) << 16)
							| ((srcByte[3] & 0xFF) << 24));
}

#if 0
//old data formate
//no use
void NFC_DataChangeToFloat(u8 *srcBuffer, uint32_t length, float *destData)
{
	uint32_t i = 0;
	uint32_t j = 1;
	uint32_t floatFlag = 0;
	
	for (i = 1; i < length; i++ )
	{
		if (srcBuffer[i] == 0x2E && floatFlag == 0)
		{
			floatFlag = 1;
		}
		else if(srcBuffer[i] != 0x2E && floatFlag == 0)
		{
			*destData += (float)srcBuffer[i];			
		}
		else
		{
			*destData += (float)(srcBuffer[i])/myPow(10,j);
			j++;
		}
	}
}

/*
* powers of ten
*/
uint32_t myPow(uint32_t a, uint32_t n)
{
	uint32_t ret = 1;
	uint32_t i = 0;
	for (i = 0; i < n; i++)
	{
		ret *= 10; 
	}
	return ret;
}
#endif


/*
* process mesg from NFC card
*/
void NFC_MsgProcess(void)
{
		u8 msgType = 0;
		uint32_t i = 0;
	
		/*have data send from card*/
		if (recvMsgFlag == TRUE)
		{
			/*get msg type for different active*/
			msgType = GetMsgType(recvbuf);
			/*copy the data from handler FIFO to self buffer*/
			for(i = 0; i < recvbuf.msgLen; i++)
			{
				recvBuffer[i] = recvbuf.msgData[i];
			}
			
			/*clean the handler FIFO*/
			recvBufLen = recvbuf.msgLen;
			recvbuf.msgFlag = NO_MESSAGE_GET_FLAG;
			recvbuf.totleLen = 0;
			memset(recvbuf.msgData,0,recvbuf.msgLen);
		
			/*MSG from card respone when find phone near*/
			if (phoneFindFlag == 1)
			{
				if(msgType == 0xFE)
				{
					/*first time to send datas request*/
					LED0(1);
					NFC_Get_Phone_data(0);
				}
				else
				{
					LED1(1);
				}
				phoneFindFlag = FALSE;
			}
			
			/*MSG from phone*/
			if (msgType == 0x17)
			{
				NFC_GetThefloatFliterParamerBuffer(recvBuffer, recvBufLen, (float* )&fliterInit_Float[fliterBufferIndex]);
				NFC_Get_Phone_data(1);
				LED1(0);
				if (fliterChangeFlag != TRUE)
				{
					fliterChangeFlag = TRUE;
					phoneCardFlag = TRUE;
				}

			}

			/*commom msg from card */
			if (msgType == 0x01)
			{
				LED1(0);
			}
			
			recvMsgFlag= FALSE;
		}
		
		if (phoneCardFlag == 0)
		{
			if (timerCount == NFC_TICK)
			{
				phoneFindFlag = TRUE;
				NFC_Get_Phone_Card();
				timerCount = 0;
			}
			else
			{
				timerCount++;
			}
		}

}

