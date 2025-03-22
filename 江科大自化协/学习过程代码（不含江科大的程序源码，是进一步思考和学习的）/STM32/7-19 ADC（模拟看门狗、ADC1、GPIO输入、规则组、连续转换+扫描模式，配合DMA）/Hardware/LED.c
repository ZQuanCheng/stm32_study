#include "stm32f10x.h"                  // Device header

/**
  * 函    数：LED初始化
  * 参    数：无
  * 返 回 值：无
  */
void LED_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);		//开启GPIOC的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);		
	
	/*设置GPIO初始化后的默认电平*/
	GPIO_SetBits(GPIOC, GPIO_Pin_13);				//设置PC13引脚为高电平
}

/**
  * 函    数：LED0开启
  * 参    数：无
  * 返 回 值：无
  */
void LED0_ON(void)
{
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);		//设置PC13引脚为低电平
}

/**
  * 函    数：LED0关闭
  * 参    数：无
  * 返 回 值：无
  */
void LED0_OFF(void)
{
	GPIO_SetBits(GPIOC, GPIO_Pin_13);		//设置PC13引脚为高电平
}

/**
  * 函    数：LED0状态翻转
  * 参    数：无
  * 返 回 值：无
  */
void LED0_Turn(void)
{
	if (GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13) == 0)		//获取输出寄存器的状态，如果当前引脚输出低电平
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_13);					//则设置PC13引脚为高电平
	}
	else													//否则，即当前引脚输出高电平
	{
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);					//则设置PC13引脚为低电平
	}
}
