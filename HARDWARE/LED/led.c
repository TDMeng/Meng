#include "led.h"

void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_Initure;
  __HAL_RCC_GPIOB_CLK_ENABLE();	
	
  GPIO_Initure.Pin=GPIO_PIN_1; 
  GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP; 
  GPIO_Initure.Pull=GPIO_PULLUP;       
  GPIO_Initure.Speed=GPIO_SPEED_HIGH;   
  HAL_GPIO_Init(GPIOB,&GPIO_Initure);
	
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET);	

	__HAL_RCC_GPIOA_CLK_ENABLE();	
	
  GPIO_Initure.Pin=GPIO_PIN_7; 
  GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP; 
  GPIO_Initure.Pull=GPIO_PULLUP;       
  GPIO_Initure.Speed=GPIO_SPEED_HIGH;   
  HAL_GPIO_Init(GPIOA,&GPIO_Initure);
	
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);	
}


