#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "TIM2_PWM_CNT_Up.h"
#include "TIM3_PWM_CNT_Down.h"
#include "TIM1_PWM_CNT_Center.h"
#include "IC.h"

int main(void)
{
	/*模块初始化*/
	OLED_Init();		//OLED初始化
	
	TIM2_PWM_Init();	//TIM2-CH2-PA1
	//TIM3_PWM_Init();	//TIM3-CH1-PA6
	TIM1_PWM_Init();	//TIM1-CH1-PA8
	
	IC_Init();		//输入捕获初始化，测量TIM2时用TIM3-CH1-PA6输入捕获；测量TIM3时用TIM2-CH1-PA0输入捕获；测量TIM1时哪个都行。
	
	/*显示静态字符串*/
	OLED_ShowString(1, 1, "Freq:00000Hz");		//1行1列显示字符串Freq:00000Hz
	OLED_ShowString(2, 1, "Duty:00%");			//2行1列显示字符串Duty:00%
	
	while (1)
	{
		OLED_ShowNum(1, 6, IC_GetFreq(), 5);	//不断刷新显示输入捕获测得的频率
		OLED_ShowNum(2, 6, IC_GetDuty(), 2);	//不断刷新显示输入捕获测得的占空比
	}
}
