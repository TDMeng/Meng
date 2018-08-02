#ifndef _AUDIODATAtRANSpROCESS_H
#define _AUDIODATAtRANSpROCESS_H
#include "sys.h"
#include "usart.h"
#include "recorder.h"
#include "string.h"
#include "arm_math.h"

typedef struct Part
{
	uint32_t member;
}PART;

typedef enum
{
	FLAG_START = 0,
	FLAG_ING,
	FLAG_END
}FLAGSTATUS;


void AudioTransDataProcess(void);
void FFTDataAnalysis(void);
void ProcessDataStream(int dataIn);
void AnalysisBuffer(void);
u8 ChangeBitToByte(u32 *src);
void TransByteToFloat(u8 *srcByte, float *destFloat);
int dataCheckSum(int8_t *src);
#endif


