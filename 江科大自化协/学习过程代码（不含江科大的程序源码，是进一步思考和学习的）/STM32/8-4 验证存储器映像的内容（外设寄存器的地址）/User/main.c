#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "MyDMA.h"

#define ADC1_DR ((uint32_t *)0x4001244C)

int main(void)
{
	/*OLED初始化*/
	OLED_Init();				

	/*显示静态字符串*/
	OLED_ShowString(1, 1, "Add:0x");	 // 地址
	OLED_ShowString(2, 1, "Val:0x");	 // 值	
	
	/*显示外设寄存器的地址*/
	//OLED_ShowHexNum(1, 7, (uint32_t)&ADC1->DR, 8);      // 如果不加(uint32_t)强制类型转换，就是指针跨级赋值了，编译器会警告
	//OLED_ShowHexNum(1, 7, (uint32_t)(&(ADC1->DR)), 8);  // 上方看不懂，写成这样，(uint32_t)&ADC1->DR 等同于 (uint32_t)(&(ADC1->DR))，加上括号更方便看出来符号优先级
	OLED_ShowHexNum(1, 7, (uint32_t)ADC1_DR, 8);   

	/*显示外设寄存器中存储的值*/	
	//OLED_ShowHexNum(2, 7, ADC1->DR, 8);
	OLED_ShowHexNum(2, 7, *ADC1_DR, 8);            // *：取地址中的值	
	
	while (1)
	{
	}
}
