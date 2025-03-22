#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "TIM1_6signals.h"

int main(void)
{
	/*模块初始化*/
	OLED_Init();		    //OLED初始化
	TIM1_6signals_Init();	//PWM初始化
	
	/*显示静态字符串*/
	OLED_ShowString(1, 1, "Outputing");		//1行1列显示字符串Outputing
	
	while (1){
	}
}
