#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "TIM2_master.h"
#include "TIM3_slave_master.h"
#include "TIM1_slave.h"

int main(void)
{
	/*模块初始化*/
	OLED_Init();		//OLED初始化
	
	TIM2_PWM_Init();	//TIM2-CH2-PA1。频率1kHz、占空比50%
	TIM3_PWM_Init();	//TIM3-CH1-PA6。频率2kHz、占空比50%
	TIM1_PWM_Init();	//TIM1-CH1-PA8。频率10kHz、占空比50%
	
	//只需要使用软件触发来启动主定时器TIM2，TIM3由TIM2的TRGO触发启动，TIM1由TIM3的TRGO触发启动
	TIM_Cmd(TIM2, ENABLE);
	
	/*显示静态字符串*/
	OLED_ShowString(1, 1, "Outing");		//1行1列显示字符串Outing
	OLED_ShowString(2, 1, "TIM2_CH2_PA1");	//2行1列显示字符串TIM2_CH2_PA1
	OLED_ShowString(3, 1, "TIM3_CH1_PA6");	//3行1列显示字符串TIM3_CH1_PA6
	OLED_ShowString(4, 1, "TIM1_CH1_PA8");	//4行1列显示字符串TIM1_CH1_PA8
	
	while (1)
	{
	}
}
