#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "MyDMA.h"

//uint8_t aa = 0x66;
const uint8_t aa = 0x66;				

int main(void)
{
	/*OLED初始化*/
	OLED_Init();				

	/*显示静态字符串*/
	OLED_ShowString(1, 1, " aa:0x");
	OLED_ShowString(2, 1, "&aa:0x");	
	
	/*显示数组的首地址*/
	OLED_ShowHexNum(1, 7, aa, 2);             // OLED显示  aa:0x66
	OLED_ShowHexNum(2, 7, (uint32_t)&aa, 8);  // OLED显示 &aa:0x20000000
	
	while (1)
	{
	}
}
