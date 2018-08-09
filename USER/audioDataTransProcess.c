#include "audioDataTransProcess.h"

#define SAMPLEBUS  10

#define FFTOUTSIZE 1024
#define FFTINSIZE  2048
#define FREQMAX    44100
#define EXCEPTION  4000
#define FFTBUFFERSIZE 2048
#define FFTOUTSIZE  1024
#define ENERGYSTEP 44
#define NOISENORM  2500000
#define HEADRMF    30000000
	
	
int StartFreq = 0;
int EndFreq = 0;
int FreqStep = 0;
int DataLength = 0;	
int DataStep = 0;	

extern u8 sairecbuf1[];
extern u8 sairecbuf2[];
extern u8 saiplaybuf1[];
extern u8 saiplaybuf2[];	
extern u8 intFlag;
extern u16 ptr;
extern int16_t tempInt;
extern int IndexNum;
extern int8_t ByteDest[4];

extern float *recvAudioBuf;	
float *testdata; 	
int testIndex = 0;
extern int recvIndex; 
	
PART listers[5];
int listIndex = 0;
int hIndex = 0;
int pdex = 0;
int count = 0;
int DataStartFlag = 0;
uint64_t alTmp = 0;
uint64_t pubRecvBuf[SAMPLEBUS] = {0};
extern uint64_t dataRecvMaxSample;
extern uint64_t dataRecvMidSample;

u32 Bits[32];
extern float filterParamers[6];
extern bool filterChangeFlag1;

//flag
extern int dataRecvFlag;
int headFlag = 0;
int tailFlag = 0;

float leftData;
float rightData;
float FFTInputBuf[FFTINSIZE] = {0};
float FFTOutputBuf[FFTINSIZE] = {0};
float testOutput[FFTOUTSIZE];
int paramerIndex = 0;


/*
process the audio stream and process the data for analysis the main data for filter paramers
*/
void AudioTransDataProcess(void)
{
	if (intFlag & 0x01)
	{
		ptr = 0;
		while (ptr < SAI_RX_DMA_BUF_SIZE)
		{
			tempInt = (sairecbuf1[ptr + 1] << 8) & 0xff00;
			tempInt |= sairecbuf1[ptr] & 0x00ff;
			
			leftData = (float)tempInt;

			if ((headFlag == 0 && tailFlag == 0 ) || 
					(headFlag == 2 && tailFlag == 0) || 
					(headFlag == 1 && tailFlag == 2)	||
					(headFlag == 1 && tailFlag == 0))
			{
				if (headFlag == 1 && tailFlag == 0)
				{
					if(recvIndex < 4000)
					{
						recvAudioBuf[recvIndex] = leftData;
						recvIndex++;					
					}
					else
					{
						memset(recvAudioBuf, 0, sizeof(float)* (recvIndex-1));
						printf("error!\n\r");
						headFlag = 0;
						tailFlag = 0;
						recvIndex = 0;
						dataRecvFlag = 0;
					}

				}	
				
				if (hIndex < 44)
				{
					alTmp += tempInt * tempInt;
					hIndex++;
				}
				else
				{
					if (dataRecvFlag == 0)
					{
						if (pdex < SAMPLEBUS)
						{
							if (30000000 < alTmp)
							{
								pubRecvBuf[pdex] = alTmp;
								pdex++;
							}
						}
						else
						{
							for (int i = 0; i < SAMPLEBUS; i++)
							{
								if (pubRecvBuf[i] / 2500000 < 10)
								{
									pdex--;
								}
							}
							if (pdex > 8)
							{
								dataRecvFlag = 1;
								pdex = 0;
								dataRecvMaxSample = pubRecvBuf[8] / 2500000;
							}
						}
					}
					else if (dataRecvFlag == 1)
					{
						alTmp = alTmp / 2000000;
						if (alTmp > dataRecvMaxSample)
						{
							DataStartFlag = 5;
						}
						else
						{
							DataStartFlag = 0;
						}
						
						ProcessDataStream(DataStartFlag);				
					}
					alTmp = 0;
					hIndex = 0;
				}					
			}		
			else if (headFlag == 1 &&  tailFlag == 1)
			{
				arm_copy_f32(recvAudioBuf,FFTInputBuf,2048);
				AnalysisBuffer();
				headFlag = 0;
				tailFlag = 0;
				recvIndex	= 0;			
			}
			ptr += 4;
		}
		intFlag &= 0xFE;
		
	}

	if (intFlag & 0x04)
	{
		ptr = 0;
		while (ptr < SAI_RX_DMA_BUF_SIZE)
		{
			tempInt = (sairecbuf2[ptr + 1] << 8) & 0xff00;
			tempInt |= sairecbuf2[ptr] & 0x00ff;
			
			leftData = (float)tempInt;
			

			if ((headFlag == 0 && tailFlag == 0 ) || 
					(headFlag == 2 && tailFlag == 0) || 
					(headFlag == 1 && tailFlag == 2) ||
					(headFlag == 1 && tailFlag == 0))
			{
				if (headFlag == 1 && tailFlag == 0)
				{
					if(recvIndex < 4000)
					{
						recvAudioBuf[recvIndex] = leftData;
						recvIndex++;					
					}
					else
					{
						memset(recvAudioBuf, 0, sizeof(float)* (recvIndex-1));
						printf("error!\n\r");
						headFlag = 0;
						tailFlag = 0;
						recvIndex = 0;
						dataRecvFlag = 0;
					}

				}	
				
				
				if (hIndex < 44)
				{
					alTmp += tempInt * tempInt;
					hIndex++;
				}
				else
				{
					if (dataRecvFlag == 0)
					{
						if (pdex < SAMPLEBUS)
						{
							if (30000000 < alTmp)
							{
								pubRecvBuf[pdex] = alTmp;
								pdex++;
							}
						}
						else
						{
							for (int i = 0; i < SAMPLEBUS; i++)
							{
								if (pubRecvBuf[i] / 2500000 < 10)
								{
									pdex--;
								}
							}
							if (pdex > 8)
							{
								dataRecvFlag = 1;
								pdex = 0;
								dataRecvMaxSample = pubRecvBuf[8] / 2500000;
							}
						}						
					}
					else if (dataRecvFlag == 1)
					{
						alTmp = alTmp / 2000000;
						if (alTmp > dataRecvMaxSample)
						{
							DataStartFlag = 5;
						}
						else
						{
							DataStartFlag = 0;
						}
						ProcessDataStream(DataStartFlag);						
					}
					alTmp = 0;
					hIndex = 0;
				}					
			}		
			else if (headFlag == 1 &&  tailFlag == 1)
			{
				arm_copy_f32(recvAudioBuf,FFTInputBuf,2048);	

				AnalysisBuffer();
				headFlag = 0;
				tailFlag = 0;
				recvIndex = 0;
			}
			ptr += 4;
		}
		intFlag &= 0xFB;
	}	
	
}

/*
do fft process with audio data from the app
*/
void AnalysisBuffer(void)
{
     arm_rfft_fast_instance_f32 S;
     uint8_t ifftFlag = 0;

     arm_rfft_fast_init_f32(&S, 1024);	
	
     arm_rfft_fast_f32(&S, FFTInputBuf, FFTOutputBuf, ifftFlag);

     arm_cmplx_mag_f32(FFTOutputBuf, testOutput, 1024);
		 FFTDataAnalysis();
}

/*
catch the head,end flag and data ,then get the data to fft process
*/
void ProcessDataStream(int dataIn)
{
	if (listIndex < 4)
	{
		listers[listIndex].member = dataIn;
		listIndex++;
	}
	else
	{
		listers[listIndex].member = dataIn;
		listIndex = 0;
	}

	if(dataIn == 5 && headFlag == 0)
	{
		headFlag = 2;
		count++;
	}
	else if (headFlag == 2 && dataIn == 5)
	{
		count++;
	}
	else if (headFlag == 2 && dataIn != 5)
	{
		printf("head count = %d\n\r",count);
		if (count < 5)
		{
			dataRecvFlag = 0;
			headFlag = 0;
		}
		else
		{
			headFlag = 1;
			tailFlag = 0;
			count = 0;
		}
	}
	
	if (headFlag == 1 && dataIn == 5 && tailFlag == 0)
	{
		tailFlag = 2;
		count++;
	}
	else if (headFlag == 1 && tailFlag == 2 && dataIn == 5)
	{
		count++;
	}
	else if (tailFlag == 2 && dataIn != 5)
	{
		if (count < 5)
		{
			tailFlag = 0;
			count = 0;
			headFlag = 0;
			dataRecvFlag = 0;
		}
		else
		{
			printf("end count = %d\n\r",count);
			tailFlag = 1;
		}
	}
		
}

/*
analysis the fft output data to bit
*/
void FFTDataAnalysis(void)
{
	int FreqHead = 0;
	int FreqTail = 0;
	StartFreq = 200;
	FreqStep = 300;
	DataLength = 32;
	EndFreq = StartFreq + FreqStep * (DataLength - 1);
	DataStep = (FFTOUTSIZE / 2) / DataLength;
	
	float MAX=0;
	for (int i = 0; i < 512; i ++)
	{
		MAX = testOutput[i] > MAX ? testOutput[i] : MAX;
	}

	float size = 44100 / 2 / 512;

	for (int i = 0; i < 512; i ++)
	{
		if (testOutput[i] > MAX / 4 && testOutput[i] > testOutput[i - 1] && testOutput[i] > testOutput[i + 1])
		{
			for (int j = 0; j < DataLength; j++)
			{
				FreqHead = StartFreq + FreqStep * j - FreqStep / 2;
				FreqTail = StartFreq + FreqStep * j + FreqStep / 2;
				
				float Start = (float)FreqHead / size;
				float End = (float)FreqTail / size;

				if (i > Start && i < End)
				{
					Bits[j] = 1;
					continue;
				}
			}
		}
	}

	memset(FFTOutputBuf, 0, 2048);
	memset(FFTInputBuf, 0,2048);
	memset(testOutput, 0, 1024);

	for (int i = 0; i < 4; i++)
	{
		ByteDest[i] = ChangeBitToByte(&Bits[i * 8]);
	}
	
	if (dataCheckSum(ByteDest) == 0 && paramerIndex < 6)
	{
		for (int i = 0; i < 3; i++)
		{
			filterParamers[paramerIndex] =  (float)ByteDest[i];
			paramerIndex++;
		}
		if (paramerIndex == 6)
		{
			filterChangeFlag1 = TRUE;
			paramerIndex = 0;
		}		
	}
	else
	{

		paramerIndex = 0;
	}
	memset(Bits,0, sizeof(int) * 32);
}

/*
check the data
*/
int dataCheckSum(int8_t *src)
{
	if ((src[0] | src[1] | src[2]) == src[3])
	{
		return 0;
	}
	else
	{
		return 1; 	
	}
}

/*
change the bit to byte
*/
u8 ChangeBitToByte(u32 *src)
{
	u32 tmp = 0;
	tmp = src[0] * 128 + src[1] * 64 + src[2] * 32 + src[3] * 16 + src[4] * 8 + src[5] * 4 + src[6] * 2 + src[7] * 1;
	return (u16)tmp;
}
