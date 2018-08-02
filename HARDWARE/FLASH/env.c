#include "stm32f7xx_hal.h"
#include "env.h"
#include "libg.h"
#include "button.h"
//#include "main.h"

extern uint32_t WorkMode;

/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base address of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base address of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base address of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base address of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base address of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base address of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base address of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base address of Sector 7, 128 Kbytes */

#define FLASH_USER_START_ADDR   ADDR_FLASH_SECTOR_7  /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     (ADDR_FLASH_SECTOR_7)   /* End @ of user Flash area */
/**
  * @brief  Gets the sector of a given address
  * @param  None
  * @retval The sector of a given address
  */
uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;

  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_SECTOR_0;
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_SECTOR_1;
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_SECTOR_2;
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_SECTOR_3;
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_SECTOR_4;
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_SECTOR_5;
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_SECTOR_6;
  }
  else
  {
    sector = FLASH_SECTOR_7;
  }
 
  return sector;
}

int env_init(void)
{
	return ENV_OK;
}

int env_get(float *data, int len)
{
  uint32_t Address = FLASH_USER_START_ADDR;
	int i = 0;

	for(i = 0; i < len; i++)
  {
    data[i]  = *(__IO uint32_t *)Address;
		libg_log_debug("%u read : %f\n", Address, data[i]);
    Address = Address + 4;
  }

	return ENV_OK;
}

int env_set(float *data, int len)
{
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t FirstSector = 0;
	uint32_t NbOfSectors = 0;
	uint32_t SectorError;

	uint32_t Address = FLASH_USER_START_ADDR;
	printf("[env_set-----]Address = %d\n\r",Address);
  HAL_FLASH_Unlock();

  FirstSector = GetSector(FLASH_USER_START_ADDR);
  NbOfSectors = GetSector(FLASH_USER_END_ADDR) - FirstSector + 1;

  EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
  EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
  EraseInitStruct.Sector        = FirstSector;
  EraseInitStruct.NbSectors     = NbOfSectors;

  if(HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK)
  {
		libg_log_error("HAL_FLASHEx_Erase failed: %u\n", HAL_FLASH_GetError());
		return ENV_ERROR;
  }
	

	for(int i = 0; i < len; i++)
  {
		libg_log_debug("[env_set]%u write : %f\n", Address, data[i]);
    if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, data[i]) == HAL_OK)
    {
      Address = Address + 4;
			//printf("[env_set]Address = %d\n\r",Address);		
    }
		else
    {
			libg_log_error("HAL_FLASH_Program failed: %u\n", HAL_FLASH_GetError());
    }
  }

  HAL_FLASH_Lock();

	return ENV_OK;
}

int env_test(void)
{
	float data[10];
	float rdata[10];
	
	int i = 0;
	uint32_t j = 0;
	int cnt = 0;
		
	env_set(data, 10);	
	while(true)
	{
		for(i = 0; i < 10; i++)
		{
			data[i] = j;
			j++;
		}
		
		env_set(data, 10);
		env_get(rdata, 10);
		
		for(i = 0; i < 10; i++)
		{
			if(data[i] != rdata[i])
			{
				libg_log_error("error: %f/%f\n", data[i], rdata[i]);
			}
		}
			
		libg_log_info("cnt: %d\n", ++cnt);
		HAL_Delay(100);
	}
}
