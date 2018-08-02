#ifndef _NFC_H
#define _NFC_H
#include "sys.h"
#include "usart.h"
#include "string.h"

/*it maybe replace by the timer,because now the delay isn't precise*/
#define NFC_TICK 500000   
#define FLITERPARAMERSIZE 32
#define FLITERBUFFERINDEX 16

#define PHONECARDMARK 0xAA



extern u8 activeCPUcmd[3];
extern u8 DataCmd0[14];
extern float fliterParameter[6];
extern bool phoneCardFlag;
extern bool phoneFindFlag;

extern float fliterInit_Float[16];
extern int fliterParameter_Int[FLITERBUFFERINDEX][FLITERPARAMERSIZE];
extern float fliterParameter_Float[FLITERBUFFERINDEX][FLITERPARAMERSIZE];

extern float left_diff[8];
extern float right_diff[8];
	
extern int DBDifference;
extern bool fliterChangeFlag;

/*
* the NFC device return check
* no use
*/
uint32_t NFC_Card_check(u8 passwd);

/*
* NFC search phone card
* send command for activing the CPU card reader function
* maybe need some delay for NFC device return checksum
*/
void NFC_Get_Phone_Card(void);

/*
* get the msg type for distinguishing msg function
*/
u8 GetMsgType(RECV_FIFO msg);


void NFC_Get_Phone_data(u8 order);

/*
* NFC device init
* we need to close the auto card searching function for phone card using
* flow: 
* 1. comfirm auto card searching function is closed
* 2. send "0xAA 0x01 0x15" to active the NFC device for phone card, return's "0xAA 0x01 0xFE" is actived successfully
* 3. send "AID" for taking data cmd
*/
void NFC_init(void);
#if 0
void NFC_DataChangeToFloat(u8 *srcBuffer, uint32_t length, float* destData);
uint32_t myPow(uint32_t a, uint32_t n);
#endif

/*
* process mesg from NFC card
*/
void NFC_MsgProcess(void);

void ChangeByteToFloat(u8 *srcByte, float *destFloat);

/*
* change byte to int 
*/
void ChangeByteToInt(u8 *srcByte, int *destInt);

void NFC_GetTheIntFliterParamerBuffer(u8 *srcBuffer, uint32_t length, int *destData);
void NFC_GetThefloatFliterParamerBuffer(u8 *srcBuffer, uint32_t length, float *destData);
#endif

