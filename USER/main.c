#include "sys.h"
#include "main.h"

u8 sairecbuf1[SAI_RX_DMA_BUF_SIZE];
u8 sairecbuf2[SAI_RX_DMA_BUF_SIZE];
u8 saiplaybuf1[SAI_RX_DMA_BUF_SIZE];
u8 saiplaybuf2[SAI_RX_DMA_BUF_SIZE];

float l2l[256];
float r2r[256];

float left_filter_sum = 0;
float right_filter_sum = 0;
	
static float banks[8] = {175, 250, 500, 1000, 2000, 4000, 8000, 11200};	

u16 ptr = 0;
u8 store_ptr = 0;

float float_left[BLOCK_SIZE];
float float_right[BLOCK_SIZE];
float float_left_output[BLOCK_SIZE];
float float_right_output[BLOCK_SIZE];

static float lFirStateF32[BLOCK_SIZE + NUM_TAPS - 1];
static float rFirStateF32[BLOCK_SIZE + NUM_TAPS - 1];
unsigned int blockSize = BLOCK_SIZE;


int16_t tempShort;
int64_t tempLong;

float gVolumeRatio;
float tempFloat = 0;
int16_t tempInt;
float tempFloatLeft;
float tempFloatRight;
int ptr2;
int j;

u8 intFlag = 0;

arm_fir_instance_f32 SL;
arm_fir_instance_f32 SR;
float *lInputF32, *lOutputF32, *rInputF32, *rOutputF32;

extern UART_HandleTypeDef UART2_Handler;

uint32_t orderFlag = 0;
uint32_t processFlag = 0;

uint32_t WorkMode = DEVICE_WORK_NOFLITER;
float gDiffMean = 0;
float dbDiff = 0;
uint32_t volumControl = 20;
float preScale = 0;
float postScale = 1.0f;

int64_t sumTmp = 0, subTmp = 0, sumTmpMeam = 0, subTmpMeam = 0;

u32 beginFlag = 0;
u32 endFlag = 0;
u32 beginIndex = 0;
u32 endIndex = 0;
u32 transFlag = 0;
float *recvAudioBuf;
u32 recvIndex = 0;
u32 recvIndexMax = 0;
int64_t *alTmpbuf; 
int32_t alIndex = 0;
int IndexNum = 0;
u32 dataRecvFlag = 0;
uint64_t dataRecvMaxSample = 0;
uint64_t dataRecvMidSample = 0;
int8_t ByteDest[4];
float filterParamers[6] = {0};
float ParamerData[8] = {0};//{0,0,0,0,5,10,20,20};;
float defaultParams[8] = {0,0,0,0,5,10,20,20};
bool filterChangeFlag1 = FALSE;

int ParamersUpdataFlag = 0;

u32 longkey = 0;

#define _ON 0
#define _OFF 1
#define _SHORTKEY 0
#define _LONGKEY 1
#define _KEYNULL 2

u32 keybuf = 0;
u32 key_handle = 0;
u32 key_status = 0;

u32 button_status = 0;

/*
* main function
*/
int main(void)
{
//	float data[10];
//	float rdata[10];
	
	SystemAndHardwareInit();
	preScale = pow(10, -STD_LESS_DB * 0.05);
	getDefaultFilterParamer();
	FliterInit();
	printf("HELLO INCUS!\n\r");
	
	//env_test();
//	for (int i = 0; i < 10; i++)
//	{
//		data[i] = i + 1;
//	}
//	env_get(rdata, 10);
//	printf("-----------------------\n\r");
//	env_set(data, 10);
//	env_get(rdata, 10);
//	while (1)
//	{
//		if (WorkMode == DEVICE_WORK_CONFIGURATION)
//		{
//			env_get(rdata, 10);
////			for (int i = 0;i < 10;i++)
////			{
////				printf("rdata[%d] = %d\n\r",i,rdata[i]);
////			}
//		}
//		else
//		{
//			//printf("WorkMode = %d\n\r",WorkMode);
//			for (int i = 0; i < 10; i++)
//			{
//				data[i] = (i+1) * 10;
//			}
//			env_set(data, 10);
//			delay_ms(3000);
//			WorkMode = DEVICE_WORK_CONFIGURATION;
//		}
//	}
	while (1)
	{
		//printf("WorkMode is %d\n\r",WorkMode);
		scan_Key();
		apps();
	}
}

/*
* System init : clock, cache(Dcache and Icache), hal library
* hardware init : delay timer, uart, LED, codec(wm8978), NFC, button
*/
void SystemAndHardwareInit(void)
{
	Cache_Enable();
	HAL_Init();
	Stm32_Clock_Init(432, 25, 2, 9);
	
	delay_init(216);
	uart_init(9600);
	LED_Init();	
	
	WM8978_Init();
	wav_recorder();
	
  Button_Init();
}


void getDefaultFilterParamer(void)
{
	for (int i = 0; i < 8; i++)
	{
		ParamerData[i] = defaultParams[i];
	}
}

/*
* fliter init
*/
void FliterInit(void)
{
	//updataFliterParamer();

	eqfilter(NUM_TAPS, SampleRate, banks, ParamerData, 8, l2l);
	arm_copy_f32(l2l, r2r, NUM_TAPS);
	
  /* Initialize input and output buffer pointers */
  lInputF32 = &float_left[0];
  lOutputF32 = &float_left_output[0];
  rInputF32 = &float_right[0];
  rOutputF32 = &float_right_output[0];
	
  /* Call FIR init function to initialize the instance structure. */
  arm_fir_init_f32(&SL, NUM_TAPS, (float *)&l2l[0], &lFirStateF32[0], blockSize);
  arm_fir_init_f32(&SR, NUM_TAPS, (float *)&r2r[0], &rFirStateF32[0], blockSize);

	getVolumeDifference();
	SetCodecVolume(63,63);
}

/*
* caculate the volume diff produced by the filter 
*/
void getVolumeDifference(void)
{
	double beforeEnergy = 0;
	
	double afterEnergy = 0;

	for(int i = 0; i < 40; i++)
	{
		for(int j = 0; j < blockSize; j ++)
		{
			tempFloat = (float)WHITE[i * blockSize + j];
			float_left[j] = tempFloat;
			beforeEnergy += pow(tempFloat, 2);
		}
		
		arm_fir_f32(&SL, lInputF32, lOutputF32, blockSize);
    //arm_fir_f32(&SR, rInputF32, rOutputF32, blockSize);

		for(int j = 0; j < blockSize; j++)
		{
			tempFloat = float_left_output[j];
			afterEnergy += pow(tempFloat, 2);
		}
	}
	gVolumeRatio = (float)sqrt(beforeEnergy  / afterEnergy);
}

/*
* separate the recv buffer to left and right buffer for creating a filter 
*/
void updataFliterParamer(void)
{
	for(int i = 1; i < 7; i++)
	{		
		ParamerData[i] = filterParamers[i - 1];
	}
	ParamerData[0] = ParamerData[1];
	ParamerData[7] = ParamerData[6];
}

/*
* run the main task here
* do audio data process and NFC configuration here
*/
void apps(void)
{
	switch(WorkMode)
	{
		case DEVICE_WORK_NOFLITER:
			LED0(1);
			LED1(0);
			AudioProcessWithoutFliter();
			break;
		case DEVICE_WORK_CONFIGURATION:
			LED0(0);
			LED1(0);
			Toggle_Leds(0);
			memset(saiplaybuf1,0,SAI_RX_DMA_BUF_SIZE);
			memset(saiplaybuf2,0,SAI_RX_DMA_BUF_SIZE); 
//			FliterInit();
//			WorkMode = DEVICE_WORK_WITH_FLITER;		
			AudioTransDataProcess();
			if (filterChangeFlag1 == TRUE)
			{
				updataFliterParamer();
				env_set(ParamerData, 8);
				ParamersUpdataFlag = 1;
				printf("ParamersUpdataFlag = %d\n\r",ParamersUpdataFlag);
				FliterInit();
				filterChangeFlag1 = FALSE;
  			WorkMode = DEVICE_WORK_WITH_FLITER;
				LED0(0);
			}
			break;
		case DEVICE_WORK_WITH_FLITER:
			LED0(0);
			LED1(1);
			AudioProcessWithFliter();
			break;
		case DEVICE_WORK_WITH_DEFAULT_FILTER:
			LED1(1);
			LED0(1);
			if (ParamersUpdataFlag == 1)
			{
				printf("using updata paramers\n\r");
				env_get(ParamerData,8);
				FliterInit();
				ParamersUpdataFlag = 0;
			}
			AudioProcessWithFliter();
			break;
		default:
			break;
	}
}

void AudioProcessWithoutFliter(void)
{
		if(intFlag&0x01)
		{
			ptr = 0;
			while(ptr<(SAI_RX_DMA_BUF_SIZE)){
				
				tempShort = (sairecbuf1[ptr+1] << 8) & 0xff00;
				tempShort |= (sairecbuf1[ptr]) & 0x00ff;
				tempShort = (int16_t)(tempShort * preScale);
				saiplaybuf1[ptr] = tempShort & 0xff;
				saiplaybuf1[ptr+1] = (tempShort >> 8) & 0xff;
				
				tempShort = (sairecbuf1[ptr+3] << 8) & 0xff00;
				tempShort |= (sairecbuf1[ptr + 2]) & 0x00ff;
				tempShort = (int16_t)(tempShort * preScale);
				saiplaybuf1[ptr + 2] = tempShort & 0xff;
				saiplaybuf1[ptr+3] = (tempShort >> 8) & 0xff;
				
				ptr+=4;
			}
			intFlag&=0xFE;
	
		}
	
		//Process the second part of Data
		if(intFlag&0X04)
		{
			ptr = 0;
			while(ptr<(SAI_RX_DMA_BUF_SIZE)){
				
				tempShort = (sairecbuf2[ptr+1] << 8) & 0xff00;
				tempShort |= (sairecbuf2[ptr]) & 0x00ff;
				tempShort = (int16_t)(tempShort * preScale);
				saiplaybuf2[ptr] = tempShort & 0xff;
				saiplaybuf2[ptr+1] = (tempShort >> 8) & 0xff;
				
				tempShort = (sairecbuf2[ptr+3] << 8) & 0xff00;
				tempShort |= (sairecbuf2[ptr + 2]) & 0x00ff;
				tempShort = (int16_t)(tempShort * preScale);
				saiplaybuf2[ptr + 2] = tempShort & 0xff;
				saiplaybuf2[ptr+3] = (tempShort >> 8) & 0xff;
		
				ptr+=4;
			}
			intFlag&=0xFB;
			
		}
}


void AudioProcessWithFliter(void)
{
		//Process the first part of Data
		if (intFlag & 0x01)
		{
			for (ptr = 0; ptr < SAI_RX_DMA_BUF_SIZE; ptr += 4)
      {
				tempShort = (sairecbuf1[ptr + 1] << 8) & 0xff00;
        tempShort |= sairecbuf1[ptr] & 0x00ff;
        float_left[ptr / 4] = tempShort * preScale;

        tempShort = (sairecbuf1[ptr + 3] << 8) & 0xff00;
        tempShort |= sairecbuf1[ptr + 2] & 0x00ff;
        float_right[ptr / 4] = tempShort * preScale;
      }

      arm_fir_f32(&SL, lInputF32, lOutputF32, blockSize);
      arm_fir_f32(&SR, rInputF32, rOutputF32, blockSize);
						
      for (ptr = 0; ptr < SAI_RX_DMA_BUF_SIZE; ptr += 4)
      {
        tempLong = (int64_t)(float_left_output[ptr / 4]);
				
				tempLong = (int64_t)(tempLong * gVolumeRatio);
				tempFloat = tempLong * 1.0f / INT16_MAX;

				tempShort = (int16_t)(tempFloat * INT16_MAX);
        saiplaybuf1[ptr + 1] = (u8)((tempShort >> 8) & 0x00FF);
        saiplaybuf1[ptr] = (u8)((tempShort)&0x00FF);

        tempLong = (int64_t)(float_right_output[ptr / 4]);
				
				tempLong = (int64_t)(tempLong * gVolumeRatio);
				tempFloat = tempLong * 1.0f / INT16_MAX;

				tempShort = (int16_t)(tempFloat * INT16_MAX);
        saiplaybuf1[ptr + 3] = (u8)((tempShort >> 8) & 0x00FF);
        saiplaybuf1[ptr + 2] = (u8)((tempShort)&0x00FF);
      }
			intFlag &= 0xFE;
		}

		//Process the second part of Data
		if (intFlag & 0X04)
		{
      for (ptr = 0; ptr < SAI_RX_DMA_BUF_SIZE; ptr += 4)
      {
				tempShort = (sairecbuf2[ptr + 1] << 8) & 0xff00;
        tempShort |= sairecbuf2[ptr] & 0x00ff;
        float_left[ptr / 4] = tempShort * preScale;

        tempShort = (sairecbuf2[ptr + 3] << 8) & 0xff00;
        tempShort |= sairecbuf2[ptr + 2] & 0x00ff;
        float_right[ptr / 4] = tempShort * preScale;
      }

      arm_fir_f32(&SL, lInputF32, lOutputF32, blockSize);
      arm_fir_f32(&SR, rInputF32, rOutputF32, blockSize);
      for (ptr = 0; ptr < SAI_RX_DMA_BUF_SIZE; ptr += 4)
      {
        tempLong = (int64_t)(float_left_output[ptr / 4]);
				
				tempLong = (int64_t)(tempLong * gVolumeRatio);
				tempFloat = tempLong * 1.0f / INT16_MAX;

				tempShort = (int16_t)(tempFloat * INT16_MAX);
        saiplaybuf2[ptr + 1] = (u8)((tempShort >> 8) & 0x00FF);
        saiplaybuf2[ptr] = (u8)((tempShort)&0x00FF);

        tempLong = (int64_t)(float_right_output[ptr / 4]);
				
				tempLong = (int64_t)(tempLong * gVolumeRatio);
				tempFloat = tempLong * 1.0f / INT16_MAX;

				tempShort = (int16_t)(tempFloat * INT16_MAX);
        saiplaybuf2[ptr + 3] = (u8)((tempShort >> 8) & 0x00FF);
        saiplaybuf2[ptr + 2] = (u8)((tempShort)&0x00FF);
      }
			intFlag &= 0xFB;
		}
}

/**
  * @brief  Toggle LEDs to show user input state.   
  * @param  None
  * @retval None
  */
void Toggle_Leds(int led)
{
  static uint32_t ticks;
  
  if(ticks++ == 0xfffff)
  {
		if(led == 0)
		{
			LED0_Toggle;
		}
		else
		{
			LED1_Toggle;
		}
    
    ticks = 0;
  }
}

void SetCodecVolume(uint32_t leftVol, uint32_t rightVol)
{
	WM8978_HPvol_Set(leftVol,rightVol);
}

/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
//  if(GPIO_Pin == WAKEUP_BUTTON_PIN)
//  {  
//		if (WorkMode == DEVICE_WORK_NOFLITER)
//		{
//			dataRecvFlag = 0;
//			WorkMode = DEVICE_WORK_CONFIGURATION;
//		}
//		else
//		{
//			WorkMode = DEVICE_WORK_NOFLITER;
//		}
//		//key_status = 1;
//  }
	if (GPIO_Pin == WAKEUP_BUTTON_PIN)
	{
		memset(saiplaybuf1,0,SAI_RX_DMA_BUF_SIZE);
		memset(saiplaybuf2,0,SAI_RX_DMA_BUF_SIZE);	
		if (button_status == 0)
		{
			button_status = 1;
		}
		else if (button_status == 1)
		{
			button_status = 2;
		}
	}
}

void scan_Key(void)
{
	if (button_status == 1)
	{
		memset(saiplaybuf1,0,SAI_RX_DMA_BUF_SIZE);
		memset(saiplaybuf2,0,SAI_RX_DMA_BUF_SIZE);		
		longkey++;
		delay_ms(5);		
	}
	else if (button_status == 2)
	{
		memset(saiplaybuf1,0,SAI_RX_DMA_BUF_SIZE);
		memset(saiplaybuf2,0,SAI_RX_DMA_BUF_SIZE);			
		if (longkey >= 3 && longkey <= 100)
		{
			keybuf = _SHORTKEY;
			longkey = 0;
			if (WorkMode == DEVICE_WORK_NOFLITER)
			{
				WorkMode = DEVICE_WORK_WITH_DEFAULT_FILTER;			
			}
			else if (WorkMode == DEVICE_WORK_WITH_DEFAULT_FILTER)
			{
				WorkMode = DEVICE_WORK_NOFLITER;
			}
			else
			{
				WorkMode = DEVICE_WORK_NOFLITER;
			}
		}
		else if (longkey > 100)
		{		
			memset(saiplaybuf1,0,SAI_RX_DMA_BUF_SIZE);
			memset(saiplaybuf2,0,SAI_RX_DMA_BUF_SIZE);				
			keybuf = _LONGKEY;
			longkey = 0;		
			dataRecvFlag = 0;
			WorkMode = DEVICE_WORK_CONFIGURATION;			
		}
		else
		{
			longkey = 0;			
		}
		button_status = 0;
	}
}

